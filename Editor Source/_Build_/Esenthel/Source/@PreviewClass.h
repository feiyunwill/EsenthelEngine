/******************************************************************************/
/******************************************************************************/
class PreviewClass : Window // create as window so it will be above other windows in the editor
{
   ObjectPtr        obj;
   MeshPtr          mesh, body;
   MaterialPtr      mtrl;
   WaterMtrlPtr     water_mtrl;
   ImagePtr         image;
   ImageAtlasPtr    atlas;
   Skeleton        *skel;
   Animation       *anim;
   SkelAnim         skel_anim;
   AnimatedSkeleton anim_skel;
   Mesh             ball;
   EnvironmentPtr   env;
   ELM_TYPE         elm_type;
   Camera           cam;
   Video            video;
   Str              sound_info;
   Game::MiniMap     mini_map;
   UID              sound_id, video_id, mini_map_id;
   int              mesh_variation;
   flt              video_time;
   GuiObjs          objs;
   Tab              objs_container; // needed in order to display objects in correct order
   UID              objs_last_id;

   void create();
   static void Render();
          void render();
   void closeElm(C UID &elm_id);
   void elmChanged(C UID &elm_id);
   void clearProj();
   void drawBack(C Rect &rect);
   void draw(Elm &elm, C Rect &rect);

   void clipRect(Rect &rect);
   void drawAround(Elm &elm, C Rect &rect, flt y=Ms.pos().y);

   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
   virtual void    update(C GuiPC &gpc)override;  
   virtual void    draw  (C GuiPC &gpc)override;

public:
   PreviewClass();
};
/******************************************************************************/
/******************************************************************************/
extern PreviewClass Preview;
/******************************************************************************/
