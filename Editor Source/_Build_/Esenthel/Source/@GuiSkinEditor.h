/******************************************************************************/
/******************************************************************************/
class GuiSkinEditor : Region
{
   class Change : Edit::_Undo::Change
   {
      EditGuiSkin data;

    virtual void create(ptr user)override;
    virtual void apply(ptr user)override;
 };
 class Objs
 {
  class ButtonPushed   : Button   {virtual void update(C GuiPC &gpc)override;  };
  class CheckBoxOff    : CheckBox {virtual void update(C GuiPC &gpc)override;  };
  class CheckBoxOn     : CheckBox {virtual void update(C GuiPC &gpc)override;  };
  class CheckBoxMulti  : CheckBox {virtual void update(C GuiPC &gpc)override;};
  class ComboBoxPushed : ComboBox {virtual void draw  (C GuiPC &gpc)override;
   };
  class WindowActive : Window {virtual void draw(C GuiPC &gpc)override;
   };

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

      static cchar8 *combobox_normal_t[]
;
      static cchar8 *combobox_pushed_t[]
;
      static cchar8 *tabs_t[]
;
      class ListElm
      {
         cchar8 *t;
      };
      static ListElm list_t[]
;
      
      static void Reset(GuiObj &go); // force reset gui object rectangle
      void setRect();
      void setSkin(C GuiSkinPtr &skin);
      void create(GuiObj &parent, C Vec2 &position);
   };
   class SkinRegion : Region
   {
      virtual void update(C GuiPC &gpc)override;
      virtual void draw(C GuiPC &gpc)override;
   };

   UID               elm_id;
   Elm              *elm;
   bool              changed;
   EditGuiSkin       edit;
       GuiSkinPtr    game;
   Button            undo, redo, locate;
   Region            props_region;
   SkinRegion         objs_region;
   Memx<PropEx>      props;
   flt               props_x;
   TextBlack         props_ts;
   Objs              objs;
   Edit::Undo<Change> undos;   void undoVis();

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static Str  DefaultFont     (C EditGuiSkin &e          );
   static void DefaultFont     (  EditGuiSkin &e, C Str &t);
   static Str  DefaultTextStyle(C EditGuiSkin &e          );
   static void DefaultTextStyle(  EditGuiSkin &e, C Str &t);

   static void        BackgroundColor(EditGuiSkin &e, C Str &t);
   static void            BorderColor(EditGuiSkin &e, C Str &t);
   static void    MouseHighlightColor(EditGuiSkin &e, C Str &t);
   static void KeyboardHighlightColor(EditGuiSkin &e, C Str &t);

   static Str  ButtonNormal             (C EditGuiSkin &e          );
   static void ButtonNormal             (  EditGuiSkin &e, C Str &t);
   static void ButtonNormalColor        (  EditGuiSkin &e, C Str &t);
   static Str  ButtonPushed             (C EditGuiSkin &e          );
   static void ButtonPushed             (  EditGuiSkin &e, C Str &t);
   static void ButtonPushedColor        (  EditGuiSkin &e, C Str &t);
   static Str  ButtonDisabled           (C EditGuiSkin &e          );
   static void ButtonDisabled           (  EditGuiSkin &e, C Str &t);
   static void ButtonDisabledColor      (  EditGuiSkin &e, C Str &t);
   static Str  ButtonPushedDisabled     (C EditGuiSkin &e          );
   static void ButtonPushedDisabled     (  EditGuiSkin &e, C Str &t);
   static void ButtonPushedDisabledColor(  EditGuiSkin &e, C Str &t);
   static void ButtonTextSize           (  EditGuiSkin &e, C Str &t);
   static void ButtonTextPadd           (  EditGuiSkin &e, C Str &t);
   static Str  ButtonTextStyle          (C EditGuiSkin &e          );
   static void ButtonTextStyle          (  EditGuiSkin &e, C Str &t);

   static Str  CheckBoxOff          (C EditGuiSkin &e          );
   static void CheckBoxOff          (  EditGuiSkin &e, C Str &t);
   static Str  CheckBoxOn           (C EditGuiSkin &e          );
   static void CheckBoxOn           (  EditGuiSkin &e, C Str &t);
   static Str  CheckBoxMulti        (C EditGuiSkin &e          );
   static void CheckBoxMulti        (  EditGuiSkin &e, C Str &t);
   static void CheckBoxNormalColor  (  EditGuiSkin &e, C Str &t);
   static void CheckBoxDisabledColor(  EditGuiSkin &e, C Str &t);
 //static void CheckBoxRectColor    (  EditGuiSkin &e, C Str &t) {e.checkbox.rect_color=TextVec4(t); e.base.checkbox.rect_color.getUTC();}

   static Str  ComboBoxSide               (C EditGuiSkin &e          );
   static void ComboBoxSide               (  EditGuiSkin &e, C Str &t);
   static void ComboBoxSideColor          (  EditGuiSkin &e, C Str &t);
 //static void ComboBoxSideColorAdd       (  EditGuiSkin &e, C Str &t) {e.combobox.image_color_add=TextVec4(t); e.base.combobox.image_color_add.getUTC();}
   static Str  ComboBoxNormal             (C EditGuiSkin &e          );
   static void ComboBoxNormal             (  EditGuiSkin &e, C Str &t);
   static void ComboBoxNormalColor        (  EditGuiSkin &e, C Str &t);
   static Str  ComboBoxPushed             (C EditGuiSkin &e          );
   static void ComboBoxPushed             (  EditGuiSkin &e, C Str &t);
   static void ComboBoxPushedColor        (  EditGuiSkin &e, C Str &t);
   static Str  ComboBoxDisabled           (C EditGuiSkin &e          );
   static void ComboBoxDisabled           (  EditGuiSkin &e, C Str &t);
   static void ComboBoxDisabledColor      (  EditGuiSkin &e, C Str &t);
   static Str  ComboBoxPushedDisabled     (C EditGuiSkin &e          );
   static void ComboBoxPushedDisabled     (  EditGuiSkin &e, C Str &t);
   static void ComboBoxPushedDisabledColor(  EditGuiSkin &e, C Str &t);
   static void ComboBoxTextSize           (  EditGuiSkin &e, C Str &t);
   static void ComboBoxTextPadd           (  EditGuiSkin &e, C Str &t);
   static Str  ComboBoxTextStyle          (C EditGuiSkin &e          );
   static void ComboBoxTextStyle          (  EditGuiSkin &e, C Str &t);

   static Str  DescNormal     (C EditGuiSkin &e          );
   static void DescNormal     (  EditGuiSkin &e, C Str &t);
   static void DescNormalColor(  EditGuiSkin &e, C Str &t);
   static Str  DescTextStyle  (C EditGuiSkin &e          );
   static void DescTextStyle  (  EditGuiSkin &e, C Str &t);
   static void DescPadding    (  EditGuiSkin &e, C Str &t);

   static Str  IMMNormal     (C EditGuiSkin &e          );
   static void IMMNormal     (  EditGuiSkin &e, C Str &t);
   static void IMMNormalColor(  EditGuiSkin &e, C Str &t);
   static Str  IMMTextStyle  (C EditGuiSkin &e          );
   static void IMMTextStyle  (  EditGuiSkin &e, C Str &t);
   static void IMMPadding    (  EditGuiSkin &e, C Str &t);

   static Str  ListColumnNormal      (C EditGuiSkin &e          );
   static void ListColumnNormal      (  EditGuiSkin &e, C Str &t);
   static void ListColumnNormalColor (  EditGuiSkin &e, C Str &t);
   static Str  ListColumnPushed      (C EditGuiSkin &e          );
   static void ListColumnPushed      (  EditGuiSkin &e, C Str &t);
   static void ListColumnPushedColor (  EditGuiSkin &e, C Str &t);
   static void ListColumnTextSize    (  EditGuiSkin &e, C Str &t);
   static void ListColumnTextPadd    (  EditGuiSkin &e, C Str &t);
   static Str  ListColumnTextStyle   (C EditGuiSkin &e          );
   static void ListColumnTextStyle   (  EditGuiSkin &e, C Str &t);
   static Str  ListCursor            (C EditGuiSkin &e          );
   static void ListCursor            (  EditGuiSkin &e, C Str &t);
   static void ListCursorColor       (  EditGuiSkin &e, C Str &t);
   static Str  ListHighlight         (C EditGuiSkin &e          );
   static void ListHighlight         (  EditGuiSkin &e, C Str &t);
   static void ListHighlightColor    (  EditGuiSkin &e, C Str &t);
   static Str  ListSelection         (C EditGuiSkin &e          );
   static void ListSelection         (  EditGuiSkin &e, C Str &t);
   static void ListSelectionColor    (  EditGuiSkin &e, C Str &t);
   static Str  ListTextStyle         (C EditGuiSkin &e          );
   static void ListTextStyle         (  EditGuiSkin &e, C Str &t);
   static Str  ListResizeColumn      (C EditGuiSkin &e          );
   static void ListResizeColumn      (  EditGuiSkin &e, C Str &t);

   static Str  MenuNormal       (C EditGuiSkin &e          );
   static void MenuNormal       (  EditGuiSkin &e, C Str &t);
   static void MenuNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  MenuCheck        (C EditGuiSkin &e          );
   static void MenuCheck        (  EditGuiSkin &e, C Str &t);
   static void MenuCheckColor   (  EditGuiSkin &e, C Str &t);
   static Str  MenuSubMenu      (C EditGuiSkin &e          );
   static void MenuSubMenu      (  EditGuiSkin &e, C Str &t);
   static void MenuSubMenuColor (  EditGuiSkin &e, C Str &t);
   static void MenuPadding      (  EditGuiSkin &e, C Str &t);
   static void MenuListElmHeight(  EditGuiSkin &e, C Str &t);

   static Str  MenuBarBackground     (C EditGuiSkin &e          );
   static void MenuBarBackground     (  EditGuiSkin &e, C Str &t);
   static void MenuBarBackgroundColor(  EditGuiSkin &e, C Str &t);
   static Str  MenuBarHighlight      (C EditGuiSkin &e          );
   static void MenuBarHighlight      (  EditGuiSkin &e, C Str &t);
   static void MenuBarHighlightColor (  EditGuiSkin &e, C Str &t);
   static void MenuBarBarHeight      (  EditGuiSkin &e, C Str &t);
   static void MenuBarTextSize       (  EditGuiSkin &e, C Str &t);
   static void MenuBarTextPadd       (  EditGuiSkin &e, C Str &t);
   static Str  MenuBarTextStyle      (C EditGuiSkin &e          );
   static void MenuBarTextStyle      (  EditGuiSkin &e, C Str &t);

   static Str  ProgressBackground     (C EditGuiSkin &e          );
   static void ProgressBackground     (  EditGuiSkin &e, C Str &t);
   static void ProgressBackgroundColor(  EditGuiSkin &e, C Str &t);
   static Str  ProgressProgress       (C EditGuiSkin &e          );
   static void ProgressProgress       (  EditGuiSkin &e, C Str &t);
   static void ProgressProgressColor  (  EditGuiSkin &e, C Str &t);
   static void ProgressProgressPart   (  EditGuiSkin &e, C Str &t);
   static void ProgressTextSize       (  EditGuiSkin &e, C Str &t);
   static Str  ProgressTextStyle      (C EditGuiSkin &e          );
   static void ProgressTextStyle      (  EditGuiSkin &e, C Str &t);

   static Str  PropertyValueImage        (C EditGuiSkin &e          );
   static void PropertyValueImage        (  EditGuiSkin &e, C Str &t);
   static void PropertyValueImageColor   (  EditGuiSkin &e, C Str &t);
 //static void PropertyValueImageColorAdd(  EditGuiSkin &e, C Str &t) {e.property.value.image_color_add=TextVec4(t); e.base.property.value.image_color_add.getUTC();}
   static Str  PropertyValueNormal       (C EditGuiSkin &e          );
   static void PropertyValueNormal       (  EditGuiSkin &e, C Str &t);
   static void PropertyValueNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  PropertyValuePushed       (C EditGuiSkin &e          );
   static void PropertyValuePushed       (  EditGuiSkin &e, C Str &t);
   static void PropertyValuePushedColor  (  EditGuiSkin &e, C Str &t);

   static Str  RegionNormal           (C EditGuiSkin &e          );
   static void RegionNormal           (  EditGuiSkin &e, C Str &t);
   static void RegionNormalColor      (  EditGuiSkin &e, C Str &t);
   static Str  RegionViewImage        (C EditGuiSkin &e          );
   static void RegionViewImage        (  EditGuiSkin &e, C Str &t);
   static void RegionViewImageColor   (  EditGuiSkin &e, C Str &t);
 //static void RegionViewImageColorAdd(  EditGuiSkin &e, C Str &t) {e.region.view.image_color_add=TextVec4(t); e.base.region.view.image_color_add.getUTC();}
   static Str  RegionViewNormal       (C EditGuiSkin &e          );
   static void RegionViewNormal       (  EditGuiSkin &e, C Str &t);
   static void RegionViewNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  RegionViewPushed       (C EditGuiSkin &e          );
   static void RegionViewPushed       (  EditGuiSkin &e, C Str &t);
   static void RegionViewPushedColor  (  EditGuiSkin &e, C Str &t);

   static Str  SlideBarBackground     (C EditGuiSkin &e          );
   static void SlideBarBackground     (  EditGuiSkin &e, C Str &t);
   static void SlideBarBackgroundColor(  EditGuiSkin &e, C Str &t);

   static Str  SlideBarLeftImage        (C EditGuiSkin &e          );
   static void SlideBarLeftImage        (  EditGuiSkin &e, C Str &t);
   static void SlideBarLeftImageColor   (  EditGuiSkin &e, C Str &t);
 //static void SlideBarLeftImageColorAdd(  EditGuiSkin &e, C Str &t) {e.slidebar.left.image_color_add=TextVec4(t); e.base.slidebar.left.image_color_add.getUTC();}
   static Str  SlideBarLeftNormal       (C EditGuiSkin &e          );
   static void SlideBarLeftNormal       (  EditGuiSkin &e, C Str &t);
   static void SlideBarLeftNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  SlideBarLeftPushed       (C EditGuiSkin &e          );
   static void SlideBarLeftPushed       (  EditGuiSkin &e, C Str &t);
   static void SlideBarLeftPushedColor  (  EditGuiSkin &e, C Str &t);

   static Str  SlideBarCenterImage        (C EditGuiSkin &e          );
   static void SlideBarCenterImage        (  EditGuiSkin &e, C Str &t);
   static void SlideBarCenterImageColor   (  EditGuiSkin &e, C Str &t);
 //static void SlideBarCenterImageColorAdd(  EditGuiSkin &e, C Str &t) {e.slidebar.center.image_color_add=TextVec4(t); e.base.slidebar.center.image_color_add.getUTC();}
   static Str  SlideBarCenterNormal       (C EditGuiSkin &e          );
   static void SlideBarCenterNormal       (  EditGuiSkin &e, C Str &t);
   static void SlideBarCenterNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  SlideBarCenterPushed       (C EditGuiSkin &e          );
   static void SlideBarCenterPushed       (  EditGuiSkin &e, C Str &t);
   static void SlideBarCenterPushedColor  (  EditGuiSkin &e, C Str &t);

   static Str  SlideBarRightImage        (C EditGuiSkin &e          );
   static void SlideBarRightImage        (  EditGuiSkin &e, C Str &t);
   static void SlideBarRightImageColor   (  EditGuiSkin &e, C Str &t);
 //static void SlideBarRightImageColorAdd(  EditGuiSkin &e, C Str &t) {e.slidebar.right.image_color_add=TextVec4(t); e.base.slidebar.right.image_color_add.getUTC();}
   static Str  SlideBarRightNormal       (C EditGuiSkin &e          );
   static void SlideBarRightNormal       (  EditGuiSkin &e, C Str &t);
   static void SlideBarRightNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  SlideBarRightPushed       (C EditGuiSkin &e          );
   static void SlideBarRightPushed       (  EditGuiSkin &e, C Str &t);
   static void SlideBarRightPushedColor  (  EditGuiSkin &e, C Str &t);

   static Str  SliderBackground      (C EditGuiSkin &e          );
   static void SliderBackground      (  EditGuiSkin &e, C Str &t);
   static void SliderBackgroundColor (  EditGuiSkin &e, C Str &t);
   static void SliderBackgroundShrink(  EditGuiSkin &e, C Str &t);
   static Str  SliderProgress        (C EditGuiSkin &e          );
   static void SliderProgress        (  EditGuiSkin &e, C Str &t);
   static void SliderProgressColor   (  EditGuiSkin &e, C Str &t);
   static void SliderProgressPart    (  EditGuiSkin &e, C Str &t);
   static Str  SliderSlider          (C EditGuiSkin &e          );
   static void SliderSlider          (  EditGuiSkin &e, C Str &t);
   static void SliderSliderColor     (  EditGuiSkin &e, C Str &t);

   static void TabNormalColor(  EditGuiSkin &e, C Str &t);
   static void TabPushedColor(  EditGuiSkin &e, C Str &t);
   static void TabTextSize   (  EditGuiSkin &e, C Str &t);
   static void TabTextPadd   (  EditGuiSkin &e, C Str &t);
   static Str  TabTextStyle  (C EditGuiSkin &e          );
   static void TabTextStyle  (  EditGuiSkin &e, C Str &t);

   static Str  TabLeftNormal(C EditGuiSkin &e          );
   static void TabLeftNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabLeftPushed(C EditGuiSkin &e          );
   static void TabLeftPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabHorizontalNormal(C EditGuiSkin &e          );
   static void TabHorizontalNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabHorizontalPushed(C EditGuiSkin &e          );
   static void TabHorizontalPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabRightNormal(C EditGuiSkin &e          );
   static void TabRightNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabRightPushed(C EditGuiSkin &e          );
   static void TabRightPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabTopNormal(C EditGuiSkin &e          );
   static void TabTopNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabTopPushed(C EditGuiSkin &e          );
   static void TabTopPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabVerticalNormal(C EditGuiSkin &e          );
   static void TabVerticalNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabVerticalPushed(C EditGuiSkin &e          );
   static void TabVerticalPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabBottomNormal(C EditGuiSkin &e          );
   static void TabBottomNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabBottomPushed(C EditGuiSkin &e          );
   static void TabBottomPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabTopLeftNormal(C EditGuiSkin &e          );
   static void TabTopLeftNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabTopLeftPushed(C EditGuiSkin &e          );
   static void TabTopLeftPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabTopRightNormal(C EditGuiSkin &e          );
   static void TabTopRightNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabTopRightPushed(C EditGuiSkin &e          );
   static void TabTopRightPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabBottomLeftNormal(C EditGuiSkin &e          );
   static void TabBottomLeftNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabBottomLeftPushed(C EditGuiSkin &e          );
   static void TabBottomLeftPushed(  EditGuiSkin &e, C Str &t);

   static Str  TabBottomRightNormal(C EditGuiSkin &e          );
   static void TabBottomRightNormal(  EditGuiSkin &e, C Str &t);
   static Str  TabBottomRightPushed(C EditGuiSkin &e          );
   static void TabBottomRightPushed(  EditGuiSkin &e, C Str &t);

   static Str  TextTextStyle(C EditGuiSkin &e          );
   static void TextTextStyle(  EditGuiSkin &e, C Str &t);

   static Str  TextLineNormal            (C EditGuiSkin &e          );
   static void TextLineNormal            (  EditGuiSkin &e, C Str &t);
   static void TextLineNormalPanelColor  (  EditGuiSkin &e, C Str &t);
   static void TextLineNormalTextColor   (  EditGuiSkin &e, C Str &t);
   static Str  TextLineDisabled          (C EditGuiSkin &e          );
   static void TextLineDisabled          (  EditGuiSkin &e, C Str &t);
   static void TextLineDisabledPanelColor(  EditGuiSkin &e, C Str &t);
   static void TextLineDisabledTextColor (  EditGuiSkin &e, C Str &t);
   static void TextLineRectColor         (  EditGuiSkin &e, C Str &t);
   static void TextLineTextSize          (  EditGuiSkin &e, C Str &t);
   static Str  TextLineTextStyle         (C EditGuiSkin &e          );
   static void TextLineTextStyle         (  EditGuiSkin &e, C Str &t);
   static Str  TextLineFindImage         (C EditGuiSkin &e          );
   static void TextLineFindImage         (  EditGuiSkin &e, C Str &t);
   static Str  TextLineClearImage        (C EditGuiSkin &e          );
   static void TextLineClearImage        (  EditGuiSkin &e, C Str &t);

   static Str  WindowNormal     (C EditGuiSkin &e          );
   static void WindowNormal     (  EditGuiSkin &e, C Str &t);
   static void WindowNormalColor(  EditGuiSkin &e, C Str &t);
   static Str  WindowActive     (C EditGuiSkin &e          );
   static void WindowActive     (  EditGuiSkin &e, C Str &t);
   static void WindowActiveColor(  EditGuiSkin &e, C Str &t);
   static Str  WindowNormalNoBar(C EditGuiSkin &e          );
   static void WindowNormalNoBar(  EditGuiSkin &e, C Str &t);
   static Str  WindowActiveNoBar(C EditGuiSkin &e          );
   static void WindowActiveNoBar(  EditGuiSkin &e, C Str &t);

   static void WindowTextSize       (  EditGuiSkin &e, C Str &t);
   static void WindowTextPadd       (  EditGuiSkin &e, C Str &t);
   static Str  WindowNormalTextStyle(C EditGuiSkin &e          );
   static void WindowNormalTextStyle(  EditGuiSkin &e, C Str &t);
   static Str  WindowActiveTextStyle(C EditGuiSkin &e          );
   static void WindowActiveTextStyle(  EditGuiSkin &e, C Str &t);

   static Str  WindowMinimizeImage        (C EditGuiSkin &e          );
   static void WindowMinimizeImage        (  EditGuiSkin &e, C Str &t);
   static void WindowMinimizeImageColor   (  EditGuiSkin &e, C Str &t);
 //static void WindowMinimizeImageColorAdd(  EditGuiSkin &e, C Str &t) {e.window.minimize.image_color_add=TextVec4(t); e.base.window.minimize.image_color_add.getUTC();}
   static Str  WindowMinimizeNormal       (C EditGuiSkin &e          );
   static void WindowMinimizeNormal       (  EditGuiSkin &e, C Str &t);
   static void WindowMinimizeNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  WindowMinimizePushed       (C EditGuiSkin &e          );
   static void WindowMinimizePushed       (  EditGuiSkin &e, C Str &t);
   static void WindowMinimizePushedColor  (  EditGuiSkin &e, C Str &t);
   static void WindowMinimizeDisabledColor(  EditGuiSkin &e, C Str &t);

   static Str  WindowMaximizeImage        (C EditGuiSkin &e          );
   static void WindowMaximizeImage        (  EditGuiSkin &e, C Str &t);
   static void WindowMaximizeImageColor   (  EditGuiSkin &e, C Str &t);
 //static void WindowMaximizeImageColorAdd(  EditGuiSkin &e, C Str &t) {e.window.maximize.image_color_add=TextVec4(t); e.base.window.maximize.image_color_add.getUTC();}
   static Str  WindowMaximizeNormal       (C EditGuiSkin &e          );
   static void WindowMaximizeNormal       (  EditGuiSkin &e, C Str &t);
   static void WindowMaximizeNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  WindowMaximizePushed       (C EditGuiSkin &e          );
   static void WindowMaximizePushed       (  EditGuiSkin &e, C Str &t);
   static void WindowMaximizePushedColor  (  EditGuiSkin &e, C Str &t);
   static void WindowMaximizeDisabledColor(  EditGuiSkin &e, C Str &t);

   static Str  WindowCloseImage        (C EditGuiSkin &e          );
   static void WindowCloseImage        (  EditGuiSkin &e, C Str &t);
   static void WindowCloseImageColor   (  EditGuiSkin &e, C Str &t);
 //static void WindowCloseImageColorAdd(  EditGuiSkin &e, C Str &t) {e.window.close.image_color_add=TextVec4(t); e.base.window.close.image_color_add.getUTC();}
   static Str  WindowCloseNormal       (C EditGuiSkin &e          );
   static void WindowCloseNormal       (  EditGuiSkin &e, C Str &t);
   static void WindowCloseNormalColor  (  EditGuiSkin &e, C Str &t);
   static Str  WindowClosePushed       (C EditGuiSkin &e          );
   static void WindowClosePushed       (  EditGuiSkin &e, C Str &t);
   static void WindowClosePushedColor  (  EditGuiSkin &e, C Str &t);
   static void WindowCloseDisabledColor(  EditGuiSkin &e, C Str &t);

   static void WindowButtonOffset(EditGuiSkin &e, C Str &t);

   static void Undo  (GuiSkinEditor &editor);
   static void Redo  (GuiSkinEditor &editor);
   static void Locate(GuiSkinEditor &editor);

   bool selected()C;      
   void selectedChanged();

                  C Rect& rect()C;                  
   virtual GuiSkinEditor& rect(C Rect&rect)override;

   void resize();

   GuiSkinEditor&create(GuiObj&parent);
   void toGame();                    
   void toGui ();                    

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);
   void toggle  (Elm *elm);
   void elmChanged(C UID &elm_id);
   void draw();
   virtual void draw(C GuiPC &gpc)override;
   void erasing(C UID &elm_id);  

public:
   GuiSkinEditor();
};
/******************************************************************************/
/******************************************************************************/
extern GuiSkinEditor GuiSkinEdit;
/******************************************************************************/
