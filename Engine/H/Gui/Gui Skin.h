/******************************************************************************/
#if EE_PRIVATE
   #define DEFAULT_FONT_FROM_CUSTOM_SKIN 0
#endif
/******************************************************************************/
struct GuiSkin
{
   struct Button
   {
      Color         normal_color, pushed_color, disabled_color, pushed_disabled_color;
      Flt           text_size, text_padd;
      PanelImagePtr normal, pushed, disabled, pushed_disabled;
      TextStylePtr  text_style;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Button() {reset();} // reset to default values
   };

   STRUCT(ButtonImage , Button) // 'Button' with an 'Image' on top
   //{
      ImagePtr image;
      Color    image_color;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   ButtonImage() {reset();} // reset to default values
   };

   struct CheckBox
   {
      Color         normal_color, disabled_color;
      PanelImagePtr off, on, multi; // these could be 'ImagePtr' however 'PanelImagePtr' is used to allow padding (for example needed for shadow/glow)

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   CheckBox() {reset();} // reset to default values
   };

   struct Menu
   {
      Color    normal_color, check_color, sub_menu_color;
      Flt      padding, list_elm_height;
      PanelPtr normal;
      ImagePtr check, sub_menu;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Menu() {reset();} // reset to default values
   };

   STRUCT(ComboBox , ButtonImage) // 'image' is used as the side arrow
   //{
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   ComboBox() {reset();} // reset to default values
   };

   struct Desc
   {
      Color        normal_color;
      Flt          padding;
      PanelPtr     normal;
      TextStylePtr text_style;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Desc() {reset();} // reset to default values
   };

   struct IMM // Input Method Manager
   {
      Color        normal_color;
      Flt          padding;
      PanelPtr     normal;
      TextStylePtr text_style;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   IMM() {reset();} // reset to default values
   };

   struct List
   {
      Color         cursor_color, highlight_color, selection_color;
      PanelImagePtr cursor, highlight, selection;
      TextStylePtr  text_style;
      Button        column;
      ImagePtr      resize_column;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   List() {reset();} // reset to default values
   };

   struct MenuBar
   {
      Color         background_color, highlight_color;
      Flt           bar_height, text_size, text_padd;
      PanelImagePtr background, highlight;
      TextStylePtr  text_style;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   MenuBar() {reset();} // reset to default values
   };

   struct Progress
   {
      Bool          draw_progress_partial; // draw progress as partial, default=false
      Color         background_color, progress_color;
      Flt           text_size;
      PanelImagePtr background, progress;
      TextStylePtr  text_style;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Progress() {reset();} // reset to default values
   };

   struct Property
   {
      ButtonImage value;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Property() {reset();} // reset to default values
   };

   struct Region
   {
      Color       normal_color;
      PanelPtr    normal;
      ButtonImage view;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Region() {reset();} // reset to default values
   };

   struct SlideBar
   {
      Color         background_color;
      PanelImagePtr background;
      ButtonImage   left, center, right;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   SlideBar() {reset();} // reset to default values
   };

   struct Slider
   {
      Bool          draw_progress_partial; // draw progress as partial, default=false
      Color         background_color, progress_color, slider_color;
      Flt           background_shrink;
      PanelImagePtr background, progress, slider; // 'slider' could be 'ImagePtr' however 'PanelImagePtr' is used to allow padding (for example needed for shadow/glow)

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Slider() {reset();} // reset to default values
   };

   struct Tab
   {
      Button left, horizontal, right, top, vertical, bottom,
             top_left, top_right, bottom_left, bottom_right;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Tab() {reset();} // reset to default values
   };

   struct Text
   {
      TextStylePtr text_style;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Text() {reset();} // reset to default values
   };

   struct TextLine
   {
      Color         normal_panel_color, normal_text_color, disabled_panel_color, disabled_text_color, rect_color;
      Flt           text_size;
      PanelImagePtr normal, disabled;
      TextStylePtr  text_style;
      ImagePtr      find_image;
      ButtonImage   clear;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   TextLine() {reset();} // reset to default values
   };

   struct Window
   {
      Color        normal_color, active_color;
      Flt          text_size, text_padd;
      Vec2         button_offset;
      PanelPtr     normal, active, normal_no_bar, active_no_bar;
      TextStylePtr normal_text_style, active_text_style;
      ButtonImage  minimize, maximize, close;

      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      void reset();   Window() {reset();} // reset to default values
   };

   Color                background_color,
                            border_color, // affects 'GuiImage', 'Property', 'Viewport'
                   mouse_highlight_color, // affects 'Button', 'CheckBox', 'ComboBox',           'SlideBar', 'Slider', 'Tabs'
                keyboard_highlight_color; // affects 'Button', 'CheckBox', 'ComboBox', 'Region', 'SlideBar', 'Slider', 'TextBox', 'TextLine'
   FontPtr      font;
   TextStylePtr text_style;
   Button       button;
   CheckBox     checkbox;
   Menu         menu;
   ComboBox     combobox;
   Desc         desc;
   IMM          imm;
   List         list;
   MenuBar      menubar;
   Progress     progress;
   Property     property;
   Region       region;
   SlideBar     slidebar;
   Slider       slider;
   Tab          tab;
   Text         text;
   TextLine     textline;
   Window       window;

   // io
   Bool save(C Str &name)C; // save, false on fail
   Bool load(C Str &name) ; // load, false on fail

   Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   void reset();   GuiSkin() {reset();} // reset to default values
}extern
   EmptyGuiSkin;
/******************************************************************************/
DECLARE_CACHE(GuiSkin, GuiSkins, GuiSkinPtr); // 'GuiSkins' cache storing 'GuiSkin' objects which can be accessed by 'GuiSkinPtr' pointer
/******************************************************************************/
