/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
CreateMaterials CreateMtrls;
State           CreateMaterialsState(UpdateCreateMaterials, DrawCreateMaterials, InitCreateMaterials, ShutCreateMaterials);
/******************************************************************************/
bool InitCreateMaterials()
{
   SetKbExclusive();
   Proj.pause();
   UpdateProgress.create(Rect_C(0, -0.05f, 1, 0.045f));
   REPA(CreateMtrls.data)WorkerThreads.queue(CreateMtrls.data[i], CreateMtrls.Process);
   return true;
}
void ShutCreateMaterials()
{
   REPA(CreateMtrls.data)WorkerThreads.cancel(CreateMtrls.data[i], CreateMtrls.Process);
   REPA(CreateMtrls.data)WorkerThreads.wait  (CreateMtrls.data[i], CreateMtrls.Process);
   UpdateProgress.del();
   Proj.refresh().resume();
   WindowSetNormal();
   WindowFlash();
}
/******************************************************************************/
bool UpdateCreateMaterials()
{
   if(Kb.bp(KB_ESC)){SetProjectState(); Gui.msgBox(S, "Creating Materials breaked on user request");}
   if(!WorkerThreads.busy())SetProjectState();

   UpdateProgress.set(CreateMtrls.data.elms()-WorkerThreads.queued(), CreateMtrls.data.elms());
   WindowSetProgress(UpdateProgress());
   Time.wait(1000/30);
     //Gui.update(); this may cause conflicts with 'Proj.elmChanged'
    Server.update(null, true);
   if(Ms.bp(3))WindowToggle();
   return true;
}
/******************************************************************************/
void DrawCreateMaterials()
{
   D.clear(BackgroundColor());
   D.text(0, 0.05f, "Creating Materials");
   GuiPC gpc;
   gpc.visible=gpc.enabled=true; 
   gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
   gpc.offset.zero();
   UpdateProgress.draw(gpc);
   D.clip();
}
/******************************************************************************/

/******************************************************************************/
      ::CreateMaterials::Src& CreateMaterials::Src::set(C UID &id  ) {T.name=Proj.elmFullName(id); T.id  =id  ; return T;}
      ::CreateMaterials::Src& CreateMaterials::Src::set(C Str &file) {T.name=file                ; T.file=file; return T;}
   int CreateMaterials::CompareSrcPath(C Src &a, C Src &b) {return ComparePathNumber(a.name, b.name);}
   int CreateMaterials::CompareSrcID(C Src &a, C Src &b)
   {
      if(int c=Compare(a.id, b.id))return c;
      return CompareSrcPath(a, b);
   }
   void CreateMaterials::Create(CreateMaterials &cm) {cm.createDo();}
          void CreateMaterials::createDo()
   {
      if(data.elms())
      {
         if(parent_id.fromText(path))path.clear();else{path.tailSlash(true); parent_id.zero();}

         shared_path.clear();
         bool has_shared_path=false;
         REPA(data)
         {
          C Str &file=data[i].file; if(file.is())
            {
               if(has_shared_path)
               {
                  for(; shared_path.is() && !StartsPath(file, shared_path); )shared_path=GetPath(shared_path);
               }else
               {
                  shared_path=GetPath(file); has_shared_path=true;
               }
            }
         }

         CreateMaterialsState.set(StateFadeTime);
      }
   }
   void CreateMaterials::Process(Src &src, ptr user, int thread_index) {CreateMtrls.process(src);}
          void CreateMaterials::process(Src &src)
   {
      ThreadMayUseGPUData();

      EditMaterial edit; edit.newData();
      edit.color_map=(src.id.valid() ? S+EncodeFileName(src.id) : CodeEdit.importPaths(src.file));
      edit.color_map_time.getUTC();

      SyncLockerEx locker(lock);
      Image base_0, base_1;
      uint  bt=Proj.createBaseTextures(base_0, base_1, edit, false);
      locker.off();
      if(base_0.is() && !base_1.is())
      {
         if(bt&BT_ALPHA) // if we ended up having an alpha texture, then set alpha-test params
         {
            edit.tech=MTECH_ALPHA_TEST;
            edit.color.w=0.5f;
         }

         // save texture
         IMAGE_TYPE ct; ImageProps(base_0, &edit.base_0_tex, &ct, ForceHQMtrlBase0 ? FORCE_HQ : 0);

         locker.on();
         if(Proj.includeTex(edit.base_0_tex)){locker.off(); base_0.copyTry(base_0, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, false); Proj.saveTex(base_0, edit.base_0_tex);}

         // save materials
         Str name=GetBase(src.name); if(src.file.is())name=GetExtNot(name); // if this is from a file, then eat the extension

         locker.on();
         Elm *src_elm=Proj.findElm(src.id);
         Elm &elm=Proj.Project::newElm(name, src_elm ? src_elm->parent_id : Proj.getPathID(path+SkipStartPath(GetPath(src.file), shared_path), parent_id), ELM_MTRL); // if creating from element then create in elements place
         locker.off();

         Save(edit, Proj.editPath(elm));
         if(ElmMaterial *data=elm.mtrlData()){data->newData(); data->from(edit);}
         Proj.makeGameVer(elm);

         // we don't need to call Server send mtrl elements, because materials are created while the Server connection is paused, it will be restarted once creation finished, and auto-sync will start
      }
      ThreadFinishedUsingGPUData();
   }
   void CreateMaterials::clearProj()
   {
      path.clear();
      data.clear();
      list.clear();
      toGui();
   }
   void CreateMaterials::create()
   {
      add("Create Materials in following Project Location:", MEMBER(CreateMaterials, path)).elmType(ELM_ANY);
      Rect r=::PropWin::create("Create Materials", Vec2(0.03f, -0.1f), 0.045f, 0.05f, 0.7f); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      autoData(this);
      rect(Rect_C(0, 0, 1.8f, 1.3f));
      T+=text.create(Vec2(clientWidth()/2, -0.05f), "Drag and Drop Image Elements and Image Files to create Materials from them:");
      T+=create_button.create(Rect_D(clientWidth()/2, -clientHeight()+0.03f, 0.3f, 0.06f), "Create").func(Create, T).desc("Create Materials from given Images");

      T+=region.create(Rect(0, create_button.rect().max.y, clientWidth(), r.min.y).extend(-0.03f));
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Src, name), LCW_DATA, "Name"),
      };
      region+=list.create(lc, Elms(lc), true).elmHeight(0.038f).textSize(0, 1); FlagDisable(list.flag, LIST_SORTABLE); list.cur_mode=LCM_MOUSE;
   }
   CreateMaterials& CreateMaterials::show()
{
      if(hidden())
      {
         clearProj();
         ::EE::Window::show();
      }
      return T;
   }
   void CreateMaterials::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(visible() && gpc.visible)
      {
         if(Ms.tappedFirst(0) && Gui.ms()==&list)if(Src *src=list())if(Elm *elm=Proj.findElm(src->id))ImageEdit.toggle(elm);
      }
   }
   void CreateMaterials::drop(C Str &name)
   {
      switch(FileInfoSystem(name).type)
      {
         case FSTD_FILE:
         {
            if(ExtType(GetExt(name))==EXT_IMAGE)data.binaryInclude(Src().set(name), CompareSrcID);
         }break;
         
         case FSTD_DIR:
         {
            for(FileFind ff(name); ff(); )drop(ff.pathName());
         }break;
      }
   }
   void CreateMaterials::drop(C MemPtr<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj))
      {
         data.sort(CompareSrcID); REPA(names)drop(names[i]);
         data.sort(CompareSrcPath);
         list.setData(data);
      }
   }
   void CreateMaterials::drag(C MemPtr<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj))
      {
         REPA(props)if(props[i].contains(obj))return; // ignore when dragging on properties

         data.sort(CompareSrcID); REPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_IMAGE))data.binaryInclude(Src().set(elm->id), CompareSrcID);
         data.sort(CompareSrcPath);
         list.setData(data);
      }
   }
CreateMaterials::CreateMaterials() : parent_id(UIDZero) {}

CreateMaterials::Src::Src() : id(UIDZero) {}

/******************************************************************************/
