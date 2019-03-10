/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_OBJ CC4('O','B','J',0)
/******************************************************************************/
Enum ObjType;
DEFINE_CACHE(Object, Objects, ObjectPtr, "Object");
/******************************************************************************/
// MANAGE
/******************************************************************************/
Object& Object::del()
{
   params  .del();
   sub_objs.del();
  _base=null;
  _mesh=null;
  _phys=null;
   zero(); return T;
}
void Object::zero()
{
  _flag  =0;
  _access=OBJ_ACCESS_TERRAIN;
  _path  =OBJ_PATH_CREATE;
  _align =(ALIGN_MAX<<2);
  _type  .zero();
  _mesh_variation_id=0;
   matrix.identity();
}
Object::Object() {zero();}
Object& Object::create(C Object &src)
{
   if(this!=&src)
   {
      del();

      matrix           =src. matrix;
     _type             =src._type;
     _flag             =src._flag;
     _access           =src._access;
     _path             =src._path;
     _align            =src._align;
     _base             =src._base;
     _mesh             =src._mesh;
     _mesh_variation_id=src._mesh_variation_id;
     _phys             =src._phys;
      FREPA(src.params  )params  .New()=       src.params  [i] ;
      FREPA(src.sub_objs)sub_objs.New().create(src.sub_objs[i]);
   }
   return T;
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
ALIGN_TYPE Object::alignX()C {return ALIGN_TYPE( _align    &0x3);}
ALIGN_TYPE Object::alignY()C {return ALIGN_TYPE((_align>>2)&0x3);}
ALIGN_TYPE Object::alignZ()C {return ALIGN_TYPE((_align>>4)&0x3);}

void Object::align(Bool custom, ALIGN_TYPE x, ALIGN_TYPE y, ALIGN_TYPE z)
{
   FlagSet(_flag, OBJ_OVR_ALIGN, custom); if(!custom)updateBaseSelf();else _align=(x|(y<<2)|(z<<4));
}
/******************************************************************************/
Matrix Object::matrixFinal()C
{
   Box box;

   // get box
   if(alignX()!=ALIGN_NONE || alignY()!=ALIGN_NONE || alignZ()!=ALIGN_NONE)
   {
      Bool found=false;
      if(_phys && _phys->is()) // don't use PhysBody.box (it's better to transform shapes first, then get their boxes)
      {
         Box             temp;
         Mems<PhysPart> &parts=_phys->parts;
         REPA(parts)
         {
            PhysPart &part=parts[i];
            if(part.type()==PHYS_SHAPE)
            {
               temp=part.shape*matrix;
               if(found)box|=temp;else{box=temp; found=true;}
            }else
            if(part.getBox(temp))
            {
               temp*=matrix;
               if(found)box|=temp;else{box=temp; found=true;}
            }
         }
      }
      if(!found)
      {
         if(_mesh && _mesh->is())box=_mesh->ext*matrix;
         else                    box=Ball(0.5f)*matrix;
      }
   }

   // set align
   Matrix O;
   switch(alignX())
   {
      default          : O.pos.x=matrix.pos.x                ; break;
      case ALIGN_MIN   : O.pos.x=matrix.pos.x*2-box.max.x    ; break;
      case ALIGN_CENTER: O.pos.x=matrix.pos.x*2-box.centerX(); break;
      case ALIGN_MAX   : O.pos.x=matrix.pos.x*2-box.min.x    ; break;
   }
   switch(alignY())
   {
      default          : O.pos.y=matrix.pos.y                ; break;
      case ALIGN_MIN   : O.pos.y=matrix.pos.y*2-box.max.y    ; break;
      case ALIGN_CENTER: O.pos.y=matrix.pos.y*2-box.centerY(); break;
      case ALIGN_MAX   : O.pos.y=matrix.pos.y*2-box.min.y    ; break;
   }
   switch(alignZ())
   {
      default          : O.pos.z=matrix.pos.z                ; break;
      case ALIGN_MIN   : O.pos.z=matrix.pos.z*2-box.max.z    ; break;
      case ALIGN_CENTER: O.pos.z=matrix.pos.z*2-box.centerZ(); break;
      case ALIGN_MAX   : O.pos.z=matrix.pos.z*2-box.min.z    ; break;
   }
   O.orn()=matrix.orn();
   return O;
}
Vec Object::centerFinal()C
{
   if(_mesh && _mesh->is())return _mesh->ext.pos*matrixFinal();

   Flt s=scale()*0.5f;
   Vec O=matrix.pos;
   switch(alignX())
   {
      case ALIGN_MIN: O.x-=s; break;
      case ALIGN_MAX: O.x+=s; break;
   }
   switch(alignY())
   {
      case ALIGN_MIN: O.y-=s; break;
      case ALIGN_MAX: O.y+=s; break;
   }
   switch(alignZ())
   {
      case ALIGN_MIN: O.z-=s; break;
      case ALIGN_MAX: O.z+=s; break;
   }
   return O;
}
/******************************************************************************/
ObjectPtr Object::firstStored()
{
   Memt<Object*, 1024> processed; for(Object *cur=this; cur && processed.include(cur); cur=cur->base()())if(Objects.contains(cur))return cur; // use 'processed' in case A is based on A
   return null;
}
Bool Object::hasBase(C UID &base_id)C
{
   if(base_id.valid())
   {
      Memt<C Object*, 1024> processed; for(C Object *cur=this; cur && processed.include(cur); cur=cur->base()())if(Objects.id(cur)==base_id)return true; // use 'processed' in case A is based on A
   }
   return false;
}
/******************************************************************************/
Object& Object::updateBaseSelf()
{
   if(!(_flag&OBJ_OVR_TYPE          ))_type             =(_base ? _base->_type              :            UIDZero);
   if(!(_flag&OBJ_OVR_MESH          ))_mesh             =(_base ? _base->_mesh              :          MeshPtr());
   if(!(_flag&OBJ_OVR_MESH_VARIATION))_mesh_variation_id=(_base ? _base->_mesh_variation_id :                  0);
   if(!(_flag&OBJ_OVR_PHYS          ))_phys             =(_base ? _base->_phys              :      PhysBodyPtr());
   if(!(_flag&OBJ_OVR_ALIGN         ))_align            =(_base ? _base->_align             :     (ALIGN_MAX<<2));
   if(!(_flag&OBJ_OVR_ACCESS        ))_access           =(_base ? _base->_access            : OBJ_ACCESS_TERRAIN);
   if(!(_flag&OBJ_OVR_PATH          ))_path             =(_base ? _base->_path              : OBJ_PATH_CREATE   );
   if(!(_flag&OBJ_OVR_CONST         ))FlagSet(_flag, OBJ_CONST, _base ? _base->constant() : false);
   if(!(_flag&OBJ_OVR_SCALE         ))
   {
      if(!_base)matrix.normalize();else
      {
         matrix.x.setLength(_base->matrix.x.length());
         matrix.y.setLength(_base->matrix.y.length());
         matrix.z.setLength(_base->matrix.z.length());
      }
   }
   return T;
}
Object& Object::updateBase()
{
   updateBaseSelf();
   REPAO(sub_objs).updateBase();
   return T;
}
void Object::base(C ObjectPtr &base)
{
   T._base=base;
   updateBaseSelf();
}
/******************************************************************************/
Flt  Object::scale             ()C {return matrix.x.length();}
Vec  Object::scale3            ()C {return matrix.scale();}
Bool Object::constant          ()C {return FlagTest(_flag, OBJ_CONST);}
Int  Object::meshVariationIndex()C {return _mesh_variation_id ? (_mesh ? _mesh->variationFind(_mesh_variation_id) : -1) : 0;}

void Object::type           (Bool custom, C UID         &type    ) {                                               FlagSet(_flag, OBJ_OVR_TYPE          , custom); if(!custom)updateBaseSelf();else T._type             =type  ; }
void Object::access         (Bool custom,   OBJ_ACCESS   access  ) {if(!custom || InRange(access, OBJ_ACCESS_NUM)){FlagSet(_flag, OBJ_OVR_ACCESS        , custom); if(!custom)updateBaseSelf();else T._access           =access;}}
void Object::path           (Bool custom,   OBJ_PATH     path    ) {if(!custom || InRange(path  , OBJ_PATH_NUM  )){FlagSet(_flag, OBJ_OVR_PATH          , custom); if(!custom)updateBaseSelf();else T._path             =path  ;}}
void Object::mesh           (Bool custom, C MeshPtr     &mesh    ) {                                               FlagSet(_flag, OBJ_OVR_MESH          , custom); if(!custom)updateBaseSelf();else T._mesh             =mesh  ; }
void Object::meshVariationID(Bool custom,   UInt         id      ) {                                               FlagSet(_flag, OBJ_OVR_MESH_VARIATION, custom); if(!custom)updateBaseSelf();else T._mesh_variation_id=id    ; }
void Object::phys           (Bool custom, C PhysBodyPtr &phys    ) {                                               FlagSet(_flag, OBJ_OVR_PHYS          , custom); if(!custom)updateBaseSelf();else T._phys             =phys  ; }
void Object::scale          (Bool custom, C Vec         &scale   ) {                                               FlagSet(_flag, OBJ_OVR_SCALE         , custom); if(!custom)updateBaseSelf();else matrix.normalize(scale);}
void Object::constant       (Bool custom,   Bool         on      ) {                                               FlagSet(_flag, OBJ_OVR_CONST         , custom); if(!custom)updateBaseSelf();else FlagSet(_flag, OBJ_CONST, on);}

Bool Object::customType         ()C {return FlagTest(_flag, OBJ_OVR_TYPE          );}
Bool Object::customAccess       ()C {return FlagTest(_flag, OBJ_OVR_ACCESS        );}
Bool Object::customPath         ()C {return FlagTest(_flag, OBJ_OVR_PATH          );}
Bool Object::customMesh         ()C {return FlagTest(_flag, OBJ_OVR_MESH          );}
Bool Object::customMeshVariation()C {return FlagTest(_flag, OBJ_OVR_MESH_VARIATION);}
Bool Object::customPhys         ()C {return FlagTest(_flag, OBJ_OVR_PHYS          );}
Bool Object::customConstant     ()C {return FlagTest(_flag, OBJ_OVR_CONST         );}
Bool Object::customScale        ()C {return FlagTest(_flag, OBJ_OVR_SCALE         );}
Bool Object::customAlign        ()C {return FlagTest(_flag, OBJ_OVR_ALIGN         );}

Bool Object::customMeshVariationAny()C
{
   Memt<C Object*, 1024> processed; for(C Object *cur=this; cur && processed.include(cur); cur=cur->base()())if(cur->customMeshVariation())return true; // use 'processed' in case A is based on A
   return false;
}
/******************************************************************************/
Param* Object::findParam(C Str &name)
{
#if 0
   REPA(params)if(Equal(params[i].name, name))return &params[i];
#else
   for(Int l=0, r=params.elms(); l<r; )
   {
      Int mid=UInt(l+r)/2,
          compare=Compare(name, params[mid].name);
      if(!compare  )return     &params[mid];
      if( compare<0)r=mid;
      else          l=mid+1;
   }
#endif
   return _base ? _base->findParam(name) : null;
}
Param& Object::getParam(C Str &name)
{
   Param  *param=findParam(name); if(!param)Exit(S+"Parameter \""+name+"\" not found in Object.");
   return *param;
}
/******************************************************************************/
Object& Object::sortParams()
{
   params.sort(Compare);
   return T;
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Object::saveData(File &f, CChar *path)C
{
   f.putMulti(Byte(7), _flag, matrix); // version
                                   f.putAsset(_base.id());
   if(_flag&OBJ_OVR_TYPE          )f<<_type;
   if(_flag&OBJ_OVR_MESH          )f.putAsset(_mesh.id());
   if(_flag&OBJ_OVR_MESH_VARIATION)f<<_mesh_variation_id;
   if(_flag&OBJ_OVR_PHYS          )f.putAsset(_phys.id());
   if(_flag&OBJ_OVR_ACCESS        )f<<_access;
   if(_flag&OBJ_OVR_PATH          )f<<_path;
   if(_flag&OBJ_OVR_ALIGN         )f<<_align;
   f.cmpUIntV(params  .elms()); FREPA(params  )if(!params  [i].save    (f, path))return false;
   f.cmpUIntV(sub_objs.elms()); FREPA(sub_objs)if(!sub_objs[i].saveData(f, path))return false;
   return f.ok();
}
/******************************************************************************/
static void SetOldAccess(Object &op, Byte b)
{
   switch(b)
   {
      default: op.access(true, OBJ_ACCESS_CUSTOM ); if( op.base() &&  op.base()->constant())op.constant(true, false); break; // old OBJ_ACCESS_DYNAMIC
      case  1: op.access(true, OBJ_ACCESS_CUSTOM ); if(!op.base() || !op.base()->constant())op.constant(true, true ); break; // old OBJ_ACCESS_CONST
      case  2: op.access(true, OBJ_ACCESS_TERRAIN); break; // old OBJ_ACCESS_EMBED
      case  3: op.access(true, OBJ_ACCESS_GRASS  ); break; // old OBJ_ACCESS_GRASS
      case  4: op.access(true, OBJ_ACCESS_OVERLAY); break; // old OBJ_ACCESS_OVERLAY
   }
}
static void AdjustOldAccess(Object &op, C Str &type_name)
{
   if(!op.base() && (type_name.is() && type_name!="OBJ_STATIC" || op.params.elms() || op.sub_objs.elms()))op.access(true, OBJ_ACCESS_CUSTOM);
}
Bool Object::loadData(File &f, CChar *path)
{
   del(); switch(f.decUIntV()) // version
   {
      case 7:
      {
         f.getMulti(_flag, matrix);
                                         _base    .require   (f.getAssetID(), path);
         if(_flag&OBJ_OVR_TYPE          )f>>_type;
         if(_flag&OBJ_OVR_MESH          )_mesh    .require   (f.getAssetID(), path);
         if(_flag&OBJ_OVR_MESH_VARIATION)f>>_mesh_variation_id;
         if(_flag&OBJ_OVR_PHYS          )_phys    .require   (f.getAssetID(), path);
         if(_flag&OBJ_OVR_ACCESS        )f>>_access;
         if(_flag&OBJ_OVR_PATH          )f>>_path;
         if(_flag&OBJ_OVR_ALIGN         )f>>_align;
         params  .setNum(f.decUIntV()); FREPA(params  )if(!params  [i].load    (f, path))goto error;
         sub_objs.setNum(f.decUIntV()); FREPA(sub_objs)if(!sub_objs[i].loadData(f, path))goto error;
         if(f.ok())
         {
            updateBaseSelf();
            return true;
         }
      }break;

      case 6:
      {
         f>>_flag>>matrix;
                                 _base    .require   (f._getStr (), path);
         if(_flag&OBJ_OVR_TYPE  ){f._getStr(); f>>_type;}
         if(_flag&OBJ_OVR_MESH  )_mesh    .require   (f._getStr (), path);
         if(_flag&        (1<<2)){f._getStr(); FlagDisable(_flag, 1<<2);} _mesh_variation_id=0; // old OBJ_OVR_MATERIAL
         if(_flag&OBJ_OVR_PHYS  )_phys    .require   (f._getStr (), path);
         if(_flag&OBJ_OVR_ACCESS)f>>_access;
         if(_flag&OBJ_OVR_PATH  )f>>_path;
         if(_flag&OBJ_OVR_ALIGN )f>>_align;
         params.setNum(f.getInt()); FREPA(params    )if(!params     [i].load    (f, path))goto error;
                                     REP (f.getInt())if(!sub_objs.New().loadData(f, path))goto error;
         if(f.ok())
         {
            updateBaseSelf();
            return true;
         }
      }break;

      case 5:
      {
        _flag=f.getByte(); f>>matrix; Str type_name;
                                 _base    .require (f._getStr (          ), path);
         if(_flag&OBJ_OVR_TYPE  ){f._getStr(type_name); _type=ObjType.findID(type_name);}
         if(_flag&OBJ_OVR_MESH  )_mesh    .require (f._getStr (          ), path);
         if(_flag&        (1<<2)){f._getStr(); FlagDisable(_flag, 1<<2);} _mesh_variation_id=0; // old OBJ_OVR_MATERIAL
         if(_flag&OBJ_OVR_PHYS  )_phys    .require (f._getStr (          ), path);
         if(_flag&OBJ_OVR_ACCESS)SetOldAccess(T,    f. getByte(          )      );
         if(_flag&OBJ_OVR_PATH  )f>>_path;
         if(_flag&OBJ_OVR_ALIGN )f>>_align;
         params.setNum(f.getInt()); FREPA(params    )if(!params     [i].load    (f, path))goto error;
                                     REP (f.getInt())if(!sub_objs.New().loadData(f, path))goto error;
         if(f.ok())
         {
            updateBaseSelf(); if(!(_flag&OBJ_OVR_ACCESS))AdjustOldAccess(T, type_name);
            return true;
         }
      }break;

      case 4:
      {
        _flag=f.getByte(); f>>matrix; Str type_name, skeleton;
                                 _base    .require(f._getStr (          ), path);
         if(_flag&OBJ_OVR_TYPE  ){f._getStr(type_name); _type=ObjType.findID(type_name);}
         if(_flag&OBJ_OVR_MESH  )_mesh    .require(f._getStr (          ), path);
         if(_flag&        (1<<2)){f._getStr(); FlagDisable(_flag, 1<<2);} _mesh_variation_id=0; // old OBJ_OVR_MATERIAL
         if(_flag&        (1<<7)){skeleton=Skeletons.name(Skeletons(f._getStr(), path)); FlagDisable(_flag, 1<<7);} // old OBJ_OVR_SKELETON
         if(_flag&OBJ_OVR_PHYS  )_phys    .require(f._getStr (          ), path);
         if(_flag&OBJ_OVR_ACCESS)SetOldAccess(T,   f. getByte(          )      );
         if(_flag&OBJ_OVR_ALIGN )f>>_align;
         params.setNum(f.getInt()); FREPA(params    )if(!params     [i].load    (f, path))goto error; if(_flag&(1<<7)){Param &param=params.New(); param.type=PARAM_STR; param.name="skeleton"; param.value.s=skeleton; sortParams(); _flag^=(1<<7);}
                                     REP (f.getInt())if(!sub_objs.New().loadData(f, path))goto error;
         if(f.ok())
         {
            updateBaseSelf(); if(!(_flag&OBJ_OVR_ACCESS))AdjustOldAccess(T, type_name);
            return true;
         }
      }break;

      case 3:
      {
        _flag=f.getByte(); f>>matrix; Str type_name, skeleton;
                                 _base    .require(f._getStr16(), path);
         if(_flag&OBJ_OVR_TYPE  ){type_name=       f._getStr16()       ; _type=ObjType.findID(type_name);}
         if(_flag&OBJ_OVR_MESH  )_mesh    .require(f._getStr16(), path);
         if(_flag&        (1<<2)){f._getStr16(); FlagDisable(_flag, 1<<2);} _mesh_variation_id=0; // old OBJ_OVR_MATERIAL
         if(_flag&        (1<<7)){skeleton=Skeletons.name(Skeletons(f._getStr16(), path)); FlagDisable(_flag, 1<<7);} // old OBJ_OVR_SKELETON
         if(_flag&OBJ_OVR_PHYS  )_phys    .require(f._getStr16(), path);
         if(_flag&OBJ_OVR_ACCESS)SetOldAccess(T,   f. getByte ()      );
         if(_flag&OBJ_OVR_ALIGN )f>>_align;
         params.setNum(f.getInt()); FREPA(params    )if(!params     [i].load    (f, path))goto error; if(_flag&(1<<7)){Param &param=params.New(); param.type=PARAM_STR; param.name="skeleton"; param.value.s=skeleton; sortParams(); _flag^=(1<<7);}
                                     REP (f.getInt())if(!sub_objs.New().loadData(f, path))goto error;
         if(f.ok())
         {
            updateBaseSelf(); if(!(_flag&OBJ_OVR_ACCESS))AdjustOldAccess(T, type_name);
            return true;
         }
      }break;

      case 2:
      {
        _flag=f.getByte(); f>>matrix; Str type_name;
                                 _base    .require(f._getStr8(), path);
         if(_flag&OBJ_OVR_TYPE  ){type_name=       f._getStr8()       ; _type=ObjType.findID(type_name);}
         if(_flag&OBJ_OVR_MESH  )_mesh    .require(f._getStr8(), path);
         if(_flag&        (1<<2)){f._getStr8(); FlagDisable(_flag, 1<<2);} _mesh_variation_id=0; // old OBJ_OVR_MATERIAL
         if(_flag&OBJ_OVR_PHYS  )_phys    .require(f._getStr8(), path);
         if(_flag&OBJ_OVR_ACCESS)SetOldAccess(T,   f. getByte()      );
         if(_flag&OBJ_OVR_ALIGN )f>>_align;
         params.setNum(f.getInt()); FREPA(params    )if(!params     [i].load    (f, path))goto error;
                                     REP (f.getInt())if(!sub_objs.New().loadData(f, path))goto error;
         if(f.ok())
         {
            updateBaseSelf(); if(!(_flag&OBJ_OVR_ACCESS))AdjustOldAccess(T, type_name);
            return true;
         }
      }break;

      case 1:
      {
         f.getByte(); Str type_name;

        _flag=f.getByte(); f>>matrix; if(!matrix.x.y && !matrix.x.z && !matrix.y.y && !matrix.y.z && !matrix.z.y && !matrix.z.z)matrix.orn().setScale(matrix.x.length()); // adjust matrix scale because of bug in the past, matrix was set to X(scale,0,0), Y(scale,0,0), Z(scale,0,0)
                                _base    .require(f._getStr8(), path);
         if(_flag&OBJ_OVR_TYPE ){type_name=       f._getStr8()       ; _type=ObjType.findID(type_name);}
         if(_flag&OBJ_OVR_MESH )_mesh    .require(f._getStr8(), path);
         if(_flag&       (1<<2)){f._getStr8(); FlagDisable(_flag, 1<<2);} _mesh_variation_id=0; // old OBJ_OVR_MATERIAL
         if(_flag&OBJ_OVR_PHYS )_phys    .require(f._getStr8(), path);
         if(_flag&OBJ_OVR_ALIGN)f>>_align;
         if(_flag&(1<<7)       ){SetOldAccess(T, 1); _flag^=(1<<7);}else SetOldAccess(T, 0); // old OBJ_CONST
         params.setNum(f.getInt()); FREPA(params    )if(!params     [i].load    (f, path))goto error;
                                     REP (f.getInt())if(!sub_objs.New().loadData(f, path))goto error;
         if(f.ok())
         {
            updateBaseSelf();
            return true;
         }
      }break;

      case 0:
      {
         f.getByte();

         Char8 name[256]; Str type_name;
         switch(f.getByte())
         {
            default: type_name="OBJ_NONE"  ; break;
            case 1 : type_name="OBJ_STATIC"; break;
            case 2 : type_name="OBJ_CHR"   ; break;
            case 3 : type_name="OBJ_ITEM"  ; break;
            case 4 : type_name="OBJ_DOOR"  ; break;
         }
        _type=ObjType.findID(type_name);
        _flag=f.getByte(); f>>matrix;
                                 f>>name; _base=name;
         if(_flag&OBJ_OVR_MESH ){f>>name; _mesh=name;}
         if(_flag&       (1<<2)){f>>name; FlagDisable(_flag, 1<<2);} _mesh_variation_id=0; // old OBJ_OVR_MATERIAL
         if(_flag&OBJ_OVR_PHYS ){f>>name; _phys=name;}
         if(_flag&OBJ_OVR_ALIGN) f>>_align;
         if(_flag&       (1<<7)){SetOldAccess(T, 1); _flag^=(1<<7);}else SetOldAccess(T, 0); // old OBJ_CONST
         REP(f.getInt())params  .New().loadOld (f);
         REP(f.getInt())sub_objs.New().loadData(f);
         if(f.ok())
         {
            updateBaseSelf();
            return true;
         }
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
Bool Object::save(File &f, CChar *path)C
{
   f.putUInt(CC4_OBJ);
   return saveData(f, path);
}
Bool Object::load(File &f, CChar *path)
{
   if(f.getUInt()==CC4_OBJ)return loadData(f, path);
   del(); return false;
}
/******************************************************************************/
Bool Object::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Object::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   del(); return false;
}
/******************************************************************************/
Bool Object::operator==(C Object &obj)C
{
   if(_type==obj._type && _flag==obj._flag && _align==obj._align && _access==obj._access && _path==obj._path && _base==obj._base && _mesh==obj._mesh && _mesh_variation_id==obj._mesh_variation_id && _phys==obj._phys && params.elms()==obj.params.elms() && sub_objs.elms()==obj.sub_objs.elms())
   {
      REPA(params  )if(Compare(params  [i], obj.params  [i]))return false;
      REPA(sub_objs)if(        sub_objs[i]!=obj.sub_objs[i] )return false;
      return true;
   }
   return false;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
void ShutObj() {Objects.del();}
/******************************************************************************/
}
/******************************************************************************/
