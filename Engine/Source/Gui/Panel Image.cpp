/******************************************************************************/
#include "stdafx.h"
#define CC4_PIMG CC4('P','I','M','G')
namespace EE{
/******************************************************************************

   Vertex Shear:

   Flt shear=Ms.pos().x, offset=shear*-rect.centerY();
   REP(vtxs)v[i].pos.x+=v[i].pos.y*shear+offset;

/******************************************************************************/
DEFINE_CACHE(PanelImage, PanelImages, PanelImagePtr, "Panel Image");
/******************************************************************************/
static inline Flt LightSpecularBase(C Vec &nrm, C Vec &light_dir, C Vec &eye_dir)
{
#if 0 // blinn
   return Sat(Dot(nrm, !(light_dir+eye_dir)));
#else // phong, use phong in this case because it has nicer specular on the edges
   Vec reflection=!(nrm*(2*Dot(nrm, light_dir)) - light_dir);
   return Sat(Dot(reflection, eye_dir));
#endif
}
static inline Flt LightSpecular(C Vec &nrm, C Vec &light_dir, C Vec &eye_dir, Flt power=64)
{
   return Pow(LightSpecularBase(nrm, light_dir, eye_dir), power);
}
static void GetFracMulAddFromValues(Flt from, Flt to, Flt &mul, Flt &add)
{
   // from*mul+add=0
   // to  *mul+add=1
   Flt d=to-from;
   mul=(d ? 1.0f/d : 0); add=-from*mul;
}
static void AdjustFracMulAdd_1_1(Flt &mul, Flt &add) // adjust frac mul add (0..1) to (-1..1)
{
   // from*mul'+add'=-1
   // to  *mul'+add'= 1
   // mul'=mul*2
   // add'=-1 - from*mul*2
   // add'=-1 + add*2
   mul*=2;
   add =add*2-1;
}
// 'x0' and 'y0' are inclusive, while 'x1' and 'y1' are exclusive
static void GetFracMulAddFromImage(Int x0, Int x1, Int y0, Int y1, C Image *image, Vec4 &frac_mul_add, Bool mirror_x=false, Bool mirror_y=false)
{
   if(image)
   {
      if(mirror_x)Swap(x0, x1);
      if(mirror_y)Swap(y0, y1);
      Int w=x1-x0, h=y1-y0;
      // x0*frac_mul_add.x+frac_mul_add.y=0
      // y0*frac_mul_add.z+frac_mul_add.w=0
      frac_mul_add.x=Flt(image->w())/w; frac_mul_add.y=-x0*frac_mul_add.x + frac_mul_add.x*0.5f-0.5f;
      frac_mul_add.z=Flt(image->h())/h; frac_mul_add.w=-y0*frac_mul_add.z + frac_mul_add.z*0.5f-0.5f;
   }
}
static void ScaleTex(Flt  &tex,   Flt   scale) {tex=(tex-0.5f)*scale+0.5f;}
static void ScaleTex(Vec2 &tex, C Vec2 &scale) {ScaleTex(tex.x, scale.x); ScaleTex(tex.y, scale.y);}
/******************************************************************************/
struct SmoothDepth
{
   Bool is[2];
   Vec2 val, sqr;

   SmoothDepth() {}
   SmoothDepth(C Vec2 &smooth_depth)
   {
      is[0]=   (smooth_depth.x>EPS);
      is[1]=   (smooth_depth.y>EPS);
      val  =    smooth_depth;
      sqr  =Sqr(smooth_depth);
   }
};
struct SectionParams
{
   Flt    size, size_top;
   Vec2   frac_mul, frac_add, norm_mul, norm_add;
   Vec4   color_top, color_bottom, color_left, color_right,
          outer_color, inner_color,
          outer_border_color, inner_border_color, prev_border_color;

   Flt    noise_intensity, noise_uv_scale, noise_uv_warp;
   Image  noise_map;

   Flt    overlay_intensity, overlay_uv_scale, overlay_uv_warp;
   Vec2   overlay_uv_offset;
   Image  overlay_image;
 C Image *overlay;

   Bool   reflection_one;
   Image  reflection_image;
 C Image *reflection; 

   SmoothDepth smooth_depth;

   SectionParams()
   {
      overlay=reflection=null;
   }
  ~SectionParams()
   {
      if(overlay   )overlay   ->unlock();
      if(reflection)reflection->unlock();
   }

   void setColors(C PanelImageParams::Section &src)
   {
      color_top   =src.color_top   .asVec4();
      color_bottom=src.color_bottom.asVec4();
      color_left  =src.color_left  .asVec4();
      color_right =src.color_right .asVec4();
      outer_color =src.outer_color .asVec4();
      inner_color =src.inner_color .asVec4();
      outer_border_color=src.outer_border_color.asVec4();
      inner_border_color=src.inner_border_color.asVec4();
       prev_border_color=src. prev_border_color.asVec4();
      Vec4 src_color=src.color.asVec4(); outer_color*=src_color; inner_color*=src_color;
   }
   void setColorY(Flt section_frac, C Vec4 &global_color_y, Vec4 &section_color_y)
   {
      section_color_y=Lerp(color_top, color_bottom, section_frac)*global_color_y;
   }
   void setDepthNoise(C PanelImageParams::Section &src, Int w, Int h, Int super_sample, Threads *threads)
   {
      if(src.depth_noise.is())
      {
         Randomizer random(UIDZero);
         noise_map.createSoftTry(w/super_sample, h/super_sample, 1, IMAGE_F32);
         REPD(y, noise_map.h())
         REPD(x, noise_map.w())noise_map.pixF(x, y)=random.f();
         noise_map.blur(src.depth_noise.blur, src.depth_noise.blur_clamp, threads);
         noise_intensity=src.depth_noise.intensity*((src.depth_noise.mode==PanelImageParams::ImageParams::SCALE) ? 1.5f : 1);
         noise_uv_scale =src.depth_noise.uv_scale/super_sample;
         noise_uv_warp  =src.depth_noise.uv_warp*noise_map.h();
      }else noise_map.del();
   }
   void setColorNoise(C PanelImageParams::Section &src, Int w, Int h, Int super_sample, Threads *threads)
   {
      if(src.color_noise.is())
      {
         Randomizer random(UIDZero);
         noise_map.createSoftTry(w/super_sample, h/super_sample, 1, IMAGE_F32);
         REPD(y, noise_map.h())
         REPD(x, noise_map.w())noise_map.pixF(x, y)=random.f();
         noise_map.blur(src.color_noise.blur, src.color_noise.blur_clamp, threads);
         noise_intensity=src.color_noise.intensity*((src.color_noise.mode==PanelImageParams::ImageParams::SCALE) ? 1.5f : 1);
         noise_uv_scale =src.color_noise.uv_scale/super_sample;
         noise_uv_warp  =src.color_noise.uv_warp*noise_map.h();
      }else noise_map.del();
   }
   void setDepthOverlay(C PanelImageParams::Section &src, Int resolution, Threads *threads)
   {
      if(overlay){overlay->unlock(); overlay=null;}
      if(overlay=src.depth_overlay)
      {
         if(overlay->compressed() || src.depth_overlay_params.blur)
            if(overlay->copyTry(overlay_image, -1, -1, -1, ImageTI[overlay->hwType()].a ? IMAGE_F32_4 : IMAGE_F32, IMAGE_SOFT, 1))overlay=&overlay_image;else overlay=null;
         if(overlay)
         {
            overlay_image.blur(src.depth_overlay_params.blur, src.depth_overlay_params.blur_clamp, threads); // this will blur only if 'src.depth_overlay_params.blur' in which case the "overlay==&overlay_image"
            overlay->lockRead();
            overlay_intensity=src.depth_overlay_params.intensity*((src.depth_overlay_params.mode==PanelImageParams::ImageParams::SCALE) ? 1.5f : 1);
            overlay_uv_scale =src.depth_overlay_params.uv_scale*(Flt(overlay->h())/resolution);
            overlay_uv_warp  =src.depth_overlay_params.uv_warp*overlay->h();
            overlay_uv_offset=overlay_uv_scale*0.5f-0.5f+src.depth_overlay_params.uv_offset*(overlay->size()*VecI2(-1, 1)); // change X sign in uv_offset so when editing it in editor using property mouse edit, the image moves along the mouse movement
         }
      }
   }
   void setColorOverlay(C PanelImageParams::Section &src, Int resolution, Threads *threads)
   {
      if(overlay){overlay->unlock(); overlay=null;}
      if(overlay=src.color_overlay)
      {
         if(overlay->compressed() || src.color_overlay_params.blur)
            if(overlay->copyTry(overlay_image, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))overlay=&overlay_image;else overlay=null;
         if(overlay)
         {
            overlay_image.blur(src.color_overlay_params.blur, src.color_overlay_params.blur_clamp, threads); // this will blur only if 'src.color_overlay_params.blur' in which case the "overlay==&overlay_image"
            overlay->lockRead();
            overlay_intensity=src.color_overlay_params.intensity*((src.color_overlay_params.mode==PanelImageParams::ImageParams::SCALE) ? 1.5f : 1);
            overlay_uv_scale =src.color_overlay_params.uv_scale*(Flt(overlay->h())/resolution);
            overlay_uv_warp  =src.color_overlay_params.uv_warp*overlay->h();
            overlay_uv_offset=overlay_uv_scale*0.5f-0.5f+src.color_overlay_params.uv_offset*(overlay->size()*VecI2(-1, 1)); // change X sign in uv_offset so when editing it in editor using property mouse edit, the image moves along the mouse movement
         }
      }
   }
   void setReflection(C PanelImageParams::Section &src)
   {
      if(reflection=src.reflection)
      {
         if(reflection->compressed() || reflection->cube())if(reflection->copyTry(reflection_image, -1, -1, -1, reflection->compressed() ? IMAGE_R8G8B8A8 : -1, IMAGE_SOFT, 1))reflection=&reflection_image;else reflection=null;
         if(reflection)
         {
            reflection->lockRead();
            reflection_one=(reflection->aspect()<AvgF(1, 6)); // source is only 1 face, not "6*face"
         }
      }
   }
   void setSize(Flt size, Flt top_offset)
   {
      T.size    =size;
      T.size_top=Max(size+top_offset, 0);
   }
   void setOffset(Flt offset, Flt offset_top, Int resolution, Int image_w, Int image_h)
   {
      Flt o=offset    *0.5f*resolution,
          t=offset_top*0.5f*resolution;
      GetFracMulAddFromValues(o, image_w-1-o, frac_mul.x, frac_add.x);
      GetFracMulAddFromValues(t, image_h-1-o, frac_mul.y, frac_add.y);

      norm_mul=frac_mul;
      norm_add=frac_add;
      REP(2)AdjustFracMulAdd_1_1(norm_mul.c[i], norm_add.c[i]);
   }
   void setParams(C PanelImageParams::Section &src, Int resolution, Threads *threads)
   {
      smooth_depth   =src.smooth_depth;
      setSize        (src.size, src.top_offset);
      setColors      (src);
      setDepthOverlay(src, resolution, threads);
      setReflection  (src);
   }
};
/******************************************************************************/
// PANEL IMAGE PARAMS
/******************************************************************************/
PanelImageParams::Light::Light()
{
   enabled=false;
   intensity=0.5f;
   back=0.25f;
   highlight=0.0f;
   highlight_cut=specular_highlight_cut=0.8f;
   specular=1.0f;
   specular_back=0.0f;
   specular_exp=32;
   specular_highlight=0.0f;
   angle.set(0, 0.4f);
}
Bool PanelImageParams::Light::save(File &f)C
{
   f.cmpUIntV(0);
   f<<enabled<<intensity<<back<<highlight<<highlight_cut<<specular<<specular_back<<specular_exp<<specular_highlight<<specular_highlight_cut<<angle;
   return f.ok();
}
Bool PanelImageParams::Light::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>enabled>>intensity>>back>>highlight>>highlight_cut>>specular>>specular_back>>specular_exp>>specular_highlight>>specular_highlight_cut>>angle;
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
PanelImageParams::ImageParams::ImageParams()
{
   blur_clamp=false;
   blur      =0;
   uv_scale  =1.0f;
   uv_offset =0.0f;
   uv_warp   =0.0f;
   intensity =0.0f;
   mode      =MULTIPLY;
}
Bool PanelImageParams::ImageParams::save(File &f)C
{
   f.cmpUIntV(0); // !! WARNING: IN THE FUTURE SAVE 'mode' AS 1 BYTE USING << >> !!
   f<<blur_clamp<<blur<<uv_scale<<uv_offset<<uv_warp<<intensity; f.putUInt(mode);
   return f.ok();
}
Bool PanelImageParams::ImageParams::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>blur_clamp>>blur>>uv_scale>>uv_offset>>uv_warp>>intensity; mode=MODE(f.getUInt());
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
PanelImageParams::Section::Section()
{
   size=0.0f;
   top_offset=0;
   round_depth   =1.0f;
   outer_depth   =1.0f;
   inner_depth   =1.0f;
   inner_distance=0.5f;
   smooth_depth.zero();

   specular=0.0f;
   reflection_intensity=0.3f;
   color=outer_color=inner_color=color_top=color_bottom=color_left=color_right=WHITE;
   outer_border_color=BLACK; outer_border_color.a=128;
   inner_border_color=WHITE; inner_border_color.a=64;
    prev_border_color=TRANSPARENT;

   color_overlay=depth_overlay=reflection=null;
   depth_overlay_params.intensity=1.0f;
   color_overlay_params.intensity=1.0f;
   color_noise         .uv_warp  =1.0f;
}
Bool PanelImageParams::Section::save(File &f)C
{
   f.cmpUIntV(0);
   f<<size<<top_offset<<round_depth<<outer_depth<<inner_depth<<inner_distance<<specular<<reflection_intensity<<smooth_depth
    <<color<<outer_color<<inner_color<<color_top<<color_bottom<<color_left<<color_right<<outer_border_color<<inner_border_color<<prev_border_color;
   if(depth_overlay_params.save(f))
   if(color_overlay_params.save(f))
   if(depth_noise         .save(f))
   if(color_noise         .save(f))
      return f.ok();
   return false;
}
Bool PanelImageParams::Section::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>size>>top_offset>>round_depth>>outer_depth>>inner_depth>>inner_distance>>specular>>reflection_intensity>>smooth_depth
          >>color>>outer_color>>inner_color>>color_top>>color_bottom>>color_left>>color_right>>outer_border_color>>inner_border_color>>prev_border_color;
         if(depth_overlay_params.load(f))
         if(color_overlay_params.load(f))
         if(depth_noise         .load(f))
         if(color_noise         .load(f))
            if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
PanelImageParams::PanelImageParams()
{
   cut_left=cut_right=cut_bottom=cut_top=false;
   Zero(cut_corners); Zero(force_uniform_stretch);
   resolution=64;
   width=height=1;
   round_corners=0.5f;
   cut_corner_slope=1.0f;
   cut_corner_amount=1.0f;
   left_slope=right_slope=0.0f;
   light_ambient=0.5f;
   border_size=2.0f/resolution; // this sets border to exactly one pixel
   outer_glow_spread=0.5f;
   outer_glow_radius=inner_glow_radius=border_size*12;
   max_side_stretch=0.2f;
   extend.zero();
   extend_inner_padd.zero();

   depth         =1.0f;
   round_depth   =1.0f;
   inner_distance=0.5f;
   smooth_depth.zero();

   color=color_top=color_bottom=color_left=color_right=WHITE;
   outer_glow_color=inner_glow_color=TRANSPARENT;

   shadow_radius=0;
   shadow_opacity=0.5f;
   shadow_spread=0.5f;

   lights[0].enabled=true;
   sections[0].inner_distance=1.0f;

   images_size=0.5f;
   top_height=bottom_height=left_right_width=top_corner_width=bottom_corner_width=1;
   top_image=bottom_image=center_image=left_image=right_image=top_left_image=top_right_image=bottom_left_image=bottom_right_image=null;
}
Bool PanelImageParams::save(File &f)C
{
   f.cmpUIntV(1);
   f<<cut_left<<cut_right<<cut_bottom<<cut_top<<cut_corners<<force_uniform_stretch
    <<resolution<<width<<height<<round_corners<<cut_corner_slope<<cut_corner_amount<<left_slope<<right_slope
    <<border_size<<outer_glow_spread<<outer_glow_radius<<inner_glow_radius
    <<light_ambient<<depth<<round_depth<<inner_distance<<shadow_radius<<shadow_opacity<<shadow_spread<<max_side_stretch<<smooth_depth<<extend<<extend_inner_padd
    <<color<<color_top<<color_bottom<<color_left<<color_right<<outer_glow_color<<inner_glow_color
    <<images_size<<top_height<<bottom_height<<left_right_width<<top_corner_width<<bottom_corner_width;
   FREPA(lights  )if(!lights  [i].save(f))return false;
   FREPA(sections)if(!sections[i].save(f))return false;
   return f.ok();
}
Bool PanelImageParams::load(File &f)
{
   switch(f.decUIntV())
   {
      case 1:
      {
         f>>cut_left>>cut_right>>cut_bottom>>cut_top>>cut_corners>>force_uniform_stretch
          >>resolution>>width>>height>>round_corners>>cut_corner_slope>>cut_corner_amount>>left_slope>>right_slope
          >>border_size>>outer_glow_spread>>outer_glow_radius>>inner_glow_radius
          >>light_ambient>>depth>>round_depth>>inner_distance>>shadow_radius>>shadow_opacity>>shadow_spread>>max_side_stretch>>smooth_depth>>extend>>extend_inner_padd
          >>color>>color_top>>color_bottom>>color_left>>color_right>>outer_glow_color>>inner_glow_color
          >>images_size>>top_height>>bottom_height>>left_right_width>>top_corner_width>>bottom_corner_width;
         FREPA(lights  )if(!lights  [i].load(f))goto error;
         FREPA(sections)if(!sections[i].load(f))goto error;
         if(f.ok())return true;
      }break;

      case 0:
      {
         f>>cut_left>>cut_right>>cut_bottom>>cut_top>>cut_corners>>force_uniform_stretch
          >>resolution>>width>>height>>round_corners>>cut_corner_slope>>cut_corner_amount>>left_slope>>right_slope
          >>border_size>>outer_glow_spread>>outer_glow_radius>>inner_glow_radius
          >>light_ambient>>depth>>round_depth>>inner_distance>>shadow_radius>>shadow_opacity>>shadow_spread>>max_side_stretch>>smooth_depth>>extend
          >>color>>color_top>>color_bottom>>color_left>>color_right>>outer_glow_color>>inner_glow_color
          >>images_size>>top_height>>bottom_height>>left_right_width>>top_corner_width>>bottom_corner_width;
         FREPA(lights  )if(!lights  [i].load(f))goto error;
         FREPA(sections)if(!sections[i].load(f))goto error;
         extend_inner_padd.zero();
         if(f.ok())return true;
      }break;
   }
error:
   return false;
}
/******************************************************************************/
// PANEL IMAGE
/******************************************************************************/
PanelImage::PanelImage() {zero();}
void PanelImage::zero()
{
  _same_x=_padd_any=false;
   REPAO(_force_uniform_stretch)=false;
   REPAD(y, _size_x)REPAD(x, _size_x[y])_size_x[y][x]=0;
   REPAD(y,  _tex_x)REPAD(x,  _tex_x[y]) _tex_x[y][x]=0;
   REPAO(   _size_y)=0;
   REPAO(    _tex_y)=0;
  _side_size       .zero();
  _padd            .zero();
  _tex_left_top    .zero();
  _tex_right_bottom.zero();
  _inner_padding   .zero();
}
void PanelImage::del() {image.del(); zero();}
/******************************************************************************/
#define MEMBER_ELMS2(Class, member) ELMS(MEMBER(Class, member)) // get elements of member in class

#define LIGHTS MEMBER_ELMS2(PanelImageParams, lights)

struct PanelImageCreate
{
   struct Line
   {
      Bool on;
      Vec2 pos, normal, dir;

      void finalize(Bool on)
      {
         T.on=on;
         normal.normalize();
         dir=Perp(normal);
      }
      Flt yAtX(Flt x)C {if(!dir.x)return pos.y; Flt dx=x-pos.x, d=dx/dir.x; return pos.y+d*dir.y;}
      Flt xAtY(Flt y)C {if(!dir.y)return pos.x; Flt dy=y-pos.y, d=dy/dir.y; return pos.x+d*dir.x;}
   };
   struct ImageSrc
   {
    C Image *src ; // we keep a pointer to the original image, to avoid creating software copies of the same image multiple times (in case it's assigned to multiple slots), also this is necessary to detect mirroring
      Image  soft;
   };

   Int               resolution, super_sample;
   PanelImage       &panel_image;
 C PanelImageParams &params;
   Image            &image, map, dist_map, *depth_map;
   Vec               light_dir[LIGHTS], light_dir_neg[LIGHTS];
   VecI2             image_size, image_size1, image_size_2i;
   Vec2              image_size_2, corner_size;
   SmoothDepth       smooth_depth;
   SectionParams     sps[MEMBER_ELMS2(PanelImageParams, sections)], &last;
   Flt               top_offset, max_scale;
   Vec4              color_top, color_bottom, color_left, color_right, inner_glow_color;
   Memb<ImageSrc>    temp_images; // use 'Memb' because pointers to elements are stored
 C Image            *top_image, *bottom_image, *center_image, *left_image, *right_image, *top_left_image, *top_right_image, *bottom_left_image, *bottom_right_image;
   Int               x3, y3, y1, y2, x1, x2, top_x1, top_x2, bottom_x1, bottom_x2;
   Vec4              image_frac_mul_add[3][3]; // [x][y]
 C Vec2             &full_frac_mul, //&full_frac_add, // this is always zero
                    &full_norm_mul,   &full_norm_add;
   Line              corner_line[2][2], // [y][x]
                       side_line[2]; // [x]
   Threads          *threads;

   PanelImageCreate(PanelImage &panel_image, C PanelImageParams &params, Image *depth_map, Int super_sample, Threads *threads)
   : panel_image(panel_image), params(params), depth_map(depth_map), image(panel_image.image), last(sps[Elms(sps)-1]), threads(threads),
     full_frac_mul(last.frac_mul), full_norm_mul(last.norm_mul), full_norm_add(last.norm_add) // last section is always full
   {
      if(depth_map)depth_map->del();
      T.super_sample=Mid(super_sample, 1, 16);
      T.resolution  =params.resolution*T.super_sample;
      top_image=bottom_image=center_image=left_image=right_image=top_left_image=top_right_image=bottom_left_image=bottom_right_image=null;
   }
  ~PanelImageCreate()
   {
      if(         top_image)         top_image->unlock();
      if(      bottom_image)      bottom_image->unlock();
      if(      center_image)      center_image->unlock();
      if(        left_image)        left_image->unlock();
      if(       right_image)       right_image->unlock();
      if(    top_left_image)    top_left_image->unlock();
      if(   top_right_image)   top_right_image->unlock();
      if( bottom_left_image) bottom_left_image->unlock();
      if(bottom_right_image)bottom_right_image->unlock();
   }
 C Image* getImage(C Image *image)
   {
      if(image && image->compressed())
      {
         REPA(temp_images)if(temp_images[i].src==image){image=&temp_images[i].soft; goto found;} // goto found and lock it, because it's always unlocked in the codes below
         ImageSrc &temp=temp_images.New(); temp.src=image;
         if(image->copyTry(temp.soft, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))image=&temp.soft;else image=null;
      }
   found:
      return (image && image->lockRead()) ? image : null;
   }
   void alignTex()
   {
      REPAD(y, panel_image._tex_x)
      {
         panel_image._tex_x[y][0]=AlignCeil (panel_image._tex_x[y][0], super_sample);
         panel_image._tex_x[y][1]=AlignFloor(panel_image._tex_x[y][1], super_sample);
      }
      panel_image._tex_y[0]=AlignCeil (panel_image._tex_y[0], super_sample);
      panel_image._tex_y[1]=AlignFloor(panel_image._tex_y[1], super_sample);
   }
   Bool create()
   {
      if(image   .createSoftTry(resolution*params.width, resolution*params.height, 1, IMAGE_R8G8B8A8)
      && map     .createSoftTry(image.w(), image.h(), 1, IMAGE_F32  )
      && dist_map.createSoftTry(image.w(), image.h(), 1, IMAGE_F32_4))
      {
         REPA(params.lights)
         {
            light_dir    [i]=Matrix3().setRotateY(Sat(params.lights[i].angle.length())*PI_2).rotateZ(-Angle(params.lights[i].angle)).z;
            light_dir_neg[i]=-light_dir[i];
         }
         image_size=image.size(); image_size1=image_size-1; image_size_2i=image_size/2; image_size_2=image_size*0.5f;
         smooth_depth=params.smooth_depth;
         corner_size=resolution*0.5f*params.round_corners;
         REPAO(sps).setParams(params.sections[i], resolution, threads); sps[0].setSize(1-params.sections[1].size, 0);
         Flt offset=0, offset_top=0;
         REPA(sps) // go from the end
         {
            sps[i].setOffset(offset, offset_top, resolution, image.w(), image.h());
            offset    +=sps[i].size    ;
            offset_top+=sps[i].size_top;
         }
         top_offset=-(last.size_top-last.size)*0.5f*resolution;

         color_top   =params.color_top   .asVec4()*params.color.asVec4();
         color_bottom=params.color_bottom.asVec4()*params.color.asVec4();
         color_left  =params.color_left  .asVec4();
         color_right =params.color_right .asVec4();
   inner_glow_color  =params.inner_glow_color.asVec4();

         Flt cut_corner_amount=params.cut_corner_amount*0.5f*resolution, cut_corner_slope=Max(0, params.cut_corner_slope);
         corner_line[0][0].pos.set(            0,             0+cut_corner_amount); corner_line[0][0].normal.set( 1,  cut_corner_slope);
         corner_line[0][1].pos.set(image_size1.x,             0+cut_corner_amount); corner_line[0][1].normal.set(-1,  cut_corner_slope);
         corner_line[1][0].pos.set(            0, image_size1.y-cut_corner_amount); corner_line[1][0].normal.set( 1, -cut_corner_slope);
         corner_line[1][1].pos.set(image_size1.x, image_size1.y-cut_corner_amount); corner_line[1][1].normal.set(-1, -cut_corner_slope);
         Bool line_on=(cut_corner_amount>0.5f);
         REPD(y, 2)
         REPD(x, 2)corner_line[y][x].finalize(line_on && params.cut_corners[y][x]);

         side_line[0].pos.set(0, (params.left_slope<0) ? 0 : image_size1.y); side_line[0].normal.set(1, params.left_slope/params.height);
         side_line[0].finalize(!Equal(params.left_slope, 0));

         side_line[1].pos.set(image_size1.x, (params.right_slope<0) ? 0 : image_size1.y); side_line[1].normal.set(-1, params.right_slope/params.height);
         side_line[1].finalize(!Equal(params.right_slope, 0));

         if(params.cut_top   ) REPD(x, 2)corner_line[0][x].on=false;
         if(params.cut_bottom) REPD(x, 2)corner_line[1][x].on=false;
         if(params.cut_left  ){REPD(y, 2)corner_line[y][0].on=false; side_line[0].on=false;}
         if(params.cut_right ){REPD(y, 2)corner_line[y][1].on=false; side_line[1].on=false;}

             top_image=getImage(params.         top_image);
          bottom_image=getImage(params.      bottom_image);
          center_image=getImage(params.      center_image);
            left_image=getImage(params.        left_image);
           right_image=getImage(params.       right_image);
        top_left_image=getImage(params.    top_left_image);
       top_right_image=getImage(params.   top_right_image);
     bottom_left_image=getImage(params. bottom_left_image);
    bottom_right_image=getImage(params.bottom_right_image);

         // following bottom-right coordinates are exclusive
         Flt mul=0.5f*resolution*params.images_size;
         x3=image_size.x; // don't subtract one because here the right  coordinate is exclusive
         y3=image_size.y; // don't subtract one because here the bottom coordinate is exclusive
         y1=Min(   Round(params.          top_height*mul), image_size_2.y);
         y2=Max(y3-Round(params.       bottom_height*mul), image_size_2.y);
         x1=Min(   Round(params.   left_right_width *mul), image_size_2.x);
         x2=Max(x3-Round(params.   left_right_width *mul), image_size_2.x);
     top_x1=Min(   Round(params.   top_corner_width *mul), image_size_2.x);
     top_x2=Max(x3-Round(params.   top_corner_width *mul), image_size_2.x);
  bottom_x1=Min(   Round(params.bottom_corner_width *mul), image_size_2.x);
  bottom_x2=Max(x3-Round(params.bottom_corner_width *mul), image_size_2.x);

         GetFracMulAddFromImage(        0,    top_x1,  0, y1,     top_left_image, image_frac_mul_add[0][0], top_left_image==top_right_image);
         GetFracMulAddFromImage(   top_x1,    top_x2,  0, y1,          top_image, image_frac_mul_add[1][0]);
         GetFracMulAddFromImage(   top_x2,        x3,  0, y1,    top_right_image, image_frac_mul_add[2][0]);

         GetFracMulAddFromImage(        0,        x1, y1, y2,         left_image, image_frac_mul_add[0][1], left_image==right_image);
         GetFracMulAddFromImage(       x1,        x2, y1, y2,       center_image, image_frac_mul_add[1][1]);
         GetFracMulAddFromImage(       x2,        x3, y1, y2,        right_image, image_frac_mul_add[2][1]);

         GetFracMulAddFromImage(        0, bottom_x1, y2, y3,  bottom_left_image, image_frac_mul_add[0][2], bottom_left_image==bottom_right_image, bottom_left_image==top_left_image);
         GetFracMulAddFromImage(bottom_x1, bottom_x2, y2, y3,       bottom_image, image_frac_mul_add[1][2], false, bottom_image==top_image);
         GetFracMulAddFromImage(bottom_x2,        x3, y2, y3, bottom_right_image, image_frac_mul_add[2][2], false, bottom_right_image==top_right_image);

         Flt tex=0, tex_top;
         // set section #0
         if(params.sections[0].outer_border_color.a)MAX(tex, params.border_size  );
         if(params.sections[0].inner_border_color.a)MAX(tex, params.border_size*2);
         if(params.depth && !Equal(params.sections[0].outer_depth, params.sections[0].inner_depth))MAX(tex, params.sections[0].inner_distance);
         // add section #1
         tex+=sps[1].size;
         // globals affected by 'top_offset'
         MAX(tex, params.round_corners);
         // calculate
         tex*=0.5f*resolution;
         tex_top=tex-top_offset;
         // globals unaffected by 'top_offset'
         Flt global=0;
         if(!Equal(params.depth, 0)  )MAX(global, params.inner_distance   );
         if(params.inner_glow_color.a)MAX(global, params.inner_glow_radius);
         global*=0.5f*resolution;
         MAX(tex    , global);
         MAX(tex_top, global);

         // set to tex
         REPAD(y, panel_image._tex_x)REPAD(x, panel_image._tex_x[y])panel_image._tex_x[y][x]=(x ? image_size.x-tex : tex);
         panel_image._tex_y[0]=         tex_top;
         panel_image._tex_y[1]=image_size.y-tex;

         // tex values based on image presence
         if(    top_left_image ||    top_image)MAX(panel_image._tex_x[0][0],    top_x1); // any of these images require    'top_x1'
         if(   top_right_image ||    top_image)MIN(panel_image._tex_x[0][1],    top_x2); // any of these images require    'top_x2'
         if(        left_image || center_image)MAX(panel_image._tex_x[1][0],        x1); // any of these images require        'x1'
         if(       right_image || center_image)MIN(panel_image._tex_x[1][1],        x2); // any of these images require        'x2'
         if( bottom_left_image || bottom_image)MAX(panel_image._tex_x[2][0], bottom_x1); // any of these images require 'bottom_x1'
         if(bottom_right_image || bottom_image)MIN(panel_image._tex_x[2][1], bottom_x2); // any of these images require 'bottom_x2'

         if(   top_left_image ||    top_image ||    top_right_image
         ||        left_image || center_image ||        right_image)MAX(panel_image._tex_y[0], y1); // any of these images require 'y1'
         if(bottom_left_image || bottom_image || bottom_right_image
         ||        left_image || center_image ||        right_image)MIN(panel_image._tex_y[1], y2); // any of these images require 'y2'

         Flt tex_ofs=(0.5f+(super_sample>1))*super_sample; // if we're doing super-sampling then it means we'll have to down-sample the image later, this means we'll be getting neighbor pixels, so we need to padd more
         alignTex();

         // lines
         Bool include_size_mid[]={false, false};
         Flt  l=image_size.x, r=0;
         REPD(y, 3)
         {
            MIN(l, panel_image._tex_x[y][0]);
            MAX(r, panel_image._tex_x[y][1]);
         }
         REPD(y, 2)
         {
          C Line &l0=corner_line[y][0]; if(l0.on)
            {
               Vec2 n=l0.normal; if(y)n.y--;else n.y++; n/=Abs(n.y); Flt lt=n.x*tex; // line.normal+Vec2(0, 1)
               MAX(l, l0.xAtY(            0)+lt);
               MAX(l, l0.xAtY(image_size1.y)+lt);
            }
          C Line &l1=corner_line[y][1]; if(l1.on)
            {
               Vec2 n=l1.normal; if(y)n.y--;else n.y++; n/=Abs(n.y); Flt rt=n.x*tex; // line.normal+Vec2(0, 1)
               MIN(r, l1.xAtY(            0)+rt);
               MIN(r, l1.xAtY(image_size1.y)+rt);
            }
         }
         if(side_line[0].on)
         {
            Vec2 n=side_line[0].normal; if(n.y>0)n.y++;else n.y--; n/=Abs(n.y); Flt t=n.x*tex; // normal+Vec2(0, 1)
            MAX(l, side_line[0].xAtY(            0)+t);
            MAX(l, side_line[0].xAtY(image_size1.y)+t);
         }
         if(side_line[1].on)
         {
            Vec2 n=side_line[1].normal; if(n.y>0)n.y++;else n.y--; n/=Abs(n.y); Flt t=n.x*tex; // normal+Vec2(0, 1)
            MIN(r, side_line[1].xAtY(            0)+t);
            MIN(r, side_line[1].xAtY(image_size1.y)+t);
         }
         if(l+tex_ofs<r)REPD(y, 3)
         {
            MAX(panel_image._tex_x[y][0], l);
            MIN(panel_image._tex_x[y][1], r);
         }else include_size_mid[0]=true; // if lines overlap then we need to force middle part in the size calculation

         Flt t=panel_image._tex_y[0], b=panel_image._tex_y[1];
         REPD(x, 2)
         {
          C Line &l0=corner_line[0][x]; if(l0.on)
            {
               Vec2 n=l0.normal; if(x)n.x--;else n.x++; n/=Abs(n.x); Flt tt=n.y*tex; // line.normal+Vec2(1, 0)
               MAX(t, l0.yAtX(            0)+tt);
               MAX(t, l0.yAtX(image_size1.x)+tt);
            }
          C Line &l1=corner_line[1][x]; if(l1.on)
            {
               Vec2 n=l1.normal; if(x)n.x--;else n.x++; n/=Abs(n.x); Flt bt=n.y*tex; // line.normal+Vec2(1, 0)
               MIN(b, l1.yAtX(            0)+bt);
               MIN(b, l1.yAtX(image_size1.x)+bt);
            }
         }
         if(t+tex_ofs<b)
         {
            MAX(panel_image._tex_y[0], t);
            MIN(panel_image._tex_y[1], b);
         }else include_size_mid[1]=true; // if lines overlap then we need to force middle part in the size calculation

         if(params.force_uniform_stretch[0])REPAD(y, panel_image._tex_x)
         {
            panel_image._tex_x[y][0]=0;
            panel_image._tex_x[y][1]=image_size.x;
         }
         if(params.force_uniform_stretch[1])
         {
            panel_image._tex_y[0]=0;
            panel_image._tex_y[1]=image_size.y;
         }

         // align, offset and scale
         alignTex();
         REPAD(y, panel_image._tex_x)
         {
            if(!Equal(panel_image._tex_x[y][0],            0))panel_image._tex_x[y][0]+=tex_ofs;
            if(!Equal(panel_image._tex_x[y][1], image_size.x))panel_image._tex_x[y][1]-=tex_ofs;
            REPAD(x, panel_image._tex_x[y])panel_image._tex_x[y][x]/=image_size.x;
            if(panel_image._tex_x[y][0]>panel_image._tex_x[y][1])panel_image._tex_x[y][0]=panel_image._tex_x[y][1]=Avg(panel_image._tex_x[y][0], panel_image._tex_x[y][1]);
         }
         if(!Equal(panel_image._tex_y[0],            0))panel_image._tex_y[0]+=tex_ofs;
         if(!Equal(panel_image._tex_y[1], image_size.y))panel_image._tex_y[1]-=tex_ofs;
         REPAO(panel_image._tex_y)/=image_size.y;
         if(panel_image._tex_y[0]>panel_image._tex_y[1])panel_image._tex_y[0]=panel_image._tex_y[1]=Avg(panel_image._tex_y[0], panel_image._tex_y[1]);

         max_scale=Max(params.max_side_stretch, 0);
         panel_image._same_x=true; REPAD(x, panel_image._tex_x[0])if(!Equal(panel_image._tex_x[0][x], panel_image._tex_x[1][x]) || !Equal(panel_image._tex_x[1][x], panel_image._tex_x[2][x]))panel_image._same_x=false;
         panel_image._tex_left_top=0;
         panel_image._tex_right_bottom=1;

         Flt  mulx  =params.width * max_scale,
              muly  =params.height* max_scale;
         Vec2 extend=params.extend*(max_scale*0.5f);
         panel_image._size_y[0]=   panel_image._tex_y[0] *muly-extend.y;
         panel_image._size_y[1]=(1-panel_image._tex_y[1])*muly-extend.y;

         REPAD(y, panel_image._size_x)
         {
            panel_image._size_x[y][0]=   panel_image._tex_x[y][0] *mulx-extend.x;
            panel_image._size_x[y][1]=(1-panel_image._tex_x[y][1])*mulx-extend.x;
         }

         if(include_size_mid[0])panel_image._side_size.x=mulx;else{panel_image._side_size.x=0; REPAD(y, panel_image._size_x)MAX(panel_image._side_size.x, Max(panel_image._size_x[y][0], 0)+Max(panel_image._size_x[y][1], 0));}
         if(include_size_mid[1])panel_image._side_size.y=muly;else panel_image._side_size.y=Max(panel_image._size_y[0], 0)+Max(panel_image._size_y[1], 0);

         REPAO(sps).setDepthNoise(params.sections[i], image.w(), image.h(), super_sample, threads);

         return true;
      }
      panel_image.del(); return false;
   }
   void setDepthY(Int y)
   {
      REPD(x, map.w())
      {
         Vec2 pos(x, y);
         Flt  dist_x=FLT_MAX, dist_y=FLT_MAX, dist_global=FLT_MAX, dist_sect=FLT_MAX, dist_depth=0.5f, dist_depth_sect=0.5f, d;
         d=               pos.x /resolution; MIN(dist_global, d); MIN(dist_sect, d); MIN(dist_x, d); if(!params.cut_left  ){MIN(dist_depth, d); MIN(dist_depth_sect, d);}
         d=               pos.y /resolution; MIN(dist_global, d);                    MIN(dist_y, d); if(!params.cut_top   ){MIN(dist_depth, d);                         }
         d=(image_size1.x-pos.x)/resolution; MIN(dist_global, d); MIN(dist_sect, d); MIN(dist_x, d); if(!params.cut_right ){MIN(dist_depth, d); MIN(dist_depth_sect, d);}
         d=(image_size1.y-pos.y)/resolution; MIN(dist_global, d); MIN(dist_sect, d); MIN(dist_y, d); if(!params.cut_bottom){MIN(dist_depth, d); MIN(dist_depth_sect, d);}
         d=(   top_offset+pos.y)/resolution;                      MIN(dist_sect, d);                 if(!params.cut_top   ){                    MIN(dist_depth_sect, d);}

         REPD(y, 2)
         REPD(x, 2)
         {
          C Line &line=corner_line[y][x]; if(line.on)
            {
               d=DistPointPlane(pos, line.pos, line.normal)/resolution;
               MIN(dist_global, d); MIN(dist_sect, d); MIN(dist_depth, d); MIN(dist_depth_sect, d);
            }
         }
         REPD(x, 2)
         {
          C Line &line=side_line[x]; if(line.on)
            {
               d=DistPointPlane(pos, line.pos, line.normal)/resolution;
               MIN(dist_global, d); MIN(dist_sect, d); MIN(dist_depth, d); MIN(dist_depth_sect, d);
            }
         }

         if(corner_size.max()>0.5f)
         {
            Vec2 corner;
            if(!params.cut_top && !params.cut_left) // top left
            {
               corner.set(corner_size.x, corner_size.y);
               Bool cx=(pos.x<corner.x), cy=(pos.y<corner.y);
               if(cx && cy){d=(-Dist(pos, corner)+corner_size.x)/resolution; MIN(dist_global, d); MIN(dist_depth, d);}
               if(cx      ){Flt y=(1-CosSin(1-pos.x/corner_size.x))*corner_size.y; d=(pos.y-y)/resolution; MIN(dist_y, d);}
               if(      cy){Flt x=(1-CosSin(1-pos.y/corner_size.y))*corner_size.x; d=(pos.x-x)/resolution; MIN(dist_x, d);}

               if(cx){corner.y-=top_offset; cy=(pos.y<corner.y); if(cy){d=(-Dist(pos, corner)+corner_size.x)/resolution; MIN(dist_sect, d); MIN(dist_depth_sect, d);}}
            }
            if(!params.cut_top && !params.cut_right) // top right
            {
               corner.set(image_size1.x-corner_size.x, corner_size.y);
               Bool cx=(pos.x>corner.x), cy=(pos.y<corner.y);
               if(cx && cy){d=(-Dist(pos, corner)+corner_size.x)/resolution; MIN(dist_global, d); MIN(dist_depth, d);}
               if(cx      ){Flt y=      (1-CosSin((corner.x-pos.x)/corner_size.x))*corner_size.y; d=(pos.y-y)/resolution; MIN(dist_y, d);}
               if(      cy){Flt x=corner.x+CosSin(        1-pos.y /corner_size.y) *corner_size.x; d=(x-pos.x)/resolution; MIN(dist_x, d);}

               if(cx){corner.y-=top_offset; cy=(pos.y<corner.y); if(cy){d=(-Dist(pos, corner)+corner_size.x)/resolution; MIN(dist_sect, d); MIN(dist_depth_sect, d);}}
            }
            if(!params.cut_bottom && !params.cut_left) // bottom left
            {
               corner.set(corner_size.x, image_size1.y-corner_size.y);
               Bool cx=(pos.x<corner.x), cy=(pos.y>corner.y);
               if(cx && cy){d=(-Dist(pos, corner)+corner_size.x)/resolution; MIN(dist_global, d); MIN(dist_sect, d); MIN(dist_depth, d); MIN(dist_depth_sect, d);}
               if(cx      ){Flt y=corner.y+CosSin(        1-pos.x /corner_size.x) *corner_size.y; d=(y-pos.y)/resolution; MIN(dist_y, d);}
               if(      cy){Flt x=      (1-CosSin((corner.y-pos.y)/corner_size.y))*corner_size.x; d=(pos.x-x)/resolution; MIN(dist_x, d);}
            }
            if(!params.cut_bottom && !params.cut_right) // bottom right
            {
               corner.set(image_size1.x-corner_size.x, image_size1.y-corner_size.y);
               Bool cx=(pos.x>corner.x), cy=(pos.y>corner.y);
               if(cx && cy){d=(-Dist(pos, corner)+corner_size.x)/resolution; MIN(dist_global, d); MIN(dist_sect, d); MIN(dist_depth, d); MIN(dist_depth_sect, d);}
               if(cx      ){Flt y=corner.y+CosSin((corner.x-pos.x)/corner_size.x)*corner_size.y; d=(y-pos.y)/resolution; MIN(dist_y, d);}
               if(      cy){Flt x=corner.x+CosSin((corner.y-pos.y)/corner_size.y)*corner_size.x; d=(x-pos.x)/resolution; MIN(dist_x, d);}
            }
         }

         // 'dist_global' is now -Inf .. 0.5
         Flt depth;
         if(dist_global>=0) // 'dist_depth' is now 0 .. 0.5
         {
            dist_x         *=2 ; // 'dist_x'          is now 0 .. width
            dist_y         *=2 ; // 'dist_y'          is now 0 .. height
            dist_global    *=2 ; // 'dist_global'     is now 0 .. 
            dist_sect      *=2 ; // 'dist_sect'       is now 0 .. 
        SAT(dist_depth     *=2); // 'dist_depth'      is now 0 .. 1.0
        SAT(dist_depth_sect*=2); // 'dist_depth_sect' is now 0 .. 1.0

            Int                        section_i=(dist_sect<sps[1].size);
          C SectionParams             &sp       =       sps     [section_i];
          C PanelImageParams::Section &section  =params.sections[section_i];

            Flt global_depth, frac=(params.inner_distance ? Sat(dist_depth/params.inner_distance) : 1);
            global_depth=Lerp(frac, CosSin(1-frac), params.round_depth); // first apply round, because it needs depth in 0..1 range
            global_depth*=params.depth;

            Flt dist=(section_i ? dist_depth : dist_depth_sect-sps[1].size), // we need to use 'dist_depth' for section #1 because of 'top_size'
                dist_frac=(sp.size ? dist/sp.size : 1), section_depth;
            if(dist_frac<section.inner_distance)
            {
               frac=dist_frac/section.inner_distance;
               frac=Lerp(frac, CosSin(1-frac), section.round_depth); // first apply round, because it needs depth in 0..1 range
               section_depth=(section_i ? Lerp(0.0f, section.outer_depth, frac) : Lerp(section.outer_depth, section.inner_depth, frac));
            }else
            {
               frac=LerpRS(section.inner_distance, 1.0f, dist_frac);
               frac=Lerp(frac, 1-CosSin(frac), section.round_depth); // first apply round, because it needs depth in 0..1 range
               section_depth=(section_i ? Lerp(section.outer_depth, section.inner_depth, frac) : section.inner_depth);
            }

            depth=global_depth*section_depth;

            // apply smooth depth
            REP(2) // section and global
            {
             C SmoothDepth &sd      =(i ? sp.smooth_depth : smooth_depth );
             C Vec2        &norm_mul=(i ? sp.norm_mul     : full_norm_mul),
                           &norm_add=(i ? sp.norm_add     : full_norm_add);
               REPA(sd.is)if(sd.is[i]) // axis X and Y
               {
                  Flt center=pos.c[i]*norm_mul.c[i]+norm_add.c[i];
                  if(i==0)
                  {
                     if(pos.y<corner_size.y && !params.cut_top)
                     {
                        if((x>image_size_2i.x) ? !params.cut_right : !params.cut_left)
                        {
                           Flt x=(1-CosSin(1-pos.y/corner_size.y))*corner_size.x; x=image_size_2.x/(image_size_2.x-x);
                           center*=Lerp(1.0f, x, sd.sqr.y); // make this adjustment smaller if the other axis is disabled
                        }
                     }else
                     if(pos.y>image_size1.y-corner_size.y && !params.cut_bottom)
                     {
                        if((x>image_size_2i.x) ? !params.cut_right : !params.cut_left)
                        {
                           Flt x=(1-CosSin(1-(image_size1.y-pos.y)/corner_size.y))*corner_size.x; x=image_size_2.x/(image_size_2.x-x);
                           center*=Lerp(1.0f, x, sd.sqr.y); // make this adjustment smaller if the other axis is disabled
                        }
                     }
                  }else
                  {
                     if(pos.x<corner_size.x && !params.cut_left)
                     {
                        if((y>image_size_2i.y) ? !params.cut_bottom : !params.cut_top)
                        {
                           Flt y=(1-CosSin(1-pos.x/corner_size.x))*corner_size.y; y=image_size_2.y/(image_size_2.y-y);
                           center*=Lerp(1.0f, y, sd.sqr.x); // make this adjustment smaller if the other axis is disabled
                        }
                     }else
                     if(pos.x>image_size1.x-corner_size.x && !params.cut_right)
                     {
                        if((y>image_size_2i.y) ? !params.cut_bottom : !params.cut_top)
                        {
                           Flt y=(1-CosSin(1-(image_size1.x-pos.x)/corner_size.x))*corner_size.y; y=image_size_2.y/(image_size_2.y-y);
                           center*=Lerp(1.0f, y, sd.sqr.x); // make this adjustment smaller if the other axis is disabled
                        }
                     }
                  }
                  depth*=CosSin(center*sd.val.c[i]);
               }
            }

            // apply depth overlay
            if(sp.overlay)
            {
               Vec4 ovr=sp.overlay->colorFCubicFast(x*sp.overlay_uv_scale+sp.overlay_uv_offset.x,
                                                    y*sp.overlay_uv_scale+sp.overlay_uv_offset.y, false);
               ovr.xyz=ovr.xyz.max();
               ovr.w *=sp.overlay_intensity;
               switch(section.depth_overlay_params.mode)
               {
                  case PanelImageParams::ImageParams::MULTIPLY  : depth*=Lerp       (1.0f, ovr.x, ovr.w); break;
                  case PanelImageParams::ImageParams::SCALE     : depth*=ScaleFactor((ovr.x-0.5f)*ovr.w); break;
                  case PanelImageParams::ImageParams::ADD       : depth+=             ovr.x      *ovr.w ; break;
                  case PanelImageParams::ImageParams::ADD_SIGNED: depth+=            (ovr.x-0.5f)*ovr.w ; break;
                  case PanelImageParams::ImageParams::BLEND     : depth =Blend(Vec4(depth, depth, depth, 1), ovr).x; break;
               }
            }

            // apply depth noise
            if(sp.noise_map.is())
            {
               Flt noise=sp.noise_map.pixelFCubicFast(x*sp.noise_uv_scale, y*sp.noise_uv_scale, false);
               switch(section.depth_noise.mode)
               {
                  case PanelImageParams::ImageParams::MULTIPLY  : depth*=Lerp       (1.0f, noise, sp.noise_intensity); break;
                  case PanelImageParams::ImageParams::SCALE     : depth*=ScaleFactor((noise-0.5f)*sp.noise_intensity); break;
                  case PanelImageParams::ImageParams::ADD       : depth+=             noise      *sp.noise_intensity ; break;
                  case PanelImageParams::ImageParams::ADD_SIGNED: depth+=            (noise-0.5f)*sp.noise_intensity ; break;
                  case PanelImageParams::ImageParams::BLEND     : depth =Blend(Vec4(depth, depth, depth, 1), Vec4(noise, noise, noise, sp.noise_intensity)).x; break;
               }
            }
         }else depth=0;
      #if DEBUG
         if(Kb.b(KB_V))depth=dist_depth_sect;
      #endif
              map.pixF (x, y)=depth;
         dist_map.pixF4(x, y).set(dist_x, dist_y, dist_global, dist_sect);
      }
   }
   void afterDepth()
   {
      if(depth_map)Swap(*depth_map, map);
      (depth_map ? *depth_map : map).bumpToNormal(map, resolution, true);

      REPA(sps)
      {
         sps[i].setColorOverlay(params.sections[i], resolution, threads);
         sps[i].setColorNoise  (params.sections[i], image_size.x, image_size.y, super_sample, threads);
      }
   }
   void setColorY(Int y)
   {
      Flt   frac_y=y*full_frac_mul.y/*+full_frac_add.y*/; // 0..1, 'full_frac_add' is always zero
      Vec4 color_y=Lerp(color_top, color_bottom, frac_y);

      Flt  image_frac_y[3][3];
      Vec4 section_color_y[ELMS(params.sections)];
      REPD(sy, 3)
      REPD(sx, 3)image_frac_y[sx][sy]=y*image_frac_mul_add[sx][sy].z+image_frac_mul_add[sx][sy].w;
      REPA(sps)
      {
         Flt section_frac_y=y*sps[i].frac_mul.y+sps[i].frac_add.y;
         sps[i].setColorY(section_frac_y, color_y, section_color_y[i]);
      }
      REPD(x, image_size.x)
      {
         Vec4  col   =0;
       C Vec4 &dist4 =dist_map.pixF4(x, y);
         Flt   dist_global =dist4.z;
         if(   dist_global>=0)
         {
            Flt   dist_x   =dist4.x, dist_y=dist4.y, dist_sect=dist4.w;
            Vec4  nrm_d    =map.pixF4(x, y);
            Flt   depth    =nrm_d.w;
          C Vec  &nrm      =nrm_d.xyz;
            Int   section_i=(dist_sect<sps[1].size);
          C SectionParams             &sp     =       sps     [section_i];
          C PanelImageParams::Section &section=params.sections[section_i];
            if(!section_i)dist_sect-=sps[1].size;
            Flt dist=(section_i ? dist_global : dist_sect), dist_frac=(sp.size ? Sat(dist/sp.size) : 1); // we need to use 'dist_global' for section #1 because of 'top_size', we need to use 'Sat' because of 'top_size'

            Flt         frac_x=x*full_frac_mul.x/*+full_frac_add.x*/, // 0..1, 'full_frac_add' is always zero
                section_frac_x=x*sp.frac_mul.x+sp.frac_add.x;
            Vec4       color_x=Lerp(color_left, color_right, frac_x);
            col=section_color_y[section_i]*color_x*Lerp(sp.color_left, sp.color_right, section_frac_x)*Lerp(sp.outer_color, sp.inner_color, dist_frac);

            // apply color overlay
            if(sp.overlay)
            {
               Vec2 tex(x, y); tex*=sp.overlay_uv_scale; tex+=sp.overlay_uv_offset;
               if(sp.overlay_uv_warp){Vec2 n=nrm.xy; n.setLength(1-CosSin(n.length())); tex+=sp.overlay_uv_warp*n;}
               Vec4 ovr=sp.overlay->colorFCubicFast(tex.x, tex.y, false);
               switch(section.color_overlay_params.mode)
               {
                  case PanelImageParams::ImageParams::MULTIPLY  : col    *=Lerp       (Vec4(1)       , ovr , sp.overlay_intensity ); break;
                  case PanelImageParams::ImageParams::SCALE     : col.xyz*=ScaleFactor((ovr.xyz-0.5f)*(ovr.w*sp.overlay_intensity)); break;
                  case PanelImageParams::ImageParams::ADD       : col.xyz+=             ovr.xyz      *(ovr.w*sp.overlay_intensity) ; break;
                  case PanelImageParams::ImageParams::ADD_SIGNED: col.xyz+=            (ovr.xyz-0.5f)*(ovr.w*sp.overlay_intensity) ; break;
                  case PanelImageParams::ImageParams::BLEND     : ovr.w  *=sp.overlay_intensity; col=Blend(col, ovr); break;
               }
            }

            // apply color noise
            if(sp.noise_map.is())
            {
               Vec2 tex(x, y); tex*=sp.noise_uv_scale;
               if(sp.noise_uv_warp){Vec2 n=nrm.xy; n.setLength(1-CosSin(n.length())); tex+=sp.noise_uv_warp*n;}
               Flt noise=sp.noise_map.pixelFCubicFast(tex.x, tex.y, false);
               switch(section.color_noise.mode)
               {
                  case PanelImageParams::ImageParams::MULTIPLY  : col.xyz*=Lerp       (1.0f, noise, sp.noise_intensity); break;
                  case PanelImageParams::ImageParams::SCALE     : col.xyz*=ScaleFactor((noise-0.5f)*sp.noise_intensity); break;
                  case PanelImageParams::ImageParams::ADD       : col.xyz+=             noise      *sp.noise_intensity ; break;
                  case PanelImageParams::ImageParams::ADD_SIGNED: col.xyz+=            (noise-0.5f)*sp.noise_intensity ; break;
                  case PanelImageParams::ImageParams::BLEND     : col.xyz =Blend(col, Vec4(noise, noise, noise, sp.noise_intensity)).xyz; break;
               }
            }

            // apply 3x3 images
            if(y<y1)
            {
               if(x<top_x1){if( top_left_image)col*= top_left_image->colorFCubicFast(x*image_frac_mul_add[0][0].x+image_frac_mul_add[0][0].y, image_frac_y[0][0], true, true);}else
               if(x<top_x2){if(      top_image)col*=      top_image->colorFCubicFast(x*image_frac_mul_add[1][0].x+image_frac_mul_add[1][0].y, image_frac_y[1][0], true, true);}else
                           {if(top_right_image)col*=top_right_image->colorFCubicFast(x*image_frac_mul_add[2][0].x+image_frac_mul_add[2][0].y, image_frac_y[2][0], true, true);}
            }else
            if(y<y2)
            {
               if(x<x1){if(  left_image)col*=  left_image->colorFCubicFast(x*image_frac_mul_add[0][1].x+image_frac_mul_add[0][1].y, image_frac_y[0][1], true, true);}else
               if(x<x2){if(center_image)col*=center_image->colorFCubicFast(x*image_frac_mul_add[1][1].x+image_frac_mul_add[1][1].y, image_frac_y[1][1], true, true);}else
                       {if( right_image)col*= right_image->colorFCubicFast(x*image_frac_mul_add[2][1].x+image_frac_mul_add[2][1].y, image_frac_y[2][1], true, true);}
            }else
            {
               if(x<bottom_x1){if( bottom_left_image)col*= bottom_left_image->colorFCubicFast(x*image_frac_mul_add[0][2].x+image_frac_mul_add[0][2].y, image_frac_y[0][2], true, true);}else
               if(x<bottom_x2){if(      bottom_image)col*=      bottom_image->colorFCubicFast(x*image_frac_mul_add[1][2].x+image_frac_mul_add[1][2].y, image_frac_y[1][2], true, true);}else
                              {if(bottom_right_image)col*=bottom_right_image->colorFCubicFast(x*image_frac_mul_add[2][2].x+image_frac_mul_add[2][2].y, image_frac_y[2][2], true, true);}
            }

            // calculate light and specular
            Flt light=params.light_ambient, specular=0;
            REPA(params.lights)if(params.lights[i].enabled)
            {
               Flt dot=Dot(light_dir[i], nrm);
               if( dot>0)light+=params.lights[i].intensity*dot;
               else      light-=params.lights[i].back     *dot;

               light+=(dot>params.lights[i].highlight_cut)*params.lights[i].highlight;

               Flt spec_base=LightSpecularBase(nrm, light_dir    [i], VecDir[DIR_FORWARD]);
               specular+=    LightSpecular    (nrm, light_dir_neg[i], VecDir[DIR_FORWARD], params.lights[i].specular_exp          )*params.lights[i].specular_back     ;
               specular+=                                                   Pow(spec_base, params.lights[i].specular_exp          )*params.lights[i].specular          ;
               specular+=                                                      (spec_base> params.lights[i].specular_highlight_cut)*params.lights[i].specular_highlight;
            }
            specular*=section.specular;

            // apply light
            MAX(light, 0);
            col.xyz*=light;
            
            // apply reflection
            if(sp.reflection)
            {
               #define FACE_LEFT  0
               #define FACE_FRONT 1
               #define FACE_RIGHT 2
               #define FACE_BACK  3
               #define FACE_DOWN  4
               #define FACE_UP    5
               Vec dir=Reflect(VecDir[DIR_BACK], nrm);
               CHS(dir.x); CHS(dir.z); // make FACE_FRONT to be visible by default
            #if DEBUG
               if(Kb.b(KB_R))dir*=Matrix().setRotateY(Time.appTime());
            #endif
               dir/=Abs(dir).max();
               Int face=0;
               switch(Abs(dir).maxI())
               {
                  case 0: face=((dir.x<0) ? FACE_LEFT : FACE_RIGHT); break;
                  case 1: face=((dir.y<0) ? FACE_DOWN : FACE_UP   ); break;
                  case 2: face=((dir.z<0) ? FACE_BACK : FACE_FRONT); break;
               }
               Vec2 uv;
               switch(face)
               {
                  case FACE_LEFT : uv.set( dir.z, -dir.y); break;
                  case FACE_FRONT: uv.set( dir.x, -dir.y); break;
                  case FACE_RIGHT: uv.set(-dir.z, -dir.y); break;
                  case FACE_BACK : uv.set(-dir.x, -dir.y); break;
                  case FACE_DOWN : uv.set( dir.x, -dir.z); break;
                  case FACE_UP   : uv.set( dir.x,  dir.z); break;
               }
               uv=(uv*0.5f+0.5f)*sp.reflection->h();
               if(!sp.reflection_one)uv.x+=sp.reflection->h()*face;
               Vec tex=sp.reflection->colorFCubicFast(uv.x, uv.y).xyz;
               col=AdditiveBlend(col, Vec4(tex, section.reflection_intensity));
            }

            // apply inner glow
            if(params.inner_glow_color.a && params.inner_glow_radius>0 && dist_global>=0 && dist_global<params.inner_glow_radius) // glow
            {
               Flt d=dist_global, x=params.inner_glow_radius-dist_x, y=params.inner_glow_radius-dist_y;
               // this increases glow in square corners
               if(x>=0 && y>=0) // only in corners (this may process circular corners too)
                  if(Equal(dist_x, dist_global)
                  || Equal(dist_y, dist_global)) // allow only square corners
                     MIN(d, Max(0, params.inner_glow_radius-Dist(x, y)));
               Flt  g=1-d/params.inner_glow_radius;
               Vec4 glow=inner_glow_color; glow.w*=g; col=Blend(col, glow);
            }

            // apply specular, do this after inner glow, so that can be used for inner shadowing
            col=Blend(col, Vec4(1, 1, 1, Sat(specular)));

            // apply outer glow and borders
            if(section.outer_border_color.a && dist>=0                  &&    dist<params.border_size  )col=Blend(col, sp.outer_border_color);else
            if(section.inner_border_color.a && dist>=params.border_size &&    dist<params.border_size*2)col=Blend(col, sp.inner_border_color);else
            if(section. prev_border_color.a && section_i && dist_sect>=sps[1].size-params.border_size  )col=Blend(col, sp. prev_border_color);

         #if DEBUG
            if(Kb.b(KB_X))col=ColorBrightness(dist_x     ).asVec4();
            if(Kb.b(KB_Y))col=ColorBrightness(dist_y     ).asVec4();
            if(Kb.b(KB_I))col=ColorBrightness(dist_sect  ).asVec4();
            if(Kb.b(KB_D))col=ColorBrightness(dist_global).asVec4();
            if(Kb.b(KB_H))col=ColorBrightness(depth      ).asVec4();
         #endif
         }
         image.color(x, y, col);
      }
   }
   void afterColor(FILTER_TYPE filter)
   {
      image.transparentToNeighbor();
   #if DEBUG
      if(Kb.b(KB_N)){map.mulAdd(Vec4(0.5f, 0.5f, 0.5f, 1), Vec4(0.5f, 0.5f, 0.5f, 0)); map.copy(image, -1, -1, -1, IMAGE_R8G8B8);}
   #endif

      image.resize(image_size.x/super_sample, image_size.y/super_sample, filter, true, true); // 'alpha_weight' needed when having 2 sections with different opacities   }

      panel_image._padd.zero();
      VecI2 old_size=image.size();
      Image glow_image, shadow_image;
      Int   glow_radius=RoundPos(params.outer_glow_radius*0.5f*params.resolution);
      if(   glow_radius>0 && params.outer_glow_color.a)
      {
         MIN(glow_radius, params.resolution/2);
         glow_image.createShadow(image, glow_radius, 1.0f, params.outer_glow_spread, glow_radius>0); // apply border padd only with a positive radius, to avoid making textures bigger with radius = 0
      }
      if(params.shadow_opacity>EPS) // create the shadow even if radius is zero, because the panel may be transparent
      {
         Int shadow_radius=RoundPos(params.shadow_radius*0.5f*params.resolution);
         MIN(shadow_radius, params.resolution/2);
         shadow_image.createShadow(image, shadow_radius, params.shadow_opacity, params.shadow_spread, shadow_radius>0); // apply border padd only with a positive radius, to avoid making textures bigger with radius = 0
      }
      if(  glow_image.is())image.applyShadow(  glow_image, params.outer_glow_color, (image.size()-  glow_image.size())/2); // yes, glow needs to be applied first (before shadows), because 'applyShadow' works in a way that it doesn't apply pixels on opaque ones
      if(shadow_image.is())image.applyShadow(shadow_image, BLACK                  , (image.size()-shadow_image.size())/2);
       image_size =image.size();
      if(old_size!=image_size)
      {
         Vec2 scale=Vec2(old_size)/image_size;

         REPAD(y, panel_image._tex_x)REPAD(x, panel_image._tex_x[y])ScaleTex(panel_image._tex_x[y][x]     , scale.x);
         REPAD(y, panel_image._tex_y)                               ScaleTex(panel_image._tex_y[y]        , scale.y);
                                                                    ScaleTex(panel_image._tex_left_top    , scale  );
                                                                    ScaleTex(panel_image._tex_right_bottom, scale  );
         panel_image._padd=(1/scale-1)*Vec2(params.width, params.height);
      }
      panel_image._padd+=params.extend;
      REPA(panel_image._force_uniform_stretch)
      {
         panel_image._padd.c[i]*=0.5f;
         panel_image._force_uniform_stretch[i]=params.force_uniform_stretch[i];
         if(!params.force_uniform_stretch[i])panel_image._padd.c[i]*=max_scale;
      }
      panel_image._padd_any=panel_image._padd.any();

      panel_image._inner_padding.set(panel_image._size_x[1][0], panel_image._size_y[1],
                                     panel_image._size_x[1][1], panel_image._size_y[0]);
      panel_image._inner_padding.min+=params.extend_inner_padd.min;
      panel_image._inner_padding.max+=params.extend_inner_padd.max;
   }
};
static void SetDepth(IntPtr y, PanelImageCreate &pic, Int thread_index) {pic.setDepthY(y);}
static void SetColor(IntPtr y, PanelImageCreate &pic, Int thread_index) {pic.setColorY(y);}
void PanelImage::create(C PanelImageParams &params, Image *depth_map, Int super_sample, FILTER_TYPE filter, Threads *threads)
{
   PanelImageCreate pic(T, params, depth_map, super_sample, threads);
   if(pic.create())
   {
      if(threads)threads->process1(pic.map.h(), SetDepth, pic);else REPD(y, pic.map  .h())pic.setDepthY(y);
      pic.afterDepth();
      if(threads)threads->process1(pic.map.h(), SetColor, pic);else REPD(y, pic.image.h())pic.setColorY(y);
      pic.afterColor(filter);
   }
}
/******************************************************************************/
Bool PanelImage::getSideScale(C Rect &rect, Flt &scale)C
{
   Flt w=rect.w(), h=rect.h();
   if(_side_size.x>w){scale=w/_side_size.x; Flt side_size_y=_side_size.y*scale; if(side_size_y>h)scale*=h/side_size_y; return true;}
   if(_side_size.y>h){scale=h/_side_size.y;                                                                            return true;}
   return false;
}
Bool PanelImage::getSideScaleVertical(C Rect &rect, Flt &scale)C
{
   Flt w=rect.h(), h=rect.w();
   if(_side_size.x>w){scale=w/_side_size.x; Flt side_size_y=_side_size.y*scale; if(side_size_y>h)scale*=h/side_size_y; return true;}
   if(_side_size.y>h){scale=h/_side_size.y;                                                                            return true;}
   return false;
}
Bool PanelImage::extendedRect(C Rect &rect, Rect &extended)C
{
   extended=rect;
   if(_padd_any)
   {
      // !! change 'extended' after using 'rect' in case they're the same 'Rect' !!
      Flt scale; if(getSideScale(rect, scale))
      {
         extended.extendX(_force_uniform_stretch[0] ? _padd.x*rect.w() : _padd.x*scale);
         extended.extendY(_force_uniform_stretch[1] ? _padd.y*rect.h() : _padd.y*scale);
      }else
      {
         extended.extendX(_force_uniform_stretch[0] ? _padd.x*rect.w() : _padd.x);
         extended.extendY(_force_uniform_stretch[1] ? _padd.y*rect.h() : _padd.y);
      }
      return true;
   }
   return false;
}
void PanelImage::innerPadding(C Rect &rect, Rect &padding)C
{
   Flt scale; Bool scale_do=getSideScale(rect, scale);
   padding=defaultInnerPadding(); // change 'padding' after using 'rect' in case they're the same 'Rect'
   if(scale_do)padding*=scale;
}
void PanelImage::defaultInnerPaddingSize(Vec2 &padd_size)C
{
   padd_size=_inner_padding.min+_inner_padding.max;
}
/******************************************************************************/
void PanelImage::draw(                                    C Rect &rect)C {draw(WHITE, TRANSPARENT, rect);}
void PanelImage::draw(C Color &color, C Color &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (&image   );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   const Int vtxs=(_same_x ? 16 : 20);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(vtxs))
   {
      Flt scale; Bool scale_do=getSideScale(rect, scale);
      Flt l =rect.min.x, r =rect.max.x, x[3][2], // [y][x]
          y0=rect.max.y, y3=rect.min.y, y1, y2;
      if(scale_do)
      {
         y1     =y0-_size_y[0]   *scale;
         y2     =y3+_size_y[1]   *scale;
         x[0][0]=l +_size_x[0][0]*scale;
         x[0][1]=r -_size_x[0][1]*scale;

         if(_padd_any)
         {
            Flt padd;
            padd=_padd.x*(_force_uniform_stretch[0] ? rect.w() : scale); l -=padd; r +=padd;
            padd=_padd.y*(_force_uniform_stretch[1] ? rect.h() : scale); y0+=padd; y3-=padd;
         }
      }else
      {
         y1     =y0-_size_y[0];
         y2     =y3+_size_y[1];
         x[0][0]=l +_size_x[0][0];
         x[0][1]=r -_size_x[0][1];

         if(_padd_any)
         {
            if(_force_uniform_stretch[0]){Flt padd=_padd.x*rect.w(); l -=padd; r +=padd;}else{l -=_padd.x; r +=_padd.x;}
            if(_force_uniform_stretch[1]){Flt padd=_padd.y*rect.h(); y0+=padd; y3-=padd;}else{y0+=_padd.y; y3-=_padd.y;}
         }
      }

      if(_same_x)
      {
         v[ 0].pos.set(      l, y0);
         v[ 1].pos.set(x[0][0], y0);
         v[ 2].pos.set(x[0][1], y0);
         v[ 3].pos.set(      r, y0);
         v[ 4].pos.set(      l, y1);
         v[ 5].pos.set(x[0][0], y1);
         v[ 6].pos.set(x[0][1], y1);
         v[ 7].pos.set(      r, y1);
         v[ 8].pos.set(      l, y2);
         v[ 9].pos.set(x[0][0], y2);
         v[10].pos.set(x[0][1], y2);
         v[11].pos.set(      r, y2);
         v[12].pos.set(      l, y3);
         v[13].pos.set(x[0][0], y3);
         v[14].pos.set(x[0][1], y3);
         v[15].pos.set(      r, y3);

         v[ 0].tex.set(           0,         0);
         v[ 1].tex.set(_tex_x[0][0],         0);
         v[ 2].tex.set(_tex_x[0][1],         0);
         v[ 3].tex.set(           1,         0);

         v[ 4].tex.set(           0, _tex_y[0]);
         v[ 5].tex.set(_tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(_tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(           1, _tex_y[0]);

         v[ 8].tex.set(           0, _tex_y[1]);
         v[ 9].tex.set(_tex_x[0][0], _tex_y[1]);
         v[10].tex.set(_tex_x[0][1], _tex_y[1]);
         v[11].tex.set(           1, _tex_y[1]);

         v[12].tex.set(           0,         1);
         v[13].tex.set(_tex_x[0][0],         1);
         v[14].tex.set(_tex_x[0][1],         1);
         v[15].tex.set(           1,         1);
      }else
      {
         if(scale_do)
         {
            x[1][0]=rect.min.x+_size_x[1][0]*scale;
            x[1][1]=rect.max.x-_size_x[1][1]*scale;

            x[2][0]=rect.min.x+_size_x[2][0]*scale;
            x[2][1]=rect.max.x-_size_x[2][1]*scale;
         }else
         {
            x[1][0]=rect.min.x+_size_x[1][0];
            x[1][1]=rect.max.x-_size_x[1][1];

            x[2][0]=rect.min.x+_size_x[2][0];
            x[2][1]=rect.max.x-_size_x[2][1];
         }

         v[ 0].pos.set(      l, y0);
         v[ 1].pos.set(x[0][0], y0);
         v[ 2].pos.set(x[0][1], y0);
         v[ 3].pos.set(      r, y0);

         v[ 4].pos.set(      l, y1);
         v[ 5].pos.set(x[0][0], y1);
         v[ 6].pos.set(x[0][1], y1);
         v[ 7].pos.set(      r, y1);
         v[ 8].pos.set(x[1][0], y1);
         v[ 9].pos.set(x[1][1], y1);

         v[10].pos.set(      l, y2);
         v[11].pos.set(x[1][0], y2);
         v[12].pos.set(x[1][1], y2);
         v[13].pos.set(      r, y2);
         v[14].pos.set(x[2][0], y2);
         v[15].pos.set(x[2][1], y2);

         v[16].pos.set(      l, y3);
         v[17].pos.set(x[2][0], y3);
         v[18].pos.set(x[2][1], y3);
         v[19].pos.set(      r, y3);

         v[ 0].tex.set(           0, 0);
         v[ 1].tex.set(_tex_x[0][0], 0);
         v[ 2].tex.set(_tex_x[0][1], 0);
         v[ 3].tex.set(           1, 0);

         v[ 4].tex.set(           0, _tex_y[0]);
         v[ 5].tex.set(_tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(_tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(           1, _tex_y[0]);
         v[ 8].tex.set(_tex_x[1][0], _tex_y[0]);
         v[ 9].tex.set(_tex_x[1][1], _tex_y[0]);

         v[10].tex.set(           0, _tex_y[1]);
         v[11].tex.set(_tex_x[1][0], _tex_y[1]);
         v[12].tex.set(_tex_x[1][1], _tex_y[1]);
         v[13].tex.set(           1, _tex_y[1]);
         v[14].tex.set(_tex_x[2][0], _tex_y[1]);
         v[15].tex.set(_tex_x[2][1], _tex_y[1]);

         v[16].tex.set(           0, 1);
         v[17].tex.set(_tex_x[2][0], 1);
         v[18].tex.set(_tex_x[2][1], 1);
         v[19].tex.set(           1, 1);
      }

      if(image.partial())REP(vtxs)v[i].tex*=image._part.xy;

      VI.flushIndexed(_same_x ? IndBufPanel : IndBufPanelEx, 3*3*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void PanelImage::drawVertical(                                    C Rect &rect)C {drawVertical(WHITE, TRANSPARENT, rect);}
void PanelImage::drawVertical(C Color &color, C Color &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (&image   );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   const Int vtxs=(_same_x ? 16 : 20);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(vtxs))
   {
      Flt scale; Bool scale_do=getSideScaleVertical(rect, scale);
      Flt b =rect.min.y, t =rect.max.y, y[3][2], // [y][x]
          x0=rect.min.x, x3=rect.max.x, x1, x2;
      if(scale_do)
      {
         x1     =x0+_size_y[0]   *scale;
         x2     =x3-_size_y[1]   *scale;
         y[0][0]=b +_size_x[0][0]*scale;
         y[0][1]=t -_size_x[0][1]*scale;

         if(_padd_any)
         {
            Flt padd;
            padd=_padd.x*(_force_uniform_stretch[0] ? rect.h() : scale); b -=padd; t +=padd;
            padd=_padd.y*(_force_uniform_stretch[1] ? rect.w() : scale); x0-=padd; x3+=padd;
         }
      }else
      {
         x1     =x0+_size_y[0];
         x2     =x3-_size_y[1];
         y[0][0]=b +_size_x[0][0];
         y[0][1]=t -_size_x[0][1];

         if(_padd_any)
         {
            if(_force_uniform_stretch[0]){Flt padd=_padd.x*rect.h(); b -=padd; t +=padd;}else{b -=_padd.x; t +=_padd.x;}
            if(_force_uniform_stretch[1]){Flt padd=_padd.y*rect.w(); x0-=padd; x3+=padd;}else{x0-=_padd.y; x3+=_padd.y;}
         }
      }

      if(_same_x)
      {
         v[ 0].pos.set(x0,       b);
         v[ 1].pos.set(x0, y[0][0]);
         v[ 2].pos.set(x0, y[0][1]);
         v[ 3].pos.set(x0,       t);
         v[ 4].pos.set(x1,       b);
         v[ 5].pos.set(x1, y[0][0]);
         v[ 6].pos.set(x1, y[0][1]);
         v[ 7].pos.set(x1,       t);
         v[ 8].pos.set(x2,       b);
         v[ 9].pos.set(x2, y[0][0]);
         v[10].pos.set(x2, y[0][1]);
         v[11].pos.set(x2,       t);
         v[12].pos.set(x3,       b);
         v[13].pos.set(x3, y[0][0]);
         v[14].pos.set(x3, y[0][1]);
         v[15].pos.set(x3,       t);

         v[ 0].tex.set(           0,         0);
         v[ 1].tex.set(_tex_x[0][0],         0);
         v[ 2].tex.set(_tex_x[0][1],         0);
         v[ 3].tex.set(           1,         0);

         v[ 4].tex.set(           0, _tex_y[0]);
         v[ 5].tex.set(_tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(_tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(           1, _tex_y[0]);

         v[ 8].tex.set(           0, _tex_y[1]);
         v[ 9].tex.set(_tex_x[0][0], _tex_y[1]);
         v[10].tex.set(_tex_x[0][1], _tex_y[1]);
         v[11].tex.set(           1, _tex_y[1]);

         v[12].tex.set(           0,         1);
         v[13].tex.set(_tex_x[0][0],         1);
         v[14].tex.set(_tex_x[0][1],         1);
         v[15].tex.set(           1,         1);
      }else
      {
         if(scale_do)
         {
            y[1][0]=rect.min.y+_size_x[1][0]*scale;
            y[1][1]=rect.max.y-_size_x[1][1]*scale;

            y[2][0]=rect.min.y+_size_x[2][0]*scale;
            y[2][1]=rect.max.y-_size_x[2][1]*scale;
         }else
         {
            y[1][0]=rect.min.y+_size_x[1][0];
            y[1][1]=rect.max.y-_size_x[1][1];

            y[2][0]=rect.min.y+_size_x[2][0];
            y[2][1]=rect.max.y-_size_x[2][1];
         }

         v[ 0].pos.set(x0,       b);
         v[ 1].pos.set(x0, y[0][0]);
         v[ 2].pos.set(x0, y[0][1]);
         v[ 3].pos.set(x0,       t);

         v[ 4].pos.set(x1,       b);
         v[ 5].pos.set(x1, y[0][0]);
         v[ 6].pos.set(x1, y[0][1]);
         v[ 7].pos.set(x1,       t);
         v[ 8].pos.set(x1, y[1][0]);
         v[ 9].pos.set(x1, y[1][1]);

         v[10].pos.set(x2,       b);
         v[11].pos.set(x2, y[1][0]);
         v[12].pos.set(x2, y[1][1]);
         v[13].pos.set(x2,       t);
         v[14].pos.set(x2, y[2][0]);
         v[15].pos.set(x2, y[2][1]);

         v[16].pos.set(x3,       b);
         v[17].pos.set(x3, y[2][0]);
         v[18].pos.set(x3, y[2][1]);
         v[19].pos.set(x3,       t);

         v[ 0].tex.set(           0, 0);
         v[ 1].tex.set(_tex_x[0][0], 0);
         v[ 2].tex.set(_tex_x[0][1], 0);
         v[ 3].tex.set(           1, 0);

         v[ 4].tex.set(           0, _tex_y[0]);
         v[ 5].tex.set(_tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(_tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(           1, _tex_y[0]);
         v[ 8].tex.set(_tex_x[1][0], _tex_y[0]);
         v[ 9].tex.set(_tex_x[1][1], _tex_y[0]);

         v[10].tex.set(           0, _tex_y[1]);
         v[11].tex.set(_tex_x[1][0], _tex_y[1]);
         v[12].tex.set(_tex_x[1][1], _tex_y[1]);
         v[13].tex.set(           1, _tex_y[1]);
         v[14].tex.set(_tex_x[2][0], _tex_y[1]);
         v[15].tex.set(_tex_x[2][1], _tex_y[1]);

         v[16].tex.set(           0, 1);
         v[17].tex.set(_tex_x[2][0], 1);
         v[18].tex.set(_tex_x[2][1], 1);
         v[19].tex.set(           1, 1);
      }

      if(image.partial())REP(vtxs)v[i].tex*=image._part.xy;

      VI.flushIndexed(_same_x ? IndBufPanel : IndBufPanelEx, 3*3*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void PanelImage::drawBorders(C Color &color, C Color &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (&image   );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   const Int vtxs=(_same_x ? 16 : 20);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(vtxs))
   {
      Flt scale; Bool scale_do=getSideScale(rect, scale);
      Flt l =rect.min.x, r =rect.max.x, x[3][2], // [y][x]
          y0=rect.max.y, y3=rect.min.y, y1, y2;
      if(scale_do)
      {
         y1     =y0-_size_y[0]   *scale;
         y2     =y3+_size_y[1]   *scale;
         x[0][0]=l +_size_x[0][0]*scale;
         x[0][1]=r -_size_x[0][1]*scale;

         if(_padd_any)
         {
            Flt padd;
            padd=_padd.x*(_force_uniform_stretch[0] ? rect.w() : scale); l -=padd; r +=padd;
            padd=_padd.y*(_force_uniform_stretch[1] ? rect.h() : scale); y0+=padd; y3-=padd;
         }
      }else
      {
         y1     =y0-_size_y[0];
         y2     =y3+_size_y[1];
         x[0][0]=l +_size_x[0][0];
         x[0][1]=r -_size_x[0][1];

         if(_padd_any)
         {
            if(_force_uniform_stretch[0]){Flt padd=_padd.x*rect.w(); l -=padd; r +=padd;}else{l -=_padd.x; r +=_padd.x;}
            if(_force_uniform_stretch[1]){Flt padd=_padd.y*rect.h(); y0+=padd; y3-=padd;}else{y0+=_padd.y; y3-=_padd.y;}
         }
      }

      if(_same_x)
      {
         v[ 0].pos.set(      l, y0);
         v[ 1].pos.set(x[0][0], y0);
         v[ 2].pos.set(x[0][1], y0);
         v[ 3].pos.set(      r, y0);
         v[ 4].pos.set(      l, y1);
         v[ 5].pos.set(x[0][0], y1);
         v[ 6].pos.set(x[0][1], y1);
         v[ 7].pos.set(      r, y1);
         v[ 8].pos.set(      l, y2);
         v[ 9].pos.set(x[0][0], y2);
         v[10].pos.set(x[0][1], y2);
         v[11].pos.set(      r, y2);
         v[12].pos.set(      l, y3);
         v[13].pos.set(x[0][0], y3);
         v[14].pos.set(x[0][1], y3);
         v[15].pos.set(      r, y3);

         v[ 0].tex.set(           0,         0);
         v[ 1].tex.set(_tex_x[0][0],         0);
         v[ 2].tex.set(_tex_x[0][1],         0);
         v[ 3].tex.set(           1,         0);

         v[ 4].tex.set(           0, _tex_y[0]);
         v[ 5].tex.set(_tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(_tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(           1, _tex_y[0]);

         v[ 8].tex.set(           0, _tex_y[1]);
         v[ 9].tex.set(_tex_x[0][0], _tex_y[1]);
         v[10].tex.set(_tex_x[0][1], _tex_y[1]);
         v[11].tex.set(           1, _tex_y[1]);

         v[12].tex.set(           0,         1);
         v[13].tex.set(_tex_x[0][0],         1);
         v[14].tex.set(_tex_x[0][1],         1);
         v[15].tex.set(           1,         1);
      }else
      {
         if(scale_do)
         {
            x[1][0]=rect.min.x+_size_x[1][0]*scale;
            x[1][1]=rect.max.x-_size_x[1][1]*scale;

            x[2][0]=rect.min.x+_size_x[2][0]*scale;
            x[2][1]=rect.max.x-_size_x[2][1]*scale;
         }else
         {
            x[1][0]=rect.min.x+_size_x[1][0];
            x[1][1]=rect.max.x-_size_x[1][1];

            x[2][0]=rect.min.x+_size_x[2][0];
            x[2][1]=rect.max.x-_size_x[2][1];
         }

         v[ 0].pos.set(      l, y0);
         v[ 1].pos.set(x[0][0], y0);
         v[ 2].pos.set(x[0][1], y0);
         v[ 3].pos.set(      r, y0);

         v[ 4].pos.set(      l, y1);
         v[ 5].pos.set(x[0][0], y1);
         v[ 6].pos.set(x[0][1], y1);
         v[ 7].pos.set(      r, y1);
         v[ 8].pos.set(x[1][0], y1);
         v[ 9].pos.set(x[1][1], y1);

         v[10].pos.set(      l, y2);
         v[11].pos.set(x[1][0], y2);
         v[12].pos.set(x[1][1], y2);
         v[13].pos.set(      r, y2);
         v[14].pos.set(x[2][0], y2);
         v[15].pos.set(x[2][1], y2);

         v[16].pos.set(      l, y3);
         v[17].pos.set(x[2][0], y3);
         v[18].pos.set(x[2][1], y3);
         v[19].pos.set(      r, y3);

         v[ 0].tex.set(           0, 0);
         v[ 1].tex.set(_tex_x[0][0], 0);
         v[ 2].tex.set(_tex_x[0][1], 0);
         v[ 3].tex.set(           1, 0);

         v[ 4].tex.set(           0, _tex_y[0]);
         v[ 5].tex.set(_tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(_tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(           1, _tex_y[0]);
         v[ 8].tex.set(_tex_x[1][0], _tex_y[0]);
         v[ 9].tex.set(_tex_x[1][1], _tex_y[0]);

         v[10].tex.set(           0, _tex_y[1]);
         v[11].tex.set(_tex_x[1][0], _tex_y[1]);
         v[12].tex.set(_tex_x[1][1], _tex_y[1]);
         v[13].tex.set(           1, _tex_y[1]);
         v[14].tex.set(_tex_x[2][0], _tex_y[1]);
         v[15].tex.set(_tex_x[2][1], _tex_y[1]);

         v[16].tex.set(           0, 1);
         v[17].tex.set(_tex_x[2][0], 1);
         v[18].tex.set(_tex_x[2][1], 1);
         v[19].tex.set(           1, 1);
      }

      if(image.partial())REP(vtxs)v[i].tex*=image._part.xy;

      VI.flushIndexed(_same_x ? IndBufPanel : IndBufPanelEx, (3*3-1)*2*3);
   }
   VI.clear();
}
/******************************************************************************/
static void Clip(Flt &min_pos, Flt &max_pos, Flt min_tex, Flt &max_tex, Flt clip_pos)
{
   if(min_pos>=clip_pos)min_pos=max_pos=clip_pos;else
   if(max_pos> clip_pos)
   {
      Flt frac=LerpR(min_pos, max_pos, clip_pos);
      max_pos=clip_pos;
      max_tex=Lerp(min_tex, max_tex, frac);
   }
}
void PanelImage::drawFrac(C Color &color, C Color &color_add, C Rect &rect, Flt frac_x, Bool include_padding)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (&image   );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   const Int vtxs=(_same_x ? 16 : 20);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(vtxs))
   {
      Flt scale; Bool scale_do=getSideScale(rect, scale);
      Flt l =rect.min.x, r =rect.max.x, x[3][2], // [y][x]
          y0=rect.max.y, y3=rect.min.y, y1, y2;
      if(scale_do)
      {
         y1     =y0-_size_y[0]   *scale;
         y2     =y3+_size_y[1]   *scale;
         x[0][0]=l +_size_x[0][0]*scale;
         x[0][1]=r -_size_x[0][1]*scale;

         if(_padd_any)
         {
            Flt padd;
            padd=_padd.x*(_force_uniform_stretch[0] ? rect.w() : scale); l -=padd; r +=padd;
            padd=_padd.y*(_force_uniform_stretch[1] ? rect.h() : scale); y0+=padd; y3-=padd;
         }
      }else
      {
         y1     =y0-_size_y[0];
         y2     =y3+_size_y[1];
         x[0][0]=l +_size_x[0][0];
         x[0][1]=r -_size_x[0][1];

         if(_padd_any)
         {
            if(_force_uniform_stretch[0]){Flt padd=_padd.x*rect.w(); l -=padd; r +=padd;}else{l -=_padd.x; r +=_padd.x;}
            if(_force_uniform_stretch[1]){Flt padd=_padd.y*rect.h(); y0+=padd; y3-=padd;}else{y0+=_padd.y; y3-=_padd.y;}
         }
      }

      Flt pos_x=(include_padding ? Lerp(l, r, frac_x) : rect.lerpX(frac_x)),
          tex_x[3][2], // [y][x]
          tex_r=1;
      tex_x[0][0]=_tex_x[0][0];
      tex_x[0][1]=_tex_x[0][1];

      if(_same_x)
      {
         Clip(      l, x[0][0],           0, tex_x[0][0], pos_x);
         Clip(x[0][0], x[0][1], tex_x[0][0], tex_x[0][1], pos_x);
         Clip(x[0][1],       r, tex_x[0][1], tex_r      , pos_x);

         v[ 0].pos.set(      l, y0);
         v[ 1].pos.set(x[0][0], y0);
         v[ 2].pos.set(x[0][1], y0);
         v[ 3].pos.set(      r, y0);
         v[ 4].pos.set(      l, y1);
         v[ 5].pos.set(x[0][0], y1);
         v[ 6].pos.set(x[0][1], y1);
         v[ 7].pos.set(      r, y1);
         v[ 8].pos.set(      l, y2);
         v[ 9].pos.set(x[0][0], y2);
         v[10].pos.set(x[0][1], y2);
         v[11].pos.set(      r, y2);
         v[12].pos.set(      l, y3);
         v[13].pos.set(x[0][0], y3);
         v[14].pos.set(x[0][1], y3);
         v[15].pos.set(      r, y3);

         v[ 0].tex.set(          0,         0);
         v[ 1].tex.set(tex_x[0][0],         0);
         v[ 2].tex.set(tex_x[0][1],         0);
         v[ 3].tex.set(tex_r      ,         0);

         v[ 4].tex.set(          0, _tex_y[0]);
         v[ 5].tex.set(tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(tex_r      , _tex_y[0]);

         v[ 8].tex.set(          0, _tex_y[1]);
         v[ 9].tex.set(tex_x[0][0], _tex_y[1]);
         v[10].tex.set(tex_x[0][1], _tex_y[1]);
         v[11].tex.set(tex_r      , _tex_y[1]);

         v[12].tex.set(          0,         1);
         v[13].tex.set(tex_x[0][0],         1);
         v[14].tex.set(tex_x[0][1],         1);
         v[15].tex.set(tex_r      ,         1);
      }else
      {
         if(scale_do)
         {
            x[1][0]=rect.min.x+_size_x[1][0]*scale;
            x[1][1]=rect.max.x-_size_x[1][1]*scale;

            x[2][0]=rect.min.x+_size_x[2][0]*scale;
            x[2][1]=rect.max.x-_size_x[2][1]*scale;
         }else
         {
            x[1][0]=rect.min.x+_size_x[1][0];
            x[1][1]=rect.max.x-_size_x[1][1];

            x[2][0]=rect.min.x+_size_x[2][0];
            x[2][1]=rect.max.x-_size_x[2][1];
         }

         tex_x[1][0]=_tex_x[1][0];
         tex_x[1][1]=_tex_x[1][1];

         tex_x[2][0]=_tex_x[2][0];
         tex_x[2][1]=_tex_x[2][1];

         Flt L=l; Clip(      l, x[0][0],           0, tex_x[0][0], pos_x);
                  Clip(x[0][0], x[0][1], tex_x[0][0], tex_x[0][1], pos_x);
                  Clip(x[0][1],       r, tex_x[0][1], tex_r      , pos_x);

             l=L; Clip(      l, x[1][0],           0, tex_x[1][0], pos_x);
                  Clip(x[1][0], x[1][1], tex_x[1][0], tex_x[1][1], pos_x);

             l=L; Clip(      l, x[2][0],           0, tex_x[2][0], pos_x);
                  Clip(x[2][0], x[2][1], tex_x[2][0], tex_x[2][1], pos_x);

         v[ 0].pos.set(      l, y0);
         v[ 1].pos.set(x[0][0], y0);
         v[ 2].pos.set(x[0][1], y0);
         v[ 3].pos.set(      r, y0);

         v[ 4].pos.set(      l, y1);
         v[ 5].pos.set(x[0][0], y1);
         v[ 6].pos.set(x[0][1], y1);
         v[ 7].pos.set(      r, y1);
         v[ 8].pos.set(x[1][0], y1);
         v[ 9].pos.set(x[1][1], y1);

         v[10].pos.set(      l, y2);
         v[11].pos.set(x[1][0], y2);
         v[12].pos.set(x[1][1], y2);
         v[13].pos.set(      r, y2);
         v[14].pos.set(x[2][0], y2);
         v[15].pos.set(x[2][1], y2);

         v[16].pos.set(      l, y3);
         v[17].pos.set(x[2][0], y3);
         v[18].pos.set(x[2][1], y3);
         v[19].pos.set(      r, y3);

         v[ 0].tex.set(          0, 0);
         v[ 1].tex.set(tex_x[0][0], 0);
         v[ 2].tex.set(tex_x[0][1], 0);
         v[ 3].tex.set(tex_r      , 0);

         v[ 4].tex.set(          0, _tex_y[0]);
         v[ 5].tex.set(tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(tex_r      , _tex_y[0]);
         v[ 8].tex.set(tex_x[1][0], _tex_y[0]);
         v[ 9].tex.set(tex_x[1][1], _tex_y[0]);

         v[10].tex.set(          0, _tex_y[1]);
         v[11].tex.set(tex_x[1][0], _tex_y[1]);
         v[12].tex.set(tex_x[1][1], _tex_y[1]);
         v[13].tex.set(tex_r      , _tex_y[1]);
         v[14].tex.set(tex_x[2][0], _tex_y[1]);
         v[15].tex.set(tex_x[2][1], _tex_y[1]);

         v[16].tex.set(          0, 1);
         v[17].tex.set(tex_x[2][0], 1);
         v[18].tex.set(tex_x[2][1], 1);
         v[19].tex.set(tex_r      , 1);
      }

      if(image.partial())REP(vtxs)v[i].tex*=image._part.xy;

      VI.flushIndexed(_same_x ? IndBufPanel : IndBufPanelEx, 3*3*2*3);
   }
   VI.clear();
}
void PanelImage::drawVerticalFrac(C Color &color, C Color &color_add, C Rect &rect, Flt frac_y, Bool include_padding)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (&image   );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   const Int vtxs=(_same_x ? 16 : 20);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(vtxs))
   {
      Flt scale; Bool scale_do=getSideScaleVertical(rect, scale);
      Flt b =rect.min.y, t =rect.max.y, y[3][2], // [y][x]
          x0=rect.min.x, x3=rect.max.x, x1, x2;
      if(scale_do)
      {
         x1     =x0+_size_y[0]   *scale;
         x2     =x3-_size_y[1]   *scale;
         y[0][0]=b +_size_x[0][0]*scale;
         y[0][1]=t -_size_x[0][1]*scale;

         if(_padd_any)
         {
            Flt padd;
            padd=_padd.x*(_force_uniform_stretch[0] ? rect.h() : scale); b -=padd; t +=padd;
            padd=_padd.y*(_force_uniform_stretch[1] ? rect.w() : scale); x0-=padd; x3+=padd;
         }
      }else
      {
         x1     =x0+_size_y[0];
         x2     =x3-_size_y[1];
         y[0][0]=b +_size_x[0][0];
         y[0][1]=t -_size_x[0][1];

         if(_padd_any)
         {
            if(_force_uniform_stretch[0]){Flt padd=_padd.x*rect.h(); b -=padd; t +=padd;}else{b -=_padd.x; t +=_padd.x;}
            if(_force_uniform_stretch[1]){Flt padd=_padd.y*rect.w(); x0-=padd; x3+=padd;}else{x0-=_padd.y; x3+=_padd.y;}
         }
      }

      Flt pos_y=(include_padding ? Lerp(b, t, frac_y) : rect.lerpY(frac_y)),
          tex_x[3][2], // [y][x]
          tex_t=1;
      tex_x[0][0]=_tex_x[0][0];
      tex_x[0][1]=_tex_x[0][1];

      if(_same_x)
      {
         Clip(      b, y[0][0],           0, tex_x[0][0], pos_y);
         Clip(y[0][0], y[0][1], tex_x[0][0], tex_x[0][1], pos_y);
         Clip(y[0][1],       t, tex_x[0][1], tex_t      , pos_y);

         v[ 0].pos.set(x0,       b);
         v[ 1].pos.set(x0, y[0][0]);
         v[ 2].pos.set(x0, y[0][1]);
         v[ 3].pos.set(x0,       t);
         v[ 4].pos.set(x1,       b);
         v[ 5].pos.set(x1, y[0][0]);
         v[ 6].pos.set(x1, y[0][1]);
         v[ 7].pos.set(x1,       t);
         v[ 8].pos.set(x2,       b);
         v[ 9].pos.set(x2, y[0][0]);
         v[10].pos.set(x2, y[0][1]);
         v[11].pos.set(x2,       t);
         v[12].pos.set(x3,       b);
         v[13].pos.set(x3, y[0][0]);
         v[14].pos.set(x3, y[0][1]);
         v[15].pos.set(x3,       t);

         v[ 0].tex.set(          0,         0);
         v[ 1].tex.set(tex_x[0][0],         0);
         v[ 2].tex.set(tex_x[0][1],         0);
         v[ 3].tex.set(tex_t      ,         0);

         v[ 4].tex.set(          0, _tex_y[0]);
         v[ 5].tex.set(tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(tex_t      , _tex_y[0]);

         v[ 8].tex.set(          0, _tex_y[1]);
         v[ 9].tex.set(tex_x[0][0], _tex_y[1]);
         v[10].tex.set(tex_x[0][1], _tex_y[1]);
         v[11].tex.set(tex_t      , _tex_y[1]);

         v[12].tex.set(          0,         1);
         v[13].tex.set(tex_x[0][0],         1);
         v[14].tex.set(tex_x[0][1],         1);
         v[15].tex.set(tex_t      ,         1);
      }else
      {
         if(scale_do)
         {
            y[1][0]=rect.min.y+_size_x[1][0]*scale;
            y[1][1]=rect.max.y-_size_x[1][1]*scale;

            y[2][0]=rect.min.y+_size_x[2][0]*scale;
            y[2][1]=rect.max.y-_size_x[2][1]*scale;
         }else
         {
            y[1][0]=rect.min.y+_size_x[1][0];
            y[1][1]=rect.max.y-_size_x[1][1];

            y[2][0]=rect.min.y+_size_x[2][0];
            y[2][1]=rect.max.y-_size_x[2][1];
         }

         tex_x[1][0]=_tex_x[1][0];
         tex_x[1][1]=_tex_x[1][1];

         tex_x[2][0]=_tex_x[2][0];
         tex_x[2][1]=_tex_x[2][1];

         Flt B=b; Clip(      b, y[0][0],           0, tex_x[0][0], pos_y);
                  Clip(y[0][0], y[0][1], tex_x[0][0], tex_x[0][1], pos_y);
                  Clip(y[0][1],       t, tex_x[0][1], tex_t      , pos_y);

             b=B; Clip(      b, y[1][0],           0, tex_x[1][0], pos_y);
                  Clip(y[1][0], y[1][1], tex_x[1][0], tex_x[1][1], pos_y);

             b=B; Clip(      b, y[2][0],           0, tex_x[2][0], pos_y);
                  Clip(y[2][0], y[2][1], tex_x[2][0], tex_x[2][1], pos_y);

         v[ 0].pos.set(x0,       b);
         v[ 1].pos.set(x0, y[0][0]);
         v[ 2].pos.set(x0, y[0][1]);
         v[ 3].pos.set(x0,       t);

         v[ 4].pos.set(x1,       b);
         v[ 5].pos.set(x1, y[0][0]);
         v[ 6].pos.set(x1, y[0][1]);
         v[ 7].pos.set(x1,       t);
         v[ 8].pos.set(x1, y[1][0]);
         v[ 9].pos.set(x1, y[1][1]);

         v[10].pos.set(x2,       b);
         v[11].pos.set(x2, y[1][0]);
         v[12].pos.set(x2, y[1][1]);
         v[13].pos.set(x2,       t);
         v[14].pos.set(x2, y[2][0]);
         v[15].pos.set(x2, y[2][1]);

         v[16].pos.set(x3,       b);
         v[17].pos.set(x3, y[2][0]);
         v[18].pos.set(x3, y[2][1]);
         v[19].pos.set(x3,       t);

         v[ 0].tex.set(          0, 0);
         v[ 1].tex.set(tex_x[0][0], 0);
         v[ 2].tex.set(tex_x[0][1], 0);
         v[ 3].tex.set(tex_t      , 0);

         v[ 4].tex.set(          0, _tex_y[0]);
         v[ 5].tex.set(tex_x[0][0], _tex_y[0]);
         v[ 6].tex.set(tex_x[0][1], _tex_y[0]);
         v[ 7].tex.set(tex_t      , _tex_y[0]);
         v[ 8].tex.set(tex_x[1][0], _tex_y[0]);
         v[ 9].tex.set(tex_x[1][1], _tex_y[0]);

         v[10].tex.set(          0, _tex_y[1]);
         v[11].tex.set(tex_x[1][0], _tex_y[1]);
         v[12].tex.set(tex_x[1][1], _tex_y[1]);
         v[13].tex.set(tex_t      , _tex_y[1]);
         v[14].tex.set(tex_x[2][0], _tex_y[1]);
         v[15].tex.set(tex_x[2][1], _tex_y[1]);

         v[16].tex.set(          0, 1);
         v[17].tex.set(tex_x[2][0], 1);
         v[18].tex.set(tex_x[2][1], 1);
         v[19].tex.set(tex_t      , 1);
      }

      if(image.partial())REP(vtxs)v[i].tex*=image._part.xy;

      VI.flushIndexed(_same_x ? IndBufPanel : IndBufPanelEx, 3*3*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void PanelImage::drawBaseLines(C Color &line_color, C Rect &rect)C
{
   Rect r=rect; r.swapY();
   Vec2 lt=r.lerp (_tex_left_top    ),
        rb=r.lerp (_tex_right_bottom);
   Flt  y1=r.lerpY(_tex_y[0]        ),
        y2=r.lerpY(_tex_y[1]        );
   Rect(lt, rb).draw(line_color, false);
         D.lineX(line_color, y1, lt.x, rb.x);
         D.lineX(line_color, y2, lt.x, rb.x);
   REP(2)D.lineY(line_color, r.lerpX(_tex_x[0][i]), lt.y, y1  );
   REP(2)D.lineY(line_color, r.lerpX(_tex_x[1][i]), y1  , y2  );
   REP(2)D.lineY(line_color, r.lerpX(_tex_x[2][i]), y2  , rb.y);
}
void PanelImage::drawScaledLines(C Color &line_color, C Rect &rect)C
{
   Flt scale; Bool scale_do=getSideScale(rect, scale);
   Flt l =rect.min.x, r =rect.max.x, x[3][2], // [y][x]
       y0=rect.max.y, y3=rect.min.y, y1, y2;
   if(scale_do)
   {
      y1     =y0-_size_y[0]   *scale;
      y2     =y3+_size_y[1]   *scale;
      x[0][0]=l +_size_x[0][0]*scale;
      x[0][1]=r -_size_x[0][1]*scale;

      x[1][0]=l+_size_x[1][0]*scale;
      x[1][1]=r-_size_x[1][1]*scale;

      x[2][0]=l+_size_x[2][0]*scale;
      x[2][1]=r-_size_x[2][1]*scale;
      if(_padd_any)
      {
         Flt padd;
         padd=_padd.x*(_force_uniform_stretch[0] ? rect.w() : scale); l -=padd; r +=padd;
         padd=_padd.y*(_force_uniform_stretch[1] ? rect.h() : scale); y0+=padd; y3-=padd;
      }
   }else
   {
      y1     =y0-_size_y[0];
      y2     =y3+_size_y[1];
      x[0][0]=l +_size_x[0][0];
      x[0][1]=r -_size_x[0][1];

      x[1][0]=l+_size_x[1][0];
      x[1][1]=r-_size_x[1][1];

      x[2][0]=l+_size_x[2][0];
      x[2][1]=r-_size_x[2][1];

      if(_padd_any)
      {
         if(_force_uniform_stretch[0]){Flt padd=_padd.x*rect.w(); l -=padd; r +=padd;}else{l -=_padd.x; r +=_padd.x;}
         if(_force_uniform_stretch[1]){Flt padd=_padd.y*rect.h(); y0+=padd; y3-=padd;}else{y0+=_padd.y; y3-=_padd.y;}
      }
   }
   Rect(l, y3, r, y0).draw(line_color, false);
         D.lineX(line_color, y1, l, r);
         D.lineX(line_color, y2, l, r);
   REP(2)D.lineY(line_color, x[0][i], y0, y1);
   REP(2)D.lineY(line_color, x[1][i], y1, y2);
   REP(2)D.lineY(line_color, x[2][i], y2, y3);
}
/******************************************************************************/
void PanelImage::drawInnerPadding(C Color &line_color, C Rect &rect)C
{
   Rect padding; innerPadding(rect, padding);
   padding.min=rect.min+padding.min;
   padding.max=rect.max-padding.max;
   padding.draw(line_color, false);
}
/******************************************************************************/
Bool PanelImage::save(File &f)C
{
   f.putUInt(CC4_PIMG);
   f.cmpUIntV(1);
   f<<_same_x<<_padd_any<<_force_uniform_stretch<<_size_x<<_size_y<<_tex_x<<_tex_y<<_side_size<<_padd<<_tex_left_top<<_tex_right_bottom<<_inner_padding;
   if(!image.save(f))return false;
   return f.ok();
}
Bool PanelImage::load(File &f)
{
   if(f.getUInt()==CC4_PIMG)switch(f.decUIntV())
   {
      case 1:
      {
         f>>_same_x>>_padd_any>>_force_uniform_stretch>>_size_x>>_size_y>>_tex_x>>_tex_y>>_side_size>>_padd>>_tex_left_top>>_tex_right_bottom>>_inner_padding;
         if(!image.load(f))goto error;
         if(f.ok())return true;
      }break;

      case 0:
      {
         f>>_same_x>>_padd_any>>_force_uniform_stretch>>_size_x>>_size_y>>_tex_x>>_tex_y>>_side_size>>_padd>>_tex_left_top>>_tex_right_bottom;
        _inner_padding.set(_size_x[1][0], _size_y[1], _size_x[1][1], _size_y[0]);
         if(!image.load(f))goto error;
         if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}
Bool PanelImage::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool PanelImage::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
