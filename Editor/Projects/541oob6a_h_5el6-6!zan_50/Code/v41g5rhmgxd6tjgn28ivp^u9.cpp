/******************************************************************************/
class Player : Game.Chr
{
   virtual Bool update()
   {
      if(action)
      {
         if(Kb.b(KB_W) || Kb.b(KB_S) || Kb.b(KB_A) || Kb.b(KB_D) || Kb.b(KB_Q) || Kb.b(KB_E))actionBreak();
      }

      if(!action)
      {
         // turn & move
         input.turn.x=Kb.b(KB_Q)-Kb.b(KB_E);
         input.turn.y=Kb.b(KB_T)-Kb.b(KB_G);
         input.move.x=Kb.b(KB_D)-Kb.b(KB_A);
         input.move.z=Kb.b(KB_W)-Kb.b(KB_S);
         input.move.y=Kb.b(KB_SPACE)-Kb.b(KB_LSHIFT);

         // dodge, crouch, walk, jump
         input.dodge = Kb.bd(KB_D)-Kb.bd(KB_A);
         input.crouch= Kb.b (KB_LSHIFT);
         input.walk  = Kb.b (KB_LCTRL );
         input.jump  =(Kb.bp(KB_SPACE ) ? 3.5 : 0);

         // mouse turn
         Flt  max=DegToRad(900)*Time.d(),
              dx =Ms.d().x*1.7,
              dy =Ms.d().y*1.7;
         angle.x-=Mid(dx, -max, max);
         angle.y+=Mid(dy, -max, max);
      }

      return super.update();
   }
}
/******************************************************************************/
