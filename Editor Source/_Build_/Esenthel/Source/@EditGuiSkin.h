/******************************************************************************/
/******************************************************************************/
class EditGuiSkin : GuiSkin
{
   class Button
   {
      UID       normal_id, pushed_id, disabled_id, pushed_disabled_id, text_style_id;
      TimeStamp normal, pushed, disabled, pushed_disabled,
                normal_color, pushed_color, disabled_color, pushed_disabled_color,
                text_size, text_padd, text_style;

      void copyTo(GuiSkin::Button &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Button &src)C;
      bool equal(C Button &src)C;
      void newData();
      bool sync(GuiSkin::Button &val, C Button &src, C GuiSkin::Button &src_val);
      bool undo(GuiSkin::Button &val, C Button &src, C GuiSkin::Button &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Button();
   };

   class ButtonImage : Button
   {
      UID       image_id;
      TimeStamp image, image_color;

      void copyTo(GuiSkin::ButtonImage &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C ButtonImage &src)C;
      bool equal(C ButtonImage &src)C;
      void newData();
      bool sync(GuiSkin::ButtonImage &val, C ButtonImage &src, C GuiSkin::ButtonImage &src_val);
      bool undo(GuiSkin::ButtonImage &val, C ButtonImage &src, C GuiSkin::ButtonImage &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   ButtonImage();
   };

   class CheckBox
   {
      UID       off_id, on_id, multi_id;
      TimeStamp off, on, multi,
                normal_color, disabled_color;

      void copyTo(GuiSkin::CheckBox &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C CheckBox &src)C;
      bool equal(C CheckBox &src)C;
      void newData();
      bool sync(GuiSkin::CheckBox &val, C CheckBox &src, C GuiSkin::CheckBox &src_val);
      bool undo(GuiSkin::CheckBox &val, C CheckBox &src, C GuiSkin::CheckBox &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   CheckBox();
   };

   class ComboBox : ButtonImage // 'image' is used as the side arrow
   {
      void copyTo(GuiSkin::ComboBox &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C ComboBox &src)C;
      bool equal(C ComboBox &src)C;
      void newData();
      bool sync(GuiSkin::ComboBox &val, C ComboBox &src, C GuiSkin::ComboBox &src_val);
      bool undo(GuiSkin::ComboBox &val, C ComboBox &src, C GuiSkin::ComboBox &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    
   };

   class Desc
   {
      UID       normal_id, text_style_id;
      TimeStamp normal,
                normal_color,
                padding,
                text_style;

      void copyTo(GuiSkin::Desc &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Desc &src)C;
      bool equal(C Desc &src)C;
      void newData();
      bool sync(GuiSkin::Desc &val, C Desc &src, C GuiSkin::Desc &src_val);
      bool undo(GuiSkin::Desc &val, C Desc &src, C GuiSkin::Desc &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Desc();
   };

   class IMM
   {
      UID       normal_id, text_style_id;
      TimeStamp normal,
                normal_color,
                padding,
                text_style;

      void copyTo(GuiSkin::IMM &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C IMM &src)C;
      bool equal(C IMM &src)C;
      void newData();
      bool sync(GuiSkin::IMM &val, C IMM &src, C GuiSkin::IMM &src_val);
      bool undo(GuiSkin::IMM &val, C IMM &src, C GuiSkin::IMM &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   IMM();
   };

   class List
   {
      UID       cursor_id, highlight_id, selection_id, text_style_id, resize_column_id;
      TimeStamp cursor, highlight, selection,
                cursor_color, highlight_color, selection_color,
                text_style, resize_column;
      Button    column;

      void copyTo(GuiSkin::List &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C List &src)C;
      bool equal(C List &src)C;
      void newData();
      bool sync(GuiSkin::List &val, C List &src, C GuiSkin::List &src_val);
      bool undo(GuiSkin::List &val, C List &src, C GuiSkin::List &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   List();
   };

   class Menu
   {
      UID       normal_id, check_id, sub_menu_id;
      TimeStamp normal,
                normal_color,
                padding,
                list_elm_height,
                check, sub_menu,
                check_color, sub_menu_color;

      void copyTo(GuiSkin::Menu &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Menu &src)C;
      bool equal(C Menu &src)C;
      void newData();
      bool sync(GuiSkin::Menu &val, C Menu &src, C GuiSkin::Menu &src_val);
      bool undo(GuiSkin::Menu &val, C Menu &src, C GuiSkin::Menu &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Menu();
   };

   class MenuBar
   {
      UID       background_id, highlight_id, text_style_id;
      TimeStamp background, highlight,
                background_color, highlight_color,
                bar_height, text_size, text_padd,
                text_style;

      void copyTo(GuiSkin::MenuBar &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C MenuBar &src)C;
      bool equal(C MenuBar &src)C;
      void newData();
      bool sync(GuiSkin::MenuBar &val, C MenuBar &src, C GuiSkin::MenuBar &src_val);
      bool undo(GuiSkin::MenuBar &val, C MenuBar &src, C GuiSkin::MenuBar &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   MenuBar();
   };

   class Progress
   {
      UID       background_id, progress_id, text_style_id;
      TimeStamp background, progress,
                background_color, progress_color,
                text_style, text_size,
                draw_progress_partial;

      void copyTo(GuiSkin::Progress &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Progress &src)C;
      bool equal(C Progress &src)C;
      void newData();
      bool sync(GuiSkin::Progress &val, C Progress &src, C GuiSkin::Progress &src_val);
      bool undo(GuiSkin::Progress &val, C Progress &src, C GuiSkin::Progress &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Progress();
   };

   class Property
   {
      ButtonImage value;

      void copyTo(GuiSkin::Property &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Property &src)C;
      bool equal(C Property &src)C;
      void newData();
      bool sync(GuiSkin::Property &val, C Property &src, C GuiSkin::Property &src_val);
      bool undo(GuiSkin::Property &val, C Property &src, C GuiSkin::Property &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    
   };

   class Region
   {
      UID         normal_id;
      TimeStamp   normal,
                  normal_color;
      ButtonImage view;

      void copyTo(GuiSkin::Region &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Region &src)C;
      bool equal(C Region &src)C;
      void newData();
      bool sync(GuiSkin::Region &val, C Region &src, C GuiSkin::Region &src_val);
      bool undo(GuiSkin::Region &val, C Region &src, C GuiSkin::Region &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Region();
   };

   class SlideBar
   {
      UID         background_id;
      TimeStamp   background, background_color;
      ButtonImage left, center, right;

      void copyTo(GuiSkin::SlideBar &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C SlideBar &src)C;
      bool equal(C SlideBar &src)C;
      void newData();
      bool sync(GuiSkin::SlideBar &val, C SlideBar &src, C GuiSkin::SlideBar &src_val);
      bool undo(GuiSkin::SlideBar &val, C SlideBar &src, C GuiSkin::SlideBar &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   SlideBar();
   };

   class Slider
   {
      UID       background_id, progress_id, slider_id;
      TimeStamp background, progress, slider,
                background_color, progress_color, slider_color,
                background_shrink,
                draw_progress_partial;

      void copyTo(GuiSkin::Slider &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Slider &src)C;
      bool equal(C Slider &src)C;
      void newData();
      bool sync(GuiSkin::Slider &val, C Slider &src, C GuiSkin::Slider &src_val);
      bool undo(GuiSkin::Slider &val, C Slider &src, C GuiSkin::Slider &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Slider();
   };

   class Tab : Button
   {
      class Kind
      {
         UID       normal_id, pushed_id;
         TimeStamp normal, pushed;

         Kind(C Button &button);      
         bool valid()C;               
         void copyTo(Button &button)C;
         bool apply (Kind   &kind  )C;     
         bool apply (Button &button)C;     
         void includeIDs(MemPtr<UID> ids)C;
         bool newer(C Kind &src)C;      
         bool equal(C Kind &src)C;      
         void newData();                
         bool sync(C Kind &src);
         bool undo(C Kind &src);
         bool save(File &f)C;
         bool load(File &f);
         void reset();    

public:
   Kind();
      };

      Kind left, horizontal, right, top, vertical, bottom, top_left, top_right, bottom_left, bottom_right;

      void copyTo(GuiSkin::Tab &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Tab &src)C;
      bool equal(C Tab &src)C;
      void newData();
      bool sync(GuiSkin::Tab &val, C Tab &src, C GuiSkin::Tab &src_val);
      bool undo(GuiSkin::Tab &val, C Tab &src, C GuiSkin::Tab &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    
   };

   class Text
   {
      UID       text_style_id;
      TimeStamp text_style;

      void copyTo(GuiSkin::Text &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Text &src)C;
      bool equal(C Text &src)C;
      void newData();
      bool sync(GuiSkin::Text &val, C Text &src, C GuiSkin::Text &src_val);
      bool undo(GuiSkin::Text &val, C Text &src, C GuiSkin::Text &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Text();
   };

   class TextLine
   {
      UID         normal_id, disabled_id, text_style_id, find_image_id;
      TimeStamp   normal, disabled,
                  normal_panel_color, normal_text_color, disabled_panel_color, disabled_text_color, rect_color,
                  text_size,
                  text_style, 
                  find_image;
      ButtonImage clear;

      void copyTo(GuiSkin::TextLine &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C TextLine &src)C;
      bool equal(C TextLine &src)C;
      void newData();
      bool sync(GuiSkin::TextLine &val, C TextLine &src, C GuiSkin::TextLine &src_val);
      bool undo(GuiSkin::TextLine &val, C TextLine &src, C GuiSkin::TextLine &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   TextLine();
   };

   class Window
   {
      UID         normal_id, active_id, normal_no_bar_id, active_no_bar_id,
                  normal_text_style_id, active_text_style_id;
      TimeStamp   normal, active, normal_no_bar, active_no_bar,
                  normal_color, active_color,
                  normal_text_style, active_text_style,
                  text_size, text_padd, button_offset;
      ButtonImage minimize, maximize, close;

      void copyTo(GuiSkin::Window &skin, C Project &proj)C;
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Window &src)C;
      bool equal(C Window &src)C;
      void newData();
      bool sync(GuiSkin::Window &val, C Window &src, C GuiSkin::Window &src_val);
      bool undo(GuiSkin::Window &val, C Window &src, C GuiSkin::Window &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Window();
   };

   class Base
   {
      UID       font_id, text_style_id;
      TimeStamp         background_color, 
                            border_color, 
                   mouse_highlight_color,
                keyboard_highlight_color,
                font, text_style;
      Button    button;
      CheckBox  checkbox;
      ComboBox  combobox;
      Desc      desc;
      IMM       imm;
      List      list;
      Menu      menu;
      MenuBar   menubar;
      Progress  progress;
      Property  property;
      Region    region;
      SlideBar  slidebar;
      Slider    slider;
      Tab       tab;
      Text      text;
      TextLine  textline;
      Window    window;
      
      void includeIDs(MemPtr<UID> ids)C;
      void copyTo(GuiSkin &skin, C Project &proj)C;
      bool newer(C Base &src)C;
      bool equal(C Base &src)C;
      void newData();
      bool sync(GuiSkin &val, C Base &src, C GuiSkin &src_val);
      bool undo(GuiSkin &val, C Base &src, C GuiSkin &src_val);
      bool save(File &f)C;
      bool load(File &f);
      void reset();    

public:
   Base();
   };

   Base base;

   void reset  (                  );                  
   void newData(                  );                  
   bool newer  (C EditGuiSkin &src)C;                 
   bool equal  (C EditGuiSkin &src)C;                 
   bool sync   (C EditGuiSkin &src);                  
   bool undo   (C EditGuiSkin &src);                  
   void copyTo (      GuiSkin &skin, C Project&proj)C; // call 'copyTo' after assignment to fixup what's missing

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
