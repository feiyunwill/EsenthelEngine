/******************************************************************************/
/******************************************************************************/
class EditWaterMtrl : EditMaterial
{
   flt density, density_add, density_underwater, density_underwater_add, scale_color, scale_normal, scale_bump,
       reflect_world, refract, refract_reflection, refract_underwater, wave_scale,
       fresnel_pow, fresnel_rough;
   Vec fresnel_color    ,
       color_underwater0,
       color_underwater1;

   TimeStamp density_time, density_underwater_time, scale_color_time, scale_normal_time, scale_bump_time,
             reflect_world_time, refract_time, refract_reflection_time, refract_underwater_time, wave_scale_time,
             fresnel_pow_time, fresnel_rough_time, fresnel_color_time, color_underwater_time;

   EditWaterMtrl();

   // get
   bool usesTexBump()C;

   bool equal(C EditWaterMtrl &src)C;
   bool newer(C EditWaterMtrl &src)C;

   // operations
   void reset();  
   void newData();
   void create(C WaterMtrl &src, C TimeStamp &time=TimeStamp().getUTC());
   void copyTo(WaterMtrl &dest, C Project &proj)C;
   uint sync(C EditWaterMtrl &src);
   uint undo(C EditWaterMtrl &src);

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
