/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
_Undo::_Undo(Bool full, Ptr user, Flt time)
{
   del();
   T._full=full;
   T. user=user;
   T. time=time;
  _max_changes=-1; _max_mem_usage=-1; 
}
_Undo& _Undo::del()
{
  _changes.del();
  _undos=_change_frame=0; _change_time=0; _change_type=null;
   return T;
}
_Undo& _Undo::forceCreateNextUndo()
{
  _change_type =null;
  _change_time =0;
  _change_frame=0;
   return T;
}
/******************************************************************************/
void _Undo::clean(Bool test_mem_usage)
{
        test_mem_usage  &=(_max_mem_usage>=0); // only if it's enabled
   Bool test_change_limit=(_max_changes  >=0);
   Long mem_usage; Memt<UInt> mem_usages;
   if(test_mem_usage)
   {
      mem_usage=0; mem_usages.setNum(_changes.elms()); REPA(mem_usages){UInt u=_changes[i].memUsage(); mem_usages[i]=u; mem_usage+=u;}
   }
   for(; test_mem_usage    &&  mem_usage     >_max_mem_usage
      || test_change_limit && _changes.elms()>_max_changes; )
   {
      if(undos()>redos()) // remove first undo
      {
        _undos--;
        _changes.removeValid(0, true);
         if(test_mem_usage){mem_usage-=mem_usages[0]; mem_usages.remove(0, true);}
      }else // remove last redo
      {
        _changes.removeLast();
         if(test_mem_usage){mem_usage-=mem_usages.last(); mem_usages.removeLast();}
      }
   }
}
/******************************************************************************/
Long _Undo::memUsage()C
{
   Long   usage=0; REPA(_changes)usage+=_changes[i].memUsage();
   return usage;
}
_Undo& _Undo::maxMemUsage(Long limit)
{
   if(limit<-1)limit=-1;
   if(_max_mem_usage!=limit)
   {
      Long old_limit=_max_mem_usage;
     _max_mem_usage=limit;
      if(_max_mem_usage>=0 && (_max_mem_usage<old_limit || old_limit<0))clean(true); // if new limit is smaller then potentially we need to remove some changes
   }
   return T;
}
_Undo& _Undo::maxChanges(Int limit)
{
   if(limit<-1)limit=-1;
   if(_max_changes!=limit)
   {
      Int old_limit=limit;
     _max_changes=limit;
      if(_max_changes>=0 && (_max_changes<old_limit || old_limit<0))clean(false); // if new limit is smaller then potentially we need to remove some changes
   }
   return T;
}
/******************************************************************************/
_Undo::Change* _Undo::getNextUndo() {return _changes.addr(undos()-1);}
_Undo::Change* _Undo::getNextRedo() {return _changes.addr(undos()  );}
/******************************************************************************/
_Undo::Change* _Undo::set(CPtr change_type, Bool force_create, Flt extra_time)
{
   Change *change=null;

   // remove all redos
   for(; _changes.elms()>undos(); )_changes.removeLast();

   if(!undos() // no undos yet
   ||  force_create // force create
   || _change_type!=change_type // new change type
   || (Time.frame()!=_change_frame && Time.frame()!=_change_frame+1 && Time.appTime()>_change_time+extra_time+Time.ad()) // enough time has passed (check the frames too in case the frame took a long time)
   )
   {
      // clean if we have to
      clean(true);

      // add new change
     _undos++;
      change=&_changes.New(); if(_full)change->create(user); // create only in full mode
   }else
   if(!_full) // in partial mode we need to re-use the last change
   {
      // return last one
      change=&_changes.last();
   }

   // adjust settings (do this always to keep up the continuousness)
  _change_type =change_type;
  _change_frame=Time.frame();
  _change_time =Time.appTime()+time;

   return change;
}
_Undo::Change* _Undo::set(Int change_type, Bool force_create, Flt extra_time)
{
   return set((CPtr)change_type, force_create, extra_time);
}
/******************************************************************************/
Bool _Undo::undo()
{
   if(undos()) // if there are any Undo Changes
   {
      if(_full && undos()==changes())if(set(null, true, 0))_undos--; // in full mode if we're at the end then we need to save current state
      if(undos()) // if there are still any Undo Changes after above codes
      {
         forceCreateNextUndo(); // call this after saving current state with 'set' because that modifies the parameters
         Change &change=_changes[--_undos]; // change the counter before calling 'apply' so inside it will be detected with correct info if undos/redos are available
         if(_full)change.apply(user);else change.swap(user);
         return true;
      }
   }
   return false;
}
Bool _Undo::redo()
{
   if(_full && undos()<changes()-1)_undos++; // go forward
   if(InRange(undos(), changes())) // if there are still any Redo Changes
   {
      forceCreateNextUndo();
      Change &change=_changes[_undos++]; // change the counter before calling 'apply' so inside it will be detected with correct info if undos/redos are available
      if(_full)
      {
         change.apply(user);
         if(undos()==changes())_changes.removeLast(); // if we're at the end, then there's no need to keep this change anymore
        _undos--; // revert the ++ counter change
      }else change.swap(user);
      return true;
   }
   return false;
}
Bool _Undo::undoAsChange()
{
   if(_full && undos()) // if there are any Undo Changes, currently supported only in full mode
   {
      set(null, true, 0); // always save current state
      if(undos()>1) // if there are still any Undo Changes (including the original one and the new created for saving state)
      {
         forceCreateNextUndo(); // call this after saving current state with 'set' because that modifies the parameters
        _changes[undos()-2].apply(user); return true;
      }
   }
   return false;
}
/******************************************************************************/
} // Edit
} // EE
/******************************************************************************/
