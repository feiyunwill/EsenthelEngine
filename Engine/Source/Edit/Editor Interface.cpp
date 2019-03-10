/******************************************************************************/
#include "stdafx.h"
namespace EE{
static Int Compare(C Edit::Elm &elm, C UID &id) {return Compare(elm.id, id);}
namespace Edit{
/******************************************************************************/
#define EI_VER 34 // this needs to be increased every time a new command is added, existing one is changed, or some of engine class file formats get updated
#define EI_STR "Esenthel Editor Network Interface"
#define CLIENT_WAIT_TIME         (   60*1000) //    60 seconds
#define CLIENT_WAIT_TIME_LONG    ( 6*60*1000) //  6*60 seconds, some operations may take a long time to complete
#define CLIENT_WAIT_TIME_PUBLISH (60*60*1000) // 60*60 seconds, publishing may take very long time (especially when creating PVRTC textures)

ASSERT( EI_NUM<=256); // because they're stored as bytes
ASSERT(ELM_NUM<=256); // because they're stored as bytes
/******************************************************************************/
static Str ElmFullData(C MemPtr<Elm> &elms, C UID &elm_id, Str &name, Bool &removed, Bool &publish) // 'elms' must be sorted by their ID
{
   name   .clear();
   removed=false;
   publish=true ;

   Memt<UID> processed;
   for( UID cur_id=elm_id; cur_id.valid(); )
   {
      if(      processed.binaryInclude(cur_id, Compare)) // not yet processed
      if(C Elm *elm=elms.binaryFind   (cur_id, Compare)) // was found in elements
      {
         name    =(name.is() ? elm->name+'\\'+name : elm->name);
         removed|=elm->removed;
         publish&=elm->publish;
         cur_id  =elm->parent_id;
         continue;
      }
      break;
   }
   return name;
}
/******************************************************************************/
// MATERIAL MAP
/******************************************************************************/
void MaterialMap::create(Int resolution)
{
  _m .createSoftTry(resolution, resolution, 1, IMAGE_R8G8B8A8);
  _i .createSoftTry(resolution, resolution, 1, IMAGE_R8G8B8A8);
  _ip.clear();
}
void MaterialMap::del()
{
  _m .del();
  _i .del();
  _ip.del();
}
Int MaterialMap::resolution()C {return _m.w();}
/******************************************************************************/
void MaterialMap::set(Int x, Int y, C UID &m0, C UID &m1, C UID &m2, C UID &m3, C VecB4 &blend)
{
  _m.color(x, y, Color(_ip.getIDIndex0(m0), _ip.getIDIndex0(m1), _ip.getIDIndex0(m2), _ip.getIDIndex0(m3)));
  _i.color(x, y, Color( blend.x           ,  blend.y           ,  blend.z           ,  blend.w           ));
}
void MaterialMap::set(Int x, Int y, C UID &m0, C UID &m1, C UID &m2, C UID &m3, C Vec4 &blend)
{
   Vec4 b=blend; if(Flt sum=b.sum())b/=sum; // normalize blends
  _m.color (x, y, Color(_ip.getIDIndex0(m0), _ip.getIDIndex0(m1), _ip.getIDIndex0(m2), _ip.getIDIndex0(m3)));
  _i.colorF(x, y, b);
}
void MaterialMap::get(Int x, Int y, UID &m0, UID &m1, UID &m2, UID &m3, VecB4 &blend)
{
   Color m=_m.color(x, y),
         i=_i.color(x, y);
   m0=(InRange(m.r, _ip) ? _ip[m.r] : UIDZero);
   m1=(InRange(m.g, _ip) ? _ip[m.g] : UIDZero);
   m2=(InRange(m.b, _ip) ? _ip[m.b] : UIDZero);
   m3=(InRange(m.a, _ip) ? _ip[m.a] : UIDZero);
   blend.set(i.r, i.g, i.b, i.a);
}
void MaterialMap::get(Int x, Int y, UID &m0, UID &m1, UID &m2, UID &m3, Vec4 &blend)
{
   Color m=_m.color (x, y);
   Vec4  i=_i.colorF(x, y);
   m0=(InRange(m.r, _ip) ? _ip[m.r] : UIDZero);
   m1=(InRange(m.g, _ip) ? _ip[m.g] : UIDZero);
   m2=(InRange(m.b, _ip) ? _ip[m.b] : UIDZero);
   m3=(InRange(m.a, _ip) ? _ip[m.a] : UIDZero);
   blend.set(i.x, i.y, i.z, i.w);
}
/******************************************************************************/
void MaterialMap::resize(Int resolution)
{
   MAX(resolution, 0);
   if (resolution!=T.resolution())
   {
     _m.resize(resolution, resolution, FILTER_NONE, true, false, true);
     _i.resize(resolution, resolution, FILTER_NONE, true, false, true);
   }
}
/******************************************************************************/
Bool MaterialMap::save(File &f)C
{
   f.cmpUIntV(0); // version
   if(_m .save(f))
   if(_i .save(f))
   if(_ip.save(f))
      return f.ok();
   return false;
}
Bool MaterialMap::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         if(_m .load(f))
         if(_i .load(f))
         if(_ip.load(f))
            if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// FILE PARAMS
/******************************************************************************/
TextParam& FileParams:: getParam(C Str &name) {if(TextParam *par=findParam(name))return *par; return params.New().setName(name);}
TextParam* FileParams::findParam(C Str &name)
{
   Int i=0;
   if(InRange(i, params))FREPAD(j, params) // process in order
   {
      XmlParam &param=params[j]; if(param.name==name)
      {
         if(i==0)return &param; i--;
      }
   }
   return null;
}
/******************************************************************************/
Str FileParams::encode()C
{
   Str s=name; FREPA(params)
   {
    C TextParam &param=params[i]; s+='?'; s+=param.name; if(param.value.is()){s+='='; s+=param.value;}
   }
   return s;
}
void FileParams::decode(C Str &s)
{
   Memc<Str> strs=Split(s, '?');
   name=(strs.elms() ? strs[0] : S);
   params.setNum(strs.elms()-1); FREPA(params)
   {
      TextParam &param=params[i];
      Str       &str  =strs[i+1]; FREPA(str)if(str()[i]=='=') // find first '=' occurrence
      {
         param.value=str()+i+1;
         param.name =str.clip(i);
         goto param_set;
      }
      param.set(str);
   param_set:;
   }
}
/******************************************************************************/
Str FileParams::Encode(C MemPtr<FileParams> &file_params)
{
   Str s; FREPA(file_params)
   {
      if(i)s+='\n'; s+=file_params[i].encode();
   }
   return s;
}
Mems<FileParams> FileParams::Decode(C Str &str)
{
   Mems<FileParams> files; if(str.is())
   {
      Memc<Str> strs=Split(str, '\n'); // get list of all files
      files.setNum(strs.elms()); FREPAO(files).decode(strs[i]);
   }
   return files;
}
Str FileParams::Merge(C Str &a, C Str &b)
{
   return a.is() ? b.is() ? a+'\n'+b : a : b;
}
/******************************************************************************/
static Str  Encode(       C Mems  <FileParams> &file_params) {return      FileParams::Encode(ConstCast(file_params));}
static void Decode(File &f, MemPtr<FileParams>  file_params) {file_params=FileParams::Decode(f.getStr());}
/******************************************************************************/
// MATERIAL
/******************************************************************************/
Material& Material::reset()
{
   technique=MTECH_DEFAULT;
   cull=true;
   flip_normal_y=false;
   high_quality_ios=false;
   downsize_tex_mobile=0;
   color=1;
   ambient=0;
   specular=0;
   glow=0;
   roughness=0;
   bump=0;
   reflection=0;
   color_map.clear();  alpha_map.clear();
    bump_map.clear(); normal_map.clear();
   specular_map.clear();
   glow_map.clear();
   reflection_map.clear();
   detail_color.clear();
   detail_bump.clear();
   detail_normal.clear();
   macro_map.clear();
   light_map.clear();
   return T;
}
void Material::save(File &f)C
{
   f.cmpUIntV(0);
   f<<technique<<cull<<flip_normal_y<<high_quality_ios<<downsize_tex_mobile<<color<<ambient<<specular<<glow<<roughness<<bump<<reflection
    <<Encode(color_map)<<Encode(alpha_map)
    <<Encode( bump_map)<<Encode(normal_map)
    <<Encode(specular_map)
    <<Encode(glow_map)
    <<Encode(reflection_map)
    <<Encode(detail_color)
    <<Encode(detail_bump)
    <<Encode(detail_normal)
    <<Encode(macro_map)
    <<Encode(light_map);
}
Bool Material::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>technique>>cull>>flip_normal_y>>high_quality_ios>>downsize_tex_mobile>>color>>ambient>>specular>>glow>>roughness>>bump>>reflection;
         Decode(f, color_map); Decode(f, alpha_map);
         Decode(f,  bump_map); Decode(f, normal_map);
         Decode(f, specular_map);
         Decode(f, glow_map);
         Decode(f, reflection_map);
         Decode(f, detail_color);
         Decode(f, detail_bump);
         Decode(f, detail_normal);
         Decode(f, macro_map);
         Decode(f, light_map);
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
// OBJ DATA
/******************************************************************************/
ObjChange::ObjChange()
{
   cmd=EI_OBJ_NONE;
   type=PARAM_TYPE(0);
   id.zero();
}
void ObjChange:: removeParam(C UID &param_id                         ) {cmd=EI_OBJ_PARAM_REMOVE_ID   ; T.id  =param_id;}
void ObjChange:: removeParam(C Str &param_name, PARAM_TYPE param_type) {cmd=EI_OBJ_PARAM_REMOVE_NAME ; T.name=param_name; T.type=param_type;}
void ObjChange::restoreParam(C UID &param_id                         ) {cmd=EI_OBJ_PARAM_RESTORE_ID  ; T.id  =param_id;}
void ObjChange::restoreParam(C Str &param_name, PARAM_TYPE param_type) {cmd=EI_OBJ_PARAM_RESTORE_NAME; T.name=param_name; T.type=param_type;}

void ObjChange::renameParam(C UID &param_id      , C Str &param_new_name                       ) {cmd=EI_OBJ_PARAM_RENAME_ID  ; T.id  =param_id      ;                    T.param.name=param_new_name;}
void ObjChange::renameParam(C Str &param_old_name, C Str &param_new_name, PARAM_TYPE param_type) {cmd=EI_OBJ_PARAM_RENAME_NAME; T.name=param_old_name; T.type=param_type; T.param.name=param_new_name;}

void ObjChange::setParamTypeValue(C UID &param_id  , C Param &type_value                           ) {cmd=EI_OBJ_PARAM_SET_TYPE_VALUE_ID  ; T.id  =param_id  ;                        T.param=type_value;}
void ObjChange::setParamTypeValue(C Str &param_name, C Param &type_value, PARAM_TYPE param_old_type) {cmd=EI_OBJ_PARAM_SET_TYPE_VALUE_NAME; T.name=param_name; T.type=param_old_type; T.param=type_value;}

Bool ObjChange::save(File &f, CChar *path)C
{
   f<<cmd<<type<<id<<name; return param.save(f, path) && f.ok();
}
Bool ObjChange::load(File &f, CChar *path)
{
   f>>cmd>>type>>id>>name; if(param.load(f, path) && f.ok())return true;
   /*del();*/ return false;
}
/******************************************************************************/
ObjData& ObjData::reset()
{
   elm_obj_class_id.zero();
   access=OBJ_ACCESS_TERRAIN;
   path  =OBJ_PATH_CREATE;
   params.clear();
   return T;
}
ObjData::Param* ObjData::findParam(C Str &name, PARAM_TYPE type, Bool include_removed, Bool include_inherited)
{
   FREPA(params) // check in order, because Editor lists params from current object first (which we're the most interested in), then adds base/parent params later
   {
      Param &param=params[i];
      if( param.name==name // don't try to break when encountered a param with the same 'name', because there can be multiple params with the same name but different type/removed
      && (type==PARAM_NUM   || type==param.type)
      && (include_removed   || !param.  removed)
      && (include_inherited || !param.inherited)
      )return &param;
   }
   return null;
}
Bool ObjData::save(File &f)C {f<<elm_obj_class_id<<access<<path; return params.save(f) && f.ok();}
Bool ObjData::load(File &f)  {f>>elm_obj_class_id>>access>>path; return params.load(f) && f.ok();}
/******************************************************************************/
// CLIENT
/******************************************************************************/
Bool EditorInterface::   connected() {_conn.updateState(0); return _conn.state()==CONNECT_GREETED;} // try to receive data so we check if connection got disconnected
void EditorInterface::disconnect  ()
{
  _conn.del();
}
struct ConnectAttempt : Connection
{
   Bool sent, bad_ver;

   ConnectAttempt() {sent=bad_ver=false;}

   void create(Int port) {clientConnectToServer(SockAddr().setLocalFast(port));}
   Bool update()
   {
      if(updateState(0))
      {
         if(!sent)
         {
            sent=true;
            data.reset().putByte(EI_VERSION_CHECK).putStr(EI_STR).cmpUIntV(EI_VER).pos(0);
            if(!send(data))del();
         }
         if(receive(0))
         {
            if(      data.getByte()==EI_VERSION_CHECK)
            if(Equal(data.getStr (), EI_STR, true))
            {
               bad_ver=(data.decUIntV()!=EI_VER);
               if(!bad_ver)
               {
                  tcpNoDelay(true);
                  return true;
               }
            }
            del();
         }
      }
      return false;
   }
};
Bool EditorInterface::connect(Str &message, Int timeout)
{
   disconnect();
   if(timeout<0)timeout=1000; UInt start_time=Time.curTimeMs();
   ConnectAttempt c[128]; FREPAO(c).create(65535-i);
   for(;;)
   {
      Bool connecting=false;
      FREPA(c)if(c[i].update())
      {
         message.clear();
         Swap(_conn, SCAST(Connection, c[i]));
         return true;
      }else if(c[i].state()!=CONNECT_INVALID)connecting=true;
      if(!connecting)break;
      if((Time.curTimeMs()-start_time)>=timeout)break; // this code was tested OK for UInt overflow
      Time.wait(1);
   }
   REPA(c)if(c[i].bad_ver){message="This Application version is not compatible with the opened Esenthel Editor version.\nPlease upgrade your software."; return false;}
   message="Esenthel Editor does not appears to be opened.\nPlease open Esenthel Editor and enable option \"Allow Incoming Connections\"."; return false;
}
/******************************************************************************/
// PROJECTS
/******************************************************************************/
Str EditorInterface::projectsPath()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_PROJECTS_PATH).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_PROJECTS_PATH)return f.getStr();
      disconnect();
   }
   return S;
}
Bool EditorInterface::projectsPath(C Str &path)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_PROJECTS_PATH).putStr(path).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_PROJECTS_PATH)return f.getBool();
      disconnect();
   }
   return false;
}

Bool EditorInterface::getProjects(MemPtr<Project> projects)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_PROJECTS).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_PROJECTS)
      if(projects.load(f))return true;
      disconnect();
   }
   projects.clear(); return false;
}

UID EditorInterface::curProject()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_PROJECT).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_PROJECT)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::openProject(C UID &proj_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_PROJECT).putUID(proj_id).pos(0);
      if(_conn.send(f))return curProject()==proj_id;
      disconnect();
   }
   return false;
}
/******************************************************************************/
// SETTINGS
/******************************************************************************/
Str EditorInterface::dataPath()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_DATA_PATH).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_DATA_PATH)return f.getStr();
      disconnect();
   }
   return S;
}
/******************************************************************************/
// ELEMENTS
/******************************************************************************/
Bool EditorInterface::getElms(MemPtr<Elm> elms, Bool include_removed)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ELMS).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ELMS)
      {
         if(f.getBool()) // if success
         if(elms.load(f))
         {
            REPA(elms)
            {
               Elm &elm=elms[i];
               ElmFullData(elms, elm.id, elm.full_name, elm.final_removed, elm.final_publish); // setup full values for convenience
            }
            if(!include_removed) // call this at the end, once all elements have been setup, so we won't remove an element that still has unprocessed children, do this on the client, to avoid overloading the server
               REPA(elms)if(elms[i].final_removed)elms.remove(i, true); // need to keep order because elements are sorted by ID
            return true;
         }
         goto fail;
      }
      disconnect();
   }
fail:;
   elms.clear(); return false;
}
Bool EditorInterface::selectedElms(MemPtr<UID> elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ELMS_SELECTED).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ELMS_SELECTED)
      {
         if(f.getBool()) // if success
         if(elms.loadRaw(f))return true;
         goto fail;
      }
      disconnect();
   }
fail:;
   elms.clear(); return false;
}
Bool EditorInterface::selectElms(C MemPtr<UID> &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELMS_SELECTED); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELMS_SELECTED)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::reloadElms(C MemPtr<UID> &elms, Bool remember_result)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS)<<remember_result; elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::cancelReloadElms(C MemPtr<UID> &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS_CANCEL); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS_CANCEL)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::reloadResult(C MemPtr<UID> &elms, MemPtr< IDParam<RELOAD_RESULT> > results)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS_GET_RESULT); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS_GET_RESULT)
      {
         if(f.getBool())
         if(results.load(f))return true;
         goto fail;
      }
      disconnect();
   }
fail:
   results.clear(); return false;
}
Bool EditorInterface::forgetReloadResult(C MemPtr<UID> &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS_FORGET_RESULT); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS_FORGET_RESULT)return f.getBool();
      disconnect();
   }
   return false;
}
UID EditorInterface::newElm(ELM_TYPE type, C Str &name, C UID &parent_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_NEW_ELM).putByte(type).putStr(name).putUID(parent_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_NEW_ELM)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
UID EditorInterface::newWorld(C Str &name, Int area_size, Int terrain_res, C UID &parent_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_NEW_WORLD).putStr(name).putInt(area_size).putInt(terrain_res).putUID(parent_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_NEW_WORLD)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
/******************************************************************************/
Bool EditorInterface::setElmName(C MemPtr< IDParam<Str> > &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_NAME); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_NAME)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setElmRemoved(C MemPtr< IDParam<Bool> > &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_REMOVED); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_REMOVED)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setElmPublish(C MemPtr< IDParam<Bool> > &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_PUBLISH); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_PUBLISH)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setElmParent(C MemPtr< IDParam<UID> > &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_PARENT); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_PARENT)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setElmSrcFile(C MemPtr< IDParam<Str> > &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_SRC_FILE); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_SRC_FILE)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// WORLD
/******************************************************************************/
UID EditorInterface::curWorld()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::openWorld(C UID &world_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD)return f.getBool();
      disconnect();
   }
   return false;
}
Int EditorInterface::worldAreaSize(C UID &world_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_AREA_SIZE).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_AREA_SIZE)return f.getInt();
      disconnect();
   }
   return 0;
}
/******************************************************************************/
// WORLD TERRAIN
/******************************************************************************/
Int EditorInterface::worldTerrainRes(C UID &world_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_RES).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_RES)return f.getInt();
      disconnect();
   }
   return 0;
}
Bool EditorInterface::worldTerrainAreas(C UID &world_id, RectI &areas)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_AREAS).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_AREAS)
      {
         if(f.getBool()){f>>areas; return true;}
         goto fail;
      }
      disconnect();
   }
fail:;
   areas.set(0, 0, -1, -1); return !world_id.valid();
}
/******************************************************************************/
Bool EditorInterface::worldTerrainDel(C UID &world_id, C VecI2 &area_xy)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_DEL_WORLD_TERRAIN).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_DEL_WORLD_TERRAIN)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::worldTerrainExists(C UID &world_id, C VecI2 &area_xy)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_IS).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_IS)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGetHeight(C UID &world_id, C VecI2 &area_xy, Image &height)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_HEIGHT).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_HEIGHT)
      {
         if(f.getBool())return height.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   height.del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSetHeight(C UID &world_id, C VecI2 &area_xy, C Image &height, C UID &material_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN_HEIGHT).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putUID(material_id); height.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN_HEIGHT)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGetColor(C UID &world_id, C VecI2 &area_xy, Image &color)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_COLOR).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_COLOR)
      {
         if(f.getBool())return color.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   color.del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSetColor(C UID &world_id, C VecI2 &area_xy, C Image &color, C UID &material_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN_COLOR).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putUID(material_id); color.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN_COLOR)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGetMaterial(C UID &world_id, C VecI2 &area_xy, MaterialMap &material)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_MATERIAL).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_MATERIAL)
      {
         if(f.getBool())return material.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   material.del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSetMaterial(C UID &world_id, C VecI2 &area_xy, C MaterialMap &material)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN_MATERIAL).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y); material.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN_MATERIAL)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGet(C UID &world_id, C VecI2 &area_xy, Image &height, MaterialMap &material, Image *color)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putBool(color!=null).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN)
      {
         if(f.getBool())if(height.load(f))if(material.load(f))if(color ? color->load(f) : true)return true;
         goto fail;
      }
      disconnect();
   }
fail:;
   height.del(); material.del(); if(color)color->del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSet(C UID &world_id, C VecI2 &area_xy, C Image &height, C MaterialMap &material, C Image *color)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putBool(color!=null); height.save(f); material.save(f); if(color)color->save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// WORLD OBJECTS
/******************************************************************************/
Bool EditorInterface::worldObjCreate(C UID &world_id, C MemPtr<WorldObjParams> &objs)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_NEW_WORLD_OBJ).putUID(world_id); objs.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_NEW_WORLD_OBJ)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::worldObjGetDesc(C UID &world_id, MemPtr<WorldObjDesc> objs, C MemPtr<UID> &world_obj_instance_ids, C RectI *areas, Bool only_selected, Bool include_removed)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_OBJ_BASIC).putUID(world_id).putBool(areas!=null); if(areas)f<<*areas; f<<only_selected<<include_removed; world_obj_instance_ids.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_OBJ_BASIC)
      {
         if(f.getBool())return objs.load(f);
         goto fail;
      }
      disconnect();
   }
fail:
   objs.clear(); return !world_id.valid();
}
Bool EditorInterface::worldObjGetData(C UID &world_id, MemPtr<WorldObjData> objs, C MemPtr<UID> &world_obj_instance_ids, C RectI *areas, Bool only_selected, Bool include_removed, Bool include_removed_params)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_OBJ_FULL).putUID(world_id).putBool(areas!=null); if(areas)f<<*areas; f<<only_selected<<include_removed<<include_removed_params; world_obj_instance_ids.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(only_selected ? CLIENT_WAIT_TIME : CLIENT_WAIT_TIME_LONG)) // if world is big then it might take some time to get all objects, so wait for a long time
      if(f.getByte()==EI_GET_WORLD_OBJ_FULL)
      {
         if(f.getBool())return objs.load(f);
         goto fail;
      }
      disconnect();
   }
fail:
   objs.clear(); return !world_id.valid();
}
// !! when sending object data - HANDLE CORRECT REL PATH FOR OBJ PARAM ENUMS !!
/******************************************************************************/
// WORLD WAYPOINTS
/******************************************************************************/
Bool EditorInterface::worldWaypointGetList(C UID &world_id, MemPtr<UID> waypoint_ids)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_WAYPOINT_LIST).putUID(world_id); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_WAYPOINT_LIST)
      {
         if(f.getBool())return waypoint_ids.loadRaw(f);
         goto fail;
      }
      disconnect();
   }
fail:
   waypoint_ids.clear(); return !world_id.valid();
}
/******************************************************************************/
// WORLD CAMERA
/******************************************************************************/
Bool EditorInterface::worldCamGet(Camera &cam)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_CAM).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_CAM)return cam.load(f);
      disconnect();
   }
   return false;
}
Bool EditorInterface::worldCamSet(C Camera &cam)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_CAM); cam.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_CAM)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// WORLD DRAW
/******************************************************************************/
Bool EditorInterface::worldDrawLines(C MemPtr<Line> &lines)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_DRAW_WORLD_LINES); lines.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_DRAW_WORLD_LINES)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// IMAGE
/******************************************************************************/
Bool EditorInterface::getImage(C UID &elm_id, Image &image)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_IMAGE).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_IMAGE)
      {
         if(f.getBool())return image.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   image.del(); return !elm_id.valid();
}
Bool EditorInterface::setImage(C UID &elm_id, C Image &image)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_IMAGE).putUID(elm_id); image.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_IMAGE)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// CODE
/******************************************************************************/
Bool EditorInterface::getCode(C UID &elm_id, Str &code)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_CODE).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_CODE)
      {
         if(f.getBool())return code.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   code.clear(); return !elm_id.valid();
}
Bool EditorInterface::setCode(C UID &elm_id, C Str &code)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_CODE).putUID(elm_id).putStr(code).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_CODE)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::codeSyncImport()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_CODE_SYNC_IMPORT).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_CODE_SYNC_IMPORT)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::codeSyncExport()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_CODE_SYNC_EXPORT).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_CODE_SYNC_EXPORT)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// FILE
/******************************************************************************/
Bool EditorInterface::getFile(C UID &elm_id, File &data)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_FILE).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_FILE)
      {
         if(f.getBool())return f.copy(data);
         return false;
      }
      disconnect();
   }
   return !elm_id.valid();
}
Bool EditorInterface::setFile(C UID &elm_id, File &data)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_FILE).putUID(elm_id); data.copy(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_FILE)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// MESH
/******************************************************************************/
Bool EditorInterface::getMesh(C UID &elm_id, Mesh &mesh, Matrix *matrix)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_MESH).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_MESH)
      {
         if(f.getBool())
         {
            if(matrix)f>>*matrix;else f.skip(SIZE(*matrix));
            if(!f.left()) // object may exist, but its mesh may not be set yet, in that case OK will be true, but no data available
            {
               mesh.del();
               return true;
            }
            if(mesh.load(f))
            {
               mesh.setAutoTanBin().setRender();
               return true;
            }
         }
         return false;
      }
      disconnect();
   }
   mesh.del(); return !elm_id.valid();
}
Bool EditorInterface::setMesh(C UID &elm_id, C Mesh &mesh)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_MESH).putUID(elm_id); mesh.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_MESH)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// MATERIAL
/******************************************************************************/
UID EditorInterface::curMaterial()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_MTRL_CUR).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_MTRL_CUR)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::curMaterial(C UID &elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_MTRL_CUR).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_MTRL_CUR)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::getMaterial(C UID &elm_id, Material &mtrl)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_MTRL).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_MTRL)
      {
         if(f.getBool())return mtrl.load(f);
         return false;
      }
      disconnect();
   }
   mtrl.reset(); return !elm_id.valid();
}
Bool EditorInterface::setMaterial(C UID &elm_id, C Material &mtrl, Bool reload_textures, Bool adjust_params)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_MTRL).putUID(elm_id).putByte(reload_textures*1 | adjust_params*2); mtrl.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(reload_textures ? CLIENT_WAIT_TIME_LONG : CLIENT_WAIT_TIME)) // reloading textures may take a long time
      if(f.getByte()==EI_SET_MTRL)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::reloadMaterialTextures(C UID &elm_id, bool base, bool reflection, bool detail, bool macro, bool light)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_MTRL_TEX).putUID(elm_id).putByte(base*1 | reflection*2 | detail*4 | macro*8 | light*16); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME_LONG)) // reloading textures may take a long time
      if(f.getByte()==EI_RLD_MTRL_TEX)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::mulMaterialTextureByColor(C UID &elm_id)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_MUL_MTRL_TEX_COL).putUID(elm_id); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME_LONG)) // reloading textures may take a long time
      if(f.getByte()==EI_MUL_MTRL_TEX_COL)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// ANIMATION
/******************************************************************************/
UID EditorInterface::curAnimation()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ANIM_CUR).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ANIM_CUR)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::curAnimation(C UID &elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ANIM_CUR).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ANIM_CUR)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::getAnimation(C UID &elm_id, Animation &anim)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ANIM).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ANIM)
      {
         if(f.getBool())return anim.load(f);
         return false;
      }
      disconnect();
   }
   anim.del(); return !elm_id.valid();
}
Bool EditorInterface::setAnimation(C UID &elm_id, C Animation &anim)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ANIM).putUID(elm_id); anim.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ANIM)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// OBJECT
/******************************************************************************/
UID EditorInterface::curObject()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_OBJ_CUR).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_OBJ_CUR)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::curObject(C UID &elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_OBJ_CUR).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_OBJ_CUR)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::getObject(C UID &elm_id, ObjData &obj, Bool include_removed_params)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_OBJ).putUID(elm_id).putBool(include_removed_params).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_OBJ)
      {
         if(f.getBool())return obj.load(f);
         return false;
      }
      disconnect();
   }
   obj.reset(); return !elm_id.valid();
}
Bool EditorInterface::modifyObject(C UID &elm_id, C MemPtr<ObjChange> &changes)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_MODIFY_OBJ).putUID(elm_id); changes.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_MODIFY_OBJ)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// APPLICATION
/******************************************************************************/
Bool EditorInterface::activeApp(C UID &elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ACTIVE_APP).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ACTIVE_APP)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::exportApp(EXPORT_MODE mode, Bool data)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_EXPORT_APP).putByte(mode).putBool(data).pos(0);
      if(_conn.send(f))
      if(_conn.receive(data ? CLIENT_WAIT_TIME_PUBLISH : CLIENT_WAIT_TIME))
      if(f.getByte()==EI_EXPORT_APP)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// BUILD SETTINGS
/******************************************************************************/
Bool EditorInterface::buildDebug(Bool debug)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_DEBUG).putBool(debug).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_DEBUG)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::build32Bit(Bool bit32)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_32BIT).putBool(bit32).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_32BIT)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::buildDX9(Bool dx9)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_DX9).putBool(dx9).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_DX9)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::buildExe(EXE_TYPE type)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_EXE).putByte(type).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_EXE)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::buildPaths(Bool relative)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_PATHS).putBool(relative).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_PATHS)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// SERVER
/******************************************************************************/
Bool EditorServer::Client::update()
{
   if(!super::update())return false;

   if(!connection_version_ok) // if not verified
      if(connection.receive(0)) // check for it
   {
      if(!connection.address().thisDevice())return false; // allow only from this device

      File &f=connection.data;
      Byte cmd =f.getByte();
      if(  cmd==EI_VERSION_CHECK)
      if(Equal(f.getStr(), EI_STR, true))
      {
         connection_version_ok=(f.decUIntV()==EI_VER);
         f.reset().putByte(EI_VERSION_CHECK).putStr(EI_STR).cmpUIntV(EI_VER).pos(0);
         connection.tcpNoDelay(true);
         connection.send(f);
      }
      if(!connection_version_ok)return false; // if received something that wasn't correct version match then remove
   }
   return true;
}
/******************************************************************************/
static Int CompareElm(C Elm &elm, C UID &id) {return Compare(elm.id, id);}

Elm* FindElm(MemPtr<Elm> elms, C UID &elm_id               ) {return elms.binaryFind(elm_id, CompareElm);}
Elm* FindElm(MemPtr<Elm> elms, C UID &elm_id, ELM_TYPE type) {if(Elm *elm=FindElm(elms, elm_id))if(elm->type==type)return elm; return null;}

static Elm* FindChild(MemPtr<Elm> elms, C UID &parent_id, ELM_TYPE type)
{
   REPA(elms)
   {
      Elm &child=elms[i]; if(child.parent_id==parent_id && child.type==type)return &child;
   }
   return null;
}

Elm* FindElm(MemPtr<Elm> elms, C Str &full_name, ELM_TYPE type)
{
   REPA(elms)
   {
      Elm &elm=elms[i]; if(EqualPath(elm.full_name, full_name))
      {
         if(elm.type==type)return &elm;
         switch(elm.type)
         {
            case ELM_MESH: if(                  type==ELM_SKEL || type==ELM_PHYS)if(Elm *child=FindChild(elms, elm.id, type))return child; break; // allow      SKEL and PHYS to be children of MESH
            case ELM_OBJ : if(type==ELM_MESH || type==ELM_SKEL || type==ELM_PHYS)REPA(elms)                                                       // allow MESH SKEL and PHYS to be children of OBJ
            {
               Elm &child=elms[i]; if(child.parent_id==elm.id)
               {
                  if(child.type==type    )return &child;
                  if(child.type==ELM_MESH)if(type==ELM_SKEL || type==ELM_PHYS)if(Elm *sub=FindChild(elms, child.id, type))return sub;
               }
            }break;
         }
      }
   }
   return null;
}

Bool ContainsElm(MemPtr<Elm> elms, C UID &parent_id, C UID &child_id)
{
   if(parent_id.valid()) // if parent valid
   {
      Memt<UID> ids;
      for(C UID *id=&child_id; id->valid() && ids.binaryInclude(*id, Compare); ) // if valid and not checked yet
      {
         if(*id==parent_id)return true;
         if(Elm *elm=FindElm(elms, *id))id=&elm->parent_id;else break;
      }
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
