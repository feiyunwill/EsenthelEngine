/******************************************************************************/
// TODO: make unavailable on Mobile
/******************************************************************************/
class FontEditor : PropWin
{
   class Params : EditFont
   {
      Str  sample_text="Sample Text";
      bool no_scale=false;
   }
   class Change : Edit._Undo.Change
   {
      EditFont data;

      virtual void create(ptr user)override
      {
         data=FontEdit.params;
         FontEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         FontEdit.params.undo(data);
         FontEdit.setChanged();
         FontEdit.toGui();
         FontEdit.undoVis();
      }
   }

   UID          elm_id=UIDZero;
   Elm         *elm=null;
   bool         changed=false;
   Params       params;
   Font         font;
   Button       undo, redo, locate;
   ViewportSkin viewport;
   SyncEvent    event;
   Thread       thread;
   SyncLock     lock;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

  ~FontEditor() {del();} // delete thread before other members

   static void Preview(Viewport &viewport) {((FontEditor*)viewport.user).preview();}
          void preview()
   {
      SyncLocker locker(lock);
      Rect       rect=D.viewRect(),
                 q[4]={rect, rect, rect, rect};
      q[0].max.x=q[1].min.x=q[2].max.x=q[3].min.x=rect.centerX();
      q[0].min.y=q[1].min.y=q[2].max.y=q[3].max.y=rect.centerY();
      TextStyleParams ts; ts.font(&font);
      REPA(q)
      {
         Rect r=q[i];
         D.viewRect(r); r.draw((i==1) ? BLACK : WHITE);
         ts.color =((i==0 || i==1) ? WHITE : GREEN);
         ts.shadow=((i==0 || i==2) ? 255   : 0    );
         if(params.no_scale)
         {
            ts.setPerPixelSize();
            ts.align.set(0, 0);
            D.text(ts, r.center(), params.sample_text);
         }else
         {
            ts.size=0.02;
            ts.align.set(0, -1);
            flt y=r.max.y;
            FREP(10)
            {
               D.text(ts, Vec2(r.centerX(), y), params.sample_text);
               y-=ts.size.y*0.9;
               ts.size*=1.3;
            }
         }
      }
   }
   static bool Make(Thread &thread) {return (*(FontEditor*)thread.user).make();}
          bool make()
   {
      if(event.wait() && !thread.wantStop())
      {
         SyncLockerEx locker(lock); Params params=T.params; locker.off();
         ThreadMayUseGPUData();
         Font temp; if(params.make(temp, &params.sample_text)){locker.on(); Swap(font, temp); locker.off();}
      }
      return true;
   }

   static void PreChanged(C Property &prop) {FontEdit.undos.set(&prop);}
   static void    Changed(C Property &prop) {FontEdit.setChanged();}

   static void ParamsFont      (Params &p, C Str &t) {p.font           =         t ; p.           font_time.getUTC();}
   static void ParamsSize      (Params &p, C Str &t) {p.size           =TextInt (t); p.           size_time.getUTC();}
   static void ParamsScale     (Params &p, C Str &t) {p.scale          =TextFlt (t); p.          scale_time.getUTC();}
   static void ParamsClearType (Params &p, C Str &t) {p.clear_type     =TextBool(t); p.     clear_type_time.getUTC();}
   static void ParamsSoftware  (Params &p, C Str &t) {p.software       =TextBool(t); p.       software_time.getUTC();}
   static void ParamsWeight    (Params &p, C Str &t) {p.weight         =TextFlt (t); p.         weight_time.getUTC();}
   static void ParamsMinFilter (Params &p, C Str &t) {p.min_filter     =TextFlt (t); p.     min_filter_time.getUTC();}
   static void ParamsDiagShadow(Params &p, C Str &t) {p.diagonal_shadow=TextBool(t); p.diagonal_shadow_time.getUTC();}
   static void ParamsMipMaps   (Params &p, C Str &t) {p.mip_maps       =TextInt (t); p.       mip_maps_time.getUTC();}
   static void ParamsShdBlur   (Params &p, C Str &t) {p.shadow_blur    =TextFlt (t); p.    shadow_blur_time.getUTC();}
   static void ParamsShdOpacity(Params &p, C Str &t) {p.shadow_opacity =TextFlt (t); p. shadow_opacity_time.getUTC();}
   static void ParamsShdSpread (Params &p, C Str &t) {p.shadow_spread  =TextFlt (t); p.  shadow_spread_time.getUTC();}
   static void ParamsAscii     (Params &p, C Str &t) {p.ascii          =TextBool(t); p.          ascii_time.getUTC();}
   static void ParamsGerman    (Params &p, C Str &t) {p.german         =TextBool(t); p.         german_time.getUTC();}
   static void ParamsFrench    (Params &p, C Str &t) {p.french         =TextBool(t); p.         french_time.getUTC();}
   static void ParamsPolish    (Params &p, C Str &t) {p.polish         =TextBool(t); p.         polish_time.getUTC();}
   static void ParamsRussian   (Params &p, C Str &t) {p.russian        =TextBool(t); p.        russian_time.getUTC();}
   static void ParamsChinese   (Params &p, C Str &t) {p.chinese        =TextBool(t); p.        chinese_time.getUTC();}
   static void ParamsJapanese  (Params &p, C Str &t) {p.japanese       =TextBool(t); p.       japanese_time.getUTC();}
   static void ParamsKorean    (Params &p, C Str &t) {p.korean         =TextBool(t); p.         korean_time.getUTC();}
   static void ParamsCustomChar(Params &p, C Str &t) {p.custom_chars   =         t ; p.   custom_chars_time.getUTC();}
   static void ParamsSampleText(Params &p, C Str &t) {p.sample_text    =         t ; FontEdit.refresh();}

   static void Undo  (FontEditor &editor) {editor.undos.undo();}
   static void Redo  (FontEditor &editor) {editor.undos.redo();}
   static void Locate(FontEditor &editor) {Proj.elmLocate(editor.elm_id);}

   void create()
   {
      add("System Font"    , MemberDesc(MEMBER(Params, font           )).setTextToDataFunc(ParamsFont      ));
      add("Size"           , MemberDesc(MEMBER(Params, size           )).setTextToDataFunc(ParamsSize      )).range(8, 128).desc("Image Resolution");
      add("Scale"          , MemberDesc(MEMBER(Params, scale          )).setTextToDataFunc(ParamsScale     )).range(0.5, 2.0).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(0.25).desc("This will scale the original source Font, however the destination area size to store the Font remains the same.\nWhich means if you set the scale too big, then Font may not fit into the destination, and may get clipped.\nThis is useful for Fonts that have very small or very big characters.");
      add("Clear Type"     , MemberDesc(MEMBER(Params, clear_type     )).setTextToDataFunc(ParamsClearType ));
      add("Software"       , MemberDesc(MEMBER(Params, software       )).setTextToDataFunc(ParamsSoftware  )).desc("Create Font in Software mode, which will allow faster software processing (drawing in software mode), however drawing using the GPU will not be allowed");
    //add("Sub Pixel"      , MemberDesc(MEMBER(Params, sub_pixel      )).setTextToDataFunc(ParamsSubPixel  )).desc("Warning: enabling this option will increase font quality only if it will be drawn with correct scale,\nin other case font quality will be worse.\nThis option has no effect if 'Clear Type' is disabled.\nUsing this option disables shadows.");
      add("Weight"         , MemberDesc(MEMBER(Params, weight         )).setTextToDataFunc(ParamsWeight    )).range(0, 1).mouseEditSpeed(0.25);
      add("Minimum Filter" , MemberDesc(MEMBER(Params, min_filter     )).setTextToDataFunc(ParamsMinFilter )).range(0, 1).mouseEditSpeed(0.25);
      add("Diagonal Shadow", MemberDesc(MEMBER(Params, diagonal_shadow)).setTextToDataFunc(ParamsDiagShadow));
      add("Mip Maps"       , MemberDesc(MEMBER(Params, mip_maps       )).setTextToDataFunc(ParamsMipMaps   )).range(0, 8).mouseEditSpeed(2).desc("Set number of Mip Maps:\n0 = Full Set\n1 = 1 Mip Map\n2 = 2 Mip Maps\n3 = 3 Mip Maps\n..");
      add("Shadow Blur"    , MemberDesc(MEMBER(Params, shadow_blur    )).setTextToDataFunc(ParamsShdBlur   )).range(0, 1).mouseEditSpeed(0.05);
      add("Shadow Opacity" , MemberDesc(MEMBER(Params, shadow_opacity )).setTextToDataFunc(ParamsShdOpacity)).range(0, 1).mouseEditSpeed(0.30);
      add("Shadow Spread"  , MemberDesc(MEMBER(Params, shadow_spread  )).setTextToDataFunc(ParamsShdSpread )).range(0, 1).mouseEditSpeed(0.30);
      add("Ascii"          , MemberDesc(MEMBER(Params, ascii          )).setTextToDataFunc(ParamsAscii     )).desc("Will include \"ABC.. 0123.. !@#$..\" (characters, digits, symbols) characters in the font");
      add("German"         , MemberDesc(MEMBER(Params, german         )).setTextToDataFunc(ParamsGerman    )).desc("Will include German characters in the font");
      add("French"         , MemberDesc(MEMBER(Params, french         )).setTextToDataFunc(ParamsFrench    )).desc("Will include French characters in the font");
      add("Polish"         , MemberDesc(MEMBER(Params, polish         )).setTextToDataFunc(ParamsPolish    )).desc("Will include Polish characters in the font");
      add("Russian"        , MemberDesc(MEMBER(Params, russian        )).setTextToDataFunc(ParamsRussian   )).desc("Will include Russian characters in the font");
      add("Chinese"        , MemberDesc(MEMBER(Params, chinese        )).setTextToDataFunc(ParamsChinese   )).desc("Will include Chinese characters in the font");
      add("Japanese"       , MemberDesc(MEMBER(Params, japanese       )).setTextToDataFunc(ParamsJapanese  )).desc("Will include Japanese characters in the font");
      add("Korean"         , MemberDesc(MEMBER(Params, korean         )).setTextToDataFunc(ParamsKorean    )).desc("Will include Korean characters in the font");
      add("Custom Chars"   , MemberDesc(MEMBER(Params, custom_chars   )).setTextToDataFunc(ParamsCustomChar)).desc("Will include custom characters in the font");
      add();
      add("Display:");
Property &text=add("Text"           , MemberDesc(MEMBER(Params, sample_text    )).setTextToDataFunc(ParamsSampleText));
Property &nos =add("No Scale"       , MemberDesc(MEMBER(Params, no_scale       )));
      autoData(&params);

      super.create("Font Editor", Vec2(0.02, -0.07), 0.036, 0.043, 0.3); super.changed(Changed, PreChanged); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); text.changed(null, null); nos.changed(null, null);
      T+=undo  .create(Rect_LU(0.02, -0.01     , 0.05, 0.05)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05, 0.05)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01, 0), 0.14, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      clientRect(Rect_C(0, 0, 2, 1.156));
      T+=viewport.create(Rect_LU(0.6, -0.04, 1.37, 0.9), Preview, this);
   }
   void toGui() {super.toGui(); refresh();}
   void setInfo()
   {
      // can't use 'font' because that's just the preview
      // saved file has only previously generated data
   }

   void stopThread() {thread.stop(); event.on();}
   void refresh   () {if(elm){event.on(); if(!thread.active() || thread.wantStop())thread.create(Make, this);}}

   virtual FontEditor& del ()override {stopThread(); thread.del(); super.del (); return T;}
   virtual FontEditor& hide()override {stopThread(); set(null   ); super.hide(); return T;}

   void flush()
   {
      if(elm && changed)
      {
         if(ElmFont *data=elm.fontData()){data.newVer(); data.from(params);} // modify just before saving/sending in case we've received data from server after edit
         Save(params, Proj.basePath(*elm)); Proj.elmReload(elm_id, false, false);
      }
      changed=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmFont *data=elm.fontData()){data.newVer(); data.from(params);}
         refresh();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_FONT)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm)params.load(Proj.basePath(*elm));else params.reset();
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void toggle  (Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void elmChanged(C UID &font_id)
   {
      if(elm && elm.id==font_id)
      {
         undos.set(null, true);
         EditFont temp; if(temp.load(Proj.basePath(*elm)))if(params.sync(temp))toGui();
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
}
FontEditor FontEdit;
/******************************************************************************/
