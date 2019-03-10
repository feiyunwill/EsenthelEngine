/******************************************************************************/
class EditEnv : Environment
{
   UID       sun_id=UIDZero, star_id=UIDZero, skybox_id=UIDZero, cloud_id[4];
   TimeStamp ambient_on_time, ambient_color_time, night_shade_color_time,
             bloom_on_time, bloom_half_time, bloom_saturate_time, bloom_maximum_time, bloom_blurs_time, bloom_original_time, bloom_scale_time, bloom_cut_time,
             clouds_on_time, clouds_vertical_scale_time, clouds_ray_mask_contrast_time,
             clouds_scale_time[4], clouds_velocity_time[4], clouds_color_time[4], clouds_image_time[4],
             fog_on_time, fog_affect_sky_time, fog_density_time, fog_color_time,
             sky_on_time, sky_frac_time, sky_atmospheric_density_exponent_time, sky_atmospheric_horizon_exponent_time, sky_atmospheric_horizon_color_time, sky_atmospheric_sky_color_time, sky_atmospheric_stars_time, sky_atmospheric_stars_orientation_time, sky_skybox_time,
             sun_on_time, sun_blend_time, sun_glow_time, sun_size_time, sun_highlight_front_time, sun_highlight_back_time, sun_pos_time, sun_light_color_time, sun_rays_color_time, sun_image_color_time, sun_image_time;

   EditEnv() {REPAO(cloud_id).zero();}

   // get
   bool newer(C EditEnv &src)C
   {
      return
      ambient_on_time>src.ambient_on_time || ambient_color_time>src.ambient_color_time || night_shade_color_time>src.night_shade_color_time
   || bloom_on_time>src.bloom_on_time || bloom_half_time>src.bloom_half_time || bloom_saturate_time>src.bloom_saturate_time || bloom_maximum_time>src.bloom_maximum_time || bloom_blurs_time>src.bloom_blurs_time
   || bloom_original_time>src.bloom_original_time || bloom_scale_time>src.bloom_scale_time || bloom_cut_time>src.bloom_cut_time
   || clouds_on_time>src.clouds_on_time || clouds_vertical_scale_time>src.clouds_vertical_scale_time || clouds_ray_mask_contrast_time>src.clouds_ray_mask_contrast_time
   || clouds_scale_time[0]>src.clouds_scale_time[0] || clouds_velocity_time[0]>src.clouds_velocity_time[0] || clouds_color_time[0]>src.clouds_color_time[0] || clouds_image_time[0]>src.clouds_image_time[0]
   || clouds_scale_time[1]>src.clouds_scale_time[1] || clouds_velocity_time[1]>src.clouds_velocity_time[1] || clouds_color_time[1]>src.clouds_color_time[1] || clouds_image_time[1]>src.clouds_image_time[1]
   || clouds_scale_time[2]>src.clouds_scale_time[2] || clouds_velocity_time[2]>src.clouds_velocity_time[2] || clouds_color_time[2]>src.clouds_color_time[2] || clouds_image_time[2]>src.clouds_image_time[2]
   || clouds_scale_time[3]>src.clouds_scale_time[3] || clouds_velocity_time[3]>src.clouds_velocity_time[3] || clouds_color_time[3]>src.clouds_color_time[3] || clouds_image_time[3]>src.clouds_image_time[3]
   || fog_on_time>src.fog_on_time || fog_affect_sky_time>src.fog_affect_sky_time || fog_density_time>src.fog_density_time || fog_color_time>src.fog_color_time
   || sky_on_time>src.sky_on_time || sky_frac_time>src.sky_frac_time || sky_atmospheric_density_exponent_time>src.sky_atmospheric_density_exponent_time
   || sky_atmospheric_horizon_exponent_time>src.sky_atmospheric_horizon_exponent_time || sky_atmospheric_horizon_color_time>src.sky_atmospheric_horizon_color_time
   || sky_atmospheric_sky_color_time>src.sky_atmospheric_sky_color_time || sky_atmospheric_stars_time>src.sky_atmospheric_stars_time
   || sky_atmospheric_stars_orientation_time>src.sky_atmospheric_stars_orientation_time || sky_skybox_time>src.sky_skybox_time
   || sun_on_time>src.sun_on_time || sun_blend_time>src.sun_blend_time || sun_glow_time>src.sun_glow_time || sun_size_time>src.sun_size_time || sun_highlight_front_time>src.sun_highlight_front_time
   || sun_highlight_back_time>src.sun_highlight_back_time || sun_pos_time>src.sun_pos_time || sun_light_color_time>src.sun_light_color_time || sun_rays_color_time>src.sun_rays_color_time
   || sun_image_color_time>src.sun_image_color_time || sun_image_time>src.sun_image_time;
   }
   bool equal(C EditEnv &src)C
   {
      return
      ambient_on_time==src.ambient_on_time && ambient_color_time==src.ambient_color_time && night_shade_color_time==src.night_shade_color_time
   && bloom_on_time==src.bloom_on_time && bloom_half_time==src.bloom_half_time && bloom_saturate_time==src.bloom_saturate_time && bloom_maximum_time==src.bloom_maximum_time && bloom_blurs_time==src.bloom_blurs_time
   && bloom_original_time==src.bloom_original_time && bloom_scale_time==src.bloom_scale_time && bloom_cut_time==src.bloom_cut_time
   && clouds_on_time==src.clouds_on_time && clouds_vertical_scale_time==src.clouds_vertical_scale_time && clouds_ray_mask_contrast_time==src.clouds_ray_mask_contrast_time
   && clouds_scale_time[0]==src.clouds_scale_time[0] && clouds_velocity_time[0]==src.clouds_velocity_time[0] && clouds_color_time[0]==src.clouds_color_time[0] && clouds_image_time[0]==src.clouds_image_time[0]
   && clouds_scale_time[1]==src.clouds_scale_time[1] && clouds_velocity_time[1]==src.clouds_velocity_time[1] && clouds_color_time[1]==src.clouds_color_time[1] && clouds_image_time[1]==src.clouds_image_time[1]
   && clouds_scale_time[2]==src.clouds_scale_time[2] && clouds_velocity_time[2]==src.clouds_velocity_time[2] && clouds_color_time[2]==src.clouds_color_time[2] && clouds_image_time[2]==src.clouds_image_time[2]
   && clouds_scale_time[3]==src.clouds_scale_time[3] && clouds_velocity_time[3]==src.clouds_velocity_time[3] && clouds_color_time[3]==src.clouds_color_time[3] && clouds_image_time[3]==src.clouds_image_time[3]
   && fog_on_time==src.fog_on_time && fog_affect_sky_time==src.fog_affect_sky_time && fog_density_time==src.fog_density_time && fog_color_time==src.fog_color_time
   && sky_on_time==src.sky_on_time && sky_frac_time==src.sky_frac_time && sky_atmospheric_density_exponent_time==src.sky_atmospheric_density_exponent_time
   && sky_atmospheric_horizon_exponent_time==src.sky_atmospheric_horizon_exponent_time && sky_atmospheric_horizon_color_time==src.sky_atmospheric_horizon_color_time
   && sky_atmospheric_sky_color_time==src.sky_atmospheric_sky_color_time && sky_atmospheric_stars_time==src.sky_atmospheric_stars_time
   && sky_atmospheric_stars_orientation_time==src.sky_atmospheric_stars_orientation_time && sky_skybox_time==src.sky_skybox_time
   && sun_on_time==src.sun_on_time && sun_blend_time==src.sun_blend_time && sun_glow_time==src.sun_glow_time && sun_size_time==src.sun_size_time && sun_highlight_front_time==src.sun_highlight_front_time
   && sun_highlight_back_time==src.sun_highlight_back_time && sun_pos_time==src.sun_pos_time && sun_light_color_time==src.sun_light_color_time && sun_rays_color_time==src.sun_rays_color_time
   && sun_image_color_time==src.sun_image_color_time && sun_image_time==src.sun_image_time;
   }

   // operations
   void newData()
   {
      ambient_on_time++; ambient_color_time++; night_shade_color_time++;
      bloom_on_time++; bloom_half_time++; bloom_saturate_time++; bloom_maximum_time++; bloom_blurs_time++; bloom_original_time++; bloom_scale_time++; bloom_cut_time++;
      clouds_on_time++; clouds_vertical_scale_time++; clouds_ray_mask_contrast_time++;
      REPAO(clouds_scale_time)++; REPAO(clouds_velocity_time)++; REPAO(clouds_color_time)++; REPAO(clouds_image_time)++;
      fog_on_time++; fog_affect_sky_time++; fog_density_time++; fog_color_time++;
      sky_on_time++; sky_frac_time++; sky_atmospheric_density_exponent_time++; sky_atmospheric_horizon_exponent_time++; sky_atmospheric_horizon_color_time++; sky_atmospheric_sky_color_time++; sky_atmospheric_stars_time++; sky_atmospheric_stars_orientation_time++; sky_skybox_time++;
      sun_on_time++; sun_blend_time++; sun_glow_time++; sun_size_time++; sun_highlight_front_time++; sun_highlight_back_time++; sun_pos_time++; sun_light_color_time++; sun_rays_color_time++; sun_image_color_time++; sun_image_time++;
   }
   bool sync(C EditEnv &src)
   {
      bool changed=false;
      changed|=Sync(ambient_on_time, src.ambient_on_time, ambient.on, src.ambient.on);
      changed|=Sync(ambient_color_time, src.ambient_color_time, ambient.color, src.ambient.color);
      changed|=Sync(night_shade_color_time, src.night_shade_color_time, ambient.night_shade_color, src.ambient.night_shade_color);
      changed|=Sync(bloom_on_time, src.bloom_on_time, bloom.on, src.bloom.on);
      changed|=Sync(bloom_half_time, src.bloom_half_time, bloom.half, src.bloom.half);
      changed|=Sync(bloom_saturate_time, src.bloom_saturate_time, bloom.saturate, src.bloom.saturate);
      changed|=Sync(bloom_maximum_time, src.bloom_maximum_time, bloom.maximum, src.bloom.maximum);
      changed|=Sync(bloom_blurs_time, src.bloom_blurs_time, bloom.blurs, src.bloom.blurs);
      changed|=Sync(bloom_original_time, src.bloom_original_time, bloom.original, src.bloom.original);
      changed|=Sync(bloom_scale_time, src.bloom_scale_time, bloom.scale, src.bloom.scale);
      changed|=Sync(bloom_cut_time, src.bloom_cut_time, bloom.cut, src.bloom.cut);
      changed|=Sync(clouds_on_time, src.clouds_on_time, clouds.on, src.clouds.on);
      changed|=Sync(clouds_vertical_scale_time, src.clouds_vertical_scale_time, clouds.vertical_scale, src.clouds.vertical_scale);
      changed|=Sync(clouds_ray_mask_contrast_time, src.clouds_ray_mask_contrast_time, clouds.ray_mask_contrast, src.clouds.ray_mask_contrast);
      REPA(clouds_scale_time)
      {
         changed|=Sync(clouds_scale_time[i], src.clouds_scale_time[i], clouds.layers[i].scale, src.clouds.layers[i].scale);
         changed|=Sync(clouds_velocity_time[i], src.clouds_velocity_time[i], clouds.layers[i].velocity, src.clouds.layers[i].velocity);
         changed|=Sync(clouds_color_time[i], src.clouds_color_time[i], clouds.layers[i].color, src.clouds.layers[i].color);
         changed|=Sync(clouds_image_time[i], src.clouds_image_time[i], cloud_id[i], src.cloud_id[i]);
      }
      changed|=Sync(fog_on_time, src.fog_on_time, fog.on, src.fog.on);
      changed|=Sync(fog_affect_sky_time, src.fog_affect_sky_time, fog.affect_sky, src.fog.affect_sky);
      changed|=Sync(fog_density_time, src.fog_density_time, fog.density, src.fog.density);
      changed|=Sync(fog_color_time, src.fog_color_time, fog.color, src.fog.color);
      changed|=Sync(sky_on_time, src.sky_on_time, sky.on, src.sky.on);
      changed|=Sync(sky_frac_time, src.sky_frac_time, sky.frac, src.sky.frac);
      changed|=Sync(sky_atmospheric_density_exponent_time, src.sky_atmospheric_density_exponent_time, sky.atmospheric_density_exponent, src.sky.atmospheric_density_exponent);
      changed|=Sync(sky_atmospheric_horizon_exponent_time, src.sky_atmospheric_horizon_exponent_time, sky.atmospheric_horizon_exponent, src.sky.atmospheric_horizon_exponent);
      changed|=Sync(sky_atmospheric_horizon_color_time, src.sky_atmospheric_horizon_color_time, sky.atmospheric_horizon_color, src.sky.atmospheric_horizon_color);
      changed|=Sync(sky_atmospheric_sky_color_time, src.sky_atmospheric_sky_color_time, sky.atmospheric_sky_color, src.sky.atmospheric_sky_color);
      changed|=Sync(sky_atmospheric_stars_time, src.sky_atmospheric_stars_time, star_id, src.star_id);
      changed|=Sync(sky_atmospheric_stars_orientation_time, src.sky_atmospheric_stars_orientation_time, sky.atmospheric_stars_orientation, src.sky.atmospheric_stars_orientation);
      changed|=Sync(sky_skybox_time, src.sky_skybox_time, skybox_id, src.skybox_id);
      changed|=Sync(sun_on_time, src.sun_on_time, sun.on, src.sun.on);
      changed|=Sync(sun_blend_time, src.sun_blend_time, sun.blend, src.sun.blend);
      changed|=Sync(sun_glow_time, src.sun_glow_time, sun.glow, src.sun.glow);
      changed|=Sync(sun_size_time, src.sun_size_time, sun.size, src.sun.size);
      changed|=Sync(sun_highlight_front_time, src.sun_highlight_front_time, sun.highlight_front, src.sun.highlight_front);
      changed|=Sync(sun_highlight_back_time, src.sun_highlight_back_time, sun.highlight_back, src.sun.highlight_back);
      changed|=Sync(sun_pos_time, src.sun_pos_time, sun.pos, src.sun.pos);
      changed|=Sync(sun_light_color_time, src.sun_light_color_time, sun.light_color, src.sun.light_color);
      changed|=Sync(sun_rays_color_time, src.sun_rays_color_time, sun.rays_color, src.sun.rays_color);
      changed|=Sync(sun_image_color_time, src.sun_image_color_time, sun.image_color, src.sun.image_color);
      changed|=Sync(sun_image_time, src.sun_image_time, sun_id, src.sun_id);
      return changed;
   }
   bool undo(C EditEnv &src)
   {
      bool changed=false;
      changed|=Undo(ambient_on_time, src.ambient_on_time, ambient.on, src.ambient.on);
      changed|=Undo(ambient_color_time, src.ambient_color_time, ambient.color, src.ambient.color);
      changed|=Undo(night_shade_color_time, src.night_shade_color_time, ambient.night_shade_color, src.ambient.night_shade_color);
      changed|=Undo(bloom_on_time, src.bloom_on_time, bloom.on, src.bloom.on);
      changed|=Undo(bloom_half_time, src.bloom_half_time, bloom.half, src.bloom.half);
      changed|=Undo(bloom_saturate_time, src.bloom_saturate_time, bloom.saturate, src.bloom.saturate);
      changed|=Undo(bloom_maximum_time, src.bloom_maximum_time, bloom.maximum, src.bloom.maximum);
      changed|=Undo(bloom_blurs_time, src.bloom_blurs_time, bloom.blurs, src.bloom.blurs);
      changed|=Undo(bloom_original_time, src.bloom_original_time, bloom.original, src.bloom.original);
      changed|=Undo(bloom_scale_time, src.bloom_scale_time, bloom.scale, src.bloom.scale);
      changed|=Undo(bloom_cut_time, src.bloom_cut_time, bloom.cut, src.bloom.cut);
      changed|=Undo(clouds_on_time, src.clouds_on_time, clouds.on, src.clouds.on);
      changed|=Undo(clouds_vertical_scale_time, src.clouds_vertical_scale_time, clouds.vertical_scale, src.clouds.vertical_scale);
      changed|=Undo(clouds_ray_mask_contrast_time, src.clouds_ray_mask_contrast_time, clouds.ray_mask_contrast, src.clouds.ray_mask_contrast);
      REPA(clouds_scale_time)
      {
         changed|=Undo(clouds_scale_time[i], src.clouds_scale_time[i], clouds.layers[i].scale, src.clouds.layers[i].scale);
         changed|=Undo(clouds_velocity_time[i], src.clouds_velocity_time[i], clouds.layers[i].velocity, src.clouds.layers[i].velocity);
         changed|=Undo(clouds_color_time[i], src.clouds_color_time[i], clouds.layers[i].color, src.clouds.layers[i].color);
         changed|=Undo(clouds_image_time[i], src.clouds_image_time[i], cloud_id[i], src.cloud_id[i]);
      }
      changed|=Undo(fog_on_time, src.fog_on_time, fog.on, src.fog.on);
      changed|=Undo(fog_affect_sky_time, src.fog_affect_sky_time, fog.affect_sky, src.fog.affect_sky);
      changed|=Undo(fog_density_time, src.fog_density_time, fog.density, src.fog.density);
      changed|=Undo(fog_color_time, src.fog_color_time, fog.color, src.fog.color);
      changed|=Undo(sky_on_time, src.sky_on_time, sky.on, src.sky.on);
      changed|=Undo(sky_frac_time, src.sky_frac_time, sky.frac, src.sky.frac);
      changed|=Undo(sky_atmospheric_density_exponent_time, src.sky_atmospheric_density_exponent_time, sky.atmospheric_density_exponent, src.sky.atmospheric_density_exponent);
      changed|=Undo(sky_atmospheric_horizon_exponent_time, src.sky_atmospheric_horizon_exponent_time, sky.atmospheric_horizon_exponent, src.sky.atmospheric_horizon_exponent);
      changed|=Undo(sky_atmospheric_horizon_color_time, src.sky_atmospheric_horizon_color_time, sky.atmospheric_horizon_color, src.sky.atmospheric_horizon_color);
      changed|=Undo(sky_atmospheric_sky_color_time, src.sky_atmospheric_sky_color_time, sky.atmospheric_sky_color, src.sky.atmospheric_sky_color);
      changed|=Undo(sky_atmospheric_stars_time, src.sky_atmospheric_stars_time, star_id, src.star_id);
      changed|=Undo(sky_atmospheric_stars_orientation_time, src.sky_atmospheric_stars_orientation_time, sky.atmospheric_stars_orientation, src.sky.atmospheric_stars_orientation);
      changed|=Undo(sky_skybox_time, src.sky_skybox_time, skybox_id, src.skybox_id);
      changed|=Undo(sun_on_time, src.sun_on_time, sun.on, src.sun.on);
      changed|=Undo(sun_blend_time, src.sun_blend_time, sun.blend, src.sun.blend);
      changed|=Undo(sun_glow_time, src.sun_glow_time, sun.glow, src.sun.glow);
      changed|=Undo(sun_size_time, src.sun_size_time, sun.size, src.sun.size);
      changed|=Undo(sun_highlight_front_time, src.sun_highlight_front_time, sun.highlight_front, src.sun.highlight_front);
      changed|=Undo(sun_highlight_back_time, src.sun_highlight_back_time, sun.highlight_back, src.sun.highlight_back);
      changed|=Undo(sun_pos_time, src.sun_pos_time, sun.pos, src.sun.pos);
      changed|=Undo(sun_light_color_time, src.sun_light_color_time, sun.light_color, src.sun.light_color);
      changed|=Undo(sun_rays_color_time, src.sun_rays_color_time, sun.rays_color, src.sun.rays_color);
      changed|=Undo(sun_image_color_time, src.sun_image_color_time, sun.image_color, src.sun.image_color);
      changed|=Undo(sun_image_time, src.sun_image_time, sun_id, src.sun_id);
      return changed;
   }
   void reset() {T=EditEnv();}
   void copyTo(Environment &dest, C Project &proj)C
   {
      dest=T;
            dest.sun   .image            =proj.gamePath(   sun_id   );
            dest.sky   .atmospheric_stars=proj.gamePath(  star_id   );
            dest.sky   .skybox           =proj.gamePath(skybox_id   );
      REPAO(dest.clouds.layers).image    =proj.gamePath( cloud_id[i]);
   }

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(2);
      super.save(f);
      f<<sun_id<<star_id<<skybox_id<<cloud_id
       <<ambient_on_time<<ambient_color_time<<night_shade_color_time
       <<bloom_on_time<<bloom_half_time<<bloom_saturate_time<<bloom_maximum_time<<bloom_blurs_time<<bloom_original_time<<bloom_scale_time<<bloom_cut_time
       <<clouds_on_time<<clouds_vertical_scale_time<<clouds_ray_mask_contrast_time
       <<clouds_scale_time<<clouds_velocity_time<<clouds_color_time<<clouds_image_time
       <<fog_on_time<<fog_affect_sky_time<<fog_density_time<<fog_color_time
       <<sky_on_time<<sky_frac_time<<sky_atmospheric_density_exponent_time<<sky_atmospheric_horizon_exponent_time<<sky_atmospheric_horizon_color_time<<sky_atmospheric_sky_color_time
       <<sky_atmospheric_stars_time<<sky_atmospheric_stars_orientation_time<<sky_skybox_time
       <<sun_on_time<<sun_blend_time<<sun_glow_time<<sun_size_time<<sun_highlight_front_time<<sun_highlight_back_time<<sun_pos_time<<sun_light_color_time<<sun_rays_color_time
       <<sun_image_color_time<<sun_image_time;
      return f.ok();
   }
   bool load(File &f)
   {
      TimeStamp bloom_contrast_time;
      reset(); switch(f.decUIntV())
      {
         case 2:
         {
            if(!super.load(f))break;
            f>>sun_id>>star_id>>skybox_id>>cloud_id
             >>ambient_on_time>>ambient_color_time>>night_shade_color_time
             >>bloom_on_time>>bloom_half_time>>bloom_saturate_time>>bloom_maximum_time>>bloom_blurs_time>>bloom_original_time>>bloom_scale_time>>bloom_cut_time
             >>clouds_on_time>>clouds_vertical_scale_time>>clouds_ray_mask_contrast_time
             >>clouds_scale_time>>clouds_velocity_time>>clouds_color_time>>clouds_image_time
             >>fog_on_time>>fog_affect_sky_time>>fog_density_time>>fog_color_time
             >>sky_on_time>>sky_frac_time>>sky_atmospheric_density_exponent_time>>sky_atmospheric_horizon_exponent_time>>sky_atmospheric_horizon_color_time>>sky_atmospheric_sky_color_time
             >>sky_atmospheric_stars_time>>sky_atmospheric_stars_orientation_time>>sky_skybox_time
             >>sun_on_time>>sun_blend_time>>sun_glow_time>>sun_size_time>>sun_highlight_front_time>>sun_highlight_back_time>>sun_pos_time>>sun_light_color_time>>sun_rays_color_time
             >>sun_image_color_time>>sun_image_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            if(!super.load(f))break;
            f>>sun_id>>star_id>>skybox_id>>cloud_id
             >>ambient_on_time>>ambient_color_time>>night_shade_color_time
             >>bloom_on_time>>bloom_half_time>>bloom_maximum_time>>bloom_blurs_time>>bloom_original_time>>bloom_scale_time>>bloom_cut_time>>bloom_contrast_time
             >>clouds_on_time>>clouds_vertical_scale_time>>clouds_ray_mask_contrast_time
             >>clouds_scale_time>>clouds_velocity_time>>clouds_color_time>>clouds_image_time
             >>fog_on_time>>fog_affect_sky_time>>fog_density_time>>fog_color_time
             >>sky_on_time>>sky_frac_time>>sky_atmospheric_density_exponent_time>>sky_atmospheric_horizon_exponent_time>>sky_atmospheric_horizon_color_time>>sky_atmospheric_sky_color_time
             >>sky_atmospheric_stars_time>>sky_atmospheric_stars_orientation_time>>sky_skybox_time
             >>sun_on_time>>sun_blend_time>>sun_glow_time>>sun_size_time>>sun_highlight_front_time>>sun_highlight_back_time>>sun_pos_time>>sun_light_color_time>>sun_rays_color_time
             >>sun_image_color_time>>sun_image_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            if(!super.load(f))break;
            f>>sun_id>>star_id>>skybox_id>>cloud_id
             >>ambient_on_time>>ambient_color_time
             >>bloom_on_time>>bloom_half_time>>bloom_maximum_time>>bloom_blurs_time>>bloom_original_time>>bloom_scale_time>>bloom_cut_time>>bloom_contrast_time
             >>clouds_on_time>>clouds_vertical_scale_time>>clouds_ray_mask_contrast_time
             >>clouds_scale_time>>clouds_velocity_time>>clouds_color_time>>clouds_image_time
             >>fog_on_time>>fog_affect_sky_time>>fog_density_time>>fog_color_time
             >>sky_on_time>>sky_frac_time>>sky_atmospheric_density_exponent_time>>sky_atmospheric_horizon_exponent_time>>sky_atmospheric_horizon_color_time>>sky_atmospheric_sky_color_time
             >>sky_atmospheric_stars_time>>sky_atmospheric_stars_orientation_time>>sky_skybox_time
             >>sun_on_time>>sun_blend_time>>sun_glow_time>>sun_size_time>>sun_highlight_front_time>>sun_highlight_back_time>>sun_pos_time>>sun_light_color_time>>sun_rays_color_time
             >>sun_image_color_time>>sun_image_time;
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
