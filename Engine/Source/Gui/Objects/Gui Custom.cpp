/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void GuiCustom::zero()
{
  _focusable=false;
}
GuiCustom::GuiCustom() {zero();}
GuiCustom& GuiCustom::del()
{
   super::del(); zero(); return T;
}
GuiCustom& GuiCustom::create(Ptr user)
{
   del();

   T._type   =GO_CUSTOM;
   T._visible=true;
   T. user   =user;

   return T;
}
GuiCustom& GuiCustom::create(C GuiCustom &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type     =GO_CUSTOM;
        _focusable=src._focusable;
      }
   }
   return T;
}
/******************************************************************************/
GuiCustom& GuiCustom::focusable(Bool on) {if(_focusable!=on){_focusable=on; if(!on)kbClear();} return T;}
/******************************************************************************/
Bool GuiCustom::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.cmpUIntV(0); // version

      f<<_focusable;
      return f.ok();
   }
   return false;
}
Bool GuiCustom::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 0:
      {
        _type=GO_CUSTOM;

         f>>_focusable;
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
