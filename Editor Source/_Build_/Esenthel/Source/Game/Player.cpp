/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   bool Player::update()
   {
      if(action) // if performing automatic action
      {
         if(Kb.b(Kb.qwerty(KB_W)) || Kb.b(Kb.qwerty(KB_S)) || Kb.b(Kb.qwerty(KB_A)) || Kb.b(Kb.qwerty(KB_D)) || Kb.b(KB_SPACE) || Kb.b(KB_LSHIFT) || DPad.axis.any() || DPadY.dir || Rot.delta.any()) // if any command key is pressed
         {
            actionBreak(); // break action
         }
      }

      if(!action) // modify input only when there is no automatic action
      {
         // turn & move
         input.turn.x=Kb.b(Kb.qwerty(KB_Q))-Kb.b(Kb.qwerty(KB_E));
         input.turn.y=Kb.b(Kb.qwerty(KB_T))-Kb.b(Kb.qwerty(KB_G));
         input.move.x=Kb.b(Kb.qwerty(KB_D))-Kb.b(Kb.qwerty(KB_A))+DPad.axis.x;
         input.move.z=Kb.b(Kb.qwerty(KB_W))-Kb.b(Kb.qwerty(KB_S))+DPad.axis.y;
         input.move.y=Kb.b(       KB_SPACE)-Kb.b(      KB_LSHIFT)+DPadY.dir;

         // dodge, crouch, walk, jump
         input.dodge =  Kb.bd(Kb.qwerty(KB_D))-Kb.bd(Kb.qwerty(KB_A));
         input.crouch=((Kb.b (KB_LSHIFT) || DPadY.dir==-1) && !ctrl.flying());
         input.walk  =  Kb.b (KB_LCTRL );
         input.jump  =((Kb.bp(KB_SPACE ) || DPadY.dir==+1) ? 3.5f : 0);

         // change flying
         if(Kb.bp(Kb.qwerty(KB_F)))ctrl.flying(!ctrl.flying());

         // adjust turn speed
         turn_speed=((ctrl.flying() && !input.walk && input.move.z==1) ? 1 : 3.5f);

         // mouse turn
         if(ViewMode!=VIEW_ISO)
         {
            flt max =DegToRad(900)*Time.d(),
                dx  =Ms.d().x*1.5f,
                dy  =Ms.d().y*1.5f;
            angle.x-=Mid(dx, -max, max);
            angle.y+=Mid(dy, -max, max);
         }
         angle.x-=Rot.delta.x*1.5f;
         angle.y+=Rot.delta.y*1.5f;
      }

      return ::EE::Game::Chr::update();
   }
/******************************************************************************/
