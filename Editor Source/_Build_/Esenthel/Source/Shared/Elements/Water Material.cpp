/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   EditWaterMtrl::EditWaterMtrl() : density(0.3f), density_add(0.45f), density_underwater(0.02f), density_underwater_add(0.6f), scale_color(200), scale_normal(10), scale_bump(100), reflect_world(0.18f), refract(0.10f), refract_reflection(0.06f), refract_underwater(0.01f), wave_scale(0.25f), fresnel_pow(5.5f), fresnel_rough(4), fresnel_color(0.10f, 0.10f, 0.10f), color_underwater0(0.26f, 0.35f, 0.42f), color_underwater1(0.10f, 0.20f, 0.30f) {::EditMaterial::color.set(0.42f, 0.50f, 0.58f, 1); ::EditMaterial::rough=1; ::EditMaterial::specular=1.5f; ::EditMaterial::reflection=0.1f;}
   bool EditWaterMtrl::usesTexBump()C {return wave_scale>EPSL && hasBumpMap();}
   bool EditWaterMtrl::equal(C EditWaterMtrl &src)C
   {
      return ::EditMaterial::equal(src) && density_time==src.density_time && density_underwater_time==src.density_underwater_time && scale_color_time==src.scale_color_time && scale_normal_time==src.scale_normal_time && scale_bump_time==src.scale_bump_time
      && reflect_world_time==src.reflect_world_time && refract_time==src.refract_time && refract_reflection_time==src.refract_reflection_time && refract_underwater_time==src.refract_underwater_time && wave_scale_time==src.wave_scale_time
      && fresnel_pow_time==src.fresnel_pow_time && fresnel_rough_time==src.fresnel_rough_time && fresnel_color_time==src.fresnel_color_time && color_underwater_time==src.color_underwater_time;
   }
   bool EditWaterMtrl::newer(C EditWaterMtrl &src)C
   {
      return ::EditMaterial::newer(src) || density_time>src.density_time || density_underwater_time>src.density_underwater_time || scale_color_time>src.scale_color_time || scale_normal_time>src.scale_normal_time || scale_bump_time>src.scale_bump_time
      || reflect_world_time>src.reflect_world_time || refract_time>src.refract_time || refract_reflection_time>src.refract_reflection_time || refract_underwater_time>src.refract_underwater_time || wave_scale_time>src.wave_scale_time
      || fresnel_pow_time>src.fresnel_pow_time || fresnel_rough_time>src.fresnel_rough_time || fresnel_color_time>src.fresnel_color_time || color_underwater_time>src.color_underwater_time;
   }
   void EditWaterMtrl::reset() {T=EditWaterMtrl();}
   void EditWaterMtrl::newData()
   {
      ::EditMaterial::newData();
      density_time++; density_underwater_time++; scale_color_time++; scale_normal_time++; scale_bump_time++;
      reflect_world_time++; refract_time++; refract_reflection_time++; refract_underwater_time++; wave_scale_time++;
      fresnel_pow_time++; fresnel_rough_time++; fresnel_color_time++; color_underwater_time++;
   }
   void EditWaterMtrl::create(C WaterMtrl &src, C TimeStamp &time)
   {
      ::EditMaterial::create(Material(), time); // call super to setup times for all values
      density               =src.density;
      density_add           =src.density_add; density_time=time;
      density_underwater    =src.density_underwater;
      density_underwater_add=src.density_underwater_add; density_underwater_time=time;
      scale_color           =src.scale_color; scale_color_time=time;
      scale_normal          =src.scale_normal; scale_normal_time=time;
      scale_bump            =src.scale_bump; scale_bump_time=time;
::EditMaterial::rough                 =src.rough; rough_bump_time=time;
::EditMaterial::reflection            =src.reflect_tex; reflection_time=time;
      reflect_world         =src.reflect_world; reflect_world_time=time;
      refract               =src.refract; refract_time=time;
      refract_reflection    =src.refract_reflection; refract_reflection_time=time;
      refract_underwater    =src.refract_underwater; refract_underwater_time=time;
::EditMaterial::specular              =src.specular; spec_time=time;
      wave_scale            =src.wave_scale; wave_scale_time=time;
      fresnel_pow           =src.fresnel_pow; fresnel_pow_time=time;
      fresnel_rough         =src.fresnel_rough; fresnel_rough_time=time;
      fresnel_color         =src.fresnel_color; fresnel_color_time=time;
::EditMaterial::color                 =Vec4(src.color, 1); color_time=time;
      color_underwater0     =src.color_underwater0; color_underwater_time=time;
      color_underwater1     =src.color_underwater1;
          base_0_tex=src.     colorMap().id();
          base_1_tex=src.    normalMap().id();
      reflection_tex=src.reflectionMap().id();
   }
   void EditWaterMtrl::copyTo(WaterMtrl &dest, C Project &proj)C
   {
      dest.density               =density               ;
      dest.density_add           =density_add           ;
      dest.density_underwater    =density_underwater    ;
      dest.density_underwater_add=density_underwater_add;
      dest.scale_color           =scale_color           ;
      dest.scale_normal          =scale_normal          ;
      dest.scale_bump            =scale_bump            ;
      dest.rough                 =::EditMaterial::rough           ;
      dest.reflect_tex           =::EditMaterial::reflection      ;
      dest.reflect_world         =reflect_world         ;
      dest.refract               =refract               ;
      dest.refract_reflection    =refract_reflection    ;
      dest.refract_underwater    =refract_underwater    ;
      dest.specular              =::EditMaterial::specular        ;
      dest.wave_scale            =wave_scale            ;
      dest.fresnel_pow           =fresnel_pow           ;
      dest.fresnel_rough         =fresnel_rough         ;
      dest.fresnel_color         =fresnel_color         ;
      dest.color                 =::EditMaterial::color.xyz       ;
      dest.color_underwater0     =color_underwater0     ;
      dest.color_underwater1     =color_underwater1     ;
      dest.     colorMap(proj.texPath(    base_0_tex))
          .    normalMap(proj.texPath(    base_1_tex))
          .reflectionMap(proj.texPath(reflection_tex));
      dest.validate();
   }
   uint EditWaterMtrl::sync(C EditWaterMtrl &src)
   {
      uint changed=::EditMaterial::sync(src);
      if(Sync(density_time, src.density_time))
      {
         changed|=CHANGED_PARAM;
         density    =src.density;
         density_add=src.density_add;
      }
      if(Sync(density_underwater_time, src.density_underwater_time))
      {
         changed|=CHANGED_PARAM;
         density_underwater     =src.density_underwater;
         density_underwater_add =src.density_underwater_add;
      }
      changed|=Sync(scale_color_time       , src.scale_color_time       , scale_color       , src.scale_color       )*CHANGED_PARAM;
      changed|=Sync(scale_normal_time      , src.scale_normal_time      , scale_normal      , src.scale_normal      )*CHANGED_PARAM;
      changed|=Sync(scale_bump_time        , src.scale_bump_time        , scale_bump        , src.scale_bump        )*CHANGED_PARAM;
      changed|=Sync(reflect_world_time     , src.reflect_world_time     , reflect_world     , src.reflect_world     )*CHANGED_PARAM;
      changed|=Sync(refract_time           , src.refract_time           , refract           , src.refract           )*CHANGED_PARAM;
      changed|=Sync(refract_reflection_time, src.refract_reflection_time, refract_reflection, src.refract_reflection)*CHANGED_PARAM;
      changed|=Sync(refract_underwater_time, src.refract_underwater_time, refract_underwater, src.refract_underwater)*CHANGED_PARAM;
      changed|=Sync(wave_scale_time        , src.wave_scale_time        , wave_scale        , src.wave_scale        )*CHANGED_PARAM;
      changed|=Sync(fresnel_pow_time       , src.fresnel_pow_time       , fresnel_pow       , src.fresnel_pow       )*CHANGED_PARAM;
      changed|=Sync(fresnel_rough_time     , src.fresnel_rough_time     , fresnel_rough     , src.fresnel_rough     )*CHANGED_PARAM;
      changed|=Sync(fresnel_color_time     , src.fresnel_color_time     , fresnel_color     , src.fresnel_color     )*CHANGED_PARAM;
      if(Sync(color_underwater_time, src.color_underwater_time))
      {
         changed|=CHANGED_PARAM;
         color_underwater0=src.color_underwater0;
         color_underwater1=src.color_underwater1;
      }
      return changed;
   }
   uint EditWaterMtrl::undo(C EditWaterMtrl &src)
   {
      uint changed=::EditMaterial::undo(src);
      if(Undo(density_time, src.density_time))
      {
         changed|=CHANGED_PARAM;
         density    =src.density;
         density_add=src.density_add;
      }
      if(Undo(density_underwater_time, src.density_underwater_time))
      {
         changed|=CHANGED_PARAM;
         density_underwater     =src.density_underwater;
         density_underwater_add =src.density_underwater_add;
      }
      changed|=Undo(scale_color_time       , src.scale_color_time       , scale_color       , src.scale_color       )*CHANGED_PARAM;
      changed|=Undo(scale_normal_time      , src.scale_normal_time      , scale_normal      , src.scale_normal      )*CHANGED_PARAM;
      changed|=Undo(scale_bump_time        , src.scale_bump_time        , scale_bump        , src.scale_bump        )*CHANGED_PARAM;
      changed|=Undo(reflect_world_time     , src.reflect_world_time     , reflect_world     , src.reflect_world     )*CHANGED_PARAM;
      changed|=Undo(refract_time           , src.refract_time           , refract           , src.refract           )*CHANGED_PARAM;
      changed|=Undo(refract_reflection_time, src.refract_reflection_time, refract_reflection, src.refract_reflection)*CHANGED_PARAM;
      changed|=Undo(refract_underwater_time, src.refract_underwater_time, refract_underwater, src.refract_underwater)*CHANGED_PARAM;
      changed|=Undo(wave_scale_time        , src.wave_scale_time        , wave_scale        , src.wave_scale        )*CHANGED_PARAM;
      changed|=Undo(fresnel_pow_time       , src.fresnel_pow_time       , fresnel_pow       , src.fresnel_pow       )*CHANGED_PARAM;
      changed|=Undo(fresnel_rough_time     , src.fresnel_rough_time     , fresnel_rough     , src.fresnel_rough     )*CHANGED_PARAM;
      changed|=Undo(fresnel_color_time     , src.fresnel_color_time     , fresnel_color     , src.fresnel_color     )*CHANGED_PARAM;
      if(Undo(color_underwater_time, src.color_underwater_time))
      {
         changed|=CHANGED_PARAM;
         color_underwater0=src.color_underwater0;
         color_underwater1=src.color_underwater1;
      }
      return changed;
   }
   bool EditWaterMtrl::save(File &f)C
   {
      f.cmpUIntV(1);
      ::EditMaterial::save(f);
      f<<density<<density_add<<density_underwater<<density_underwater_add<<scale_color<<scale_normal<<scale_bump
       <<reflect_world<<refract<<refract_reflection<<refract_underwater<<wave_scale
       <<fresnel_pow<<fresnel_rough<<fresnel_color<<color_underwater0<<color_underwater1
       <<density_time<<density_underwater_time<<scale_color_time<<scale_normal_time<<scale_bump_time
       <<reflect_world_time<<refract_time<<refract_reflection_time<<refract_underwater_time<<wave_scale_time
       <<fresnel_pow_time<<fresnel_rough_time<<fresnel_color_time<<color_underwater_time;
      return f.ok();
   }
   bool EditWaterMtrl::load(File &f)
   {
      switch(f.decUIntV())
      {
         case 1: if(::EditMaterial::load(f))
         {
            f>>density>>density_add>>density_underwater>>density_underwater_add>>scale_color>>scale_normal>>scale_bump
             >>reflect_world>>refract>>refract_reflection>>refract_underwater>>wave_scale
             >>fresnel_pow>>fresnel_rough>>fresnel_color>>color_underwater0>>color_underwater1
             >>density_time>>density_underwater_time>>scale_color_time>>scale_normal_time>>scale_bump_time
             >>reflect_world_time>>refract_time>>refract_reflection_time>>refract_underwater_time>>wave_scale_time
             >>fresnel_pow_time>>fresnel_rough_time>>fresnel_color_time>>color_underwater_time;
            if(f.ok())return true;
         }break;

         case 0: reset(); return f.ok(); // empty
      }
      reset(); return false;
   }
   bool EditWaterMtrl::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
/******************************************************************************/
