/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
      void EditPanelImage::Light::reset() {T=Light();}
      bool EditPanelImage::Light::newer(C Light &src)C
      {
         return enabled>src.enabled
             || intensity>src.intensity
             || back>src.back
             || highlight>src.highlight
             || highlight_cut>src.highlight_cut
             || specular>src.specular
             || specular_back>src.specular_back
             || specular_exp>src.specular_exp
             || specular_highlight>src.specular_highlight
             || specular_highlight_cut>src.specular_highlight_cut
             || angle>src.angle;
      }
      bool EditPanelImage::Light::equal(C Light &src)C
      {
         return enabled==src.enabled
             && intensity==src.intensity
             && back==src.back
             && highlight==src.highlight
             && highlight_cut==src.highlight_cut
             && specular==src.specular
             && specular_back==src.specular_back
             && specular_exp==src.specular_exp
             && specular_highlight==src.specular_highlight
             && specular_highlight_cut==src.specular_highlight_cut
             && angle==src.angle;
      }
      void EditPanelImage::Light::newData()
      {
         enabled++; intensity++; back++; highlight++; highlight_cut++; specular++; specular_back++; specular_exp++; specular_highlight++; specular_highlight_cut++; angle++;
      }
      bool EditPanelImage::Light::sync(PanelImageParams::Light &val, C Light &src, C PanelImageParams::Light &src_val)
      {
         bool changed=false;
         changed|=Sync(enabled               , src.enabled               , val.enabled               , src_val.enabled               );
         changed|=Sync(intensity             , src.intensity             , val.intensity             , src_val.intensity             );
         changed|=Sync(back                  , src.back                  , val.back                  , src_val.back                  );
         changed|=Sync(highlight             , src.highlight             , val.highlight             , src_val.highlight             );
         changed|=Sync(highlight_cut         , src.highlight_cut         , val.highlight_cut         , src_val.highlight_cut         );
         changed|=Sync(specular              , src.specular              , val.specular              , src_val.specular              );
         changed|=Sync(specular_back         , src.specular_back         , val.specular_back         , src_val.specular_back         );
         changed|=Sync(specular_exp          , src.specular_exp          , val.specular_exp          , src_val.specular_exp          );
         changed|=Sync(specular_highlight    , src.specular_highlight    , val.specular_highlight    , src_val.specular_highlight    );
         changed|=Sync(specular_highlight_cut, src.specular_highlight_cut, val.specular_highlight_cut, src_val.specular_highlight_cut);
         changed|=Sync(angle                 , src.angle                 , val.angle                 , src_val.angle                 );
         return changed;
      }
      bool EditPanelImage::Light::undo(PanelImageParams::Light &val, C Light &src, C PanelImageParams::Light &src_val)
      {
         bool changed=false;
         changed|=Undo(enabled               , src.enabled               , val.enabled               , src_val.enabled               );
         changed|=Undo(intensity             , src.intensity             , val.intensity             , src_val.intensity             );
         changed|=Undo(back                  , src.back                  , val.back                  , src_val.back                  );
         changed|=Undo(highlight             , src.highlight             , val.highlight             , src_val.highlight             );
         changed|=Undo(highlight_cut         , src.highlight_cut         , val.highlight_cut         , src_val.highlight_cut         );
         changed|=Undo(specular              , src.specular              , val.specular              , src_val.specular              );
         changed|=Undo(specular_back         , src.specular_back         , val.specular_back         , src_val.specular_back         );
         changed|=Undo(specular_exp          , src.specular_exp          , val.specular_exp          , src_val.specular_exp          );
         changed|=Undo(specular_highlight    , src.specular_highlight    , val.specular_highlight    , src_val.specular_highlight    );
         changed|=Undo(specular_highlight_cut, src.specular_highlight_cut, val.specular_highlight_cut, src_val.specular_highlight_cut);
         changed|=Undo(angle                 , src.angle                 , val.angle                 , src_val.angle                 );
         return changed;
      }
      bool EditPanelImage::Light::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<enabled<<intensity<<back<<highlight<<highlight_cut<<specular<<specular_back<<specular_exp<<specular_highlight<<specular_highlight_cut<<angle;
         return f.ok();
      }
      bool EditPanelImage::Light::load(File &f)
      {
         switch(f.decUIntV())
         {
            case 0:
            {
               f>>enabled>>intensity>>back>>highlight>>highlight_cut>>specular>>specular_back>>specular_exp>>specular_highlight>>specular_highlight_cut>>angle;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditPanelImage::ImageParams::reset() {T=ImageParams();}
      bool EditPanelImage::ImageParams::newer(C ImageParams &src)C
      {
         return blur_clamp>src.blur_clamp
             || blur>src.blur
             || uv_scale>src.uv_scale
             || uv_offset>src.uv_offset
             || uv_warp>src.uv_warp
             || intensity>src.intensity
             || mode>src.mode;
      }
      bool EditPanelImage::ImageParams::equal(C ImageParams &src)C
      {
         return blur_clamp==src.blur_clamp
             && blur==src.blur
             && uv_scale==src.uv_scale
             && uv_offset==src.uv_offset
             && uv_warp==src.uv_warp
             && intensity==src.intensity
             && mode==src.mode;
      }
      void EditPanelImage::ImageParams::newData()
      {
         blur_clamp++; blur++; uv_scale++; uv_offset++; uv_warp++; intensity++; mode++;
      }
      bool EditPanelImage::ImageParams::sync(PanelImageParams::ImageParams &val, C ImageParams &src, C PanelImageParams::ImageParams &src_val)
      {
         bool changed=false;
         changed|=Sync(blur_clamp, src.blur_clamp, val.blur_clamp, src_val.blur_clamp);
         changed|=Sync(blur      , src.blur      , val.blur      , src_val.blur      );
         changed|=Sync(uv_scale  , src.uv_scale  , val.uv_scale  , src_val.uv_scale  );
         changed|=Sync(uv_offset , src.uv_offset , val.uv_offset , src_val.uv_offset );
         changed|=Sync(uv_warp   , src.uv_warp   , val.uv_warp   , src_val.uv_warp   );
         changed|=Sync(intensity , src.intensity , val.intensity , src_val.intensity );
         changed|=Sync(mode      , src.mode      , val.mode      , src_val.mode      );
         return changed;
      }
      bool EditPanelImage::ImageParams::undo(PanelImageParams::ImageParams &val, C ImageParams &src, C PanelImageParams::ImageParams &src_val)
      {
         bool changed=false;
         changed|=Undo(blur_clamp, src.blur_clamp, val.blur_clamp, src_val.blur_clamp);
         changed|=Undo(blur      , src.blur      , val.blur      , src_val.blur      );
         changed|=Undo(uv_scale  , src.uv_scale  , val.uv_scale  , src_val.uv_scale  );
         changed|=Undo(uv_offset , src.uv_offset , val.uv_offset , src_val.uv_offset );
         changed|=Undo(uv_warp   , src.uv_warp   , val.uv_warp   , src_val.uv_warp   );
         changed|=Undo(intensity , src.intensity , val.intensity , src_val.intensity );
         changed|=Undo(mode      , src.mode      , val.mode      , src_val.mode      );
         return changed;
      }
      bool EditPanelImage::ImageParams::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<blur_clamp<<blur<<uv_scale<<uv_offset<<uv_warp<<intensity<<mode;
         return f.ok();
      }
      bool EditPanelImage::ImageParams::load(File &f)
      {
         switch(f.decUIntV())
         {
            case 0:
            {
               f>>blur_clamp>>blur>>uv_scale>>uv_offset>>uv_warp>>intensity>>mode;
               if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditPanelImage::Section::reset() {T=Section();}
      void EditPanelImage::Section::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, depth_overlay_id);
         Include(ids, color_overlay_id);
         Include(ids,    reflection_id);
      }
      bool EditPanelImage::Section::newer(C Section &src)C
      {
         return size>src.size
             || top_offset>src.top_offset
             || round_depth>src.round_depth
             || outer_depth>src.outer_depth
             || inner_depth>src.inner_depth
             || inner_distance>src.inner_distance
             || specular>src.specular
             || reflection_intensity>src.reflection_intensity
             || smooth_depth>src.smooth_depth
             || color>src.color
             || outer_color>src.outer_color
             || inner_color>src.inner_color
             || color_top>src.color_top
             || color_bottom>src.color_bottom
             || color_left>src.color_left
             || color_right>src.color_right
             || outer_border_color>src.outer_border_color
             || inner_border_color>src.inner_border_color
             || prev_border_color>src.prev_border_color
             || depth_overlay>src.depth_overlay || color_overlay>src.color_overlay || reflection>src.reflection
             || depth_overlay_params.newer(src.depth_overlay_params) || color_overlay_params.newer(src.color_overlay_params) || depth_noise.newer(src.depth_noise) || color_noise.newer(src.color_noise);
      }
      bool EditPanelImage::Section::equal(C Section &src)C
      {
         return size==src.size
             && top_offset==src.top_offset
             && round_depth==src.round_depth
             && outer_depth==src.outer_depth
             && inner_depth==src.inner_depth
             && inner_distance==src.inner_distance
             && specular==src.specular
             && reflection_intensity==src.reflection_intensity
             && smooth_depth==src.smooth_depth
             && color==src.color
             && outer_color==src.outer_color
             && inner_color==src.inner_color
             && color_top==src.color_top
             && color_bottom==src.color_bottom
             && color_left==src.color_left
             && color_right==src.color_right
             && outer_border_color==src.outer_border_color
             && inner_border_color==src.inner_border_color
             && prev_border_color==src.prev_border_color
             && depth_overlay==src.depth_overlay && color_overlay==src.color_overlay && reflection==src.reflection
             && depth_overlay_params.equal(src.depth_overlay_params) && color_overlay_params.equal(src.color_overlay_params) && depth_noise.equal(src.depth_noise) && color_noise.equal(src.color_noise);
      }
      void EditPanelImage::Section::newData()
      {
         size++; top_offset++; round_depth++; outer_depth++; inner_depth++; inner_distance++; specular++; reflection_intensity++; smooth_depth++;
         color++; outer_color++; inner_color++; color_top++; color_bottom++; color_left++; color_right++; outer_border_color++; inner_border_color++; prev_border_color++;
         depth_overlay++; color_overlay++; reflection++;
         depth_overlay_params.newData(); color_overlay_params.newData(); depth_noise.newData(); color_noise.newData();
      }
      bool EditPanelImage::Section::sync(PanelImageParams::Section &val, C Section &src, C PanelImageParams::Section &src_val)
      {
         bool changed=false;
         changed|=Sync(size                , src.size                , val.size                , src_val.size                );
         changed|=Sync(top_offset          , src.top_offset          , val.top_offset          , src_val.top_offset          );
         changed|=Sync(round_depth         , src.round_depth         , val.round_depth         , src_val.round_depth         );
         changed|=Sync(outer_depth         , src.outer_depth         , val.outer_depth         , src_val.outer_depth         );
         changed|=Sync(inner_depth         , src.inner_depth         , val.inner_depth         , src_val.inner_depth         );
         changed|=Sync(inner_distance      , src.inner_distance      , val.inner_distance      , src_val.inner_distance      );
         changed|=Sync(specular            , src.specular            , val.specular            , src_val.specular            );
         changed|=Sync(reflection_intensity, src.reflection_intensity, val.reflection_intensity, src_val.reflection_intensity);
         changed|=Sync(smooth_depth        , src.smooth_depth        , val.smooth_depth        , src_val.smooth_depth        );
         changed|=Sync(color               , src.color               , val.color               , src_val.color               );
         changed|=Sync(outer_color         , src.outer_color         , val.outer_color         , src_val.outer_color         );
         changed|=Sync(inner_color         , src.inner_color         , val.inner_color         , src_val.inner_color         );
         changed|=Sync(color_top           , src.color_top           , val.color_top           , src_val.color_top           );
         changed|=Sync(color_bottom        , src.color_bottom        , val.color_bottom        , src_val.color_bottom        );
         changed|=Sync(color_left          , src.color_left          , val.color_left          , src_val.color_left          );
         changed|=Sync(color_right         , src.color_right         , val.color_right         , src_val.color_right         );
         changed|=Sync(outer_border_color  , src.outer_border_color  , val.outer_border_color  , src_val.outer_border_color  );
         changed|=Sync(inner_border_color  , src.inner_border_color  , val.inner_border_color  , src_val.inner_border_color  );
         changed|=Sync(prev_border_color   , src.prev_border_color   , val.prev_border_color   , src_val.prev_border_color   );
         changed|=Sync(depth_overlay       , src.depth_overlay       , depth_overlay_id        , src.depth_overlay_id        );
         changed|=Sync(color_overlay       , src.color_overlay       , color_overlay_id        , src.color_overlay_id        );
         changed|=Sync(reflection          , src.reflection          , reflection_id           , src.reflection_id           );
         changed|=depth_overlay_params.sync(val.depth_overlay_params, src.depth_overlay_params, src_val.depth_overlay_params);
         changed|=color_overlay_params.sync(val.color_overlay_params, src.color_overlay_params, src_val.color_overlay_params);
         changed|=depth_noise         .sync(val.depth_noise         , src.depth_noise         , src_val.depth_noise         );
         changed|=color_noise         .sync(val.color_noise         , src.color_noise         , src_val.color_noise         );
         return changed;
      }
      bool EditPanelImage::Section::undo(PanelImageParams::Section &val, C Section &src, C PanelImageParams::Section &src_val)
      {
         bool changed=false;
         changed|=Undo(size                , src.size                , val.size                , src_val.size                );
         changed|=Undo(top_offset          , src.top_offset          , val.top_offset          , src_val.top_offset          );
         changed|=Undo(round_depth         , src.round_depth         , val.round_depth         , src_val.round_depth         );
         changed|=Undo(outer_depth         , src.outer_depth         , val.outer_depth         , src_val.outer_depth         );
         changed|=Undo(inner_depth         , src.inner_depth         , val.inner_depth         , src_val.inner_depth         );
         changed|=Undo(inner_distance      , src.inner_distance      , val.inner_distance      , src_val.inner_distance      );
         changed|=Undo(specular            , src.specular            , val.specular            , src_val.specular            );
         changed|=Undo(reflection_intensity, src.reflection_intensity, val.reflection_intensity, src_val.reflection_intensity);
         changed|=Undo(smooth_depth        , src.smooth_depth        , val.smooth_depth        , src_val.smooth_depth        );
         changed|=Undo(color               , src.color               , val.color               , src_val.color               );
         changed|=Undo(outer_color         , src.outer_color         , val.outer_color         , src_val.outer_color         );
         changed|=Undo(inner_color         , src.inner_color         , val.inner_color         , src_val.inner_color         );
         changed|=Undo(color_top           , src.color_top           , val.color_top           , src_val.color_top           );
         changed|=Undo(color_bottom        , src.color_bottom        , val.color_bottom        , src_val.color_bottom        );
         changed|=Undo(color_left          , src.color_left          , val.color_left          , src_val.color_left          );
         changed|=Undo(color_right         , src.color_right         , val.color_right         , src_val.color_right         );
         changed|=Undo(outer_border_color  , src.outer_border_color  , val.outer_border_color  , src_val.outer_border_color  );
         changed|=Undo(inner_border_color  , src.inner_border_color  , val.inner_border_color  , src_val.inner_border_color  );
         changed|=Undo(prev_border_color   , src.prev_border_color   , val.prev_border_color   , src_val.prev_border_color   );
         changed|=Undo(depth_overlay       , src.depth_overlay       , depth_overlay_id        , src.depth_overlay_id        );
         changed|=Undo(color_overlay       , src.color_overlay       , color_overlay_id        , src.color_overlay_id        );
         changed|=Undo(reflection          , src.reflection          , reflection_id           , src.reflection_id           );
         changed|=depth_overlay_params.undo(val.depth_overlay_params, src.depth_overlay_params, src_val.depth_overlay_params);
         changed|=color_overlay_params.undo(val.color_overlay_params, src.color_overlay_params, src_val.color_overlay_params);
         changed|=depth_noise         .undo(val.depth_noise         , src.depth_noise         , src_val.depth_noise         );
         changed|=color_noise         .undo(val.color_noise         , src.color_noise         , src_val.color_noise         );
         return changed;
      }
      bool EditPanelImage::Section::save(File &f)C
      {
         f.cmpUIntV(0);
         f<<size<<top_offset<<round_depth<<outer_depth<<inner_depth<<inner_distance<<specular<<reflection_intensity<<smooth_depth
          <<color<<outer_color<<inner_color<<color_top<<color_bottom<<color_left<<color_right<<outer_border_color<<inner_border_color<<prev_border_color
          <<depth_overlay<<color_overlay<<reflection
          <<depth_overlay_id<<color_overlay_id<<reflection_id;
         depth_overlay_params.save(f);
         color_overlay_params.save(f);
         depth_noise         .save(f);
         color_noise         .save(f);
         return f.ok();
      }
      bool EditPanelImage::Section::load(File &f)
      {
         switch(f.decUIntV())
         {
            case 0:
            {
               f>>size>>top_offset>>round_depth>>outer_depth>>inner_depth>>inner_distance>>specular>>reflection_intensity>>smooth_depth
                >>color>>outer_color>>inner_color>>color_top>>color_bottom>>color_left>>color_right>>outer_border_color>>inner_border_color>>prev_border_color
                >>depth_overlay>>color_overlay>>reflection
                >>depth_overlay_id>>color_overlay_id>>reflection_id;
               if(depth_overlay_params.load(f))
               if(color_overlay_params.load(f))
               if(depth_noise         .load(f))
               if(color_noise         .load(f))
                  if(f.ok())return true;
            }break;
         }
         reset(); return false;
      }
      void EditPanelImage::Base::reset() {T=Base();}
      void EditPanelImage::Base::includeIDs(MemPtr<UID> ids)C
      {
         Include(ids, top_id); Include(ids, bottom_id); Include(ids, center_id); Include(ids, left_id); Include(ids, right_id);
         Include(ids, top_left_id); Include(ids, top_right_id); Include(ids, bottom_left_id); Include(ids, bottom_right_id);
         REPAO(sections).includeIDs(ids);
      }
      bool EditPanelImage::Base::newer(C Base &src)C
      {
         return cut_left>src.cut_left || cut_right>src.cut_right || cut_bottom>src.cut_bottom || cut_top>src.cut_top
             || cut_corners[0][0]>src.cut_corners[0][0] || cut_corners[0][1]>src.cut_corners[0][1] || cut_corners[1][0]>src.cut_corners[1][0] || cut_corners[1][1]>src.cut_corners[1][1]
             || cut_corner_slope>src.cut_corner_slope || cut_corner_amount>src.cut_corner_amount
             || force_uniform_stretch[0]>src.force_uniform_stretch[0] || force_uniform_stretch[1]>src.force_uniform_stretch[1]
             || resolution>src.resolution || width>src.width || height>src.height
             || round_corners>src.round_corners || left_slope>src.left_slope || right_slope>src.right_slope
             || max_side_stretch>src.max_side_stretch || extend>src.extend || extend_inner_padd>src.extend_inner_padd
             || border_size>src.border_size
             || outer_glow_spread>src.outer_glow_spread || outer_glow_radius>src.outer_glow_radius || inner_glow_radius>src.inner_glow_radius || outer_glow_color>src.outer_glow_color || inner_glow_color>src.inner_glow_color
             || light_ambient>src.light_ambient
             || depth>src.depth || round_depth>src.round_depth || inner_distance>src.inner_distance || smooth_depth>src.smooth_depth
             || shadow_radius>src.shadow_radius || shadow_opacity>src.shadow_opacity || shadow_spread>src.shadow_spread
             || color>src.color || color_top>src.color_top || color_bottom>src.color_bottom || color_left>src.color_left || color_right>src.color_right
             || images_size>src.images_size || top_height>src.top_height || bottom_height>src.bottom_height || left_right_width>src.left_right_width || top_corner_width>src.top_corner_width || bottom_corner_width>src.bottom_corner_width
             || top_image>src.top_image || bottom_image>src.bottom_image || center_image>src.center_image || left_image>src.left_image || right_image>src.right_image
             || top_left_image>src.top_left_image || top_right_image>src.top_right_image || bottom_left_image>src.bottom_left_image || bottom_right_image>src.bottom_right_image
             || compressed_time>src.compressed_time || mip_maps_time>src.mip_maps_time
             || lights[0].newer(src.lights[0])
             || lights[1].newer(src.lights[1])
             || sections[0].newer(src.sections[0])
             || sections[1].newer(src.sections[1]);
      }
      bool EditPanelImage::Base::equal(C Base &src)C
      {
         return cut_left==src.cut_left && cut_right==src.cut_right && cut_bottom==src.cut_bottom && cut_top==src.cut_top
             && cut_corners[0][0]==src.cut_corners[0][0] && cut_corners[0][1]==src.cut_corners[0][1] && cut_corners[1][0]==src.cut_corners[1][0] && cut_corners[1][1]==src.cut_corners[1][1]
             && cut_corner_slope==src.cut_corner_slope && cut_corner_amount==src.cut_corner_amount
             && force_uniform_stretch[0]==src.force_uniform_stretch[0] && force_uniform_stretch[1]==src.force_uniform_stretch[1]
             && resolution==src.resolution && width==src.width && height==src.height
             && round_corners==src.round_corners && left_slope==src.left_slope && right_slope==src.right_slope
             && max_side_stretch==src.max_side_stretch && extend==src.extend && extend_inner_padd==src.extend_inner_padd
             && border_size==src.border_size
             && outer_glow_spread==src.outer_glow_spread && outer_glow_radius==src.outer_glow_radius && inner_glow_radius==src.inner_glow_radius && outer_glow_color==src.outer_glow_color && inner_glow_color==src.inner_glow_color
             && light_ambient==src.light_ambient
             && depth==src.depth && round_depth==src.round_depth && inner_distance==src.inner_distance && smooth_depth==src.smooth_depth
             && shadow_radius==src.shadow_radius && shadow_opacity==src.shadow_opacity && shadow_spread==src.shadow_spread
             && color==src.color && color_top==src.color_top && color_bottom==src.color_bottom && color_left==src.color_left && color_right==src.color_right
             && images_size==src.images_size && top_height==src.top_height && bottom_height==src.bottom_height && left_right_width==src.left_right_width && top_corner_width==src.top_corner_width && bottom_corner_width==src.bottom_corner_width
             && top_image==src.top_image && bottom_image==src.bottom_image && center_image==src.center_image && left_image==src.left_image && right_image==src.right_image
             && top_left_image==src.top_left_image && top_right_image==src.top_right_image && bottom_left_image==src.bottom_left_image && bottom_right_image==src.bottom_right_image
             && compressed_time==src.compressed_time && mip_maps_time==src.mip_maps_time
             && lights[0].equal(src.lights[0])
             && lights[1].equal(src.lights[1])
             && sections[0].equal(src.sections[0])
             && sections[1].equal(src.sections[1]);
      }
      void EditPanelImage::Base::newData()
      {
         cut_left++; cut_right++; cut_bottom++; cut_top++;
         REPAD(y, cut_corners)REPAD(x, cut_corners[y])cut_corners[y][x]++; cut_corner_slope++; cut_corner_amount++;
         REPAO(force_uniform_stretch)++;
         resolution++; width++; height++;
         round_corners++; left_slope++; right_slope++;
         max_side_stretch++; extend++; extend_inner_padd++;
         border_size++;
         outer_glow_spread++; outer_glow_radius++; inner_glow_radius++; outer_glow_color++; inner_glow_color++;
         light_ambient++;
         depth++; round_depth++; inner_distance++; smooth_depth++;
         shadow_radius++; shadow_opacity++; shadow_spread++;
         color++; color_top++; color_bottom++; color_left++; color_right++;
         images_size++; top_height++; bottom_height++; left_right_width++; top_corner_width++; bottom_corner_width++;
         top_image++; bottom_image++; center_image++; left_image++; right_image++; top_left_image++; top_right_image++; bottom_left_image++; bottom_right_image++;
         compressed_time++; mip_maps_time++;
         REPAO(lights  ).newData();
         REPAO(sections).newData();
      }
      bool EditPanelImage::Base::sync(PanelImageParams &val, C Base &src, C PanelImageParams &src_val)
      {
         bool changed=false;
         changed|=Sync(cut_left  , src.cut_left  , val.cut_left  , src_val.cut_left  );
         changed|=Sync(cut_right , src.cut_right , val.cut_right , src_val.cut_right );
         changed|=Sync(cut_bottom, src.cut_bottom, val.cut_bottom, src_val.cut_bottom);
         changed|=Sync(cut_top   , src.cut_top   , val.cut_top   , src_val.cut_top   );

         REPAD(y, cut_corners   )
         REPAD(x, cut_corners[y])changed|=Sync(cut_corners[y][x], src.cut_corners[y][x], val.cut_corners[y][x], src_val.cut_corners[y][x]);

         changed|=Sync(cut_corner_slope , src.cut_corner_slope , val.cut_corner_slope , src_val.cut_corner_slope );
         changed|=Sync(cut_corner_amount, src.cut_corner_amount, val.cut_corner_amount, src_val.cut_corner_amount);
         
         REPA(force_uniform_stretch)changed|=Sync(force_uniform_stretch[i], src.force_uniform_stretch[i], val.force_uniform_stretch[i], src_val.force_uniform_stretch[i]);

         changed|=Sync(resolution          , src.resolution          , val.resolution          , src_val.resolution          );
         changed|=Sync(width               , src.width               , val.width               , src_val.width               );
         changed|=Sync(height              , src.height              , val.height              , src_val.height              );
         changed|=Sync(round_corners       , src.round_corners       , val.round_corners       , src_val.round_corners       );
         changed|=Sync(left_slope          , src.left_slope          , val.left_slope          , src_val.left_slope          );
         changed|=Sync(right_slope         , src.right_slope         , val.right_slope         , src_val.right_slope         );
         changed|=Sync(max_side_stretch    , src.max_side_stretch    , val.max_side_stretch    , src_val.max_side_stretch    );
         changed|=Sync(extend              , src.extend              , val.extend              , src_val.extend              );
         changed|=Sync(extend_inner_padd   , src.extend_inner_padd   , val.extend_inner_padd   , src_val.extend_inner_padd   );
         changed|=Sync(border_size         , src.border_size         , val.border_size         , src_val.border_size         );
         changed|=Sync(outer_glow_spread   , src.outer_glow_spread   , val.outer_glow_spread   , src_val.outer_glow_spread   );
         changed|=Sync(outer_glow_radius   , src.outer_glow_radius   , val.outer_glow_radius   , src_val.outer_glow_radius   );
         changed|=Sync(inner_glow_radius   , src.inner_glow_radius   , val.inner_glow_radius   , src_val.inner_glow_radius   );
         changed|=Sync(outer_glow_color    , src.outer_glow_color    , val.outer_glow_color    , src_val.outer_glow_color    );
         changed|=Sync(inner_glow_color    , src.inner_glow_color    , val.inner_glow_color    , src_val.inner_glow_color    );
         changed|=Sync(light_ambient       , src.light_ambient       , val.light_ambient       , src_val.light_ambient       );
         changed|=Sync(depth               , src.depth               , val.depth               , src_val.depth               );
         changed|=Sync(round_depth         , src.round_depth         , val.round_depth         , src_val.round_depth         );
         changed|=Sync(inner_distance      , src.inner_distance      , val.inner_distance      , src_val.inner_distance      );
         changed|=Sync(smooth_depth        , src.smooth_depth        , val.smooth_depth        , src_val.smooth_depth        );
         changed|=Sync(shadow_radius       , src.shadow_radius       , val.shadow_radius       , src_val.shadow_radius       );
         changed|=Sync(shadow_opacity      , src.shadow_opacity      , val.shadow_opacity      , src_val.shadow_opacity      );
         changed|=Sync(shadow_spread       , src.shadow_spread       , val.shadow_spread       , src_val.shadow_spread       );
         changed|=Sync(color               , src.color               , val.color               , src_val.color               );
         changed|=Sync(color_top           , src.color_top           , val.color_top           , src_val.color_top           );
         changed|=Sync(color_bottom        , src.color_bottom        , val.color_bottom        , src_val.color_bottom        );
         changed|=Sync(color_left          , src.color_left          , val.color_left          , src_val.color_left          );
         changed|=Sync(color_right         , src.color_right         , val.color_right         , src_val.color_right         );
         changed|=Sync(images_size         , src.images_size         , val.images_size         , src_val.images_size         );
         changed|=Sync(top_height          , src.top_height          , val.top_height          , src_val.top_height          );
         changed|=Sync(bottom_height       , src.bottom_height       , val.bottom_height       , src_val.bottom_height       );
         changed|=Sync(left_right_width    , src.left_right_width    , val.left_right_width    , src_val.left_right_width    );
         changed|=Sync(top_corner_width    , src.top_corner_width    , val.top_corner_width    , src_val.top_corner_width    );
         changed|=Sync(bottom_corner_width , src.bottom_corner_width , val.bottom_corner_width , src_val.bottom_corner_width );
         changed|=Sync(top_image           , src.top_image           ,     top_id              , src    .top_id              );
         changed|=Sync(bottom_image        , src.bottom_image        ,     bottom_id           , src    .bottom_id           );
         changed|=Sync(center_image        , src.center_image        ,     center_id           , src    .center_id           );
         changed|=Sync(left_image          , src.left_image          ,     left_id             , src    .left_id             );
         changed|=Sync(right_image         , src.right_image         ,     right_id            , src    .right_id            );
         changed|=Sync(top_left_image      , src.top_left_image      ,     top_left_id         , src    .top_left_id         );
         changed|=Sync(top_right_image     , src.top_right_image     ,     top_right_id        , src    .top_right_id        );
         changed|=Sync(bottom_left_image   , src.bottom_left_image   ,     bottom_left_id      , src    .bottom_left_id      );
         changed|=Sync(bottom_right_image  , src.bottom_right_image  ,     bottom_right_id     , src    .bottom_right_id     );
         changed|=Sync(compressed_time     , src.compressed_time     ,     compressed          , src    .compressed          );
         changed|=Sync(mip_maps_time       , src.mip_maps_time       ,     mip_maps            , src    .mip_maps            );
         REPA(lights  )changed|=lights  [i].sync(val.lights  [i], src.lights  [i], src_val.lights  [i]);
         REPA(sections)changed|=sections[i].sync(val.sections[i], src.sections[i], src_val.sections[i]);
         return changed;
      }
      bool EditPanelImage::Base::undo(PanelImageParams &val, C Base &src, C PanelImageParams &src_val)
      {
         bool changed=false;
         changed|=Undo(cut_left  , src.cut_left  , val.cut_left  , src_val.cut_left  );
         changed|=Undo(cut_right , src.cut_right , val.cut_right , src_val.cut_right );
         changed|=Undo(cut_bottom, src.cut_bottom, val.cut_bottom, src_val.cut_bottom);
         changed|=Undo(cut_top   , src.cut_top   , val.cut_top   , src_val.cut_top   );

         REPAD(y, cut_corners   )
         REPAD(x, cut_corners[y])changed|=Undo(cut_corners[y][x], src.cut_corners[y][x], val.cut_corners[y][x], src_val.cut_corners[y][x]);

         changed|=Undo(cut_corner_slope , src.cut_corner_slope , val.cut_corner_slope , src_val.cut_corner_slope );
         changed|=Undo(cut_corner_amount, src.cut_corner_amount, val.cut_corner_amount, src_val.cut_corner_amount);
         
         REPA(force_uniform_stretch)changed|=Undo(force_uniform_stretch[i], src.force_uniform_stretch[i], val.force_uniform_stretch[i], src_val.force_uniform_stretch[i]);

         changed|=Undo(resolution          , src.resolution          , val.resolution          , src_val.resolution          );
         changed|=Undo(width               , src.width               , val.width               , src_val.width               );
         changed|=Undo(height              , src.height              , val.height              , src_val.height              );
         changed|=Undo(round_corners       , src.round_corners       , val.round_corners       , src_val.round_corners       );
         changed|=Undo(left_slope          , src.left_slope          , val.left_slope          , src_val.left_slope          );
         changed|=Undo(right_slope         , src.right_slope         , val.right_slope         , src_val.right_slope         );
         changed|=Undo(max_side_stretch    , src.max_side_stretch    , val.max_side_stretch    , src_val.max_side_stretch    );
         changed|=Undo(extend              , src.extend              , val.extend              , src_val.extend              );
         changed|=Undo(extend_inner_padd   , src.extend_inner_padd   , val.extend_inner_padd   , src_val.extend_inner_padd   );
         changed|=Undo(border_size         , src.border_size         , val.border_size         , src_val.border_size         );
         changed|=Undo(outer_glow_spread   , src.outer_glow_spread   , val.outer_glow_spread   , src_val.outer_glow_spread   );
         changed|=Undo(outer_glow_radius   , src.outer_glow_radius   , val.outer_glow_radius   , src_val.outer_glow_radius   );
         changed|=Undo(inner_glow_radius   , src.inner_glow_radius   , val.inner_glow_radius   , src_val.inner_glow_radius   );
         changed|=Undo(outer_glow_color    , src.outer_glow_color    , val.outer_glow_color    , src_val.outer_glow_color    );
         changed|=Undo(inner_glow_color    , src.inner_glow_color    , val.inner_glow_color    , src_val.inner_glow_color    );
         changed|=Undo(light_ambient       , src.light_ambient       , val.light_ambient       , src_val.light_ambient       );
         changed|=Undo(depth               , src.depth               , val.depth               , src_val.depth               );
         changed|=Undo(round_depth         , src.round_depth         , val.round_depth         , src_val.round_depth         );
         changed|=Undo(inner_distance      , src.inner_distance      , val.inner_distance      , src_val.inner_distance      );
         changed|=Undo(smooth_depth        , src.smooth_depth        , val.smooth_depth        , src_val.smooth_depth        );
         changed|=Undo(shadow_radius       , src.shadow_radius       , val.shadow_radius       , src_val.shadow_radius       );
         changed|=Undo(shadow_opacity      , src.shadow_opacity      , val.shadow_opacity      , src_val.shadow_opacity      );
         changed|=Undo(shadow_spread       , src.shadow_spread       , val.shadow_spread       , src_val.shadow_spread       );
         changed|=Undo(color               , src.color               , val.color               , src_val.color               );
         changed|=Undo(color_top           , src.color_top           , val.color_top           , src_val.color_top           );
         changed|=Undo(color_bottom        , src.color_bottom        , val.color_bottom        , src_val.color_bottom        );
         changed|=Undo(color_left          , src.color_left          , val.color_left          , src_val.color_left          );
         changed|=Undo(color_right         , src.color_right         , val.color_right         , src_val.color_right         );
         changed|=Undo(images_size         , src.images_size         , val.images_size         , src_val.images_size         );
         changed|=Undo(top_height          , src.top_height          , val.top_height          , src_val.top_height          );
         changed|=Undo(bottom_height       , src.bottom_height       , val.bottom_height       , src_val.bottom_height       );
         changed|=Undo(left_right_width    , src.left_right_width    , val.left_right_width    , src_val.left_right_width    );
         changed|=Undo(top_corner_width    , src.top_corner_width    , val.top_corner_width    , src_val.top_corner_width    );
         changed|=Undo(bottom_corner_width , src.bottom_corner_width , val.bottom_corner_width , src_val.bottom_corner_width );
         changed|=Undo(top_image           , src.top_image           ,     top_id              , src    .top_id              );
         changed|=Undo(bottom_image        , src.bottom_image        ,     bottom_id           , src    .bottom_id           );
         changed|=Undo(center_image        , src.center_image        ,     center_id           , src    .center_id           );
         changed|=Undo(left_image          , src.left_image          ,     left_id             , src    .left_id             );
         changed|=Undo(right_image         , src.right_image         ,     right_id            , src    .right_id            );
         changed|=Undo(top_left_image      , src.top_left_image      ,     top_left_id         , src    .top_left_id         );
         changed|=Undo(top_right_image     , src.top_right_image     ,     top_right_id        , src    .top_right_id        );
         changed|=Undo(bottom_left_image   , src.bottom_left_image   ,     bottom_left_id      , src    .bottom_left_id      );
         changed|=Undo(bottom_right_image  , src.bottom_right_image  ,     bottom_right_id     , src    .bottom_right_id     );
         changed|=Undo(compressed_time     , src.compressed_time     ,     compressed          , src    .compressed          );
         changed|=Undo(mip_maps_time       , src.mip_maps_time       ,     mip_maps            , src    .mip_maps            );
         REPA(lights  )changed|=lights  [i].undo(val.lights  [i], src.lights  [i], src_val.lights  [i]);
         REPA(sections)changed|=sections[i].undo(val.sections[i], src.sections[i], src_val.sections[i]);
         return changed;
      }
      bool EditPanelImage::Base::save(File &f)C
      {
         f.cmpUIntV(1);
         f<<cut_left<<cut_right<<cut_bottom<<cut_top
          <<cut_corners<<cut_corner_slope<<cut_corner_amount
          <<force_uniform_stretch
          <<resolution<<width<<height
          <<round_corners<<left_slope<<right_slope
          <<max_side_stretch<<extend<<extend_inner_padd
          <<border_size
          <<outer_glow_spread<<outer_glow_radius<<inner_glow_radius<<outer_glow_color<<inner_glow_color
          <<light_ambient
          <<depth<<round_depth<<inner_distance<<smooth_depth
          <<shadow_radius<<shadow_opacity<<shadow_spread
          <<color<<color_top<<color_bottom<<color_left<<color_right
          <<images_size<<top_height<<bottom_height<<left_right_width<<top_corner_width<<bottom_corner_width
          <<top_image<<bottom_image<<center_image<<left_image<<right_image<<top_left_image<<top_right_image<<bottom_left_image<<bottom_right_image
          <<compressed_time<<mip_maps_time<<compressed<<mip_maps
          <<top_id<<bottom_id<<center_id<<left_id<<right_id<<top_left_id<<top_right_id<<bottom_left_id<<bottom_right_id;
         FREPAO(lights  ).save(f);
         FREPAO(sections).save(f);
         return f.ok();
      }
      bool EditPanelImage::Base::load(File &f)
      {
         reset(); switch(f.decUIntV())
         {
            case 1:
            {
               f>>cut_left>>cut_right>>cut_bottom>>cut_top
                >>cut_corners>>cut_corner_slope>>cut_corner_amount
                >>force_uniform_stretch
                >>resolution>>width>>height
                >>round_corners>>left_slope>>right_slope
                >>max_side_stretch>>extend>>extend_inner_padd
                >>border_size
                >>outer_glow_spread>>outer_glow_radius>>inner_glow_radius>>outer_glow_color>>inner_glow_color
                >>light_ambient
                >>depth>>round_depth>>inner_distance>>smooth_depth
                >>shadow_radius>>shadow_opacity>>shadow_spread
                >>color>>color_top>>color_bottom>>color_left>>color_right
                >>images_size>>top_height>>bottom_height>>left_right_width>>top_corner_width>>bottom_corner_width
                >>top_image>>bottom_image>>center_image>>left_image>>right_image>>top_left_image>>top_right_image>>bottom_left_image>>bottom_right_image
                >>compressed_time>>mip_maps_time>>compressed>>mip_maps
                >>top_id>>bottom_id>>center_id>>left_id>>right_id>>top_left_id>>top_right_id>>bottom_left_id>>bottom_right_id;
               FREPA(lights  )if(!lights  [i].load(f))goto error;
               FREPA(sections)if(!sections[i].load(f))goto error;
               if(f.ok())return true;
            }break;

            case 0:
            {
               f>>cut_left>>cut_right>>cut_bottom>>cut_top
                >>cut_corners>>cut_corner_slope>>cut_corner_amount
                >>force_uniform_stretch
                >>resolution>>width>>height
                >>round_corners>>left_slope>>right_slope
                >>max_side_stretch>>extend
                >>border_size
                >>outer_glow_spread>>outer_glow_radius>>inner_glow_radius>>outer_glow_color>>inner_glow_color
                >>light_ambient
                >>depth>>round_depth>>inner_distance>>smooth_depth
                >>shadow_radius>>shadow_opacity>>shadow_spread
                >>color>>color_top>>color_bottom>>color_left>>color_right
                >>images_size>>top_height>>bottom_height>>left_right_width>>top_corner_width>>bottom_corner_width
                >>top_image>>bottom_image>>center_image>>left_image>>right_image>>top_left_image>>top_right_image>>bottom_left_image>>bottom_right_image
                >>compressed_time>>mip_maps_time>>compressed>>mip_maps
                >>top_id>>bottom_id>>center_id>>left_id>>right_id>>top_left_id>>top_right_id>>bottom_left_id>>bottom_right_id;
               FREPA(lights  )if(!lights  [i].load(f))goto error;
               FREPA(sections)if(!sections[i].load(f))goto error;
               if(f.ok())return true;
            }break;
         }
      error:
         reset(); return false;
      }
   void EditPanelImage::reset(                     )  {T=EditPanelImage();}
   void EditPanelImage::newData(                     )  {       base.newData();}
   bool EditPanelImage::newer(C EditPanelImage &src)C {return base.newer(src.base);}
   bool EditPanelImage::equal(C EditPanelImage &src)C {return base.equal(src.base);}
   bool EditPanelImage::sync(C EditPanelImage &src)  {return base.sync(T, src.base, src);}
   bool EditPanelImage::undo(C EditPanelImage &src)  {return base.undo(T, src.base, src);}
      EditPanelImage::Images::Images(Project &proj, bool fast) : proj(proj) {T.fast=fast;}
      Image* EditPanelImage::Images::get(C UID &image_id)
      {
         if(!image_id.valid())return null;
         REPA(images)if(images[i].id==image_id)return images[i].image_ptr();
         Src &src=images.New(); src.id=image_id;
         if(!fast && src.image.ImportTry(proj.editPath(image_id)))src.image_ptr=&src.image;
         if(!src.image_ptr)src.image_ptr=proj.gamePath(image_id);
         return src.image_ptr();
      }
   void EditPanelImage::make(PanelImage &panel_image, Threads &threads, Project &proj, Image *soft, Image *depth_map, bool fast)
   {
      Images images(proj, fast);
      PanelImageParams params=T;
      params.top_image=images.get(base.top_id);
      params.bottom_image=images.get(base.bottom_id);
      params.center_image=images.get(base.center_id);
      params.left_image=images.get(base.left_id);
      params.right_image=images.get(base.right_id);
      params.top_left_image=images.get(base.top_left_id);
      params.top_right_image=images.get(base.top_right_id);
      params.bottom_left_image=images.get(base.bottom_left_id);
      params.bottom_right_image=images.get(base.bottom_right_id);
      REPA(sections)
      {
         params.sections[i].depth_overlay=images.get(base.sections[i].depth_overlay_id);
         params.sections[i].color_overlay=images.get(base.sections[i].color_overlay_id);
         params.sections[i].reflection   =images.get(base.sections[i].   reflection_id);
      }
      panel_image.create(params, depth_map, fast ? 2 : 6, FILTER_BEST, &threads);
      IMAGE_TYPE type=panel_image.image.type(); if(base.compressed)ImageProps(panel_image.image, null, &type);
      Image *src=&panel_image.image;
      if(soft){Swap(*soft, *src); src=soft;}
      src->copyTry(panel_image.image, -1, -1, -1, type, IMAGE_2D, base.mip_maps ? 0 : 1, FILTER_BEST, true, true);
   }
   bool EditPanelImage::save(File &f)C
   {
      f.cmpUIntV(0);
       base.save(f);
      ::EE::PanelImageParams::save(f);
      return f.ok();
   }
   bool EditPanelImage::load(File &f)
   {
      switch(f.decUIntV())
      {
         case 0:
         {
            if( base.load(f))
            if(::EE::PanelImageParams::load(f))
               if(f.ok())return true;
         }break;
      }
      reset(); return false;
   }
   bool EditPanelImage::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
EditPanelImage::Section::Section() : depth_overlay_id(UIDZero), color_overlay_id(UIDZero), reflection_id(UIDZero) {}

EditPanelImage::Base::Base() : compressed(false), mip_maps(false), top_id(UIDZero), bottom_id(UIDZero), center_id(UIDZero), left_id(UIDZero), right_id(UIDZero), top_left_id(UIDZero), top_right_id(UIDZero), bottom_left_id(UIDZero), bottom_right_id(UIDZero) {}

EditPanelImage::Images::Src::Src() : id(UIDZero) {}

/******************************************************************************/
