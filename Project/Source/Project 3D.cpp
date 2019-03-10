/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
void InitPre()
{
   App.name("Project 3D");
   App.flag=APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_RESIZABLE|APP_FULL_TOGGLE;
#ifdef DEBUG
   App.flag|=APP_MEM_LEAKS|APP_BREAKPOINT_ON_ERROR;
#endif
   DataPath("../Data");
   //Paks.add("engine.pak");

   Cam.dist=1;
}
Bool Init()
{
   Sky.atmospheric();
   return true;
}
void Shut()
{
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   if(!App.active())Time.wait(1);
   Cam.transformByMouse(0.01, 1000, CAMH_ZOOM|(Ms.b(0) ? CAMH_MOVE_XZ : Ms.b(1) ? CAMH_MOVE : CAMH_ROT));
   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
      }break;

      case RM_SHADOW:
      {
      }break;
   }
}
void Draw()
{
   Renderer.wire=Kb.b(KB_TILDE);
   Renderer(Render);
   D.text(0, 0.9f, S+Time.fps());
}
/******************************************************************************/
