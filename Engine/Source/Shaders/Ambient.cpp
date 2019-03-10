/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#define PARAMS             \
   uniform Bool skin      ,\
   uniform Int  alpha_test
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out Vec2 outTex:TEXCOORD,
   out Vec4 outVtx:POSITION,

   IF_IS_CLIP

   PARAMS
)
{
   if(alpha_test)outTex=vtx.tex();

   Vec      pos;
   if(!skin)pos=TransformPos(vtx.pos());
   else     pos=TransformPos(vtx.pos(), vtx.bone(), vtx.weight());
       CLIP(pos); outVtx=Project(pos);
}
/******************************************************************************/
Vec4 PS
(
   Vec2 inTex:TEXCOORD,

   PARAMS
):COLOR
{
   if(alpha_test==1)clip(Tex(Col, inTex).a + MaterialAlpha()-1);else
   if(alpha_test==2)clip(Tex(Nrm, inTex).a + MaterialAlpha()-1); // #MaterialTextureChannelOrder

   return Vec4(MaterialAmbient(), 0);
}
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
