/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
// AREA OBJECT
/******************************************************************************/
Bool Area::Data::AreaObj::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<id;
   if(super::saveData(f, path))
      return f.ok();
   return false;
}
Bool Area::Data::AreaObj::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>id;
         if(super::loadData(f, path))
            if(f.ok())return true;
      }break;
   }
   return false;
}
Bool Area::Data::AreaObj::loadOld(File &f)
{
   id.randomize();
   if(super::loadData(f))
      if(f.ok())return true;
   return false;
}
/******************************************************************************/
// AREA PATH
/******************************************************************************/
void AreaPath2D::zero()
{
  _changed=false;
  _groups =0;
}
AreaPath2D::AreaPath2D() {zero();}
void AreaPath2D::del()
{
  _map.del();
  _neighbor.del();
   zero();
}
void AreaPath2D::create(Int size)
{
  _map.createSoftTry(size, size, 1, IMAGE_I8);
  _map.clear();
   group();
}
void AreaPath2D::createFromQuarter(AreaPath2D &src, Bool right, Bool forward, WorldSettings &settings)
{
   src._map.crop(_map, right ? src._map.w()/2 : 0, forward ? src._map.h()/2 : 0, src._map.w()/2, src._map.h()/2);
  _map.resize(settings.path2DRes(), settings.path2DRes(), FILTER_NONE);
   group();
}
void AreaPath2D::createFromQuad(AreaPath2D *lb, AreaPath2D *rb, AreaPath2D *lf, AreaPath2D *rf, WorldSettings &settings)
{
   create(settings.path2DRes());
   REPD(y, _map.h())
   REPD(x, _map.w())
   {
      AreaPath2D *src=null;
      Bool right  =(x>=(_map.w()/2)), // if right   half should be taken
           forward=(y>=(_map.h()/2)); // if forward half should be taken
      if(right)
      {
         if(forward)src=rf;else src=rb;
      }
      else // left
      {
         if(forward)src=lf;else src=lb;
      }
      if(src)
      {
         Int src_x=((right   ? x-_map.w()/2 : x)*src->_map.w())/(_map.w()/2),
             src_y=((forward ? y-_map.h()/2 : y)*src->_map.h())/(_map.h()/2);
         _map.pixel(x, y, src->_map.pixel(src_x, src_y));
      }
   }
   group();
}
/******************************************************************************/
Bool AreaPath2D::walkable(Int x, Int y)C
{
   return (InRange(x, _map.w()) && InRange(y, _map.h())) ? _map.pixB(x, y)!=0xFF : false;
}
AreaPath2D& AreaPath2D::walkable(Int x, Int y, Bool walkable)
{
   if(InRange(x, _map.w())
   && InRange(y, _map.h()))
   {
     _map.pixB(x, y)=(walkable ? 0xFE : 0xFF);
     _changed=true;
   }
   return T;
}
/******************************************************************************/
Bool AreaPath2D::fullyWalkable()
{
   REPD(y, _map.h())
   REPD(x, _map.w())if(!walkable(x, y))return false;
   return true;
}
/******************************************************************************/
void AreaPath2D::group()
{
   // set groups
   {
      #define BLOCKED 0xFFFF

      UShort path     [MAX_PATH_RES][MAX_PATH_RES], // [y][x]
             new_group[65536];
      Bool   parent   [65536];

      // set map
      Int groups=0;
      REPD(y, _map.h())
      REPD(x, _map.w())path[y][x]=(walkable(x, y) ? groups++ : BLOCKED);

      // set group remap
      REP(groups)new_group[i]=i;

      // join
      REP((Max(_map.w(), _map.h())>=48) ? 3 : 2) // steps
      {
         REP(groups)parent[i]=false;

         // link with neighbors
         FREPD(y, _map.h())
         FREPD(x, _map.w())
         {
            UInt g=path[y][x];
            if(  g!=BLOCKED && new_group[g]==g) // it's not blocked and it's a main group (it can be linked)
            {
               Bool right=false, left=false, up=false, down=false;
               Int  r    =x+1  , l   =x-1  , u =y+1  , d   =y-1  ;

               // try linking right
               if(InRange(r, _map.w()))
               {
                  UInt n =path[y][r]; // neighbor
                  if(  n!=BLOCKED)
                  {
                     right=true;
                     if(n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
                  }
               }

               // try linking up
               if(InRange(u, _map.h()))
               {
                  UInt n =path[u][x]; // neighbor
                  if(  n!=BLOCKED)
                  {
                     up=true;
                     if(n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
                  }
               }
               
            #if 0
               // try linking left
               if(InRange(l, _map.w()))
               {
                  UInt n =path[y][l]; // neighbor
                  if(  n!=BLOCKED)
                  {
                     left=true;
                     if(n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
                  }
               }

               // try linking down
               if(InRange(d, _map.h()))
               {
                  UInt n =path[d][x]; // neighbor
                  if(  n!=BLOCKED)
                  {
                     down=true;
                     if(n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
                  }
               }
            #endif
               
               // try linking diagonal
            #if 1
               if(right || up)if(InRange(r, _map.w()) && InRange(u, _map.h()))
               {
                  UInt n =path[u][r]; // neighbor
                  if(  n!=BLOCKED && n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
               }
            #if 0
               if(left || up)if(InRange(l, AREA_PATH_RES) && InRange(u, AREA_PATH_RES))
               {
                  UInt n =path[u][l]; // neighbor
                  if(  n!=BLOCKED && n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
               }
               if(right || down)if(InRange(r, AREA_PATH_RES) && InRange(d, AREA_PATH_RES))
               {
                  UInt n =path[d][r]; // neighbor
                  if(  n!=BLOCKED && n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
               }
               if(left || down)if(InRange(l, AREA_PATH_RES) && InRange(d, AREA_PATH_RES))
               {
                  UInt n =path[d][l]; // neighbor
                  if(  n!=BLOCKED && n!=g && new_group[n]==n && !parent[n]){new_group[n]=g; parent[g]=true;} // link
               }
            #endif
            #endif
            }
         }

         // update groups map
         REPD(y, _map.h())
         REPD(x, _map.w())
         {
            UInt g =path[y][x];
            if(  g!=BLOCKED)path[y][x]=new_group[g];
         }
      }

      // set target map
      T._groups=0;
      REP(groups)new_group[i]=BLOCKED; // set not yet added to target 'path' map
      REPD(y, _map.h())
      REPD(x, _map.w())
      {
         UInt g =path[y][x];
         if(  g!=BLOCKED)
         {
            UShort &n=new_group[g];
            if(     n==BLOCKED)n=T._groups++; // not yet added
           _map.pixB(x, y)=n; // set target map
         }
      }
   }

   // find neighbors
   VecI2 offset[]=
   {
      VecI2(-1, 1),
      VecI2( 0, 1),
      VecI2( 1, 1),
      VecI2(-1, 0),
   };

  _neighbor.clear();
   REPD(y, _map.h())
   REPD(x, _map.w())
   {
      Int a =_map.pixB(x, y);
      if( a!=0xFF)
      {
         REPA(offset)
         {
            Int sx=x+offset[i].x,
                sy=y+offset[i].y;
            if(InRange(sx, _map.w())
            && InRange(sy, _map.h()))
            {
               Int b =_map.pixB(sx,sy);
               if( b!=0xFF && a!=b)
               {
                  Byte cost=5;
                  if(offset[i].x && offset[i].y) // if diagonal
                  {
                     // if blocked diagonal
                     if(!walkable(sx,  y)
                     && !walkable( x, sy))continue; // don't add
                     cost=7; // increase the movement cost
                  }

                  // check if the neighbor entry already exists
                  Int min, max; MinMax(a, b, min, max);
                  REPA(_neighbor)
                  {
                     Neighbor &n=_neighbor[i];
                     if(n.a==min && n.b==max){MIN(n.cost, cost); goto exists;}
                  }
                 _neighbor.New().set(min, max, cost);
               exists:;
               }
            }
         }
      }
   }
  _changed=false;
}
/******************************************************************************/
void AreaPath2D::resize(Int size)
{
  _map.resize(size, size, FILTER_NONE);
   group();
}
/******************************************************************************/
Bool AreaPath2D::save(File &f)C
{
   f.cmpUIntV(1); // version
   f<<_groups;
   if(_map     . save   (f))
   if(_neighbor._saveRaw(f))
      return f.ok();
   return false;
}
Bool AreaPath2D::load(File &f)
{
  _changed=false;
   switch(f.decUIntV()) // version
   {
      case 1:
      {
         f>>_groups;
         if(_map     . load   (f))
         if(_neighbor._loadRaw(f))
            if(f.ok())return true;
      }break;

      case 0:
      {
         Byte path[32][32]; // [y][x]
         f>>path>>_groups;
         if(_neighbor._loadRaw(f))
         if(_map.createSoftTry(32, 32, 1, IMAGE_I8))
         {
            REPD(y, _map.h())
            REPD(x, _map.w())_map.pixB(x, y)=path[y][x];
            if(f.ok())return true;
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// AREA DATA
/******************************************************************************/
Area::Data::~Data()
{
   Delete(_path_mesh);
   Delete(_path2D   );
}
Area::Data::Data(Area &area)
{
   T._area     =&area;
   T._path_mesh=null;
   T._path2D   =null;
   T._path_node_offset=0;
}
Bool Area::Data::save(File &f)
{
   ChunkWriter cw(f);
   if(height.is() || material_map.is() || materials.elms())if(File *f=cw.beginChunk("Heightmap", 0))
   {
      if(!height      .save(*f))return false;
      if(!material_map.save(*f))return false;
      f->putInt(materials.elms()); REPA(materials)f->_putStr(materials[i].name());
   }
   if(mesh         .is  ())if(File *f=cw.beginChunk("Mesh"        , 0))if(!mesh.save(*f))return false;
   if(phys         .is  ())if(File *f=cw.beginChunk("Phys"        , 0))if(!phys.save(*f))return false;
   if(objs         .elms())if(File *f=cw.beginChunk("Object"      , 1))FREPA(objs)if(!objs[i].save(*f))return false;
   if(decals       .elms())if(File *f=cw.beginChunk("Decal"       , 0))if(!decals       ._save(*f))return false;
   if(mesh_overlays.elms())if(File *f=cw.beginChunk("Mesh Overlay", 0))if(!mesh_overlays._save(*f))return false;
   if(_path_mesh          )if(File *f=cw.beginChunk("PathMesh"    , 0))if(!_path_mesh  -> save(*f))return false;
   if(_path2D             )if(File *f=cw.beginChunk("AreaPath"    , 0))if(!_path2D     -> save(*f))return false;
   if(waters       .elms())if(File *f=cw.beginChunk("Water"       , 0))if(!waters       ._save(*f))return false;

   if(!customSave(cw))return false;
   cw.endChunkList();
   return f.ok();
}
Bool Area::Data::load(File &f)
{
   for(ChunkReader cr(f); File *f=cr();)
   {
      if(cr.name()=="HeightmapMesh")switch(cr.ver())
      {
         case 0: mesh.loadAdd(*f); break;
      }else
      if(cr.name()=="HeightmapPhys")switch(cr.ver())
      {
         case 0: phys.loadAdd(*f); break;
      }else
      if(cr.name()=="ObjectMesh")switch(cr.ver())
      {
         case 0: mesh.loadAdd(*f); break;
      }else
      if(cr.name()=="ObjectPhys")switch(cr.ver())
      {
         case 0: phys.loadAdd(*f); break;
      }else
      if(cr.name()=="Heightmap")switch(cr.ver())
      {
         case 0: if(!height.load(*f))break; if(!material_map.load(*f))break; materials.setNum(f->getInt()); REPAO(materials)=f->_getStr(); break;
      }else
      if(cr.name()=="Mesh")switch(cr.ver())
      {
         case 0: mesh.loadAdd(*f); break;
      }else
      if(cr.name()=="Phys")switch(cr.ver())
      {
         case 0: phys.loadAdd(*f); break;
      }else
      if(cr.name()=="Object")switch(cr.ver())
      {
         case 1: for(; !f->end(); )if(!objs.New().load   (*f)){objs.removeLast(); break;} break;
         case 0: for(; !f->end(); )if(!objs.New().loadOld(*f)){objs.removeLast(); break;} break;
      }else
      if(cr.name()=="Decal")switch(cr.ver())
      {
         case 0: decals._load(*f); break;
      }else
      if(cr.name()=="Mesh Overlay")switch(cr.ver())
      {
         case 0: mesh_overlays._load(*f); break;
      }else
      if(cr.name()=="PathMesh")switch(cr.ver())
      {
         case 0: New(Delete(_path_mesh))->load(*f); break;
      }else
      if(cr.name()=="AreaPath")switch(cr.ver())
      {
         case 0: New(Delete(_path2D))->load(*f); break;
      }else
      if(cr.name()=="Water")switch(cr.ver())
      {
         case 0: waters._load(*f); break;
      }else
      {
         customLoad(*f, cr.name(), cr.ver());
      }
   }

   // setup terrain/foliage objects (these containers should not be saved, because they are dynamically created from 'objs' which are already stored)
   terrain_objs_box.zero(); Bool have_terrain_box=false, have_foliage_box=false;
   foliage_objs_box.zero();
   terrain_objs.clear();
   foliage_objs.clear();
   FREPA(objs) // add in order
   {
    C AreaObj &obj=objs[i];
      switch(obj.access())
      {
         case OBJ_ACCESS_TERRAIN: if(obj.base())
         {
            TerrainObj &ter_obj=terrain_objs.New();
            ter_obj.obj           =obj.base();
            ter_obj.matrix        =obj.matrixFinal();
            ter_obj.mesh_variation=obj.meshVariationIndex();
            if(ter_obj.obj->mesh())
            {
               Box box=ter_obj.obj->mesh()->ext*ter_obj.matrix;
               if(have_terrain_box)terrain_objs_box|=box;
               else               {terrain_objs_box =box; have_terrain_box=true;}
            }
         }break;

         case OBJ_ACCESS_GRASS:
         {
            Int mesh_variation=obj.meshVariationIndex();
            GrassObj  *foliage=null; REPA(foliage_objs){GrassObj &fo=foliage_objs[i]; if(fo.mesh==obj.mesh() && fo.phys==obj.phys() && fo.mesh_variation==mesh_variation){foliage=&fo; break;}}
            if(!foliage) // create new one
            {
               foliage=&foliage_objs.New();
               foliage->shrink        =false;
               foliage->mesh          =obj.mesh();
               foliage->phys          =obj.phys();
               foliage->mesh_variation=mesh_variation;
            }
            GrassObj::Instance &instance=foliage->instances.New(); // set new instance
            instance.matrix=obj.matrixFinal();
            if(obj.mesh())
            {
               Box box=obj.mesh()->ext*instance.matrix;
               if(have_foliage_box)foliage_objs_box|=box;
               else               {foliage_objs_box =box; have_foliage_box=true;}
            }
         }break;
      }
   }

   if(f.ok())return true;
   /*del();*/ return false;
}
/******************************************************************************/
// AREA
/******************************************************************************/
Area::~Area()
{
   Delete(_data);
}
Area::Area(C VecI2 &xy, Ptr grid_user)
{
  T._visited=false;
  T._temp   =false;
  T._xz     =xy;
  T._state  =AREA_UNLOADED;
  T._data   =null;
  T._saved_obj.writeMem(1024);
  T._world  =(WorldManager*)grid_user;
}
/******************************************************************************/
Flt Area::hmHeight(C Vec2 &xz, Bool smooth)C
{
   if(_data)
   {
      Image &height=_data->height;
      if(    height.is())
      {
         Vec2 pixel=xz/world()->areaSize(); // in area coordinates space
              pixel.x-=T._xz.x; //
              pixel.y-=T._xz.y; // local fraction
              pixel.x*=height.w()-1;
              pixel.y*=height.h()-1; // image space
         return smooth ? height.pixelFLinear(         pixel.x ,          pixel.y, true)
                       : height.pixelF      (RoundPos(pixel.x), RoundPos(pixel.y)     );
      }
   }
   return 0;
}
/******************************************************************************/
C MaterialPtr& Area::hmMaterial(C Vec2 &xz)C
{
   if(_data)
   {
      Image &map=_data->material_map;
      if(    map.is())
      {
         Vec2 pixel=xz/world()->areaSize(); // in area coordinates space
              pixel.x-=T._xz.x; //
              pixel.y-=T._xz.y; // local fraction
              pixel.x*=map.w()-1;
              pixel.y*=map.h()-1; // image space

         UInt index=map.pixel(RoundPos(pixel.x), RoundPos(pixel.y)); // material index
         if(InRange(index, _data->materials))return _data->materials[index];
      }
   }
   return MaterialNull;
}
/******************************************************************************/
Area::Data* Area::getData()
{
   if(state()!=AREA_UNLOADED)if(!data())world()->_area_data(_data, T); // first check state, then data (in case the Area is being loaded in background thread)
   return data();
}
/******************************************************************************/
void Area::setShader()
{
   if(_data)
   {
     _data-> mesh.setShader();
     _data->customSetShader();
   }
}
/******************************************************************************/
Bool Area::saveObj(Obj &obj)
{
#if 1 // save using Obj Type ID (this allows to load objects correctly after obj type name has changed, as long as it has the same ID as before)
 C UID &obj_type_id=ObjType.elmID(world()->objType(obj));
   if(  obj_type_id.valid())
   {
      ChunkWriter cw; cw.appendChunkList(_saved_obj);
      if(File  *f=cw.beginChunk(EncodeRaw(obj_type_id), 1)) // OBJ_TYPE id
      {
         if(!obj.save(*f))return false; // object data
         cw.endChunk();
      }
   }
#else // save using Obj Type Name
   CChar8 *obj_type_name=ObjType.elmName(world()->objType(obj));
   if(Is(  obj_type_name))
   {
      ChunkWriter cw; cw.appendChunkList(_saved_obj);
      if(File  *f=cw.beginChunk(obj_type_name, 0)) // OBJ_TYPE name
      {
         if(!obj.save(*f))return false; // object data
         cw.endChunk();
      }
   }
#endif
   return _saved_obj.ok();
}
Bool Area::saveObjs()
{
  _saved_obj.reset();
   FREPA(_objs)
   {
      Obj &obj=*_objs[i];
      if( !obj.isConst() && obj.canBeSaved())if(!saveObj(obj))return false; // don't save constant objects, and save only those which want to be saved
   }
   return true;
}
Bool Area::save(File &f, C VecI2 &xy)
{
   if(_visited || _saved_obj.size()) // if visited or has any objects to save
   {
      f<<xy<<_visited;
      f.cmpUIntV(_saved_obj.size());
     _saved_obj.pos(0); if(!_saved_obj.copy(f))return false;
      return f.ok();
   }
   return true;
}
Bool Area::load(File &f)
{
   f>>_visited;
   Int size=f.decUIntV();
   return f.ok() && f.copy(_saved_obj, size);
}
/******************************************************************************/
}}
/******************************************************************************/
