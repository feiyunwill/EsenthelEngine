/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Grab::zero()
{
  _immediate=false;
  _power=0;
  _local_pos.zero();
  _actor=null;
}
Grab::Grab() {zero();}
/******************************************************************************/
Grab& Grab::del()
{
   if(is())
   {
     _joint.del();
     _grab .del();
      zero();
   }
   return T;
}
Grab& Grab::create(Actor &actor, C Vec &local_pos, Flt power)
{
   del();

   T._local_pos= local_pos;
   T._actor    =&actor;

   Vec world=local_pos*actor.matrix();
   T._grab.create   (Ball(0.1f, world), 100, null, true) // increase density (this is needed for Bullet)
          .collision(false)
          .ray      (false);

   T._power    =power;
   T._immediate=true;
   T._joint.createSpherical(actor, &_grab, world, Vec(0, 1, 0));

   return T;
}
Grab& Grab::pos(C Vec &pos)
{
   if(is())
   {
      Vec   world    =_local_pos*_actor->matrix();
      Bool  immediate=(!_actor->cuts() && !_actor->sweep(pos-world));
      if(T._immediate!=immediate)
      {
         T._immediate=immediate;
         if(immediate)
         {
           _grab .pos(world);
           _joint.createSpherical(*_actor, &_grab, world, Vec(0, 1, 0));
         }else
         {
           _joint.createDist(*_actor, &_grab, _local_pos, VecZero, 0, 0, &Spring(_power, 5));
         }
      }
     _grab.kinematicMoveTo(pos);
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
