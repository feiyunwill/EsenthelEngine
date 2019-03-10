/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   'PtrCounter' value means:
      -1 =unloaded
       0 =loaded and     listed in delay remove
     >=1 =loaded and not listed in delay remove

/******************************************************************************/
#define DELAY_REMOVE_STEP (1.0f/8) // number of steps per 'delay_remove' time to check for element removal

struct DelayRemove
{
   PtrCounter *data;
   void      (*unload)(Ptr elm);
   Flt         time;
};

static Memc<DelayRemove> DelayRemoves;
static Flt              _DelayRemoveTime, DelayRemoveCheck;
/******************************************************************************/
static Int        FindDelayRemove(Ptr  data) {REPA(DelayRemoves)if(DelayRemoves[i].data==data)return i; return -1;}
static void RemoveFromDelayRemove(Ptr  data) {DelayRemoves.remove(FindDelayRemove(data));}
static void    ProcessDelayRemove(Bool always)
{
   if(DelayRemoves.elms())
   {
      DelayRemoveCheck=Time.appTime()+_DelayRemoveTime*DELAY_REMOVE_STEP; // perform next check at this time
      REPA(DelayRemoves)
      {
         DelayRemove &remove=DelayRemoves[i];
         if(always || Time.appTime()>=remove.time) // if always remove or enough time has passed (use >= so when having zero delay time then it will be processed immediately)
         {
            PtrCounter *data=remove.data; void (*unload)(Ptr elm)=remove.unload; DelayRemoves.remove(i); // access before removal and remove afterwards
            DEBUG_ASSERT(data->_ptrs==0, "'ProcessDelayRemove' '_ptrs' should be zero");
            data->_ptrs=-1; unload(data); // set '_ptrs' before calling 'unload'
         }
      }
   }
}
void DelayRemoveNow   () {                                                           ProcessDelayRemove(true );}
void DelayRemoveUpdate() {if(DelayRemoves.elms() && Time.appTime()>=DelayRemoveCheck)ProcessDelayRemove(false);}
void DelayRemoveTime  (Flt time)
{
   Bool adjust_existing=true;
   MAX(time, 0); if(time!=_DelayRemoveTime)
   {
      Flt delta=time-_DelayRemoveTime; // how much are we increasing the delay
     _DelayRemoveTime  =time; // set new value
      DelayRemoveCheck+=delta*DELAY_REMOVE_STEP; // adjust check time
      if(adjust_existing)REPAO(DelayRemoves).time+=delta; // adjust element removal times
      DelayRemoveUpdate();
   }
}
/******************************************************************************/
PtrCounter::~PtrCounter() {if(_ptrs==0){_ptrs=-1; RemoveFromDelayRemove(this);}} // if object is being deleted, but it's marked to be removed later, then remove it from the list

void PtrCounter::decRef(void (*unload)(Ptr elm))
{
   if(this) // this method is called for null objects too
   {
      DEBUG_ASSERT(_ptrs>0, "'PtrCounter.decRef' Decreasing '_ptrs' when it's already zero");
      if(!--_ptrs) // if there are no more pointers accessing this element
      {
         if(_DelayRemoveTime>0) // use delay remove
         {
            DelayRemove &delay_remove=DelayRemoves.New();
            delay_remove.data  =this;
            delay_remove.unload=unload;
            delay_remove.time  =Time.appTime()+_DelayRemoveTime;
         }else // remove now
         {
           _ptrs=-1; unload(this); // set '_ptrs' before calling 'unload'
         }
      }
   }
}
void PtrCounter::incRef(Bool (*load)(Ptr elm))
{
   if(this) // this method is called for null objects too
      switch(_ptrs)
   {
      case -1: _ptrs=1;                  load(this); break; // if unloaded    , adjust '_ptrs' before anything else
      case  0: _ptrs=1; RemoveFromDelayRemove(this); break; // if delay remove, adjust '_ptrs' before anything else
      default: _ptrs++;                              break;
   }
}
/******************************************************************************/
}
/******************************************************************************/
