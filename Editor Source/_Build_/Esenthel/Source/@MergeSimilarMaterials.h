/******************************************************************************/
/******************************************************************************/
class MergeSimilarMaterials : PropWin
{
   class Mtrl
   {
      Str  name, color_name;
      UID  elm_id, base_0_tex, base_1_tex, detail_tex, macro_tex, reflect_tex, light_tex;
      bool cull;
      MATERIAL_TECHNIQUE tech;
      Vec4 col;
      flt  bump, spec, glow, reflect, uv_scale;

      bool similar(C Mtrl &m)C;
      Mtrl& set(C UID &elm_id, C EditMaterial &m);
   };
   static int CompareMtrl(C Mtrl &a, C Mtrl &b);   // compare reversed because we're going to process materials in reversed order later
   static void Detect(MergeSimilarMaterials &msm);
   void display(C MemPtr<UID> &elm_ids);
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
   static void Merge(MergeSimilarMaterials &msm);

   class Data
   {
      UID id;

      static Str AsText(C Data &data);

public:
   Data();
   };
   Memc<IDReplace> replace;
   Memc<Data>      data;
   List<Data>      list;
   Region          region;

   bool     name, color_name, color_is, color_tex, normal_tex, detail_tex, macro_tex, reflect_tex, light_tex, tech, cull;
   flt      color_value, bump_value, spec_value, glow_value, reflect_value, uv_scale;
   CheckBox color_value_on, bump_value_on, spec_value_on, glow_value_on, reflect_value_on, uv_scale_on;
   Button   detect, merge;
   Text     detected;

   void clearProj();
   void create();
   virtual void update(C GuiPC &gpc)override;

public:
   MergeSimilarMaterials();
};
/******************************************************************************/
/******************************************************************************/
extern MergeSimilarMaterials MSM;
/******************************************************************************/
