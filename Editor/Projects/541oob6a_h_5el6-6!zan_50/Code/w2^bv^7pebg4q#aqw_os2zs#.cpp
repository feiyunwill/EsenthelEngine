/******************************************************************************/
class Player : Game.Chr
{
   Motion attack;

   virtual void animate() // extend skeleton animation
   {
      super.animate(); // call default animations

      // now the skeleton is animated with default character animations (walking, running, crouching, ..)
      // so after the basic animations we can optionally replace them with custom ones, for example attacking:
      skel.animate(attack, true); // animate skeleton with 'attack' animation motion, 'true' for replace animation mode
   }

   virtual bool update()
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
         Flt max=DegToRad(900)*Time.d();
         angle.x-=Mid(Ms.d().x*1.7, -max, max);
         angle.y+=Mid(Ms.d().y*1.7, -max, max);
      }

      // update animation
      {
         if(Kb.bp(KB_ENTER)) // on enter pressed
            attack.set(skel, UID(4036032190, 1277766898, 1630466228, 321593117)); // initialize "right-hand swing to left direction" attack animation

         attack.updateAuto(3, 3, 1); // update attack animation motion
      }

      return super.update();
   }
   virtual bool save(File &f)
   {
      if(super.save(f))
      {
         if(attack.save(f))
            return f.ok();
      }
      return false;
   }
   virtual bool load(File &f)
   {
      if(super.load(f))
      {
         if(attack.load(f, skel))
            if(f.ok())return true;
      }
      return false;
   }
}
/******************************************************************************/
Game.ObjMap<Game.Item  > Items;
Game.ObjMap<     Player> Players;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.3);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType(Items  , OBJ_ITEM)
             .setObjType(Players, OBJ_CHR );
   Game.World.New(UID(4053788456, 1284500709, 3533893555, 3086486877));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   Cam.setSpherical(Vec(16,0,16), -PI_4, -0.5f, 0, 10).set();

   return true;
}
/******************************************************************************/
void Shut()
{
   Game.World.del();
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Game.World.update(Cam.at);
   return true;
}
/******************************************************************************/
void Render()
{
   Game.World.draw();
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, "Press Enter to play a custom animation");
}
/******************************************************************************/
