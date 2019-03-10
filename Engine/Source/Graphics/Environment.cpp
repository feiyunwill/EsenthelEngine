/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_ENV CC4('E','N','V',0)
/******************************************************************************/
DEFINE_CACHE(Environment, Environments, EnvironmentPtr, "Environment");
/******************************************************************************/
// AMBIENT
/******************************************************************************/
void Environment::Ambient::set  ()C {D.ambientColor(on ? color : VecZero); D.nightShadeColor(on ? night_shade_color : VecZero);}
void Environment::Ambient::get  ()  {color=D.ambientColor(); night_shade_color=D.nightShadeColor(); on=(color.any() || night_shade_color.any());}
void Environment::Ambient::reset()  {on=true; color=0.4f; night_shade_color.zero();}

Bool Environment::Ambient::save(File &f, CChar *path)C
{
   f.cmpUIntV(1); // version
   f<<on<<color<<night_shade_color;
   return f.ok();
}
Bool Environment::Ambient::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 1:
      {
         f>>on>>color>>night_shade_color;
         if(f.ok())return true;
      }break;

      case 0:
      {
         f>>on>>color; night_shade_color.zero();
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// BLOOM
/******************************************************************************/
void Environment::Bloom::set()C
{
   D.bloomOriginal(on ? original : 1).bloomScale(on ? scale : 0).bloomCut(cut).bloomSaturate(saturate).bloomMaximum(maximum).bloomHalf(half).bloomBlurs(blurs);
}
void Environment::Bloom::get()
{
   original=D.bloomOriginal(); scale=D.bloomScale(); cut=D.bloomCut(); saturate=D.bloomSaturate(); maximum=D.bloomMaximum(); half=D.bloomHalf(); blurs=D.bloomBlurs();
   on=!(Equal(original, 1) && Equal(scale, 0));
}
void Environment::Bloom::reset()
{
   on=true; half=true; saturate=true; maximum=false; blurs=1; original=1.0f; scale=0.5f; cut=0.3f;
}

Bool Environment::Bloom::save(File &f, CChar *path)C
{
   f.cmpUIntV(2); // version
   f<<on<<half<<saturate<<maximum<<blurs<<original<<scale<<cut;
   return f.ok();
}
Bool Environment::Bloom::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 2:
      {
         f>>on>>half>>saturate>>maximum>>blurs>>original>>scale>>cut;
         if(f.ok())return true;
      }break;

      case 1:
      {
         Flt contrast; f>>on>>half>>maximum>>blurs>>original>>scale>>cut>>contrast; saturate=true;
         if(f.ok())return true;
      }break;

      case 0:
      {
         Flt contrast; f>>on>>half>>maximum>>blurs>>original>>scale>>cut>>contrast; saturate=true; scale*=2;
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// CLOUDS
/******************************************************************************/
void Environment::Clouds::set()C
{
   Int active=0; FREPA(layers)if(layers[i].image)
   {
                   C Layer &src =                 layers[i       ];
      LayeredClouds::Layer &dest=::Clouds.layered.layer [active++];
      dest.scale=src.scale; dest.velocity=src.velocity; dest.color=src.color; dest.image=src.image;
   }
 ::Clouds.layered.set(on ? active : false);
 ::Clouds.layered.scaleY(vertical_scale);
 ::Clouds.layered.rayMaskContrast(ray_mask_contrast);
}
void Environment::Clouds::get()
{
   REPA(layers)
   {
    C LayeredClouds::Layer &src =::Clouds.layered.layer [i];
                     Layer &dest=                 layers[i];
      dest.scale=src.scale; dest.velocity=src.velocity; dest.color=src.color.asVec4(); dest.image=src.image;
   }
   on=(::Clouds.layered.layers()>0);
   vertical_scale   =::Clouds.layered.scaleY();
   ray_mask_contrast=::Clouds.layered.rayMaskContrast();
}
void Environment::Clouds::reset()
{
   on=true; vertical_scale=1.05f; ray_mask_contrast=4;
   REPA(layers){Layer &layer=layers[i]; layer.color=1; layer.image=null;}
   layers[0].scale=1.0f/2.8f; layers[0].velocity=0.010f;
   layers[1].scale=1.0f/2.4f; layers[1].velocity=0.008f;
   layers[2].scale=1.0f/2.0f; layers[2].velocity=0.006f;
   layers[3].scale=1.0f/1.6f; layers[3].velocity=0.004f;
}

Bool Environment::Clouds::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<on<<vertical_scale<<ray_mask_contrast;
   FREPA(layers)
   {
    C Layer &layer=layers[i]; f<<layer.scale<<layer.velocity<<layer.color; f._putStr(layer.image.name(path));
   }
   return f.ok();
}
Bool Environment::Clouds::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>on>>vertical_scale>>ray_mask_contrast;
         FREPA(layers)
         {
            Layer &layer=layers[i]; f>>layer.scale>>layer.velocity>>layer.color; layer.image.require(f._getStr(), path);
         }
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// FOG
/******************************************************************************/
void Environment::Fog::set  ()C {::Fog.draw=on; ::Fog.affect_sky=affect_sky; ::Fog.density=density; ::Fog.color=color;}
void Environment::Fog::get  ()  {on=::Fog.draw; affect_sky=::Fog.affect_sky; density=::Fog.density; color=::Fog.color;}
void Environment::Fog::reset()  {on=false; affect_sky=false; density=0.02f; color=0.5f;}

Bool Environment::Fog::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<on<<affect_sky<<density<<color;
   return f.ok();
}
Bool Environment::Fog::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>on>>affect_sky>>density>>color;
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// SKY
/******************************************************************************/
void Environment::Sky::set()C
{
 ::Sky.frac(frac)
      .atmosphericDensityExponent(atmospheric_density_exponent).atmosphericHorizonExponent (atmospheric_horizon_exponent )
      .atmosphericHorizonColor   (atmospheric_horizon_color   ).atmosphericSkyColor        (atmospheric_sky_color        )
      .atmosphericStars          (atmospheric_stars           ).atmosphericStarsOrientation(atmospheric_stars_orientation);
   if(!on    )::Sky.clear();else
   if( skybox)::Sky.skybox(skybox);else
              ::Sky.atmospheric();
}
void Environment::Sky::get()
{
       on=::Sky._is;
     frac=::Sky.frac();
   skybox=::Sky.skybox();
   atmospheric_density_exponent=::Sky.atmosphericDensityExponent(); atmospheric_horizon_exponent =::Sky.atmosphericHorizonExponent ();
   atmospheric_horizon_color   =::Sky.atmosphericHorizonColor   (); atmospheric_sky_color        =::Sky.atmosphericSkyColor        ();
   atmospheric_stars           =::Sky.atmosphericStars          (); atmospheric_stars_orientation=::Sky.atmosphericStarsOrientation();
}
void Environment::Sky::reset()
{
       on=true;
     frac=0.8f;
   skybox=null;
   atmospheric_density_exponent=1;
   atmospheric_horizon_exponent=3.5f;
   atmospheric_horizon_color.set(0.32f, 0.46f, 0.58f, 1.0f);
   atmospheric_sky_color    .set(0.16f, 0.36f, 0.54f, 1.0f);
   atmospheric_stars            =null;
   atmospheric_stars_orientation.identity();
}

Bool Environment::Sky::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<on<<frac<<atmospheric_density_exponent<<atmospheric_horizon_exponent<<atmospheric_horizon_color<<atmospheric_sky_color<<atmospheric_stars_orientation;
   f._putStr(atmospheric_stars.name(path))._putStr(skybox.name(path));
   return f.ok();
}
Bool Environment::Sky::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>on>>frac>>atmospheric_density_exponent>>atmospheric_horizon_exponent>>atmospheric_horizon_color>>atmospheric_sky_color>>atmospheric_stars_orientation;
         atmospheric_stars.require(f._getStr(), path);
         skybox           .require(f._getStr(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// SUN
/******************************************************************************/
void Environment::Sun::set()C
{
 ::Sun.draw           =on;
 ::Sun.blend          =blend;
 ::Sun.glow           =glow;
 ::Sun.size           =size;
 ::Sun.highlight_front=highlight_front;
 ::Sun.highlight_back =highlight_back;
 ::Sun.pos            =pos;
 ::Sun.light_color    =light_color;
 ::Sun. rays_color    = rays_color;
 ::Sun.image_color    =image_color;
 ::Sun.image          =image;
}
void Environment::Sun::get()
{
   on             =::Sun.draw;
   blend          =::Sun.blend;
   glow           =::Sun.glow;
   size           =::Sun.size;
   highlight_front=::Sun.highlight_front;
   highlight_back =::Sun.highlight_back;
   pos            =::Sun.pos;
   light_color    =::Sun.light_color;
    rays_color    =::Sun. rays_color;
   image_color    =::Sun.image_color.asVec4();
   image          =::Sun.image;
}
void Environment::Sun::reset()
{
   on=true; blend=true; glow=128; size=0.15; highlight_front=0.10f; highlight_back=0.07f; pos.set(-SQRT3_3, SQRT3_3, -SQRT3_3); light_color=0.7f; rays_color=0.05f; image_color=1; image=null;
}

Bool Environment::Sun::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<on<<blend<<glow<<size<<highlight_front<<highlight_back<<pos<<light_color<<rays_color<<image_color; f._putStr(image.name(path));
   return f.ok();
}
Bool Environment::Sun::load(File &f, CChar *path)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>on>>blend>>glow>>size>>highlight_front>>highlight_back>>pos>>light_color>>rays_color>>image_color; image.require(f._getStr(), path);
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// ENVIRONMENT
/******************************************************************************/
void Environment::set  ()C {ambient.set  (); bloom.set  (); clouds.set  (); fog.set  (); sky.set  (); sun.set  ();}
void Environment::get  ()  {ambient.get  (); bloom.get  (); clouds.get  (); fog.get  (); sky.get  (); sun.get  ();}
void Environment::reset()  {ambient.reset(); bloom.reset(); clouds.reset(); fog.reset(); sky.reset(); sun.reset();}

Bool Environment::save(File &f, CChar *path)C
{
   f.putUInt(CC4_ENV).cmpUIntV(0); // version
   if(ambient.save(f, path))
   if(bloom  .save(f, path))
   if(clouds .save(f, path))
   if(fog    .save(f, path))
   if(sky    .save(f, path))
   if(sun    .save(f, path))
      return f.ok();
   return false;
}
Bool Environment::load(File &f, CChar *path)
{
   if(f.getUInt()==CC4_ENV)switch(f.decUIntV())
   {
      case 0:
      {
         if(ambient.load(f, path))
         if(bloom  .load(f, path))
         if(clouds .load(f, path))
         if(fog    .load(f, path))
         if(sky    .load(f, path))
         if(sun    .load(f, path))
            if(f.ok())return true;
      }break;
   }
   reset(); return false;
}

Bool Environment::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Environment::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
