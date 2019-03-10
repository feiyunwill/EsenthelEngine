/******************************************************************************/
class EditGuiSkin : GuiSkin
{
   class Button
   {
      UID       normal_id=UIDZero, pushed_id=UIDZero, disabled_id=UIDZero, pushed_disabled_id=UIDZero, text_style_id=UIDZero;
      TimeStamp normal, pushed, disabled, pushed_disabled,
                normal_color, pushed_color, disabled_color, pushed_disabled_color,
                text_size, text_padd, text_style;

      void copyTo(GuiSkin.Button &skin, C Project &proj)C
      {
         skin.normal         =proj.gamePath(normal_id);
         skin.pushed         =proj.gamePath(pushed_id.valid() ? pushed_id : normal_id);
         skin.disabled       =proj.gamePath(disabled_id.valid() ? disabled_id : normal_id);
         skin.pushed_disabled=proj.gamePath(pushed_disabled_id.valid() ? pushed_disabled_id : pushed_id.valid() ? pushed_id : disabled_id.valid() ? disabled_id : normal_id);
         skin.text_style     =proj.gamePath(text_style_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, pushed_id);
         Include(ids, disabled_id);
         Include(ids, pushed_disabled_id);
         Include(ids, text_style_id);
      }
      bool newer(C Button &src)C
      {
         return normal>src.normal || pushed>src.pushed || disabled>src.disabled || pushed_disabled>src.pushed_disabled
             || normal_color>src.normal_color || pushed_color>src.pushed_color || disabled_color>src.disabled_color || pushed_disabled_color>src.pushed_disabled_color
             || text_size>src.text_size || text_padd>src.text_padd || text_style>src.text_style;
      }
      bool equal(C Button &src)C
      {
         return normal==src.normal && pushed==src.pushed && disabled==src.disabled && pushed_disabled==src.pushed_disabled
             && normal_color==src.normal_color && pushed_color==src.pushed_color && disabled_color==src.disabled_color && pushed_disabled_color==src.pushed_disabled_color
             && text_size==src.text_size && text_padd==src.text_padd && text_style==src.text_style;
      }
      void newData()
      {
         normal++; pushed++; disabled++; pushed_disabled++;
         normal_color++; pushed_color++; disabled_color++; pushed_disabled_color++;
         text_size++; text_padd++; text_style++;
      }
      bool sync(GuiSkin.Button &val, C Button &src, C GuiSkin.Button &src_val)
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
      bool undo(GuiSkin.Button &val, C Button &src, C GuiSkin.Button &src_val)
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
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<pushed_id<<disabled_id<<pushed_disabled_id<<text_style_id
          <<normal<<pushed<<disabled<<pushed_disabled
          <<normal_color<<pushed_color<<disabled_color<<pushed_disabled_color
          <<text_size<<text_padd<<text_style;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=Button();}
   }

   class ButtonImage : Button
   {
      UID       image_id=UIDZero;
      TimeStamp image, image_color;

      void copyTo(GuiSkin.ButtonImage &skin, C Project &proj)C
      {
         super.copyTo(skin, proj);
         skin.image=proj.gamePath(image_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         super.includeIDs(ids);
         Include(ids, image_id);
      }
      bool newer(C ButtonImage &src)C
      {
         return super.newer(src) || image>src.image || image_color>src.image_color; // || image_color_add>src.image_color_add
      }
      bool equal(C ButtonImage &src)C
      {
         return super.equal(src) && image==src.image && image_color==src.image_color; // && image_color_add==src.image_color_add
      }
      void newData()
      {
         super.newData(); image++; image_color++; // image_color_add++;
      }
      bool sync(GuiSkin.ButtonImage &val, C ButtonImage &src, C GuiSkin.ButtonImage &src_val)
      {
         bool changed=super.sync(val, src, src_val);
         changed|=Sync(image, src.image, image_id, src.image_id);
         changed|=Sync(image_color, src.image_color, val.image_color, src_val.image_color);
       //changed|=Sync(image_color_add, src.image_color_add, val.image_color_add, src_val.image_color_add);
         return changed;
      }
      bool undo(GuiSkin.ButtonImage &val, C ButtonImage &src, C GuiSkin.ButtonImage &src_val)
      {
         bool changed=super.undo(val, src, src_val);
         changed|=Undo(image, src.image, image_id, src.image_id);
         changed|=Undo(image_color, src.image_color, val.image_color, src_val.image_color);
       //changed|=Undo(image_color_add, src.image_color_add, val.image_color_add, src_val.image_color_add);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         super.save(f);
         f<<image_id<<image<<image_color;
         return f.ok();
      }
      bool load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0: if(super.load(f))
            {
               f>>image_id>>image>>image_color;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void reset() {T=ButtonImage();}
   }

   class CheckBox
   {
      UID       off_id=UIDZero, on_id=UIDZero, multi_id=UIDZero;
      TimeStamp off, on, multi,
                normal_color, disabled_color;

      void copyTo(GuiSkin.CheckBox &skin, C Project &proj)C
      {
         skin.off  =proj.gamePath(  off_id);
         skin.on   =proj.gamePath(   on_id);
         skin.multi=proj.gamePath(multi_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids,   off_id);
         Include(ids,    on_id);
         Include(ids, multi_id);
      }
      bool newer(C CheckBox &src)C
      {
         return off>src.off || on>src.on || multi>src.multi || normal_color>src.normal_color || disabled_color>src.disabled_color;
      }
      bool equal(C CheckBox &src)C
      {
         return off==src.off && on==src.on && multi==src.multi && normal_color==src.normal_color && disabled_color==src.disabled_color;
      }
      void newData()
      {
         off++; on++; multi++; normal_color++; disabled_color++;
      }
      bool sync(GuiSkin.CheckBox &val, C CheckBox &src, C GuiSkin.CheckBox &src_val)
      {
         bool changed=false;
         changed|=Sync(off, src.off, off_id, src.off_id);
         changed|=Sync(on , src.on , on_id, src.on_id);
         changed|=Sync(multi, src.multi, multi_id, src.multi_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(disabled_color, src.disabled_color, val.disabled_color, src_val.disabled_color);
         return changed;
      }
      bool undo(GuiSkin.CheckBox &val, C CheckBox &src, C GuiSkin.CheckBox &src_val)
      {
         bool changed=false;
         changed|=Undo(off, src.off, off_id, src.off_id);
         changed|=Undo(on , src.on , on_id, src.on_id);
         changed|=Undo(multi, src.multi, multi_id, src.multi_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(disabled_color, src.disabled_color, val.disabled_color, src_val.disabled_color);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<off_id<<on_id<<multi_id
          <<off<<on<<multi
          <<normal_color<<disabled_color;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=CheckBox();}
   }

   class ComboBox : ButtonImage // 'image' is used as the side arrow
   {
      void copyTo(GuiSkin.ComboBox &skin, C Project &proj)C
      {
         super.copyTo(skin, proj);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         super.includeIDs(ids);
      }
      bool newer(C ComboBox &src)C
      {
         return super.newer(src);
      }
      bool equal(C ComboBox &src)C
      {
         return super.equal(src);
      }
      void newData()
      {
         super.newData();
      }
      bool sync(GuiSkin.ComboBox &val, C ComboBox &src, C GuiSkin.ComboBox &src_val)
      {
         bool   changed=super.sync(val, src, src_val);
         return changed;
      }
      bool undo(GuiSkin.ComboBox &val, C ComboBox &src, C GuiSkin.ComboBox &src_val)
      {
         bool   changed=super.undo(val, src, src_val);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         return super.save(f);
      }
      bool load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0: if(super.load(f))
            {
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void reset() {T=ComboBox();}
   }

   class Desc
   {
      UID       normal_id=UIDZero, text_style_id=UIDZero;
      TimeStamp normal,
                normal_color,
                padding,
                text_style;

      void copyTo(GuiSkin.Desc &skin, C Project &proj)C
      {
         skin.normal    =proj.gamePath(normal_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, text_style_id);
      }
      bool newer(C Desc &src)C
      {
         return normal>src.normal || normal_color>src.normal_color || padding>src.padding || text_style>src.text_style;
      }
      bool equal(C Desc &src)C
      {
         return normal==src.normal && normal_color==src.normal_color && padding==src.padding && text_style==src.text_style;
      }
      void newData()
      {
         normal++; normal_color++; padding++; text_style++;
      }
      bool sync(GuiSkin.Desc &val, C Desc &src, C GuiSkin.Desc &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool undo(GuiSkin.Desc &val, C Desc &src, C GuiSkin.Desc &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<text_style_id
          <<normal<<normal_color<<padding<<text_style;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=Desc();}
   }

   class IMM
   {
      UID       normal_id=UIDZero, text_style_id=UIDZero;
      TimeStamp normal,
                normal_color,
                padding,
                text_style;

      void copyTo(GuiSkin.IMM &skin, C Project &proj)C
      {
         skin.normal    =proj.gamePath(normal_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, text_style_id);
      }
      bool newer(C IMM &src)C
      {
         return normal>src.normal || normal_color>src.normal_color || padding>src.padding || text_style>src.text_style;
      }
      bool equal(C IMM &src)C
      {
         return normal==src.normal && normal_color==src.normal_color && padding==src.padding && text_style==src.text_style;
      }
      void newData()
      {
         normal++; normal_color++; padding++; text_style++;
      }
      bool sync(GuiSkin.IMM &val, C IMM &src, C GuiSkin.IMM &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Sync(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool undo(GuiSkin.IMM &val, C IMM &src, C GuiSkin.IMM &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=Undo(padding, src.padding, val.padding, src_val.padding);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<text_style_id
          <<normal<<normal_color<<padding<<text_style;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=IMM();}
   }

   class List
   {
      UID       cursor_id=UIDZero, highlight_id=UIDZero, selection_id=UIDZero, text_style_id=UIDZero, resize_column_id=UIDZero;
      TimeStamp cursor, highlight, selection,
                cursor_color, highlight_color, selection_color,
                text_style, resize_column;
      Button    column;

      void copyTo(GuiSkin.List &skin, C Project &proj)C
      {
         skin.cursor       =proj.gamePath(cursor_id.valid() ? cursor_id : selection_id);
         skin.highlight    =proj.gamePath(highlight_id);
         skin.selection    =proj.gamePath(selection_id.valid() ? selection_id : cursor_id);
         skin.text_style   =proj.gamePath(text_style_id);
         skin.resize_column=proj.gamePath(resize_column_id);
         column.copyTo(skin.column, proj);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, cursor_id);
         Include(ids, highlight_id);
         Include(ids, selection_id);
         Include(ids, text_style_id);
         Include(ids, resize_column_id);
         column.includeIDs(ids);
      }
      bool newer(C List &src)C
      {
         return cursor>src.cursor || highlight>src.highlight || selection>src.selection
             || cursor_color>src.cursor_color || highlight_color>src.highlight_color || selection_color>src.selection_color
             || text_style>src.text_style || resize_column>src.resize_column
             || column.newer(src.column);
      }
      bool equal(C List &src)C
      {
         return cursor==src.cursor && highlight==src.highlight && selection==src.selection
             && cursor_color==src.cursor_color && highlight_color==src.highlight_color && selection_color==src.selection_color
             && text_style==src.text_style && resize_column==src.resize_column
             && column.equal(src.column);
      }
      void newData()
      {
         cursor++; highlight++; selection++;
         cursor_color++; highlight_color++; selection_color++;
         text_style++; resize_column++;
         column.newData();
      }
      bool sync(GuiSkin.List &val, C List &src, C GuiSkin.List &src_val)
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
      bool undo(GuiSkin.List &val, C List &src, C GuiSkin.List &src_val)
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
      bool save(File &f)C
      {
         f.cmpUIntV(1);
         f<<cursor_id<<highlight_id<<selection_id<<text_style_id<<resize_column_id
          <<cursor<<highlight<<selection
          <<cursor_color<<highlight_color<<selection_color
          <<text_style<<resize_column;
         column.save(f);
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=List();}
   }

   class Menu
   {
      UID       normal_id=UIDZero, check_id=UIDZero, sub_menu_id=UIDZero;
      TimeStamp normal,
                normal_color,
                padding,
                list_elm_height,
                check, sub_menu,
                check_color, sub_menu_color;

      void copyTo(GuiSkin.Menu &skin, C Project &proj)C
      {
         skin.normal  =proj.gamePath(normal_id);
         skin.check   =proj.gamePath(check_id);
         skin.sub_menu=proj.gamePath(sub_menu_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, check_id);
         Include(ids, sub_menu_id);
      }
      bool newer(C Menu &src)C
      {
         return normal>src.normal || normal_color>src.normal_color
             || padding>src.padding
             || list_elm_height>src.list_elm_height
             || check>src.check || sub_menu>src.sub_menu
             || check_color>src.check_color || sub_menu_color>src.sub_menu_color;
      }
      bool equal(C Menu &src)C
      {
         return normal==src.normal && normal_color==src.normal_color
             && padding==src.padding
             && list_elm_height==src.list_elm_height
             && check==src.check && sub_menu==src.sub_menu
             && check_color==src.check_color && sub_menu_color==src.sub_menu_color;
      }
      void newData()
      {
         normal++; normal_color++;
         padding++;
         list_elm_height++;
         check++; sub_menu++;
         check_color++; sub_menu_color++;
      }
      bool sync(GuiSkin.Menu &val, C Menu &src, C GuiSkin.Menu &src_val)
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
      bool undo(GuiSkin.Menu &val, C Menu &src, C GuiSkin.Menu &src_val)
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
      bool save(File &f)C
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
      bool load(File &f)
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
      void reset() {T=Menu();}
   }

   class MenuBar
   {
      UID       background_id=UIDZero, highlight_id=UIDZero, text_style_id=UIDZero;
      TimeStamp background, highlight,
                background_color, highlight_color,
                bar_height, text_size, text_padd,
                text_style;

      void copyTo(GuiSkin.MenuBar &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         skin.highlight =proj.gamePath( highlight_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         Include(ids,  highlight_id);
         Include(ids, text_style_id);
      }
      bool newer(C MenuBar &src)C
      {
         return background>src.background || highlight>src.highlight || background_color>src.background_color || highlight_color>src.highlight_color
             || bar_height>src.bar_height || text_size>src.text_size || text_padd>src.text_padd
             || text_style>src.text_style;
      }
      bool equal(C MenuBar &src)C
      {
         return background==src.background && highlight==src.highlight && background_color==src.background_color && highlight_color==src.highlight_color
             && bar_height==src.bar_height && text_size==src.text_size && text_padd==src.text_padd
             && text_style==src.text_style;
      }
      void newData()
      {
         background++; highlight++; background_color++; highlight_color++;
         bar_height++; text_size++; text_padd++;
         text_style++;
      }
      bool sync(GuiSkin.MenuBar &val, C MenuBar &src, C GuiSkin.MenuBar &src_val)
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
      bool undo(GuiSkin.MenuBar &val, C MenuBar &src, C GuiSkin.MenuBar &src_val)
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
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<highlight_id<<text_style_id
          <<background<<highlight
          <<background_color<<highlight_color
          <<bar_height<<text_size<<text_padd
          <<text_style;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=MenuBar();}
   }

   class Progress
   {
      UID       background_id=UIDZero, progress_id=UIDZero, text_style_id=UIDZero;
      TimeStamp background, progress,
                background_color, progress_color,
                text_style, text_size,
                draw_progress_partial;

      void copyTo(GuiSkin.Progress &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         skin.progress  =proj.gamePath(  progress_id);
         skin.text_style=proj.gamePath(text_style_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         Include(ids,   progress_id);
         Include(ids, text_style_id);
      }
      bool newer(C Progress &src)C
      {
         return background>src.background || progress>src.progress
             || background_color>src.background_color || progress_color>src.progress_color
             || text_style>src.text_style || text_size>src.text_size
             || draw_progress_partial>src.draw_progress_partial;
      }
      bool equal(C Progress &src)C
      {
         return background==src.background && progress==src.progress
             && background_color==src.background_color && progress_color==src.progress_color
             && text_style==src.text_style && text_size==src.text_size
             && draw_progress_partial==src.draw_progress_partial;
      }
      void newData()
      {
         background++; progress++;
         background_color++; progress_color++;
         text_style++; text_size++;
         draw_progress_partial++;
      }
      bool sync(GuiSkin.Progress &val, C Progress &src, C GuiSkin.Progress &src_val)
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
      bool undo(GuiSkin.Progress &val, C Progress &src, C GuiSkin.Progress &src_val)
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
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<progress_id<<text_style_id
          <<background<<progress
          <<background_color<<progress_color
          <<text_style<<text_size
          <<draw_progress_partial;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=Progress();}
   }

   class Property
   {
      ButtonImage value;

      void copyTo(GuiSkin.Property &skin, C Project &proj)C
      {
         value.copyTo(skin.value, proj);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         value.includeIDs(ids);
      }
      bool newer(C Property &src)C
      {
         return value.newer(src.value);
      }
      bool equal(C Property &src)C
      {
         return value.equal(src.value);
      }
      void newData()
      {
         value.newData();
      }
      bool sync(GuiSkin.Property &val, C Property &src, C GuiSkin.Property &src_val)
      {
         bool changed=false;
         changed|=value.sync(val.value, src.value, src_val.value);
         return changed;
      }
      bool undo(GuiSkin.Property &val, C Property &src, C GuiSkin.Property &src_val)
      {
         bool changed=false;
         changed|=value.undo(val.value, src.value, src_val.value);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         value.save(f);
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=Property();}
   }

   class Region
   {
      UID         normal_id=UIDZero;
      TimeStamp   normal,
                  normal_color;
      ButtonImage view;

      void copyTo(GuiSkin.Region &skin, C Project &proj)C
      {
         skin.normal=proj.gamePath(normal_id);
         view.copyTo(skin.view, proj);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         view.includeIDs(ids);
      }
      bool newer(C Region &src)C
      {
         return normal>src.normal || normal_color>src.normal_color || view.newer(src.view);
      }
      bool equal(C Region &src)C
      {
         return normal==src.normal && normal_color==src.normal_color && view.equal(src.view);
      }
      void newData()
      {
         normal++; normal_color++;
         view.newData();
      }
      bool sync(GuiSkin.Region &val, C Region &src, C GuiSkin.Region &src_val)
      {
         bool changed=false;
         changed|=Sync(normal, src.normal, normal_id, src.normal_id);
         changed|=Sync(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=view.sync(val.view, src.view, src_val.view);
         return changed;
      }
      bool undo(GuiSkin.Region &val, C Region &src, C GuiSkin.Region &src_val)
      {
         bool changed=false;
         changed|=Undo(normal, src.normal, normal_id, src.normal_id);
         changed|=Undo(normal_color, src.normal_color, val.normal_color, src_val.normal_color);
         changed|=view.undo(val.view, src.view, src_val.view);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<normal_id<<normal<<normal_color;
         view.save(f);
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=Region();}
   }

   class SlideBar
   {
      UID         background_id=UIDZero;
      TimeStamp   background, background_color;
      ButtonImage left, center, right;

      void copyTo(GuiSkin.SlideBar &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         left  .copyTo(skin.left  , proj);
         center.copyTo(skin.center, proj);
         right .copyTo(skin.right , proj);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         left  .includeIDs(ids);
         center.includeIDs(ids);
         right .includeIDs(ids);
      }
      bool newer(C SlideBar &src)C
      {
         return background>src.background || background_color>src.background_color || left.newer(src.left) || center.newer(src.center) || right.newer(src.right);
      }
      bool equal(C SlideBar &src)C
      {
         return background==src.background && background_color==src.background_color && left.equal(src.left) && center.equal(src.center) && right.equal(src.right);
      }
      void newData()
      {
         background++; background_color++; left.newData(); center.newData(); right.newData();
      }
      bool sync(GuiSkin.SlideBar &val, C SlideBar &src, C GuiSkin.SlideBar &src_val)
      {
         bool changed=false;
         changed|=Sync(background, src.background, background_id, src.background_id);
         changed|=left.sync(val.left, src.left, src_val.left);
         changed|=center.sync(val.center, src.center, src_val.center);
         changed|=right.sync(val.right, src.right, src_val.right);
         return changed;
      }
      bool undo(GuiSkin.SlideBar &val, C SlideBar &src, C GuiSkin.SlideBar &src_val)
      {
         bool changed=false;
         changed|=Undo(background, src.background, background_id, src.background_id);
         changed|=left.undo(val.left, src.left, src_val.left);
         changed|=center.undo(val.center, src.center, src_val.center);
         changed|=right.undo(val.right, src.right, src_val.right);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<background<<background_color;
         left  .save(f);
         center.save(f);
         right .save(f);
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=SlideBar();}
   }

   class Slider
   {
      UID       background_id=UIDZero, progress_id=UIDZero, slider_id=UIDZero;
      TimeStamp background, progress, slider,
                background_color, progress_color, slider_color,
                background_shrink,
                draw_progress_partial;

      void copyTo(GuiSkin.Slider &skin, C Project &proj)C
      {
         skin.background=proj.gamePath(background_id);
         skin.progress  =proj.gamePath(  progress_id);
         skin.slider    =proj.gamePath(    slider_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, background_id);
         Include(ids,   progress_id);
         Include(ids,     slider_id);
      }
      bool newer(C Slider &src)C
      {
         return background>src.background || progress>src.progress || slider>src.slider
             || background_color>src.background_color || progress_color>src.progress_color || slider_color>src.slider_color
             || background_shrink>src.background_shrink
             || draw_progress_partial>src.draw_progress_partial;
      }
      bool equal(C Slider &src)C
      {
         return background==src.background && progress==src.progress && slider==src.slider
             && background_color==src.background_color && progress_color==src.progress_color && slider_color==src.slider_color
             && background_shrink==src.background_shrink
             && draw_progress_partial==src.draw_progress_partial;
      }
      void newData()
      {
         background++; progress++; slider++;
         background_color++; progress_color++; slider_color++;
         background_shrink++;
         draw_progress_partial++;
      }
      bool sync(GuiSkin.Slider &val, C Slider &src, C GuiSkin.Slider &src_val)
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
      bool undo(GuiSkin.Slider &val, C Slider &src, C GuiSkin.Slider &src_val)
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
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<background_id<<progress_id<<slider_id
          <<background<<progress<<slider
          <<background_color<<progress_color<<slider_color
          <<background_shrink
          <<draw_progress_partial;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=Slider();}
   }

   class Tab : Button
   {
      class Kind
      {
         UID       normal_id=UIDZero, pushed_id=UIDZero;
         TimeStamp normal, pushed;

         Kind(C Button &button) : normal_id(button.normal_id), pushed_id(button.pushed_id) {}
         bool valid()C {return normal_id.valid() || pushed_id.valid();}
         void copyTo(Button &button)C
         {
            button.normal_id=normal_id;
            button.pushed_id=pushed_id;
         }
         bool apply (Kind   &kind  )C {if(valid()){kind=T;         return true;} return false;}
         bool apply (Button &button)C {if(valid()){copyTo(button); return true;} return false;}
         void includeIDs(MemPtr<UID> ids)C
         {
            Include(ids, normal_id);
            Include(ids, pushed_id);
         }
         bool newer(C Kind &src)C {return normal> src.normal || pushed> src.pushed;}
         bool equal(C Kind &src)C {return normal==src.normal && pushed==src.pushed;}
         void newData() {normal++; pushed++;}
         bool sync(C Kind &src)
         {
            bool changed=false;
            changed|=Sync(normal, src.normal, normal_id, src.normal_id);
            changed|=Sync(pushed, src.pushed, pushed_id, src.pushed_id);
            return changed;
         }
         bool undo(C Kind &src)
         {
            bool changed=false;
            changed|=Undo(normal, src.normal, normal_id, src.normal_id);
            changed|=Undo(pushed, src.pushed, pushed_id, src.pushed_id);
            return changed;
         }
         bool save(File &f)C
         {
            f.cmpUIntV(0);
            f<<normal_id<<pushed_id<<normal<<pushed;
            return f.ok();
         }
         bool load(File &f)
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
         void reset() {T=Kind();}
      }

      Kind left, horizontal, right, top, vertical, bottom, top_left, top_right, bottom_left, bottom_right;

      void copyTo(GuiSkin.Tab &skin, C Project &proj)C
      {
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         super.includeIDs(ids);
         left.includeIDs(ids); horizontal.includeIDs(ids); right.includeIDs(ids); top.includeIDs(ids); vertical.includeIDs(ids); bottom.includeIDs(ids);
         top_left.includeIDs(ids); top_right.includeIDs(ids); bottom_left.includeIDs(ids); bottom_right.includeIDs(ids);
      }
      bool newer(C Tab &src)C
      {
         return super.newer(src)
             || left.newer(src.left) || horizontal.newer(src.horizontal) || right.newer(src.right)
             || top.newer(src.top) || vertical.newer(src.vertical) || bottom.newer(src.bottom)
             || top_left.newer(src.top_left) || top_right.newer(src.top_right) || bottom_left.newer(src.bottom_left) || bottom_right.newer(src.bottom_right);
      }
      bool equal(C Tab &src)C
      {
         return super.equal(src)
             && left.equal(src.left) && horizontal.equal(src.horizontal) && right.equal(src.right)
             && top.equal(src.top) && vertical.equal(src.vertical) && bottom.equal(src.bottom)
             && top_left.equal(src.top_left) && top_right.equal(src.top_right) && bottom_left.equal(src.bottom_left) && bottom_right.equal(src.bottom_right);
      }
      void newData()
      {
         super.newData();
         left.newData(); horizontal.newData(); right.newData(); top.newData(); vertical.newData(); bottom.newData();
         top_left.newData(); top_right.newData(); bottom_left.newData(); bottom_right.newData();
      }
      bool sync(GuiSkin.Tab &val, C Tab &src, C GuiSkin.Tab &src_val)
      {
         bool changed=super   .sync(val.left, src, src_val.left);
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
      bool undo(GuiSkin.Tab &val, C Tab &src, C GuiSkin.Tab &src_val)
      {
         bool changed=super   .undo(val.left, src, src_val.left);
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
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         super       .save(f);
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
      bool load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 0: if(super.load(f))
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
      void reset() {T=Tab();}
   }

   class Text
   {
      UID       text_style_id=UIDZero;
      TimeStamp text_style;

      void copyTo(GuiSkin.Text &skin, C Project &proj)C
      {
         skin.text_style=proj.gamePath(text_style_id);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, text_style_id);
      }
      bool newer(C Text &src)C
      {
         return text_style>src.text_style;
      }
      bool equal(C Text &src)C
      {
         return text_style==src.text_style;
      }
      void newData()
      {
         text_style++;
      }
      bool sync(GuiSkin.Text &val, C Text &src, C GuiSkin.Text &src_val)
      {
         bool changed=false;
         changed|=Sync(text_style, src.text_style, text_style_id, src.text_style_id);
         return changed;
      }
      bool undo(GuiSkin.Text &val, C Text &src, C GuiSkin.Text &src_val)
      {
         bool changed=false;
         changed|=Undo(text_style, src.text_style, text_style_id, src.text_style_id);
         return changed;
      }
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<text_style_id<<text_style;
         return f.ok();
      }
      bool load(File &f)
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
      void reset() {T=Text();}
   }

   class TextLine
   {
      UID         normal_id=UIDZero, disabled_id=UIDZero, text_style_id=UIDZero, find_image_id=UIDZero;
      TimeStamp   normal, disabled,
                  normal_panel_color, normal_text_color, disabled_panel_color, disabled_text_color, rect_color,
                  text_size,
                  text_style, 
                  find_image;
      ButtonImage clear;

      void copyTo(GuiSkin.TextLine &skin, C Project &proj)C
      {
         skin.normal=proj.gamePath(normal_id);
         skin.disabled=proj.gamePath(disabled_id.valid() ? disabled_id : normal_id);
         skin.text_style=proj.gamePath(text_style_id);
         skin.find_image=proj.gamePath(find_image_id);
         clear.copyTo(skin.clear, proj);
      }
      void includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, normal_id);
         Include(ids, disabled_id);
         Include(ids, text_style_id);
         Include(ids, find_image_id);
         clear.includeIDs(ids);
      }
      bool newer(C TextLine &src)C
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
      bool equal(C TextLine &src)C
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
      void newData()
      {
         normal++; disabled++;
         normal_panel_color++; normal_text_color++; disabled_panel_color++; disabled_text_color++;
         rect_color++;
         text_size++;
         text_style++;
         find_image++;
         clear.newData();
      }
      bool sync(GuiSkin.TextLine &val, C TextLine &src, C GuiSkin.TextLine &src_val)
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
      bool undo(GuiSkin.TextLine &val, C TextLine &src, C GuiSkin.TextLine &src_val)
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
      bool save(File &f)C
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
      bool load(File &f)
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
      void reset() {T=TextLine();}
   }

   class Window
   {
      UID         normal_id=UIDZero, active_id=UIDZero, normal_no_bar_id=UIDZero, active_no_bar_id=UIDZero,
                  normal_text_style_id=UIDZero, active_text_style_id=UIDZero;
      TimeStamp   normal, active, normal_no_bar, active_no_bar,
                  normal_color, active_color,
                  normal_text_style, active_text_style,
                  text_size, text_padd, button_offset;
      ButtonImage minimize, maximize, close;

      void copyTo(GuiSkin.Window &skin, C Project &proj)C
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
      void includeIDs(MemPtr<UID> ids)C
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
      bool newer(C Window &src)C
      {
         return normal>src.normal || active>src.active || normal_no_bar>src.normal_no_bar || active_no_bar>src.active_no_bar
             || normal_color>src.normal_color || active_color>src.active_color
             || normal_text_style>src.normal_text_style || active_text_style>src.active_text_style
             || text_size>src.text_size || text_padd>src.text_padd || button_offset>src.button_offset
             || minimize.newer(src.minimize) || maximize.newer(src.maximize) || close.newer(src.close);
      }
      bool equal(C Window &src)C
      {
         return normal==src.normal && active==src.active && normal_no_bar==src.normal_no_bar && active_no_bar==src.active_no_bar
             && normal_color==src.normal_color && active_color==src.active_color
             && normal_text_style==src.normal_text_style && active_text_style==src.active_text_style
             && text_size==src.text_size && text_padd==src.text_padd && button_offset==src.button_offset
             && minimize.equal(src.minimize) && maximize.equal(src.maximize) && close.equal(src.close);
      }
      void newData()
      {
         normal++; active++; normal_no_bar++; active_no_bar++;
         normal_color++; active_color++;
         normal_text_style++; active_text_style++;
         text_size++; text_padd++; button_offset++;
         minimize.newData(); maximize.newData(); close.newData();
      }
      bool sync(GuiSkin.Window &val, C Window &src, C GuiSkin.Window &src_val)
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
      bool undo(GuiSkin.Window &val, C Window &src, C GuiSkin.Window &src_val)
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
      bool save(File &f)C
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
      bool load(File &f)
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
      void reset() {T=Window();}
   }

   class Base
   {
      UID       font_id=UIDZero, text_style_id=UIDZero;
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
      
      void includeIDs(MemPtr<UID> ids)C
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
      void copyTo(GuiSkin &skin, C Project &proj)C
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
            Tab.Kind hor, ver, any;
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
      bool newer(C Base &src)C
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
      bool equal(C Base &src)C
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
      void newData()
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
      bool sync(GuiSkin &val, C Base &src, C GuiSkin &src_val)
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
      bool undo(GuiSkin &val, C Base &src, C GuiSkin &src_val)
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
      bool save(File &f)C
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
      bool load(File &f)
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
      void reset() {T=Base();}
   }

   Base base;

   void reset  (                  )  {T=EditGuiSkin();}
   void newData(                  )  {       base.newData();}
   bool newer  (C EditGuiSkin &src)C {return base.newer(src.base);}
   bool equal  (C EditGuiSkin &src)C {return base.equal(src.base);}
   bool sync   (C EditGuiSkin &src)  {bool changed=base.sync(T, src.base, src); return changed;}
   bool undo   (C EditGuiSkin &src)  {bool changed=base.undo(T, src.base, src); return changed;}
   void copyTo (      GuiSkin &skin, C Project &proj)C {skin=T; base.copyTo(skin, proj);} // call 'copyTo' after assignment to fixup what's missing

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(0);
      super.save(f);
      base .save(f);
      return f.ok();
   }
   bool load(File &f)
   {
      reset(); switch(f.decUIntV())
      {
         case 0: if(super.load(f))
         {
            if(base.load(f))
               if(f.ok())return true;
         }break;
      }
      reset(); return false;
   }
   bool load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
}
/******************************************************************************/
