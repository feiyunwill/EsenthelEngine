/******************************************************************************/
/******************************************************************************/
class EditPanelImage : PanelImageParams
{
   class Light
   {
      TimeStamp enabled, intensity, back, highlight, highlight_cut, specular, specular_back, specular_exp, specular_highlight, specular_highlight_cut, angle;

      void reset();           
      bool newer(C Light&src)C;
      bool equal(C Light&src)C;
      void newData();
      bool sync(PanelImageParams::Light &val, C Light &src, C PanelImageParams::Light &src_val);
      bool undo(PanelImageParams::Light &val, C Light &src, C PanelImageParams::Light &src_val);

      // io
      bool save(File &f)C;
      bool load(File &f);
   };
   class ImageParams
   {
      TimeStamp blur_clamp, blur, uv_scale, uv_offset, uv_warp, intensity, mode;

      void reset();                 
      bool newer(C ImageParams&src)C;
      bool equal(C ImageParams&src)C;
      void newData();
      bool sync(PanelImageParams::ImageParams &val, C ImageParams &src, C PanelImageParams::ImageParams &src_val);
      bool undo(PanelImageParams::ImageParams &val, C ImageParams &src, C PanelImageParams::ImageParams &src_val);

      // io
      bool save(File &f)C;
      bool load(File &f);
   };
   class Section
   {
      TimeStamp   size, top_offset, round_depth, outer_depth, inner_depth, inner_distance, specular, reflection_intensity, smooth_depth,
                  color, outer_color, inner_color, color_top, color_bottom, color_left, color_right, outer_border_color, inner_border_color, prev_border_color,
                  depth_overlay, color_overlay, reflection;
      UID         depth_overlay_id, color_overlay_id, reflection_id;
      ImageParams depth_overlay_params, color_overlay_params, depth_noise, color_noise;

      void reset();              
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Section &src)C;
      bool equal(C Section &src)C;
      void newData();
      bool sync(PanelImageParams::Section &val, C Section &src, C PanelImageParams::Section &src_val);
      bool undo(PanelImageParams::Section &val, C Section &src, C PanelImageParams::Section &src_val);

      // io
      bool save(File &f)C;
      bool load(File &f);

public:
   Section();
   };
   class Base
   {
      TimeStamp cut_left, cut_right, cut_bottom, cut_top,
                cut_corners[2][2], cut_corner_slope, cut_corner_amount,
                force_uniform_stretch[2],
                resolution, width, height,
                round_corners, left_slope, right_slope,
                max_side_stretch, extend, extend_inner_padd,
                border_size,
                outer_glow_spread, outer_glow_radius, inner_glow_radius, outer_glow_color, inner_glow_color,
                light_ambient,
                depth, round_depth, inner_distance, smooth_depth,
                shadow_radius, shadow_opacity, shadow_spread,
                color, color_top, color_bottom, color_left, color_right,
                images_size, top_height, bottom_height, left_right_width, top_corner_width, bottom_corner_width,
                top_image, bottom_image, center_image, left_image, right_image, top_left_image, top_right_image, bottom_left_image, bottom_right_image,
                compressed_time, mip_maps_time;
      bool      compressed, mip_maps; // mip-maps disabled by default, because when texture is stretched lower mip maps are used which can look as if outside of the rectangle
      UID       top_id, bottom_id, center_id, left_id, right_id, top_left_id, top_right_id, bottom_left_id, bottom_right_id;
      Light     lights  [2];
      Section   sections[2];

      void reset();           
      void includeIDs(MemPtr<UID> ids)C;
      bool newer(C Base &src)C;
      bool equal(C Base &src)C;
      void newData();
      bool sync(PanelImageParams &val, C Base &src, C PanelImageParams &src_val);
      bool undo(PanelImageParams &val, C Base &src, C PanelImageParams &src_val);

      // io
      bool save(File &f)C;
      bool load(File &f);

public:
   Base();
   };

   Base base;

   void reset  (                     ); 
   void newData(                     ); 
   bool newer  (C EditPanelImage &src)C;
   bool equal  (C EditPanelImage &src)C;
   bool sync   (C EditPanelImage &src); 
   bool undo   (C EditPanelImage &src); 

   class Images
   {
      class Src
      {
         UID      id;
         Image    image;
         ImagePtr image_ptr;

public:
   Src();
      };
      bool      fast;
      Memb<Src> images; // use 'Memb' because pointers are returned
      Project  &proj;

      Images(Project &proj, bool fast);

      Image* get(C UID &image_id);
   };

   void make(PanelImage &panel_image, Threads &threads, Project &proj, Image *soft=null, Image *depth_map=null, bool fast=false);

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
