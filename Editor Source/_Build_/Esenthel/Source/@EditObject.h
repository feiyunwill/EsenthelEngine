/******************************************************************************/
/******************************************************************************/
class EditObject : EditParams
{
   class SubObj
   {
      bool      removed;
      UID       id, // id of the sub object instance
                elm_obj_id; // project ELM_OBJ element id
      TimeStamp removed_time;

      // get
      bool old  (C TimeStamp &now=TimeStamp().getUTC())C;
      bool equal(C SubObj    &src                     )C;
      bool newer(C SubObj    &src                     )C;

      // operations
      void setRemoved(bool removed);

      bool sync(C SubObj &src);
      bool undo(C SubObj &src);

      // io
      bool save(File &f)C;
      bool load(File &f);

public:
   SubObj();
   };

   enum FLAG
   {
      OVR_ACCESS        =1<<0,
      OVR_TYPE          =1<<1,
      OVR_CONST         =1<<2,
      OVR_PATH          =1<<3,
      CONSTANT          =1<<4,
      OVR_MESH_VARIATION=1<<5,
   };

   byte          flag; // FLAG
   OBJ_ACCESS    access;
   OBJ_PATH      path  ;
   uint          mesh_variation_id;
   UID           type, // elm.id of ELM_OBJ_CLASS
                 editor_type; // how to draw the object in the editor
   TimeStamp     base_time, access_time, type_time, editor_type_time, const_time, path_time, mesh_variation_time;
   EditObjectPtr base; // this can be set to ELM_OBJ and ELM_OBJ_CLASS
   Memc<SubObj>  sub_objs;

   bool constant()C;

   uint memUsage()C;

   SubObj* findSubObj(C UID &id); 
 C SubObj* findSubObj(C UID &id)C;

   void del();

   // get / set
 C EditParam* findParam(C Str &name)C;
 C EditParam* baseFindParam(C Str &name, PARAM_TYPE type)C;
   bool paramValuesContain(C Str &text, Project &proj)C;

   bool hasBase(C UID &obj_id)C;
   EditObject& setBase(C EditObjectPtr &base, cchar *edit_path);
   EditObject& setAccess(bool custom, OBJ_ACCESS access=OBJ_ACCESS_TERRAIN);
   EditObject& setType(bool custom, C UID &type, cchar *edit_path);
   EditObject& setEditorType(C UID &type=UIDZero);
   EditObject& setConst(bool custom, bool on=false);
   EditObject& setPath(bool custom, OBJ_PATH path=OBJ_PATH_CREATE);
   EditObject& setMeshVariation(bool custom, uint mesh_variation_id=0);

   bool subObjsOld  (C TimeStamp &now=TimeStamp().getUTC())C;
   bool subObjsEqual(C EditObject &src)C;
   bool subObjsNewer(C EditObject &src)C;

   bool old  (C TimeStamp &now=TimeStamp().getUTC())C;
   bool equal(C EditObject &src                    )C;
   bool newer(C EditObject &src                    )C;

   // operations
   void newData();
   void create(C EditObject &src);
   void create(C Object &src, C UID &type, C EditObjectPtr &base, C TimeStamp &time=TimeStamp().getUTC());
   void copyTo(Object &obj, C Project &proj, bool zero_align, C UID *mesh_id, C UID *phys_id)C; // 'zero_align' should be set only to ELM_OBJ
   void copyTo(Edit::ObjData &obj, bool include_removed_params)C;
   void updateEditorType(cchar *edit_path);
   void updateBase(cchar *edit_path);
   bool newParam(C Str &name, PARAM_TYPE type, C Project &proj);
   bool sync(C EditObject &src, cchar *edit_path);
   bool undo(C EditObject &src, cchar *edit_path);
   bool modify(C MemPtr<Edit::ObjChange> &changes, Project &proj);

   // io
   bool save(File &f, cchar *edit_path=null)C;
   bool load(File &f, cchar *edit_path=null);
   bool load(C Str &name);

public:
   EditObject();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
