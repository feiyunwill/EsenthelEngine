/******************************************************************************

   Game objects can contain custom parameters,
      this tutorial will present how to access them.

/******************************************************************************/
class Item : Game.Item // extend items
{
   int value=0; // add new parameter 'value'
   Str name   ; // add new parameter 'name'

   virtual void create(Object &obj) // extend default creation
   {
      super.create(obj); // default create

      // now setup custom parameters from 'obj'
      if(Param *par=obj.findParam("name" ))name =par.asText();
      if(Param *par=obj.findParam("value"))value=par.asInt ();
   }

   // io methods
   virtual bool save(File &f)
   {
      if(super.save(f)) // default save
      {
         f<<value<<name; // save custom parameters
         return f.ok();
      }
      return false;
   }
   virtual bool load(File &f)
   {
      if(super.load(f)) // if default load was successful
      {
         f>>value>>name; // load custom parameters
         if(f.ok())return true; // return success
      }
      return false; // return failure
   }
}
/******************************************************************************/
Game.ObjMap<Item> Items;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.3).viewRange(50);

   Cam.at.set(16, 0, 16);
   Cam.dist = 10;
   Cam.yaw  =-0.3;
   Cam.pitch=-0.2;
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .setObjType(Items, OBJ_ITEM)  // set 'Items' memory container for 'OBJ_ITEM' objects
             .New(UID(4053788456, 1284500709, 3533893555, 3086486877));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

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
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
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

   // draw item parameters
   REPA(Items)
   {
      Item  &item  =Items[i];                // get i-th Items
      Vec2   screen=PosToScreen(item.pos()); // convert world position to screen position
      D.text(screen, item.name);             // draw item's name
   }
}
/******************************************************************************/
