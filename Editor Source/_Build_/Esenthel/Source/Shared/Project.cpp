/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
uint CC4_PRDT=CC4('P', 'R', 'D', 'T'); // Project Data
/******************************************************************************/

/******************************************************************************/
   Project::Project() : text_data(false), synchronize(true), compress_level(9), cipher(CIPHER_NONE), compress_type(COMPRESS_NONE), material_simplify(MS_NEVER), id(UIDZero), app_id(UIDZero), hm_mtrl_id(UIDZero), water_mtrl_id(UIDZero)
   {
      REPAO(cipher_key)=0;
      REPAO(mtrl_brush_id).zero();
         world_vers.mode(CACHE_DUMMY); // to allow creating new elements
      mini_map_vers.mode(CACHE_DUMMY); // to allow creating new elements
   }
   Project& Project::del()
   {
      // delete this first in case it uses this project members
      world_vers .del(); mini_map_vers .del();
      world_paths.del(); mini_map_paths.del();

      elms.del();
      texs.del(); texs_update.del();

      cipher=CIPHER_NONE; REPAO(cipher_key)=0;
      compress_type =COMPRESS_NONE;
      compress_level=9;
      material_simplify=MS_NEVER;
      cipher_time=cipher_key_time=compress_type_time=compress_level_time=material_simplify_time=0;
      text_data=false; synchronize=true; app_id.zero(); hm_mtrl_id.zero(); water_mtrl_id.zero(); REPAO(mtrl_brush_id).zero();

      path.del(); code_path.del(); code_base_path.del(); edit_path.del(); game_path.del(); temp_path.del(); tex_path.del(); temp_tex_path.del(); temp_tex_dynamic_path.del();

      id.zero(); name.del();
      return T;
   }
   int Project::CompareID(C Elm &elm, C UID &id) {return Compare(elm.id, id);}
   bool Project::valid()C {return id.valid();}
   bool Project::needUpdate()C {return texs_update.elms()>0;}
   bool  Project::hasElm(C UID &id)C {           return  id.valid() && elms.binaryHas   (id,        CompareID);}
   int  Project::findElmI(C UID &id)C {int index; return (id.valid() && elms.binarySearch(id, index, CompareID)) ? index : -1;}
   Elm* Project::findElm(C UID &id)  {return elms.addr(findElmI(id));}
 C Elm* Project::findElm(C UID &id)C {return elms.addr(findElmI(id));}
   Elm&  Project::getElm(C UID &id)  {int index; if(elms.binarySearch(id, index, CompareID))return elms[index]; Elm &elm=elms.NewAt(index); elm.id=id; return elm;}
   Elm&  Project::newElm(         ) // create new element with random id
   {
   #if 0 // simple version
      for(;;){Elm &elm=getElm(UID().randomizeValid()); if(!elm.type)return elm;}
   #else // version that tests that we don't overlap with any existing ID's using ELM_OFFSET_NUM range
      UID id, test; again: id.randomizeValid(); test=id-ELM_OFFSET_NUM;
      REP(ELM_OFFSET_NUM*2+1){if(!test.valid() || hasElm(test))goto again; test++;}
      return getElm(id);
   #endif
   }
   Elm& Project::newElm(C Str &name, C UID &parent_id, ELM_TYPE type) {Elm &elm=newElm().setName(name, NewElmTime).setParent(parent_id, NewElmTime); elm.type=type; return elm;}
   Elm* Project::findElmByPath(C Str &path)
   {
      if(path.is())
      {
         UID parent=UIDZero; Str p=path; for(;;)
         {
            Str  name=GetStart(p); p=GetStartNot(p);
            Elm *found_elm=null;
            REPA(elms)
            {
               Elm &elm=elms[i]; if(elm.parent_id==parent && elm.name==name && ElmVisible(elm.type)) // don't list hidden types
               {
                  found_elm=&elm;
                  if(!elm.removed())break; // stop looking if this element exists
               }
            }
            if(!p.is()   )return found_elm;
            if(!found_elm)break;
            parent=found_elm->id;
         }
      }
      return null;
   }
   Elm* Project::findElm(C Str &text)
   {
      UID id; if(id.fromText(text))return findElm(id);
      return findElmByPath(text);
   }
   UID  Project::findElmID(C Str &text) {if(Elm *elm=findElm(text))return elm->id; return UIDZero;}
   Elm* Project::findElm(C UID &id  , ELM_TYPE type) {if(  Elm *elm=findElm(id  ))if(elm->type==type)return elm   ; return null   ;}
 C Elm* Project::findElm(C UID &id  , ELM_TYPE type)C{if(C Elm *elm=findElm(id  ))if(elm->type==type)return elm   ; return null   ;}
   Elm* Project::findElm(C Str &text, ELM_TYPE type) {if(  Elm *elm=findElm(text))if(elm->type==type)return elm   ; return null   ;}
   UID  Project::findElmID(C UID &id  , ELM_TYPE type) {if(  Elm *elm=findElm(id  , type            ))return elm->id; return UIDZero;}
   UID  Project::findElmID(C Str &text, ELM_TYPE type) {if(  Elm *elm=findElm(text, type            ))return elm->id; return UIDZero;}
   Elm* Project::findElmImage(C Str &text) {if(Elm *elm=findElm     (text))if(ElmImageLike(elm->type))return elm   ; return null   ;}
   UID  Project::findElmImageID(C Str &text) {if(Elm *elm=findElmImage(text                          ))return elm->id; return UIDZero;}
   Elm* Project::findElmByTexture(C UID &tex_id)
   {
      Elm *ret=null;
      REPA(elms) // find first material containing this texture
      {
         Elm &elm=elms[i]; if(elm.data && elm.data->containsTex(tex_id, true))
         {
            if(elm.finalPublish())return &elm; // if found a publishable then return immediately
            if(elm.finalExists ())ret   =&elm; // save existing for later
         }
      }
      return ret;
   }
           Str Project::elmFullName(C UID &elm_id, int max_elms)C {if(C Elm *elm=findElm(elm_id))return elmFullName(elm, max_elms); return elm_id.valid() ? UnknownName : S;}
   Str Project::elmFullName(C Elm *elm   , int max_elms)C
   {
      int length=0; Memt<C Elm*> processed; Str name;
      for(; elm && processed.include(elm); )
      {
         if(ElmVisible(elm->type)) // don't include name of elements that are not visible
         {
            if(!max_elms--){processed.removeLast(); name.reserve(length+3)="..\\"; break;} // if reached the allowed limit
            length+=elm->name.length()+1; // 1 extra for '\\'
         }
         elm=findElm(elm->parent_id);
      }
      name.reserve(length); bool slash=false;
      REPA(processed)
      {
       C Elm &elm=*processed[i]; if(ElmVisible(elm.type))
         {
            if(slash)name+='\\';else slash=true;
            name+=elm.name;
         }
      }
      return name;
   }
   Str Project::basePath(C Elm &elm   )C {return elm.id.valid() ? (ElmEdit(elm.type) ? edit_path : game_path)+EncodeFileName(elm.id)         : S;}
   Str Project::codeBasePath(C UID &elm_id)C {return elm_id.valid() ?                      code_base_path        +EncodeFileName(elm_id)+CodeExt : S;}
   Str Project::codePath(C UID &elm_id)C {return elm_id.valid() ?                      code_path             +EncodeFileName(elm_id)+CodeExt : S;}
   Str Project::editPath(C UID &elm_id)C {return elm_id.valid() ?                      edit_path             +EncodeFileName(elm_id)         : S;}
   Str Project::gamePath(C UID &elm_id)C {return elm_id.valid() ?                                  game_path +EncodeFileName(elm_id)         : S;}
   Str Project::codeBasePath(C Elm &elm   )C {return codeBasePath(elm.id);}
   Str Project::codePath(C Elm &elm   )C {return     codePath(elm.id);}
   Str Project::editPath(C Elm &elm   )C {return     editPath(elm.id);}
   Str Project::gamePath(C Elm &elm   )C {return     gamePath(elm.id);}
   Str  Project::texPath(C UID &tex_id)C {return tex_id.valid() ?                                    tex_path +EncodeFileName(tex_id) : S;}
   Str  Project::texDynamicPath(C UID &tex_id)C {return tex_id.valid() ?                       temp_tex_dynamic_path +EncodeFileName(tex_id) : S;}
   Str  Project::texFormatPath(C UID &tex_id, cchar8 *format, int downsize)C {return tex_id.valid() ? temp_tex_path +EncodeFileName(tex_id)+format+ImageDownSizeSuffix(downsize) : S;}
   Str     Project::formatPath(C UID &elm_id, cchar8 *suffix              )C {return elm_id.valid() ?     temp_path +EncodeFileName(elm_id)+suffix                               : S;}
   Str Project::gameAreaPath(C UID &world_id, C VecI2 &area_xy)C {return game_path+EncodeFileName(world_id)+"\\Area\\"+area_xy;}
   Str Project::editAreaPath(C UID &world_id, C VecI2 &area_xy)C {return edit_path+EncodeFileName(world_id)+"\\Area\\"+area_xy;}
   Str Project::gameWaypointPath(C UID &world_id, C UID &waypoint_id)C {return game_path+EncodeFileName(world_id)+"\\Waypoint\\"+EncodeFileName(waypoint_id);}
   Str Project::editWaypointPath(C UID &world_id, C UID &waypoint_id)C {return edit_path+EncodeFileName(world_id)+"\\Waypoint\\"+EncodeFileName(waypoint_id);}
   bool Project::waypointExists(C UID &world_id, C UID &waypoint_id)C {return FExist(gameWaypointPath(world_id, waypoint_id));}
   Str Project::EditLakePath(C Str &edit_path, C UID &world_id, C UID & lake_id)  {return edit_path+EncodeFileName(world_id)+"\\Lake\\" +EncodeFileName( lake_id);}
   Str Project::EditRiverPath(C Str &edit_path, C UID &world_id, C UID &river_id)  {return edit_path+EncodeFileName(world_id)+"\\River\\"+EncodeFileName(river_id);}
          Str Project::editLakePath(                  C UID &world_id, C UID & lake_id)C {return EditLakePath (edit_path, world_id,  lake_id);}
          Str Project::editRiverPath(                  C UID &world_id, C UID &river_id)C {return EditRiverPath(edit_path, world_id, river_id);}
   Str       Project::worldVerPath(C UID &world_id)C{return editPath(world_id)+WorldVerSuffix;}
   WorldVer* Project::worldVerFind(C UID &world_id) {return world_vers.find(worldVerPath(world_id));}
   WorldVer* Project::worldVerGet(C UID &world_id) {return world_vers.get (worldVerPath(world_id));}
   WorldVer* Project::worldVerRequire(C UID &world_id) {return world_vers     (worldVerPath(world_id));}
   Str         Project::miniMapVerPath(C UID &mini_map_id)C{return editPath(mini_map_id);}
   MiniMapVer* Project::miniMapVerFind(C UID &mini_map_id) {return mini_map_vers.find(miniMapVerPath(mini_map_id));}
   MiniMapVer* Project::miniMapVerGet(C UID &mini_map_id) {return mini_map_vers.get (miniMapVerPath(mini_map_id));}
   MiniMapVer* Project::miniMapVerRequire(C UID &mini_map_id) {return mini_map_vers     (miniMapVerPath(mini_map_id));}
   UID Project::physToMesh(C Elm *phys) {if(phys)if(C ElmPhys *phys_data=phys->physData())return phys_data->mesh_id; return UIDZero;}
   UID Project::animToSkel(C Elm *anim) {if(anim)if(C ElmAnim *anim_data=anim->animData())return anim_data->skel_id; return UIDZero;}
   UID Project::skelToMesh(C Elm *skel) {if(skel)if(C ElmSkel *skel_data=skel->skelData())return skel_data->mesh_id; return UIDZero;}
   UID Project::meshToSkel(C Elm *mesh) {if(mesh)if(C ElmMesh *mesh_data=mesh->meshData())return mesh_data->skel_id; return UIDZero;}
   UID Project::meshToObj(C Elm *mesh) {if(mesh)if(C ElmMesh *mesh_data=mesh->meshData())return mesh_data-> obj_id; return UIDZero;}
   UID  Project::objToMesh(C Elm *obj ) {if(obj )if(C ElmObj  * obj_data= obj-> objData())return  obj_data->mesh_id; return UIDZero;}
   UID Project::animToMesh(C Elm *anim) {return skelToMesh(animToSkel(anim));}
   UID Project::animToObj(C Elm *anim) {return meshToObj (animToMesh(anim));}
   UID Project::skelToObj(C Elm *skel) {return meshToObj (skelToMesh(skel));}
   UID Project::physToObj(C Elm *phys) {return meshToObj (physToMesh(phys));}
   UID  Project::objToSkel(C Elm *obj ) {return meshToSkel( objToMesh(obj ));}
   UID Project::animToSkel(C UID &anim_id) {return animToSkel(findElm(anim_id));}
   UID Project::animToMesh(C UID &anim_id) {return animToMesh(findElm(anim_id));}
   UID Project::animToObj(C UID &anim_id) {return animToObj (findElm(anim_id));}
   UID Project::skelToMesh(C UID &skel_id) {return skelToMesh(findElm(skel_id));}
   UID Project::skelToObj(C UID &skel_id) {return skelToObj (findElm(skel_id));}
   UID Project::meshToSkel(C UID &mesh_id) {return meshToSkel(findElm(mesh_id));}
   UID Project::meshToObj(C UID &mesh_id) {return meshToObj (findElm(mesh_id));}
   UID Project::physToObj(C UID &phys_id) {return physToObj (findElm(phys_id));}
   UID  Project::objToMesh(C UID & obj_id) {return  objToMesh(findElm( obj_id));}
   UID  Project::objToSkel(C UID & obj_id) {return  objToSkel(findElm( obj_id));}
   Elm* Project::objToMeshElm(C Elm * obj   ) {return findElm(objToMesh( obj   ), ELM_MESH);}
   Elm* Project::objToMeshElm(C UID & obj_id) {return findElm(objToMesh( obj_id), ELM_MESH);}
   Elm* Project::skelToObjElm(C Elm *skel   ) {return findElm(skelToObj(skel   ), ELM_OBJ );}
   Elm* Project::skelToObjElm(C UID &skel_id) {return findElm(skelToObj(skel_id), ELM_OBJ );}
   Elm* Project::meshToObjElm(C Elm *mesh   ) {return findElm(meshToObj(mesh   ), ELM_OBJ );}
   Elm* Project::meshToObjElm(C UID &mesh_id) {return findElm(meshToObj(mesh_id), ELM_OBJ );}
   Elm* Project::animToObjElm(C Elm *anim   ) {return findElm(animToObj(anim   ), ELM_OBJ );}
   Elm* Project::animToObjElm(C UID &anim_id) {return findElm(animToObj(anim_id), ELM_OBJ );}
   Elm* Project::physToObjElm(C Elm *phys   ) {return findElm(physToObj(phys   ), ELM_OBJ );}
   Elm* Project::physToObjElm(C UID &phys_id) {return findElm(physToObj(phys_id), ELM_OBJ );}
   Elm* Project::mtrlToMeshElm(C UID &mtrl_id)
   {
      if(mtrl_id.valid())REPA(elms){Elm &elm=elms[i]; if(C ElmMesh *mesh_data=elm.meshData())if(mesh_data->mtrl_ids.binaryHas(mtrl_id, Compare))return &elm;}
      return null;
   }
   UID Project::mtrlToObj(C UID &mtrl_id)
   {
      if(Elm *mesh=mtrlToMeshElm(mtrl_id))return meshToObj(mesh);
      return UIDZero;
   }
   void Project::getTextures(MemPtr<UID> texs, bool existing_elms)C // 'existing_elms'=if get only from elements that exist (are not removed)
   {
      texs.clear();
      REPA(elms){C Elm &elm=elms[i]; if(existing_elms ? elm.finalExists() : true)if(elm.data)elm.data->listTexs(texs);} // get textures from elements
   }
   void Project::getUsedMaterials(MemPtr<UID> used, bool publish)C // !! this needs to have hierarchy set !! 'publish'=if true then get materials used by publishable elements, if false then get materials used by existing elements
   {
      used.clear();
      REPA(elms)
      {
       C Elm &elm=elms[i]; if(publish ? elm.finalPublish() : elm.finalExists())switch(elm.type)
         {
            case ELM_MESH:
            {
               if(C ElmMesh *mesh_data=elm.meshData())REPA(mesh_data->mtrl_ids)used.binaryInclude(mesh_data->mtrl_ids[i], Compare);
            }break;
         }
      }
   }
   Enum* Project::getEnum(C UID &enum_id)
   {
      return Enums(gamePath(enum_id));
   }
   void Project::getMeshSkels(ElmMesh *mesh_data, UID *mesh_skel, UID *body_skel) // get skeletons for specified mesh
   {
      if(mesh_skel)*mesh_skel=(mesh_data ? mesh_data->skel_id : UIDZero);
      // get the last body
      if(body_skel)
      {
         for(Memt<UID> bodies; mesh_data && bodies.include(mesh_data->body_id); )if(Elm *body=findElm(mesh_data->body_id))mesh_data=body->meshData();
        *body_skel=(mesh_data ? mesh_data->skel_id : UIDZero);
      }
   }
   void Project::getMeshSkels(ElmMesh *mesh_data, Skeleton* *mesh_skel, Skeleton* *body_skel) // get skeletons for specified mesh
   {
      UID mesh_skel_id, body_skel_id;
      getMeshSkels(mesh_data, mesh_skel ? &mesh_skel_id : null, body_skel ? &body_skel_id : null);
      if(mesh_skel)*mesh_skel=Skeletons(gamePath(mesh_skel_id));
      if(body_skel)*body_skel=Skeletons(gamePath(body_skel_id));
   }
   bool Project::getObjBox(C UID &elm_id, Box &box) // get box of ELM_OBJ (mesh.box|phys.box)
   {
      Memt<UID> processed; for(UID id=elm_id; processed.binaryInclude(id, Compare); ) // to avoid potential infinite loops (A is based on A)
         if(Elm *obj_elm=findElm(id))if(ElmObj *obj=obj_elm->objData())
      {
         if(Elm *mesh_elm=findElm(obj->mesh_id))if(ElmMesh *mesh=mesh_elm->meshData())
         {
            bool have=false;                                                            if(mesh->box.valid())if(have)box|=mesh->box;else{box=mesh->box; have=true;}
            if(Elm *phys_elm=findElm(mesh->phys_id))if(ElmPhys *phys=phys_elm->physData())if(phys->box.valid())if(have)box|=phys->box;else{box=phys->box; have=true;}
            return have;
         }
         id=obj->base_id; // check base
      }
      box.zero(); return false;
   }
   bool Project::getObjTerrain(C UID &elm_id) // get if ELM_OBJ has OBJ_ACCESS_TERRAIN
   {
      Memt<UID> processed; for(UID id=elm_id; processed.binaryInclude(id, Compare); ) // to avoid potential infinite loops (A is based on A)
         if(Elm *obj_elm=findElm(id))
      {
         if(ElmObjClass *obj=obj_elm->objClassData())return obj->terrain();
         if(ElmObj      *obj=obj_elm->     objData()){if(obj->ovrAccess())return obj->terrain(); id=obj->base_id;} // check base
      }
      return true;
   }
   OBJ_PATH Project::getObjPath(C UID &elm_id) // get OBJ_PATH of ELM_OBJ
   {
      Memt<UID> processed; for(UID id=elm_id; processed.binaryInclude(id, Compare); ) // to avoid potential infinite loops (A is based on A)
         if(Elm *obj_elm=findElm(id))
      {
         if(ElmObjClass *obj=obj_elm->objClassData())                  return obj->pathSelf();
         if(ElmObj      *obj=obj_elm->     objData()){if(obj->ovrPath())return obj->pathSelf(); id=obj->base_id;} // check base
      }
      return OBJ_PATH_CREATE;
   }
   RectI Project::getWorldAreas(C UID &world_id, bool water)
   {
      RectI rect(0, -1);
      if(Elm *world=findElm(world_id, ELM_WORLD))
         if(WorldVer *world_ver=worldVerGet(world_id))
      {
                  Include(rect, world_ver->getTerrainAreas ());
                  Include(rect, world_ver->getObjAreas     ());
                  Include(rect, world_ver->getObjEmbedAreas());
         if(water)Include(rect, world_ver->getLakeAreas    ());
         if(water)Include(rect, world_ver->getRiverAreas   ());
      }
      return rect;
   }
   int Project::worldAreasToRebuild(C UID *world_id)
   {
      int n=0;
      if(!world_id)REPA(world_vers)n+=world_vers.lockedData(i).rebuild.elms();else // check all worlds
      if(WorldVer *world_ver=worldVerFind(*world_id)) n+=world_ver->rebuild.elms(); // check only specified world
      return n;
   }
   bool Project::materialSimplify(Edit::EXE_TYPE type)C
   {
      switch(material_simplify)
      {
         default       : return false; // MS_NEVER
         case MS_MOBILE: return type==Edit::EXE_APK || type==Edit::EXE_IOS;
         case MS_ALWAYS: return true;
      }
   }
   bool Project::isBasedOnObjs(C Elm &elm, C Memt<UID> &objs)C // check if 'elm' is based on 'objs' (assumes that 'objs' is sorted)
   {
      Memt<UID> processed;
      if(elm.type==ELM_OBJ || elm.type==ELM_OBJ_CLASS)for(UID id=elm.id; processed.binaryInclude(id, Compare); ) // to avoid potential infinite loops (A is based on A)
         if(C Elm *obj_elm=findElm(id))
      {
         if(objs.binaryHas(id, Compare))return true; // if this object is in the 'objs'
         if(C ElmObj *obj=obj_elm->objData())id=obj->base_id; // proceed to base
      }
      return false;
   }
   void Project::getExtendedObjs(C Memt<UID> &objs, Memt<UID> &exts)C // get list of all objects that are based on 'objs' (assumes that 'objs' is sorted)
   {
      REPA(elms){C Elm &elm=elms[i]; if(isBasedOnObjs(elm, objs))exts.binaryInclude(elm.id, Compare);}
   }
   bool Project::idToValid(C UID &id) // if target is valid (not removed)
   {
      if(id.valid())
      {
         if(Elm *elm=findElm(id))return elm->finalPublish();

         CacheLock cl(world_vers);
         REPA(world_vers) // check all worlds for objects/waypoints
         {
             WorldVer &world_ver=world_vers.lockedData(i);
            if(ObjVer *  obj_ver=world_ver.obj      .find(id))return !obj_ver->removed();
          //if(                  world_ver.waypoints.find(id))return ;
         }
      }
      return true;
   }
   Str Project::idToText(C UID &id, bool *valid, ID_MODE id_mode) // 'valid'=if target is valid (not removed)
   {
      if(valid)*valid=true;
      if(id.valid())
      {
         if(Elm *elm=findElm(id)) // check project element
         {
            if(valid && elm->finalNoPublish())*valid=false;
            return elmFullName(elm);
         }

         CacheLock cl(world_vers);
         REPA(world_vers) // check all worlds for objects/waypoints
         {
             WorldVer &world_ver=world_vers.lockedData(i);
            if(ObjVer *  obj_ver=world_ver.obj      .find(id)){if(valid && obj_ver->removed())*valid=false; return (S+"Object "  +((id_mode==ID_SKIP_UNKNOWN) ? S : id.asCString())).space()+"in World \""+elmFullName(world_ver.world_id)+'"';}
            if(                  world_ver.waypoints.find(id)){                                            return (S+"Waypoint "+((id_mode==ID_SKIP_UNKNOWN) ? S : id.asCString())).space()+"in World \""+elmFullName(world_ver.world_id)+'"';}
         }
      }
      if(id_mode==ID_ALWAYS
      || id_mode==ID_SKIP_ZERO && id.valid())return id.asCString();
      return S;
   }
   bool Project::invalidSrc(C Str &src, Str *invalid)C // if specified and is not present
   {
      if(src.is())
      {
         Mems<Edit::FileParams> files=Edit::FileParams::Decode(src); REPA(files)
         {
          C Str &name=files[i].name; if(name.is())
            {
               UID id; if(DecodeFileName(name, id)) // project element
               {
                C Elm *elm=findElm(id); if(!elm){if(invalid)*invalid=name; return true;} // INVALID
               }else // source file
               {
                  if(FileInfoSystem(name        ).type!=FSTD_FILE
                //&& FileInfoSystem(name+".cmpr").type!=FSTD_FILE // check for compressed alternative
                  )
                  {
                     if(invalid)*invalid=name; return true; // INVALID
                  }
               }
            }
         }
      }
      if(invalid)invalid->clear(); return false; // OK
   }
   bool Project::invalidTexSrc(C Str &src, Str *invalid)C // if specified and is not present
   {
      if(src.is())
      {
         Mems<Edit::FileParams> files=Edit::FileParams::Decode(src); REPA(files)
         {
          C Str &name=files[i].name; if(name.is())
            {
               UID id;
               if(name[0]=='<') // another image
               {
                  
               }else
               if(DecodeFileName(name, id)) // project element
               {
                C Elm *elm=findElm(id); if(!elm || !ElmImageLike(elm->type)){if(invalid)*invalid=name; return true;} // INVALID
               }else // source file
               {
                  if(FileInfoSystem(name        ).type!=FSTD_FILE
                //&& FileInfoSystem(name+".cmpr").type!=FSTD_FILE // check for compressed alternative
                  )
                  {
                     if(invalid)*invalid=name; return true; // INVALID
                  }
               }
            }
         }
      }
      if(invalid)invalid->clear(); return false; // OK
   }
   bool Project::invalidTex(C UID &tex_id                     )C {return tex_id.valid() && !texs.binaryHas(tex_id, Compare);}
   bool Project::invalidRef(C UID &elm_id, bool optional)C // is specified and is not present
   {
      if(!elm_id.valid())return false; // empty/null reference is valid
      if(C Elm *elm=findElm(elm_id))return optional ? elm->finalRemoved() : elm->finalNoPublish(); // if it's optional, then we need to check if it exists, if required then we need to check if exists and is publishable
      return true; // element was not found = invalid
   }
   bool Project::invalidRefs(Elm &elm) // check if this element has invalid references
   {
      switch(elm.type)
      {
         case ELM_OBJ: if(ElmObj *data=elm.objData())
         {
            if(Elm *mesh_elm=findElm(data->mesh_id))if(invalidRefs(*mesh_elm))return true; // process mesh because it's hidden
            return invalidRef(data->mesh_id) || invalidRef(data->base_id);
         }break;

         case ELM_MESH: if(ElmMesh *data=elm.meshData())
         {
          //if(Elm * obj_elm=findElm(data. obj_id))if(invalidRefs(* obj_elm))return true; here we don't do this because 'obj' is a parent and is visible
            if(Elm *skel_elm=findElm(data->skel_id))if(invalidRefs(*skel_elm))return true; // process skel because it's hidden
            if(Elm *phys_elm=findElm(data->phys_id))if(invalidRefs(*phys_elm))return true; // process phys because it's hidden
            REPA(data->mtrl_ids)if(invalidRef(data->mtrl_ids[i]))return true;
            return invalidRef(data->obj_id) || invalidRef(data->skel_id) || invalidRef(data->body_id) || invalidRef(data->phys_id) || invalidRef(data->draw_group_id);
         }break;

         case ELM_SKEL: if(ElmSkel *data=elm.skelData())
         {
            return invalidRef(data->mesh_id);
         }break;

         case ELM_PHYS: if(ElmPhys *data=elm.physData())
         {
            return invalidRef(data->mesh_id) || invalidRef(data->mtrl_id);
         }break;

         case ELM_MTRL: if(ElmMaterial *data=elm.mtrlData())
         {
            return invalidTex(data->base_0_tex) || invalidTex(data->base_1_tex) || invalidTex(data->detail_tex) || invalidTex(data->macro_tex) || invalidTex(data->reflection_tex) || invalidTex(data->light_tex);
         }break;

         case ELM_WATER_MTRL: if(ElmWaterMtrl *data=elm.waterMtrlData())
         {
            return invalidTex(data->base_0_tex) || invalidTex(data->base_1_tex) || invalidTex(data->reflection_tex);
         }break;

         case ELM_ANIM: if(ElmAnim *data=elm.animData())
         {
            return invalidRef(data->skel_id);
         }break;

         case ELM_IMAGE_ATLAS: if(ElmImageAtlas *data=elm.imageAtlasData())
         {
            REPA(data->images)if(!data->images[i].removed && invalidRef(data->images[i].id, true))return true;
         }break;

         case ELM_ICON: if(ElmIcon *data=elm.iconData())
         {
            return invalidRef(data->icon_settings_id, true) || invalidRef(data->obj_id, true);
         }break;

         case ELM_TEXT_STYLE: if(ElmTextStyle *data=elm.textStyleData())
         {
            return invalidRef(data->font_id);
         }break;

         case ELM_PANEL_IMAGE: if(ElmPanelImage *data=elm.panelImageData())
         {
            REPA(data->image_ids)if(invalidRef(data->image_ids[i], true))return true;
         }break;

         case ELM_PANEL: if(ElmPanel *data=elm.panelData())
         {
            REPA(data->image_ids)if(invalidRef(data->image_ids[i]))return true;
         }break;

         case ELM_GUI_SKIN: if(ElmGuiSkin *data=elm.guiSkinData())
         {
            REPA(data->elm_ids)if(invalidRef(data->elm_ids[i]))return true;
         }break;

         case ELM_ENV: if(ElmEnv *data=elm.envData())
         {
            REPA(data->cloud_id)if(invalidRef(data->cloud_id[i]))return true;
            return invalidRef(data->sun_id) || invalidRef(data->skybox_id) || invalidRef(data->star_id);
         }break;

         case ELM_WORLD: if(ElmWorld *data=elm.worldData())
         {
            return invalidRef(data->env_id);
         }break;

         case ELM_MINI_MAP: if(ElmMiniMap *data=elm.miniMapData())
         {
            return invalidRef(data->world_id, true) || invalidRef(data->env_id, true);
         }break;

         case ELM_APP: if(ElmApp *data=elm.appData())
         {
            return invalidRef(data->icon, true) || invalidRef(data->image_portrait, true) || invalidRef(data->image_landscape, true);
         }break;
      }
      return false;
   }
   Elm& Project::getFolder(C Str &name, C UID &parent_id, bool &added, bool ignore_removed)
   {
      // can't use hierarchy because this func is used when new elements are added
      added=false; REPA(elms)if(elms[i].parent_id==parent_id && elms[i].name==name && (ignore_removed ? !elms[i].removed() : true))return elms[i];
      added=true ; return newElm(name, parent_id, ELM_FOLDER);
   }
   UID Project::getPathID(C Str &path, C UID &parent_id, bool ignore_removed) // create all project element folders from 'path' and return last element's id
   {
      UID parent=parent_id;
      for(Str p=path; p.is(); )
      {
         Str start=GetStart   (p);
                 p=GetStartNot(p);
         bool added; parent=getFolder(start, parent, added, ignore_removed).id;
      }
      return parent;
   }
   bool Project::getWorldPaths(C UID &world_id, Str &world_edit_path, Str &world_game_path)
   {
      if(world_id.valid())
      {
         world_edit_path=T.edit_path+EncodeFileName(world_id).tailSlash(true);
         world_game_path=T.game_path+EncodeFileName(world_id).tailSlash(true);
         return true;
      }else
      {
         world_edit_path.clear();
         world_game_path.clear();
         return false;
      }
   }
   void Project::createWorldPaths(C UID &world_id)
   {
      if(world_id.valid() && world_paths.binaryInclude(world_id, Compare)) // create paths only at first time
      {
         Str edit, game; if(getWorldPaths(world_id, edit, game))
         {
            FCreateDirs(edit+"Area");
            FCreateDirs(game+"Area");
            FCreateDirs(edit+"Waypoint");
            FCreateDirs(game+"Waypoint");
            FCreateDirs(edit+"Lake");
            FCreateDirs(edit+"River");
         }
      }
   }
   void Project::createMiniMapPaths(C UID &mini_map_id)
   {
      if(mini_map_id.valid() && mini_map_paths.binaryInclude(mini_map_id, Compare)) // create paths only at first time
         FCreateDirs(gamePath(mini_map_id));
   }
   int Project::ChannelIndex(char c)
   {
      switch(c)
      {
         case 'r': case 'R': case 'x': case 'X': return 0;
         case 'g': case 'G': case 'y': case 'Y': return 1;
         case 'b': case 'B': case 'z': case 'Z': return 2;
         case 'a': case 'A': case 'w': case 'W': return 3;
      }
      return -1;
   }
   IMAGE_TYPE Project::HighPrecType(IMAGE_TYPE type)
   {
      switch(type)
      {
         case IMAGE_R8    : case IMAGE_I8: case IMAGE_I16: case IMAGE_I24: case IMAGE_I32: return IMAGE_F32;
         case IMAGE_R8G8  :                                                                return IMAGE_F32_2;
         case IMAGE_R8G8B8:                                                                return IMAGE_F32_3;
         default          :                                                                return IMAGE_F32_4;
      }
   }
   void Project::MakeHighPrec(Image &image)
   {
      if(!image.highPrecision())image.copyTry(image, -1, -1, -1, HighPrecType(image.type()));
   }
   void Project::ContrastLum(Image &image, flt contrast, flt avg_lum, C BoxI &box)
   {
      if(contrast!=1 && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            flt  c_lum=c.xyz.max(), want_lum=(c_lum-avg_lum)*contrast+avg_lum;
            if(c_lum>EPS)c.xyz*=want_lum/c_lum;else c.xyz=want_lum;
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }      
   }
   void Project::AvgContrastLum(Image &image, flt contrast, dbl avg_lum, C BoxI &box)
   {
      if(avg_lum && image.lock()) // lock for writing because we will use this lock for applying contrast too
      {
         dbl contrast_total=0, weight_total=0;
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z); dbl c_lum=c.xyz.max();
            if(dbl d=c_lum-avg_lum)
            {
               dbl contrast=Abs(d)/avg_lum, // div by 'avg_lum' so for bright values contrast will be proportionally the same
                   weight=Sqr(d); // squared distance from avg_lum
               contrast_total+=weight*contrast;
                 weight_total+=weight;
            }
         }
         if(weight_total)if(contrast_total/=weight_total)ContrastLum(image, contrast/contrast_total, avg_lum, box);
         image.unlock();
      }
   }
   void Project::AddHue(Image &image, flt hue, C BoxI &box)
   {
      hue=Frac(hue);
      if(hue && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            c.xyz=RgbToHsb(c.xyz);
            c.x +=hue;
            c.xyz=HsbToRgb(c.xyz);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }
   void Project::MulRGBH(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, C BoxI &box)
   {
      flt mul[]={red, yellow, green, cyan, blue, purple, red, yellow}; // red and yellow are listed extra for wraparound
      REP(6)if(mul[i]!=1)goto mul; return; mul:
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         Vec  hsb=RgbToHsb(c.xyz);
         flt  hue=hsb.x*6; int hue_i=Trunc(hue); flt hue_frac=hue-hue_i;
         flt  hue_mul=Lerp(mul[hue_i], mul[hue_i+1], hue_frac);
         c.xyz*=hue_mul;
         image.color3DF(x, y, z, c);
      }
   }
   void Project::MulRGBHS(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, C BoxI &box)
   {
      flt mul[]={red, yellow, green, cyan, blue, purple, red, yellow}; // red and yellow are listed extra for wraparound
      REP(6)if(mul[i]!=1)goto mul; return; mul:
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         Vec  hsb=RgbToHsb(c.xyz);
         flt  hue=hsb.x*6; int hue_i=Trunc(hue); flt hue_frac=hue-hue_i;
         flt  hue_mul=Lerp(mul[hue_i], mul[hue_i+1], hue_frac);
         c.xyz*=Lerp(1.0f, hue_mul, hsb.y);
         image.color3DF(x, y, z, c);
      }
   }
   void Project::MulSatH(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, bool sat, bool photo, C BoxI &box)
   {
      flt mul[]={red, yellow, green, cyan, blue, purple, red, yellow}; // red and yellow are listed extra for wraparound
      REP(6)if(mul[i]!=1)goto mul; return; mul:
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
         Vec  hsb=RgbToHsb(c.xyz);
         flt  hue=hsb.x*6; int hue_i=Trunc(hue); flt hue_frac=hue-hue_i;
         flt  sat_mul=Lerp(mul[hue_i], mul[hue_i+1], hue_frac);
         if(sat)sat_mul=Lerp(1.0f, sat_mul, hsb.y);
         hsb.y*=sat_mul;
         c.xyz=HsbToRgb(hsb);
         if(photo)
         {
            c.xyz=SRGBToLinear(c.xyz);
            if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum;
            c.xyz=LinearToSRGB(c.xyz);
         }
         image.color3DF(x, y, z, c);
      }
   }
   flt Project::HueDelta(flt a, flt b) // returns -0.5 .. 0.5
   {
      flt d=Frac(b-a); if(d>0.5f)d-=1; return d;
   }
   Vec2  Project::LerpToMad(flt from, flt to) {return Vec2(to-from, from);}
   Vec2 Project::ILerpToMad(flt from, flt to) {return Vec2(1/(to-from), from/(from-to));}
   flt   Project::FloatSelf(flt x) {return x;}
   flt   Project::PowMax(flt x, flt y) {return (x<=0) ? 0 : Pow(x, y);}
   void Project::IncludeAlpha(Image &image) {image.copyTry(image, -1, -1, -1, ImageTypeIncludeAlpha(image.type()));}
   void Project::TransformImage(Image &image, C MemPtr<TextParam> &params, bool clamp)
   {
      int high_prec=0; REPA(params)high_prec+=HighPrecTransform(params[i].name); // how many transforms are high precision
      if( high_prec>1)MakeHighPrec(image);
      FREPA(params)
      {
         TextParam param=params[i];
         BoxI box(0, image.size3());
         int at_pos=TextPosI(param.value, '@'); if(at_pos>=0)
         {
            VecI4 v=TextVecI4(param.value()+at_pos+1);
            RectI r(v.xy, v.xy+v.zw);
            box&=BoxI(VecI(r.min, 0), VecI(r.max, box.max.z));
            param.value.clip(at_pos);
         }
         if(param.name=="crop")
         {
            VecI4 v=TextVecI4(param.value);
            image.crop(image, v.x, v.y, v.z, v.w);
         }else
         if(param.name=="resize" || param.name=="resizeWrap" || param.name=="resizeClamp" || param.name=="resizeLinear" || param.name=="resizeCubic" || param.name=="resizeNoStretch")
         {
            VecI2 s;
            if(param.value=="quarter")s.set(Max(1, image.w()/4), Max(1, image.h()/4));else
            if(param.value=="half"   )s.set(Max(1, image.w()/2), Max(1, image.h()/2));else
            if(param.value=="double" )s=image.size()*2;else
            {
               Vec2 sf; if(Contains(param.value, ','))sf=param.asVec2();else sf=param.asFlt();
               UNIT_TYPE unit=GetUnitType(param.value);
               s.x=Round(ConvertUnitType(sf.x, image.w(), unit));
               s.y=Round(ConvertUnitType(sf.y, image.h(), unit));
            }
            s=ImageSize(image.size3(), s, false).xy;
            image.resize(s.x, s.y, (param.name=="resizeNoStretch") ? FILTER_NO_STRETCH : (param.name=="resizeLinear") ? FILTER_LINEAR : (param.name=="resizeCubic") ? FILTER_CUBIC_FAST : FILTER_BEST, (param.name=="resizeClamp") ? true : (param.name=="resizeWrap" || param.name=="resizeNoStretch") ? false : clamp);
         }else
         if(param.name=="maxSize")
         {
            VecI2 s;
            if(param.value=="quarter")s.set(Max(1, image.w()/4), Max(1, image.h()/4));else
            if(param.value=="half"   )s.set(Max(1, image.w()/2), Max(1, image.h()/2));else
            if(param.value=="double" )s=image.size()*2;else
            {
               Vec2 sf; if(Contains(param.value, ','))sf=param.asVec2();else sf=param.asFlt();
               UNIT_TYPE unit=GetUnitType(param.value);
               s.x=Round(ConvertUnitType(sf.x, image.w(), unit));
               s.y=Round(ConvertUnitType(sf.y, image.h(), unit));
            }
            s=ImageSize(image.size3(), s, false).xy;
            image.resize(Min(image.w(), s.x), Min(image.h(), s.y), FILTER_BEST, clamp);
         }else
         if(param.name=="tile")image.tile(param.asInt());else
         if(param.name=="inverseRGB")
         {
            if(image.highPrecision())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.xyz=1-c.xyz; image.color3DF(x, y, z, c);}
            }else
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.r=255-c.r; c.g=255-c.g; c.b=255-c.b; image.color3D(x, y, z, c);}
            }
         }else
         if(param.name=="inverseR")
         {
            if(image.highPrecision())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.x=1-c.x; image.color3DF(x, y, z, c);}
            }else
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.r=255-c.r; image.color3D(x, y, z, c);}
            }
         }else
         if(param.name=="inverseG")
         {
            if(image.highPrecision())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.y=1-c.y; image.color3DF(x, y, z, c);}
            }else
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.g=255-c.g; image.color3D(x, y, z, c);}
            }
         }else
         if(param.name=="inverseRG")
         {
            if(image.highPrecision())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.x=1-c.x; c.y=1-c.y; image.color3DF(x, y, z, c);}
            }else
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.r=255-c.r; c.g=255-c.g; image.color3D(x, y, z, c);}
            }
         }else
         if(param.name=="swapRG")
         {
            if(image.highPrecision())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); Swap(c.x, c.y); image.color3DF(x, y, z, c);}
            }else
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); Swap(c.r, c.g); image.color3D(x, y, z, c);}
            }
         }else
         if(param.name=="swapXY")
         {
            Image temp; temp.createSoftTry(image.h(), image.w(), image.d(), image.type());
            if(temp.highPrecision())
            {
               REPD(y, image.h())
               REPD(x, image.w())temp.colorF(y, x, image.colorF(x, y));
            }else
            {
               REPD(y, image.h())
               REPD(x, image.w())temp.color(y, x, image.color(x, y));
            }
            Swap(temp, image);
         }else
         if(param.name=="mirrorX")image.mirrorX();else
         if(param.name=="mirrorY")image.mirrorY();else
         if(param.name=="normalize")image.normalize(true, true, true, true, &box);else
         if(param.name=="sat")
         {
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++)
            {
               Vec4 c=image.color3DF(x, y, z);
               c.sat();
               image.color3DF(x, y, z, c);
            }
         }else
         if(param.name=="blur")
         {
            UNIT_TYPE unit=GetUnitType(param.value);
            Vec       r   =TextVecEx  (param.value);
            r.x=ConvertUnitType(r.x, image.w(), unit);
            r.y=ConvertUnitType(r.y, image.h(), unit);
            r.z=ConvertUnitType(r.z, image.d(), unit);
            image.blur(r, clamp, &WorkerThreads);
         }else
         if(param.name=="lerpRGB")
         {
            Memc<Str> c; Split(c, param.value, ',');
            switch(c.elms())
            {
               case 2: {Vec2 ma=LerpToMad(TextFlt(c[0]), TextFlt(c[1])); image.mulAdd(Vec4(Vec(ma.x), 1), Vec4(Vec(ma.y), 0), &box);} break;
               case 6: {Vec2 ma[3]={LerpToMad(TextFlt(c[0]), TextFlt(c[3])), LerpToMad(TextFlt(c[1]), TextFlt(c[4])), LerpToMad(TextFlt(c[2]), TextFlt(c[5]))}; image.mulAdd(Vec4(ma[0].x, ma[1].x, ma[2].x, 1), Vec4(ma[0].y, ma[1].y, ma[2].y, 0), &box);} break;
            }
         }else
         if(param.name=="ilerpRGB")
         {
            Memc<Str> c; Split(c, param.value, ',');
            switch(c.elms())
            {
               case 2: {Vec2 ma=ILerpToMad(TextFlt(c[0]), TextFlt(c[1])); image.mulAdd(Vec4(Vec(ma.x), 1), Vec4(Vec(ma.y), 0), &box);} break;
               case 6: {Vec2 ma[3]={ILerpToMad(TextFlt(c[0]), TextFlt(c[3])), ILerpToMad(TextFlt(c[1]), TextFlt(c[4])), ILerpToMad(TextFlt(c[2]), TextFlt(c[5]))}; image.mulAdd(Vec4(ma[0].x, ma[1].x, ma[2].x, 1), Vec4(ma[0].y, ma[1].y, ma[2].y, 0), &box);} break;
            }
         }else
         if(param.name=="mulA"  ){flt alpha=param.asFlt(); if(alpha!=1){IncludeAlpha(image); image.mulAdd(Vec4(1, 1, 1, alpha), 0, &box);}}else
         if(param.name=="mulRGB")image.mulAdd(Vec4(TextVecEx(param.value), 1), 0, &box);else
         if(param.name=="addRGB")image.mulAdd(1, Vec4(TextVecEx(param.value), 0), &box);else
         if(param.name=="mulAddRGB")
         {
            Memc<Str> c; Split(c, param.value, ',');
            switch(c.elms())
            {
               case 2: image.mulAdd(Vec4(Vec(TextFlt(c[0])), 1), Vec4(Vec(TextFlt(c[1])), 0), &box); break;
               case 6: image.mulAdd(Vec4(TextFlt(c[0]), TextFlt(c[1]), TextFlt(c[2]), 1), Vec4(TextFlt(c[3]), TextFlt(c[4]), TextFlt(c[5]), 0), &box); break;
            }
         }else
         if(param.name=="addMulRGB")
         {
            Memc<Str> c; Split(c, param.value, ',');
            switch(c.elms())
            {
               // x=x*m+a, x=(x+A)*M
               case 2: {flt add=TextFlt(c[0]), mul=TextFlt(c[1]);                                                               image.mulAdd(Vec4(Vec(mul), 1), Vec4(Vec(add*mul), 0), &box);} break;
               case 6: {Vec add(TextFlt(c[0]), TextFlt(c[1]), TextFlt(c[2])), mul(TextFlt(c[3]), TextFlt(c[4]), TextFlt(c[5])); image.mulAdd(Vec4(    mul , 1), Vec4(    add*mul , 0), &box);} break;
            }
         }else
         if(param.name=="mulRGBS")
         {
            Vec mul=TextVecEx(param.value);
            if(mul!=VecOne)
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++)
            {
               Vec4 c=image.color3DF(x, y, z);
               flt  sat=RgbToHsb(c.xyz).y;
               c.x=Lerp(c.x, c.x*mul.x, sat);  // red
               c.y=Lerp(c.y, c.y*mul.y, sat);  // green
               c.z=Lerp(c.z, c.z*mul.z, sat);  // blue
               image.color3DF(x, y, z, c);
            }
         }else
         if(param.name=="mulRGBH")
         {
            Mems<Str> vals; Split(vals, param.value, ',');
            switch(vals.elms())
            {
               case 1: {flt v=TextFlt(vals[0]); MulRGBH(image, v, v, v, v, v, v, box);} break;
               case 3: MulRGBH(image, TextFlt(vals[0]), 1, TextFlt(vals[1]), 1, TextFlt(vals[2]), 1, box); break;
               case 6: MulRGBH(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), box); break;
            }
         }else
         if(param.name=="mulRGBHS")
         {
            Mems<Str> vals; Split(vals, param.value, ',');
            switch(vals.elms())
            {
               case 1: {flt v=TextFlt(vals[0]); MulRGBHS(image, v, v, v, v, v, v, box);} break;
               case 3: MulRGBHS(image, TextFlt(vals[0]), 1, TextFlt(vals[1]), 1, TextFlt(vals[2]), 1, box); break;
               case 6: MulRGBHS(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), box); break;
            }
         }else
         if(param.name=="gamma")
         {
            Vec g=TextVecEx(param.value);
            if(g!=VecOne)
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.xyz.set(PowMax(c.x, g.x), PowMax(c.y, g.y), PowMax(c.z, g.z)); image.color3DF(x, y, z, c);}
         }else
         if(param.name=="gammaLum")
         {
            flt g=param.asFlt();
            if(g!=1)
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); if(flt lum=c.xyz.max()){c.xyz*=PowMax(lum, g)/lum; image.color3DF(x, y, z, c);}}
         }else
         if(param.name=="brightness")
         {
            Vec b=TextVecEx(param.value), mul; if(b.any())
            {
               flt (*R)(flt);
               flt (*G)(flt);
               flt (*B)(flt);
               if(!b.x){b.x=1; mul.x=1; R=FloatSelf;}else if(b.x<0){b.x=SigmoidSqrt(b.x); mul.x=1/SigmoidSqrtInv(b.x); R=SigmoidSqrtInv;}else{mul.x=1/SigmoidSqrt(b.x); R=SigmoidSqrt;}
               if(!b.y){b.y=1; mul.y=1; G=FloatSelf;}else if(b.y<0){b.y=SigmoidSqrt(b.y); mul.y=1/SigmoidSqrtInv(b.y); G=SigmoidSqrtInv;}else{mul.y=1/SigmoidSqrt(b.y); G=SigmoidSqrt;}
               if(!b.z){b.z=1; mul.z=1; B=FloatSelf;}else if(b.z<0){b.z=SigmoidSqrt(b.z); mul.z=1/SigmoidSqrtInv(b.z); B=SigmoidSqrtInv;}else{mul.z=1/SigmoidSqrt(b.z); B=SigmoidSqrt;}
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=Sqr(c.xyz);
                  c.x=R(c.x*b.x)*mul.x;
                  c.y=G(c.y*b.y)*mul.y;
                  c.z=B(c.z*b.z)*mul.z;
                  c.xyz=Sqrt(c.xyz);
                  image.color3DF(x, y, z, c);
               }
            }
         }else
         if(param.name=="brightnessLum")
         {
            flt b=param.asFlt(), mul; flt (*f)(flt);
            if(b)
            {
               if(b<0){b=SigmoidSqrt(b); mul=1/SigmoidSqrtInv(b); f=SigmoidSqrtInv;}else{mul=1/SigmoidSqrt(b); f=SigmoidSqrt;}
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  if(flt l=c.xyz.max())
                  {
                     flt new_lum=Sqr(l);
                     new_lum=f(new_lum*b)*mul;
                     new_lum=Sqrt(new_lum);
                     c.xyz*=new_lum/l;
                     image.color3DF(x, y, z, c);
                  }
               }
            }
         }else
         if(param.name=="contrast")
         {
            Vec contrast=TextVecEx(param.value); if(contrast!=VecOne)
            {
               Vec4 avg; if(image.stats(null, null, &avg, null, null, null, &box))
               {
                  // col=(col-avg)*contrast+avg
                  // col=col*contrast+avg*(1-contrast)
                  image.mulAdd(Vec4(contrast, 1), Vec4(avg.xyz*(Vec(1)-contrast), 0), &box);
               }
            }
         }else
         if(param.name=="contrastAlphaWeight")
         {
            Vec contrast=TextVecEx(param.value); if(contrast!=VecOne)
            {
               Vec avg; if(image.stats(null, null, null, null, null, &avg, &box))
               {
                  image.mulAdd(Vec4(contrast, 1), Vec4(avg*(Vec(1)-contrast), 0), &box);
               }
            }
         }else
         if(param.name=="contrastLum")
         {
            flt contrast=param.asFlt(); if(contrast!=1)
            {
               Vec4 avg; if(image.stats(null, null, &avg, null, null, null, &box))ContrastLum(image, contrast, avg.xyz.max(), box);
            }
         }else
         if(param.name=="contrastLumAlphaWeight")
         {
            flt contrast=param.asFlt(); if(contrast!=1)
            {
               Vec avg; if(image.stats(null, null, null, null, null, &avg, &box))ContrastLum(image, contrast, avg.max(), box);
            }
         }else
         if(param.name=="contrastHue")
         {
            flt contrast=param.asFlt(); if(contrast!=1)
            {
               Vec4 avg; if(image.stats(null, null, &avg, null, null, null, &box) && image.lock())
               {
                  flt avg_hue=RgbToHsb(avg.xyz).x;
                  for(int z=box.min.z; z<box.max.z; z++)
                  for(int y=box.min.y; y<box.max.y; y++)
                  for(int x=box.min.x; x<box.max.x; x++)
                  {
                     Vec4 c=image.color3DF(x, y, z);
                     c.xyz=RgbToHsb(c.xyz);
                     flt d_hue=HueDelta(avg_hue, c.x);
                     d_hue*=contrast;
                     Clamp(d_hue, -0.5f, 0.5f); // clamp so we don't go back
                     c.x=d_hue+avg_hue;
                     c.xyz=HsbToRgb(c.xyz);
                     image.color3DF(x, y, z, c);
                  }
                  image.unlock();
               }
            }
         }else
         if(param.name=="contrastHueAlphaWeight")
         {
            flt contrast=param.asFlt(); if(contrast!=1)
            {
               Vec avg; if(image.stats(null, null, null, null, null, &avg, &box) && image.lock())
               {
                  flt avg_hue=RgbToHsb(avg).x;
                  for(int z=box.min.z; z<box.max.z; z++)
                  for(int y=box.min.y; y<box.max.y; y++)
                  for(int x=box.min.x; x<box.max.x; x++)
                  {
                     Vec4 c=image.color3DF(x, y, z);
                     c.xyz=RgbToHsb(c.xyz);
                     flt d_hue=HueDelta(avg_hue, c.x);
                     d_hue*=contrast;
                     Clamp(d_hue, -0.5f, 0.5f); // clamp so we don't go back
                     c.x=d_hue+avg_hue;
                     c.xyz=HsbToRgb(c.xyz);
                     image.color3DF(x, y, z, c);
                  }
                  image.unlock();
               }
            }
         }else
         if(param.name=="contrastHuePow")
         {
            flt contrast=param.asFlt(); if(contrast!=1)
            {
               Vec4 avg; if(image.stats(null, null, &avg, null, null, null, &box) && image.lock())
               {
                  flt avg_hue=RgbToHsb(avg.xyz).x;
                  for(int z=box.min.z; z<box.max.z; z++)
                  for(int y=box.min.y; y<box.max.y; y++)
                  for(int x=box.min.x; x<box.max.x; x++)
                  {
                     Vec4 c=image.color3DF(x, y, z);
                     c.xyz=RgbToHsb(c.xyz);
                     flt d_hue=HueDelta(avg_hue, c.x);
                     d_hue=Sign(d_hue)*Pow(Abs(d_hue)*2, contrast)/2; // *2 to get -1..1 range
                     Clamp(d_hue, -0.5f, 0.5f); // clamp so we don't go back
                     c.x=d_hue+avg_hue;
                     c.xyz=HsbToRgb(c.xyz);
                     image.color3DF(x, y, z, c);
                  }
                  image.unlock();
               }
            }
         }else
         if(param.name=="contrastSat")
         {
            flt contrast=param.asFlt(); if(contrast!=1)
            {
               flt avg; if(image.statsSat(null, null, &avg, null, null, null, &box) && image.lock())
               {
                  for(int z=box.min.z; z<box.max.z; z++)
                  for(int y=box.min.y; y<box.max.y; y++)
                  for(int x=box.min.x; x<box.max.x; x++)
                  {
                     Vec4 c=image.color3DF(x, y, z);
                     c.xyz=RgbToHsb(c.xyz);
                     c.y=(c.y-avg)*contrast+avg;
                     c.xyz=HsbToRgb(c.xyz);
                     image.color3DF(x, y, z, c);
                  }
                  image.unlock();
               }
            }
         }else
         if(param.name=="contrastSatAlphaWeight")
         {
            flt contrast=param.asFlt(); if(contrast!=1)
            {
               flt avg; if(image.statsSat(null, null, null, null, null, &avg, &box) && image.lock())
               {
                  for(int z=box.min.z; z<box.max.z; z++)
                  for(int y=box.min.y; y<box.max.y; y++)
                  for(int x=box.min.x; x<box.max.x; x++)
                  {
                     Vec4 c=image.color3DF(x, y, z);
                     c.xyz=RgbToHsb(c.xyz);
                     c.y=(c.y-avg)*contrast+avg;
                     c.xyz=HsbToRgb(c.xyz);
                     image.color3DF(x, y, z, c);
                  }
                  image.unlock();
               }
            }
         }else
         if(param.name=="avgSat")
         {
            flt avg; if(image.statsSat(null, null, &avg, null, null, null, &box))if(avg && image.lock())
            {
               flt mul=param.asFlt()/avg;
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=RgbToHsb(c.xyz);
                  c.y*=mul;
                  c.xyz=HsbToRgb(c.xyz);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="avgLum")
         {
            Vec4 avg; if(image.stats(null, null, &avg, null, null, null, &box))if(flt avg_l=avg.xyz.max())image.mulAdd(Vec4(Vec(param.asFlt()/avg_l), 1), 0, &box);
         }else
         if(param.name=="medLum")
         {
            Vec4 med; if(image.stats(null, null, null, &med, null, null, &box))if(flt med_l=med.xyz.max())image.mulAdd(Vec4(Vec(param.asFlt()/med_l), 1), 0, &box);
         }else
         if(param.name=="avgContrastLum")
         {
            Vec4 avg; if(image.stats(null, null, &avg, null, null, null, &box))AvgContrastLum(image, param.asFlt(), avg.xyz.max(), box);
         }else
         if(param.name=="medContrastLum")
         {
            Vec4 med; if(image.stats(null, null, null, &med, null, null, &box))AvgContrastLum(image, param.asFlt(), med.xyz.max(), box);
         }else
         if(param.name=="addHue")AddHue(image, param.asFlt(), box);else
         if(param.name=="avgHue")
         {
            if(image.lock()) // lock for writing because we will use this lock for applying hue too
            {
               Vec4 col; if(image.stats(null, null, &col, null, null, null, &box))AddHue(image, HueDelta(RgbToHsb(col.xyz).x, param.asFlt()), box);
               image.unlock();
            }
         }else
         if(param.name=="medHue")
         {
            if(image.lock()) // lock for writing because we will use this lock for applying hue too
            {
               Vec4 col; if(image.stats(null, null, null, &col, null, null, &box))AddHue(image, HueDelta(RgbToHsb(col.xyz).x, param.asFlt()), box);
               image.unlock();
            }
         }else
         if(param.name=="addSat")
         {
            flt sat=param.asFlt(); if(sat && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=RgbToHsb(c.xyz);
                  c.y+=sat;
                  c.xyz=HsbToRgb(c.xyz);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="mulSat")
         {
            flt sat=param.asFlt(); if(sat!=1 && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=RgbToHsb(c.xyz);
                  c.y*=sat;
                  c.xyz=HsbToRgb(c.xyz);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="mulSatPhoto")
         {
            flt sat=param.asFlt(); if(sat!=1 && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  flt  lin_lum=LinearLumOfSRGBColor(c.xyz);

                  c.xyz=RgbToHsb(c.xyz);
                  c.y*=sat;
                  c.xyz=HsbToRgb(c.xyz);

                  c.xyz=SRGBToLinear(c.xyz);
                  if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum;
                  c.xyz=LinearToSRGB(c.xyz);

                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="mulSatH"
         || param.name=="mulSatHS")
         {
            bool sat=(param.name=="mulSatHS");
            Mems<Str> vals; Split(vals, param.value, ',');
            switch(vals.elms())
            {
               case 1: {flt v=TextFlt(vals[0]); MulSatH(image, v, v, v, v, v, v, sat, false, box);} break;
               case 3: MulSatH(image, TextFlt(vals[0]), 1, TextFlt(vals[1]), 1, TextFlt(vals[2]), 1, sat, false, box); break;
               case 6: MulSatH(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), sat, false, box); break;
            }
         }else
         if(param.name=="mulSatHPhoto"
         || param.name=="mulSatHSPhoto")
         {
            bool sat=(param.name=="mulSatHSPhoto");
            Mems<Str> vals; Split(vals, param.value, ',');
            switch(vals.elms())
            {
               case 1: {flt v=TextFlt(vals[0]); MulSatH(image, v, v, v, v, v, v, sat, true, box);} break;
               case 3: MulSatH(image, TextFlt(vals[0]), 1, TextFlt(vals[1]), 1, TextFlt(vals[2]), 1, sat, true, box); break;
               case 6: MulSatH(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), sat, true, box); break;
            }
         }else
         if(param.name=="addHueSat")
         {
            Vec2 hue_sat=param.asVec2(); if(hue_sat.any() && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=RgbToHsb(c.xyz);
                  c.xy+=hue_sat;
                  c.xyz=HsbToRgb(c.xyz);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="setHue")
         {
            if(image.lock())
            {
               flt hue=param.asFlt();
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=RgbToHsb(c.xyz);
                  c.x=hue;
                  c.xyz=HsbToRgb(c.xyz);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="setHueSat")
         {
            if(image.lock())
            {
               Vec2 hue_sat=param.asVec2();
               Vec  rgb=HsbToRgb(Vec(hue_sat, 1));
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=rgb*c.xyz.max();
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="setHueSatPhoto") // photometric
         {
            if(image.lock())
            {
               Vec2 hue_sat=param.asVec2();
               Vec  rgb=HsbToRgb(Vec(hue_sat, 1));
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=rgb*SRGBLumOfSRGBColor(c.xyz);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="lerpHue")
         {
            Vec2 hue_alpha=param.asVec2(); if(hue_alpha.y && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  Vec hsb=RgbToHsb(c.xyz);
                  hsb.x=hue_alpha.x;
                  c.xyz=Lerp(c.xyz, HsbToRgb(hsb), hue_alpha.y);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="lerpHueSat")
         {
            Vec hue_sat_alpha=param.asVec(); if(hue_sat_alpha.z && image.lock())
            {
               Vec rgb=HsbToRgb(Vec(hue_sat_alpha.xy, 1));
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=Lerp(c.xyz, rgb*c.xyz.max(), hue_sat_alpha.z);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="rollHue")
         {
            Vec2 hue_alpha=param.asVec2(); if(hue_alpha.y && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  Vec hsb=RgbToHsb(c.xyz);
                  hsb.x+=HueDelta(hsb.x, hue_alpha.x)*hue_alpha.y;
                  c.xyz=HsbToRgb(hsb);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="rollHueSat")
         {
            Vec hue_sat_alpha=param.asVec(); if(hue_sat_alpha.z && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  Vec hsb=RgbToHsb(c.xyz);
                  hsb.x+=HueDelta(hsb.x, hue_sat_alpha.x)*hue_sat_alpha.z;
                  hsb.y =Lerp    (hsb.y, hue_sat_alpha.y, hue_sat_alpha.z);
                  c.xyz=HsbToRgb(hsb);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="rollHuePhoto")
         {
            Vec2 hue_alpha=param.asVec2(); if(hue_alpha.y && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  if(flt l=SRGBLumOfSRGBColor(c.xyz))
                  {
                     Vec hsb=RgbToHsb(c.xyz);
                     hsb.x+=HueDelta(hsb.x, hue_alpha.x)*hue_alpha.y;
                     c.xyz=HsbToRgb(hsb);
                     c.xyz*=l/SRGBLumOfSRGBColor(c.xyz);
                     image.color3DF(x, y, z, c);
                  }
               }
               image.unlock();
            }
         }else
         if(param.name=="rollHueSatPhoto")
         {
            Vec hue_sat_alpha=param.asVec(); if(hue_sat_alpha.z && image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  if(flt l=SRGBLumOfSRGBColor(c.xyz))
                  {
                     Vec hsb=RgbToHsb(c.xyz);
                     hsb.x+=HueDelta(hsb.x, hue_sat_alpha.x)*hue_sat_alpha.z;
                     hsb.y =Lerp    (hsb.y, hue_sat_alpha.y, hue_sat_alpha.z);
                     c.xyz=HsbToRgb(hsb);
                     c.xyz*=l/SRGBLumOfSRGBColor(c.xyz);
                     image.color3DF(x, y, z, c);
                  }
               }
               image.unlock();
            }
         }else
         if(param.name=="grey")
         {
            if(image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=c.xyz.max();
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="greyPhoto")
         {
            if(image.lock())
            {
               for(int z=box.min.z; z<box.max.z; z++)
               for(int y=box.min.y; y<box.max.y; y++)
               for(int x=box.min.x; x<box.max.x; x++)
               {
                  Vec4 c=image.color3DF(x, y, z);
                  c.xyz=SRGBLumOfSRGBColor(c.xyz);
                  image.color3DF(x, y, z, c);
               }
               image.unlock();
            }
         }else
         if(param.name=="channel")
         {
            int channels=param.value.length();
            if( channels>=1 && channels<=4)
            {
               int   chn[4]; REPAO(chn)=ChannelIndex(param.value[i]);
               Image temp;
               if(image.highPrecision())
               {
                  temp.createSoftTry(image.w(), image.h(), image.d(), channels==1 ? IMAGE_F32 : channels==2 ? IMAGE_F32_2 : channels==3 ? IMAGE_F32_3 : IMAGE_F32_4);
                  Vec4 d(0, 0, 0, 1);
                  REPD(z, image.d())
                  REPD(y, image.h())
                  REPD(x, image.w())
                  {
                     Vec4 c=image.color3DF(x, y, z);
                     REPA(d.c){int ch=chn[i]; if(InRange(ch, c.c))d.c[i]=c.c[ch];}
                     temp.color3DF(x, y, z, d);
                  }
               }else
               {
                  temp.createSoftTry(image.w(), image.h(), image.d(), channels==1 ? IMAGE_R8 : channels==2 ? IMAGE_R8G8 : channels==3 ? IMAGE_R8G8B8 : IMAGE_R8G8B8A8);
                  Color d(0, 0, 0, 255);
                  REPD(z, image.d())
                  REPD(y, image.h())
                  REPD(x, image.w())
                  {
                     Color c=image.color3D(x, y, z);
                     REPA(d.c){int ch=chn[i]; if(InRange(ch, c.c))d.c[i]=c.c[ch];}
                     temp.color3D(x, y, z, d);
                  }
               }
               Swap(temp, image);
            }
         }else
         if(param.name=="alphaFromBrightness" || param.name=="alphaFromLum" || param.name=="alphaFromLuminance")
         {
            image.alphaFromBrightness();
         }else
         if(param.name=="bump")
         {
            Vec2 blur=-1; // x=min, y=max, -1=auto
            if(param.value.is())
            {
               UNIT_TYPE unit=GetUnitType(param.value);
               flt       full=image.size().avgF();
               if(Contains(param.value, ','))
               {
                  blur=param.asVec2(); // use 2 values if specified
                  blur.x=ConvertUnitType(blur.x, full, unit);
                  blur.y=ConvertUnitType(blur.y, full, unit);
               }else
               {
                  blur.y=param.asFlt(); // if 1 value specified then use as max
                  blur.y=ConvertUnitType(blur.y, full, unit);
               }
            }
            CreateBumpFromColor(image, image, blur.x, blur.y, &WorkerThreads);
         }else
         if(param.name=="scale") // the formula is ok (for normal too), it works as if the bump was scaled vertically by 'scale' factor
         {
            flt scale=param.asFlt(); if(scale!=1)
            {
               if(image.monochromatic())image.mulAdd(Vec4(Vec(scale), 1), Vec4(Vec(-0.5f*scale+0.5f), 0), &box);else // if image is monochromatic then we need to transform all RGB together
               if(!scale               )image.mulAdd(Vec4(Vec(    0), 1), Vec4(        0.5f, 0.5f, 1, 0), &box);else // if zero scale then set Vec(0.5, 0.5, 1)
               if(image.lock())
               {
                  scale=1/scale;
                  for(int z=box.min.z; z<box.max.z; z++)
                  for(int y=box.min.y; y<box.max.y; y++)
                  for(int x=box.min.x; x<box.max.x; x++)
                  {
                     Vec4 c=image.color3DF(x, y, z); Vec &n=c.xyz;
                     n=n*2-1;
                     if(!image.highPrecision())n=DequantizeNormal(n); // TODO: image could have 'MakeHighPrec'
                     n.z*=scale;
                     n.normalize();
                     n=n*0.5f+0.5f;
                     image.color3DF(x, y, z, c);
                  }
                  image.unlock();
               }
            }
         }else
         if(param.name=="scaleXY")
         {
            Vec2 r=TextVec2Ex(param.value);
            // v2=(v2-0.5)*r+0.5
            // v2=v2*r -0.5*r+0.5
            if(image.monochromatic()){flt a=r.avg(); image.mulAdd(Vec4(Vec(a), 1), Vec4(Vec(-0.5f*a+0.5f)  , 0), &box);} // if image is monochromatic then we need to transform all RGB together
            else                                     image.mulAdd(Vec4(  r, 1, 1), Vec4(    -0.5f*r+0.5f, 0, 0), &box);
         }else
         if(param.name=="fixTransparent")
         {
            image.transparentToNeighbor(true, param.value.is() ? param.asFlt() : 1);
         }
      }
   }
   bool Project::loadImage(Image &image, C Edit::FileParams &fp, bool clamp, C Image *color, C Image *spec, C Image *bump)C
   {
      if(!fp.name.is()){image.del(); return true;}
      Str  name=fp.name;
      bool lum_to_alpha=false;
      UID  image_id;
      if(name[0]=='<')
      {
         if( name=="<color>"                       && color)color->copyTry(image);else
         if((name=="<spec>" || name=="<specular>") && spec )spec ->copyTry(image);else
         if( name=="<bump>"                        && bump )bump ->copyTry(image);else
            image.del();
         goto imported;
      }
      if(DecodeFileName(name, image_id))
      {
         name=editPath(image_id); // if the filename is in UID format then it's ELM_IMAGE
         if(C Elm *image=findElm(image_id))if(C ElmImage *data=image->imageData())lum_to_alpha=data->alphaLum();
      }
      if(ImportImage(image, name, -1, IMAGE_SOFT, 1, true))
      {
      imported:
         if(lum_to_alpha)image.alphaFromBrightness().divRgbByAlpha();
         TransformImage(image, ConstCast(fp.params), clamp);
         return true;
      }
      image.del(); return false;
   }
   bool Project::loadImages(Image &image, C Str &src, bool clamp, C Color &background, C Image *color, C Image *spec, C Image *bump)C
   {
      Mems<Edit::FileParams> fps=Edit::FileParams::Decode(src);
      if(!fps.elms()){image.del(); return true;}
      if( fps.elms()==1 && !fps[0].findParam("position") && !fps[0].findParam("pos"))return loadImage(image, fps[0], clamp, color, spec, bump); // can load as a single image only if doesn't have position specified
      image.del();
      bool ok=true, hp=false;
      Image single;
       REPA(fps)if(C TextParam *p=fps[i].findParam("mode"))if(p->value!="set"){hp=true; break;}
      FREPA(fps)if(loadImage(single, fps[i], clamp, color, spec, bump)) // process in order
      {
         VecI2 pos=0; {C TextParam *p=fps[i].findParam("position"); if(!p)p=fps[i].findParam("pos"); if(p)pos=p->asVecI2();}
         VecI2 size=single.size()+pos;
         APPLY_MODE mode=APPLY_SET; if(C TextParam *p=fps[i].findParam("mode"))
         {
            if(p->value=="blend"                                              )mode=APPLY_BLEND;else
            if(p->value=="blendPremultiplied" || p->value=="premultipliedBlend")mode=APPLY_BLEND_PREMUL;else
            if(p->value=="mul"                                                )mode=APPLY_MUL;else
            if(p->value=="div"                                                )mode=APPLY_DIV;else
            if(p->value=="add"                                                )mode=APPLY_ADD;else
            if(p->value=="sub"                                                )mode=APPLY_SUB;else
            if(p->value=="max"                                                )mode=APPLY_MAX;
         }
         if(size.x>image.w() || size.y>image.h()) // make room for 'single', do this even if 'single' doesn't exist, because we may have 'pos' specified
         {
            VecI2 old_size=image.size();
            if(image.is())image.crop(image, 0, 0, Max(image.w(), size.x), Max(image.h(), size.y));
            else         {image.createSoftTry(size.x, size.y, 1, (hp || single.highPrecision()) ? IMAGE_F32_4 : IMAGE_R8G8B8A8); image.clear();}
            if(background!=TRANSPARENT)
               REPD(y, image.h())
               REPD(x, image.w())if(x>=old_size.x || y>=old_size.y)image.color(x, y, background);
         }
         if(single.is())
         {
            // put 'single' into image
            if(single.highPrecision())MakeHighPrec(image);
            REPD(y, single.h())
            REPD(x, single.w())
            {
               Vec4 s=single.colorF(x, y);
               if(mode==APPLY_SET)
               {
                  image.colorF(x+pos.x, y+pos.y, s);
               }else
               {
                  Vec4 d=image.colorF(x+pos.x, y+pos.y);
                  switch(mode)
                  {
                     case APPLY_BLEND       : d =             Blend(d, s); break;
                     case APPLY_BLEND_PREMUL: d =PremultipliedBlend(d, s); break;
                     case APPLY_MUL         : d*=s; break;
                     case APPLY_DIV         : d/=s; break;
                     case APPLY_ADD         : d+=s; break;
                     case APPLY_SUB         : d-=s; break;
                     case APPLY_MAX         : d=Max(s, d); break;
                  }
                  image.colorF(x+pos.x, y+pos.y, d);
               }
            }
         }else TransformImage(image, fps[i].params, clamp); // if this 'single' is empty, then apply params to the entire 'image', so we can process entire atlas
      }else ok=false;
      return ok;
   }
   void Project::savedGame(Elm &elm, C Str &name) {elm.file_size=FSize(name);}
   void Project::savedGame(Elm &elm             ) {savedGame(elm, gamePath(elm));}
   void Project::makeGameVer(Elm &elm, File *file)
   {
      if(IsServer)return; // this doesn't need to be performed on the server
      Str file_edit_path=editPath(elm), file_game_path=gamePath(elm);
      switch(elm.type)
      {
         case ELM_MTRL:
         {
            EditMaterial edit; edit.load(file_edit_path);
                Material game; edit.copyTo(game, T); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_WATER_MTRL:
         {
            EditWaterMtrl edit; edit.load(file_edit_path);
                WaterMtrl game; edit.copyTo(game, T); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_PHYS_MTRL:
         {
            EditPhysMtrl edit; edit.load(file_edit_path);
                PhysMtrl game; edit.copyTo(game); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_MESH:
         {
            ElmMesh *data=elm.meshData(); Matrix matrix; if(data)matrix=data->transform();else matrix.identity(); Skeleton *body_skel; getMeshSkels(data, null, &body_skel); Enum *draw_group=(data ? getEnum(data->draw_group_id) : null);
            Mesh mesh; if(file)mesh.load(*file, game_path);else Load(mesh, file_edit_path, game_path); EditToGameMesh(mesh, mesh, body_skel, draw_group, &matrix);
            Save(mesh, file_game_path); if(data)data->from(mesh); savedGame(elm, file_game_path);
         }break;

         case ELM_ENUM:
         {
            EditEnums edit; if(file)edit.load(*file);else edit.load(file_edit_path);
                Enum  game; edit.copyTo(game, elm.name); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_IMAGE:
         {
            Image image; image.ImportTry(file_edit_path); EditToGameImage(image, image, *elm.imageData());
            Save( image, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_IMAGE_ATLAS: break; // not done here
         case ELM_ICON       : break; // not done here
         case ELM_FONT       : break; // not done here

         case ELM_PANEL_IMAGE : // not done here
         {
          /*EditPanelImage edit; edit.load(file_edit_path);
                PanelImage game; edit.make(game); Save(game, file_game_path); */
         }break;

         case ELM_OBJ:
         {
            ElmObj  * obj_data=(                elm. objData()       ); Elm *mesh_elm=(obj_data ? findElm(obj_data->mesh_id) : null);
            ElmMesh *mesh_data=(mesh_elm ? mesh_elm->meshData() : null);

            // set mesh/phys only if they're not empty
            bool override_mesh=( obj_data &&  obj_data->mesh_id.valid()); if(override_mesh)if(    MeshPtr mesh=gamePath( obj_data->mesh_id))override_mesh&=OverrideMeshSkel(mesh(), mesh->skeleton());
            bool override_phys=(mesh_data && mesh_data->phys_id.valid()); if(override_phys)if(PhysBodyPtr phys=gamePath(mesh_data->phys_id))override_phys&=OverridePhys    (phys());

            EditObjectPtr edit=file_edit_path;
                Object    game; edit->copyTo(game, T, true, override_mesh ? &obj_data->mesh_id : null, override_phys ? &mesh_data->phys_id : null); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_OBJ_CLASS:
         {
            EditObjectPtr edit=file_edit_path;
                Object    game; edit->copyTo(game, T, false, null, null); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_TEXT_STYLE:
         {
            EditTextStyle edit; edit.load(file_edit_path);
                TextStyle game; edit.copyTo(game, T); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_GUI_SKIN:
         {
            EditGuiSkin edit; edit.load(file_edit_path);
                GuiSkin game; edit.copyTo(game, T); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_PANEL:
         {
            EditPanel edit; edit.load(file_edit_path);
                Panel game; edit.copyTo(game, T); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_ENV:
         {
            EditEnv         edit; edit.load(file_edit_path);
                Environment game; edit.copyTo(game, T); Save(game, file_game_path); savedGame(elm, file_game_path);
         }break;

         case ELM_WORLD: if(ElmWorld *data=elm.worldData())if(data->valid())
         {
            Str world_edit_path, world_game_path;
            createWorldPaths(elm.id);
            if(getWorldPaths(elm.id, world_edit_path, world_game_path))
            {
               Game::WorldSettings settings; data->copyTo(settings, T); Save(settings, world_game_path+"Settings", game_path);
            }
         }break;

         case ELM_MINI_MAP: break; // not done here
      }
   }
   void Project::removeOrphanedElms()
   {
      Memt<UID> used;
      TimeStamp time; time.getUTC();

      // get all used meshes
      FREPA(elms) // go forward because most likely 'used' ID's will be increasing
      {
       C Elm &elm=elms[i]; if(C ElmObj *obj_data=elm.objData())used.binaryInclude(obj_data->mesh_id, Compare);
      }
      // remove unused meshes
      REPA(elms)
      {
         Elm &elm=elms[i]; if(elm.type==ELM_MESH && !elm.removed() && !used.binaryHas(elm.id, Compare))elm.setRemoved(true, time);
      }

      used.clear();
      
      // get all used skel
      FREPA(elms) // go forward because most likely 'used' ID's will be increasing
      {
       C Elm &elm=elms[i]; if(C ElmMesh *mesh_data=elm.meshData())if(mesh_data->skel_id.valid())used.binaryInclude(mesh_data->skel_id, Compare);
      }
      // remove unused skel
      REPA(elms)
      {
         Elm &elm=elms[i]; if(elm.type==ELM_SKEL && !elm.removed() && !used.binaryHas(elm.id, Compare))elm.setRemoved(true, time);
      }

      used.clear();
      
      // get all used phys
      FREPA(elms) // go forward because most likely 'used' ID's will be increasing
      {
       C Elm &elm=elms[i]; if(C ElmMesh *mesh_data=elm.meshData())if(mesh_data->phys_id.valid())used.binaryInclude(mesh_data->phys_id, Compare);
      }
      // remove unused phys
      REPA(elms)
      {
         Elm &elm=elms[i]; if(elm.type==ELM_PHYS && !elm.removed() && !used.binaryHas(elm.id, Compare))elm.setRemoved(true, time);
      }
   }
   void Project::eraseElm(C UID &elm_id)
   {
      if(Elm *elm=findElm(elm_id))
      {
         if(elm->type==ELM_WORLD) // worlds need to have their 'WorldVer' removed from the cache
         {
            CacheLock cl(world_vers); REPA(world_vers)if(world_vers.lockedData(i).world_id==elm_id)world_vers.removeData(&world_vers.lockedData(i));
         }
         if(elm->type==ELM_MINI_MAP) // mini maps need to have their 'MiniMapVer' removed from the cache
         {
            CacheLock cl(mini_map_vers); REPA(mini_map_vers)if(mini_map_vers.lockedData(i).mini_map_id==elm_id)mini_map_vers.removeData(&mini_map_vers.lockedData(i));
         }
         // elms.removeData(elm, true); don't remove element here, in case some 'eraseElm' function uses 'hierarchy' or 'elms' containers (for example CodeEditor.removeSource uses findSource and SourceLoc.setID which uses sourceFullName which uses hierarchy and elms)
         if(elm->type==ELM_CODE)
         {
            FDelFile(codePath    (elm_id));
            FDelFile(codeBasePath(elm_id));
         }else
         {
            FDel(editPath(elm_id)); // delete edit version
            FDel(gamePath(elm_id)); // delete game version
            REP(FormatSuffixElms)FDel(formatPath(elm_id, FormatSuffixes[i])); // delete all possible formats (these can be folder, for example Mini Maps)
         }
      }
   }
   bool Project::eraseElms(C MemPtr<UID> &elm_ids)
   {
      if(elm_ids.elms())
      {
         FREPA(elm_ids)eraseElm(elm_ids[i]); // process removal of all elements
         FREPA(elm_ids)elms.removeData(findElm(elm_ids[i]), true); // remove from container
         return true;
      }
      return false;
   }
   void Project::eraseTexFormats(C UID &tex_id)
   {
      REPD(format, FormatSuffixElms+1) // process 1 extra format as empty
      REPD(downsize, MaxMaterialDownsize)
         FDelFile(texFormatPath(tex_id, format ? FormatSuffixes[format-1] : null, downsize));
   }
   bool Project::eraseTex(C UID &_tex_id)
   {
      UID tex_id=_tex_id; // copy this to a temp var in case '_tex_id' actually belongs to 'texs' container which we're removing from below, which would make that UID invalid !!
      if(texs.binaryExclude(tex_id, Compare)) // if found and removed
      {
         FDelFile(texPath(tex_id)); // delete the texture
         eraseTexFormats(tex_id);
         return true;
      }
      return false;
   }
   bool Project::eraseTexs()
   {
      bool erased=false;

      // erase regular textures
      Memt<UID> used; getTextures(used);
      REPA(texs)if(!used.binaryHas(texs[i], Compare))erased|=eraseTex(texs[i]); // go from the end because of removal

      // erase dynamic textures
      used.clear();
      REPA(elms)if(ElmMaterial *mtrl_data=elms[i].mtrlData())
         if(mtrl_data->base_0_tex.valid() && mtrl_data->base_1_tex.valid())
            used.binaryInclude(MergedBaseTexturesID(mtrl_data->base_0_tex, mtrl_data->base_1_tex), Compare);
      for(FileFind ff(temp_tex_dynamic_path); ff(); )
      {
         bool tex_used=false;
         UID  tex_id; if(DecodeFileName(ff.name, tex_id))
         {
            tex_used=used.binaryHas(tex_id, Compare);
            if(!tex_used)eraseTexFormats(tex_id);
         }
         if(!tex_used)FDelFile(ff.pathName());
      }

      return erased;
   }
   void Project::eraseWorldAreaObjs(C UID &world_id, C VecI2 &area_xy)
   {
      Str    name=editAreaPath(world_id, area_xy);
      Chunks chunks; chunks.load(name, WorldAreaSync);
      if(Chunk *chunk=chunks.findChunk("Object"))
      {
         Memc<ObjData> objs;
         if(LoadEditObject(chunk->ver, File().readMem(chunk->data(), chunk->elms()), objs, edit_path)) // load edit objects
         {
            REPA(objs)if(objs[i].removed)objs.remove(i, true);
            SaveEditObject(chunks, objs, edit_path);
            chunks.save(name, WorldAreaSync);
         }
      }
   }
   bool Project::eraseWorldObjs()
   {
      bool        erased=false;
      CacheLock   lock(world_vers);
      Memc<VecI2> areas;
      REPA(world_vers) // iterate all worlds
      {
         WorldVer &world_ver=world_vers.lockedData(i);
         MapLock   ml(world_ver.obj);
         REPA(world_ver.obj) // iterate all objects
         {
            ObjVer &obj_ver=world_ver.obj.lockedData(i);
            if(obj_ver.removed()) // if object is removed
            {
               world_ver.setChanged();
               areas.binaryInclude(obj_ver.area_xy, Compare); // mark for processing
               world_ver.obj.remove(i); // we're completely erasing so remove from obj database too !! after this call don't operate on 'obj_ver' as it was just removed !!
            }
         }
         if(areas.elms())
         {
            erased=true;
            REPA(areas)eraseWorldAreaObjs(world_ver.world_id, areas[i]);
            areas.clear();
         }
      }
      return erased;
   }
   void Project::quickUpdateVersion(int ver) // this is called inside 'load', it occurs when opening projects and loading from EsenthelProject file, we can't modify files here !!
   {
      if(ver<0)return;

      if(ver<=21) // there was a bug in 'duplicate' which did not set 'mesh_id' correctly, so fix it
         REPA(elms)
      {
         Elm &elm=elms[i]; if(ElmMesh *mesh_data=elm.meshData())
         {
            if(Elm *skel_elm=findElm(mesh_data->skel_id))if(ElmSkel *skel_data=skel_elm->skelData())skel_data->mesh_id=elm.id;
            if(Elm *phys_elm=findElm(mesh_data->phys_id))if(ElmPhys *phys_data=phys_elm->physData())phys_data->mesh_id=elm.id;
         }
      }
      if(ver<=45) // 'ElmMesh' didn't have 'obj_id'
         REPA(elms)
      {
         Elm &mesh_elm=elms[i]; if(ElmMesh *mesh_data=mesh_elm.meshData())REPA(elms)if(ElmObj *obj_data=elms[i].objData())if(obj_data->mesh_id==mesh_elm.id)
         {
            mesh_data->obj_id=elms[i].id; break;
         }
      }

      if(ver<=27) // project version 27 and below
      {
         // mtrl base 1 and detail textures need to have channels swapped
         texs_update.clear();
         REPA(elms)
         {
          C Elm &elm=elms[i]; switch(elm.type)
            {
               case ELM_MTRL: if(C ElmMaterial *mtrl_data=elm.mtrlData())
               {
                  if(mtrl_data->base_1_tex.valid() && texs.binaryHas(mtrl_data->base_1_tex, Compare))texs_update.binaryInclude(mtrl_data->base_1_tex, Compare);
                  if(mtrl_data->detail_tex.valid() && texs.binaryHas(mtrl_data->detail_tex, Compare))texs_update.binaryInclude(mtrl_data->detail_tex, Compare);
               }break;

               case ELM_WATER_MTRL: if(C ElmWaterMtrl *water_mtrl_data=elm.waterMtrlData())
               {
                  if(water_mtrl_data->base_1_tex.valid() && texs.binaryHas(water_mtrl_data->base_1_tex, Compare))texs_update.binaryInclude(water_mtrl_data->base_1_tex, Compare);
               }break;
            }
         }

         // src file was encoded differently
         REPA(elms)
         {
            Elm &elm=elms[i]; if(elm.data)
            {
               Mems<Edit::FileParams> files=_DecodeFileParams(elm.srcFile());
               if(elm.type==ELM_ANIM || elm.type==ELM_MTRL)FREPA(files)
               {
                  Edit::FileParams &file=files[i];
                  file.name.tailSlash(false); // DAE has empty animation names, so this could have been "file.dae/"
                  Str path=GetPath(file.name);
                  if(ExtType(GetExt(path))==EXT_MESH) // if file was stored as "file.ext/inner_name", for example "Character.fbx/run" run animation inside fbx file
                  {
                     file.getParam("name").value=GetBase(file.name);
                     file.name=path;
                  }
               }
               elm.data->src_file=Edit::FileParams::Encode(files);
            }
         }
      }
      if(ver<=46) // 46 and below used '|' as separator
      {
         REPA(elms)
         {
            Elm &elm=elms[i]; if(elm.type!=ELM_ANIM && elm.data)elm.data->src_file.replace('|', '\n'); // keep anim as | because some animations can have | in the name "Para.fbx?name=Para|SitLoop"
         }
      }

   #if 0 // force upload of skeletons
      REPA(elms)
      {
         Elm &elm=elms[i]; if(ElmSkel *skel_data=elm.skelData())if(skel_data.ver)
         {
            skel_data.newVer();
            skel_data.file_time.getUTC();
         }
      }
   #endif

   #if 0 // fix transforms (this can be executed if copied Project Data file from the Server version to the Client version)
      don't use as it may be broken (skeletons not transformed properly)
      REPA(elms){Elm &elm=elms[i]; if(ElmSkel *skel_data=elm.skelData())if(Elm *mesh_elm=findElm(skel_data.mesh_id))if(ElmMesh *mesh_data=mesh_elm.meshData())skel_data.transform=mesh_data.transform;} // skeletons  first
      REPA(elms){Elm &elm=elms[i]; if(ElmAnim *anim_data=elm.animData())if(Elm *skel_elm=findElm(anim_data.skel_id))if(ElmSkel *skel_data=skel_elm.skelData())anim_data.transform=skel_data.transform;} // animations next
   #endif

   #if 0 // test transforms
      REPA(elms)
      {
         Elm &elm=elms[i]; switch(elm.type)
         {
            case ELM_ANIM: if(ElmAnim *anim_data=elm.animData())if(Elm *skel_elm=findElm(anim_data.skel_id))if(ElmSkel *skel_data=skel_elm.skelData())if(anim_data.transform!=skel_data.transform)LogN(S+"Different Anim-Skel transform:\n"+Project.elmFullName(&elm)+'\n'+anim_data.transform.asText()+'\n'+skel_data.transform.asText()); break;
            case ELM_SKEL: if(ElmSkel *skel_data=elm.skelData())if(Elm *mesh_elm=findElm(skel_data.mesh_id))if(ElmMesh *mesh_data=mesh_elm.meshData())if(skel_data.transform!=mesh_data.transform)LogN(S+"Different Skel-Mesh transform:\n"+Project.elmFullName(&elm)+'\n'+skel_data.transform.asText()+'\n'+mesh_data.transform.asText()); break;
         }
      }
   #endif

   #if 0 // test mesh->obj references count
      REPA(elms)
      {
         Elm &elm=elms[i]; switch(elm.type)
         {
            case ELM_MESH:
            {
               int count=0; REPA(elms)
               {
                  Elm &obj_elm=elms[i]; if(ElmObj *obj_data=obj_elm.objData())if(obj_data.mesh_id==elm.id)count++;
               }
               if(count!=1)Exit(S+elmFullName(&elm)+" is used by "+count+" Objects");
            }break;
         }
      }
   #endif
   }
   void Project::updateVersion(int ver, bool this_project, C MemPtr<UID> &elm_ids) // if 'elm_ids' is null, then all elements are processed, this is called inside 'open' or after copying elements to project
   {
      if(ver<0)return;

      File f, temp, *src;
      bool compress_skel=(IsServer && ElmCompressable(ELM_SKEL)),
           compress_anim=(IsServer && ElmCompressable(ELM_ANIM));

      // first process compression changes to convert old to latest compression state
      if(IsServer && ver<=27 && ElmCompressable(ELM_SKEL)) // ver 27 and below kept skeleton uncompressed on the server, and if we want it to be compressed now, then compress it:
      {
         REPA(elms)if(elms[i].type==ELM_SKEL)
         {
            Str path=gamePath(elms[i]); if(f.readTry(path) && Compress(f, temp.writeMem(), ClientNetworkCompression, ClientNetworkCompressionLevel))
            {
               f.del(); temp.pos(0); SafeOverwrite(temp, path);
            }
         }
      }

      if(ver<=48)
      {
         // have to update animations first with old skeleton bone types
         REP(elm_ids ? elm_ids.elms() : elms.elms())
            if(Elm *elm=(elm_ids ? findElm(elm_ids[i]) : &elms[i]))
               switch(elm->type)
         {
            case ELM_CODE: if(ver<=48) // in ver 48 and below, codes were stored in "Edit" folder without extension
            {
               Code code; if(code.load(editPath(*elm)))
               {
                  if(SaveCode(code.current, codePath(*elm)))FDelFile(editPath(*elm));
                  if(!IsServer)
                  {
                     if(code.base.is())SaveCode(code.base, codeBasePath(*elm));
                     else                         FDelFile(codeBasePath(*elm));
                  }
               }
            }break;

            case ELM_ANIM: if(ver<=47)if(ElmAnim *anim_data=elm->animData())
            {
               src=&f; if(src->readTry(gamePath(*elm)))
               {
                  if(compress_anim)if(Decompress(*src, temp, true)){src=&temp; src->pos(0);}else src=null;
                  if(src)
                  {
                     Animation anim; if(anim.load(*src))
                     {
                        // load skeleton
                        if(Elm *skel_elm=findElm(anim_data->skel_id))
                        {
                           src=&f; if(src->readTry(gamePath(*skel_elm)))
                           {
                              if(compress_skel)if(Decompress(*src, temp, true)){src=&temp; src->pos(0);}else src=null;
                              if(src)
                              {
                                 Skeleton skel; if(skel.load(*src))
                                 {
                                    if(ver<=47)
                                    {
                                       anim.setBoneNameTypeIndexesFromSkeleton(skel); // Warning: be careful with calling this method when 'Skeleton.setBoneTypes' was called because it could mess up links if 'skel' bone types indexes have changed due to some changes in how 'setBoneTypes' works, so we call it as the first thing
                                       skel.setBoneTypes(); // !! if we do this here, then we have to do it for skeletons below with same 'ver' check !! make sure we have the latest types
                                       anim.setBoneTypeIndexesFromSkeleton(skel); // after we've set bone names, we can update bone types to latest skel data
                                    }
                                 }
                              }
                           }
                        }
                        // save animation
                        anim.save(f.writeMem()); src=&f; src->pos(0);
                        if(compress_anim)if(Compress(*src, temp.writeMem(), ClientNetworkCompression, ClientNetworkCompressionLevel)){src=&temp; src->pos(0);}else src=null;
                        if(src)SafeOverwrite(*src, gamePath(*elm));
                     }
                  }
               }
            }break;
         }

         REP(elm_ids ? elm_ids.elms() : elms.elms())
            if(Elm *elm=(elm_ids ? findElm(elm_ids[i]) : &elms[i]))
               switch(elm->type)
         {
            case ELM_SKEL: if(ver<=47)
            {
               src=&f; if(src->readTry(gamePath(*elm)))
               {
                  if(compress_skel)if(Decompress(*src, temp, true)){src=&temp; src->pos(0);}else continue;
                  Skeleton skel; if(skel.load(*src))
                  {
                     if(ver<=47)skel.setBoneTypes(); // !! we can call it only if we've adjusted animations to new bone types above with the same 'ver' check !!

                     skel.save(f.writeMem()); src=&f; src->pos(0);
                     if(compress_skel)if(Compress(*src, temp.writeMem(), ClientNetworkCompression, ClientNetworkCompressionLevel)){src=&temp; src->pos(0);}else src=null;
                     if(src)SafeOverwrite(*src, gamePath(*elm));

                     if(ver<=21 && !FExistSystem(editPath(*elm))) // if there's no skeleton edit version
                     {
                        // create skeleton edit version
                        if(ElmSkel *skel_data=elm->skelData())skel.transform(~skel_data->transform()); // !! transform after saving game version, and before creating edit version !!
                        EditSkeleton edit; edit.create(skel, null);
                        edit.save(f.writeMem()); src=&f; src->pos(0);
                        if(compress_skel)if(Compress(*src, temp.writeMem(), ClientNetworkCompression, ClientNetworkCompressionLevel)){src=&temp; src->pos(0);}else src=null;
                        if(src)SafeOverwrite(*src, editPath(*elm));
                     }
                  }
               }
            }break;

            case ELM_MTRL: if(ver<=46)if(ElmMaterial *mtrl_data=elm->mtrlData()) // ver 46 and below used '|' as separator, and didn't have some 'ElmMaterial' members set
            {
               EditMaterial edit; if(edit.load(editPath(elm->id)))
               {
                  if(ver<=46)
                  {
                     edit.   color_map  .replace('|', '\n');
                     edit.   alpha_map  .replace('|', '\n');
                     edit.    bump_map  .replace('|', '\n');
                     edit.  normal_map  .replace('|', '\n');
                     edit.specular_map  .replace('|', '\n');
                     edit.    glow_map  .replace('|', '\n');
                     edit.detail_color  .replace('|', '\n');
                     edit.detail_bump   .replace('|', '\n');
                     edit.detail_normal .replace('|', '\n');
                     edit.     macro_map.replace('|', '\n');
                     edit.reflection_map.replace('|', '\n');
                     edit.     light_map.replace('|', '\n');
                     Save(edit, editPath(elm->id));
                     mtrl_data->from(edit);
                  }
               }
            }break;
         }
      }

   #if 0 // reset game meshes
      {
         int c=0;
         REPA(elms)if(elms[i].type==ELM_MESH)
         {
            if(!((c++)&0xFF))
            {
               Materials.delayRemoveNow();
               Images   .delayRemoveNow();
            }
            makeGameVer(elms[i]);
         }
      }
   #endif
   #if 0 // reset mesh bone maps
      {
         int c=0;
         REPA(elms)if(elms[i].type==ELM_MESH)if(ElmMesh *mesh_data=elms[i].meshData())if(mesh_data.skel_id.valid())
         {
            Skeleton skel; Mesh mesh; if(Load(mesh, editPath(elms[i]), game_path))if(skel.load(gamePath(mesh_data.skel_id)))
            {
               mesh.skeleton(&skel, true).skeleton(null);
               Save(mesh, editPath(elms[i]), game_path);
               mesh_data.newVer();
               mesh_data.file_time.getUTC();
               makeGameVer(elms[i]);
               if(!((c++)&0xFF))
               {
                  Materials.delayRemoveNow();
                  Images   .delayRemoveNow();
               }
            }
         }
      }
   #endif
   #if 0 // reset game skeletons
      {
         REPA(elms)if(elms[i].type==ELM_SKEL)
         {
            Skeleton skel; if(skel.load(gamePath(elms[i])))Save(skel, gamePath(elms[i]));
         }
      }
   #endif
   #if 0 // test anim loop
      {
         REPA(elms)if(elms[i].type==ELM_ANIM)
         {
            Animation anim; if(!anim.load(gamePath(elms[i])))Exit("anim load"); if(anim.loop()!=elms[i].animData().loop())LogN(S+anim.loop()+' '+Project.elmFullName(&elms[i]));
         }
      }
   #endif
   }
   void Project::textData(bool on)
   {
      if(text_data!=on)
      {
         text_data=on;
         if(save())
         {
            if(!text_data && path.is())FDelFile(path+"Data.txt"); // if disabled text_data then delete the text file
         }
      }
   }
   void Project::meshSetAutoTanBin(Elm &elm, C MaterialPtr &material)
   {
      if(elm.type==ELM_MESH)
      {
         Mesh mesh; if(mesh.load(gamePath(elm.id)))
         {
            bool changed=false;
            REP(mesh.lods())
            {
               MeshLod &lod=mesh.lod(i); REPA(lod)
               {
                  MeshPart &part=lod.parts[i]; if(!material || HasMaterial(part, material))
                  {
                     uint flag =((part.base.flag()|part.render.flag())&VTX_TAN_BIN); part.setAutoTanBin();
                     if(  flag!=((part.base.flag()|part.render.flag())&VTX_TAN_BIN))changed=true;
                  }
               }
            }
            if(changed){Save(mesh, gamePath(elm.id)); savedGame(elm);}
         }
      }
   }
   void Project::mtrlSetAutoTanBin(C UID &mtrl_id)
   {
      MaterialPtr material;
      if(mtrl_id.valid())REPA(elms)
      {
         Elm &elm=elms[i];
         if(C ElmMesh *data=elm.meshData())
            if(data->mtrl_ids.binaryHas(mtrl_id, Compare))
         {
            if(!material)material=gamePath(mtrl_id); // load only when needed
            meshSetAutoTanBin(elm, material);
         }
      }
   }
   void Project::animTransformChanged(Elm &elm_anim)
   {
   }
   void Project::setAnimTransform(Elm &elm_anim)
   {
      if(ElmAnim *anim_data=elm_anim.animData())
         if(Elm *skel_elm=findElm(anim_data->skel_id))
            if(ElmSkel *skel_data=skel_elm->skelData())
               if(anim_data->transform!=skel_data->transform)
                  if(Animation *anim=Animations.get(gamePath(elm_anim.id)))
                     if(Skeleton *skel=Skeletons.get(gamePath(skel_elm->id)))
                        if(!anim->bones.elms() || skel->is()) // if there are 'anim.bones' then process only if the skeleton is valid (known bones, because 'Animation.transform' relies on correct bone information)
      {
         anim->transform(GetTransform(anim_data->transform(), skel_data->transform()), *skel);
         Save(*anim, gamePath(elm_anim.id)); savedGame(elm_anim);
         anim_data->transform=skel_data->transform;
         animTransformChanged(elm_anim);
      }
   }
   void Project::setPhysParams(Elm &phys_elm)
   {
      if(ElmPhys *phys_data=phys_elm.physData())
         if(PhysBodyPtr phys=PhysBodyPtr().get(gamePath(phys_elm.id)))
      {
         PhysMtrl *mtrl=PhysMtrls(gamePath(phys_data->mtrl_id));
         {CacheLock cl(PhysBodies); phys->density=phys_data->density; phys->material=mtrl;}
         Save(*phys, gamePath(phys_elm.id)); savedGame(phys_elm);
      }
   }
   void Project::skelTransformChanged(C UID &skel_id)
   {
      if(skel_id.valid())REPA(elms)
      {
         Elm &elm=elms[i]; if(ElmAnim *data=elm.animData())if(data->skel_id==skel_id)setAnimTransform(elm); // transform all animations linked with this skeleton
      }
   }
   void Project::skelChanged(Elm &skel_elm)
   {
      if(skel_elm.type==ELM_SKEL)
      {
         REPA(elms)
         {
            Elm &elm=elms[i];
            bool uses_skel=false;
            switch(elm.type)
            {
               case ELM_MESH:
               {
                  UID body_skel; getMeshSkels(elm.meshData(), null, &body_skel);
                  if( body_skel==skel_elm.id)uses_skel=true; // if mesh uses the skeleton
               }break;

             /*case ELM_ANIM: if(C ElmAnim *anim_data=elm.animData())
               {
                  if(anim_data.skel_id==skel_elm.id)uses_skel=true;
               }break;*/
            }
            if(uses_skel)makeGameVer(elm);
         }

         // because animation transforms will be adjusted only when skeleton is available ('setAnimTransform': "if(!anim.bones.elms() || skel.is())") then upon skeleton being changed it's possible we've received it from the server and now we need to transform the animations
         skelTransformChanged(skel_elm.id);
      }
   }
   void Project::setSkelTransform(Elm &skel_elm)
   {
      if(ElmSkel *skel_data=skel_elm.skelData())
         if(Elm *mesh_elm=findElm(skel_data->mesh_id))
            if(ElmMesh *mesh_data=mesh_elm->meshData())
               if(skel_data->transform!=mesh_data->transform)
                  if(Skeleton *skel=Skeletons.get(gamePath(skel_elm.id)))
      {
         Matrix matrix=GetTransform(skel_data->transform(), mesh_data->transform());
         skel->transform(matrix);
         // edit version doesn't need to be changed since it's always in Identity
         Save(*skel, gamePath(skel_elm.id)); savedGame(skel_elm);
         skel_data->transform=mesh_data->transform;
         skelTransformChanged(skel_elm.id);
      }
   }
   bool Project::setPhysTransform(Elm &phys_elm)
   {
      if(ElmPhys *phys_data=phys_elm.physData())
         if(Elm *mesh_elm=findElm(phys_data->mesh_id))
            if(ElmMesh *mesh_data=mesh_elm->meshData())
               if(phys_data->transform!=mesh_data->transform)
                  if(PhysBodyPtr phys=PhysBodyPtr().get(gamePath(phys_elm.id)))
      {
         Matrix matrix=GetTransform(phys_data->transform(), mesh_data->transform());
         {CacheLock cl(PhysBodies); phys->transform(matrix);}
         Save(*phys, gamePath(phys_elm.id)); savedGame(phys_elm);
         phys_data->transform=mesh_data->transform;
         phys_data->from(*phys);
         physChanged(phys_elm);
         return true;
      }
      return false;
   }
   void Project::meshTransformChanged(Elm &mesh_elm, bool body_changed) {Memt<UID>  processed; meshTransformChanged(mesh_elm, body_changed, processed);}
   void Project::meshTransformChanged(Elm &mesh_elm, bool body_changed,        Memt<UID> &processed) // can't do default param "=Memt<UID>()" on Mac
   {
      if(processed.binaryInclude(mesh_elm.id, Compare)) // to avoid potential infinite loops
      if(ElmMesh *mesh_data=mesh_elm.meshData())
      {
         // force transform from body
         bool transform_changed=false;
         if(Elm *body_elm=findElm(mesh_data->body_id))
         if(ElmMesh *body_mesh_data=body_elm->meshData())if(mesh_data->transform!=body_mesh_data->transform)
         {
            mesh_data->transform=body_mesh_data->transform;
            transform_changed=true;
         }
         if(body_changed || transform_changed)makeGameVer(mesh_elm); // have to make game mesh if body changes (because we will have a new target skeleton) or transform changes

         // set skel+phys transforms according to this transform
         if(Elm *skel_elm=findElm(mesh_data->skel_id))setSkelTransform(*skel_elm);
         if(Elm *phys_elm=findElm(mesh_data->phys_id))setPhysTransform(*phys_elm);

         // set cloths that use this mesh as a body
         REPA(elms)
         {
            Elm &elm=elms[i]; if(ElmMesh *cloth_mesh_data=elm.meshData())if(cloth_mesh_data->body_id==mesh_elm.id)meshTransformChanged(elm, body_changed, processed);
         }

         // notify of change
         meshChanged(mesh_elm);
      }
   }
   void Project::rebuildEmbedForObj(C    UID  &obj ) {Memt<UID> objs, exts; objs.add(obj); getExtendedObjs(objs, exts); rebuildEmbedForObjs(exts);}
   void Project::rebuildEmbedForObjs(Memt<UID> &objs)
   {
      // verify embed for all objects based on 'objs'
      REPA(world_vers)
      {
         WorldVer &world_ver=world_vers.lockedData(i);
         REPA(world_ver.obj)
         {
            ObjVer &obj=world_ver.obj.lockedData(i);
            if(!obj.removed() && objs.binaryHas(obj.elm_obj_id, Compare)) // if object exists and its base is contained in 'objs'
            {
             C UID &obj_id=world_ver.obj.lockedKey(i);
               rebuildEmbedObj(obj_id, obj.area_xy, world_ver, true); // 'rebuild_game_area_objs=true' because obj base changed, and world object embed state could be changed
            }
         }
      }
   }
   void Project::rebuildPathsForObj(C    UID  &obj , bool only_not_ovr) {Memt<UID> objs, exts; objs.add(obj); getExtendedObjs(objs, exts); rebuildPathsForObjs(exts, only_not_ovr);}
   void Project::rebuildPathsForObjs(Memt<UID> &objs, bool only_not_ovr) // 'only_not_ovr'=if rebuild paths only for objects not overriding the path mode
   {
      // rebuild paths for all areas which have objects based on 'objs'
      REPA(world_vers)
      {
         WorldVer &world_ver=world_vers.lockedData(i);
         REPA(world_ver.obj)
         {
            ObjVer &obj=world_ver.obj.lockedData(i);
            if(!obj.removed() && objs.binaryHas(obj.elm_obj_id, Compare)) // if object exists and its base is contained in 'objs'
               if(only_not_ovr ? !obj.ovrPath() : obj.path(T)!=OBJ_PATH_IGNORE) // if custom condition met, or want to create paths
            {
             C UID &obj_id=world_ver.obj.lockedKey(i);
               world_ver.rebuildPaths(obj_id, obj.area_xy);
            }
         }
      }
   }
   void Project::verifyPathsForObjClass(C UID &obj_class)
   {
      if(obj_class.valid())
      {
         Memt<UID> objs, exts;
         REPA(elms) // iterate all objects
         {
            Elm &elm=elms[i]; if(ElmObj *data=elm.objData())if(data->base_id==obj_class && !data->ovrPath()) // if they have this base and don't override path
               if(Elm *mesh_elm=findElm(     data->mesh_id))if(ElmMesh *mesh_data=mesh_elm->meshData())                        // if has mesh
               if(Elm *phys_elm=findElm(mesh_data->phys_id))if(ElmPhys *phys_data=phys_elm->physData())if(phys_data->hasBody()) // if has phys
            {
               objs.binaryInclude(elm.id, Compare);
            }
         }
             getExtendedObjs(objs, exts);
         rebuildPathsForObjs(exts, true); // rebuild only for objects that don't override paths (if they override then it means that changing the base doesn't affect their path mode), we must rebuild this also for objects with final path mode set to ignore, in case we've just disabled paths
      }
   }
   void Project::physChanged(Elm &phys)
   {
      Memt<UID> objs, exts; // list of all objects which use this phys body
      if(ElmPhys *phys_data=phys.physData())REPA(elms) // iterate all objects
      {
         Elm &obj=elms[i]; if(ElmObj *obj_data=obj.objData())if(obj_data->mesh_id==phys_data->mesh_id) // check if object mesh matches phys mesh
         {
            objs.binaryInclude(obj.id, Compare);
         }
      }
          getExtendedObjs(objs, exts);
      rebuildEmbedForObjs(exts);
      rebuildPathsForObjs(exts, false);
   }
   void Project::objChanged(Elm &obj)
   {
      {CacheLock cl(EditObjects); REPA(EditObjects)EditObjects.lockedData(i).updateBase(edit_path);}
      {CacheLock cl(    Objects); REPA(    Objects)    Objects.lockedData(i).updateBase();}
   }
   void Project::meshChanged(Elm &mesh) // objects rely on mesh information (physical body "obj -> mesh -> phys"), so rebuild objects
   {
      Memt<UID> objs, exts;
      REPA(elms)
      {
         Elm &obj=elms[i]; if(ElmObj *obj_data=obj.objData())if(obj_data->mesh_id==mesh.id)
         {
            makeGameVer(obj);
             objChanged(obj);
             objs.binaryInclude(obj.id, Compare);
         }
      }
          getExtendedObjs(objs, exts);
      rebuildEmbedForObjs(exts);
   }
   void Project::rebuildWorldAreas(Elm &elm, uint flag)
   {
      if(elm.type==ELM_WORLD && flag)
         if(WorldVer *world_ver=worldVerGet(elm.id))
            REPA(world_ver->areas)world_ver->rebuildAreaNeighbor(world_ver->areas.lockedKey(i), flag);
   }
   void Project::hmDel(C UID &world_id, C VecI2 &area_xy, C TimeStamp *time)
   {
      if(world_id.valid())
         if(WorldVer *world_ver=worldVerGet(world_id))
      {
         createWorldPaths(world_id);

         world_ver->changed=true;
         world_ver->rebuildArea(area_xy, AREA_SYNC_REMOVED);
         world_ver->areas.get(area_xy)->hm_removed_time=(time ? *time : TimeStamp().getUTC());
         RemoveChunk(editAreaPath(world_id, area_xy), "Heightmap", WorldAreaSync);
      }
   }
   Heightmap* Project::hmGet(C UID &world_id, C VecI2 &area_xy, Heightmap &temp)
   {
      if(world_id.valid())
      {
         if(LoadEditHeightmap(editAreaPath(world_id, area_xy), temp, game_path))return &temp;
      }
      return null;
   }
   uint Project::hmUpdate(C UID &world_id, C VecI2 &area_xy, uint area_sync_flag, C AreaVer &src_area, Heightmap &src_hm)
   {
      if(world_id.valid() && area_sync_flag)
         if(WorldVer *world_ver=worldVerRequire(world_id))
      {
         AreaVer  *dest_area=world_ver->areas.get(area_xy);
         Heightmap dest_hm;
         Str       name=editAreaPath(world_id, area_xy);
         Chunks    chunks; chunks.load(name, WorldAreaSync);
         Chunk    *chunk=chunks.findChunk("Heightmap"); if(!chunk)chunk=&chunks.chunks.New();else if(chunk->ver==0)dest_hm.load(File().readMem(chunk->data(), chunk->elms()), game_path);

         if(uint synced=dest_area->sync(src_area, dest_hm, src_hm, area_sync_flag))
         {
            world_ver->changed=true;
            world_ver->rebuildArea(area_xy, synced);
            File temp; dest_hm.save(temp.writeMem(), game_path); temp.pos(0); chunk->create("Heightmap", 0, temp);
            createWorldPaths(world_id);
            chunks.save(name, WorldAreaSync);
            return synced;
         }
      }
      return 0;
   }
   void Project::objGet(C UID &world_id, C VecI2 &area_xy, C Memc<UID> &obj_ids, Memc<ObjData> &objs) // assumes that 'obj_ids' is sorted
   {
      if(world_id.valid())
      {
         Memc<ObjData> file_objs;
         if(LoadEditObject(editAreaPath(world_id, area_xy), file_objs, edit_path)) // load objects from file
            REPA(file_objs) // iterate all file objects
               if(obj_ids.binaryHas(file_objs[i].id, Compare)) // if this is wanted object
                  Swap(objs.New(), file_objs[i]); // move to output container (Swap without remove is okay since we're iterating 'file_objs' from the end)
      }
   }
   Heightmap* Project::hmObjGet(C UID &world_id, C VecI2 &area_xy, Heightmap &temp, Memc<ObjData> &objs, bool get_hm, bool get_objs)
   {
      if(world_id.valid())
      {
         if(LoadEdit(editAreaPath(world_id, area_xy), get_hm ? &temp : null, get_objs ? &objs : null, game_path, edit_path))return get_hm ? &temp : null;
      }
      return null;
   }
   bool Project::syncElm(Elm &elm, Elm &src, File &src_data, File &src_extra, bool sync_long, bool &elm_newer_src, bool &src_newer_elm)
   {
      elm_newer_src=false;
      src_newer_elm=false; // later this needs to be set only if we don't have long data and need to get it

      bool has_file=(sync_long || ElmFileInShort(elm.type));
      uint data_changed=elm.syncData(src), file_changed=0;
      Str  path=basePath(elm), game_path=gamePath(elm);
      if(elm.type==src.type)switch(elm.type)
      {
         case ELM_MTRL:
         {
            ElmMaterial &mtrl_data=*elm.mtrlData(), &src_mtrl_data=*src.mtrlData();
            if(has_file) // has 'EditMaterial'
            {
               EditMaterial mtrl, src_mtrl; mtrl.load(path); src_mtrl.load(src_data);
               if(file_changed=mtrl.sync(src_mtrl)){mtrl_data.from(mtrl); Save(mtrl, path); makeGameVer(elm);}
               if(mtrl_data.equal(src_mtrl_data) && mtrl.equal(src_mtrl))mtrl_data.ver=src_mtrl_data.ver;else if(data_changed || file_changed)mtrl_data.newVer();
               elm_newer_src=mtrl.newer(src_mtrl);
             //src_newer_elm=src_mtrl.newer(mtrl); no need to set because we already have everything
            }
         }break;

         case ELM_WATER_MTRL:
         {
            ElmWaterMtrl &mtrl_data=*elm.waterMtrlData(), &src_mtrl_data=*src.waterMtrlData();
            if(has_file) // has 'EditWaterMtrl'
            {
               EditWaterMtrl mtrl, src_mtrl; mtrl.load(path); src_mtrl.load(src_data);
               if(file_changed=mtrl.sync(src_mtrl)){mtrl_data.from(mtrl); Save(mtrl, path); makeGameVer(elm);}
               if(mtrl_data.equal(src_mtrl_data) && mtrl.equal(src_mtrl))mtrl_data.ver=src_mtrl_data.ver;else if(data_changed || file_changed)mtrl_data.newVer();
               elm_newer_src=mtrl.newer(src_mtrl);
             //src_newer_elm=src_mtrl.newer(mtrl); no need to set because we already have everything
            }
         }break;

         case ELM_PHYS_MTRL:
         {
            ElmPhysMtrl &mtrl_data=*elm.physMtrlData(), &src_mtrl_data=*src.physMtrlData();
            if(has_file) // has 'EditPhysMtrl'
            {
               EditPhysMtrl mtrl, src_mtrl; mtrl.load(path); src_mtrl.load(src_data);
               if(file_changed=mtrl.sync(src_mtrl)){mtrl_data.from(mtrl); Save(mtrl, path); makeGameVer(elm);}
               if(mtrl_data.equal(src_mtrl_data) && mtrl.equal(src_mtrl))mtrl_data.ver=src_mtrl_data.ver;else if(data_changed || file_changed)mtrl_data.newVer();
               elm_newer_src=mtrl.newer(src_mtrl);
             //src_newer_elm=src_mtrl.newer(mtrl); no need to set because we already have everything
            }
         }break;

         case ELM_IMAGE:
         {
            ElmImage &image_data=*elm.imageData(), &src_image_data=*src.imageData();
            if(has_file && image_data.syncFile(src_image_data)){file_changed=true; SafeOverwrite(src_data, path);} // we're saving to Edit so there's no need to call 'SavedImage'
            if((data_changed&CHANGE_AFFECT_FILE) || file_changed)makeGameVer(elm);
         }break;

         case ELM_IMAGE_ATLAS:
         {
            ElmImageAtlas &image_data=*elm.imageAtlasData(), &src_image_data=*src.imageAtlasData();
            if(has_file && image_data.syncFile(src_image_data)){file_changed=true; if(SafeOverwrite(src_data, path)){SavedImageAtlas(path); savedGame(elm, game_path);}}
         }break;

         case ELM_ICON_SETTS:
         {
            if(has_file) // has 'IconSettings'
            {
               ElmIconSetts &icon_data=*elm.iconSettsData(), &src_icon_data=*src.iconSettsData();
               IconSettings  icon, src_icon; icon.load(path); src_icon.load(src_data);
               if(file_changed=icon.sync(src_icon)){icon_data.from(icon); Save(icon, path); makeGameVer(elm);}
               if(icon_data.equal(src_icon_data) && icon.equal(src_icon))icon_data.ver=src_icon_data.ver;else if(data_changed || file_changed)icon_data.newVer();
               elm_newer_src=icon.newer(src_icon);
             //src_newer_elm=src_icon.newer(icon); no need to set because we already have everything
            }
         }break;

         case ELM_ICON:
         {
            if(has_file && elm.iconData()->syncFile(*src.iconData())){file_changed=true; if(SafeOverwrite(src_data, path)){SavedImage(path); savedGame(elm, game_path);}}
         }break;

         case ELM_FONT: // for fonts we first send short: data+edit (ElmFont+EditFont), if EditFont is newer, then we need to request long: data+edit+game
         {
            if(has_file) // has 'EditFont'
            {
                ElmFont &font_data=*elm.fontData(), &src_font_data=*src.fontData();
               EditFont  font, src_font; font.load(path); src_font.load(src_data);
               if(sync_long) // has 'Font'
               {  // we can synchronize 'edit' only if we have 'game', because both of them are tied together and saved at the same time
                  if(file_changed=font.sync(src_font)){font_data.from(font); Save(font, path); if(SafeOverwrite(src_extra, game_path)){SavedFont(game_path); savedGame(elm, game_path);}}
               }else
               {
                  src_newer_elm=src_font.newer(font); // check if we need to request long (game)
               }
               if(font_data.equal(src_font_data) && font.equal(src_font))font_data.ver=src_font_data.ver;else if(data_changed || file_changed)font_data.newVer();
               elm_newer_src=font.newer(src_font);
            }
         }break;

         case ELM_PANEL_IMAGE:
         {
            if(has_file) // has 'EditPanelImage'
            {
                ElmPanelImage &panel_image_data=*elm.panelImageData(), &src_panel_image_data=*src.panelImageData();
               EditPanelImage  panel_image, src_panel_image; panel_image.load(path); src_panel_image.load(src_data);
               if(sync_long) // has 'PanelImage'
               {  // we can synchronize 'edit' only if we have 'game', because both of them are tied together and saved at the same time
                  if(file_changed=panel_image.sync(src_panel_image)){panel_image_data.from(panel_image); Save(panel_image, path); if(SafeOverwrite(src_extra, game_path)){SavedPanelImage(game_path); savedGame(elm, game_path);}}
               }else
               {
                  src_newer_elm=src_panel_image.newer(panel_image); // check if we need to request long (game)
               }
               if(panel_image_data.equal(src_panel_image_data) && panel_image.equal(src_panel_image))panel_image_data.ver=src_panel_image_data.ver;else if(data_changed || file_changed)panel_image_data.newVer();
               elm_newer_src=panel_image.newer(src_panel_image);
            }
         }break;

         case ELM_TEXT_STYLE:
         {
            if(has_file) // has 'EditTextStyle'
            {
                ElmTextStyle &ts_data=*elm.textStyleData(), &src_ts_data=*src.textStyleData();
               EditTextStyle  ts, src_ts; ts.load(path); src_ts.load(src_data);
               if(file_changed=ts.sync(src_ts)){ts_data.from(ts); Save(ts, path); makeGameVer(elm);}
               if(ts_data.equal(src_ts_data) && ts.equal(src_ts))ts_data.ver=src_ts_data.ver;else if(data_changed || file_changed)ts_data.newVer();
               elm_newer_src=ts.newer(src_ts);
             //src_newer_elm=src_ts.newer(ts); no need to set because we already have everything
            }
         }break;

         case ELM_PANEL:
         {
            if(has_file) // has 'EditPanel'
            {
                ElmPanel &panel_data=*elm.panelData(), &src_panel_data=*src.panelData();
               EditPanel  panel, src_panel; panel.load(path); src_panel.load(src_data);
               if(file_changed=panel.sync(src_panel)){panel_data.from(panel); Save(panel, path); makeGameVer(elm);}
               if(panel_data.equal(src_panel_data) && panel.equal(src_panel))panel_data.ver=src_panel_data.ver;else if(data_changed || file_changed)panel_data.newVer();
               elm_newer_src=panel.newer(src_panel);
             //src_newer_elm=src_panel.newer(panel); no need to set because we already have everything
            }
         }break;

         case ELM_GUI_SKIN:
         {
            if(has_file) // has 'EditGuiSkin'
            {
                ElmGuiSkin &gui_skin_data=*elm.guiSkinData(), &src_gui_skin_data=*src.guiSkinData();
               EditGuiSkin  gui_skin, src_gui_skin; gui_skin.load(path); src_gui_skin.load(src_data);
               if(file_changed=gui_skin.sync(src_gui_skin)){gui_skin_data.from(gui_skin); Save(gui_skin, path); makeGameVer(elm);}
               if(gui_skin_data.equal(src_gui_skin_data) && gui_skin.equal(src_gui_skin))gui_skin_data.ver=src_gui_skin_data.ver;else if(data_changed || file_changed)gui_skin_data.newVer();
               elm_newer_src=gui_skin.newer(src_gui_skin);
             //src_newer_elm=src_gui_skin.newer(gui_skin); no need to set because we already have everything
            }
         }break;

         case ELM_ENV:
         {
            if(has_file) // has 'EditEnv'
            {
                ElmEnv &env_data=*elm.envData(), &src_env_data=*src.envData();
               EditEnv  env, src_env; env.load(path); src_env.load(src_data);
               if(file_changed=env.sync(src_env)){env_data.from(env); Save(env, path); makeGameVer(elm);}
               if(env_data.equal(src_env_data) && env.equal(src_env))env_data.ver=src_env_data.ver;else if(data_changed || file_changed)env_data.newVer();
               elm_newer_src=env.newer(src_env);
             //src_newer_elm=src_env.newer(env); no need to set because we already have everything
            }
         }break;

         case ELM_MESH:
         {
            ElmMesh &mesh_data=*elm.meshData(), &src_mesh_data=*src.meshData();
            Elm *rebuild=null;
            if(has_file && mesh_data.syncFile(src_mesh_data))
            {
               if(!IsServer) // check if mesh existence has changed for possible 'makeGameVer' of object which depends on mesh existence, this doesn't need to be done on the server
               {
                  Mesh original, updated;
                  Load(original, path, T.game_path); updated.load(src_data, T.game_path); src_data.pos(0);
                  if(OverrideMeshSkel(&original, null)!=OverrideMeshSkel(&updated, null))rebuild=meshToObjElm(&elm); // set object of this mesh to be rebuilt
               }
               file_changed=true; if(SafeOverwrite(src_data, path))SavedMesh(path);
            }
            if((data_changed&CHANGE_AFFECT_FILE) || file_changed){src_data.pos(0); makeGameVer(elm, has_file ? &src_data : null);}
            if(rebuild)makeGameVer(*rebuild); // rebuild after 'makeGameVer' of the mesh
         }break;

         case ELM_SKEL:
         {
            ElmSkel &skel_data=*elm.skelData(), &src_skel_data=*src.skelData();
            if(has_file && skel_data.syncFile(src_skel_data))
            {
               Elm *rebuild=null; if(!IsServer) // check if skeleton existence has changed for possible 'makeGameVer' of object which depends on skeleton existence, this doesn't need to be done on the server
               {
                  Skeleton original, updated;
                  original.load(game_path); updated.load(src_extra); src_extra.pos(0);
                  if(OverrideMeshSkel(null, &original)!=OverrideMeshSkel(null, &updated))rebuild=skelToObjElm(&elm); // set object of this skeleton to be rebuilt
               }
               file_changed=true; if(SafeOverwrite(src_data, path))SavedEditSkel(path);
               if(SafeOverwrite(src_extra, game_path))
               {
                  SavedSkel(game_path); savedGame(elm, game_path);
                  if(rebuild)makeGameVer(*rebuild); // rebuild after saving the skeleton
               }
            }
         }break;

         case ELM_PHYS:
         {
            ElmPhys &phys_data=*elm.physData(), &src_phys_data=*src.physData();
            if(has_file && phys_data.syncFile(src_phys_data))
            {
               Elm *rebuild=null; if(!IsServer) // check if phys body existence has changed for possible 'makeGameVer' of object which depends on phys body existence, this doesn't need to be done on the server
               {
                  PhysBody original, updated;
                  original.load(path); updated.load(src_data, T.game_path); src_data.pos(0);
                  if(OverridePhys(&original)!=OverridePhys(&updated))rebuild=physToObjElm(&elm); // set object of this phys body to be rebuilt
               }
               file_changed=true; if(SafeOverwrite(src_data, path))
               {
                  SavedPhys(path); savedGame(elm, game_path);
                  if(rebuild)makeGameVer(*rebuild); // rebuild after saving the phys body
               }
            }
         }break;

         case ELM_ANIM:
         {
            ElmAnim &anim_data=*elm.animData(), &src_anim_data=*src.animData();
            if(has_file && anim_data.syncFile(src_anim_data)){file_changed=true; /*anim_data.from(anim);*/ if(SafeOverwrite(src_data, path)){SavedAnim(path); savedGame(elm, game_path);}}
         }break;

         case ELM_WORLD:
         {
            if(data_changed&CHANGE_AFFECT_FILE)makeGameVer(elm);
         }break;

         case ELM_MINI_MAP:
         {
         }break;

         case ELM_ENUM:
         {
             ElmEnum &enum_data=*elm.enumData(), &src_enum_data=*src.enumData();
            EditEnums enums, src_enums; enums.load(path); src_enums.load(src_data);
            if(file_changed=enums.sync(src_enums)){enum_data.from(enums); Save(enums, path); makeGameVer(elm);}
            if(enum_data.equal(src_enum_data) && enums.equal(src_enums))enum_data.ver=src_enum_data.ver;else if(data_changed || file_changed)enum_data.newVer();
            elm_newer_src=enums.newer(src_enums);
          //src_newer_elm=src_enums.newer(enums); no need to set because we already have everything
         }break;

         case ELM_OBJ:
         {
             ElmObj   &obj_data=*elm.objData(), &src_obj_data=*src.objData();
            EditObject params, src_params; if(IsServer)Load(params, path, edit_path);else params=*EditObjectPtr(path); src_params.load(src_data, edit_path); // 'EditObject' must use 'edit_path', on client load from cache in case we're editing the file
            if(file_changed=params.sync(src_params, edit_path)){obj_data.from(params); Save(params, path, edit_path);} // 'EditObject' must use 'edit_path'
            if((data_changed&CHANGE_AFFECT_FILE) || file_changed)makeGameVer(elm);
            if(obj_data.equal(src_obj_data) && params.equal(src_params))obj_data.ver=src_obj_data.ver;else if(data_changed || file_changed)obj_data.newVer();
            elm_newer_src=params.newer(src_params);
          //src_newer_elm=src_params.newer(params); no need to set because we already have everything
         }break;

         case ELM_OBJ_CLASS:
         {
             ElmObjClass &obj_data=*elm.objClassData(), &src_obj_data=*src.objClassData();
            EditObject    params, src_params; if(IsServer)Load(params, path, edit_path);else params=*EditObjectPtr(path); src_params.load(src_data, edit_path); // 'EditObject' must use 'edit_path', on client load from cache in case we're editing the file
            if(file_changed=params.sync(src_params, edit_path)){obj_data.from(params); Save(params, path, edit_path);} // 'EditObject' must use 'edit_path'
            if((data_changed&CHANGE_AFFECT_FILE) || file_changed)makeGameVer(elm);
            if(obj_data.equal(src_obj_data) && params.equal(src_params))obj_data.ver=src_obj_data.ver;else if(data_changed || file_changed)obj_data.newVer();
            elm_newer_src=params.newer(src_params);
          //src_newer_elm=src_params.newer(params); no need to set because we already have everything
         }break;

         case ELM_GUI:
         {
            if(has_file && elm.guiData()->syncFile(*src.guiData())){file_changed=true; SafeOverwrite(src_data, path);}
         }break;

         case ELM_SOUND:
         {
            if(has_file && elm.soundData()->syncFile(*src.soundData())){file_changed=true; SafeOverwrite(src_data, path);}
         }break;

         case ELM_VIDEO:
         {
            if(has_file && elm.videoData()->syncFile(*src.videoData())){file_changed=true; SafeOverwrite(src_data, path);}
         }break;

         case ELM_FILE:
         {
            if(has_file && elm.fileData()->syncFile(*src.fileData())){file_changed=true; SafeOverwrite(src_data, path);}
         }break;

       //case ELM_CODE: break; // this is synchronized manually elsewhere
       //case ELM_APP : break; // this is synchronized manually elsewhere
      }
      return data_changed || file_changed;
   }
   uint Project::syncArea(C UID &world_id, C VecI2 &area_xy, uint area_sync_flag, C AreaVer &src_area, Heightmap &src_hm, Memc<ObjData> &src_objs, Memc<UID> *local_objs_newer)
   {
      if(world_id.valid())
         if(WorldVer *world_ver=worldVerRequire(world_id))
         if(AreaVer  *dest_area=world_ver->areas.get(area_xy))
      {
         createWorldPaths(world_id);

         if(area_sync_flag&AREA_SYNC_OBJ) // do this before heightmap sync because those use 'return'
         {
            if(syncObj(world_id, area_xy, src_objs, null, local_objs_newer)){dest_area->obj_ver=src_area.obj_ver; world_ver->changed=true;} // if after syncing objects they are in equal state, then set same 'obj_ver' as the source
         }

         if((area_sync_flag&AREA_SYNC_REMOVED) && dest_area->hasHm() && !src_area.hasHm() && !AreaVer::HasHm(src_area.hm_removed_time, dest_area->hm_height_time)) // dest has heightmap, and shouldn't have after syncing
         {
            hmDel(world_id, area_xy, &src_area.hm_removed_time); return AREA_SYNC_REMOVED;
         }
         if(dest_area->hasHm() || AreaVer::HasHm(dest_area->hm_removed_time, src_area.hm_height_time)) // dest has heightmap or it will have after syncing
         {
            uint do_sync_flag=(((area_sync_flag&AREA_SYNC_HEIGHT) && src_area.hm_height_time>dest_area->hm_height_time) ? AREA_SYNC_HEIGHT : 0)
                             |(((area_sync_flag&AREA_SYNC_MTRL  ) && src_area.  hm_mtrl_time>dest_area->  hm_mtrl_time) ? AREA_SYNC_MTRL   : 0)
                             |(((area_sync_flag&AREA_SYNC_COLOR ) && src_area. hm_color_time>dest_area-> hm_color_time) ? AREA_SYNC_COLOR  : 0);
            if(do_sync_flag)return hmUpdate(world_id, area_xy, area_sync_flag, src_area, src_hm); // if any element is newer
         }
      }
      return 0;
   }
      Project::AreaSyncObjData::AreaSyncObjData(Project &proj, C UID &world_id, WorldVer &world_ver) : proj(proj), world_id(world_id), world_ver(world_ver) {}
     Project::AreaSyncObj::~AreaSyncObj()
      {
         if(changed)
         {
            area_ver->obj_ver.randomize();
                                                                       SaveEditObject(chunks, objs,  edit_path          ); chunks.save(chunk_edit_path, WorldAreaSync);  // save edit
            if(!IsServer){chunks.load(chunk_game_path, WorldAreaSync); SaveGameObject(chunks, objs, *project, *world_ver); chunks.save(chunk_game_path, WorldAreaSync);} // save game
         }
      }
      bool Project::AreaSyncObj::Create(AreaSyncObj &area, C VecI2 &area_xy, ptr asod_ptr)
      {
         AreaSyncObjData &asod=*(AreaSyncObjData*)asod_ptr;
         area.xy             =area_xy;
         area.world_ver      =&asod.world_ver;
         area.area_ver       =asod.world_ver.areas.get(area_xy);
         area.project        =&asod.proj;
         area.chunk_edit_path=asod.proj.editAreaPath(asod.world_id, area_xy);
         area.chunk_game_path=asod.proj.gameAreaPath(asod.world_id, area_xy);
         area.game_path      =asod.proj.game_path;
         area.edit_path      =asod.proj.edit_path;
         area.chunks.load(area.chunk_edit_path, WorldAreaSync);
         if(Chunk *chunk=area.chunks.findChunk("Object"))LoadEditObject(chunk->ver, File().readMem(chunk->data(), chunk->elms()), area.objs, area.edit_path); // load edit objects
         return true;
      }
   void Project::rebuildEmbedObj(C UID &world_obj_instance_id, C VecI2 &area_xy, WorldVer &world_ver, bool rebuild_game_area_objs) // this must be called after 'changedObj'
   {
      if(!IsServer) // this doesn't need to be performed on the server
         if(Elm *world=findElm(world_ver.world_id))
            if(ElmWorld *world_data=world->worldData())
               if(world_data->valid())
                  if(ObjVer *obj_ver=world_ver.obj.find(world_obj_instance_id)) // get obj ver
      {
         RectI *is_embed=world_ver.obj_embed.find(world_obj_instance_id); // if already is embedded
         bool want_embed=false; RectI want_embed_rect; Box box;
         if(!obj_ver->removed() && obj_ver->terrain(T) && getObjBox(obj_ver->elm_obj_id, box)) // if can be embedded
         {
            box*=obj_ver->matrix();
            flt area_size=world_data->area_size;
            if(EmbedObject(box, area_xy, area_size)) // if should be embedded
            {
               want_embed=true; want_embed_rect.set(Floor(box.min.xz()/area_size), Floor(box.max.xz()/area_size));
            }
         }

         // rebuild if needed
         if(  is_embed)world_ver.rebuildEmbedObj( *is_embed     ); // rebuild old areas
         if(want_embed)world_ver.rebuildEmbedObj(want_embed_rect); // rebuild new areas
         if(rebuild_game_area_objs && (is_embed!=null)!=want_embed)world_ver.rebuildGameAreaObjs(area_xy); // if changing embedded state then we need to rebuild game area objects (add or remove the object from area obj list)

         // setup in world ver (do this after using 'is_embed')
         {
            MapLock ml(world_ver.obj_embed); // lock before changing 'obj_embed'
            if(want_embed)*world_ver.obj_embed(world_obj_instance_id)=want_embed_rect;else world_ver.obj_embed.removeKey(world_obj_instance_id); world_ver.changed=true;
         }
      }
   }
   void Project::rebuildWater(Lake *lake, River *river, C UID &water_id, WorldVer &world_ver)
   {
      if(!IsServer) // this doesn't need to be performed on the server
         if(lake || river)
            if(Elm *world=findElm(world_ver.world_id))
               if(ElmWorld *world_data=world->worldData())
                  if(world_data->valid())
                     if(WaterVer *water_ver=(lake ? world_ver.lakes.find(water_id) : world_ver.rivers.find(water_id)))
      {
         flt   area_size=world_data->area_size;
         RectI  old_area=water_ver ->areas,
                new_area(0, -1);
         Rect  rect; if(lake ? (!lake->removed && lake->getRect(rect)) : (!river->removed && river->getRect(rect)))new_area.set(Floor(rect.min/area_size), Floor(rect.max/area_size));

         // rebuild if needed
         if(old_area.valid())world_ver.rebuildWater(old_area);
         if(new_area.valid())world_ver.rebuildWater(new_area);

         // setup in world ver
         if(lake ){MapLock ml(world_ver.lakes ); water_ver->areas=new_area; world_ver.changed=true;} // lock before changing 'world_ver'
         if(river){MapLock ml(world_ver.rivers); water_ver->areas=new_area; world_ver.changed=true;} // lock before changing 'world_ver'
      }
   }
   bool Project::syncObj(C UID &world_id, C VecI2 &area_xy, Memc<ObjData> &objs, Map<VecI2, Memc<ObjData> > *obj_modified, Memc<UID> *local_newer)
   {
      if(world_id.valid())
         if(WorldVer *world_ver=worldVerRequire(world_id))
      {
         AreaSyncObjData         asod(T, world_id, *world_ver);
         Map<VecI2, AreaSyncObj> areas(Compare, AreaSyncObj::Create, &asod);
         AreaSyncObj            *target_area=areas(area_xy);
         REPA(objs)
         {
            ObjData &obj=objs[i];
            ObjVer  *obj_ver=world_ver->obj.find(obj.id); // use 'find' to get null if not found
            if(!obj_ver) // not present in this world, then add to target area
            {
               if(obj_modified)(*obj_modified)(target_area->xy)->New()=obj;
               target_area->changed=true; world_ver->changed=true; createWorldPaths(world_id);
               world_ver->changedObj(obj, target_area->xy); // call before 'rebuildEmbedObj'
               target_area->objs.New()=obj;
               if(!IsServer                  )rebuildEmbedObj(obj.id, target_area->xy, *world_ver, false); // call after 'changedObj' (doesn't need to be performed on the server), 'rebuild_game_area_objs=false' because we're saving game area here anyway
               if(!IsServer && obj.physPath())world_ver->rebuildPaths(obj.id, target_area->xy); // don't check for 'physPath' on server because loading 'Object' may fail (doesn't need to be performed on the server)
            }else // present in some area
            if(AreaSyncObj *cur_area=areas(obj_ver->area_xy)) // load that area
               REPA(cur_area->objs)if(cur_area->objs[i].id==obj.id) // found that object
            {
               ObjData   &cur_obj        =cur_area->objs[i];
               TimeStamp  old_matrix_time=cur_obj.matrix_time ; // remember old matrix time before syncing
               TerrainObj old_terrain    =cur_obj.terrainObj(); // remember old terrain obj before syncing
               PhysPath   old_phys; if(!IsServer)old_phys=cur_obj.physPath(); // don't check for 'physPath' on server because loading 'Object' may fail (doesn't need to be performed on the server)
               if(cur_obj.sync(obj, edit_path)) // if performed any change
               {
                  bool changed_matrix   =(cur_obj.matrix_time>old_matrix_time),
                       changed_embed    =false; if(!IsServer)changed_embed    =(cur_obj.terrainObj()!=old_terrain ||  changed_matrix); // if changed terrain or changed matrix (doesn't need to be performed on the server)
                  bool changed_phys_path=false; if(!IsServer)changed_phys_path=(cur_obj.physPath  ()!=old_phys    || (changed_matrix && (old_phys || cur_obj.physPath()))); // if changed phys, or if changed matrix and have phys (doesn't need to be performed on the server)
                  AreaSyncObj &new_area=(changed_matrix ? *target_area : *cur_area); // check if received newer position
                  if(obj_modified)(*obj_modified)(new_area.xy)->New()=cur_obj;
                  cur_area->changed=true; world_ver->changed=true; createWorldPaths(world_id);
                  world_ver->changedObj(cur_obj, new_area.xy); // call before 'rebuildEmbedObj'

                  if(changed_embed    )rebuildEmbedObj(cur_obj.id, new_area.xy, *world_ver, false); // call after 'changedObj' and before 'rebuildPaths', 'rebuild_game_area_objs=false' because we're saving game area here anyway
                  if(changed_phys_path)world_ver->rebuildPaths(obj.id, cur_area->xy); // call after 'rebuildEmbedObj' for old area
                  if(cur_area!=target_area && changed_matrix) // we're moving to different area
                  {
                     target_area->changed=true; 
                     if(changed_phys_path)world_ver->rebuildPaths(obj.id, target_area->xy); // call after 'rebuildEmbedObj' for new area
                     Swap(target_area->objs.New(), cur_obj); cur_area->objs.remove(i); // move to new area and remove from old area
                  }
               }
               break;
            }
         }
         if(local_newer)GetNewer(target_area->objs, objs, *local_newer);
         return Same(target_area->objs, objs);
      }
      return false;
   }
   bool Project::syncWaypoint(C UID &world_id, C UID &waypoint_id, Version &src_ver, EditWaypoint &src) // this should modify 'src_ver' and 'src' according to final data, because Server CS_SET_WORLD_WAYPOINT relies on that
   {
      if(world_id.valid())
         if(WorldVer *world_ver   =worldVerRequire(world_id))
         if(Version  *waypoint_ver=world_ver->waypoints.get(waypoint_id))
            if(src_ver!=*waypoint_ver)
      {
         Str          edit=editWaypointPath(world_id, waypoint_id);
         EditWaypoint waypoint; waypoint.load(edit);
         bool changed=waypoint.sync(src);
         if(  changed)
         {
            createWorldPaths(world_id);
            Save(waypoint, edit); // save edit
            if(!IsServer){Game::Waypoint w; Str game=gameWaypointPath(world_id, waypoint_id); if(waypoint.copyTo(w))Save(w, game);else FDelFile(game);} // make game
         }
         if(waypoint.equal(src)){*waypoint_ver=src_ver; world_ver->changed=true;}else if(changed){waypoint_ver->randomize(); world_ver->changed=true;}

         // set output because of comment at start of function
         src_ver=*waypoint_ver;
         src    = waypoint;
         return changed;
      }
      return false;
   }
   bool Project::syncLake(C UID &world_id, C UID &lake_id, Version &src_ver, Lake &src) // this should modify 'src_ver' and 'src' according to final data, because Server CS_SET_WORLD_WAYPOINT relies on that
   {
      if(world_id.valid())
         if(WorldVer *world_ver=worldVerRequire(world_id))
         if(WaterVer *lake_ver =world_ver->lakes.get(lake_id))
            if(src_ver!=lake_ver->ver)
      {
         Str  edit=editLakePath(world_id, lake_id);
         Lake lake; lake.load(edit);
         bool changed=lake.sync(src);
         if(  changed)
         {
            createWorldPaths(world_id);
            Save(lake, edit); // save edit
            rebuildWater(&lake, null, lake_id, *world_ver);
         }
         if(lake.equal(src)){lake_ver->ver=src_ver; world_ver->changed=true;}else if(changed){lake_ver->ver.randomize(); world_ver->changed=true;}

         // set output because of comment at start of function
         src_ver=lake_ver->ver;
         src    =lake;
         return changed;
      }
      return false;
   }
   bool Project::syncRiver(C UID &world_id, C UID &river_id, Version &src_ver, River &src) // this should modify 'src_ver' and 'src' according to final data, because Server CS_SET_WORLD_WAYPOINT relies on that
   {
      if(world_id.valid())
         if(WorldVer *world_ver=worldVerRequire(world_id))
         if(WaterVer *river_ver=world_ver->rivers.get(river_id))
            if(src_ver!=river_ver->ver)
      {
         Str   edit=editRiverPath(world_id, river_id);
         River river; river.load(edit);
         bool  changed=river.sync(src);
         if(   changed)
         {
            createWorldPaths(world_id);
            Save(river, edit); // save edit
            rebuildWater(null, &river, river_id, *world_ver);
         }
         if(river.equal(src)){river_ver->ver=src_ver; world_ver->changed=true;}else if(changed){river_ver->ver.randomize(); world_ver->changed=true;}

         // set output because of comment at start of function
         src_ver=river_ver->ver;
         src    =river;
         return changed;
      }
      return false;
   }
   bool Project::syncMiniMapSettings(C UID &mini_map_id, C Game::MiniMap::Settings &settings, C TimeStamp &settings_time)
   {
      if(mini_map_id.valid())
         if(MiniMapVer *mini_map_ver=miniMapVerRequire(mini_map_id))
            if(settings_time>mini_map_ver->time)
      {
         FDelInside(gamePath(mini_map_id));
          createMiniMapPaths(mini_map_id);
         mini_map_ver->time    =settings_time;
         mini_map_ver->settings=settings; settings.save(gamePath(mini_map_id).tailSlash(true)+"Settings");
         mini_map_ver->images.clear();
         mini_map_ver->changed=true;
         return true;
      }
      return false;
   }
   bool Project::syncMiniMapImage(C UID &mini_map_id, C VecI2 &image_xy, C TimeStamp &image_time, File &image_data)
   {
      if(mini_map_id.valid())
         if(MiniMapVer *mini_map_ver=miniMapVerRequire(mini_map_id))
            if(image_time==mini_map_ver->time)
      {
         Str image_name=gamePath(mini_map_id).tailSlash(true)+image_xy;
         if( image_data.is()) // if image data exists then save it
         {
            image_data.pos(0); if(SafeOverwrite(image_data, image_name))if(mini_map_ver->images.binaryInclude(image_xy, Compare))mini_map_ver->changed=true;
         }else // otherwise delete it
         {
            FDelFile(image_name); if(mini_map_ver->images.binaryExclude(image_xy, Compare))mini_map_ver->changed=true;
         }
         return true;
      }
      return false;
   }
   bool Project::newerSettings(C Project &src)C
   {
      return cipher_time>src.cipher_time || cipher_key_time>src.cipher_key_time || compress_type_time>src.compress_type_time
          || compress_level_time>src.compress_level_time || material_simplify_time>src.material_simplify_time;
   }
   bool Project::oldSettings(C TimeStamp &now)C
   {
      return cipher_time<now && cipher_key_time<now && compress_type_time<now && compress_level_time<now && material_simplify_time<now;
   }
   bool Project::syncSettings(C Project &src)
   {
      bool changed=false;
      changed|=Sync(cipher_time           , src.cipher_time           , cipher           , src.cipher           );
      changed|=Sync(compress_type_time    , src.compress_type_time    , compress_type    , src.compress_type    );
      changed|=Sync(compress_level_time   , src.compress_level_time   , compress_level   , src.compress_level   );
      changed|=Sync(material_simplify_time, src.material_simplify_time, material_simplify, src.material_simplify);
      if(Sync(cipher_key_time, src.cipher_key_time)){changed=true; Copy(cipher_key, src.cipher_key);}
      return changed;
   }
   void Project::initSettings(C Project &src) // this is called when finished copying elements to an empty project (for example after importing *.EsenthelProject file)
   {
      syncSettings(src);
      app_id=src.app_id;
              hm_mtrl_id= src.   hm_mtrl_id;
           water_mtrl_id= src.water_mtrl_id;
      Copy(mtrl_brush_id, src.mtrl_brush_id, SIZE(mtrl_brush_id));
   }
   void Project::flush(SAVE_MODE save_mode)
   {
      {CacheLock cl(   world_vers); REPA(   world_vers)   world_vers.lockedData(i).flush();}
      {CacheLock cl(mini_map_vers); REPA(mini_map_vers)mini_map_vers.lockedData(i).flush();}
   }
   bool Project::loadOldSettings(File &f)
   {
      if(f.getUInt()==CC4('P', 'R', 'S', 'T'))
      {
         UID proj_id; byte encrypt_key[32];
         switch(f.decUIntV())
         {
            default: goto error;

            case 3:
            {
               f>>proj_id; GetStr2(f, name); f>>cipher>>encrypt_key>>compress_type>>compress_level>>material_simplify
                >>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time>>material_simplify_time;
               f>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];
            }break;

            case 2:
            {
               byte max_tex_size; TimeStamp max_tex_size_time;
               f>>proj_id; GetStr2(f, name); f>>cipher>>encrypt_key>>compress_type>>compress_level>>material_simplify>>max_tex_size
                >>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time>>material_simplify_time>>max_tex_size_time;
               f>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];
            }break;

            case 1:
            {
               f>>proj_id; GetStr(f, name);
               f>>cipher>>encrypt_key>>compress_type>>compress_level>>material_simplify>>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time>>material_simplify_time;
               f>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];
            }break;

            case 0:
            {
               f>>proj_id; GetStr(f, name);
               f>>cipher>>encrypt_key>>compress_type>>compress_level>>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time;
               f>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];
            }break;
         }
         if(f.ok())return true;
      }
   error:
      return false;
   }
   bool Project::loadOldSettings(C Str &name)  {File f; return f.readTry(name) && loadOldSettings(f);}
   bool Project::loadOldSettings2(C Str &name)  {return loadOldSettings(name) || loadOldSettings(name+".old");}
   bool Project::save(File &f, bool network, SAVE_DATA mode)C
   {
      f.putUInt(CC4_PRDT);

      // short header first, which will be used when loading project list (only ID and name)
      f.cmpUIntV(4); // version
      if(network)f<<id; // ID needed only for network, on local it is obtained from folder name
      f<<name;
      if(!network)f<<synchronize;

      if(mode>=SAVE_SETTINGS)
      {
         // 'ProjectVersion'
         f.cmpUIntV(ProjectVersion);

         // settings
         f.cmpUIntV(0); // version
         f<<cipher<<cipher_key<<compress_type<<compress_level<<material_simplify
          <<cipher_time<<cipher_key_time<<compress_type_time<<compress_level_time<<material_simplify_time;
         if(!network){f<<text_data<<app_id<<hm_mtrl_id<<water_mtrl_id; FREPA(mtrl_brush_id)f<<mtrl_brush_id[i];}

         if(mode>=SAVE_ALL)
         {
            // data
            f.cmpUIntV(0); // version
            f.cmpUIntV(elms.elms()); FREPAO(elms).save(f, network, network);
            f.cmpUIntV(texs.elms()); FREPA (texs)f<<texs[i];

            // update
            if(!network) // not sent over network, because projects are first updated locally fully, and sent once everything finished
            {
               f.cmpUIntV(texs_update.elms()); FREPA(texs_update)f<<texs_update[i];
            }
         }
      }
      return f.ok();
   }
   LOAD_RESULT Project::load(File &f, int &ver, bool network, SAVE_DATA mode)
   {
      ver=-1;
      del();
      if(!f.left   ())return LOAD_EMPTY;
      if( f.getUInt()!=CC4_PRDT)goto error;
      switch(f.decUIntV())
      {
         default: goto newer;

         case 4:
         {
            if(network)f>>id;
            f>>name;
            if(!network)f>>synchronize;
            if(mode>=SAVE_SETTINGS)
            {
               // 'ProjectVersion'
               ver=f.decUIntV(); if(ver>ProjectVersion)goto newer; // if project requires a newer version then don't load it

               // settings
               switch(f.decUIntV())
               {
                  default: goto newer;

                  case 0:
                  {
                     f>>cipher>>cipher_key>>compress_type>>compress_level>>material_simplify
                      >>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time>>material_simplify_time;
                     if(!network){f>>text_data>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];}
                  }break;
               }

               // data
               if(mode>=SAVE_ALL)
                  switch(f.decUIntV())
               {
                  default: goto newer;

                  case 0:
                  {
                     elms.setNum(f.decUIntV()); FREPA(elms)if(!elms[i].load(f, network, network)){if(f.ok())goto newer; goto error;}
                     texs.setNum(f.decUIntV()); FREPA(texs)f>>texs[i];

                     // update
                     if(!network){texs_update.setNum(f.decUIntV()); FREPA(texs_update)f>>texs_update[i];}
                  }break;
               }
            }
         }break;

         case 3:
         {
            if(network)f>>id;
            GetStr2(f, name);
            if(!network)f>>synchronize;
            if(mode>=SAVE_SETTINGS)
            {
               // 'ProjectVersion'
               ver=f.decUIntV(); if(ver>ProjectVersion)goto newer; // if project requires a newer version then don't load it

               // settings
               switch(f.decUIntV())
               {
                  default: goto newer;

                  case 0:
                  {
                     f>>cipher>>cipher_key>>compress_type>>compress_level>>material_simplify
                      >>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time>>material_simplify_time;
                     if(!network){f>>text_data>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];}
                  }break;
               }

               // data
               if(mode>=SAVE_ALL)
                  switch(f.decUIntV())
               {
                  default: goto newer;

                  case 0:
                  {
                     elms.setNum(f.decUIntV()); FREPA(elms)if(!elms[i].load(f, network, network)){if(f.ok())goto newer; goto error;}
                     texs.setNum(f.decUIntV()); FREPA(texs)f>>texs[i];

                     // update
                     if(!network){texs_update.setNum(f.decUIntV()); FREPA(texs_update)f>>texs_update[i];}
                  }break;
               }
            }
         }break;

         case 2:
         {
            if(network)f>>id;
            GetStr2(f, name);
            if(mode>=SAVE_SETTINGS)
            {
               // 'ProjectVersion'
               ver=f.decUIntV(); if(ver>ProjectVersion)goto newer; // if project requires a newer version then don't load it

               // settings
               switch(f.decUIntV())
               {
                  default: goto newer;

                  case 1:
                  {
                     f>>cipher>>cipher_key>>compress_type>>compress_level>>material_simplify
                      >>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time>>material_simplify_time;
                     if(!network){f>>text_data>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];}
                  }break;

                  case 0:
                  {
                     byte encrypt_key[32];
                     f>>cipher>>encrypt_key>>compress_type>>compress_level>>material_simplify
                      >>cipher_time>>cipher_key_time>>compress_type_time>>compress_level_time>>material_simplify_time;
                     if(!network){f>>text_data>>app_id>>hm_mtrl_id>>water_mtrl_id; FREPA(mtrl_brush_id)f>>mtrl_brush_id[i];}
                  }break;
               }

               // data
               if(mode>=SAVE_ALL)
                  switch(f.decUIntV())
               {
                  default: goto newer;

                  case 0:
                  {
                     elms.setNum(f.decUIntV()); FREPA(elms)if(!elms[i].load(f, network, network)){if(f.ok())goto newer; goto error;}
                     texs.setNum(f.decUIntV()); FREPA(texs)f>>texs[i];

                     // update
                     if(!network){texs_update.setNum(f.decUIntV()); FREPA(texs_update)f>>texs_update[i];}
                  }break;
               }
            }
         }break;

         case 1:
         {
            ver=f.decUIntV();
            f.getUID();
            if(mode>=SAVE_ALL)
            {
               elms.setNum(f.decUIntV()); FREPA(elms)if(!elms[i].load(f, network, network)){if(f.ok())goto newer; goto error;}
               texs.setNum(f.decUIntV()); FREPA(texs)f>>texs[i];

               // update
               if(ver>27){texs_update.setNum(f.decUIntV()); FREPA(texs_update)f>>texs_update[i];}
            }
         }break;

         case 0:
         {
            ver=0;
            f.getUID();
            if(mode>=SAVE_ALL)
            {
               elms.setNum(f.decUIntV()); FREPA(elms)if(!elms[i].load(f, network, network)){if(f.ok())goto newer; goto error;}
               texs.setNum(f.decUIntV()); FREPA(texs)f>>texs[i];
            }
         }break;
      }
      if(f.ok())
      {
         if(mode>=SAVE_ALL)quickUpdateVersion(ver);
         return LOAD_OK;
      }
   error:
      del(); return LOAD_ERROR;
   newer:
      del(); return LOAD_NEWER;
   }
   void Project::save(MemPtr<TextNode> nodes)C
   {
      const bool save_all=true; // this is needed when not calling 'del' in 'load'
                                                    nodes.New().set   ("Version"              , ProjectVersion);
                                                    nodes.New().set   ("Name"                 , name);
                      if(save_all || synchronize  ) nodes.New().set   ("Synchronize"          , synchronize);
                      if(save_all || cipher       ) nodes.New().set   ("Encrypt"              , cipher);
      REPA(cipher_key)if(save_all || cipher_key[i]){nodes.New().setRaw("EncryptionKey"        , cipher_key); break;}
                      if(save_all || compress_type) nodes.New().set   ("Compress"             , CompressionName(compress_type));
                                                    nodes.New().set   ("CompressLevel"        , compress_level);
                                                    nodes.New().set   ("SimplifyMaterials"    , material_simplify);
                                                    nodes.New().set   ("EncryptTime"          , cipher_time.text());
                                                    nodes.New().set   ("EncryptionKeyTime"    , cipher_key_time.text());
                                                    nodes.New().set   ("CompressTime"         , compress_type_time.text());
                                                    nodes.New().set   ("CompressLevelTime"    , compress_level_time.text());
                                                    nodes.New().set   ("SimplifyMaterialsTime", material_simplify_time.text());
      if(save_all || elms.elms())
      {
         TextNode &node=nodes.New().setName("Elements");
         FREPAO(elms).save(node.nodes.New()); // save in order
      }
      if(save_all || texs.elms())
      {
         TextNode &node=nodes.New().setName("Textures");
         FREPA(texs)node.nodes.New().setValueFN(texs[i]); // save in order
      }
   }
   LOAD_RESULT Project::load(C MemPtr<TextNode> &nodes, int &ver, Str &error) // !! this assumes that binary was already loaded and 'ver' already set !!
   {
      error.clear();
    //del(); don't delete, instead let text values override existing members from binary, so we can keep settings not saved in text files, such as element IMPORTING/OPENED, and local user Project Settings (current heightmap material, etc.)
      FREPA(nodes)
      {
       C TextNode &node=nodes[i];
         if(node.name=="Version"              ){node.getValue(ver); if(ver>ProjectVersion)goto newer;}else
         if(node.name=="Name"                 )node.getValue(name);else
         if(node.name=="Synchronize"          )synchronize=node.asBool1();else
         if(node.name=="Encrypt"              )cipher=(CIPHER_TYPE)node.asInt();else
         if(node.name=="EncryptionKey"        )node.getValueRaw(cipher_key);else
         if(node.name=="Compress"             ){REP(COMPRESS_NUM)if(node.value==CompressionName(COMPRESS_TYPE(i))){compress_type=COMPRESS_TYPE(i); break;}}else
         if(node.name=="CompressLevel"        )compress_level        =node.asInt ();else
         if(node.name=="SimplifyMaterials"    )material_simplify     =(MATERIAL_SIMPLIFY)node.asInt();else
         if(node.name=="EncryptTime"          )cipher_time           =node.asText();else
         if(node.name=="EncryptionKeyTime"    )cipher_key_time       =node.asText();else
         if(node.name=="CompressTime"         )compress_type_time    =node.asText();else
         if(node.name=="CompressLevelTime"    )compress_level_time   =node.asText();else
         if(node.name=="SimplifyMaterialsTime")material_simplify_time=node.asText();else
         if(node.name=="Elements"             )
         {
            // remember 'IMPORTING' and 'OPENED' which are not saved in text
            Memc<UID> importing, opened;
            FREPA(elms)
            {
             C Elm &elm=elms[i];
               if(elm.importing())importing.add(elm.id);
               if(elm.opened   ())opened   .add(elm.id);
            }

            elms.clear(); // clear all binary elements, and keep those only from the text file, also text based load methods assume that element are clean (don't have existing data, but straight after constructor)
            FREPA(node.nodes) // process in order because they are sorted to avoid moving elements in 'getElm'
            {
             C TextNode &elm_node=node.nodes[i];
               UID id; if(id.fromText(elm_node.name) && id.valid())
               {
                  Elm &elm=getElm(id);
                  if(  elm.type){error=S+"Element \""+elm_node.name+"\" listed more than 1 time"; goto error;}
                  if( !elm.load(elm_node, error))goto error;
               }else {error=S+"Invalid Element ID \""+elm_node.name+'"'; goto error;}
            }

            FREPA(importing)if(Elm *elm=findElm(importing[i]))elm->importing(true);
            FREPA(opened   )if(Elm *elm=findElm(opened   [i]))elm->opened   (true);
         }else
         if(node.name=="Textures")
         {
            texs.clear();
            FREPA(node.nodes) // process in order because they are sorted to avoid moving elements in 'texs.binaryInclude'
            {
               UID id; if(node.nodes[i].getValue(id) && id.valid())texs.binaryInclude(id, Compare); // use 'binaryInclude' in case text file is messed up
               else {error=S+"Invalid Texture ID \""+node.nodes[i].value+'"'; goto error;}
            }
         }
      }
      quickUpdateVersion(ver);
      return LOAD_OK;
   error:
      del(); return LOAD_ERROR;
   newer:
      del(); return LOAD_NEWER;
   }
   LOAD_RESULT Project::load(C Str &name, int &ver, SAVE_DATA mode)  {File f; if(f.readTry(name))return load(f, ver, false, mode); ver=-1; del(); return LOAD_EMPTY;}
   bool        Project::save(C Str &name                                   )C {File f; save(f.writeMem()); f.pos(0); return SafeOverwrite(f, name);}
   LOAD_RESULT Project::load2(C Str &name, int &ver, SAVE_DATA mode)  {LOAD_RESULT result=load(name, ver, mode); if(result==LOAD_EMPTY || result==LOAD_ERROR)result=load(name+".old", ver, mode); return result;}
   bool        Project::save2(C Str &name                                   )C {if(FExistSystem(name))if(!FRename(name, name+".old"))return false; return save(name);}
   bool Project::saveTxt(C Str &name)C
   {
      if(name.is())
      {
         TextData data; save(data.nodes);
         Str temp=name+"@new";
         if(data.save(temp) && FRename(temp, name))return true;
         FDelFile(temp);
      }
      return false;
   }
   LOAD_RESULT Project::loadTxt(C Str &name, int &ver, Str &error) // !! this assumes that binary was already loaded and 'ver' already set !!
   {
      FileText file; if(file.read(name))
      {
         TextData data; if(data.load(file))return load(data.nodes, ver, error);
         VecI2 col_line; char c=file.posInfo(file.pos()-1, col_line); // if TextData failed to load, due to unexpected character, then it stopped reading after that character, so get what was last read
         error=S+"Unexpected character '"+c+"', Column: "+(col_line.x+1)+", Line: "+(col_line.y+1)+", in File: \""+Str(name).replace(' ', Nbsp)+'"'; ver=-1; del(); return LOAD_ERROR;
      }else
      if(FExistSystem(name)){error=S+"Can't access file \""+Str(name).replace(' ', Nbsp)+"\""; ver=-1; del(); return LOAD_ERROR;}
      error.clear(); return LOAD_EMPTY; // here unlike 'load', 'ver' is not cleared and 'del' is not called for LOAD_EMPTY because we assume that binary was already loaded
   }
   LOAD_RESULT Project::load3(Str path, int &ver, Str &error, SAVE_DATA mode) // this loads "Data" and "Data.txt" (and old "Settings" file)
   {
      path.tailSlash(true);
      error.clear();
      LOAD_RESULT result=load2(path+"Data", ver, mode);
      if(LoadOK(result))
      {
         if(ver>=0 && ver<=34)loadOldSettings2(path+"Settings"); // ver 34 and below had settings in a separate file
         if(text_data && !IsServer) // this should not be done on the Server
         {
            LOAD_RESULT text_result=loadTxt(path+"Data.txt", ver, error);
            if(text_result!=LOAD_EMPTY)result=text_result;
         }
      }
      return result;
   }
   bool Project::isProject(C FileFind &ff) // this will only set 'id' and 'name', but not 'path'
   {
      if(ff.type==FSTD_DIR)
      {
         UID id; if(DecodeFileName(ff.name, id))
         {
            int ver; Str error; LOAD_RESULT result=load3(ff.pathName(), ver, error, SAVE_ID_NAME);
            switch(result)
            {
               case LOAD_OK    :
               case LOAD_NEWER :
               case LOAD_LOCKED: T.id=id; return true; // don't set 'path' because if 'close' is called later then it would save data
            }
         }
      }
      return false;
   }
   void Project::setIDPath(C UID &id, C Str &path)
   {
      T.id=id;
                T.path=path; T.path.tailSlash(true);
             code_path=T.path+"Code\\"; code_base_path=code_path+"Base\\";
             edit_path=T.path+"Edit\\";
             game_path=T.path+"Game\\";
             temp_path=T.path+"Temp\\";
              tex_path=game_path+"Tex\\";
         temp_tex_path=temp_path+"Tex\\";
 temp_tex_dynamic_path=temp_tex_path+"Dynamic\\";
   }
   LOAD_RESULT Project::open(C UID &id, C Str &name, C Str &path, Str &error, bool ignore_lock)
   {
      error.clear();
      UID _id=id; Str _name=name, _path=path; _path.tailSlash(true); // copy to temp vars in case params are set to this, and would get cleared in 'close'

      close(); // close existing project

      if(!_id.valid()){error="Invalid ID"  ; return LOAD_ERROR;} // ID   must be valid
      if(!_path.is() ){error="Invalid Path"; return LOAD_ERROR;} // path must be valid
      Str lock_name=_path+"Lock";
      if(!ignore_lock && FExistSystem(lock_name))return LOAD_LOCKED; // check if it's already opened, do this at start so the project won't be opened
      File f; f.writeTry(lock_name); f.del(); // lock it

      int ver; LOAD_RESULT result=load3(_path, ver, error);
      if(LoadOK(result))
      {
         if(result==LOAD_EMPTY)T.name=_name;

         setIDPath(_id, _path);
         FCreateDirs(code_path);
         FCreateDirs(edit_path);
         FCreateDirs(game_path);
         FCreateDirs( tex_path);
         if(!IsServer)
         {
            FCreateDirs(code_base_path);
            FCreateDirs(temp_path);
            FCreateDirs(temp_tex_path);
            FCreateDirs(temp_tex_dynamic_path);
         }

         updateVersion(ver);

         if(ver<=34) // ver 34 and below had settings in a separate file
            if(save2(_path+"Data")) // save with new format to include settings
         {
            FDelFile(_path+"Settings"    ); // delete old settings file
            FDelFile(_path+"Settings.old"); // delete old settings file
         }
      }else FDelFile(lock_name); // unlock on error

      return result;
   }
   bool Project::save(SAVE_MODE save_mode)
   {
      bool ok=true;
      if(path.is())
      {
         flush(save_mode); // flush first in case flushing will modify some data (like elm versions for example)
         if(!save2(path+"Data"))
         {
            ok=false;
            Gui.msgBox(S, "Can't save Project Data");
         }
         if(text_data && !IsServer) // this should not be done on the Server
            if(!saveTxt(path+"Data.txt"))
         {
            ok=false;
            Gui.msgBox(S, "Can't save Project Data.txt");
         }
      }
      return ok;
   }
   void Project::close()
   {
      save();
      if(path.is())FDelFile(path+"Lock"); // unlock
      del ();
   }
   void ElmNode::clear() {added=false; flag=0; parent=-1; children.clear();}
   ProjectHierarchy& ProjectHierarchy::del()
{
      root.clear();
      hierarchy.del();
      ::Project::del(); return T;
   }
   void ProjectHierarchy::floodRemoved(Memc<UID> &removed, ElmNode &node, bool parent_removed)
   {
      FREPA(node.children) // list in order
      {
         int      child_i    =node.children[i];
         ElmNode &child      =hierarchy[child_i];
         Elm     &elm        =elms     [child_i];
         bool     elm_removed=(elm.removed() || parent_removed);
         if(elm_removed)removed.add(elm.id);
         floodRemoved(removed, child, elm_removed);
      }
   }
   void ProjectHierarchy::floodHierarchy(ElmNode &node)
   {
      REPA(node.children)
      {
         int      child_i=node.children[i];
         ElmNode &child  =hierarchy[child_i];
         child.added=true;
         floodHierarchy(child);
      }
   }
   void ProjectHierarchy::addHierarchy(ElmNode &node, int node_i, ElmNode &target, int target_i)
   {
      if(InRange(node.parent, hierarchy))hierarchy[node.parent].children.exclude(node_i); // remove from previous parent
      node.parent=target_i; target.children.add(node_i); // add to new parent
      node.added=true; // mark as added
      floodHierarchy(node); // add children of node
   }
   void ProjectHierarchy::setHierarchy()
   {
      root.clear(); hierarchy.clear().setNum(elms.elms());
      FREPA(elms)
      {
         Elm &elm=elms[i];
         int  parent=findElmI(elm.parent_id); hierarchy[i].parent=parent;
         if(InRange(parent, hierarchy))hierarchy[parent].children.add(i);else root.children.add(i);
      }
      floodHierarchy(root);
      // here still may be some unadded elements, those which have parents incorrectly setup in a loop (A -> B -> A -> B), they can also have children (C -> B)
      Memt<int> parents;
      FREPA(elms)
      {
         ElmNode &node=hierarchy[i];
         if(!node.added) // if not yet added
         {
            // check if it goes in a loop
            parents.clear();
            for(ElmNode *cur=&node; ; )
            {
               int parent=cur->parent; if(!InRange(parent, elms))goto no_loop; // if it has no parent
               if(!parents.include(parent)) // if parent was present in the parents list, then this is a loop
               {
                  addHierarchy(node, i, root, -1);
                  break;
               }
               cur=&hierarchy[parent]; // proceed to next parent
            }
            no_loop:;
         }
      }
   }
   bool ProjectHierarchy::contains(C Elm &a, C Elm *b)C // if 'a' contains 'b'
   {
      if(!b)return false;
      int ai=elms.validIndex(&a),
          bi=elms.validIndex( b);
      for(; InRange(bi, hierarchy); ){if(bi==ai)return true; bi=hierarchy[bi].parent;}
      return false;
   }
   int ProjectHierarchy::depth(C Elm *elm)C
   {
      int    depth=-1; for(int i=elms.validIndex(elm); InRange(i, hierarchy); i=hierarchy[i].parent)depth++;
      return depth;
   }
   Elm* ProjectHierarchy::firstParent(Elm *elm, ELM_TYPE type)
   {
      for(int i=elms.validIndex(elm); InRange(i, hierarchy); i=hierarchy[i].parent)
      {
         Elm &elm=elms[i]; if(elm.type==type)return &elm;
      }
      return null;
   }
   Elm* ProjectHierarchy::firstVisibleParent(Elm *elm)
   {
      for(int i=elms.validIndex(elm); InRange(i, hierarchy); i=hierarchy[i].parent)
      {
         Elm &elm=elms[i]; if(ElmVisible(elm.type))return &elm;
      }
      return null;
   }
   Str ProjectHierarchy::elmSrcFileFirst(C Elm *elm)C
   {
      for(int i=elms.validIndex(elm); InRange(i, hierarchy); i=hierarchy[i].parent)
      {
       C Elm &elm=elms[i]; if(elm.srcFile().is())
         {
            Mems<Edit::FileParams> files=Edit::FileParams::Decode(elm.srcFile());
            FREPA(files)
            {
               Str path=FFirstUp(files[i].name); if(path.is())return path;
            }
         }
      }
      return S;
   }
   Elm* ProjectHierarchy::findElmByPath(C Str &path)
{
      if(path.is())
      {
         ElmNode *parent=&root; Str p=path; for(;;)
         {
            Str      name=GetStart(p); p=GetStartNot(p);
            Elm     *found_elm =null;
            ElmNode *found_node=null;
            REPA(parent->children)
            {
               int child_i=parent->children[i]; Elm &elm=elms[child_i]; if(elm.name==name && ElmVisible(elm.type)) // don't list hidden types
               {
                  found_elm =&elm;
                  found_node=&hierarchy[child_i];
                  if(!elm.removed())break; // stop looking if this element exists
               }
            }
            if(!p.is()    )return found_elm;
            if(!found_node)break;
            parent=found_node;
         }
      }
      return null;
   }
           Str ProjectHierarchy::elmFullName(C UID &elm_id, int max_elms)C {return ::Project::elmFullName(elm_id, max_elms);}
   Str ProjectHierarchy::elmFullName(C Elm *elm   , int max_elms)C 
{
      int length=0; Memt<C Elm*> processed; Str name;
      for(int i=elms.validIndex(elm); InRange(i, hierarchy); i=hierarchy[i].parent)
      {
       C Elm &elm=elms[i]; if(ElmVisible(elm.type)) // don't include name of elements that are not visible
         {
            if(!max_elms--){name.reserve(length+3)="..\\"; break;} // if reached the allowed limit
            processed.add(&elm); length+=elm.name.length()+1; // 1 extra for '\\'
         }
      }
      name.reserve(length); REPA(processed){name+=processed[i]->name; if(i)name+='\\';}
      return name;
   }
   void ProjectHierarchy::eraseRemoved()
   {
      removeOrphanedElms();
      bool erased=false; Memc<UID> remove; floodRemoved(remove, root);
      erased|=eraseElms     (remove);
      erased|=eraseTexs     ();
      erased|=eraseWorldObjs();
      if(erased)save(); // save immediately after erase, just in case
   }
Project::AreaSyncObj::AreaSyncObj() : changed(false), xy(0), area_ver(null), world_ver(null), project(null) {}

ElmNode::ElmNode() : added(false), flag(0), parent(-1) {}

/******************************************************************************/
