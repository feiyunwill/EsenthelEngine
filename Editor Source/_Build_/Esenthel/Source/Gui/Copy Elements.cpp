/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
CopyElements CopyElms;
/******************************************************************************/
State StateCopyElms(UpdateCopyElms, DrawCopyElms, InitCopyElms, ShutCopyElms);
/******************************************************************************/
bool CopyElmsFunc(Thread &thread)
{
   ThreadMayUseGPUData();

   int      total =CopyElms.texs_to_copy.elms()+CopyElms.elms_to_copy.elms();
   cchar   *suffix=u"@new";
   Project &src=*CopyElms.src, &dest=*CopyElms.dest;
   Pak     &pak= CopyElms.esenthel_project_pak;

   // copy textures first because elements rely on them
   FREPA(CopyElms.texs_to_copy)
   {
      if(thread.wantStop())return false;
      UID id=CopyElms.texs_to_copy[i];
      if(src.texs.binaryHas(id, Compare) && !dest.texs.binaryHas(id, Compare)) // if source has and dest doesn't
      {
         Str  sp=src.texPath(id), dp=dest.texPath(id);
         bool ok=false;
         if(pak.totalFiles())
         {
            if(C PakFile *pf=pak.find(sp))ok=FCopy(pak, *pf, dp, FILE_OVERWRITE_ALWAYS, null, suffix);
         }else
         {
            ok=FCopy(sp, dp, FILE_OVERWRITE_ALWAYS, null, null, suffix);
         }
         if(ok)
         {
            dest.texs.binaryInclude(id, Compare); // include in dest on success
            if(src.texs_update.binaryHas(id, Compare))dest.texs_update.binaryInclude(id, Compare);
         }else goto error;
      }
      UpdateProgress.set(i, total);
   }

   // copy elements
   FREPA(CopyElms.elms_to_copy)
   {
      if(thread.wantStop())return false;
      UID id=CopyElms.elms_to_copy[i];
      if(Elm *s=src.findElm(id))
      {
         Elm *d=dest.findElm(id);
         if(!d || d->type==s->type) // don't copy if are of different types
         {
            Str sep=src.editPath(id), dep=dest.editPath(id),
                sgp=src.gamePath(id), dgp=dest.gamePath(id);

            // copy files first
            if(s->type==ELM_WORLD) // World
            {
               WorldVer *src_world_ver=null, world_ver_temp;
               if(pak.totalFiles())
               {
                  if(C PakFile *pf=pak.find(sep))if(!FCopy(pak, *pf, dep+suffix)){FDelDirs(dep+suffix);                       goto error;} // copy to temp folder
                  if(C PakFile *pf=pak.find(sgp))if(!FCopy(pak, *pf, dgp+suffix)){FDelDirs(dep+suffix); FDelDirs(dgp+suffix); goto error;} // copy to temp folder
                  File f; if(f.readTry(src.worldVerPath(id), pak))if(world_ver_temp.load(f))src_world_ver=&world_ver_temp;
               }else
               {
                  if(FExist(sep) && !FCopyDir(sep, dep+suffix)){FDelDirs(dep+suffix);                       goto error;} // copy to temp folder
                  if(FExist(sgp) && !FCopyDir(sgp, dgp+suffix)){FDelDirs(dep+suffix); FDelDirs(dgp+suffix); goto error;} // copy to temp folder
                  src_world_ver=src.worldVerRequire(id);
               }
               FDelDirs(dep); FRename(dep+suffix, dep); // replace target folder with temp
               FDelDirs(dgp); FRename(dgp+suffix, dgp); // replace target folder with temp
               if(src_world_ver)if(WorldVer *dest_world_ver=dest.worldVerRequire(id))*dest_world_ver=*src_world_ver;
            }else
            if(s->type==ELM_MINI_MAP) // Mini Map
            {
               MiniMapVer *src_mini_map_ver=null, mini_map_ver_temp;
               if(pak.totalFiles())
               {
                  if(C PakFile *pf=pak.find(sgp))if(!FCopy(pak, *pf, dgp+suffix                              ))                           {FDelDirs(dgp+suffix); goto error;} // copy to temp folder
                  if(C PakFile *pf=pak.find(sep)){if(FCopy(pak, *pf, dep, FILE_OVERWRITE_ALWAYS, null, suffix))SavedEdit(s->type, dep);else{FDelDirs(dgp+suffix); goto error;}}else FDelFile(dep);
                  File f; if(f.readTry(src.miniMapVerPath(id), pak))if(mini_map_ver_temp.load(f))src_mini_map_ver=&mini_map_ver_temp;
               }else
               {
                  if( FExist(sgp) && !FCopyDir(sgp, dgp+suffix))                                                                              {FDelDirs(dgp+suffix); goto error;} // copy to temp folder
                  if(!FExist(sep))FDelFile(dep);else if(FCopy(sep, dep, FILE_OVERWRITE_ALWAYS, null, null, suffix))SavedEdit(s->type, dep);else{FDelDirs(dgp+suffix); goto error;}
                  src_mini_map_ver=src.miniMapVerRequire(id);
               }
               FDelDirs(dgp); FRename(dgp+suffix, dgp); // replace target folder with temp
               if(src_mini_map_ver)if(MiniMapVer *dest_mini_map_ver=dest.miniMapVerRequire(id))*dest_mini_map_ver=*src_mini_map_ver;
            }else
            if(s->type==ELM_CODE && CopyElms.src_ver>=49) // Code (stored separately only since version 49)
            {
               Str scp=src.codePath(id), dcp=dest.codePath(id);
               if(pak.totalFiles())
               {
                  if(C PakFile *pf=pak.find(scp)){if(FCopy(pak, *pf, dcp, FILE_OVERWRITE_ALWAYS, null, suffix))SavedCode(dcp);else goto error;}else FDelFile(dcp);
               }else
               {
                  if(!FExist(scp))FDelFile(dcp);else if(FCopy(scp, dcp, FILE_OVERWRITE_ALWAYS, null, null, suffix))SavedCode(dcp);else goto error;
               }
               // code base shouldn't be copied here
            }else // regular file
            {
             //Str scp=src.codePath(id), dcp=dest.codePath(id); this is performed above
               if(pak.totalFiles())
               {
                //if(C PakFile *pf=pak.find(scp)){if(FCopy(pak, *pf, dcp, FILE_OVERWRITE_ALWAYS, null, suffix))SavedCode(        dcp);else goto error;}else FDelFile(dcp);
                  if(C PakFile *pf=pak.find(sep)){if(FCopy(pak, *pf, dep, FILE_OVERWRITE_ALWAYS, null, suffix))SavedEdit(s->type, dep);else goto error;}else FDelFile(dep);
                  if(C PakFile *pf=pak.find(sgp)){if(FCopy(pak, *pf, dgp, FILE_OVERWRITE_ALWAYS, null, suffix))SavedGame(s->type, dgp);else goto error;}else FDelFile(dgp);
               }else
               {
                //if(!FExist(scp))FDelFile(dcp);else if(FCopy(scp, dcp, FILE_OVERWRITE_ALWAYS, null, null, suffix))SavedCode(        dcp);else goto error;
                  if(!FExist(sep))FDelFile(dep);else if(FCopy(sep, dep, FILE_OVERWRITE_ALWAYS, null, null, suffix))SavedEdit(s->type, dep);else goto error;
                  if(!FExist(sgp))FDelFile(dgp);else if(FCopy(sgp, dgp, FILE_OVERWRITE_ALWAYS, null, null, suffix))SavedGame(s->type, dgp);else goto error;
               }
            }

            // setup project element after files have been copied
            if(d)
            {
               CopyElms.elms_replaced.add(id); // if element already exists, then we're replacing it
               if(s->type==ELM_MESH) // check if we're changing body
               {
                  UID s_body, d_body; // have to check final body instead of 'meshData().body_id' because this could remain the same, but if that body has a different body in src and dest projects, then we have to make the adjustment
                   src.getMeshSkels(s->meshData(), null, &s_body); // get body in source project mesh
                  dest.getMeshSkels(d->meshData(), null, &d_body); // get body in dest   project mesh
                  if(s_body!=d_body)CopyElms.changed_body.add(id);
               }
            }else d=&dest.getElm(id); // create new element
            *d=*s;
            if(!d->parent_id.valid() && d->parent_id!=CopyElms.root)d->setParent(CopyElms.root);
            CopyElms.elms_copied.add(id);
         }
      }
      UpdateProgress.set(CopyElms.texs_to_copy.elms()+i, total);
   }
   if(0)
   {
   error:
      Gui.msgBox(S, "Can't copy file from one project to another.\nPlease verify that you have enough free disk space.");
   }
   return false;
}
/******************************************************************************/
bool InitCopyElms()
{
   SetKbExclusive();
   Proj.pause();
   UpdateProgress.create(Rect_C(0, -0.05f, 1, 0.045f));
   UpdateThread  .create(CopyElmsFunc); // create thread as last thing
   return true;
}
void ShutCopyElms()
{
   UpdateThread  .del(); // del thread as first thing
   UpdateProgress.del();

   if(CopyElms.dest)
   {
      if(Elm *elm=CopyElms.dest->findElm(CopyElms.root))elm->opened(true);
      CopyElms.dest->updateVersion(CopyElms.src_ver, false, CopyElms.elms_copied); // update copied elements in target project

      FREPA(CopyElms.elms_replaced) // process only replaced elements, to avoid having to rebuild everything when just importing completely new elements to a project
         if(Elm *elm=CopyElms.dest->findElm(CopyElms.elms_replaced[i]))switch(elm->type)
      {
         case ELM_MESH: CopyElms.dest->meshTransformChanged(*elm, CopyElms.changed_body.binaryHas(elm->id, Compare)); break; // need to call 'meshTransformChanged' in case mesh transform was changed and need to setup transform for skeletons/anims/phys/bodies, or if we need to rebuild world embedded objects
         case ELM_SKEL: CopyElms.dest->skelTransformChanged( elm->id                                               ); break; // need to call 'skelTransformChanged' because 'meshTransformChanged' will not trigger this, since 'setSkelTransform' calls this only when it detects a transform change, however transform was already adjusted during copying
         case ELM_MTRL: CopyElms.dest->mtrlSetAutoTanBin   ( elm->id                                               ); break;
      }
   }
   CopyElms.elms_copied  .clear();
   CopyElms.elms_replaced.clear();
   CopyElms.changed_body .clear();
   CopyElms.close();

   if(!Proj.valid()) // if there was no opened project
   { // select the newly created project
      Projs.refresh(); // need to refresh first in case it wasn't listed yet
      Projs.selectProj(CopyElms.target.id);
   }

   Proj.resume();
   CompareProjs.changed(CopyElms.target.id);
   WindowSetNormal();
   WindowFlash();
}
bool UpdateCopyElms()
{
   if(Kb.bp(KB_ESC)){SetProjectState(); Gui.msgBox(S, "Copying elements breaked on user request");}
   if(!UpdateThread.active())SetProjectState();
   WindowSetProgress(UpdateProgress());
   Time.wait(1000/30);
      Gui.update();
   Server.update(null, true);
   if(Ms.bp(3))WindowToggle();
   return true;
}
void DrawCopyElms()
{
   D.clear(BackgroundColor());
   D.text(0, 0.05f, CopyElms.esenthel_project_pak.totalFiles() ? "Importing Elements" : "Copying Elements");
   GuiPC gpc;
   gpc.visible=gpc.enabled=true; 
   gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
   gpc.offset.zero();
   UpdateProgress.draw(gpc);
   D.clip();
   Gui.draw();
}
/******************************************************************************/

/******************************************************************************/
      CopyElements::ProjListElm::ProjListElm(C Projects::Elm &src) : id(UIDZero) {name=src.local_name; path=src.path; id=src.id;}
      CopyElements::ProjListElm::ProjListElm(C Project      &src) : id(UIDZero) {name=src.      name; path=src.path; id=src.id;}
         CopyElements::ReplaceElms::ElmListElm::ElmListElm(C UID &elm_id) : id(UIDZero) {T.id=elm_id; T.name=CopyElms.src->elmFullName(elm_id);}
         int CopyElements::ReplaceElms::ElmListElm::Compare(C ElmListElm &a, C ElmListElm &b) {return ComparePathNumber(a.name, b.name);}
      void CopyElements::ReplaceElms::Copy(ReplaceElms &re) {re.copyDo();}
             void CopyElements::ReplaceElms::copyDo()
      {
         hide();
         if(CopyElms.canCopy())
         {
            LOAD_RESULT result; Str error;
            if(CopyElms.target.id==Proj.id){CopyElms.dest=&Proj              ; result=LOAD_OK;}
            else                           {CopyElms.dest=&CopyElms.temp_dest; result=CopyElms.dest->open(CopyElms.target.id, CopyElms.target.name, CopyElms.target.path, error); CopyElms.root.zero();} // can't use root because we're not copying to 'Proj'

            if(result==LOAD_NEWER        )Gui.msgBox(S,   "Target project was created with a newer version of Esenthel Engine.\nPlease upgrade your Esenthel software and try again.");else
            if(result==LOAD_LOCKED       )Gui.msgBox(S,   "Target project appears to be already opened in another instance of the Editor.\nIf it isn't, then please try re-opening it manually first.");else
            if(result==LOAD_ERROR        )Gui.msgBox(S, S+"Target project failed to load."+(error.is() ? '\n' : '\0')+error);else
            if(CopyElms.dest->needUpdate())Gui.msgBox(S,   "Target project needs to be updated first.\nPlease first open the target project normally in order to update it.");else
            {
               if(result==LOAD_EMPTY)CopyElms.dest->initSettings(*CopyElms.src); // if dest project didn't exist yet, then copy all settings from source

               // remove those elements which are not selected
               list.sel.sort(Compare);
               FREPA(list) // iterate all elements
                  if(!list.sel.binaryHas(i, Compare)) // if i-th element is not selected
                     if(ElmListElm *data=list.absToData(i)) // get data of that element
               {
                                     CopyElms.elms_to_copy.binaryExclude(data->id         , Compare); // don't copy that element
                  REPA(data->children)CopyElms.elms_to_copy.binaryExclude(data->children[i], Compare); // don't copy its  children
               }

               // start copying
               StateCopyElms.set(StateFadeTime);

               return; // return so that 'close' is not called below
            }

            CopyElms.close();
         }
      }
      void CopyElements::ReplaceElms::create()
      {
         Gui+=::EE::Window::create(Rect_C(0, 0, 1.65f, 1.1f), "Replace elements").hide(); button[2].show();
         T  +=text  .create(Rect_LU(0, 0, clientWidth(), 0.19f).extend(-0.02f), "Following elements already exist in target project.\nPlease select which elements should be copied and replace those in target project.\nThe elements that are not selected will not be copied."); text.auto_line=AUTO_LINE_SPACE_SPLIT;
         T  +=copy  .create(Rect_RD(clientWidth()-0.04f, -clientHeight()+0.04f, 0.25f, 0.06f), "Copy").func(Copy, T);
         T  +=region.create(Rect(0, copy.rect().max.y, clientWidth(), text.rect().min.y).extend(-0.04f));
         ListColumn lc[]=
         {
            ListColumn(MEMBER(ElmListElm, name), LCW_DATA, "Name"),
         };
         region+=list.create(lc, Elms(lc), true); list.flag|=LIST_MULTI_SEL; list.cur_mode=LCM_ALWAYS; list.sel_mode=LSM_TOGGLE;
      }
      void CopyElements::ReplaceElms::check()
      {
         LOAD_RESULT result; Str error;
         if(CopyElms.target.id==Proj.id){         CopyElms.dest=&Proj              ; result=LOAD_OK;}
         else                           {int ver; CopyElms.dest=&CopyElms.temp_dest; result=CopyElms.dest->load3(CopyElms.target.path, ver, error);} // just load data without opening it for editing (so we don't have to close it after this stage yet)

         if(result==LOAD_NEWER        )Gui.msgBox(S,   "Target project was created with a newer version of Esenthel Engine.\nPlease upgrade your Esenthel software and try again.");else
         if(result==LOAD_LOCKED       )Gui.msgBox(S,   "Target project appears to be already opened in another instance of the Editor.\nIf it isn't, then please try re-opening it manually first.");else
         if(result==LOAD_ERROR        )Gui.msgBox(S, S+"Target project failed to load."+(error.is() ? '\n' : '\0')+error);else
         if(CopyElms.dest->needUpdate())Gui.msgBox(S,   "Target project needs to be updated first.\nPlease first open the target project normally in order to update it.");else
         {
            data.clear();
            REPA(CopyElms.elms_to_copy)
            {
               Elm *src =CopyElms.src ->findElm(CopyElms.elms_to_copy[i]),
                   *dest=CopyElms.dest->findElm(CopyElms.elms_to_copy[i]);
               if(!src || (src && dest && src->type!=dest->type))CopyElms.elms_to_copy.remove(i, true);else // if source doesn't exist for some reason, or both projects have this element but it's of different type, then delete it and don't copy
               if(src && dest)data.add(CopyElms.elms_to_copy[i]); // both exist -> display to select whether it should be copied or not
            }

            // merge hidden elements with their parent
            REPA(data)
               if(Elm *elm=CopyElms.src->findElm(data[i].id))
                  if(!ElmVisible(elm->type))
                     if(Elm *parent=CopyElms.src->firstVisibleParent(elm))
                        REPAD(p, data)
                           if(data[p].id==parent->id) // find first visible parent of that element in data container
            {
               data[p].children.add(elm->id); // move element to visible parent
               data.remove(i);               // remove element
               break;
            }

            // setup list
            data.sort(ElmListElm::Compare); list.setData(data); list.sel.clear(); FREPA(data)list.sel.add(i); // select all elements by default

            if(data.elms())activate(); // if there are elements to overwrite then display the window
            else           copyDo  (); // if all elements are new then proceed with copy
            
            return; // return so that 'close' is not called below
         }
         CopyElms.close();
      }
   void CopyElements::Refresh(CopyElements &ce) {ce.refresh();}
   void CopyElements::Copy(CopyElements &ce) {ce.copyDo ();}
   bool CopyElements::canCopy()C
   {
      if(StateActive==&StateProject || StateActive==&StateProjectList)return true;
      Gui.msgBox(S, "Copying Elements can't be performed in this Editor State");
      return false;
   }
   void CopyElements::create()
   {
      Gui+=::EE::Window::create(Rect_C(0, 0, 1.8f, 1.1f), "Copy elements").hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      T+=proj_region.create(Rect_RU(clientWidth()-0.04f, -0.10f, 0.5f, 0.82f));
      T+=elms_region.create(Rect   (0.04f, proj_region.rect().min.y, proj_region.rect().min.x-0.04f, proj_region.rect().max.y));
      T+=elms_text  .create(elms_region.rect().up()+Vec2(0, 0.04f), "Following elements will be copied");
      T+=proj_text  .create(proj_region.rect().up()+Vec2(0, 0.04f), "Select target project");
      ListColumn elms_lc[]=
      {
         ListColumn(DATA_STR, 0, SIZE(Str), LCW_DATA, "Name"),
      };
      ListColumn proj_lc[]=
      {
         ListColumn(MEMBER(ProjListElm, name), LCW_DATA, "Name"),
      };
      elms_region+=elms_list.create(elms_lc, Elms(elms_lc), true); elms_list.cur_mode=LCM_ALWAYS;
      proj_region+=proj_list.create(proj_lc, Elms(proj_lc), true); proj_list.cur_mode=LCM_ALWAYS; proj_list.sort_column[0]=0;
      T+=include_children    .create(Rect_LD(                  0.04f, -clientHeight()+0.04f  , 0.38f, 0.06f), "Include children"    ).func(Refresh, T).focusable(false).desc("If include elements which are assigned as children to selected elements."                                                                                             ); include_children    .mode=BUTTON_TOGGLE; include_children    .set(true, QUIET);
      T+=include_dependencies.create(Rect_LU(include_children    .rect().ru()+Vec2(0.03f, 0), 0.48f, 0.06f), "Include dependencies").func(Refresh, T).focusable(false).desc("If include elements on which selected elements depend on.\nFor example if a mesh is selected, then enabling this option will select all materials that the mesh uses."); include_dependencies.mode=BUTTON_TOGGLE; include_dependencies.set(true, QUIET);
      T+=include_parents     .create(Rect_LU(include_dependencies.rect().ru()+Vec2(0.03f, 0), 0.48f, 0.06f), "Include parents"     ).func(Refresh, T).focusable(false).desc("If include elements which are assigned as parents of selected elements."                                                                                              ); include_parents     .mode=BUTTON_TOGGLE;
      T+=copy                .create(Rect_RD(clientWidth()-0.04f, -clientHeight()+0.04f      , 0.25f, 0.06f), "Copy"                ).func(Copy   , T);
      replace_elms.create();
   }
   void CopyElements::floodSelected(ElmNode &node, bool parent_selected, bool parent_removed)
   {
      REPA(node.children)
      {
         int      child_i    =node.children [i];
         ElmNode &child      =src ->hierarchy[child_i];
         Elm     &elm        =src ->elms     [child_i];
         bool     elm_sel    =sel .binaryHas(elm.id, Compare),
                  elm_removed=(elm.removed() || parent_removed);

         if(elm_sel                          // if this one is specifically selected
         || parent_selected && !elm_removed) // or parent is selected and this one is not removed
            elms_to_copy.binaryInclude(elm.id, Compare);

         floodSelected(child, elm_sel || parent_selected, elm_removed);
      }
   }
   void CopyElements::includeTex(C UID &tex_id)
   {
      if(tex_id.valid())texs_to_copy.binaryInclude(tex_id, Compare);
   }
   void CopyElements::includeDep(C UID &elm_id)
   {
      if(elm_id.valid())if(processed_dep.binaryInclude(elm_id, Compare))if(Elm *elm=src->findElm(elm_id))switch(elm->type)
      {
         case ELM_APP: if(ElmApp *data=elm->appData())
         {
            if(include_dependencies()){includeDep(data->icon); includeDep(data->image_portrait); includeDep(data->image_landscape);}
         }break;

         case ELM_OBJ: if(ElmObj *data=elm->objData())
         {
            includeDep(data->mesh_id); // always include
            if(include_dependencies())includeDep(data->base_id);
         }break;

         case ELM_MESH: if(ElmMesh *data=elm->meshData())
         {
            includeDep(data-> obj_id); // always include
            includeDep(data->skel_id); // always include
            includeDep(data->phys_id); // always include
            if(include_dependencies()){includeDep(Proj.meshToObj(data->body_id)); includeDep(data->draw_group_id); REPA(data->mtrl_ids)includeDep(data->mtrl_ids[i]);}
         }break;

         case ELM_PHYS: if(ElmPhys *data=elm->physData())
         {
            if(include_dependencies())includeDep(data->mtrl_id);
         }break;

         case ELM_MTRL: if(ElmMaterial *data=elm->mtrlData())
         {
            includeTex(data->base_0_tex); includeTex(data->base_1_tex); includeTex(data->detail_tex); includeTex(data->macro_tex); includeTex(data->reflection_tex); includeTex(data->light_tex); // always include
         }break;

         case ELM_WATER_MTRL: if(ElmWaterMtrl *data=elm->waterMtrlData())
         {
            includeTex(data->base_0_tex); includeTex(data->base_1_tex); includeTex(data->reflection_tex); // always include
         }break;

         case ELM_IMAGE_ATLAS: if(ElmImageAtlas *data=elm->imageAtlasData())
         {
            if(include_dependencies())REPA(data->images)if(!data->images[i].removed)includeDep(data->images[i].id);
         }break;

         case ELM_ICON: if(ElmIcon *data=elm->iconData())
         {
            if(include_dependencies()){includeDep(data->icon_settings_id); includeDep(data->obj_id);}
         }break;

         case ELM_TEXT_STYLE: if(ElmTextStyle *data=elm->textStyleData())
         {
            if(include_dependencies())includeDep(data->font_id);
         }break;

         case ELM_PANEL_IMAGE: if(ElmPanelImage *data=elm->panelImageData())
         {
            if(include_dependencies())REPA(data->image_ids)includeDep(data->image_ids[i]);
         }break;

         case ELM_PANEL: if(ElmPanel *data=elm->panelData())
         {
            if(include_dependencies())REPA(data->image_ids)includeDep(data->image_ids[i]);
         }break;

         case ELM_GUI_SKIN: if(ElmGuiSkin *data=elm->guiSkinData())
         {
            if(include_dependencies())REPA(data->elm_ids)includeDep(data->elm_ids[i]);
         }break;

         case ELM_ENV: if(ElmEnv *data=elm->envData())
         {
            if(include_dependencies())
            {
               REPA(data->cloud_id)includeDep(data->cloud_id[i]);
               includeDep(data->sun_id); includeDep(data->skybox_id); includeDep(data->star_id);
            }
         }break;

         case ELM_WORLD: if(ElmWorld *data=elm->worldData())
         {
            if(include_dependencies())
            {
               includeDep(data->env_id);
               if(WorldVer *world_ver=src->worldVerGet(elm->id))
               {
                  // copy all existing objects
                  REPA(world_ver->obj)
                  {
                     ObjVer &obj_ver=world_ver->obj.lockedData(i); if(!obj_ver.removed())includeDep(obj_ver.elm_obj_id);
                  }

                  // copy all materials from all existing waters
                  REPA(world_ver->lakes)
                  {
                     WaterVer &water_ver=world_ver->lakes.lockedData(i); if(!water_ver.removed())
                     {
                        Lake lake; if(lake.load(src->editLakePath(elm->id, world_ver->lakes.lockedKey(i))))includeDep(lake.material);
                     }
                  }
                  REPA(world_ver->rivers)
                  {
                     WaterVer &water_ver=world_ver->rivers.lockedData(i); if(!water_ver.removed())
                     {
                        River river; if(river.load(src->editRiverPath(elm->id, world_ver->rivers.lockedKey(i))))includeDep(river.material);
                     }
                  }
               }
            }
         }break;

         case ELM_MINI_MAP: if(ElmMiniMap *data=elm->miniMapData())
         {
            if(include_dependencies()){includeDep(data->world_id); includeDep(data->env_id);}
         }break;

         case ELM_GUI: if(ElmGui *data=elm->guiData())
         {
            if(include_dependencies())
            {
               GuiObjs objs; if(objs.load(src->gamePath(*elm)))
               {
                  REP(GO_NUM)if(C Memb<const_mem_addr GuiObj> *o=objs.objects(GUI_OBJ_TYPE(i)))REPA(*o)includeDep((*o)[i]);
               }
            }
         }break;
      }
   }
   void CopyElements::includeDep(C GuiObj &obj) // here all objects should include their dependencies
   {  // don't include children because this function is already called for all elements
      switch(obj.type())
      {
         case GO_BUTTON:
         {
            includeDep(obj.asButton().image.id());
            includeDep(obj.asButton().skin .id());
         }break;

         case GO_CHECKBOX:
         {
            includeDep(obj.asCheckBox().skin.id());
         }break;

         case GO_COMBOBOX:
         {
            includeDep(obj.asComboBox().image .id());
            includeDep(obj.asComboBox().skin().id());
         }break;

         case GO_IMAGE:
         {
            includeDep(obj.asImage().image.id());
         }break;

         case GO_MENU:
         {
            includeDep(obj.asMenu().skin().id());
         }break;

         case GO_PROGRESS:
         {
            includeDep(obj.asProgress().skin.id());
         }break;

         case GO_REGION:
         {
            includeDep(obj.asRegion().skin().id());
            includeDep(obj.asRegion().view);
            REPA(obj.asRegion().slidebar)includeDep(obj.asRegion().slidebar[i]);
         }break;

         case GO_SLIDEBAR:
         {
            includeDep(obj.asSlideBar().skin().id());
            REPA(obj.asSlideBar().button)includeDep(obj.asSlideBar().button[i]);
         }break;

         case GO_SLIDER:
         {
            includeDep(obj.asSlider().skin.id());
         }break;

         case GO_TAB:
         {
            includeDep(obj.asTab().image.id());
            includeDep(obj.asTab().skin .id());
         }break;

         case GO_TABS:
         {
            includeDep(obj.asTabs().skin().id());
            REPA(obj.asTabs())includeDep(obj.asTabs().tab(i));
         }break;

         case GO_TEXT:
         {
            includeDep(obj.asText().skin.id());
            includeDep(obj.asText().text_style.id());
         }break;

         case GO_TEXTLINE:
         {
            includeDep(obj.asTextLine().reset.skin.id());
            includeDep(obj.asTextLine().skin().id());
         }break;

         case GO_WINDOW:
         {
            includeDep(obj.asWindow().skin().id());
            REPA(obj.asWindow().button)includeDep(obj.asWindow().button[i]);
         }break;
      }
   }
   void CopyElements::refresh()
   {
      elms_to_copy.clear();
      texs_to_copy.clear();

      // include children
      if(include_children())floodSelected(src->root);else elms_to_copy=sel;

      // include parents
      if(include_parents())
      {
         Memc<UID> elms=elms_to_copy;
         for(; elms.elms(); )
         {
            UID elm_id=elms.pop(); if(Elm *elm=Proj.findElm(elm_id))if(Proj.findElm(elm->parent_id))if(elms_to_copy.binaryInclude(elm->parent_id, Compare))elms.add(elm->parent_id);
         }
      }

      // include dependencies
      processed_dep.clear(); FREPA(elms_to_copy)includeDep(elms_to_copy[i]); Swap(processed_dep, elms_to_copy);

      elms_data.clear(); FREPA(elms_to_copy)if(Elm *elm=src->findElm(elms_to_copy[i]))if(ElmVisible(elm->type))elms_data.add(src->elmFullName(elm)); elms_data.sort(ComparePathNumberCI);
      elms_list.setData(elms_data);
   }
   void CopyElements::resize()
   {
              elms_list.elmHeight(Proj.list.elmHeight()).textSize(Proj.list.textSizeBase(), Proj.list.textSizeRel());
      replace_elms.list.elmHeight(elms_list.elmHeight()).textSize(elms_list.textSizeBase(), elms_list.textSizeRel());
   }
   void CopyElements::display(C Str &esenthel_project_file)
   {
      replace_elms.hide();
                   hide();
      if(canCopy())
      {
         src_ver=-1;
         root.zero(); if(ListElm *elm=Proj.list.litToListElm())if(elm->elm)root=elm->elm->id;
         if(esenthel_project_file.is())
         {
            long expected_size, actual_size;
            switch(esenthel_project_pak.loadEx(esenthel_project_file, null, 0, &expected_size, &actual_size))
            {
               case PAK_LOAD_OK                 : break;
               case PAK_LOAD_UNSUPPORTED_VERSION: Gui.msgBox(S, "Selected project was created with a newer version of the engine."); return;
               case PAK_LOAD_INCOMPLETE_DATA    : Gui.msgBox(S, S+"Selected project file is incomplete:\nFile size is: "+FileSize(actual_size)+"\nIt should be: "+FileSize(expected_size)+"\nIf downloaded from Esenthel Store, please make sure that the download was not interrupted.\nConsider downloading using Esenthel Editor which supports resuming interrupted downloads."); return;
               default                          : Gui.msgBox(S, "Can't load selected project"); return;
            }

            src=&temp_src;
            File f; f.readTry("Data", esenthel_project_pak); switch(src->load(f, src_ver)) // call 'load' always, and not only when 'readTry' succeeded, since we can't update the "EsenthelProject" file, we will update the destination project once it finished copying
            {
               case LOAD_EMPTY :
               case LOAD_ERROR :
               case LOAD_LOCKED: Gui.msgBox(S, "Invalid project file"); return;
               case LOAD_NEWER : Gui.msgBox(S, "This project was created with a newer version of Esenthel Engine.\nPlease upgrade your Esenthel software and try again."); return;
            }
            if(src_ver<=34 && f.readTry("Settings", esenthel_project_pak))src->loadOldSettings(f); // ver 34 and below had settings in a separate file
            src->setIDPath(UIDZero, S); // set paths too because for example when copying textures, then 'tex_path' is used
            src->setHierarchy();

            // select all elements and textures to be copied
            elms_to_copy.clear(); FREPA(src->elms)elms_to_copy.binaryInclude(src->elms[i].id, Compare);
            texs_to_copy.clear(); FREPA(src->texs)texs_to_copy.binaryInclude(src->texs[i]   , Compare);

            if(Proj.valid()) // if there's a project opened, then import into that one
            {
               if(Demo){Gui.msgBox(S, "Importing projects into existing projects is not available in the demo version."); return;}
               target=Proj;
            }else
            {
               target.id.randomizeValid(); // when no project is opened then always create a new one
               target.name=src->name; // re-use original name
               target.path=ProjectsPath+EncodeFileName(target.id).tailSlash(true); // path needs to be set
            }
            replace_elms.check();
         }else
         {
            esenthel_project_pak.del();
            src=&Proj; src_ver=ProjectVersion;
            Proj.setListSel(sel); sel.sort(Compare);

            refresh();

            int cur=-1;
            proj_data.clear(); FREPA(Projs.proj_data){Projects::Elm &src=Projs.proj_data[i]; if(src.id!=Proj.id){if(src.id==target.id)cur=proj_data.elms(); proj_data.add(src);}} // add all except currently opened project, if there's a project to which we've copied last time, then select it
            proj_list.setData(proj_data); proj_list.setCur(proj_list.absToVis(cur)); // !! do not merge into one instruction !!

            ::EE::GuiObj::activate();
         }
      }
   }
   void CopyElements::copyDo()
   {
      if(!elms_to_copy.elms())Gui.msgBox(S, "No elements to copy");else
      if(ProjListElm *dest=proj_list()){if(src==&Proj)REPA(elms_to_copy)Proj.flushElm(elms_to_copy[i]); hide(); target=*dest; replace_elms.check();}else
         Gui.msgBox(S, "No target project selected");
   }
   void CopyElements::drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      REPA(names)if(GetExt(names[i])==EsenthelProjectExt)
      {
         display(names[i]);
         names.remove(i, true);
         break;
      }
   }
   void CopyElements::close()
   {
      esenthel_project_pak.del();

      if(dest==&Proj)
      {
         Proj.setList();
         Proj.enumChanged(); // before 'activateSources' because this generates C++ auto header
         Proj.fontChanged();
         Proj.activateSources(CodeEdit.initialized() ? 1 : -1); // set active sources but without rebuilding so code editor doesn't need to be loaded every time (unless it's already loaded), call after 'enumChanged'
      }else if(dest)dest->close();
      dest=null;

      if(src && src!=&Proj)src->del(); src=null; // we shouldn't 'close' 'src' because it's either 'Proj' or an "EsenthelProject" file
   }
CopyElements::CopyElements() : src_ver(-1), src(null), dest(null), root(UIDZero) {}

CopyElements::ProjListElm::ProjListElm() : id(UIDZero) {}

CopyElements::ReplaceElms::ElmListElm::ElmListElm() : id(UIDZero) {}

/******************************************************************************/
