/******************************************************************************/
class IconSettings
{
   bool          mip_maps      =false,
                 auto_center   =true,
                 light0_shadow =true,
                 light1_shadow =true;
   ElmImage.TYPE type          =ElmImage.COMPRESSED;
   int           width         =128,
                 height        =128;
   flt           scale         =1,
                 fov           =PI_6,
                 ambient_occl  =1.2,
                 ambient_range =0.1,
                 bloom_original=1.0,
                 bloom_scale   =0.5,
                 bloom_cut     =0.3;
   Vec           cam_angle     =0,
                 cam_focus     =0,
                 ambient_col   =0.4,
                 light0_col    =0.7,
                 light1_col    =0.0;
   Vec2          light0_angle  ( PI_4,  PI_4),
                 light1_angle  (-PI_4, -PI_4);
   TimeStamp     mip_maps_time, auto_center_time, light0_shadow_time, light1_shadow_time, type_time, width_time, height_time, scale_time, fov_time,
                 cam_angle_time, cam_focus_time,
                 ambient_col_time, ambient_occl_time, ambient_range_time,
                 bloom_original_time, bloom_scale_time, bloom_cut_time,
                 light0_col_time, light1_col_time, light0_angle_time, light1_angle_time;

   bool newer(C IconSettings &src)C
   {
      return mip_maps_time>src.mip_maps_time || auto_center_time>src.auto_center_time || light0_shadow_time>src.light0_shadow_time || light1_shadow_time>src.light1_shadow_time || type_time>src.type_time
          || width_time>src.width_time || height_time>src.height_time || scale_time>src.scale_time || fov_time>src.fov_time
          || cam_angle_time>src.cam_angle_time || cam_focus_time>src.cam_focus_time
          || ambient_col_time>src.ambient_col_time || ambient_occl_time>src.ambient_occl_time || ambient_range_time>src.ambient_range_time
          || bloom_original_time>src.bloom_original_time || bloom_scale_time>src.bloom_scale_time || bloom_cut_time>src.bloom_cut_time
          || light0_col_time>src.light0_col_time || light1_col_time>src.light1_col_time || light0_angle_time>src.light0_angle_time || light1_angle_time>src.light1_angle_time;
   }
   bool equal(C IconSettings &src)C
   {
      return mip_maps_time==src.mip_maps_time && auto_center_time==src.auto_center_time && light0_shadow_time==src.light0_shadow_time && light1_shadow_time==src.light1_shadow_time && type_time==src.type_time
          && width_time==src.width_time && height_time==src.height_time && scale_time==src.scale_time && fov_time==src.fov_time
          && cam_angle_time==src.cam_angle_time && cam_focus_time==src.cam_focus_time
          && ambient_col_time==src.ambient_col_time && ambient_occl_time==src.ambient_occl_time && ambient_range_time==src.ambient_range_time
          && bloom_original_time==src.bloom_original_time && bloom_scale_time==src.bloom_scale_time && bloom_cut_time==src.bloom_cut_time
          && light0_col_time==src.light0_col_time && light1_col_time==src.light1_col_time && light0_angle_time==src.light0_angle_time && light1_angle_time==src.light1_angle_time;
   }

   // operations
   void reset() {T=IconSettings();}

   bool sync(C IconSettings &src)
   {
      bool changed=false;
      changed|=Sync(      mip_maps_time, src.      mip_maps_time, mip_maps      , src.mip_maps);
      changed|=Sync(   auto_center_time, src.   auto_center_time, auto_center   , src.auto_center);
      changed|=Sync( light0_shadow_time, src. light0_shadow_time, light0_shadow , src.light0_shadow);
      changed|=Sync( light1_shadow_time, src. light1_shadow_time, light1_shadow , src.light1_shadow);
      changed|=Sync(          type_time, src.          type_time, type          , src.type);
      changed|=Sync(         width_time, src.         width_time, width         , src.width);
      changed|=Sync(        height_time, src.        height_time, height        , src.height);
      changed|=Sync(         scale_time, src.         scale_time, scale         , src.scale);
      changed|=Sync(           fov_time, src.           fov_time, fov           , src.fov);
      changed|=Sync(     cam_angle_time, src.     cam_angle_time, cam_angle     , src.cam_angle);
      changed|=Sync(     cam_focus_time, src.     cam_focus_time, cam_focus     , src.cam_focus);
      changed|=Sync(   ambient_col_time, src.   ambient_col_time, ambient_col   , src.ambient_col);
      changed|=Sync(  ambient_occl_time, src.  ambient_occl_time, ambient_occl  , src.ambient_occl);
      changed|=Sync( ambient_range_time, src. ambient_range_time, ambient_range , src.ambient_range);
      changed|=Sync(bloom_original_time, src.bloom_original_time, bloom_original, src.bloom_original);
      changed|=Sync(   bloom_scale_time, src.   bloom_scale_time, bloom_scale   , src.bloom_scale);
      changed|=Sync(     bloom_cut_time, src.     bloom_cut_time, bloom_cut     , src.bloom_cut);
      changed|=Sync(    light0_col_time, src.    light0_col_time, light0_col    , src.light0_col);
      changed|=Sync(    light1_col_time, src.    light1_col_time, light1_col    , src.light1_col);
      changed|=Sync(  light0_angle_time, src.  light0_angle_time, light0_angle  , src.light0_angle);
      changed|=Sync(  light1_angle_time, src.  light1_angle_time, light1_angle  , src.light1_angle);
      return changed;
   }
   bool undo(C IconSettings &src)
   {
      bool changed=false;
      changed|=Undo(      mip_maps_time, src.      mip_maps_time, mip_maps      , src.mip_maps);
      changed|=Undo(   auto_center_time, src.   auto_center_time, auto_center   , src.auto_center);
      changed|=Undo( light0_shadow_time, src. light0_shadow_time, light0_shadow , src.light0_shadow);
      changed|=Undo( light1_shadow_time, src. light1_shadow_time, light1_shadow , src.light1_shadow);
      changed|=Undo(          type_time, src.          type_time, type          , src.type);
      changed|=Undo(         width_time, src.         width_time, width         , src.width);
      changed|=Undo(        height_time, src.        height_time, height        , src.height);
      changed|=Undo(         scale_time, src.         scale_time, scale         , src.scale);
      changed|=Undo(           fov_time, src.           fov_time, fov           , src.fov);
      changed|=Undo(     cam_angle_time, src.     cam_angle_time, cam_angle     , src.cam_angle);
      changed|=Undo(     cam_focus_time, src.     cam_focus_time, cam_focus     , src.cam_focus);
      changed|=Undo(   ambient_col_time, src.   ambient_col_time, ambient_col   , src.ambient_col);
      changed|=Undo(  ambient_occl_time, src.  ambient_occl_time, ambient_occl  , src.ambient_occl);
      changed|=Undo( ambient_range_time, src. ambient_range_time, ambient_range , src.ambient_range);
      changed|=Undo(bloom_original_time, src.bloom_original_time, bloom_original, src.bloom_original);
      changed|=Undo(   bloom_scale_time, src.   bloom_scale_time, bloom_scale   , src.bloom_scale);
      changed|=Undo(     bloom_cut_time, src.     bloom_cut_time, bloom_cut     , src.bloom_cut);
      changed|=Undo(    light0_col_time, src.    light0_col_time, light0_col    , src.light0_col);
      changed|=Undo(    light1_col_time, src.    light1_col_time, light1_col    , src.light1_col);
      changed|=Undo(  light0_angle_time, src.  light0_angle_time, light0_angle  , src.light0_angle);
      changed|=Undo(  light1_angle_time, src.  light1_angle_time, light1_angle  , src.light1_angle);
      return changed;
   }
   bool save(File &f)C
   {
      f.cmpUIntV(3);
      f<<mip_maps<<auto_center<<light0_shadow<<light1_shadow<<type<<width<<height<<scale<<fov<<ambient_occl<<ambient_range<<cam_angle<<cam_focus<<ambient_col
       <<bloom_original<<bloom_scale<<bloom_cut
       <<light0_col<<light1_col<<light0_angle<<light1_angle
       <<mip_maps_time<<auto_center_time<<light0_shadow_time<<light1_shadow_time<<type_time<<width_time<<height_time<<scale_time<<fov_time
       <<cam_angle_time<<cam_focus_time<<ambient_col_time<<ambient_occl_time<<ambient_range_time
       <<bloom_original_time<<bloom_scale_time<<bloom_cut_time
       <<light0_col_time<<light1_col_time<<light0_angle_time<<light1_angle_time;
      return f.ok();
   }
   bool load(File &f)
   {
      flt bloom_contrast; TimeStamp bloom_contrast_time;
      switch(f.decUIntV())
      {
         case 3:
         {
            f>>mip_maps>>auto_center>>light0_shadow>>light1_shadow>>type>>width>>height>>scale>>fov>>ambient_occl>>ambient_range>>cam_angle>>cam_focus>>ambient_col
             >>bloom_original>>bloom_scale>>bloom_cut
             >>light0_col>>light1_col>>light0_angle>>light1_angle
             >>mip_maps_time>>auto_center_time>>light0_shadow_time>>light1_shadow_time>>type_time>>width_time>>height_time>>scale_time>>fov_time
             >>cam_angle_time>>cam_focus_time>>ambient_col_time>>ambient_occl_time>>ambient_range_time
             >>bloom_original_time>>bloom_scale_time>>bloom_cut_time
             >>light0_col_time>>light1_col_time>>light0_angle_time>>light1_angle_time;
            if(f.ok())return true;
         }break;

         case 2:
         {
            f>>mip_maps>>auto_center>>light0_shadow>>light1_shadow>>type>>width>>height>>scale>>fov>>ambient_occl>>ambient_range>>cam_angle>>cam_focus>>ambient_col
             >>bloom_original>>bloom_scale>>bloom_cut>>bloom_contrast
             >>light0_col>>light1_col>>light0_angle>>light1_angle
             >>mip_maps_time>>auto_center_time>>light0_shadow_time>>light1_shadow_time>>type_time>>width_time>>height_time>>scale_time>>fov_time
             >>cam_angle_time>>cam_focus_time>>ambient_col_time>>ambient_occl_time>>ambient_range_time
             >>bloom_original_time>>bloom_scale_time>>bloom_cut_time>>bloom_contrast_time
             >>light0_col_time>>light1_col_time>>light0_angle_time>>light1_angle_time;
            if(type>=1)type=ElmImage.TYPE(type+1); if(f.ok())return true;
         }break;

         case 1:
         {
            f>>mip_maps>>auto_center>>light0_shadow>>type>>width>>height>>scale>>fov>>ambient_occl>>ambient_range>>cam_angle>>cam_focus>>ambient_col
             >>bloom_original>>bloom_scale>>bloom_cut>>bloom_contrast
             >>light0_col>>light1_col>>light0_angle>>light1_angle
             >>mip_maps_time>>auto_center_time>>light0_shadow_time>>type_time>>width_time>>height_time>>scale_time>>fov_time
             >>cam_angle_time>>cam_focus_time>>ambient_col_time>>ambient_occl_time>>ambient_range_time
             >>bloom_original_time>>bloom_scale_time>>bloom_cut_time>>bloom_contrast_time
             >>light0_col_time>>light1_col_time>>light0_angle_time>>light1_angle_time;
             light1_shadow=light0_shadow; light1_shadow_time=light0_shadow_time;
            if(type>=1)type=ElmImage.TYPE(type+1); if(f.ok())return true;
         }break;

         case 0: reset(); return f.ok(); // 0 -> as default values
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
