/******************************************************************************/
/******************************************************************************/
class ObjVer
{
   enum FLAG // !! these enums are saved !!
   {
      REMOVED           =1<<0,
      OVR_ACCESS        =1<<1,
      OVR_PATH          =1<<2,
      TERRAIN           =1<<3,
      PATH_SHIFT        =   4,
      PATH_MASK         = 1|2, // use 2 bits for mask to store up to 4 path modes
      OVR_MESH_VARIATION=1<<6,
   };
   VecI2       area_xy            ; // in which area the object is located                                (helper used to quickly locate the area and load object data)
   SmallMatrix matrix; // object matrix                                                      (helper used to check if object needs to be stored as Terrain Mesh split into multiple areas)
   UID         elm_obj_id   ; // elm.id of the ELM_OBJ project element that this object is based on (helper used to check if object needs to be stored as Terrain Mesh split into multiple areas)
   uint        mesh_variation_id  ; // mesh variation id
   byte        flag               ; // FLAG

   // get / set
   bool     removed   (             )C; // if object is removed
   bool     ovrPath   (             )C;
   bool     meshVarOvr(             )C;
   bool     terrain   (Project &proj)C;
   OBJ_PATH path      (Project &proj)C;
   OBJ_PATH pathSelf  (             )C; // get path settings of self (don't check bases)

   bool set(C ObjData &obj, C VecI2 &area_xy); // return true if any member was changed

   // io
   bool save(File &f)C;
   bool load(File &f);

public:
   ObjVer();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
