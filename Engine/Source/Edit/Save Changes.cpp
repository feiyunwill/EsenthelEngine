/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
Bool SaveChanges::Elm::doSaveClose() {if(_save )if(!_save(T))return false; doClose(); return true;} // if failed to save then don't close
void SaveChanges::Elm::doClose    () {if(_close)_close(T);}
/******************************************************************************/
void SaveChanges::create()
{
   Gui+=super::create("Save changes to following elements?").hide(); button[2].show(); flag|=WIN_RESIZABLE;
   T  +=region.create();
   T  +=save  .create("Yes"   ).func(Save  , T); save.kbSet();
   T  +=close .create("No"    ).func(Close , T);
   T  +=cancel.create("Cancel").func(Cancel, T);

   ListColumn lc[]=
   {
      ListColumn(MEMBER(Elm, display), LCW_DATA, "name"),
   };
   region+=list.create(lc, Elms(lc), true).elmHeight(0.035f).textSize(0.035f);

   rect(Rect_C(0, 0, 1.1f, 0.57f));
}
/******************************************************************************/
void SaveChanges::set(C MemPtr<Elm> &elms, void (*after_save_close)(Bool all_saved, Ptr user), Ptr user)
{
   T. elms            =elms;
   T._after_save_close=after_save_close;
   T._user            =user;
   list.setData(T.elms);
   if(elms.elms())activate();else{if(_after_save_close)_after_save_close(true, _user); hide();}
}
/******************************************************************************/
void SaveChanges::clear  () {_after_save_close=null; list.clear(); elms.del(); hide();}
void SaveChanges::doSave () {Bool all_saved=true; FREPA (elms)all_saved&=elms[i].doSaveClose(); if(_after_save_close)_after_save_close(all_saved, _user); clear();} // save  in specified order
void SaveChanges::doClose() {                     FREPAO(elms)                  .doClose    (); if(_after_save_close)_after_save_close(true     , _user); clear();} // close in specified order
/******************************************************************************/
Rect    SaveChanges::sizeLimit(            )C {Rect r=super::sizeLimit(); r.min.set(0.9f, 0.3f); return r;}
Window& SaveChanges::rect     (C Rect &rect)
{
   super::rect(rect);
   save  .rect(Rect_D(clientWidth()*0.25f, -clientHeight()+0.035f, 0.22f, 0.05f));
   close .rect(Rect_D(clientWidth()*0.50f, -clientHeight()+0.035f, 0.22f, 0.05f));
   cancel.rect(Rect_D(clientWidth()*0.75f, -clientHeight()+0.035f, 0.22f, 0.05f));
   region.rect(Rect  (0.05f, save.rect().max.y+0.025f, clientWidth()-0.05f, -0.025f));
   return T;
}
/******************************************************************************/
}}
/******************************************************************************/
