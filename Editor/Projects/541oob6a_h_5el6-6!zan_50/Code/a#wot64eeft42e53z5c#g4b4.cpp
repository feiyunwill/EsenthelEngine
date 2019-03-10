/******************************************************************************/
enum SLOTS
{
   SLOT_TEMP , // temporary slot (current item moved with mouse cursor)
   SLOT_HEAD , // head      slot
   SLOT_ARM_L, // left  arm slot
   SLOT_ARM_R, // right arm slot
   SLOT_BODY , // body      slot
   SLOT_NUM  , // number of slots
}
/******************************************************************************/
class Inventory
{
   Memx     <Item> items         ; // all character items are stored in this container
   Reference<Item> slot[SLOT_NUM]; // these are references to items which are placed at certain slots
   InventoryGui   *inv_gui=null  ; // pointer to InventoryGui linked with Inventory

  ~Inventory() {if(inv_gui)inv_gui.unlink();} // unlink from gui

   // get
   bool slotCanBePutTo(int src, int dest) // test if slot 'src' can be put to 'dest' slot , 'src dest' are slot indexes (SLOTS)
   {
      if(!InRange(src, SLOT_NUM) || !InRange(dest, SLOT_NUM))return false;

      if(slot[src].valid())switch(dest)
      {
         case SLOT_HEAD:
         case SLOT_BODY:
            if(slot[src]().type==ITEM_WEAPON)return false;
         break;
      }

      return true;
   }
   bool slotsCanBeSwapped(int a, int b) // test if slot 'a' can be swapped with slot 'b', 'a b' are slot indexes (SLOTS)
   {
      return slotCanBePutTo(a, b) && slotCanBePutTo(b, a);
   }

   // operations
   void itemRemoved(Game.Obj &item) // called when an item is being removed
   {
      // perform check if it is a equipped item
      REPA(slot) // for all slots
         if(slot[i]==item) // if i-th slot is set to item which is being removed
            slot[i].clear(); // clear the slot so it can no longer be referenced to the removed item
   }
   void itemRemoved() // called when an item has been removed
   {
      setGui();
   }
   void itemAdded() // called when an item has been added
   {
      setGui();
   }
   void setGui() // update visual gui components
   {
      if(inv_gui)inv_gui.setGui();
   }

   // update
   void update(Player &owner) // handle moving items with mouse and setting equipped item matrixes
   {
      if(inv_gui)inv_gui.update(owner);

      // set matrixes for items in hands
      if(slot[SLOT_ARM_L].valid())
         if(C OrientP *hand=owner.skel.findSlot("HandL"))
            slot[SLOT_ARM_L]().matrix(Matrix().setPosDir(hand.pos, hand.perp, hand.dir));

      if(slot[SLOT_ARM_R].valid())
         if(C OrientP *hand=owner.skel.findSlot("HandR"))
            slot[SLOT_ARM_R]().matrix(Matrix().setPosDir(hand.pos, hand.perp, hand.dir));
   }

   // draw
   void drawPrepare() // draw equipped items in 3d world
   {
      // draw items in hands
      if(slot[SLOT_ARM_L].valid())slot[SLOT_ARM_L]().drawPrepare();
      if(slot[SLOT_ARM_R].valid())slot[SLOT_ARM_R]().drawPrepare();
   }
   void drawShadow() // draw equipped items in 3d world
   {
      // draw items in hands
      if(slot[SLOT_ARM_L].valid())slot[SLOT_ARM_L]().drawShadow();
      if(slot[SLOT_ARM_R].valid())slot[SLOT_ARM_R]().drawShadow();
   }

   // io
   bool save(File &f)
   {
      if(items.save(f)) // save items
      {
         FREPA(slot) // for all slots
            f.putInt(items.validIndex(&slot[i]())); // store the valid index of i-th slot item in 'items' container
         return f.ok();
      }
      return false;
   }
   bool load(File &f)
   {
      if(items.load(f))
      {
         FREPA(slot) // for all slots
         {
            int item_index=f.getInt(); // read index of i-th slot in 'items' container

            if(InRange(item_index, items)) // if the index is in valid range
               slot[i]=items[item_index];  // set the slot to point to requested item
            else
               slot[i].clear(); // clear the item reference in i-th slot
         }
         if(f.ok())
         {
            setGui();
            return true;
         }
      }
      return false;
   }
}
/******************************************************************************/
