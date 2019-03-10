/******************************************************************************/
/******************************************************************************/
class IconSettings
{
   bool          mip_maps      ,
                 auto_center   ,
                 light0_shadow ,
                 light1_shadow ;
   ElmImage::TYPE type          ;
   int           width         ,
                 height        ;
   flt           scale         ,
                 fov           ,
                 ambient_occl  ,
                 ambient_range ,
                 bloom_original,
                 bloom_scale   ,
                 bloom_cut     ;
   Vec           cam_angle     ,
                 cam_focus     ,
                 ambient_col   ,
                 light0_col    ,
                 light1_col    ;
   Vec2          light0_angle  ,
                 light1_angle  ;
   TimeStamp     mip_maps_time, auto_center_time, light0_shadow_time, light1_shadow_time, type_time, width_time, height_time, scale_time, fov_time,
                 cam_angle_time, cam_focus_time,
                 ambient_col_time, ambient_occl_time, ambient_range_time,
                 bloom_original_time, bloom_scale_time, bloom_cut_time,
                 light0_col_time, light1_col_time, light0_angle_time, light1_angle_time;

   bool newer(C IconSettings &src)C;
   bool equal(C IconSettings &src)C;

   // operations
   void reset();

   bool sync(C IconSettings &src);
   bool undo(C IconSettings &src);
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   IconSettings();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
