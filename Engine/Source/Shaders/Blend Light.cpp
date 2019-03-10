/******************************************************************************/
#include "!Header.h"
#include "Ambient Occlusion.h"
#include "Vertex Fog.h"
#include "Fog.h"
#include "Sky.h"
/******************************************************************************/
#define ALPHA_CLIP 0.5f
/******************************************************************************/
#define PARAMS              \
   uniform Bool skin       ,\
   uniform Bool color      ,\
   uniform Int  textures   ,\
   uniform Int  bump_mode  ,\
   uniform Bool alpha_test ,\
   uniform Bool alpha      ,\
   uniform Bool light_map  ,\
   uniform Bool rflct      ,\
   uniform Int  fx         ,\
   uniform Bool per_pixel  ,\
   uniform Int  shadow_maps
// when adding "Bool tesselate" here, then remove "Bool tesselate=false;" below
#define use_vel alpha_test
/******************************************************************************/
struct VS_PS
{
   Vec      pos    :TEXCOORD0;
   Vec2     tex    :TEXCOORD1;
   MatrixH3 mtrx   :TEXCOORD2; // !! may not be Normalized !!
   Vec2     tex_l  :TEXCOORD5;
   Vec      rfl    :TEXCOORD6;
   Vec      vel    :TEXCOORD7;
   VecH4    col    :COLOR0   ;
   VecH     col_add:COLOR1   ;
};
/******************************************************************************/
// VS
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out VS_PS O,
   out Vec4  O_vtx:POSITION,

   PARAMS
)
{
   Bool heightmap=false, tesselate=false;

   Vec  pos=vtx.pos();
   VecH nrm, tan; if(bump_mode>=SBUMP_FLAT)nrm=vtx.nrm(); if(bump_mode>SBUMP_FLAT)tan=vtx.tan(nrm, heightmap);

   if(textures   )O.tex  =vtx.tex (heightmap);
   if(light_map  )O.tex_l=vtx.tex1();
                  O.col  =MaterialColor();
   if(color      )O.col *=vtx.color();
   if(fx==FX_LEAF)
   {
      if(bump_mode> SBUMP_FLAT)BendLeaf(vtx.hlp(), pos, nrm, tan);else
      if(bump_mode==SBUMP_FLAT)BendLeaf(vtx.hlp(), pos, nrm     );else
                               BendLeaf(vtx.hlp(), pos          );
   }
   if(fx==FX_LEAFS)
   {
      if(bump_mode> SBUMP_FLAT)BendLeafs(vtx.hlp(), vtx.size(), pos, nrm, tan);else
      if(bump_mode==SBUMP_FLAT)BendLeafs(vtx.hlp(), vtx.size(), pos, nrm     );else
                               BendLeafs(vtx.hlp(), vtx.size(), pos          );
   }

   if(!skin)
   {
   #if MODEL>=SM_4 || MODEL==SM_GL
      if(true) // instance
      {
                    O.pos=TransformPos(pos, vtx.instance());

         if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm, vtx.instance());
         if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan, vtx.instance());
         if(fx==FX_GRASS)
         {
              BendGrass(pos, O.pos, vtx.instance());
            O.col.a*=1-GrassFadeOut(vtx.instance());
         }
         if(use_vel){O.vel=ObjVel[vtx.instance()]; UpdateVelocities_VS(O.vel, pos, O.pos, vtx.instance());} // #PER_INSTANCE_VEL
      }else
   #endif
      {
                    O.pos=TransformPos(pos);

         if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm);
         if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan);
         if(fx==FX_GRASS)
         {
            BendGrass(pos, O.pos);
            O.col.a*=1-GrassFadeOut();
         }
         if(use_vel){O.vel=ObjVel[0]; UpdateVelocities_VS(O.vel, pos, O.pos);}
      }
   }else
   {
      VecI bone=vtx.bone();
                                   O.pos=TransformPos(pos, bone, vtx.weight());
      if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm, bone, vtx.weight());
      if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan, bone, vtx.weight());
      if(use_vel){O.vel=GetBoneVel(bone, vtx.weight()); UpdateVelocities_VS(O.vel, pos, O.pos);}
   }

   // normalize (have to do all at the same time, so all have the same lengths)
   if(bump_mode>SBUMP_FLAT // calculating binormal (this also covers the case when we have tangent from heightmap which is not Normalized)
   || rflct && !(per_pixel && bump_mode>SBUMP_FLAT) // per-vertex reflection
   || !per_pixel && bump_mode>=SBUMP_FLAT // per-vertex lighting
   || tesselate) // needed for tesselation
   {
                              O.mtrx[2]=Normalize(O.mtrx[2]);
      if(bump_mode>SBUMP_FLAT)O.mtrx[0]=Normalize(O.mtrx[0]);
   }

   if(bump_mode>SBUMP_FLAT)O.mtrx[1]=vtx.bin(O.mtrx[2], O.mtrx[0], heightmap);

   if(rflct && !(per_pixel && bump_mode>SBUMP_FLAT))O.rfl=Transform3(reflect(Normalize(O.pos), O.mtrx[2]), CamMatrix);

   if(per_pixel)
   {
      Flt d=Length(O.pos);
      O.col.a*=Sat(d*SkyFracMulAdd.x + SkyFracMulAdd.y);

      Half fog_rev=       VisibleOpacity(FogDensity(), d); // fog_rev=1-fog
      O.col.rgb*=                                fog_rev ; //       *=1-fog
      O.col_add =Lerp(FogColor(), Highlight.rgb, fog_rev); //         1-fog
   }else
   {
      // lighting
      if(bump_mode>=SBUMP_FLAT)
      {
         Half d  =Sat(Dot(O.mtrx[2], Light_dir.dir));
         VecH lum=Light_dir.color.rgb*d + AmbColor;
         O.col.rgb*=lum;
      }

      // fog
      Half fog_rev=Sat(Length2(O.pos)*VertexFogMulAdd.x+VertexFogMulAdd.y); // fog_rev=1-fog
      O.col.rgb*=                                        fog_rev ;          //       *=1-fog
      O.col_add =Lerp(VertexFogColor.rgb, Highlight.rgb, fog_rev);          //         1-fog
   }
   O_vtx=Project(O.pos);
}
/******************************************************************************/
// PS
/******************************************************************************/
void PS
(
   VS_PS I,
 //PIXEL,
   IF_IS_FRONT

out Vec4 outCol:COLOR0,
out Vec4 outVel:COLOR1, // #BlendRT

   PARAMS
)
{
   VecH  nrm;
   Half  specular=MaterialSpecular();
   VecH4 tex_nrm; // #MaterialTextureChannelOrder

   if(textures==0)
   {
      if(per_pixel && bump_mode>=SBUMP_FLAT)nrm=Normalize(I.mtrx[2]);
   }else
   if(textures==1)
   {
      VecH4 tex_col=Tex(Col, I.tex); if(alpha_test)clip(tex_col.a-ALPHA_CLIP);
      if(alpha)I.col*=tex_col;else I.col.rgb*=tex_col.rgb;
      if(per_pixel && bump_mode>=SBUMP_FLAT)nrm=Normalize(I.mtrx[2]);
   }else
   if(textures==2)
   {
      tex_nrm=Tex(Nrm, I.tex); if(alpha_test)clip(tex_nrm.a-ALPHA_CLIP);
               specular *=tex_nrm.z;
      if(alpha)I.col.a  *=tex_nrm.a;
               I.col.rgb*=Tex(Col, I.tex).rgb;

      if(per_pixel)
      {
         if(bump_mode==SBUMP_FLAT)nrm=Normalize(I.mtrx[2]);else
         if(bump_mode> SBUMP_FLAT)
         {
            nrm.xy=(tex_nrm.xy*2-1)*MaterialRough();
            nrm.z =CalcZ(nrm.xy);
            nrm   =Normalize(Transform(nrm, I.mtrx));
         }
      }
   }

   if(light_map)I.col.rgb*=Tex(Lum, I.tex_l).rgb;

   // reflection
   if(rflct)
   {
      if(per_pixel && bump_mode>SBUMP_FLAT)I.rfl=Transform3(reflect(I.pos, nrm), CamMatrix);
      I.col.rgb+=TexCube(Rfl, I.rfl).rgb * ((textures==2) ? MaterialReflect()*tex_nrm.z : MaterialReflect());
   }

   // calculate lighting
   if(per_pixel && bump_mode>=SBUMP_FLAT)
   {
      VecH total_lum=AmbColor;//+AmbMaterial*MaterialAmbient();
    //VecH total_specular=0;

      if(fx!=FX_GRASS && fx!=FX_LEAF && fx!=FX_LEAFS)BackFlip(nrm, front);

      // directional light
      {
         // shadow
         Half shd; if(shadow_maps)shd=Sat(ShadowDirValue(I.pos, 0, false, shadow_maps, false)); // for improved performance don't use shadow jittering because it's not much needed for blended objects

         // diffuse
         VecH light_dir=Light_dir.dir;
         Half lum      =LightDiffuse(nrm, light_dir); if(shadow_maps)lum*=shd;

         // specular
      /*#if MODEL>=SM_4 // don't use specular at all
         BRANCH if(lum*specular>EPS_COL)
      #endif
         {
            Vec eye_dir=Normalize    (-I.pos);
            Flt spec   =LightSpecular( nrm, specular, light_dir, eye_dir); if(shadow_maps)spec*=shd;
            total_specular+=Light_dir.color.rgb*spec;
         }*/total_lum     +=Light_dir.color.rgb*lum ;
      }

    //UpdateColorBySss(col, nrm, MaterialSss()); // don't use sub-surface scattering at all

      // perform lighting
      I.col.rgb=I.col.rgb*total_lum;// + total_specular;
   }
   I.col.rgb+=I.col_add;
   outCol=I.col;
   if(use_vel){UpdateVelocities_PS(I.vel, I.pos); outVel.xyz=I.vel; outVel.w=I.col.a;} // alpha needed because of blending
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
#if 0
@GROUP "Main"

   @SHARED
      #include "Glsl.h"
      #include "Glsl Light.h"
      #include "Glsl Material.h"

      #define ALPHA_CLIP 0.5
      #define use_vel alpha_test

         VAR LP Vec4 IO_col;
         VAR LP Vec  IO_col_add;
      #if textures>=1
         VAR HP Vec2 IO_tex;
      #endif
      #if light_map==1
         VAR HP Vec2 IO_tex_l;
      #endif
      #if per_pixel!=0 && bump_mode>=SBUMP_FLAT
         VAR MP Vec  IO_nrm;
      #endif
      #if rflct!=0
         VAR MP Vec  IO_rfl;
      #endif
      #if use_vel!=0
         VAR HP Vec  IO_pos;
         VAR MP Vec  IO_vel;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 3D.h"

      void main()
      {
         HP Vec pos=vtx_pos();
         MP Vec nrm=vtx_nrm();
      #if use_vel==0
         HP Vec IO_pos;
      #endif

      #if textures>=1
         IO_tex=vtx_tex();
      #endif

      #if light_map==1
         IO_tex_l=vtx_tex1();
      #endif
                     IO_col =MaterialColor();
         if(COLOR!=0)IO_col*=    vtx_color();
      #if fx==FX_GRASS
         IO_col.a*=1.0-GrassFadeOut();
      #elif fx==FX_LEAF
         pos=BendLeaf(vtx_hlp(), pos);
      #elif fx==FX_LEAFS
         pos=BendLeafs(vtx_hlp(), vtx_size(), pos);
      #endif

         #if skin==0
         {
                                  IO_pos=          TransformPos(pos, gl_InstanceID) ; if(fx==FX_GRASS)IO_pos+=BendGrass(pos);
            if(bump_mode>=SBUMP_FLAT)nrm=Normalize(TransformDir(nrm, gl_InstanceID));
                                   O_vtx=          Project     (IO_pos);
         #if use_vel!=0
            IO_vel=ObjVel[gl_InstanceID]; // #PER_INSTANCE_VEL
         #endif
         }
         #else
         {
            MP VecI bone  =vtx_bone  ();
            MP Vec  weight=vtx_weight();
                                  IO_pos=          TransformPos(pos, bone, weight) ;
            if(bump_mode>=SBUMP_FLAT)nrm=Normalize(TransformDir(nrm, bone, weight));
                                   O_vtx=          Project     (IO_pos);
         #if use_vel!=0
            IO_vel=GetBoneVel(bone, weight);
         #endif
         }
         #endif

         #if per_pixel==0
         {
            if(bump_mode>=SBUMP_FLAT)
            {
               LP Flt d  =Max(Dot(nrm, Light_dir.dir), 0.0);
               LP Vec lum=Light_dir.color.rgb*d + AmbColor;
               IO_col.rgb*=lum;
            }

            LP Flt fog_rev=Sat(Length2(IO_pos)*VertexFogMulAdd.x+VertexFogMulAdd.y);
            IO_col.rgb*=                                        fog_rev ;
            IO_col_add =Lerp(VertexFogColor.rgb, Highlight.rgb, fog_rev);
         }
         #else
         {
            #if bump_mode>=SBUMP_FLAT
               IO_nrm=nrm;
            #endif

            MP Vec mp_pos =IO_pos;
            MP Flt d      =Length(mp_pos);
            LP Flt opacity=Sat(d*SkyFracMulAdd.x + SkyFracMulAdd.y);
            IO_col.a*=opacity;

            LP Flt fog_rev=      VisibleOpacity(FogDensity(), d);
            IO_col.rgb*=                                fog_rev ;
            IO_col_add =Lerp(FogColor(), Highlight.rgb, fog_rev);
         }
         #endif

         #if rflct!=0
         {
            IO_rfl=Normalize(IO_pos); // convert to MP
            #if !(per_pixel!=0 && bump_mode>=SBUMP_FLAT)
               IO_rfl=Transform3(reflect(IO_rfl, nrm), CamMatrix);
            #endif
         }
         #endif
         #if use_vel!=0
            UpdateVelocities_VS(IO_vel, pos, IO_pos);
         #endif
      }
   @VS_END

   @PS
      #include "Glsl PS.h"
      #include "Glsl 3D.h"

      void main()
      {
         MP Vec  nrm;
         LP Vec4 col=IO_col;

         #if   textures==0
            #if per_pixel!=0 && bump_mode>=SBUMP_FLAT
               nrm=Normalize(IO_nrm);
            #endif
         #elif textures==1
            LP Vec4 tex_col=Tex(Col, IO_tex);
            #if alpha_test
               if(tex_col.a<ALPHA_CLIP)discard;
            #endif
            #if ALPHA
               col*=tex_col;
            #else
               col.rgb*=tex_col.rgb;
            #endif
            #if per_pixel!=0 && bump_mode>=SBUMP_FLAT
               nrm=Normalize(IO_nrm);
            #endif
         #elif textures==2
            LP Vec4 tex_nrm=Tex(Nrm, IO_tex); // #MaterialTextureChannelOrder
            #if alpha_test
               if(tex_nrm.a<ALPHA_CLIP)discard;
            #endif
               col.rgb*=Tex(Col, IO_tex).rgb;
            #if ALPHA
               col.a  *=tex_nrm.a;
            #endif
            #if per_pixel!=0 && bump_mode>=SBUMP_FLAT
               nrm=Normalize(IO_nrm);
            #endif
         #endif

         #if light_map==1
            col.rgb*=Tex(Lum, IO_tex_l).rgb;
         #endif

         #if rflct!=0
            MP Vec rfl=IO_rfl;
            #if per_pixel!=0 && bump_mode>=SBUMP_FLAT
               rfl=Transform3(reflect(rfl, nrm), CamMatrix);
            #endif
            #if textures==2
               col.rgb+=TexCube(Rfl, rfl).rgb*(MaterialReflect()*tex_nrm.z);
            #else
               col.rgb+=TexCube(Rfl, rfl).rgb*MaterialReflect();
            #endif
         #endif

         #if per_pixel!=0 && bump_mode>=SBUMP_FLAT
         {
            LP Vec total_lum=AmbColor;
            if(fx!=FX_GRASS && fx!=FX_LEAF && fx!=FX_LEAFS)BackFlip(nrm);

            // directional light
            LP Flt d  =Max(Dot(nrm, Light_dir.dir), 0.0);
            total_lum+=Light_dir.color.rgb*d;

            col.rgb*=total_lum;
         }
         #endif
         col.rgb+=IO_col_add;
         gl_FragData[0]=col;
      #if use_vel!=0
         gl_FragData[1].xyz=UpdateVelocities_PS(IO_vel, IO_pos);
         gl_FragData[1].w  =col.a; // alpha needed because of blending
      #endif
      }
   @PS_END

@GROUP_END
#endif
/******************************************************************************/
