/******************************************************************************/
class SizeStatistics : ClosableWindow
{
   class ElmListElm // project element as list element
   {
      bool     proj_elm=true; // if this is a project element or some other file
      cchar8  *type_name=null;
      Str      name, path;
      UID      id=UIDZero;
      ELM_TYPE type=ELM_NONE;
      uint     size, size_compressed;

      Str finalName()C {return proj_elm ? Proj.elmFullName(id) : name;}

      void setElm(C UID &id, uint size, uint size_compressed)
      {
         T.id=id; T.size=size; T.size_compressed=size_compressed; 
         if(Elm *elm=Proj.findElm(id))type_name=ElmTypeName[T.type=elm.type];
      }
      void setTex(C Str &name, C Str &path, uint size, uint size_compressed)
      {
         T.proj_elm=false;
         T.name=S+"Texture \""+name+'"';
         T.path=path;
         T.id=FileNameID(name);
         T.size=size; T.size_compressed=size_compressed;
         T.type_name="Texture";
         T.type=ELM_IMAGE;
      }

      static Str FinalName(C ElmListElm &elm) {return elm.finalName();}
      static Str Size     (C ElmListElm &elm) {return FileSize(elm.size);}
      static Str SizeCmpr (C ElmListElm &elm) {return FileSize(elm.size_compressed);}
   }

   Memc<ElmListElm> data;
   List<ElmListElm> list;
   Menu             menu;
   Memc<UID>        menu_elm;
   Region           region;
   ViewportSkin     preview;
   MemberDesc       sort_size[2];
   Str              path;
   Pak              pak;
   Image            image;
   UID              image_id=UIDZero;
   int              packed_col=-1;

   static void SelectSel(SizeStatistics &ss) {Proj.elmSelect(ss.menu_elm);}

   static void DrawPreview(Viewport &viewport) {SizeStats.drawPreview();}
          void drawPreview()
   {
      if(ElmListElm *list_elm=list())
      {
         if(list_elm.type==ELM_IMAGE) // for images and textures read them directly from the data
         {
            // load
            if(image_id!=list_elm.id)
            {
               image_id=list_elm.id;
               Str name=(list_elm.path.is() ? list_elm.path : path+EncodeFileName(image_id));
               File f; if(pak.totalFiles())f.readTry(name, pak);else f.readTry(name);
               image.load(f);
            }

            // draw
            ALPHA_MODE alpha=D.alpha();
            if(!list_elm.proj_elm)D.alpha(ALPHA_NONE); // disable alpha blending for textures
            Rect rect=image.fit(D.viewRect());
            if(image.mode()==IMAGE_CUBE)image.drawCubeFace(WHITE, TRANSPARENT, rect, DIR_FORWARD);
            else                        image.draw        (rect);
            D.alpha(alpha);
            TextStyleParams ts; ts.align.set(1, 1); D.text(ts, D.viewRect().ld(), S+image.w()+'x'+image.h()+' '+ImageTI[image.type()].name+", "+image.mipMaps()+" MipMap"+CountS(image.mipMaps()));
         }else // other elements preview as normal for simplification
         if(list_elm.proj_elm)if(Elm *elm=Proj.findElm(list_elm.id))Preview.draw(*elm, D.viewRect());
      }
   }

   void release()
   {
      list .clear();
      data .clear();
      path .clear();
      pak  .del  ();
      image.del  ();
      image_id.zero();
   }
   virtual SizeStatistics& del ()override {release(); super.del (); return T;}
   virtual SizeStatistics& hide()override {release(); super.hide(); return T;}

   virtual SizeStatistics& rect(C Rect &rect)override
   {
      super.rect(rect);
      flt p=0.02;
      region .rect(Rect(p, -clientHeight()+p, (clientWidth()-p)/2, -p));
      preview.rect(Rect((clientWidth()+p)/2, -clientHeight()+p, clientWidth()-p, -p));
      return T;
   }
   void create()
   {
      ListColumn lc[]=
      {
         ListColumn(ElmListElm.FinalName         , 0.75, "Name"  ), // 0
         ListColumn(MEMBER(ElmListElm, type_name), 0.22, "Type"  ), // 1
         ListColumn(ElmListElm.Size              , 0.15, "Size"  ), // 2
         ListColumn(ElmListElm.SizeCmpr          , 0.15, "Packed"), // 3
      }; lc[2].sort=&sort_size[0]; lc[3].sort=&sort_size[1]; packed_col=3;
      sort_size[0]=MEMBER(ElmListElm, size);
      sort_size[1]=MEMBER(ElmListElm, size_compressed);

      Gui+=super.create("Project Data Size Statistics").hide(); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=region .create();
      T+=preview.create(DrawPreview);
      region+=list.create(lc, Elms(lc)).elmHeight(0.038).textSize(0, 1); list.flag|=LIST_RESIZABLE_COLUMNS|LIST_MULTI_SEL; list.cur_mode=LCM_ALWAYS;
      list.sort_column[0]=3; list.sort_column[1]=2; list.sort_swap[0]=list.sort_swap[1]=true; // sort by compressed size (from biggest to smallest)
      rect(Rect_C(0, 0, 2.70, 1.8));
   }
   void refresh(C Str &path, Cipher *cipher)
   {
      release();
      if(path.is())
      {
         FileInfoSystem fi(path);
         if(fi.type==FSTD_FILE) // PAK file
         {
            if(pak.load(path, cipher))
            {
               REP(pak.rootFiles())
               {
                C PakFile &pf=pak.file(i);
                  UID elm_id=FileNameID(pf.name);
                  if(elm_id.valid())data.New().setElm(elm_id, pak.totalSize(pf, false), pak.totalSize(pf, true));
               }
               if(C PakFile *pf=pak.find("Tex", false))REP(pf.children_num)
               {
                C PakFile &tex=pak.file(pf.children_offset+i);
                  data.New().setTex(tex.name, pak.fullName(tex), tex.data_size, tex.data_size_compressed);
               }
            }
         }else
         if(fi.type==FSTD_DIR) // DIR of separate files
         {
            T.path=path; T.path.tailSlash(true);
            for(FileFind ff(path); ff(); )
            {
               UID elm_id=FileNameID(ff.name);
               if( elm_id.valid())
               {
                  if(ff.type==FSTD_DIR)ff.size=FSize(ff.pathName());
                  data.New().setElm(elm_id, ff.size, ff.size);
               }
            }
            for(FileFind ff(Str(path).tailSlash(true)+"Tex"); ff(); )
            {
               data.New().setTex(ff.name, ff.pathName(), ff.size, ff.size);
            }
         }
      }else // set from Project
      {
         T.path=Proj.game_path; T.path.tailSlash(true);
         Memb<PakFileData> files; GetPublishFiles(files);
         FREPA(files)
         {
          C PakFileData &pfd=files[i];
            UID elm_id=FileNameID(pfd.name);
            Str start=GetStart(pfd.name);
            if( start=="Tex")
            {
               // !! this can be a dynamic texture which doesn't exist in 'texPath', if it wasn't yet created, then the size may be 0 and preview not available !!
               // !! this can also be a texture that was requested to be downsampled, but hasn't been yet, then the size may be 0 and preview not available !!
               Str  path=pfd.data.name; if(!FExist(path) && elm_id.valid())path=Proj.texPath(elm_id);
               long size=FSize(path);
               data.New().setTex(GetBase(pfd.name), path, size, size);
            }else
            if(start.length()==pfd.name.length() && elm_id.valid())
            {
               long size=FSize(Proj.gamePath(elm_id));
               data.New().setElm(elm_id, size, size);
            }
         }
      }
      list.setData(data);

      // detect if we need to display the packed column
      bool packed=false; REPA(data)if(data[i].size!=data[i].size_compressed){packed=true; break;}
      list.columnVisible(packed_col, packed);
   }
   void display(C Str &path, Cipher *cipher=null)
   {
      refresh(path, cipher);
      activate();
   }
   void displayUnusedMaterials()
   {
      release();
      T.path=Proj.game_path; T.path.tailSlash(true); // set from Project
      Memc<UID> used_mtrls; Proj.getUsedMaterials(used_mtrls, true);
      REPA(Proj.elms)
      {
         Elm &elm=Proj.elms[i]; if(elm.finalPublish())switch(elm.type)
         {
            case ELM_MTRL:
            {
               if(!used_mtrls.binaryHas(elm.id, Compare))
               {
                  long size=FSize(Proj.gamePath(elm.id));
                  data.New().setElm(elm.id, size, size);
               }
            }break;
         }
      }
      list.setData(data);
      list.columnVisible(packed_col, false); // hide packed column
      activate();
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         if(Ms.bd(       0) && Gui.ms()==&list
         || Kb.k (KB_ENTER) && Gui.kb()==&list)
            if(ElmListElm *list_elm=list())
         {
            Kb.eatKey();
            if(list_elm.proj_elm)
            {
               if(Elm *elm=Proj.findElm(list_elm.id))
               {
                  if(!ElmVisible(elm.type))elm=Proj.firstVisibleParent(elm);
                  Proj.elmToggle(elm);
               }
            }else 
            if(list_elm.type==ELM_IMAGE) // texture
            {
               Proj.elmToggle(Proj.findElmByTexture(list_elm.id));
            }
         }
         if(Ms.bp(1) && Gui.ms()==&list)
         {
            if(!list.sel.has(list.visToAbs(list.lit)))list.setCur(list.lit);
            if( list.sel.elms())
            {
               menu_elm.clear(); FREPA(list.sel)if(ElmListElm *elm=list.absToData(list.sel[i]))if(elm.proj_elm)menu_elm.add(elm.id); // add in order
               Node<MenuElm> n;
               n.New().create("Select Picked").func(SelectSel, T).desc("This option will select the Project Elements which are picked on this list");
               Gui+=menu.create(n);
               menu.activate().posRU(Ms.pos());
            }
         }
      }
   }
}
SizeStatistics SizeStats;
/******************************************************************************/
