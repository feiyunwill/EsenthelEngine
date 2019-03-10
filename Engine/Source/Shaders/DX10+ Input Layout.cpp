/******************************************************************************

   This Shader is used only to create the most basic Vertex Shader which can be used for Input Layout creation.

/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
void VS(VtxInput vtx, out Vec4 O_vtx:POSITION)
{
   O_vtx=vtx.pos4();
}
Vec4 PS():COLOR
{
   return 0;
}
TECHNIQUE(Tech, VS(), PS());
/******************************************************************************/
