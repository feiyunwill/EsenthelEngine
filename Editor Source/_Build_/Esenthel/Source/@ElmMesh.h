/******************************************************************************/
/******************************************************************************/
class ElmMesh : ElmData
{
   Pose      transform;
   UID        obj_id,
             skel_id,
             phys_id,
             body_id, // if this is a cloth then 'body_id' points to the body mesh ELM_MESH that this cloth can be applied to
       draw_group_id;
   Mems<UID> mtrl_ids; // materials used by this mesh
   Box       box; // this should not be synced (it is set according to mesh data), set as invalid when mesh is empty
   TimeStamp file_time, body_time, draw_group_time, transform_time;

   // get
   bool equal(C ElmMesh &src)C;
   bool newer(C ElmMesh &src)C;

   virtual bool mayContain(C UID &id)C override;
   virtual void clearLinked()override;          

   flt  posScale    ()C;          
   flt  vtxDupPosEps()C;          
   bool canHaveCustomTransform()C; // meshes which have body assigned, can't have custom transform

   // operations
   virtual void newData()override;
   void fromMtrl(C Mesh &mesh);
   void from(C Mesh &game_mesh); // game version of mesh must be passed here
   uint undo(C ElmMesh &src);
   uint sync(C ElmMesh &src);
   bool syncFile(C ElmMesh &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmMesh();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
