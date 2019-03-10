/******************************************************************************/
class EditorServer : Edit.EditorServer
{
   bool busy=false, received=false;
   uint start_time=0;

   static void ConvertHeight(C Heightmap &src, Image &dest, flt area_size)
   {
      dest.createSoftTry(src.resolution(), src.resolution(), 1, IMAGE_F32);
      REPD(y, dest.h())
      REPD(x, dest.w())dest.pixF(x, y)=src.height(x, y)*area_size;
   }
   static void ConvertHeight(Image &src, Heightmap &dest, flt area_size) // 'src' will get modified !!
   {
      src.resize(dest.resolution(), dest.resolution(), FILTER_BEST, true, false, true);
      REPD(y, dest.resolution())
      REPD(x, dest.resolution())dest.height(x, y, src.pixelF(x, y)/area_size);
   }
   static void ConvertColor(C Heightmap &src, Image &dest)
   {
      dest.createSoftTry(src.resolution(), src.resolution(), 1, IMAGE_R8G8B8);
      REPD(y, dest.h())
      REPD(x, dest.w())dest.color(x, y, src.color(x, y));
   }
   static void ConvertColor(Image &src, Heightmap &dest) // 'src' will get modified !!
   {
      src.resize(dest.resolution(), dest.resolution(), FILTER_BEST, true, false, true);
      REPD(y, dest.resolution())
      REPD(x, dest.resolution())dest.color(x, y, src.color(x, y));
   }
   static void ConvertMaterial(C Heightmap &src, Edit.MaterialMap &dest)
   {
      Memt<UID> materials; materials.setNum(src.materials()); REPAO(materials)=src.material(i).id(); // set material palette
      dest.create(src.resolution());
      REPD(y, dest.resolution())
      REPD(x, dest.resolution())
      {
         VecB4 m, i;
         src.getMaterial(x, y, m, i);
         dest.set(x, y, InRange(m.x, materials) ? materials[m.x] : UIDZero,
                        InRange(m.y, materials) ? materials[m.y] : UIDZero,
                        InRange(m.z, materials) ? materials[m.z] : UIDZero,
                        InRange(m.w, materials) ? materials[m.w] : UIDZero, i);
      }
   }
   static void ConvertMaterial(Edit.MaterialMap &src, Heightmap &dest) // 'src' will get modified !!
   {
      src.resize(dest.resolution());
      REPD(y, dest.resolution())
      REPD(x, dest.resolution())
      {
         UID   m[4];
         VecB4 i;
         src .get(x, y, m[0], m[1], m[2], m[3], i);
         dest.setMaterial(x, y, Proj.gamePath(m[0]), Proj.gamePath(m[1]), Proj.gamePath(m[2]), Proj.gamePath(m[3]), i);
      }
   }

   class Client : Edit.EditorServer.Client
   {
      int queued=-1; // queued command to be processed

      virtual bool update()override
      {
         if(!super.update())return false;

         int timeout=0;
      check_again:
         if(EditServer.busy)return true; // don't do anything if the editor is busy, wait until it finishes processing to update clients

         if(queued>=0) // if there was a queued command to be processed later, at this stage the 'EditServer' is no longer busy (if any publishing was requested, then it already finished)
         {
            File &f=connection.data.reset().putByte(queued);
            switch(queued)
            {
               case Edit.EI_EXPORT_APP: f.putBool(PublishOk); break; // save last publishing result
            }
            f.pos(0); connection.send(f);
            queued=-1;
         }

         if(connection_version_ok)
            if(connection.receive(timeout))
         {
            if(!EditServer.received) // if that's the first command we've received this frame
            {
               EditServer.received  =true;
               EditServer.start_time=Time.curTimeMs(); // set time at the moment of first command receival, do this before processing of the command
            }

            byte   cmd=connection.data.getByte();
            switch(cmd)
            {
               default: return false; // invalid command

               // PROJECT
               case Edit.EI_GET_PROJECTS_PATH:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_PROJECTS_PATH).putStr(ProjectsPath); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_PROJECTS_PATH:
               {
                  File &f=connection.data; Projs.proj_path.set(f.getStr());
                  f.reset().putByte(Edit.EI_SET_PROJECTS_PATH).putBool(true); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_PROJECTS:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_PROJECTS);
                  Mems<Edit.Project> projects; projects.setNum(Projs.proj_data.elms());
                  REPAO(projects).set(Projs.proj_data[i].id, Projs.proj_data[i].name);
                  projects.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_PROJECT:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_PROJECT).putUID(Proj.id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_PROJECT:
               {
                  UID proj_id; connection.data>>proj_id;
                  if( proj_id!=Proj.id)
                  {
                     if(SaveChanges(null)) // nothing to save
                     {
                        EditServer.busy=true; // set as busy, so we don't respond to this client until we finish changing projects
                        Projects.Elm *proj=Projs.findProj(proj_id);
                        if( proj)if(!Projs.open(*proj))proj=null;
                        if(!proj)StateProjectList.set(StateFadeTime);
                     }
                  }
               }break;

               // SETTINGS
               case Edit.EI_GET_DATA_PATH:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_DATA_PATH)<<Proj.game_path; f.pos(0); connection.send(f);
               }break;

               // ELMS
               case Edit.EI_GET_ELMS:
               {
                  Mems  <Edit.Elm> mems;
                //Memc  <Edit.Elm> memc;
                  MemPtr<Edit.Elm> elms;
                  if(Proj.valid())
                  {
                     const bool include_removed=true;
                     if(include_removed)
                     {
                        mems.setNum(Proj.elms.elms()); FREPA(mems)
                        {
                           Edit.Elm &elm=     mems[i];
                           Elm      &src=Proj.elms[i];
                           elm.type     =Edit.ELM_TYPE(src.type);
                           elm.removed  = src.  removed();
                           elm.publish  =!src.noPublish();
                           elm.       id= src.       id;
                           elm.parent_id= src.parent_id;
                           elm.name     = src.name;
                           elm.src_file = src.srcFile();
                        }
                        elms.point(mems);
                     }else
                     {
                      /*memc.reserve(Proj.elms.elms());
                        Proj.floodExisting(Proj.root, memc);
                        elms.point(memc); */
                     }
                  }
                  File &f=connection.data.reset().putByte(Edit.EI_GET_ELMS).putBool(elms); if(elms)elms.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_ELMS_SELECTED:
               {
                  bool ok=false;
                  Mems<UID> elms;
                  if(Proj.valid())
                  {
                     ok=true;
                     Proj.setListCurSel();
                     elms=Proj.list_sel;
                  }
                  File &f=connection.data.reset().putByte(Edit.EI_GET_ELMS_SELECTED).putBool(ok); if(ok)elms.saveRaw(f); f.pos(0);
                  connection.send(f);
               }break;

               case Edit.EI_SET_ELMS_SELECTED:
               {
                  File &f=connection.data;
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Mems<UID> elms; if(ok=elms.loadRaw(f))Proj.elmSelect(elms);
                  }
                  f.reset().putByte(Edit.EI_SET_ELMS_SELECTED).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_RLD_ELMS:
               {
                  File &f=connection.data; bool remember_result=f.getBool();
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Mems<UID> elms; if(elms.loadRaw(f))ok=Proj.elmReload(elms, remember_result);
                  }
                  f.reset().putByte(Edit.EI_RLD_ELMS).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_RLD_ELMS_CANCEL:
               {
                  File &f=connection.data;
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Mems<UID> elms; if(ok=elms.loadRaw(f))Proj.cancelReload(elms);
                  }
                  f.reset().putByte(Edit.EI_RLD_ELMS_CANCEL).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_RLD_ELMS_GET_RESULT:
               {
                  File &f=connection.data;
                  bool ok=false;
                  Mems<Edit.IDParam<Edit.RELOAD_RESULT>> results;
                  Mems<UID> elms; if(Proj.valid() && elms.loadRaw(f)){ok=true; Importer.getResult(elms, results);}
                  f.reset().putByte(Edit.EI_RLD_ELMS_GET_RESULT).putBool(ok); if(ok)results.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_RLD_ELMS_FORGET_RESULT:
               {
                  File &f=connection.data;
                  bool ok=false;
                  Mems<UID> elms; if(ok=elms.loadRaw(f))Importer.forgetResult(elms);
                  f.reset().putByte(Edit.EI_RLD_ELMS_FORGET_RESULT).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_NEW_ELM:
               {
                  File &f  =connection.data; ELM_TYPE type=ELM_TYPE(f.getByte()); Str name=f.getStr(); UID parent=f.getUID();
                  Elm  *elm=(Proj.valid() ? Proj.newElm(type, parent, &name) : null);
                  f.reset().putByte(Edit.EI_NEW_ELM).putUID(AsID(elm)).pos(0); connection.send(f);
               }break;

               case Edit.EI_NEW_WORLD:
               {
                  File &f  =connection.data; Str name=f.getStr(); int area_size=f.getInt(); int terrain_res=f.getInt(); UID parent=f.getUID();
                  Elm  *elm=(Proj.valid() ? Proj.newWorld(name, area_size, terrain_res, parent, false) : null);
                  f.reset().putByte(Edit.EI_NEW_WORLD).putUID(AsID(elm)).pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_ELM_NAME:
               {
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Memc<Edit.IDParam<Str>> elms; if(elms.load(connection.data))
                     {
                        ok=true;
                        Proj.setElmNames(elms);
                     }
                  }
                  File &f=connection.data.reset().putByte(Edit.EI_SET_ELM_NAME).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_ELM_REMOVED:
               {
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Memc<Edit.IDParam<bool>> elms; if(elms.load(connection.data))
                     {
                        ok=true;
                        Memc<UID> remove, restore; FREPA(elms)if(elms[i].value)remove.add(elms[i].id);else restore.add(elms[i].id);
                        Proj.remove (remove, false);
                        Proj.restore(restore);
                     }
                  }
                  File &f=connection.data.reset().putByte(Edit.EI_SET_ELM_REMOVED).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_ELM_PUBLISH:
               {
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Memc<Edit.IDParam<bool>> elms; if(elms.load(connection.data))
                     {
                        ok=true;
                        Memc<UID> publish, no_publish; FREPA(elms)if(elms[i].value)publish.add(elms[i].id);else no_publish.add(elms[i].id);
                        Proj.disablePublish(no_publish, false);
                        Proj. enablePublish(   publish);
                     }
                  }
                  File &f=connection.data.reset().putByte(Edit.EI_SET_ELM_PUBLISH).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_ELM_PARENT:
               {
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Memc<Edit.IDParam<UID>> elms; if(elms.load(connection.data))
                     {
                        ok=true;
                        Proj.setElmParent(elms);
                     }
                  }
                  File &f=connection.data.reset().putByte(Edit.EI_SET_ELM_PARENT).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_ELM_SRC_FILE:
               {
                  bool ok=false;
                  if(Proj.valid())
                  {
                     Memc<Edit.IDParam<Str>> elms; if(elms.load(connection.data))
                     {
                        ok=true;
                        TimeStamp time; time.getUTC();
                        FREPA(elms)if(Elm *elm=Proj.findElm(elms[i].id))
                        {
                           elm.setSrcFile(elms[i].value, time);
                           Server.setElmShort(elm.id);
                        }
                     }
                  }
                  File &f=connection.data.reset().putByte(Edit.EI_SET_ELM_SRC_FILE).putBool(ok); f.pos(0); connection.send(f);
               }break;

               // WORLD
               case Edit.EI_GET_WORLD:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_WORLD).putUID(WorldEdit.elm_id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_WORLD:
               {
                  File &f=connection.data; UID world_id=f.getUID();
                  WorldEdit.activate(Proj.findElm(world_id, ELM_WORLD));
                  f.reset().putByte(Edit.EI_SET_WORLD).putBool(WorldEdit.elm_id==world_id).pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_AREA_SIZE:
               {
                  File &f=connection.data; UID world_id=f.getUID();
                  int area_size=0; if(Elm *world=Proj.findElm(world_id, ELM_WORLD))if(ElmWorld *data=world.worldData())area_size=data.area_size;
                  f.reset().putByte(Edit.EI_GET_WORLD_AREA_SIZE).putInt(area_size); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_TERRAIN_RES:
               {
                  File &f=connection.data; UID world_id=f.getUID();
                  int terrain_res=0; if(Elm *world=Proj.findElm(world_id, ELM_WORLD))if(ElmWorld *data=world.worldData())terrain_res=data.hmRes();
                  f.reset().putByte(Edit.EI_GET_WORLD_TERRAIN_RES).putInt(terrain_res); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_TERRAIN_AREAS:
               {
                  File &f=connection.data; UID world_id=f.getUID();
                  bool ok=false; RectI areas;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true; areas=world_ver.getTerrainAreas();
                  }
                  f.reset().putByte(Edit.EI_GET_WORLD_TERRAIN_AREAS).putBool(ok); if(ok)f<<areas; f.pos(0); connection.send(f);
               }break;

               // WORLD TERRAIN
               case Edit.EI_DEL_WORLD_TERRAIN:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV();
                  bool ok=false;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                  {
                     ok=true;
                     Proj.hmDel(world_id, area_xy);
                     Synchronizer.delayedSetArea(world_id, area_xy);
                  }
                  f.reset().putByte(Edit.EI_DEL_WORLD_TERRAIN).putBool(ok).pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_TERRAIN_IS:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV();
                  bool is=false;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))is=world_ver.hasHm(area_xy);
                  f.reset().putByte(Edit.EI_GET_WORLD_TERRAIN_IS).putBool(is).pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_TERRAIN_HEIGHT:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV();
                  bool ok=false;
                  Image height;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true;
                     Heightmap temp, *hm=null;
                     if(hm=Proj.hmGet(world_id, area_xy, temp))ConvertHeight(*hm, height, world.worldData().area_size);
                  }
                  f.reset().putByte(Edit.EI_GET_WORLD_TERRAIN_HEIGHT).putBool(ok); if(ok)height.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_WORLD_TERRAIN_HEIGHT:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV(); UID material_id=f.getUID(); Image height;
                  bool ok=false;
                  if(height.load(f))
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     AreaVer *dest_area_ver=world_ver.areas.find(area_xy);
                     if(!Proj.findElm(material_id, ELM_MTRL))material_id=Proj.hm_mtrl_id; // use default material if specified doesn't exist
                     if(material_id.valid() || (dest_area_ver && dest_area_ver.hasHm())) // we have a valid material or there already exists terrain (which has material set)
                     {
                        ok=true;
                        Heightmap hm;
                        hm.create(world.worldData().hm_res, 0, Proj.gamePath(material_id), false, null, null, null, null, null, null, null, null);
                        ConvertHeight(height, hm, world.worldData().area_size);

                        AreaVer area_ver;
                        area_ver.hm_height_time.getUTC();
                        area_ver.hm_mtrl_time  =(dest_area_ver ? dest_area_ver.hm_removed_time.u : 0)+1; // don't inherit material if the heightmap is currently removed
                        area_ver.hm_color_time =(dest_area_ver ? dest_area_ver.hm_removed_time.u : 0)+1; // don't inherit color    if the heightmap is currently removed

                        if(Proj.hmUpdate(world_id, area_xy, AREA_SYNC_HM, area_ver, hm))Synchronizer.delayedSetArea(world_id, area_xy);
                     }
                  }
                  f.reset().putByte(Edit.EI_SET_WORLD_TERRAIN_HEIGHT).putBool(ok).pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_TERRAIN_COLOR:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV();
                  bool ok=false;
                  Image color;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true;
                     Heightmap temp, *hm=null;
                     if(hm=Proj.hmGet(world_id, area_xy, temp))ConvertColor(*hm, color);
                  }
                  f.reset().putByte(Edit.EI_GET_WORLD_TERRAIN_COLOR).putBool(ok); if(ok)color.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_WORLD_TERRAIN_COLOR:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV(); UID material_id=f.getUID(); Image color;
                  bool ok=false;
                  if(color.load(f))
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     AreaVer *dest_area_ver=world_ver.areas.find(area_xy);
                     if(!Proj.findElm(material_id, ELM_MTRL))material_id=Proj.hm_mtrl_id; // use default material if specified doesn't exist
                     if(material_id.valid() || (dest_area_ver && dest_area_ver.hasHm())) // we have a valid material or there already exists terrain (which has material set)
                     {
                        ok=true;
                        Heightmap hm;
                        hm.create(world.worldData().hm_res, 0, Proj.gamePath(material_id), false, null, null, null, null, null, null, null, null);
                        ConvertColor(color, hm);

                        // the destination heightmap may not exist or be removed, in that case we want to create it (height_time must be one more than dest removed_time)
                        AreaVer area_ver;
                        area_ver.hm_height_time=(dest_area_ver ? dest_area_ver.hm_removed_time.u : 0)+1;
                        area_ver.hm_mtrl_time  =(dest_area_ver ? dest_area_ver.hm_removed_time.u : 0)+1; // don't inherit material if the heightmap is currently removed
                        area_ver.hm_color_time.getUTC();

                        if(Proj.hmUpdate(world_id, area_xy, AREA_SYNC_HM, area_ver, hm))Synchronizer.delayedSetArea(world_id, area_xy);
                     }
                  }
                  f.reset().putByte(Edit.EI_SET_WORLD_TERRAIN_COLOR).putBool(ok).pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_TERRAIN_MATERIAL:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV();
                  bool ok=false;
                  Edit.MaterialMap material;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true;
                     Heightmap temp, *hm=null;
                     if(hm=Proj.hmGet(world_id, area_xy, temp))ConvertMaterial(*hm, material);
                  }
                  f.reset().putByte(Edit.EI_GET_WORLD_TERRAIN_MATERIAL).putBool(ok); if(ok)material.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_WORLD_TERRAIN_MATERIAL:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV(); Edit.MaterialMap material;
                  bool ok=false;
                  if(material.load(f))
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     AreaVer *dest_area_ver=world_ver.areas.find(area_xy);
                     ok=true;
                     Heightmap hm;
                     hm.create(world.worldData().hm_res, 0, S, false, null, null, null, null, null, null, null, null);
                     ConvertMaterial(material, hm);

                     // the destination heightmap may not exist or be removed, in that case we want to create it (height_time must be one more than dest removed_time)
                     AreaVer area_ver;
                     area_ver.hm_height_time=(dest_area_ver ? dest_area_ver.hm_removed_time.u : 0)+1;
                     area_ver.hm_mtrl_time  .getUTC();
                     area_ver.hm_color_time =(dest_area_ver ? dest_area_ver.hm_removed_time.u : 0)+1; // don't inherit color if the heightmap is currently removed

                     if(Proj.hmUpdate(world_id, area_xy, AREA_SYNC_HM, area_ver, hm))Synchronizer.delayedSetArea(world_id, area_xy);
                  }
                  f.reset().putByte(Edit.EI_SET_WORLD_TERRAIN_MATERIAL).putBool(ok).pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_TERRAIN:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV(); bool expects_color=f.getBool();
                  bool ok=false;
                  Image            height, color;
                  Edit.MaterialMap material;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true;
                     Heightmap temp, *hm=null;
                     if(hm=Proj.hmGet(world_id, area_xy, temp))
                     {
                                         ConvertHeight  (*hm, height, world.worldData().area_size);
                                         ConvertMaterial(*hm, material);
                        if(expects_color)ConvertColor   (*hm, color);
                     }
                  }
                  f.reset().putByte(Edit.EI_GET_WORLD_TERRAIN).putBool(ok); if(ok){height.save(f); material.save(f); if(expects_color)color.save(f);} f.pos(0);
                  connection.send(f);
               }break;

               case Edit.EI_SET_WORLD_TERRAIN:
               {
                  File &f=connection.data; UID world_id=f.getUID(); VecI2 area_xy; area_xy.x=f.decIntV(); area_xy.y=f.decIntV(); bool has_color=f.getBool();
                  Image height, color; Edit.MaterialMap material;
                  bool ok=false;
                  if(            height  .load(f))
                  if(            material.load(f))
                  if(has_color ? color   .load(f) : true)
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                  if(world.worldData().valid())
                  if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     AreaVer *dest_area_ver=world_ver.areas.find(area_xy);
                     ok=true;
                     Heightmap hm;
                     hm.create(world.worldData().hm_res, 0, S, false, null, null, null, null, null, null, null, null);
                                  ConvertHeight  (height  , hm, world.worldData().area_size);
                                  ConvertMaterial(material, hm);
                     if(has_color)ConvertColor   (color   , hm);

                     // the destination heightmap may not exist or be removed, in that case we want to create it (height_time must be one more than dest removed_time)
                     AreaVer area_ver; TimeStamp utc; utc.getUTC();
                     area_ver.hm_height_time=utc;
                     area_ver.hm_mtrl_time  =utc;
                     area_ver.hm_color_time =utc;

                     if(Proj.hmUpdate(world_id, area_xy, AREA_SYNC_HM, area_ver, hm))Synchronizer.delayedSetArea(world_id, area_xy);
                  }
                  f.reset().putByte(Edit.EI_SET_WORLD_TERRAIN).putBool(ok).pos(0); connection.send(f);
               }break;

               // WORLD OBJECTS
               case Edit.EI_NEW_WORLD_OBJ:
               {
                  File &f=connection.data; UID world_id=f.getUID();
                  bool ok=false;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(flt area_size=world.worldData().area_size)
                  {
                     Memc<Edit.WorldObjParams> objs; if(objs.load(f))
                     {
                        ok=true;
                        Memc<ObjData> area_objs;
                        Memc<UID    > added_objs;
                        Heightmap     hm_temp, *hm=null;
                        for(; objs.elms(); )
                        {
                           bool       have_area=false, have_hm=false;
                           VecI2      area_xy;
                           Heightmap *hm=null;
                           REPA(objs) // go from end because we're removing
                           {
                              Edit.WorldObjParams &src=objs[i];
                              VecI2 obj_area=Floor(src.matrix.pos.xz()/area_size);
                              if(!have_area){have_area=true; area_xy=obj_area;} // setup area of the first object
                              if(obj_area==area_xy) // object matches target area
                              {
                                 if(Elm *obj_elm=Proj.findElm(src.id, ELM_OBJ)) // if it's a valid ELM_OBJ
                                 {
                                    ObjData &dest=area_objs.New().create(src, Proj.edit_path);
                                    if(src.align_to_terrain || src.align_to_terrain_normal>EPS) // want to align
                                    {
                                       if(!have_hm) // load heightmap
                                       {
                                          have_hm=true;
                                          hm=Proj.hmGet(world_id, area_xy, hm_temp);
                                       }
                                       if(hm)
                                       {
                                          if(src.align_to_terrain) // set position
                                          {
                                             dest.matrix.pos.y=HmHeight(*hm, area_size, area_xy, dest.matrix.pos);
                                          }
                                          if(src.align_to_terrain_normal>EPS) // align to terrain normal
                                          {
                                             // get object mesh
                                             MeshPtr mesh;
                                             if(ElmObj *obj_data=obj_elm.objData())mesh=Proj.gamePath(obj_data.mesh_id);

                                             Vec normal=HmNormalAvg(*hm, area_size, area_xy, dest.matrix, mesh ? mesh->ext : Extent(0.5));
                                             dest.matrix.orn()*=Matrix3().setRotation(Vec(0, 1, 0), normal, Sat(src.align_to_terrain_normal));
                                          }
                                       }
                                    }
                                    added_objs.add(dest.id); // add the world object id (and not the ELM_OBJ id)
                                 }
                                 objs.remove(i); // we've processed it so remove it
                              }
                           }

                           // insert those objects
                           if(area_objs.elms())
                           {
                              Proj.syncObj(world_id, area_xy, area_objs);
                              area_objs.clear();
                           }
                        }
                        Synchronizer.delayedSetObj(world_id, added_objs);
                     }
                  }
                  f.reset().putByte(Edit.EI_NEW_WORLD_OBJ).putBool(ok).pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_OBJ_BASIC:
               {
                  File &f=connection.data; UID world_id=f.getUID(); bool limit_areas=f.getBool(); RectI areas; if(limit_areas)f>>areas; bool only_selected=f.getBool(); bool include_removed=f.getBool();
                  Memc<UID> world_obj_instance_ids; world_obj_instance_ids.loadRaw(f); world_obj_instance_ids.sort(Compare); bool limit_obj_ids=(world_obj_instance_ids.elms()>0);
                  bool ok=false;
                  Memc<Edit.WorldObjDesc> objs;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true;
                     if(only_selected)
                     {
                        if(world_id==WorldEdit.elm_id)REPA(Selection)
                        {
                           Obj &src=Selection[i];
                           if(src.area)
                           if(include_removed ? true                                              : !src.removed)
                           if(limit_areas     ? Cuts(src.area.xy, areas)                          : true        )
                           if(limit_obj_ids   ? world_obj_instance_ids.binaryHas(src.id, Compare) : true        )
                           {
                              Edit.WorldObjDesc &obj=objs.New();
                              obj.instance_id=src.id;
                              obj. elm_obj_id=src.params.base.id();
                              obj.     matrix=src.matrix;
                              obj.    removed=src.removed;
                              obj.   selected=true;
                           }
                        }
                     }else
                     {
                        MapLock ml(world_ver.obj);
                        if(limit_obj_ids)
                        {
                           FREPA(world_obj_instance_ids)
                           {
                                 C UID    &src_id=world_obj_instance_ids[i];
                              if(C ObjVer *src   =world_ver.obj.find(src_id)) // if object actually exists
                              if(include_removed ? true                     : !src.removed())
                              if(limit_areas     ? Cuts(src.area_xy, areas) : true          )
                              {
                                 Edit.WorldObjDesc &obj=objs.New();
                                 obj.instance_id=src_id;
                                 obj. elm_obj_id=src.elm_obj_id;
                                 obj.     matrix=src.matrix();
                                 obj.    removed=src.removed();
                                 obj.   selected=Selection.has(src_id);
                              }
                           }
                        }else
                        {
                           REPA(world_ver.obj)
                           {
                              C UID    &src_id=world_ver.obj.lockedKey (i);
                              C ObjVer &src   =world_ver.obj.lockedData(i);
                              if(include_removed ? true                     : !src.removed())
                              if(limit_areas     ? Cuts(src.area_xy, areas) : true          )
                              {
                                 Edit.WorldObjDesc &obj=objs.New();
                                 obj.instance_id=src_id;
                                 obj. elm_obj_id=src.elm_obj_id;
                                 obj.     matrix=src.matrix();
                                 obj.    removed=src.removed();
                                 obj.   selected=Selection.has(src_id);
                              }
                           }
                        }
                     }
                  }
                  f.reset().putByte(Edit.EI_GET_WORLD_OBJ_BASIC).putBool(ok); if(ok)objs.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_WORLD_OBJ_FULL:
               {
                  File &f=connection.data; UID world_id=f.getUID(); bool limit_areas=f.getBool(); RectI areas; if(limit_areas)f>>areas; bool only_selected=f.getBool(); bool include_removed=f.getBool(); bool include_removed_params=f.getBool();
                  Memc<UID> world_obj_instance_ids; world_obj_instance_ids.loadRaw(f); bool limit_obj_ids=(world_obj_instance_ids.elms()>0);
                  bool ok=false;
                  Memc<Edit.WorldObjData> objs;
                  if(Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true;
                     if(world_obj_instance_ids.elms()) // filter out selected objects
                     {
                        REPA(world_obj_instance_ids)
                        {
                              C UID    &src_id=world_obj_instance_ids[i];
                           if(C ObjVer *src   =world_ver.obj.find(src_id)) // if object actually exists
                           if(include_removed ? true                     : !src.removed())
                           if(limit_areas     ? Cuts(src.area_xy, areas) : true          )
                           if(only_selected   ? Selection.has(src_id)    : true          )continue;
                           world_obj_instance_ids.remove(i); // doesn't meet the criteria so remove it
                        }
                     }else // setup objects based on criteria
                     {
                        if(only_selected)
                        {
                           if(world_id==WorldEdit.elm_id)REPA(Selection)
                           {
                              Obj &src=Selection[i];
                              if(src.area)
                              if(include_removed ? true                     : !src.removed)
                              if(limit_areas     ? Cuts(src.area.xy, areas) : true        )world_obj_instance_ids.add(src.id);
                           }
                        }else
                        {
                           MapLock ml(world_ver.obj);
                           REPA(world_ver.obj)
                           {
                              C UID    &src_id=world_ver.obj.lockedKey (i);
                              C ObjVer &src   =world_ver.obj.lockedData(i);
                              if(include_removed ? true                     : !src.removed())
                              if(limit_areas     ? Cuts(src.area_xy, areas) : true          )world_obj_instance_ids.add(src_id);
                           }
                        }
                     }

                     // gather all areas
                     Memc<VecI2> areas;
                     REPA(world_obj_instance_ids)
                        if(C ObjVer *src=world_ver.obj.find(world_obj_instance_ids[i]))
                           areas.binaryInclude(src.area_xy, Compare);
                     world_obj_instance_ids.sort(Compare); // sort obj id's because that's required for the 'objGet' below
                     Memc<ObjData> area_objs;
                     FREPA(areas)
                     {
                        Proj.objGet(world_id, areas[i], world_obj_instance_ids, area_objs);
                        FREPA(area_objs)
                        {
                                   C ObjData &src=area_objs[i];
                           Edit.WorldObjData &obj=objs.New();
                           obj.instance_id=src.id;
                           obj. elm_obj_id=src.params.base.id();
                           obj.     matrix=src.matrix;
                           obj.    removed=src.removed;
                           obj.   selected=Selection.has(src.id);
                           src.params.copyTo(obj, include_removed_params);
                        }
                        area_objs.clear();
                     }
                  }
                  f.reset().putByte(Edit.EI_GET_WORLD_OBJ_FULL).putBool(ok); if(ok)objs.save(f); f.pos(0); connection.send(f);
               }break;

               // WORLD WAYPOINTS
               case Edit.EI_GET_WORLD_WAYPOINT_LIST:
               {
                  File &f=connection.data; UID world_id=f.getUID();
                  f.reset().putByte(Edit.EI_GET_WORLD_WAYPOINT_LIST);
                  bool ok=false; Memc<UID> waypoints;
                  if(C Elm *world=Proj.findElm(world_id, ELM_WORLD))
                     if(world.worldData().valid())
                     if(C WorldVer *world_ver=Proj.worldVerGet(world_id))
                  {
                     ok=true;
                     FREPA(world_ver.waypoints)if(Proj.waypointExists(world_id, world_ver.waypoints.lockedKey(i)))waypoints.add(world_ver.waypoints.lockedKey(i));
                  }
                  f.putBool(ok); if(ok)waypoints.saveRaw(f); f.pos(0); connection.send(f);
               }break;

               // WORLD CAMERA
               case Edit.EI_GET_WORLD_CAM:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_WORLD_CAM); WorldEdit.v4.view[Edit.Viewport4.VIEW_FRONT].camera.save(f); f.pos(0);
                  connection.send(f);
               }break;

               case Edit.EI_SET_WORLD_CAM:
               {
                  File &f=connection.data; Camera cam; bool ok=cam.load(f); if(ok)WorldEdit.v4.view[Edit.Viewport4.VIEW_FRONT].camera=cam;
                  f.reset().putByte(Edit.EI_SET_WORLD_CAM).putBool(ok).pos(0); connection.send(f);
               }break;

               // WORLD DRAW
               case Edit.EI_DRAW_WORLD_LINES:
               {
                  File &f=connection.data; bool ok=WorldEdit.lines.load(f);
                  f.reset().putByte(Edit.EI_DRAW_WORLD_LINES).putBool(ok).pos(0); connection.send(f);
               }break;

               // IMAGE
               case Edit.EI_GET_IMAGE:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  Image image; bool ok=Proj.imageGet(elm_id, image);
                  f.reset().putByte(Edit.EI_GET_IMAGE).putBool(ok); if(ok)image.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_IMAGE:
               {
                  File &f=connection.data; UID elm_id=f.getUID(); Image image;
                  bool ok=false; if(image.load(f))if(Proj.imageSet(elm_id, image))ok=true;
                  f.reset().putByte(Edit.EI_SET_IMAGE).putBool(ok).pos(0); connection.send(f);
               }break;

               // CODE
               case Edit.EI_GET_CODE:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  Str code; bool ok=Proj.codeGet(elm_id, code);
                  f.reset().putByte(Edit.EI_GET_CODE).putBool(ok); if(ok)code.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_CODE:
               {
                  File &f=connection.data; UID elm_id=f.getUID(); Str code=f.getStr();
                  bool ok=Proj.codeSet(elm_id, code);
                  f.reset().putByte(Edit.EI_SET_CODE).putBool(ok).pos(0); connection.send(f);
               }break;

               case Edit.EI_CODE_SYNC_IMPORT:
               {
                  File &f=connection.data;
                  f.reset().putByte(Edit.EI_CODE_SYNC_IMPORT).putBool(Proj.valid() && Proj.codeImport(false)).pos(0); connection.send(f);
               }break;

               case Edit.EI_CODE_SYNC_EXPORT:
               {
                  File &f=connection.data;
                  f.reset().putByte(Edit.EI_CODE_SYNC_EXPORT).putBool(Proj.valid() && Proj.codeExport(false)).pos(0); connection.send(f);
               }break;

               // FILE
               case Edit.EI_GET_FILE:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  File data; bool ok=Proj.fileRead(elm_id, data);
                  f.reset().putByte(Edit.EI_GET_FILE).putBool(ok); if(ok)data.copy(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_FILE:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  bool ok=Proj.fileSet(elm_id, f);
                  f.reset().putByte(Edit.EI_SET_FILE).putBool(ok).pos(0); connection.send(f);
               }break;
               
               // MTRL
               case Edit.EI_GET_MTRL_CUR:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_MTRL_CUR).putUID(MtrlEdit.elm_id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_MTRL_CUR:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  MtrlEdit.activate(Proj.findElm(elm_id));
                  f.reset().putByte(Edit.EI_SET_MTRL_CUR).putBool(MtrlEdit.elm_id==elm_id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_MTRL:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  EditMaterial mtrl; bool ok=Proj.mtrlGet(elm_id, mtrl);
                  f.reset().putByte(Edit.EI_GET_MTRL).putBool(ok); if(ok){Edit.Material edit_mtrl; mtrl.copyTo(edit_mtrl); edit_mtrl.save(f);} f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_MTRL:
               {
                  File &f=connection.data; UID elm_id=f.getUID(); byte set=f.getByte(); Edit.Material mtrl; bool ok=mtrl.load(f);
                  if(ok)
                  {
                     bool reload_textures=FlagTest(set, 1), adjust_params=FlagTest(set, 2);
                     ok=Proj.mtrlSync(elm_id, mtrl, reload_textures, adjust_params);
                  }
                  f.reset().putByte(Edit.EI_SET_MTRL).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_RLD_MTRL_TEX:
               {
                  File &f=connection.data; UID elm_id=f.getUID(); byte texs=f.getByte();
                  bool base=FlagTest(texs, 1), reflection=FlagTest(texs, 2), detail=FlagTest(texs, 4), macro=FlagTest(texs, 8), light=FlagTest(texs, 16);
                  bool ok=Proj.mtrlReloadTextures(elm_id, base, reflection, detail, macro, light);
                  f.reset().putByte(Edit.EI_RLD_MTRL_TEX).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_MUL_MTRL_TEX_COL:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  bool ok=Proj.mtrlMulTexCol(elm_id);
                  f.reset().putByte(Edit.EI_MUL_MTRL_TEX_COL).putBool(ok); f.pos(0); connection.send(f);
               }break;

               // MESH
               case Edit.EI_GET_MESH:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  bool ok=false; Matrix matrix; File data;
                  if(Elm *elm=Proj.findElm(elm_id))
                  {
                     if(elm.type==ELM_OBJ){ok=true; matrix.identity(); elm=Proj.objToMeshElm(elm);} // if this is an object, then set ok to true, in case it has no mesh, we will just send empty data
                     if(elm && elm.type==ELM_MESH)if(ok=data.readTry(Proj.editPath(*elm)))matrix=elm.meshData().transform();
                  }
                  f.reset().putByte(Edit.EI_GET_MESH).putBool(ok); if(ok){f<<matrix; data.copy(f);} f.pos(0); connection.send(f);
               }break;
               
               case Edit.EI_SET_MESH:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  bool ok=Proj.meshSet(elm_id, f);
                  f.reset().putByte(Edit.EI_SET_MESH).putBool(ok).pos(0); connection.send(f);
               }break;

               // ANIM
               case Edit.EI_GET_ANIM_CUR:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_ANIM_CUR).putUID(AnimEdit.elm_id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_ANIM_CUR:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  AnimEdit.activate(Proj.findElm(elm_id));
                  f.reset().putByte(Edit.EI_SET_ANIM_CUR).putBool(AnimEdit.elm_id==elm_id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_ANIM:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  Animation anim; bool ok=Proj.animGet(elm_id, anim);
                  f.reset().putByte(Edit.EI_GET_ANIM).putBool(ok); if(ok)anim.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_ANIM:
               {
                  File &f=connection.data; UID elm_id=f.getUID(); Animation anim;
                  bool ok=false; if(anim.load(f))ok=Proj.animSet(elm_id, anim);
                  f.reset().putByte(Edit.EI_SET_ANIM).putBool(ok).pos(0); connection.send(f);
               }break;

               // OBJ
               case Edit.EI_GET_OBJ_CUR:
               {
                  File &f=connection.data.reset().putByte(Edit.EI_GET_OBJ_CUR).putUID(ObjEdit.obj_id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_SET_OBJ_CUR:
               {
                  File &f=connection.data; UID elm_id=f.getUID();
                  Elm *elm=Proj.findElm(elm_id);
                  if(elm)switch(elm.type)
                  {
                     case ELM_MESH: elm=Proj.meshToObjElm(elm); break;
                     case ELM_SKEL: elm=Proj.skelToObjElm(elm); break;
                  }
                  ObjEdit.activate(elm);
                  f.reset().putByte(Edit.EI_SET_OBJ_CUR).putBool(ObjEdit.obj_id==elm_id); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_GET_OBJ:
               {
                  File &f=connection.data; UID elm_id=f.getUID(); bool include_removed_params=f.getBool();
                  bool ok=false; Edit.ObjData data;
                  if(Elm *elm=Proj.findElm(elm_id))
                  if(elm.type==ELM_OBJ || elm.type==ELM_OBJ_CLASS)
                  if(EditObjectPtr params=Proj.editPath(elm_id))
                  {
                     ok=true;
                     params->copyTo(data, include_removed_params);
                  }
                  f.reset().putByte(Edit.EI_GET_OBJ).putBool(ok); if(ok)data.save(f); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_MODIFY_OBJ:
               {
                  File &f=connection.data; UID elm_id=f.getUID(); Memc<Edit.ObjChange> changes; changes.load(f, Proj.game_path);
                  bool ok=Proj.modifyObj(elm_id, changes);
                  f.reset().putByte(Edit.EI_MODIFY_OBJ).putBool(ok); f.pos(0); connection.send(f);
               }break;

               // APP
               case Edit.EI_SET_ACTIVE_APP:
               {
                  File &f=connection.data; bool ok=Proj.activateApp(f.getUID());
                  f.reset().putByte(Edit.EI_SET_ACTIVE_APP).putBool(ok); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_EXPORT_APP:
               {
                  File &f=connection.data; Edit.EXPORT_MODE mode=Edit.EXPORT_MODE(f.getByte()); bool data=f.getBool(); f.reset(); bool ok=CodeEdit.Export(mode, data);
                  if(ok && StateNext==&StatePublish) // if export request initiated publishing state, then don't reply now, but wait until publishing finishes
                  {
                     queued=cmd; return true; // queue this command to be processed ASAP, return and don't process any other commands, because we still haven't replied to this command yet
                  }
                  f.putByte(Edit.EI_EXPORT_APP).putBool(ok); f.pos(0); connection.send(f);
               }break;
               
               // BUILD SETTINGS
               case Edit.EI_BUILD_DEBUG:
               {
                  File &f=connection.data; CodeEdit.configDebug(f.getBool());
                  f.reset().putByte(Edit.EI_BUILD_DEBUG).putBool(true); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_BUILD_32BIT:
               {
                  File &f=connection.data; CodeEdit.config32Bit(f.getBool());
                  f.reset().putByte(Edit.EI_BUILD_32BIT).putBool(true); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_BUILD_DX9:
               {
                  File &f=connection.data; CodeEdit.configDX9(f.getBool());
                  f.reset().putByte(Edit.EI_BUILD_DX9).putBool(true); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_BUILD_EXE:
               {
                  File &f=connection.data; CodeEdit.configEXE(Edit.EXE_TYPE(f.getByte()));
                  f.reset().putByte(Edit.EI_BUILD_EXE).putBool(true); f.pos(0); connection.send(f);
               }break;

               case Edit.EI_BUILD_PATHS:
               {
                  File &f=connection.data; CodeEdit.exportPaths(f.getBool());
                  f.reset().putByte(Edit.EI_BUILD_PATHS).putBool(true); f.pos(0); connection.send(f);
               }break;
            }

            // try to process a few commands at the same time
            {
               timeout=(EditServer.start_time+(App.active() ? 2 : 32))-Time.curTimeMs(); // if is active then wait only up to 2 ms (1st in case we're at the end of that ms, and 2nd to actually have 1 left, otherwise do 32 ms wait for 30 fps)
               if(timeout>=0)goto check_again;
            }
         }
         return true;
      }
   }

   void update(bool busy)
   {
      T.busy    =busy;
      T.received=false;
      super.update();
      if(!received && !busy && !App.active())Time.wait(10);
   }

   EditorServer() {clients.replaceClass<Client>();}
}
EditorServer EditServer;
/******************************************************************************/
