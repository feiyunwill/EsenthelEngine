/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#define PARAMS \
   uniform Bool skin
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out Vec4 O_vtx:POSITION,

   PARAMS
)
{
   if(!skin)
   {
      O_vtx=Project(TransformPos(vtx.pos()));
   }else
   {
      O_vtx=Project(TransformPos(vtx.pos(), vtx.bone(), vtx.weight()));
   }
}
/******************************************************************************/
void PS
(
#if MODEL==SM_3
   out Vec4 ret:COLOR,
#endif
   PARAMS
)
{
#if MODEL==SM_3
   ret=0;
#endif
}
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
