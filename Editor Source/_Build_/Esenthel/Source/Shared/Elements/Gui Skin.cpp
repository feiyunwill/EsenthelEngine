/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
      void EditGuiSkin::Button::copyTo(GuiSkin::Button &skin, C Project &proj)C
      {
         skin.normal         =proj.gamePath(normal_id);
         skin.pushed         =proj.gamePath(pushed_id.valid() ? pushed_id : normal_id);
         skin.disabled       =proj.gamePath(disabled_id.valid() ? disabled_id : normal_id);
         skin.pushed_disabled=proj.gamePath(pushed_disabled_id.valid() ? pushed_disabled_id : pushed_id.valid() ? pushed_id : disabled_id.valid() ? disabled_id : normal_id);
         skin.text_style     =proj.gamePath(text_style_id);
      }
      void EditGuiSkin::Button::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, pushed_id);
         Include(ids, disabled_id);
         Include(ids, pushed_disabled_id);
         Include(ids, text_style_id);
      }
      bool EditGuiSkin::Button::newer(C Button &src)C
      {
         return normal>src.normal || pushed>src.pushed || disabled>src.disabled || pushed_disabled>src.pushed_disabled
             || normal_color>src.normal_color || pushed_color>src.pushed_color || disabled_color>src.disabled_color || pushed_disabled_color>src.pushed_disabled_color
             || text_size>src.text_size || text_padd>src.text_padd || text_style>src.text_style;
      }
      bool EditGuiSkin::Button::equal(C Button &src)C
      {
         return normal==src.normal && pushed==src.pushed && disabled==src.disabled && pushed_disabled==src.pushed_disabled
             && normal_color==src.normal_color && pushed_color==src.pushed_color && disabled_color==src.disabled_color && pushed_disabled_color==src.pushed_disabled_color
             && text_size==src.text_size && text_padd==src.text_padd && text_style==src.text_style;
      }
      void EditGuiSkin::Button::newData()
      {
         normal++; pushed++; disabled++; pushed_disabled++;
         normal_color++; pushed_color++; disabled_color++; pushed_disabled_color++;
         text_size++; text_padd++; text_style++;
      }
      bool EditGuiSkin::Button::sync(GuiSkin::Button &val, C Button &src, C GuiSkin::Button &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(pushed, src.pushed, pushed_id, src.pushed_id);
         changed|=Sync(disabled, src.disabled, disabled_id, src.disabled_id);
         changed|=Sync(pushed_disabled, src.pushed_disabled, pushed_disabled_id, src.pushed_disabled_id);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(pushed_color, src.pushed_color, val.pushed_color, src_val.pushed_color);
         changed|=Sync(disabled_color, src.disabled_color, val.disabled_color, src_val.disabled_color);
         changed|=Sync(pushed_disabled_color, src.pushed_disabled_color, val.pushed_disabled_color, src_val.pushed_disabled_color);
         changed|=Sync(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Sync(text_padd, src.text_padd, val.text_padd, src_val.text_padd);
         return changed;
      }
      bool EditGuiSkin::Button::undo(GuiSkin::Button &val, C Button &src, C GuiSkin::Button &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(pushed, src.pushed, pushed_id, src.pushed_id);
         changed|=Undo(disabled, src.disabled, disabled_id, src.disabled_id);
         changed|=Undo(pushed_disabled, src.pushed_disabled, pushed_disabled_id, src.pushed_disabled_id);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(pushed_color, src.pushed_color, val.pushed_color, src_val.pushed_color);
         changed|=Undo(disabled_color, src.disabled_color, val.disabled_color, src_val.disabled_color);
         changed|=Undo(pushed_disabled_color, src.pushed_disabled_color, val.pushed_disabled_color, src_val.pushed_disabled_color);
         changed|=Undo(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Undo(text_padd, src.text_padd, val.text_padd, src_val.text_padd);
         return changed;
      }
      bool EditGuiSkin::Button::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<pushed_id<<disabled_id<<pushed_disabled_id<<text_style_id
          <<normal<<pushed<<disabled<<pushed_disabled
          <<normal_color<<pushed_color<<disabled_color<<pushed_disabled_color
          <<text_size<<text_padd<<text_style;
         return f.ok();
      }
      bool EditGuiSkin::Button::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>normal_id>>pushed_id>>disabled_id>>pushed_disabled_id>>text_style_id
                >>normal>>pushed>>disabled>>pushed_disabled
                >>normal_color>>pushed_color>>disabled_color>>pushed_disabled_color
                >>text_size>>text_padd>>text_style;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Button::reset() {T=Button();}
      void EditGuiSkin::ButtonImage::copyTo(GuiSkin::ButtonImage &skin, C Project &proj)C
      {
         ::EditGuiSkin::Button::copyTo(skin, proj);
         skin.image=proj.gamePath(image_id);
      }
      void EditGuiSkin::ButtonImage::includeIDs(MemPtr<UID> ids)C
      {
         ::EditGuiSkin::Button::includeIDs(ids);
         Include(ids, image_id);
      }
      bool EditGuiSkin::ButtonImage::newer(C ButtonImage &src)C
      {
         return ::EditGuiSkin::Button::newer(src) || image>src.image || image_color>src.image_color; // || image_color_add>src.image_color_add
      }
      bool EditGuiSkin::ButtonImage::equal(C ButtonImage &src)C
      {
         return ::EditGuiSkin::Button::equal(src) && image==src.image && image_color==src.image_color; // && image_color_add==src.image_color_add
      }
      void EditGuiSkin::ButtonImage::newData()
      {
         ::EditGuiSkin::Button::newData(); image++; image_color++; // image_color_add++;
      }
      bool EditGuiSkin::ButtonImage::sync(GuiSkin::ButtonImage &val, C ButtonImage &src, C GuiSkin::ButtonImage &src_val)
      {
         bool changed=::EditGuiSkin::Button::sync(val, src, src_val);
         changed|=Sync(image, src.image, image_id, src.image_id);
         changed|=Sync(image_color, src.image_color, val.image_color, src_val.image_color);
       //changed|=Sync(image_color_add, src.image_color_add, val.image_color_add, src_val.image_color_add);
         return changed;
      }
      bool EditGuiSkin::ButtonImage::undo(GuiSkin::ButtonImage &val, C ButtonImage &src, C GuiSkin::ButtonImage &src_val)
      {
         bool changed=::EditGuiSkin::Button::undo(val, src, src_val);
         changed|=Undo(image, src.image, image_id, src.image_id);
         changed|=Undo(image_color, src.image_color, val.image_color, src_val.image_color);
       //changed|=Undo(image_color_add, src.image_color_add, val.image_color_add, src_val.image_color_add);
         return changed;
      }
      bool EditGuiSkin::ButtonImage::save(File &f)C
      {
         f.cmpUIntV(0);
         ::EditGuiSkin::Button::save(f);
         f<<image_id<<image<<image_color;
         return f.ok();
      }
      bool EditGuiSkin::ButtonImage::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0: if(::EditGuiSkin::Button::load(f))
            {
               f>>image_id>>image>>image_color;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::ButtonImage::reset() {T=ButtonImage();}
      void EditGuiSkin::CheckBox::copyTo(GuiSkin::CheckBox &skin, C Project &proj)C
      {
         skin.off  =proj.gamePath(  off_id);
         skin.on   =proj.gamePath(   on_id);
         skin.multi=proj.gamePath(multi_id);
      }
      void EditGuiSkin::CheckBox::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids,   off_id);
         Include(ids,    on_id);
         Include(ids, multi_id);
      }
      bool EditGuiSkin::CheckBox::newer(C CheckBox &src)C
      {
         return off>src.off || on>src.on || multi>src.multi || normal_color>src.normal_color || disabled_color>src.disabled_color;
      }
      bool EditGuiSkin::CheckBox::equal(C CheckBox &src)C
      {
         return off==src.off && on==src.on && multi==src.multi && normal_color==src.normal_color && disabled_color==src.disabled_color;
      }
      void EditGuiSkin::CheckBox::newData()
      {
         off++; on++; multi++; normal_color++; disabled_color++;
      }
      bool EditGuiSkin::CheckBox::sync(GuiSkin::CheckBox &val, C CheckBox &src, C GuiSkin::CheckBox &src_val)
      {
         bool changed=false;
         changed|=Sync(off, src.off, off_id, src.off_id);
         changed|=Sync(on , src.on , on_id, src.on_id);
         changed|=Sync(multi, src.multi, multi_id, src.multi_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(disabled_color, src.disabled_color, val.disabled_color, src_val.disabled_color);
         return changed;
      }
      bool EditGuiSkin::CheckBox::undo(GuiSkin::CheckBox &val, C CheckBox &src, C GuiSkin::CheckBox &src_val)
      {
         bool changed=false;
         changed|=Undo(off, src.off, off_id, src.off_id);
         changed|=Undo(on , src.on , on_id, src.on_id);
         changed|=Undo(multi, src.multi, multi_id, src.multi_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(disabled_color, src.disabled_color, val.disabled_color, src_val.disabled_color);
         return changed;
      }
      bool EditGuiSkin::CheckBox::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<off_id<<on_id<<multi_id
          <<off<<on<<multi
          <<normal_color<<disabled_color;
         return f.ok();
      }
      bool EditGuiSkin::CheckBox::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>off_id>>on_id>>multi_id
                >>off>>on>>multi
                >>normal_color>>disabled_color;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::CheckBox::reset() {T=CheckBox();}
      void EditGuiSkin::ComboBox::copyTo(GuiSkin::ComboBox &skin, C Project &proj)C
      {
         ::EditGuiSkin::ButtonImage::copyTo(skin, proj);
      }
      void EditGuiSkin::ComboBox::includeIDs(MemPtr<UID> ids)C
      {
         ::EditGuiSkin::ButtonImage::includeIDs(ids);
      }
      bool EditGuiSkin::ComboBox::newer(C ComboBox &src)C
      {
         return ::EditGuiSkin::ButtonImage::newer(src);
      }
      bool EditGuiSkin::ComboBox::equal(C ComboBox &src)C
      {
         return ::EditGuiSkin::ButtonImage::equal(src);
      }
      void EditGuiSkin::ComboBox::newData()
      {
         ::EditGuiSkin::ButtonImage::newData();
      }
      bool EditGuiSkin::ComboBox::sync(GuiSkin::ComboBox &val, C ComboBox &src, C GuiSkin::ComboBox &src_val)
      {
         bool   changed=::EditGuiSkin::ButtonImage::sync(val, src, src_val);
         return changed;
      }
      bool EditGuiSkin::ComboBox::undo(GuiSkin::ComboBox &val, C ComboBox &src, C GuiSkin::ComboBox &src_val)
      {
         bool   changed=::EditGuiSkin::ButtonImage::undo(val, src, src_val);
         return changed;
      }
      bool EditGuiSkin::ComboBox::save(File &f)C
      {
         f.cmpUIntV(0);
         return ::EditGuiSkin::ButtonImage::save(f);
      }
      bool EditGuiSkin::ComboBox::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0: if(::EditGuiSkin::ButtonImage::load(f))
            {
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::ComboBox::reset() {T=ComboBox();}
      void EditGuiSkin::Desc::copyTo(GuiSkin::Desc &skin, C Project &proj)C
      {
         skin.normal    =proj.gamePath(normal_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void EditGuiSkin::Desc::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, text_style_id);
      }
      bool EditGuiSkin::Desc::newer(C Desc &src)C
      {
         return normal>src.normal || normal_color>src.normal_color || padding>src.padding || text_style>src.text_style;
      }
      bool EditGuiSkin::Desc::equal(C Desc &src)C
      {
         return normal==src.normal && normal_color==src.normal_color && padding==src.padding && text_style==src.text_style;
      }
      void EditGuiSkin::Desc::newData()
      {
         normal++; normal_color++; padding++; text_style++;
      }
      bool EditGuiSkin::Desc::sync(GuiSkin::Desc &val, C Desc &src, C GuiSkin::Desc &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool EditGuiSkin::Desc::undo(GuiSkin::Desc &val, C Desc &src, C GuiSkin::Desc &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool EditGuiSkin::Desc::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<text_style_id
          <<normal<<normal_color<<padding<<text_style;
         return f.ok();
      }
      bool EditGuiSkin::Desc::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>normal_id>>text_style_id
                >>normal>>normal_color>>padding>>text_style;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Desc::reset() {T=Desc();}
      void EditGuiSkin::IMM::copyTo(GuiSkin::IMM &skin, C Project &proj)C
      {
         skin.normal    =proj.gamePath(normal_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void EditGuiSkin::IMM::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, text_style_id);
      }
      bool EditGuiSkin::IMM::newer(C IMM &src)C
      {
         return normal>src.normal || normal_color>src.normal_color || padding>src.padding || text_style>src.text_style;
      }
      bool EditGuiSkin::IMM::equal(C IMM &src)C
      {
         return normal==src.normal && normal_color==src.normal_color && padding==src.padding && text_style==src.text_style;
      }
      void EditGuiSkin::IMM::newData()
      {
         normal++; normal_color++; padding++; text_style++;
      }
      bool EditGuiSkin::IMM::sync(GuiSkin::IMM &val, C IMM &src, C GuiSkin::IMM &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool EditGuiSkin::IMM::undo(GuiSkin::IMM &val, C IMM &src, C GuiSkin::IMM &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool EditGuiSkin::IMM::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<text_style_id
          <<normal<<normal_color<<padding<<text_style;
         return f.ok();
      }
      bool EditGuiSkin::IMM::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>normal_id>>text_style_id
                >>normal>>normal_color>>padding>>text_style;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::IMM::reset() {T=IMM();}
      void EditGuiSkin::List::copyTo(GuiSkin::List &skin, C Project &proj)C
      {
         skin.cursor       =proj.gamePath(cursor_id.valid() ? cursor_id : selection_id);
         skin.highlight    =proj.gamePath(highlight_id);
         skin.selection    =proj.gamePath(selection_id.valid() ? selection_id : cursor_id);
         skin.text_style   =proj.gamePath(text_style_id);
         skin.resize_column=proj.gamePath(resize_column_id);
         column.copyTo(skin.column, proj);
      }
      void EditGuiSkin::List::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, cursor_id);
         Include(ids, highlight_id);
         Include(ids, selection_id);
         Include(ids, text_style_id);
         Include(ids, resize_column_id);
         column.includeIDs(ids);
      }
      bool EditGuiSkin::List::newer(C List &src)C
      {
         return cursor>src.cursor || highlight>src.highlight || selection>src.selection
             || cursor_color>src.cursor_color || highlight_color>src.highlight_color || selection_color>src.selection_color
             || text_style>src.text_style || resize_column>src.resize_column
             || column.newer(src.column);
      }
      bool EditGuiSkin::List::equal(C List &src)C
      {
         return cursor==src.cursor && highlight==src.highlight && selection==src.selection
             && cursor_color==src.cursor_color && highlight_color==src.highlight_color && selection_color==src.selection_color
             && text_style==src.text_style && resize_column==src.resize_column
             && column.equal(src.column);
      }
      void EditGuiSkin::List::newData()
      {
         cursor++; highlight++; selection++;
         cursor_color++; highlight_color++; selection_color++;
         text_style++; resize_column++;
         column.newData();
      }
      bool EditGuiSkin::List::sync(GuiSkin::List &val, C List &src, C GuiSkin::List &src_val)
      {
         bool changed=false;
         changed|=Sync(cursor, src.cursor, cursor_id, src.cursor_id);
         changed|=Sync(highlight, src.highlight, highlight_id, src.highlight_id);
         changed|=Sync(selection, src.selection, selection_id, src.selection_id);
         changed|=Sync(cursor_color, src.cursor_color, val.cursor_color, src_val.cursor_color);
         changed|=Sync(highlight_color, src.highlight_color, val.highlight_color, src_val.highlight_color);
         changed|=Sync(selection_color, src.selection_color, val.selection_color, src_val.selection_color);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(resize_column, src.resize_column, resize_column_id, src.resize_column_id);
         changed|=column.sync(val.column, src.column, src_val.column);
         return changed;
      }
      bool EditGuiSkin::List::undo(GuiSkin::List &val, C List &src, C GuiSkin::List &src_val)
      {
         bool changed=false;
         changed|=Undo(cursor, src.cursor, cursor_id, src.cursor_id);
         changed|=Undo(highlight, src.highlight, highlight_id, src.highlight_id);
         changed|=Undo(selection, src.selection, selection_id, src.selection_id);
         changed|=Undo(cursor_color, src.cursor_color, val.cursor_color, src_val.cursor_color);
         changed|=Undo(highlight_color, src.highlight_color, val.highlight_color, src_val.highlight_color);
         changed|=Undo(selection_color, src.selection_color, val.selection_color, src_val.selection_color);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(resize_column, src.resize_column, resize_column_id, src.resize_column_id);
         changed|=column.undo(val.column, src.column, src_val.column);
         return changed;
      }
      bool EditGuiSkin::List::save(File &f)C
      {
         f.cmpUIntV(1);
         f<<cursor_id<<highlight_id<<selection_id<<text_style_id<<resize_column_id
          <<cursor<<highlight<<selection
          <<cursor_color<<highlight_color<<selection_color
          <<text_style<<resize_column;
         column.save(f);
         return f.ok();
      }
      bool EditGuiSkin::List::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 1:
            {
               f>>cursor_id>>highlight_id>>selection_id>>text_style_id>>resize_column_id
                >>cursor>>highlight>>selection
                >>cursor_color>>highlight_color>>selection_color
                >>text_style>>resize_column;
               if(column.load(f))
                  if(f.ok())return true;
            }break;

            case 0:
            {
               f>>cursor_id>>highlight_id>>selection_id>>text_style_id
                >>cursor>>highlight>>selection
                >>cursor_color>>highlight_color>>selection_color
                >>text_style;
               if(column.load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::List::reset() {T=List();}
      void EditGuiSkin::Menu::copyTo(GuiSkin::Menu &skin, C Project &proj)C
      {
         skin.normal  =proj.gamePath(normal_id);
         skin.check   =proj.gamePath(check_id);
         skin.sub_menu=proj.gamePath(sub_menu_id);
      }
      void EditGuiSkin::Menu::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, check_id);
         Include(ids, sub_menu_id);
      }
      bool EditGuiSkin::Menu::newer(C Menu &src)C
      {
         return normal>src.normal || normal_color>src.normal_color
             || padding>src.padding
             || list_elm_height>src.list_elm_height
             || check>src.check || sub_menu>src.sub_menu
             || check_color>src.check_color || sub_menu_color>src.sub_menu_color;
      }
      bool EditGuiSkin::Menu::equal(C Menu &src)C
      {
         return normal==src.normal && normal_color==src.normal_color
             && padding==src.padding
             && list_elm_height==src.list_elm_height
             && check==src.check && sub_menu==src.sub_menu
             && check_color==src.check_color && sub_menu_color==src.sub_menu_color;
      }
      void EditGuiSkin::Menu::newData()
      {
         normal++; normal_color++;
         padding++;
         list_elm_height++;
         check++; sub_menu++;
         check_color++; sub_menu_color++;
      }
      bool EditGuiSkin::Menu::sync(GuiSkin::Menu &val, C Menu &src, C GuiSkin::Menu &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(padding, src.padding, val.padding, src_val.padding);
         changed|=Sync(list_elm_height, src.list_elm_height, val.list_elm_height, src_val.list_elm_height);
         changed|=Sync(check, src.check, check_id, src.check_id);
         changed|=Sync(sub_menu, src.sub_menu, sub_menu_id, src.sub_menu_id);
         changed|=Sync(check_color, src.check_color, val.check_color, src_val.check_color);
         changed|=Sync(sub_menu_color, src.sub_menu_color, val.sub_menu_color, src_val.sub_menu_color);
         return changed;
      }
      bool EditGuiSkin::Menu::undo(GuiSkin::Menu &val, C Menu &src, C GuiSkin::Menu &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(padding, src.padding, val.padding, src_val.padding);
         changed|=Undo(list_elm_height, src.list_elm_height, val.list_elm_height, src_val.list_elm_height);
         changed|=Undo(check, src.check, check_id, src.check_id);
         changed|=Undo(sub_menu, src.sub_menu, sub_menu_id, src.sub_menu_id);
         changed|=Undo(check_color, src.check_color, val.check_color, src_val.check_color);
         changed|=Undo(sub_menu_color, src.sub_menu_color, val.sub_menu_color, src_val.sub_menu_color);
         return changed;
      }
      bool EditGuiSkin::Menu::save(File &f)C
      {
         f.cmpUIntV(1);
         f<<normal_id<<check_id<<sub_menu_id
          <<normal<<normal_color
          <<padding
          <<list_elm_height
          <<check<<sub_menu
          <<check_color<<sub_menu_color;
         return f.ok();
      }
      bool EditGuiSkin::Menu::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 1:
            {
               f>>normal_id>>check_id>>sub_menu_id
                >>normal>>normal_color
                >>padding
                >>list_elm_height
                >>check>>sub_menu
                >>check_color>>sub_menu_color;
               if(f.ok())return true;
            }break;

            case 0:
            {
               f>>normal_id>>check_id>>sub_menu_id
                >>normal>>normal_color
                >>padding
                >>check>>sub_menu
                >>check_color>>sub_menu_color;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Menu::reset() {T=Menu();}
      void EditGuiSkin::MenuBar::copyTo(GuiSkin::MenuBar &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         skin.highlight =proj.gamePath( highlight_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void EditGuiSkin::MenuBar::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         Include(ids,  highlight_id);
         Include(ids, text_style_id);
      }
      bool EditGuiSkin::MenuBar::newer(C MenuBar &src)C
      {
         return background>src.background || highlight>src.highlight || background_color>src.background_color || highlight_color>src.highlight_color
             || bar_height>src.bar_height || text_size>src.text_size || text_padd>src.text_padd
             || text_style>src.text_style;
      }
      bool EditGuiSkin::MenuBar::equal(C MenuBar &src)C
      {
         return background==src.background && highlight==src.highlight && background_color==src.background_color && highlight_color==src.highlight_color
             && bar_height==src.bar_height && text_size==src.text_size && text_padd==src.text_padd
             && text_style==src.text_style;
      }
      void EditGuiSkin::MenuBar::newData()
      {
         background++; highlight++; background_color++; highlight_color++;
         bar_height++; text_size++; text_padd++;
         text_style++;
      }
      bool EditGuiSkin::MenuBar::sync(GuiSkin::MenuBar &val, C MenuBar &src, C GuiSkin::MenuBar &src_val)
      {
         bool changed=false;
         changed|=Sync(background, src.background, background_id, src.background_id);
         changed|=Sync(highlight, src.highlight, highlight_id, src.highlight_id);
         changed|=Sync(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Sync(highlight_color, src.highlight_color, val.highlight_color, src_val.highlight_color);
         changed|=Sync(bar_height, src.bar_height, val.bar_height, src_val.bar_height);
         changed|=Sync(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Sync(text_padd, src.text_padd, val.text_padd, src_val.text_padd);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         return changed;
      }
      bool EditGuiSkin::MenuBar::undo(GuiSkin::MenuBar &val, C MenuBar &src, C GuiSkin::MenuBar &src_val)
      {
         bool changed=false;
         changed|=Undo(background, src.background, background_id, src.background_id);
         changed|=Undo(highlight, src.highlight, highlight_id, src.highlight_id);
         changed|=Undo(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Undo(highlight_color, src.highlight_color, val.highlight_color, src_val.highlight_color);
         changed|=Undo(bar_height, src.bar_height, val.bar_height, src_val.bar_height);
         changed|=Undo(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Undo(text_padd, src.text_padd, val.text_padd, src_val.text_padd);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         return changed;
      }
      bool EditGuiSkin::MenuBar::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<highlight_id<<text_style_id
          <<background<<highlight
          <<background_color<<highlight_color
          <<bar_height<<text_size<<text_padd
          <<text_style;
         return f.ok();
      }
      bool EditGuiSkin::MenuBar::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>background_id>>highlight_id>>text_style_id
                >>background>>highlight
                >>background_color>>highlight_color
                >>bar_height>>text_size>>text_padd
                >>text_style;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::MenuBar::reset() {T=MenuBar();}
      void EditGuiSkin::Progress::copyTo(GuiSkin::Progress &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         skin.progress  =proj.gamePath(  progress_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void EditGuiSkin::Progress::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         Include(ids,   progress_id);
         Include(ids, text_style_id);
      }
      bool EditGuiSkin::Progress::newer(C Progress &src)C
      {
         return background>src.background || progress>src.progress
             || background_color>src.background_color || progress_color>src.progress_color
             || text_style>src.text_style || text_size>src.text_size
             || draw_progress_partial>src.draw_progress_partial;
      }
      bool EditGuiSkin::Progress::equal(C Progress &src)C
      {
         return background==src.background && progress==src.progress
             && background_color==src.background_color && progress_color==src.progress_color
             && text_style==src.text_style && text_size==src.text_size
             && draw_progress_partial==src.draw_progress_partial;
      }
      void EditGuiSkin::Progress::newData()
      {
         background++; progress++;
         background_color++; progress_color++;
         text_style++; text_size++;
         draw_progress_partial++;
      }
      bool EditGuiSkin::Progress::sync(GuiSkin::Progress &val, C Progress &src, C GuiSkin::Progress &src_val)
      {
         bool changed=false;
         changed|=Sync(background, src.background, background_id, src.background_id);
         changed|=Sync(progress, src.progress, progress_id, src.progress_id);
         changed|=Sync(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Sync(progress_color, src.progress_color, val.progress_color, src_val.progress_color);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Sync(draw_progress_partial, src.draw_progress_partial, val.draw_progress_partial, src_val.draw_progress_partial);
         return changed;
      }
      bool EditGuiSkin::Progress::undo(GuiSkin::Progress &val, C Progress &src, C GuiSkin::Progress &src_val)
      {
         bool changed=false;
         changed|=Undo(background, src.background, background_id, src.background_id);
         changed|=Undo(progress, src.progress, progress_id, src.progress_id);
         changed|=Undo(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Undo(progress_color, src.progress_color, val.progress_color, src_val.progress_color);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Undo(draw_progress_partial, src.draw_progress_partial, val.draw_progress_partial, src_val.draw_progress_partial);
         return changed;
      }
      bool EditGuiSkin::Progress::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<progress_id<<text_style_id
          <<background<<progress
          <<background_color<<progress_color
          <<text_style<<text_size
          <<draw_progress_partial;
         return f.ok();
      }
      bool EditGuiSkin::Progress::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>background_id>>progress_id>>text_style_id
                >>background>>progress
                >>background_color>>progress_color
                >>text_style>>text_size
                >>draw_progress_partial;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Progress::reset() {T=Progress();}
      void EditGuiSkin::Property::copyTo(GuiSkin::Property &skin, C Project &proj)C
      {
         value.copyTo(skin.value, proj);
      }
      void EditGuiSkin::Property::includeIDs(MemPtr<UID> ids)C
      {
         value.includeIDs(ids);
      }
      bool EditGuiSkin::Property::newer(C Property &src)C
      {
         return value.newer(src.value);
      }
      bool EditGuiSkin::Property::equal(C Property &src)C
      {
         return value.equal(src.value);
      }
      void EditGuiSkin::Property::newData()
      {
         value.newData();
      }
      bool EditGuiSkin::Property::sync(GuiSkin::Property &val, C Property &src, C GuiSkin::Property &src_val)
      {
         bool changed=false;
         changed|=value.sync(val.value, src.value, src_val.value);
         return changed;
      }
      bool EditGuiSkin::Property::undo(GuiSkin::Property &val, C Property &src, C GuiSkin::Property &src_val)
      {
         bool changed=false;
         changed|=value.undo(val.value, src.value, src_val.value);
         return changed;
      }
      bool EditGuiSkin::Property::save(File &f)C
      {
         f.cmpUIntV(0);
         value.save(f);
         return f.ok();
      }
      bool EditGuiSkin::Property::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               if(value.load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Property::reset() {T=Property();}
      void EditGuiSkin::Region::copyTo(GuiSkin::Region &skin, C Project &proj)C
      {
         skin.normal=proj.gamePath(normal_id);
         view.copyTo(skin.view, proj);
      }
      void EditGuiSkin::Region::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         view.includeIDs(ids);
      }
      bool EditGuiSkin::Region::newer(C Region &src)C
      {
         return normal>src.normal || normal_color>src.normal_color || view.newer(src.view);
      }
      bool EditGuiSkin::Region::equal(C Region &src)C
      {
         return normal==src.normal && normal_color==src.normal_color && view.equal(src.view);
      }
      void EditGuiSkin::Region::newData()
      {
         normal++; normal_color++;
         view.newData();
      }
      bool EditGuiSkin::Region::sync(GuiSkin::Region &val, C Region &src, C GuiSkin::Region &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=view.sync(val.view, src.view, src_val.view);
         return changed;
      }
      bool EditGuiSkin::Region::undo(GuiSkin::Region &val, C Region &src, C GuiSkin::Region &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=view.undo(val.view, src.view, src_val.view);
         return changed;
      }
      bool EditGuiSkin::Region::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<normal<<normal_color;
         view.save(f);
         return f.ok();
      }
      bool EditGuiSkin::Region::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>normal_id>>normal>>normal_color;
               if(view.load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Region::reset() {T=Region();}
      void EditGuiSkin::SlideBar::copyTo(GuiSkin::SlideBar &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         left  .copyTo(skin.left  , proj);
         center.copyTo(skin.center, proj);
         right .copyTo(skin.right , proj);
      }
      void EditGuiSkin::SlideBar::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         left  .includeIDs(ids);
         center.includeIDs(ids);
         right .includeIDs(ids);
      }
      bool EditGuiSkin::SlideBar::newer(C SlideBar &src)C
      {
         return background>src.background || background_color>src.background_color || left.newer(src.left) || center.newer(src.center) || right.newer(src.right);
      }
      bool EditGuiSkin::SlideBar::equal(C SlideBar &src)C
      {
         return background==src.background && background_color==src.background_color && left.equal(src.left) && center.equal(src.center) && right.equal(src.right);
      }
      void EditGuiSkin::SlideBar::newData()
      {
         background++; background_color++; left.newData(); center.newData(); right.newData();
      }
      bool EditGuiSkin::SlideBar::sync(GuiSkin::SlideBar &val, C SlideBar &src, C GuiSkin::SlideBar &src_val)
      {
         bool changed=false;
         changed|=Sync(background, src.background, background_id, src.background_id);
         changed|=left.sync(val.left, src.left, src_val.left);
         changed|=center.sync(val.center, src.center, src_val.center);
         changed|=right.sync(val.right, src.right, src_val.right);
         return changed;
      }
      bool EditGuiSkin::SlideBar::undo(GuiSkin::SlideBar &val, C SlideBar &src, C GuiSkin::SlideBar &src_val)
      {
         bool changed=false;
         changed|=Undo(background, src.background, background_id, src.background_id);
         changed|=left.undo(val.left, src.left, src_val.left);
         changed|=center.undo(val.center, src.center, src_val.center);
         changed|=right.undo(val.right, src.right, src_val.right);
         return changed;
      }
      bool EditGuiSkin::SlideBar::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<background<<background_color;
         left  .save(f);
         center.save(f);
         right .save(f);
         return f.ok();
      }
      bool EditGuiSkin::SlideBar::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>background_id>>background>>background_color;
               if(left  .load(f))
               if(center.load(f))
               if(right .load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::SlideBar::reset() {T=SlideBar();}
      void EditGuiSkin::Slider::copyTo(GuiSkin::Slider &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         skin.progress  =proj.gamePath(  progress_id);
         skin.slider    =proj.gamePath(    slider_id);
      }
      void EditGuiSkin::Slider::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         Include(ids,   progress_id);
         Include(ids,     slider_id);
      }
      bool EditGuiSkin::Slider::newer(C Slider &src)C
      {
         return background>src.background || progress>src.progress || slider>src.slider
             || background_color>src.background_color || progress_color>src.progress_color || slider_color>src.slider_color
             || background_shrink>src.background_shrink
             || draw_progress_partial>src.draw_progress_partial;
      }
      bool EditGuiSkin::Slider::equal(C Slider &src)C
      {
         return background==src.background && progress==src.progress && slider==src.slider
             && background_color==src.background_color && progress_color==src.progress_color && slider_color==src.slider_color
             && background_shrink==src.background_shrink
             && draw_progress_partial==src.draw_progress_partial;
      }
      void EditGuiSkin::Slider::newData()
      {
         background++; progress++; slider++;
         background_color++; progress_color++; slider_color++;
         background_shrink++;
         draw_progress_partial++;
      }
      bool EditGuiSkin::Slider::sync(GuiSkin::Slider &val, C Slider &src, C GuiSkin::Slider &src_val)
      {
         bool changed=false;
         changed|=Sync(background, src.background, background_id, src.background_id);
         changed|=Sync(progress, src.progress, progress_id, src.progress_id);
         changed|=Sync(slider, src.slider, slider_id, src.slider_id);
         changed|=Sync(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Sync(progress_color, src.progress_color, val.progress_color, src_val.progress_color);
         changed|=Sync(slider_color, src.slider_color, val.slider_color, src_val.slider_color);
         changed|=Sync(background_shrink, src.background_shrink, val.background_shrink, src_val.background_shrink);
         changed|=Sync(draw_progress_partial, src.draw_progress_partial, val.draw_progress_partial, src_val.draw_progress_partial);
         return changed;
      }
      bool EditGuiSkin::Slider::undo(GuiSkin::Slider &val, C Slider &src, C GuiSkin::Slider &src_val)
      {
         bool changed=false;
         changed|=Undo(background, src.background, background_id, src.background_id);
         changed|=Undo(progress, src.progress, progress_id, src.progress_id);
         changed|=Undo(slider, src.slider, slider_id, src.slider_id);
         changed|=Undo(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Undo(progress_color, src.progress_color, val.progress_color, src_val.progress_color);
         changed|=Undo(slider_color, src.slider_color, val.slider_color, src_val.slider_color);
         changed|=Undo(background_shrink, src.background_shrink, val.background_shrink, src_val.background_shrink);
         changed|=Undo(draw_progress_partial, src.draw_progress_partial, val.draw_progress_partial, src_val.draw_progress_partial);
         return changed;
      }
      bool EditGuiSkin::Slider::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<progress_id<<slider_id
          <<background<<progress<<slider
          <<background_color<<progress_color<<slider_color
          <<background_shrink
          <<draw_progress_partial;
         return f.ok();
      }
      bool EditGuiSkin::Slider::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>background_id>>progress_id>>slider_id
                >>background>>progress>>slider
                >>background_color>>progress_color>>slider_color
                >>background_shrink
                >>draw_progress_partial;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Slider::reset() {T=Slider();}
         EditGuiSkin::Tab::Kind::Kind(C Button &button) : normal_id(button.normal_id), pushed_id(button.pushed_id) {}
         bool EditGuiSkin::Tab::Kind::valid()C {return normal_id.valid() || pushed_id.valid();}
         void EditGuiSkin::Tab::Kind::copyTo(Button &button)C
         {
            button.normal_id=normal_id;
            button.pushed_id=pushed_id;
         }
         bool EditGuiSkin::Tab::Kind::apply(Kind   &kind  )C {if(valid()){kind=T;         return true;} return false;}
         bool EditGuiSkin::Tab::Kind::apply(Button &button)C {if(valid()){copyTo(button); return true;} return false;}
         void EditGuiSkin::Tab::Kind::includeIDs(MemPtr<UID> ids)C
         {
            Include(ids, normal_id);
            Include(ids, pushed_id);
         }
         bool EditGuiSkin::Tab::Kind::newer(C Kind &src)C {return normal> src.normal || pushed> src.pushed;}
         bool EditGuiSkin::Tab::Kind::equal(C Kind &src)C {return normal==src.normal && pushed==src.pushed;}
         void EditGuiSkin::Tab::Kind::newData() {normal++; pushed++;}
         bool EditGuiSkin::Tab::Kind::sync(C Kind &src)
         {
            bool changed=false;
            changed|=Sync(normal, src.normal, normal_id, src.normal_id);
            changed|=Sync(pushed, src.pushed, pushed_id, src.pushed_id);
            return changed;
         }
         bool EditGuiSkin::Tab::Kind::undo(C Kind &src)
         {
            bool changed=false;
            changed|=Undo(normal, src.normal, normal_id, src.normal_id);
            changed|=Undo(pushed, src.pushed, pushed_id, src.pushed_id);
            return changed;
         }
         bool EditGuiSkin::Tab::Kind::save(File &f)C
         {
            f.cmpUIntV(0);
            f<<normal_id<<pushed_id<<normal<<pushed;
            return f.ok();
         }
         bool EditGuiSkin::Tab::Kind::load(File &f)
         {
            reset(); switch(f.decUIntV())
            {
               case 0:
               {
                  f>>normal_id>>pushed_id>>normal>>pushed;
                  if(f.ok())return true;
               }break;
            }
            reset(); return false;
         }
         void EditGuiSkin::Tab::Kind::reset() {T=Kind();}
      void EditGuiSkin::Tab::copyTo(GuiSkin::Tab &skin, C Project &proj)C
      {
      }
      void EditGuiSkin::Tab::includeIDs(MemPtr<UID> ids)C
      {
         ::EditGuiSkin::Button::includeIDs(ids);
         left.includeIDs(ids); horizontal.includeIDs(ids); right.includeIDs(ids); top.includeIDs(ids); vertical.includeIDs(ids); bottom.includeIDs(ids);
         top_left.includeIDs(ids); top_right.includeIDs(ids); bottom_left.includeIDs(ids); bottom_right.includeIDs(ids);
      }
      bool EditGuiSkin::Tab::newer(C Tab &src)C
      {
         return ::EditGuiSkin::Button::newer(src)
             || left.newer(src.left) || horizontal.newer(src.horizontal) || right.newer(src.right)
             || top.newer(src.top) || vertical.newer(src.vertical) || bottom.newer(src.bottom)
             || top_left.newer(src.top_left) || top_right.newer(src.top_right) || bottom_left.newer(src.bottom_left) || bottom_right.newer(src.bottom_right);
      }
      bool EditGuiSkin::Tab::equal(C Tab &src)C
      {
         return ::EditGuiSkin::Button::equal(src)
             && left.equal(src.left) && horizontal.equal(src.horizontal) && right.equal(src.right)
             && top.equal(src.top) && vertical.equal(src.vertical) && bottom.equal(src.bottom)
             && top_left.equal(src.top_left) && top_right.equal(src.top_right) && bottom_left.equal(src.bottom_left) && bottom_right.equal(src.bottom_right);
      }
      void EditGuiSkin::Tab::newData()
      {
         ::EditGuiSkin::Button::newData();
         left.newData(); horizontal.newData(); right.newData(); top.newData(); vertical.newData(); bottom.newData();
         top_left.newData(); top_right.newData(); bottom_left.newData(); bottom_right.newData();
      }
      bool EditGuiSkin::Tab::sync(GuiSkin::Tab &val, C Tab &src, C GuiSkin::Tab &src_val)
      {
         bool changed=::EditGuiSkin::Button::sync(val.left, src, src_val.left);
         changed|=left        .sync(src.left        );
         changed|=horizontal  .sync(src.horizontal  );
         changed|=right       .sync(src.right       );
         changed|=top         .sync(src.top         );
         changed|=vertical    .sync(src.vertical    );
         changed|=bottom      .sync(src.bottom      );
         changed|=top_left    .sync(src.top_left    );
         changed|=top_right   .sync(src.top_right   );
         changed|=bottom_left .sync(src.bottom_left );
         changed|=bottom_right.sync(src.bottom_right);
         return changed;
      }
      bool EditGuiSkin::Tab::undo(GuiSkin::Tab &val, C Tab &src, C GuiSkin::Tab &src_val)
      {
         bool changed=::EditGuiSkin::Button::undo(val.left, src, src_val.left);
         changed|=left        .undo(src.left        );
         changed|=horizontal  .undo(src.horizontal  );
         changed|=right       .undo(src.right       );
         changed|=top         .undo(src.top         );
         changed|=vertical    .undo(src.vertical    );
         changed|=bottom      .undo(src.bottom      );
         changed|=top_left    .undo(src.top_left    );
         changed|=top_right   .undo(src.top_right   );
         changed|=bottom_left .undo(src.bottom_left );
         changed|=bottom_right.undo(src.bottom_right);
         return changed;
      }
      bool EditGuiSkin::Tab::save(File &f)C
      {
         f.cmpUIntV(0);
         ::EditGuiSkin::Button::save(f);
         left        .save(f);
         horizontal  .save(f);
         right       .save(f);
         top         .save(f);
         vertical    .save(f);
         bottom      .save(f);
         top_left    .save(f);
         top_right   .save(f);
         bottom_left .save(f);
         bottom_right.save(f);
         return f.ok();
      }
      bool EditGuiSkin::Tab::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0: if(::EditGuiSkin::Button::load(f))
            {
               if(left        .load(f))
               if(horizontal  .load(f))
               if(right       .load(f))
               if(top         .load(f))
               if(vertical    .load(f))
               if(bottom      .load(f))
               if(top_left    .load(f))
               if(top_right   .load(f))
               if(bottom_left .load(f))
               if(bottom_right.load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Tab::reset() {T=Tab();}
      void EditGuiSkin::Text::copyTo(GuiSkin::Text &skin, C Project &proj)C
      {
         skin.text_style=proj.gamePath(text_style_id);
      }
      void EditGuiSkin::Text::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, text_style_id);
      }
      bool EditGuiSkin::Text::newer(C Text &src)C
      {
         return text_style>src.text_style;
      }
      bool EditGuiSkin::Text::equal(C Text &src)C
      {
         return text_style==src.text_style;
      }
      void EditGuiSkin::Text::newData()
      {
         text_style++;
      }
      bool EditGuiSkin::Text::sync(GuiSkin::Text &val, C Text &src, C GuiSkin::Text &src_val)
      {
         bool changed=false;
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         return changed;
      }
      bool EditGuiSkin::Text::undo(GuiSkin::Text &val, C Text &src, C GuiSkin::Text &src_val)
      {
         bool changed=false;
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         return changed;
      }
      bool EditGuiSkin::Text::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<text_style_id<<text_style;
         return f.ok();
      }
      bool EditGuiSkin::Text::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0:
            {
               f>>text_style_id>>text_style;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Text::reset() {T=Text();}
      void EditGuiSkin::TextLine::copyTo(GuiSkin::TextLine &skin, C Project &proj)C
      {
         skin.normal=proj.gamePath(normal_id);
         skin.disabled=proj.gamePath(disabled_id.valid() ? disabled_id : normal_id);
         skin.text_style=proj.gamePath(text_style_id);
         skin.find_image=proj.gamePath(find_image_id);
         clear.copyTo(skin.clear, proj);
      }
      void EditGuiSkin::TextLine::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, disabled_id);
         Include(ids, text_style_id);
         Include(ids, find_image_id);
         clear.includeIDs(ids);
      }
      bool EditGuiSkin::TextLine::newer(C TextLine &src)C
      {
         return normal>src.normal || disabled>src.disabled
             || normal_panel_color>src.normal_panel_color || normal_text_color>src.normal_text_color
             || disabled_panel_color>src.disabled_panel_color || disabled_text_color>src.disabled_text_color
             || rect_color>src.rect_color
             || text_size>src.text_size
             || text_style>src.text_style
             || find_image>src.find_image
             || clear.newer(src.clear);
      }
      bool EditGuiSkin::TextLine::equal(C TextLine &src)C
      {
         return normal==src.normal && disabled==src.disabled
             && normal_panel_color==src.normal_panel_color && normal_text_color==src.normal_text_color
             && disabled_panel_color==src.disabled_panel_color && disabled_text_color==src.disabled_text_color
             && rect_color==src.rect_color
             && text_size==src.text_size
             && text_style==src.text_style
             && find_image==src.find_image
             && clear.equal(src.clear);
      }
      void EditGuiSkin::TextLine::newData()
      {
         normal++; disabled++;
         normal_panel_color++; normal_text_color++; disabled_panel_color++; disabled_text_color++;
         rect_color++;
         text_size++;
         text_style++;
         find_image++;
         clear.newData();
      }
      bool EditGuiSkin::TextLine::sync(GuiSkin::TextLine &val, C TextLine &src, C GuiSkin::TextLine &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(disabled, src.disabled, disabled_id, src.disabled_id);
         changed|=Sync(normal_panel_color, src.normal_panel_color, val.normal_panel_color, src_val.normal_panel_color);
         changed|=Sync(normal_text_color, src.normal_text_color, val.normal_text_color, src_val.normal_text_color);
         changed|=Sync(disabled_panel_color, src.disabled_panel_color, val.disabled_panel_color, src_val.disabled_panel_color);
         changed|=Sync(disabled_text_color, src.disabled_text_color, val.disabled_text_color, src_val.disabled_text_color);
         changed|=Sync(rect_color, src.rect_color, val.rect_color, src_val.rect_color);
         changed|=Sync(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(find_image, src.find_image, find_image_id, src.find_image_id);
         changed|=clear.sync(val.clear, src.clear, src_val.clear);
         return changed;
      }
      bool EditGuiSkin::TextLine::undo(GuiSkin::TextLine &val, C TextLine &src, C GuiSkin::TextLine &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(disabled, src.disabled, disabled_id, src.disabled_id);
         changed|=Undo(normal_panel_color, src.normal_panel_color, val.normal_panel_color, src_val.normal_panel_color);
         changed|=Undo(normal_text_color, src.normal_text_color, val.normal_text_color, src_val.normal_text_color);
         changed|=Undo(disabled_panel_color, src.disabled_panel_color, val.disabled_panel_color, src_val.disabled_panel_color);
         changed|=Undo(disabled_text_color, src.disabled_text_color, val.disabled_text_color, src_val.disabled_text_color);
         changed|=Undo(rect_color, src.rect_color, val.rect_color, src_val.rect_color);
         changed|=Undo(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(find_image, src.find_image, find_image_id, src.find_image_id);
         changed|=clear.undo(val.clear, src.clear, src_val.clear);
         return changed;
      }
      bool EditGuiSkin::TextLine::save(File &f)C
      {
         f.cmpUIntV(3);
         f<<normal_id<<disabled_id<<text_style_id<<find_image_id
          <<normal<<disabled
          <<normal_panel_color<<normal_text_color<<disabled_panel_color<<disabled_text_color<<rect_color
          <<text_size
          <<text_style
          <<find_image;
         if(clear.save(f))
            return f.ok();
         return false;
      }
      bool EditGuiSkin::TextLine::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 3:
            {
               f>>normal_id>>disabled_id>>text_style_id>>find_image_id
                >>normal>>disabled
                >>normal_panel_color>>normal_text_color>>disabled_panel_color>>disabled_text_color>>rect_color
                >>text_size
                >>text_style
                >>find_image;
                if(clear.load(f))
                  if(f.ok())return true;
            }break;

            case 2:
            {
               f>>normal_id>>disabled_id>>text_style_id>>find_image_id
                >>normal>>disabled
                >>normal_panel_color>>normal_text_color>>disabled_panel_color>>disabled_text_color>>rect_color
                >>text_style
                >>find_image;
                if(clear.load(f))
                  if(f.ok())return true;
            }break;

            case 1:
            {
               f>>normal_id>>disabled_id>>text_style_id>>find_image_id
                >>normal>>disabled
                >>normal_panel_color>>normal_text_color>>disabled_panel_color>>disabled_text_color>>rect_color
                >>text_style
                >>find_image;
               if(f.ok())return true;
            }break;

            case 0:
            {
               f>>normal_id>>disabled_id>>text_style_id>>find_image_id
                >>normal>>disabled
                >>normal_panel_color>>disabled_panel_color>>rect_color
                >>text_style
                >>find_image;
               normal_text_color=normal_panel_color; disabled_text_color=disabled_panel_color;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::TextLine::reset() {T=TextLine();}
      void EditGuiSkin::Window::copyTo(GuiSkin::Window &skin, C Project &proj)C
      {
         skin.normal           =proj.gamePath(normal_id.valid() ? normal_id : active_id.valid() ? active_id : normal_no_bar_id.valid() ? normal_no_bar_id : active_no_bar_id);
         skin.active           =proj.gamePath(active_id.valid() ? active_id : normal_id.valid() ? normal_id : active_no_bar_id.valid() ? active_no_bar_id : normal_no_bar_id);
         skin.normal_no_bar    =proj.gamePath(normal_no_bar_id.valid() ? normal_no_bar_id : active_no_bar_id.valid() ? active_no_bar_id : normal_id.valid() ? normal_id : active_id);
         skin.active_no_bar    =proj.gamePath(active_no_bar_id.valid() ? active_no_bar_id : normal_no_bar_id.valid() ? normal_no_bar_id : active_id.valid() ? active_id : normal_id);
         skin.normal_text_style=proj.gamePath(normal_text_style_id.valid() ? normal_text_style_id : active_text_style_id);
         skin.active_text_style=proj.gamePath(active_text_style_id.valid() ? active_text_style_id : normal_text_style_id);
         minimize.copyTo(skin.minimize, proj);
         maximize.copyTo(skin.maximize, proj);
         close   .copyTo(skin.close   , proj);
      }
      void EditGuiSkin::Window::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, active_id);
         Include(ids, normal_no_bar_id);
         Include(ids, active_no_bar_id);
         Include(ids, normal_text_style_id);
         Include(ids, active_text_style_id);
         minimize.includeIDs(ids);
         maximize.includeIDs(ids);
         close   .includeIDs(ids);
      }
      bool EditGuiSkin::Window::newer(C Window &src)C
      {
         return normal>src.normal || active>src.active || normal_no_bar>src.normal_no_bar || active_no_bar>src.active_no_bar
             || normal_color>src.normal_color || active_color>src.active_color
             || normal_text_style>src.normal_text_style || active_text_style>src.active_text_style
             || text_size>src.text_size || text_padd>src.text_padd || button_offset>src.button_offset
             || minimize.newer(src.minimize) || maximize.newer(src.maximize) || close.newer(src.close);
      }
      bool EditGuiSkin::Window::equal(C Window &src)C
      {
         return normal==src.normal && active==src.active && normal_no_bar==src.normal_no_bar && active_no_bar==src.active_no_bar
             && normal_color==src.normal_color && active_color==src.active_color
             && normal_text_style==src.normal_text_style && active_text_style==src.active_text_style
             && text_size==src.text_size && text_padd==src.text_padd && button_offset==src.button_offset
             && minimize.equal(src.minimize) && maximize.equal(src.maximize) && close.equal(src.close);
      }
      void EditGuiSkin::Window::newData()
      {
         normal++; active++; normal_no_bar++; active_no_bar++;
         normal_color++; active_color++;
         normal_text_style++; active_text_style++;
         text_size++; text_padd++; button_offset++;
         minimize.newData(); maximize.newData(); close.newData();
      }
      bool EditGuiSkin::Window::sync(GuiSkin::Window &val, C Window &src, C GuiSkin::Window &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(active, src.active, active_id, src.active_id);
         changed|=Sync(normal_no_bar, src.normal_no_bar, normal_no_bar_id, src.normal_no_bar_id);
         changed|=Sync(active_no_bar, src.active_no_bar, active_no_bar_id, src.active_no_bar_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(active_color, src.active_color, val.active_color, src_val.active_color);
         changed|=Sync(normal_text_style, src.normal_text_style, normal_text_style_id, src.normal_text_style_id);
         changed|=Sync(active_text_style, src.active_text_style, active_text_style_id, src.active_text_style_id);
         changed|=Sync(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Sync(text_padd, src.text_padd, val.text_padd, src_val.text_padd);
         changed|=Sync(button_offset, src.button_offset, val.button_offset, src_val.button_offset);
         changed|=minimize.sync(val.minimize, src.minimize, src_val.minimize);
         changed|=maximize.sync(val.maximize, src.maximize, src_val.maximize);
         changed|=close   .sync(val.close   , src.close   , src_val.close   );
         return changed;
      }
      bool EditGuiSkin::Window::undo(GuiSkin::Window &val, C Window &src, C GuiSkin::Window &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(active, src.active, active_id, src.active_id);
         changed|=Undo(normal_no_bar, src.normal_no_bar, normal_no_bar_id, src.normal_no_bar_id);
         changed|=Undo(active_no_bar, src.active_no_bar, active_no_bar_id, src.active_no_bar_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(active_color, src.active_color, val.active_color, src_val.active_color);
         changed|=Undo(normal_text_style, src.normal_text_style, normal_text_style_id, src.normal_text_style_id);
         changed|=Undo(active_text_style, src.active_text_style, active_text_style_id, src.active_text_style_id);
         changed|=Undo(text_size, src.text_size, val.text_size, src_val.text_size);
         changed|=Undo(text_padd, src.text_padd, val.text_padd, src_val.text_padd);
         changed|=Undo(button_offset, src.button_offset, val.button_offset, src_val.button_offset);
         changed|=minimize.undo(val.minimize, src.minimize, src_val.minimize);
         changed|=maximize.undo(val.maximize, src.maximize, src_val.maximize);
         changed|=close   .undo(val.close   , src.close   , src_val.close   );
         return changed;
      }
      bool EditGuiSkin::Window::save(File &f)C
      {
         f.cmpUIntV(1);
         f<<normal_id<<active_id<<normal_no_bar_id<<active_no_bar_id
          <<normal_text_style_id<<active_text_style_id
          <<normal<<active<<normal_no_bar<<active_no_bar
          <<normal_color<<active_color
          <<normal_text_style<<active_text_style
          <<text_size<<text_padd<<button_offset;
         minimize.save(f);
         maximize.save(f);
         close   .save(f);
         return f.ok();
      }
      bool EditGuiSkin::Window::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 1:
            {
               f>>normal_id>>active_id>>normal_no_bar_id>>active_no_bar_id
                >>normal_text_style_id>>active_text_style_id
                >>normal>>active>>normal_no_bar>>active_no_bar
                >>normal_color>>active_color
                >>normal_text_style>>active_text_style
                >>text_size>>text_padd>>button_offset;
               if(minimize.load(f))
               if(maximize.load(f))
               if(close   .load(f))
                  if(f.ok())return true;
            }break;

            case 0:
            {
               f>>normal_id>>active_id>>normal_no_bar_id>>active_no_bar_id
                >>normal_text_style_id>>active_text_style_id
                >>normal>>active>>normal_no_bar>>active_no_bar
                >>normal_color>>active_color
                >>normal_text_style>>active_text_style
                >>text_size>>text_padd;
               if(minimize.load(f))
               if(maximize.load(f))
               if(close   .load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Window::reset() {T=Window();}
      void EditGuiSkin::Base::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, font_id);
         Include(ids, text_style_id);
         button.includeIDs(ids);
         checkbox.includeIDs(ids);
         combobox.includeIDs(ids);
         desc.includeIDs(ids);
         imm .includeIDs(ids);
         list.includeIDs(ids);
         menu.includeIDs(ids);
         menubar.includeIDs(ids);
         progress.includeIDs(ids);
         property.includeIDs(ids);
         region.includeIDs(ids);
         slidebar.includeIDs(ids);
         slider.includeIDs(ids);
         tab.includeIDs(ids);
         text.includeIDs(ids);
         textline.includeIDs(ids);
         window.includeIDs(ids);
      }
      void EditGuiSkin::Base::copyTo(GuiSkin &skin, C Project &proj)C
      {
         skin.font      =proj.gamePath(font_id);
         skin.text_style=proj.gamePath(text_style_id);
         button.copyTo(skin.button, proj);
         checkbox.copyTo(skin.checkbox, proj);
         combobox.copyTo(skin.combobox, proj);
         desc.copyTo(skin.desc, proj);
         imm .copyTo(skin.imm , proj);
         list.copyTo(skin.list, proj);
         menu.copyTo(skin.menu, proj);
         menubar.copyTo(skin.menubar, proj);
         progress.copyTo(skin.progress, proj);
         property.copyTo(skin.property, proj);
         region.copyTo(skin.region, proj);
         slidebar.copyTo(skin.slidebar, proj);
         slider.copyTo(skin.slider, proj);
         tab.copyTo(skin.tab, proj);
         text.copyTo(skin.text, proj);
         textline.copyTo(skin.textline, proj);
         window.copyTo(skin.window, proj);
         
         if(!skin.button.text_style)skin.button.text_style=skin.text_style;
         if(!skin.combobox.text_style)skin.combobox.text_style=skin.button.text_style;
         if(!skin.list.column.text_style)skin.list.column.text_style=skin.button.text_style;
         if(!skin.property.value.text_style)skin.property.value.text_style=skin.button.text_style;
         if(!skin.textline.clear.text_style)skin.textline.clear.text_style=skin.button.text_style;

         if(!skin.desc.text_style)skin.desc.text_style=skin.text_style;
         if(!skin.imm .text_style)skin.imm.text_style=skin.desc.text_style;

         if(!skin.text.text_style)skin.text.text_style=skin.text_style;
         if(!skin.list.text_style)skin.list.text_style=skin.text_style;
         if(!skin.menubar.text_style)skin.menubar.text_style=skin.text_style;
         if(!skin.progress.text_style)skin.progress.text_style=skin.text_style;
         if(!skin.textline.text_style)skin.textline.text_style=skin.text_style;
         if(!skin.window.normal_text_style)skin.window.normal_text_style=skin.text_style;
         if(!skin.window.active_text_style)skin.window.active_text_style=skin.text_style;

         // tabs
         {
            // prepare
            Button   temp=tab; // get text and colors
            Tab::Kind hor, ver, any;
            if(!tab.horizontal.apply(hor) && !tab.left.apply(hor) && !tab.right .apply(hor)){}
            if(!tab.vertical  .apply(ver) && !tab.top .apply(ver) && !tab.bottom.apply(ver)){}
            if(!hor.apply(any) && !ver.apply(any)){}
            if(!temp.text_style_id.valid())temp.text_style_id=skin.button.text_style.id(); // adjust text

            // copy base data from main
            skin.tab.horizontal=skin.tab.left;
            skin.tab.right=skin.tab.left;
            skin.tab.top=skin.tab.left;
            skin.tab.vertical=skin.tab.left;
            skin.tab.bottom=skin.tab.left;
            skin.tab.top_left=skin.tab.left;
            skin.tab.top_right=skin.tab.left;
            skin.tab.bottom_left=skin.tab.left;
            skin.tab.bottom_right=skin.tab.left;

            // copy ID based
            if(!tab.left        .apply(temp) && !hor.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.left, proj);
            if(!tab.horizontal  .apply(temp) && !hor.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.horizontal, proj);
            if(!tab.right       .apply(temp) && !hor.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.right, proj);
            if(!tab.top         .apply(temp) && !ver.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.top, proj);
            if(!tab.vertical    .apply(temp) && !ver.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.vertical, proj);
            if(!tab.bottom      .apply(temp) && !ver.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.bottom, proj);
            if(!tab.top_left    .apply(temp) && !hor.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.top_left, proj);
            if(!tab.top_right   .apply(temp) && !hor.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.top_right, proj);
            if(!tab.bottom_left .apply(temp) && !hor.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.bottom_left, proj);
            if(!tab.bottom_right.apply(temp) && !hor.apply(temp))any.copyTo(temp); temp.copyTo(skin.tab.bottom_right, proj);
         }
      }
      bool EditGuiSkin::Base::newer(C Base &src)C
      {
         return      background_color>src.background_color
         ||              border_color>src.border_color
         ||     mouse_highlight_color>src.mouse_highlight_color
         ||  keyboard_highlight_color>src.keyboard_highlight_color
         || font>src.font || text_style>src.text_style
         || button.newer(src.button)
         || checkbox.newer(src.checkbox)
         || combobox.newer(src.combobox)
         || desc.newer(src.desc)
         || imm.newer(src.imm)
         || list.newer(src.list)
         || menu.newer(src.menu)
         || menubar.newer(src.menubar)
         || progress.newer(src.progress)
         || property.newer(src.property)
         || region.newer(src.region)
         || slidebar.newer(src.slidebar)
         || slider.newer(src.slider)
         || tab.newer(src.tab)
         || text.newer(src.text)
         || textline.newer(src.textline)
         || window.newer(src.window);
      }
      bool EditGuiSkin::Base::equal(C Base &src)C
      {
         return     background_color==src.background_color
         &&             border_color==src.border_color
         &&    mouse_highlight_color==src.mouse_highlight_color
         && keyboard_highlight_color==src.keyboard_highlight_color
         && font==src.font && text_style==src.text_style
         && button.equal(src.button)
         && checkbox.equal(src.checkbox)
         && combobox.equal(src.combobox)
         && desc.equal(src.desc)
         && imm.equal(src.imm)
         && list.equal(src.list)
         && menu.equal(src.menu)
         && menubar.equal(src.menubar)
         && progress.equal(src.progress)
         && property.equal(src.property)
         && region.equal(src.region)
         && slidebar.equal(src.slidebar)
         && slider.equal(src.slider)
         && tab.equal(src.tab)
         && text.equal(src.text)
         && textline.equal(src.textline)
         && window.equal(src.window);
      }
      void EditGuiSkin::Base::newData()
      {
         background_color++; border_color++; mouse_highlight_color++; keyboard_highlight_color++;
         font++; text_style++;
         button.newData();
         checkbox.newData();
         combobox.newData();
         desc.newData();
         imm.newData();
         list.newData();
         menu.newData();
         menubar.newData();
         progress.newData();
         property.newData();
         region.newData();
         slidebar.newData();
         slider.newData();
         tab.newData();
         text.newData();
         textline.newData();
         window.newData();
      }
      bool EditGuiSkin::Base::sync(GuiSkin &val, C Base &src, C GuiSkin &src_val)
      {
         bool changed=false;
         changed|=Sync(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Sync(border_color, src.border_color, val.border_color, src_val.border_color);
         changed|=Sync(mouse_highlight_color, src.mouse_highlight_color, val.mouse_highlight_color, src_val.mouse_highlight_color);
         changed|=Sync(keyboard_highlight_color, src.keyboard_highlight_color, val.keyboard_highlight_color, src_val.keyboard_highlight_color);
         changed|=Sync(font, src.font, font_id, src.font_id);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=button.sync(val.button, src.button, src_val.button);
         changed|=checkbox.sync(val.checkbox, src.checkbox, src_val.checkbox);
         changed|=combobox.sync(val.combobox, src.combobox, src_val.combobox);
         changed|=desc.sync(val.desc, src.desc, src_val.desc);
         changed|=imm.sync(val.imm, src.imm, src_val.imm);
         changed|=list.sync(val.list, src.list, src_val.list);
         changed|=menu.sync(val.menu, src.menu, src_val.menu);
         changed|=menubar.sync(val.menubar, src.menubar, src_val.menubar);
         changed|=progress.sync(val.progress, src.progress, src_val.progress);
         changed|=property.sync(val.property, src.property, src_val.property);
         changed|=region.sync(val.region, src.region, src_val.region);
         changed|=slidebar.sync(val.slidebar, src.slidebar, src_val.slidebar);
         changed|=slider.sync(val.slider, src.slider, src_val.slider);
         changed|=tab.sync(val.tab, src.tab, src_val.tab);
         changed|=text.sync(val.text, src.text, src_val.text);
         changed|=textline.sync(val.textline, src.textline, src_val.textline);
         changed|=window.sync(val.window, src.window, src_val.window);
         return changed;
      }
      bool EditGuiSkin::Base::undo(GuiSkin &val, C Base &src, C GuiSkin &src_val)
      {
         bool changed=false;
         changed|=Undo(background_color, src.background_color, val.background_color, src_val.background_color);
         changed|=Undo(border_color, src.border_color, val.border_color, src_val.border_color);
         changed|=Undo(mouse_highlight_color, src.mouse_highlight_color, val.mouse_highlight_color, src_val.mouse_highlight_color);
         changed|=Undo(keyboard_highlight_color, src.keyboard_highlight_color, val.keyboard_highlight_color, src_val.keyboard_highlight_color);
         changed|=Undo(font, src.font, font_id, src.font_id);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=button.undo(val.button, src.button, src_val.button);
         changed|=checkbox.undo(val.checkbox, src.checkbox, src_val.checkbox);
         changed|=combobox.undo(val.combobox, src.combobox, src_val.combobox);
         changed|=desc.undo(val.desc, src.desc, src_val.desc);
         changed|=imm.undo(val.imm, src.imm, src_val.imm);
         changed|=list.undo(val.list, src.list, src_val.list);
         changed|=menu.undo(val.menu, src.menu, src_val.menu);
         changed|=menubar.undo(val.menubar, src.menubar, src_val.menubar);
         changed|=progress.undo(val.progress, src.progress, src_val.progress);
         changed|=property.undo(val.property, src.property, src_val.property);
         changed|=region.undo(val.region, src.region, src_val.region);
         changed|=slidebar.undo(val.slidebar, src.slidebar, src_val.slidebar);
         changed|=slider.undo(val.slider, src.slider, src_val.slider);
         changed|=tab.undo(val.tab, src.tab, src_val.tab);
         changed|=text.undo(val.text, src.text, src_val.text);
         changed|=textline.undo(val.textline, src.textline, src_val.textline);
         changed|=window.undo(val.window, src.window, src_val.window);
         return changed;
      }
      bool EditGuiSkin::Base::save(File &f)C
      {
         f.cmpUIntV(1);
         f<<background_color<<border_color<<mouse_highlight_color<<keyboard_highlight_color
          <<font_id<<text_style_id
          <<font<<text_style;
         button.save(f);
         checkbox.save(f);
         combobox.save(f);
         desc.save(f);
         imm.save(f);
         list.save(f);
         menu.save(f);
         menubar.save(f);
         progress.save(f);
         property.save(f);
         region.save(f);
         slidebar.save(f);
         slider.save(f);
         tab.save(f);
         text.save(f);
         textline.save(f);
         window.save(f);
         return f.ok();
      }
      bool EditGuiSkin::Base::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 1:
            {
               f>>background_color>>border_color>>mouse_highlight_color>>keyboard_highlight_color
                >>font_id>>text_style_id
                >>font>>text_style;
               if(button.load(f))
               if(checkbox.load(f))
               if(combobox.load(f))
               if(desc.load(f))
               if(imm.load(f))
               if(list.load(f))
               if(menu.load(f))
               if(menubar.load(f))
               if(progress.load(f))
               if(property.load(f))
               if(region.load(f))
               if(slidebar.load(f))
               if(slider.load(f))
               if(tab.load(f))
               if(text.load(f))
               if(textline.load(f))
               if(window.load(f))
                  if(f.ok())return true;
            }break;

            case 0:
            {
               f>>background_color>>border_color>>mouse_highlight_color>>keyboard_highlight_color
                >>font_id>>text_style_id
                >>font>>text_style;
               if(button.load(f))
               if(checkbox.load(f))
               if(combobox.load(f))
               if(desc.load(f))
               if(imm.load(f))
               if(list.load(f))
               if(menu.load(f))
               if(menubar.load(f))
               if(progress.load(f))
               if(region.load(f))
               if(slidebar.load(f))
               if(slider.load(f))
               if(tab.load(f))
               if(text.load(f))
               if(textline.load(f))
               if(window.load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditGuiSkin::Base::reset() {T=Base();}
   void EditGuiSkin::reset(                  )  {T=EditGuiSkin();}
   void EditGuiSkin::newData(                  )  {       base.newData();}
   bool EditGuiSkin::newer(C EditGuiSkin &src)C {return base.newer(src.base);}
   bool EditGuiSkin::equal(C EditGuiSkin &src)C {return base.equal(src.base);}
   bool EditGuiSkin::sync(C EditGuiSkin &src)  {bool changed=base.sync(T, src.base, src); return changed;}
   bool EditGuiSkin::undo(C EditGuiSkin &src)  {bool changed=base.undo(T, src.base, src); return changed;}
   void EditGuiSkin::copyTo(      GuiSkin &skin, C Project &proj)C {skin=T; base.copyTo(skin, proj);}
   bool EditGuiSkin::save(File &f)C
   {
      f.cmpUIntV(0);
      ::EE::GuiSkin::save(f);
      base .save(f);
      return f.ok();
   }
   bool EditGuiSkin::load(File &f)
   {
      reset(); switch(f.decUIntV())
      {
         case 0: if(::EE::GuiSkin::load(f))
         {
            if(base.load(f))
               if(f.ok())return true;
         }break;
      }
      reset(); return false;
   }
   bool EditGuiSkin::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
EditGuiSkin::Button::Button() : normal_id(UIDZero), pushed_id(UIDZero), disabled_id(UIDZero), pushed_disabled_id(UIDZero), text_style_id(UIDZero) {}

EditGuiSkin::ButtonImage::ButtonImage() : image_id(UIDZero) {}

EditGuiSkin::CheckBox::CheckBox() : off_id(UIDZero), on_id(UIDZero), multi_id(UIDZero) {}

EditGuiSkin::Desc::Desc() : normal_id(UIDZero), text_style_id(UIDZero) {}

EditGuiSkin::IMM::IMM() : normal_id(UIDZero), text_style_id(UIDZero) {}

EditGuiSkin::List::List() : cursor_id(UIDZero), highlight_id(UIDZero), selection_id(UIDZero), text_style_id(UIDZero), resize_column_id(UIDZero) {}

EditGuiSkin::Menu::Menu() : normal_id(UIDZero), check_id(UIDZero), sub_menu_id(UIDZero) {}

EditGuiSkin::MenuBar::MenuBar() : background_id(UIDZero), highlight_id(UIDZero), text_style_id(UIDZero) {}

EditGuiSkin::Progress::Progress() : background_id(UIDZero), progress_id(UIDZero), text_style_id(UIDZero) {}

EditGuiSkin::Region::Region() : normal_id(UIDZero) {}

EditGuiSkin::SlideBar::SlideBar() : background_id(UIDZero) {}

EditGuiSkin::Slider::Slider() : background_id(UIDZero), progress_id(UIDZero), slider_id(UIDZero) {}

EditGuiSkin::Tab::Kind::Kind() : normal_id(UIDZero), pushed_id(UIDZero) {}

EditGuiSkin::Text::Text() : text_style_id(UIDZero) {}

EditGuiSkin::TextLine::TextLine() : normal_id(UIDZero), disabled_id(UIDZero), text_style_id(UIDZero), find_image_id(UIDZero) {}

EditGuiSkin::Window::Window() : normal_id(UIDZero), active_id(UIDZero), normal_no_bar_id(UIDZero), active_no_bar_id(UIDZero), normal_text_style_id(UIDZero), active_text_style_id(UIDZero) {}

EditGuiSkin::Base::Base() : font_id(UIDZero), text_style_id(UIDZero) {}

/******************************************************************************/
