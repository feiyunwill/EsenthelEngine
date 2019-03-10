/******************************************************************************/
/******************************************************************************/
class LodRegion : Region
{
   Memx<LodView>  lods;
   Tabs           edit_dist;
   Text           edit_dist_text;
   TextWhite      ts;
   Memx<Property> props;
   Region         props_region;
   Menu           menu;

   static void Erase  (LodView   &lod);
   static void Disable(LodView   &lod);
   static void Disable(LodRegion &lr );

   static Str  Dist(C LodRegion &lr          );
   static void Dist(  LodRegion &lr, C Str &t); // preserve sign because it specifies if LOD is disabled or not

   static void EditDistChanged(LodRegion &lr);
   static void EditDistToggle (LodRegion &lr);

   LodRegion& create();
   void resize();
   void toGui(); 

   virtual void update(C GuiPC &gpc)override;

   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos);
   void drag(Memc<UID> &elms, GuiObj* &focus_obj, C Vec2 &screen_pos);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
