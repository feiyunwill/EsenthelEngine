/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
void Chr::ragdollValidate()
{
   if(!ragdoll.is()) // if ragdoll hasn't yet been created
   {
      ragdoll.create(skel      )  // create from 'AnimatedSkeleton'
             .obj   (this      )  // set Game.Chr object
             .ignore(ctrl.actor); // ignore collisions with the character controller
   }
}
/******************************************************************************/
void Chr::ragdollEnable()
{
   if(ragdoll_mode!=RAGDOLL_FULL)
   {
      ragdollValidate(); // make sure the ragdoll is created

      ctrl.actor.active  (false); // disable character controller completely
      ragdoll   .active  (true )  // enable ragdoll actors
                .gravity (true )  // gravity should be enabled for full ragdoll mode
                .fromSkel(skel, ctrl.actor.vel()); // set ragdoll initial pose

      ragdoll_mode=RAGDOLL_FULL;
   }
}
/******************************************************************************/
void Chr::ragdollDisable()
{
   if(ragdoll_mode==RAGDOLL_FULL)
   {
      ragdoll   .active(false);
      ctrl.actor.active(true );

      ragdoll_mode=RAGDOLL_NONE;
   }
}
/******************************************************************************/
Bool Chr::ragdollBlend()
{
   if(ragdoll_mode!=RAGDOLL_FULL)
   {
      ragdollValidate(); // make sure the ragdoll is created

      ragdoll.active  (true ) // enable ragdoll collisions
             .gravity (false) // disable gravity for hit-simulation effects because they look better this way
             .fromSkel(skel, ctrl.actor.vel()); // set ragdoll initial pose

      ragdoll_time=0;
      ragdoll_mode=RAGDOLL_PART;

      return true; // ragdoll set successfully
   }
   return false; // can't set ragdoll mode
}
/******************************************************************************/
}}
/******************************************************************************/
