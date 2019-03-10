/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
#if 0
   #define LOG(x) LogN(x)
#else
   #define LOG(x)
#endif
/******************************************************************************

   on start the areas have state AREA_UNLOADED

   during update:
      -nearest  areas are instantly loaded to AREA_ACTIVE and AREA_INACTIVE
      -neighbor areas are collected to list, and in secondary thread they are loaded into memory, at the moment of loading their state is changed into AREA_CACHE

/******************************************************************************/
Bool WorldManager::update_objects_after_physics=true, WorldManager::use_background_loading=true, WorldManager::use_early_z_for_terrain=false, WorldManager::low_memory_usage=false;
     WorldManager World;
/******************************************************************************/
// WORLD SETTINGS
/******************************************************************************/
WorldSettings& WorldSettings::  areaSize(Flt size) {T._area_size =            Max(size, EPS            )   ; return T;}
WorldSettings& WorldSettings::    hmRes (Int res ) {T._hm_res    =NearestPow2(Mid(res , 1, MAX_HM_RES  ))+1; return T;}
WorldSettings& WorldSettings::path2DRes (Int res ) {T._path2d_res=            Mid(res , 1, MAX_PATH_RES)   ; return T;}

Bool WorldSettings::compatible(C WorldSettings &settings)C
{
   return Equal(areaSize(), settings.  areaSize())
       &&         hmRes ()==settings.    hmRes ()
       &&     path2DRes ()==settings.path2DRes ();
}
Bool WorldSettings::operator==(C WorldSettings &settings)C
{
   return Equal(areaSize(), settings.  areaSize())
       &&         hmRes ()==settings.    hmRes ()
       &&     path2DRes ()==settings.path2DRes ();
}
Str WorldSettings::asText()C
{
   return S
      +"Area Size: "+areaSize()+'\n'
      +"Heightmap Resolution: "+(hmRes()-1)+'\n'
      +"2D Path Resolution: "+path2DRes();
}
WorldSettings& WorldSettings::reset()
{
   environment=null;
  _area_size  =32;
    _hm_res   =65;
  _path2d_res =32;
   return T;
}
WorldSettings& WorldSettings::shr()
{
     areaSize(  areaSize()   /2);
       hmRes ((   hmRes ()-1)/2);
   path2DRes (path2DRes ()   /2);
   return T;
}
WorldSettings& WorldSettings::shl()
{
     areaSize(  areaSize()   *2);
       hmRes ((   hmRes ()-1)*2);
   path2DRes (path2DRes ()   *2);
   return T;
}
Bool WorldSettings::save(File &f, CChar *path)C
{
   f.cmpUIntV(1); // version
   f<<_area_size<<_hm_res<<_path2d_res;
   f._putStr(environment.name(path));
   return f.ok();
}
Bool WorldSettings::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 1:
      {
         f>>_area_size>>_hm_res>>_path2d_res; environment.require(f._getStr(), path);
         if(f.ok())return true;
      }break;

      case 0:
      {
         f>>_area_size>>_hm_res>>_path2d_res; environment=null;
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
Bool WorldSettings::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool WorldSettings::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
/******************************************************************************/
// RESOURCES
/******************************************************************************/
static void DelayRemoveInc()
{
   Environments.delayRemoveInc();
   Objects     .delayRemoveInc();
   Meshes      .delayRemoveInc();
   PhysBodies  .delayRemoveInc();
   WaterMtrls  .delayRemoveInc();
   Materials   .delayRemoveInc();
   ImageAtlases.delayRemoveInc();
   Images      .delayRemoveInc();
}
static void DelayRemoveDec()
{
   Environments.delayRemoveDec();
   Objects     .delayRemoveDec();
   Meshes      .delayRemoveDec();
   PhysBodies  .delayRemoveDec();
   WaterMtrls  .delayRemoveDec();
   Materials   .delayRemoveDec();
   ImageAtlases.delayRemoveDec();
   Images      .delayRemoveDec();
}
/******************************************************************************/
// WORLD MANAGER
/******************************************************************************/
static Bool WorldThread(Thread &thread) {((WorldManager*)thread.user)->threadFunc(); return true;}
       void WorldManager::threadFunc()
{
  _thread_event.wait(); // load data only on demand
   if(_area_background.elms()) // if want some areas to be loaded
   {
      // important: do not create new grid objects, but use already created in the main thread, and now access them using 'area_background' array (creating grid objects on secondary thread would require 'SyncLocker' for every operation on grid)
      File f; // declare the file outside the loop, to make use of performance optimization when using the same file loading files from 1 pak
      ThreadMayUseGPUData();
   again:
      SyncLocker locker(_lock);
      if(_area_background.elms())
      {
         Area &area=*_area_background.pop();
         areaCache(area, false, f);
         if(_area_background.elms())goto again;
      }
      ThreadFinishedUsingGPUData();
   }
}
/******************************************************************************/
WorldManager& WorldManager::mode(WORLD_MODE mode)
{
   if(T.mode()!=mode)
   {
      del();
      T._mode=mode;
   }
   return T;
}
void WorldManager::setRanges()
{
   Int rangei_prev=T._rangei;
   T._rangei          =Trunc   (    T._range/areaSize()+2.2f);
   T._rangei2         =RoundPos(Sqr(T._rangei+0.5f         )); // +  0.5
   T._rangei2_inactive=RoundPos(Sqr(T._rangei+0.5f+SQRT2   )); // + ~1.9
   T._rangei2_cache   =RoundPos(Sqr(T._rangei+1.5f+SQRT2   )); // + ~2.9
   if(_mode==WORLD_STREAM && _updated && rangei_prev!=T._rangei)areaUpdateState();
}
WorldManager& WorldManager::activeRange(Flt range)
{
   if(T._range!=range)
   {
      T._range=range;
      setRanges();
   }
   return T;
}
WorldManager& WorldManager::_setObjType(ObjMap<Obj> &obj_map, Int obj_type, CPtr c_type)
{
   SyncLocker locker(_lock);
   if(obj_type>=0)T._obj_container(obj_type).set(obj_map, c_type);
   return T;
}
/******************************************************************************/
static void UnloadAreas(Cell<Area> &cell, WorldManager &world) {world.areaUnload(cell());}
WorldManager& WorldManager::del()
{
   if(is())
   {
      SyncLocker locker(_lock);

      // unload areas in case they want save data to file
      updateObjectAreas(); // first update object areas so they are placed in correct areas (this is needed if object position was modified just before this place in code)
     _grid.func(UnloadAreas, T);

      REPA(_obj_container)if(ObjMap<Obj> *obj_map=_obj_container[i].map)obj_map->clear(); // delete objects, use 'clear' in case there are any 'Reference' objects left pointing to memory adresses inside containers

     _area_active_rect.set(0, 0, -1, -1);
     _area_loaded_rect.set(0, 0, -1, -1);
     _area_active     .del(); _area_draw.del(); _area_draw_shadow.del(); _area_draw_secondary.del();
     _area_inactive   .del();
     _area_cache      .del();
     _area_background .del();

              _path.del();
              _grid.fastAccess(null).del();
         _path_node.del();
     _path_neighbor.del();
            _decals.del();
     _mesh_overlays.del();
           mini_map.del();
          _settings.reset();

     _id       .zero();
     _name     .del();
     _data_path.del();
     _xz .zero();
     _xzi.zero();
     _updated=false;

      Time.skipUpdate();
   }
   return T;
}
Bool WorldManager::NewTry(C Str &world_name)
{
   del();
   if(!world_name.is())return true;

   SyncLocker locker(_lock);

   if(FExist(world_name))
   {
      if(!DecodeFileName(world_name, _id))_id.zero();
      T._name=world_name;
      T._data_path=world_name; _data_path.tailSlash(true);

      // load settings
      {
        _settings.reset();
         File f; if(!f.readTry(dataPath()+"Settings") || !_settings.load(f))goto error;
         setRanges(); // after loading settings we need to update the ranges, because they're dependent on world area size
      }

      // path finder
      if(!_path.create(_settings.areaSize()))Exit("Can't initialize PathWorld");

      // mini maps
      mini_map.load(world_name); // this is OK to fail as it is optional

      // start background loading thread
      if(D.canUseGPUDataOnSecondaryThread()) // only if we can operate on GPU on secondary threads
         if(!_thread.created())_thread.create(WorldThread, this, 0, false, "EE.Game.World");

      return true;
   }
error:
   del(); return false;
}
WorldManager& WorldManager::New(C Str &world_name)
{
   if(!NewTry(world_name))Exit(MLT(S+"Can't load world \""        +world_name+"\".",
                               PL,S+u"Nie można wczytać świata \""+world_name+"\"."));
   return T;
}
Bool          WorldManager::NewTry(C UID &world_id) {return NewTry(world_id.valid() ? _EncodeFileName(world_id) : null);}
WorldManager& WorldManager::New   (C UID &world_id) {return New   (world_id.valid() ? _EncodeFileName(world_id) : null);}

void WorldManager::Create(C Str &world_name, C WorldSettings &settings)
{
     FCreateDirs(world_name+"/Area"    );
   settings.save(world_name+"/Settings");
}
/******************************************************************************/
// GET
/******************************************************************************/
Int WorldManager::objType(Obj &obj)
{
   CPtr obj_c_type=CType(obj);
   if(InRange(obj._type, _obj_container))if(obj_c_type==_obj_container[obj._type].type)return obj._type  ; // check if obj.type precisely matches the object container class
                    REPA(_obj_container) if(obj_c_type==_obj_container[i        ].type)return obj._type=i; // search all object containers for matching type
   return -1;
}
AREA_STATE WorldManager::rangeState(C VecI2 &xzi)
{
   switch(_mode)
   {
      case WORLD_STREAM:
      {
         Int dist =Dist2(T._xzi, xzi);
         if( dist<=_rangei2         )return AREA_ACTIVE  ;
         if( dist<=_rangei2_inactive)return AREA_INACTIVE;
         if( dist<=_rangei2_cache   )return AREA_CACHE   ;
                                     return AREA_UNLOADED;
      }
      case WORLD_FULL: return AREA_ACTIVE;
   }
   return AREA_UNLOADED;
}
Int   WorldManager::areaActiveNum(           )C {return _area_active.elms();}
Area* WorldManager::areaActive   (  Int    i )C {return InRange(i, _area_active) ? _area_active[i] : null;}
Area* WorldManager::areaActive   (C VecI2 &xz)C
{
   if(Cuts(xz, _area_active_rect))
      if(Cell<Area> *cell=_grid.find(xz))
         if(Area *area=cell->data())
            if(area->state()==AREA_ACTIVE)
               return area;
   return null;
}
Area* WorldManager::areaLoaded(C VecI2 &xz)C
{
   if(Cuts(xz, _area_loaded_rect))
      if(Cell<Area> *cell=_grid.find(xz))
         if(Area *area=cell->data())
            if(area->state()!=AREA_UNLOADED)
               return area;
   return null;
}
Flt WorldManager::hmHeight(C Vec2 &xz, Bool smooth)
{
   if(Area *area=areaLoaded(worldToArea(xz)))return area->hmHeight(xz, smooth);
   return 0;
}
C MaterialPtr& WorldManager::hmMaterial(C Vec2 &xz)
{
   if(Area *area=areaLoaded(worldToArea(xz)))return area->hmMaterial(xz);
   return MaterialNull;
}
static WaterMtrlPtr WaterMtrlGlobal=&Water;
C WaterMtrlPtr& WorldManager::waterUnder(C Vec &pos, Flt *depth)
{
   // water plane
   if(Water.draw)
   {
      Flt d=Dist(pos, Water.plane);
      if( d<=0)
      {
         if(depth)*depth=d;
         return WaterMtrlGlobal;
      }
   }

   // water areas
   if(Area *area=areaLoaded(worldToArea(pos)))
      if(Area::Data *data=area->data())
         REPA(data->waters)
            if(data->waters[i].under(pos, depth))
               if(C WaterMtrlPtr &mtrl=data->waters[i].material())return mtrl;else return WaterMtrlGlobal; // WaterMesh material can return null in which case Water is used

   return WaterMtrlNull;
}
/******************************************************************************/
// SET
/******************************************************************************/
void WorldManager::setShader()
{
   SyncLocker locker(_lock);
   REPAO(_area_active  )->setShader();
   REPAO(_area_inactive)->setShader();
   REPAO(_area_cache   )->setShader();
}
/******************************************************************************/
// LOAD / UNLOAD
/******************************************************************************/
void WorldManager::areaUnload(Area &area)
{
   if(area.state())
   {
#if DEBUG
   Dbl t=Time.curTime();
#endif

      if(area.loaded())
      {
         area.saveObjs();
      }

      // path
     _path.set(null, area.xz());

      // data
      if(area.data())
      {
         // save changes
         if(area.data()->customSaveWant())
         {
            File f; if(area.data()->save(f.writeMem()))
            {
               Str path=dataPath()+"Area/"; if(!FExistSystem(path))FCreateDirs(path);
               f.pos(0); SafeOverwrite(f, path+area.xz(), null, area_cipher);
            }
         }
         Delete(area._data);
      }

      // objects
      REPA(area._objs)
      {
         Obj &obj=*area._objs[i];
         if(ObjMap<Obj> *obj_map=obj.worldObjMap())obj_map->removeObj(&obj);
      }
      area._objs.del();

      // remove
      switch(area.state())
      {
         case AREA_ACTIVE  : _area_active  .exclude(&area); break;
         case AREA_INACTIVE: _area_inactive.exclude(&area); break;
         case AREA_CACHE   : _area_cache   .exclude(&area); break;
      }
      area._state=AREA_UNLOADED;

#if DEBUG
  _time_area_update_state_unload+=Time.curTime()-t;
#endif
   }
}
void WorldManager::areaUnloadToCache(Area &area)
{
   if(area.loaded())
   {
#if DEBUG
   Dbl t=Time.curTime();
#endif

      if(area.loaded())
      {
         if(Area::Data *data=area._data)
         {
            data->customUnloadToCache();
            data->actor.del();
            REPAO(data->terrain_objs).actor.del();
            REPA (data->foliage_objs){Area::Data::GrassObj &go=data->foliage_objs[i]; REPAO(go.instances).actor.del();}
         }
         area.saveObjs();
      }

      // path
     _path.set(null, area.xz());

      // objects
      REPA(area._objs)
      {
         Obj &obj=*area._objs[i];
         if(ObjMap<Obj> *obj_map=obj.worldObjMap())obj_map->removeObj(&obj);
      }
      area._objs.del();

      // remove
      switch(area.state())
      {
         case AREA_ACTIVE  : _area_active  .exclude(&area); break;
         case AREA_INACTIVE: _area_inactive.exclude(&area); break;
      }
     _area_cache.add(&area);
      area._state=AREA_CACHE;

#if DEBUG
  _time_area_update_state_unload_cache+=Time.curTime()-t;
#endif
   }
}
/******************************************************************************/
void WorldManager::areaCache(Area &area, Bool inc_progress, File &file_area)
{
   if(area.state()==AREA_UNLOADED)
   {
      if(file_area.readTry(dataPath()+"Area/"+area.xz(), area_cipher))
      {
         Area::Data *data; _area_data(data, area).load(file_area); // first create
         area._data= data; // then set (this is important for multi-threading, we need to have fully valid 'Area::Data' object before setting pointer to it)
      }
     _area_cache.add(&area);
      area._state=AREA_CACHE;

      if(inc_progress && _areas_to_load)_update_progress=Sat(_update_progress+1.0f/_areas_to_load);
   }
}
/******************************************************************************/
void WorldManager::areaLoad(Area &area, Bool active, File &file_area)
{
   if(area.state()==AREA_UNLOADED || area.state()==AREA_CACHE)
   {
#if DEBUG
   Dbl t=Time.curTime();
#endif

      // load area data
      if(area.state()==AREA_UNLOADED)
      {
         if(file_area.readTry(dataPath()+"Area/"+area.xz(), area_cipher))_area_data(area._data, area).load(file_area);
      }

#if DEBUG
  _time_area_update_state_load_data+=Time.curTime()-t;
#endif

      // objects
      {
         // original world data
         if(area._data)
         {
            Memc<Area::Data::AreaObj> &objs=area._data->objs;
            REPA(objs)
            {
               Area::Data::AreaObj &obj=objs[i];
               switch(obj.access())
               {
                  case OBJ_ACCESS_CUSTOM:
                  {
                     if(!area._visited || obj.constant()) // non-constant objects should be created only the first time we're entering an area, since then they are stored in savegame
                        if(!loadObj(area, active, obj, obj.constant())){} // don't break since objects are stored in container and can proceed
                  }break;
               }
            }
         }

         // temporary/save data
         {
            File &f=area._saved_obj; f.pos(0);
            ChunkReader cr; if(cr.read(f))for(; File *obj_file=cr(); )
            {
               switch(cr.ver())
               {
                  case 0:                                      loadObj(area, active, ObjType.find(cr.name()), *obj_file);  break;
                  case 1: {UID id; if(DecodeRaw(cr.name(), id))loadObj(area, active, ObjType.find(id       ), *obj_file);} break;
               }
            }
            f.reset(); // zero data
         }
      }

      // finalize
      if(area.state()==AREA_CACHE)_area_cache   .exclude(&area);
      if(active                  )_area_active  .add    (&area);
      else                        _area_inactive.add    (&area);
      area._state=(active ? AREA_ACTIVE : AREA_INACTIVE);

      if(Area::Data *data=area._data)
      {
         // create actors for terrain and objects
         if(Physics.created())
         {
            data->actor.create(data->phys, 0).group(AG_TERRAIN);
            FREPA(data->terrain_objs)
            {
               Area::Data::TerrainObj &obj=data->terrain_objs[i];
               if(obj.obj->phys())
               {
                  obj.actor.create(*obj.obj->phys(), 0, obj.matrix.scale()).matrix(Matrix(obj.matrix).normalize()).group(AG_TERRAIN);
               }
            }
            FREPA(data->foliage_objs)
            {
               Area::Data::GrassObj &obj=data->foliage_objs[i];
               if(obj.phys)FREPA(obj.instances)
               {
                  Area::Data::GrassObj::Instance &instance=obj.instances[i];
                  instance.actor.create(*obj.phys, 0, instance.matrix.scale()).matrix(Matrix(instance.matrix).normalize()).group(AG_TERRAIN);
               }
            }
         }
        _path.set(data->_path_mesh, area.xz());

                   data->customLoadFromCache();
         if(active)data->customActivate     ();
         else      data->customDeactivate   ();
      }

      area._visited=true;
      if(_areas_to_load)_update_progress=Sat(_update_progress+1.0f/_areas_to_load);
      Time.skipUpdate();
      UpdateThreads();

#if DEBUG
  _time_area_update_state_load+=Time.curTime()-t;
#endif
   }
}
/******************************************************************************/
// REFERENCES
/******************************************************************************/
void WorldManager::linkReferences()
{
   // world object link
   REPA(_area_active  ){Memc<Obj*> &objs=_area_active  [i]->_objs; REPAO(objs)->linkReferences();}
   REPA(_area_inactive){Memc<Obj*> &objs=_area_inactive[i]->_objs; REPAO(objs)->linkReferences();}

   // custom link
   if(link_references)link_references();

   // clear
  _obj_newly_added .clear();
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
void WorldManager::areaActivate(Area &area)
{
   if(area.state()==AREA_INACTIVE)
   {
#if DEBUG
   Dbl t=Time.curTime();
#endif

      REPA(area._objs)area._objs[i]->enable();

     _area_inactive.exclude(&area); // remove
     _area_active  .add    (&area); // put
      area._state=AREA_ACTIVE;

      if(area._data)area._data->customActivate();

#if DEBUG
  _time_area_update_state_activate+=Time.curTime()-t;
#endif
   }
}
void WorldManager::areaDeactivate(Area &area)
{
   if(area.state()==AREA_ACTIVE)
   {
#if DEBUG
   Dbl t=Time.curTime();
#endif

      REPA(area._objs)area._objs[i]->disable();

     _area_active  .exclude(&area); // remove
     _area_inactive.add    (&area); // put
      area._state=AREA_INACTIVE;

      if(area._data)area._data->customDeactivate();

#if DEBUG
  _time_area_update_state_deactivate+=Time.curTime()-t;
#endif
   }
}
/******************************************************************************/
static Bool NeedLoad(AREA_STATE src, AREA_STATE dest) // warning: must be in sync with function below
{
   switch(dest)
   {
      case AREA_ACTIVE  :
      case AREA_INACTIVE:
         switch(src)
         {
            case AREA_CACHE   :
            case AREA_UNLOADED: return true;
         }
      break;

      case AREA_LOAD:
      {
         switch(src)
         {
            case AREA_UNLOADED: return true;
         }
      }break;
   }
   return false;
}
static Bool NeedUnload(AREA_STATE src, AREA_STATE dest) // warning: must be in sync with function below
{
   switch(dest)
   {
      case AREA_CACHE   :
      case AREA_UNLOADED:
         switch(src)
         {
            case AREA_ACTIVE  :
            case AREA_INACTIVE: return true;
         }
      break;
   }
   return false;
}
void WorldManager::areaUpdateState(Area &area, AREA_STATE state, File &file_area) // warning: must be in sync with function above
{
   if(!_update_break)switch(state)
   {
      case AREA_ACTIVE:
         switch(area.state())
         {
            case AREA_INACTIVE: areaActivate(area                 ); break;
            case AREA_CACHE   :
            case AREA_UNLOADED: areaLoad    (area, true, file_area); break;
         }
      break;

      case AREA_INACTIVE:
         switch(area.state())
         {
            case AREA_ACTIVE  : areaDeactivate(area                  ); break;
            case AREA_CACHE   :
            case AREA_UNLOADED: areaLoad      (area, false, file_area); break;
         }
      break;

      case AREA_CACHE:
         switch(area.state())
         {
            case AREA_ACTIVE  :
            case AREA_INACTIVE: areaUnloadToCache(area); break;
         }
      break;

      case AREA_UNLOADED:
         switch(area.state())
         {
            case AREA_ACTIVE  :
            case AREA_INACTIVE:
            case AREA_CACHE   : areaUnload(area); break;
         }
      break;

      case AREA_LOAD:
      {
         switch(area.state())
         {
            case AREA_UNLOADED: areaCache(area, true, file_area); break;
         }
      }break;
   }
}
void WorldManager::areaUpdateState(Area &area, File &file_area)
{
   areaUpdateState(area, rangeState(area.xz()), file_area);
}
static void UpdateState  (Cell<Area> &cell, File     &file_area) {cell().world()->areaUpdateState(cell(), file_area);}
static void GetBackground(Cell<Area> &cell, WorldManager &world)
{
   if(cell().state()==AREA_UNLOADED && world.rangeState(cell.xy())==AREA_CACHE) // not loaded and to cache
      world._area_background.add(&cell()); // add to list of loaded in the background
}
static void TestState(Cell<Area> &cell, WorldManager &world) {world._areas_to_load+=NeedLoad(cell().state(), world.rangeState(cell.xy()));}
/******************************************************************************/
struct RectSetter
{
   Bool   active_set ,  loaded_set;
   RectI *active_rect, *loaded_rect;

   void includeActive(C VecI2 &xy) {if(active_set)active_rect->include(xy);else{active_set=true; *active_rect=xy;}}
   void includeLoaded(C VecI2 &xy) {if(loaded_set)loaded_rect->include(xy);else{loaded_set=true; *loaded_rect=xy;}}

   RectSetter(RectI &active_rect, RectI &loaded_rect) {active_set=loaded_set=false; T.active_rect=&active_rect.zero(); T.loaded_rect=&loaded_rect.zero();}

   static void Func(Cell<Area> &cell, RectSetter &rs) {if(cell().state()==AREA_ACTIVE)rs.includeActive(cell.xy()); if(cell().state()!=AREA_UNLOADED)rs.includeLoaded(cell.xy());}
};
void WorldManager::areaSetLoadedRect()
{
   // set area active/loaded rect
   switch(_mode)
   {
      case WORLD_STREAM:
      {
        _area_active_rect=_xzi;
        _area_active_rect.extend(_rangei);
        _area_loaded_rect=_area_active_rect;
        _area_loaded_rect.extend(2); // +1=INACTIVE, +1=CACHE
      }break;

      case WORLD_FULL:
      {
        _grid.size(_area_loaded_rect); // detect according to loaded 'grid' objects
        _area_active_rect=_area_loaded_rect;
      }break;
      
      case WORLD_MANUAL:
      {
         RectSetter rs(_area_active_rect, _area_loaded_rect); _grid.func(RectSetter::Func, rs);
      }break;
   }

   // set fast access
   if(_area_loaded_rect.w()>=16384
   || _area_loaded_rect.h()>=16384)Exit(S+"Loaded 'Game.Area' coverage exceeds allowed memory limits!\nLoaded Area coordinates:\n"+_area_loaded_rect.asText()+"\nWidth: "+_area_loaded_rect.w()+", Height: "+_area_loaded_rect.h()+"\nTotal Areas: "+_area_loaded_rect.w()*_area_loaded_rect.h());
  _grid.fastAccess(&_area_loaded_rect);
}
void WorldManager::areaSetVisibility(Memc<Area*> &area_draw, Bool sort)
{
   if(is()) // if world exists
   {
      Memt<VecI2> areas; Frustum.getIntersectingAreas(areas, areaSize(), true, sort, true, &_area_active_rect);
      area_draw.clear(); FREPA(areas)if(Area *area=areaActive(areas[i]))area_draw.add(area); // add in same order
   }
}
void WorldManager::areaUpdateState()
{
   if(_mode!=WORLD_MANUAL)
   {
   #if DEBUG
      Dbl t=Time.curTime(),
          time_area_update_state    =t,
          time_area_update_state_now=t;

     _time_area_update_state_unload_cache=0;
     _time_area_update_state_unload      =0;
     _time_area_update_state_load        =0;
     _time_area_update_state_load_data   =0;
     _time_area_update_state_activate    =0;
     _time_area_update_state_deactivate  =0;
   #endif

      RectI rect(_xzi); rect.extend(_rangei+1); // +1=INACTIVE

      // pause background thread
   LOG("areaUpdateState lock");
     _lock.on();

      // make a quick test of which areas need to be loaded (used for 'updateProgress')
   LOG("areaUpdateState TestState");
     _areas_to_load=0;
      if(_mode==WORLD_FULL)_grid.func      (      TestState, T);
      else                 _grid.funcCreate(rect, TestState, T);

      // before making any changes to area states, make sure that objects are correctly assigned to them
      updateObjectAreas();

      // process data required for this moment instantly
   LOG("areaUpdateState process data required for this moment instantly");
      File file_area; // use one file object to make use of the performance improvement for pak load
      if(_mode==WORLD_FULL)_grid.func(UpdateState, file_area);else // process all areas for 'WORLD_FULL'
      {
         Bool delay_remove=(_areas_to_load && !low_memory_usage); // if want to load something and we don't want low memory usage
         if(  delay_remove)DelayRemoveInc();
         REPA(_area_active  )areaUpdateState(*_area_active  [i], file_area); // order is important
         REPA(_area_inactive)areaUpdateState(*_area_inactive[i], file_area); // order is important
         REPA(_area_cache   )areaUpdateState(*_area_cache   [i], file_area); // order is important
          _grid.funcCreate(rect, UpdateState,                    file_area);
         if(  delay_remove)DelayRemoveDec();
      }

   #if DEBUG
      t=Time.curTime();
         _time_area_update_state_now =t-time_area_update_state_now;
      Dbl time_area_update_state_back=t;
   #endif

      rect.extend(1); // +1=CACHE

      // set areas to be loaded in the background thread
   LOG("areaUpdateState set areas to be loaded in the background thread");
         _area_background.clear(); if(use_background_loading && _thread.created() && _mode==WORLD_STREAM)_grid.funcCreate(rect, GetBackground, T);
      if(_area_background.elms ())_thread_event.on(); // activate the thread

      // enable background thread
   LOG("areaUpdateState unlock");
     _lock.off();

   #if DEBUG
      t=Time.curTime();
         _time_area_update_state_back=t-time_area_update_state_back;
      Dbl time_area_update_state_path=t;
   #endif

      // set data after updating the areas
   LOG("areaUpdateState areaSetLoadedRect");
      areaSetLoadedRect(); // set loaded areas lookup table
   LOG("areaUpdateState pathBuild");
      path2DBuild      (); // build paths after setting active areas

      // delete unused mini map images
      if(_mode==WORLD_STREAM)
      {
         // at this line of code, 'rect' specifies range up to CACHE
         RectI leave(Floor(mini_map.areaToImage(Vec2(rect.min))),
                     Floor(mini_map.areaToImage(Vec2(rect.max))));
         mini_map.clear(&leave);
      }

   LOG("areaUpdateState end");
   #if DEBUG
      t=Time.curTime();
     _time_area_update_state_path=t-time_area_update_state_path;
     _time_area_update_state     =t-time_area_update_state;
   #endif
   }
}
static void AreaUnloadRemaining(Cell<Area> &cell, WorldManager &world)
{
   if(cell()._temp)cell()._temp=false;       // was     marked, so it was     processed manually, we can disable the mark flag now
   else            world.areaUnload(cell()); // was not marked,    it was not processed manually, we need to unload the area
}
void WorldManager::areaSetState(C MemPtr<AreaState> &area_states, Bool unload_remaining)
{
   if(_mode==WORLD_MANUAL)
   {
     _update_progress=0;

      // pause background thread
     _lock.on();

      // make a quick test of which areas need to be loaded (used for 'updateProgress')
         _areas_to_load  =0;
      Int areas_to_unload=0;
      REPA(area_states)
      {
       C AreaState &area_state= area_states[i];
         AREA_STATE  cur_state=_grid.get(area_state.xz).data()->state();
        _areas_to_load  +=NeedLoad  (cur_state, area_state.state);
         areas_to_unload+=NeedUnload(cur_state, area_state.state);
      }

      // before making any changes to area states, make sure that objects are correctly assigned to them
      Bool delay_remove=false;
      if(unload_remaining || areas_to_unload)
      {
         updateObjectAreas(); // if unloading at least one area then update area objects

         if(!low_memory_usage && _areas_to_load) // if don't want low memory usage and unloading and loading
         {
            delay_remove=true;
            DelayRemoveInc();
         }
      }

      // update area states
      File file_area; // use one file object to make use of the performance improvement for pak load
     _area_background.clear();
      FREPA(area_states) // process in given order
      {
       C AreaState  &area_state=  area_states[i];
         Area       &area      =*_grid.get(area_state.xz).data(); if(unload_remaining)area._temp=true; // if want to unload remaining, then mark this area as "manually processed"
         areaUpdateState(area, area_state.state, file_area);
         if(use_background_loading && _thread.created() && area_state.state==AREA_CACHE && area.state()==AREA_UNLOADED)_area_background.add(&area); // if after update is still unloaded, but wants to be cached then add to background list
      }

      // unload remaining
      if(unload_remaining)_grid.func(AreaUnloadRemaining, T);

      // set areas to be loaded in the background thread
      if(_area_background.elms())_thread_event.on(); // activate the thread

      // enable background thread
     _lock.off();

      // release resource pointer copies after loading all areas
      if(delay_remove)DelayRemoveDec();

      // set data after updating the areas
      areaSetLoadedRect(); // set loaded areas lookup table
      path2DBuild      (); // build paths after setting active areas
   }
}
void WorldManager::updateBreak()
{
  _update_break=true;
}
void WorldManager::updateObjectAreas()
{
   // update object area assignments (area which object is assigned to)
   REPA(_obj_container)if(ObjMap<Obj> *obj_map=_obj_container[i].map) // update areas of all objects (active and inactive, because objects can be accessed from both area states, and they can be repositioned manually)
      REPA(*obj_map) // go from end to start so remove can be used
   {
      Obj &obj=(*obj_map)[i];
      if( !obj.updateArea())obj_map->removeObj(&obj);
   }
}
#if !__GNUC__ // fails to compile on GCC
INLINE
#endif
void UpdateObject(WorldManager &world, Obj &obj)
{
   obj._update_count=world._update_count;

   if(Obj *parent=obj.reliesOn())
      if(  parent->_update_count!=world._update_count && parent->_area && parent->_area->state()==AREA_ACTIVE) // testing 'area' also guarantees that the object is existing
   {
      UID id =obj.id(); UpdateObject(world, *parent);
      if( id!=obj.id())return; // after parent update, this object could have been removed, that's why check if its 'id' hasn't been changed
   }

   if(!obj.update())
      if(ObjMap<Obj> *obj_map=obj.worldObjMap())obj_map->removeObj(&obj);
}
void WorldManager::updateObjects()
{
   Dbl time=Time.curTime();

   FREPA(_area_active) // FREPA #2 (rep/frep)
   {
      Memc<Obj*> &objs=_area_active[i]->_objs;
      REPA(objs) // order is important in case of removing
      {
         Obj *o=objs[i];
         if(  o->_update_count!=_update_count)UpdateObject(T, *o);

         if(i>objs.elms())i=objs.elms(); // in case if some 'Obj::update' would remove other objects and suddenly 'i' would be out of 'objs' container range
      }
   }

  _time_obj_update=Time.curTime()-time;
}
void WorldManager::update(C Vec2 &xz)
{
   if(is())
   {
     _update_break   =false;
     _update_progress=0;
     _update_count   ++;

      // area change
      switch(_mode)
      {
         case WORLD_STREAM:
         {
            if(!_updated || (xz-T._xz).abs().max()>areaSize())
            {
               VecI2 xzi=worldToArea(xz);
               if(!_updated || xzi!=T._xzi)
               {
                  T._xz     =xz  ;
                  T._xzi    =xzi ;
                  T._updated=true;
                  areaUpdateState();
               }
            }
         }break;

         case WORLD_FULL:
         {
            if(!_updated)
            {
              _updated=true;
               // create empty Cell<Area> objects for each files inside Paks and Std
               if(!FullPath(dataPath()))
               {
                                     if (C PaksFile *pf=Paks.find(           dataPath()+"Area")        )REP(pf->children_num  )_grid.get(TextVecI2(Paks.file(pf->children_offset+i).file->name)); //          Pak
                  if(DataPath().is())for(FileFind    ff          (DataPath()+dataPath()+"Area"); ff(); )if (ff.type==FSTD_FILE)_grid.get(TextVecI2(                                 ff   .name)); // DataPath+Std
               }                     for(FileFind    ff          (           dataPath()+"Area"); ff(); )if (ff.type==FSTD_FILE)_grid.get(TextVecI2(                                 ff   .name)); //          Std
               areaUpdateState();
            }
         }break;

         case WORLD_MANUAL:
         {
           _updated=true;
            // no action required
         }break;
      }

      // link references
      if(_obj_newly_added.elms())linkReferences();

      // update physics
      Bool update_objects_after_physics=T.update_objects_after_physics,
           background_simulation       =PHYSX;
      if(  background_simulation)
      {
         Physics.stopSimulation();
         // TODO: update cloth here
         if(physics_update)physics_update();
         Physics.startSimulation();
         update_objects_after_physics=true;
      }

      // update objects
      if(update_objects_after_physics==false)updateObjects();

      // start physics
      if(!background_simulation)Physics.startSimulation();

      // overlays
      REPA(_decals) // order is important
      {
         Decal2 &d=_decals[i];
            d.time-=Time.d();
         if(d.time<=0)_decals.remove(i, true);
      }
      REPA(_mesh_overlays) // order is important
      {
         MeshOverlay2 &mo=_mesh_overlays[i];
            mo.time-=Time.d();
         if(mo.time<=0)_mesh_overlays.remove(i, true);
      }

      // effects
      D.grassUpdate();

      // stop physics
      if(!background_simulation){Physics.stopSimulation(); if(physics_update)physics_update();}

      // update object areas, call after physics to immediately validate eventual new area position
      updateObjectAreas();

      // update objects
      if(update_objects_after_physics==true)updateObjects();

      // update path world
     _path.update(); // call this right away after updating objects, to make sure that we request rebuilds ASAP
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
static void AreaSave(Cell<Area> &cell, File &f)
{
   if(cell().loaded())cell().saveObjs(); // first save loaded areas to temp
      cell().save  (f, cell.xy());       // then  save the temp to file
}
Bool WorldManager::save(File &f)
{
   // timer
   Time.skipUpdate();

   // before saving, make sure that objects are correctly assigned to areas
   updateObjectAreas();

   // save
   f.cmpUIntV(     0); // version
   f.putStr  (name()); // world name
  _grid.func (AreaSave, f); // area data
   f<<VecI2(SIGN_BIT, SIGN_BIT); // end marker
   return f.ok();
}
Bool WorldManager::save(C Str &save_name, Bool (*save)(File &f), Cipher *cipher)
{
   File f; if(f.writeTry(save_name, cipher))
   {
      if(T.save(f))
      if(!save || save(f))
         if(f.flushOK())return true;

      f.del(); FDelFile(save_name);
   }
   return false;
}
/******************************************************************************/
Bool WorldManager::load(File &f)
{
   Str world_name;
   switch(f.decUIntV()) // version
   {
      case  0: f.getStr(world_name); break;
      default: goto error;
   }
   if(NewTry(world_name))
   {
      // area data
      for(;;)
      {
         VecI2 xz; f>>xz;
         if(xz.x==SIGN_BIT
         && xz.y==SIGN_BIT)break; // end marker

         if(!f.ok())goto error;

         Area &area=*_grid.get(xz).data();
         if(  !area.load(f))goto error;
      }

      // success
      if(f.ok())return true;
   }
error:;
   del(); return false;
}
Bool WorldManager::load(C Str &save_name, Bool (*load)(File &f), Cipher *cipher)
{
   File f; if(f.readTry(save_name, cipher))if(T.load(f))if(!load || load(f))return true;
   del(); return false;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
WorldManager::~WorldManager()
{
  _thread.stop(); // first mark the thread to be stopped so it can do this on its own
  _lock.on (); _area_background.clear(); // clear areas for load, so the thread won't process them
  _lock.off();
  _thread_event.on(); // turn on the event in case the thread is waiting for it
  _thread.del(); // delete the thread before other members

   del(); // delete everything
}
WorldManager::WorldManager()
{
   area_cipher=null;
   link_references=null;
   physics_update=null;

  _id.zero();
  _mode=WORLD_STREAM;
  _updated=_update_break=false;
  _update_count=0;
  _update_progress=0;
  _areas_to_load=0;
  _range=0;
  _rangei=_rangei2=_rangei2_inactive=_rangei2_cache=0;
  _xz .zero();
  _xzi.zero();

  _area_active_rect.set(0, 0, -1, -1);
  _area_loaded_rect.set(0, 0, -1, -1);
  _area_data=null; setAreaData<Area::Data>();
  _grid.user(this);

  _path_iteration=0;
  _path_find.create(MAX_PATH_RES*2, MAX_PATH_RES*2);

   activeRange(100);
}
/******************************************************************************/
}}
/******************************************************************************/
