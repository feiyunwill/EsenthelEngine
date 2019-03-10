/******************************************************************************/
// Helper functions/classes used for dropping down item
int UtoS(uint x) // return unsigned to signed value
{
   // 0 ->  0
   // 1 ->  1
   // 2 -> -1
   // 3 ->  2
   // 4 -> -2
   // ..
   return (x&1) ? (x+1)>>1 : -int((x+1)>>1);
}
class ChrItemDropDownCuts : PhysCutsCallback
{
   bool      collision=false;
   Game.Chr *chr;

   ChrItemDropDownCuts(Game.Chr &chr) : chr(&chr) {}

   virtual bool hit(ActorInfo &actor_info)
   {
      if(actor_info.obj!=chr){collision=true; return false;} // if encountered actor which isn't the character then report collision
      return true;
   }
}
class ChrItemDropDownSweep : PhysHitCallback
{
   flt       fraction=1; // set full movement fraction at start
   Game.Chr *chr;

   ChrItemDropDownSweep(Game.Chr &chr) : chr(&chr) {}

   virtual bool hit(PhysHit &phys_hit)
   {
      if(phys_hit.obj!=chr)MIN(fraction, phys_hit.frac); // if encountered actor which isn't the character then minimize the movement fraction
      return true;
   }
}
/******************************************************************************/
class Player : Game.Chr
{
   Inventory inv;

   Matrix getItemDropDownMatrix(Item &item) // get matrix for item when wanting to drop it down
   {
      Matrix matrix; matrix.setRotateY(-angle.x).move(ctrl.center()); item.actor.matrix(matrix); // set item actor to an initial matrix

      ChrItemDropDownCuts cuts(T); item.actor.cuts(cuts);
      if(cuts.collision) // test if the item actor already cuts some other actor in the world
      {
         Vec d; CosSin(d.x, d.z, angle.x+PI_2); d*=ctrl.radius(); d.y=ctrl.height()*0.25;
         matrix+=d; // move the matrix slightly
      }
      else // if the item actor doesn't cut any other actor, we'll try to move it further away from the player
      {
         flt max   =-1,
             length=ctrl.radius()+0.3;
         Vec move;
         FREPD(x, 5) // try 5 horizontal attempts
          REPD(y, 3) // try 3 vertical   attempts
         {
            int  sx=UtoS(x), // 0, 1, -1, 2, -2
                 sy=y-1    ; // 2, 1, 0
            Vec  d; CosSin(d.x, d.z, angle.x+PI_2+sx*PI_3); d*=length; d.y=sy*0.5; // set actor movement
            ChrItemDropDownSweep sweep(T); item.actor.sweep(d, sweep);             // test for any obstacles along the movement
            if(sweep.fraction>max){max=sweep.fraction; move=d;}                    // if this move was the furthest than last remembered then store its distance and movement
         }
         matrix+=move; // move the matrix
      }
      return matrix;
   }
   void itemPickUp(Item &item)
   {
      if(Game.Obj *inv_item=Game.World.moveWorldObjToStorage(item, inv.items)) // after this call don't operate on 'item' because it may be invalid
         inv.itemAdded(); // if moving world obj to inventory succeeded then notify about it the inventory
   }
   void itemDropDown(Item &item)
   {
      inv.itemRemoved(item);
      if(Game.World.moveStorageObjToWorld(item, inv.items, &getItemDropDownMatrix(item)))
         inv.itemRemoved();
   }

   // update / draw
   virtual bool update()
   {
      if(super.update())
      {
         // detect if the player wants to pickup an item
         if(Lit && Ms.bp(0)) // if hase highlighted object and mouse button pressed
            if(Item *item=CAST(Item, Lit)) // if the object is an item
               if(item.icon) // pick up items only with icons
                  itemPickUp(*item); // pick it up

         // update the inventory
         inv.update(T);

         return true;
      }
      return false;
   }
   virtual uint drawPrepare()
   {
               inv.drawPrepare();
      return super.drawPrepare();
   }
   virtual void drawShadow()
   {
               inv.drawShadow();
      return super.drawShadow();
   }

   // io
   bool save(File &f)
   {
      if(super.save(f))
      {
         if(inv.save(f))
            return f.ok();
      }
      return false;
   }
   bool load(File &f)
   {
      if(super.load(f))
      {
         if(inv.load(f))
            if(f.ok())return true;
      }
      return false;
   }

   Player()
   {
      InvGui.link(&inv); // when creating a player automatically link him with InvGui
   }
}
/******************************************************************************/
