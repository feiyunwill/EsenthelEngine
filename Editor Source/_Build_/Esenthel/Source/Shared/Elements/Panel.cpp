/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   bool EditPanel::newer(C EditPanel &src)C
   {
      return center_stretch_time>src.center_stretch_time || side_stretch_time>src.side_stretch_time || center_shadow_time>src.center_shadow_time
          || center_color_time>src.center_color_time || border_color_time>src.border_color_time || side_color_time>src.side_color_time || blur_color_time>src.blur_color_time
          || shadow_opacity_time>src.shadow_opacity_time || shadow_radius_time>src.shadow_radius_time || shadow_offset_time>src.shadow_offset_time
          || border_size_time>src.border_size_time || center_scale_time>src.center_scale_time || top_size_time>src.top_size_time || bottom_size_time>src.bottom_size_time || left_right_size_time>src.left_right_size_time || top_corner_size_time>src.top_corner_size_time || bottom_corner_size_time>src.bottom_corner_size_time
          || top_offset_time>src.top_offset_time || bottom_offset_time>src.bottom_offset_time || left_right_offset_time>src.left_right_offset_time || top_corner_offset_time>src.top_corner_offset_time || bottom_corner_offset_time>src.bottom_corner_offset_time
          || center_image_time>src.center_image_time || border_image_time>src.border_image_time || top_image_time>src.top_image_time || bottom_image_time>src.bottom_image_time || left_right_image_time>src.left_right_image_time || top_corner_image_time>src.top_corner_image_time || bottom_corner_image_time>src.bottom_corner_image_time
          || panel_image_time>src.panel_image_time;
   }
   bool EditPanel::equal(C EditPanel &src)C
   {
      return center_stretch_time==src.center_stretch_time && side_stretch_time==src.side_stretch_time && center_shadow_time==src.center_shadow_time
          && center_color_time==src.center_color_time && border_color_time==src.border_color_time && side_color_time==src.side_color_time && blur_color_time==src.blur_color_time
          && shadow_opacity_time==src.shadow_opacity_time && shadow_radius_time==src.shadow_radius_time && shadow_offset_time==src.shadow_offset_time
          && border_size_time==src.border_size_time && center_scale_time==src.center_scale_time && top_size_time==src.top_size_time && bottom_size_time==src.bottom_size_time && left_right_size_time==src.left_right_size_time && top_corner_size_time==src.top_corner_size_time && bottom_corner_size_time==src.bottom_corner_size_time
          && top_offset_time==src.top_offset_time && bottom_offset_time==src.bottom_offset_time && left_right_offset_time==src.left_right_offset_time && top_corner_offset_time==src.top_corner_offset_time && bottom_corner_offset_time==src.bottom_corner_offset_time
          && center_image_time==src.center_image_time && border_image_time==src.border_image_time && top_image_time==src.top_image_time && bottom_image_time==src.bottom_image_time && left_right_image_time==src.left_right_image_time && top_corner_image_time==src.top_corner_image_time && bottom_corner_image_time==src.bottom_corner_image_time
          && panel_image_time==src.panel_image_time;
   }
   void EditPanel::newData()
   {
      center_stretch_time++; side_stretch_time++; center_shadow_time++;
      center_color_time++; border_color_time++; side_color_time++; blur_color_time++;
      shadow_opacity_time++; shadow_radius_time++; shadow_offset_time++;
      border_size_time++; center_scale_time++; top_size_time++; bottom_size_time++; left_right_size_time++; top_corner_size_time++; bottom_corner_size_time++;
      top_offset_time++; bottom_offset_time++; left_right_offset_time++; top_corner_offset_time++; bottom_corner_offset_time++;
      center_image_time++; border_image_time++; top_image_time++; bottom_image_time++; left_right_image_time++; top_corner_image_time++; bottom_corner_image_time++;
      panel_image_time++;
   }
   void EditPanel::includeIDs(MemPtr<UID> ids)C
   {
      Include(ids, center_image); Include(ids, border_image); Include(ids, top_image); Include(ids, bottom_image);
      Include(ids, left_right_image); Include(ids, top_corner_image); Include(ids, bottom_corner_image); Include(ids, panel_image);
   }
   bool EditPanel::sync(C EditPanel &src)
   {
      bool changed=false;
      changed|=Sync(center_stretch_time, src.center_stretch_time, center_stretch, src.center_stretch);
      changed|=Sync(  side_stretch_time, src.  side_stretch_time,   side_stretch, src.  side_stretch);
      changed|=Sync( center_shadow_time, src. center_shadow_time,  center_shadow, src. center_shadow);

      changed|=Sync(center_color_time, src.center_color_time, center_color, src.center_color);
      changed|=Sync(border_color_time, src.border_color_time, border_color, src.border_color);
      changed|=Sync(  side_color_time, src.  side_color_time,   side_color, src.  side_color);
      changed|=Sync(  blur_color_time, src.  blur_color_time,   blur_color, src.  blur_color);
      
      changed|=Sync(shadow_opacity_time, src.shadow_opacity_time, shadow_opacity, src.shadow_opacity);
      changed|=Sync( shadow_radius_time, src. shadow_radius_time, shadow_radius , src.shadow_radius );
      changed|=Sync( shadow_offset_time, src. shadow_offset_time, shadow_offset , src.shadow_offset );

      changed|=Sync(       border_size_time, src.       border_size_time,        border_size, src.       border_size);
      changed|=Sync(      center_scale_time, src.      center_scale_time,       center_scale, src.      center_scale);
      changed|=Sync(          top_size_time, src.          top_size_time,           top_size, src.          top_size);
      changed|=Sync(       bottom_size_time, src.       bottom_size_time,        bottom_size, src.       bottom_size);
      changed|=Sync(   left_right_size_time, src.   left_right_size_time,    left_right_size, src.   left_right_size);
      changed|=Sync(   top_corner_size_time, src.   top_corner_size_time,    top_corner_size, src.   top_corner_size);
      changed|=Sync(bottom_corner_size_time, src.bottom_corner_size_time, bottom_corner_size, src.bottom_corner_size);

      changed|=Sync(          top_offset_time, src.          top_offset_time,           top_offset, src.          top_offset);
      changed|=Sync(       bottom_offset_time, src.       bottom_offset_time,        bottom_offset, src.       bottom_offset);
      changed|=Sync(   left_right_offset_time, src.   left_right_offset_time,    left_right_offset, src.   left_right_offset);
      changed|=Sync(   top_corner_offset_time, src.   top_corner_offset_time,    top_corner_offset, src.   top_corner_offset);
      changed|=Sync(bottom_corner_offset_time, src.bottom_corner_offset_time, bottom_corner_offset, src.bottom_corner_offset);

      changed|=Sync(       center_image_time, src.       center_image_time,        center_image, src.       center_image);
      changed|=Sync(       border_image_time, src.       border_image_time,        border_image, src.       border_image);
      changed|=Sync(          top_image_time, src.          top_image_time,           top_image, src.          top_image);
      changed|=Sync(       bottom_image_time, src.       bottom_image_time,        bottom_image, src.       bottom_image);
      changed|=Sync(   left_right_image_time, src.   left_right_image_time,    left_right_image, src.   left_right_image);
      changed|=Sync(   top_corner_image_time, src.   top_corner_image_time,    top_corner_image, src.   top_corner_image);
      changed|=Sync(bottom_corner_image_time, src.bottom_corner_image_time, bottom_corner_image, src.bottom_corner_image);

      changed|=Sync(panel_image_time, src.panel_image_time, panel_image, src.panel_image);
      return changed;
   }
   bool EditPanel::undo(C EditPanel &src)
   {
      bool changed=false;
      changed|=Undo(center_stretch_time, src.center_stretch_time, center_stretch, src.center_stretch);
      changed|=Undo(  side_stretch_time, src.  side_stretch_time,   side_stretch, src.  side_stretch);
      changed|=Undo( center_shadow_time, src. center_shadow_time,  center_shadow, src. center_shadow);

      changed|=Undo(center_color_time, src.center_color_time, center_color, src.center_color);
      changed|=Undo(border_color_time, src.border_color_time, border_color, src.border_color);
      changed|=Undo(  side_color_time, src.  side_color_time,   side_color, src.  side_color);
      changed|=Undo(  blur_color_time, src.  blur_color_time,   blur_color, src.  blur_color);
      
      changed|=Undo(shadow_opacity_time, src.shadow_opacity_time, shadow_opacity, src.shadow_opacity);
      changed|=Undo( shadow_radius_time, src. shadow_radius_time, shadow_radius , src.shadow_radius );
      changed|=Undo( shadow_offset_time, src. shadow_offset_time, shadow_offset , src.shadow_offset );

      changed|=Undo(       border_size_time, src.       border_size_time,        border_size, src.       border_size);
      changed|=Undo(      center_scale_time, src.      center_scale_time,       center_scale, src.      center_scale);
      changed|=Undo(          top_size_time, src.          top_size_time,           top_size, src.          top_size);
      changed|=Undo(       bottom_size_time, src.       bottom_size_time,        bottom_size, src.       bottom_size);
      changed|=Undo(   left_right_size_time, src.   left_right_size_time,    left_right_size, src.   left_right_size);
      changed|=Undo(   top_corner_size_time, src.   top_corner_size_time,    top_corner_size, src.   top_corner_size);
      changed|=Undo(bottom_corner_size_time, src.bottom_corner_size_time, bottom_corner_size, src.bottom_corner_size);

      changed|=Undo(          top_offset_time, src.          top_offset_time,           top_offset, src.          top_offset);
      changed|=Undo(       bottom_offset_time, src.       bottom_offset_time,        bottom_offset, src.       bottom_offset);
      changed|=Undo(   left_right_offset_time, src.   left_right_offset_time,    left_right_offset, src.   left_right_offset);
      changed|=Undo(   top_corner_offset_time, src.   top_corner_offset_time,    top_corner_offset, src.   top_corner_offset);
      changed|=Undo(bottom_corner_offset_time, src.bottom_corner_offset_time, bottom_corner_offset, src.bottom_corner_offset);

      changed|=Undo(       center_image_time, src.       center_image_time,        center_image, src.       center_image);
      changed|=Undo(       border_image_time, src.       border_image_time,        border_image, src.       border_image);
      changed|=Undo(          top_image_time, src.          top_image_time,           top_image, src.          top_image);
      changed|=Undo(       bottom_image_time, src.       bottom_image_time,        bottom_image, src.       bottom_image);
      changed|=Undo(   left_right_image_time, src.   left_right_image_time,    left_right_image, src.   left_right_image);
      changed|=Undo(   top_corner_image_time, src.   top_corner_image_time,    top_corner_image, src.   top_corner_image);
      changed|=Undo(bottom_corner_image_time, src.bottom_corner_image_time, bottom_corner_image, src.bottom_corner_image);

      changed|=Undo(panel_image_time, src.panel_image_time, panel_image, src.panel_image);
      return changed;
   }
   void EditPanel::reset() {T=EditPanel();}
   void EditPanel::create(C Panel &src, C UID &center_image, C UID &border_image, C UID &top_image, C UID &bottom_image, C UID &left_right_image, C UID &top_corner_image, C UID &bottom_corner_image, C UID &panel_image, C TimeStamp &time)
   {
      SCAST(Panel, T)=src;
      ::EE::Panel::center_image=::EE::Panel::border_image=::EE::Panel::top_image=::EE::Panel::bottom_image=::EE::Panel::left_right_image=::EE::Panel::top_corner_image=::EE::Panel::bottom_corner_image=null; ::EE::Panel::panel_image=null;
          T.center_image=center_image; T.border_image=border_image; T.top_image=top_image; T.bottom_image=bottom_image; T.left_right_image=left_right_image; T.top_corner_image=top_corner_image; T.bottom_corner_image=bottom_corner_image; T.panel_image=panel_image;
      center_stretch_time=side_stretch_time
         =center_color_time=border_color_time=side_color_time=blur_color_time
         =shadow_opacity_time=shadow_radius_time=shadow_offset_time
         =border_size_time=center_scale_time=top_size_time=bottom_size_time=left_right_size_time=top_corner_size_time=bottom_corner_size_time
         =top_offset_time=bottom_offset_time=left_right_offset_time=top_corner_offset_time=bottom_corner_offset_time
         =center_image_time=border_image_time=top_image_time=bottom_image_time=left_right_image_time=top_corner_image_time=bottom_corner_image_time
         =panel_image_time=time;
   }
   void EditPanel::copyTo(Panel &panel, C Project &proj)C
   {
      panel=T;
      panel.       center_image=proj.gamePath(       center_image);
      panel.       border_image=proj.gamePath(       border_image);
      panel.          top_image=proj.gamePath(          top_image);
      panel.       bottom_image=proj.gamePath(       bottom_image);
      panel.   left_right_image=proj.gamePath(   left_right_image);
      panel.   top_corner_image=proj.gamePath(   top_corner_image);
      panel.bottom_corner_image=proj.gamePath(bottom_corner_image);
      panel.        panel_image=proj.gamePath(        panel_image);
      if(!Equal(panel.shadow_offset, 0))panel.center_shadow=true; // if the user requested shadow offset then 'center_shadow' must be enabled
   }
   bool EditPanel::save(File &f)C
   {
      f.cmpUIntV(2);
      ::EE::Panel::save(f);
      f<<center_image<<border_image<<top_image<<bottom_image<<left_right_image<<top_corner_image<<bottom_corner_image
       <<panel_image
       <<center_stretch_time<<side_stretch_time<<center_shadow_time
       <<center_color_time<<border_color_time<<side_color_time<<blur_color_time
       <<shadow_opacity_time<<shadow_radius_time<<shadow_offset_time
       <<border_size_time<<center_scale_time<<top_size_time<<bottom_size_time<<left_right_size_time<<top_corner_size_time<<bottom_corner_size_time
       <<top_offset_time<<bottom_offset_time<<left_right_offset_time<<top_corner_offset_time<<bottom_corner_offset_time
       <<center_image_time<<border_image_time<<top_image_time<<bottom_image_time<<left_right_image_time<<top_corner_image_time<<bottom_corner_image_time
       <<panel_image_time;
      return f.ok();
   }
   bool EditPanel::load(File &f)
   {
      reset(); switch(f.decUIntV())
      {
         case 2:
         {
            if(!::EE::Panel::load(f))break;
            f>>center_image>>border_image>>top_image>>bottom_image>>left_right_image>>top_corner_image>>bottom_corner_image
             >>panel_image
             >>center_stretch_time>>side_stretch_time>>center_shadow_time
             >>center_color_time>>border_color_time>>side_color_time>>blur_color_time
             >>shadow_opacity_time>>shadow_radius_time>>shadow_offset_time
             >>border_size_time>>center_scale_time>>top_size_time>>bottom_size_time>>left_right_size_time>>top_corner_size_time>>bottom_corner_size_time
             >>top_offset_time>>bottom_offset_time>>left_right_offset_time>>top_corner_offset_time>>bottom_corner_offset_time
             >>center_image_time>>border_image_time>>top_image_time>>bottom_image_time>>left_right_image_time>>top_corner_image_time>>bottom_corner_image_time
             >>panel_image_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            if(!::EE::Panel::load(f))break;
            f>>center_image>>border_image>>top_corner_image>>top_image>>bottom_image>>left_right_image
             >>center_stretch_time>>shadow_opacity_time>>center_color_time>>border_color_time>>blur_color_time>>shadow_offset_time>>shadow_radius_time>>border_size_time
             >>center_scale_time>>top_corner_size_time>>top_size_time>>bottom_size_time>>left_right_size_time
             >>top_corner_offset_time>>top_offset_time>>bottom_offset_time>>left_right_offset_time
             >>center_image_time>>border_image_time>>top_corner_image_time>>top_image_time>>bottom_image_time>>left_right_image_time;
             bottom_corner_image=top_corner_image; bottom_corner_image_time=top_corner_image_time; bottom_corner_size_time=top_corner_size_time; bottom_corner_offset_time=top_corner_offset_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            if(!::EE::Panel::load(f))break;
            f>>center_image>>border_image>>top_corner_image>>top_image
             >>center_stretch_time>>shadow_opacity_time>>center_color_time>>border_color_time>>blur_color_time>>shadow_offset_time>>shadow_radius_time>>border_size_time
             >>center_scale_time>>top_corner_size_time>>top_size_time>>top_corner_offset_time>>top_offset_time>>center_image_time>>border_image_time>>top_corner_image_time>>top_image_time;
            bottom_corner_image=top_corner_image; bottom_corner_image_time=top_corner_image_time; bottom_corner_size_time=top_corner_size_time; bottom_corner_offset_time=top_corner_offset_time;
            if(f.ok())return true;
         }break;
      }
      reset(); return false;
   }
   bool EditPanel::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
EditPanel::EditPanel() : center_image(UIDZero), border_image(UIDZero), top_image(UIDZero), bottom_image(UIDZero), left_right_image(UIDZero), top_corner_image(UIDZero), bottom_corner_image(UIDZero), panel_image(UIDZero) {}

/******************************************************************************/
