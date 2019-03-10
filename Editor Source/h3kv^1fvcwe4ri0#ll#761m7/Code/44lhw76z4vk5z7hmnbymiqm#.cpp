/******************************************************************************/
class ImageAtlasEditor : PropWin
{
   class Change : Edit._Undo.Change
   {
      ElmImageAtlas data;

      virtual void create(ptr user)override
      {
         if(ElmImageAtlas *data=ImageAtlasEdit.data())
         {
            T.data=*data;
            ImageAtlasEdit.undoVis();
         }
      }
      virtual void apply(ptr user)override
      {
         if(ElmImageAtlas *data=ImageAtlasEdit.data())
         {
            data.undo(T.data);
            ImageAtlasEdit.setChanged();
            ImageAtlasEdit.toGui();
            ImageAtlasEdit.undoVis();
         }
      }
   }
   class ListElm
   {
      bool   removed=false;
      Color  color=BLACK;
      UID    img_id=UIDZero;
      Button remove;

      void setColor() {color=GuiListTextColor(); if(removed)color.a/=2;}
      void operator=(C ElmImageAtlas.Img &src) {img_id=src.id; removed=src.removed; setColor();}

      static int ComparePath(C ListElm &a, C ListElm &b) {return .ComparePathNumber(ElmName(a), ElmName(b));}
   }

   Region        region;
   Memx<ListElm> list_data; // Memx because of Button
   List<ListElm> list;
   Button        make, undo, redo, locate, show_removed;
   UID           elm_id=UIDZero;
   Elm          *elm=null;
   bool          changed=false, changed_file=false;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   static void ShowRemoved(ImageAtlasEditor &iae) {iae.toGui ();}
   static void Make       (ImageAtlasEditor &iae) {iae.makeDo();}

   static Str ElmFullName(C ListElm &le) {return      Proj.elmFullName(le.img_id);}
   static Str ElmName    (C ListElm &le) {if(Elm *elm=Proj.findElm    (le.img_id))return elm.name; return UnknownName;}

   static int CompareSource(C ImageAtlas.Source &a, C ImageAtlas.Source &b) {return ComparePathNumber(a.name, b.name);}

   static void MipMaps(  ImageAtlasEditor &iae, C Str &t) {if(ElmImageAtlas *d=iae.data()){iae.undos.set("mms"); d.mip_maps=TextBool(t); d.mip_maps_time.getUTC(); iae.setChanged();}}
   static Str  MipMaps(C ImageAtlasEditor &iae          ) {if(ElmImageAtlas *d=iae.data())return d.mip_maps; return S;}

   static void Undo  (ImageAtlasEditor &editor) {editor.undos.undo();}
   static void Redo  (ImageAtlasEditor &editor) {editor.undos.redo();}
   static void Locate(ImageAtlasEditor &editor) {Proj.elmLocate(editor.elm_id);}

   ElmImageAtlas* data()C {return elm ? elm.imageAtlasData() : null;}

   GuiObj& desc(C Str &desc)override
   {
              list.desc(desc);
            region.desc(desc);
      return super.desc(desc);
   }
   void makeDo()
   {
      if(ElmImageAtlas *data=T.data())
      {
         ImageAtlas atlas;
         Memb<Image>             images; // use 'Memb' because we're storing pointers
         Memc<ImageAtlas.Source> source;
         FREPA(data.images)
         {
            ElmImageAtlas.Img &img=data.images[i]; if(!img.removed)if(Elm *elm=Proj.findElm(img.id))
            {
               images.New().ImportTry((elm.type==ELM_IMAGE) ? Proj.editPath(*elm) : Proj.gamePath(*elm), -1, IMAGE_SOFT, 1); // use edit path for ELM_IMAGE and game path for ELM_ICON
               source.New().set(&images.last(), elm.name);
            }
         }
         source.sort(CompareSource);
         if(!atlas.create(source, SupportBC7 ? IMAGE_BC7 : IMAGE_BC3, data.mip_maps ? 0 : 1))Gui.msgBox(S, "Error creating Image Atlas");else
         {
            Save(atlas, Proj.gamePath(*elm)); Proj.savedGame(*elm);
            setChanged(true);
         }
      }
   }

   void create()
   {
      Property &mip_maps=add("Mip Maps", MemberDesc(DATA_BOOL).setFunc(MipMaps, MipMaps));
      autoData(this);

      ListColumn lc[]=
      {
         ListColumn(DATA_NONE, 0, 0, 0.050, S              ), // column for "remove" button
         ListColumn(ElmFullName    , 0.8  , "Element"      ),
         ListColumn(ElmName        , 0.4  , "Name in Atlas"),
      };
      super.create("Image Atlas Editor"); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=make.create(Rect_LU(0.01, -0.01, 0.25, 0.055), "Create").func(Make, T).focusable(false).desc("Create and save image atlas from currently listed images");
      mip_maps.pos(make.rect().right()+Vec2(make.size().y, 0));
      T+=undo  .create().func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create().func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create("Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      T+=show_removed.create().func(ShowRemoved, T).focusable(false).desc("Show removed elements"); show_removed.image="Gui/Misc/trash.img"; show_removed.mode=BUTTON_TOGGLE;
      T+=region.create();
      region+=list.create(lc, Elms(lc)); list.setElmTextColor(MEMBER(ListElm, color)); list.flag|=LIST_RESIZABLE_COLUMNS; FlagDisable(list.flag, LIST_SORTABLE); // disable sorting because of remove buttons
      list.elmHeight(0.043).textSize(0, 1);
      flt w=0.02+region.slidebarSize(); REPA(lc)w+=lc[i].width; rect(Rect_C(0, 0, w, 1));
      desc("Drag and drop an image here");
   }
   virtual Rect sizeLimit()C override {Rect r=super.sizeLimit(); r.min.set(0.85, 0.4); return r;}
   virtual ImageAtlasEditor& rect(C Rect &rect)override
   {
      super.rect(rect);
      show_removed.rect(Rect_RU(Vec2(clientWidth()-0.01, -0.01) ,       make.rect().h()));
      locate      .rect(Rect_RU(show_removed.pos()-Vec2(0.01, 0), 0.15, make.rect().h()));
      redo        .rect(Rect_RU(locate      .pos()-Vec2(0.01, 0),       make.rect().h()));
      undo        .rect(Rect_RU(redo        .pos()              ,       make.rect().h()));
      region      .rect(Rect(0, -clientHeight(), clientWidth(), show_removed.rect().min.y).extend(-0.01));
      return T;
   }

   void skinChanged()
   {
      REPAO(list_data).setColor();
   }
   void flush()
   {
      if(elm && changed)
      {
         if(ElmImageAtlas *data=T.data())data.newVer(); // modify just before saving/sending in case we've received data from server after edit
         if(changed_file)Server.setElmLong(elm.id);else Server.setElmShort(elm.id);
      }
      changed=changed_file=false;
   }
   void setChanged(bool file=false)
   {
      if(elm)
      {
         changed=true;
         if(ElmImageAtlas *data=T.data())
         {
            data.newVer();
            if(file){data.file_time.getUTC(); changed_file=true;}
         }
         toGui();
      }
   }
   static void Remove(ListElm &le) {ImageAtlasEdit.remove(le);}
          void remove(ListElm &le)
   {
      if(ElmImageAtlas *data=T.data())if(ElmImageAtlas.Img *img=data.find(le.img_id))
      {
         undos.set(null, true);
         img.removed^=1; img.removed_time.getUTC();
         setChanged();
      }
   }
   void toGui()
   {
      super.toGui();
      int v=0; flt h=list.elmHeight();
      list_data.clear(); if(ElmImageAtlas *data=T.data())FREPA(data.images)if(!data.images[i].removed || show_removed())
      {
         ListElm &le=list_data.New(); le=data.images[i];
         le.remove.create().func(Remove, le).desc(data.images[i].removed ? "Restore this element" : "Remove this element");
         if(data.images[i].removed)le.remove.text="R";else le.remove.image="Gui/close.img";
      }
      list_data.sort(ListElm.ComparePath);
      FREPA(list_data)region+=list_data[i].remove.rect(Rect_LU(0, -(v++)*h-list.columnHeight(), h, h));
      list.setData(list_data);
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_IMAGE_ATLAS)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
           void              activate(Elm *elm)         {set(elm); if(T.elm)super.activate();}
           void              toggle  (Elm *elm)         {if(elm==T.elm)elm=null; set(elm);}
   virtual ImageAtlasEditor& hide    (        )override {if(visible()){super.hide(); set(null);} return T;}

   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
         toGui();
      }
   }
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(elm && contains(obj))REPA(elms)if(Elm *image=Proj.findElm(elms[i]))if(ElmImageLike(image.type)) // add image element to the atlas
      {
         if(ElmImageAtlas *data=elm.imageAtlasData())
         {
            undos.set(null, true);
            ElmImageAtlas.Img &img=data.get(image.id);
            img.id=image.id; img.removed=false; img.removed_time.getUTC();
            setChanged();
         }
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
}
ImageAtlasEditor ImageAtlasEdit;
/******************************************************************************/
