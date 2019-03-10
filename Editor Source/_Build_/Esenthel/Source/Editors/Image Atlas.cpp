/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ImageAtlasEditor ImageAtlasEdit;
/******************************************************************************/

/******************************************************************************/
      void ImageAtlasEditor::Change::create(ptr user)
{
         if(ElmImageAtlas *data=ImageAtlasEdit.data())
         {
            T.data=*data;
            ImageAtlasEdit.undoVis();
         }
      }
      void ImageAtlasEditor::Change::apply(ptr user)
{
         if(ElmImageAtlas *data=ImageAtlasEdit.data())
         {
            data->undo(T.data);
            ImageAtlasEdit.setChanged();
            ImageAtlasEdit.toGui();
            ImageAtlasEdit.undoVis();
         }
      }
      void ImageAtlasEditor::ListElm::setColor() {color=GuiListTextColor(); if(removed)color.a/=2;}
      void ImageAtlasEditor::ListElm::operator=(C ElmImageAtlas::Img &src) {img_id=src.id; removed=src.removed; setColor();}
      int ImageAtlasEditor::ListElm::ComparePath(C ListElm &a, C ListElm &b) {return ::ComparePathNumber(ElmName(a), ElmName(b));}
   void ImageAtlasEditor::undoVis() {SetUndo(undos, undo, redo);}
   void ImageAtlasEditor::ShowRemoved(ImageAtlasEditor &iae) {iae.toGui ();}
   void ImageAtlasEditor::Make(ImageAtlasEditor &iae) {iae.makeDo();}
   Str ImageAtlasEditor::ElmFullName(C ListElm &le) {return      Proj.elmFullName(le.img_id);}
   Str ImageAtlasEditor::ElmName(C ListElm &le) {if(Elm *elm=Proj.findElm    (le.img_id))return elm->name; return UnknownName;}
   int ImageAtlasEditor::CompareSource(C ImageAtlas::Source &a, C ImageAtlas::Source &b) {return ComparePathNumber(a.name, b.name);}
   void ImageAtlasEditor::MipMaps(  ImageAtlasEditor &iae, C Str &t) {if(ElmImageAtlas *d=iae.data()){iae.undos.set("mms"); d->mip_maps=TextBool(t); d->mip_maps_time.getUTC(); iae.setChanged();}}
   Str  ImageAtlasEditor::MipMaps(C ImageAtlasEditor &iae          ) {if(ElmImageAtlas *d=iae.data())return d->mip_maps; return S;}
   void ImageAtlasEditor::Undo(ImageAtlasEditor &editor) {editor.undos.undo();}
   void ImageAtlasEditor::Redo(ImageAtlasEditor &editor) {editor.undos.redo();}
   void ImageAtlasEditor::Locate(ImageAtlasEditor &editor) {Proj.elmLocate(editor.elm_id);}
   ElmImageAtlas* ImageAtlasEditor::data()C {return elm ? elm->imageAtlasData() : null;}
   GuiObj& ImageAtlasEditor::desc(C Str &desc)
{
              list.desc(desc);
            region.desc(desc);
      return ::EE::GuiObj::desc(desc);
   }
   void ImageAtlasEditor::makeDo()
   {
      if(ElmImageAtlas *data=T.data())
      {
         ImageAtlas atlas;
         Memb<Image>             images; // use 'Memb' because we're storing pointers
         Memc<ImageAtlas::Source> source;
         FREPA(data->images)
         {
            ElmImageAtlas::Img &img=data->images[i]; if(!img.removed)if(Elm *elm=Proj.findElm(img.id))
            {
               images.New().ImportTry((elm->type==ELM_IMAGE) ? Proj.editPath(*elm) : Proj.gamePath(*elm), -1, IMAGE_SOFT, 1); // use edit path for ELM_IMAGE and game path for ELM_ICON
               source.New().set(&images.last(), elm->name);
            }
         }
         source.sort(CompareSource);
         if(!atlas.create(source, SupportBC7 ? IMAGE_BC7 : IMAGE_BC3, data->mip_maps ? 0 : 1))Gui.msgBox(S, "Error creating Image Atlas");else
         {
            Save(atlas, Proj.gamePath(*elm)); Proj.savedGame(*elm);
            setChanged(true);
         }
      }
   }
   void ImageAtlasEditor::create()
   {
      Property &mip_maps=add("Mip Maps", MemberDesc(DATA_BOOL).setFunc(MipMaps, MipMaps));
      autoData(this);

      ListColumn lc[]=
      {
         ListColumn(DATA_NONE, 0, 0, 0.050f, S              ), // column for "remove" button
         ListColumn(ElmFullName    , 0.8f  , "Element"      ),
         ListColumn(ElmName        , 0.4f  , "Name in Atlas"),
      };
      ::PropWin::create("Image Atlas Editor"); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=make.create(Rect_LU(0.01f, -0.01f, 0.25f, 0.055f), "Create").func(Make, T).focusable(false).desc("Create and save image atlas from currently listed images");
      mip_maps.pos(make.rect().right()+Vec2(make.size().y, 0));
      T+=undo  .create().func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create().func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create("Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      T+=show_removed.create().func(ShowRemoved, T).focusable(false).desc("Show removed elements"); show_removed.image="Gui/Misc/trash.img"; show_removed.mode=BUTTON_TOGGLE;
      T+=region.create();
      region+=list.create(lc, Elms(lc)); list.setElmTextColor(MEMBER(ListElm, color)); list.flag|=LIST_RESIZABLE_COLUMNS; FlagDisable(list.flag, LIST_SORTABLE); // disable sorting because of remove buttons
      list.elmHeight(0.043f).textSize(0, 1);
      flt w=0.02f+region.slidebarSize(); REPA(lc)w+=lc[i].width; rect(Rect_C(0, 0, w, 1));
      desc("Drag and drop an image here");
   }
   Rect ImageAtlasEditor::sizeLimit()C {Rect r=::EE::Window::sizeLimit(); r.min.set(0.85f, 0.4f); return r;}
   ImageAtlasEditor& ImageAtlasEditor::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      show_removed.rect(Rect_RU(Vec2(clientWidth()-0.01f, -0.01f) ,       make.rect().h()));
      locate      .rect(Rect_RU(show_removed.pos()-Vec2(0.01f, 0), 0.15f, make.rect().h()));
      redo        .rect(Rect_RU(locate      .pos()-Vec2(0.01f, 0),       make.rect().h()));
      undo        .rect(Rect_RU(redo        .pos()              ,       make.rect().h()));
      region      .rect(Rect(0, -clientHeight(), clientWidth(), show_removed.rect().min.y).extend(-0.01f));
      return T;
   }
   void ImageAtlasEditor::skinChanged()
   {
      REPAO(list_data).setColor();
   }
   void ImageAtlasEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmImageAtlas *data=T.data())data->newVer(); // modify just before saving/sending in case we've received data from server after edit
         if(changed_file)Server.setElmLong(elm->id);else Server.setElmShort(elm->id);
      }
      changed=changed_file=false;
   }
   void ImageAtlasEditor::setChanged(bool file)
   {
      if(elm)
      {
         changed=true;
         if(ElmImageAtlas *data=T.data())
         {
            data->newVer();
            if(file){data->file_time.getUTC(); changed_file=true;}
         }
         toGui();
      }
   }
   void ImageAtlasEditor::Remove(ListElm &le) {ImageAtlasEdit.remove(le);}
          void ImageAtlasEditor::remove(ListElm &le)
   {
      if(ElmImageAtlas *data=T.data())if(ElmImageAtlas::Img *img=data->find(le.img_id))
      {
         undos.set(null, true);
         img->removed^=1; img->removed_time.getUTC();
         setChanged();
      }
   }
   void ImageAtlasEditor::toGui()
   {
      ::PropWin::toGui();
      int v=0; flt h=list.elmHeight();
      list_data.clear(); if(ElmImageAtlas *data=T.data())FREPA(data->images)if(!data->images[i].removed || show_removed())
      {
         ListElm &le=list_data.New(); le=data->images[i];
         le.remove.create().func(Remove, le).desc(data->images[i].removed ? "Restore this element" : "Remove this element");
         if(data->images[i].removed)le.remove.text="R";else le.remove.image="Gui/close.img";
      }
      list_data.sort(ListElm::ComparePath);
      FREPA(list_data)region+=list_data[i].remove.rect(Rect_LU(0, -(v++)*h-list.columnHeight(), h, h));
      list.setData(list_data);
   }
   void ImageAtlasEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_IMAGE_ATLAS)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
           void              ImageAtlasEditor::activate(Elm *elm)         {set(elm); if(T.elm)::EE::GuiObj::activate();}
           void              ImageAtlasEditor::toggle(Elm *elm)         {if(elm==T.elm)elm=null; set(elm);}
   ImageAtlasEditor& ImageAtlasEditor::hide(        ){if(visible()){::PropWin::hide(); set(null);} return T;}
   void ImageAtlasEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         undos.set(null, true);
         toGui();
      }
   }
   void ImageAtlasEditor::drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(elm && contains(obj))REPA(elms)if(Elm *image=Proj.findElm(elms[i]))if(ElmImageLike(image->type)) // add image element to the atlas
      {
         if(ElmImageAtlas *data=elm->imageAtlasData())
         {
            undos.set(null, true);
            ElmImageAtlas::Img &img=data->get(image->id);
            img.id=image->id; img.removed=false; img.removed_time.getUTC();
            setChanged();
         }
      }
   }
   void ImageAtlasEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
ImageAtlasEditor::ImageAtlasEditor() : elm_id(UIDZero), elm(null), changed(false), changed_file(false), undos(true) {}

ImageAtlasEditor::ListElm::ListElm() : removed(false), color(BLACK), img_id(UIDZero) {}

/******************************************************************************/
