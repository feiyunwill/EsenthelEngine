/******************************************************************************/
/******************************************************************************

   Meshes are stored in:
      "Edit" (untransformed, skeleton not set but has BoneMap, parts not merged                          , only MeshBase   available)
      "Game" (  transformed, skeleton     set with    BoneMap, parts     merged by material and DrawGroup, only MeshRender available)

   Upon opening a mesh in Object Editor, "Edit" version is loaded (and MeshRender created)
      every usage of the mesh must be combined with transforming by its current "Game" matrix.

   Upon saving a mesh, it's saved as both "Edit" and "Game" versions.

/******************************************************************************/
class ObjView : Viewport4Region
{
   enum MODE
   {
      TRANSFORM ,
      PARAM     ,
      LOD       ,
      MESH      ,
      VARIATIONS,
      SLOTS     ,
      BONES     ,
      SKIN      ,
      PHYS      ,
      GROUP     ,
      REMOVE    ,
      RAGDOLL   ,
      BODY      ,
      BACKGROUND,
   };
   static cchar8 *mode_t[]
;
   enum TRANS_OP
   {
      TRANS_MOVE,
      TRANS_ROT,
      TRANS_SCALE,
   };
   enum SKIN_MODE
   {
      SKIN_SEL_MESH_PART,
      SKIN_SEL_BONE,
      SKIN_CHANGE_SKIN,
   };
   static cchar8 *skin_mode_t[]
;
   static cchar8 *skin_mode_desc[]
;

   class BackMesh
   {
      MeshPtr mesh;
      Matrix  matrix;

      void draw();
      void drawBlend();

public:
   BackMesh();
   };
   class SlotMesh
   {
      MeshPtr mesh;
      Str8    name;
      flt     scale;

      void set(C MeshPtr &mesh, C Str &name);
      void draw(Skeleton &skel);

public:
   SlotMesh();
   };

   class Info : GuiCustom
   {
      virtual void draw(C GuiPC &gpc)override;
   };

   class MeshChange : Edit::_Undo::Change
   {
      class NameWeight
      {
         Str name;
         flt weight;
      };
      class BoneWeight : Mems<NameWeight>
      {
         Str8 name;
      };

      bool         sel_only,
                   can_use_cur_skel_to_saved_skel; // this needs to be disabled if we've flushed skeleton ('saved_skel' was changed)
      Mesh         data;
      ElmMesh      mesh_data;
      EditSkeleton edit_skel, cur_skel_to_saved_skel;
      Skeleton          skel;
      Matrix       mesh_matrix;
      Memc<VecI2>  sel_vtx, sel_face;
      Mems<Rename>     bone_renames; // renames applied in this change only
      Mems<Str   >     bone_removals; // removals applied in this change only
      Mems<BoneWeight> bone_restores;
      
      void removeBone(C Str &name);

      void selOnly();

      virtual uint memUsage()C override;

      virtual void create(ptr user)override;
      void apply(bool undo);
      virtual void apply(ptr user)override;

public:
   MeshChange();
   };

   class PhysChange : Edit::_Undo::Change
   {
      ElmPhys  phys_data;
      PhysBody data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   class SkinBrush : BrushClass
   {
      SkinBrush& create(GuiObj &parent, C Vec2 &rd);

      virtual bool hasMsWheelFocus()C override;
   };
   
   class BoneRoot
   {
      Str                  display;
      EditSkeleton::NodePtr node;
   };

   Mesh            mesh; // in original matrix as it was imported
   MeshPtr         mesh_ptr, // used to hold reference to      game mesh (this is needed if we're editing 'mesh' on a temp var, and 'skel' on cache var, this way we can keep 'mesh' cache var in the same state as temp var)
                   body    ; // used to hold reference to body game mesh
   Box             mesh_box; // mesh box after transformation
   Matrix          mesh_matrix; // mesh transformation matrix
   Matrix          phys_part_matrix; // used for axis drawing
   PhysBodyPtr     phys; // phys body (in 'mesh_matrix')
   Skeleton       *mesh_skel, mesh_skel_temp, // skeleton of the mesh (in 'mesh_matrix'), if exists points to 'mesh_skel_temp' (otherwise null), it is important to operate on a temporary skeleton, so that modifying skeleton will not affect the existing animations, until it's flushed
                  *body_skel, // skeleton of the body mesh
                   saved_skel; // 'mesh_skel' as it is on the disk without any modifications
   Matrix          saved_skel_matrix; // matrix of 'saved_skel'
   EditSkeleton    saved_edit_skel,
                   edit_skel, // always in original matrix as it was imported
                   cur_skel_to_saved_skel; // this will store mapping from current skeleton to saved skeleton, 'nodes'=saved_skel, 'bones'=mesh_skel
   Particles       particles;
   UID             obj_id;
   Elm            *obj_elm, *mesh_elm, *skel_elm, *phys_elm;
   bool            has_cur_pos;
   Vec             cur_pos, axis_vec;
   int             lit_lod, sel_lod, sel_variation, lit_part, lit_vf_part, lit_vtx, lit_face, trans_axis, // 'lit_vf_part'=part of the ('lit_vtx' or 'lit_face'), 'lit_face' can contain SIGN_BIT for quads
                   lit_bone, lit_bone_vis, lit_slot, sel_bone, sel_bone_vis, sel_slot, slot_axis, bone_axis, // 'vis' operate on the temporary 'getVisSkel'
                   lit_phys, sel_phys, phys_axis;
   Memc<VecI2>     sel_vtx, sel_face; // binary sorted, x=part, y=element, 'sel_face.y' can contain SIGN_BIT for quads
   Str8            sel_bone_name, sel_slot_name;
   int             vtx_dup_mode;
   flt             vtx_sel_r, edit_time;
   Tabs            mode;
   Button          axis, box, show_cur_pos, vtxs, vtxs_front, vtxs_normals, light_dir;
   Tabs            slot_tabs, bone_tabs, phys_tabs, ragdoll_tabs, trans_tabs;
   Memc<SlotMesh>  slot_meshes;
   Memc<BackMesh>  back_meshes;
   TransformRegion trans, trans_mesh;
   LeafRegion      leaf;
   LodRegion       lod;
   GroupRegion     group;
   Info            info;
   ParamEditor     param_edit;
   bool            changed_obj, changed_mesh, changed_skel, changed_phys;
   TimeStamp       mesh_file_time, skel_file_time;
   Button          mesh_undo, mesh_redo, locate, phys_undo, phys_redo, phys_box, phys_ball, phys_capsule, phys_tube, phys_convex[5], phys_mesh, phys_del, goto_phys_mtrl, goto_body, goto_group, clear_background;
   Memx<Property>  phys_props, body_props, group_props;
   Tabs            vtx_face_sel_mode, lod_tabs, variation_tabs, skin_tabs, bone_move_tabs;
   Text            vtx_face_sel_text, background_alpha_t, bone_root_t;
   Slider          background_alpha;
   ComboBox        mesh_ops, skin_ops, slot_ops, bone_ops, phys_ops, bone_root, bone_children, bone_children_rot;
   Memc<BoneRoot>  bone_root_data;
   TextWhite       ts;
   MeshParts       mesh_parts;
   MeshVariations  mesh_variations;
   AdjustBoneOrns  adjust_bone_orns;
   SkinBrush       skin_brush;
   Menu            menu;
   Memc<UID>       menu_ids;
   Edit::Undo<MeshChange> mesh_undos;   bool mesh_undos_undo;
   Edit::Undo<PhysChange> phys_undos;   void undoVis();      

   // get
   bool selected()C;                          
   bool lodEditDist  ()C;                                          
   bool lodDrawAtDist()C;                                          
   bool customDrawMatrix()C;                                       
   bool partVisible(int p, C MeshPart &part, bool allow_lit=true)C;
   bool partOp (int p)C;                                        
   bool partSel(int p)C;                                        
   bool frontFace(C Vec &pos, C Vec *nrm, C Matrix &cam)C;
   bool editMeshParts()C;                              
   bool transMesh    ()C;                              
   bool showBoxes    ()C;                              
   bool showMainBox  ()C;                              
   Matrix transformMatrix()C;                          
   Matrix transformMatrix(bool sel)C;                  
   Circle   vtxSelCircle()C;      
   bool showVtxSelCircle();
   bool showChangeSkin     ()C;
   bool showChangeSkinShape(); 
   Shape changeSkinShape()C;
   int boneTabs()C;
   int boneAxis()C;
   flt posScale()C;
   flt vtxDupPosEps()C; 
   Vec selMeshCenter()C;
   Skeleton* getVisSkel();

   void setMatrixAtDist(Matrix &matrix, flt dist);

   static void Render();
          void render();

   static void Draw(Viewport &viewport);        
          void draw(Edit::Viewport4::View&view);

   static void       LodChanged(ObjView &editor);
   static void VariationChanged(ObjView &editor);
   static void      ModeChanged(ObjView &editor);
   static void Mode1          (ObjView &editor); 
   static void Mode2          (ObjView &editor); 
   static void Mode3          (ObjView &editor); 
   static void Mode4          (ObjView &editor); 
   static void Mode5          (ObjView &editor); 
   static void Mode6          (ObjView &editor); 
   static void Mode7          (ObjView &editor); 
   static void Mode8          (ObjView &editor); 
   static void Mode9          (ObjView &editor); 
   static void Mode0          (ObjView &editor); 
   static void ModeS0         (ObjView &editor); 
   static void ModeS1         (ObjView &editor); 
   static void ModeS2         (ObjView &editor); 
   static void ModeS3         (ObjView &editor); 
   static void ModeS4         (ObjView &editor); 
   static void ModeS5         (ObjView &editor); 
   static void ModeS6         (ObjView &editor); 
   static void ModeS7         (ObjView &editor); 
   static void ModeS8         (ObjView &editor); 
   static void ModeS9         (ObjView &editor); 
   static void ModeS10        (ObjView &editor); 
   static void ModeS11        (ObjView &editor); 
   static void Identity       (ObjView &editor); 
   static void LightMode      (ObjView &editor); 
   static void PrevObj        (ObjView &editor); 
   static void NextObj        (ObjView &editor); 
   static void ShowBox        (ObjView &editor); 
   static void ShowCur        (ObjView &editor); 
   static void VtxsChanged    (ObjView &editor);                      
   static void MeshDelete     (ObjView &editor); void meshDelete   ();
   static void MeshSplit      (ObjView &editor); void meshSplit    ();
   static void MeshAlignXZ    (ObjView &editor);      void meshAlign(bool xz);
   static void MeshAlign      (ObjView &editor);   
   static void MeshWeldPos    (ObjView &editor); void meshWeldPos  ();   void meshWeldPos(flt pos_eps);
   static void MeshSetPos     (ObjView &editor); void meshSetPos   ();
   static void MeshReverse    (ObjView &editor); void meshReverse  ();
   static void MeshReverseN   (ObjView &editor); void meshReverseN ();
   static void MeshSetNormalN (ObjView &editor);         void meshSetNrm(uint vtx_test);
   static void MeshSetNormalT (ObjView &editor);      
   static void MeshSetNormalP (ObjView &editor);      
   static void MeshSetNormal  (ObjView &editor);      
   static void MeshSetNormalH (ObjView &editor);         void meshSetNrmH();
   static void MeshNormalY    (ObjView &editor); void meshNrmY      ();
   static void MeshSetVtxAO   (ObjView &editor); 
   static void MeshCreateFace (ObjView &editor); void meshCreateFace();
   static void MeshMergeFaces (ObjView &editor); void meshMergeFaces();
   static void MeshRotQuads   (ObjView &editor); void meshRotQuads  ();
   static void MeshQuadToTri  (ObjView &editor); void meshQuadToTri ();
   static void MeshTesselate  (ObjView &editor); void meshTesselate ();
   static void MeshSubdivide  (ObjView &editor); void meshSubdivide ();
   static void MeshColorBrght (ObjView &editor); void meshColorBrght();
   static void MeshDelDblSide (ObjView &editor); void meshDelDblSide();
   static void MeshSeparate1  (ObjView &editor); void meshSeparate1 ();
   static void MeshSeparateN  (ObjView &editor); void meshSeparateN ();
   static void MeshCopyParts  (ObjView &editor); void meshCopyParts ();
   static void SetBody        (ObjView &editor);                     
   static void AnimTargetObj  (ObjView &editor); 
   static void MeshSkinFull   (ObjView &editor); void meshSkinFull  ();
   static void MeshSkinFullP  (ObjView &editor); void meshSkinFullP ();
   static void MeshSkinFullU  (ObjView &editor); void meshSkinFullU ();
   static void MeshSkinAuto   (ObjView &editor); void meshSkinAuto  ();
   static void MeshEditSel    (ObjView &editor);             
   static void MeshVFS0       (ObjView &editor);         
   static void MeshVFS1       (ObjView &editor);         
   static void MeshVFS2       (ObjView &editor);         
   static void MeshVFS3       (ObjView &editor);         
   static void ShowVtxs       (ObjView &editor); 
   static void ShowVtxsF      (ObjView &editor); 
   static void ShowVtxsN      (ObjView &editor); 
   static void  GotoPhMtrl    (ObjView &editor); 
   static void ClearPhMtrl    (ObjView &editor); 
   static void  GotoBody      (ObjView &editor); 
   static void ClearBody      (ObjView &editor); 
   static void  GotoGroups    (ObjView &editor); 
   static void ClearGroups    (ObjView &editor); 
   static void ClearBack      (ObjView &editor); 
   static void MeshRemVtxTex1 (ObjView &editor); 
   static void MeshRemVtxTex2 (ObjView &editor); 
   static void MeshRemVtxTex12(ObjView &editor); 
   static void MeshRemVtxColor(ObjView &editor); 
   static void MeshRemVtxSkin (ObjView &editor); 

   void modeS(int i);

   void remVtx(uint flag, C MaterialPtr &mtrl=null, bool only_selected=false);

   static void MeshUndo(ObjView &editor);
   static void MeshRedo(ObjView &editor);

   static void Locate(ObjView &editor);

   // get
   flt  absLodDist(C MeshLod&lod          );
   flt     lodDist(C MeshLod&lod          );
   void    lodDist(  MeshLod&lod, flt dist);
   void setLodDist(  MeshLod&lod, flt dist);
 C MeshLod& getLod()C;                    
   MeshLod& getLod();
 C MeshLod& getDrawLod()C;
   MeshLod& getDrawLod();
 C MeshPart* getPart(int i)C;                                
   MeshPart* getPart(int i);                                 
   int       getPart(GuiObj *go, C Vec2 &screen_pos, Vec *hit_pos=null);
   int getVtxFace(GuiObj *go, C Vec2 &screen_pos, int *hit_vtx=null, int *hit_face=null, Vec *hit_pos=null);
   void getVtxs(GuiObj *go, C Shape &shape, MemPtr<VecI2> vtxs); // 'vtxs' will be sorted
   void getSkel(GuiObj *go, C Vec2 &screen_pos, int *bone_i, int *slot_i);
   void getSkelSlot(C Vec2 &screen_pos);
   int getPhys(GuiObj *go, C Vec2 &screen_pos);
   MeshPart* validateDup(int part);
   void getVtxNeighbors(MemPtr<int> vtxs, int vtx, int part);
   void getFaceNeighbors(MemPtr<int> faces, int face, int part);
   void getSelectedVtxs(MemPtr<VecI2> vtxs, bool from_vtxs=true, bool from_faces=true);

   // set
   void getSamePos(int part, int vtx, MemPtr<VecI2> vtxs);
   void includeSamePos(int part, MemPtr<VecI2> vtxs); // !! assumes that 'vtxs' is sorted and valid (point to valid indexes) !!
   void selVFClear();
   void litSelVFClear();
   void selVFSet(int part, int vtx, int face, bool same_pos=true);
   void selVFToggle(int part, int vtx, int face, bool same_pos=true);
   void selVFInclude(int part, int vtx, int face, bool same_pos=true);
   void selVFExclude(int part, int vtx, int face, bool same_pos=true);
   void selVFDo();
   void selVFDo(bool include, bool same_pos=true);
   void selUndo();

   // create
   void setMenu();
   void setMenu(Node<MenuElm> &menu, C Str &prefix);
   ObjView& create();
   void createMesh();
   virtual void resize()override;
   void setShader();   

   // operations
   void flushObj();
   void flushMeshSkel(SAVE_MODE save_mode=SAVE_DEFAULT);
   void flushPhys();
   void flush(SAVE_MODE save_mode=SAVE_DEFAULT);
   void setChangedObj (                        );
   void setChangedMesh(bool file, bool box=true);
   void setChangedSkel(bool bone               );
   void setChangedPhys(                        );
   void setBox        (                        );

   void gotoPhysMtrl();
   void gotoBody();
   void gotoDrawGroupEnum();
   void setPhysMtrl(C UID &mtrl_id);
   void setBody(C UID &body_id);
   void setDrawGroupEnum(C UID &enum_id);
   void animTargetObj();

   void resetTransform();
   void setTransform(C Matrix &matrix);
   void applyTransform(C Matrix &matrix);
   void skinChanged();
   void selectedChanged();
   bool selectionZoom(flt &dist);
   virtual void camCenter(bool zoom)override;
   void reloadObj();
   void syncObj();
   void reloadMeshSkel();
   void reloadPhys();
   void reload();
   Str nodeDisplayName(int node_i)C;
   void addBoneRootData(int parent, int max_depth);
   void setBoneRootTextSize();
   void toGuiSkel();
   void toGuiPhys();
   void toGui();
   void set(Elm *elm);
   void activate(Elm *elm);
   void toggle(Elm *elm);
   void resetDrawGroupEnum();
   void enumChanged(C UID &enum_id);
   void meshVariationChanged();   
   void elmChanged(C UID &elm_id);
   void meshChanged();
   void skelTransformChanged();
   void erasing(C UID &elm_id);

   Elm* getMeshElm();
   Elm* getSkelElm();

   void dragPhysMtrl(Memc<UID> &elms);
   void dragDrawGroup(Memc<UID> &elms);
   void dragBody(Memc<UID> &elms);
   
   void setAutoTanBin(C MaterialPtr &material);
   void setMaterial(int part_i, C MaterialPtr &material);
   void drag(Memc<UID> &elms, GuiObj* &focus_obj, C Vec2 &screen_pos);
   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos);

   void animate(C AnimSkel &anim_skel);

   virtual void update(C GuiPC &gpc)override;
   static void Add(Str &s, cchar8 *t);       
   void drawInfo(C GuiPC &gpc);

   /////////////////////////////////////////
   // MESH
   /////////////////////////////////////////
   int visibleVariation()C;
   int  selVariation()C;                                       
   void selVariation(int variation, SET_MODE mode=SET_DEFAULT);
   int  selLod()C;                                        
   void selLod(int lod);
   void eraseLOD(int i);
   void disableLOD(int i);
   void updateMesh();

   /////////////////////////////////////////
   // SLOT
   /////////////////////////////////////////
   static void SlotModeChanged(ObjView &editor);

   static void SlotCopy(ObjView &editor);
   static void SlotPaste(ObjView &editor);
   static void SlotReplace(ObjView &editor);
   void slotRot(flt x, flt y, flt z);
   static void SlotRotX (ObjView &editor);
   static void SlotRotXN(ObjView &editor);
   static void SlotRotY (ObjView &editor);
   static void SlotRotYN(ObjView &editor);
   static void SlotRotZ (ObjView &editor);
   static void SlotRotZN(ObjView &editor);

   static void SlotSetSelMirror(ObjView &editor);
   static void SlotSetMirrorSel(ObjView &editor);
          void slotSetSelMirror(bool set_other);

   enum SLOT_MODE
   {
      SLOT_ADD,
      SLOT_DEL,
      SLOT_MOVE,
      SLOT_ROT,
      SLOT_SCALE,
      SLOT_RENAME,
      SLOT_PARENT,
   };
   static cchar8 *slot_desc[]
;
   void createSlots();
   static void DragSlotsSlot(ptr slot_index, GuiObj *go, C Vec2 &screen_pos);
   static void DragSlotsBone(ptr bone_index, GuiObj *go, C Vec2 &screen_pos);
   void updateSlots();
   void renameSlot(int index, C Str &old_name, C Str &new_name);
   void putMeshToSlot(C MeshPtr &mesh, int slot_index);

   /////////////////////////////////////////
   // BONE
   /////////////////////////////////////////
   static cchar8 *del_root_bone_dialog_id;

   static void DelRootBone(ObjView &editor);
          void delRootBone();
   void delBone(int bone_i);

   static void BoneModeChanged(ObjView &editor); //if(editor.bone_tabs()!=BONE_MOVE && editor.bone_tabs()!=BONE_ROT && editor.bone_tabs()!=BONE_SCALE)editor.selBone(-1);}
   static void BoneRootChanged(ObjView &editor);

   static void SkelCopy (ObjView &editor);
   static void SkelPaste(ObjView &editor);
   static void SkelReplace(ObjView &editor);
   static void SkelSetSelMirror(ObjView &editor);
   static void SkelSetMirrorSel(ObjView &editor);
          void skelSetSelMirror(bool set_other);
   static void RotBoneRoll(ObjView &editor);
   static void AdjustBoneOrnsDo(ObjView &editor);
   static void SkelDelBones    (ObjView &editor);
          void skelDelBones    ();
   static void SkelDelLeafBones(ObjView &editor);
          void skelDelLeafBones();

   enum BONE_MODE
   {
      BONE_ADD,
      BONE_DEL,
      BONE_MOVE,
      BONE_ROT,
      BONE_SCALE,
      BONE_RENAME,
      BONE_PARENT,
   };
   static cchar8 *bone_desc[]
;
   enum BONE_MOVE_MODE
   {
      BONE_MOVE_START,
      BONE_MOVE_END  ,
   };
   static cchar8 *bone_move_desc[]
;
   enum BONE_CHILDREN_MODE
   {
      BONE_CHILDREN_NONE   ,
      BONE_CHILDREN_NEAREST,
      BONE_CHILDREN_ALL    ,
   };
   static cchar8 *bone_child_desc[]
;
   static cchar8 *bone_child_rot_desc[]
;
   void clearBones();   
   void selectLit ();   
   int boneToVis(int i);
   int visToBone(int i);
   void  selBone(int i);
   void createBones();
   static void DragBonesBone(ptr bone_index, GuiObj *go, C Vec2 &screen_pos);
   void updateBones();
   bool renameBone(C Str &old_name, C Str &new_name);

   /////////////////////////////////////////
   // PHYS
   /////////////////////////////////////////
   enum PHYS_MODE
   {
      PHYS_DEL,
      PHYS_MOVE,
      PHYS_ROT,
      PHYS_SCALE,
      PHYS_TOGGLE,
   };
   static cchar8 *phys_desc[]
;

   static void PhysUndo(ObjView &editor);
   static void PhysRedo(ObjView &editor);

   static void PhysChanged(ObjView &editor);

   static void NewBox    (ObjView &editor);
   static void NewBall   (ObjView &editor);
   static void NewCapsule(ObjView &editor);
   static void NewTube   (ObjView &editor);
   static void Convex8   (ObjView &editor);
   static void Convex16  (ObjView &editor);
   static void Convex24  (ObjView &editor);
   static void Convex32  (ObjView &editor);
   static void Convex64  (ObjView &editor);
   static void PhysMesh  (ObjView &editor);
   static void PhysDel   (ObjView &editor);
   static void PhysCopy  (ObjView &editor);

   static Str  PhysDensity(C ObjView &editor             );
   static void PhysDensity(  ObjView &editor, C Str &text);

   Elm* getPhysElm();
   bool hasPhysMeshOrConvex();
   void physMesh(MeshBase &mesh);
   void physSetConvex(int max_vtxs);
   void physSetMesh();
   void physCopy();
   void physDel();   
   void createPhys();
   void setPhysPartMatrix();
   void setPhys(int part);
   void updatePhys();
   
   /////////////////////////////////////////
   // RAGDOLL
   /////////////////////////////////////////
   enum RAGDOLL_MODE
   {
      RAGDOLL_TOGGLE,
      RAGDOLL_SCALE ,
      RAGDOLL_MOVE  ,
   };
   static cchar8 *ragdoll_desc[]
;
   void createRagdoll();
   void updateRagdoll();

public:
   ObjView();
};
/******************************************************************************/
/******************************************************************************/
extern ObjView ObjEdit;
/******************************************************************************/
