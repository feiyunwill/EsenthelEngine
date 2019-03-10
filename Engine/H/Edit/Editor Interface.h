/******************************************************************************/
namespace Edit{
/******************************************************************************/
enum ELM_TYPE : Byte // Project Element Type
{
   ELM_NONE       , // None
   ELM_FOLDER     , // Folder
   ELM_ENUM       , // Enum
   ELM_IMAGE      , // Image
   ELM_IMAGE_ATLAS, // Image Atlas
   ELM_FONT       , // Font
   ELM_TEXT_STYLE , // Text Style
   ELM_PANEL      , // Panel
   ELM_GUI        , // Gui Objects
   ELM_SHADER     , // Shader
   ELM_MTRL       , //          Material
   ELM_WATER_MTRL , // Water    Material
   ELM_PHYS_MTRL  , // Physical Material
   ELM_OBJ_CLASS  , // Object Class
   ELM_OBJ        , // Object
   ELM_MESH       , // Mesh
   ELM_SKEL       , // Skeleton
   ELM_PHYS       , // Physical Body
   ELM_ANIM       , // Animation
   ELM_PANEL_IMAGE, // Panel Image
   ELM_ICON       , // Icon
   ELM_ENV        , // Environment
   ELM_WORLD      , // World
   ELM_SOUND      , // Sound
   ELM_VIDEO      , // Video
   ELM_FILE       , // Raw File
   ELM_CODE       , // Code
   ELM_LIB        , // Code Library
   ELM_APP        , // Code Application
   ELM_ICON_SETTS , // Icon Settings
   ELM_MINI_MAP   , // World Mini Map
   ELM_GUI_SKIN   , // Gui Skin
   ELM_NUM        , // Number of Element Types
};
/******************************************************************************/
enum RELOAD_RESULT : Byte // Element Reload Result
{
   RELOAD_ELM_NOT_FOUND, // element   was not found
   RELOAD_NOT_REQUESTED, // reloading was not requested
   RELOAD_IN_PROGRESS  , // reloading is still in progress
   RELOAD_CANCELED     , // reloading was canceled
   RELOAD_FAILED       , // reloading failed
   RELOAD_SUCCESS      , // reloading succeeded
};
/******************************************************************************/
struct Elm // Project Element
{
   ELM_TYPE   type   ; // type of the element
   Bool       removed, // if this element is marked   as removed    (this does not include parents state which may affect the final result, see 'final_removed' for final value)
              publish, // if this element is included in publishing (this does not include parents state which may affect the final result, see 'final_publish' for final value)
        final_removed, // if this element or  its parents are marked   as removed
        final_publish; // if this element and its parents are included in publishing
   UID             id, //      ID   of the element
            parent_id; //      ID   of the element's parent ('UIDZero' means no parent)
   Str           name, //      name of the element
            full_name, // full name of the element (including its parents)
             src_file; // source file from which this element was created

   Bool save(File &f)C {f<<type<<removed<<publish<<id<<parent_id<<name<<src_file; return f.ok();}
   Bool load(File &f)  {f>>type>>removed>>publish>>id>>parent_id>>name>>src_file; return f.ok();}
};
/******************************************************************************/
struct Project // Project
{
   UID id  ; // ID   of the project
   Str name; // name of the project

   Project& set(C UID &id, C Str &name) {T.id=id; T.name=name; return T;}

   Bool save(File &f)C {f<<id<<name; return f.ok();}
   Bool load(File &f)  {f>>id>>name; return f.ok();}
};
/******************************************************************************/
T1(TYPE) struct IDParam // ID with a custom parameter
{
   UID  id;
   TYPE value;

   Bool save(File &f)C {f<<id<<value; return f.ok();}
   Bool load(File &f)  {f>>id>>value; return f.ok();}

   IDParam(C UID &id=UIDZero       ) : id(id)               {}
   IDParam(C UID &id, C TYPE &value) : id(id), value(value) {}
};
/******************************************************************************/
struct ObjData
{
   struct Param : EE::Param
   {
      UID  id       ; // ID of the parameter (each parameter has its unique ID)
      bool removed  , // if this parameter is currently removed
           inherited; // if this parameter is inherited from a base object or object class (if this is true then the parameter does not directly exist in the object itself, but it exists in the base/class of the object)

      Bool save(File &f)C {if(EE::Param::save(f)){f<<id<<removed<<inherited; return f.ok();} return false;}
      Bool load(File &f)  {if(EE::Param::load(f)){f>>id>>removed>>inherited; return f.ok();} return false;}

      Param() {id.zero(); removed=inherited=false;}
   };

   UID         elm_obj_class_id; // ID of ELM_OBJ_CLASS element in the project (this is the project element object class of this object)
   OBJ_ACCESS  access; // access mode of the object
   OBJ_PATH    path  ; // path   mode of the object
   Memc<Param> params; // parameters  of the object

   Param* findParam(C Str &name, PARAM_TYPE type=PARAM_NUM, Bool include_removed=false, Bool include_inherited=true); // find the first parameter from 'params' of 'name' and 'type' (use PARAM_NUM to accept all parameter types), 'include_removed'=if return removed parameters as well, 'include_inherited'=if return inherited parameters as well, null on fail (if not found)

   Bool save(File &f)C;
   Bool load(File &f);

   ObjData&   reset(); // reset to default values
   ObjData() {reset();}
};
/******************************************************************************/
struct ObjChange
{
   void  removeParam(C UID &param_id                                   ); // remove  parameter by specifying its ID                                                                       , if this parameter does not exist then nothing will happen
   void  removeParam(C Str &param_name, PARAM_TYPE param_type=PARAM_NUM); // remove  parameter by specifying its name and optionally type (setting PARAM_NUM will make the 'type' ignored), if this parameter does not exist then nothing will happen
   void restoreParam(C UID &param_id                                   ); // restore parameter by specifying its ID                                                                       , if this parameter does not exist then nothing will happen
   void restoreParam(C Str &param_name, PARAM_TYPE param_type=PARAM_NUM); // restore parameter by specifying its name and optionally type (setting PARAM_NUM will make the 'type' ignored), if this parameter does not exist then nothing will happen

   void renameParam(C UID &param_id      , C Str &param_new_name                                 ); // rename parameter by specifying its ID                                                                               and new name, if this parameter does not exist then nothing will happen
   void renameParam(C Str &param_old_name, C Str &param_new_name, PARAM_TYPE param_type=PARAM_NUM); // rename parameter by specifying its old name, optionally type (setting PARAM_NUM will make the 'param_type' ignored) and new name, if this parameter does not exist then nothing will happen

   void setParamTypeValue(C UID &param_id  , C Param &type_value                                     ); // change parameter type and value to 'type_value', parameter is specified by its ID                                                                                                  , if this parameter is removed then this method will restore it, if this parameter does not exist then nothing will happen
   void setParamTypeValue(C Str &param_name, C Param &type_value, PARAM_TYPE param_old_type=PARAM_NUM); // change parameter type and value to 'type_value', parameter is specified by its name and optionally 'param_old_type' type (setting PARAM_NUM will make the 'param_old_type' ignored), if this parameter is removed then this method will restore it, if this parameter does not exist then it will be automatically created

   Bool save(File &f, CChar *path=null)C;
   Bool load(File &f, CChar *path=null) ;

   ObjChange();

   Byte       cmd;
   PARAM_TYPE type;
   UID        id;
   Str        name;
   Param      param;
};
/******************************************************************************/
struct WorldObjParams // World Object parameters
{
   UID    id; // ID of ELM_OBJ element in the project
   Matrix matrix; // object matrix (its scale determines the object size)
   Bool   align_to_terrain; // if align vertical position of the object to the terrain at that location (in that case 'matrix.pos.y' will be ignored)
   Flt    align_to_terrain_normal; // factor 0..1 of how much to align the object to the terrain surface normal vector (if this is enabled then the object will be additionally rotated according to the terrain shape)

   WorldObjParams& set(C UID &id, C Matrix &matrix, Bool align_to_terrain=false, Flt align_to_terrain_normal=0) {T.id=id; T.matrix=matrix; T.align_to_terrain=align_to_terrain; T.align_to_terrain_normal=align_to_terrain_normal; return T;}

   Bool save(File &f)C {f<<id<<matrix<<align_to_terrain<<align_to_terrain_normal; return f.ok();}
   Bool load(File &f)  {f>>id>>matrix>>align_to_terrain>>align_to_terrain_normal; return f.ok();}

   WorldObjParams(C UID &id=UIDZero, C Matrix &matrix=MatrixIdentity, Bool align_to_terrain=false, Flt align_to_terrain_normal=0) {set(id, matrix, align_to_terrain, align_to_terrain_normal);}
};
struct WorldObjDesc // Basic Description of a World Object
{
   UID   instance_id, // ID of world object instance (each object in the world has its unique ID)
          elm_obj_id; // ID of ELM_OBJ element in the project (this is the project element object on which the world object is based)
   Matrix matrix    ; // matrix of the world object (its scale determines the object size)
   Bool   selected  , // if the object is currently selected
          removed   ; // if the object is currently marked as removed

   Bool save(File &f)C {f<<instance_id<<elm_obj_id<<matrix<<selected<<removed; return f.ok();}
   Bool load(File &f)  {f>>instance_id>>elm_obj_id>>matrix>>selected>>removed; return f.ok();}

   WorldObjDesc() {instance_id.zero(); elm_obj_id.zero(); matrix.identity(); selected=removed=false;}
};
struct WorldObjData : WorldObjDesc, ObjData // Full Data of a World Object
{
   Bool save(File &f)C {return WorldObjDesc::save(f) && ObjData::save(f);}
   Bool load(File &f)  {return WorldObjDesc::load(f) && ObjData::load(f);}
};
/******************************************************************************/
struct MaterialMap
{
   void create(Int resolution); // create using specified image resolution (the image will be created as square "resolution x resolution")

   Int resolution()C;

   void set(Int x, Int y, C UID &m0, C UID &m1, C UID &m2, C UID &m3, C VecB4 &blend); // set 'm0 m1 m2 m3' ID's of project ELM_MTRL materials, with their corresponding 'blend' intensities (range of 0..255), at 'x, y' image coordinates
   void set(Int x, Int y, C UID &m0, C UID &m1, C UID &m2, C UID &m3, C Vec4  &blend); // set 'm0 m1 m2 m3' ID's of project ELM_MTRL materials, with their corresponding 'blend' intensities (range of 0..  1), at 'x, y' image coordinates
   void get(Int x, Int y,   UID &m0,   UID &m1,   UID &m2,   UID &m3,   VecB4 &blend); // get 'm0 m1 m2 m3' ID's of project ELM_MTRL materials, with their corresponding 'blend' intensities (range of 0..255), at 'x, y' image coordinates
   void get(Int x, Int y,   UID &m0,   UID &m1,   UID &m2,   UID &m3,   Vec4  &blend); // get 'm0 m1 m2 m3' ID's of project ELM_MTRL materials, with their corresponding 'blend' intensities (range of 0..  1), at 'x, y' image coordinates

   void resize(Int resolution);

   Bool save(File &f)C; // save to   file, false on fail
   Bool load(File &f) ; // load from file, false on fail

   void           del(); // delete manually
  ~MaterialMap() {del();}

private:
   Image _m, _i;
   IDPalette _ip;
};
/******************************************************************************/
struct FileParams
{
   Str             name;
   Mems<TextParam> params;

   Bool is()C {return name.is() || params.elms();}

   TextParam* findParam(C Str &name);
 C TextParam* findParam(C Str &name)C;
   TextParam&  getParam(C Str &name);

   Str  encode(        )C; // encode into string
   void decode(C Str &s) ; // decode from string

   FileParams(        ) {}
   FileParams(C Str &s) {decode(s);}

   static Str              Encode(C MemPtr<FileParams> &file_params); // encode 'file_params' array into string
   static Mems<FileParams> Decode(C Str                &str        ); // decode 'str' string into file params array
   static Str              Merge (C Str                &a, C Str &b); // merge  'a' 'b' strings
};
/******************************************************************************/
struct Material
{
   MATERIAL_TECHNIQUE technique;
   Bool               cull, flip_normal_y,
                      high_quality_ios; // if use higher quality texture format on iOS platform
   Byte               downsize_tex_mobile; // how much to downsize textures for Mobile platforms, 0=full size, 1=half size, 2=quarter size, ..
   Vec4               color;
   Vec                ambient;
   Flt                specular, glow, roughness, bump, reflection;
   Mems<FileParams>   color_map, alpha_map,
                      bump_map, normal_map,
                      specular_map,
                      glow_map,
                      reflection_map,
                      detail_color, detail_bump, detail_normal,
                      macro_map,
                      light_map;

   Bool hasColorMap ()C {return  color_map  .elms()>0;}
   Bool hasBumpMap  ()C {return   bump_map  .elms()>0;}
   Bool hasNormalMap()C {return normal_map  .elms()>0 || hasBumpMap();}
   Bool hasDetailMap()C {return detail_color.elms()>0 || detail_bump.elms()>0 || detail_normal.elms()>0;}

   void save(File &f)C;
   Bool load(File &f);

   Material&   reset(); // reset to default values
   Material() {reset();}
};
/******************************************************************************/
struct EditorInterface
{
   static Str ElmFileName(C UID &elm_id, C Str &data_path=S) {return elm_id.valid() ? Str(data_path).tailSlash(true)+EncodeFileName(elm_id) : S;} // get path to the element file, you can use it to access resources in read-only mode, do not write to the files under no condition

   // connection
   Bool    connected(                            ); // if connected
   void disconnect  (                            ); // disconnect
   Bool    connect  (Str &message, Int timeout=-1); //    connect to a running instance of Esenthel Editor, 'timeout'=time in milliseconds (-1=default) to wait for a connection, false on fail

   // projects
   Str  projectsPath(           ); // get path where your projects are located
   Bool projectsPath(C Str &path); // set path where your projects are located

   Bool  getProjects(MemPtr<Project> projects); // get list of projects, false on fail
   UID   curProject (                        ); // get ID of currently opened project, 'UIDZero' is returned if no project is opened
   Bool openProject (C UID &proj_id=UIDZero  ); // change active project to the one specified by its ID, if 'UIDZero' is passed then editor will open the project list menu, false on fail

   // following methods operate on currently opened project
      // settings
      Str dataPath(); // get project game data path, you can use it to access resources in read-only mode, do not write data to the path under no condition, "" on fail

      // elements
      Bool      getElms (  MemPtr<Elm>  elms, Bool include_removed=false); // get list of          elements     (elements will be sorted by their ID                ), false on fail, 'include_removed'=if include elements that were removed
      Bool selectedElms (  MemPtr<UID>  elms                            ); // get list of selected element ID's (ID's     will be sorted by which was selected first), false on fail
      Bool   selectElms (C MemPtr<UID> &elms                            ); // select elements based on their ID's                                                    , false on fail
      UID       newElm  (ELM_TYPE type, C Str &name,                                       C UID &parent_id=UIDZero); // create a new element in the project of 'type', 'name' and assigned to 'parent_id' parent (use 'UIDZero' for no parent), this method does not support following types: ELM_MESH, ELM_SKEL, ELM_PHYS, ELM_WORLD (for creating worlds please use 'newWorld' method)      , ID of the newly created element will be returned or 'UIDZero' if failed
      UID       newWorld(               C Str &name, Int area_size=64, Int terrain_res=64, C UID &parent_id=UIDZero); // create a new world   in the project            'name' and assigned to 'parent_id' parent (use 'UIDZero' for no parent), 'area_size'=size of a single area (in meters, valid values are: 32, 64, 128), 'terrain_res'=terrain resolution (valid values are: 32, 64, 128), ID of the newly created element will be returned or 'UIDZero' if failed

      Bool setElmName   (C MemPtr< IDParam<Str > > &elms); // set 'name'          for elements, where 'IDParam.id'=element ID, 'IDParam.value'=element name
      Bool setElmRemoved(C MemPtr< IDParam<Bool> > &elms); // set 'removed' state for elements, where 'IDParam.id'=element ID, 'IDParam.value'=element removed state
      Bool setElmPublish(C MemPtr< IDParam<Bool> > &elms); // set 'publish' state for elements, where 'IDParam.id'=element ID, 'IDParam.value'=element publish state
      Bool setElmParent (C MemPtr< IDParam<UID > > &elms); // set 'parent'        for elements, where 'IDParam.id'=element ID, 'IDParam.value'=parent ID (use 'UIDZero' for no parent)
      Bool setElmSrcFile(C MemPtr< IDParam<Str > > &elms); // set 'src_file'      for elements, where 'IDParam.id'=element ID, 'IDParam.value'=element source file (this does not reload the element, it only adjusts the source file for it)

      Bool       reloadElms  (C MemPtr<UID> &elms, Bool remember_result                    ); // reload elements specified by their ID, elements will be reloaded from their current 'Elm.src_file', which can be changed using the 'setElmSrcFile' method. This method does not wait until elements finish reloading, it only requests the reload, and returns true if the request was accepted, and false if request failed. Which means that even if this method returns true, reload may still fail, for example if the element 'src_file' was not found. 'remember_result'=if this is set to true, then upon completion of reload process, the Editor will remember reload result for each element, which can be obtained later using 'reloadResult' method, while 'forgetReloadResult' can be called to forget those results.
      Bool cancelReloadElms  (C MemPtr<UID> &elms                                          ); // cancel reloading elements specified by their ID
      Bool       reloadResult(C MemPtr<UID> &elms, MemPtr< IDParam<RELOAD_RESULT> > results); // get element reload result for each element, you can call this after calling 'reloadElms' with 'remember_result' set to true
      Bool forgetReloadResult(C MemPtr<UID> &elms                                          ); // if you've called 'reloadElms' with 'remember_result' set to true then the Editor will remember reload result for specified elements, with this function you can forget those results

      // world
      UID   curWorld(                       ); // get ID of currently opened ELM_WORLD element, 'UIDZero' is returned if no world is opened
      Bool openWorld(C UID &world_id=UIDZero); // open world in the world editor, 'world_id'=ID of ELM_WORLD element in the project, if 'UIDZero' is passed then editor will close the world editor, false on fail

      Int worldAreaSize(C UID &world_id); // get size (in meters) of a single area in specified world, 'world_id'=ID of ELM_WORLD element in the project, 0 on fail

         // world terrain
         Int  worldTerrainRes  (C UID &world_id              ); // get terrain resolution of a single area in specified world, 'world_id'=ID of ELM_WORLD element in the project, 0 on fail
         Bool worldTerrainAreas(C UID &world_id, RectI &areas); // get coverage of all areas in the world which have terrain, coordinates are from 'areas.min' to 'areas.max' inclusive, if no areas in the world have terrain then 'RectI(0, 0, -1, -1)' will be returned, 'world_id'=ID of ELM_WORLD element in the project, false on fail

         Bool worldTerrainDel        (C UID &world_id, C VecI2 &area_xy                                                                                      ); // delete          terrain          at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail
         Bool worldTerrainExists     (C UID &world_id, C VecI2 &area_xy                                                                                      ); // test if there's terrain          at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail
         Bool worldTerrainGetHeight  (C UID &world_id, C VecI2 &area_xy,   Image &height                                                                     ); // get             terrain height   at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, 'height'   =height   map (its         resolution will be "worldTerrainRes(world_id)", IMAGE_F32    type, IMAGE_SOFT mode)
         Bool worldTerrainSetHeight  (C UID &world_id, C VecI2 &area_xy, C Image &height,                                          C UID &material_id=UIDZero); // set             terrain height   at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, 'height'   =height   map (recommended resolution      is "worldTerrainRes(world_id)" however it will be automatically resized if different, recommended format is IMAGE_F32   ), each terrain must have a material set, if there is already a terrain at that location, then its material will not be modified, if there is no terrain yet, then 'material_id' will be used (which is ID of ELM_MTRL element in the project), if the material was not found or it was set to 'UIDZero', then default material from the editor terrain material slot will be used, if that material was not set, then the method will fail and the terrain will not be created
         Bool worldTerrainGetColor   (C UID &world_id, C VecI2 &area_xy,                                             Image &color                            ); // get             terrain color    at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, 'color'    =color    map (its         resolution will be "worldTerrainRes(world_id)", IMAGE_R8G8B8 type, IMAGE_SOFT mode)
         Bool worldTerrainSetColor   (C UID &world_id, C VecI2 &area_xy,                                           C Image &color, C UID &material_id=UIDZero); // set             terrain color    at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, 'color'    =color    map (recommended resolution      is "worldTerrainRes(world_id)" however it will be automatically resized if different, recommended format is IMAGE_R8G8B8), each terrain must have a material set, if there is already a terrain at that location, then its material will not be modified, if there is no terrain yet, then 'material_id' will be used (which is ID of ELM_MTRL element in the project), if the material was not found or it was set to 'UIDZero', then default material from the editor terrain material slot will be used, if that material was not set, then the method will fail and the terrain will not be created
         Bool worldTerrainGetMaterial(C UID &world_id, C VecI2 &area_xy,                    MaterialMap &material                                            ); // get             terrain material at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, 'material' =material map (its         resolution will be "worldTerrainRes(world_id)")
         Bool worldTerrainSetMaterial(C UID &world_id, C VecI2 &area_xy,                  C MaterialMap &material                                            ); // set             terrain material at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, 'material' =material map (recommended resolution      is "worldTerrainRes(world_id)" however it will be automatically resized if different)
         Bool worldTerrainGet        (C UID &world_id, C VecI2 &area_xy,   Image &height,   MaterialMap &material,   Image *color=null                       ); // get             terrain          at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, this method gets terrain height, material and color all at the same time
         Bool worldTerrainSet        (C UID &world_id, C VecI2 &area_xy, C Image &height, C MaterialMap &material, C Image *color=null                       ); // set             terrain          at 'area_xy' area coordinates in specified world, 'world_id'=ID of ELM_WORLD element in the project, false on fail, this method sets terrain height, material and color all at the same time, if 'color' is null then it will be set as fully white, each terrain must have a material specified

         // world objects
         Bool worldObjCreate (C UID &world_id, C MemPtr<WorldObjParams> &objs                                                                                                                                                          ); // create objects in world, 'world_id'=ID of ELM_WORLD element in the project, 'objs'=array of objects to create, false on fail
         Bool worldObjGetDesc(C UID &world_id,   MemPtr<WorldObjDesc  >  objs, C MemPtr<UID> &world_obj_instance_ids=null, C RectI *areas=null, Bool only_selected=false, Bool include_removed=false                                   ); // get    objects in world, 'world_id'=ID of ELM_WORLD element in the project, 'objs'=array of objects to fill with those from the world, 'world_obj_instance_ids'=if get data only from the objects specified in this container (use null for all objects), 'areas'=get objects only from the specified world areas (use null for all areas), 'only_selected'=if get only objects that are currently selected, 'include_removed'=if include objects that were removed,                                                                          this method is much faster than the one below as it returns only basic data, false on fail
         Bool worldObjGetData(C UID &world_id,   MemPtr<WorldObjData  >  objs, C MemPtr<UID> &world_obj_instance_ids=null, C RectI *areas=null, Bool only_selected=false, Bool include_removed=false, Bool include_removed_params=false); // get    objects in world, 'world_id'=ID of ELM_WORLD element in the project, 'objs'=array of objects to fill with those from the world, 'world_obj_instance_ids'=if get data only from the objects specified in this container (use null for all objects), 'areas'=get objects only from the specified world areas (use null for all areas), 'only_selected'=if get only objects that are currently selected, 'include_removed'=if include objects that were removed, 'include_removed_params'=if include object parameters that were removed, this method is slower      than the one above as it returns full       data, false on fail

         // world waypoints
         Bool worldWaypointGetList(C UID &world_id, MemPtr<UID> waypoint_ids); // get list of all waypoint ID's in world, 'world_id'=ID of ELM_WORLD element in the project, false on fail

         // world camera
         Bool worldCamGet(  Camera &cam); // get camera settings in the world editor, false on fail
         Bool worldCamSet(C Camera &cam); // set camera settings in the world editor, false on fail

         // world draw
         struct Line
         {
            Color color;
            Edge  edge;

            Bool save(File &f)C {f<<color<<edge; return f.ok();}
            Bool load(File &f)  {f>>color>>edge; return f.ok();}

            Line& set(C Color &color, C Edge &edge) {T.color=color; T.edge=edge; return T;}

            Line(                            ) {}
            Line(C Color &color, C Edge &edge) {set(color, edge);}
         };
         Bool worldDrawLines(C MemPtr<Line> &lines); // request the world editor to draw specified lines, after you make this call, the world editor will draw the lines in each frame, if you no longer wish to draw lines, then call this method with empty container, false on fail

      // image
      Bool getImage(C UID &elm_id,   Image &image); // get image of 'elm_id' ELM_IMAGE element in the project, false on fail
      Bool setImage(C UID &elm_id, C Image &image); // set image of 'elm_id' ELM_IMAGE element in the project, false on fail

      // code
      Bool getCode(C UID &elm_id,   Str &code); // get source code of 'elm_id' ELM_CODE element in the project, false on fail
      Bool setCode(C UID &elm_id, C Str &code); // set source code of 'elm_id' ELM_CODE element in the project, false on fail

      Bool codeSyncImport(); // import all codes into active Project from the Code Synchronization folder, false on fail
      Bool codeSyncExport(); // export all codes from active Project into the Code Synchronization folder, false on fail

      // file
      Bool getFile(C UID &elm_id, File &data); // get data of 'elm_id' ELM_FILE ELM_SOUND ELM_VIDEO elements in the project, 'data' must be already opened for writing as the method will write  to it, false on fail
      Bool setFile(C UID &elm_id, File &data); // set data of 'elm_id' ELM_FILE ELM_SOUND ELM_VIDEO elements in the project, 'data' must be already opened for reading as the method will read from it, false on fail

      // material
      UID     curMaterial              (                                                                              ); // get ID of currently opened ELM_MTRL element, 'UIDZero' is returned if no material is opened
      Bool    curMaterial              (C UID &elm_id                                                                 ); // open     material editor            of 'elm_id' ELM_MTRL                element in the project, if 'UIDZero' is passed then editor will close the material editor, false on fail
      Bool    getMaterial              (C UID &elm_id,   Material &material                                           ); // get      material parameters        of 'elm_id' ELM_MTRL ELM_WATER_MTRL element in the project, false on fail
      Bool    setMaterial              (C UID &elm_id, C Material &material, Bool reload_textures, Bool adjust_params ); // set      material parameters        of 'elm_id' ELM_MTRL ELM_WATER_MTRL element in the project, false on fail, 'reload_textures'=if reload the textures that were changed during this update (if this is set to false, then texture files names will get updated, however the texture images will remain the same), 'adjust_params'=if automatically adjust some parameters based on detected texture change (for example default specular value is 0, however if a specular map was added to the material in this change, then specular intensity value should be set to value >0 so the specular effect is visible, in this case 'adjust_params'=will automatically adjust specular intensity to 1 if a specular map was added and the intensity was 0. 'adjust_params' may adjust some other parameters as well)
      Bool reloadMaterialTextures      (C UID &elm_id, bool base, bool reflection, bool detail, bool macro, bool light); // reload   material textures          of 'elm_id' ELM_MTRL ELM_WATER_MTRL element in the project, false on fail
      Bool    mulMaterialTextureByColor(C UID &elm_id                                                                 ); // multiply material textures by color of 'elm_id' ELM_MTRL ELM_WATER_MTRL element in the project, false on fail

      // mesh
      Bool getMesh(C UID &elm_id,   Mesh &mesh, Matrix *matrix=null); // get mesh of 'elm_id' ELM_OBJ ELM_MESH elements in the project, false on fail, 'matrix'=matrix by which the mesh is going to be transformed by the editor
      Bool setMesh(C UID &elm_id, C Mesh &mesh                     ); // set mesh of 'elm_id' ELM_OBJ ELM_MESH elements in the project, false on fail

      // animation
      UID  curAnimation(                                ); // get ID of currently opened ELM_ANIM element, 'UIDZero' is returned if no animation is opened
      Bool curAnimation(C UID &elm_id                   ); // open animation editor of 'elm_id' ELM_ANIM element in the project, if 'UIDZero' is passed then editor will close the animation editor, false on fail
      Bool getAnimation(C UID &elm_id,   Animation &anim); // get  animation        of 'elm_id' ELM_ANIM element in the project, false on fail
      Bool setAnimation(C UID &elm_id, C Animation &anim); // set  animation        of 'elm_id' ELM_ANIM element in the project, false on fail

      // object
      UID     curObject(                                                              ); // get ID of currently opened ELM_OBJ element, 'UIDZero' is returned if no object is opened
      Bool    curObject(C UID &elm_id                                                 ); // open   object editor of 'elm_id' ELM_OBJ               element  in the project, if 'UIDZero' is passed then editor will close the object editor, false on fail
      Bool    getObject(C UID &elm_id, ObjData &obj, Bool include_removed_params=false); // get    object data   of 'elm_id' ELM_OBJ ELM_OBJ_CLASS elements in the project, 'include_removed_params'=if include object parameters that were removed, false on fail
      Bool modifyObject(C UID &elm_id, C MemPtr<ObjChange> &changes                   ); // modify object data   of 'elm_id' ELM_OBJ ELM_OBJ_CLASS elements in the project according to specified list of 'changes', false on fail

      // application
      Bool activeApp(C UID      &elm_id         ); // set    active application of 'elm_id' ELM_APP element in the project, false on fail
      Bool exportApp(EXPORT_MODE mode, Bool data); // export active application, 'data'=if export project data according to selected mode (if mode is Visual Studio then data for Universal Windows will be exported, if mode is Xcode then data for iOS will be exported, if mode is Android then data for Android will be exported), false on fail

   // build settings
   Bool buildDebug(Bool     debug   ); // set build debug/release    configuration (true=debug   , false=release )
   Bool build32Bit(Bool     bit32   ); // set build    32/64 bit     configuration (true=32      , false=64      )
   Bool buildDX9  (Bool     dx9     ); // set build   DX9/DX10+      configuration (true=DX9     , false=DX10+   )
   Bool buildExe  (EXE_TYPE type    ); // set build executable type  configuration
   Bool buildPaths(Bool     relative); // set build export     paths configuration (true=relative, false=absolute)

#if !EE_PRIVATE
private:
#endif
   Connection _conn;
};
/******************************************************************************/
Elm* FindElm(MemPtr<Elm> elms, C UID &elm_id               ); // find element with 'elm_id' ID                    , this function assumes that 'elms' is sorted by element ID's, null is returned if element was not found
Elm* FindElm(MemPtr<Elm> elms, C UID &elm_id, ELM_TYPE type); // find element with 'elm_id' ID and 'type' ELM_TYPE, this function assumes that 'elms' is sorted by element ID's, null is returned if element was not found or it had different type than requested

Elm* FindElm(MemPtr<Elm> elms, C Str &full_name, ELM_TYPE type); // find element with 'full_name' path and name and 'type' ELM_TYPE, null is returned if element was not found or it had different type than requested

Bool ContainsElm(MemPtr<Elm> elms, C UID &parent_id, C UID &child_id); // if element with 'parent_id' ID contains element with 'child_id' ID, this function assumes that 'elms' is sorted by element ID's
/******************************************************************************/
// SERVER SIDE
/******************************************************************************/
struct EditorServer : ConnectionServer
{
   STRUCT(Client , ConnectionServer::Client)
   //{
      Bool connection_version_ok;

      virtual Bool update();

      Client() {connection_version_ok=false;}
   };

   Client& client(Int i) {return (Client&)clients[i];}

   EditorServer() {clients.replaceClass<Client>();}
};
/******************************************************************************/
enum EDITOR_INTERFACE_COMMANDS
{
   EI_VERSION_CHECK,

   EI_GET_PROJECTS,
   EI_GET_PROJECT,
   EI_SET_PROJECT,

   EI_GET_DATA_PATH,

   EI_GET_ELMS,
   EI_GET_ELMS_SELECTED,
   EI_SET_ELMS_SELECTED,
   EI_RLD_ELMS,
   EI_RLD_ELMS_CANCEL,
   EI_RLD_ELMS_GET_RESULT,
   EI_RLD_ELMS_FORGET_RESULT,
   EI_NEW_ELM,
   EI_NEW_WORLD,
   EI_SET_ELM_NAME,
   EI_SET_ELM_REMOVED,
   EI_SET_ELM_PUBLISH,
   EI_SET_ELM_PARENT,
   EI_SET_ELM_SRC_FILE,

   EI_GET_WORLD,
   EI_SET_WORLD,
   EI_GET_WORLD_AREA_SIZE,
   EI_GET_WORLD_TERRAIN_RES,
   EI_GET_WORLD_TERRAIN_AREAS,
   EI_DEL_WORLD_TERRAIN,
   EI_GET_WORLD_TERRAIN_IS,
   EI_GET_WORLD_TERRAIN_HEIGHT,
   EI_SET_WORLD_TERRAIN_HEIGHT,
   EI_GET_WORLD_TERRAIN_COLOR,
   EI_SET_WORLD_TERRAIN_COLOR,
   EI_GET_WORLD_TERRAIN_MATERIAL,
   EI_SET_WORLD_TERRAIN_MATERIAL,
   EI_GET_WORLD_TERRAIN,
   EI_SET_WORLD_TERRAIN,

   EI_NEW_WORLD_OBJ,
   EI_GET_WORLD_OBJ_BASIC,
   EI_GET_WORLD_OBJ_FULL,

   EI_GET_WORLD_WAYPOINT_LIST,

   EI_GET_WORLD_CAM,
   EI_SET_WORLD_CAM,

   EI_DRAW_WORLD_LINES,

   EI_GET_IMAGE,
   EI_SET_IMAGE,
   EI_GET_CODE,
   EI_SET_CODE,
   EI_GET_FILE,
   EI_SET_FILE,
   EI_GET_MTRL_CUR,
   EI_SET_MTRL_CUR,
   EI_GET_MTRL,
   EI_SET_MTRL,
   EI_RLD_MTRL_TEX,
   EI_MUL_MTRL_TEX_COL,
   EI_GET_MESH,
   EI_SET_MESH,
   EI_GET_ANIM_CUR,
   EI_SET_ANIM_CUR,
   EI_GET_ANIM,
   EI_SET_ANIM,
   EI_GET_OBJ_CUR,
   EI_SET_OBJ_CUR,
   EI_GET_OBJ,
   EI_MODIFY_OBJ,
   EI_SET_ACTIVE_APP,
   EI_EXPORT_APP,
   EI_CODE_SYNC_IMPORT,
   EI_CODE_SYNC_EXPORT,

   EI_GET_PROJECTS_PATH,
   EI_SET_PROJECTS_PATH,

   EI_BUILD_DEBUG,
   EI_BUILD_32BIT,
   EI_BUILD_DX9  ,
   EI_BUILD_EXE  ,
   EI_BUILD_PATHS,

   EI_NUM,
#if EE_PRIVATE
   // !! when changing this, don't forget to increment EI_VER !!
#endif
};
enum EDITOR_INTERFACE_OBJ_COMMANDS : Byte
{
   EI_OBJ_NONE,
   EI_OBJ_PARAM_REMOVE_ID,
   EI_OBJ_PARAM_REMOVE_NAME,
   EI_OBJ_PARAM_RESTORE_ID,
   EI_OBJ_PARAM_RESTORE_NAME,
   EI_OBJ_PARAM_RENAME_ID,
   EI_OBJ_PARAM_RENAME_NAME,
   EI_OBJ_PARAM_SET_TYPE_VALUE_ID,
   EI_OBJ_PARAM_SET_TYPE_VALUE_NAME,
};
/******************************************************************************/
} // namespace
/******************************************************************************/
