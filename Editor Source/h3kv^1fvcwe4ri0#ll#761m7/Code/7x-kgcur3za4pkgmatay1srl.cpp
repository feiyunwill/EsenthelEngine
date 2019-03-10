/******************************************************************************/
class EditMaterial
{
   bool               flip_normal_y=false, cull=true, high_quality_ios=false;
   byte               downsize_tex_mobile=0;
   MATERIAL_TECHNIQUE tech=MTECH_DEFAULT;
   Vec4               color(1, 1, 1, 1);
   Vec                ambient(0, 0, 0);
   flt                specular=0, sss=0, glow=0, rough=0, bump=0,
                      tex_scale=1, det_scale=4, det_power=0.3, reflection=0.2;
   UID                base_0_tex=UIDZero, base_1_tex=UIDZero, detail_tex=UIDZero, macro_tex=UIDZero, reflection_tex=UIDZero, light_tex=UIDZero;
   Str                color_map, alpha_map, bump_map, normal_map, specular_map, glow_map,
                      detail_color, detail_bump, detail_normal,
                      macro_map,
                      reflection_map,
                      light_map;
   TimeStamp          flip_normal_y_time, high_quality_ios_time,
                      color_map_time, alpha_map_time, bump_map_time, normal_map_time, specular_map_time, glow_map_time,
                      detail_map_time, macro_map_time, reflection_map_time, light_map_time,
                      cull_time, tech_time, downsize_tex_mobile_time,
                      color_time, ambient_time, spec_time, sss_time, rough_bump_time, glow_time, tex_scale_time, detail_time, reflection_time;

   // get
   bool hasBumpMap   ()C {return   bump_map.is() /*|| bump_from_color && color_map.is()*/;}
   bool hasNormalMap ()C {return normal_map.is() || hasBumpMap();}
   bool hasDetailMap ()C {return detail_color.is() || detail_bump.is() || detail_normal.is();}
   bool hasBase1Tex  ()C {return hasNormalMap() || specular_map.is() || glow_map.is();}
   uint baseTex      ()C {return (color_map.is() ? BT_COLOR : 0)|(alpha_map.is() ? BT_ALPHA : 0)|(hasBumpMap() ? BT_BUMP : 0)|(hasNormalMap() ? BT_NORMAL : 0)|(specular_map.is() ? BT_SPECULAR : 0)|(glow_map.is() ? BT_GLOW : 0);}
   bool usesTexAlpha ()C {return tech!=MTECH_DEFAULT && (color_map.is() || alpha_map.is());}
   bool usesTexBump  ()C {return (bump     >EPS_MATERIAL_BUMP || 1) && hasBumpMap  ();} // always keep bump map because it can be used for multi-material per-pixel blending
   bool usesTexNormal()C {return  rough    >EPS_COL                 && hasNormalMap();}
   bool usesTexGlow  ()C {return  glow     >EPS_COL                 && glow_map.is ();}
   bool usesTexDetail()C {return  det_power>EPS_COL                 && hasDetailMap();}
   bool wantTanBin   ()C
   {
      return usesTexBump  ()
          || usesTexNormal()
          || usesTexDetail();
   }
   

   bool equal(C EditMaterial &src)C
   {
      return flip_normal_y_time==src.flip_normal_y_time && high_quality_ios_time==src.high_quality_ios_time
      && color_map_time==src.color_map_time && alpha_map_time==src.alpha_map_time && bump_map_time==src.bump_map_time && normal_map_time==src.normal_map_time && specular_map_time==src.specular_map_time && glow_map_time==src.glow_map_time
      && detail_map_time==src.detail_map_time && macro_map_time==src.macro_map_time && reflection_map_time==src.reflection_map_time && light_map_time==src.light_map_time
      && cull_time==src.cull_time && tech_time==src.tech_time && downsize_tex_mobile_time==src.downsize_tex_mobile_time
      && color_time==src.color_time && ambient_time==src.ambient_time && spec_time==src.spec_time && sss_time==src.sss_time && rough_bump_time==src.rough_bump_time
      && glow_time==src.glow_time && tex_scale_time==src.tex_scale_time && detail_time==src.detail_time && reflection_time==src.reflection_time;
   }
   bool newer(C EditMaterial &src)C
   {
      return flip_normal_y_time>src.flip_normal_y_time || high_quality_ios_time>src.high_quality_ios_time
      || color_map_time>src.color_map_time || alpha_map_time>src.alpha_map_time || bump_map_time>src.bump_map_time || normal_map_time>src.normal_map_time || specular_map_time>src.specular_map_time || glow_map_time>src.glow_map_time
      || detail_map_time>src.detail_map_time || macro_map_time>src.macro_map_time || reflection_map_time>src.reflection_map_time || light_map_time>src.light_map_time
      || cull_time>src.cull_time || tech_time>src.tech_time || downsize_tex_mobile_time>src.downsize_tex_mobile_time
      || color_time>src.color_time || ambient_time>src.ambient_time || spec_time>src.spec_time || sss_time>src.sss_time || rough_bump_time>src.rough_bump_time
      || glow_time>src.glow_time || tex_scale_time>src.tex_scale_time || detail_time>src.detail_time || reflection_time>src.reflection_time;
   }

   // operations
   void reset() {T=EditMaterial();}
   void resetAlpha()
   {
      switch(tech)
      {
         case MTECH_ALPHA_TEST:
         case MTECH_GRASS     :
         case MTECH_LEAF      : color.w=0.5; break;

         default: color.w=1; break;
      }
      color_time.getUTC();
   }
   void separateNormalMap(C TimeStamp &time=TimeStamp().getUTC())
   {
      if(!normal_map.is() && hasNormalMap()) // if normal map is not specified, but is created from some other map
      {
                                               normal_map="<bump>"; // set normal map from bump map
         if(!ForcesMono(bump_map))SetTransform(normal_map, "grey"); // force grey scale, in case 'bump_map' may be RGB
                                               normal_map_time=time;
      }
   }
   void separateBaseTexs(C Project &proj, C TimeStamp &time=TimeStamp().getUTC())
   {
      if(!alpha_map.is() && color_map.is() && hasBase1Tex()) // if alpha map not specified, but may come from color map, and will go to Base1 texture, #MaterialTextureChannelOrder
      {
         Image color; if(proj.loadImages(color, color_map))if(HasAlpha(color)) // if color has alpha
         {
                         alpha_map="<color>"; // set alpha map from color map
            SetTransform(alpha_map, "channel", "a");
                         alpha_map_time=time;
         }
      }
      separateNormalMap(time);
   }
   void cleanupMaps()
   { // no need to adjust time because this is called after maps have been changed
      if( alpha_map=="<color>") alpha_map.clear();
      if(normal_map=="<bump>" )normal_map.clear();
   }

   void newData()
   {
      flip_normal_y_time++; high_quality_ios_time++;
      color_map_time++; alpha_map_time++; bump_map_time++; normal_map_time++; specular_map_time++; glow_map_time++;
      detail_map_time++; macro_map_time++; reflection_map_time++; light_map_time++;
      cull_time++; tech_time++; downsize_tex_mobile_time++;
      color_time++; ambient_time++; spec_time++; sss_time++; rough_bump_time++; glow_time++; tex_scale_time++; detail_time++; reflection_time++;
   }
   void create(C Material &src, C TimeStamp &time=TimeStamp().getUTC())
   {
      cull=src.cull; cull_time=time;
      tech=src.technique; tech_time=time;
      color=src.color; color_time=time;
      ambient=src.ambient; ambient_time=time;
      specular=src.specular; spec_time=time;
      sss=src.sss; sss_time=time;
      glow=src.glow; glow_time=time;
      rough=src.rough; rough_bump_time=time;
      bump=src.bump; rough_bump_time=time;
      tex_scale=src.tex_scale; tex_scale_time=time;
      det_scale=src.det_scale; detail_time=time;
      det_power=src.det_power; detail_time=time;
      reflection=src.reflect; reflection_time=time;
          base_0_tex=src.        base_0.id();
          base_1_tex=src.        base_1.id();
          detail_tex=src.    detail_map.id();
           macro_tex=src.     macro_map.id();
      reflection_tex=src.reflection_map.id();
           light_tex=src.     light_map.id();
   }
   void copyTo(Material &dest, C Project &proj)C
   {
      dest.cull=cull;
      dest.technique=tech;
      dest.color=color;
      dest.ambient=ambient;
      dest.specular=specular;
      dest.sss=sss;
      dest.glow=glow;
      dest.rough=rough;
      dest.bump=bump;
      dest.tex_scale=tex_scale;
      dest.det_scale=det_scale;
      dest.det_power=det_power;
      dest.reflect=reflection;
      dest.      base_0  =proj.texPath(    base_0_tex);
      dest.      base_1  =proj.texPath(    base_1_tex);
      dest.    detail_map=proj.texPath(    detail_tex);
      dest.     macro_map=proj.texPath(     macro_tex);
      dest.reflection_map=proj.texPath(reflection_tex);
      dest.     light_map=proj.texPath(     light_tex);
      dest.validate();
   }
   void copyTo(Edit.Material &dest)C
   {
      dest.technique=tech;
      dest.downsize_tex_mobile=downsize_tex_mobile;
      dest.cull=cull;
      dest.flip_normal_y=flip_normal_y;
      dest.high_quality_ios=high_quality_ios;
      dest.color=color;
      dest.ambient=ambient;
      dest.specular=specular;
      dest.glow=glow;
      dest.roughness=rough;
      dest.bump=bump;
      dest.reflection=reflection;
      dest.     color_map=Edit.FileParams.Decode(color_map);
      dest.     alpha_map=Edit.FileParams.Decode(alpha_map);
      dest.      bump_map=Edit.FileParams.Decode(bump_map);
      dest.    normal_map=Edit.FileParams.Decode(normal_map);
      dest.  specular_map=Edit.FileParams.Decode(specular_map);
      dest.      glow_map=Edit.FileParams.Decode(glow_map);
      dest.reflection_map=Edit.FileParams.Decode(reflection_map);
      dest. detail_color =Edit.FileParams.Decode(detail_color);
      dest. detail_bump  =Edit.FileParams.Decode(detail_bump);
      dest. detail_normal=Edit.FileParams.Decode(detail_normal);
      dest.  macro_map   =Edit.FileParams.Decode(macro_map);
      dest.  light_map   =Edit.FileParams.Decode(light_map);
   }
   enum
   {
      CHANGED_PARAM=1<<0,
      CHANGED_FNY  =1<<1,
      CHANGED_BASE =1<<2,
      CHANGED_REFL =1<<3,
      CHANGED_DET  =1<<4,
      CHANGED_MACRO=1<<5,
      CHANGED_LIGHT=1<<6,
   }
   uint sync(C Edit.Material &src)
   {
      TimeStamp time; time.getUTC();
      uint changed=0;

      changed|=CHANGED_PARAM*SyncByValue(               tech_time, time, tech               , src.technique          );
      changed|=CHANGED_PARAM*SyncByValue(               cull_time, time, cull               , src.cull               );
      changed|=              SyncByValue(      flip_normal_y_time, time, flip_normal_y      , src.flip_normal_y      )*(CHANGED_PARAM|CHANGED_BASE|CHANGED_FNY); // set CHANGED_BASE too because this should trigger reloading base textures
      changed|=CHANGED_PARAM*SyncByValue(   high_quality_ios_time, time, high_quality_ios   , src.high_quality_ios   );
      changed|=CHANGED_PARAM*SyncByValue(downsize_tex_mobile_time, time, downsize_tex_mobile, src.downsize_tex_mobile);

      changed|=CHANGED_PARAM*SyncByValueEqual(     color_time, time,      color, src.     color);
      changed|=CHANGED_PARAM*SyncByValueEqual(   ambient_time, time,    ambient, src.   ambient);
      changed|=CHANGED_PARAM*SyncByValueEqual(      spec_time, time,   specular, src.  specular);
      changed|=CHANGED_PARAM*SyncByValueEqual(      glow_time, time,       glow, src.      glow);
      changed|=CHANGED_PARAM*SyncByValueEqual(rough_bump_time, time,      rough, src. roughness);
      changed|=CHANGED_PARAM*SyncByValueEqual(rough_bump_time, time,       bump, src.      bump);
      changed|=CHANGED_PARAM*SyncByValueEqual(reflection_time, time, reflection, src.reflection);

      changed|=CHANGED_BASE *SyncByValue(     color_map_time, time,      color_map, Edit.FileParams.Encode(ConstCast(src.     color_map)));
      changed|=CHANGED_BASE *SyncByValue(     alpha_map_time, time,      alpha_map, Edit.FileParams.Encode(ConstCast(src.     alpha_map)));
      changed|=CHANGED_BASE *SyncByValue(      bump_map_time, time,       bump_map, Edit.FileParams.Encode(ConstCast(src.      bump_map)));
      changed|=CHANGED_BASE *SyncByValue(    normal_map_time, time,     normal_map, Edit.FileParams.Encode(ConstCast(src.    normal_map)));
      changed|=CHANGED_BASE *SyncByValue(  specular_map_time, time,   specular_map, Edit.FileParams.Encode(ConstCast(src.  specular_map)));
      changed|=CHANGED_BASE *SyncByValue(      glow_map_time, time,       glow_map, Edit.FileParams.Encode(ConstCast(src.      glow_map)));
      changed|=CHANGED_REFL *SyncByValue(reflection_map_time, time, reflection_map, Edit.FileParams.Encode(ConstCast(src.reflection_map)));
      changed|=CHANGED_DET  *SyncByValue(    detail_map_time, time,  detail_color , Edit.FileParams.Encode(ConstCast(src. detail_color )));
      changed|=CHANGED_DET  *SyncByValue(    detail_map_time, time,  detail_bump  , Edit.FileParams.Encode(ConstCast(src. detail_bump  )));
      changed|=CHANGED_DET  *SyncByValue(    detail_map_time, time,  detail_normal, Edit.FileParams.Encode(ConstCast(src. detail_normal)));
      changed|=CHANGED_MACRO*SyncByValue(     macro_map_time, time,      macro_map, Edit.FileParams.Encode(ConstCast(src.     macro_map)));
      changed|=CHANGED_LIGHT*SyncByValue(     light_map_time, time,      light_map, Edit.FileParams.Encode(ConstCast(src.     light_map)));

      return changed;
   }
   uint sync(C EditMaterial &src)
   {
      uint changed=0;

      changed|=Sync(flip_normal_y_time, src.flip_normal_y_time, flip_normal_y, src.flip_normal_y)*(CHANGED_PARAM|CHANGED_BASE|CHANGED_FNY); // set CHANGED_BASE too because this should trigger reloading base textures

      changed|=Sync(   color_map_time, src.   color_map_time,    color_map, src.   color_map)*CHANGED_BASE;
      changed|=Sync(   alpha_map_time, src.   alpha_map_time,    alpha_map, src.   alpha_map)*CHANGED_BASE;
      changed|=Sync(    bump_map_time, src.    bump_map_time,     bump_map, src.    bump_map)*CHANGED_BASE;
      changed|=Sync(  normal_map_time, src.  normal_map_time,   normal_map, src.  normal_map)*CHANGED_BASE;
      changed|=Sync(specular_map_time, src.specular_map_time, specular_map, src.specular_map)*CHANGED_BASE;
      changed|=Sync(    glow_map_time, src.    glow_map_time,     glow_map, src.    glow_map)*CHANGED_BASE;

      if(changed&CHANGED_BASE)
      {
         base_0_tex=src.base_0_tex;
         base_1_tex=src.base_1_tex;
      }
      if(Sync(reflection_map_time, src.reflection_map_time))
      {
         changed|=CHANGED_REFL;
         reflection_map=src.reflection_map;
         reflection_tex=src.reflection_tex;
      }
      if(Sync(detail_map_time, src.detail_map_time))
      {
         changed|=CHANGED_DET;
         detail_color =src.detail_color;
         detail_bump  =src.detail_bump;
         detail_normal=src.detail_normal;
         detail_tex   =src.detail_tex;
      }
      if(Sync(macro_map_time, src.macro_map_time))
      {
         changed|=CHANGED_MACRO;
         macro_map=src.macro_map;
         macro_tex=src.macro_tex;
      }
      if(Sync(light_map_time, src.light_map_time))
      {
         changed|=CHANGED_LIGHT;
         light_map=src.light_map;
         light_tex=src.light_tex;
      }
      changed|=Sync(               cull_time, src.               cull_time,                cull, src.               cull)*CHANGED_PARAM;
      changed|=Sync(               tech_time, src.               tech_time,                tech, src.               tech)*CHANGED_PARAM;
      changed|=Sync(   high_quality_ios_time, src.   high_quality_ios_time,    high_quality_ios, src.   high_quality_ios)*CHANGED_PARAM;
      changed|=Sync(downsize_tex_mobile_time, src.downsize_tex_mobile_time, downsize_tex_mobile, src.downsize_tex_mobile)*CHANGED_PARAM;

      changed|=Sync(     color_time, src.     color_time, color     , src.color     )*CHANGED_PARAM;
      changed|=Sync(   ambient_time, src.   ambient_time, ambient   , src.ambient   )*CHANGED_PARAM;
      changed|=Sync(      spec_time, src.      spec_time, specular  , src.specular  )*CHANGED_PARAM;
      changed|=Sync(       sss_time, src.       sss_time, sss       , src.sss       )*CHANGED_PARAM;
      changed|=Sync(      glow_time, src.      glow_time, glow      , src.glow      )*CHANGED_PARAM;
      changed|=Sync( tex_scale_time, src. tex_scale_time, tex_scale , src.tex_scale )*CHANGED_PARAM;
      changed|=Sync(reflection_time, src.reflection_time, reflection, src.reflection)*CHANGED_PARAM;
      if(Sync(rough_bump_time, src.rough_bump_time))
      {
         changed|=CHANGED_PARAM;
         rough=src.rough;
         bump =src.bump ;
      }
      if(Sync(detail_time, src.detail_time))
      {
         changed|=CHANGED_PARAM;
         det_scale=src.det_scale;
         det_power=src.det_power;
      }
      return changed;
   }
   uint undo(C EditMaterial &src)
   {
      uint changed=0;

      changed|=Undo(flip_normal_y_time, src.flip_normal_y_time, flip_normal_y, src.flip_normal_y)*(CHANGED_PARAM|CHANGED_BASE|CHANGED_FNY); // set CHANGED_BASE too because this should trigger reloading base textures

      changed|=Undo(   color_map_time, src.   color_map_time,    color_map, src.   color_map)*CHANGED_BASE;
      changed|=Undo(   alpha_map_time, src.   alpha_map_time,    alpha_map, src.   alpha_map)*CHANGED_BASE;
      changed|=Undo(    bump_map_time, src.    bump_map_time,     bump_map, src.    bump_map)*CHANGED_BASE;
      changed|=Undo(  normal_map_time, src.  normal_map_time,   normal_map, src.  normal_map)*CHANGED_BASE;
      changed|=Undo(specular_map_time, src.specular_map_time, specular_map, src.specular_map)*CHANGED_BASE;
      changed|=Undo(    glow_map_time, src.    glow_map_time,     glow_map, src.    glow_map)*CHANGED_BASE;

      if(changed&CHANGED_BASE)
      {
         base_0_tex=src.base_0_tex;
         base_1_tex=src.base_1_tex;
      }
      if(Undo(reflection_map_time, src.reflection_map_time))
      {
         changed|=CHANGED_REFL;
         reflection_map=src.reflection_map;
         reflection_tex=src.reflection_tex;
      }
      if(Undo(detail_map_time, src.detail_map_time))
      {
         changed|=CHANGED_DET;
         detail_color =src.detail_color;
         detail_bump  =src.detail_bump;
         detail_normal=src.detail_normal;
         detail_tex   =src.detail_tex;
      }
      if(Undo(macro_map_time, src.macro_map_time))
      {
         changed|=CHANGED_MACRO;
         macro_map=src.macro_map;
         macro_tex=src.macro_tex;
      }
      if(Undo(light_map_time, src.light_map_time))
      {
         changed|=CHANGED_LIGHT;
         light_map=src.light_map;
         light_tex=src.light_tex;
      }
      changed|=Undo(               cull_time, src.               cull_time,                cull, src.               cull)*CHANGED_PARAM;
      changed|=Undo(               tech_time, src.               tech_time,                tech, src.               tech)*CHANGED_PARAM;
      changed|=Undo(   high_quality_ios_time, src.   high_quality_ios_time,    high_quality_ios, src.   high_quality_ios)*CHANGED_PARAM;
      changed|=Undo(downsize_tex_mobile_time, src.downsize_tex_mobile_time, downsize_tex_mobile, src.downsize_tex_mobile)*CHANGED_PARAM;

      changed|=Undo(     color_time, src.     color_time, color     , src.color     )*CHANGED_PARAM;
      changed|=Undo(   ambient_time, src.   ambient_time, ambient   , src.ambient   )*CHANGED_PARAM;
      changed|=Undo(      spec_time, src.      spec_time, specular  , src.specular  )*CHANGED_PARAM;
      changed|=Undo(       sss_time, src.       sss_time, sss       , src.sss       )*CHANGED_PARAM;
      changed|=Undo(      glow_time, src.      glow_time, glow      , src.glow      )*CHANGED_PARAM;
      changed|=Undo( tex_scale_time, src. tex_scale_time, tex_scale , src.tex_scale )*CHANGED_PARAM;
      changed|=Undo(reflection_time, src.reflection_time, reflection, src.reflection)*CHANGED_PARAM;
      if(Undo(rough_bump_time, src.rough_bump_time))
      {
         changed|=CHANGED_PARAM;
         rough=src.rough;
         bump =src.bump ;
      }
      if(Undo(detail_time, src.detail_time))
      {
         changed|=CHANGED_PARAM;
         det_scale=src.det_scale;
         det_power=src.det_power;
      }
      return changed;
   }

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(10);
      f<<flip_normal_y<<cull<<high_quality_ios<<tech<<downsize_tex_mobile;
      f<<color<<ambient<<specular<<sss<<glow<<rough<<bump<<tex_scale<<det_scale<<det_power<<reflection;
      f<<base_0_tex<<base_1_tex<<detail_tex<<macro_tex<<reflection_tex<<light_tex;

      f<<color_map<<alpha_map<<bump_map<<normal_map<<specular_map<<glow_map
       <<detail_color<<detail_bump<<detail_normal
       <<macro_map
       <<reflection_map
       <<light_map;

      f<<flip_normal_y_time<<high_quality_ios_time;
      f<<color_map_time<<alpha_map_time<<bump_map_time<<normal_map_time<<specular_map_time<<glow_map_time;
      f<<detail_map_time<<macro_map_time<<reflection_map_time<<light_map_time;
      f<<cull_time<<tech_time<<downsize_tex_mobile_time;
      f<<color_time<<ambient_time<<spec_time<<sss_time<<rough_bump_time<<glow_time<<tex_scale_time<<detail_time<<reflection_time;
      return f.ok();
   }
   bool load(File &f)
   {
      bool bump_from_color=false; byte mip_map_blur; TimeStamp mip_map_blur_time, bump_from_color_time;
      reset(); switch(f.decUIntV())
      {
         case 10:
         {
            f>>flip_normal_y>>cull>>high_quality_ios>>tech>>downsize_tex_mobile;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>specular_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal
             >>macro_map
             >>reflection_map
             >>light_map;

            f>>flip_normal_y_time>>high_quality_ios_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>downsize_tex_mobile_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 9:
         {
            f>>flip_normal_y>>cull>>high_quality_ios>>tech>>downsize_tex_mobile;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, specular_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, reflection_map);
            GetStr2(f, light_map);

            f>>flip_normal_y_time>>high_quality_ios_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>downsize_tex_mobile_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 8:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>high_quality_ios>>tech>>downsize_tex_mobile;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, specular_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, reflection_map);
            GetStr2(f, light_map);

            f>>bump_from_color_time>>flip_normal_y_time>>high_quality_ios_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>downsize_tex_mobile_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 7:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>high_quality_ios>>tech>>downsize_tex_mobile>>mip_map_blur;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, specular_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, reflection_map);
            GetStr2(f, light_map);

            f>>bump_from_color_time>>flip_normal_y_time>>high_quality_ios_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>downsize_tex_mobile_time>>mip_map_blur_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 6:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech>>downsize_tex_mobile>>mip_map_blur;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, specular_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, reflection_map);
            GetStr2(f, light_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>downsize_tex_mobile_time>>mip_map_blur_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 5:
         {
            byte max_tex_size; f>>bump_from_color>>flip_normal_y>>cull>>tech>>max_tex_size>>mip_map_blur; downsize_tex_mobile=(max_tex_size>=1 && max_tex_size<=10);
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, specular_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, reflection_map);
            GetStr2(f, light_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>downsize_tex_mobile_time>>mip_map_blur_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 4:
         {
            byte max_tex_size; f>>bump_from_color>>flip_normal_y>>cull>>tech>>max_tex_size>>mip_map_blur; downsize_tex_mobile=(max_tex_size>=1 && max_tex_size<=10);
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, specular_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, reflection_map);
            GetStr(f, light_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>downsize_tex_mobile_time>>mip_map_blur_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 3:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech>>mip_map_blur;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, specular_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, reflection_map);
            GetStr(f, light_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time>>mip_map_blur_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 2:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, specular_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, reflection_map);
            GetStr(f, light_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time>>light_map_time;
            f>>cull_time>>tech_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 1:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech;
            f>>color>>ambient>>specular>>sss>>glow>>rough>>bump>>tex_scale>>det_scale>>det_power>>reflection;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, specular_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, reflection_map);
            light_map.clear();

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>specular_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>reflection_map_time;
            f>>cull_time>>tech_time;
            f>>color_time>>ambient_time>>spec_time>>sss_time>>rough_bump_time>>glow_time>>tex_scale_time>>detail_time>>reflection_time;
         }break;

         case 0: break; // empty, this requires 'reset' to be called before

         default: goto error;
      }
      if(f.ok())
      {
         if(bump_from_color && !bump_map.is() && color_map.is()){bump_map=BumpFromColTransform(color_map, 3); if(!bump_map_time.is())bump_map_time++;}
         return true;
      }
   error:
      reset(); return false;
   }
   bool load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
}
/******************************************************************************/
