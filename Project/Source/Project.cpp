/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
void InitPre()
{
   App.name("Project");
   App.flag=APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_RESIZABLE|APP_FULL_TOGGLE;
#ifdef DEBUG
   App.flag|=APP_MEM_LEAKS|APP_BREAKPOINT_ON_ERROR;
#endif
   DataPath("../Data");
   //Paks.add("engine.pak");
}
Bool Init()
{
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
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text(0, 0.9f, S+Time.fps());
}
/******************************************************************************/
