/******************************************************************************/
#include "!Header.h"
#include "Ambient Occlusion.h"
/******************************************************************************/
#define PARAMS                    \
   uniform Bool skin             ,\
   uniform Int  materials        ,\
   uniform Int  textures         ,\
   uniform Int  bump_mode        ,\
   uniform Bool alpha_test       ,\
   uniform Bool light_map        ,\
   uniform Bool detail           ,\
   uniform Bool rflct            ,\
   uniform Bool color            ,\
   uniform Bool mtrl_blend       ,\
   uniform Bool heightmap        ,\
   uniform Int  fx               ,\
                                  \
   uniform Bool light_dir        ,\
   uniform Bool light_dir_shd    ,\
   uniform Int  light_dir_shd_num,\
                                  \
   uniform Bool light_point      ,\
   uniform Bool light_point_shd  ,\
                                  \
   uniform Bool light_sqr        ,\
   uniform Bool light_sqr_shd    ,\
                                  \
   uniform Bool light_cone       ,\
   uniform Bool light_cone_shd   ,\
                                  \
   uniform Bool tesselate
/******************************************************************************/
struct VS_PS
{
   Vec2    tex     :TEXCOORD0;
   Vec     pos     :TEXCOORD1;
   Matrix3 mtrx    :TEXCOORD2; // !! may not be Normalized !!
   Vec     rfl     :TEXCOORD6;
   Vec2    tex_l   :TEXCOORD7;
   Half    fade_out:TEXCOORD5;
   VecH    col     :COLOR0   ;
   VecH4   material:COLOR1   ;
};
/******************************************************************************/
// VS
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
   VecH nrm, tan; if(bump_mode>=SBUMP_FLAT)nrm=vtx.nrm(); if(bump_mode>SBUMP_FLAT)tan=vtx.tan(nrm, heightmap);

   if(textures || detail)O.tex     =vtx.tex     (heightmap);
   if(light_map         )O.tex_l   =vtx.tex1    ();
   if(materials>1       )O.material=vtx.material();

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
            O.fade_out=GrassFadeOut(vtx.instance());
         }
      }else
   #endif
      {
         O.pos=TransformPos(pos);

         if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm);
         if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan);
         if(fx==FX_GRASS)
         {
            BendGrass(pos, O.pos);
            O.fade_out=GrassFadeOut();
         }
      }
      CLIP(O.pos);
   }else
   {
      VecI bone=vtx.bone();
      O.pos=TransformPos(pos, bone, vtx.weight()); CLIP(O.pos);

      if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm, bone, vtx.weight());
      if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan, bone, vtx.weight());
   }

   // normalize (have to do all at the same time, so all have the same lengths)
   if(bump_mode>SBUMP_FLAT // calculating binormal (this also covers the case when we have tangent from heightmap which is not Normalized)
   || rflct && bump_mode==SBUMP_FLAT // calculating per-vertex reflection
   || tesselate) // needed for tesselation
   {
                              O.mtrx[2]=Normalize(O.mtrx[2]);
      if(bump_mode>SBUMP_FLAT)O.mtrx[0]=Normalize(O.mtrx[0]);
   }

   if(bump_mode>SBUMP_FLAT)O.mtrx[1]=vtx.bin(O.mtrx[2], O.mtrx[0], heightmap);

   if(rflct && bump_mode==SBUMP_FLAT)O.rfl=Transform3(reflect(Normalize(O.pos), O.mtrx[2]), CamMatrix);

   O_vtx=Project(O.pos);
}
/******************************************************************************/
// PS
/******************************************************************************/
Vec4 PS
(
   VS_PS I,
   PIXEL,
   IF_IS_FRONT

   PARAMS
):COLOR
{
   Bool secondary=(light_point || light_sqr || light_cone); // local lights are enabled only for secondary shader passes

   VecH nrm;
   Half glow, specular, sss;

   if(bump_mode==SBUMP_ZERO)
   {
      nrm     =0;
      glow    =MaterialGlow();
      specular=0;
      sss     =0;

      light_dir  =false;
      light_point=false;
      light_sqr  =false;
      light_cone =false;
   }else
   if(materials==1)
   {
      VecH4 tex_nrm; // #MaterialTextureChannelOrder
      if(textures==0)
      {
         Half tex_col=1; if(detail)tex_col+=GetDetail(I.tex).z; I.col.rgb*=tex_col;
         nrm     =Normalize(I.mtrx[2]);
         glow    =MaterialGlow    ();
         specular=MaterialSpecular();
      }else
      if(textures==1)
      {
         VecH4 tex_col=Tex(Col, I.tex); if(alpha_test)AlphaTest(tex_col.w, I.fade_out, fx);
         nrm     =Normalize(I.mtrx[2]);
         glow    =MaterialGlow    ();
         specular=MaterialSpecular();

         if(detail)tex_col.rgb+=GetDetail(I.tex).z; I.col.rgb*=tex_col.rgb;
      }else
      if(textures==2)
      {
         tex_nrm     =Tex(Nrm, I.tex)   ; if( alpha_test)AlphaTest(tex_nrm.a, I.fade_out, fx); // #MaterialTextureChannelOrder
         glow        =MaterialGlow    (); if(!alpha_test)glow*=tex_nrm.a;
         specular    =MaterialSpecular()*tex_nrm.z;
         if(bump_mode==SBUMP_FLAT)
         {
            nrm=Normalize(I.mtrx[2]);
            VecH tex_col=Tex(Col, I.tex).rgb; if(detail)tex_col+=GetDetail(I.tex).z; I.col.rgb*=tex_col;
         }else // normal mapping
         {
            VecH      det;
            if(detail)det=GetDetail(I.tex);

                      nrm.xy =(tex_nrm.xy*2-1)*MaterialRough();
            if(detail)nrm.xy+=det.xy;
                      nrm.z  =CalcZ(nrm.xy);
                      nrm    =Normalize(Transform(nrm, I.mtrx));

            VecH tex_col=Tex(Col, I.tex).rgb; if(detail)tex_col+=det.z; I.col.rgb*=tex_col;
         }
      }

      if(light_map)I.col.rgb*=Tex(Lum, I.tex_l).rgb;
      sss=MaterialSss();

      // reflection
      if(rflct)
      {
         if(bump_mode>SBUMP_FLAT)I.rfl=Transform3(reflect(I.pos, nrm), CamMatrix);
         I.col.rgb+=TexCube(Rfl, I.rfl).rgb*((textures==2) ? MaterialReflect()*tex_nrm.z : MaterialReflect());
      }
   }else // materials>1
   {
      // assuming that in multi materials textures!=0
      Vec2 tex0, tex1, tex2, tex3;
                      tex0=I.tex*MultiMaterial0TexScale();
                      tex1=I.tex*MultiMaterial1TexScale();
      if(materials>=3)tex2=I.tex*MultiMaterial2TexScale();
      if(materials>=4)tex3=I.tex*MultiMaterial3TexScale();

      // color !! do this first because it may modify 'I.material' which affects secondary texture !!
      VecH tex;
      if(mtrl_blend)
      {
         VecH4 col0, col1, col2, col3;
                          col0=Tex(Col , tex0); col0.rgb*=MultiMaterial0Color3(); I.material.x=MultiMaterialWeight(I.material.x, col0.a);
                          col1=Tex(Col1, tex1); col1.rgb*=MultiMaterial1Color3(); I.material.y=MultiMaterialWeight(I.material.y, col1.a); if(materials==2)I.material.xy  /=I.material.x+I.material.y;
         if(materials>=3){col2=Tex(Col2, tex2); col2.rgb*=MultiMaterial2Color3(); I.material.z=MultiMaterialWeight(I.material.z, col2.a); if(materials==3)I.material.xyz /=I.material.x+I.material.y+I.material.z;}
         if(materials>=4){col3=Tex(Col3, tex3); col3.rgb*=MultiMaterial3Color3(); I.material.w=MultiMaterialWeight(I.material.w, col3.a); if(materials==4)I.material.xyzw/=I.material.x+I.material.y+I.material.z+I.material.w;}

                         tex =I.material.x*col0.rgb
                             +I.material.y*col1.rgb;
         if(materials>=3)tex+=I.material.z*col2.rgb;
         if(materials>=4)tex+=I.material.w*col3.rgb;
      }else
      {
                         tex =I.material.x*Tex(Col , tex0).rgb*MultiMaterial0Color3()
                             +I.material.y*Tex(Col1, tex1).rgb*MultiMaterial1Color3();
         if(materials>=3)tex+=I.material.z*Tex(Col2, tex2).rgb*MultiMaterial2Color3();
         if(materials>=4)tex+=I.material.w*Tex(Col3, tex3).rgb*MultiMaterial3Color3();
      }
      if(materials<=1 /*|| !mtrl_blend*/ || color)I.col.rgb*=tex;
      else                                        I.col.rgb =tex;

      // normal, specular, glow !! do this second after modifying 'I.material' !!
      if(textures<=1)
      {
         nrm=Normalize(I.mtrx[2]);

                   VecH2 tex =I.material.x*MultiMaterial0NormalAdd().zw // #MaterialTextureChannelOrder
                             +I.material.y*MultiMaterial1NormalAdd().zw;
         if(materials>=3)tex+=I.material.z*MultiMaterial2NormalAdd().zw;
         if(materials>=4)tex+=I.material.w*MultiMaterial3NormalAdd().zw;

         specular=tex.x;
         glow    =tex.y;

         // reflection
         if(rflct)
         {
            if(bump_mode>SBUMP_FLAT)I.rfl=Transform3(reflect(I.pos, nrm), CamMatrix);
                            I.col.rgb+=TexCube(Rfl , I.rfl).rgb*(MultiMaterial0Reflect()*I.material.x);
                            I.col.rgb+=TexCube(Rfl1, I.rfl).rgb*(MultiMaterial1Reflect()*I.material.y);
            if(materials>=3)I.col.rgb+=TexCube(Rfl2, I.rfl).rgb*(MultiMaterial2Reflect()*I.material.z);
            if(materials>=4)I.col.rgb+=TexCube(Rfl3, I.rfl).rgb*(MultiMaterial3Reflect()*I.material.w);
         }
      }else
      {
         Half tex_spec[4];

         if(bump_mode==SBUMP_FLAT)
         {
            nrm=Normalize(I.mtrx[2]);

                             VecH2 tex; // #MaterialTextureChannelOrder
                            {VecH2 nrm0; nrm0=Tex(Nrm , tex0).zw; if(rflct)tex_spec[0]=nrm0.x; nrm0=nrm0*MultiMaterial0NormalMul().zw+MultiMaterial0NormalAdd().zw; tex =I.material.x*nrm0;}
                            {VecH2 nrm1; nrm1=Tex(Nrm1, tex1).zw; if(rflct)tex_spec[1]=nrm1.x; nrm1=nrm1*MultiMaterial1NormalMul().zw+MultiMaterial1NormalAdd().zw; tex+=I.material.y*nrm1;}
            if(materials>=3){VecH2 nrm2; nrm2=Tex(Nrm2, tex2).zw; if(rflct)tex_spec[2]=nrm2.x; nrm2=nrm2*MultiMaterial2NormalMul().zw+MultiMaterial2NormalAdd().zw; tex+=I.material.z*nrm2;}
            if(materials>=4){VecH2 nrm3; nrm3=Tex(Nrm3, tex3).zw; if(rflct)tex_spec[3]=nrm3.x; nrm3=nrm3*MultiMaterial3NormalMul().zw+MultiMaterial3NormalAdd().zw; tex+=I.material.w*nrm3;}

            specular=tex.x;
            glow    =tex.y;
         }else // normal mapping
         {
                             VecH4 tex; // #MaterialTextureChannelOrder
                            {VecH4 nrm0=Tex(Nrm , tex0); if(rflct)tex_spec[0]=nrm0.z; nrm0=nrm0*MultiMaterial0NormalMul()+MultiMaterial0NormalAdd(); tex =I.material.x*nrm0;}
                            {VecH4 nrm1=Tex(Nrm1, tex1); if(rflct)tex_spec[1]=nrm1.z; nrm1=nrm1*MultiMaterial1NormalMul()+MultiMaterial1NormalAdd(); tex+=I.material.y*nrm1;}
            if(materials>=3){VecH4 nrm2=Tex(Nrm2, tex2); if(rflct)tex_spec[2]=nrm2.z; nrm2=nrm2*MultiMaterial2NormalMul()+MultiMaterial2NormalAdd(); tex+=I.material.z*nrm2;}
            if(materials>=4){VecH4 nrm3=Tex(Nrm3, tex3); if(rflct)tex_spec[3]=nrm3.z; nrm3=nrm3*MultiMaterial3NormalMul()+MultiMaterial3NormalAdd(); tex+=I.material.w*nrm3;}

            nrm=VecH(tex.xy, CalcZ(tex.xy));
            nrm=Normalize(Transform(nrm, I.mtrx));
            
            specular=tex.z;
            glow    =tex.w;
         }

         // reflection
         if(rflct)
         {
            if(bump_mode>SBUMP_FLAT)I.rfl=Transform3(reflect(I.pos, nrm), CamMatrix);
                            I.col.rgb+=TexCube(Rfl , I.rfl).rgb*(MultiMaterial0Reflect()*I.material.x*tex_spec[0]);
                            I.col.rgb+=TexCube(Rfl1, I.rfl).rgb*(MultiMaterial1Reflect()*I.material.y*tex_spec[1]);
            if(materials>=3)I.col.rgb+=TexCube(Rfl2, I.rfl).rgb*(MultiMaterial2Reflect()*I.material.z*tex_spec[2]);
            if(materials>=4)I.col.rgb+=TexCube(Rfl3, I.rfl).rgb*(MultiMaterial3Reflect()*I.material.w*tex_spec[3]);
         }
      }

      sss=0;
   }

   if(fx!=FX_GRASS && fx!=FX_LEAF && fx!=FX_LEAFS)BackFlip(nrm, front);

   // lighting
   VecH total_lum,
        total_specular=0;

   if(bump_mode==SBUMP_ZERO     )total_lum =1;
   else                          total_lum =AmbColor;
   if(materials<=1 && !secondary)total_lum+=MaterialAmbient()*AmbMaterial; // ambient values are always disabled for secondary passes (so don't bother adding them)

   VecH2 jitter_value;
   if(light_dir_shd || light_point_shd || light_sqr_shd || light_cone_shd)jitter_value=ShadowJitter(pixel.xy);

   if(light_dir)
   {
      // shadow
      Half shadow; if(light_dir_shd)shadow=Sat(ShadowDirValue(I.pos, jitter_value, true, light_dir_shd_num, false));

      // diffuse
      VecH light_dir=Light_dir.dir;
      Half lum      =LightDiffuse(nrm, light_dir); if(light_dir_shd)lum*=shadow;

      // specular
   #if MODEL>=SM_4
      BRANCH if(lum*specular>EPS_COL)
   #endif
      {
         VecH eye_dir=Normalize    (-I.pos);
         Half spec   =LightSpecular(   nrm, specular, light_dir, eye_dir); if(light_dir_shd)spec*=shadow;
         total_specular+=Light_dir.color.rgb*spec;
      }  total_lum     +=Light_dir.color.rgb*lum ;
   }

   if(light_point)
   {
      // shadow
      Half shadow; if(light_point_shd)shadow=ShadowFinal(ShadowPointValue(I.pos, jitter_value, true));

      // distance
      VecH light_dir=Light_point.pos-I.pos;
      Half power    =LightPointDist(light_dir); if(light_point_shd)power*=shadow;

      // diffuse
           light_dir=Normalize   (light_dir);
      Half lum      =LightDiffuse(nrm, light_dir);

      // specular
   #if MODEL>=SM_4
      BRANCH if(lum*specular>EPS_COL)
   #endif
      {
         VecH eye_dir=Normalize    (-I.pos);
         Half spec   =LightSpecular(   nrm, specular, light_dir, eye_dir);
         total_specular+=Light_point.color.rgb*(spec*power);
      }  total_lum     +=Light_point.color.rgb*(lum *power);
   }

   if(light_sqr)
   {
      // shadow
      Half shadow; if(light_sqr_shd)shadow=ShadowFinal(ShadowPointValue(I.pos, jitter_value, true));

      // distance
      VecH light_dir=Light_sqr.pos-I.pos;
      Half power    =LightSqrDist(light_dir); if(light_sqr_shd)power*=shadow;

      // diffuse
           light_dir=Normalize   (light_dir);
      Half lum      =LightDiffuse(nrm, light_dir);

      // specular
   #if MODEL>=SM_4
      BRANCH if(lum*specular>EPS_COL)
   #endif
      {
         VecH eye_dir=Normalize    (-I.pos);
         Half spec   =LightSpecular(   nrm, specular, light_dir, eye_dir);
         total_specular+=Light_sqr.color.rgb*(spec*power);
      }  total_lum     +=Light_sqr.color.rgb*(lum *power);
   }

   if(light_cone)
   {
      // shadow
      Half shadow; if(light_cone_shd)shadow=ShadowFinal(ShadowConeValue(I.pos, jitter_value, true));

      // distance & angle
      VecH light_dir=Light_cone.pos-I.pos,
           dir      =mul(Light_cone.mtrx, light_dir); dir.xy/=dir.z; // clip(Vec(1-Abs(dir.xy), dir.z));
      Half power    =LightConeAngle(dir.xy)*LightConeDist(light_dir); if(light_cone_shd)power*=shadow; power*=(dir.z>0);

      // diffuse
           light_dir=Normalize   (light_dir);
      Half lum      =LightDiffuse(nrm, light_dir);

      // specular
   #if MODEL>=SM_4
      BRANCH if(lum*specular>EPS_COL)
   #endif
      {
         VecH eye_dir=Normalize    (-I.pos);
         Half spec   =LightSpecular(   nrm, specular, light_dir, eye_dir);
         total_specular+=Light_cone.color.rgb*(spec*power);
      }  total_lum     +=Light_cone.color.rgb*(lum *power);
   }

   UpdateColorBySss(I.col.rgb, nrm, sss);
   I.col.rgb=(I.col.rgb+Highlight.rgb)*total_lum + total_specular;

   return Vec4(I.col.rgb, glow);
}
/******************************************************************************/
// HULL / DOMAIN
/******************************************************************************/
#if MODEL>=SM_4
HSData HSConstant(InputPatch<VS_PS,3> I) {return GetHSData(I[0].pos, I[1].pos, I[2].pos, I[0].mtrx[2], I[1].mtrx[2], I[2].mtrx[2]);}
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
                                               O.pos     =I[cp_id].pos     ;
   if(materials<=1 /*|| !mtrl_blend*/ || color)O.col     =I[cp_id].col     ;
   if(textures || detail                      )O.tex     =I[cp_id].tex     ;
   if(light_map                               )O.tex_l   =I[cp_id].tex_l   ;
   if(rflct && bump_mode==SBUMP_FLAT          )O.rfl     =I[cp_id].rfl     ;
   if(materials>1                             )O.material=I[cp_id].material;
   if(fx==FX_GRASS                            )O.fade_out=I[cp_id].fade_out;
   if(bump_mode==SBUMP_FLAT                   )O.mtrx[2] =I[cp_id].mtrx[2] ;
   if(bump_mode> SBUMP_FLAT                   )O.mtrx    =I[cp_id].mtrx    ;
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
   if(materials<=1 /*|| !mtrl_blend*/ || color)O.col     =I[0].col     *B.z + I[1].col     *B.x + I[2].col     *B.y;
   if(textures || detail                      )O.tex     =I[0].tex     *B.z + I[1].tex     *B.x + I[2].tex     *B.y;
   if(light_map                               )O.tex_l   =I[0].tex_l   *B.z + I[1].tex_l   *B.x + I[2].tex_l   *B.y;
   if(rflct && bump_mode==SBUMP_FLAT          )O.rfl     =I[0].rfl     *B.z + I[1].rfl     *B.x + I[2].rfl     *B.y;
   if(materials>1                             )O.material=I[0].material*B.z + I[1].material*B.x + I[2].material*B.y;
   if(fx==FX_GRASS                            )O.fade_out=I[0].fade_out*B.z + I[1].fade_out*B.x + I[2].fade_out*B.y;

   if(bump_mode>SBUMP_FLAT)
   {
      O.mtrx[0]=I[0].mtrx[0]*B.z + I[1].mtrx[0]*B.x + I[2].mtrx[0]*B.y;
      O.mtrx[1]=I[0].mtrx[1]*B.z + I[1].mtrx[1]*B.x + I[2].mtrx[1]*B.y;
      //mtrx[2] is handled below
   }

   SetDSPosNrm(O.pos, O.mtrx[2], I[0].pos, I[1].pos, I[2].pos, I[0].mtrx[2], I[1].mtrx[2], I[2].mtrx[2], B, hs_data, false, 0);
   O_vtx=Project(O.pos);
}
#endif
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
