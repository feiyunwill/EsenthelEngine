/******************************************************************************/
#include "!Header.h"
#include "Overlay.h"
/******************************************************************************/
#define PARAMS         \
   uniform Bool skin  ,\
   uniform Bool normal
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out Vec     outTex   :TEXCOORD0, // xy=uv, z=alpha
   out Matrix3 outMatrix:TEXCOORD1,
   out Vec4    outVtx   :POSITION ,

   PARAMS
)
{
   Matrix3 m;
   m[0]=OverlayParams.mtrx[0]/Length2(OverlayParams.mtrx[0]);
   m[1]=OverlayParams.mtrx[1]/Length2(OverlayParams.mtrx[1]);
   m[2]=OverlayParams.mtrx[2]/Length2(OverlayParams.mtrx[2]);
   outTex   =mul(m, vtx.pos()-OverlayParams.mtrx[3]);
   outTex.xy=outTex.xy*0.5f+0.5f;
   outTex.z =1 - (Abs(outTex.z)-OverlayOpaqueFrac())/(1-OverlayOpaqueFrac()); // Abs(outTex.z)/-(1-OverlayOpaqueFrac()) + (OverlayOpaqueFrac()/(1-OverlayOpaqueFrac())+1)

   if(!skin)
   {
      outMatrix[1]=Normalize(TransformDir(OverlayParams.mtrx[1]));
      outMatrix[2]=Normalize(TransformDir(OverlayParams.mtrx[2]));

      outVtx=Project(TransformPos(vtx.pos()));
   }else
   {
      VecI bone=vtx.bone();

      outMatrix[1]=Normalize(TransformDir(OverlayParams.mtrx[1], bone, vtx.weight()));
      outMatrix[2]=Normalize(TransformDir(OverlayParams.mtrx[2], bone, vtx.weight()));

      outVtx=Project(TransformPos(vtx.pos(), bone, vtx.weight()));
   }
   outMatrix[0]=Cross(outMatrix[1], outMatrix[2]);
}
/******************************************************************************/
Vec4 PS
(
   Vec     inTex   :TEXCOORD0,
   Matrix3 inMatrix:TEXCOORD1,

   out Vec4 outNrm:COLOR1,

   PARAMS
):COLOR
{
   Vec4 col  =Tex(Col, inTex.xy);
   Flt  alpha=Sat(inTex.z)*OverlayAlpha();

   if(normal)
   {
      Vec4 tex     =Tex(Nrm, inTex.xy); // #MaterialTextureChannelOrder
    //Flt  specular=tex.z*MaterialSpecular();

           VecH nrm;
                nrm.xy =(tex.xy*2-1)*MaterialRough();
    //if(detail)nrm.xy+=det.xy;
                nrm.z  =CalcZ(nrm.xy);
                nrm    =Transform(nrm, inMatrix);

      col.a=tex.w;

   #if SIGNED_NRM_RT
      outNrm.xyz=nrm;
   #else
      outNrm.xyz=nrm*0.5f+0.5f;
   #endif
   }
   col.a*=alpha;
   col  *=MaterialColor();
   if(normal)outNrm.w=col.a;
   return col;
}
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
