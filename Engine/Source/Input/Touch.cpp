/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   On some platforms we need to monitor released touches in order to detect double clicks.

/******************************************************************************/
#define TouchSelectDist2       Sqr(0.025f)
#define TouchSelectBigDist2    Sqr(0.065f)
#define TouchDoubleClickRange2 Sqr(0.080f)
#define StylusSelectTime           0.070f
#define DETECT_DOUBLE_TAP      (!IOS) // iOS already provides the number of taps

#if DETECT_DOUBLE_TAP
struct ReleasedTouch
{
   Dbl  app_time;
   Vec2 pos;

   void set(Dbl app_time, C Vec2 &pos) {T.app_time=app_time; T.pos=pos;}
};
static MemtN<ReleasedTouch, 2> ReleasedTouches;
#endif

MemtN<Touch, 10> Touches;
MouseTouch       MT;
Bool             TouchesSupported;

static Bool TouchesSimulated;
static UInt TouchesID;
/******************************************************************************/
Touch::Touch()
{
   user_type=0;
   user_ptr =null;
  _selecting=_dragging=_scrolling=_remove=false;
  _first=true;
  _state=0;
  _axis_moved=0;
  _id=0;
  _force=0;
  _start_time=0;
  _start_pos=_prev_pos=_pos=_sm_pos=_delta=_abs_delta=_vel=0; _posi=_deltai=0;
  _handle=null;
  _gui_obj=null;
}
Touch& Touch::init(C VecI2 &posi, C Vec2 &pos, CPtr handle, Bool stylus)
{
   if(TouchesID==0)TouchesID=1; _id=TouchesID++; // don't select zero for the ID
  _start_time=Time.appTime();
  _start_pos=_prev_pos=_pos=_sm_pos=pos; _sv_pos.init(pos);
  _posi=posi;
  _handle=handle;
  _gui_obj=Gui.objAtPos(pos);
  _stylus=stylus;
   return T;
}
Touch& Touch::reinit(C VecI2 &posi, C Vec2 &pos)
{
  _start_time=Time.appTime();
  _start_pos =_pos=pos;
  _posi      =posi;
  _gui_obj   =Gui.objAtPos(_pos);
  _axis_moved=0;
   user_type =0;
   user_ptr  =null;
   return T;
}
void Touch::eat()
{
   FlagDisable(_state, BS_NOT_ON);
}
/******************************************************************************/
void MouseTouch::guiObj(Int i, GuiObj *obj) {if(InRange(i, Touches))Touches[i].guiObj(obj);else Gui._ms=Gui._ms_src=obj;}
/******************************************************************************/
Touch* FindTouch(UInt id) // 0 is reserved for mouse
{
   if(id)REPA(Touches)if(Touches[i].id()==id)return &Touches[i];
   return null;
}
Touch* FindTouchByHandle(CPtr handle)
{
   REPA(Touches)if(Touches[i]._handle==handle)return &Touches[i];
   return null;
}
/******************************************************************************/
void SimulateTouches(Bool on)
{
#if DESKTOP
   if(TouchesSimulated!=on)
   {
      Touches.del();
      TouchesSimulated=on;
   }
#endif
}
/******************************************************************************/
Bool SupportedTouches()
{
#if WINDOWS
   return TouchesSupported;
#elif MOBILE
   return true;
#else
   return false;
#endif
}
/******************************************************************************/
void TouchesUpdate()
{
   // simulate touches using mouse
   if(TouchesSimulated)
   {
      CPtr  handle=&Ms      ; // touch handle
      Vec2  pos   = Ms.pos(); // touch position
      VecI2 posi  = Ms.desktopPos();
      if(Ms.bp(0)) // simulate 'touchesBegan'
      {
         Touch *t=FindTouchByHandle(handle); // find existing one
         if(   !t)t=&Touches.New().init(posi, pos, handle, false);else // create new one
         {
            t->_posi=posi;
            t->_pos =pos ;
         }
         t->_state=BS_ON|BS_PUSHED;
         t->_first=!Ms.bd(0);
         if(Ms.bd(0))t->_state|=BS_DOUBLE;
      }else
      if(Ms.b(0)) // simulate 'touchesMoved'
      {
         Touch *t=FindTouchByHandle(handle); // find existing one
         if(   !t) // create new one
         {
      	   t=&Touches.New().init(posi, pos, handle, false);
            t->_state=BS_ON|BS_PUSHED;
         }else
         {
            t->_posi=posi;
            t->_pos =pos ;
         }
         t->_deltai=Ms.pixelDelta();
      }
      if(Ms.br(0) || !Ms.b(0))if(Touch *t=FindTouchByHandle(handle)) // simulate 'touchesReleased'
      {
         t->_posi  =posi;
         t->_pos   =pos ;
         t->_deltai=Ms.pixelDelta();
         t->_remove=true;
         if(t->_state&BS_ON) // check for state in case it was manually eaten
         {
            t->_state|= BS_RELEASED;
            t->_state&=~BS_ON;
            if(Ms.tapped(0))t->_state|=BS_TAPPED;
         }
      }
   }

   // update, calculate delta, detect selecting/dragging/tapping and detect double click
   REPA(Touches)
   {
      Touch &t=Touches[i];
      t._delta    =t._pos-t._prev_pos; t._prev_pos=t._pos;
      t._abs_delta=t._delta*D.scale();
      t._sm_pos   =t._sv_pos.update(t._pos);

      // update velocity
      if(!(t.state()&(BS_PUSHED|BS_RELEASED)) // don't update on push/release because on those events position may not be updated
      || t.ad().any())
         if(Time.ad())t._vel=t._sv_vel.update(t._abs_delta/Time.ad(), Time.ad());

      // dragging
      if(t.on())
      {
         if(!t.selecting())
         {
            Flt dist2=Dist2(t.pos(), t.startPos())*Sqr(D.scale()*(D.smallSize() ? 0.5f : 1.0f));
            if(t.stylus() ? (dist2>=TouchSelectDist2 && t.life()>=StylusSelectTime+Time.ad()) || dist2>=TouchSelectBigDist2 // stylus can be slippery, because of that process it differently (for short distance require time, or allow big distance in case user made long swipe)
                          :  dist2>=TouchSelectDist2                                                                       )t._selecting=true;
         }
         if(!t.dragging() && t.selecting() && t.life()>=DragTime+Time.ad())t._dragging=true;

         // scroll regions
         if(t.id()!=Gui._drag_touch_id) // only if not dragging something
         {
            if(t.vel().any())
            {
               if(Abs(t.vel().x)>Abs(t.vel().y))t._axis_moved|=1;else t._axis_moved|=2;
            }
            if(Region *region=t.guiObj()->firstScrollableRegion())
               if(!region->slidebar[0].contains(t.guiObj()) && !region->slidebar[1].contains(t.guiObj()) && t.guiObj()!=&region->view)
                  t._scrolling=true;
         }
      }else
      if(!t.rs())t._selecting=t._dragging=t._scrolling=false;

      if(t._scrolling) // process for 'on' and 'rs'
         if(Region *region=t.guiObj()->firstScrollableRegion())
      {
         if(t._axis_moved&1)region->scrollX(-t.d().x, true);
         if(t._axis_moved&2)region->scrollY( t.d().y, true);
         if(t.selecting())switch(t.guiObj()->type()) // if enough movement occurred, then switch focus to the region itself, in case it belonged to a child of the region that is clickable, to avoid clicking that object on scroll end, however allow other objects in case we want to drag them
         {
            case GO_BUTTON  :
            case GO_CHECKBOX:
            case GO_COMBOBOX:
            case GO_SLIDEBAR:
            case GO_SLIDER  :
            case GO_TABS    :
            case GO_TEXTBOX :
            case GO_TEXTLINE:
               t.guiObj(region);
            break;
         }
      }

      if(t.rs())
      {
         t._force=0;
         if(!t.selecting() && t.life()<=0.25f+Time.ad())t._state|=BS_TAPPED;

         // scroll regions (don't check for 't.scrolling' here because we want to process dragging region slidebars too and for those the scrolling is not enabled)
         if(t._axis_moved && t.id()!=Gui._drag_touch_id) // only if not dragging something
            if(Region *region=t.guiObj()->firstScrollableRegion())
               if(t.guiObj()!=&region->view)
         {
            Vec2 vel=t.vel()*Vec2((t._axis_moved&1) ? -0.1f : 0.0f, (t._axis_moved&2) ? 0.1f : 0.0f);
            if(region->slidebar[0].contains(t.guiObj()))vel.set(-vel.x, 0);else
            if(region->slidebar[1].contains(t.guiObj()))vel.set(0, -vel.y);
            if(vel.length2()>=Sqr(0.15f))region->scrollX(vel.x).scrollY(vel.y);
         }
      }

   #if DETECT_DOUBLE_TAP
      if(t.pd())REPA(ReleasedTouches) // if pushed then check for possibility of double click
      {
         ReleasedTouch &rt=ReleasedTouches[i];
         if(Time.appTime()-rt.app_time>TouchDoubleClickTime+Time.ad())ReleasedTouches.remove(i);else // too long ago
         if(Dist2(rt.pos, t.pos())*Sqr(D.scale()*(D.smallSize() ? 0.5f : 1.0f))<=TouchDoubleClickRange2) // within range
         {
            t._state|=BS_DOUBLE;
            t._first =false; // disable further double clicks for this touch to prevent 2xDbl from 3xTap (2nd release would be stored, and 3rd tap could trigger additional double)
            ReleasedTouches.remove(i);
            break;
         }
      }
   #endif
   }
}
/******************************************************************************/
void TouchesClear()
{
   // remove or disable "pushed|released|double|tapped" state
   REPA(Touches)
   {
      Touch &t=Touches[i];
   #if DETECT_DOUBLE_TAP
      if(t.rs() && t._first)ReleasedTouches.New().set(t.startTime(), t.startPos()); // detect based on releases (and not '_remove' because hovers may not be removed), only for first clicks
   #endif
      if(t._remove)
      {
         Touches.remove(i, true);
      }else
      {
         t._deltai.zero();
         FlagDisable(t._state, BS_NOT_ON);
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
