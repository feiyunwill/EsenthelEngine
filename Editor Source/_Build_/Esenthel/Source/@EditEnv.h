/******************************************************************************/
/******************************************************************************/
class EditEnv : Environment
{
   UID       sun_id, star_id, skybox_id, cloud_id[4];
   TimeStamp ambient_on_time, ambient_color_time, night_shade_color_time,
             bloom_on_time, bloom_half_time, bloom_saturate_time, bloom_maximum_time, bloom_blurs_time, bloom_original_time, bloom_scale_time, bloom_cut_time,
             clouds_on_time, clouds_vertical_scale_time, clouds_ray_mask_contrast_time,
             clouds_scale_time[4], clouds_velocity_time[4], clouds_color_time[4], clouds_image_time[4],
             fog_on_time, fog_affect_sky_time, fog_density_time, fog_color_time,
             sky_on_time, sky_frac_time, sky_atmospheric_density_exponent_time, sky_atmospheric_horizon_exponent_time, sky_atmospheric_horizon_color_time, sky_atmospheric_sky_color_time, sky_atmospheric_stars_time, sky_atmospheric_stars_orientation_time, sky_skybox_time,
             sun_on_time, sun_blend_time, sun_glow_time, sun_size_time, sun_highlight_front_time, sun_highlight_back_time, sun_pos_time, sun_light_color_time, sun_rays_color_time, sun_image_color_time, sun_image_time;

   EditEnv();

   // get
   bool newer(C EditEnv &src)C;
   bool equal(C EditEnv &src)C;

   // operations
   void newData();
   bool sync(C EditEnv &src);
   bool undo(C EditEnv &src);
   void reset();              
   void copyTo(Environment &dest, C Project &proj)C;

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
