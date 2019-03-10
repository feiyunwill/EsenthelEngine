/******************************************************************************/
Progress UpdateProgress;
Thread   UpdateThread;

Image    WhiteImage;
ImagePtr GlowBorder;
Panel    WindowNoShadow, ActiveWindowNoShadow;
GuiSkin  DarkSkin, LightSkin, TransparentSkin, HalfTransparentSkin, LitSkin, RedSkin, NoComboBoxImage, NoShadowSkin;
/******************************************************************************/
bool BigVisible  (          ) {return MtrlEdit.bigVisible() || WaterMtrlEdit.bigVisible() || Theater.visible();}
void HideBig     (          ) {MtrlEdit.hideBig(); WaterMtrlEdit.hideBig(); Theater.hideDo();}
void HideProjsAct(GuiObj &go) {go.hide(); Projs.proj_list.kbSet();} // hide and set keyboard focus to the Projects list
void HideProjAct (GuiObj &go) {go.hide(); Proj .list     .kbSet();} // hide and set keyboard focus to the Project  view
void HideEditAct (GuiObj &go) {go.hide(); Mode .          kbSet();} // hide and set keyboard focus to the Edit     view
/******************************************************************************/
<TYPE> class ObjPtrs // warning: this must be stored only in global space
{
   bool         deleted=false;
   Memc<TYPE*> *objs; //=null; do not set this to null because this may have been already used before the constructor was called

  ~ObjPtrs() {deleted=true; Delete(objs);}

   static int ComparePtr(TYPE*C &a, TYPE*C &b) {return Compare(a, b);}

   void include(TYPE &obj) {if(!deleted){if(!objs)New(objs); objs.binaryInclude(&obj, ComparePtr);}}
   void exclude(TYPE &obj) {             if( objs)           objs.binaryExclude(&obj, ComparePtr); }

   TYPE& operator[](int i) {return *((*objs)[i]);}
}
<TYPE> int Elms(C ObjPtrs<TYPE> &obj_ptrs) {return obj_ptrs.objs ? obj_ptrs.objs.elms() : 0;}
/******************************************************************************/
class TextWhite : TextStyle // text style which resets its color upon changing skin with preference to black
{
   static ObjPtrs<TextWhite> texts;

   void       skinChanged() {resetColors(false);}
   TextWhite& reset      () {super.reset(false); return T;}
   TextWhite& operator=  (C TextStyle &ts) {SCAST(TextStyle, T)=ts; return T;}

   TextWhite() {texts.include(T);}
  ~TextWhite() {texts.exclude(T);}
}
class TextBlack : TextStyle // text style which resets its color upon changing skin with preference to black
{
   static ObjPtrs<TextBlack> texts;

   void       skinChanged() {resetColors(true);}
   TextBlack& reset      () {super.reset(true); return T;}
   TextBlack& operator=  (C TextStyle &ts) {SCAST(TextStyle, T)=ts; return T;}

   TextBlack() {texts.include(T);}
  ~TextBlack() {texts.exclude(T);}
}
class ViewportSkin : Viewport
{
   static ObjPtrs<ViewportSkin> viewports;

   void skinChanged() {rect_color=Gui.borderColor();}

   ViewportSkin() {viewports.include(T);}
  ~ViewportSkin() {viewports.exclude(T);}
}
class ImageSkin : GuiImage
{
   static ObjPtrs<ImageSkin> images;

   void skinChanged() {rect_color=Gui.borderColor();}

   ImageSkin() {images.include(T);}
  ~ImageSkin() {images.exclude(T);}
}
/******************************************************************************/
class TextNoTest : Text
{
   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return null;}
}
/******************************************************************************/
class PropEx : Property
{
   static ObjPtrs<PropEx> props;

   static void Clear(PropEx &prop) {prop.clear();}

   ELM_TYPE elm_type=ELM_NONE;
   Str     _desc;

   bool compatible(ELM_TYPE elm_type)C {return ElmCompatible(elm_type, T.elm_type);}

   void    setDesc(                        ) {Str d=_desc; if(elm_type)d.line()+=S+"Drag and drop "+ElmTypeName[elm_type]+" element here\nCtrl+LeftClick to open it"; super.desc(d);}
   PropEx& desc   (C Str &desc             ) {_desc=desc; setDesc(); return T;}
   PropEx& setEnum(                        ) {super.setEnum(          ); return T;}
   PropEx& setEnum(cchar8 *data[], int elms) {super.setEnum(data, elms); return T;}
   PropEx& setEnum(cchar  *data[], int elms) {super.setEnum(data, elms); return T;}
   PropEx& elmType(ELM_TYPE elm_type       ) {T.elm_type=elm_type; setDesc(); if(elm_type)button.create("C").func(Clear, T).desc(S+"Clear this "+((elm_type==ELM_ANY) ? "element" : ElmTypeName[elm_type])); return T;}
   void    clear  (                        ) {super.set(S);}

   T1(TYPE) PropEx& create(C Str &name,   TYPE       &member         ) {super.create(name, member); return T;}
            PropEx& create(C Str &name, C MemberDesc &md=MemberDesc()) {super.create(name, md    ); return T;}

   void dragStart(bool elm_types[ELM_NUM_ANY]) {textline.enabled(elm_types[elm_type]);} // enable textline only if the source has the same element type as this property
   void dragEnd  (                           ) {textline.enabled(true               );}

   PropEx() {props.include(T);}
  ~PropEx() {props.exclude(T);}
}
class PropWin : ClosableWindow
{
   Memx<PropEx> props;
   TextBlack    ts;

   Rect create(C Str &name, C Vec2 &lu=Vec2(0.02, -0.02), C Vec2 &text_scale=0.036, flt property_height=0.043, flt value_width=0.3)
   {
      ts.reset().size=text_scale; ts.align.set(1, 0);
      Gui+=super.create(name).hide();
      Rect rect =AddProperties(props, T, lu, property_height, value_width, &ts);
      flt  right=Max(rect.max.x, defaultBarFullWidth()+button[2].rect().w()); // here 'button[2]' is hidden so add it manually
      Vec2 padd =defaultInnerPaddingSize()+0.02; T.rect(Rect_RD(D.w(), -D.h(), right+padd.x, -rect.min.y+padd.y));
      return rect;
   }
   PropEx &     add (C Str &name=S, C MemberDesc &md=MemberDesc()) {return props.New().create(name, md);}
   PropWin&   toGui (         ) {REPAO(props).  toGui (    ); return T;}
   PropWin&   toGui (cptr data) {REPAO(props).  toGui (data); return T;}
   PropWin& autoData( ptr data) {REPAO(props).autoData(data); return T;}   ptr autoData()C {FREPA(props)if(ptr data=props[i].autoData())return data; return null;}
   PropWin& changed (void (*changed)(C Property &prop), void (*pre_changed)(C Property &prop)=null) {REPAO(props).changed(changed, pre_changed); return T;}

   virtual PropWin& hide()override {super.hide(); REPAO(props).close(); return T;}
}
/******************************************************************************/
class RenameElmClass : ClosableWindow
{
   UID      elm_id=UIDZero;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Rename Element").level(1).hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void activate(C UID &elm_id, C Str &name)
   {
      T.elm_id=elm_id;
      textline.set(name).selectAll().activate();
      super.activate();
   }
   void activate(C UID &elm_id) {activate(Proj.findElm(elm_id));}
   void activate(  Elm *elm   ) {if(elm)activate(elm.id, elm.name);}
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER)){Proj.renameElm(elm_id, textline()); hide(); Kb.eatKey(); Proj.list.activate();} // set keyboard focus to the Project view
      }
   }
}
RenameElmClass RenameElm;
/******************************************************************************/
class ReplaceNameClass : ClosableWindow
{
   Memc<UID> elm_ids;
   Text      t_from, t_to;
   TextLine  from, to;

   void create()
   {
      flt y=-0.05;
      Gui+=super.create(Rect_C(0, 0, 1, 0.23), "Replace Name Part").level(1).hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      T+=t_from .create(Vec2(0.1, y), "From"); T+=from.create(Rect(0.2, y-0.03, clientWidth()-0.01, y+0.03)); y-=0.07;
      T+=t_to   .create(Vec2(0.1, y), "To"  ); T+=to  .create(Rect(0.2, y-0.03, clientWidth()-0.01, y+0.03)); y-=0.07;
   }
   void activate(C MemPtr<UID> &elm_ids)
   {
      T.elm_ids=elm_ids;
      if(elm_ids.elms()){from.selectAll().activate(); super.activate();}else button[2].push();
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER)){Proj.replaceName(elm_ids, from(), to()); hide(); Kb.eatKey(); Proj.list.activate();} // set keyboard focus to the Project view
      }
   }
}
ReplaceNameClass ReplaceName;
/******************************************************************************/
class RenameSlotClass : ClosableWindow
{
   int      slot_index=-1; // number of slots before this one with similar name
   Str      slot_name;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Rename Slot").hide(); button[2].show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void activate(int slot)
   {
      if(ObjEdit.mesh_skel && InRange(slot, ObjEdit.mesh_skel.slots))
      {
         slot_name =ObjEdit.mesh_skel.slots[slot].name;
         slot_index=0; for(int i=0; i<slot; i++)if(slot_name==ObjEdit.mesh_skel.slots[i].name)slot_index++;
         textline.set(slot_name).selectAll().activate();
         super.activate();
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER)){ObjEdit.renameSlot(slot_index, slot_name, textline()); hide(); Kb.eatKey();}
      }
   }
}
RenameSlotClass RenameSlot;
/******************************************************************************/
class RenameBoneClass : ClosableWindow
{
   Str      bone_name;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Rename Bone").hide(); button[2].show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void activate(int bone)
   {
      if(ObjEdit.mesh_skel && InRange(bone, ObjEdit.mesh_skel.bones))
      {
         bone_name=ObjEdit.mesh_skel.bones[bone].name;
         textline.set(bone_name).selectAll().activate();
         super.activate();
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER)){Kb.eatKey(); if(ObjEdit.renameBone(bone_name, textline()))hide();}
      }
   }
}
RenameBoneClass RenameBone;
/******************************************************************************/
class RenameEventClass : ClosableWindow
{
   int      event_index=-1; // number of events before this one with similar name
   Str      event_name;
   TextLine textline;

   static void Hide(RenameEventClass &re) {re.hide(); AnimEdit.focus();}

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Rename Event").hide(); button[2].show().func(Hide, T);
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void activate(int event)
   {
      if(AnimEdit.anim && InRange(event, AnimEdit.anim.events))
      {
         event_name =AnimEdit.anim.events[event].name;
         event_index=0; for(int i=0; i<event; i++)if(event_name==AnimEdit.anim.events[i].name)event_index++;
         textline.set(event_name).selectAll().activate();
         super.activate();
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER)){AnimEdit.renameEvent(event_index, event_name, textline()); Kb.eatKey(); Hide(T);}
      }
   }
}
RenameEventClass RenameEvent;
/******************************************************************************/
class NewWorldClass : ClosableWindow
{
   TextBlack   ts;
   TextNoTest tname, tarea_size, theightmap_res, tdensity, density;
   TextLine    name;
   ComboBox           area_size,  heightmap_res;
   Button     ok, cancel;
   UID        parent_id=UIDZero;

   static void OK(NewWorldClass &nw) {Proj.newWorld(nw.name(), nw.areaSize(), nw.heightmapRes(), nw.parent_id, true); HideProjAct(nw);}

   int      areaSize() {return TextInt(     area_size.text());}
   int heightmapRes () {return TextInt(heightmap_res .text());}

   void create()
   {
      static cchar8 *area_siz[]=
      {
         "32",
         "64 (default)",
         "128",
       // don't allow more because there will be conflict of lods with Macro textures from the shaders (which fade out at a specified distance)
      };
      static cchar8 *hm_res[]=
      {
         "32",
         "64 (default)",
         "128",
      }; //ASSERT(MIN_HM_RES==33 && MAX_HM_RES==129);

      ts.reset().align.set(1, 0);

      Gui+=super.create(Rect_C(0, 0, 1.0, 0.7), MLTC(u"New World", PL, u"Nowy Świat", DE, u"Neue Welt", RU, u"Новый мир", PO, u"Novo Mundo")).hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();

      T+=tname.create(Vec2(0.05, -0.1), "Name:", &ts);
      T+= name.create(Rect_L(clientWidth()/2+0.05, -0.1, 0.35, 0.06));

      T+=tarea_size.create(Vec2(0.05, -0.2), MLTC(u"Area Size (meters):", PL, u"Rozmiar Obszaru (metry):", DE, u"Gebiets Größe (Meter):", RU, u"Размер области (в метрах):", PO, u"Área (metros):"), &ts);
      T+= area_size.create(Rect_L(clientWidth()/2+0.05, -0.2, 0.35, 0.06), area_siz, Elms(area_siz)).set(1).desc(MLTC(u"Lower Area Size is for Games which target Lower View Range with More Details.\nHigher Area Size is for Games which target Higher View Range with Less Details.", DE, u"Kleinere Gebietsgröße ist für Spiele mit geringerer Sichtweite und höherem Detail.\nGrößere Gebietsgrößen ist für Spiele die höhere Sichtweite mit weniger Detail wollen.", RU, u"Области с более низким размером. Для игр, где видимость меньше и большое количество деталей.\nОбласти с более высоким размером. Для игр, где видимость больше и меньше количество деталей.", PO, u"Áreas pequenas é para Jogos que tęm um menor alcance de visăo mas mais Detalhado.\nÁreas grandes é para Jogos que tęm maior alcance da visăo mas menos Detalhado."));

      T+=theightmap_res.create(Vec2(0.05, -0.3), MLTC(u"Heightmap Resolution:", PL, u"Rozdzielczość Heightmapy:", DE, u"Heightmap Auflösung:", RU, u"Разрешение карты высот:", PO, u"Resoluçăo do Heightmap:"), &ts);
      T+= heightmap_res.create(Rect_L(clientWidth()/2+0.05, -0.3, 0.35, 0.06), hm_res, Elms(hm_res)).set(1);

      T+=tdensity.create(Vec2(0.05, -0.4), MLTC(u"Heightmap Density:", PL, u"Gęstość Heightmapy:", DE, u"Heightmap Dichte:", RU, u"Плотность карты высот:", PO, u"Densidade do Heightmap:"), &ts);
      T+= density.create(Vec2(clientWidth()/2+0.05, -0.4), S, &ts);

      T+=    ok.create(Rect_C(clientWidth()*3/8, -0.55, 0.2, 0.06), MLTC(u"OK"                                                                     )).func(OK  , T);
      T+=cancel.create(Rect_C(clientWidth()*5/8, -0.55, 0.2, 0.06), MLTC(u"Cancel", PL, u"Anuluj", DE, u"Abbrechen", RU, u"Отмена", PO, u"Cancelar")).func(HideProjAct, SCAST(GuiObj, T));
   }
   void display()
   {
      if(Proj.testElmsNum())
      {
         Proj.setMenuListSel();
         parent_id=(Proj.menu_list_sel.elms() ? Proj.menu_list_sel[0] : UIDZero);
         name.set(Proj.newElmName(ELM_WORLD, parent_id));
         super.activate();
      }
   }
   void update(C GuiPC &gpc)
   {
      super.update(gpc);

      if(visible() && gpc.visible)
      {
         density.set(S+TextReal(flt(heightmapRes())/areaSize(), -3)+MLTC(u" per meter", PL, u" na metr", DE, u" pro Meter", RU, u" за метр", PO, u" por metro"));
         if(Gui.window()==this && Kb.k(KB_ENTER))ok.push();
      }
   }
}
NewWorldClass NewWorld;
/******************************************************************************/
enum MODE
{
   MODE_OBJ     ,
   MODE_ANIM    ,
   MODE_WORLD   ,
   MODE_GUI_SKIN,
   MODE_GUI     ,
   MODE_CODE    ,
   MODE_TEX_DOWN,
   MODE_NUM     ,
}
cchar8 *ModeText[]=
{
   "Object",
   "Anim",
   "World",
   "GuiSkin",
   "Gui",
   "Code",
   "Texture Downsize",
};
class ModeTabs : Tabs
{
   class TabEx : Tab
   {
      virtual void update(C GuiPC &gpc)override
      {
         GuiSkinPtr temp; if(Mode.Gui_skin){Swap(temp, Gui.skin); Gui.skin=Mode.Gui_skin;} // restore Editor's Gui.skin before updating this Tab
         super.update(gpc);
         if(temp)Swap(Gui.skin, temp);
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(gpc.visible && visible())
         {
            GuiSkinPtr temp; if(Mode.Gui_skin){Swap(temp, Gui.skin); Gui.skin=Mode.Gui_skin;} // restore Editor's Gui.skin before drawing this Tab
            super.draw(gpc);
            if(temp)Swap(Gui.skin, temp);
         }
      }
   }

   Memc<MODE> order;
   Button     close;
   bool       available[MODE_NUM];
   GuiSkinPtr Gui_skin; // this is set to Editor's Gui.skin, and not current App's GuiSkin

   ModeTabs() {Zero(available); replaceClass<TabEx>();}

   static void CloseActive(ModeTabs &mt ) {mt.closeActive(true);}
          void closeActive(bool      all) {closeTab(T(), all);}

   static void Changed(ModeTabs &mt) {mt.changed();}
          void changed()
   {
      Misc.updateMove();
        ObjEdit.selectedChanged();
       AnimEdit.selectedChanged();
      WorldEdit.selectedChanged();
    GuiSkinEdit.selectedChanged();
        GuiEdit.selectedChanged();
       CodeEdit.selectedChanged();
    TexDownsize.selectedChanged();
      if(InRange(T(), MODE_NUM))
      {
         order.exclude(MODE(T()), true);
         order.include(MODE(T()));
      }
      setCloseVisibility(); // because close should always be visible for MODE_TEX_DOWN
      SetTitle();
      SetKbExclusive();
      if(T()==MODE_CODE)CodeEdit.resize();
   }
   Elm* elm(int i)
   {
      switch(i)
      {
         case MODE_OBJ     : return     ObjEdit.obj_elm;
         case MODE_ANIM    : return    AnimEdit.elm;
         case MODE_WORLD   : return   WorldEdit.elm;
         case MODE_GUI_SKIN: return GuiSkinEdit.elm;
         case MODE_GUI     : return     GuiEdit.elm;
      }
      return null;
   }
   void closeTab(int i, bool all)
   {
      switch(i)
      {
         case MODE_OBJ     :     ObjEdit.set(null); break;
         case MODE_ANIM    :    AnimEdit.set(null); break;
         case MODE_WORLD   :   WorldEdit.set(null); break;
         case MODE_GUI_SKIN: GuiSkinEdit.set(null); break;
         case MODE_GUI     :     GuiEdit.set(null); break;
         case MODE_CODE    : if(all)CodeEdit.closeAll();else CodeEdit.close(); break;
         case MODE_TEX_DOWN: TexDownsize.close(); break;
      }
   }
   void kbSet()
   {
      switch(T())
      {
         case MODE_OBJ     :     ObjEdit.kbSet(); break;
         case MODE_ANIM    :    AnimEdit.kbSet(); break;
         case MODE_WORLD   :   WorldEdit.kbSet(); break;
         case MODE_GUI_SKIN: GuiSkinEdit.kbSet(); break;
         case MODE_GUI     :     GuiEdit.kbSet(); break;
         case MODE_CODE    :    CodeEdit.kbSet(); break;
         case MODE_TEX_DOWN: TexDownsize.kbSet(); break;
         default           :   Proj.list.kbSet(); break;
      }
   }
   void kbToggle()
   {
      if(Proj.list.contains(Gui.kb()) || !Proj.list.visibleFull())kbSet();else Proj.list.kbSet();
   }
   void activateLast()
   {
      REPA(order)if(available[order[i]]){set(order[i]); return;} valid(false).set(-1).valid(true); kbSet();
   }
   void create()
   {
      Gui+=super.create(ModeText, Elms(ModeText)).func(Changed, T).valid(true).hide();
      Gui+=close.create().func(CloseActive, T).desc("Keyboard Shortcut: Alt+F3"); close.image="Gui/close.img"; close.skin=&EmptyGuiSkin;
      tab(MODE_OBJ     )+=    ObjEdit.create();
      tab(MODE_ANIM    )+=   AnimEdit.create();
      tab(MODE_WORLD   )+=  WorldEdit.create();
      tab(MODE_TEX_DOWN)+=TexDownsize.create();
   GuiSkinEdit.create(tab(MODE_GUI_SKIN));
       GuiEdit.create(tab(MODE_GUI     ));
      CodeEdit.create(tab(MODE_CODE    ));
      setTabs();
      changed(); // call to setup initial settings
   }
   void setCloseVisibility()
   {
      close.visible(StateActive==&StateProject && (visibleTabs() || Mode()==MODE_TEX_DOWN));
   }
   void setTabs()
   {
      flt        x=rect().min.x; if(Misc.visible() && Misc.pos==VecI2(0, 1)){x+=rect().h()*0.1; if(Misc.move_misc.visible())x+=Misc.move_misc.rect().w();}
      bool visible=visibleTabs();
      int v=0; FREPA(T)if(tab(i).visible(visible && available[i]).visible()){tab(i).pos(Vec2(x, rect().max.y)); v++; tab(i).desc(S+"Keyboard Shortcut: "+Kb.ctrlCmdName()+"+F"+v); x+=tab(i).rect().w();}
      setCloseVisibility();
   }
   virtual Tabs& rect(C Rect &rect)override {super.rect(rect); setTabs(); return T;}   C Rect& rect()C {return super.rect();}
   void resize()
   {
      flt l=-D.w(), r=D.w();
      if(          Misc.visible() && Misc.pos==VecI2(0, 1))MAX(l,           Misc.rect().max.x);else
      if(          Proj.visible()                         )MAX(l,           Proj.rect().max.x);

      if(Misc.move_misc.visible() && Misc.pos==VecI2(1, 1))MIN(r, Misc.move_misc.rect().min.x);else
      if(          Misc.visible() && Misc.pos==VecI2(1, 1))MIN(r,           Misc.rect().min.x);
      if(      MtrlEdit.visible()                         )MIN(r,       MtrlEdit.rect().min.x);
      if( WaterMtrlEdit.visible()                         )MIN(r,  WaterMtrlEdit.rect().min.x);

      flt h=0.06;
             rect(Rect_LU(l, D.h()  , 0.17*MODE_NUM,   h  )); //   Tabs.rect to set rects of every single Tab
      GuiObj.rect(Rect   (l, D.h()-h,             r, D.h())); // GuiObj.rect to set rect  of              Tabs
       close.rect(Rect_RU(r, D.h(), rect().h(), rect().h()));
      if(InRange(MODE_TEX_DOWN, tabs()))tab(MODE_TEX_DOWN).resize(Vec2(0.2, 0)); // make this tab wider
   }
   bool tabAvailable(MODE mode         )C {return available[mode];}
   void tabAvailable(MODE mode, bool on)
   {
      if(available[mode]!=on)
      {
         available[mode]=on; setTabs(); CodeEdit.resize();
         if(!on && T()==mode)activateLast();
      }
   }
   bool visibleTabs() {int v=0; REPA(T)v+=available[i]; return v>(ModeTabsAlwaysVisible ? 0 : 1);}
   Tab* visibleTab(int t) {if(InRange(t, T))FREPA(T)if(tab(i).visible())if(!t--)return &tab(i); return null;}

   virtual GuiObj& show()override {setCloseVisibility(); return super.show();}
   virtual GuiObj& hide()override {close.hide        (); return super.hide();}

   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible && Ms.bp(2))REPA(T)if(Gui.ms()==&tab(i)) // close Tab on middle mouse
      {
         closeTab(i, true);
         break;
      }
   }
   virtual void draw(C GuiPC &gpc)override
   {
      if(gpc.visible && visible())
      {
         if(visibleTabs()){D.clip(gpc.clip); (rect()+gpc.offset).draw(BackgroundColorLight());}
         bool gui=(T()==MODE_GUI);
         if(gui){Gui_skin=Gui.skin; if(GuiSkinPtr app_skin=Proj.appGuiSkin())Swap(app_skin, Gui.skin);}
         super.draw(gpc);
         if(gui){Swap(Gui.skin, Gui_skin); Gui_skin.clear();}
      }
   }
}
ModeTabs Mode;
/******************************************************************************/
class ReloadElmClass : ClosableWindow
{
   TextNoTest text, text_all, t_name;
   TextLine   path, name, start_frame, end_frame, speed, optimize;
   Button     path_sel, yes, yes_all, set_start_frame, set_end_frame, set_speed, set_optimize, mirror;
   WindowIO   win_io;
   Memc<UID>  elms;

   static void Select(C Str &name, ReloadElmClass &re) {re.path.set(name);}
   static void Select(ReloadElmClass &re) {SetPath(re.win_io.activate(), re.path(), true);}
   static void Reload(ReloadElmClass &re) {re.reload();}
          void reload()
   {
      if(path.visible() && elms.elms()==1) // single element
         if(Elm *elm=Proj.findElm(elms[0]))
      {
         // adjust its source path according to textline
         Mems<Edit.FileParams> files=Edit.FileParams.Decode(path());
         if(files.elms())
         {
            if(elm.type==ELM_ANIM)
            {
               if(set_start_frame())files[0].getParam("start_frame").value=start_frame();
               if(set_end_frame  ())files[0].getParam(  "end_frame").value=  end_frame();
               if(set_speed      ())files[0].getParam("speed"      ).value=speed      ();
               if(set_optimize   ())files[0].getParam("optimize"   ).value=optimize   ();
               if(mirror         ())files[0].getParam("mirror"     ); // uses TextBool1 so no need to specify =1
            }
            if(elm.type==ELM_ANIM || elm.type==ELM_MTRL)
            {
               files[0].getParam("name").value=name();
            }
         }
         elm.setSrcFile(Edit.FileParams.Encode(files));
         Server.setElmShort(elm.id);
      }

      Proj.elmReload(elms); // reload all elements
      hide();
   }
   void activate(Memc<UID> &elms) // multiple elements
   {
      win_io.hide(); T-=text; T-=path; T-=path_sel; T-=yes; T-=t_name; T-=name; T-=set_start_frame; T-=set_end_frame; T-=set_speed; T-=set_optimize; T-=mirror; T-=start_frame; T-=end_frame; T-=speed; T-=optimize;
      rect(Rect_C(0, 0, 1, 0.36)); T+=text_all; T+=yes_all;
      Swap(elms, T.elms);
      super.activate();
   }
   void activate(C UID &elm_id) // single element
   {
      elms.clear().add(elm_id);
      T-=text_all; T-=yes_all;
      rect(Rect_C(0, 0, 1.4, 0.544)); T+=text; T+=yes; T+=path; T+=path_sel; T+=yes; T-=t_name; T-=name; T-=set_start_frame; T-=set_end_frame; T-=set_speed; T-=set_optimize; T-=mirror; T-=start_frame; T-=end_frame; T-=speed; T-=optimize;
      path.clear();
      if(Elm *elm=Proj.findElm(elm_id))if(elm.type!=ELM_FOLDER && elm.type!=ELM_LIB && elm.type!=ELM_APP)
      {
         Mems<Edit.FileParams> files=Edit.FileParams.Decode(elm.srcFile());
         // check for which params we provide a dedicated gui, remove them from the param list because they're set in the gui, parameters without dedicated gui, keep in the name
         switch(elm.type)
         {
            case ELM_ANIM:
            {
               T+=t_name; T+=name.desc("This is the Animation name inside the source file.\nIf source file has multiple Animations, then one will be selected that has this name.");
               T+=set_start_frame; T+=set_end_frame; T+=set_speed; T+=set_optimize; T+=mirror; T+=start_frame; T+=end_frame; T+=speed; T+=optimize;
               if(files.elms())
               {
                C TextParam *start_frame=files[0].findParam("start_frame"); set_start_frame.set(start_frame!=null); T.start_frame.set(start_frame ? start_frame.value : S    ); files[0].params.removeData(start_frame);
                C TextParam *  end_frame=files[0].findParam(  "end_frame"); set_end_frame  .set(  end_frame!=null); T.  end_frame.set(  end_frame ?   end_frame.value : S    ); files[0].params.removeData(  end_frame);
                C TextParam *speed      =files[0].findParam("speed"      ); set_speed      .set(speed      !=null); T.speed      .set(speed       ? speed      .value : S+"1"); files[0].params.removeData(speed      );
                C TextParam *optimize   =files[0].findParam("optimize"   ); set_optimize   .set(optimize   !=null); T.optimize   .set(optimize    ? optimize   .value : S+"1"); files[0].params.removeData(optimize   );
                C TextParam *name       =files[0].findParam("name"       );                                         T.name       .set(name        ? name       .value : S    ); files[0].params.removeData(name       );
                C TextParam *mirror     =files[0].findParam("mirror"     );                                         T.mirror     .set(mirror      ? mirror.asBool1()  : false); files[0].params.removeData(mirror     );
               }
            }break;

            case ELM_MTRL:
            {
               T+=t_name; T+=name.desc("This is the Material name inside the source file.\nIf source file has multiple Materials, then one will be selected that has this name.");
               if(files.elms())
               {
                C TextParam *name=files[0].findParam("name"); T.name.set(name ? name.value : S); files[0].params.removeData(name);
               }
            }break;
         }
         path.set(Edit.FileParams.Encode(files));
         super.activate();
      }
   }
   virtual ReloadElmClass& hide()override
   {
      win_io.hide();
      super.hide();
      return T;
   }

   static void ToggleStart   (ReloadElmClass &re) {re.start_frame.visible(re.set_start_frame());}
   static void ToggleEnd     (ReloadElmClass &re) {re.  end_frame.visible(re.set_end_frame  ());}
   static void ToggleSpeed   (ReloadElmClass &re) {re.speed      .visible(re.set_speed      ());}
   static void ToggleOptimize(ReloadElmClass &re) {re.optimize   .visible(re.set_optimize   ());}

   void create()
   {
      cchar8 *speed_t="Set custom speed factor, default=1", *optimize_t="Set custom keyframe optimization (reduction) factor, default=1\nIf not specified, then optimization of 1 is applied.\nValue of 0 completely disables keyframe reduction.";
      Gui+=super.create("Reload Elements").hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      text_all .create(Vec2  (0.5  , -0.09), "Are you sure you wish to reload all selected\nelements from their original files?");
      text     .create(Vec2  (0.7  , -0.09), "Are you sure you wish to reload selected\nelement from its original file?");
      path     .create(Rect_C(0.675, -0.20, 1.3 , 0.055));
      path_sel .create(Rect_LU(path.rect().ru(), path.rect().h()), "...").func(Select, T).focusable(false);
    t_name     .create(Vec2(0.08, -0.264), "Name");
      name     .create(Rect_L(t_name.rect().right()+Vec2(0.055, 0), 0.3, 0.0475));
set_start_frame.create(Rect_L(0.50, -0.264, 0.27, 0.0475), "Set Start Frame").func(ToggleStart   , T)                 ; set_start_frame.mode=BUTTON_TOGGLE; start_frame.create(Rect_LU(set_start_frame.rect().ru(), 0.15, set_start_frame.rect().h())).hide();
set_end_frame  .create(Rect_L(0.98, -0.264, 0.25, 0.0475), "Set End Frame"  ).func(ToggleEnd     , T)                 ; set_end_frame  .mode=BUTTON_TOGGLE;   end_frame.create(Rect_LU(set_end_frame  .rect().ru(), 0.15, set_end_frame  .rect().h())).hide();
set_speed      .create(Rect_L(0.50, -0.328, 0.27, 0.0475), "Set Speed"      ).func(ToggleSpeed   , T).desc(   speed_t); set_speed      .mode=BUTTON_TOGGLE;       speed.create(Rect_LU(set_speed      .rect().ru(), 0.15, set_speed      .rect().h())).hide().desc(   speed_t);
set_optimize   .create(Rect_L(0.98, -0.328, 0.25, 0.0475), "Set Optimize"   ).func(ToggleOptimize, T).desc(optimize_t); set_optimize   .mode=BUTTON_TOGGLE;    optimize.create(Rect_LU(set_optimize   .rect().ru(), 0.15, set_optimize   .rect().h())).hide().desc(optimize_t);
      mirror   .create(Rect_L(path.rect().min.x, -0.328, 0.23, 0.0475), "Mirror");                                      mirror         .mode=BUTTON_TOGGLE;
      yes_all  .create(Rect_C(0.5 , -0.22 , 0.29, 0.07), "Yes").func(Reload, T).focusable(false);
      yes      .create(Rect_C(0.7 , -0.408, 0.29, 0.07), "Yes").func(Reload, T).focusable(false);
      win_io.create(S, S, S, Select, Select, T);
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))yes.push();
      }
   }
}
ReloadElmClass ReloadElm;
/******************************************************************************/
class ImportCodeClass : Window
{
   class Update
   {
      UID id=UIDZero;
      Str data;

      Update& set(C UID &id) {T.id=id; return T;}

      static Str AsText(C Update &code) {return Proj.elmFullName(code.id);}
   }

   TextNoTest   text;
   Memc<Update> updates;
   List<Update> list;
   Region       region;
   Button       yes, no;

   static void No (ImportCodeClass &is) {            is.del();}
   static void Yes(ImportCodeClass &is) {is.apply(); is.del();}

   virtual Rect sizeLimit()C override {Rect size=super.sizeLimit(); size.min.set(0.9, 0.5); return size;}
                    C Rect& rect()C {return super::rect();}
   virtual ImportCodeClass& rect(C Rect &rect)override
   {
      super .rect(rect);
      text  .rect(Vec2(clientWidth()/2, -0.10));
      region.rect(Rect_LU(0, -0.14, clientWidth(), clientHeight()-0.205).extend(-0.05));
      yes   .rect(Rect_C(clientWidth()*0.3, -clientHeight()+0.06, 0.25, 0.06));
      no    .rect(Rect_C(clientWidth()*0.7, -clientHeight()+0.06, 0.25, 0.06));
      return T;
   }
   bool apply()
   {
      bool ok=true;
      REPA(updates)
      {
         Update &update=updates[i];
         if(Elm *elm=Proj.findElm(update.id, ELM_CODE))
            if(elm.finalExists())
               ok&=Proj.codeSet(elm.id, update.data);
      }
      updates.clear();
      return ok;
   }
   void import()
   {
      if(updates.elms())
      {
         Gui+=super .create("Code Import"); button[2].func(No, T).show(); flag|=WIN_RESIZABLE;
         T  +=text  .create("Are you sure you wish to import codes\nfrom Code Synchronization folder?\nFollowing files will be modified:");
         T  +=region.create();
         T  +=yes   .create("OK"    ).func(Yes, T);
         T  +=no    .create("Cancel").func(No , T);
         ListColumn lc[]=
         {
            ListColumn(Update.AsText, LCW_DATA, "File"),
         };
         region+=list.create(lc, Elms(lc), true); list.sort_column[0]=0; list.setData(updates);
         rect(Rect_C(0, 0, 1.0, 1.0));
         activate();
      }else
      {
         Gui.msgBox(S, "Importing Codes completed.\nNo files were modified.");
         del();
      }
   }
}
ImportCodeClass ImportCode;
/******************************************************************************/
class SetMtrlColorClass : PropWin
{
   bool      mul;
   Vec       rgb=1;
   Button    ok;
   Memc<UID> elms;

   static void OK(SetMtrlColorClass &smc)
   {
      Proj.mtrlRGB(smc.elms, smc.rgb, smc.mul);
      smc.hide();
   }

   void create()
   {
      add("Red"  , MEMBER(SetMtrlColorClass, rgb.x)).range(0, 4);
      add("Green", MEMBER(SetMtrlColorClass, rgb.y)).range(0, 4);
      add("Blue" , MEMBER(SetMtrlColorClass, rgb.z)).range(0, 4);
      autoData(this);

      Rect r=super.create(S, Vec2(0.02, -0.02), 0.04, 0.045, 0.2); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); hide();
      T+=ok.create(Rect_U(clientWidth()/2, r.min.y-0.03, 0.25, 0.053), "OK").func(OK, T);
      rect(Rect_C(0, size()+Vec2(0, 0.09)));
   }
   void display(C MemPtr<UID> &elms, bool mul=false)
   {
      T.mul=mul;
      T.elms=elms;
      setTitle(mul ? "Multiply Material Color" : "Set Material Color");
      activate();
   }

   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER))
      {
         Kb.eatKey();
         OK(T);
      }
   }
}
SetMtrlColorClass SetMtrlColor;
/******************************************************************************/
class MulSoundVolumeClass : PropWin
{
   flt       volume=1;
   Button    ok;
   Memc<UID> elms;

   static void OK(MulSoundVolumeClass &smc)
   {
      Proj.mulSoundVolume(smc.elms, smc.volume);
      smc.hide();
   }

   void create()
   {
      add("Volume", MEMBER(MulSoundVolumeClass, volume)).range(0, 10);
      autoData(this);

      Rect r=super.create("Multiply Sound Volume", Vec2(0.02, -0.02), 0.04, 0.045, 0.2); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); hide();
      T+=ok.create(Rect_U(clientWidth()/2, r.min.y-0.03, 0.25, 0.053), "OK").func(OK, T);
      rect(Rect_C(0, size()+Vec2(0, 0.09)));
   }
   void display(C MemPtr<UID> &elms)
   {
      T.elms=elms;
      activate();
   }

   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER))
      {
         Kb.eatKey();
         OK(T);
      }
   }
}
MulSoundVolumeClass MulSoundVolume;
/******************************************************************************/
class EraseRemovedElms : ClosableWindow
{
   static void OK(EraseRemovedElms &ere) {ere.hide(); Proj.eraseRemoved();}

   class Elm
   {
      Str name;
      UID id=UIDZero;
      
      void create(C .Elm &src)
      {
         name=Proj.elmFullName(&src);
         id  =src.id;
      }
   }

   TextNoTest text;
   Button     ok, cancel;
   Region     region;
   Memc<Elm>  data;
   List<Elm>  list;

   void create()
   {
      Gui+=super .create(Rect_C(0, 0, 1.44, 1.5), "Erase Removed Elements").hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      T  +=text  .create(Rect_C(clientWidth()/2  , -0.15, clientWidth()-0.05, 0.1), "Are you sure you wish to erase all removed elements from the project?\nWarning: This operation cannot be undone!\n\nThis will remove files only from the local computer - when connected to server it will redownload the elements."); text.auto_line=AUTO_LINE_SPACE_SPLIT;
      T  +=ok    .create(Rect_C(clientWidth()*1/3, -0.34, 0.29, 0.07), "OK"    ).func(OK         ,               T ).focusable(false);
      T  +=cancel.create(Rect_C(clientWidth()*2/3, -0.34, 0.29, 0.07), "Cancel").func(HideProjAct, SCAST(GuiObj, T)).focusable(false);
      T  +=region.create(Rect  (0, -clientHeight(), clientWidth(), ok.rect().min.y-0.01).extend(-0.03));
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Elm, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      region+=list.create(lc, Elms(lc), true).elmHeight(0.038).textSize(0, 1); list.cur_mode=LCM_ALWAYS;
   }
   virtual EraseRemovedElms& show()override
   {
      if(hidden())
      {
         Memc<UID> remove; Proj.floodRemoved(remove, Proj.root);
         data.clear();
         FREPA(remove)if(.Elm *elm=Proj.findElm(remove[i]))if(ElmVisible(elm.type))data.New().create(*elm);
         list.setData(data);
         super.show();
      }
      return T;
   }
   virtual EraseRemovedElms& hide()override
   {
      list.clear();
      data.clear();
      super.hide(); return T;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(Ms.bd(0) && Gui.ms()==&list)if(Elm *elm=list())Proj.elmToggle(elm.id);
   }
}
EraseRemovedElms EraseRemoved;
/******************************************************************************/
class ProjectLocked : ClosableWindow
{
   static void OK(ProjectLocked &pl)
   {
      pl.del();
      if(Projects.Elm *proj=Projs.findProj(pl.proj_id))Projs.open(*proj, true);
   }

   TextNoTest text;
   Button     ok, cancel;
   UID        proj_id=UIDZero;

   void create(C UID &proj_id)
   {
      T.proj_id=proj_id;
      Gui+=super .create(Rect_C(0, 0, 1.59, 0.38), "Project Already Opened"); button[2].show();
      T  +=text  .create(Rect_C(clientWidth()/2  , -0.10, clientWidth()-0.08, 0.1), "This project appears to be already opened in another instance of the Editor.\nOpening the same project in multiple instances of the Editor may corrupt its data.\nWould you like to open it anyway?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
      T  +=ok    .create(Rect_C(clientWidth()*1/3, -0.25, 0.29, 0.07), "OK"    ).func(OK  ,               T ).focusable(false);
      T  +=cancel.create(Rect_C(clientWidth()*2/3, -0.25, 0.29, 0.07), "Cancel").func(Hide, SCAST(GuiObj, T)).focusable(false);
   }
}
ProjectLocked ProjectLock;
/******************************************************************************/
class ElmProperties : ClosableWindow
{
   TextBlack ts, ts_small;
   Text    t_id, t_file, t_size, t_class, t_src;
   Text      id,   file,   size,   Class,   src;
   Button  b_id, b_file,                  b_src;
   UID     elm_id=UIDZero;

   static void CopyID  (ElmProperties &ep) {ClipSet(ep.id  ());}
   static void CopyFile(ElmProperties &ep) {ClipSet(ep.file());}
   static void Explore (ElmProperties &ep) {if(Elm *elm=Proj.findElm(ep.elm_id))Proj.explore(*elm);}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1.3, 0.47)).hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      ts.reset().size=0.046; ts.align.set(1, 0); ts_small=ts; ts_small.size=0.04; ts_small.align.set(1, -1);
      T+=t_id   .create(Vec2(0.02, -0.05), "ID:"       , &ts); T+=id   .create(Vec2(0.23, -0.03), S, &ts_small); T+=b_id  .create(Rect_R(clientWidth()-0.02, -0.05, 0.15, 0.05), "Copy").func(CopyID  , T).desc(S+"Element ID can be accessed in additional ways:\n- Drag and drop the element to the codes to insert its ID at that location\n- "+Kb.ctrlCmdName()+"+RightClick the element to copy its ID to current location in codes\n- "+Kb.ctrlCmdName()+"+MiddleClick the element to copy its ID to system clipboard");
      T+=t_file .create(Vec2(0.02, -0.12), "File ID:"  , &ts); T+=file .create(Vec2(0.23, -0.10), S, &ts_small); T+=b_file.create(Rect_R(clientWidth()-0.02, -0.12, 0.15, 0.05), "Copy").func(CopyFile, T);
      T+=t_size .create(Vec2(0.02, -0.19), "Size:"     , &ts); T+=size .create(Vec2(0.23, -0.17), S, &ts_small);
      T+=t_class.create(Vec2(0.02, -0.26), "C++ Class:", &ts); T+=Class.create(Vec2(0.23, -0.24), S, &ts_small);
      T+=t_src  .create(Vec2(0.02, -0.33), "Source:"   , &ts); T+=b_src.create(Rect_R(clientWidth()-0.02, -0.33, 0.15, 0.05), "Explore").func(Explore, T).desc("Open folder containing the source file"); T+=src.create(Rect(0.23, -0.38, b_src.rect().min.x-0.02, -0.31), S, &ts_small); src.auto_line=AUTO_LINE_SPACE_SPLIT;
   }
   void activate(Elm &elm)
   {
      if(elm.type==ELM_APP)AppPropsEdit.set(&elm);else
      if(!ElmPublish(elm.type))Gui.msgBox(S, "This type of element doesn't have any properties.");else
      {
         setTitle(S+"Element Properties - "+Proj.elmFullName(&elm)).activate();
         elm_id=elm.id;
         id   .set(elm.id.asCString());
         file .set(EncodeFileName(elm.id));
         size .clear(); if(FExist(Proj.gamePath(elm.id)))size.set(FileSize(FSize(Proj.gamePath(elm.id))));
         Class.set(ElmTypeClass[elm.type]);
         Mems<Edit.FileParams> files=Edit.FileParams.Decode(elm.srcFile());
         Str src_text; FREPA(files)src_text.line()+=files[i].name; // ignore parameters
         src  .set(src_text);
         b_src.visible(src().is());
      }
   }
}
ElmProperties ElmProps;
/******************************************************************************/
void Highlight(C Rect &rect, flt alpha, C Color &color=Color(255, 64, 64, 0), flt e=0.08) // Color(100, 240, 255, 0)
{
   if(GlowBorder && alpha>0)
   {
      MIN(e, rect.size().min());
      Rect r=rect; r&=Rect(-D.w(), -D.h(), D.w(), D.h()); r.extend(-e/2);
      GlowBorder->drawBorder(Color(0, 0, 0, Mid(Round(alpha*255), 0, 255)), Color(color.r, color.g, color.b, 0), r, -e);
   }
}
/******************************************************************************/
Color MaterialColor(C MaterialPtr &mtrl)
{
   return mtrl ? mtrl->color.xyz : WHITE;
}
ImagePtr MaterialImage(C MaterialPtr &mtrl)
{
   return mtrl ? (mtrl->base_0 && mtrl->base_0->is()) ? mtrl->base_0 : &WhiteImage : ImagePtr();
}
Color MaterialColor(C WaterMtrlPtr &mtrl)
{
   return mtrl ? mtrl->color : WHITE;
}
ImagePtr MaterialImage(C WaterMtrlPtr &mtrl)
{
   return mtrl ? (mtrl->colorMap() && mtrl->colorMap()->is()) ? mtrl->colorMap() : &WhiteImage : ImagePtr();
}
/******************************************************************************/
void MakeRed(Color &col)
{
   col.r=AvgI(col.r, 255);
   col.g=AvgI(col.g,   0);
   col.b=AvgI(col.b,   0);
}
void MakeLit(Color &col)
{
   col.r=AvgI(col.r,   0);
   col.g=AvgI(col.g, 128);
   col.b=AvgI(col.b, 255);
}
void MakeTransparent(Color &col)
{
   col.a/=2;
}
void MakeTransparent(GuiSkin.Button &button)
{
   MakeTransparent(button.pushed_color);
   MakeTransparent(button.normal_color);
   MakeTransparent(button.disabled_color);
   MakeTransparent(button.pushed_disabled_color);
}
void MakeTransparent(GuiSkin.ButtonImage &button)
{
   MakeTransparent(SCAST(GuiSkin.Button, button));
   MakeTransparent(button.image_color);
}
void SetGuiSkin(UID id=Gui.default_skin)
{
   if(!id.valid())id=Gui.default_skin;
   CACHE_MODE mode=GuiSkins.mode(CACHE_NULL);
   if(!Gui.skin.get(id))Gui.skin=Gui.default_skin;
   GuiSkins.mode(mode);

   REPAO(TextBlack   .texts    ).skinChanged();
   REPAO(TextWhite   .texts    ).skinChanged();
   REPAO(ImageSkin   .images   ).skinChanged();
   REPAO(ViewportSkin.viewports).skinChanged();
   Proj          .refresh();
   Projs         .refresh();
   CodeEdit      .skinChanged();
   EnumEdit      .skinChanged();
   ImageAtlasEdit.skinChanged();
   ObjEdit       .skinChanged();
   WorldEdit     .skinChanged();

   Paks.remove("Gui/white.pak"); // always remove in case 'add' adds multiple times
   Paks.remove("Gui/neon.pak" );
   Color color=Gui.backgroundColor();
   if(color.b>color.r*1.2)Paks.addTry("Gui/neon.pak" );else
   if(color.lum()<64     )Paks.addTry("Gui/white.pak");
   Pak pak; if(pak.load("Gui/white.pak"))REPA(pak)if(ImagePtr image=ImagePtr().find(pak.fullName(i)))image->load(pak.fullName(i)); // reload images listed in the pak

   if(Gui.skin)
   {
      DarkSkin=TransparentSkin=*Gui.skin;
      if(Gui.backgroundColor().lum()>64) // light
      {
          DarkSkin.region.normal_color=ColorBrightness(DarkSkin.region.normal_color, 0.5);
      }
   }
   TransparentSkin.region.normal_color=ColorAlpha(TransparentSkin.region.normal_color, 0.35);

   if(Gui.skin)
   {
      EmptyGuiSkin.font=Gui.skin->font;
      EmptyGuiSkin.text_style=Gui.skin->text_style;
      EmptyGuiSkin.button.text_size=Gui.skin->button.text_size;
      EmptyGuiSkin.button.text_style=Gui.skin->text_style;
      EmptyGuiSkin.list.cursor_color=EmptyGuiSkin.list.highlight_color=EmptyGuiSkin.list.selection_color=TRANSPARENT;
   }
   {
      if(Gui.skin)NoComboBoxImage=*Gui.skin;
      NoComboBoxImage.combobox.image=null;
   }
   {
      if(Gui.skin)NoShadowSkin=*Gui.skin;
      if(NoShadowSkin.window.normal_no_bar){      WindowNoShadow=*NoShadowSkin.window.normal_no_bar;       WindowNoShadow.shadow_opacity=0;       WindowNoShadow.border_color=Gui.borderColor(); NoShadowSkin.window.normal_no_bar=&      WindowNoShadow;}
      if(NoShadowSkin.window.active_no_bar){ActiveWindowNoShadow=*NoShadowSkin.window.active_no_bar; ActiveWindowNoShadow.shadow_opacity=0; ActiveWindowNoShadow.border_color=Gui.borderColor(); NoShadowSkin.window.active_no_bar=&ActiveWindowNoShadow;}
   }
   {
      LightSkin=NoShadowSkin;
      LightSkin.region.normal=LightSkin.window.active_no_bar;
      LightSkin.region.normal_color=ColorBrightness(LightSkin.window.active_color, 0.92);
      LightSkin.region.normal_color.a=255;
   }
   {
      if(Gui.skin)HalfTransparentSkin=*Gui.skin;

      MakeTransparent(HalfTransparentSkin.button);

      MakeTransparent(HalfTransparentSkin.slidebar.left);
      MakeTransparent(HalfTransparentSkin.slidebar.right);
      MakeTransparent(HalfTransparentSkin.slidebar.center);
      MakeTransparent(HalfTransparentSkin.slidebar.background_color);

      MakeTransparent(HalfTransparentSkin.checkbox.  normal_color);
      MakeTransparent(HalfTransparentSkin.checkbox.disabled_color);

      MakeTransparent(HalfTransparentSkin.combobox);

      MakeTransparent(HalfTransparentSkin.textline.  normal_panel_color);
      MakeTransparent(HalfTransparentSkin.textline.  normal_text_color );
      MakeTransparent(HalfTransparentSkin.textline.disabled_panel_color);
      MakeTransparent(HalfTransparentSkin.textline.disabled_text_color );
   }
   {
      if(Gui.skin)RedSkin=*Gui.skin;

      MakeRed(RedSkin.button.pushed_color);
      MakeRed(RedSkin.button.normal_color);
      MakeRed(RedSkin.button.disabled_color);
      MakeRed(RedSkin.button.pushed_disabled_color);

      MakeRed(RedSkin.checkbox.  normal_color);
      MakeRed(RedSkin.checkbox.disabled_color);

      MakeRed(RedSkin.combobox.pushed_color);
      MakeRed(RedSkin.combobox.normal_color);
      MakeRed(RedSkin.combobox.disabled_color);
      MakeRed(RedSkin.combobox.pushed_disabled_color);
      MakeRed(RedSkin.combobox.image_color);

      MakeRed(RedSkin.textline.  normal_panel_color);
      MakeRed(RedSkin.textline.  normal_text_color );
      MakeRed(RedSkin.textline.disabled_panel_color);
      MakeRed(RedSkin.textline.disabled_text_color );
   }
   {
      if(Gui.skin)LitSkin=*Gui.skin;

      MakeLit(LitSkin.button.pushed_color);
      MakeLit(LitSkin.button.normal_color);
      MakeLit(LitSkin.button.disabled_color);
      MakeLit(LitSkin.button.pushed_disabled_color);

      MakeLit(LitSkin.checkbox.  normal_color);
      MakeLit(LitSkin.checkbox.disabled_color);

      MakeLit(LitSkin.combobox.pushed_color);
      MakeLit(LitSkin.combobox.normal_color);
      MakeLit(LitSkin.combobox.disabled_color);
      MakeLit(LitSkin.combobox.pushed_disabled_color);
      MakeLit(LitSkin.combobox.image_color);

      MakeLit(LitSkin.textline.  normal_panel_color);
      MakeLit(LitSkin.textline.  normal_text_color );
      MakeLit(LitSkin.textline.disabled_panel_color);
      MakeLit(LitSkin.textline.disabled_text_color );
   }
}
void InitGui()
{
      WhiteImage.create(1, 1, 1, IMAGE_L8, IMAGE_2D, 1);
   if(WhiteImage.lock(LOCK_WRITE))
   {
      WhiteImage.color(0, 0, WHITE);
      WhiteImage.unlock();
   }
   GlowBorder="Gui/Misc/glow border.img";

   UID skin_id(3070792110, 1173521724, 2244074370, 3689187718); // this is "Dark Grey"
   if(C TextNode *video=Settings.findNode("Video"))if(C TextNode *p=video.findNode("GuiSkin"))skin_id=VidOpt.skinID(p.asText());
   SetGuiSkin(skin_id);
}
void ShutGui()
{
}
/******************************************************************************/
enum OP_HM_MODE
{
   OP_HM_NONE,
   OP_HM_ADD_REM,
   OP_HM_HEIGHT,
   OP_HM_COLOR,
   OP_HM_MTRL,
   OP_HM_MTRL_SCALE,
   OP_HM_MTRL_HOLE,
   OP_HM_MTRL_MAX_1,
   OP_HM_MTRL_MAX_2,
}
enum OP_OBJ_MODE
{
   OP_OBJ_NONE     ,
   OP_OBJ_SET_MTRL ,
   OP_OBJ_SET_GROUP,
}
OP_HM_MODE  OpHm =OP_HM_NONE;
OP_OBJ_MODE OpObj=OP_OBJ_NONE;

void SetHmOp(OP_HM_MODE op)
{
   if(OpHm!=op)
   {
      OpHm=op;
      WorldEdit.hm_add_rem.set(op==OP_HM_ADD_REM, QUIET);
      if(op!=OP_HM_HEIGHT)HeightBrush.mode.set(-1, QUIET);
      if(op!=OP_HM_COLOR ) ColorBrush.     set(-1, QUIET);
      MtrlBrush.scale.set(op==OP_HM_MTRL_SCALE, QUIET);
      MtrlBrush.hole .set(op==OP_HM_MTRL_HOLE , QUIET);
      MtrlBrush.max1 .set(op==OP_HM_MTRL_MAX_1, QUIET);
      MtrlBrush.max2 .set(op==OP_HM_MTRL_MAX_2, QUIET);
      if(op!=OP_HM_MTRL)MtrlBrush.set(-1, QUIET);
   }
}
void SetObjOp(OP_OBJ_MODE op)
{
   if(OpObj!=op)
   {
      OpObj=op;
      MtrlEdit.set_mtrl.set(op==OP_OBJ_SET_MTRL, QUIET);
      if(op!=OP_OBJ_SET_GROUP)REPAO(ObjEdit.group.set_groups).set(false, QUIET);
      if(op!=OP_OBJ_NONE &&
         (ObjEdit.mode()==ObjView.SLOTS
       || ObjEdit.mode()==ObjView.RAGDOLL
       || ObjEdit.mode()==ObjView.PHYS
       || ObjEdit.mode()==ObjView.BONES
       || ObjEdit.mode()==ObjView.SKIN
       || ObjEdit.mode()==ObjView.REMOVE))ObjEdit.mode.set(-1);
   }
}

void Drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
{
            Proj.drag(elms, obj, screen_pos);
        MtrlEdit.drag(elms, obj, screen_pos);
   WaterMtrlEdit.drag(elms, obj, screen_pos);
         ObjEdit.drag(elms, obj, screen_pos);
       WorldEdit.drag(elms, obj, screen_pos);
   ImportTerrain.drag(elms, obj, screen_pos);
    ObjClassEdit.drag(elms, obj, screen_pos);
   TextStyleEdit.drag(elms, obj, screen_pos);
       PanelEdit.drag(elms, obj, screen_pos);
        CodeEdit.drag(elms, obj, screen_pos);
    AppPropsEdit.drag(elms, obj, screen_pos);
       ImageEdit.drag(elms, obj, screen_pos);
  ImageAtlasEdit.drag(elms, obj, screen_pos);
        AnimEdit.drag(elms, obj, screen_pos);
         GuiEdit.drag(elms, obj, screen_pos);
        ObjPaint.drag(elms, obj, screen_pos);
             MSM.drag(elms, obj, screen_pos);
     CreateMtrls.drag(elms, obj, screen_pos);
  ConvertToAtlas.drag(elms, obj, screen_pos);
ConvertToDeAtlas.drag(elms, obj, screen_pos);

   if(obj && obj.type()==GO_TEXTLINE && elms.elms())REPA(PropEx.props) // first check if this is a PropEx
   {
      PropEx &prop=PropEx.props[i]; if(&prop.textline==obj)
      {
         // find compatible element
         if(prop.elm_type)REPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(prop.compatible(elm.type))
         {
            prop.set(elm.id.asHex()) // first set using HEX to make sure it detects correct Elm in case of multiple elements with same path
                .set(Proj.elmFullName(elm), QUIET) // then set textline to be as path and not HEX
                .textline.selectAll();
            return;
         }
         Gui.msgBox("Incompatible Element", prop.elm_type ? S+"This property accepts only "+ElmTypeName[prop.elm_type]+" elements." : S+"This property doesn't accepts elements");
         return;
      }
   }
}
/******************************************************************************/
