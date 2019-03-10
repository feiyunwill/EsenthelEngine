/******************************************************************************

   Use 'Object' to create custom object templates in the Editor.
   They will be used for creation of the actual game objects 'Game.Obj'.

/******************************************************************************/
#if EE_PRIVATE
enum OBJ_FLAG // Object Flag
{
   OBJ_OVR_SCALE         =1<<0, // override default scale
   OBJ_OVR_MESH          =1<<1, // override default mesh
   OBJ_OVR_MESH_VARIATION=1<<2, // override default mesh variation
   OBJ_OVR_ALIGN         =1<<3, // override default align
   OBJ_OVR_PHYS          =1<<4, // override default phys
   OBJ_OVR_TYPE          =1<<5, // override default type
   OBJ_OVR_ACCESS        =1<<6, // override default access mode
   OBJ_OVR_PATH          =1<<7, // override default path
   OBJ_OVR_CONST         =1<<8, // override default const
   OBJ_CONST             =1<<9, //                  const
};
#endif
enum OBJ_ACCESS : Byte // object access mode
{
   OBJ_ACCESS_CUSTOM , // object can   be accessed and modified during the game any modifications will be saved in SaveGame, this is suited for all dynamic objects which       require custom class
   OBJ_ACCESS_TERRAIN, // object can't be accessed, it will be embedded into the terrain and rendered as static mesh       , this is suited for all static  objects which don't require custom class, these objects are managed completely by the engine
   OBJ_ACCESS_GRASS  , // works like OBJ_ACCESS_TERRAIN however it is optimized for instanced grass rendering, additionaly objects of this mode can be controlled by D.grass methods
   OBJ_ACCESS_OVERLAY, // works like OBJ_ACCESS_TERRAIN however it will generate a mesh overlay on the terrain using objects material
   OBJ_ACCESS_NUM    , // number of access modes
};
enum OBJ_PATH : Byte // object path mesh mode
{
   OBJ_PATH_CREATE, //       create path mesh from this object
   OBJ_PATH_IGNORE, // don't create path mesh from this object
   OBJ_PATH_BLOCK , // fully block  path mesh from this object
   OBJ_PATH_NUM   , // number of path modes
};
/******************************************************************************/
DECLARE_CACHE(Object, Objects, ObjectPtr); // 'Objects' cache storing 'Object' objects which can be accessed by 'ObjectPtr' pointer
/******************************************************************************/
struct Object // Object Parameters - they're created in the Editor and are used for creating typical Game Objects 'Game.Obj'
{
   // members
   Matrix       matrix  ; // matrix, its scale determines object's scale
   Memc<Param>  params  ; // parameters
   Memc<Object> sub_objs; // sub-objects

   // manage
   Object& del   (             ); // delete manually
   Object& create(C Object &src); // create from 'src'

   // get / set
   Matrix matrixFinal()C; // get final matrix ('matrix' adjusted by 'align'), its scale determines object's scale
   Vec    centerFinal()C; // get final matrix center

   ObjectPtr firstStored(              ) ; // get first object (starting from self to its all bases) which is stored in the cache (which means that it's a project element and not dynamically created), this will iterate through object and all of its bases and return the first one which is stored in the cache, null on fail
   Bool        hasBase  (C UID &base_id)C; // check if this object or one of its bases have 'base_id' base

   void base           (             C ObjectPtr   &base                     ); C ObjectPtr  & base              ()C {return _base             ;} // set/get base object
   void type           (Bool custom, C UID         &type  =UIDZero           ); C UID        & type              ()C {return _type             ;} // set/get object type        (OBJ_TYPE 'ObjType' enum element ID), 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to false then base value will be used)
   void access         (Bool custom,   OBJ_ACCESS   access=OBJ_ACCESS_TERRAIN);   OBJ_ACCESS   access            ()C {return _access           ;} // set/get object access mode (OBJ_ACCESS                        ), 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to false then base value will be used)
   void path           (Bool custom,   OBJ_PATH     path  =OBJ_PATH_CREATE   );   OBJ_PATH     path              ()C {return _path             ;} // set/get object path   mode (OBJ_PATH                          ), 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to false then base value will be used)
   void mesh           (Bool custom, C MeshPtr     &mesh  =null              ); C MeshPtr    & mesh              ()C {return _mesh             ;} // set/get mesh                                                   , 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to false then base value will be used)
   void meshVariationID(Bool custom,   UInt         id    =0                 );   UInt         meshVariationID   ()C {return _mesh_variation_id;} // set/get mesh                          variation ID             , 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to false then base value will be used), based on this value, 'meshVariationIndex' can be obtained. This ID is the unique ID of the mesh variation - 'Mesh.variationID'. It should not be passed to the 'SetVariation' function, instead please use 'meshVariationIndex' for that.
                                                                                  Int          meshVariationIndex()C;                             //     get mesh variation index based on variation ID             , you can use this index for 'SetVariation' function before drawing the mesh
   void phys           (Bool custom, C PhysBodyPtr &phys  =null              ); C PhysBodyPtr& phys              ()C {return _phys             ;} // set/get physical body                                          , 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to then false base value will be used)
   void constant       (Bool custom,   Bool         on    =false             );   Bool         constant          ()C;                             // set/get object constant mode                                   , 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to then false base value will be used), this is special modifier for OBJ_ACCESS_CUSTOM access specifying that the object is always constant (never changes) and should not be saved in SaveGame, but should always created as new object from its default parameters (this reduces SaveGame size)
   void scale          (Bool custom, C Vec         &scale =1                 );   Flt          scale             ()C;                             // set/get object scale                                           , 'custom'=if override base defaults and set custom value (when set to true then custom value will be used, if set to then false base value will be used)
                                                                                  Vec          scale3            ()C;                             //     get object scale in 3 dimensions

   Bool customType            ()C; // if this object                     uses custom type           (if 'custom' was set to true when setting the value)
   Bool customAccess          ()C; // if this object                     uses custom access         (if 'custom' was set to true when setting the value)
   Bool customPath            ()C; // if this object                     uses custom path           (if 'custom' was set to true when setting the value)
   Bool customMesh            ()C; // if this object                     uses custom mesh           (if 'custom' was set to true when setting the value)
   Bool customMeshVariation   ()C; // if this object                     uses custom mesh variation (if 'custom' was set to true when setting the value)
   Bool customMeshVariationAny()C; // if this object or any of its bases use  custom mesh variation (if 'custom' was set to true when setting the value)
   Bool customPhys            ()C; // if this object                     uses custom phys           (if 'custom' was set to true when setting the value)
   Bool customConstant        ()C; // if this object                     uses custom constant       (if 'custom' was set to true when setting the value)
   Bool customScale           ()C; // if this object                     uses custom scale          (if 'custom' was set to true when setting the value)
   Bool customAlign           ()C; // if this object                     uses custom align          (if 'custom' was set to true when setting the value)

   // params
   Param * findParam (C Str &name) ; // find parameter by name, first local parameters are searched, if not found then bases are searched recursively, null on fail !! each objects parameters are required to be sorted according to their name !!
 C Param * findParam (C Str &name)C; // find parameter by name, first local parameters are searched, if not found then bases are searched recursively, null on fail !! each objects parameters are required to be sorted according to their name !!
   Param &  getParam (C Str &name) ; // get  parameter by name, first local parameters are searched, if not found then bases are searched recursively, Exit on fail !! each objects parameters are required to be sorted according to their name !!
 C Param &  getParam (C Str &name)C; // get  parameter by name, first local parameters are searched, if not found then bases are searched recursively, Exit on fail !! each objects parameters are required to be sorted according to their name !!
   Object& sortParams(           ) ; // sort parameters according to their names, this needs to be called after manual adding/removing custom parameters (this is needed for using 'findParam getParam' methods which assume that parameters are sorted)

   // alignment
   ALIGN_TYPE alignX()C; // get x alignment
   ALIGN_TYPE alignY()C; // get y alignment
   ALIGN_TYPE alignZ()C; // get z alignment
   void       align (Bool custom, ALIGN_TYPE x=ALIGN_NONE, ALIGN_TYPE y=ALIGN_NONE, ALIGN_TYPE z=ALIGN_NONE); // set alignment

   // operations
   Object& updateBase(); // update members according to base (you should call this if base members were changed)
#if EE_PRIVATE
   Object& updateBaseSelf(); // update members according to base (of self only, without sub objects)
#endif

   // io
   Bool save(C Str &name)C; // save, false on fail
   Bool load(C Str &name) ; // load, false on fail
   Bool save(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
#if EE_PRIVATE
   Bool saveData(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool loadData(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   void zero();
#endif

   // compare
   Bool operator==(C Object &obj)C;                    // if     equal
   Bool operator!=(C Object &obj)C {return !(T==obj);} // if not equal

   Object();

#if !EE_PRIVATE
private:
#endif
   UShort      _flag;
   Byte        _align;
   OBJ_ACCESS  _access;
   OBJ_PATH    _path;
   UID         _type;
   ObjectPtr   _base;
   MeshPtr     _mesh;
   UInt        _mesh_variation_id;
   PhysBodyPtr _phys;
};
/******************************************************************************/
extern Enum ObjType; // OBJ_TYPE enum responsible for object type management
/******************************************************************************/
#if EE_PRIVATE
void ShutObj();
#endif
/******************************************************************************/
