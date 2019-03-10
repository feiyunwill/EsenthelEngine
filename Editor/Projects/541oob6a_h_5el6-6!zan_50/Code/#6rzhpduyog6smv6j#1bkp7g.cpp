/******************************************************************************

   In this tutorial is presented how to properly use references to world objects using 'Reference' class.

   Let's assume a Character wants to reference an Item laying on the ground.

   So first let's think how we would do this the easy way:

      Add a pointer to an item?

         class Chr : Game.Chr
         {
            Item *item=null;
            ...

      This looks ok, but what happens if 'item' points to the Item object,
      and then during a Game Update, the Item Object is removed/deleted.

         For example another player picked up the item,
         so item's memory address has moved to a inventory container.
         Or the item has become too far from the center of action,
         that the world had to flush the item object to a temporary file on the disk.

      In that scenario 'Chr.item' still points to a memory address but at which
      there is no original Item.

   This means that additional caution needs to be taken.
   To solve this problem we'll use object's 'id' value.

   Before going any further let's sum up how objects are handled in the world:

      -When each object is created it has its 'id' set to a random value different than zero

      -When each object is deleted its 'id' is set to zero

      -Object's 'id' values are saved and loaded in SaveGame

      -After removing/deleting a world object, its memory is still accessible, but:
         -it's not used at all ('id' value is zero)
         -it's used but by another object created later ('id' value is set to new object's 'id')

   So to properly reference an object we must store a pointer to its memory and its 'id',
   in case the referenced objects memory will be used by another object later with another 'id'.

   For this - 'Reference' class is used which stores a pointer and an 'id'.

   So for example let's use a World Character and a World Item.
   The Character will hold a reference to the item as 'desired_item'.

/******************************************************************************/
class Chr : Game.Chr // extend default character
{
   Reference<Game.Item> desired_item; // add a new parameter, which is a reference to an item (something like "Item *desired_item")

   // draw
   void draw2D() // this is a helper method for drawing text on the screen
   {
      if(desired_item.valid()) // check if the reference is valid
      {
         Vec pos=desired_item().pos(); // referenced objects can be accessed using operator(), but only after validation using 'valid' method as above
         D.text(0, 0.8, S+"My desired item is at: ("+pos+") position");
      }
      else // if the reference has become invalid
      {
         D.text(0, 0.8, "My desired item is gone"); // display a text that the reference is now invalid
      }
   }

   // handle saving and loading, following methods are needed only for saving and loading the Reference
   virtual bool save(File &f)
   {
      if(super.save(f))
      {
         if(desired_item.save(f)) // save Reference
            return f.ok();
      }
      return false;
   }
   virtual bool load(File &f)
   {
      if(super.load(f))
      {
         if(desired_item.load(f)) // load Reference, here only the Reference 'id' is loaded, the pointer to the object is not set because the object may not exist yet
                                  // searching for the actual object will be performed later (inside 'linkReferences') after all objects have been loaded
            if(f.ok())return true;
      }
      return false;
   }
   virtual void linkReferences() // this is called when all of the objects have been loaded
   {
      // in this method you should call 'link' on all references which the object contains
      desired_item.link(Game.World); // by calling 'link' the world manager will try to find the referenced object according to the 'id', and store its pointer into the Reference
   }
};
/******************************************************************************/
Game.ObjMap<Game.Item> Items;
Game.ObjMap<     Chr > Chrs;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.3);

   Cam.dist =10;
   Cam.yaw  =-PI_4;
   Cam.pitch=-0.5;
   Cam.at.set(16, 0, 16);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType (Items, OBJ_ITEM)
             .setObjType (Chrs , OBJ_CHR )
             .New        (UID(4053788456, 1284500709, 3533893555, 3086486877));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   // after loading the world, let's set the 'desired_item' for the Character
   {
      Game.World.update(Cam.at); // first world needs to be updated to a location to make sure that objects at that location are loaded

      if(Chrs.elms() && Items.elms()) // now check if the world has at least one character and one item
         Chrs[0].desired_item=Items[0]; // set the Character's reference to point to the first Item (this stores the item's memory address and its 'id')
   }

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
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1) ? CAMH_MOVE : CAMH_ROT));

   if(Kb.bp(KB_F2))Game.World.save("save.sav"); // save game
   if(Kb.bp(KB_F3))Game.World.load("save.sav"); // load game

   Game.World.update(Cam.at);

   if(Kb.bp(KB_SPACE) && Items.elms()) // when space pressed and if World Items has elements
      Items.remove(0); // remove 0-th item

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

   D.text(0, 0.9, "Press Space to remove the item");
   if(Chrs.elms())Chrs[0].draw2D(); // draw characters text
}
/******************************************************************************/
