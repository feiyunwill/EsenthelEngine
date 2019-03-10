/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Obj::~Obj()
{
   removeFromArea();
  _type=-1;
  _id.zero();
}
Obj::Obj()
{
  _type =-1;
  _area =null ;
  _const=false;
  _id.randomize();
   clearUpdate();
}
/******************************************************************************/
// OPERATIONS / GET / SET
/******************************************************************************/
Int Obj::type()
{
   if(_type<0)_type=World.objType(T);
   return _type;
}
void Obj::clearUpdate()
{
  _update_count=(World._update_count+128)&0xFF;
}
ObjMap<Obj>* Obj::worldObjMap()
{
   return World.objMap(_type);
}
void Obj::removeFromArea()
{
   if(_area)
   {
     _area->_objs.exclude(this);
     _area=null;
   }
}
void Obj::putToArea(Area &area)
{
   if(T._area!=&area)
   {
      removeFromArea();
      T._area=&area;
      area._objs.add(this);
   }
}
Bool Obj::updateArea()
{
   VecI2 pos=World.worldToArea(T.pos());
   if(!_area || _area->xz()!=pos)
   {
      AREA_STATE obj_state=(_area ? _area->state() : AREA_UNLOADED);
      return World.areaInsertObject(*World._grid.get(pos).data(), T, obj_state);
   }
   return true;
}
/******************************************************************************/
// ENABLE / DISABLE
/******************************************************************************/
void Obj::disable() {}
void Obj:: enable() {clearUpdate();}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Obj::save(File &f)
{
   f.putMulti(Byte(0), _id); // version
   return f.ok();
}
/******************************************************************************/
Bool Obj::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>_id;
         if(f.ok())return true;
      }break;
   }
  _id.zero(); return false;
}
/******************************************************************************/
}}
/******************************************************************************/
