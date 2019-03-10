/******************************************************************************/
/******************************************************************************/
class EditPanel : Panel
{
   UID       center_image, border_image, top_image, bottom_image, left_right_image, top_corner_image, bottom_corner_image, 
             panel_image;
   TimeStamp center_stretch_time, side_stretch_time, center_shadow_time,
             center_color_time, border_color_time, side_color_time, blur_color_time,
             shadow_opacity_time, shadow_radius_time, shadow_offset_time,
             border_size_time, center_scale_time, top_size_time, bottom_size_time, left_right_size_time, top_corner_size_time, bottom_corner_size_time,
             top_offset_time, bottom_offset_time, left_right_offset_time, top_corner_offset_time, bottom_corner_offset_time,
             center_image_time, border_image_time, top_image_time, bottom_image_time, left_right_image_time, top_corner_image_time, bottom_corner_image_time, 
             panel_image_time;

   // get
   bool newer(C EditPanel &src)C;
   bool equal(C EditPanel &src)C;

   // operations
   void newData();
   void includeIDs(MemPtr<UID> ids)C;
   bool sync(C EditPanel &src);
   bool undo(C EditPanel &src);
   void reset();                
   void create(C Panel &src, C UID &center_image, C UID &border_image, C UID &top_image, C UID &bottom_image, C UID &left_right_image, C UID &top_corner_image, C UID &bottom_corner_image, C UID &panel_image, C TimeStamp &time=TimeStamp().getUTC());
   void copyTo(Panel &panel, C Project &proj)C;

   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   EditPanel();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
