/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
GuiSkinEditor GuiSkinEdit;
/******************************************************************************/

/******************************************************************************/
      cchar8 *GuiSkinEditor::Objs::combobox_normal_t[]=
      {
         "Normal 0",
         "Normal 1",
         "Normal 2",
      };
      cchar8 *GuiSkinEditor::Objs::combobox_pushed_t[]=
      {
         "Pushed 0",
         "Pushed 1",
         "Pushed 2",
      };
      cchar8 *GuiSkinEditor::Objs::tabs_t[]=
      {
         "Tab 0",
         "Tab 1",
         "Tab 2",
      };
      ::GuiSkinEditor::Objs::ListElm GuiSkinEditor::Objs::list_t[]=
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
/******************************************************************************/
      void GuiSkinEditor::Change::create(ptr user)
{
         data=GuiSkinEdit.edit;
         GuiSkinEdit.undoVis();
      }
      void GuiSkinEditor::Change::apply(ptr user)
{
         GuiSkinEdit.edit.undo(data);
         GuiSkinEdit.setChanged();
         GuiSkinEdit.toGui();
         GuiSkinEdit.undoVis();
      }
      void GuiSkinEditor::Objs::ButtonPushed::update(C GuiPC &gpc){::EE::Button::update(gpc); set(true , QUIET);}
      void GuiSkinEditor::Objs::CheckBoxOff::update(C GuiPC &gpc){::EE::CheckBox::update(gpc); set(false, QUIET);}
      void GuiSkinEditor::Objs::CheckBoxOn::update(C GuiPC &gpc){::EE::CheckBox::update(gpc); set(true , QUIET);}
      void GuiSkinEditor::Objs::CheckBoxMulti::update(C GuiPC &gpc){::EE::CheckBox::update(gpc); setMulti();}
      void GuiSkinEditor::Objs::ComboBoxPushed::draw(C GuiPC &gpc)
{
         GuiSkinPtr old=skin();
         GuiSkin temp; if(C GuiSkin *skin=getSkin())temp=*skin; skin(&temp, false);
                                          temp.combobox.normal_color  =temp.combobox.pushed_color;
         if(temp.combobox.pushed         )temp.combobox.normal        =temp.combobox.pushed;
                                          temp.combobox.disabled_color=temp.combobox.pushed_disabled_color;
         if(temp.combobox.pushed_disabled)temp.combobox.disabled      =temp.combobox.pushed_disabled;
         ::EE::Button::draw(gpc);
         skin(old, false);
      }
      void GuiSkinEditor::Objs::WindowActive::draw(C GuiPC &gpc)
{
         GuiSkinPtr old=skin();
         GuiSkin temp; if(C GuiSkin *skin=getSkin())temp=*skin;
                                          temp.window.normal_color     =temp.window.active_color;
         if(temp.window.active           )temp.window.normal           =temp.window.active;
         if(temp.window.active_no_bar    )temp.window.normal_no_bar    =temp.window.active_no_bar;
         if(temp.window.active_text_style)temp.window.normal_text_style=temp.window.active_text_style;
         skin(&temp); ::EE::Window::draw(gpc);
         skin( old );
      }
      void GuiSkinEditor::Objs::Reset(GuiObj &go) {Rect r=go.rect(); go.rect(Rect(0, 0)); go.rect(r);}
      void GuiSkinEditor::Objs::setRect()
      {
         Reset(region);
         Reset(region_list);
         Reset(slidebar);
         Reset(slidebar_vertical);
         Reset(window);
         Reset(window_active);
         Reset(window_menu_buttons);
      }
      void GuiSkinEditor::Objs::setSkin(C GuiSkinPtr &skin)
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
      void GuiSkinEditor::Objs::create(GuiObj &parent, C Vec2 &position)
      {
         flt  h=0.06f, w=h*4, l=w*1.7f, W=w*2, s=h*0.7f, prog_h=0.05f;
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
         parent+=progress_half.create(Rect_LU(p, W, prog_h)).set(0.5f).desc("Half Progress"); p.y-=prog_h+s;
         parent+=progress_full.create(Rect_LU(p, W, prog_h)).set(1.0f).desc("Full Progress"); p.y-=prog_h+s;
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
            ListColumn(MEMBER(ListElm, t), l*1.5f, "Name"),
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
      void GuiSkinEditor::SkinRegion::update(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            GuiSkinPtr temp=Gui.skin; Gui.skin=GuiSkinEdit.game;
            ::EE::Region::update(gpc);
            Gui.skin=temp;
         }
      }
      void GuiSkinEditor::SkinRegion::draw(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            GuiSkinPtr temp=Gui.skin; Gui.skin=GuiSkinEdit.game;
            ::EE::Region::draw(gpc);
            Gui.skin=temp;
         }
      }
   void GuiSkinEditor::undoVis() {SetUndo(undos, undo, redo);}
   void GuiSkinEditor::PreChanged(C Property &prop) {GuiSkinEdit.undos.set(&prop);}
   void    GuiSkinEditor::Changed(C Property &prop) {GuiSkinEdit.setChanged();}
   Str  GuiSkinEditor::DefaultFont(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.font_id);}
   void GuiSkinEditor::DefaultFont(  EditGuiSkin &e, C Str &t) {e.base.font_id=Proj.findElmID(t, ELM_FONT); e.base.font.getUTC();}
   Str  GuiSkinEditor::DefaultTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.text_style_id);}
   void GuiSkinEditor::DefaultTextStyle(  EditGuiSkin &e, C Str &t) {e.base.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.text_style.getUTC();}
   void        GuiSkinEditor::BackgroundColor(EditGuiSkin &e, C Str &t) {e.        background_color=TextVec4(t); e.base.        background_color.getUTC();}
   void            GuiSkinEditor::BorderColor(EditGuiSkin &e, C Str &t) {e.            border_color=TextVec4(t); e.base.            border_color.getUTC();}
   void    GuiSkinEditor::MouseHighlightColor(EditGuiSkin &e, C Str &t) {e.   mouse_highlight_color=TextVec4(t); e.base.   mouse_highlight_color.getUTC();}
   void GuiSkinEditor::KeyboardHighlightColor(EditGuiSkin &e, C Str &t) {e.keyboard_highlight_color=TextVec4(t); e.base.keyboard_highlight_color.getUTC();}
   Str  GuiSkinEditor::ButtonNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.normal_id);}
   void GuiSkinEditor::ButtonNormal(  EditGuiSkin &e, C Str &t) {e.base.button.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.normal.getUTC();}
   void GuiSkinEditor::ButtonNormalColor(  EditGuiSkin &e, C Str &t) {e.button.normal_color=TextVec4(t); e.base.button.normal_color.getUTC();}
   Str  GuiSkinEditor::ButtonPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.pushed_id);}
   void GuiSkinEditor::ButtonPushed(  EditGuiSkin &e, C Str &t) {e.base.button.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.pushed.getUTC();}
   void GuiSkinEditor::ButtonPushedColor(  EditGuiSkin &e, C Str &t) {e.button.pushed_color=TextVec4(t); e.base.button.pushed_color.getUTC();}
   Str  GuiSkinEditor::ButtonDisabled(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.disabled_id);}
   void GuiSkinEditor::ButtonDisabled(  EditGuiSkin &e, C Str &t) {e.base.button.disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.disabled.getUTC();}
   void GuiSkinEditor::ButtonDisabledColor(  EditGuiSkin &e, C Str &t) {e.button.disabled_color=TextVec4(t); e.base.button.disabled_color.getUTC();}
   Str  GuiSkinEditor::ButtonPushedDisabled(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.pushed_disabled_id);}
   void GuiSkinEditor::ButtonPushedDisabled(  EditGuiSkin &e, C Str &t) {e.base.button.pushed_disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.button.pushed_disabled.getUTC();}
   void GuiSkinEditor::ButtonPushedDisabledColor(  EditGuiSkin &e, C Str &t) {e.button.pushed_disabled_color=TextVec4(t); e.base.button.pushed_disabled_color.getUTC();}
   void GuiSkinEditor::ButtonTextSize(  EditGuiSkin &e, C Str &t) {e.button.text_size=TextFlt(t); e.base.button.text_size.getUTC();}
   void GuiSkinEditor::ButtonTextPadd(  EditGuiSkin &e, C Str &t) {e.button.text_padd=TextFlt(t); e.base.button.text_padd.getUTC();}
   Str  GuiSkinEditor::ButtonTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.button.text_style_id);}
   void GuiSkinEditor::ButtonTextStyle(  EditGuiSkin &e, C Str &t) {e.base.button.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.button.text_style.getUTC();}
   Str  GuiSkinEditor::CheckBoxOff(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.checkbox.off_id);}
   void GuiSkinEditor::CheckBoxOff(  EditGuiSkin &e, C Str &t) {e.base.checkbox.off_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.checkbox.off.getUTC();}
   Str  GuiSkinEditor::CheckBoxOn(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.checkbox.on_id);}
   void GuiSkinEditor::CheckBoxOn(  EditGuiSkin &e, C Str &t) {e.base.checkbox.on_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.checkbox.on.getUTC();}
   Str  GuiSkinEditor::CheckBoxMulti(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.checkbox.multi_id);}
   void GuiSkinEditor::CheckBoxMulti(  EditGuiSkin &e, C Str &t) {e.base.checkbox.multi_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.checkbox.multi.getUTC();}
   void GuiSkinEditor::CheckBoxNormalColor(  EditGuiSkin &e, C Str &t) {e.checkbox.normal_color=TextVec4(t); e.base.checkbox.normal_color.getUTC();}
   void GuiSkinEditor::CheckBoxDisabledColor(  EditGuiSkin &e, C Str &t) {e.checkbox.disabled_color=TextVec4(t); e.base.checkbox.disabled_color.getUTC();}
   Str  GuiSkinEditor::ComboBoxSide(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.image_id);}
   void GuiSkinEditor::ComboBoxSide(  EditGuiSkin &e, C Str &t) {e.base.combobox.image_id=Proj.findElmImageID(t); e.base.combobox.image.getUTC();}
   void GuiSkinEditor::ComboBoxSideColor(  EditGuiSkin &e, C Str &t) {e.combobox.image_color=TextVec4(t); e.base.combobox.image_color.getUTC();}
   Str  GuiSkinEditor::ComboBoxNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.normal_id);}
   void GuiSkinEditor::ComboBoxNormal(  EditGuiSkin &e, C Str &t) {e.base.combobox.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.normal.getUTC();}
   void GuiSkinEditor::ComboBoxNormalColor(  EditGuiSkin &e, C Str &t) {e.combobox.normal_color=TextVec4(t); e.base.combobox.normal_color.getUTC();}
   Str  GuiSkinEditor::ComboBoxPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.pushed_id);}
   void GuiSkinEditor::ComboBoxPushed(  EditGuiSkin &e, C Str &t) {e.base.combobox.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.pushed.getUTC();}
   void GuiSkinEditor::ComboBoxPushedColor(  EditGuiSkin &e, C Str &t) {e.combobox.pushed_color=TextVec4(t); e.base.combobox.pushed_color.getUTC();}
   Str  GuiSkinEditor::ComboBoxDisabled(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.disabled_id);}
   void GuiSkinEditor::ComboBoxDisabled(  EditGuiSkin &e, C Str &t) {e.base.combobox.disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.disabled.getUTC();}
   void GuiSkinEditor::ComboBoxDisabledColor(  EditGuiSkin &e, C Str &t) {e.combobox.disabled_color=TextVec4(t); e.base.combobox.disabled_color.getUTC();}
   Str  GuiSkinEditor::ComboBoxPushedDisabled(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.pushed_disabled_id);}
   void GuiSkinEditor::ComboBoxPushedDisabled(  EditGuiSkin &e, C Str &t) {e.base.combobox.pushed_disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.combobox.pushed_disabled.getUTC();}
   void GuiSkinEditor::ComboBoxPushedDisabledColor(  EditGuiSkin &e, C Str &t) {e.combobox.pushed_disabled_color=TextVec4(t); e.base.combobox.pushed_disabled_color.getUTC();}
   void GuiSkinEditor::ComboBoxTextSize(  EditGuiSkin &e, C Str &t) {e.combobox.text_size=TextFlt(t); e.base.combobox.text_size.getUTC();}
   void GuiSkinEditor::ComboBoxTextPadd(  EditGuiSkin &e, C Str &t) {e.combobox.text_padd=TextFlt(t); e.base.combobox.text_padd.getUTC();}
   Str  GuiSkinEditor::ComboBoxTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.combobox.text_style_id);}
   void GuiSkinEditor::ComboBoxTextStyle(  EditGuiSkin &e, C Str &t) {e.base.combobox.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.combobox.text_style.getUTC();}
   Str  GuiSkinEditor::DescNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.desc.normal_id);}
   void GuiSkinEditor::DescNormal(  EditGuiSkin &e, C Str &t) {e.base.desc.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.desc.normal.getUTC();}
   void GuiSkinEditor::DescNormalColor(  EditGuiSkin &e, C Str &t) {e.desc.normal_color=TextVec4(t); e.base.desc.normal_color.getUTC();}
   Str  GuiSkinEditor::DescTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.desc.text_style_id);}
   void GuiSkinEditor::DescTextStyle(  EditGuiSkin &e, C Str &t) {e.base.desc.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.desc.text_style.getUTC();}
   void GuiSkinEditor::DescPadding(  EditGuiSkin &e, C Str &t) {e.desc.padding=TextFlt(t); e.base.desc.padding.getUTC();}
   Str  GuiSkinEditor::IMMNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.imm.normal_id);}
   void GuiSkinEditor::IMMNormal(  EditGuiSkin &e, C Str &t) {e.base.imm.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.imm.normal.getUTC();}
   void GuiSkinEditor::IMMNormalColor(  EditGuiSkin &e, C Str &t) {e.imm.normal_color=TextVec4(t); e.base.imm.normal_color.getUTC();}
   Str  GuiSkinEditor::IMMTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.imm.text_style_id);}
   void GuiSkinEditor::IMMTextStyle(  EditGuiSkin &e, C Str &t) {e.base.imm.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.imm.text_style.getUTC();}
   void GuiSkinEditor::IMMPadding(  EditGuiSkin &e, C Str &t) {e.imm.padding=TextFlt(t); e.base.imm.padding.getUTC();}
   Str  GuiSkinEditor::ListColumnNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.column.normal_id);}
   void GuiSkinEditor::ListColumnNormal(  EditGuiSkin &e, C Str &t) {e.base.list.column.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.column.normal.getUTC();}
   void GuiSkinEditor::ListColumnNormalColor(  EditGuiSkin &e, C Str &t) {e.list.column.normal_color=TextVec4(t); e.base.list.column.normal_color.getUTC();}
   Str  GuiSkinEditor::ListColumnPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.column.pushed_id);}
   void GuiSkinEditor::ListColumnPushed(  EditGuiSkin &e, C Str &t) {e.base.list.column.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.column.pushed.getUTC();}
   void GuiSkinEditor::ListColumnPushedColor(  EditGuiSkin &e, C Str &t) {e.list.column.pushed_color=TextVec4(t); e.base.list.column.pushed_color.getUTC();}
   void GuiSkinEditor::ListColumnTextSize(  EditGuiSkin &e, C Str &t) {e.list.column.text_size=TextFlt(t); e.base.list.column.text_size.getUTC();}
   void GuiSkinEditor::ListColumnTextPadd(  EditGuiSkin &e, C Str &t) {e.list.column.text_padd=TextFlt(t); e.base.list.column.text_padd.getUTC();}
   Str  GuiSkinEditor::ListColumnTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.column.text_style_id);}
   void GuiSkinEditor::ListColumnTextStyle(  EditGuiSkin &e, C Str &t) {e.base.list.column.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.list.column.text_style.getUTC();}
   Str  GuiSkinEditor::ListCursor(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.cursor_id);}
   void GuiSkinEditor::ListCursor(  EditGuiSkin &e, C Str &t) {e.base.list.cursor_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.cursor.getUTC();}
   void GuiSkinEditor::ListCursorColor(  EditGuiSkin &e, C Str &t) {e.list.cursor_color=TextVec4(t); e.base.list.cursor_color.getUTC();}
   Str  GuiSkinEditor::ListHighlight(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.highlight_id);}
   void GuiSkinEditor::ListHighlight(  EditGuiSkin &e, C Str &t) {e.base.list.highlight_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.highlight.getUTC();}
   void GuiSkinEditor::ListHighlightColor(  EditGuiSkin &e, C Str &t) {e.list.highlight_color=TextVec4(t); e.base.list.highlight_color.getUTC();}
   Str  GuiSkinEditor::ListSelection(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.selection_id);}
   void GuiSkinEditor::ListSelection(  EditGuiSkin &e, C Str &t) {e.base.list.selection_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.list.selection.getUTC();}
   void GuiSkinEditor::ListSelectionColor(  EditGuiSkin &e, C Str &t) {e.list.selection_color=TextVec4(t); e.base.list.selection_color.getUTC();}
   Str  GuiSkinEditor::ListTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.text_style_id);}
   void GuiSkinEditor::ListTextStyle(  EditGuiSkin &e, C Str &t) {e.base.list.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.list.text_style.getUTC();}
   Str  GuiSkinEditor::ListResizeColumn(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.list.resize_column_id);}
   void GuiSkinEditor::ListResizeColumn(  EditGuiSkin &e, C Str &t) {e.base.list.resize_column_id=Proj.findElmImageID(t); e.base.list.resize_column.getUTC();}
   Str  GuiSkinEditor::MenuNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menu.normal_id);}
   void GuiSkinEditor::MenuNormal(  EditGuiSkin &e, C Str &t) {e.base.menu.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.menu.normal.getUTC();}
   void GuiSkinEditor::MenuNormalColor(  EditGuiSkin &e, C Str &t) {e.menu.normal_color=TextVec4(t); e.base.menu.normal_color.getUTC();}
   Str  GuiSkinEditor::MenuCheck(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menu.check_id);}
   void GuiSkinEditor::MenuCheck(  EditGuiSkin &e, C Str &t) {e.base.menu.check_id=Proj.findElmImageID(t); e.base.menu.check.getUTC();}
   void GuiSkinEditor::MenuCheckColor(  EditGuiSkin &e, C Str &t) {e.menu.check_color=TextVec4(t); e.base.menu.check_color.getUTC();}
   Str  GuiSkinEditor::MenuSubMenu(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menu.sub_menu_id);}
   void GuiSkinEditor::MenuSubMenu(  EditGuiSkin &e, C Str &t) {e.base.menu.sub_menu_id=Proj.findElmImageID(t); e.base.menu.sub_menu.getUTC();}
   void GuiSkinEditor::MenuSubMenuColor(  EditGuiSkin &e, C Str &t) {e.menu.sub_menu_color=TextVec4(t); e.base.menu.sub_menu_color.getUTC();}
   void GuiSkinEditor::MenuPadding(  EditGuiSkin &e, C Str &t) {e.menu.padding=TextFlt(t); e.base.menu.padding.getUTC();}
   void GuiSkinEditor::MenuListElmHeight(  EditGuiSkin &e, C Str &t) {e.menu.list_elm_height=TextFlt(t); e.base.menu.list_elm_height.getUTC();}
   Str  GuiSkinEditor::MenuBarBackground(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menubar.background_id);}
   void GuiSkinEditor::MenuBarBackground(  EditGuiSkin &e, C Str &t) {e.base.menubar.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.menubar.background.getUTC();}
   void GuiSkinEditor::MenuBarBackgroundColor(  EditGuiSkin &e, C Str &t) {e.menubar.background_color=TextVec4(t); e.base.menubar.background_color.getUTC();}
   Str  GuiSkinEditor::MenuBarHighlight(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menubar.highlight_id);}
   void GuiSkinEditor::MenuBarHighlight(  EditGuiSkin &e, C Str &t) {e.base.menubar.highlight_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.menubar.highlight.getUTC();}
   void GuiSkinEditor::MenuBarHighlightColor(  EditGuiSkin &e, C Str &t) {e.menubar.highlight_color=TextVec4(t); e.base.menubar.highlight_color.getUTC();}
   void GuiSkinEditor::MenuBarBarHeight(  EditGuiSkin &e, C Str &t) {e.menubar.bar_height=TextFlt(t); e.base.menubar.bar_height.getUTC();}
   void GuiSkinEditor::MenuBarTextSize(  EditGuiSkin &e, C Str &t) {e.menubar.text_size=TextFlt(t); e.base.menubar.text_size.getUTC();}
   void GuiSkinEditor::MenuBarTextPadd(  EditGuiSkin &e, C Str &t) {e.menubar.text_padd=TextFlt(t); e.base.menubar.text_padd.getUTC();}
   Str  GuiSkinEditor::MenuBarTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.menubar.text_style_id);}
   void GuiSkinEditor::MenuBarTextStyle(  EditGuiSkin &e, C Str &t) {e.base.menubar.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.menubar.text_style.getUTC();}
   Str  GuiSkinEditor::ProgressBackground(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.progress.background_id);}
   void GuiSkinEditor::ProgressBackground(  EditGuiSkin &e, C Str &t) {e.base.progress.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.progress.background.getUTC();}
   void GuiSkinEditor::ProgressBackgroundColor(  EditGuiSkin &e, C Str &t) {e.progress.background_color=TextVec4(t); e.base.progress.background_color.getUTC();}
   Str  GuiSkinEditor::ProgressProgress(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.progress.progress_id);}
   void GuiSkinEditor::ProgressProgress(  EditGuiSkin &e, C Str &t) {e.base.progress.progress_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.progress.progress.getUTC();}
   void GuiSkinEditor::ProgressProgressColor(  EditGuiSkin &e, C Str &t) {e.progress.progress_color=TextVec4(t); e.base.progress.progress_color.getUTC();}
   void GuiSkinEditor::ProgressProgressPart(  EditGuiSkin &e, C Str &t) {e.progress.draw_progress_partial=TextBool(t); e.base.progress.draw_progress_partial.getUTC();}
   void GuiSkinEditor::ProgressTextSize(  EditGuiSkin &e, C Str &t) {e.progress.text_size=TextFlt(t); e.base.progress.text_size.getUTC();}
   Str  GuiSkinEditor::ProgressTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.progress.text_style_id);}
   void GuiSkinEditor::ProgressTextStyle(  EditGuiSkin &e, C Str &t) {e.base.progress.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.progress.text_style.getUTC();}
   Str  GuiSkinEditor::PropertyValueImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.property.value.image_id);}
   void GuiSkinEditor::PropertyValueImage(  EditGuiSkin &e, C Str &t) {e.base.property.value.image_id=Proj.findElmImageID(t); e.base.property.value.image.getUTC();}
   void GuiSkinEditor::PropertyValueImageColor(  EditGuiSkin &e, C Str &t) {e.property.value.image_color=TextVec4(t); e.base.property.value.image_color.getUTC();}
   Str  GuiSkinEditor::PropertyValueNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.property.value.normal_id);}
   void GuiSkinEditor::PropertyValueNormal(  EditGuiSkin &e, C Str &t) {e.base.property.value.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.property.value.normal.getUTC();}
   void GuiSkinEditor::PropertyValueNormalColor(  EditGuiSkin &e, C Str &t) {e.property.value.normal_color=TextVec4(t); e.base.property.value.normal_color.getUTC();}
   Str  GuiSkinEditor::PropertyValuePushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.property.value.pushed_id);}
   void GuiSkinEditor::PropertyValuePushed(  EditGuiSkin &e, C Str &t) {e.base.property.value.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.property.value.pushed.getUTC();}
   void GuiSkinEditor::PropertyValuePushedColor(  EditGuiSkin &e, C Str &t) {e.property.value.pushed_color=TextVec4(t); e.base.property.value.pushed_color.getUTC();}
   Str  GuiSkinEditor::RegionNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.normal_id);}
   void GuiSkinEditor::RegionNormal(  EditGuiSkin &e, C Str &t) {e.base.region.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.region.normal.getUTC();}
   void GuiSkinEditor::RegionNormalColor(  EditGuiSkin &e, C Str &t) {e.region.normal_color=TextVec4(t); e.base.region.normal_color.getUTC();}
   Str  GuiSkinEditor::RegionViewImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.view.image_id);}
   void GuiSkinEditor::RegionViewImage(  EditGuiSkin &e, C Str &t) {e.base.region.view.image_id=Proj.findElmImageID(t); e.base.region.view.image.getUTC();}
   void GuiSkinEditor::RegionViewImageColor(  EditGuiSkin &e, C Str &t) {e.region.view.image_color=TextVec4(t); e.base.region.view.image_color.getUTC();}
   Str  GuiSkinEditor::RegionViewNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.view.normal_id);}
   void GuiSkinEditor::RegionViewNormal(  EditGuiSkin &e, C Str &t) {e.base.region.view.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.region.view.normal.getUTC();}
   void GuiSkinEditor::RegionViewNormalColor(  EditGuiSkin &e, C Str &t) {e.region.view.normal_color=TextVec4(t); e.base.region.view.normal_color.getUTC();}
   Str  GuiSkinEditor::RegionViewPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.region.view.pushed_id);}
   void GuiSkinEditor::RegionViewPushed(  EditGuiSkin &e, C Str &t) {e.base.region.view.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.region.view.pushed.getUTC();}
   void GuiSkinEditor::RegionViewPushedColor(  EditGuiSkin &e, C Str &t) {e.region.view.pushed_color=TextVec4(t); e.base.region.view.pushed_color.getUTC();}
   Str  GuiSkinEditor::SlideBarBackground(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.background_id);}
   void GuiSkinEditor::SlideBarBackground(  EditGuiSkin &e, C Str &t) {e.base.slidebar.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.background.getUTC();}
   void GuiSkinEditor::SlideBarBackgroundColor(  EditGuiSkin &e, C Str &t) {e.slidebar.background_color=TextVec4(t); e.base.slidebar.background_color.getUTC();}
   Str  GuiSkinEditor::SlideBarLeftImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.left.image_id);}
   void GuiSkinEditor::SlideBarLeftImage(  EditGuiSkin &e, C Str &t) {e.base.slidebar.left.image_id=Proj.findElmImageID(t); e.base.slidebar.left.image.getUTC();}
   void GuiSkinEditor::SlideBarLeftImageColor(  EditGuiSkin &e, C Str &t) {e.slidebar.left.image_color=TextVec4(t); e.base.slidebar.left.image_color.getUTC();}
   Str  GuiSkinEditor::SlideBarLeftNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.left.normal_id);}
   void GuiSkinEditor::SlideBarLeftNormal(  EditGuiSkin &e, C Str &t) {e.base.slidebar.left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.left.normal.getUTC();}
   void GuiSkinEditor::SlideBarLeftNormalColor(  EditGuiSkin &e, C Str &t) {e.slidebar.left.normal_color=TextVec4(t); e.base.slidebar.left.normal_color.getUTC();}
   Str  GuiSkinEditor::SlideBarLeftPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.left.pushed_id);}
   void GuiSkinEditor::SlideBarLeftPushed(  EditGuiSkin &e, C Str &t) {e.base.slidebar.left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.left.pushed.getUTC();}
   void GuiSkinEditor::SlideBarLeftPushedColor(  EditGuiSkin &e, C Str &t) {e.slidebar.left.pushed_color=TextVec4(t); e.base.slidebar.left.pushed_color.getUTC();}
   Str  GuiSkinEditor::SlideBarCenterImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.center.image_id);}
   void GuiSkinEditor::SlideBarCenterImage(  EditGuiSkin &e, C Str &t) {e.base.slidebar.center.image_id=Proj.findElmImageID(t); e.base.slidebar.center.image.getUTC();}
   void GuiSkinEditor::SlideBarCenterImageColor(  EditGuiSkin &e, C Str &t) {e.slidebar.center.image_color=TextVec4(t); e.base.slidebar.center.image_color.getUTC();}
   Str  GuiSkinEditor::SlideBarCenterNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.center.normal_id);}
   void GuiSkinEditor::SlideBarCenterNormal(  EditGuiSkin &e, C Str &t) {e.base.slidebar.center.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.center.normal.getUTC();}
   void GuiSkinEditor::SlideBarCenterNormalColor(  EditGuiSkin &e, C Str &t) {e.slidebar.center.normal_color=TextVec4(t); e.base.slidebar.center.normal_color.getUTC();}
   Str  GuiSkinEditor::SlideBarCenterPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.center.pushed_id);}
   void GuiSkinEditor::SlideBarCenterPushed(  EditGuiSkin &e, C Str &t) {e.base.slidebar.center.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.center.pushed.getUTC();}
   void GuiSkinEditor::SlideBarCenterPushedColor(  EditGuiSkin &e, C Str &t) {e.slidebar.center.pushed_color=TextVec4(t); e.base.slidebar.center.pushed_color.getUTC();}
   Str  GuiSkinEditor::SlideBarRightImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.right.image_id);}
   void GuiSkinEditor::SlideBarRightImage(  EditGuiSkin &e, C Str &t) {e.base.slidebar.right.image_id=Proj.findElmImageID(t); e.base.slidebar.right.image.getUTC();}
   void GuiSkinEditor::SlideBarRightImageColor(  EditGuiSkin &e, C Str &t) {e.slidebar.right.image_color=TextVec4(t); e.base.slidebar.right.image_color.getUTC();}
   Str  GuiSkinEditor::SlideBarRightNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.right.normal_id);}
   void GuiSkinEditor::SlideBarRightNormal(  EditGuiSkin &e, C Str &t) {e.base.slidebar.right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.right.normal.getUTC();}
   void GuiSkinEditor::SlideBarRightNormalColor(  EditGuiSkin &e, C Str &t) {e.slidebar.right.normal_color=TextVec4(t); e.base.slidebar.right.normal_color.getUTC();}
   Str  GuiSkinEditor::SlideBarRightPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slidebar.right.pushed_id);}
   void GuiSkinEditor::SlideBarRightPushed(  EditGuiSkin &e, C Str &t) {e.base.slidebar.right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slidebar.right.pushed.getUTC();}
   void GuiSkinEditor::SlideBarRightPushedColor(  EditGuiSkin &e, C Str &t) {e.slidebar.right.pushed_color=TextVec4(t); e.base.slidebar.right.pushed_color.getUTC();}
   Str  GuiSkinEditor::SliderBackground(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slider.background_id);}
   void GuiSkinEditor::SliderBackground(  EditGuiSkin &e, C Str &t) {e.base.slider.background_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slider.background.getUTC();}
   void GuiSkinEditor::SliderBackgroundColor(  EditGuiSkin &e, C Str &t) {e.slider.background_color=TextVec4(t); e.base.slider.background_color.getUTC();}
   void GuiSkinEditor::SliderBackgroundShrink(  EditGuiSkin &e, C Str &t) {e.slider.background_shrink=TextFlt(t); e.base.slider.background_shrink.getUTC();}
   Str  GuiSkinEditor::SliderProgress(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slider.progress_id);}
   void GuiSkinEditor::SliderProgress(  EditGuiSkin &e, C Str &t) {e.base.slider.progress_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slider.progress.getUTC();}
   void GuiSkinEditor::SliderProgressColor(  EditGuiSkin &e, C Str &t) {e.slider.progress_color=TextVec4(t); e.base.slider.progress_color.getUTC();}
   void GuiSkinEditor::SliderProgressPart(  EditGuiSkin &e, C Str &t) {e.slider.draw_progress_partial=TextBool(t); e.base.slider.draw_progress_partial.getUTC();}
   Str  GuiSkinEditor::SliderSlider(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.slider.slider_id);}
   void GuiSkinEditor::SliderSlider(  EditGuiSkin &e, C Str &t) {e.base.slider.slider_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.slider.slider.getUTC();}
   void GuiSkinEditor::SliderSliderColor(  EditGuiSkin &e, C Str &t) {e.slider.slider_color=TextVec4(t); e.base.slider.slider_color.getUTC();}
   void GuiSkinEditor::TabNormalColor(  EditGuiSkin &e, C Str &t) {e.tab.left.normal_color=TextVec4(t); e.base.tab.normal_color.getUTC();}
   void GuiSkinEditor::TabPushedColor(  EditGuiSkin &e, C Str &t) {e.tab.left.pushed_color=TextVec4(t); e.base.tab.pushed_color.getUTC();}
   void GuiSkinEditor::TabTextSize(  EditGuiSkin &e, C Str &t) {e.tab.left.text_size=TextFlt(t); e.base.tab.text_size.getUTC();}
   void GuiSkinEditor::TabTextPadd(  EditGuiSkin &e, C Str &t) {e.tab.left.text_padd=TextFlt(t); e.base.tab.text_padd.getUTC();}
   Str  GuiSkinEditor::TabTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.text_style_id);}
   void GuiSkinEditor::TabTextStyle(  EditGuiSkin &e, C Str &t) {e.base.tab.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.tab.text_style.getUTC();}
   Str  GuiSkinEditor::TabLeftNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.left.normal_id);}
   void GuiSkinEditor::TabLeftNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.left.normal.getUTC();}
   Str  GuiSkinEditor::TabLeftPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.left.pushed_id);}
   void GuiSkinEditor::TabLeftPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.left.pushed.getUTC();}
   Str  GuiSkinEditor::TabHorizontalNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.horizontal.normal_id);}
   void GuiSkinEditor::TabHorizontalNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.horizontal.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.horizontal.normal.getUTC();}
   Str  GuiSkinEditor::TabHorizontalPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.horizontal.pushed_id);}
   void GuiSkinEditor::TabHorizontalPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.horizontal.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.horizontal.pushed.getUTC();}
   Str  GuiSkinEditor::TabRightNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.right.normal_id);}
   void GuiSkinEditor::TabRightNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.right.normal.getUTC();}
   Str  GuiSkinEditor::TabRightPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.right.pushed_id);}
   void GuiSkinEditor::TabRightPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.right.pushed.getUTC();}
   Str  GuiSkinEditor::TabTopNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top.normal_id);}
   void GuiSkinEditor::TabTopNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.top.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top.normal.getUTC();}
   Str  GuiSkinEditor::TabTopPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top.pushed_id);}
   void GuiSkinEditor::TabTopPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.top.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top.pushed.getUTC();}
   Str  GuiSkinEditor::TabVerticalNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.vertical.normal_id);}
   void GuiSkinEditor::TabVerticalNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.vertical.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.vertical.normal.getUTC();}
   Str  GuiSkinEditor::TabVerticalPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.vertical.pushed_id);}
   void GuiSkinEditor::TabVerticalPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.vertical.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.vertical.pushed.getUTC();}
   Str  GuiSkinEditor::TabBottomNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom.normal_id);}
   void GuiSkinEditor::TabBottomNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom.normal.getUTC();}
   Str  GuiSkinEditor::TabBottomPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom.pushed_id);}
   void GuiSkinEditor::TabBottomPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom.pushed.getUTC();}
   Str  GuiSkinEditor::TabTopLeftNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_left.normal_id);}
   void GuiSkinEditor::TabTopLeftNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.top_left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_left.normal.getUTC();}
   Str  GuiSkinEditor::TabTopLeftPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_left.pushed_id);}
   void GuiSkinEditor::TabTopLeftPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.top_left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_left.pushed.getUTC();}
   Str  GuiSkinEditor::TabTopRightNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_right.normal_id);}
   void GuiSkinEditor::TabTopRightNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.top_right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_right.normal.getUTC();}
   Str  GuiSkinEditor::TabTopRightPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.top_right.pushed_id);}
   void GuiSkinEditor::TabTopRightPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.top_right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.top_right.pushed.getUTC();}
   Str  GuiSkinEditor::TabBottomLeftNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_left.normal_id);}
   void GuiSkinEditor::TabBottomLeftNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_left.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_left.normal.getUTC();}
   Str  GuiSkinEditor::TabBottomLeftPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_left.pushed_id);}
   void GuiSkinEditor::TabBottomLeftPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_left.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_left.pushed.getUTC();}
   Str  GuiSkinEditor::TabBottomRightNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_right.normal_id);}
   void GuiSkinEditor::TabBottomRightNormal(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_right.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_right.normal.getUTC();}
   Str  GuiSkinEditor::TabBottomRightPushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.tab.bottom_right.pushed_id);}
   void GuiSkinEditor::TabBottomRightPushed(  EditGuiSkin &e, C Str &t) {e.base.tab.bottom_right.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.tab.bottom_right.pushed.getUTC();}
   Str  GuiSkinEditor::TextTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.text.text_style_id);}
   void GuiSkinEditor::TextTextStyle(  EditGuiSkin &e, C Str &t) {e.base.text.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.text.text_style.getUTC();}
   Str  GuiSkinEditor::TextLineNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.normal_id);}
   void GuiSkinEditor::TextLineNormal(  EditGuiSkin &e, C Str &t) {e.base.textline.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.textline.normal.getUTC();}
   void GuiSkinEditor::TextLineNormalPanelColor(  EditGuiSkin &e, C Str &t) {e.textline.normal_panel_color=TextVec4(t); e.base.textline.normal_panel_color.getUTC();}
   void GuiSkinEditor::TextLineNormalTextColor(  EditGuiSkin &e, C Str &t) {e.textline.normal_text_color =TextVec4(t); e.base.textline.normal_text_color .getUTC();}
   Str  GuiSkinEditor::TextLineDisabled(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.disabled_id);}
   void GuiSkinEditor::TextLineDisabled(  EditGuiSkin &e, C Str &t) {e.base.textline.disabled_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.textline.disabled.getUTC();}
   void GuiSkinEditor::TextLineDisabledPanelColor(  EditGuiSkin &e, C Str &t) {e.textline.disabled_panel_color=TextVec4(t); e.base.textline.disabled_panel_color.getUTC();}
   void GuiSkinEditor::TextLineDisabledTextColor(  EditGuiSkin &e, C Str &t) {e.textline.disabled_text_color =TextVec4(t); e.base.textline.disabled_text_color .getUTC();}
   void GuiSkinEditor::TextLineRectColor(  EditGuiSkin &e, C Str &t) {e.textline.rect_color=TextVec4(t); e.base.textline.rect_color.getUTC();}
   void GuiSkinEditor::TextLineTextSize(  EditGuiSkin &e, C Str &t) {e.textline.text_size=TextFlt(t); e.base.textline.text_size.getUTC();}
   Str  GuiSkinEditor::TextLineTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.text_style_id);}
   void GuiSkinEditor::TextLineTextStyle(  EditGuiSkin &e, C Str &t) {e.base.textline.text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.textline.text_style.getUTC();}
   Str  GuiSkinEditor::TextLineFindImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.find_image_id);}
   void GuiSkinEditor::TextLineFindImage(  EditGuiSkin &e, C Str &t) {e.base.textline.find_image_id=Proj.findElmImageID(t); e.base.textline.find_image.getUTC();}
   Str  GuiSkinEditor::TextLineClearImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.textline.clear.image_id);}
   void GuiSkinEditor::TextLineClearImage(  EditGuiSkin &e, C Str &t) {e.base.textline.clear.image_id=Proj.findElmImageID(t); e.base.textline.clear.image.getUTC();}
   Str  GuiSkinEditor::WindowNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.normal_id);}
   void GuiSkinEditor::WindowNormal(  EditGuiSkin &e, C Str &t) {e.base.window.normal_id=Proj.findElmID(t, ELM_PANEL); e.base.window.normal.getUTC();}
   void GuiSkinEditor::WindowNormalColor(  EditGuiSkin &e, C Str &t) {e.window.normal_color=TextVec4(t); e.base.window.normal_color.getUTC();}
   Str  GuiSkinEditor::WindowActive(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.active_id);}
   void GuiSkinEditor::WindowActive(  EditGuiSkin &e, C Str &t) {e.base.window.active_id=Proj.findElmID(t, ELM_PANEL); e.base.window.active.getUTC();}
   void GuiSkinEditor::WindowActiveColor(  EditGuiSkin &e, C Str &t) {e.window.active_color=TextVec4(t); e.base.window.active_color.getUTC();}
   Str  GuiSkinEditor::WindowNormalNoBar(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.normal_no_bar_id);}
   void GuiSkinEditor::WindowNormalNoBar(  EditGuiSkin &e, C Str &t) {e.base.window.normal_no_bar_id=Proj.findElmID(t, ELM_PANEL); e.base.window.normal_no_bar.getUTC();}
   Str  GuiSkinEditor::WindowActiveNoBar(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.active_no_bar_id);}
   void GuiSkinEditor::WindowActiveNoBar(  EditGuiSkin &e, C Str &t) {e.base.window.active_no_bar_id=Proj.findElmID(t, ELM_PANEL); e.base.window.active_no_bar.getUTC();}
   void GuiSkinEditor::WindowTextSize(  EditGuiSkin &e, C Str &t) {e.window.text_size=TextFlt(t); e.base.window.text_size.getUTC();}
   void GuiSkinEditor::WindowTextPadd(  EditGuiSkin &e, C Str &t) {e.window.text_padd=TextFlt(t); e.base.window.text_padd.getUTC();}
   Str  GuiSkinEditor::WindowNormalTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.normal_text_style_id);}
   void GuiSkinEditor::WindowNormalTextStyle(  EditGuiSkin &e, C Str &t) {e.base.window.normal_text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.window.normal_text_style.getUTC();}
   Str  GuiSkinEditor::WindowActiveTextStyle(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.active_text_style_id);}
   void GuiSkinEditor::WindowActiveTextStyle(  EditGuiSkin &e, C Str &t) {e.base.window.active_text_style_id=Proj.findElmID(t, ELM_TEXT_STYLE); e.base.window.active_text_style.getUTC();}
   Str  GuiSkinEditor::WindowMinimizeImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.minimize.image_id);}
   void GuiSkinEditor::WindowMinimizeImage(  EditGuiSkin &e, C Str &t) {e.base.window.minimize.image_id=Proj.findElmImageID(t); e.base.window.minimize.image.getUTC();}
   void GuiSkinEditor::WindowMinimizeImageColor(  EditGuiSkin &e, C Str &t) {e.window.minimize.image_color=TextVec4(t); e.base.window.minimize.image_color.getUTC();}
   Str  GuiSkinEditor::WindowMinimizeNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.minimize.normal_id);}
   void GuiSkinEditor::WindowMinimizeNormal(  EditGuiSkin &e, C Str &t) {e.base.window.minimize.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.minimize.normal.getUTC();}
   void GuiSkinEditor::WindowMinimizeNormalColor(  EditGuiSkin &e, C Str &t) {e.window.minimize.normal_color=TextVec4(t); e.base.window.minimize.normal_color.getUTC();}
   Str  GuiSkinEditor::WindowMinimizePushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.minimize.pushed_id);}
   void GuiSkinEditor::WindowMinimizePushed(  EditGuiSkin &e, C Str &t) {e.base.window.minimize.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.minimize.pushed.getUTC();}
   void GuiSkinEditor::WindowMinimizePushedColor(  EditGuiSkin &e, C Str &t) {e.window.minimize.pushed_color=TextVec4(t); e.base.window.minimize.pushed_color.getUTC();}
   void GuiSkinEditor::WindowMinimizeDisabledColor(  EditGuiSkin &e, C Str &t) {e.window.minimize.disabled_color=TextVec4(t); e.base.window.minimize.disabled_color.getUTC();}
   Str  GuiSkinEditor::WindowMaximizeImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.maximize.image_id);}
   void GuiSkinEditor::WindowMaximizeImage(  EditGuiSkin &e, C Str &t) {e.base.window.maximize.image_id=Proj.findElmImageID(t); e.base.window.maximize.image.getUTC();}
   void GuiSkinEditor::WindowMaximizeImageColor(  EditGuiSkin &e, C Str &t) {e.window.maximize.image_color=TextVec4(t); e.base.window.maximize.image_color.getUTC();}
   Str  GuiSkinEditor::WindowMaximizeNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.maximize.normal_id);}
   void GuiSkinEditor::WindowMaximizeNormal(  EditGuiSkin &e, C Str &t) {e.base.window.maximize.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.maximize.normal.getUTC();}
   void GuiSkinEditor::WindowMaximizeNormalColor(  EditGuiSkin &e, C Str &t) {e.window.maximize.normal_color=TextVec4(t); e.base.window.maximize.normal_color.getUTC();}
   Str  GuiSkinEditor::WindowMaximizePushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.maximize.pushed_id);}
   void GuiSkinEditor::WindowMaximizePushed(  EditGuiSkin &e, C Str &t) {e.base.window.maximize.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.maximize.pushed.getUTC();}
   void GuiSkinEditor::WindowMaximizePushedColor(  EditGuiSkin &e, C Str &t) {e.window.maximize.pushed_color=TextVec4(t); e.base.window.maximize.pushed_color.getUTC();}
   void GuiSkinEditor::WindowMaximizeDisabledColor(  EditGuiSkin &e, C Str &t) {e.window.maximize.disabled_color=TextVec4(t); e.base.window.maximize.disabled_color.getUTC();}
   Str  GuiSkinEditor::WindowCloseImage(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.close.image_id);}
   void GuiSkinEditor::WindowCloseImage(  EditGuiSkin &e, C Str &t) {e.base.window.close.image_id=Proj.findElmImageID(t); e.base.window.close.image.getUTC();}
   void GuiSkinEditor::WindowCloseImageColor(  EditGuiSkin &e, C Str &t) {e.window.close.image_color=TextVec4(t); e.base.window.close.image_color.getUTC();}
   Str  GuiSkinEditor::WindowCloseNormal(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.close.normal_id);}
   void GuiSkinEditor::WindowCloseNormal(  EditGuiSkin &e, C Str &t) {e.base.window.close.normal_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.close.normal.getUTC();}
   void GuiSkinEditor::WindowCloseNormalColor(  EditGuiSkin &e, C Str &t) {e.window.close.normal_color=TextVec4(t); e.base.window.close.normal_color.getUTC();}
   Str  GuiSkinEditor::WindowClosePushed(C EditGuiSkin &e          ) {return Proj.elmFullName(e.base.window.close.pushed_id);}
   void GuiSkinEditor::WindowClosePushed(  EditGuiSkin &e, C Str &t) {e.base.window.close.pushed_id=Proj.findElmID(t, ELM_PANEL_IMAGE); e.base.window.close.pushed.getUTC();}
   void GuiSkinEditor::WindowClosePushedColor(  EditGuiSkin &e, C Str &t) {e.window.close.pushed_color=TextVec4(t); e.base.window.close.pushed_color.getUTC();}
   void GuiSkinEditor::WindowCloseDisabledColor(  EditGuiSkin &e, C Str &t) {e.window.close.disabled_color=TextVec4(t); e.base.window.close.disabled_color.getUTC();}
   void GuiSkinEditor::WindowButtonOffset(EditGuiSkin &e, C Str &t) {e.window.button_offset=TextVec2(t); e.base.window.button_offset.getUTC();}
   void GuiSkinEditor::Undo(GuiSkinEditor &editor) {editor.undos.undo();}
   void GuiSkinEditor::Redo(GuiSkinEditor &editor) {editor.undos.redo();}
   void GuiSkinEditor::Locate(GuiSkinEditor &editor) {Proj.elmLocate(editor.elm_id);}
   bool GuiSkinEditor::selected()C {return Mode()==MODE_GUI_SKIN;}
   void GuiSkinEditor::selectedChanged() {}
                  C Rect& GuiSkinEditor::rect()C {return ::EE::Region::rect();}
   GuiSkinEditor& GuiSkinEditor::rect(C Rect &rect)
{
      ::EE::Region::rect(rect);
      props_region.rect(Rect(0.01f, -clientHeight()+0.01f, 0.01f+props_x, undo.rect().min.y-0.01f));
       objs_region.rect(Rect(props_region.rect().max.x, props_region.rect().min.y, clientWidth(), undo.rect().min.y-0.01f));
      flt s=0.12f;
      objs.window.pos(objs.tabs.rect().ld()+Vec2(0.15f, -s));
      objs.window_active.pos(objs.window.rect().ld()-Vec2(0, s));
      objs.window_menu_buttons.pos(objs.window_active.rect().ld()-Vec2(0, s));
      return T;
   }
   void GuiSkinEditor::resize() {rect(EditRect());}
   GuiSkinEditor& GuiSkinEditor::create(GuiObj &parent)
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
      props.New().create("Text Size"                  , MemberDesc(MEMBER(EditGuiSkin, button.text_size)).setTextToDataFunc(ButtonTextSize)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Text Padd"                  , MemberDesc(MEMBER(EditGuiSkin, button.text_padd)).setTextToDataFunc(ButtonTextPadd)).range(0, 1).mouseEditSpeed(0.2f);
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
      props.New().create("Text Size"                  , MemberDesc(MEMBER(EditGuiSkin, combobox.text_size)).setTextToDataFunc(ComboBoxTextSize)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Text Padd"                  , MemberDesc(MEMBER(EditGuiSkin, combobox.text_padd)).setTextToDataFunc(ComboBoxTextPadd)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Text Style"                 , MemberDesc(MEMBER(EditGuiSkin, base.combobox.text_style_id)).setFunc(ComboBoxTextStyle, ComboBoxTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Side Image"                 , MemberDesc(MEMBER(EditGuiSkin, base.combobox.image_id)).setFunc(ComboBoxSide, ComboBoxSide)).elmType(ELM_IMAGE);
      props.New().create("Side Color"                 , MemberDesc(MEMBER(EditGuiSkin, combobox.image_color)).setTextToDataFunc(ComboBoxSideColor)).setColor();
    //props.New().create("Side Color Add"             , MemberDesc(MEMBER(EditGuiSkin, combobox.image_color_add)).setTextToDataFunc(ComboBoxSideColorAdd)).setColor();
      props.New();
      props.New().create("Description:");
      props.New().create("Normal Panel", MemberDesc(MEMBER(EditGuiSkin, base.desc.normal_id)).setFunc(DescNormal, DescNormal)).elmType(ELM_PANEL);
      props.New().create("Normal Color", MemberDesc(MEMBER(EditGuiSkin, desc.normal_color)).setTextToDataFunc(DescNormalColor)).setColor();
      props.New().create("Text Style"  , MemberDesc(MEMBER(EditGuiSkin, base.desc.text_style_id)).setFunc(DescTextStyle, DescTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Padding"     , MemberDesc(MEMBER(EditGuiSkin, desc.padding)).setTextToDataFunc(DescPadding)).range(0, 0.1f).mouseEditSpeed(0.02f);
      props.New();
      props.New().create("Input Method Manager:");
      props.New().create("Normal Panel", MemberDesc(MEMBER(EditGuiSkin, base.imm.normal_id)).setFunc(IMMNormal, IMMNormal)).elmType(ELM_PANEL);
      props.New().create("Normal Color", MemberDesc(MEMBER(EditGuiSkin, imm.normal_color)).setTextToDataFunc(IMMNormalColor)).setColor();
      props.New().create("Text Style"  , MemberDesc(MEMBER(EditGuiSkin, base.imm.text_style_id)).setFunc(IMMTextStyle, IMMTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New().create("Padding"     , MemberDesc(MEMBER(EditGuiSkin, imm.padding)).setTextToDataFunc(IMMPadding)).range(0, 0.1f).mouseEditSpeed(0.02f);
      props.New();
      props.New().create("List:");
      props.New().create("Column Normal Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.list.column.normal_id)).setFunc(ListColumnNormal, ListColumnNormal)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Column Normal Color"      , MemberDesc(MEMBER(EditGuiSkin, list.column.normal_color)).setTextToDataFunc(ListColumnNormalColor)).setColor();
      props.New().create("Column Pushed Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.list.column.pushed_id)).setFunc(ListColumnPushed, ListColumnPushed)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Column Pushed Color"      , MemberDesc(MEMBER(EditGuiSkin, list.column.pushed_color)).setTextToDataFunc(ListColumnPushedColor)).setColor();
      props.New().create("Column Text Size"         , MemberDesc(MEMBER(EditGuiSkin, list.column.text_size)).setTextToDataFunc(ListColumnTextSize)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Column Text Padd"         , MemberDesc(MEMBER(EditGuiSkin, list.column.text_padd)).setTextToDataFunc(ListColumnTextPadd)).range(0, 1).mouseEditSpeed(0.2f);
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
      props.New().create("Padding"            , MemberDesc(MEMBER(EditGuiSkin, menu.padding)).setTextToDataFunc(MenuPadding)).range(0, 0.1f).mouseEditSpeed(0.02f);
      props.New().create("List Element Height", MemberDesc(MEMBER(EditGuiSkin, menu.list_elm_height)).setTextToDataFunc(MenuListElmHeight)).range(0, 1).mouseEditSpeed(0.02f);
      props.New();
      props.New().create("MenuBar:");
      props.New().create("Background Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.menubar.background_id)).setFunc(MenuBarBackground, MenuBarBackground)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Background Color"      , MemberDesc(MEMBER(EditGuiSkin, menubar.background_color)).setTextToDataFunc(MenuBarBackgroundColor)).setColor();
      props.New().create("Highlight Panel Image" , MemberDesc(MEMBER(EditGuiSkin, base.menubar.highlight_id)).setFunc(MenuBarHighlight, MenuBarHighlight)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Highlight Color"       , MemberDesc(MEMBER(EditGuiSkin, menubar.highlight_color)).setTextToDataFunc(MenuBarHighlightColor)).setColor();
      props.New().create("Bar Height"            , MemberDesc(MEMBER(EditGuiSkin, menubar.bar_height)).setTextToDataFunc(MenuBarBarHeight)).range(0, 1).mouseEditSpeed(0.05f);
      props.New().create("Text Size"             , MemberDesc(MEMBER(EditGuiSkin, menubar.text_size)).setTextToDataFunc(MenuBarTextSize)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Text Padd"             , MemberDesc(MEMBER(EditGuiSkin, menubar.text_padd)).setTextToDataFunc(MenuBarTextPadd)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Text Style"            , MemberDesc(MEMBER(EditGuiSkin, base.menubar.text_style_id)).setFunc(MenuBarTextStyle, MenuBarTextStyle)).elmType(ELM_TEXT_STYLE);
      props.New();
      props.New().create("Progress:");
      props.New().create("Background Panel Image", MemberDesc(MEMBER(EditGuiSkin, base.progress.background_id)).setFunc(ProgressBackground, ProgressBackground)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Background Color"      , MemberDesc(MEMBER(EditGuiSkin, progress.background_color)).setTextToDataFunc(ProgressBackgroundColor)).setColor();
      props.New().create("Progress Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.progress.progress_id)).setFunc(ProgressProgress, ProgressProgress)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Progress Color"        , MemberDesc(MEMBER(EditGuiSkin, progress.progress_color)).setTextToDataFunc(ProgressProgressColor)).setColor();
      props.New().create("Progress Partial"      , MemberDesc(MEMBER(EditGuiSkin, progress.draw_progress_partial)).setTextToDataFunc(ProgressProgressPart));
      props.New().create("Text Size"             , MemberDesc(MEMBER(EditGuiSkin, progress.text_size)).setTextToDataFunc(ProgressTextSize)).range(0, 1).mouseEditSpeed(0.2f);
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
      props.New().create("Background Shrink"     , MemberDesc(MEMBER(EditGuiSkin, slider.background_shrink)).setTextToDataFunc(SliderBackgroundShrink)).range(0, 0.5f);
      props.New().create("Progress Panel Image"  , MemberDesc(MEMBER(EditGuiSkin, base.slider.progress_id)).setFunc(SliderProgress, SliderProgress)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Progress Color"        , MemberDesc(MEMBER(EditGuiSkin, slider.progress_color)).setTextToDataFunc(SliderProgressColor)).setColor();
      props.New().create("Progress Partial"      , MemberDesc(MEMBER(EditGuiSkin, slider.draw_progress_partial)).setTextToDataFunc(SliderProgressPart));
      props.New().create("Slider Panel Image"    , MemberDesc(MEMBER(EditGuiSkin, base.slider.slider_id)).setFunc(SliderSlider, SliderSlider)).elmType(ELM_PANEL_IMAGE);
      props.New().create("Slider Color"          , MemberDesc(MEMBER(EditGuiSkin, slider.slider_color)).setTextToDataFunc(SliderSliderColor)).setColor();
      props.New();
      props.New().create("Tabs:");
      props.New().create("Normal Color", MemberDesc(MEMBER(EditGuiSkin, tab.left.normal_color)).setTextToDataFunc(TabNormalColor)).setColor();
      props.New().create("Pushed Color", MemberDesc(MEMBER(EditGuiSkin, tab.left.pushed_color)).setTextToDataFunc(TabPushedColor)).setColor();
      props.New().create("Text Size"   , MemberDesc(MEMBER(EditGuiSkin, tab.left.text_size)).setTextToDataFunc(TabTextSize)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Text Padd"   , MemberDesc(MEMBER(EditGuiSkin, tab.left.text_padd)).setTextToDataFunc(TabTextPadd)).range(0, 1).mouseEditSpeed(0.2f);
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
      props.New().create("Text Size"           , MemberDesc(MEMBER(EditGuiSkin, textline.text_size)).setTextToDataFunc(TextLineTextSize)).range(0, 1).mouseEditSpeed(0.2f);
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
      props.New().create("Text Size"                  , MemberDesc(MEMBER(EditGuiSkin, window.text_size)).setTextToDataFunc(WindowTextSize)).range(0, 1).mouseEditSpeed(0.2f);
      props.New().create("Text Padd"                  , MemberDesc(MEMBER(EditGuiSkin, window.text_padd)).setTextToDataFunc(WindowTextPadd)).range(0, 1).mouseEditSpeed(0.2f);
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
      props.New().create("Button Offset"              , MemberDesc(MEMBER(EditGuiSkin, window.button_offset)).setTextToDataFunc(WindowButtonOffset)).range(-1, 1).mouseEditSpeed(0.2f);

      REPAO(props).autoData(&edit).changed(Changed, PreChanged);

      parent+=::EE::Region::create().skin(&EmptyGuiSkin, false); T.kb_lit=false;
      T+=undo      .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo      .create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate    .create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      T+=props_region.create();
      T+= objs_region.create().removeSlideBars().skin(&EmptyGuiSkin, false); objs_region.kb_lit=false;
      props_ts.reset().size=0.036f; props_ts.align.set(1, 0);
      props_x=AddProperties(props, props_region, Vec2(0.01f, -0.01f), 0.044f, 0.5f, &props_ts).max.x+props_region.slidebarSize()+0.02f;
      objs.create(objs_region, Vec2(0.05f, -0.03f));
      removeSlideBars();
      return T;
   }
   void GuiSkinEditor::toGame() {if(game)edit.copyTo(*game, Proj); objs.setRect();}
   void GuiSkinEditor::toGui() {REPAO(props).toGui(); toGame();}
   void GuiSkinEditor::flush()
   {
      if(elm && game && changed)
      {
         if(ElmGuiSkin *data=elm->guiSkinData()){data->newVer(); data->from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(*elm)); // edit
         Save(*game, Proj.gamePath(*elm)); Proj.savedGame(*elm); // game
         Server.setElmLong(elm->id);
      }
      changed=false;
   }
   void GuiSkinEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmGuiSkin *data=elm->guiSkinData()){data->newVer(); data->from(edit);}
         toGame();
      }
   }
   void GuiSkinEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_GUI_SKIN)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm){edit.load(Proj.editPath(*elm)); game=Proj.gamePath(elm_id);}else{edit.reset(); game.clear(); REPAO(props).close();}
         objs.setSkin(game);
         toGui();
         Proj.refresh(false, false);
         Mode.tabAvailable(MODE_GUI_SKIN, elm!=null);
      }
   }
   void GuiSkinEditor::activate(Elm *elm) {set(elm); if(T.elm){Mode.set(MODE_GUI_SKIN); HideBig();}}
   void GuiSkinEditor::toggle(Elm *elm)
   {
      if(elm==T.elm && selected())elm=null;
      activate(elm);
   }
   void GuiSkinEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         undos.set(null, true);
         EditGuiSkin temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp))toGui();
      }
   }
   void GuiSkinEditor::draw()
   {
      D.clear(edit.background_color);
   }
   void GuiSkinEditor::draw(C GuiPC &gpc)
{
      ::EE::Region::draw(gpc);
      if(gpc.visible)
      {
         D.clip();
         GuiSkinPtr temp=Gui.skin; Gui.skin=game;
         DrawDescription(null, objs.checkbox_multi.screenRect().ru()+Vec2(0.06f, 0), u"Description", false);
         Vec2 pos(objs.slider_vertical.screenRect().centerX(), 
                     Avg(objs.textline.screenRect().centerY(), objs.textline_disabled.screenRect().centerY()));
         Rect_C(pos, 0.045f).draw(Gui.borderColor(), false);
         Gui.skin=temp;
      }
   }
   void GuiSkinEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
GuiSkinEditor::GuiSkinEditor() : elm_id(UIDZero), elm(null), changed(false), undos(true) {}

/******************************************************************************/
