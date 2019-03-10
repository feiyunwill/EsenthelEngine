/******************************************************************************/
#include "!Header.h"
#include "Simple.h"
#include "Ambient Occlusion.h"
#include "Vertex Fog.h"
/******************************************************************************/
#define PARAMS             \
   uniform Bool skin      ,\
   uniform Int  materials ,\
   uniform Int  textures  ,\
   uniform Int  bump_mode ,\
   uniform Bool alpha_test,\
   uniform Bool light_map ,\
   uniform Bool rflct     ,\
   uniform Bool color     ,\
   uniform Bool mtrl_blend,\
   uniform Bool heightmap ,\
   uniform Int  fx        ,\
   uniform Bool per_pixel ,\
   uniform Bool tesselate
/******************************************************************************/
struct VS_PS
{
   VecH  nrm     :TEXCOORD0; // !! may not be Normalized !!
   Vec2  tex     :TEXCOORD1;
   Vec2  tex_l   :TEXCOORD2;
   Vec   pos     :TEXCOORD3;
   Half  fade_out:TEXCOORD4;
   VecH  col_add :TEXCOORD5;
   Vec   rfl     :TEXCOORD6;
   VecH  col     :COLOR0   ;
   VecH4 material:COLOR1   ;
};
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out VS_PS O,
   out Vec4  O_vtx:POSITION,

   IF_IS_CLIP
   PARAMS
)
{
   Vec  pos=vtx.pos();
   VecH nrm; if(bump_mode>=SBUMP_FLAT)nrm=vtx.nrm();

   if(textures   )O.tex     =vtx.tex     (heightmap);
   if(light_map  )O.tex_l   =vtx.tex1    ();
   if(materials>1)O.material=vtx.material();

   if(materials<=1)O.col.rgb=MaterialColor3();/*else
   if(!mtrl_blend )
   {
                      O.col.rgb =O.material.x*MultiMaterial0Color3()
                                +O.material.y*MultiMaterial1Color3();
      if(materials>=3)O.col.rgb+=O.material.z*MultiMaterial2Color3();
      if(materials>=4)O.col.rgb+=O.material.w*MultiMaterial3Color3();
   }*/
   if(color)
   {
      if(materials<=1/* || !mtrl_blend*/)O.col.rgb*=vtx.color3();
      else                               O.col.rgb =vtx.color3();
   }

   if(heightmap && textures && materials==1)O.tex*=MaterialTexScale();

   if(fx==FX_LEAF)
   {
      if(bump_mode==SBUMP_FLAT)BendLeaf(vtx.hlp(), pos, nrm);
      else                     BendLeaf(vtx.hlp(), pos);
   }
   if(fx==FX_LEAFS)
   {
      if(bump_mode==SBUMP_FLAT)BendLeafs(vtx.hlp(), vtx.size(), pos, nrm);
      else                     BendLeafs(vtx.hlp(), vtx.size(), pos);
   }

   if(!skin)
   {
   #if MODEL>=SM_4 || MODEL==SM_GL
      if(true) // instance
      {
                                  O.pos=TransformPos(pos, vtx.instance());
         if(bump_mode==SBUMP_FLAT)O.nrm=TransformDir(nrm, vtx.instance());
         if(fx==FX_GRASS)
         {
              BendGrass(pos, O.pos, vtx.instance());
            O.fade_out=GrassFadeOut(vtx.instance());
         }
      }else
   #endif
      {
                                  O.pos=TransformPos(pos);
         if(bump_mode==SBUMP_FLAT)O.nrm=TransformDir(nrm);
         if(fx==FX_GRASS)
         {
            BendGrass(pos, O.pos);
            O.fade_out=GrassFadeOut();
         }
      }
   }else
   {
      VecI bone=vtx.bone();
                               O.pos=TransformPos(pos, bone, vtx.weight());
      if(bump_mode==SBUMP_FLAT)O.nrm=TransformDir(nrm, bone, vtx.weight());
   }

   // normalize
   if(bump_mode==SBUMP_FLAT && (tesselate || rflct || !per_pixel))O.nrm=Normalize(O.nrm);

   if(fx==FX_BONE)
   {
      Flt b; if(!skin)b=0;else
      {
         VecI bone=vtx._bone; // use real bone regardless if skinning is enabled
         b=vtx.weight().x*Sat(1-Abs(bone.x-BoneHighlight))
          +vtx.weight().y*Sat(1-Abs(bone.y-BoneHighlight))
          +vtx.weight().z*Sat(1-Abs(bone.z-BoneHighlight));
      }
      O.col=((b>EPS) ? Vec(b, 0, 1-b) : Vec(1, 1, 1));
   }

   if(rflct)O.rfl=Transform3(reflect(Normalize(O.pos), O.nrm), CamMatrix);

   if(!per_pixel)
   {
      if(!(materials<=1 /*|| !mtrl_blend*/ || color || fx==FX_BONE))O.col=1;

      // lighting
      if(bump_mode>=SBUMP_FLAT)
      {
         Half d  =Sat(Dot(O.nrm, Light_dir.dir));
         VecH lum=Light_dir.color.rgb*d + AmbColor;
         if(materials<=1 && fx!=FX_BONE)lum+=MaterialAmbient();
         O.col.rgb*=lum;
      }

      // fog
      Half fog_rev=Sat(Length2(O.pos)*VertexFogMulAdd.x+VertexFogMulAdd.y); // fog_rev=1-fog
      O.col.rgb*=                                        fog_rev ;          //       *=1-fog
      O.col_add =Lerp(VertexFogColor.rgb, Highlight.rgb, fog_rev);          //         1-fog
   }

   O_vtx=Project(O.pos); CLIP(O.pos);
}
/******************************************************************************/
Vec4 PS
(
   VS_PS I,

   IF_IS_FRONT

   PARAMS
):COLOR
{
   Half glow;

   if(fx==FX_BONE)
   {
      glow=0;
   }else
   {
      if(materials<=1)
      {
         if(textures==0)
         {
            glow=MaterialGlow();
         }else
         {
            VecH4 tex_nrm; // #MaterialTextureChannelOrder
            if(textures==1)
            {
               VecH4 tex_col=Tex(Col, I.tex); if(alpha_test)AlphaTest(tex_col.w, I.fade_out, fx);
               I.col.rgb*=tex_col.rgb;
               glow=MaterialGlow();
            }else
            if(textures==2)
            {
               /*if(per_pixel || alpha_test || rflct || is_glow)*/
               {
                  tex_nrm=Tex(Nrm, I.tex); if( alpha_test)AlphaTest(tex_nrm.a, I.fade_out, fx);
                  glow   =MaterialGlow() ; if(!alpha_test)glow*=tex_nrm.a;
               }//else glow=MaterialGlow(); always read 2nd texture to set glow, but in the future add a separate shader path that doesn't do this if material has no glow
               I.col.rgb*=Tex(Col, I.tex).rgb;
            }

            if(light_map)I.col.rgb*=Tex(Lum, I.tex_l).rgb;

            // reflection
            if(rflct)I.col.rgb+=TexCube(Rfl, I.rfl).rgb*((textures==2) ? MaterialReflect()*tex_nrm.z : MaterialReflect());
         }
      }else // materials>1
      {
         glow=0;
         VecH tex;
         if(mtrl_blend)
         {
            VecH4 col0, col1, col2, col3;
                             col0=Tex(Col , I.tex*MultiMaterial0TexScale()); col0.rgb*=MultiMaterial0Color3(); I.material.x=MultiMaterialWeight(I.material.x, col0.a);
                             col1=Tex(Col1, I.tex*MultiMaterial1TexScale()); col1.rgb*=MultiMaterial1Color3(); I.material.y=MultiMaterialWeight(I.material.y, col1.a); if(materials==2)I.material.xy  /=I.material.x+I.material.y;
            if(materials>=3){col2=Tex(Col2, I.tex*MultiMaterial2TexScale()); col2.rgb*=MultiMaterial2Color3(); I.material.z=MultiMaterialWeight(I.material.z, col2.a); if(materials==3)I.material.xyz /=I.material.x+I.material.y+I.material.z;}
            if(materials>=4){col3=Tex(Col3, I.tex*MultiMaterial3TexScale()); col3.rgb*=MultiMaterial3Color3(); I.material.w=MultiMaterialWeight(I.material.w, col3.a); if(materials==4)I.material.xyzw/=I.material.x+I.material.y+I.material.z+I.material.w;}
                            tex =I.material.x*col0.rgb
                                +I.material.y*col1.rgb;
            if(materials>=3)tex+=I.material.z*col2.rgb;
            if(materials>=4)tex+=I.material.w*col3.rgb;
         }else
         {
                            tex =I.material.x*Tex(Col , I.tex*MultiMaterial0TexScale()).rgb*MultiMaterial0Color3()
                                +I.material.y*Tex(Col1, I.tex*MultiMaterial1TexScale()).rgb*MultiMaterial1Color3();
            if(materials>=3)tex+=I.material.z*Tex(Col2, I.tex*MultiMaterial2TexScale()).rgb*MultiMaterial2Color3();
            if(materials>=4)tex+=I.material.w*Tex(Col3, I.tex*MultiMaterial3TexScale()).rgb*MultiMaterial3Color3();
         }
         if(materials<=1 /*|| !mtrl_blend*/ || color || fx==FX_BONE || !per_pixel)I.col.rgb*=tex;
         else                                                                     I.col.rgb =tex;

         // reflection
         if(rflct)
         {
                            I.col.rgb+=TexCube(Rfl , I.rfl).rgb*(MultiMaterial0Reflect()*I.material.x*((textures==2) ? Tex(Nrm , I.tex*MultiMaterial0TexScale()).z : 1));
                            I.col.rgb+=TexCube(Rfl1, I.rfl).rgb*(MultiMaterial1Reflect()*I.material.y*((textures==2) ? Tex(Nrm1, I.tex*MultiMaterial1TexScale()).z : 1));
            if(materials>=3)I.col.rgb+=TexCube(Rfl2, I.rfl).rgb*(MultiMaterial2Reflect()*I.material.z*((textures==2) ? Tex(Nrm2, I.tex*MultiMaterial2TexScale()).z : 1));
            if(materials>=4)I.col.rgb+=TexCube(Rfl3, I.rfl).rgb*(MultiMaterial3Reflect()*I.material.w*((textures==2) ? Tex(Nrm3, I.tex*MultiMaterial3TexScale()).z : 1));
         }
      }
   }

   // perform lighting
   if(per_pixel)
   {
      I.col.rgb+=Highlight.rgb;
      if(bump_mode==SBUMP_FLAT)
      {
         VecH nrm=Normalize(I.nrm); if(fx!=FX_GRASS && fx!=FX_LEAF && fx!=FX_LEAFS)BackFlip(nrm, front);
         Half d  =Sat(Dot(nrm, Light_dir.dir));
         VecH lum=Light_dir.color.rgb*d + AmbColor;
         if(materials<=1 && fx!=FX_BONE)lum+=MaterialAmbient();
         I.col.rgb*=lum;
      }
   }else
   {
      I.col.rgb+=I.col_add;
   }

   return Vec4(I.col.rgb, glow);
}
/******************************************************************************/
// HULL / DOMAIN
/******************************************************************************/
#if MODEL>=SM_4
HSData HSConstant(InputPatch<VS_PS,3> I) {return GetHSData(I[0].pos, I[1].pos, I[2].pos, I[0].nrm, I[1].nrm, I[2].nrm);}
[maxtessfactor(5.0)]
[domain("tri")]
[partitioning("fractional_odd")] // use 'odd' because it supports range from 1.0 ('even' supports range from 2.0)
[outputtopology("triangle_cw")]
[patchconstantfunc("HSConstant")]
[outputcontrolpoints(3)]
VS_PS HS
(
   InputPatch<VS_PS,3> I, UInt cp_id:SV_OutputControlPointID,
   PARAMS
)
{
   VS_PS O;
                                                                            O.pos     =I[cp_id].pos;
                                                                            O.nrm     =I[cp_id].nrm;
   if(materials<=1 /*|| !mtrl_blend*/ || color || fx==FX_BONE || !per_pixel)O.col     =I[cp_id].col;
   if(textures                                                             )O.tex     =I[cp_id].tex;
   if(light_map                                                            )O.tex_l   =I[cp_id].tex_l;
   if(rflct                                                                )O.rfl     =I[cp_id].rfl;
   if(materials>1                                                          )O.material=I[cp_id].material;
   if(fx==FX_GRASS                                                         )O.fade_out=I[cp_id].fade_out;
   if(!per_pixel                                                           )O.col_add =I[cp_id].col_add;
   return O;
}
/******************************************************************************/
[domain("tri")]
void DS
(
   HSData hs_data, const OutputPatch<VS_PS,3> I, Vec B:SV_DomainLocation,

   out VS_PS O,
   out Vec4  O_vtx:POSITION,

   PARAMS
)
{
   if(materials<=1 /*|| !mtrl_blend*/ || color || fx==FX_BONE || !per_pixel)O.col     =I[0].col     *B.z + I[1].col     *B.x + I[2].col     *B.y;
   if(textures                                                             )O.tex     =I[0].tex     *B.z + I[1].tex     *B.x + I[2].tex     *B.y;
   if(light_map                                                            )O.tex_l   =I[0].tex_l   *B.z + I[1].tex_l   *B.x + I[2].tex_l   *B.y;
   if(rflct                                                                )O.rfl     =I[0].rfl     *B.z + I[1].rfl     *B.x + I[2].rfl     *B.y;
   if(materials>1                                                          )O.material=I[0].material*B.z + I[1].material*B.x + I[2].material*B.y;
   if(fx==FX_GRASS                                                         )O.fade_out=I[0].fade_out*B.z + I[1].fade_out*B.x + I[2].fade_out*B.y;
   if(!per_pixel                                                           )O.col_add =I[0].col_add *B.z + I[1].col_add *B.x + I[2].col_add *B.y;

   SetDSPosNrm(O.pos, O.nrm, I[0].pos, I[1].pos, I[2].pos, I[0].nrm, I[1].nrm, I[2].nrm, B, hs_data, false, 0);
   O_vtx=Project(O.pos);
}
#endif
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
#if 0
@GROUP "Main"

   @SHARED
      #include "Glsl.h"
      #include "Glsl Material.h"
      #include "Glsl MultiMaterial.h"
      #include "Glsl Light.h"

      #define MULTI_TEXCOORD 1

      #if materials<=1 /*|| mtrl_blend==0*/ || COLOR!=0 || per_pixel==0
         VAR LP Vec IO_col;
      #endif
      #if per_pixel==0
         VAR LP Vec IO_col_add;
      #else
         VAR MP Vec IO_nrm;
      #endif
      #if fx==FX_GRASS
         VAR MP Flt IO_fade_out;
      #endif
      #if materials>1
         VAR LP Vec4 IO_material;
      #endif
      #if textures>=1
         VAR HP Vec2 IO_tex;
         #if MULTI_TEXCOORD!=0
            #if materials>=2
               VAR HP Vec2 IO_tex1;
            #endif
            #if materials>=3
               VAR HP Vec2 IO_tex2;
            #endif
            #if materials>=4
               VAR HP Vec2 IO_tex3;
            #endif
         #endif
      #endif
      #if light_map==1
         VAR HP Vec2 IO_tex_l;
      #endif
      #if rflct!=0
         VAR MP Vec IO_rfl;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 3D.h"

      void main()
      {
         HP Vec pos=vtx_pos();
         MP Vec nrm=vtx_nrm();
         HP Vec O_pos;

      #if textures>=1
         IO_tex=((heightmap!=0) ? vtx_texHM() : vtx_tex());
         if(heightmap!=0 && textures>=1)
         {
            #if MULTI_TEXCOORD!=0
               #if materials>=2
                  IO_tex1=IO_tex*MultiMaterial1TexScale();
               #endif
               #if materials>=3
                  IO_tex2=IO_tex*MultiMaterial2TexScale();
               #endif
               #if materials>=4
                  IO_tex3=IO_tex*MultiMaterial3TexScale();
               #endif
               if(materials>=2)IO_tex*=MultiMaterial0TexScale();else
               if(materials==1)IO_tex*=     MaterialTexScale ();
            #else
               if(materials==1)IO_tex*=MaterialTexScale();
            #endif
         }
      #endif
      #if light_map==1
         IO_tex_l=vtx_tex1();
      #endif
      #if materials<=1
         IO_col.rgb=MaterialColor3();
      #else
         IO_material=vtx_material();
         /*#if mtrl_blend==0
                            IO_col.rgb =IO_material.x*MultiMaterial0Color3()
                                       +IO_material.y*MultiMaterial1Color3();
            if(materials>=3)IO_col.rgb+=IO_material.z*MultiMaterial2Color3();
            if(materials>=4)IO_col.rgb+=IO_material.w*MultiMaterial3Color3();
         #endif*/
      #endif
      #if COLOR!=0
         if(materials<=1/* || mtrl_blend==0*/)IO_col.rgb*=vtx_color3();
         else                                 IO_col.rgb =vtx_color3();
      #endif

      #if fx==FX_GRASS
         IO_fade_out=GrassFadeOut();
      #endif
         if(fx==FX_LEAF )pos=BendLeaf (vtx_hlp(),             pos);
         if(fx==FX_LEAFS)pos=BendLeafs(vtx_hlp(), vtx_size(), pos);

         #if skin==0
         {
                                   O_pos=          TransformPos(pos, gl_InstanceID) ; if(fx==FX_GRASS)O_pos+=BendGrass(pos);
            if(bump_mode==SBUMP_FLAT)nrm=Normalize(TransformDir(nrm, gl_InstanceID));
         }
         #else
         {
            MP VecI bone  =vtx_bone  ();
            MP Vec  weight=vtx_weight();
                                   O_pos=          TransformPos(pos, bone, weight) ;
            if(bump_mode==SBUMP_FLAT)nrm=Normalize(TransformDir(nrm, bone, weight));
         }
         #endif
         O_vtx=Project(O_pos);
         gl_ClipDistance[0]=Dot(Vec4(O_pos, 1), ClipPlane);

         #if rflct!=0
            IO_rfl=Normalize(O_pos); // convert to MP
            IO_rfl=Transform3(reflect(IO_rfl, nrm), CamMatrix);
         #endif

         #if per_pixel==0
         {
            if(!(materials<=1 /*|| mtrl_blend==0*/ || COLOR!=0))IO_col.rgb=Vec(1.0, 1.0, 1.0);

            #if bump_mode>=SBUMP_FLAT
            {
               LP Flt d  =Max(Dot(nrm, Light_dir.dir), 0.0);
               LP Vec lum=Light_dir.color.rgb*d + AmbColor;
               if(materials<=1 && fx!=FX_BONE)lum+=MaterialAmbient();
               IO_col.rgb*=lum;
            }
            #endif

            LP Flt fog_rev=Sat(Length2(O_pos)*VertexFogMulAdd.x+VertexFogMulAdd.y);
            IO_col.rgb*=                                        fog_rev ;
            IO_col_add =Lerp(VertexFogColor.rgb, Highlight.rgb, fog_rev);
         }
         #else
            IO_nrm=nrm;
         #endif
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      void main()
      {
         LP Vec col;
      #if materials<=1 /*|| mtrl_blend==0*/ || COLOR!=0 || per_pixel==0
         col=IO_col.rgb;
      #endif
         MP Flt glow;
      #if materials<=1
         #if textures==0
            glow=MaterialGlow();
         #else
            #if textures==1
               LP Vec4 tex_col=Tex(Col, IO_tex);
               #if alpha_test!=0
                  LP Flt alpha=tex_col.a;
                  #if fx==FX_GRASS
                     alpha-=IO_fade_out;
                  #endif
                  AlphaTest(alpha);
               #endif
               glow=MaterialGlow();
            #elif textures==2
               LP Vec4 tex_nrm=Tex(Nrm, IO_tex); // #MaterialTextureChannelOrder
               #if alpha_test!=0
                  LP Flt alpha=tex_nrm.a;
                  #if fx==FX_GRASS
                     alpha-=IO_fade_out;
                  #endif
                  AlphaTest(alpha);
               #endif
               glow=MaterialGlow();
               #if alpha_test==0
                  glow*=tex_nrm.a;
               #endif
            #endif

            #if textures==1
               col*=tex_col.rgb;
            #elif textures==2
               col*=Tex(Col, IO_tex).rgb;
            #endif

            #if light_map==1
               col*=Tex(Lum, IO_tex_l).rgb;
            #endif

            #if rflct!=0
               #if textures==2
                  col.rgb+=TexCube(Rfl, IO_rfl).rgb*(MaterialReflect()*tex_nrm.z);
               #else
                  col.rgb+=TexCube(Rfl, IO_rfl).rgb*MaterialReflect();
               #endif
            #endif
         #endif
      #else // materials>1
         glow=0.0;
         LP Vec tex;
         #if mtrl_blend!=0
            LP Vec4 col0, col1, col2, col3;
            #if MULTI_TEXCOORD!=0
                  col0=Tex(Col , IO_tex );
                  col1=Tex(Col1, IO_tex1);
               #if materials>=3
                  col2=Tex(Col2, IO_tex2);
               #endif
               #if materials>=4
                  col3=Tex(Col3, IO_tex3);
               #endif
            #else
                               col0=Tex(Col , IO_tex*MultiMaterial0TexScale());
                               col1=Tex(Col1, IO_tex*MultiMaterial1TexScale());
               if(materials>=3)col2=Tex(Col2, IO_tex*MultiMaterial2TexScale());
               if(materials>=4)col3=Tex(Col3, IO_tex*MultiMaterial3TexScale());
            #endif
                            col0.rgb*=MultiMaterial0Color3();
                            col1.rgb*=MultiMaterial1Color3();
            if(materials>=3)col2.rgb*=MultiMaterial2Color3();
            if(materials>=4)col3.rgb*=MultiMaterial3Color3();
            MP Vec4 material;
                             material.x=MultiMaterialWeight(IO_material.x, col0.a);
                             material.y=MultiMaterialWeight(IO_material.y, col1.a); if(materials==2)material.xy  /=material.x+material.y;
            if(materials>=3){material.z=MultiMaterialWeight(IO_material.z, col2.a); if(materials==3)material.xyz /=material.x+material.y+material.z;}
            if(materials>=4){material.w=MultiMaterialWeight(IO_material.w, col3.a); if(materials==4)material.xyzw/=material.x+material.y+material.z+material.w;}
                            tex =material.x*col0.rgb
                                +material.y*col1.rgb;
            if(materials>=3)tex+=material.z*col2.rgb;
            if(materials>=4)tex+=material.w*col3.rgb;
         #else
            #if MULTI_TEXCOORD!=0
                  tex =IO_material.x*Tex(Col , IO_tex ).rgb*MultiMaterial0Color3()
                      +IO_material.y*Tex(Col1, IO_tex1).rgb*MultiMaterial1Color3();
               #if materials>=3
                  tex+=IO_material.z*Tex(Col2, IO_tex2).rgb*MultiMaterial2Color3();
               #endif
               #if materials>=4
                  tex+=IO_material.w*Tex(Col3, IO_tex3).rgb*MultiMaterial3Color3();
               #endif
            #else
                               tex =IO_material.x*Tex(Col , IO_tex*MultiMaterial0TexScale()).rgb
                                   +IO_material.y*Tex(Col1, IO_tex*MultiMaterial1TexScale()).rgb;
               if(materials>=3)tex+=IO_material.z*Tex(Col2, IO_tex*MultiMaterial2TexScale()).rgb;
               if(materials>=4)tex+=IO_material.w*Tex(Col3, IO_tex*MultiMaterial3TexScale()).rgb;
            #endif
         #endif
         #if materials<=1 /*|| mtrl_blend==0*/ || COLOR!=0 || per_pixel==0
            col*=tex;
         #else
            col=tex;
         #endif
      #endif
         // lighting
         #if per_pixel!=0
         {
            col+=Highlight.rgb;
            #if bump_mode==SBUMP_FLAT
            {
               MP Vec nrm=Normalize(IO_nrm); if(fx!=FX_GRASS && fx!=FX_LEAF && fx!=FX_LEAFS)BackFlip(nrm);
               LP Flt d  =Max(Dot(nrm, Light_dir.dir), 0.0);
               LP Vec lum=Light_dir.color.rgb*d + AmbColor;
               if(materials<=1 && fx!=FX_BONE)lum+=MaterialAmbient();
               col*=lum;
            }
            #endif
         }
         #else
            col+=IO_col_add;
         #endif

         gl_FragColor.rgb=col; // set 'gl_FragColor' at end since it's MP
         gl_FragColor.a  =glow;
      }
   @PS_END

@GROUP_END
#endif
/******************************************************************************/
