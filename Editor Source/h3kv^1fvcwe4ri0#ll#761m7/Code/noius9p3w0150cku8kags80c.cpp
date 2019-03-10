/******************************************************************************/
class GuiSkinEditor : Region
{
   class Change : Edit._Undo.Change
   {
      EditGuiSkin data;

      virtual void create(ptr user)override
      {
         data=GuiSkinEdit.edit;
         GuiSkinEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         GuiSkinEdit.edit.undo(data);
         GuiSkinEdit.setChanged();
         GuiSkinEdit.toGui();
         GuiSkinEdit.undoVis();
      }
   }
   class Objs
   {
      class ButtonPushed   : Button   {virtual void update(C GuiPC &gpc)override {super.update(gpc); set(true , QUIET);}}
      class CheckBoxOff    : CheckBox {virtual void update(C GuiPC &gpc)override {super.update(gpc); set(false, QUIET);}}
      class CheckBoxOn     : CheckBox {virtual void update(C GuiPC &gpc)override {super.update(gpc); set(true , QUIET);}}
      class CheckBoxMulti  : CheckBox {virtual void update(C GuiPC &gpc)override {super.update(gpc); setMulti();}}
      class ComboBoxPushed : ComboBox {virtual void draw  (C GuiPC &gpc)override
      {
         GuiSkinPtr old=skin();
         GuiSkin temp; if(C GuiSkin *skin=getSkin())temp=*skin; skin(&temp, false);
                                          temp.combobox.normal_color  =temp.combobox.pushed_color;
         if(temp.combobox.pushed         )temp.combobox.normal        =temp.combobox.pushed;
                                          temp.combobox.disabled_color=temp.combobox.pushed_disabled_color;
         if(temp.combobox.pushed_disabled)temp.combobox.disabled      =temp.combobox.pushed_disabled;
         super.draw(gpc);
         skin(old, false);
      }}
      class WindowActive : Window {virtual void draw(C GuiPC &gpc)override
      {
         GuiSkinPtr old=skin();
         GuiSkin temp; if(C GuiSkin *skin=getSkin())temp=*skin;
                                          temp.window.normal_color     =temp.window.active_color;
         if(temp.window.active           )temp.window.normal           =temp.window.active;
         if(temp.window.active_no_bar    )temp.window.normal_no_bar    =temp.window.active_no_bar;
         if(temp.window.active_text_style)temp.window.normal_text_style=temp.window.active_text_style;
         skin(&temp); super.draw(gpc);
         skin( old );
      }}

      Button         button_normal, button_disabled;
      ButtonPushed   button_pushed, button_pushed_disabled;
      CheckBoxOff    checkbox_off, checkbox_off_disabled;
      CheckBoxOn     checkbox_on, checkbox_on_disabled;
      CheckBoxMulti  checkbox_multi, checkbox_multi_disabled;
      ComboBox       combobox_normal, combobox_disabled;
      ComboBoxPushed combobox_pushed, combobox_pushed_disabled;
      MenuBar        menu;
      List<ListElm>  list;
      Progress       progress_zero, progress_half, progress_full;
      Region         region, region_list;
      SlideBar       slidebar, slidebar_vertical;
      Slider         slider  ,   slider_vertical;
      Tabs           tabs;
      Text           text;
      TextLine       textline, textline_disabled;
      Window         window, window_menu_buttons;
      WindowActive   window_active;

      static cchar8 *combobox_normal_t[]=
      {
         "Normal 0",
         "Normal 1",
         "Normal 2",
      };
      static cchar8 *combobox_pushed_t[]=
      {
         "Pushed 0",
         "Pushed 1",
         "Pushed 2",
      };
      static cchar8 *tabs_t[]=
      {
         "Tab 0",
         "Tab 1",
         "Tab 2",
      };
      class ListElm
      {
         cchar8 *t;
      }
      static ListElm list_t[]=
      {
         "List Element 0",
         "List Element 1",
         "List Element 2",
         "List Element 3",
         "List Element 4",
         "List Element 5",
         "List Element 6",
         "List Element 7",
         "List Element 8",
         "List Element 9",
         "List Element 10",
         "List Element 11",
         "List Element 12",
      };
      
      static void Reset(GuiObj &go) {Rect r=go.rect(); go.rect(Rect(0, 0)); go.rect(r);} // force reset gui object rectangle
      void setRect()
      {
         Reset(region);
         Reset(region_list);
         Reset(slidebar);
         Reset(slidebar_vertical);
         Reset(window);
         Reset(window_active);
         Reset(window_menu_buttons);
      }
      void setSkin(C GuiSkinPtr &skin)
      {
         button_normal.skin=button_disabled.skin=button_pushed.skin=button_pushed_disabled.skin=skin;
         checkbox_off.skin=checkbox_off_disabled.skin=checkbox_on.skin=checkbox_on_disabled.skin=checkbox_multi.skin=checkbox_multi_disabled.skin=skin;
         combobox_normal.skin(skin); combobox_disabled.skin(skin); combobox_pushed.skin(skin); combobox_pushed_disabled.skin(skin);
         menu.skin(skin);
         list.skin(skin);
         progress_zero.skin=progress_half.skin=progress_full.skin=skin;
         region.skin(skin); region_list.skin(skin);
         slidebar.skin(skin); slidebar_vertical.skin(skin);
         slider.skin=slider_vertical.skin=skin;
         tabs.skin(skin);
         text.skin=skin;
         textline.skin(skin); textline_disabled.skin(skin);
         window.skin(skin); window_active.skin(skin); window_menu_buttons.skin(skin);
      }
      void create(GuiObj &parent, C Vec2 &position)
      {
         flt  h=0.06, w=h*4, l=w*1.7, W=w*2, s=h*0.7, prog_h=0.05;
         Vec2 pos=position, p;

         p  =pos  ; parent+=button_normal.create(Rect_LU(p, w, h), "Normal"); p.x+=w+s; parent+=button_disabled       .create(Rect_LU(p, l, h),        "Disabled").disabled(true); p.y-=h+s;
         p.x=pos.x; parent+=button_pushed.create(Rect_LU(p, w, h), "Pushed"); p.x+=w+s; parent+=button_pushed_disabled.create(Rect_LU(p, l, h), "Pushed Disabled").disabled(true); p.y-=h+s;
         button_pushed.mode=button_pushed_disabled.mode=BUTTON_TOGGLE; button_pushed.set(true); button_pushed_disabled.set(true);

         pos.y-=(h+s)*2+s;
         p  =pos  ; parent+=combobox_normal.create(Rect_LU(p, w, h), combobox_normal_t, Elms(combobox_normal_t)).set(0); p.x+=w+s; parent+=combobox_disabled       .create(Rect_LU(p, l, h), combobox_normal_t, Elms(combobox_normal_t)).setText(       "Disabled", true, QUIET).disabled(true); p.y-=h+s;
         p.x=pos.x; parent+=combobox_pushed.create(Rect_LU(p, w, h), combobox_pushed_t, Elms(combobox_pushed_t)).set(0); p.x+=w+s; parent+=combobox_pushed_disabled.create(Rect_LU(p, l, h), combobox_pushed_t, Elms(combobox_pushed_t)).setText("Pushed Disabled", true, QUIET).disabled(true); p.y-=h+s;

         pos.y-=(h+s)*2;
         p  =pos  ; parent+=checkbox_off.create(Rect_LU(p, h), false).desc("CheckBox Off"); p.x+=h+s; parent+=checkbox_on.create(Rect_LU(p, h, h), true).desc("CheckBox On"); p.x+=h+s; parent+=checkbox_multi.create(Rect_LU(p, h, h)).setMulti().desc("CheckBox Multi"); p.y-=h+s;
         p.x=pos.x; parent+=checkbox_off_disabled.create(Rect_LU(p, h), false).desc("CheckBox Off Disabled").disabled(true); p.x+=h+s; parent+=checkbox_on_disabled.create(Rect_LU(p, h, h), true).desc("CheckBox On Disabled").disabled(true); p.x+=h+s; parent+=checkbox_multi_disabled.create(Rect_LU(p, h, h)).setMulti().desc("CheckBox Multi Disabled").disabled(true); p.y-=h+s;

         pos.y-=(h+s)*2;
         p=pos;
         parent+=progress_zero.create(Rect_LU(p, W, prog_h)).set(0  ).desc("Zero Progress"); p.y-=prog_h+s;
         parent+=progress_half.create(Rect_LU(p, W, prog_h)).set(0.5).desc("Half Progress"); p.y-=prog_h+s;
         parent+=progress_full.create(Rect_LU(p, W, prog_h)).set(1.0).desc("Full Progress"); p.y-=prog_h+s;
         parent+=slidebar.create(Rect_LU(p, W, h)).desc("SlideBar"); p.y-=h+s;
         parent+=slider  .create(Rect_LU(p, W, h)).desc("Slider"  ); p.y-=h+s;
         p=pos; p.x+=W+s;
         parent+=slidebar_vertical.create(Rect_LU(p, h, W)).desc("SlideBar"); p.x+=h+s;
         parent+=  slider_vertical.create(Rect_LU(p, h, W)).desc("Slider"  ); p.x+=h+s;

         pos.y-=(prog_h+s)*3+(h+s)*2;
         p=pos;
         parent+=textline.create(Rect_LU(p, W, h), "TextLine"); p.y-=h+s;
         parent+=textline_disabled.create(Rect_LU(p, W, h), "TextLine Disabled").disabled(true); p.y-=h+s;

         pos.y-=(h+s)*2;
         p=pos;
         parent+=region.create(Rect_LU(p, l, l));
         p.x+=l+s;
         parent+=region_list.create(Rect_LU(p, l, l));

         ListColumn lc[]=
         {
            ListColumn(MEMBER(ListElm, t), l*1.5, "Name"),
         };
         region_list+=list.create(lc, Elms(lc)).setData(list_t, Elms(list_t));

         p=position; p.x+=w+s+l+s*2; flt w2=w*Elms(tabs_t);
         parent+=text.create(Rect_LU(p, w2, h), "Sample Text"); p.y-=h+s;
         parent+=tabs.create(Rect_LU(p, w2, h), 0, tabs_t, Elms(tabs_t)).set(0); p.y-=h+s;

         parent+=window.create(Rect_LU(p, W, l), "Normal").disabled(true); window.flag=0; p.y-=l+s;
         parent+=window_active.create(Rect_LU(p, W, l), "Active"); window_active.flag=0; p.y-=l+s;
         parent+=window_menu_buttons.create(Rect_LU(p, W, l), "Menu"); window_menu_buttons.flag=0; REPAO(window_menu_buttons.button).func(null).show(); p.y-=l+s;

         Node<MenuElm> n;
         {
            Node<MenuElm> &file=(n+="File"); file+="New"; file+="Open"; file+="Save";
            Node<MenuElm> &view=(n+="View"); view.New().create("Option 0").flag(MENU_TOGGLABLE); view.New().create("Option 1").flag(MENU_TOGGLABLE); view.New().create("Option 2").flag(MENU_TOGGLABLE);
         }
         window_menu_buttons+=menu.create(n);
      }
   }
   class SkinRegion : Region
   {
      virtual void update(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            GuiSkinPtr temp=Gui.skin; Gui.skin=GuiSkinEdit.game;
            super.update(gpc);
            Gui.skin=temp;
         }
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            GuiSkinPtr temp=Gui.skin; Gui.skin=GuiSkinEdit.game;
            super.draw(gpc);
            Gui.skin=temp;
         }
      }
   }

   UID               elm_id=UIDZero;
   Elm              *elm=null;
   bool              changed=false;
   EditGuiSkin       edit;
       GuiSkinPtr    game;
   Button            undo, redo, locate;
   Region            props_region;
   SkinRegion         objs_region;
   Memx<PropEx>      props;
   flt               props_x;
   TextBlack         props_ts;
   Objs              objs;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   static void PreChanged(C Property &prop) {GuiSkinEdit.undos.set(&prop);}
   static void    Changed(C Property &prop) {GuiSkinEdit.setChanged();}

   static Str  DefaultFont     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.font_id);}
   static void DefaultFont     (  EditGuiSkin &e, C Str &t) {e.base.font_id=Proj.findElmID(t, ELM_FONT); e.base.font.getUTC();}
   static Str  DefaultTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.text_style_id);}
   static void DefaultTextStyle(  EditGuiSkin &e, C Str &t) {e.base.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.text_style.getUTC();}

   static void        BackgroundColor(EditGuiSkin &e, C Str &t) {e.        background_color=TextVec4(t); e.base.        background_color.getUTC();}
   static void            BorderColor(EditGuiSkin &e, C Str &t) {e.            border_color=TextVec4(t); e.base.            border_color.getUTC();}
   static void    MouseHighlightColor(EditGuiSkin &e, C Str &t) {e.   mouse_highlight_color=TextVec4(t); e.base.   mouse_highlight_color.getUTC();}
   static void KeyboardHighlightColor(EditGuiSkin &e, C Str &t) {e.keyboard_highlight_color=TextVec4(t); e.base.keyboard_highlight_color.getUTC();}

   static Str  ButtonNormal             (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.normal_id);}
   static void ButtonNormal             (  EditGuiSkin &e, C Str &t) {e.base.button.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.normal.getUTC();}
   static void ButtonNormalColor        (  EditGuiSkin &e, C Str &t) {e.button.normal_color=TextVec4(t); e.base.button.normal_color.getUTC();}
   static Str  ButtonPushed             (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.pushed_id);}
   static void ButtonPushed             (  EditGuiSkin &e, C Str &t) {e.base.button.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.pushed.getUTC();}
   static void ButtonPushedColor        (  EditGuiSkin &e, C Str &t) {e.button.pushed_color=TextVec4(t); e.base.button.pushed_color.getUTC();}
   static Str  ButtonDisabled           (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.disabled_id);}
   static void ButtonDisabled           (  EditGuiSkin &e, C Str &t) {e.base.button.disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.disabled.getUTC();}
   static void ButtonDisabledColor      (  EditGuiSkin &e, C Str &t) {e.button.disabled_color=TextVec4(t); e.base.button.disabled_color.getUTC();}
   static Str  ButtonPushedDisabled     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.pushed_disabled_id);}
   static void ButtonPushedDisabled     (  EditGuiSkin &e, C Str &t) {e.base.button.pushed_disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.pushed_disabled.getUTC();}
   static void ButtonPushedDisabledColor(  EditGuiSkin &e, C Str &t) {e.button.pushed_disabled_color=TextVec4(t); e.base.button.pushed_disabled_color.getUTC();}
   static void ButtonTextSize           (  EditGuiSkin &e, C Str &t) {e.button.text_size=TextFlt(t); e.base.button.text_size.getUTC();}
   static void ButtonTextPadd           (  EditGuiSkin &e, C Str &t) {e.button.text_padd=TextFlt(t); e.base.button.text_padd.getUTC();}
   static Str  ButtonTextStyle          (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.text_style_id);}
   static void ButtonTextStyle          (  EditGuiSkin &e, C Str &t) {e.base.button.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.button.text_style.getUTC();}

   static Str  CheckBoxOff          (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.checkbox.off_id);}
   static void CheckBoxOff          (  EditGuiSkin &e, C Str &t) {e.base.checkbox.off_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.checkbox.off.getUTC();}
   static Str  CheckBoxOn           (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.checkbox.on_id);}
   static void CheckBoxOn           (  EditGuiSkin &e, C Str &t) {e.base.checkbox.on_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.checkbox.on.getUTC();}
   static Str  CheckBoxMulti        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.checkbox.multi_id);}
   static void CheckBoxMulti        (  EditGuiSkin &e, C Str &t) {e.base.checkbox.multi_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.checkbox.multi.getUTC();}
   static void CheckBoxNormalColor  (  EditGuiSkin &e, C Str &t) {e.checkbox.normal_color=TextVec4(t); e.base.checkbox.normal_color.getUTC();}
   static void CheckBoxDisabledColor(  EditGuiSkin &e, C Str &t) {e.checkbox.disabled_color=TextVec4(t); e.base.checkbox.disabled_color.getUTC();}
 //static void CheckBoxRectColor    (  EditGuiSkin &e, C Str &t) {e.checkbox.rect_color=TextVec4(t); e.base.checkbox.rect_color.getUTC();}

   static Str  ComboBoxSide               (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.image_id);}
   static void ComboBoxSide               (  EditGuiSkin &e, C Str &t) {e.base.combobox.image_id=Proj.findElmImageID(t); e.base.combobox.image.getUTC();}
   static void ComboBoxSideColor          (  EditGuiSkin &e, C Str &t) {e.combobox.image_color=TextVec4(t); e.base.combobox.image_color.getUTC();}
 //static void ComboBoxSideColorAdd       (  EditGuiSkin &e, C Str &t) {e.combobox.image_color_add=TextVec4(t); e.base.combobox.image_color_add.getUTC();}
   static Str  ComboBoxNormal             (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.normal_id);}
   static void ComboBoxNormal             (  EditGuiSkin &e, C Str &t) {e.base.combobox.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.normal.getUTC();}
   static void ComboBoxNormalColor        (  EditGuiSkin &e, C Str &t) {e.combobox.normal_color=TextVec4(t); e.base.combobox.normal_color.getUTC();}
   static Str  ComboBoxPushed             (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.pushed_id);}
   static void ComboBoxPushed             (  EditGuiSkin &e, C Str &t) {e.base.combobox.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.pushed.getUTC();}
   static void ComboBoxPushedColor        (  EditGuiSkin &e, C Str &t) {e.combobox.pushed_color=TextVec4(t); e.base.combobox.pushed_color.getUTC();}
   static Str  ComboBoxDisabled           (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.disabled_id);}
   static void ComboBoxDisabled           (  EditGuiSkin &e, C Str &t) {e.base.combobox.disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.disabled.getUTC();}
   static void ComboBoxDisabledColor      (  EditGuiSkin &e, C Str &t) {e.combobox.disabled_color=TextVec4(t); e.base.combobox.disabled_color.getUTC();}
   static Str  ComboBoxPushedDisabled     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.pushed_disabled_id);}
   static void ComboBoxPushedDisabled     (  EditGuiSkin &e, C Str &t) {e.base.combobox.pushed_disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.pushed_disabled.getUTC();}
   static void ComboBoxPushedDisabledColor(  EditGuiSkin &e, C Str &t) {e.combobox.pushed_disabled_color=TextVec4(t); e.base.combobox.pushed_disabled_color.getUTC();}
   static void ComboBoxTextSize           (  EditGuiSkin &e, C Str &t) {e.combobox.text_size=TextFlt(t); e.base.combobox.text_size.getUTC();}
   static void ComboBoxTextPadd           (  EditGuiSkin &e, C Str &t) {e.combobox.text_padd=TextFlt(t); e.base.combobox.text_padd.getUTC();}
   static Str  ComboBoxTextStyle          (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.text_style_id);}
   static void ComboBoxTextStyle          (  EditGuiSkin &e, C Str &t) {e.base.combobox.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.combobox.text_style.getUTC();}

   static Str  DescNormal     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.desc.normal_id);}
   static void DescNormal     (  EditGuiSkin &e, C Str &t) {e.base.desc.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.desc.normal.getUTC();}
   static void DescNormalColor(  EditGuiSkin &e, C Str &t) {e.desc.normal_color=TextVec4(t); e.base.desc.normal_color.getUTC();}
   static Str  DescTextStyle  (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.desc.text_style_id);}
   static void DescTextStyle  (  EditGuiSkin &e, C Str &t) {e.base.desc.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.desc.text_style.getUTC();}
   static void DescPadding    (  EditGuiSkin &e, C Str &t) {e.desc.padding=TextFlt(t); e.base.desc.padding.getUTC();}

   static Str  IMMNormal     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.imm.normal_id);}
   static void IMMNormal     (  EditGuiSkin &e, C Str &t) {e.base.imm.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.imm.normal.getUTC();}
   static void IMMNormalColor(  EditGuiSkin &e, C Str &t) {e.imm.normal_color=TextVec4(t); e.base.imm.normal_color.getUTC();}
   static Str  IMMTextStyle  (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.imm.text_style_id);}
   static void IMMTextStyle  (  EditGuiSkin &e, C Str &t) {e.base.imm.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.imm.text_style.getUTC();}
   static void IMMPadding    (  EditGuiSkin &e, C Str &t) {e.imm.padding=TextFlt(t); e.base.imm.padding.getUTC();}

   static Str  ListColumnNormal      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.column.normal_id);}
   static void ListColumnNormal      (  EditGuiSkin &e, C Str &t) {e.base.list.column.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.column.normal.getUTC();}
   static void ListColumnNormalColor (  EditGuiSkin &e, C Str &t) {e.list.column.normal_color=TextVec4(t); e.base.list.column.normal_color.getUTC();}
   static Str  ListColumnPushed      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.column.pushed_id);}
   static void ListColumnPushed      (  EditGuiSkin &e, C Str &t) {e.base.list.column.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.column.pushed.getUTC();}
   static void ListColumnPushedColor (  EditGuiSkin &e, C Str &t) {e.list.column.pushed_color=TextVec4(t); e.base.list.column.pushed_color.getUTC();}
   static void ListColumnTextSize    (  EditGuiSkin &e, C Str &t) {e.list.column.text_size=TextFlt(t); e.base.list.column.text_size.getUTC();}
   static void ListColumnTextPadd    (  EditGuiSkin &e, C Str &t) {e.list.column.text_padd=TextFlt(t); e.base.list.column.text_padd.getUTC();}
   static Str  ListColumnTextStyle   (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.column.text_style_id);}
   static void ListColumnTextStyle   (  EditGuiSkin &e, C Str &t) {e.base.list.column.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.list.column.text_style.getUTC();}
   static Str  ListCursor            (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.cursor_id);}
   static void ListCursor            (  EditGuiSkin &e, C Str &t) {e.base.list.cursor_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.cursor.getUTC();}
   static void ListCursorColor       (  EditGuiSkin &e, C Str &t) {e.list.cursor_color=TextVec4(t); e.base.list.cursor_color.getUTC();}
   static Str  ListHighlight         (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.highlight_id);}
   static void ListHighlight         (  EditGuiSkin &e, C Str &t) {e.base.list.highlight_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.highlight.getUTC();}
   static void ListHighlightColor    (  EditGuiSkin &e, C Str &t) {e.list.highlight_color=TextVec4(t); e.base.list.highlight_color.getUTC();}
   static Str  ListSelection         (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.selection_id);}
   static void ListSelection         (  EditGuiSkin &e, C Str &t) {e.base.list.selection_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.selection.getUTC();}
   static void ListSelectionColor    (  EditGuiSkin &e, C Str &t) {e.list.selection_color=TextVec4(t); e.base.list.selection_color.getUTC();}
   static Str  ListTextStyle         (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.text_style_id);}
   static void ListTextStyle         (  EditGuiSkin &e, C Str &t) {e.base.list.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.list.text_style.getUTC();}
   static Str  ListResizeColumn      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.resize_column_id);}
   static void ListResizeColumn      (  EditGuiSkin &e, C Str &t) {e.base.list.resize_column_id=Proj.findElmImageID(t); e.base.list.resize_column.getUTC();}

   static Str  MenuNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menu.normal_id);}
   static void MenuNormal       (  EditGuiSkin &e, C Str &t) {e.base.menu.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.menu.normal.getUTC();}
   static void MenuNormalColor  (  EditGuiSkin &e, C Str &t) {e.menu.normal_color=TextVec4(t); e.base.menu.normal_color.getUTC();}
   static Str  MenuCheck        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menu.check_id);}
   static void MenuCheck        (  EditGuiSkin &e, C Str &t) {e.base.menu.check_id=Proj.findElmImageID(t); e.base.menu.check.getUTC();}
   static void MenuCheckColor   (  EditGuiSkin &e, C Str &t) {e.menu.check_color=TextVec4(t); e.base.menu.check_color.getUTC();}
   static Str  MenuSubMenu      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menu.sub_menu_id);}
   static void MenuSubMenu      (  EditGuiSkin &e, C Str &t) {e.base.menu.sub_menu_id=Proj.findElmImageID(t); e.base.menu.sub_menu.getUTC();}
   static void MenuSubMenuColor (  EditGuiSkin &e, C Str &t) {e.menu.sub_menu_color=TextVec4(t); e.base.menu.sub_menu_color.getUTC();}
   static void MenuPadding      (  EditGuiSkin &e, C Str &t) {e.menu.padding=TextFlt(t); e.base.menu.padding.getUTC();}
   static void MenuListElmHeight(  EditGuiSkin &e, C Str &t) {e.menu.list_elm_height=TextFlt(t); e.base.menu.list_elm_height.getUTC();}

   static Str  MenuBarBackground     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menubar.background_id);}
   static void MenuBarBackground     (  EditGuiSkin &e, C Str &t) {e.base.menubar.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.menubar.background.getUTC();}
   static void MenuBarBackgroundColor(  EditGuiSkin &e, C Str &t) {e.menubar.background_color=TextVec4(t); e.base.menubar.background_color.getUTC();}
   static Str  MenuBarHighlight      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menubar.highlight_id);}
   static void MenuBarHighlight      (  EditGuiSkin &e, C Str &t) {e.base.menubar.highlight_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.menubar.highlight.getUTC();}
   static void MenuBarHighlightColor (  EditGuiSkin &e, C Str &t) {e.menubar.highlight_color=TextVec4(t); e.base.menubar.highlight_color.getUTC();}
   static void MenuBarBarHeight      (  EditGuiSkin &e, C Str &t) {e.menubar.bar_height=TextFlt(t); e.base.menubar.bar_height.getUTC();}
   static void MenuBarTextSize       (  EditGuiSkin &e, C Str &t) {e.menubar.text_size=TextFlt(t); e.base.menubar.text_size.getUTC();}
   static void MenuBarTextPadd       (  EditGuiSkin &e, C Str &t) {e.menubar.text_padd=TextFlt(t); e.base.menubar.text_padd.getUTC();}
   static Str  MenuBarTextStyle      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menubar.text_style_id);}
   static void MenuBarTextStyle      (  EditGuiSkin &e, C Str &t) {e.base.menubar.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.menubar.text_style.getUTC();}

   static Str  ProgressBackground     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.progress.background_id);}
   static void ProgressBackground     (  EditGuiSkin &e, C Str &t) {e.base.progress.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.progress.background.getUTC();}
   static void ProgressBackgroundColor(  EditGuiSkin &e, C Str &t) {e.progress.background_color=TextVec4(t); e.base.progress.background_color.getUTC();}
   static Str  ProgressProgress       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.progress.progress_id);}
   static void ProgressProgress       (  EditGuiSkin &e, C Str &t) {e.base.progress.progress_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.progress.progress.getUTC();}
   static void ProgressProgressColor  (  EditGuiSkin &e, C Str &t) {e.progress.progress_color=TextVec4(t); e.base.progress.progress_color.getUTC();}
   static void ProgressProgressPart   (  EditGuiSkin &e, C Str &t) {e.progress.draw_progress_partial=TextBool(t); e.base.progress.draw_progress_partial.getUTC();}
   static void ProgressTextSize       (  EditGuiSkin &e, C Str &t) {e.progress.text_size=TextFlt(t); e.base.progress.text_size.getUTC();}
   static Str  ProgressTextStyle      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.progress.text_style_id);}
   static void ProgressTextStyle      (  EditGuiSkin &e, C Str &t) {e.base.progress.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.progress.text_style.getUTC();}

   static Str  PropertyValueImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.property.value.image_id);}
   static void PropertyValueImage        (  EditGuiSkin &e, C Str &t) {e.base.property.value.image_id=Proj.findElmImageID(t); e.base.property.value.image.getUTC();}
   static void PropertyValueImageColor   (  EditGuiSkin &e, C Str &t) {e.property.value.image_color=TextVec4(t); e.base.property.value.image_color.getUTC();}
 //static void PropertyValueImageColorAdd(  EditGuiSkin &e, C Str &t) {e.property.value.image_color_add=TextVec4(t); e.base.property.value.image_color_add.getUTC();}
   static Str  PropertyValueNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.property.value.normal_id);}
   static void PropertyValueNormal       (  EditGuiSkin &e, C Str &t) {e.base.property.value.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.property.value.normal.getUTC();}
   static void PropertyValueNormalColor  (  EditGuiSkin &e, C Str &t) {e.property.value.normal_color=TextVec4(t); e.base.property.value.normal_color.getUTC();}
   static Str  PropertyValuePushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.property.value.pushed_id);}
   static void PropertyValuePushed       (  EditGuiSkin &e, C Str &t) {e.base.property.value.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.property.value.pushed.getUTC();}
   static void PropertyValuePushedColor  (  EditGuiSkin &e, C Str &t) {e.property.value.pushed_color=TextVec4(t); e.base.property.value.pushed_color.getUTC();}

   static Str  RegionNormal           (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.normal_id);}
   static void RegionNormal           (  EditGuiSkin &e, C Str &t) {e.base.region.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.region.normal.getUTC();}
   static void RegionNormalColor      (  EditGuiSkin &e, C Str &t) {e.region.normal_color=TextVec4(t); e.base.region.normal_color.getUTC();}
   static Str  RegionViewImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.view.image_id);}
   static void RegionViewImage        (  EditGuiSkin &e, C Str &t) {e.base.region.view.image_id=Proj.findElmImageID(t); e.base.region.view.image.getUTC();}
   static void RegionViewImageColor   (  EditGuiSkin &e, C Str &t) {e.region.view.image_color=TextVec4(t); e.base.region.view.image_color.getUTC();}
 //static void RegionViewImageColorAdd(  EditGuiSkin &e, C Str &t) {e.region.view.image_color_add=TextVec4(t); e.base.region.view.image_color_add.getUTC();}
   static Str  RegionViewNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.view.normal_id);}
   static void RegionViewNormal       (  EditGuiSkin &e, C Str &t) {e.base.region.view.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.region.view.normal.getUTC();}
   static void RegionViewNormalColor  (  EditGuiSkin &e, C Str &t) {e.region.view.normal_color=TextVec4(t); e.base.region.view.normal_color.getUTC();}
   static Str  RegionViewPushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.view.pushed_id);}
   static void RegionViewPushed       (  EditGuiSkin &e, C Str &t) {e.base.region.view.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.region.view.pushed.getUTC();}
   static void RegionViewPushedColor  (  EditGuiSkin &e, C Str &t) {e.region.view.pushed_color=TextVec4(t); e.base.region.view.pushed_color.getUTC();}

   static Str  SlideBarBackground     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.background_id);}
   static void SlideBarBackground     (  EditGuiSkin &e, C Str &t) {e.base.slidebar.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.background.getUTC();}
   static void SlideBarBackgroundColor(  EditGuiSkin &e, C Str &t) {e.slidebar.background_color=TextVec4(t); e.base.slidebar.background_color.getUTC();}

   static Str  SlideBarLeftImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.left.image_id);}
   static void SlideBarLeftImage        (  EditGuiSkin &e, C Str &t) {e.base.slidebar.left.image_id=Proj.findElmImageID(t); e.base.slidebar.left.image.getUTC();}
   static void SlideBarLeftImageColor   (  EditGuiSkin &e, C Str &t) {e.slidebar.left.image_color=TextVec4(t); e.base.slidebar.left.image_color.getUTC();}
 //static void SlideBarLeftImageColorAdd(  EditGuiSkin &e, C Str &t) {e.slidebar.left.image_color_add=TextVec4(t); e.base.slidebar.left.image_color_add.getUTC();}
   static Str  SlideBarLeftNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.left.normal_id);}
   static void SlideBarLeftNormal       (  EditGuiSkin &e, C Str &t) {e.base.slidebar.left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.left.normal.getUTC();}
   static void SlideBarLeftNormalColor  (  EditGuiSkin &e, C Str &t) {e.slidebar.left.normal_color=TextVec4(t); e.base.slidebar.left.normal_color.getUTC();}
   static Str  SlideBarLeftPushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.left.pushed_id);}
   static void SlideBarLeftPushed       (  EditGuiSkin &e, C Str &t) {e.base.slidebar.left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.left.pushed.getUTC();}
   static void SlideBarLeftPushedColor  (  EditGuiSkin &e, C Str &t) {e.slidebar.left.pushed_color=TextVec4(t); e.base.slidebar.left.pushed_color.getUTC();}

   static Str  SlideBarCenterImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.center.image_id);}
   static void SlideBarCenterImage        (  EditGuiSkin &e, C Str &t) {e.base.slidebar.center.image_id=Proj.findElmImageID(t); e.base.slidebar.center.image.getUTC();}
   static void SlideBarCenterImageColor   (  EditGuiSkin &e, C Str &t) {e.slidebar.center.image_color=TextVec4(t); e.base.slidebar.center.image_color.getUTC();}
 //static void SlideBarCenterImageColorAdd(  EditGuiSkin &e, C Str &t) {e.slidebar.center.image_color_add=TextVec4(t); e.base.slidebar.center.image_color_add.getUTC();}
   static Str  SlideBarCenterNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.center.normal_id);}
   static void SlideBarCenterNormal       (  EditGuiSkin &e, C Str &t) {e.base.slidebar.center.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.center.normal.getUTC();}
   static void SlideBarCenterNormalColor  (  EditGuiSkin &e, C Str &t) {e.slidebar.center.normal_color=TextVec4(t); e.base.slidebar.center.normal_color.getUTC();}
   static Str  SlideBarCenterPushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.center.pushed_id);}
   static void SlideBarCenterPushed       (  EditGuiSkin &e, C Str &t) {e.base.slidebar.center.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.center.pushed.getUTC();}
   static void SlideBarCenterPushedColor  (  EditGuiSkin &e, C Str &t) {e.slidebar.center.pushed_color=TextVec4(t); e.base.slidebar.center.pushed_color.getUTC();}

   static Str  SlideBarRightImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.right.image_id);}
   static void SlideBarRightImage        (  EditGuiSkin &e, C Str &t) {e.base.slidebar.right.image_id=Proj.findElmImageID(t); e.base.slidebar.right.image.getUTC();}
   static void SlideBarRightImageColor   (  EditGuiSkin &e, C Str &t) {e.slidebar.right.image_color=TextVec4(t); e.base.slidebar.right.image_color.getUTC();}
 //static void SlideBarRightImageColorAdd(  EditGuiSkin &e, C Str &t) {e.slidebar.right.image_color_add=TextVec4(t); e.base.slidebar.right.image_color_add.getUTC();}
   static Str  SlideBarRightNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.right.normal_id);}
   static void SlideBarRightNormal       (  EditGuiSkin &e, C Str &t) {e.base.slidebar.right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.right.normal.getUTC();}
   static void SlideBarRightNormalColor  (  EditGuiSkin &e, C Str &t) {e.slidebar.right.normal_color=TextVec4(t); e.base.slidebar.right.normal_color.getUTC();}
   static Str  SlideBarRightPushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.right.pushed_id);}
   static void SlideBarRightPushed       (  EditGuiSkin &e, C Str &t) {e.base.slidebar.right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.right.pushed.getUTC();}
   static void SlideBarRightPushedColor  (  EditGuiSkin &e, C Str &t) {e.slidebar.right.pushed_color=TextVec4(t); e.base.slidebar.right.pushed_color.getUTC();}

   static Str  SliderBackground      (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slider.background_id);}
   static void SliderBackground      (  EditGuiSkin &e, C Str &t) {e.base.slider.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slider.background.getUTC();}
   static void SliderBackgroundColor (  EditGuiSkin &e, C Str &t) {e.slider.background_color=TextVec4(t); e.base.slider.background_color.getUTC();}
   static void SliderBackgroundShrink(  EditGuiSkin &e, C Str &t) {e.slider.background_shrink=TextFlt(t); e.base.slider.background_shrink.getUTC();}
   static Str  SliderProgress        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slider.progress_id);}
   static void SliderProgress        (  EditGuiSkin &e, C Str &t) {e.base.slider.progress_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slider.progress.getUTC();}
   static void SliderProgressColor   (  EditGuiSkin &e, C Str &t) {e.slider.progress_color=TextVec4(t); e.base.slider.progress_color.getUTC();}
   static void SliderProgressPart    (  EditGuiSkin &e, C Str &t) {e.slider.draw_progress_partial=TextBool(t); e.base.slider.draw_progress_partial.getUTC();}
   static Str  SliderSlider          (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slider.slider_id);}
   static void SliderSlider          (  EditGuiSkin &e, C Str &t) {e.base.slider.slider_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slider.slider.getUTC();}
   static void SliderSliderColor     (  EditGuiSkin &e, C Str &t) {e.slider.slider_color=TextVec4(t); e.base.slider.slider_color.getUTC();}

   static void TabNormalColor(  EditGuiSkin &e, C Str &t) {e.tab.left.normal_color=TextVec4(t); e.base.tab.normal_color.getUTC();}
   static void TabPushedColor(  EditGuiSkin &e, C Str &t) {e.tab.left.pushed_color=TextVec4(t); e.base.tab.pushed_color.getUTC();}
   static void TabTextSize   (  EditGuiSkin &e, C Str &t) {e.tab.left.text_size=TextFlt(t); e.base.tab.text_size.getUTC();}
   static void TabTextPadd   (  EditGuiSkin &e, C Str &t) {e.tab.left.text_padd=TextFlt(t); e.base.tab.text_padd.getUTC();}
   static Str  TabTextStyle  (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.text_style_id);}
   static void TabTextStyle  (  EditGuiSkin &e, C Str &t) {e.base.tab.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.tab.text_style.getUTC();}

   static Str  TabLeftNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.left.normal_id);}
   static void TabLeftNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.left.normal.getUTC();}
   static Str  TabLeftPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.left.pushed_id);}
   static void TabLeftPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.left.pushed.getUTC();}

   static Str  TabHorizontalNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.horizontal.normal_id);}
   static void TabHorizontalNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.horizontal.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.horizontal.normal.getUTC();}
   static Str  TabHorizontalPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.horizontal.pushed_id);}
   static void TabHorizontalPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.horizontal.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.horizontal.pushed.getUTC();}

   static Str  TabRightNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.right.normal_id);}
   static void TabRightNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.right.normal.getUTC();}
   static Str  TabRightPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.right.pushed_id);}
   static void TabRightPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.right.pushed.getUTC();}

   static Str  TabTopNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top.normal_id);}
   static void TabTopNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.top.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top.normal.getUTC();}
   static Str  TabTopPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top.pushed_id);}
   static void TabTopPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.top.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top.pushed.getUTC();}

   static Str  TabVerticalNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.vertical.normal_id);}
   static void TabVerticalNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.vertical.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.vertical.normal.getUTC();}
   static Str  TabVerticalPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.vertical.pushed_id);}
   static void TabVerticalPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.vertical.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.vertical.pushed.getUTC();}

   static Str  TabBottomNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom.normal_id);}
   static void TabBottomNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom.normal.getUTC();}
   static Str  TabBottomPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom.pushed_id);}
   static void TabBottomPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom.pushed.getUTC();}

   static Str  TabTopLeftNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_left.normal_id);}
   static void TabTopLeftNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.top_left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_left.normal.getUTC();}
   static Str  TabTopLeftPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_left.pushed_id);}
   static void TabTopLeftPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.top_left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_left.pushed.getUTC();}

   static Str  TabTopRightNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_right.normal_id);}
   static void TabTopRightNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.top_right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_right.normal.getUTC();}
   static Str  TabTopRightPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_right.pushed_id);}
   static void TabTopRightPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.top_right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_right.pushed.getUTC();}

   static Str  TabBottomLeftNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_left.normal_id);}
   static void TabBottomLeftNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_left.normal.getUTC();}
   static Str  TabBottomLeftPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_left.pushed_id);}
   static void TabBottomLeftPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_left.pushed.getUTC();}

   static Str  TabBottomRightNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_right.normal_id);}
   static void TabBottomRightNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_right.normal.getUTC();}
   static Str  TabBottomRightPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_right.pushed_id);}
   static void TabBottomRightPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_right.pushed.getUTC();}

   static Str  TextTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.text.text_style_id);}
   static void TextTextStyle(  EditGuiSkin &e, C Str &t) {e.base.text.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.text.text_style.getUTC();}

   static Str  TextLineNormal            (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.normal_id);}
   static void TextLineNormal            (  EditGuiSkin &e, C Str &t) {e.base.textline.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.textline.normal.getUTC();}
   static void TextLineNormalPanelColor  (  EditGuiSkin &e, C Str &t) {e.textline.normal_panel_color=TextVec4(t); e.base.textline.normal_panel_color.getUTC();}
   static void TextLineNormalTextColor   (  EditGuiSkin &e, C Str &t) {e.textline.normal_text_color =TextVec4(t); e.base.textline.normal_text_color .getUTC();}
   static Str  TextLineDisabled          (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.disabled_id);}
   static void TextLineDisabled          (  EditGuiSkin &e, C Str &t) {e.base.textline.disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.textline.disabled.getUTC();}
   static void TextLineDisabledPanelColor(  EditGuiSkin &e, C Str &t) {e.textline.disabled_panel_color=TextVec4(t); e.base.textline.disabled_panel_color.getUTC();}
   static void TextLineDisabledTextColor (  EditGuiSkin &e, C Str &t) {e.textline.disabled_text_color =TextVec4(t); e.base.textline.disabled_text_color .getUTC();}
   static void TextLineRectColor         (  EditGuiSkin &e, C Str &t) {e.textline.rect_color=TextVec4(t); e.base.textline.rect_color.getUTC();}
   static void TextLineTextSize          (  EditGuiSkin &e, C Str &t) {e.textline.text_size=TextFlt(t); e.base.textline.text_size.getUTC();}
   static Str  TextLineTextStyle         (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.text_style_id);}
   static void TextLineTextStyle         (  EditGuiSkin &e, C Str &t) {e.base.textline.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.textline.text_style.getUTC();}
   static Str  TextLineFindImage         (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.find_image_id);}
   static void TextLineFindImage         (  EditGuiSkin &e, C Str &t) {e.base.textline.find_image_id=Proj.findElmImageID(t); e.base.textline.find_image.getUTC();}
   static Str  TextLineClearImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.clear.image_id);}
   static void TextLineClearImage        (  EditGuiSkin &e, C Str &t) {e.base.textline.clear.image_id=Proj.findElmImageID(t); e.base.textline.clear.image.getUTC();}

   static Str  WindowNormal     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.normal_id);}
   static void WindowNormal     (  EditGuiSkin &e, C Str &t) {e.base.window.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.window.normal.getUTC();}
   static void WindowNormalColor(  EditGuiSkin &e, C Str &t) {e.window.normal_color=TextVec4(t); e.base.window.normal_color.getUTC();}
   static Str  WindowActive     (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.active_id);}
   static void WindowActive     (  EditGuiSkin &e, C Str &t) {e.base.window.active_id=Proj.findElmID(t, ELM_PANEL); e.base.window.active.getUTC();}
   static void WindowActiveColor(  EditGuiSkin &e, C Str &t) {e.window.active_color=TextVec4(t); e.base.window.active_color.getUTC();}
   static Str  WindowNormalNoBar(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.normal_no_bar_id);}
   static void WindowNormalNoBar(  EditGuiSkin &e, C Str &t) {e.base.window.normal_no_bar_id=Proj.findElmID(t, ELM_PANEL); e.base.window.normal_no_bar.getUTC();}
   static Str  WindowActiveNoBar(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.active_no_bar_id);}
   static void WindowActiveNoBar(  EditGuiSkin &e, C Str &t) {e.base.window.active_no_bar_id=Proj.findElmID(t, ELM_PANEL); e.base.window.active_no_bar.getUTC();}

   static void WindowTextSize       (  EditGuiSkin &e, C Str &t) {e.window.text_size=TextFlt(t); e.base.window.text_size.getUTC();}
   static void WindowTextPadd       (  EditGuiSkin &e, C Str &t) {e.window.text_padd=TextFlt(t); e.base.window.text_padd.getUTC();}
   static Str  WindowNormalTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.normal_text_style_id);}
   static void WindowNormalTextStyle(  EditGuiSkin &e, C Str &t) {e.base.window.normal_text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.window.normal_text_style.getUTC();}
   static Str  WindowActiveTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.active_text_style_id);}
   static void WindowActiveTextStyle(  EditGuiSkin &e, C Str &t) {e.base.window.active_text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.window.active_text_style.getUTC();}

   static Str  WindowMinimizeImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.minimize.image_id);}
   static void WindowMinimizeImage        (  EditGuiSkin &e, C Str &t) {e.base.window.minimize.image_id=Proj.findElmImageID(t); e.base.window.minimize.image.getUTC();}
   static void WindowMinimizeImageColor   (  EditGuiSkin &e, C Str &t) {e.window.minimize.image_color=TextVec4(t); e.base.window.minimize.image_color.getUTC();}
 //static void WindowMinimizeImageColorAdd(  EditGuiSkin &e, C Str &t) {e.window.minimize.image_color_add=TextVec4(t); e.base.window.minimize.image_color_add.getUTC();}
   static Str  WindowMinimizeNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.minimize.normal_id);}
   static void WindowMinimizeNormal       (  EditGuiSkin &e, C Str &t) {e.base.window.minimize.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.minimize.normal.getUTC();}
   static void WindowMinimizeNormalColor  (  EditGuiSkin &e, C Str &t) {e.window.minimize.normal_color=TextVec4(t); e.base.window.minimize.normal_color.getUTC();}
   static Str  WindowMinimizePushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.minimize.pushed_id);}
   static void WindowMinimizePushed       (  EditGuiSkin &e, C Str &t) {e.base.window.minimize.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.minimize.pushed.getUTC();}
   static void WindowMinimizePushedColor  (  EditGuiSkin &e, C Str &t) {e.window.minimize.pushed_color=TextVec4(t); e.base.window.minimize.pushed_color.getUTC();}
   static void WindowMinimizeDisabledColor(  EditGuiSkin &e, C Str &t) {e.window.minimize.disabled_color=TextVec4(t); e.base.window.minimize.disabled_color.getUTC();}

   static Str  WindowMaximizeImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.maximize.image_id);}
   static void WindowMaximizeImage        (  EditGuiSkin &e, C Str &t) {e.base.window.maximize.image_id=Proj.findElmImageID(t); e.base.window.maximize.image.getUTC();}
   static void WindowMaximizeImageColor   (  EditGuiSkin &e, C Str &t) {e.window.maximize.image_color=TextVec4(t); e.base.window.maximize.image_color.getUTC();}
 //static void WindowMaximizeImageColorAdd(  EditGuiSkin &e, C Str &t) {e.window.maximize.image_color_add=TextVec4(t); e.base.window.maximize.image_color_add.getUTC();}
   static Str  WindowMaximizeNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.maximize.normal_id);}
   static void WindowMaximizeNormal       (  EditGuiSkin &e, C Str &t) {e.base.window.maximize.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.maximize.normal.getUTC();}
   static void WindowMaximizeNormalColor  (  EditGuiSkin &e, C Str &t) {e.window.maximize.normal_color=TextVec4(t); e.base.window.maximize.normal_color.getUTC();}
   static Str  WindowMaximizePushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.maximize.pushed_id);}
   static void WindowMaximizePushed       (  EditGuiSkin &e, C Str &t) {e.base.window.maximize.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.maximize.pushed.getUTC();}
   static void WindowMaximizePushedColor  (  EditGuiSkin &e, C Str &t) {e.window.maximize.pushed_color=TextVec4(t); e.base.window.maximize.pushed_color.getUTC();}
   static void WindowMaximizeDisabledColor(  EditGuiSkin &e, C Str &t) {e.window.maximize.disabled_color=TextVec4(t); e.base.window.maximize.disabled_color.getUTC();}

   static Str  WindowCloseImage        (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.close.image_id);}
   static void WindowCloseImage        (  EditGuiSkin &e, C Str &t) {e.base.window.close.image_id=Proj.findElmImageID(t); e.base.window.close.image.getUTC();}
   static void WindowCloseImageColor   (  EditGuiSkin &e, C Str &t) {e.window.close.image_color=TextVec4(t); e.base.window.close.image_color.getUTC();}
 //static void WindowCloseImageColorAdd(  EditGuiSkin &e, C Str &t) {e.window.close.image_color_add=TextVec4(t); e.base.window.close.image_color_add.getUTC();}
   static Str  WindowCloseNormal       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.close.normal_id);}
   static void WindowCloseNormal       (  EditGuiSkin &e, C Str &t) {e.base.window.close.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.close.normal.getUTC();}
   static void WindowCloseNormalColor  (  EditGuiSkin &e, C Str &t) {e.window.close.normal_color=TextVec4(t); e.base.window.close.normal_color.getUTC();}
   static Str  WindowClosePushed       (C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.close.pushed_id);}
   static void WindowClosePushed       (  EditGuiSkin &e, C Str &t) {e.base.window.close.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.close.pushed.getUTC();}
   static void WindowClosePushedColor  (  EditGuiSkin &e, C Str &t) {e.window.close.pushed_color=TextVec4(t); e.base.window.close.pushed_color.getUTC();}
   static void WindowCloseDisabledColor(  EditGuiSkin &e, C Str &t) {e.window.close.disabled_color=TextVec4(t); e.base.window.close.disabled_color.getUTC();}

   static void WindowButtonOffset(EditGuiSkin &e, C Str &t) {e.window.button_offset=TextVec2(t); e.base.window.button_offset.getUTC();}

   static void Undo  (GuiSkinEditor &editor) {editor.undos.undo();}
   static void Redo  (GuiSkinEditor &editor) {editor.undos.redo();}
   static void Locate(GuiSkinEditor &editor) {Proj.elmLocate(editor.elm_id);}

   bool selected()C {return Mode()==MODE_GUI_SKIN;}
   void selectedChanged() {}

                  C Rect& rect()C {return super.rect();}
   virtual GuiSkinEditor& rect(C Rect &rect)override
   {
      super.rect(rect);
      props_region.rect(Rect(0.01, -clientHeight()+0.01, 0.01+props_x, undo.rect().min.y-0.01));
       objs_region.rect(Rect(props_region.rect().max.x, props_region.rect().min.y, clientWidth(), undo.rect().min.y-0.01));
      flt s=0.12;
      objs.window.pos(objs.tabs.rect().ld()+Vec2(0.15, -s));
      objs.window_active.pos(objs.window.rect().ld()-Vec2(0, s));
      objs.window_menu_buttons.pos(objs.window_active.rect().ld()-Vec2(0, s));
      return T;
   }

   void resize() {rect(EditRect());}

   GuiSkinEditor& create(GuiObj &parent)
   {
      props.New().create("Background Color"        , MemberDesc(MEMBER(EditGuiSkin,         background_color)).setTextToDataFunc(       BackgroundColor)).setColor();
      props.New().create("Border Color"            , MemberDesc(MEMBER(EditGuiSkin,             border_color)).setTextToDataFunc(           BorderColor)).setColor().desc("Affects borders of 'GuiImage', 'Property' and 'Viewport'");
      props.New().create("Mouse Highlight Color"   , MemberDesc(MEMBER(EditGuiSkin,    mouse_highlight_color)).setTextToDataFunc(   MouseHighlightColor)).setColor();
      props.New().create("Keyboard Highlight Color", MemberDesc(MEMBER(EditGuiSkin, keyboard_highlight_color)).setTextToDataFunc(KeyboardHighlightColor)).setColor();
      props.New();
      props.New().create("Default Font"      , MemberDesc(MEMBER(EditGuiSkin, base.font_id)).setFunc(DefaultFont, DefaultFont)).elmType(ELM_FONT);
      props.New().create("Default Text Style", MemberDesc(MEMBER(EditGuiSkin, base.text_style_id)).setFunc(DefaultTextStyle, DefaultTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New();
      props.New().create("Button:");
      props.New().create("Normal Panel Image"         , MemberDesc(MEMBER(EditGuiSkin, base.button.normal_id)).setFunc(ButtonNormal, ButtonNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Normal Color"               , MemberDesc(MEMBER(EditGuiSkin, button.normal_color)).setTextToDataFunc(ButtonNormalColor)).setColor();
      props.New().create("Pushed Panel Image"         , MemberDesc(MEMBER(EditGuiSkin, base.button.pushed_id)).setFunc(ButtonPushed, ButtonPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Pushed Color"               , MemberDesc(MEMBER(EditGuiSkin, button.pushed_color)).setTextToDataFunc(ButtonPushedColor)).setColor();
      props.New().create("Disabled Panel Image"       , MemberDesc(MEMBER(EditGuiSkin, base.button.disabled_id)).setFunc(ButtonDisabled, ButtonDisabled)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Disabled Color"             , MemberDesc(MEMBER(EditGuiSkin, button.disabled_color)).setTextToDataFunc(ButtonDisabledColor)).setColor();
      props.New().create("Pushed Disabled Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.button.pushed_disabled_id)).setFunc(ButtonPushedDisabled, ButtonPushedDisabled)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Pushed Disabled Color"      , MemberDesc(MEMBER(EditGuiSkin, button.pushed_disabled_color)).setTextToDataFunc(ButtonPushedDisabledColor)).setColor();
      props.New().create("Text Size"                  , MemberDesc(MEMBER(EditGuiSkin, button.text_size)).setTextToDataFunc(ButtonTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Padd"                  , MemberDesc(MEMBER(EditGuiSkin, button.text_padd)).setTextToDataFunc(ButtonTextPadd)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Style"                 , MemberDesc(MEMBER(EditGuiSkin, base.button.text_style_id)).setFunc(ButtonTextStyle, ButtonTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New();
      props.New().create("CheckBox:");
      props.New().create("Off Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.checkbox.off_id)).setFunc(CheckBoxOff, CheckBoxOff)).elmType(ELM_PANEL_IMAGE);
      props.New().create("On Panel Image"   , MemberDesc(MEMBER(EditGuiSkin, base.checkbox.on_id)).setFunc(CheckBoxOn, CheckBoxOn)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Multi Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.checkbox.multi_id)).setFunc(CheckBoxMulti, CheckBoxMulti)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Normal Color"     , MemberDesc(MEMBER(EditGuiSkin, checkbox.normal_color)).setTextToDataFunc(CheckBoxNormalColor)).setColor();
      props.New().create("Disabled Color"   , MemberDesc(MEMBER(EditGuiSkin, checkbox.disabled_color)).setTextToDataFunc(CheckBoxDisabledColor)).setColor();
    //props.New().create("Rect Color"       , MemberDesc(MEMBER(EditGuiSkin, checkbox.rect_color)).setTextToDataFunc(CheckBoxRectColor)).setColor();
      props.New();
      props.New().create("ComboBox:");
      props.New().create("Normal Panel Image"         , MemberDesc(MEMBER(EditGuiSkin, base.combobox.normal_id)).setFunc(ComboBoxNormal, ComboBoxNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Normal Color"               , MemberDesc(MEMBER(EditGuiSkin, combobox.normal_color)).setTextToDataFunc(ComboBoxNormalColor)).setColor();
      props.New().create("Pushed Panel Image"         , MemberDesc(MEMBER(EditGuiSkin, base.combobox.pushed_id)).setFunc(ComboBoxPushed, ComboBoxPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Pushed Color"               , MemberDesc(MEMBER(EditGuiSkin, combobox.pushed_color)).setTextToDataFunc(ComboBoxPushedColor)).setColor();
      props.New().create("Disabled Panel Image"       , MemberDesc(MEMBER(EditGuiSkin, base.combobox.disabled_id)).setFunc(ComboBoxDisabled, ComboBoxDisabled)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Disabled Color"             , MemberDesc(MEMBER(EditGuiSkin, combobox.disabled_color)).setTextToDataFunc(ComboBoxDisabledColor)).setColor();
      props.New().create("Pushed Disabled Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.combobox.pushed_disabled_id)).setFunc(ComboBoxPushedDisabled, ComboBoxPushedDisabled)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Pushed Disabled Color"      , MemberDesc(MEMBER(EditGuiSkin, combobox.pushed_disabled_color)).setTextToDataFunc(ComboBoxPushedDisabledColor)).setColor();
      props.New().create("Text Size"                  , MemberDesc(MEMBER(EditGuiSkin, combobox.text_size)).setTextToDataFunc(ComboBoxTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Padd"                  , MemberDesc(MEMBER(EditGuiSkin, combobox.text_padd)).setTextToDataFunc(ComboBoxTextPadd)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Style"                 , MemberDesc(MEMBER(EditGuiSkin, base.combobox.text_style_id)).setFunc(ComboBoxTextStyle, ComboBoxTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Side Image"                 , MemberDesc(MEMBER(EditGuiSkin, base.combobox.image_id)).setFunc(ComboBoxSide, ComboBoxSide)).elmType(ELM_IMAGE);
      props.New().create("Side Color"                 , MemberDesc(MEMBER(EditGuiSkin, combobox.image_color)).setTextToDataFunc(ComboBoxSideColor)).setColor();
    //props.New().create("Side Color Add"             , MemberDesc(MEMBER(EditGuiSkin, combobox.image_color_add)).setTextToDataFunc(ComboBoxSideColorAdd)).setColor();
      props.New();
      props.New().create("Description:");
      props.New().create("Normal Panel", MemberDesc(MEMBER(EditGuiSkin, base.desc.normal_id)).setFunc(DescNormal, DescNormal)).elmType(ELM_PANEL);
      props.New().create("Normal Color", MemberDesc(MEMBER(EditGuiSkin, desc.normal_color)).setTextToDataFunc(DescNormalColor)).setColor();
      props.New().create("Text Style"  , MemberDesc(MEMBER(EditGuiSkin, base.desc.text_style_id)).setFunc(DescTextStyle, DescTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Padding"     , MemberDesc(MEMBER(EditGuiSkin, desc.padding)).setTextToDataFunc(DescPadding)).range(0, 0.1).mouseEditSpeed(0.02);
      props.New();
      props.New().create("Input Method Manager:");
      props.New().create("Normal Panel", MemberDesc(MEMBER(EditGuiSkin, base.imm.normal_id)).setFunc(IMMNormal, IMMNormal)).elmType(ELM_PANEL);
      props.New().create("Normal Color", MemberDesc(MEMBER(EditGuiSkin, imm.normal_color)).setTextToDataFunc(IMMNormalColor)).setColor();
      props.New().create("Text Style"  , MemberDesc(MEMBER(EditGuiSkin, base.imm.text_style_id)).setFunc(IMMTextStyle, IMMTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Padding"     , MemberDesc(MEMBER(EditGuiSkin, imm.padding)).setTextToDataFunc(IMMPadding)).range(0, 0.1).mouseEditSpeed(0.02);
      props.New();
      props.New().create("List:");
      props.New().create("Column Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.list.column.normal_id)).setFunc(ListColumnNormal, ListColumnNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Column Normal Color"      , MemberDesc(MEMBER(EditGuiSkin, list.column.normal_color)).setTextToDataFunc(ListColumnNormalColor)).setColor();
      props.New().create("Column Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.list.column.pushed_id)).setFunc(ListColumnPushed, ListColumnPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Column Pushed Color"      , MemberDesc(MEMBER(EditGuiSkin, list.column.pushed_color)).setTextToDataFunc(ListColumnPushedColor)).setColor();
      props.New().create("Column Text Size"         , MemberDesc(MEMBER(EditGuiSkin, list.column.text_size)).setTextToDataFunc(ListColumnTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Column Text Padd"         , MemberDesc(MEMBER(EditGuiSkin, list.column.text_padd)).setTextToDataFunc(ListColumnTextPadd)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Column Text Style"        , MemberDesc(MEMBER(EditGuiSkin, base.list.column.text_style_id)).setFunc(ListColumnTextStyle, ListColumnTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Cursor Panel Image"       , MemberDesc(MEMBER(EditGuiSkin, base.list.cursor_id)).setFunc(ListCursor, ListCursor)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Cursor Color"             , MemberDesc(MEMBER(EditGuiSkin, list.cursor_color)).setTextToDataFunc(ListCursorColor)).setColor();
      props.New().create("Highlight Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.list.highlight_id)).setFunc(ListHighlight, ListHighlight)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Highlight Color"          , MemberDesc(MEMBER(EditGuiSkin, list.highlight_color)).setTextToDataFunc(ListHighlightColor)).setColor();
      props.New().create("Selection Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.list.selection_id)).setFunc(ListSelection, ListSelection)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Selection Color"          , MemberDesc(MEMBER(EditGuiSkin, list.selection_color)).setTextToDataFunc(ListSelectionColor)).setColor();
      props.New().create("Text Style"               , MemberDesc(MEMBER(EditGuiSkin, base.list.text_style_id)).setFunc(ListTextStyle, ListTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Resize Column Image"      , MemberDesc(MEMBER(EditGuiSkin, base.list.resize_column_id)).setFunc(ListResizeColumn, ListResizeColumn)).elmType(ELM_IMAGE);
      props.New();
      props.New().create("Menu:");
      props.New().create("Normal Panel"       , MemberDesc(MEMBER(EditGuiSkin, base.menu.normal_id)).setFunc(MenuNormal, MenuNormal)).elmType(ELM_PANEL);
      props.New().create("Normal Color"       , MemberDesc(MEMBER(EditGuiSkin, menu.normal_color)).setTextToDataFunc(MenuNormalColor)).setColor();
      props.New().create("Check Image"        , MemberDesc(MEMBER(EditGuiSkin, base.menu.check_id)).setFunc(MenuCheck, MenuCheck)).elmType(ELM_IMAGE);
      props.New().create("Check Color"        , MemberDesc(MEMBER(EditGuiSkin, menu.check_color)).setTextToDataFunc(MenuCheckColor)).setColor();
      props.New().create("Sub-Menu Image"     , MemberDesc(MEMBER(EditGuiSkin, base.menu.sub_menu_id)).setFunc(MenuSubMenu, MenuSubMenu)).elmType(ELM_IMAGE);
      props.New().create("Sub-Menu Color"     , MemberDesc(MEMBER(EditGuiSkin, menu.sub_menu_color)).setTextToDataFunc(MenuSubMenuColor)).setColor();
      props.New().create("Padding"            , MemberDesc(MEMBER(EditGuiSkin, menu.padding)).setTextToDataFunc(MenuPadding)).range(0, 0.1).mouseEditSpeed(0.02);
      props.New().create("List Element Height", MemberDesc(MEMBER(EditGuiSkin, menu.list_elm_height)).setTextToDataFunc(MenuListElmHeight)).range(0, 1).mouseEditSpeed(0.02);
      props.New();
      props.New().create("MenuBar:");
      props.New().create("Background Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.menubar.background_id)).setFunc(MenuBarBackground, MenuBarBackground)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Background Color"      , MemberDesc(MEMBER(EditGuiSkin, menubar.background_color)).setTextToDataFunc(MenuBarBackgroundColor)).setColor();
      props.New().create("Highlight Panel Image" , MemberDesc(MEMBER(EditGuiSkin, base.menubar.highlight_id)).setFunc(MenuBarHighlight, MenuBarHighlight)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Highlight Color"       , MemberDesc(MEMBER(EditGuiSkin, menubar.highlight_color)).setTextToDataFunc(MenuBarHighlightColor)).setColor();
      props.New().create("Bar Height"            , MemberDesc(MEMBER(EditGuiSkin, menubar.bar_height)).setTextToDataFunc(MenuBarBarHeight)).range(0, 1).mouseEditSpeed(0.05);
      props.New().create("Text Size"             , MemberDesc(MEMBER(EditGuiSkin, menubar.text_size)).setTextToDataFunc(MenuBarTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Padd"             , MemberDesc(MEMBER(EditGuiSkin, menubar.text_padd)).setTextToDataFunc(MenuBarTextPadd)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Style"            , MemberDesc(MEMBER(EditGuiSkin, base.menubar.text_style_id)).setFunc(MenuBarTextStyle, MenuBarTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New();
      props.New().create("Progress:");
      props.New().create("Background Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.progress.background_id)).setFunc(ProgressBackground, ProgressBackground)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Background Color"      , MemberDesc(MEMBER(EditGuiSkin, progress.background_color)).setTextToDataFunc(ProgressBackgroundColor)).setColor();
      props.New().create("Progress Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.progress.progress_id)).setFunc(ProgressProgress, ProgressProgress)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Progress Color"        , MemberDesc(MEMBER(EditGuiSkin, progress.progress_color)).setTextToDataFunc(ProgressProgressColor)).setColor();
      props.New().create("Progress Partial"      , MemberDesc(MEMBER(EditGuiSkin, progress.draw_progress_partial)).setTextToDataFunc(ProgressProgressPart));
      props.New().create("Text Size"             , MemberDesc(MEMBER(EditGuiSkin, progress.text_size)).setTextToDataFunc(ProgressTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Style"            , MemberDesc(MEMBER(EditGuiSkin, base.progress.text_style_id)).setFunc(ProgressTextStyle, ProgressTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New();
      props.New().create("Property:");
      props.New().create("Value Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.property.value.normal_id)).setFunc(PropertyValueNormal, PropertyValueNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Value Normal Color"      , MemberDesc(MEMBER(EditGuiSkin, property.value.normal_color)).setTextToDataFunc(PropertyValueNormalColor)).setColor();
      props.New().create("Value Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.property.value.pushed_id)).setFunc(PropertyValuePushed, PropertyValuePushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Value Pushed Color"      , MemberDesc(MEMBER(EditGuiSkin, property.value.pushed_color)).setTextToDataFunc(PropertyValuePushedColor)).setColor();
      props.New().create("Value Image"             , MemberDesc(MEMBER(EditGuiSkin, base.property.value.image_id)).setFunc(PropertyValueImage, PropertyValueImage)).elmType(ELM_IMAGE);
      props.New().create("Value Image Color"       , MemberDesc(MEMBER(EditGuiSkin, property.value.image_color)).setTextToDataFunc(PropertyValueImageColor)).setColor();
    //props.New().create("Value Image Color Add"   , MemberDesc(MEMBER(EditGuiSkin, property.value.image_color_add)).setTextToDataFunc(PropertyValueImageColorAdd)).setColor();
      props.New();
      props.New().create("Region:");
      props.New().create("Normal Panel"           , MemberDesc(MEMBER(EditGuiSkin, base.region.normal_id)).setFunc(RegionNormal, RegionNormal)).elmType(ELM_PANEL);
      props.New().create("Normal Color"           , MemberDesc(MEMBER(EditGuiSkin, region.normal_color)).setTextToDataFunc(RegionNormalColor)).setColor();
      props.New().create("View Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.region.view.normal_id)).setFunc(RegionViewNormal, RegionViewNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("View Normal Color"      , MemberDesc(MEMBER(EditGuiSkin, region.view.normal_color)).setTextToDataFunc(RegionViewNormalColor)).setColor();
      props.New().create("View Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.region.view.pushed_id)).setFunc(RegionViewPushed, RegionViewPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("View Pushed Color"      , MemberDesc(MEMBER(EditGuiSkin, region.view.pushed_color)).setTextToDataFunc(RegionViewPushedColor)).setColor();
      props.New().create("View Image"             , MemberDesc(MEMBER(EditGuiSkin, base.region.view.image_id)).setFunc(RegionViewImage, RegionViewImage)).elmType(ELM_IMAGE);
      props.New().create("View Image Color"       , MemberDesc(MEMBER(EditGuiSkin, region.view.image_color)).setTextToDataFunc(RegionViewImageColor)).setColor();
    //props.New().create("View Image Color Add"   , MemberDesc(MEMBER(EditGuiSkin, region.view.image_color_add)).setTextToDataFunc(RegionViewImageColorAdd)).setColor();
      props.New();
      props.New().create("SlideBar:");
      props.New().create("Background Panel Image"   , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.background_id)).setFunc(SlideBarBackground, SlideBarBackground)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Background Color"         , MemberDesc(MEMBER(EditGuiSkin, slidebar.background_color)).setTextToDataFunc(SlideBarBackgroundColor)).setColor();
      props.New().create("Left Normal Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.left.normal_id)).setFunc(SlideBarLeftNormal, SlideBarLeftNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Left Normal Color"        , MemberDesc(MEMBER(EditGuiSkin, slidebar.left.normal_color)).setTextToDataFunc(SlideBarLeftNormalColor)).setColor();
      props.New().create("Left Pushed Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.left.pushed_id)).setFunc(SlideBarLeftPushed, SlideBarLeftPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Left Pushed Color"        , MemberDesc(MEMBER(EditGuiSkin, slidebar.left.pushed_color)).setTextToDataFunc(SlideBarLeftPushedColor)).setColor();
      props.New().create("Left Image"               , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.left.image_id)).setFunc(SlideBarLeftImage, SlideBarLeftImage)).elmType(ELM_IMAGE);
      props.New().create("Left Image Color"         , MemberDesc(MEMBER(EditGuiSkin, slidebar.left.image_color)).setTextToDataFunc(SlideBarLeftImageColor)).setColor();
    //props.New().create("Left Image Color Add"     , MemberDesc(MEMBER(EditGuiSkin, slidebar.left.image_color_add)).setTextToDataFunc(SlideBarLeftImageColorAdd)).setColor();
      props.New().create("Right Normal Panel Image" , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.right.normal_id)).setFunc(SlideBarRightNormal, SlideBarRightNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Right Normal Color"       , MemberDesc(MEMBER(EditGuiSkin, slidebar.right.normal_color)).setTextToDataFunc(SlideBarRightNormalColor)).setColor();
      props.New().create("Right Pushed Panel Image" , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.right.pushed_id)).setFunc(SlideBarRightPushed, SlideBarRightPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Right Pushed Color"       , MemberDesc(MEMBER(EditGuiSkin, slidebar.right.pushed_color)).setTextToDataFunc(SlideBarRightPushedColor)).setColor();
      props.New().create("Right Image"              , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.right.image_id)).setFunc(SlideBarRightImage, SlideBarRightImage)).elmType(ELM_IMAGE);
      props.New().create("Right Image Color"        , MemberDesc(MEMBER(EditGuiSkin, slidebar.right.image_color)).setTextToDataFunc(SlideBarRightImageColor)).setColor();
    //props.New().create("Right Image Color Add"    , MemberDesc(MEMBER(EditGuiSkin, slidebar.right.image_color_add)).setTextToDataFunc(SlideBarRightImageColorAdd)).setColor();
      props.New().create("Center Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.slidebar.center.normal_id)).setFunc(SlideBarCenterNormal, SlideBarCenterNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Center Normal Color"      , MemberDesc(MEMBER(EditGuiSkin, slidebar.center.normal_color)).setTextToDataFunc(SlideBarCenterNormalColor)).setColor();
      props.New().create("Center Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.slidebar.center.pushed_id)).setFunc(SlideBarCenterPushed, SlideBarCenterPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Center Pushed Color"      , MemberDesc(MEMBER(EditGuiSkin, slidebar.center.pushed_color)).setTextToDataFunc(SlideBarCenterPushedColor)).setColor();
      props.New().create("Center Image"             , MemberDesc(MEMBER(EditGuiSkin, base.slidebar.center.image_id)).setFunc(SlideBarCenterImage, SlideBarCenterImage)).elmType(ELM_IMAGE);
      props.New().create("Center Image Color"       , MemberDesc(MEMBER(EditGuiSkin, slidebar.center.image_color)).setTextToDataFunc(SlideBarCenterImageColor)).setColor();
    //props.New().create("Center Image Color Add"   , MemberDesc(MEMBER(EditGuiSkin, slidebar.center.image_color_add)).setTextToDataFunc(SlideBarCenterImageColorAdd)).setColor();
      props.New();
      props.New().create("Slider:");
      props.New().create("Background Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.slider.background_id)).setFunc(SliderBackground, SliderBackground)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Background Color"      , MemberDesc(MEMBER(EditGuiSkin, slider.background_color)).setTextToDataFunc(SliderBackgroundColor)).setColor();
      props.New().create("Background Shrink"     , MemberDesc(MEMBER(EditGuiSkin, slider.background_shrink)).setTextToDataFunc(SliderBackgroundShrink)).range(0, 0.5);
      props.New().create("Progress Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.slider.progress_id)).setFunc(SliderProgress, SliderProgress)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Progress Color"        , MemberDesc(MEMBER(EditGuiSkin, slider.progress_color)).setTextToDataFunc(SliderProgressColor)).setColor();
      props.New().create("Progress Partial"      , MemberDesc(MEMBER(EditGuiSkin, slider.draw_progress_partial)).setTextToDataFunc(SliderProgressPart));
      props.New().create("Slider Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.slider.slider_id)).setFunc(SliderSlider, SliderSlider)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Slider Color"          , MemberDesc(MEMBER(EditGuiSkin, slider.slider_color)).setTextToDataFunc(SliderSliderColor)).setColor();
      props.New();
      props.New().create("Tabs:");
      props.New().create("Normal Color", MemberDesc(MEMBER(EditGuiSkin, tab.left.normal_color)).setTextToDataFunc(TabNormalColor)).setColor();
      props.New().create("Pushed Color", MemberDesc(MEMBER(EditGuiSkin, tab.left.pushed_color)).setTextToDataFunc(TabPushedColor)).setColor();
      props.New().create("Text Size"   , MemberDesc(MEMBER(EditGuiSkin, tab.left.text_size)).setTextToDataFunc(TabTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Padd"   , MemberDesc(MEMBER(EditGuiSkin, tab.left.text_padd)).setTextToDataFunc(TabTextPadd)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Style"  , MemberDesc(MEMBER(EditGuiSkin, base.tab.text_style_id)).setFunc(TabTextStyle, TabTextStyle)).elmType(ELM_TEXT_STYLE);

      props.New().create("Horizontal Normal Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.tab.horizontal  .normal_id)).setFunc(TabHorizontalNormal, TabHorizontalNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Horizontal Pushed Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.tab.horizontal  .pushed_id)).setFunc(TabHorizontalPushed, TabHorizontalPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Vertical Normal Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.tab.vertical    .normal_id)).setFunc(TabVerticalNormal, TabVerticalNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Vertical Pushed Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.tab.vertical    .pushed_id)).setFunc(TabVerticalPushed, TabVerticalPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Left Normal Panel Image"        , MemberDesc(MEMBER(EditGuiSkin, base.tab.left        .normal_id)).setFunc(TabLeftNormal, TabLeftNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Left Pushed Panel Image"        , MemberDesc(MEMBER(EditGuiSkin, base.tab.left        .pushed_id)).setFunc(TabLeftPushed, TabLeftPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Right Normal Panel Image"       , MemberDesc(MEMBER(EditGuiSkin, base.tab.right       .normal_id)).setFunc(TabRightNormal, TabRightNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Right Pushed Panel Image"       , MemberDesc(MEMBER(EditGuiSkin, base.tab.right       .pushed_id)).setFunc(TabRightPushed, TabRightPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Top Normal Panel Image"         , MemberDesc(MEMBER(EditGuiSkin, base.tab.top         .normal_id)).setFunc(TabTopNormal, TabTopNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Top Pushed Panel Image"         , MemberDesc(MEMBER(EditGuiSkin, base.tab.top         .pushed_id)).setFunc(TabTopPushed, TabTopPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Bottom Normal Panel Image"      , MemberDesc(MEMBER(EditGuiSkin, base.tab.bottom      .normal_id)).setFunc(TabBottomNormal, TabBottomNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Bottom Pushed Panel Image"      , MemberDesc(MEMBER(EditGuiSkin, base.tab.bottom      .pushed_id)).setFunc(TabBottomPushed, TabBottomPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Top Left Normal Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.tab.top_left    .normal_id)).setFunc(TabTopLeftNormal, TabTopLeftNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Top Left Pushed Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.tab.top_left    .pushed_id)).setFunc(TabTopLeftPushed, TabTopLeftPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Top Right Normal Panel Image"   , MemberDesc(MEMBER(EditGuiSkin, base.tab.top_right   .normal_id)).setFunc(TabTopRightNormal, TabTopRightNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Top Right Pushed Panel Image"   , MemberDesc(MEMBER(EditGuiSkin, base.tab.top_right   .pushed_id)).setFunc(TabTopRightPushed, TabTopRightPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Bottom Left Normal Panel Image" , MemberDesc(MEMBER(EditGuiSkin, base.tab.bottom_left .normal_id)).setFunc(TabBottomLeftNormal, TabBottomLeftNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Bottom Left Pushed Panel Image" , MemberDesc(MEMBER(EditGuiSkin, base.tab.bottom_left .pushed_id)).setFunc(TabBottomLeftPushed, TabBottomLeftPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Bottom Right Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.tab.bottom_right.normal_id)).setFunc(TabBottomRightNormal, TabBottomRightNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Bottom Right Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.tab.bottom_right.pushed_id)).setFunc(TabBottomRightPushed, TabBottomRightPushed)).elmType(ELM_PANEL_IMAGE);
      props.New();
      props.New().create("Text:");
      props.New().create("Text Style", MemberDesc(MEMBER(EditGuiSkin, base.text.text_style_id)).setFunc(TextTextStyle, TextTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New();
      props.New().create("TextLine:");
      props.New().create("Normal Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.textline.normal_id)).setFunc(TextLineNormal, TextLineNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Normal Panel Color"  , MemberDesc(MEMBER(EditGuiSkin, textline.normal_panel_color)).setTextToDataFunc(TextLineNormalPanelColor)).setColor();
      props.New().create("Normal Text Color"   , MemberDesc(MEMBER(EditGuiSkin, textline.normal_text_color )).setTextToDataFunc(TextLineNormalTextColor )).setColor();
      props.New().create("Disabled Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.textline.disabled_id)).setFunc(TextLineDisabled, TextLineDisabled)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Disabled Panel Color", MemberDesc(MEMBER(EditGuiSkin, textline.disabled_panel_color)).setTextToDataFunc(TextLineDisabledPanelColor)).setColor();
      props.New().create("Disabled Text Color" , MemberDesc(MEMBER(EditGuiSkin, textline.disabled_text_color )).setTextToDataFunc(TextLineDisabledTextColor )).setColor();
      props.New().create("Rect Color"          , MemberDesc(MEMBER(EditGuiSkin, textline.rect_color)).setTextToDataFunc(TextLineRectColor)).setColor();
      props.New().create("Text Size"           , MemberDesc(MEMBER(EditGuiSkin, textline.text_size)).setTextToDataFunc(TextLineTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Style"          , MemberDesc(MEMBER(EditGuiSkin, base.textline.text_style_id)).setFunc(TextLineTextStyle, TextLineTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Find Image"          , MemberDesc(MEMBER(EditGuiSkin, base.textline.find_image_id)).setFunc(TextLineFindImage, TextLineFindImage)).elmType(ELM_IMAGE);
      props.New().create("Clear Image"         , MemberDesc(MEMBER(EditGuiSkin, base.textline.clear.image_id)).setFunc(TextLineClearImage, TextLineClearImage)).elmType(ELM_IMAGE);
      props.New();
      props.New().create("Window:");
      props.New().create("Normal Panel"               , MemberDesc(MEMBER(EditGuiSkin, base.window.normal_id)).setFunc(WindowNormal, WindowNormal)).elmType(ELM_PANEL);
      props.New().create("Normal Color"               , MemberDesc(MEMBER(EditGuiSkin, window.normal_color)).setTextToDataFunc(WindowNormalColor)).setColor();
      props.New().create("Active Panel"               , MemberDesc(MEMBER(EditGuiSkin, base.window.active_id)).setFunc(WindowActive, WindowActive)).elmType(ELM_PANEL);
      props.New().create("Active Color"               , MemberDesc(MEMBER(EditGuiSkin, window.active_color)).setTextToDataFunc(WindowActiveColor)).setColor();
      props.New().create("Normal No Bar Panel"        , MemberDesc(MEMBER(EditGuiSkin, base.window.normal_no_bar_id)).setFunc(WindowNormalNoBar, WindowNormalNoBar)).elmType(ELM_PANEL);
      props.New().create("Active No Bar Panel"        , MemberDesc(MEMBER(EditGuiSkin, base.window.active_no_bar_id)).setFunc(WindowActiveNoBar, WindowActiveNoBar)).elmType(ELM_PANEL);
      props.New().create("Text Size"                  , MemberDesc(MEMBER(EditGuiSkin, window.text_size)).setTextToDataFunc(WindowTextSize)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Text Padd"                  , MemberDesc(MEMBER(EditGuiSkin, window.text_padd)).setTextToDataFunc(WindowTextPadd)).range(0, 1).mouseEditSpeed(0.2);
      props.New().create("Normal Text Style"          , MemberDesc(MEMBER(EditGuiSkin, base.window.normal_text_style_id)).setFunc(WindowNormalTextStyle, WindowNormalTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Active Text Style"          , MemberDesc(MEMBER(EditGuiSkin, base.window.active_text_style_id)).setFunc(WindowActiveTextStyle, WindowActiveTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Minimize Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.window.minimize.normal_id)).setFunc(WindowMinimizeNormal, WindowMinimizeNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Minimize Normal Color"      , MemberDesc(MEMBER(EditGuiSkin, window.minimize.normal_color)).setTextToDataFunc(WindowMinimizeNormalColor)).setColor();
      props.New().create("Minimize Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.window.minimize.pushed_id)).setFunc(WindowMinimizePushed, WindowMinimizePushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Minimize Pushed Color"      , MemberDesc(MEMBER(EditGuiSkin, window.minimize.pushed_color)).setTextToDataFunc(WindowMinimizePushedColor)).setColor();
      props.New().create("Minimize Disabled Color"    , MemberDesc(MEMBER(EditGuiSkin, window.minimize.disabled_color)).setTextToDataFunc(WindowMinimizeDisabledColor)).setColor();
      props.New().create("Minimize Image"             , MemberDesc(MEMBER(EditGuiSkin, base.window.minimize.image_id)).setFunc(WindowMinimizeImage, WindowMinimizeImage)).elmType(ELM_IMAGE);
      props.New().create("Minimize Image Color"       , MemberDesc(MEMBER(EditGuiSkin, window.minimize.image_color)).setTextToDataFunc(WindowMinimizeImageColor)).setColor();
    //props.New().create("Minimize Image Color Add"   , MemberDesc(MEMBER(EditGuiSkin, window.minimize.image_color_add)).setTextToDataFunc(WindowMinimizeImageColorAdd)).setColor();
      props.New().create("Maximize Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.window.maximize.normal_id)).setFunc(WindowMaximizeNormal, WindowMaximizeNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Maximize Normal Color"      , MemberDesc(MEMBER(EditGuiSkin, window.maximize.normal_color)).setTextToDataFunc(WindowMaximizeNormalColor)).setColor();
      props.New().create("Maximize Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.window.maximize.pushed_id)).setFunc(WindowMaximizePushed, WindowMaximizePushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Maximize Pushed Color"      , MemberDesc(MEMBER(EditGuiSkin, window.maximize.pushed_color)).setTextToDataFunc(WindowMaximizePushedColor)).setColor();
      props.New().create("Maximize Disabled Color"    , MemberDesc(MEMBER(EditGuiSkin, window.maximize.disabled_color)).setTextToDataFunc(WindowMaximizeDisabledColor)).setColor();
      props.New().create("Maximize Image"             , MemberDesc(MEMBER(EditGuiSkin, base.window.maximize.image_id)).setFunc(WindowMaximizeImage, WindowMaximizeImage)).elmType(ELM_IMAGE);
      props.New().create("Maximize Image Color"       , MemberDesc(MEMBER(EditGuiSkin, window.maximize.image_color)).setTextToDataFunc(WindowMaximizeImageColor)).setColor();
    //props.New().create("Maximize Image Color Add"   , MemberDesc(MEMBER(EditGuiSkin, window.maximize.image_color_add)).setTextToDataFunc(WindowMaximizeImageColorAdd)).setColor();
      props.New().create("Close Normal Panel Image"   , MemberDesc(MEMBER(EditGuiSkin, base.window.close.normal_id)).setFunc(WindowCloseNormal, WindowCloseNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Close Normal Color"         , MemberDesc(MEMBER(EditGuiSkin, window.close.normal_color)).setTextToDataFunc(WindowCloseNormalColor)).setColor();
      props.New().create("Close Pushed Panel Image"   , MemberDesc(MEMBER(EditGuiSkin, base.window.close.pushed_id)).setFunc(WindowClosePushed, WindowClosePushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Close Pushed Color"         , MemberDesc(MEMBER(EditGuiSkin, window.close.pushed_color)).setTextToDataFunc(WindowClosePushedColor)).setColor();
      props.New().create("Close Disabled Color"       , MemberDesc(MEMBER(EditGuiSkin, window.close.disabled_color)).setTextToDataFunc(WindowCloseDisabledColor)).setColor();
      props.New().create("Close Image"                , MemberDesc(MEMBER(EditGuiSkin, base.window.close.image_id)).setFunc(WindowCloseImage, WindowCloseImage)).elmType(ELM_IMAGE);
      props.New().create("Close Image Color"          , MemberDesc(MEMBER(EditGuiSkin, window.close.image_color)).setTextToDataFunc(WindowCloseImageColor)).setColor();
    //props.New().create("Close Image Color Add"      , MemberDesc(MEMBER(EditGuiSkin, window.close.image_color_add)).setTextToDataFunc(WindowCloseImageColorAdd)).setColor();
      props.New().create("Button Offset"              , MemberDesc(MEMBER(EditGuiSkin, window.button_offset)).setTextToDataFunc(WindowButtonOffset)).range(-1, 1).mouseEditSpeed(0.2);

      REPAO(props).autoData(&edit).changed(Changed, PreChanged);

      parent+=super.create().skin(&EmptyGuiSkin, false); T.kb_lit=false;
      T+=undo      .create(Rect_LU(0.02, -0.01     , 0.05, 0.05)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo      .create(Rect_LU(undo.rect().ru(), 0.05, 0.05)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate    .create(Rect_LU(redo.rect().ru()+Vec2(0.01, 0), 0.14, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      T+=props_region.create();
      T+= objs_region.create().removeSlideBars().skin(&EmptyGuiSkin, false); objs_region.kb_lit=false;
      props_ts.reset().size=0.036; props_ts.align.set(1, 0);
      props_x=AddProperties(props, props_region, Vec2(0.01, -0.01), 0.044, 0.5, &props_ts).max.x+props_region.slidebarSize()+0.02;
      objs.create(objs_region, Vec2(0.05, -0.03));
      removeSlideBars();
      return T;
   }
   void toGame() {if(game)edit.copyTo(*game, Proj); objs.setRect();}
   void toGui () {REPAO(props).toGui(); toGame();}

   void flush()
   {
      if(elm && game && changed)
      {
         if(ElmGuiSkin *data=elm.guiSkinData()){data.newVer(); data.from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(*elm)); // edit
         Save(*game, Proj.gamePath(*elm)); Proj.savedGame(*elm); // game
         Server.setElmLong(elm.id);
      }
      changed=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmGuiSkin *data=elm.guiSkinData()){data.newVer(); data.from(edit);}
         toGame();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_GUI_SKIN)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm){edit.load(Proj.editPath(*elm)); game=Proj.gamePath(elm_id);}else{edit.reset(); game.clear(); REPAO(props).close();}
         objs.setSkin(game);
         toGui();
         Proj.refresh(false, false);
         Mode.tabAvailable(MODE_GUI_SKIN, elm!=null);
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm){Mode.set(MODE_GUI_SKIN); HideBig();}}
   void toggle  (Elm *elm)
   {
      if(elm==T.elm && selected())elm=null;
      activate(elm);
   }
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
         EditGuiSkin temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp))toGui();
      }
   }
   void draw()
   {
      D.clear(edit.background_color);
   }
   virtual void draw(C GuiPC &gpc)override
   {
      super.draw(gpc);
      if(gpc.visible)
      {
         D.clip();
         GuiSkinPtr temp=Gui.skin; Gui.skin=game;
         DrawDescription(null, objs.checkbox_multi.screenRect().ru()+Vec2(0.06, 0), u"Description", false);
         Vec2 pos(objs.slider_vertical.screenRect().centerX(), 
                     Avg(objs.textline.screenRect().centerY(), objs.textline_disabled.screenRect().centerY()));
         Rect_C(pos, 0.045).draw(Gui.borderColor(), false);
         Gui.skin=temp;
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
}
GuiSkinEditor GuiSkinEdit;
/******************************************************************************/
