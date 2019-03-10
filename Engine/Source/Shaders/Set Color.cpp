/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#define PARAMS            \
   uniform Bool skin     ,\
   uniform Int  textures ,\
   uniform Bool tesselate
/******************************************************************************/
struct VS_PS
{
   Vec2 tex:TEXCOORD0;
   Vec  pos:TEXCOORD1;
   VecH nrm:TEXCOORD2;
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
   if(textures)O.tex=vtx.tex();

   if(!skin)
   {
       if(tesselate)O.nrm=Normalize(TransformDir(vtx.nrm()));
                    O.pos=          TransformPos(vtx.pos());
   }else
   {
      VecI bone=vtx.bone();
      if(tesselate)O.nrm=Normalize(TransformDir(vtx.nrm(), bone, vtx.weight()));
                   O.pos=          TransformPos(vtx.pos(), bone, vtx.weight());
   }
   O_vtx=Project(O.pos);
}
/******************************************************************************/
// PS
/******************************************************************************/
Vec4 PS
(
   VS_PS I,

   PARAMS
):COLOR
{
   if(textures==1)clip(Tex(Col, I.tex).a+(MaterialAlpha()-1));else
   if(textures==2)clip(Tex(Nrm, I.tex).a+(MaterialAlpha()-1)); // #MaterialTextureChannelOrder

   return Highlight;
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
               O.pos=I[cp_id].pos;
               O.nrm=I[cp_id].nrm;
   if(textures)O.tex=I[cp_id].tex;
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
   if(textures)O.tex=I[0].tex*B.z + I[1].tex*B.x + I[2].tex*B.y;

   SetDSPosNrm(O.pos, O.nrm, I[0].pos, I[1].pos, I[2].pos, I[0].nrm, I[1].nrm, I[2].nrm, B, hs_data, false, 0);
   O_vtx=Project(O.pos);
}
#endif
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
