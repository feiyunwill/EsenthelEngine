/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Bool WorldManager::areaInsertObject(Area &area, Obj &obj, AREA_STATE obj_state)
{
   // test the state of target area
   if(!area.loaded()) // if target area hasn't yet been loaded
   {
      switch(_mode)
      {
         case WORLD_STREAM:
         case WORLD_MANUAL:
         {
            // in stream and manual modes, we flush the object to a temporary file
            // don't delete object, deletion will be handled outside
            if(obj.canBeSaved())area.saveObj(obj);
         }return false; // return false stating that the object should be deleted

         case WORLD_FULL:
         {
            if(!obj.canBeSaved())return false; // WORLD_FULL does not imply any limits to the area dimensions, so just for safety, check if the object wants to be put in that area ('canBeSaved' can be used to skip objects moved too far away)

            File f;
            areaUpdateState  (area, f); // for WORLD_FULL an Area can be not yet loaded, so we need to update its state (to loaded)
            areaSetLoadedRect(       ); // after adding new area we need to update loaded rectangles
         }break;
      }
   }

   obj.putToArea(area);
   if(obj_state!=area.state())
   {
      if(area.state()==AREA_ACTIVE)obj. enable();
      else                         obj.disable();
   }
   return true;
}
/******************************************************************************/
Bool WorldManager::loadObj(Area &area, Bool active, Area::Data::AreaObj &area_obj, Bool _const)
{
      Int          obj_type=ObjType.find(area_obj.type()); // OBJ_TYPE
   if(ObjMap<Obj> *obj_map =objMap(obj_type))
   if(Obj         *obj     =obj_map->_map(area_obj.id))
   {
      obj->init                (_const, obj_type, &area_obj.id);
      obj->create              (area_obj);
      obj->memoryAddressChanged();
      obj->putToArea           (area);
      if(active)obj-> enable();
      else      obj->disable();
     _obj_newly_added.add(obj);
      return true;
   }
   return false;
}
Bool WorldManager::loadObj(Area &area, Bool active, Int obj_type, File &f)
{
   if(ObjMap<Obj> *obj_map=objMap(obj_type))
   if(Obj         *obj    =obj_map->_map(UIDZero)) // we don't know yet the ID of the object, because it's stored in 'obj_save_data', so create with 'UIDZero' and later replace it
   {
         obj->init(false, obj_type);
      if(obj->load(f))
      {
         obj_map->_map.replaceKey(UIDZero, obj->id()); // replace with proper key
         obj->memoryAddressChanged();
         obj->putToArea           (area);
         if(active)obj-> enable();
         else      obj->disable();
        _obj_newly_added.add(obj);
         return true;
      }else
      {
         obj_map->removeObj(obj);
      }
   }
   return false;
}
/******************************************************************************/
Obj* WorldManager::objCreateNear(Object &object, C Matrix &matrix, C UID *obj_id)
{
   Int obj_type=ObjType.find(object.type()); // OBJ_TYPE
   if(ObjMap<Obj> *obj_map=objMap(obj_type))
   {
      UID   oid; if(obj_id)oid=*obj_id;else oid.randomize();
      VecI2 xz=worldToArea(matrix.pos);
      if(Area *area=_grid.get(xz).data())if(area->loaded())
      if(Obj  *obj =obj_map->_map(oid))
      {
         Object child; child.base(&object); child.matrix=matrix;
         obj->init                (false, obj_type, &oid);
         obj->create              (child);
         obj->memoryAddressChanged();
         obj->putToArea           (*area);
         if(area->state()==AREA_ACTIVE)obj->enable ();
         else                          obj->disable();
         return obj;
      }
   }
   return null;
}
Bool WorldManager::objCreate(Object &object, C Matrix &matrix, C UID *obj_id)
{
   Int obj_type=ObjType.find(object.type()); // OBJ_TYPE
   if(ObjMap<Obj> *obj_map=objMap(obj_type))
   {
      UID oid; if(obj_id)oid=*obj_id;else oid.randomize();
      if(Obj *obj=obj_map->_map(oid))
      {
         Object child; child.base(&object); child.matrix=matrix;
             obj->init                (false, obj_type, &oid);
             obj->create              (child);
             obj->memoryAddressChanged();
         if(!obj->updateArea          ())obj_map->removeObj(obj);
         return true;
      }
   }
   return false;
}
Bool WorldManager::objInject(Int obj_type, File &obj_save_data, C Vec *pos)
{
   if(ObjMap<Obj> *obj_map=objMap(obj_type))
   if(Obj *obj=obj_map->_map(UIDZero)) // we don't know yet the ID of the object, because it's stored in 'obj_save_data', so create with 'UIDZero' and later replace it
   {
         obj->init(false, obj_type);
      if(obj->load(obj_save_data))
      {
         obj_map->_map.replaceKey(UIDZero, obj->id()); // replace with proper key
         obj->memoryAddressChanged();
         if(pos)obj->pos(*pos);
         if(!obj->updateArea())obj_map->removeObj(obj);else _obj_newly_added.add(obj);
         return true;
      }
      obj_map->removeObj(obj);
   }
   return false;
}
/******************************************************************************/
Obj* WorldManager::moveWorldObjToStorage(Obj &world_obj, Memx<Obj> &storage)
{
   if(                           world_obj._area)               // if object belongs to an area
   if(ObjMap<Obj> *world_storage=world_obj.worldObjMap())       // if item is stored in world container
   if(             world_storage->elmSize()==storage.elmSize()) // if elements stored in containers are of the same sizes
   {
      Obj &storage_obj=storage.New();          // create new object in the storage
      if(CType(world_obj)==CType(storage_obj)) // if objects are of same class
      {
         world_obj.willBeMovedFromWorldToStorage();         // callback
         world_obj.removeFromArea();                        // remove from area
         Swap(&world_obj, &storage_obj, storage.elmSize()); // swap object data
         world_storage->removeObj(&world_obj);              // remove from world container
         storage_obj.memoryAddressChanged();                // notify that memory address was changed
         storage_obj.wasMovedFromWorldToStorage();          // callback
         return &storage_obj;
      }else
      {
         storage.removeData(&storage_obj);
      }
   }
   return null;
}
Bool WorldManager::moveStorageObjToWorld(Obj &storage_obj, Memx<Obj> &storage, C Matrix *obj_matrix)
{
   if(storage.contains(&storage_obj))                               // if belongs to this storage
   if(ObjMap<Obj> *world_storage=objMap(objType(storage_obj)))      // if world has storage for this object
   if(             world_storage->elmSize()==storage.elmSize())     // if elements stored in containers are of the same sizes
   if(Obj         *world_obj=world_storage->_map(storage_obj.id())) // if created object
   {
      if(CType(storage_obj)==CType(*world_obj)) // if objects are of same class
      {
         storage_obj.willBeMovedFromStorageToWorld();             // callback
         Swap(&storage_obj, world_obj, world_storage->elmSize()); // swap object data
         storage.removeData(&storage_obj);                        // remove from storage
         world_obj->memoryAddressChanged();                       // notify that memory address was changed
         if(obj_matrix)world_obj->matrix(*obj_matrix);            // set matrix
         world_obj->wasMovedFromStorageToWorld();                 // callback

         if(!areaInsertObject(*_grid.get(worldToArea(world_obj->pos())).data(), *world_obj, AREA_UNLOADED)) // add to area
         {
            world_storage->removeObj(world_obj); // remove from world container if object was added to unloaded area
         }
         return true;
      }else
      {
         world_storage->removeObj(world_obj);
      }
   }
   return false;
}
Obj* WorldManager::MoveStorageObjToStorage(Obj &storage_obj, Memx<Obj> &src_storage, Memx<Obj> &dest_storage)
{
   if(&src_storage          ==&dest_storage)return &storage_obj; // same containers
   if( src_storage.elmSize()== dest_storage.elmSize())           // if elements stored in containers are of the same sizes
   if( src_storage.contains(&storage_obj))                       // if belongs to this storage
   {
      Obj &dest_obj=dest_storage.New();       // create new object in world
      if(CType(storage_obj)==CType(dest_obj)) // if objects are of same class
      {
         Swap(&storage_obj, &dest_obj, dest_storage.elmSize()); // swap object data
         src_storage.removeData(&storage_obj);                  // remove from src storage
         dest_obj.memoryAddressChanged();                       // notify that memory address was changed
         return &dest_obj;
      }else
      {  
         dest_storage.removeData(&dest_obj);
      }
   }
   return null;
}
/******************************************************************************/
struct ObjGetBall
{
   Int           obj_type;
   Vec           ball_pos;
   Flt           ball_r2 ; // squared radius for faster calculations
   MemPtr<Obj*> &objects ;

   ObjGetBall(MemPtr<Obj*> &objects, C Ball &ball, Int obj_type) : objects(objects), obj_type(obj_type) {ball_pos=ball.pos; ball_r2=Sqr(ball.r);}
};
static void ObjGetAddBall(Cell<Area> &cell, ObjGetBall &oq)
{
   if(cell().loaded())
   {
      Int         type=oq.obj_type;
      Memc<Obj*> &objs=cell()._objs;
      REPA(objs)
      {
         Obj &o=*objs[i];
         if(type<0 || o.type()==type)if(Dist2(o.pos(), oq.ball_pos)<=oq.ball_r2)oq.objects.add(&o);
      }
   }
}
struct ObjGetCapsule
{
   Int           obj_type;
   Edge          edge    ;
   Flt           radius  ;
   MemPtr<Obj*> &objects ;

   ObjGetCapsule(MemPtr<Obj*> &objects, C Capsule &capsule, Int obj_type) : objects(objects), obj_type(obj_type)
   {
      Vec up=(capsule.h*0.5f-capsule.r)*capsule.up;
      edge.p[0]= capsule.pos-up;
      edge.p[1]= capsule.pos+up;
      radius   = capsule.r;
   }
};
static void ObjGetAddCapsule(Cell<Area> &cell, ObjGetCapsule &oq)
{
   if(cell().loaded())
   {
      Int         type=oq.obj_type;
      Memc<Obj*> &objs=cell()._objs;
      REPA(objs)
      {
         Obj &o=*objs[i];
         if(type<0 || o.type()==type)if(Dist(o.pos(), oq.edge)<=oq.radius)oq.objects.add(&o);
      }
   }
}
struct ObjGetBox
{
   Int           obj_type;
   Box           box     ;
   MemPtr<Obj*> &objects ;
   
   ObjGetBox(MemPtr<Obj*> &objects, C Box &box, Int obj_type) : objects(objects), box(box), obj_type(obj_type) {}
};
static void ObjGetAddBox(Cell<Area> &cell, ObjGetBox &oq)
{
   if(cell().loaded())
   {
      Int         type=oq.obj_type;
      Memc<Obj*> &objs=cell()._objs;
      REPA(objs)
      {
         Obj &o=*objs[i];
         if(type<0 || o.type()==type)if(Cuts(o.pos(), oq.box))oq.objects.add(&o);
      }
   }
}
struct ObjGetOBox
{
   Int           obj_type;
   OBox          obox    ;
   MemPtr<Obj*> &objects ;

   ObjGetOBox(MemPtr<Obj*> &objects, C OBox &obox, Int obj_type) : objects(objects), obox(obox), obj_type(obj_type) {}
};
static void ObjGetAddOBox(Cell<Area> &cell, ObjGetOBox &oq)
{
   if(cell().loaded())
   {
      Int         type=oq.obj_type;
      Memc<Obj*> &objs=cell()._objs;
      REPA(objs)
      {
         Obj &o=*objs[i];
         if(type<0 || o.type()==type)if(Cuts(o.pos(), oq.obox))oq.objects.add(&o);
      }
   }
}
WorldManager& WorldManager::objGetAdd(MemPtr<Obj*> objects, C Ball &ball, Int obj_type)
{
   ObjGetBall oq(objects, ball, obj_type);
  _grid.func(worldToArea(Rect(ball.pos.x-ball.r, ball.pos.z-ball.r, ball.pos.x+ball.r, ball.pos.z+ball.r))&_area_loaded_rect, ObjGetAddBall, oq);
   return T;
}
WorldManager& WorldManager::objGetAdd(MemPtr<Obj*> objects, C Capsule &capsule, Int obj_type)
{
   ObjGetCapsule oq(objects, capsule, obj_type);
  _grid.func(worldToArea(Rect().from(oq.edge.p[0].xz(), oq.edge.p[1].xz()).extend(capsule.r))&_area_loaded_rect, ObjGetAddCapsule, oq);
   return T;
}
WorldManager& WorldManager::objGetAdd(MemPtr<Obj*> objects, C Box &box, Int obj_type)
{
   ObjGetBox oq(objects, box, obj_type);
  _grid.func(worldToArea(box)&_area_loaded_rect, ObjGetAddBox, oq);
   return T;
}
WorldManager& WorldManager::objGetAdd(MemPtr<Obj*> objects, C OBox &obox, Int obj_type)
{
   ObjGetOBox oq(objects, obox, obj_type);
  _grid.func(worldToArea(obox)&_area_loaded_rect, ObjGetAddOBox, oq);
   return T;
}
/******************************************************************************/
Obj* WorldManager::findObjById(C UID &obj_id, Int obj_type)
{
   if(obj_id.valid())
   {
      if(obj_type<0) // search in all containers
      {
         REPA(_obj_container)if(ObjMap<Obj> *obj_map=_obj_container[i].map)if(Obj *obj=obj_map->find(obj_id))return obj;
      }else
      if(ObjMap<Obj> *obj_map=objMap(obj_type))
      {
         return obj_map->find(obj_id);
      }
   }
   return null;
}
/******************************************************************************/
}}
/******************************************************************************/
