/******************************************************************************/
#include "!Header.h"
#include "Sky.h"
/******************************************************************************/
#define PARAMS            \
   uniform Bool skin     ,\
   uniform Bool color    ,\
   uniform Bool rflct    ,\
   uniform Int  textures ,\
   uniform Bool light_map
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out Vec2  outTex  :TEXCOORD0,
   out Vec2  outTexL :TEXCOORD1,
   out Vec   outRfl  :TEXCOORD2,
   out VecH4 outColor:COLOR    ,
   out Vec4  outVtx  :POSITION ,

   PARAMS
)
{
   if(textures )outTex   =vtx.tex ();
   if(light_map)outTexL  =vtx.tex1();
                outColor =MaterialColor();
   if(color    )outColor*=vtx.color();

   Vec pos; VecH nrm;
   if(!skin)
   {
   #if MODEL>=SM_4 || MODEL==SM_GL
      if(true) // instance
      {
                  pos=TransformPos(vtx.pos(), vtx.instance());
         if(rflct)nrm=TransformDir(vtx.nrm(), vtx.instance());
      }else
   #endif
      {
                  pos=TransformPos(vtx.pos());
         if(rflct)nrm=TransformDir(vtx.nrm());
      }
   }else
   {
      VecI    bone=vtx.bone();
               pos=TransformPos(vtx.pos(), bone, vtx.weight());
      if(rflct)nrm=TransformDir(vtx.nrm(), bone, vtx.weight());
   }
   if(rflct)outRfl=Transform3(reflect(Normalize(pos), Normalize(nrm)), CamMatrix);

   outVtx=Project(pos);

   outColor.a*=Sat(Length(pos)*SkyFracMulAdd.x + SkyFracMulAdd.y);
}
/******************************************************************************/
Vec4 PS
(
   Vec2  inTex  :TEXCOORD0,
   Vec2  inTexL :TEXCOORD1,
   Vec   inRfl  :TEXCOORD2,
   VecH4 inColor:COLOR    ,

   PARAMS
):COLOR
{
   VecH4 tex_nrm; // #MaterialTextureChannelOrder

   if(textures==1) inColor    *=Tex(Col, inTex);else                                                 // alpha in 'Col' texture
   if(textures==2){inColor.rgb*=Tex(Col, inTex).rgb; tex_nrm=Tex(Nrm, inTex); inColor.a*=tex_nrm.a;} // alpha in 'Nrm' texture

   if(light_map)inColor.rgb*=Tex(Lum, inTexL).rgb;

   inColor.rgb+=Highlight.rgb;

   // reflection
   if(rflct)inColor.rgb+=TexCube(Rfl, inRfl).rgb*((textures==2) ? MaterialReflect()*tex_nrm.z : MaterialReflect());

   return inColor;
}
/******************************************************************************/
CUSTOM_TECHNIQUE // this is defined in C++ as a macro
/******************************************************************************/
#if 0
@GROUP "Main"

   @SHARED
      #include "Glsl.h"
      #include "Glsl Material.h"
      #include "Glsl Light.h"

         VAR LP Vec4 IO_col;
      #if textures>0
         VAR HP Vec2 IO_tex;
      #endif
      #if light_map!=0
         VAR HP Vec2 IO_tex_l;
      #endif
      #if rflct!=0
         VAR MP Vec  IO_rfl;
      #endif
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 3D.h"

      void main()
      {
         HP Vec pos=vtx_pos();

      #if textures>0
         IO_tex=vtx_tex();
      #endif
      #if light_map!=0
         IO_tex_l=vtx_tex1();
      #endif
         IO_col=MaterialColor();
      #if COLOR!=0
         IO_col*=vtx_color();
      #endif

      #if skin==0
                   pos=          TransformPos(pos      , gl_InstanceID) ;
         #if rflct!=0
            MP Vec nrm=Normalize(TransformDir(vtx_nrm(), gl_InstanceID));
         #endif
                 O_vtx=          Project     (pos);
      #else
            MP VecI bone  =vtx_bone  ();
            MP Vec  weight=vtx_weight();
                   pos=          TransformPos(pos      , bone, weight) ;
         #if rflct!=0
            MP Vec nrm=Normalize(TransformDir(vtx_nrm(), bone, weight));
         #endif
                 O_vtx=          Project     (pos);
      #endif

         MP Vec mp_pos =pos;
         MP Flt d      =Length(mp_pos);
         LP Flt opacity=Sat(d*SkyFracMulAdd.x + SkyFracMulAdd.y);
         IO_col.a*=opacity;

      #if rflct!=0
         IO_rfl=Normalize(pos); // convert to MP
         IO_rfl=Transform3(reflect(IO_rfl, nrm), CamMatrix);
      #endif
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      void main()
      {
         LP Vec4 col=IO_col;

      #if textures==1
         col*=Tex(Col, IO_tex);
      #elif textures==2
         LP Vec4 tex_nrm=Tex(Nrm, IO_tex); // #MaterialTextureChannelOrder
         LP Vec4 tex_col=Tex(Col, IO_tex); tex_col.a=tex_nrm.a; col*=tex_col;
      #endif

      #if light_map!=0
         col.rgb*=Tex(Lum, IO_tex_l).rgb;
      #endif

         col.rgb+=Highlight.rgb;

      #if rflct!=0
         #if textures==2
            col.rgb+=TexCube(Rfl, IO_rfl).rgb*(MaterialReflect()*tex_nrm.z);
         #else
            col.rgb+=TexCube(Rfl, IO_rfl).rgb*MaterialReflect();
         #endif
      #endif

         gl_FragColor=col; // set 'gl_FragColor' at end since it's MP
      }
   @PS_END

@GROUP_END
#endif
/******************************************************************************/
