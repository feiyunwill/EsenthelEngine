/******************************************************************************/
/******************************************************************************/
class MtrlBrushClass : Window
{
   int         sel;
   GuiImage    image  [MtrlBrushSlots];
   MaterialPtr mtrl   [MtrlBrushSlots];
   UID         mtrl_id[MtrlBrushSlots];
   Button      scale, hole, max1, max2;
   MaterialPtr scale_material;
   UID         scale_material_id;

   MtrlBrushClass();

   static void PushedScale(MtrlBrushClass &mb);
   static void PushedHole (MtrlBrushClass &mb);
   static void PushedMax1 (MtrlBrushClass &mb);
   static void PushedMax2 (MtrlBrushClass &mb);

   MtrlBrushClass& create();
   void validateIcons();  

   MaterialPtr getMaterial();                    
   void        setMaterial(int i, C UID&mtrl_id);
   void setColors();
   void set(int i, SET_MODE mode=SET_DEFAULT);
   void erasing(C UID &elm_id);
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);

   static void UpdateMaterial(Cell<Area> &cell, MaterialPtr &material, int thread_index);

   class MtrlBlend
   {
      MaterialPtr mtrl;
      flt         blend;
      
      static int Compare(C MtrlBlend &a, C MtrlBlend &b); // compare in reverse order to list those with highest blend first
   };
   static void ReduceMaterial(Cell<Area> &cell, ptr user, int thread_index);

   static void DragMtrl(ptr slot_index, GuiObj *go, C Vec2 &screen_pos);

   void update();
};
/******************************************************************************/
/******************************************************************************/
extern MtrlBrushClass MtrlBrush;
/******************************************************************************/
