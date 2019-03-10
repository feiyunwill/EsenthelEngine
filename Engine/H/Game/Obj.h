/******************************************************************************

   Use 'Obj' as a base class for creating your own custom game object classes.

/******************************************************************************/
namespace Game{
/******************************************************************************/
struct Obj // Game Object interface inherited by all Game Object classes (Game.Static, Game.Kinematic, Game.Chr, Game.Item, ..)
{
   // manage
   virtual void create(Object &obj)=NULL; // create from object

   // get / set
           Int  type      ();                  // get object's OBJ_TYPE
           Bool isConst   ()C {return _const;} // if  object is constant
         C UID& id        ()C {return _id   ;} // get object's id, 'id' is a unique identifier different than 'UIDZero' for valid objects, and 'UIDZero' for invalid objects (invalid object is an object which has been removed/deleted), id's are randomly generated upon object creation (in World Editor or dynamically in the game), they are stored in SaveGame so object's id is restored when loaded
   virtual Bool canBeSaved()  {return  true ;} // if  object wants to be saved, you can override this method and return false when object is useless and doesn't need to be stored in a SaveGame. This method is also called for objects in worlds initialized with WORLD_FULL, when objects enter new areas (that were not yet created), the engine will call this method and place the object in that area only if 'canBeSaved' will return true, in other case the object will get deleted, WORLD_FULL doesn't impose any limits to the number of areas created, that's why you can use this method to make sure that objects will not travel to very distant areas.
   virtual Obj* reliesOn  ()  {return  null ;} // you can override this method and optionally return an object on which the current object relies on, this will make the WorldManager to try to update the returned object before the current one, this can be useful for example in situation when animation of object A is dependent on animation of object B, for example if player is riding a horse, then horse animations must be setup first (horse must be updated before player) so animating player can be done basing on the position of the horse, in this situation the player class should override 'reliesOn' method and return the horse object, so that horse will be updated first

   virtual Vec    pos   (                )=NULL; // get position
   virtual void   pos   (C Vec    &pos   )=NULL; // set position
   virtual Matrix matrix(                )=NULL; // get matrix
   virtual void   matrix(C Matrix &matrix)=NULL; // set matrix, for most classes 'matrix' should be normalized

   // callbacks
   virtual void memoryAddressChanged() {} // called when object memory address has been changed, you should override it and adjust Actor.obj pointer for all actors

   virtual void willBeMovedFromWorldToStorage() {} // called before object was moved from world                   to custom object container
   virtual void    wasMovedFromWorldToStorage() {} // called after  object was moved from world                   to custom object container
   virtual void willBeMovedFromStorageToWorld() {} // called before object was moved from custom object container to world
   virtual void    wasMovedFromStorageToWorld() {} // called after  object was moved from custom object container to world

   virtual void linkReferences() {} // this method is called after loading all game objects, you should override it and call 'link' on every Reference that is referencing external objects

   // update
   virtual Bool update()=NULL; // update, return false when object wants to be deleted, and true if wants to exist

   // draw
   virtual UInt drawPrepare ()=NULL; // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh.draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)
   virtual void drawShadow  () {}    // in this method you should add the meshes to the shadow draw list (Mesh.drawShadow)
   virtual void drawOverlay () {}    // in this method you should draw custom objects for the RM_OVERLAY  mode, this method will be called only if the 'drawPrepare' has requested RM_OVERLAY  mode
   virtual void drawBlend   () {}    // in this method you should draw custom objects for the RM_BLEND    mode, this method will be called only if the 'drawPrepare' has requested RM_BLEND    mode
   virtual void drawPalette () {}    // in this method you should draw custom objects for the RM_PALETTE  mode, this method will be called only if the 'drawPrepare' has requested RM_PALETTE  mode
   virtual void drawPalette1() {}    // in this method you should draw custom objects for the RM_PALETTE1 mode, this method will be called only if the 'drawPrepare' has requested RM_PALETTE1 mode
   virtual void drawSolid   () {}    // in this method you should draw custom objects for the RM_SOLID    mode, this method will be called only if the 'drawPrepare' has requested RM_SOLID    mode
   virtual void drawAmbient () {}    // in this method you should draw custom objects for the RM_AMBIENT  mode, this method will be called only if the 'drawPrepare' has requested RM_AMBIENT  mode
   virtual void drawOutline () {}    // in this method you should draw custom objects for the RM_OUTLINE  mode, this method will be called only if the 'drawPrepare' has requested RM_OUTLINE  mode
   virtual void drawBehind  () {}    // in this method you should draw custom objects for the RM_BEHIND   mode, this method will be called only if the 'drawPrepare' has requested RM_BEHIND   mode

   // enable / disable
   virtual void disable(); // disable object when becomes too far away, here all dynamic actors should  enable kinematic flag - actor.kinematic(true ), this is called when an object changes its state to AREA_INACTIVE
   virtual void  enable(); //  enable object when closes in           , here all dynamic actors should disable kinematic flag - actor.kinematic(false), this is called when an object changes its state to AREA_ACTIVE

   // io
   virtual Bool save(File &f); // save, false on fail
   virtual Bool load(File &f); // load, false on fail

  ~Obj();
   Obj();

#if EE_PRIVATE
   void         init          (Bool _const, Int type, C UID *id=null) {T._const=_const; T._type=type; if(id)T._id=*id;}
   void         clearUpdate   ();
   ObjMap<Obj>* worldObjMap   ();
   void         removeFromArea();
   void              putToArea(Area &area);
   Bool             updateArea();

   friend struct WorldManager;
#endif

#if !EE_PRIVATE
private:
#endif
   Bool  _const       ;
   Byte  _update_count;
   Int   _type        ; // index of world container in which object is stored (equal to OBJ_TYPE)
   UID   _id          ; // object unique identifier
   Area *_area        ; // area                     in which object is stored
   NO_COPY_CONSTRUCTOR(Obj);
};
/******************************************************************************/
} // namespace
/******************************************************************************/
