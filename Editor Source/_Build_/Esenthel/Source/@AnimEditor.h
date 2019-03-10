/******************************************************************************/
/******************************************************************************/
class AnimEditor : Viewport4Region
{
   enum ANIM_OP
   {
      OP_ORN  ,
      OP_ORN2 ,
      OP_POS  ,
      OP_SCALE,
      OP_NUM  ,
   };
   enum TRACK_TYPE
   {
      TRACK_ORN,
      TRACK_POS,
      TRACK_SCALE,
      TRACK_NUM,
   };
   enum EVENT_OP
   {
      EVENT_MOVE,
      EVENT_RENAME,
      EVENT_NEW,
      EVENT_DEL,
   };
   static cchar8 *event_op_t[]
;
   static flt TimeEps();

   class Change : Edit::_Undo::Change
   {
      Animation anim;
      ElmAnim   anim_data;
      Skeleton  skel;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   class Track : GuiCustom
   {
      bool events;

      flt screenToTime(flt x)C;
      Track& create(bool events);
      static flt ElmPosX(C Rect &rect, flt time);
      static Rect ElmRect(C Rect &rect, flt time);
      static flt Slide(flt &time, flt dt, flt length);
      virtual void update(C GuiPC &gpc)override;
      static void DrawKey(flt time, C Rect &rect, flt y, flt r);
      static void DrawKeys(AnimKeys &keys, C Rect &rect, C Color &color, flt y, flt r, int row, bool lit=false);
      virtual void draw(C GuiPC &gpc)override;
   };

   class Preview : PropWin
   {
      ViewportSkin viewport;
      Button       split, apply_speed, edit, locate;
      Tabs         event_op;
      Track        track;
      bool         draw_bones, draw_slots, draw_axis, draw_plane;
      int          event_lit, event_sel;
      flt          time_speed, prop_max_x,
                   cam_yaw, cam_pitch, cam_zoom;
      Camera       cam;
      Property    *length, *event;
      
      static void Play  (  Preview &editor, C Str &t);
      static Str  Play  (C Preview &editor          );
      static void Loop  (  Preview &editor, C Str &t);
      static Str  Loop  (C Preview &editor          );
      static void Linear(  Preview &editor, C Str &t);
      static Str  Linear(C Preview &editor          );
      static void Target(  Preview &editor, C Str &t);
      static Str  Target(C Preview &editor          );
      static void Split (  Preview &editor          );
      static void Speed (  Preview &editor          );

      static void Render();
      static void Draw(Viewport &viewport);
             void draw();

      Animation* anim()C;
      void create();
      void toGui();
      void removedEvent(int index);

      virtual Preview& hide     (            )  override;
      virtual Rect     sizeLimit(            )C override;
      virtual Preview& rect     (C Rect &rect)  override;
      virtual void update(C GuiPC &gpc)override;

public:
   Preview();
   };

   class OptimizeAnim : PropWin
   {
      bool      refresh_needed, preview;
      dbl       angle_eps, pos_eps, scale_eps;
      Property *file_size, *optimized_size;
      Button    optimize;
      Animation anim;

      static void Changed(C Property &prop); 
      static void Optimize(OptimizeAnim &oa);

      void optimizeDo(Animation &anim);
      Animation* getAnim();
      void refresh();        
      OptimizeAnim& create();

public:
   OptimizeAnim();
   };

   class TimeRangeSpeed : PropWin
   {
      flt    start, end, speed;
      Button apply;
      Property *st, *e, *sp;

      static void Apply(TimeRangeSpeed &tsr);
      TimeRangeSpeed& create();
      void display();
   };

   static void Fullscreen(AnimEditor &editor);

   static void Render();
   static void Draw(Viewport &viewport);        
          void draw(Edit::Viewport4::View&view);

   UID               elm_id;
   Elm              *elm;
   bool              changed, fullscreen, copied_bone_pos_relative;
   flt               blend_range; // amount of seconds to use blend when operating on multiple keyframes (using Ctrl), use -1 to disable TODO: make this configurable via UI
   Animation        *anim;
   Skeleton         *skel;
   AnimatedSkeleton  anim_skel;
   SkelAnim          skel_anim;
   MeshPtr           mesh;
   Preview           preview;
   Track             track;
   Button            axis, draw_bones, draw_mesh, show_grid, undo, redo, locate, edit, force_play, start, end, prev_frame, next_frame,
                     loop, linear, root_from_body,
                     root_del_pos, root_del_pos_x, root_del_pos_y, root_del_pos_z,
                     root_del_rot, root_del_rot_x, root_del_rot_y, root_del_rot_z,
                     root_del_scale, root_2_keys, root_set_move, root_set_rot, reload;
   CheckBox          play;
   Memx<Property>    props, root_props;
   Tabs              op, settings;
   ComboBox          cmd;
   Region            settings_region;
   Text            t_settings, t_settings_root;
   TextWhite         ts;
   flt               anim_speed, key_time;
   int               lit_bone, sel_bone, bone_axis;
   Str8              sel_bone_name;
   Vec               orn_target, orn_perp, copied_bone_pos;
   OptimizeAnim      optimize_anim;
   TimeRangeSpeed    time_range_speed;
   Edit::Undo<Change> undos;   void undoVis();

   static cchar8 *transform_obj_dialog_id;

   static void PrevAnim       (AnimEditor &editor);
   static void NextAnim       (AnimEditor &editor);
   static void Mode1          (AnimEditor &editor);
   static void Mode2          (AnimEditor &editor);
   static void Mode3          (AnimEditor &editor);
   static void Mode4          (AnimEditor &editor);
   static void Play           (AnimEditor &editor);
   static void Identity       (AnimEditor &editor);
   static void Settings       (AnimEditor &editor);
   static void Start          (AnimEditor &editor);
   static void End            (AnimEditor &editor);
   static void PrevFrame      (AnimEditor &editor);
   static void NextFrame      (AnimEditor &editor);
   static void  DelFrame      (AnimEditor &editor);
   static void  DelFrames     (AnimEditor &editor);
   static void  DelFramesAtEnd(AnimEditor &editor);
   static void Optimize       (AnimEditor &editor);
   static void TimeRangeSp    (AnimEditor &editor);
   static void ReverseFrames  (AnimEditor &editor);
   static void RemMovement    (AnimEditor &editor);
   static void FreezeBone     (AnimEditor &editor);
   static void Mirror         (AnimEditor &editor);
   static void RotX           (AnimEditor &editor);
   static void RotY           (AnimEditor &editor);
   static void RotZ           (AnimEditor &editor);
   static void DrawBones      (AnimEditor &editor);
   static void DrawMesh       (AnimEditor &editor);
   static void Grid           (AnimEditor &editor);
   static void TransformObj   (AnimEditor &editor);
   static void TransformObjDo(AnimEditor &editor);

   static void Undo  (AnimEditor &editor);
   static void Redo  (AnimEditor &editor);
   static void Locate(AnimEditor &editor);
   static void Reload(AnimEditor &editor);

   static void Loop  (AnimEditor &editor);
   static void Linear(AnimEditor &editor);

   static void RootDelPos(AnimEditor &editor);
   static void RootDelRot(AnimEditor &editor);
   static void RootDel(AnimEditor &editor);
   static void RootDelPosX (AnimEditor &editor);
   static void RootDelPosY (AnimEditor &editor);
   static void RootDelPosZ (AnimEditor &editor);
   static void RootDelRotX (AnimEditor &editor);
   static void RootDelRotY (AnimEditor &editor);
   static void RootDelRotZ (AnimEditor &editor);
 //static void RootDelScale(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelScale"); FlagToggle(d.flag, ElmAnim.ROOT_DEL_SCALE); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d.flag&ElmAnim.ROOT_DEL_SCALE){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim.adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_SCALE     ); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   static void Root2Keys   (AnimEditor &editor);
   static void RootFromBody(AnimEditor &editor);
   static void RootFromBodyX(AnimEditor &editor);
   static void RootFromBodyZ(AnimEditor &editor);
   static void RootSetMove(AnimEditor &editor);
   static void RootSetRot(AnimEditor &editor);
   static Str  RootMoveX(C AnimEditor &editor);         
   static Str  RootMoveY(C AnimEditor &editor);         
   static Str  RootMoveZ(C AnimEditor &editor);         
   static void RootMoveX(  AnimEditor &editor, C Str&t);
   static void RootMoveY(  AnimEditor &editor, C Str&t);
   static void RootMoveZ(  AnimEditor &editor, C Str&t);

   static Str  RootRotX(C AnimEditor &editor);         
   static Str  RootRotY(C AnimEditor &editor);         
   static Str  RootRotZ(C AnimEditor &editor);         
   static void RootRotX(  AnimEditor &editor, C Str&t);
   static void RootRotY(  AnimEditor &editor, C Str&t);
   static void RootRotZ(  AnimEditor &editor, C Str&t);

   static void SetSelMirror(AnimEditor &editor);
   static void SetMirrorSel(AnimEditor &editor);
          void setSelMirror(bool set_other);
   static void SkelBonePosCopy (AnimEditor &editor);
   static void SkelBonePosCopyR(AnimEditor &editor);
          void skelBonePosCopy (bool relative);
   static void SkelBonePosPaste (AnimEditor &editor);
   static void SkelBonePosPasteX(AnimEditor &editor);
   static void SkelBonePosPasteZ(AnimEditor &editor);
          void skelBonePosPaste (C Vec &mask);

   bool selected()C;      
   void selectedChanged();

   ElmAnim* data()C;       
   Animation* getVisAnim();

   flt  timeToFrac(flt time)C;
   flt  animTime  (        )C;
   void animTime  (flt time);
   void setLoop(bool loop);
   void setLinear(bool linear);
   void selBone(int bone);
   void setOrnTarget();
   void setMenu();
   void setMenu(Node<MenuElm> &menu, C Str &prefix);
   AnimEditor& create();
   SkelBone transformedBone(int i)C;
   Matrix transformedBoneAxis(int i)C;
   int getBone(GuiObj *go, C Vec2 &screen_pos);
   int boneParent(int bone)C;
   AnimKeys* findVisKeys(int sbon_index, bool root=true);
   AnimKeys* findKeys(int sbon_index, bool root=true);
   AnimKeys* getKeys(int sbon_index, bool root=true);
   AnimKeys* findKeysParent(int sbon_index, bool root=true); // return null for root, because 'findKeys' already returns root for -1, and 'findKeysParent' would return the same

   static int CompareKey(C AnimKeys::Orn   &key, C flt &time);
   static int CompareKey(C AnimKeys::Pos   &key, C flt &time);
   static int CompareKey(C AnimKeys::Scale &key, C flt &time);

   static AnimKeys::Orn  * FindOrn  (AnimKeys &keys, flt time);
   static AnimKeys::Pos  * FindPos  (AnimKeys &keys, flt time);
   static AnimKeys::Scale* FindScale(AnimKeys &keys, flt time);

   static AnimKeys::Orn  & GetOrn  (AnimKeys &keys, flt time, C Orient &Default);
   static AnimKeys::Pos  & GetPos  (AnimKeys &keys, flt time, C Vec    &Default);
   static AnimKeys::Scale& GetScale(AnimKeys &keys, flt time, C Vec    &Default);

   flt getBlend(C AnimKeys::Key &key )C;
   flt getBlend(           flt  time)C;

   virtual void update(C GuiPC &gpc)override;
   bool selectionZoom(flt &dist);
   bool getHit(GuiObj *go, C Vec2 &screen_pos, Vec &hit_pos);
   virtual void camCenter(bool zoom)override;
   virtual void resize()override;
   void frame(int d);
   bool delFrameOrn(int bone);
   bool delFramePos(int bone);
   bool delFrameScale(int bone);
   bool delFramesOrn(int bone);
   bool delFramesPos(int bone);
   bool delFramesScale(int bone);
   void delFrame();
   void delFrames();
   void delFramesAtEnd();
   void reverseFrames();
   void removeMovement();
   void freezeBone();
   void playToggle();
   void playUpdate(flt multiplier=1);
   void prepMeshSkel();
   void toGui();
   void applySpeed();
   void moveEvent(int event, flt time);
   void newEvent(flt time);
   void delEvent(int index);
   void renameEvent(int index, C Str &old_name, C Str &new_name);
   void flush();
   void setChanged(bool file=true);
   void validateFullscreen();
   void toggleFullscreen();
   void set(Elm *elm);
   void focus();
   void activate(Elm *elm);
   void toggle(Elm *elm);
   void elmChanging(C UID &elm_id);
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id);   
   void setTarget(C Str &obj_id);
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);

private:
   flt _anim_time;

public:
   AnimEditor();
};
/******************************************************************************/
/******************************************************************************/
extern AnimEditor AnimEdit;
/******************************************************************************/
