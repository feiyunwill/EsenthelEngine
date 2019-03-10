/******************************************************************************/
#include "!Header.h"
#include "Fur.h"
#define FACTOR (-0.7f) // prevents complete darkness at the bottom layers, gives ambient=0.3, it will match the 'size' version
/******************************************************************************/
inline VecH GetBoneFurVel(VecI bone, VecH weight) {return weight.x*FurVel[bone.x] + weight.y*FurVel[bone.y] + weight.z*FurVel[bone.z];}
/******************************************************************************/
#define BASE_PARAMS     \
   uniform Bool skin   ,\
   uniform Bool size   ,\
   uniform Bool diffuse

#define SOFT_PARAMS     \
   uniform Bool skin   ,\
   uniform Bool size   ,\
   uniform Bool diffuse
/******************************************************************************/
void Base_VS
(
   VtxInput vtx,

   out Vec2 outTex:TEXCOORD0,
   out VecH outNrm:TEXCOORD1, // !! not Normalized !!
   out Vec  outPos:TEXCOORD2,
   out Vec  outVel:TEXCOORD3,
   out Half outLen:TEXCOORD4,
   out Vec4 outVtx:POSITION ,

   IF_IS_CLIP

   BASE_PARAMS
)
{
   outTex=vtx.tex();

   if(!skin)
   {
   #if MODEL>=SM_4 || MODEL==SM_GL
      if(true) // instance
      {
         outVel=ObjVel[vtx.instance()]; // #PER_INSTANCE_VEL
         outPos=TransformPos(vtx.pos(), vtx.instance());
         outNrm=TransformDir(vtx.nrm(), vtx.instance());
         UpdateVelocities_VS(outVel, vtx.pos(), outPos, vtx.instance());
      }else
   #endif
      {
         outVel=ObjVel[0];
         outPos=TransformPos(vtx.pos());
         outNrm=TransformDir(vtx.nrm());
         UpdateVelocities_VS(outVel, vtx.pos(), outPos);
      }
   }else
   {
      VecI bone=vtx.bone();
      outVel=GetBoneVel  (           bone, vtx.weight());
      outPos=TransformPos(vtx.pos(), bone, vtx.weight());
      outNrm=TransformDir(vtx.nrm(), bone, vtx.weight());
      UpdateVelocities_VS(outVel, vtx.pos(), outPos);
   }
   if(size)outLen=vtx.size();
   CLIP(outPos); outVtx=Project(outPos);
}
/******************************************************************************/
void Base_PS
(
   Vec2 inTex:TEXCOORD0,
   VecH inNrm:TEXCOORD1,
   Vec  inPos:TEXCOORD2,
   Vec  inVel:TEXCOORD3,
   Half inLen:TEXCOORD4,

   out DeferredSolidOutput output,

   BASE_PARAMS
)
{
   Half fur=Tex(FurCol, inTex*MaterialDetScale()).r;
   VecH col=Sat(size ? inLen*-fur+1 : fur*FACTOR+1); // inLen*-fur+step+1 : fur*FACTOR+step+1, here step=0
   if(diffuse)col*=Tex(Col, inTex).rgb;
   col=col*MaterialColor3()+Highlight.rgb;

   inNrm=Normalize(inNrm);
 //UpdateColorBySss(col, inNrm, MaterialSss());

   output.color   (col);
   output.glow    (0);
   output.normal  (inNrm);
   output.specular(MaterialSpecular());
   output.velocity(inVel, inPos);
}
/******************************************************************************/
void Soft_VS
(
   VtxInput vtx,

   out Vec2 outTex :TEXCOORD0,
   out Vec4 outPos4:TEXCOORD1,
   out Half outLen :TEXCOORD2,
   out Vec4 outVtx :POSITION ,

   SOFT_PARAMS
)
{
   Vec  pos=vtx.pos();
   VecH nrm=vtx.nrm();

   outTex=vtx.tex();

   if(!skin)
   {
      pos=TransformPos(pos); nrm+=FurVel[0]; nrm=Normalize(nrm);
      nrm=TransformDir(nrm);
   }else
   {
      VecI bone=vtx.bone();
      pos =TransformPos (pos, bone, vtx.weight());
      nrm+=GetBoneFurVel(     bone, vtx.weight()); nrm=Normalize(nrm);
      nrm =TransformDir (nrm, bone, vtx.weight());
   }
   outPos4=Project(pos); // set in 'outPos4' the original position without expansion
   if(size)outLen=vtx.size();
   pos+=nrm*(size ? vtx.size()*MaterialDetPower()*FurStep.x : MaterialDetPower()*FurStep.x);
   outVtx=Project(pos);
}
/******************************************************************************/
Vec4 Soft_PS
(
   Vec2 inTex :TEXCOORD0,
   Vec4 inPos4:TEXCOORD1,
   Half inLen :TEXCOORD2,

   SOFT_PARAMS
):COLOR
{
   Half fur=Tex(FurCol, inTex*MaterialDetScale()).r;

   VecH4 color;
   color.rgb=Sat(size ? inLen*-fur+FurStep.y    : fur*FACTOR+FurStep.y); // inLen*-fur+step+1 : fur*FACTOR+step+1
   color.a  =Sat(size ? inLen*(1-FurStep.x/fur) : 1-FurStep.x/fur     ); // alternative: Sat(1-FurStep.x/(fur*inLen))

   if(diffuse)color.rgb*=Tex(Col, inTex).rgb;
              color.rgb =(color.rgb*MaterialColor3()+Highlight.rgb)*TexPoint(FurLight, PosToScreen(inPos4)).rgb; // we need to access the un-expanded pixel and not current pixel
   return     color;
}
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
