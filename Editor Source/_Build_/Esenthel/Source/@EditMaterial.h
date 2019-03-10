/******************************************************************************/
/******************************************************************************/
class EditMaterial
{
   bool               flip_normal_y, cull, high_quality_ios;
   byte               downsize_tex_mobile;
   MATERIAL_TECHNIQUE tech;
   Vec4               color;
   Vec                ambient;
   flt                specular, sss, glow, rough, bump,
                      tex_scale, det_scale, det_power, reflection;
   UID                base_0_tex, base_1_tex, detail_tex, macro_tex, reflection_tex, light_tex;
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
   bool hasBumpMap   ()C;
   bool hasNormalMap ()C;
   bool hasDetailMap ()C;
   bool hasBase1Tex  ()C;
   uint baseTex      ()C;
   bool usesTexAlpha ()C;
   bool usesTexBump  ()C; // always keep bump map because it can be used for multi-material per-pixel blending
   bool usesTexNormal()C;
   bool usesTexGlow  ()C;
   bool usesTexDetail()C;
   bool wantTanBin   ()C;
   

   bool equal(C EditMaterial &src)C;
   bool newer(C EditMaterial &src)C;

   // operations
   void reset();     
   void resetAlpha();
   void separateNormalMap(C TimeStamp &time=TimeStamp().getUTC());
   void separateBaseTexs(C Project &proj, C TimeStamp &time=TimeStamp().getUTC());
   void cleanupMaps();

   void newData();
   void create(C Material &src, C TimeStamp &time=TimeStamp().getUTC());
   void copyTo(Material &dest, C Project &proj)C;
   void copyTo(Edit::Material &dest)C;
   enum
   {
      CHANGED_PARAM=1<<0,
      CHANGED_FNY  =1<<1,
      CHANGED_BASE =1<<2,
      CHANGED_REFL =1<<3,
      CHANGED_DET  =1<<4,
      CHANGED_MACRO=1<<5,
      CHANGED_LIGHT=1<<6,
   };
   uint sync(C Edit::Material &src);
   uint sync(C EditMaterial &src);
   uint undo(C EditMaterial &src);

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   EditMaterial();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
