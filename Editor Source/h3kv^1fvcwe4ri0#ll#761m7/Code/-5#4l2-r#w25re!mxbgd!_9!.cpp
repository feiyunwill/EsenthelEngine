/******************************************************************************/
class ImageEditor : PropWin
{
   static void Render() {} // we just want the skybox, so no need to draw anything else

   class GuiImage2 : ImageSkin
   {
      Camera cam;

      virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return (image && image->mode()==IMAGE_CUBE) ? super.test(gpc, pos, mouse_wheel) : null;}
      virtual void    update(C GuiPC &gpc)override
      {
         if(Gui.ms()==this && Ms.b(0))
         {
            cam.yaw  -=Ms.d().x;
            cam.pitch+=Ms.d().y;
            cam.setSpherical().updateVelocities(CAM_ATTACH_FREE);
            Ms.freeze();
         }else
         if(visible())
         {
            cam.yaw+=Time.ad()/2;
            cam.setSpherical().updateVelocities(CAM_ATTACH_FREE);
         }
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            Rect rect=T.rect()+gpc.offset;
            if(image)
            {
               D.clip(rect&gpc.clip);
               if(image->mode()==IMAGE_CUBE)
               {
                  int          clouds   =Clouds.layered.layers(); Clouds.layered.set(0);
                  AMBIENT_MODE ambient  =D.  ambientMode(); D.  ambientMode(AMBIENT_FLAT);
                  DOF_MODE     dof      =D.      dofMode(); D.      dofMode(    DOF_NONE);
                  bool         eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(       false);
                  bool         astros   =AstrosDraw       ; AstrosDraw     =false;
                  bool         ocean    =Water.draw       ; Water.draw     =false;
                  bool         fog      =Fog  .draw       ; Fog  .draw     =false;
                  flt fov=D.viewFov(); Camera c=ActiveCam;
                  D.viewRect(rect).viewFov(DefaultFOV); cam.set();
                  Sky.skybox(image);
                  CurrentEnvironment().bloom.set();
                  Renderer(ImageEditor.Render);
                  Sky.atmospheric();
                  D.viewRect(null).viewFov(fov); c.set();
                  D.      dofMode(dof      );
                  D.  ambientMode(ambient  );
                  D.eyeAdaptation(eye_adapt);
                  AstrosDraw     =astros;
                  Water.draw     =ocean;
                  Fog  .draw     =fog;
                  Clouds.layered.set(clouds);
               }else
               {
                  Rect r=rect;
                  if(ImageEdit.no_scale)
                  {
                     r.setC(r.center(), D.pixelToScreenSize(image->size()));
                     D.alignScreenToPixel(r);
                  }
                  ALPHA_MODE alpha=D.alpha(ImageEdit.channels ? ALPHA_NONE : ALPHA_BLEND);
                  if(ImageEdit.channels==2)VI.shader(ShaderFiles("Main").get("DrawTexW"));
                  image->drawFit(color, color_add, r);
                  D.alpha(alpha);
               }
            }
            D.clip(gpc.clip);
            rect.draw(rect_color, false);
            if(ImageEdit.elm && ImageEdit.elm.importing()){TextStyleParams ts; ts.size*=0.6; ts.align.set(0, 1); D.text(ts, rect.down(), "Reloading");}
         }
      }
   }
   class CubeFace : ImageSkin
   {
      Vec2     ofs=0;
      DIR_ENUM face=DIR_RIGHT;

      CubeFace& create(DIR_ENUM dir, int x, int y)
      {
         super.create().desc(DirToText(dir));
         T.face=dir;
         T.ofs.set(x, y);
         return T;
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            Rect rect=T.rect()+gpc.offset;
            if(image && image->mode()==IMAGE_CUBE)
            {
               D.clip(rect&gpc.clip);
               image->drawCubeFace(WHITE, TRANSPARENT, rect, face);
            }
            D.clip(gpc.clip);
            rect.draw(ColorAlpha(rect_color, 0.25), false);
         }
      }
   }
   class Change : Edit._Undo.Change
   {
      ElmImage data;

      virtual void create(ptr user)override
      {
         if(ElmImage *d=ImageEdit.data())data=*d;
         ImageEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         if(ElmImage *d=ImageEdit.data())d.undo(data);
         ImageEdit.setChanged();
         ImageEdit.toGui();
         ImageEdit.undoVis();
      }
   }

   UID         elm_id=UIDZero;
   Elm        *elm=null;
   bool        changed=false, no_scale=false, source=true;
   int         channels=0;
   Button      undo, redo, locate;
   GuiImage2   gui_image;
   Image      edit_image, image_2d;
   ImagePtr   game_image;
   CubeFace   cube_faces[6];
   Property  *width=null, *height=null, *type=null, *mip_maps=null, *mem_size=null, *chn=null, *nos=null, *src=null;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   ElmImage* data()C {return elm ? elm.imageData() : null;}

   class ImageMode
   {
      IMAGE_MODE mode;
      cchar8    *name;
      cchar     *desc;
   }
   static ImageMode ImageModes[]=
   {
      {IMAGE_2D  , "2D Texture", null},
      {IMAGE_CUBE, "Cube Texture", u"SkyBox"},
      {IMAGE_SOFT, "Software", u"This mode is used for custom processing only\nIt cannot be used for drawing"},
   };
   static cchar8 *Channels[]=
   {
      "RGBA", "RGB", "A",
   };
   static int ChannelsElms=Elms(Channels);

   static void PreChanged(C Property &prop) {ImageEdit.undos.set(&prop);}
   static void    Changed(C Property &prop) {ImageEdit.setChanged();}

   static void Type    (  ImageEditor &ie, C Str &t) {if(ElmImage *d=ie.data()){int i=TextInt(t); if(InRange(i, ElmImage.NUM))d.type=ElmImage.TYPE(i); d.type_time.getUTC();}}
   static Str  Type    (C ImageEditor &ie          ) {if(ElmImage *d=ie.data())return d.type; return -1;}
   static void Mode    (  ImageEditor &ie, C Str &t) {if(ElmImage *d=ie.data()){int i=TextInt(t); if(InRange(i, ImageModes)){d.mode=ImageModes[i].mode; d.mode_time.getUTC();}}}
   static Str  Mode    (C ImageEditor &ie          ) {if(ElmImage *d=ie.data())REPA(ImageModes)if(ImageModes[i].mode==d.mode)return i; return -1;}
   static void MipMaps (  ImageEditor &ie, C Str &t) {if(ElmImage *d=ie.data()){d.mipMaps(TextBool(t)); d.mip_maps_time.getUTC();}}
   static Str  MipMaps (C ImageEditor &ie          ) {if(ElmImage *d=ie.data())return d.mipMaps(); return S;}
   static void Pow2    (  ImageEditor &ie, C Str &t) {if(ElmImage *d=ie.data()){d.pow2(TextBool(t)); d.pow2_time.getUTC();}}
   static Str  Pow2    (C ImageEditor &ie          ) {if(ElmImage *d=ie.data())return d.pow2(); return S;}
   static void Width   (  ImageEditor &ie, C Str &t) {if(ElmImage *d=ie.data()){d.size.x=TextInt(t); d.size_time.getUTC();}}
   static Str  Width   (C ImageEditor &ie          ) {if(ElmImage *d=ie.data())return d.size.x; return S;}
   static void Height  (  ImageEditor &ie, C Str &t) {if(ElmImage *d=ie.data()){d.size.y=TextInt(t); d.size_time.getUTC();}}
   static Str  Height  (C ImageEditor &ie          ) {if(ElmImage *d=ie.data())return d.size.y; return S;}
   static void AlphaLum(  ImageEditor &ie, C Str &t) {if(ElmImage *d=ie.data()){d.alphaLum(TextBool(t)); d.alpha_lum_time.getUTC();}}
   static Str  AlphaLum(C ImageEditor &ie          ) {if(ElmImage *d=ie.data())return d.alphaLum(); return S;}

   static void Undo  (ImageEditor &editor) {editor.undos.undo();}
   static void Redo  (ImageEditor &editor) {editor.undos.redo();}
   static void Locate(ImageEditor &editor) {Proj.elmLocate(editor.elm_id);}

   void setMipMap(bool on) {undos.set("mipMap"); MipMaps(T, on); setChanged(); toGui();}

   void create()
   {
      ListColumn lct[]=
      {
         ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      ListColumn lcm[]=
      {
         ListColumn(MEMBER(ImageMode, name), LCW_MAX_DATA_PARENT, "name"),
      };
      add("Type"                , MemberDesc(         ).setFunc(Type    , Type    )).setEnum().combobox.setColumns(lct, Elms(lct)).setData(ElmImage.ImageTypes, ElmImage.ImageTypesElms).menu.list.setElmDesc(MEMBER(NameDesc, desc));
      add("Mip Maps"            , MemberDesc(DATA_BOOL).setFunc(MipMaps , MipMaps ));
      add("Power of 2"          , MemberDesc(DATA_BOOL).setFunc(Pow2    , Pow2    )).desc("Resize image to nearest power of 2");
      add("Alpha from Luminance", MemberDesc(DATA_BOOL).setFunc(AlphaLum, AlphaLum)).desc("Set image opacity from its brightness");
      add("Mode"                , MemberDesc(         ).setFunc(Mode    , Mode    )).setEnum().combobox.setColumns(lcm, Elms(lcm)).setData(ImageModes, Elms(ImageModes)).menu.list.setElmDesc(MEMBER(ImageMode, desc));
      add("Width"               , MemberDesc(DATA_INT ).setFunc(Width   , Width   )).range(-1, 65536).desc("Set custom image width (0=default, -1=keep original)");
      add("Height"              , MemberDesc(DATA_INT ).setFunc(Height  , Height  )).range(-1, 65536).desc("Set custom image height (0=default, -1=keep original)");
                add();
                add("Info:");
      width   =&add();
      height  =&add();
      type    =&add();
      mip_maps=&add();
      mem_size=&add();
                add();
                add("Display:");
      chn     =&add("Channels", MemberDesc(MEMBER(ImageEditor, channels))).setEnum(Channels, Elms(Channels));
      nos     =&add("No Scale", MemberDesc(MEMBER(ImageEditor, no_scale)));
      src     =&add("Source"  , MemberDesc(MEMBER(ImageEditor, source  ))).desc("Display source cube face images separately");
      autoData(this);

      flt h=0.043;
      super.create("Image Editor", Vec2(0.02, -0.07), 0.036, h, 0.28); super.changed(Changed, PreChanged); chn.changed(null, null); nos.changed(null, null); src.changed(null, null);
      button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=undo  .create(Rect_LU(0.02, -0.01     , 0.05, 0.05)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05, 0.05)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01, 0), 0.14, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");

      src.move(Vec2(0, h*2));
      T+=gui_image.create(); gui_image.moveToBottom();
      T+=cube_faces[0].create(DIR_LEFT   , 0, -1);
      T+=cube_faces[1].create(DIR_FORWARD, 1, -1);
      T+=cube_faces[2].create(DIR_RIGHT  , 2, -1);
      T+=cube_faces[3].create(DIR_BACK   , 3, -1);
      T+=cube_faces[4].create(DIR_DOWN   , 1, -2);
      T+=cube_faces[5].create(DIR_UP     , 1,  0);
      rect(Rect_C(0, 0, Min(1.7, D.w()*2), Min(1.07, D.h()*2)));
   }
   void make2D()
   {
      gui_image.image=((data() && data().mode==IMAGE_SOFT) ? &image_2d : game_image); REPAO(cube_faces).image=gui_image.image;
      image_2d.del();
      if(gui_image.image==&image_2d && game_image)game_image->copyTry(image_2d, -1, -1, 1, IMAGE_R8G8B8A8, IMAGE_2D);
   }
   void setInfo()
   {
      if(width   )width   .name.set(S+"Width: "   +        (game_image ? game_image->w           () : 0));
      if(height  )height  .name.set(S+"Height: "  +        (game_image ? game_image->h           () : 0));
      if(type    )type    .name.set(S+"Type: "    + ImageTI[game_image ? game_image->type        () : IMAGE_NONE].name);
      if(mip_maps)mip_maps.name.set(S+"Mip Maps: "+        (game_image ? game_image->mipMaps     () : 0));
      if(mem_size)mem_size.name.set(S+"Size: "    +FileSize(game_image ? game_image->typeMemUsage() : 0)); // use 'typeMemUsage' because we need this only for stats
      if(chn     )chn.visible(data() && data().mode!=IMAGE_CUBE);
      if(nos     )nos.visible(data() && data().mode!=IMAGE_CUBE);
      if(src     )src.visible(data() && data().mode==IMAGE_CUBE);
      REPAO(cube_faces).visible(src && src.visible() && source);
   }

   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(gpc.visible && visible())setInfo();
   }
   virtual ImageEditor& hide     (            )  override {set(null); super.hide(); return T;}
   virtual Rect         sizeLimit(            )C override {Rect r=super.sizeLimit(); r.min.set(0.7, 0.26); return r;}
   virtual ImageEditor& rect     (C Rect &rect)  override
   {
      super.rect(rect);
      flt x=0; if(props.elms())x=props[0].combobox.rect().max.x;
      gui_image.rect(Rect(x, -clientHeight(), clientWidth(), 0).extend(-0.02));
      Rect r=gui_image.rect();
      flt  face_size=(r.size()/Vec2(4, 3)).min();
      REPAO(cube_faces).rect(Rect_LU(r.lu()+cube_faces[i].ofs*face_size, face_size));
      return T;
   }

   void flush()
   {
      if(elm && changed)
      {
         if(ElmImage *data=T.data())data.newVer(); // modify just before saving/sending in case we've received data from server after edit
         if(game_image){Save(*game_image, Proj.gamePath(*elm)); Proj.savedGame(*elm);}
         Server.setElmShort(elm.id);
      }
      changed=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmImage *data=T.data())
         {
            data.newVer();
            if(!edit_image.is())edit_image.ImportTry(Proj.editPath(*elm));
            if( game_image)EditToGameImage(edit_image, *game_image, *data);
            make2D();
         }
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_IMAGE)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         edit_image.del();
         if(elm)game_image=Proj.gamePath(*elm);else game_image.clear();
         toGui();
         make2D();
         setInfo();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void toggle  (Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
         edit_image.del(); toGui(); make2D(); setInfo(); // delete 'edit_image' so it will be reloaded when needed
      }
   }
   class ImageName
   {
      Str name, file;
      void set(C Str &name, C Str &file) {T.name=name; T.file=file;}
   }
   int FaceToIndex(DIR_ENUM dir)
   {
      REPA(cube_faces)if(cube_faces[i].face==dir)return i;
      return -1;
   }
   void setImage(Memc<ImageName> &images, GuiObj *obj)
   {
      if(ElmImage *d=data())
         if(images.elms())
      {
         Str import=images[0].file;
         if(d.mode==IMAGE_CUBE)
         {
            if(images.elms()>1) // multi
            {
               import=elm.srcFile();
               REPA(images)
               {
                  DIR_ENUM dir=DIR_NUM;
                C Str &name=images[i].name;
                  if(Contains(name, "front" ) || Contains(name, "forward"))dir=DIR_FORWARD;else
                  if(Contains(name, "back"  )                             )dir=DIR_BACK   ;else
                  if(Contains(name, "top"   ) || Contains(name, "up"     ))dir=DIR_UP     ;else
                  if(Contains(name, "bottom") || Contains(name, "down"   ))dir=DIR_DOWN   ;else
                  if(Contains(name, "left"  )                             )dir=DIR_LEFT   ;else
                  if(Contains(name, "right" )                             )dir=DIR_RIGHT  ;
                  import=SetCubeFile(import, FaceToIndex(dir), images[i].file);
               }
            }else
            REPA(cube_faces)if(cube_faces[i].contains(obj)) // check if cube face is selected
            {
               import=SetCubeFile(elm.srcFile(), i, images[0].file);
               break;
            }
         }
         elm.setSrcFile(import); Server.setElmShort(elm_id); Proj.elmReload(elm_id);
      }
   }
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj))
      {
         Memc<ImageName> images;
         REPA(names)if(ExtType(GetExt(names[i]))==EXT_IMAGE)images.New().set(GetBaseNoExt(names[i]), names[i]);
         setImage(images, obj);
      }
   }
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj))
      {
         Memc<ImageName> images;
         REPA(elms)if(Elm *image=Proj.findElm(elms[i], ELM_IMAGE))if(elm!=image)images.New().set(image.name, EncodeFileName(image.id));
         setImage(images, obj);
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
}
ImageEditor ImageEdit;
/******************************************************************************/
