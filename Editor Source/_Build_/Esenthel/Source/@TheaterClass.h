/******************************************************************************/
/******************************************************************************/
class TheaterClass : Region
{
   enum SHOW
   {
      ALL,
      PUBLISHABLE,
      NON_PUBLISHABLE,
      INVALID,
   };
   static cchar8 *t_show[]
;
   class Options : PropWin
   {
      bool      horizontal, rotate, auto_rot_flat_objects, scale_fit, center, draw_axis, axis_on_top, draw_box, light_dir;
      int       rows;
      flt       item_size, item_3d_scale, axis_size, time_angle, rot_speed, var_speed;
      Vec2      angle;
      UID       env_id;
      SHOW      show;
      Property *p_rows, *p_size, *p_scale, *p_angle;

      static void Horizontal(  Options &options, C Str &text);
      static void Rows      (  Options &options, C Str &text);
      static void Size      (  Options &options, C Str &text);
      static void Show      (  Options &options, C Str &text);
      static void FOV       (  Options &options, C Str &text);
      static Str  FOV       (C Options &options             );
      static void Angle     (  Options &options, C Str &text);
      static Str  Angle     (C Options &options             );
      static void Env       (  Options &options, C Str &text);
      static Str  Env       (C Options &options             );
      static void EditMode  (  Options &options, C Str &text);
      static Str  EditMode  (C Options &options             );

      Options& create();
      void  rowsDelta (int delta );
      void  sizeFactor(flt factor);
      void scaleFactor(flt factor);

public:
   Options();
   };

   class ListElm
   {
      UID id;

public:
   ListElm();
   };
   static ELM_TYPE modes[]
;
   enum OBJ_MODE
   {
      OBJ_ROT  ,
      OBJ_SCALE,
   };
   static cchar8 *t_obj_mode[]
;

   class List2 : List<ListElm>
   {
      virtual int screenToVis(C Vec2 &screen, C GuiPC *gpc=null)C override;
   };
   enum
   {
      REFRESH_SIZE=1<<0,
      REFRESH_DATA=1<<1,
   };

   byte          refresh;
   int           old_rows;
   Menu          menu;
   ViewportSkin  viewport;
   Memc<ListElm> data;
   List2         list;
   Tabs          mode, obj_mode, show_options;
   Options       options;
   Memc<UID>     transform_ids;
   int           transform_rotate_y;
   flt           transform_rotate_y_frac, transform_scale;
   Matrix        transform_matrix;

   static void ModeChanged(TheaterClass &theater);

   static void Mode0(TheaterClass &theater);
   static void Mode1(TheaterClass &theater);
   static void Mode2(TheaterClass &theater);

   static void ModeS0(TheaterClass &theater);
   static void ModeS1(TheaterClass &theater);

   static void ToggleAxis   (TheaterClass &theater);
   static void ToggleAxisTop(TheaterClass &theater);
   static void ToggleBox    (TheaterClass &theater);
   static void ToggleCenter (TheaterClass &theater);
   static void ToggleScale  (TheaterClass &theater);
   static void ToggleLight  (TheaterClass &theater);
   static void ToggleOptions(TheaterClass &theater);

   static void DragElmsStart(ProjectEx &proj);

   flt depth()C;

   Matrix getMatrix(C UID &elm_id, Box box, C Rect &rect);
   void draw(C UID &elm_id, C Mesh &mesh, C Rect &rect);
   bool litSel()C;                                    
   bool highlighted(int i)C;                     
   bool selected   (int i)C;                     

   // this is a list of meshes with preallocated room for variations which we can reuse during rendering, because changing materials and adding new variations while rendering is now illegal
   class MeshMtrls : Mesh
   {
      int variations;
      void        clean();                                 // skip setting shaders
      MeshMtrls& create();                                  // pre-allocate 32 variations so we can reuse them later
      bool    available()C;                                
      Mesh&         set(C MaterialPtr&mtrl, int&variation);

public:
   MeshMtrls();
   };
   Memc<MeshMtrls> mesh_mtrls;
   void cleanMeshMaterial();                          
   Mesh&  getMeshMaterial(C MaterialPtr&mtrl, int&variation);

   static void Render();
          void render();

   static void Draw(Viewport &viewport);
          void draw();
   void setVisibility(bool vis);
   void hideDo();
   void create();
   void moveAbove(GuiObj &go);
   void listElms(ElmNode &node, bool parent_contains_name=false);
   Vec2 offset()C;
   void offset(C Vec2 &offset);
   void refreshSize();       
   void refreshData();       
   void refreshNow();
   virtual void update(C GuiPC &gpc)override;
   void resize();

public:
   TheaterClass();
};
/******************************************************************************/
/******************************************************************************/
extern TheaterClass Theater;
/******************************************************************************/
