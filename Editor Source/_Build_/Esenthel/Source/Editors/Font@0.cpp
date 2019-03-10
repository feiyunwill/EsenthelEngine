/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
// TODO: make unavailable on Mobile
/******************************************************************************/
FontEditor FontEdit;
/******************************************************************************/

/******************************************************************************/
      void FontEditor::Change::create(ptr user)
{
         data=FontEdit.params;
         FontEdit.undoVis();
      }
      void FontEditor::Change::apply(ptr user)
{
         FontEdit.params.undo(data);
         FontEdit.setChanged();
         FontEdit.toGui();
         FontEdit.undoVis();
      }
   void FontEditor::undoVis() {SetUndo(undos, undo, redo);}
  FontEditor::~FontEditor() {del();}
   void FontEditor::Preview(Viewport &viewport) {((FontEditor*)viewport.user)->preview();}
          void FontEditor::preview()
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
            ts.size=0.02f;
            ts.align.set(0, -1);
            flt y=r.max.y;
            FREP(10)
            {
               D.text(ts, Vec2(r.centerX(), y), params.sample_text);
               y-=ts.size.y*0.9f;
               ts.size*=1.3f;
            }
         }
      }
   }
   bool FontEditor::Make(Thread &thread) {return (*(FontEditor*)thread.user).make();}
          bool FontEditor::make()
   {
      if(event.wait() && !thread.wantStop())
      {
         SyncLockerEx locker(lock); Params params=T.params; locker.off();
         ThreadMayUseGPUData();
         Font temp; if(params.make(temp, &params.sample_text)){locker.on(); Swap(font, temp); locker.off();}
      }
      return true;
   }
   void FontEditor::PreChanged(C Property &prop) {FontEdit.undos.set(&prop);}
   void    FontEditor::Changed(C Property &prop) {FontEdit.setChanged();}
   void FontEditor::ParamsFont(Params &p, C Str &t) {p.font           =         t ; p.           font_time.getUTC();}
   void FontEditor::ParamsSize(Params &p, C Str &t) {p.size           =TextInt (t); p.           size_time.getUTC();}
   void FontEditor::ParamsScale(Params &p, C Str &t) {p.scale          =TextFlt (t); p.          scale_time.getUTC();}
   void FontEditor::ParamsClearType(Params &p, C Str &t) {p.clear_type     =TextBool(t); p.     clear_type_time.getUTC();}
   void FontEditor::ParamsSoftware(Params &p, C Str &t) {p.software       =TextBool(t); p.       software_time.getUTC();}
   void FontEditor::ParamsWeight(Params &p, C Str &t) {p.weight         =TextFlt (t); p.         weight_time.getUTC();}
   void FontEditor::ParamsMinFilter(Params &p, C Str &t) {p.min_filter     =TextFlt (t); p.     min_filter_time.getUTC();}
   void FontEditor::ParamsDiagShadow(Params &p, C Str &t) {p.diagonal_shadow=TextBool(t); p.diagonal_shadow_time.getUTC();}
   void FontEditor::ParamsMipMaps(Params &p, C Str &t) {p.mip_maps       =TextInt (t); p.       mip_maps_time.getUTC();}
   void FontEditor::ParamsShdBlur(Params &p, C Str &t) {p.shadow_blur    =TextFlt (t); p.    shadow_blur_time.getUTC();}
   void FontEditor::ParamsShdOpacity(Params &p, C Str &t) {p.shadow_opacity =TextFlt (t); p. shadow_opacity_time.getUTC();}
   void FontEditor::ParamsShdSpread(Params &p, C Str &t) {p.shadow_spread  =TextFlt (t); p.  shadow_spread_time.getUTC();}
   void FontEditor::ParamsAscii(Params &p, C Str &t) {p.ascii          =TextBool(t); p.          ascii_time.getUTC();}
   void FontEditor::ParamsGerman(Params &p, C Str &t) {p.german         =TextBool(t); p.         german_time.getUTC();}
   void FontEditor::ParamsFrench(Params &p, C Str &t) {p.french         =TextBool(t); p.         french_time.getUTC();}
   void FontEditor::ParamsPolish(Params &p, C Str &t) {p.polish         =TextBool(t); p.         polish_time.getUTC();}
   void FontEditor::ParamsRussian(Params &p, C Str &t) {p.russian        =TextBool(t); p.        russian_time.getUTC();}
   void FontEditor::ParamsChinese(Params &p, C Str &t) {p.chinese        =TextBool(t); p.        chinese_time.getUTC();}
   void FontEditor::ParamsJapanese(Params &p, C Str &t) {p.japanese       =TextBool(t); p.       japanese_time.getUTC();}
   void FontEditor::ParamsKorean(Params &p, C Str &t) {p.korean         =TextBool(t); p.         korean_time.getUTC();}
   void FontEditor::ParamsCustomChar(Params &p, C Str &t) {p.custom_chars   =         t ; p.   custom_chars_time.getUTC();}
   void FontEditor::ParamsSampleText(Params &p, C Str &t) {p.sample_text    =         t ; FontEdit.refresh();}
   void FontEditor::Undo(FontEditor &editor) {editor.undos.undo();}
   void FontEditor::Redo(FontEditor &editor) {editor.undos.redo();}
   void FontEditor::Locate(FontEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void FontEditor::create()
   {
      add("System Font"    , MemberDesc(MEMBER(Params, font           )).setTextToDataFunc(ParamsFont      ));
      add("Size"           , MemberDesc(MEMBER(Params, size           )).setTextToDataFunc(ParamsSize      )).range(8, 128).desc("Image Resolution");
      add("Scale"          , MemberDesc(MEMBER(Params, scale          )).setTextToDataFunc(ParamsScale     )).range(0.5f, 2.0f).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(0.25f).desc("This will scale the original source Font, however the destination area size to store the Font remains the same.\nWhich means if you set the scale too big, then Font may not fit into the destination, and may get clipped.\nThis is useful for Fonts that have very small or very big characters.");
      add("Clear Type"     , MemberDesc(MEMBER(Params, clear_type     )).setTextToDataFunc(ParamsClearType ));
      add("Software"       , MemberDesc(MEMBER(Params, software       )).setTextToDataFunc(ParamsSoftware  )).desc("Create Font in Software mode, which will allow faster software processing (drawing in software mode), however drawing using the GPU will not be allowed");
    //add("Sub Pixel"      , MemberDesc(MEMBER(Params, sub_pixel      )).setTextToDataFunc(ParamsSubPixel  )).desc("Warning: enabling this option will increase font quality only if it will be drawn with correct scale,\nin other case font quality will be worse.\nThis option has no effect if 'Clear Type' is disabled.\nUsing this option disables shadows.");
      add("Weight"         , MemberDesc(MEMBER(Params, weight         )).setTextToDataFunc(ParamsWeight    )).range(0, 1).mouseEditSpeed(0.25f);
      add("Minimum Filter" , MemberDesc(MEMBER(Params, min_filter     )).setTextToDataFunc(ParamsMinFilter )).range(0, 1).mouseEditSpeed(0.25f);
      add("Diagonal Shadow", MemberDesc(MEMBER(Params, diagonal_shadow)).setTextToDataFunc(ParamsDiagShadow));
      add("Mip Maps"       , MemberDesc(MEMBER(Params, mip_maps       )).setTextToDataFunc(ParamsMipMaps   )).range(0, 8).mouseEditSpeed(2).desc("Set number of Mip Maps:\n0 = Full Set\n1 = 1 Mip Map\n2 = 2 Mip Maps\n3 = 3 Mip Maps\n..");
      add("Shadow Blur"    , MemberDesc(MEMBER(Params, shadow_blur    )).setTextToDataFunc(ParamsShdBlur   )).range(0, 1).mouseEditSpeed(0.05f);
      add("Shadow Opacity" , MemberDesc(MEMBER(Params, shadow_opacity )).setTextToDataFunc(ParamsShdOpacity)).range(0, 1).mouseEditSpeed(0.30f);
      add("Shadow Spread"  , MemberDesc(MEMBER(Params, shadow_spread  )).setTextToDataFunc(ParamsShdSpread )).range(0, 1).mouseEditSpeed(0.30f);
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

      ::PropWin::create("Font Editor", Vec2(0.02f, -0.07f), 0.036f, 0.043f, 0.3f); ::PropWin::changed(Changed, PreChanged); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); text.changed(null, null); nos.changed(null, null);
      T+=undo  .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      clientRect(Rect_C(0, 0, 2, 1.156f));
      T+=viewport.create(Rect_LU(0.6f, -0.04f, 1.37f, 0.9f), Preview, this);
   }
   void FontEditor::toGui() {::PropWin::toGui(); refresh();}
   void FontEditor::setInfo()
   {
      // can't use 'font' because that's just the preview
      // saved file has only previously generated data
   }
   void FontEditor::stopThread() {thread.stop(); event.on();}
   void FontEditor::refresh() {if(elm){event.on(); if(!thread.active() || thread.wantStop())thread.create(Make, this);}}
   FontEditor& FontEditor::del(){stopThread(); thread.del(); ::EE::Window::del (); return T;}
   FontEditor& FontEditor::hide(){stopThread(); set(null   ); ::PropWin::hide(); return T;}
   void FontEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmFont *data=elm->fontData()){data->newVer(); data->from(params);} // modify just before saving/sending in case we've received data from server after edit
         Save(params, Proj.basePath(*elm)); Proj.elmReload(elm_id, false, false);
      }
      changed=false;
   }
   void FontEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmFont *data=elm->fontData()){data->newVer(); data->from(params);}
         refresh();
      }
   }
   void FontEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_FONT)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm)params.load(Proj.basePath(*elm));else params.reset();
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void FontEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void FontEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void FontEditor::elmChanged(C UID &font_id)
   {
      if(elm && elm->id==font_id)
      {
         undos.set(null, true);
         EditFont temp; if(temp.load(Proj.basePath(*elm)))if(params.sync(temp))toGui();
      }
   }
   void FontEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
FontEditor::FontEditor() : elm_id(UIDZero), elm(null), changed(false), undos(true) {}

FontEditor::Params::Params() : sample_text("Sample Text"), no_scale(false) {}

/******************************************************************************/
