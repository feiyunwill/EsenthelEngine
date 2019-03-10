/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Aggregate::del()
{
#if PHYSX
   if(_aggr)
   {
      SafeWriteLock lock(Physics._rws);
      if(_aggr)
      {
         if(Physx.physics)_aggr->release();
        _aggr=null;
      }
   }
#endif
}
void Aggregate::create(Int max_actors, Bool self_collision)
{
   del();
#if PHYSX
   WriteLock lock(Physics._rws);
   if(Physx.physics)
      if(_aggr=Physx.physics->createAggregate(max_actors, self_collision))Physx.world->addAggregate(*_aggr);
#endif
}
void Aggregate::add(Actor &actor)
{
#if PHYSX
   if(_aggr && actor._actor)
   {
      WriteLock lock(Physics._rws);
      if(_aggr && actor._actor && Physx.world)
      {
         Physx.world->removeActor(*actor._actor); // actor needs to be first removed from the scene
        _aggr->addActor(*actor._actor); // now add to aggregate
      }
   }
#endif
}
/******************************************************************************/
}
/******************************************************************************/
