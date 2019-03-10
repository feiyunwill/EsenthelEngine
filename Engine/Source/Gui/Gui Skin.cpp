/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_GSKN CC4('G','S','K','N')
/******************************************************************************/
GuiSkin EmptyGuiSkin;
DEFINE_CACHE(GuiSkin, GuiSkins, GuiSkinPtr, "Gui Skin");
/******************************************************************************/
void GuiSkin::Button::reset()
{
          disabled_color=normal_color.set(255);        disabled_color.a/=2;
   pushed_disabled_color=pushed_color.set(192); pushed_disabled_color.a/=2;
   normal=pushed=disabled=pushed_disabled=null;
   text_size=0.85f; text_padd=0.15f; text_style=null;
}
Bool GuiSkin::Button::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<normal_color<<pushed_color<<disabled_color<<pushed_disabled_color<<text_size<<text_padd;
   f._putAsset(normal         .name(path));
   f._putAsset(pushed         .name(path));
   f._putAsset(disabled       .name(path));
   f._putAsset(pushed_disabled.name(path));
   f._putAsset(text_style     .name(path));
   return f.ok();
}
Bool GuiSkin::Button::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>normal_color>>pushed_color>>disabled_color>>pushed_disabled_color>>text_size>>text_padd;
         normal         .require(f._getAsset(), path);
         pushed         .require(f._getAsset(), path);
         disabled       .require(f._getAsset(), path);
         pushed_disabled.require(f._getAsset(), path);
         text_style     .require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::ButtonImage::reset()
{
   super::reset();
   image_color.set(255);
   image=null;
}
Bool GuiSkin::ButtonImage::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   super::save(f, path);
   f<<image_color;
   f._putAsset(image.name(path));
   return f.ok();
}
Bool GuiSkin::ButtonImage::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0: if(super::load(f, path))
      {
         f>>image_color;
         image.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::CheckBox::reset()
{
   disabled_color=normal_color.set(255); disabled_color.a/=2;
   off=on=multi=null;
}
Bool GuiSkin::CheckBox::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<normal_color<<disabled_color;
   f._putAsset(off  .name(path));
   f._putAsset(on   .name(path));
   f._putAsset(multi.name(path));
   return f.ok();
}
Bool GuiSkin::CheckBox::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>normal_color>>disabled_color;
         off  .require(f._getAsset(), path);
         on   .require(f._getAsset(), path);
         multi.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::ComboBox::reset()
{
   super::reset();
}
Bool GuiSkin::ComboBox::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   if(super::save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::ComboBox::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0: if(super::load(f, path))
      {
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Desc::reset()
{
   normal_color.set(255);
   padding=0.01f;
   normal=null;
   text_style=null;
}
Bool GuiSkin::Desc::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<normal_color<<padding;
   f._putAsset(text_style.name(path));
   f._putAsset(normal    .name(path));
   return f.ok();
}
Bool GuiSkin::Desc::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>normal_color>>padding;
         text_style.require(f._getAsset(), path);
         normal    .require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::IMM::reset()
{
   normal_color.set(255);
   padding=0.01f;
   normal=null;
   text_style=null;
}
Bool GuiSkin::IMM::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<normal_color<<padding;
   f._putAsset(text_style.name(path));
   f._putAsset(normal    .name(path));
   return f.ok();
}
Bool GuiSkin::IMM::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>normal_color>>padding;
         text_style.require(f._getAsset(), path);
         normal    .require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::List::reset()
{
   selection_color=   cursor_color.set(0, 108, 255, 56);
   highlight_color=selection_color; highlight_color.a=29;
   cursor=highlight=selection=null;
   text_style=null;
   column.reset();
   resize_column=null;
}
Bool GuiSkin::List::save(File &f, CChar *path)C
{
   f.cmpUIntV(1); // version
   f<<cursor_color<<highlight_color<<selection_color;
   f.putAsset(cursor       .id());
   f.putAsset(highlight    .id());
   f.putAsset(selection    .id());
   f.putAsset(text_style   .id());
   f.putAsset(resize_column.id());
   if(column.save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::List::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 1:
      {
         f>>cursor_color>>highlight_color>>selection_color;
         cursor       .require(f.getAssetID(), path);
         highlight    .require(f.getAssetID(), path);
         selection    .require(f.getAssetID(), path);
         text_style   .require(f.getAssetID(), path);
         resize_column.require(f.getAssetID(), path);
         if(column.load(f, path))
            if(f.ok())return true;
      }break;

      case 0:
      {
         f>>cursor_color>>highlight_color>>selection_color;
         cursor    .require(f._getAsset(), path);
         highlight .require(f._getAsset(), path);
         selection .require(f._getAsset(), path);
         text_style.require(f._getAsset(), path);
         resize_column=null;
         if(column.load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Menu::reset()
{
   normal_color=check_color=sub_menu_color.set(255);
   padding=0.01f;
   list_elm_height=0.043f;
   normal=null;
   check=sub_menu=null;
}
Bool GuiSkin::Menu::save(File &f, CChar *path)C
{
   f.cmpUIntV(1); // version
   f<<normal_color<<check_color<<sub_menu_color<<padding<<list_elm_height;
   f.putAsset(normal  .id());
   f.putAsset(check   .id());
   f.putAsset(sub_menu.id());
   return f.ok();
}
Bool GuiSkin::Menu::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 1:
      {
         f>>normal_color>>check_color>>sub_menu_color>>padding>>list_elm_height;
         normal  .require(f.getAssetID(), path);
         check   .require(f.getAssetID(), path);
         sub_menu.require(f.getAssetID(), path);
         if(f.ok())return true;
      }break;

      case 0:
      {
         f>>normal_color>>check_color>>sub_menu_color>>padding; list_elm_height=0.043f;
         normal  .require(f._getAsset(), path);
         check   .require(f._getAsset(), path);
         sub_menu.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::MenuBar::reset()
{
   background_color.set(255);
    highlight_color.set(0, 108, 255, 64);
   bar_height=0.055f; text_size=0.9f; text_padd=0.8f;
   background=highlight=null;
   text_style=null;
}
Bool GuiSkin::MenuBar::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<background_color<<highlight_color<<bar_height<<text_size<<text_padd;
   f._putAsset(background.name(path));
   f._putAsset(highlight .name(path));
   f._putAsset(text_style.name(path));
   return f.ok();
}
Bool GuiSkin::MenuBar::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>background_color>>highlight_color>>bar_height>>text_size>>text_padd;
         background.require(f._getAsset(), path);
         highlight .require(f._getAsset(), path);
         text_style.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Progress::reset()
{
   draw_progress_partial=false;
   background_color=progress_color.set(255);
   text_size=0.8f;
   background=progress=null;
   text_style=null;
}
Bool GuiSkin::Progress::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<draw_progress_partial<<background_color<<progress_color<<text_size;
   f._putAsset(background.name(path));
   f._putAsset(progress  .name(path));
   f._putAsset(text_style.name(path));
   return f.ok();
}
Bool GuiSkin::Progress::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>draw_progress_partial>>background_color>>progress_color>>text_size;
         background.require(f._getAsset(), path);
         progress  .require(f._getAsset(), path);
         text_style.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Property::reset()
{
   value.reset();
}
Bool GuiSkin::Property::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   if(value.save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::Property::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         if(value.load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Region::reset()
{
   normal_color.set(255);
   normal=null;
   view.reset();
}
Bool GuiSkin::Region::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<normal_color;
   f._putAsset(normal.name(path));
   if(view.save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::Region::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>normal_color;
         normal.require(f._getAsset(), path);
         if(view.load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::SlideBar::reset()
{
   background_color.set(255);
   background=null;
   left.reset(); center.reset(); right.reset();
}
Bool GuiSkin::SlideBar::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<background_color;
   f._putAsset(background.name(path));
   if(left  .save(f, path))
   if(center.save(f, path))
   if(right .save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::SlideBar::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>background_color;
         background.require(f._getAsset(), path);
         if(left  .load(f, path))
         if(center.load(f, path))
         if(right .load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Slider::reset()
{
   draw_progress_partial=false;
   background_color=progress_color=slider_color.set(255);
   background_shrink=0;
   background=progress=slider=null;
}
Bool GuiSkin::Slider::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<draw_progress_partial<<background_color<<progress_color<<slider_color<<background_shrink;
   f._putAsset(background.name(path));
   f._putAsset(progress  .name(path));
   f._putAsset(slider    .name(path));
   return f.ok();
}
Bool GuiSkin::Slider::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>draw_progress_partial>>background_color>>progress_color>>slider_color>>background_shrink;
         background.require(f._getAsset(), path);
         progress  .require(f._getAsset(), path);
         slider    .require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Tab::reset()
{
   left.reset(); horizontal.reset(); right.reset(); top.reset(); vertical.reset(); bottom.reset();
   top_left.reset(); top_right.reset(); bottom_left.reset(); bottom_right.reset();
}
Bool GuiSkin::Tab::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   if(left        .save(f, path))
   if(horizontal  .save(f, path))
   if(right       .save(f, path))
   if(top         .save(f, path))
   if(vertical    .save(f, path))
   if(bottom      .save(f, path))
   if(top_left    .save(f, path))
   if(top_right   .save(f, path))
   if(bottom_left .save(f, path))
   if(bottom_right.save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::Tab::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         if(left        .load(f, path))
         if(horizontal  .load(f, path))
         if(right       .load(f, path))
         if(top         .load(f, path))
         if(vertical    .load(f, path))
         if(bottom      .load(f, path))
         if(top_left    .load(f, path))
         if(top_right   .load(f, path))
         if(bottom_left .load(f, path))
         if(bottom_right.load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Text::reset()
{
   text_style=null;
}
Bool GuiSkin::Text::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f._putAsset(text_style.name(path));
   return f.ok();
}
Bool GuiSkin::Text::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         text_style.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::TextLine::reset()
{
   normal_panel_color=normal_text_color=disabled_panel_color=disabled_text_color.set(255); disabled_panel_color.a/=2; disabled_text_color.a/=2;
   rect_color.set(0, 112);
   text_size=0.8f;
   normal=disabled=null;
   text_style=null;
   find_image=null;
   clear.reset();
}
Bool GuiSkin::TextLine::save(File &f, CChar *path)C
{
   f.cmpUIntV(3); // version
   f<<normal_panel_color<<normal_text_color<<disabled_panel_color<<disabled_text_color<<rect_color<<text_size;
   f.putAsset(normal    .id());
   f.putAsset(disabled  .id());
   f.putAsset(text_style.id());
   f.putAsset(find_image.id());
   if(clear.save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::TextLine::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 3:
      {
         f>>normal_panel_color>>normal_text_color>>disabled_panel_color>>disabled_text_color>>rect_color>>text_size;
         normal    .require(f.getAssetID(), path);
         disabled  .require(f.getAssetID(), path);
         text_style.require(f.getAssetID(), path);
         find_image.require(f.getAssetID(), path);
         if(clear.load(f, path))
            if(f.ok())return true;
      }break;

      case 2:
      {
         f>>normal_panel_color>>normal_text_color>>disabled_panel_color>>disabled_text_color>>rect_color; text_size=0.8f;
         normal    .require(f.getAssetID(), path);
         disabled  .require(f.getAssetID(), path);
         text_style.require(f.getAssetID(), path);
         find_image.require(f.getAssetID(), path);
         if(clear.load(f, path))
            if(f.ok())return true;
      }break;

      case 1:
      {
         f>>normal_panel_color>>normal_text_color>>disabled_panel_color>>disabled_text_color>>rect_color; text_size=0.8f;
         normal    .require(f.getAssetID(), path);
         disabled  .require(f.getAssetID(), path);
         text_style.require(f.getAssetID(), path);
         find_image.require(f.getAssetID(), path);
         clear.reset();
         if(f.ok())return true;
      }break;

      case 0:
      {
         f>>normal_panel_color>>disabled_panel_color>>rect_color; normal_text_color=normal_panel_color; disabled_text_color=disabled_panel_color; text_size=0.8f;
         normal    .require(f._getAsset(), path);
         disabled  .require(f._getAsset(), path);
         text_style.require(f._getAsset(), path);
         find_image.require(f._getAsset(), path);
         clear.reset();
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::Window::reset()
{
   normal_color=active_color.set(255);
   text_size=0.87f; text_padd=0.15f;
   button_offset.zero();
   normal=active=normal_no_bar=active_no_bar=null;
   normal_text_style=active_text_style=null;
   minimize.reset();
   maximize.reset();
   close   .reset();
}
Bool GuiSkin::Window::save(File &f, CChar *path)C
{
   f.cmpUIntV(1); // version
   f<<normal_color<<active_color<<text_size<<text_padd<<button_offset;
   f.putAsset(normal           .id());
   f.putAsset(active           .id());
   f.putAsset(normal_no_bar    .id());
   f.putAsset(active_no_bar    .id());
   f.putAsset(normal_text_style.id());
   f.putAsset(active_text_style.id());
   if(minimize.save(f, path))
   if(maximize.save(f, path))
   if(close   .save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::Window::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 1:
      {
         f>>normal_color>>active_color>>text_size>>text_padd>>button_offset;
         normal           .require(f.getAssetID(), path);
         active           .require(f.getAssetID(), path);
         normal_no_bar    .require(f.getAssetID(), path);
         active_no_bar    .require(f.getAssetID(), path);
         normal_text_style.require(f.getAssetID(), path);
         active_text_style.require(f.getAssetID(), path);
         if(minimize.load(f, path))
         if(maximize.load(f, path))
         if(close   .load(f, path))
            if(f.ok())return true;
      }break;

      case 0:
      {
         f>>normal_color>>active_color>>text_size>>text_padd; button_offset.zero();
         normal           .require(f._getAsset(), path);
         active           .require(f._getAsset(), path);
         normal_no_bar    .require(f._getAsset(), path);
         active_no_bar    .require(f._getAsset(), path);
         normal_text_style.require(f._getAsset(), path);
         active_text_style.require(f._getAsset(), path);
         if(minimize.load(f, path))
         if(maximize.load(f, path))
         if(close   .load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
void GuiSkin::reset()
{
        background_color.set(128);
            border_color.set(0, 112);
   mouse_highlight_color.set(20, 0);
keyboard_highlight_color.set(86, 157, 229);

   font      .clear();
   text_style.clear();

   button  .reset();
   checkbox.reset();
   combobox.reset();
   desc    .reset();
   imm     .reset();
   list    .reset();
   menu    .reset();
   menubar .reset();
   progress.reset();
   property.reset();
   region  .reset();
   slidebar.reset();
   slider  .reset();
   tab     .reset();
   text    .reset();
   textline.reset();
   window  .reset();
}
/******************************************************************************/
Bool GuiSkin::save(File &f, CChar *path)C
{
   f.putUInt (CC4_GSKN);
   f.cmpUIntV(1       ); // version

   f<<background_color<<border_color<<mouse_highlight_color<<keyboard_highlight_color;
   f.putAsset(font      .id());
   f.putAsset(text_style.id());
   if(button  .save(f, path))
   if(checkbox.save(f, path))
   if(combobox.save(f, path))
   if(desc    .save(f, path))
   if(imm     .save(f, path))
   if(list    .save(f, path))
   if(menu    .save(f, path))
   if(menubar .save(f, path))
   if(progress.save(f, path))
   if(property.save(f, path))
   if(region  .save(f, path))
   if(slidebar.save(f, path))
   if(slider  .save(f, path))
   if(tab     .save(f, path))
   if(text    .save(f, path))
   if(textline.save(f, path))
   if(window  .save(f, path))
      return f.ok();
   return false;
}
Bool GuiSkin::load(File &f, CChar *path)
{
   if(f.getUInt()==CC4_GSKN)switch(f.decUIntV()) // version
   {
      case 1:
      {
         f>>background_color>>border_color>>mouse_highlight_color>>keyboard_highlight_color;
         font      .require(f.getAssetID(), path);
         text_style.require(f.getAssetID(), path);
         if(button  .load(f, path))
         if(checkbox.load(f, path))
         if(combobox.load(f, path))
         if(desc    .load(f, path))
         if(imm     .load(f, path))
         if(list    .load(f, path))
         if(menu    .load(f, path))
         if(menubar .load(f, path))
         if(progress.load(f, path))
         if(property.load(f, path))
         if(region  .load(f, path))
         if(slidebar.load(f, path))
         if(slider  .load(f, path))
         if(tab     .load(f, path))
         if(text    .load(f, path))
         if(textline.load(f, path))
         if(window  .load(f, path))
            if(f.ok())return true;
      }break;

      case 0:
      {
         f>>background_color>>border_color>>mouse_highlight_color>>keyboard_highlight_color;
         font      .require(f._getAsset(), path);
         text_style.require(f._getAsset(), path);
         property.reset();
         if(button  .load(f, path))
         if(checkbox.load(f, path))
         if(combobox.load(f, path))
         if(desc    .load(f, path))
         if(imm     .load(f, path))
         if(list    .load(f, path))
         if(menu    .load(f, path))
         if(menubar .load(f, path))
         if(progress.load(f, path))
         if(region  .load(f, path))
         if(slidebar.load(f, path))
         if(slider  .load(f, path))
         if(tab     .load(f, path))
         if(text    .load(f, path))
         if(textline.load(f, path))
         if(window  .load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
Bool GuiSkin::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool GuiSkin::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
/*void GuiSkin::operator=(C Str &name)
{
   if(!load(name))Exit(MLT(S+"Can't load Gui Skin \""      +name+"\"",
                       PL,S+u"Nie można wczytać GuiSkin \""+name+"\""));
}
/******************************************************************************/
}
/******************************************************************************/
