/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#define PARAMS           \
   uniform Bool skin    ,\
   uniform Int  textures
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out Vec4 outVtx:POSITION ,
   out Vec2 outTex:TEXCOORD0,
   out VecH outNrm:TEXCOORD1, // !! not Normalized !!
   out Vec  outPos:TEXCOORD2,

   PARAMS
)
{
   outTex=vtx.tex();

   if(!skin)
   {
                     outNrm=TransformDir(vtx.nrm());
      outVtx=Project(outPos=TransformPos(vtx.pos()));
   }else
   {
      VecI bone=vtx.bone();
                     outNrm=TransformDir(vtx.nrm(), bone, vtx.weight());
      outVtx=Project(outPos=TransformPos(vtx.pos(), bone, vtx.weight()));
   }
}
/******************************************************************************/
Vec4 PS
(
   PIXEL,
   Vec2 inTex:TEXCOORD0,
   Vec  inNrm:TEXCOORD1,
   Vec  inPos:TEXCOORD2,

   PARAMS
):COLOR
{
   // perform alpha testing
   if(textures==1)clip(Tex(Col, inTex).a + MaterialAlpha()-1);else // alpha in 'Col' texture
   if(textures==2)clip(Tex(Nrm, inTex).a + MaterialAlpha()-1);     // alpha in 'Nrm' texture, #MaterialTextureChannelOrder

   Flt alpha=Sat((inPos.z-TexDepthPoint(PIXEL_TO_SCREEN)-BehindBias)/0.3f);

   Vec4   col   =Lerp(Color[0], Color[1], Abs(Normalize(inNrm).z));
          col.a*=alpha;
   return col;
}
/******************************************************************************/
CUSTOM_TECHNIQUE // this is defined in C++ as a macro
/******************************************************************************/
