/******************************************************************************

   In this tutorial is presented how to combine extending base classes with World Manager usage

/******************************************************************************/
class Player : Game.Chr // extend character class by defining a player class based on the character
{
   Memx<Game.Item> items; // here is the characters inventory, a container of items

   void updateItems()
   {
      if(Kb.bp(KB_1)) // try to pickup an item
         if(Items.elms()) // if world items container has some elements
            itemPickUp(Items[0]); // pick up the first valid item

      if(Kb.bp(KB_2)) // try to drop down an item
         if(items.elms()) // if inventory has some items
            itemDropDown(items[0]); // drop down the first item

      if(!Kb.alt())grabStop();else // if don't want to grab
      {
         if(grab.is()) // if already grabbing
         {
            Vec pos;
            CosSin(pos.x, pos.z, angle.x+PI_2); // set direction according to player angle
            pos *=ctrl.radius()+0.5;            // set radius    according to player controller radius
            pos.y=ctrl.height()*0.4;            // set vertical  position
            pos +=T.pos();
            grab.pos(pos);                      // set desired grab position
         }else
         if(Items.elms()) // if isn't grabbing anything check for presence of world items
         {
            grabStart(Items[0]); // grab first present
         }
      }
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
         flt max=DegToRad(900)*Time.d();
         angle.x-=Mid(Ms.d().x*1.7, -max, max);
         angle.y+=Mid(Ms.d().y*1.7, -max, max);
      }

      updateItems();
      return super.update();
   }

   void itemPickUp(Game.Item &item)
   {
      Game.World.moveWorldObjToStorage(item, items);
   }
   void itemDropDown(Game.Item &item)
   {
      Game.World.moveStorageObjToWorld(item, items);
   }

   virtual bool save(File &f)
   {
      if(super.save(f))
      {
         if(items.save(f))
            return f.ok();
      }
      return false;
   }
   virtual bool load(File &f)
   {
      if(super.load(f))
      {
         if(!items.load(f))return false;
         if(f.ok())return true;
      }
      return false;
   }
}
/******************************************************************************/
Game.ObjMap<Game.Item  > Items  ; // container for item   objects
Game.ObjMap<     Player> Players; // container for player objects
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
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

   Cam.setSpherical(Vec(16, 0, 16), -PI_4, -0.5, 0, 10).set(); // set initial camera

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

   Game.World.update(Cam.at); // update world to given position

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
                     D.text(0, 0.9, "Press WSAD keys to move, 1/2 to pick up/drop item, Alt to grab");
   if(Players.elms())D.text(0, 0.8, S+"Items in inventory : "+Players[0].items.elms());
}
/******************************************************************************/
