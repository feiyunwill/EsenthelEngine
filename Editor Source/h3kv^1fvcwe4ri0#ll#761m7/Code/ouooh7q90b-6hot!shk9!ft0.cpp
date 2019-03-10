/******************************************************************************/
class EditTextStyle : TextStyle
{
   UID       font=UIDZero;
   TimeStamp shadow_time, shade_time, color_time, selection_time, align_time, size_time, space_time, font_time;

   // get
   bool newer(C EditTextStyle &src)C
   {
      return shadow_time>src.shadow_time || shade_time>src.shade_time || color_time>src.color_time || selection_time>src.selection_time || align_time>src.align_time || size_time>src.size_time || space_time>src.space_time || font_time>src.font_time;
   }
   bool equal(C EditTextStyle &src)C
   {
      return shadow_time==src.shadow_time && shade_time==src.shade_time && color_time==src.color_time && selection_time==src.selection_time && align_time==src.align_time && size_time==src.size_time && space_time==src.space_time && font_time==src.font_time;
   }

   // operations
   void newData()
   {
      shadow_time++; shade_time++; color_time++; selection_time++; align_time++; size_time++; space_time++; font_time++;
   }
   bool sync(C EditTextStyle &src)
   {
      bool changed=false;
      changed|=Sync(   shadow_time, src.    shadow_time, shadow    , src.shadow    );
      changed|=Sync(    shade_time, src.     shade_time, shade     , src.shade     );
      changed|=Sync(    color_time, src.     color_time, color     , src.color     );
      changed|=Sync(selection_time, src. selection_time, selection , src.selection );
      changed|=Sync(    align_time, src.     align_time, align     , src.align     );
      changed|=Sync(     size_time, src.      size_time, size      , src.size      );
      changed|=Sync(    space_time, src.     space_time, space     , src.space     );
      changed|=Sync(     font_time, src.      font_time, font      , src.font      );
      return changed;
   }
   bool undo(C EditTextStyle &src)
   {
      bool changed=false;
      changed|=Undo(   shadow_time, src.    shadow_time, shadow    , src.shadow    );
      changed|=Undo(    shade_time, src.     shade_time, shade     , src.shade     );
      changed|=Undo(    color_time, src.     color_time, color     , src.color     );
      changed|=Undo(selection_time, src. selection_time, selection , src.selection );
      changed|=Undo(    align_time, src.     align_time, align     , src.align     );
      changed|=Undo(     size_time, src.      size_time, size      , src.size      );
      changed|=Undo(    space_time, src.     space_time, space     , src.space     );
      changed|=Undo(     font_time, src.      font_time, font      , src.font      );
      return changed;
   }
   void reset() {T=EditTextStyle();}
   void create(C TextStyle &src, C UID &font, C TimeStamp &time=TimeStamp().getUTC())
   {
      SCAST(TextStyle, T)=src;
      super.font(null);
          T.font=font;
      shadow_time=shade_time=color_time=selection_time=align_time=size_time=space_time=font_time=time;
   }
   void copyTo(TextStyle &dest, C Project &proj)C
   {
      dest=T;
      dest.font(proj.gamePath(font));
   }

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(1);
      super.save(f);
      f<<font<<shadow_time<<shade_time<<color_time<<selection_time<<align_time<<size_time<<space_time<<font_time;
      return f.ok();
   }
   bool load(File &f)
   {
      reset(); switch(f.decUIntV())
      {
         case 1: if(super.load(f))
         {
            f>>font>>shadow_time>>shade_time>>color_time>>selection_time>>align_time>>size_time>>space_time>>font_time;
            if(f.ok())return true;
         }break;

         case 0: if(super.load(f))
         {
            f>>font>>shadow_time>>shade_time>>color_time>>align_time>>size_time>>space_time>>font_time;
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
