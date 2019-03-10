/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
void Chr::actionBreak() // break current action
{
   action=ACTION_NONE;
   path.clear();

   // clear input
   input.move.zero();
   input.turn.zero();
}
/******************************************************************************/
Bool Chr::actionMoveTo(C Vec &pos) // set 'move to' action
{
   // break any current action
   actionBreak();

   // try to set new action
   path_target=pos;
   if(World.path().find(T.pos(), path_target, path)) // if found a path to the target destination
   {
      path.reverseOrder();
      action=ACTION_MOVE_TO; // set new action
      return true;
   }
   return false;
}
/******************************************************************************/
void Chr::actionMoveDir(C Vec &dir) // set 'move to direction' action
{
   // break any current action
   actionBreak();

   // set new action
   action  =ACTION_MOVE_DIR;
   move_dir=dir.xz();
}
/******************************************************************************/
#define ANGLE_INC 10.0f
#define   DIR_INC 10.0f
#define   DIR_DEC  4.0f
/******************************************************************************/
// set character 'input' according to 'from to' angles and character speeds
// returns true  - when the character is near the target angle and can proceed moving forward
// returns false - if it should stay still and rotate only
static Bool InputToAngle(Chr &chr, Flt from, Flt to, Flt turn_speed, Flt desired_speed)
{
   Flt d=AngleDelta(from,to),
      ad=Abs       (d      );
   if(ad<0.001f)chr.input_turn.x=chr.input.turn.x=0;else
   {
      Flt div=turn_speed*Time.d();
      if( div>EPS)
      {
         Flt can   =22*Time.d(),
             max   =Lerp(0.1f, 3.0f, Mid((ad-DegToRad(5))/DegToRad(40), 0.0f, 1.0f)),
             want_d=d/div,
             d     =want_d-chr.input_turn.x;
         Clamp(d               , -can, can); chr.input_turn.x+=d;
         Clamp(chr.input_turn.x, -max, max);
         chr.input.turn.x=Sign(chr.input_turn.x);
      }
   }
   
   Flt limit=DegToRad(90);
   if(desired_speed>EPS)limit/=Max(1, desired_speed/4.1f);
   if(   turn_speed>EPS)limit/=Max(1,    turn_speed/3.5f);

   return ad<limit;
}
static Bool ChrToAngle(Chr &chr, C Vec2 &dir) {return InputToAngle(chr, chr.angle.x+PI_2, Angle(dir), chr.turn_speed, chr.desiredSpeed());}
static Bool ChrToAngle(Chr &chr, C Vec  &pos) {return   ChrToAngle(chr, pos.xz()-chr.ctrl.actor.pos().xz());}
/******************************************************************************/
static void StopAngleInput(Chr &chr) // stop angle input
{
   Flt d=ANGLE_INC*Time.d();
   AdjustValDir(chr.input_turn.x, chr.input.turn.x=0, d);
   AdjustValDir(chr.input_turn.y, chr.input.turn.y=0, d);
}
static void StopDirInput(Chr &chr) // stop direction movement input
{
   Flt inc=DIR_INC*Time.d(),
       dec=DIR_DEC*Time.d();
   AdjustValDir(chr.input_move.x, chr.input.move.x=0, inc, dec);
   AdjustValDir(chr.input_move.z, chr.input.move.z=0, inc, dec);
   AdjustValDir(chr.input_move.y, chr.input.move.y=0, inc, dec);
}
static void StopInput(Chr &chr) // stop angle and direction movement input
{
   StopAngleInput(chr);
   StopDirInput  (chr);
}
static void ActionBreak(Chr &chr)
{
   chr.actionBreak();
   StopInput(chr);
}
/******************************************************************************/
static void UpdateMoveDir(Chr &chr, Vec2 &dir)
{
   chr.input.move.x=0;
   chr.input.move.z=ChrToAngle(chr, dir); // if the character is facing the target angle then move forward
   chr.input.move.y=0;

   Flt inc=DIR_INC*Time.d(),
       dec=DIR_DEC*Time.d();
   AdjustValDir(chr.input_move.x, chr.input.move.x, inc, dec);
   AdjustValDir(chr.input_move.z, chr.input.move.z, inc, dec);
   AdjustValDir(chr.input_move.y, chr.input.move.y, inc, dec);
}
static void UpdateMoveTo(Chr &chr)
{
   Vec  from=chr.ctrl.actor.pos(); // get current position
   Bool path_searched=false;
   for(;;)
   {
      if(!chr.path.elms()) // if don't have a waypoint
      {
         if(!path_searched) // if didn't yet searched
         {
            path_searched=true; // don't try anymore searches
            if(World.path().find(from, chr.path_target, chr.path))chr.path.reverseOrder(); // search the path
         }
         if(!chr.path.elms()) // if still don't have a waypoint
         {
            ActionBreak(chr); // break the action
            break;            // return
         }
      }

      Vec2 to =chr.path.last().xz(), // get the next waypoint
           dir=to-from.xz();         // set direction from current to the next position

      if(dir.length()<=chr.ctrl.radius()) // we've reached the waypoint
      {
         chr.path.removeLast(); // remove this point
      }else
      {
         UpdateMoveDir(chr, dir); // go in direction of the waypoint
         break;                   // return
      }
   }
}
/******************************************************************************/
void Chr::updateAction()
{
   // update input according to actions
   switch(action)
   {
      case ACTION_MOVE_TO: // move to target
      {
         input.jump  =0;
         input.dodge =false;
         input.crouch=false;
         input.walk  =move_walking;
         UpdateMoveTo(T); // update the 'move to' action
      }break;

      case ACTION_MOVE_DIR: // move in direction
      {
         input.jump  =0;
         input.dodge =false;
         input.crouch=false;
         input.walk  =move_walking;
         UpdateMoveDir(T, move_dir);
      }break;

      default: // manual input
      {
         Flt inc, dec;
         if(input.adjust_move)
         {
            inc=DIR_INC*Time.d();
            dec=DIR_DEC*Time.d();
            AdjustValDir(input_move.x, input.move.x, inc, dec);
            AdjustValDir(input_move.z, input.move.z, inc, dec);
            AdjustValDir(input_move.y, input.move.y, inc, dec);
         }
         if(input.adjust_turn)
         {
            inc=ANGLE_INC*Time.d();
            AdjustValDir(input_turn.x, input.turn.x, inc);
            AdjustValDir(input_turn.y, input.turn.y, inc);
         }
      }break;
   }

   // update character according to input
   {
      // update angles
      angle+=input_turn*(Time.d()*turn_speed);
      Clamp(angle.y, -PI_2, PI_2); // limit vertical angle to -PI_2..PI_2 range

      // dodge (dodge is a fast strafe acivated by tapping Left or Right direction twice)
      dodge_availability-=Time.d(); if(input.dodge && !ctrl.crouched() && ctrl.onGround() && dodge_availability<=0)
      {
         dodging           =input.dodge;
         dodge_step        =1;
         dodge_availability=0.5f;
      }
   }
}
/******************************************************************************/
void Chr::updateController()
{
   // set movement velocity
   Vec velocity;
   if(dodging) // set according to dodging
   {
      velocity.y=0; CosSin(velocity.x, velocity.z, angle.x); if(dodging<0)velocity.chs();
      velocity.setLength(speed*2.5f*Pow(dodge_step, 0.25f));
      dodge_step-=Time.d()*3.33f; if(dodge_step<=0)dodging=0;
   }
   else // set according to the input
   {
      // set desired 'wish' vector movement according to the angles and movement
      Flt ax =angle.x+PI_2,
          ay =angle.y;
      Vec dir=input_move;
      if(!ctrl.flying())
      {
         ay   =0;
         dir.y=0;
      }
      Flt cx, sx, cy, sy;
      CosSin(cx, sx, ax);
      CosSin(cy, sy, ay);
      Vec wish(dir.z*cx*cy + dir.x*sx, dir.z*sy + dir.y,
               dir.z*sx*cy - dir.x*cx);

      // slow down when running backwards
      Flt length=1; if(dir.z<0)length=Lerp(Lerp(1.0f, 0.75f, anim.walk_run), 1.0f, dir.z+1); // if dir.z is in range -1..0
      wish.clipLength(length);

      // set final velocity according to 'wish' vector and desired speed
      velocity=wish*desiredSpeed();
   }

   // update character controller
   ctrl.update(velocity, ctrl.crouched() ? input.crouch : (anim.stand_crouch>=1), input.jump);
}
/******************************************************************************/
}}
/******************************************************************************/
