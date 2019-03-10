/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
cchar8 *ElmTypeName[]= // !! these names are saved in Project TextData, do not change !!
{
   "None",
   "Folder",
   "Enum",
   "Image",
   "Image Atlas",
   "Font",
   "Text Style",
   "Panel",
   "Gui",
   "Shader",
   "Material",
   "Water Material",
   "Physics Material",
   "Object Class",
   "Object",
   "Mesh",
   "Skeleton",
   "Physical Body",
   "Animation",
   "Panel Image",
   "Icon",
   "Environment",
   "World",
   "Sound",
   "Video",
   "File",
   "Code",
   "Library",
   "Application",
   "Icon Settings",
   "Mini Map",
   "Gui Skin",
   "Any",
};
cchar8 *ElmTypeClass[]=
{
   "",
   "", // Folder
   "Enum, Enum*, Enums",
   "Image, ImagePtr, Images",
   "ImageAtlas, ImageAtlasPtr, ImageAtlases",
   "Font, Font*, Fonts",
   "TextStyle, TextStylePtr, TextStyles",
   "Panel, PanelPtr, Panels",
   "GuiObjs",
   "Shader, Shader*, Shaders",
   "Material, MaterialPtr, Materials",
   "WaterMtrl, WaterMtrlPtr, WaterMtrls",
   "PhysMtrl, PhysMtrl*, PhysMtrls",
   "Object, ObjectPtr, Objects",
   "Object, ObjectPtr, Objects",
   "Mesh, MeshPtr, Meshes",
   "Skeleton, Skeleton*, Skeletons",
   "PhysBody, PhysBodyPtr, PhysBodies",
   "Animation, Animation*, Animations",
   "PanelImage, PanelImagePtr, PanelImages",
   "Image, ImagePtr, Images",
   "Environment, EnvironmentPtr, Environments",
   "Game.WorldManager",
   "Sound",
   "Video",
   "File",
   "", // Code
   "", // Library
   "", // Application
   "", // Icon Settings
   "Game.MiniMap",
   "GuiSkin, GuiSkinPtr, GuiSkins",
};
cchar8 *ElmTypeDesc[]=
{
   null, //"None"
   "Folder is mainly used for better organization of project elements by storing multiple elements inside it.\nFolder doesn't have any special properties.",
   "Enum allows to create C++ style enums using a visual editor.\nEach enum that is created this way will be automatically available in the C++ codes.\nAdditionally it will be stored as a game file resource in project data.\nSuch enums can be used for Object parameter types and values.",
   "Image can be used for displaying it on the screen through codes, as Image of a Gui Object, as a Material texture or for other custom purpose.\nIt can be a 2D / 3D / Cube / Software texture.",
   "Image Atlas is a set of multiple Images stored in as few textures as possible.\nThis is achieved by combining multiple Images into the same texture.\nImage Atlas is perfectly suited for storing 2D sprite animations.",
   null, //"Font",
   "Text Style is a set of properties related to drawing text.\nSuch as font, color, alignment, shadow, etc.",
   "Panel is a set of properties related to drawing Gui Frames (like window borders).\nSuch as background Image, border, corner and top Images, shadow, etc.",
   "Gui is a set of multiple Gui Objects such as Windows, Buttons, etc.",
   null, //"Shader",
   "Material is a set of properties used for rendering 3D meshes.\nSuch as Color, Normal, Specular textures, opacity and various other parameters.",
   "Water Material  is a set of properties used for rendering 3D water surfaces.\nSuch as Color, Normal Textures, opacity and various other parameters.",
   "Physics Material is a set of properties for Physical Actors.\nSuch as Friction, Bounciness, etc.",
   "Object Class is a type of Object.\nEach Object can have only one Class.\nHere are few examples of Object Classes: Characters, Items, Vehicles, ..\nEach Object Class can have a set of base parameters that all Objects of this Class will automatically inherit.",
   "Object is a set of multiple properties, such as Mesh, Physical Body, Object Parameters, Object Class, etc.",
   null, //"Mesh",
   null, //"Skeleton",
   null, //"Physical Body",
   "Animation is a set of KeyFrames which are used for animating an Object.",
   "Panel Image is a specially created Image which supports stretching while preserving scale on borders, it is used for creating Panels (Gui Frames).",
   "Icon is a 2D Image which is made out of a 3D Object.",
   "Environment is a set of properties describing a World Environment, such as Sun, Clouds, Ambient Light, Fog, etc.\nEnvironment elements can be drag and dropped onto opened Worlds, to set them as default environment for edited world.",
   "World is a set of multiple World Areas in which can be stored Terrain Heightmaps, Objects, etc.",
   null, //"Sound",
   null, //"Video",
   "Custom binary file, allows you to include custom type of files.\nThey will be imported to the project directly in the same format as the source file.",
   "Source Code file which can contain functions, variables, classes, etc.",
   "Library is meant to work as a Folder for Source Code files which are used by all Applications in the Project.\nPlease store inside it only Source Code files and Folders.\nLibraries are useful for example in networked applications, where there is Client Application, Server Application and they both use some shared set of Codes.\nIn that case shared Codes should be stored separately outside of the Applications, inside Library element.\nThis will make all of the Code files from Library element to be accessible by all Applications.",
   "Application is a set of Source Code files which upon compilation create an executable, such as EXE DLL APP APK.\nIn general you should not store other elements than Folders and Codes in an Application.\nIncluding other elements in an Application will make them automatically embedded inside the executable file, making it bigger.\nBy default project resources are stored in separate \"Project.pak\" file.\nHowever for certain applications such as Installers it may be desirable to distribute only the executable file without any additional files.\nIn such case you can embed some of the resources such as Images used by the installer in its executable file, allowing it to access the resources without additional pak files.",
   "Icon Settings contains information required for creating 2D Icon elements from 3D Objects, such as icon resolution, camera placement, lighting, etc.\nOne Icon Settings element can be used for multiple Icons.",
   "Mini Map is a set of multiple Images of a World from the bird's eye view.",
   "Gui Skin is a set of images used to define a Graphical User Interface",
};

ElmTypeNameNoSpace ElmTypeNameNoSpaceDummy; // !! this needs to exist so that constructor is called !!

bool ElmCompressable(ELM_TYPE type) {return type==ELM_MESH || type==ELM_SKEL || type==ELM_PHYS || type==ELM_ANIM || type==ELM_FILE || type==ELM_FONT || type==ELM_PANEL_IMAGE || type==ELM_SOUND || type==ELM_GUI || type==ELM_IMAGE_ATLAS;} // if element file should be compressed before sending to server (don't compress ELM_IMAGE because it's already stored as JPG/PNG/WEBP), for 'ElmSendBoth' then only game version will be compressed, and edit will not (so it can be synchronized per-member on the server) !! don't change this for existing elements because this defines how element files are stored on the server (compressed or not) !!
bool ElmHasFile     (ELM_TYPE type) {return type==ELM_MESH || type==ELM_MTRL || type==ELM_SKEL || type==ELM_PHYS || type==ELM_ANIM || type==ELM_IMAGE || type==ELM_ICON || type==ELM_OBJ || type==ELM_ENUM || type==ELM_FILE || type==ELM_FONT || type==ELM_PANEL_IMAGE || type==ELM_GUI || type==ELM_SOUND || type==ELM_VIDEO || type==ELM_OBJ_CLASS || type==ELM_TEXT_STYLE || type==ELM_PANEL || type==ELM_GUI_SKIN || type==ELM_WATER_MTRL || type==ELM_PHYS_MTRL || type==ELM_ENV || type==ELM_ICON_SETTS || type==ELM_IMAGE_ATLAS/* || type==ELM_CODE*/;} // if element has a file, because ELM_CODE is synchronized manually, then it's not included here
bool ElmFileInShort (ELM_TYPE type) {return type==ELM_ENUM || type==ELM_FONT || type==ELM_PANEL_IMAGE || type==ELM_OBJ  || type==ELM_MTRL || type==ELM_TEXT_STYLE || type==ELM_PANEL || type==ELM_GUI_SKIN || type==ELM_OBJ_CLASS || type==ELM_WATER_MTRL || type==ELM_PHYS_MTRL || type==ELM_ENV || type==ELM_ICON_SETTS/* || type==ELM_CODE*/;} // if include file data in CS_SET_ELM_SHORT, because ELM_CODE is synchronized manually, then it's not included here
bool ElmEdit        (ELM_TYPE type) {return type==ELM_MESH || type==ELM_MTRL || type==ELM_SKEL || type==ELM_OBJ  || type==ELM_ENUM || type==ELM_FONT || type==ELM_PANEL_IMAGE || type==ELM_IMAGE || type==ELM_OBJ_CLASS || type==ELM_TEXT_STYLE || type==ELM_PANEL || type==ELM_GUI_SKIN || type==ELM_WATER_MTRL || type==ELM_PHYS_MTRL || type==ELM_ENV || type==ELM_ICON_SETTS;} // if element file is stored in the "edit" folder
bool ElmGame        (ELM_TYPE type) {return type==ELM_ENUM || type==ELM_IMAGE || type==ELM_IMAGE_ATLAS || type==ELM_FONT || type==ELM_TEXT_STYLE || type==ELM_PANEL || type==ELM_GUI || type==ELM_MTRL || type==ELM_WATER_MTRL || type==ELM_PHYS_MTRL || type==ELM_OBJ_CLASS || type==ELM_OBJ || type==ELM_MESH || type==ELM_SKEL || type==ELM_PHYS || type==ELM_ANIM || type==ELM_PANEL_IMAGE || type==ELM_ICON || type==ELM_ENV || type==ELM_SOUND || type==ELM_VIDEO || type==ELM_FILE || type==ELM_GUI_SKIN;} // if element file is stored in the "game" folder
bool ElmSendBoth    (ELM_TYPE type) {return type==ELM_SKEL || type==ELM_FONT || type==ELM_PANEL_IMAGE;} // if both Edit and Game file data should be sent in client<->server synchronization
bool ElmInFolder    (ELM_TYPE type) {return type==ELM_WORLD || type==ELM_MINI_MAP;} // if element data is stored inside a folder

bool ElmManualSync       (ELM_TYPE type) {return type==ELM_CODE  || type==ELM_APP;}
bool ElmPublish          (ELM_TYPE type) {return type!=ELM_NONE  && type!=ELM_FOLDER && type!=ELM_LIB && type!=ELM_APP && type!=ELM_CODE && type!=ELM_ICON_SETTS;} // if element should be included in publishing
bool ElmPublishNoCompress(ELM_TYPE type) {return type==ELM_SOUND || type==ELM_VIDEO;} // if element should have disabled compression when publishing data
bool ElmCanAccessID      (ELM_TYPE type) {return type!=ELM_NONE;} // allow ELM_CODE too if user wants to set/get code data, and ELM_APP if it will be used for activating app using Editor Interface

bool ElmImageLike (ELM_TYPE type              ) {return type==ELM_IMAGE || type==ELM_ICON;}
bool ElmCompatible(ELM_TYPE src, ELM_TYPE dest) {return src==dest || (ElmImageLike(src) && ElmImageLike(dest)) || (src==ELM_ANY || dest==ELM_ANY);}

bool ElmMovable        (ELM_TYPE type) {return type!=ELM_MESH && type!=ELM_SKEL && type!=ELM_PHYS;}
bool ElmVisible        (ELM_TYPE type) {return type!=ELM_MESH && type!=ELM_SKEL && type!=ELM_PHYS;}
bool ElmCanHaveChildren(ELM_TYPE type) {return type!=ELM_MESH && type!=ELM_SKEL && type!=ELM_PHYS;}

bool EqualID(C UID &id, C UID &src_id) {return id==src_id;}
bool NewerID(C UID &id, C UID &src_id) {return Compare(id, src_id)>0;} // if 'id' is newer than 'src_id'. Since no timestamp is provided for this function, it will choose the ID which is bigger, so that 'UIDZero' is always replaced
bool  SyncID(  UID &id, C UID &src_id) {if(NewerID(src_id, id)){id=src_id; return true;} return false;}
bool  UndoID(  UID &id, C UID &src_id) {if(NewerID(src_id, id)){id=src_id; return true;} return false;}
/******************************************************************************/

/******************************************************************************/
   NameDesc ElmImage::ImageTypes[]= // !! These names are saved !!
   {
      {u"Compressed" , u"Image will be stored as a compressed texture\nLower quality, smaller size, 4/8-bit per pixel"},
      {u"Compressed+", u"Image will be stored as a compressed texture\nGood quality, small size, 8-bit per pixel"},
      {u"Full"       , u"Image will be stored in full size\nBest quality, biggest size, 32-bit per pixel"},
      {u"Alpha"      , u"Image will be stored as Alpha channel only\n8-bit per pixel"},
   };
   int ElmImage::ImageTypesElms=Elms(ImageTypes);
   ::ElmImage::ImageMode ElmImage::ImageModes[]= // !! These names are saved !!
   {
      {IMAGE_SOFT, "Soft"},
      {IMAGE_2D  , "2D"  },
      {IMAGE_3D  , "3D"  },
      {IMAGE_CUBE, "Cube"},
   };
   const uint ElmIcon::InheritVariation=~0;
   ::ElmApp::StorageMode ElmApp::StorageModes[]= // !! These names are saved !!
   {
      {Edit::STORAGE_INTERNAL, "Internal"},
      {Edit::STORAGE_EXTERNAL, "External"},
      {Edit::STORAGE_AUTO    , "Auto"    },
   };
/******************************************************************************/
   ElmTypeNameNoSpace::ElmTypeNameNoSpace() {REPAO(names)=Replace(ElmTypeName[i], ' ', '\0');}
   bool ElmData::equal(C ElmData &src)C {return src_file_time==src.src_file_time;}
   bool ElmData::newer(C ElmData &src)C {return src_file_time> src.src_file_time;}
   bool ElmData::mayContain(C UID &id)C {return true;}
   bool ElmData::containsTex(C UID &id, bool test_merged)C {return false;}
   void ElmData::listTexs(MemPtr<UID> texs)C {}
           void ElmData::newVer() {ver.randomize();}
   void ElmData::newData() {newVer(); src_file_time++;}
   void  ElmData::setSrcFile(C Str &src_file, C TimeStamp &time) {T.src_file=src_file; T.src_file_time=time;}
   uint ElmData::undoSrcFile(C ElmData &src) {return Undo(src_file_time, src.src_file_time, src_file, src.src_file)*CHANGE_NORMAL;}
   uint ElmData::undo(C ElmData &src)
   {
      uint changed=0;
    //changed|=undoSrcFile(src); // this should not be undone here, because this method is called inside the element editors, however none of these editors provide GUI for the 'src_file', instead the 'src_file' is changed using a separate 'ReloadElm' GUI. So if Element is opened in its Editor, some change is made, then it's reloaded from different source file, and in Editor some change is made, and undos presed to the start, then original source file would be restored, which is not what we want
      return changed;
   }
   uint ElmData::sync(C ElmData &src)
   {
      return Sync(src_file_time, src.src_file_time, src_file, src.src_file)*CHANGE_NORMAL;
   }
   void ElmData::clearLinked() {}
   bool ElmData::save(File &f)C
   {
      f.cmpUIntV(2);
      f<<ver<<src_file<<src_file_time;
      return f.ok();
   }
   bool ElmData::load(File &f)
   {
      switch(f.decUIntV())
      {
         case 2:
         {
            f>>ver>>src_file>>src_file_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            f>>ver; GetStr2(f, src_file); f>>src_file_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>ver;
            GetStr(f, src_file);
            src_file_time=src_file.is();
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmData::save(MemPtr<TextNode> nodes)C
   {
                            nodes.New().set("Version"    , ver.ver);
      if(src_file     .is())nodes.New().set("SrcFile"    , src_file);
      if(src_file_time.is())nodes.New().set("SrcFileTime", src_file_time.text());
   }
   void ElmData::load(C MemPtr<TextNode> &nodes)
   {
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Version"    )n.getValue(ver.ver );else
         if(n.name=="SrcFile"    )n.getValue(src_file);else
         if(n.name=="SrcFileTime")src_file_time=n.asText();
      }
   }
   ElmData::~ElmData() {}
   bool     ElmObjClass::ovrAccess()C {return FlagTest( flag, OVR_ACCESS           );}
   void ElmObjClass::ovrAccess(bool     on  ) {FlagSet(flag, OVR_ACCESS, on);}
   bool     ElmObjClass::terrain()C {return FlagTest( flag, TERRAIN              );}
   void ElmObjClass::terrain(bool     on  ) {FlagSet(flag, TERRAIN   , on);}
   bool     ElmObjClass::ovrPath()C {return FlagTest( flag, OVR_PATH             );}
   void ElmObjClass::ovrPath(bool     on  ) {FlagSet(flag, OVR_PATH  , on);}
   OBJ_PATH ElmObjClass::pathSelf()C {return OBJ_PATH((flag>>PATH_SHIFT)&PATH_MASK);}
   void ElmObjClass::pathSelf(OBJ_PATH path) {FlagDisable(flag, PATH_MASK<<PATH_SHIFT); flag|=((path&PATH_MASK)<<PATH_SHIFT);}
   void ElmObjClass::from(C EditObject &params)
   {
      flag=0;
      ovrAccess(FlagTest(params.flag, EditObject::OVR_ACCESS)); terrain (params.access==OBJ_ACCESS_TERRAIN);
      ovrPath  (FlagTest(params.flag, EditObject::OVR_PATH  )); pathSelf(params.path                      );
   }
   uint ElmObjClass::undo(C ElmObjClass &src) {return ::ElmData::undo(src);}
   uint ElmObjClass::sync(C ElmObjClass &src) {return ::ElmData::sync(src);}
   bool ElmObjClass::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<flag;
      return f.ok();
   }
   bool ElmObjClass::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>flag;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmObjClass::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(ovrAccess())nodes.New().set("OverrideAccess");
      if(ovrPath  ())nodes.New().set("OverridePath"  );
                     nodes.New().set("Terrain", terrain ());
                     nodes.New().set("Path"   , pathSelf());
   }
   void ElmObjClass::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="OverrideAccess")ovrAccess(          n.asBool1());else
         if(n.name=="OverridePath"  )ovrPath  (          n.asBool1());else
         if(n.name=="Terrain"       )terrain  (          n.asBool1());else
         if(n.name=="Path"          )pathSelf ((OBJ_PATH)n.asUInt ());
      }
   }
   bool ElmObj::equal(C ElmObj &src)C {return ::ElmData::equal(src) && EqualID(mesh_id, src.mesh_id);}
   bool ElmObj::newer(C ElmObj &src)C {return ::ElmData::newer(src) || NewerID(mesh_id, src.mesh_id);}
   void ElmObj::clearLinked(){mesh_id.zero();}
   uint ElmObj::undo(C ElmObj &src)
   {
      uint   changed =::ElmObjClass::undo(src);
             changed|=UndoID(mesh_id, src.mesh_id)*CHANGE_NORMAL;
      return changed; // don't adjust 'ver' here because it also relies on 'EditObject', because of that this is included in 'ElmFileInShort'
   }
   uint ElmObj::sync(C ElmObj &src)
   {
      uint   changed =::ElmObjClass::sync(src);
             changed|=SyncID(mesh_id, src.mesh_id)*CHANGE_NORMAL;
      return changed; // don't adjust 'ver' here because it also relies on 'EditObject', because of that this is included in 'ElmFileInShort'
   }
   void ElmObj::from(C EditObject &params)
   {
      ::ElmObjClass::from(params);
      base_id=params.base.id();
   }
   bool ElmObj::save(File &f)C 
{
      ::ElmObjClass::save(f);
      f.cmpUIntV(0);
      f<<mesh_id<<base_id;
      return f.ok();
   }
   bool ElmObj::load(File &f)
{
      if(::ElmObjClass::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>mesh_id>>base_id;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmObj::save(MemPtr<TextNode> nodes)C 
{
      ::ElmObjClass::save(nodes);
      if(mesh_id.valid())nodes.New().setFN("Mesh", mesh_id);
      if(base_id.valid())nodes.New().setFN("Base", base_id);
   }
   void ElmObj::load(C MemPtr<TextNode> &nodes)
{
      ::ElmObjClass::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Mesh")n.getValue(mesh_id);else
         if(n.name=="Base")n.getValue(base_id);
      }
   }
   bool ElmMesh::equal(C ElmMesh &src)C {return ::ElmData::equal(src) && file_time==src.file_time && body_time==src.body_time && draw_group_time==src.draw_group_time && transform_time==src.transform_time && EqualID(obj_id, src.obj_id) && EqualID(skel_id, src.skel_id) && EqualID(phys_id, src.phys_id);}
   bool ElmMesh::newer(C ElmMesh &src)C {return ::ElmData::newer(src) || file_time> src.file_time || body_time> src.body_time || draw_group_time> src.draw_group_time || transform_time> src.transform_time || NewerID(obj_id, src.obj_id) || NewerID(skel_id, src.skel_id) || NewerID(phys_id, src.phys_id);}
   bool ElmMesh::mayContain(C UID &id)C {return id==obj_id || id==skel_id || id==phys_id || id==body_id || id==draw_group_id || mtrl_ids.binaryHas(id, Compare);}
   void ElmMesh::clearLinked(){obj_id.zero(); skel_id.zero(); phys_id.zero();}
   flt  ElmMesh::posScale()C {return 1/transform.scale;}
   flt  ElmMesh::vtxDupPosEps()C {return VtxDupPosEps*posScale();}
   bool ElmMesh::canHaveCustomTransform()C {return !body_id.valid();}
   void ElmMesh::newData()
{
      ::ElmData::newData();
      file_time++;
      body_time++;
      draw_group_time++;
      transform_time++;
   }
   void ElmMesh::fromMtrl(C Mesh &mesh)
   {
      mtrl_ids.clear();
      REPD(l, mesh.lods())
      {
       C MeshLod &lod=mesh.lod(l); REPA(lod)
         {
          C MeshPart &part=lod.parts[i];
            if(!(part.part_flag&MSHP_HIDDEN)) // skip hidden parts
            {
               REP(4)if(C MaterialPtr &mtrl=part.multiMaterial(i))
               {
                  UID mtrl_id=mtrl.id();
                  if( mtrl_id.valid())mtrl_ids.binaryInclude(mtrl_id, Compare);
               }
               REP(part.variations())if(i)if(C MaterialPtr &mtrl=part.variation(i))
               {
                  UID mtrl_id=mtrl.id();
                  if( mtrl_id.valid())mtrl_ids.binaryInclude(mtrl_id, Compare);
               }
            }
         }
      }
   }
   void ElmMesh::from(C Mesh &game_mesh) // game version of mesh must be passed here
   {
      fromMtrl(game_mesh);
      if(game_mesh.is())box=game_mesh.ext;else box.set(Vec(0), Vec(-1));
   }
   uint ElmMesh::undo(C ElmMesh &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=UndoID( obj_id, src. obj_id)*CHANGE_NORMAL;
      changed|=UndoID(skel_id, src.skel_id)*CHANGE_AFFECT_FILE;
      changed|=UndoID(phys_id, src.phys_id)*CHANGE_NORMAL;
      changed|=Undo(      body_time, src.      body_time,       body_id, src.      body_id)*CHANGE_AFFECT_FILE;
      changed|=Undo(draw_group_time, src.draw_group_time, draw_group_id, src.draw_group_id)*CHANGE_AFFECT_FILE;
    //changed|=Undo( transform_time, src. transform_time, transform    , src.transform    )*CHANGE_AFFECT_FILE; transform is not undone here

      if(changed)newVer();
      return changed;
   }
   uint ElmMesh::sync(C ElmMesh &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=SyncID( obj_id, src. obj_id)*CHANGE_NORMAL;
      changed|=SyncID(skel_id, src.skel_id)*CHANGE_AFFECT_FILE;
      changed|=SyncID(phys_id, src.phys_id)*CHANGE_NORMAL;
      changed|=Sync(      body_time, src.      body_time,       body_id, src.      body_id)*CHANGE_AFFECT_FILE;
      changed|=Sync(draw_group_time, src.draw_group_time, draw_group_id, src.draw_group_id)*CHANGE_AFFECT_FILE;
      changed|=Sync( transform_time, src. transform_time, transform    , src.transform    )*CHANGE_AFFECT_FILE;

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmMesh::syncFile(C ElmMesh &src)
   {
      bool changed=false;

      if(Sync(file_time, src.file_time)){changed|=true; mtrl_ids=src.mtrl_ids;}

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmMesh::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(2);
      f<<transform<<obj_id<<skel_id<<body_id<<phys_id<<draw_group_id; f.cmpUIntV(mtrl_ids.elms()); FREPA(mtrl_ids)f<<mtrl_ids[i]; f<<box;
      f<<file_time<<body_time<<transform_time<<draw_group_time;
      return f.ok();
   }
   bool ElmMesh::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 2:
         {
            f>>transform>>obj_id>>skel_id>>body_id>>phys_id>>draw_group_id; mtrl_ids.setNum(f.decUIntV()); FREPA(mtrl_ids)f>>mtrl_ids[i]; f>>box;
            f>>file_time>>body_time>>transform_time>>draw_group_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            obj_id.zero();
            f>>transform>>skel_id>>body_id>>phys_id>>draw_group_id; mtrl_ids.setNum(f.decUIntV()); FREPA(mtrl_ids)f>>mtrl_ids[i]; f>>box;
            f>>file_time>>body_time>>transform_time>>draw_group_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            obj_id.zero();
            f>>transform>>skel_id>>body_id>>phys_id; draw_group_id.zero(); mtrl_ids.setNum(f.decUIntV()); FREPA(mtrl_ids)f>>mtrl_ids[i]; f>>box;
            f>>file_time>>body_time>>transform_time; draw_group_time.zero();
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmMesh::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
                               nodes.New().setRaw("Pose"         , transform);
                               nodes.New().setRaw("Box"          , box      );
      if(       obj_id.valid())nodes.New().setFN ("Object"       ,  obj_id  );
      if(      skel_id.valid())nodes.New().setFN ("Skeleton"     , skel_id  );
      if(      phys_id.valid())nodes.New().setFN ("PhysBody"     , phys_id  );
      if(      body_id.valid())nodes.New().setFN ("Body"         , body_id  );
      if(draw_group_id.valid())nodes.New().setFN ("DrawGroup"    , draw_group_id);
                               nodes.New().set   ("FileTime"     ,       file_time.text());
                               nodes.New().set   ("BodyTime"     ,       body_time.text());
                               nodes.New().set   ("DrawGroupTime", draw_group_time.text());
                               nodes.New().set   ("PoseTime"     ,  transform_time.text());
      if(mtrl_ids.elms())
      {
         TextNode &mtrls=nodes.New().setName("Materials");
         FREPA(mtrl_ids)mtrls.nodes.New().setValueFN(mtrl_ids[i]); // list in order
      }
   }
   void ElmMesh::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Pose"         )n.getValueRaw(transform    );else
         if(n.name=="Box"          )n.getValueRaw(box          );else
         if(n.name=="Object"       )n.getValue   ( obj_id      );else
         if(n.name=="Skeleton"     )n.getValue   (skel_id      );else
         if(n.name=="PhysBody"     )n.getValue   (phys_id      );else
         if(n.name=="Body"         )n.getValue   (body_id      );else
         if(n.name=="DrawGroup"    )n.getValue   (draw_group_id);else
         if(n.name=="FileTime"     )      file_time=n.asText();else
         if(n.name=="BodyTime"     )      body_time=n.asText();else
         if(n.name=="DrawGroupTime")draw_group_time=n.asText();else
         if(n.name=="PoseTime"     ) transform_time=n.asText();else
         if(n.name=="Materials"    )
         {
            FREPA(n.nodes) // get in order
            {
               UID id; if(n.nodes[i].getValue(id) && id.valid())mtrl_ids.binaryInclude(id, Compare);
            }
         }
      }
   }
   bool ElmMaterial::equal(C ElmMaterial &src)C {return ::ElmData::equal(src);}
   bool ElmMaterial::newer(C ElmMaterial &src)C {return ::ElmData::newer(src);}
   bool ElmMaterial::usesTexAlpha()C {return FlagTest(flag, USES_TEX_ALPHA );}
   void ElmMaterial::usesTexAlpha(bool on) {return FlagSet(flag, USES_TEX_ALPHA , on);}
   bool ElmMaterial::usesTexBump()C {return FlagTest(flag, USES_TEX_BUMP  );}
   void ElmMaterial::usesTexBump(bool on) {return FlagSet(flag, USES_TEX_BUMP  , on);}
   bool ElmMaterial::usesTexGlow()C {return FlagTest(flag, USES_TEX_GLOW  );}
   void ElmMaterial::usesTexGlow(bool on) {return FlagSet(flag, USES_TEX_GLOW  , on);}
   bool ElmMaterial::texQualityiOS()C {return FlagTest(flag, TEX_QUALITY_IOS);}
   void ElmMaterial::texQualityiOS(bool on) {return FlagSet(flag, TEX_QUALITY_IOS, on);}
   bool ElmMaterial::mayContain(C UID &id)C {return false;}
   bool ElmMaterial::containsTex(C UID &id, bool test_merged)C 
{
      if(id.valid())
      {
         if(base_0_tex==id || base_1_tex==id || detail_tex==id || macro_tex==id || reflection_tex==id || light_tex==id)return true;
         if(test_merged && MergedBaseTexturesID(base_0_tex, base_1_tex)==id)return true;
      }
      return false;
   }
   void ElmMaterial::listTexs(MemPtr<UID> texs)C 
{
      if(    base_0_tex.valid())texs.binaryInclude(    base_0_tex, Compare);
      if(    base_1_tex.valid())texs.binaryInclude(    base_1_tex, Compare);
      if(    detail_tex.valid())texs.binaryInclude(    detail_tex, Compare);
      if(     macro_tex.valid())texs.binaryInclude(     macro_tex, Compare);
      if(reflection_tex.valid())texs.binaryInclude(reflection_tex, Compare);
      if(     light_tex.valid())texs.binaryInclude(     light_tex, Compare);
   }
   void ElmMaterial::from(C EditMaterial &src)
   {
          base_0_tex=src.    base_0_tex;
          base_1_tex=src.    base_1_tex;
          detail_tex=src.    detail_tex;
           macro_tex=src.     macro_tex;
      reflection_tex=src.reflection_tex;
           light_tex=src.     light_tex;

      downsize_tex_mobile=src.downsize_tex_mobile;

      usesTexAlpha (src.usesTexAlpha());
      usesTexBump  (src.usesTexBump ());
      usesTexGlow  (src.usesTexGlow ());
      texQualityiOS(src.high_quality_ios);
   }
   uint ElmMaterial::undo(C ElmMaterial &src) // don't undo 'downsize_tex_mobile', 'flag' because they should be set only in 'from'
   {
      uint   changed=::ElmData::undo(src);
      return changed; // don't adjust 'ver' here because it also relies on 'EditMaterial', because of that this is included in 'ElmFileInShort'
   }
   uint ElmMaterial::sync(C ElmMaterial &src) // don't sync 'downsize_tex_mobile', 'flag' because they should be set only in 'from'
   {
      uint   changed=::ElmData::sync(src);
      return changed; // don't adjust 'ver' here because it also relies on 'EditMaterial', because of that this is included in 'ElmFileInShort'
   }
   bool ElmMaterial::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(4);
      f<<base_0_tex<<base_1_tex<<detail_tex<<macro_tex<<reflection_tex<<light_tex<<downsize_tex_mobile<<flag;
      return f.ok();
   }
   bool ElmMaterial::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 4:
         {
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex>>downsize_tex_mobile>>flag;
            if(f.ok())return true;
         }break;

         case 3:
         {
            byte max_tex_size; f>>max_tex_size>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex; downsize_tex_mobile=(max_tex_size>=1 && max_tex_size<=10); flag=0;
            if(f.ok())return true;
         }break;

         case 2:
         {
            byte max_tex_size; UID mesh_id; f>>max_tex_size>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex>>mesh_id; downsize_tex_mobile=(max_tex_size>=1 && max_tex_size<=10); flag=0;
            if(f.ok())return true;
         }break;

         case 1:
         {
            UID mesh_id; f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>light_tex>>mesh_id; downsize_tex_mobile=0; flag=0;
            if(f.ok())return true;
         }break;

         case 0:
         {
            UID mesh_id; f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>reflection_tex>>mesh_id; downsize_tex_mobile=0; flag=0; light_tex.zero();
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmMaterial::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(    base_0_tex.valid())nodes.New().setFN("Base0"            ,     base_0_tex);
      if(    base_1_tex.valid())nodes.New().setFN("Base1"            ,     base_1_tex);
      if(    detail_tex.valid())nodes.New().setFN("Detail"           ,     detail_tex);
      if(     macro_tex.valid())nodes.New().setFN("Macro"            ,      macro_tex);
      if(reflection_tex.valid())nodes.New().setFN("Reflection"       , reflection_tex);
      if(     light_tex.valid())nodes.New().setFN("Light"            ,      light_tex);
      if(  downsize_tex_mobile )nodes.New().set  ("MobileTexDownsize", downsize_tex_mobile);
                                nodes.New().set  ("iOSTexQuality"    , texQualityiOS());
      if(usesTexAlpha())nodes.New().set("UsesTexAlpha");
      if(usesTexBump ())nodes.New().set("UsesTexBump" );
      if(usesTexGlow ())nodes.New().set("UsesTexGlow" );
   }
   void ElmMaterial::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Base0"            )n.getValue(    base_0_tex);else
         if(n.name=="Base1"            )n.getValue(    base_1_tex);else
         if(n.name=="Detail"           )n.getValue(    detail_tex);else
         if(n.name=="Macro"            )n.getValue(     macro_tex);else
         if(n.name=="Reflection"       )n.getValue(reflection_tex);else
         if(n.name=="Light"            )n.getValue(     light_tex);else
         if(n.name=="MobileTexDownsize")downsize_tex_mobile    =n.asInt () ;else
         if(n.name=="iOSTexQuality"    )texQualityiOS          (n.asBool());else
         if(n.name=="UsesTexAlpha"     )FlagSet(flag, USES_TEX_ALPHA, n.asBool1());else
         if(n.name=="UsesTexBump"      )FlagSet(flag, USES_TEX_BUMP , n.asBool1());else
         if(n.name=="UsesTexGlow"      )FlagSet(flag, USES_TEX_GLOW , n.asBool1());
      }
   }
   bool ElmWaterMtrl::equal(C ElmMaterial &src)C {return ::ElmData::equal(src);}
   bool ElmWaterMtrl::newer(C ElmMaterial &src)C {return ::ElmData::newer(src);}
   bool ElmWaterMtrl::usesTexAlpha()C {return FlagTest(flag, USES_TEX_ALPHA );}
   void ElmWaterMtrl::usesTexAlpha(bool on) {return FlagSet(flag, USES_TEX_ALPHA , on);}
   bool ElmWaterMtrl::usesTexBump()C {return FlagTest(flag, USES_TEX_BUMP  );}
   void ElmWaterMtrl::usesTexBump(bool on) {return FlagSet(flag, USES_TEX_BUMP  , on);}
   bool ElmWaterMtrl::usesTexGlow()C {return FlagTest(flag, USES_TEX_GLOW  );}
   void ElmWaterMtrl::usesTexGlow(bool on) {return FlagSet(flag, USES_TEX_GLOW  , on);}
   bool ElmWaterMtrl::texQualityiOS()C {return FlagTest(flag, TEX_QUALITY_IOS);}
   void ElmWaterMtrl::texQualityiOS(bool on) {return FlagSet(flag, TEX_QUALITY_IOS, on);}
   bool ElmWaterMtrl::equal(C ElmWaterMtrl &src)C {return ::ElmData::equal(src);}
   bool ElmWaterMtrl::newer(C ElmWaterMtrl &src)C {return ::ElmData::newer(src);}
   bool ElmWaterMtrl::mayContain(C UID &id)C {return false;}
   bool ElmWaterMtrl::containsTex(C UID &id, bool test_merged)C 
{
      if(id.valid())
      {
         if(base_0_tex==id || base_1_tex==id || reflection_tex==id)return true;
         if(test_merged && MergedBaseTexturesID(base_0_tex, base_1_tex)==id)return true;
      }
      return false;
   }
   void ElmWaterMtrl::listTexs(MemPtr<UID> texs)C 
{
      if(    base_0_tex.valid())texs.binaryInclude(    base_0_tex, Compare);
      if(    base_1_tex.valid())texs.binaryInclude(    base_1_tex, Compare);
      if(reflection_tex.valid())texs.binaryInclude(reflection_tex, Compare);
   }
   void ElmWaterMtrl::from(C EditWaterMtrl &src)
   {
          base_0_tex=src.    base_0_tex;
          base_1_tex=src.    base_1_tex;
      reflection_tex=src.reflection_tex;

      usesTexAlpha (src.usesTexAlpha());
      usesTexBump  (src.usesTexBump ());
      usesTexGlow  (src.usesTexGlow ());
      texQualityiOS(src.high_quality_ios);
   }
   uint ElmWaterMtrl::undo(C ElmWaterMtrl &src) {return ::ElmData::undo(src);}
   uint ElmWaterMtrl::sync(C ElmWaterMtrl &src) {return ::ElmData::sync(src);}
   bool ElmWaterMtrl::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(1);
      f<<base_0_tex<<base_1_tex<<reflection_tex<<flag;
      return f.ok();
   }
   bool ElmWaterMtrl::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 1:
         {
            f>>base_0_tex>>base_1_tex>>reflection_tex>>flag;
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>base_0_tex>>base_1_tex>>reflection_tex; flag=0;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmWaterMtrl::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(    base_0_tex.valid())nodes.New().setFN("Base0"        ,     base_0_tex);
      if(    base_1_tex.valid())nodes.New().setFN("Base1"        ,     base_1_tex);
      if(reflection_tex.valid())nodes.New().setFN("Reflection"   , reflection_tex);
                                nodes.New().set  ("iOSTexQuality", texQualityiOS());
      if(usesTexAlpha()        )nodes.New().set  ("UsesTexAlpha");
      if(usesTexBump ()        )nodes.New().set  ("UsesTexBump" );
      if(usesTexGlow ()        )nodes.New().set  ("UsesTexGlow" );
   }
   void ElmWaterMtrl::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Base0"        )n.getValue(    base_0_tex);else
         if(n.name=="Base1"        )n.getValue(    base_1_tex);else
         if(n.name=="Reflection"   )n.getValue(reflection_tex);else
         if(n.name=="iOSTexQuality")texQualityiOS(n.asBool());else
         if(n.name=="UsesTexAlpha" )FlagSet(flag, USES_TEX_ALPHA, n.asBool1());else
         if(n.name=="UsesTexBump"  )FlagSet(flag, USES_TEX_BUMP , n.asBool1());else
         if(n.name=="UsesTexGlow"  )FlagSet(flag, USES_TEX_GLOW , n.asBool1());
      }
   }
   bool ElmPhysMtrl::equal(C ElmPhysMtrl &src)C {return ::ElmData::equal(src);}
   bool ElmPhysMtrl::newer(C ElmPhysMtrl &src)C {return ::ElmData::newer(src);}
   bool ElmPhysMtrl::mayContain(C UID &id)C {return false;}
   void ElmPhysMtrl::from(C EditPhysMtrl &src) {}
   uint ElmPhysMtrl::undo(C  ElmPhysMtrl &src) {return ::ElmData::undo(src);}
   uint ElmPhysMtrl::sync(C  ElmPhysMtrl &src) {return ::ElmData::sync(src);}
   bool ElmPhysMtrl::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      return f.ok();
   }
   bool ElmPhysMtrl::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmPhysMtrl::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
   }
   void ElmPhysMtrl::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
      }
   }
   bool ElmSkel::equal(C ElmSkel &src)C {return ::ElmData::equal(src) && file_time==src.file_time && EqualID(mesh_id, src.mesh_id);}
   bool ElmSkel::newer(C ElmSkel &src)C {return ::ElmData::newer(src) || file_time> src.file_time || NewerID(mesh_id, src.mesh_id);}
   bool ElmSkel::mayContain(C UID &id)C {return id==mesh_id;}
   void ElmSkel::clearLinked(         )  {mesh_id.zero();}
   void ElmSkel::newData()
{
      ::ElmData::newData();
      file_time++;
   }
   uint ElmSkel::undo(C ElmSkel &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=UndoID(mesh_id, src.mesh_id)*CHANGE_NORMAL;

      if(changed)newVer();
      return changed;
   }
   uint ElmSkel::sync(C ElmSkel &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=SyncID(mesh_id, src.mesh_id)*CHANGE_NORMAL;

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmSkel::syncFile(C ElmSkel &src)
   {
      bool changed=false;

      if(Sync(file_time, src.file_time)){changed|=true; transform=src.transform;}

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmSkel::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<mesh_id<<transform<<file_time;
      return f.ok();
   }
   bool ElmSkel::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>mesh_id>>transform>>file_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmSkel::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
                         nodes.New().setRaw("Pose"    , transform);
      if(mesh_id.valid())nodes.New().setFN ("Mesh"    , mesh_id);
                         nodes.New().set   ("FileTime", file_time.text());
   }
   void ElmSkel::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Pose"    )n.getValueRaw(transform);else
         if(n.name=="Mesh"    )n.getValue   (mesh_id  );else
         if(n.name=="FileTime")file_time=n.asText();
      }
   }
   bool ElmPhys::hasBody()C {return box.valid();}
   bool ElmPhys::equal(C ElmPhys &src)C {return ::ElmData::equal(src) && file_time==src.file_time && mtrl_time==src.mtrl_time && density_time==src.density_time && EqualID(mesh_id, src.mesh_id);}
   bool ElmPhys::newer(C ElmPhys &src)C {return ::ElmData::newer(src) || file_time> src.file_time || mtrl_time> src.mtrl_time || density_time> src.density_time || NewerID(mesh_id, src.mesh_id);}
   bool ElmPhys::mayContain(C UID &id)C {return id==mesh_id || id==mtrl_id;}
   void ElmPhys::clearLinked(         )  {mesh_id.zero();}
   void ElmPhys::newData()
{
      ::ElmData::newData();
      file_time++;
      mtrl_time++;
      density_time++;
   }
   uint ElmPhys::undo(C ElmPhys &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=UndoID(mesh_id, src.mesh_id)*CHANGE_NORMAL;
      changed|=Undo(   mtrl_time, src.   mtrl_time, mtrl_id, src.mtrl_id)*CHANGE_AFFECT_FILE;
      changed|=Undo(density_time, src.density_time, density, src.density)*CHANGE_AFFECT_FILE;

      if(Undo(file_time, src.file_time)){changed|=CHANGE_AFFECT_FILE; transform=src.transform; box=src.box;}

      if(changed)newVer();
      return changed;
   }
   uint ElmPhys::sync(C ElmPhys &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=SyncID(mesh_id, src.mesh_id)*CHANGE_NORMAL;
      changed|=Sync(   mtrl_time, src.   mtrl_time, mtrl_id, src.mtrl_id)*CHANGE_AFFECT_FILE;
      changed|=Sync(density_time, src.density_time, density, src.density)*CHANGE_AFFECT_FILE;

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmPhys::syncFile(C ElmPhys &src)
   {
      bool changed=false;

      if(Sync(file_time, src.file_time)){changed|=true; transform=src.transform; box=src.box;}

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   void ElmPhys::from(C PhysBody &game_phys) // game version of phys must be passed here
   {
      if(game_phys.is())box=game_phys.box;else box.set(Vec(0), Vec(-1));
   }
   bool ElmPhys::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<mesh_id<<mtrl_id<<density<<transform<<box<<file_time<<mtrl_time<<density_time;
      return f.ok();
   }
   bool ElmPhys::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>mesh_id>>mtrl_id>>density>>transform>>box>>file_time>>mtrl_time>>density_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmPhys::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
                         nodes.New().setRaw("Pose"        , transform);
                         nodes.New().setRaw("Box"         , box      );
                         nodes.New().setRaw("Density"     , density  );
      if(mesh_id.valid())nodes.New().setFN ("Mesh"        , mesh_id  );
      if(mtrl_id.valid())nodes.New().setFN ("Material"    , mtrl_id  );
                         nodes.New().set   ("FileTime"    ,    file_time.text());
                         nodes.New().set   ("MaterialTime",    mtrl_time.text());
                         nodes.New().set   ("DensityTime" , density_time.text());
   }
   void ElmPhys::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Pose"        )n.getValueRaw(transform);else
         if(n.name=="Box"         )n.getValueRaw(box      );else
         if(n.name=="Density"     )n.getValueRaw(density  );else
         if(n.name=="Mesh"        )n.getValue   (mesh_id  );else
         if(n.name=="Material"    )n.getValue   (mtrl_id  );else
         if(n.name=="FileTime"    )   file_time=n.asText();else
         if(n.name=="MaterialTime")   mtrl_time=n.asText();else
         if(n.name=="DensityTime" )density_time=n.asText();
      }
   }
   bool ElmAnim::loop()C {return FlagTest(flag, LOOP  );}
   ElmAnim& ElmAnim::loop(bool on) {FlagSet(flag, LOOP  , on); return T;}
   bool ElmAnim::linear()C {return FlagTest(flag, LINEAR);}
   ElmAnim& ElmAnim::linear(bool on) {FlagSet(flag, LINEAR, on); return T;}
   bool ElmAnim::equal(C ElmAnim &src)C {return ::ElmData::equal(src) && loop_time==src.loop_time && linear_time==src.linear_time && skel_time==src.skel_time && file_time==src.file_time;}
   bool ElmAnim::newer(C ElmAnim &src)C {return ::ElmData::newer(src) || loop_time> src.loop_time || linear_time> src.linear_time || skel_time> src.skel_time || file_time> src.file_time;}
   bool ElmAnim::rootMove(           )C {return !EqualMem(root_move, VecZero);}
   bool ElmAnim::rootRot(           )C {return !EqualMem(root_rot , VecZero);}
   void ElmAnim::rootMoveZero(           )  {root_move.zero();}
   void ElmAnim::rootRotZero(           )  {root_rot .zero();}
   void ElmAnim::rootMove(C Vec &move)  {root_move=move; if(!rootMove())CHSSB(root_move.x);}
   void ElmAnim::rootRot(C Vec &rot )  {root_rot =rot ; if(!rootRot ())CHSSB(root_rot .x);}
   uint ElmAnim::rootFlags(           )C
   {
      uint f=0;
      if(flag&ROOT_DEL_POS_X)f|=::ROOT_DEL_POSITION_X;
      if(flag&ROOT_DEL_POS_Y)f|=::ROOT_DEL_POSITION_Y;
      if(flag&ROOT_DEL_POS_Z)f|=::ROOT_DEL_POSITION_Z;
      if(flag&ROOT_DEL_ROT_X)f|=::ROOT_DEL_ROTATION_X;
      if(flag&ROOT_DEL_ROT_Y)f|=::ROOT_DEL_ROTATION_Y;
      if(flag&ROOT_DEL_ROT_Z)f|=::ROOT_DEL_ROTATION_Z;
      if(flag&ROOT_2_KEYS   )f|=::ROOT_2_KEYS;
      if(flag&ROOT_FROM_BODY)f|=::ROOT_BONE_POSITION|::ROOT_START_IDENTITY;
      return f;
   }
   void ElmAnim::setRoot(Animation &anim) {SetRootMoveRot(anim, rootMove() ? &(root_move*transform.scale) : null, rootRot() ? &root_rot : null);}
   bool ElmAnim::mayContain(C UID &id)C {return id==skel_id;}
   void ElmAnim::newData()
{
      ::ElmData::newData();
      loop_time++;
      linear_time++;
      skel_time++;
      file_time++;
   }
   void ElmAnim::from(C Animation &anim) {}
   uint ElmAnim::undo(C ElmAnim &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=Undo(skel_time, src.skel_time, skel_id, src.skel_id)*CHANGE_NORMAL;
      if(Undo(  loop_time, src.  loop_time)){changed|=CHANGE_AFFECT_FILE; loop  (src.loop  ());}
      if(Undo(linear_time, src.linear_time)){changed|=CHANGE_AFFECT_FILE; linear(src.linear());}
      if(Undo(  file_time, src.  file_time)){changed|=CHANGE_AFFECT_FILE; transform=src.transform; root_move=src.root_move; root_rot=src.root_rot; FlagCopy(flag, src.flag, ROOT_ALL);}

      if(changed)newVer();
      return changed;
   }
   uint ElmAnim::sync(C ElmAnim &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=Sync(skel_time, src.skel_time, skel_id, src.skel_id)*CHANGE_NORMAL;
      if(Sync(  loop_time, src.  loop_time)){changed|=CHANGE_AFFECT_FILE; loop  (src.loop  ());}
      if(Sync(linear_time, src.linear_time)){changed|=CHANGE_AFFECT_FILE; linear(src.linear());}

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmAnim::syncFile(C ElmAnim &src)
   {
      bool changed=false;

      if(Sync(file_time, src.file_time)){changed|=true; transform=src.transform; root_move=src.root_move; root_rot=src.root_rot; FlagCopy(flag, src.flag, ROOT_ALL);}

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return true;
   }
   uint ElmAnim::OldFlag(byte old)
   {
      uint f=0;
      if(old&(1<<0))f|=LOOP;
      if(old&(1<<1))f|=LINEAR;
      if(old&(1<<2))f|=ROOT_DEL_POS_X;
      if(old&(1<<3))f|=ROOT_DEL_POS_Y;
      if(old&(1<<4))f|=ROOT_DEL_POS_Z;
      if(old&(1<<5))f|=ROOT_DEL_ROT;
      if(old&(1<<6))f|=ROOT_2_KEYS;
      if(old&(1<<7))f|=ROOT_FROM_BODY;
      return f;
   }
   bool ElmAnim::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(3);
      f<<skel_id<<transform<<root_move<<root_rot<<flag<<loop_time<<linear_time<<skel_time<<file_time;
      return f.ok();
   }
   bool ElmAnim::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 3:
         {
            f>>skel_id>>transform>>root_move>>root_rot>>flag>>loop_time>>linear_time>>skel_time>>file_time;
            if(f.ok())return true;
         }break;

         case 2:
         {
            f>>skel_id>>transform>>root_move>>root_rot; flag=OldFlag(f.getByte()); f>>loop_time>>linear_time>>skel_time>>file_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            f>>skel_id>>transform>>root_move; flag=OldFlag(f.getByte()); f>>loop_time>>linear_time>>skel_time>>file_time; rootRotZero();
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>skel_id>>transform; flag=OldFlag(f.getByte()); f>>loop_time>>linear_time>>skel_time>>file_time; rootMoveZero(); rootRotZero();
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmAnim::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(skel_id.valid())nodes.New().setFN ("Skeleton"    , skel_id);
                         nodes.New().setRaw("Pose"        , transform);
                         nodes.New().set   ("Loop"        , loop());
                         nodes.New().set   ("Linear"      , linear());
    if(rootMove()       )nodes.New().setRaw("RootMove"    , root_move);
    if(rootRot ()       )nodes.New().setRaw("RootRot"     , root_rot );
    if(flag&ROOT_DEL_POS)nodes.New().set   ("RootDelPos"  , FlagAll(flag, ROOT_DEL_POS) ? S : S+(FlagTest(flag, ROOT_DEL_POS_X) ? 'X' : '\0')+(FlagTest(flag, ROOT_DEL_POS_Y) ? 'Y' : '\0')+(FlagTest(flag, ROOT_DEL_POS_Z) ? 'Z' : '\0'));
    if(flag&ROOT_DEL_ROT)nodes.New().set   ("RootDelRot"  , FlagAll(flag, ROOT_DEL_ROT) ? S : S+(FlagTest(flag, ROOT_DEL_ROT_X) ? 'X' : '\0')+(FlagTest(flag, ROOT_DEL_ROT_Y) ? 'Y' : '\0')+(FlagTest(flag, ROOT_DEL_ROT_Z) ? 'Z' : '\0'));
    if(flag&ROOT_2_KEYS )nodes.New().set   ("Root2Keys"   );
                         nodes.New().set   ("LoopTime"    ,   loop_time.text());
                         nodes.New().set   ("LinearTime"  , linear_time.text());
                         nodes.New().set   ("SkeletonTime",   skel_time.text());
                         nodes.New().set   ("FileTime"    ,   file_time.text());
   }
   void ElmAnim::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Skeleton"    )n.getValue   (skel_id);else
         if(n.name=="Pose"        )n.getValueRaw(transform);else
         if(n.name=="RootMove"    )n.getValueRaw(root_move);else
         if(n.name=="RootRot"     )n.getValueRaw(root_rot);else
         if(n.name=="Loop"        )loop  (n.asBool1());else
         if(n.name=="Linear"      )linear(n.asBool1());else
         if(n.name=="Root2Keys"   )FlagSet(flag, ROOT_2_KEYS, n.asBool1());else
         if(n.name=="LoopTime"    )  loop_time=n.asText();else
         if(n.name=="LinearTime"  )linear_time=n.asText();else
         if(n.name=="SkeletonTime")  skel_time=n.asText();else
         if(n.name=="FileTime"    )  file_time=n.asText();else
         if(n.name=="RootDelPos"  )
         {
            if(        !n.value.is()         )FlagEnable(flag, ROOT_DEL_POS);else
            if(CharFlag(n.value[0])&CHARF_DIG)FlagSet   (flag, ROOT_DEL_POS, n.asBool());else
            {
               FlagSet(flag, ROOT_DEL_POS_X, Contains(n.value, 'x'));
               FlagSet(flag, ROOT_DEL_POS_Y, Contains(n.value, 'y'));
               FlagSet(flag, ROOT_DEL_POS_Z, Contains(n.value, 'z'));
            }
         }else
         if(n.name=="RootDelRot")
         {
            if(        !n.value.is()         )FlagEnable(flag, ROOT_DEL_ROT);else
            if(CharFlag(n.value[0])&CHARF_DIG)FlagSet   (flag, ROOT_DEL_ROT, n.asBool());else
            {
               FlagSet(flag, ROOT_DEL_ROT_X, Contains(n.value, 'x'));
               FlagSet(flag, ROOT_DEL_ROT_Y, Contains(n.value, 'y'));
               FlagSet(flag, ROOT_DEL_ROT_Z, Contains(n.value, 'z'));
            }
         }
      }
   }
   ElmEnv::ElmEnv() : sun_id(UIDZero), star_id(UIDZero), skybox_id(UIDZero) {REPAO(cloud_id).zero();}
   void ElmEnv::from(C EditEnv &src)
   {
      sun_id=src.sun_id; star_id=src.star_id; skybox_id=src.skybox_id; REPAO(cloud_id)=src.cloud_id[i];
   }
   uint ElmEnv::undo(C ElmEnv &src) {return ::ElmData::undo(src);}
   uint ElmEnv::sync(C ElmEnv &src) {return ::ElmData::sync(src);}
   bool ElmEnv::mayContain(C UID &id)C {return id==sun_id || id==star_id || id==skybox_id || id==cloud_id[0] || id==cloud_id[1] || id==cloud_id[2] || id==cloud_id[3];}
   bool ElmEnv::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<sun_id<<star_id<<skybox_id<<cloud_id;
      return f.ok();
   }
   bool ElmEnv::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>sun_id>>star_id>>skybox_id>>cloud_id;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmEnv::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(   sun_id.valid())nodes.New().setFN("Sun"   ,    sun_id);
      if(  star_id.valid())nodes.New().setFN("Stars" ,   star_id);
      if(skybox_id.valid())nodes.New().setFN("SkyBox", skybox_id);
      {
         TextNode &clouds=nodes.New().setName("Clouds");
         FREPA(cloud_id) // list in order
         {
            TextParam &cloud=clouds.nodes.New();
            if(cloud_id[i].valid())cloud.setValueFN(cloud_id[i]);
         }
      }
   }
   void ElmEnv::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Sun"   )n.getValue(   sun_id);else
         if(n.name=="Stars" )n.getValue(  star_id);else
         if(n.name=="SkyBox")n.getValue(skybox_id);else
         if(n.name=="Clouds")
         {
            REP(Min(n.nodes.elms(), Elms(cloud_id)))n.nodes[i].getValue(cloud_id[i]);
         }
      }
   }
   bool ElmWorld::valid()C {return area_size>0;}
   bool ElmWorld::equal(C ElmWorld &src)C
   {
      return ::ElmData::equal(src)
          && hm_res_time==src.hm_res_time
          && ctrl_r_time==src.ctrl_r_time && ctrl_h_time==src.ctrl_h_time && max_climb_time==src.max_climb_time && max_slope_time==src.max_slope_time && cell_size_time==src.cell_size_time && cell_h_time==src.cell_h_time
          && env_time==src.env_time;
   }
   bool ElmWorld::newer(C ElmWorld &src)C
   {
      return ::ElmData::newer(src)
          || hm_res_time>src.hm_res_time
          || ctrl_r_time>src.ctrl_r_time || ctrl_h_time>src.ctrl_h_time || max_climb_time>src.max_climb_time || max_slope_time>src.max_slope_time || cell_size_time>src.cell_size_time || cell_h_time>src.cell_h_time
          || env_time>src.env_time;
   }
   int ElmWorld::hmRes()C {return hm_res ? NearestPow2(hm_res)+1 : 0;}
   ElmWorld& ElmWorld::ctrlRadius(flt r    ) {ctrl_r   =Max(r    ,         0); ctrl_r_time   .getUTC(); newVer(); return T;}
   ElmWorld& ElmWorld::ctrlHeight(flt h    ) {ctrl_h   =Max(h    ,         0); ctrl_h_time   .getUTC(); newVer(); return T;}
   ElmWorld& ElmWorld::maxClimb(flt climb) {max_climb=Max(climb,         0); max_climb_time.getUTC(); newVer(); return T;}
   ElmWorld& ElmWorld::maxSlope(flt slope) {max_slope=Mid(slope, 0.0f, PI_2); max_slope_time.getUTC(); newVer(); return T;}
   ElmWorld& ElmWorld::cellSize(flt size ) {cell_size=Max(size ,     0.001f); cell_size_time.getUTC(); newVer(); return T;}
   ElmWorld& ElmWorld::cellHeight(flt h    ) {cell_h   =Max(h    ,     0.001f); cell_h_time   .getUTC(); newVer(); return T;}
   ElmWorld& ElmWorld::env(C UID &id) {env_id   =id                   ; env_time      .getUTC(); newVer(); return T;}
   void ElmWorld::newData()
{
      ::ElmData::newData();
      hm_res_time++;
      ctrl_r_time++;
      ctrl_h_time++;
      max_climb_time++;
      max_slope_time++;
      cell_size_time++;
      cell_h_time++;
      env_time++;
   }
   void ElmWorld::copyTo(Game::WorldSettings &settings, C Project &proj)C
   {
      settings.environment=proj.gamePath(env_id);
      settings.areaSize(area_size).hmRes(hm_res);
   }
   void ElmWorld::copyTo(PathSettings &settings)C
   {
      settings.areaSize(area_size).ctrlRadius(ctrl_r).ctrlHeight(ctrl_h).maxClimb(max_climb).maxSlope(max_slope).cellSize(cell_size).cellHeight(cell_h);
   }
   uint ElmWorld::undo(C ElmWorld &src)
   {
      uint changed=::ElmData::undo(src);

      if(!area_size && src.area_size){area_size=src.area_size; changed|=CHANGE_AFFECT_FILE;}
      changed|=Undo(hm_res_time   , src.hm_res_time   , hm_res   , src.hm_res   )*CHANGE_AFFECT_FILE;
      changed|=Undo(ctrl_r_time   , src.ctrl_r_time   , ctrl_r   , src.ctrl_r   )*CHANGE_AFFECT_FILE;
      changed|=Undo(ctrl_h_time   , src.ctrl_h_time   , ctrl_h   , src.ctrl_h   )*CHANGE_AFFECT_FILE;
      changed|=Undo(max_climb_time, src.max_climb_time, max_climb, src.max_climb)*CHANGE_AFFECT_FILE;
      changed|=Undo(max_slope_time, src.max_slope_time, max_slope, src.max_slope)*CHANGE_AFFECT_FILE;
      changed|=Undo(cell_size_time, src.cell_size_time, cell_size, src.cell_size)*CHANGE_AFFECT_FILE;
      changed|=Undo(cell_h_time   , src.cell_h_time   , cell_h   , src.cell_h   )*CHANGE_AFFECT_FILE;
      changed|=Undo(env_time      , src.env_time      , env_id   , src.env_id   )*CHANGE_AFFECT_FILE;

      if(changed)newVer();
      return changed;
   }
   uint ElmWorld::sync(C ElmWorld &src)
   {
      uint changed=::ElmData::sync(src);

      if(!area_size && src.area_size){area_size=src.area_size; changed|=CHANGE_AFFECT_FILE;}
      changed|=Sync(hm_res_time   , src.hm_res_time   , hm_res   , src.hm_res   )*CHANGE_AFFECT_FILE;
      changed|=Sync(ctrl_r_time   , src.ctrl_r_time   , ctrl_r   , src.ctrl_r   )*CHANGE_AFFECT_FILE;
      changed|=Sync(ctrl_h_time   , src.ctrl_h_time   , ctrl_h   , src.ctrl_h   )*CHANGE_AFFECT_FILE;
      changed|=Sync(max_climb_time, src.max_climb_time, max_climb, src.max_climb)*CHANGE_AFFECT_FILE;
      changed|=Sync(max_slope_time, src.max_slope_time, max_slope, src.max_slope)*CHANGE_AFFECT_FILE;
      changed|=Sync(cell_size_time, src.cell_size_time, cell_size, src.cell_size)*CHANGE_AFFECT_FILE;
      changed|=Sync(cell_h_time   , src.cell_h_time   , cell_h   , src.cell_h   )*CHANGE_AFFECT_FILE;
      changed|=Sync(env_time      , src.env_time      , env_id   , src.env_id   )*CHANGE_AFFECT_FILE;

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmWorld::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<area_size<<hm_res<<hm_res_time<<ctrl_r<<ctrl_h<<max_climb<<max_slope<<cell_size<<cell_h<<env_id<<ctrl_r_time<<ctrl_h_time<<max_climb_time<<max_slope_time<<cell_size_time<<cell_h_time<<env_time;
      return f.ok();
   }
   bool ElmWorld::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>area_size>>hm_res>>hm_res_time>>ctrl_r>>ctrl_h>>max_climb>>max_slope>>cell_size>>cell_h>>env_id>>ctrl_r_time>>ctrl_h_time>>max_climb_time>>max_slope_time>>cell_size_time>>cell_h_time>>env_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmWorld::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
                        nodes.New().set   ("AreaSize"           , area_size);
                        nodes.New().set   ("HeightmapResolution", hm_res);
                        nodes.New().setRaw("ControllerRadius"   , ctrl_r);
                        nodes.New().setRaw("ControllerHeight"   , ctrl_h);
                        nodes.New().setRaw("MaxClimb"           , max_climb);
                        nodes.New().setRaw("MaxSlope"           , max_slope);
                        nodes.New().setRaw("CellSize"           , cell_size);
                        nodes.New().setRaw("CellHeight"         , cell_h);
      if(env_id.valid())nodes.New().setFN ("Environment"        , env_id);
                        nodes.New().set   ("HeightmapResolutionTime",    hm_res_time.text());
                        nodes.New().set   ("ControllerRadiusTime"   ,    ctrl_r_time.text());
                        nodes.New().set   ("ControllerHeightTime"   ,    ctrl_h_time.text());
                        nodes.New().set   ("MaxClimbTime"           , max_climb_time.text());
                        nodes.New().set   ("MaxSlopeTime"           , max_slope_time.text());
                        nodes.New().set   ("CellSizeTime"           , cell_size_time.text());
                        nodes.New().set   ("CellHeightTime"         ,    cell_h_time.text());
                        nodes.New().set   ("EnvironmentTime"        ,       env_time.text());
   }
   void ElmWorld::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="AreaSize"           )n.getValue   (area_size);else
         if(n.name=="HeightmapResolution")n.getValue   (hm_res   );else
         if(n.name=="ControllerRadius"   )n.getValueRaw(ctrl_r   );else
         if(n.name=="ControllerHeight"   )n.getValueRaw(ctrl_h   );else
         if(n.name=="MaxClimb"           )n.getValueRaw(max_climb);else
         if(n.name=="MaxSlope"           )n.getValueRaw(max_slope);else
         if(n.name=="CellSize"           )n.getValueRaw(cell_size);else
         if(n.name=="CellHeight"         )n.getValueRaw(cell_h   );else
         if(n.name=="Environment"        )n.getValue   (env_id   );else
         if(n.name=="HeightmapResolutionTime")   hm_res_time=n.asText();else
         if(n.name=="ControllerRadiusTime"   )   ctrl_r_time=n.asText();else
         if(n.name=="ControllerHeightTime"   )   ctrl_h_time=n.asText();else
         if(n.name=="MaxClimbTime"           )max_climb_time=n.asText();else
         if(n.name=="MaxSlopeTime"           )max_slope_time=n.asText();else
         if(n.name=="CellSizeTime"           )cell_size_time=n.asText();else
         if(n.name=="CellHeightTime"         )   cell_h_time=n.asText();else
         if(n.name=="EnvironmentTime"        )      env_time=n.asText();
      }
   }
   bool ElmEnum::mayContain(C UID &id)C {return false;}
   void ElmEnum::from(C EditEnums &enums) {type=enums.type;}
   uint ElmEnum::undo(C  ElmEnum  &src) {return ::ElmData::undo(src);}
   uint ElmEnum::sync(C  ElmEnum  &src) {return ::ElmData::sync(src);}
   bool ElmEnum::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(1)<<type;
      return f.ok();
   }
   bool ElmEnum::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 1:
         {
            f>>type;
            if(f.ok())return true;
         }break;

         case 0:
         {
            type=EditEnums::DEFAULT;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmEnum::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(type)nodes.New().set("Type", type);
   }
   void ElmEnum::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Type")type=EditEnums::TYPE(n.asInt());
      }
   }
   bool       ElmImage::ignoreAlpha()C {return mode==IMAGE_CUBE;}
   bool       ElmImage::mipMaps()C {return FlagTest(flag, MIP_MAPS );}
   void ElmImage::mipMaps(bool on) {FlagSet(flag, MIP_MAPS , on);}
   bool       ElmImage::pow2()C {return FlagTest(flag, POW2     );}
   void ElmImage::pow2(bool on) {FlagSet(flag, POW2     , on);}
   bool       ElmImage::alphaLum()C {return FlagTest(flag, ALPHA_LUM);}
   void ElmImage::alphaLum(bool on) {FlagSet(flag, ALPHA_LUM, on);}
   bool       ElmImage::hasColor()C {return FlagTest(flag, HAS_COLOR);}
   void ElmImage::hasColor(bool on) {FlagSet(flag, HAS_COLOR, on);}
   bool       ElmImage::hasAlpha()C {return FlagTest(flag, HAS_ALPHA);}
   void ElmImage::hasAlpha(bool on) {FlagSet(flag, HAS_ALPHA, on);}
   bool       ElmImage::hasAlpha2()C {return hasAlpha() || alphaLum();}
   bool       ElmImage::hasAlpha3()C {return ignoreAlpha() ? false : hasAlpha2();}
   IMAGE_TYPE ElmImage::androidType()C {return (type==COMPRESSED || type==COMPRESSED2) ? hasAlpha3() ? IMAGE_ETC2_A8 : ((AndroidETC2 || type==COMPRESSED2) ? IMAGE_ETC2 : IMAGE_ETC1) : IMAGE_NONE;}
   IMAGE_TYPE     ElmImage::iOSType()C {return (type==COMPRESSED || type==COMPRESSED2) ?               IMAGE_PVRTC1_4                                                                 : IMAGE_NONE;}
   IMAGE_TYPE     ElmImage::webType()C {return (!WebBC7 && (type==COMPRESSED || type==COMPRESSED2) && hasAlpha3()) ? IMAGE_BC3 :                                                        IMAGE_NONE;}
   bool ElmImage::equal(C ElmImage &src)C {return ::ElmData::equal(src) && mip_maps_time==src.mip_maps_time && pow2_time==src.pow2_time && alpha_lum_time==src.alpha_lum_time && type_time==src.type_time && mode_time==src.mode_time && size_time==src.size_time && file_time==src.file_time;}
   bool ElmImage::newer(C ElmImage &src)C {return ::ElmData::newer(src) || mip_maps_time> src.mip_maps_time || pow2_time> src.pow2_time || alpha_lum_time> src.alpha_lum_time || type_time> src.type_time || mode_time> src.mode_time || size_time> src.size_time || file_time> src.file_time;}
   bool ElmImage::mayContain(C UID &id)C {return false;}
   void ElmImage::newData()
{
      ::ElmData::newData(); mip_maps_time++; pow2_time++; alpha_lum_time++; type_time++; mode_time++; size_time++; file_time++;
   }
   uint ElmImage::undo(C ElmImage &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=Undo(type_time, src.type_time, type, src.type)*CHANGE_AFFECT_FILE;
      changed|=Undo(mode_time, src.mode_time, mode, src.mode)*CHANGE_AFFECT_FILE;
      changed|=Undo(size_time, src.size_time, size, src.size)*CHANGE_AFFECT_FILE;

      if(Undo( mip_maps_time, src. mip_maps_time)){changed|=CHANGE_AFFECT_FILE; mipMaps (src.mipMaps ());}
      if(Undo(     pow2_time, src.     pow2_time)){changed|=CHANGE_AFFECT_FILE; pow2    (src.pow2    ());}
      if(Undo(alpha_lum_time, src.alpha_lum_time)){changed|=CHANGE_AFFECT_FILE; alphaLum(src.alphaLum());}

      // do not undo 'hasColor, hasAlpha' as they're inherited from image data

      if(changed)newVer();
      return changed;
   }
   uint ElmImage::sync(C ElmImage &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=Sync(type_time, src.type_time, type, src.type)*CHANGE_AFFECT_FILE;
      changed|=Sync(mode_time, src.mode_time, mode, src.mode)*CHANGE_AFFECT_FILE;
      changed|=Sync(size_time, src.size_time, size, src.size)*CHANGE_AFFECT_FILE;

      if(Sync( mip_maps_time, src. mip_maps_time)){changed|=CHANGE_AFFECT_FILE; mipMaps (src.mipMaps ());}
      if(Sync(     pow2_time, src.     pow2_time)){changed|=CHANGE_AFFECT_FILE; pow2    (src.pow2    ());}
      if(Sync(alpha_lum_time, src.alpha_lum_time)){changed|=CHANGE_AFFECT_FILE; alphaLum(src.alphaLum());}

      // do not sync 'hasColor, hasAlpha' as they're inherited from image data

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmImage::syncFile(C ElmImage &src)
   {
      bool changed=false;

      if(Sync(file_time, src.file_time)){changed|=true; hasColor(src.hasColor()); hasAlpha(src.hasAlpha());}

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmImage::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(2);
      f<<flag<<type<<mode<<size<<mip_maps_time<<pow2_time<<alpha_lum_time<<type_time<<mode_time<<size_time<<file_time;
      return f.ok();
   }
   bool ElmImage::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 2:
         {
            f>>flag>>type>>mode>>size>>mip_maps_time>>pow2_time>>alpha_lum_time>>type_time>>mode_time>>size_time>>file_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            f>>flag>>type>>mode>>size>>mip_maps_time>>pow2_time>>alpha_lum_time>>type_time>>mode_time>>size_time>>file_time; if(type>=1)type=ElmImage::TYPE(type+1);
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>flag>>type>>mode>>mip_maps_time>>pow2_time>>alpha_lum_time>>type_time>>mode_time>>file_time; size=0; size_time.zero(); if(type>=1)type=ElmImage::TYPE(type+1);
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmImage::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(InRange(type, ImageTypes))nodes.New().set("Type", ImageTypes[type].name);
      REPA(ImageModes)if(mode==ImageModes[i].mode){nodes.New().set("Mode", ImageModes[i].name); break;}
      if(size.any())nodes.New().set("Size"   , size);
                    nodes.New().set("MipMaps", mipMaps());
      if(pow2    ())nodes.New().set("Pow2");
      if(alphaLum())nodes.New().set("AlphaFromLum");
      if(hasColor())nodes.New().set("HasColor");
      if(hasAlpha())nodes.New().set("HasAlpha");
      nodes.New().set("MipMapsTime"     ,  mip_maps_time.text());
      nodes.New().set("Pow2Time"        ,      pow2_time.text());
      nodes.New().set("AlphaFromLumTime", alpha_lum_time.text());
      nodes.New().set("TypeTime"        ,      type_time.text());
      nodes.New().set("ModeTime"        ,      mode_time.text());
      nodes.New().set("SizeTime"        ,      size_time.text());
      nodes.New().set("FileTime"        ,      file_time.text());
   }
   void ElmImage::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Type"){REPA(ImageTypes)if(n.value==ImageTypes[i].name){type=      TYPE(i)     ; break;}}else
         if(n.name=="Mode"){REPA(ImageModes)if(n.value==ImageModes[i].name){mode=ImageModes[i].mode; break;}}else
         if(n.name=="Size"        )n.getValue(size);else
         if(n.name=="MipMaps"     )mipMaps (n.asBool1());else
         if(n.name=="Pow2"        )pow2    (n.asBool1());else
         if(n.name=="AlphaFromLum")alphaLum(n.asBool1());else
         if(n.name=="HasColor"    )hasColor(n.asBool1());else
         if(n.name=="HasAlpha"    )hasAlpha(n.asBool1());else
         if(n.name=="MipMapsTime"     ) mip_maps_time=n.value;else
         if(n.name=="Pow2Time"        )     pow2_time=n.value;else
         if(n.name=="AlphaFromLumTime")alpha_lum_time=n.value;else
         if(n.name=="TypeTime"        )     type_time=n.value;else
         if(n.name=="ModeTime"        )     mode_time=n.value;else
         if(n.name=="SizeTime"        )     size_time=n.value;else
         if(n.name=="FileTime"        )     file_time=n.value;
      }
   }
      int ElmImageAtlas::Img::Compare(C Img &img, C UID &id) {return ::Compare(img.id, id);}
      bool ElmImageAtlas::Img::equal(C Img &src)C {return removed_time==src.removed_time;}
      bool ElmImageAtlas::Img::newer(C Img &src)C {return removed_time> src.removed_time;}
      bool ElmImageAtlas::Img::undo(C Img &src) {return Undo(removed_time, src.removed_time, removed, src.removed);}
 C ::ElmImageAtlas::Img* ElmImageAtlas::find(C UID &id)C {return ConstCast(T).find(id);}
   ::ElmImageAtlas::Img* ElmImageAtlas::find(C UID &id)  {       return images.binaryFind  (id,    Img::Compare);}
   ::ElmImageAtlas::Img&  ElmImageAtlas::get(C UID &id)  {int i; return images.binarySearch(id, i, Img::Compare) ? images[i] : images.NewAt(i);}
   void ElmImageAtlas::newData(){::ElmData::newData(); file_time++; mip_maps_time++;}
   bool ElmImageAtlas::equal(C ElmImageAtlas &src)C
   {
      if(file_time!=src.file_time || mip_maps_time!=src.mip_maps_time)return false;
      if(images.elms()!=src.images.elms())return false;
      REPA(images){C Img &img=images[i]; C Img *s=src.find(img.id); if(!s || !img.equal(*s))return false;}
      return ::ElmData::equal(src);
   }
   bool ElmImageAtlas::newer(C ElmImageAtlas &src)C
   {
      if(file_time>src.file_time || mip_maps_time>src.mip_maps_time)return true;
      REPA(images){C Img &img=images[i]; C Img *s=src.find(img.id); if(!s || img.newer(*s))return true;}
      return ::ElmData::newer(src);
   }
   bool ElmImageAtlas::mayContain(C UID &id)C {REPA(images)if(id==images[i].id)return true; return false;}
   uint ElmImageAtlas::undo(C ElmImageAtlas &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=Undo(mip_maps_time, src.mip_maps_time, mip_maps, src.mip_maps)*CHANGE_AFFECT_FILE;
      // mark as removed those that aren't present in 'src'
      REPA(images)
      {
         Img &img=images[i];
         if( !img.removed && !src.find(img.id)){img.removed=true; img.removed_time++; changed|=CHANGE_AFFECT_FILE;}
      }
      // undo/insert from 'src'
      REPA(src.images)
      {
       C Img &s=src.images[i];
         Img *t=find(s.id);
         if(t)changed|=t->undo(s)*CHANGE_AFFECT_FILE;else{get(s.id)=s; changed|=CHANGE_AFFECT_FILE;}
      }

      if(changed)newVer();
      return changed;
   }
   uint ElmImageAtlas::sync(C ElmImageAtlas &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=Sync(mip_maps_time, src.mip_maps_time, mip_maps, src.mip_maps)*CHANGE_AFFECT_FILE;
      REPA(src.images)
      {
       C Img &s=src.images[i];
         Img &d=get(s.id); if(!d.id.valid()){d.id=s.id; changed|=CHANGE_AFFECT_FILE;}
         changed|=Sync(d.removed_time, s.removed_time, d.removed, s.removed)*CHANGE_AFFECT_FILE;
      }

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmImageAtlas::syncFile(C ElmImageAtlas &src)
   {
      bool changed=false;

      changed|=Sync(file_time, src.file_time);

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmImageAtlas::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(1);
      f<<mip_maps<<file_time<<mip_maps_time;
      f.cmpUIntV(images.elms());
      FREPA(images)f<<images[i].removed<<images[i].id<<images[i].removed_time;
      return f.ok();
   }
   bool ElmImageAtlas::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 1:
         {
            f>>mip_maps>>file_time>>mip_maps_time;
            images.setNum(f.decUIntV());
            FREPA(images)f>>images[i].removed>>images[i].id>>images[i].removed_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            mip_maps=true; mip_maps_time=1;
            f>>file_time;
            images.setNum(f.decUIntV());
            FREPA(images)f>>images[i].removed>>images[i].id>>images[i].removed_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmImageAtlas::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      nodes.New().set("MipMaps"    , mip_maps);
      nodes.New().set("MipMapsTime", mip_maps_time.text());
      nodes.New().set("FileTime"   ,     file_time.text());
      TextNode &images=nodes.New().setName("Images"); FREPA(T.images) // list in order
      {
       C Img      &src =T.images[i];
         TextNode &dest=images.nodes.New().setName(src.id.asFileName());
         if(src.removed          )dest.nodes.New().set("Removed");
         if(src.removed_time.is())dest.nodes.New().set("RemovedTime", src.removed_time.text());
      }
   }
   void ElmImageAtlas::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="MipMaps"    )mip_maps     =n.asBool1();else
         if(n.name=="MipMapsTime")mip_maps_time=n.value    ;else
         if(n.name=="FileTime"   )    file_time=n.value    ;else
         if(n.name=="Images"     )FREPA(n.nodes) // get in order
         {
          C TextNode &src=n.nodes[i]; UID id; if(id.fromText(src.name) && id.valid())
            {
               Img &dest=images.New(); dest.id=id;
               REPA(src.nodes)
               {
                C TextNode &n=src.nodes[i];
                  if(n.name=="Removed"    )dest.removed     =n.asBool1();else
                  if(n.name=="RemovedTime")dest.removed_time=n.asText ();
               }
            }
         }
      }
   }
   bool ElmIconSetts::mayContain(C UID &id)C {return false;}
   void ElmIconSetts::from(C IconSettings &src) {type=src.type;}
   uint ElmIconSetts::undo(C ElmIconSetts &src) {return ::ElmData::undo(src);}
   uint ElmIconSetts::sync(C ElmIconSetts &src) {return ::ElmData::sync(src);}
   bool ElmIconSetts::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(1);
      f<<type;
      return f.ok();
   }
   bool ElmIconSetts::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 1:
         {
            f>>type;
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>type; if(type>=1)type=ElmImage::TYPE(type+1);
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmIconSetts::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(InRange(type, ElmImage::ImageTypes))nodes.New().set("Type", ElmImage::ImageTypes[type].name);
   }
   void ElmIconSetts::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Type"){REPA(ElmImage::ImageTypes)if(n.value==ElmImage::ImageTypes[i].name){type=ElmImage::TYPE(i); break;}}
      }
   }
   ElmImage::TYPE     ElmIcon::type(Project *proj)C {if(proj)if(Elm *elm=proj->findElm(icon_settings_id))if(ElmIconSetts *data=elm->iconSettsData())return data->type; return ElmImage::COMPRESSED;}
   bool          ElmIcon::hasColor(             )C {return FlagTest(flag, HAS_COLOR);}
   ElmIcon& ElmIcon::hasColor(bool on) {FlagSet(flag, HAS_COLOR, on); return T;}
   bool          ElmIcon::hasAlpha(             )C {return FlagTest(flag, HAS_ALPHA);}
   ElmIcon& ElmIcon::hasAlpha(bool on) {FlagSet(flag, HAS_ALPHA, on); return T;}
   IMAGE_TYPE ElmIcon::androidType(Project *proj)C {ElmImage::TYPE type=T.type(proj); return (type==ElmImage::COMPRESSED || type==ElmImage::COMPRESSED2) ? hasAlpha() ? IMAGE_ETC2_A8 : ((AndroidETC2 || type==ElmImage::COMPRESSED2) ? IMAGE_ETC2 : IMAGE_ETC1) : IMAGE_NONE;}
   IMAGE_TYPE     ElmIcon::iOSType(Project *proj)C {ElmImage::TYPE type=T.type(proj); return (type==ElmImage::COMPRESSED || type==ElmImage::COMPRESSED2) ?              IMAGE_PVRTC1_4                                                                          : IMAGE_NONE;}
   IMAGE_TYPE     ElmIcon::webType(Project *proj)C {ElmImage::TYPE type=T.type(proj); return (!WebBC7 && (type==ElmImage::COMPRESSED || type==ElmImage::COMPRESSED2) && hasAlpha()) ? IMAGE_BC3 :                                                                 IMAGE_NONE;}
   bool ElmIcon::equal(C ElmIcon &src)C {return ::ElmData::equal(src) && icon_settings_time==src.icon_settings_time && obj_time==src.obj_time && file_time==src.file_time && anim_id_time==src.anim_id_time && anim_pos_time==src.anim_pos_time && variation_time==src.variation_time;}
   bool ElmIcon::newer(C ElmIcon &src)C {return ::ElmData::newer(src) || icon_settings_time> src.icon_settings_time || obj_time> src.obj_time || file_time> src.file_time || anim_id_time> src.anim_id_time || anim_pos_time> src.anim_pos_time || variation_time> src.variation_time;}
   bool ElmIcon::mayContain(C UID &id)C {return id==icon_settings_id || id==obj_id || id==anim_id;}
   void ElmIcon::newData()
{
      ::ElmData::newData();
      icon_settings_time++;
      obj_time++;
      file_time++;
      anim_id_time++;
      anim_pos_time++;
      variation_time++;
   }
   uint ElmIcon::undo(C ElmIcon &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=Undo(icon_settings_time, src.icon_settings_time, icon_settings_id, src.icon_settings_id)*CHANGE_AFFECT_FILE;
      changed|=Undo(          obj_time, src.          obj_time,           obj_id, src.          obj_id)*CHANGE_AFFECT_FILE;
      changed|=Undo(      anim_id_time, src.      anim_id_time,          anim_id, src.         anim_id)*CHANGE_AFFECT_FILE;
      changed|=Undo(     anim_pos_time, src.     anim_pos_time,         anim_pos, src.        anim_pos)*CHANGE_AFFECT_FILE;
      changed|=Undo(    variation_time, src.    variation_time,     variation_id, src.    variation_id)*CHANGE_AFFECT_FILE;

      if(changed)newVer();
      return changed;
   }
   uint ElmIcon::sync(C ElmIcon &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=Sync(icon_settings_time, src.icon_settings_time, icon_settings_id, src.icon_settings_id)*CHANGE_AFFECT_FILE;
      changed|=Sync(          obj_time, src.          obj_time,           obj_id, src.          obj_id)*CHANGE_AFFECT_FILE;
      changed|=Sync(      anim_id_time, src.      anim_id_time,          anim_id, src.         anim_id)*CHANGE_AFFECT_FILE;
      changed|=Sync(     anim_pos_time, src.     anim_pos_time,         anim_pos, src.        anim_pos)*CHANGE_AFFECT_FILE;
      changed|=Sync(    variation_time, src.    variation_time,     variation_id, src.    variation_id)*CHANGE_AFFECT_FILE;

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmIcon::syncFile(C ElmIcon &src)
   {
      bool changed=false;

      if(Sync(file_time, src.file_time)){changed|=true; hasColor(src.hasColor()); hasAlpha(src.hasAlpha());}

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmIcon::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(2);
      f<<flag<<icon_settings_id<<obj_id<<anim_id<<anim_pos<<variation_id<<icon_settings_time<<obj_time<<anim_id_time<<anim_pos_time<<file_time<<variation_time;
      return f.ok();
   }
   bool ElmIcon::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 2:
         {
            f>>flag>>icon_settings_id>>obj_id>>anim_id>>anim_pos>>variation_id>>icon_settings_time>>obj_time>>anim_id_time>>anim_pos_time>>file_time>>variation_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            f>>flag>>icon_settings_id>>obj_id>>anim_id>>anim_pos>>icon_settings_time>>obj_time>>anim_id_time>>anim_pos_time>>file_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>flag>>icon_settings_id>>obj_id>>icon_settings_time>>obj_time>>file_time; anim_id.zero(); anim_pos=0;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmIcon::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(icon_settings_id.valid())nodes.New().setFN ("IconSettings"     , icon_settings_id);
      if(          obj_id.valid())nodes.New().setFN ("Object"           , obj_id);
      if(         anim_id.valid())nodes.New().setFN ("Animation"        , anim_id);
                                  nodes.New().set   ("Variation"        , variation_id);
      if(anim_pos                )nodes.New().setRaw("AnimationPosition", anim_pos);
      if(hasColor()              )nodes.New().set   ("HasColor");
      if(hasAlpha()              )nodes.New().set   ("HasAlpha");
                                  nodes.New().set   ("IconSettingsTime"     , icon_settings_time.text());
                                  nodes.New().set   ("ObjectTime"           ,           obj_time.text());
                                  nodes.New().set   ("FileTime"             ,          file_time.text());
                                  nodes.New().set   ("AnimationTime"        ,       anim_id_time.text());
                                  nodes.New().set   ("AnimationPositionTime",      anim_pos_time.text());
                                  nodes.New().set   ("VariationTime"        ,     variation_time.text());
   }
   void ElmIcon::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="IconSettings"     )n.getValue   (icon_settings_id);else
         if(n.name=="Object"           )n.getValue   (          obj_id);else
         if(n.name=="Animation"        )n.getValue   (         anim_id);else
         if(n.name=="Variation"        )n.getValue   (    variation_id);else
         if(n.name=="AnimationPosition")n.getValueRaw(        anim_pos);else
         if(n.name=="HasColor"         )hasColor(n.asBool1());else
         if(n.name=="HasAlpha"         )hasAlpha(n.asBool1());else
         if(n.name=="IconSettingsTime"     )icon_settings_time=n.asText();else
         if(n.name=="ObjectTime"           )          obj_time=n.asText();else
         if(n.name=="FileTime"             )         file_time=n.asText();else
         if(n.name=="AnimationTime"        )      anim_id_time=n.asText();else
         if(n.name=="AnimationPositionTime")     anim_pos_time=n.asText();else
         if(n.name=="VariationTime"        )    variation_time=n.asText();
      }
   }
   bool ElmFont::mayContain(C UID &id)C {return false;}
   void ElmFont::from(C EditFont &font) {}
   uint ElmFont::undo(C  ElmFont &src) {return ::ElmData::undo(src);}
   uint ElmFont::sync(C  ElmFont &src) {return ::ElmData::sync(src);}
   bool ElmFont::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      return f.ok();
   }
   bool ElmFont::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmFont::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
   }
   void ElmFont::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
      }
   }
   bool ElmTextStyle::mayContain(C UID &id)C {return id==font_id;}
   void ElmTextStyle::from(C EditTextStyle &ts ) {font_id=ts.font;}
   uint ElmTextStyle::undo(C  ElmTextStyle &src) {return ::ElmData::undo(src);}
   uint ElmTextStyle::sync(C  ElmTextStyle &src) {return ::ElmData::sync(src);}
   bool ElmTextStyle::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<font_id;
      return f.ok();
   }
   bool ElmTextStyle::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>font_id;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmTextStyle::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      if(font_id.valid())nodes.New().setFN("Font", font_id);
   }
   void ElmTextStyle::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Font")n.getValue(font_id);
      }
   }
   bool ElmPanelImage::mayContain(C UID &id)C {return image_ids.binaryHas(id, Compare);}
   void ElmPanelImage::from(C EditPanelImage &pi)
   {
      Memt<UID> temp; pi.base.includeIDs(temp); image_ids=temp;
      compressed=pi.base.compressed;
   }
   uint ElmPanelImage::undo(C ElmPanelImage &src) {return ::ElmData::undo(src);}
   uint ElmPanelImage::sync(C ElmPanelImage &src) {return ::ElmData::sync(src);}
   bool ElmPanelImage::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<compressed;
      image_ids.saveRaw(f);
      return f.ok();
   }
   bool ElmPanelImage::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>compressed;
            if(image_ids.loadRaw(f))
               if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmPanelImage::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      nodes.New().set("Compressed", compressed);
      {
         TextNode &images=nodes.New().setName("Images");
         FREPA(image_ids)images.nodes.New().setValueFN(image_ids[i]); // list in order
      }
   }
   void ElmPanelImage::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Compressed")compressed=n.asBool1();else
         if(n.name=="Images")
         {
            FREPA(n.nodes) // add in order
            {
               UID id; if(n.nodes[i].getValue(id) && id.valid())image_ids.binaryInclude(id, Compare);
            }
         }
      }
   }
   bool ElmPanel::mayContain(C UID &id)C {return image_ids.binaryHas(id, Compare);}
   void ElmPanel::from(C EditPanel &panel)
   {
      Memt<UID> temp; panel.includeIDs(temp); image_ids=temp;
   }
   uint ElmPanel::undo(C ElmPanel &src) {return ::ElmData::undo(src);}
   uint ElmPanel::sync(C ElmPanel &src) {return ::ElmData::sync(src);}
   bool ElmPanel::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(2);
      image_ids.saveRaw(f);
      return f.ok();
   }
   bool ElmPanel::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 2:
         {
            if(image_ids.loadRaw(f))
               if(f.ok())return true;
         }break;

         case 1: {Memt<UID> temp; REP(6)Include(temp, f.getUID()); image_ids=temp;} if(f.ok())return true; break;
         case 0: {Memt<UID> temp; REP(4)Include(temp, f.getUID()); image_ids=temp;} if(f.ok())return true; break;
      }
      return false;
   }
   void ElmPanel::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      {
         TextNode &images=nodes.New().setName("Images");
         FREPA(image_ids)images.nodes.New().setValueFN(image_ids[i]); // list in order
      }
   }
   void ElmPanel::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Images")
         {
            FREPA(n.nodes) // add in order
            {
               UID id; if(n.nodes[i].getValue(id) && id.valid())image_ids.binaryInclude(id, Compare);
            }
         }
      }
   }
   bool ElmGuiSkin::mayContain(C UID &id)C {return elm_ids.binaryHas(id, Compare);}
   void ElmGuiSkin::from(C EditGuiSkin &pi)
   {
      Memt<UID> temp; pi.base.includeIDs(temp); elm_ids=temp;
   }
   uint ElmGuiSkin::undo(C ElmGuiSkin &src) {return ::ElmData::undo(src);}
   uint ElmGuiSkin::sync(C ElmGuiSkin &src) {return ::ElmData::sync(src);}
   bool ElmGuiSkin::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      elm_ids.saveRaw(f);
      return f.ok();
   }
   bool ElmGuiSkin::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            if(elm_ids.loadRaw(f))
               if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmGuiSkin::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      {
         TextNode &elms=nodes.New().setName("Elements");
         FREPA(elm_ids)elms.nodes.New().setValueFN(elm_ids[i]); // list in order
      }
   }
   void ElmGuiSkin::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Elements")
         {
            FREPA(n.nodes) // add in order
            {
               UID id; if(n.nodes[i].getValue(id) && id.valid())elm_ids.binaryInclude(id, Compare);
            }
         }
      }
   }
   bool ElmGui::equal(C ElmGui &src)C {return ::ElmData::equal(src) && file_time==src.file_time;}
   bool ElmGui::newer(C ElmGui &src)C {return ::ElmData::newer(src) || file_time> src.file_time;}
   void ElmGui::newData()
{
      ::ElmData::newData();
      file_time++;
   }
   uint ElmGui::undo(C ElmGui &src)
   {
      uint   changed=::ElmData::undo(src);
      if(    changed)newVer();
      return changed;
   }
   uint ElmGui::sync(C ElmGui &src)
   {
      uint changed=::ElmData::sync(src);
      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmGui::syncFile(C ElmGui &src)
   {
      bool changed=false;

      changed|=Sync(file_time, src.file_time);

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmGui::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<file_time;
      return f.ok();
   }
   bool ElmGui::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>file_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmGui::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      nodes.New().set("FileTime", file_time.text());
   }
   void ElmGui::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="FileTime")file_time=n.asText();
      }
   }
   bool ElmSound::equal(C ElmSound &src)C {return ::ElmData::equal(src) && file_time==src.file_time;}
   bool ElmSound::newer(C ElmSound &src)C {return ::ElmData::newer(src) || file_time> src.file_time;}
   bool ElmSound::mayContain(C UID &id)C {return false;}
   void ElmSound::newData()
{
      ::ElmData::newData();
      file_time++;
   }
   uint ElmSound::undo(C ElmSound &src)
   {
      uint   changed=::ElmData::undo(src);
      if(    changed)newVer();
      return changed;
   }
   uint ElmSound::sync(C ElmSound &src)
   {
      uint changed=::ElmData::sync(src);
      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmSound::syncFile(C ElmSound &src)
   {
      bool changed=false;

      changed|=Sync(file_time, src.file_time);

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmSound::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<file_time;
      return f.ok();
   }
   bool ElmSound::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>file_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmSound::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      nodes.New().set("FileTime", file_time.text());
   }
   void ElmSound::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="FileTime")file_time=n.asText();
      }
   }
   bool ElmVideo::equal(C ElmVideo &src)C {return ::ElmData::equal(src) && file_time==src.file_time;}
   bool ElmVideo::newer(C ElmVideo &src)C {return ::ElmData::newer(src) || file_time> src.file_time;}
   bool ElmVideo::mayContain(C UID &id)C {return false;}
   void ElmVideo::newData()
{
      ::ElmData::newData();
      file_time++;
   }
   uint ElmVideo::undo(C ElmVideo &src)
   {
      uint   changed=::ElmData::undo(src);
      if(    changed)newVer();
      return changed;
   }
   uint ElmVideo::sync(C ElmVideo &src)
   {
      uint changed=::ElmData::sync(src);
      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmVideo::syncFile(C ElmVideo &src)
   {
      bool changed=false;

      changed|=Sync(file_time, src.file_time);

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmVideo::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<file_time;
      return f.ok();
   }
   bool ElmVideo::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>file_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmVideo::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      nodes.New().set("FileTime", file_time.text());
   }
   void ElmVideo::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="FileTime")file_time=n.asText();
      }
   }
   bool ElmFile::equal(C ElmFile &src)C {return ::ElmData::equal(src) && file_time==src.file_time;}
   bool ElmFile::newer(C ElmFile &src)C {return ::ElmData::newer(src) || file_time> src.file_time;}
   bool ElmFile::mayContain(C UID &id)C {return false;}
   void ElmFile::newData()
{
      ::ElmData::newData();
      file_time++;
   }
   uint ElmFile::undo(C ElmFile &src)
   {
      uint   changed=::ElmData::undo(src);
      if(    changed)newVer();
      return changed;
   }
   uint ElmFile::sync(C ElmFile &src)
   {
      uint changed=::ElmData::sync(src);
      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmFile::syncFile(C ElmFile &src)
   {
      bool changed=false;

      changed|=Sync(file_time, src.file_time);

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmFile::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<file_time;
      return f.ok();
   }
   bool ElmFile::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>file_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmFile::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
      nodes.New().set("FileTime", file_time.text());
   }
   void ElmFile::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="FileTime")file_time=n.asText();
      }
   }
   uint ElmCode::undo(C ElmCode &src) {return ::ElmData::undo(src);}
   uint ElmCode::sync(C ElmCode &src) {return ::ElmData::sync(src);}
   void ElmCode::from(C Str &code) {}
   bool ElmCode::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      return f.ok();
   }
   bool ElmCode::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmCode::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
   }
   void ElmCode::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
      }
   }
   bool ElmApp::equal(C ElmApp &src)C
   {
      return ::ElmData::equal(src)
          && dirs_windows_time==src.dirs_windows_time && dirs_nonwindows_time==src.dirs_nonwindows_time
          && headers_windows_time==src.headers_windows_time && headers_mac_time==src.headers_mac_time && headers_linux_time==src.headers_linux_time && headers_android_time==src.headers_android_time && headers_ios_time==src.headers_ios_time
          && libs_windows_time==src.libs_windows_time && libs_mac_time==src.libs_mac_time && libs_linux_time==src.libs_linux_time && libs_android_time==src.libs_android_time && libs_ios_time==src.libs_ios_time
          && package_time==src.package_time && android_license_key_time==src.android_license_key_time && location_usage_reason_time==src.location_usage_reason_time && build_time==src.build_time
          && fb_app_id_time==src.fb_app_id_time
          && cb_app_id_ios_time==src.cb_app_id_ios_time && cb_app_signature_ios_time==src.cb_app_signature_ios_time && cb_app_id_google_time==src.cb_app_id_google_time && cb_app_signature_google_time==src.cb_app_signature_google_time
          && storage_time==src.storage_time && supported_orientations_time==src.supported_orientations_time
          && embed_engine_data_time==src.embed_engine_data_time && publish_proj_data_time==src.publish_proj_data_time && publish_physx_dll_time==src.publish_physx_dll_time && publish_steam_dll_time==src.publish_steam_dll_time && publish_open_vr_dll_time==src.publish_open_vr_dll_time && publish_data_as_pak_time==src.publish_data_as_pak_time && android_expansion_time==src.android_expansion_time
          && icon_time==src.icon_time && notification_icon_time==src.notification_icon_time && image_portrait_time==src.image_portrait_time && image_landscape_time==src.image_landscape_time && gui_skin_time==src.gui_skin_time;
   }
   bool ElmApp::newer(C ElmApp &src)C
   {
      return ::ElmData::newer(src)
          || dirs_windows_time>src.dirs_windows_time || dirs_nonwindows_time>src.dirs_nonwindows_time
          || headers_windows_time>src.headers_windows_time || headers_mac_time>src.headers_mac_time || headers_linux_time>src.headers_linux_time || headers_android_time>src.headers_android_time || headers_ios_time>src.headers_ios_time
          || libs_windows_time>src.libs_windows_time || libs_mac_time>src.libs_mac_time || libs_linux_time>src.libs_linux_time || libs_android_time>src.libs_android_time || libs_ios_time>src.libs_ios_time
          || package_time>src.package_time || android_license_key_time>src.android_license_key_time || location_usage_reason_time>src.location_usage_reason_time || build_time>src.build_time
          || fb_app_id_time>src.fb_app_id_time
          || cb_app_id_ios_time>src.cb_app_id_ios_time || cb_app_signature_ios_time>src.cb_app_signature_ios_time || cb_app_id_google_time>src.cb_app_id_google_time || cb_app_signature_google_time>src.cb_app_signature_google_time
          || storage_time>src.storage_time || supported_orientations_time>src.supported_orientations_time
          || embed_engine_data_time>src.embed_engine_data_time || publish_proj_data_time>src.publish_proj_data_time || publish_physx_dll_time>src.publish_physx_dll_time || publish_steam_dll_time>src.publish_steam_dll_time || publish_open_vr_dll_time>src.publish_open_vr_dll_time || publish_data_as_pak_time>src.publish_data_as_pak_time || android_expansion_time>src.android_expansion_time
          || icon_time>src.icon_time || notification_icon_time>src.notification_icon_time || image_portrait_time>src.image_portrait_time || image_landscape_time>src.image_landscape_time || gui_skin_time>src.gui_skin_time;
   }
   bool ElmApp::mayContain(C UID &id)C {return id==icon || id==notification_icon || id==image_portrait || id==image_landscape || id==gui_skin;}
   int     ElmApp::embedEngineData(     )C {return FlagTest(flag, EMBED_ENGINE_DATA) ? FlagTest(flag, EMBED_ENGINE_DATA_FULL) ? 2 : 1 : 0;}
   ElmApp& ElmApp::embedEngineData(int e)  {       FlagSet (flag, EMBED_ENGINE_DATA, e!=0); FlagSet(flag, EMBED_ENGINE_DATA_FULL, e>1); return T;}
   bool ElmApp::publishProjData()C {return FlagTest(flag, PUBLISH_PROJ_DATA  );}
   ElmApp& ElmApp::publishProjData(bool on) {FlagSet(flag, PUBLISH_PROJ_DATA  , on); return T;}
   bool ElmApp::publishPhysxDll()C {return FlagTest(flag, PUBLISH_PHYSX_DLL  );}
   ElmApp& ElmApp::publishPhysxDll(bool on) {FlagSet(flag, PUBLISH_PHYSX_DLL  , on); return T;}
   bool ElmApp::publishSteamDll()C {return FlagTest(flag, PUBLISH_STEAM_DLL  );}
   ElmApp& ElmApp::publishSteamDll(bool on) {FlagSet(flag, PUBLISH_STEAM_DLL  , on); return T;}
   bool ElmApp::publishOpenVRDll()C {return FlagTest(flag, PUBLISH_OPEN_VR_DLL);}
   ElmApp& ElmApp::publishOpenVRDll(bool on) {FlagSet(flag, PUBLISH_OPEN_VR_DLL, on); return T;}
   bool ElmApp::publishDataAsPak()C {return FlagTest(flag, PUBLISH_DATA_AS_PAK);}
   ElmApp& ElmApp::publishDataAsPak(bool on) {FlagSet(flag, PUBLISH_DATA_AS_PAK, on); return T;}
   bool ElmApp::androidExpansion()C {return FlagTest(flag, ANDROID_EXPANSION  );}
   ElmApp& ElmApp::androidExpansion(bool on) {FlagSet(flag, ANDROID_EXPANSION  , on); return T;}
   void ElmApp::newData()
{
      ::ElmData::newData();
      dirs_windows_time++; dirs_nonwindows_time++;
      headers_windows_time++; headers_mac_time++; headers_linux_time++; headers_android_time++; headers_ios_time++;
      libs_windows_time++; libs_mac_time++; libs_linux_time++; libs_android_time++; libs_ios_time++;
      fb_app_id_time++;
      cb_app_id_ios_time++; cb_app_signature_ios_time++; cb_app_id_google_time++; cb_app_signature_google_time++;
      package_time++; android_license_key_time++; location_usage_reason_time++; build_time++; storage_time++; supported_orientations_time++;
      embed_engine_data_time++; publish_proj_data_time++; publish_physx_dll_time++; publish_steam_dll_time++; publish_open_vr_dll_time++; publish_data_as_pak_time++; android_expansion_time++;
      icon_time++; notification_icon_time++; image_portrait_time++; image_landscape_time++; gui_skin_time++;
   }
   uint ElmApp::undo(C ElmApp &src)
   {
      uint changed=::ElmData::undo(src);
      bool ch     =false;

      {
         ch|=Undo(           dirs_windows_time, src.           dirs_windows_time, dirs_windows           , src.dirs_windows           );
         ch|=Undo(        dirs_nonwindows_time, src.        dirs_nonwindows_time, dirs_nonwindows        , src.dirs_nonwindows        );
         ch|=Undo(        headers_windows_time, src.        headers_windows_time, headers_windows        , src.headers_windows        );
         ch|=Undo(            headers_mac_time, src.            headers_mac_time, headers_mac            , src.headers_mac            );
         ch|=Undo(          headers_linux_time, src.          headers_linux_time, headers_linux          , src.headers_linux          );
         ch|=Undo(        headers_android_time, src.        headers_android_time, headers_android        , src.headers_android        );
         ch|=Undo(            headers_ios_time, src.            headers_ios_time, headers_ios            , src.headers_ios            );
         ch|=Undo(           libs_windows_time, src.           libs_windows_time, libs_windows           , src.libs_windows           );
         ch|=Undo(               libs_mac_time, src.               libs_mac_time, libs_mac               , src.libs_mac               );
         ch|=Undo(             libs_linux_time, src.             libs_linux_time, libs_linux             , src.libs_linux             );
         ch|=Undo(           libs_android_time, src.           libs_android_time, libs_android           , src.libs_android           );
         ch|=Undo(               libs_ios_time, src.               libs_ios_time, libs_ios               , src.libs_ios               );
         ch|=Undo(                package_time, src.                package_time, package                , src.package                );
         ch|=Undo(    android_license_key_time, src.    android_license_key_time, android_license_key    , src.android_license_key    );
         ch|=Undo(  location_usage_reason_time, src.  location_usage_reason_time, location_usage_reason  , src.location_usage_reason  );
         ch|=Undo(                  build_time, src.                  build_time, build                  , src.build                  );
         ch|=Undo(                storage_time, src.                storage_time, storage                , src.storage                );
         ch|=Undo(              fb_app_id_time, src.              fb_app_id_time, fb_app_id              , src.fb_app_id              );
         ch|=Undo(          cb_app_id_ios_time, src.          cb_app_id_ios_time, cb_app_id_ios          , src.cb_app_id_ios          );
         ch|=Undo(   cb_app_signature_ios_time, src.   cb_app_signature_ios_time, cb_app_signature_ios   , src.cb_app_signature_ios   );
         ch|=Undo(       cb_app_id_google_time, src.       cb_app_id_google_time, cb_app_id_google       , src.cb_app_id_google       );
         ch|=Undo(cb_app_signature_google_time, src.cb_app_signature_google_time, cb_app_signature_google, src.cb_app_signature_google);

         if(Undo(embed_engine_data_time  , src.embed_engine_data_time  )){ch=true; embedEngineData (src.embedEngineData ());}
         if(Undo(publish_proj_data_time  , src.publish_proj_data_time  )){ch=true; publishProjData (src.publishProjData ());}
         if(Undo(publish_physx_dll_time  , src.publish_physx_dll_time  )){ch=true; publishPhysxDll (src.publishPhysxDll ());}
         if(Undo(publish_steam_dll_time  , src.publish_steam_dll_time  )){ch=true; publishSteamDll (src.publishSteamDll ());}
         if(Undo(publish_open_vr_dll_time, src.publish_open_vr_dll_time)){ch=true; publishOpenVRDll(src.publishOpenVRDll());}
         if(Undo(publish_data_as_pak_time, src.publish_data_as_pak_time)){ch=true; publishDataAsPak(src.publishDataAsPak());}
         if(Undo(android_expansion_time  , src.android_expansion_time  )){ch=true; androidExpansion(src.androidExpansion());}
       //if(Undo(windows_code_sign_time  , src.windows_code_sign_time  )){ch=true; windowsCodeSign (src.windowsCodeSign ());}
      }
      ch|=Undo(supported_orientations_time, src.supported_orientations_time, supported_orientations, src.supported_orientations);
      ch|=Undo(                  icon_time, src.                  icon_time, icon                  , src.icon                  );
      ch|=Undo(     notification_icon_time, src.     notification_icon_time, notification_icon     , src.notification_icon     );
      ch|=Undo(        image_portrait_time, src.        image_portrait_time, image_portrait        , src.image_portrait        );
      ch|=Undo(       image_landscape_time, src.       image_landscape_time, image_landscape       , src.image_landscape       );
      ch|=Undo(              gui_skin_time, src.              gui_skin_time, gui_skin              , src.gui_skin              );

      changed|=ch*CHANGE_NORMAL;
      if(changed)newVer();
      return changed;
   }
   uint ElmApp::sync(C ElmApp &src, bool manual)
   {
      uint changed=::ElmData::sync(src);
      bool ch     =false;

      if(manual) // these are advanced settings and can be synced only manually
      {
         ch|=Sync(           dirs_windows_time, src.           dirs_windows_time, dirs_windows           , src.dirs_windows           );
         ch|=Sync(        dirs_nonwindows_time, src.        dirs_nonwindows_time, dirs_nonwindows        , src.dirs_nonwindows        );
         ch|=Sync(        headers_windows_time, src.        headers_windows_time, headers_windows        , src.headers_windows        );
         ch|=Sync(            headers_mac_time, src.            headers_mac_time, headers_mac            , src.headers_mac            );
         ch|=Sync(          headers_linux_time, src.          headers_linux_time, headers_linux          , src.headers_linux          );
         ch|=Sync(        headers_android_time, src.        headers_android_time, headers_android        , src.headers_android        );
         ch|=Sync(            headers_ios_time, src.            headers_ios_time, headers_ios            , src.headers_ios            );
         ch|=Sync(           libs_windows_time, src.           libs_windows_time, libs_windows           , src.libs_windows           );
         ch|=Sync(               libs_mac_time, src.               libs_mac_time, libs_mac               , src.libs_mac               );
         ch|=Sync(             libs_linux_time, src.             libs_linux_time, libs_linux             , src.libs_linux             );
         ch|=Sync(           libs_android_time, src.           libs_android_time, libs_android           , src.libs_android           );
         ch|=Sync(               libs_ios_time, src.               libs_ios_time, libs_ios               , src.libs_ios               );
         ch|=Sync(                package_time, src.                package_time, package                , src.package                );
         ch|=Sync(    android_license_key_time, src.    android_license_key_time, android_license_key    , src.android_license_key    );
         ch|=Sync(  location_usage_reason_time, src.  location_usage_reason_time, location_usage_reason  , src.location_usage_reason  );
         ch|=Sync(                  build_time, src.                  build_time, build                  , src.build                  );
         ch|=Sync(                storage_time, src.                storage_time, storage                , src.storage                );
         ch|=Sync(              fb_app_id_time, src.              fb_app_id_time, fb_app_id              , src.fb_app_id              );
         ch|=Sync(          cb_app_id_ios_time, src.          cb_app_id_ios_time, cb_app_id_ios          , src.cb_app_id_ios          );
         ch|=Sync(   cb_app_signature_ios_time, src.   cb_app_signature_ios_time, cb_app_signature_ios   , src.cb_app_signature_ios   );
         ch|=Sync(       cb_app_id_google_time, src.       cb_app_id_google_time, cb_app_id_google       , src.cb_app_id_google       );
         ch|=Sync(cb_app_signature_google_time, src.cb_app_signature_google_time, cb_app_signature_google, src.cb_app_signature_google);

         if(Sync(embed_engine_data_time  , src.embed_engine_data_time  )){ch=true; embedEngineData (src.embedEngineData ());}
         if(Sync(publish_proj_data_time  , src.publish_proj_data_time  )){ch=true; publishProjData (src.publishProjData ());}
         if(Sync(publish_physx_dll_time  , src.publish_physx_dll_time  )){ch=true; publishPhysxDll (src.publishPhysxDll ());}
         if(Sync(publish_steam_dll_time  , src.publish_steam_dll_time  )){ch=true; publishSteamDll (src.publishSteamDll ());}
         if(Sync(publish_open_vr_dll_time, src.publish_open_vr_dll_time)){ch=true; publishOpenVRDll(src.publishOpenVRDll());}
         if(Sync(publish_data_as_pak_time, src.publish_data_as_pak_time)){ch=true; publishDataAsPak(src.publishDataAsPak());}
         if(Sync(android_expansion_time  , src.android_expansion_time  )){ch=true; androidExpansion(src.androidExpansion());}
       //if(Sync(windows_code_sign_time  , src.windows_code_sign_time  )){ch=true; windowsCodeSign (src.windowsCodeSign ());}
      }
      ch|=Sync(supported_orientations_time, src.supported_orientations_time, supported_orientations, src.supported_orientations);
      ch|=Sync(                  icon_time, src.                  icon_time, icon                  , src.icon                  );
      ch|=Undo(     notification_icon_time, src.     notification_icon_time, notification_icon     , src.notification_icon     );
      ch|=Sync(        image_portrait_time, src.        image_portrait_time, image_portrait        , src.image_portrait        );
      ch|=Sync(       image_landscape_time, src.       image_landscape_time, image_landscape       , src.image_landscape       );
      ch|=Sync(              gui_skin_time, src.              gui_skin_time, gui_skin              , src.gui_skin              );

      changed|=ch*CHANGE_NORMAL;
      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   bool ElmApp::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(16);
      f<<dirs_windows<<dirs_nonwindows;
      f<<headers_windows<<headers_mac<<headers_linux<<headers_android<<headers_ios;
      f<<libs_windows<<libs_mac<<libs_linux<<libs_android<<libs_ios;
      f<<package<<android_license_key<<location_usage_reason<<build<<storage<<supported_orientations<<flag;
      f<<fb_app_id;
      f<<cb_app_id_ios<<cb_app_signature_ios<<cb_app_id_google<<cb_app_signature_google;
      f<<icon<<notification_icon<<image_portrait<<image_landscape<<gui_skin;
      f<<dirs_windows_time<<dirs_nonwindows_time;
      f<<headers_windows_time<<headers_mac_time<<headers_linux_time<<headers_android_time<<headers_ios_time;
      f<<libs_windows_time<<libs_mac_time<<libs_linux_time<<libs_android_time<<libs_ios_time;
      f<<package_time<<android_license_key_time<<location_usage_reason_time<<build_time<<storage_time<<supported_orientations_time;
      f<<fb_app_id_time;
      f<<cb_app_id_ios_time<<cb_app_signature_ios_time<<cb_app_id_google_time<<cb_app_signature_google_time;
      f<<embed_engine_data_time<<publish_proj_data_time<<publish_physx_dll_time<<publish_steam_dll_time<<publish_open_vr_dll_time<<publish_data_as_pak_time<<android_expansion_time;
      f<<icon_time<<notification_icon_time<<image_portrait_time<<image_landscape_time<<gui_skin_time;
      return f.ok();
   }
   bool ElmApp::load(File &f)
{
      Str       headers_nonwindows;
      TimeStamp headers_nonwindows_time;
      T=ElmApp(); // reset to default, in case this is needed (for example when loading data from reused objects for code synchronization)
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 16:
         {
            f>>dirs_windows>>dirs_nonwindows;
            f>>headers_windows>>headers_mac>>headers_linux>>headers_android>>headers_ios;
            f>>libs_windows>>libs_mac>>libs_linux>>libs_android>>libs_ios;
            f>>package>>android_license_key>>location_usage_reason>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>cb_app_id_ios>>cb_app_signature_ios>>cb_app_id_google>>cb_app_signature_google;
            f>>icon>>notification_icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time;
            f>>headers_windows_time>>headers_mac_time>>headers_linux_time>>headers_android_time>>headers_ios_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>cb_app_id_ios_time>>cb_app_signature_ios_time>>cb_app_id_google_time>>cb_app_signature_google_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_steam_dll_time>>publish_open_vr_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>notification_icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 15:
         {
            f>>dirs_windows>>dirs_nonwindows>>headers_windows>>headers_nonwindows>>libs_windows>>libs_mac>>libs_linux>>libs_android>>libs_ios;
            f>>package>>android_license_key>>location_usage_reason>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>cb_app_id_ios>>cb_app_signature_ios>>cb_app_id_google>>cb_app_signature_google;
            f>>icon>>notification_icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>cb_app_id_ios_time>>cb_app_signature_ios_time>>cb_app_id_google_time>>cb_app_signature_google_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_steam_dll_time>>publish_open_vr_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>notification_icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 14:
         {
            f>>dirs_windows>>dirs_nonwindows>>headers_windows>>headers_nonwindows>>libs_windows>>libs_mac>>libs_linux>>libs_android>>libs_ios;
            f>>package>>android_license_key>>location_usage_reason>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>cb_app_id_ios>>cb_app_signature_ios>>cb_app_id_google>>cb_app_signature_google;
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>cb_app_id_ios_time>>cb_app_signature_ios_time>>cb_app_id_google_time>>cb_app_signature_google_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_steam_dll_time>>publish_open_vr_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 13:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); GetStr2(f, android_license_key); GetStr2(f, location_usage_reason); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            GetStr2(f, cb_app_id_ios); GetStr2(f, cb_app_signature_ios); GetStr2(f, cb_app_id_google); GetStr2(f, cb_app_signature_google);
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>cb_app_id_ios_time>>cb_app_signature_ios_time>>cb_app_id_google_time>>cb_app_signature_google_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_steam_dll_time>>publish_open_vr_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 12:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); GetStr2(f, android_license_key); GetStr2(f, location_usage_reason); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            GetStr2(f, cb_app_id_ios); GetStr2(f, cb_app_signature_ios); GetStr2(f, cb_app_id_google); GetStr2(f, cb_app_signature_google);
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>cb_app_id_ios_time>>cb_app_signature_ios_time>>cb_app_id_google_time>>cb_app_signature_google_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_steam_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 11:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); GetStr2(f, android_license_key); GetStr2(f, location_usage_reason); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id; GetStr2(f, cb_app_id_ios); GetStr2(f, cb_app_signature_ios);
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time>>cb_app_id_ios_time>>cb_app_signature_ios_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_steam_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 10:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); GetStr2(f, android_license_key); GetStr2(f, location_usage_reason); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_steam_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 9:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); GetStr2(f, android_license_key); GetStr2(f, location_usage_reason); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time>>android_expansion_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 8:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); GetStr2(f, android_license_key); GetStr2(f, location_usage_reason); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>android_license_key_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 7:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); GetStr2(f, location_usage_reason); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>location_usage_reason_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 6:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id;
            f>>icon>>image_portrait>>image_landscape>>gui_skin;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time>>gui_skin_time;
            if(f.ok())return true;
         }break;

         case 5:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios);
            GetStr2(f, package); f>>build>>storage>>supported_orientations>>flag;
            f>>fb_app_id; GetStr2(f);
            f>>icon>>image_portrait>>image_landscape;
            f>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>build_time>>storage_time>>supported_orientations_time;
            f>>fb_app_id_time; TimeStamp ts; f>>ts;
            f>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time;
            f>>icon_time>>image_portrait_time>>image_landscape_time;
            if(f.ok())return true;
         }break;

         case 4:
         {
            GetStr2(f, dirs_windows); GetStr2(f, dirs_nonwindows); GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios); GetStr2(f, package);
            f>>build>>storage>>supported_orientations>>flag>>icon>>image_portrait>>image_landscape>>dirs_windows_time>>dirs_nonwindows_time>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>build_time>>storage_time>>supported_orientations_time>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time>>icon_time>>image_portrait_time>>image_landscape_time;
            if(f.ok())return true;
         }break;

         case 3:
         {
            GetStr2(f, headers_windows); GetStr2(f, headers_nonwindows); GetStr2(f, libs_windows); GetStr2(f, libs_mac); GetStr2(f, libs_linux); GetStr2(f, libs_android); GetStr2(f, libs_ios); GetStr2(f, package);
            f>>build>>storage>>supported_orientations>>flag>>icon>>image_portrait>>image_landscape>>headers_windows_time>>headers_nonwindows_time;
            f>>libs_windows_time>>libs_mac_time>>libs_linux_time>>libs_android_time>>libs_ios_time;
            f>>package_time>>build_time>>storage_time>>supported_orientations_time>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time>>icon_time>>image_portrait_time>>image_landscape_time;
            if(f.ok())return true;
         }break;

         case 2:
         {
            GetStr(f, headers_windows); GetStr(f, libs_windows); GetStr(f, package); f>>build>>storage>>supported_orientations>>flag>>icon>>image_portrait>>image_landscape>>headers_windows_time>>libs_windows_time>>package_time>>build_time>>storage_time>>supported_orientations_time>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time>>icon_time>>image_portrait_time>>image_landscape_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            GetStr(f, headers_windows); GetStr(f, libs_windows); GetStr(f, package); f>>build>>storage>>supported_orientations>>flag>>icon>>image_portrait>>headers_windows_time>>libs_windows_time>>package_time>>build_time>>storage_time>>supported_orientations_time>>embed_engine_data_time>>publish_proj_data_time>>publish_physx_dll_time>>publish_data_as_pak_time>>icon_time>>image_portrait_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            bool embed_engine_data;
            GetStr(f, headers_windows); GetStr(f, libs_windows); GetStr(f, package); f>>build>>storage>>supported_orientations>>embed_engine_data>>icon>>image_portrait>>headers_windows_time>>libs_windows_time>>package_time>>build_time>>storage_time>>supported_orientations_time>>embed_engine_data_time>>icon_time>>image_portrait_time;
            embedEngineData(embed_engine_data).publishProjData(true).publishPhysxDll(true).publishDataAsPak(true).androidExpansion(false); // set non-saved options
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmApp::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
                      nodes.New().set("Build"  , build);
      if(package.is())nodes.New().set("Package", package);

      if(icon             .valid())nodes.New().setFN("Icon"            , icon);
      if(notification_icon.valid())nodes.New().setFN("NotificationIcon", notification_icon);
      if(image_portrait   .valid())nodes.New().setFN("SplashPortrait"  , image_portrait);
      if(image_landscape  .valid())nodes.New().setFN("SplashLandscape" , image_landscape);
      if(gui_skin         .valid())nodes.New().setFN("GuiSkin"         , gui_skin);

      if( embedEngineData ())nodes.New().set("EmbedEngineData"   , embedEngineData ());
      if(!publishProjData ())nodes.New().set("PublishProjectData", publishProjData ());
      if(!publishDataAsPak())nodes.New().set("PublishDataAsPAK"  , publishDataAsPak());
                             nodes.New().set("PublishPhysXDLL"   , publishPhysxDll ());
      if( publishSteamDll ())nodes.New().set("PublishSteamDLL" );
      if( publishOpenVRDll())nodes.New().set("PublishOpenVRDLL");

      if(   dirs_windows   .is())nodes.New().set("IncludePathsWindows"     ,    dirs_windows);
      if(   dirs_nonwindows.is())nodes.New().set("IncludePathsNonWindows"  ,    dirs_nonwindows);
      if(headers_windows   .is())nodes.New().set("IncludeHeadersWindows"   , headers_windows);
      if(headers_mac       .is())nodes.New().set("IncludeHeadersMac"       , headers_mac);
      if(headers_linux     .is())nodes.New().set("IncludeHeadersLinux"     , headers_linux);
      if(headers_android   .is())nodes.New().set("IncludeHeadersAndroid"   , headers_android);
      if(headers_ios       .is())nodes.New().set("IncludeHeadersiOS"       , headers_ios);
      if(   libs_windows   .is())nodes.New().set("IncludeLibrariesWindows" ,    libs_windows);
      if(   libs_mac       .is())nodes.New().set("IncludeLibrariesMac"     ,    libs_mac);
      if(   libs_linux     .is())nodes.New().set("IncludeLibrariesLinux"   ,    libs_linux);
      if(   libs_android   .is())nodes.New().set("IncludeLibrariesAndroid" ,    libs_android);
      if(   libs_ios       .is())nodes.New().set("IncludeLibrariesiOS"     ,    libs_ios);

      REPA(StorageModes)if(storage==StorageModes[i].mode){nodes.New().set("Storage", StorageModes[i].name); break;}
                                    nodes.New().set("SupportedOrientations", supported_orientations);
      if(location_usage_reason.is())nodes.New().set("LocationUsageReason"  , location_usage_reason);
      if(androidExpansion        ())nodes.New().set("AndroidExpansion"     );
      if(android_license_key  .is())nodes.New().set("AndroidLicenseKey"    , android_license_key);

      if(fb_app_id)nodes.New().set("FacebookAppID", fb_app_id);

      {
         TextNode &cb=nodes.New().setName("Chartboost");
         if(cb_app_id_ios          .is())cb.nodes.New().set("AppIDiOS"          , cb_app_id_ios);
         if(cb_app_signature_ios   .is())cb.nodes.New().set("AppSignatureiOS"   , cb_app_signature_ios);
         if(cb_app_id_google       .is())cb.nodes.New().set("AppIDGoogle"       , cb_app_id_google);
         if(cb_app_signature_google.is())cb.nodes.New().set("AppSignatureGoogle", cb_app_signature_google);
         cb.nodes.New().set("AppIDiOSTime"          , cb_app_id_ios_time.text());
         cb.nodes.New().set("AppSignatureiOSTime"   , cb_app_signature_ios_time.text());
         cb.nodes.New().set("AppIDGoogleTime"       , cb_app_id_google_time.text());
         cb.nodes.New().set("AppSignatureGoogleTime", cb_app_signature_google_time.text());
      }

      nodes.New().set("BuildTime"  ,   build_time.text());
      nodes.New().set("PackageTime", package_time.text());

      nodes.New().set("IconTime"            , icon_time.text());
      nodes.New().set("NotificationIconTime", notification_icon_time.text());
      nodes.New().set("SplashPortraitTime"  , image_portrait_time.text());
      nodes.New().set("SplashLandscapeTime" , image_landscape_time.text());
      nodes.New().set("GuiSkinTime"         , gui_skin_time.text());

      nodes.New().set("EmbedEngineDataTime"   , embed_engine_data_time  .text());
      nodes.New().set("PublishProjectDataTime", publish_proj_data_time  .text());
      nodes.New().set("PublishDataAsPAKTime"  , publish_data_as_pak_time.text());
      nodes.New().set("PublishPhysXDLLTime"   , publish_physx_dll_time  .text());
      nodes.New().set("PublishSteamDLLTime"   , publish_steam_dll_time  .text());
      nodes.New().set("PublishOpenVRDLLTime"  , publish_open_vr_dll_time.text());

      nodes.New().set("IncludePathsWindowsTime"     , dirs_windows_time.text());
      nodes.New().set("IncludePathsNonWindowsTime"  , dirs_nonwindows_time.text());
      nodes.New().set("IncludeHeadersWindowsTime"   , headers_windows_time.text());
      nodes.New().set("IncludeHeadersMacTime"       , headers_mac_time.text());
      nodes.New().set("IncludeHeadersLinuxTime"     , headers_linux_time.text());
      nodes.New().set("IncludeHeadersAndroidTime"   , headers_android_time.text());
      nodes.New().set("IncludeHeadersiOSTime"       , headers_ios_time.text());
      nodes.New().set("IncludeLibrariesWindowsTime" , libs_windows_time.text());
      nodes.New().set("IncludeLibrariesMacTime"     , libs_mac_time.text());
      nodes.New().set("IncludeLibrariesLinuxTime"   , libs_linux_time.text());
      nodes.New().set("IncludeLibrariesAndroidTime" , libs_android_time.text());
      nodes.New().set("IncludeLibrariesiOSTime"     , libs_ios_time.text());

      nodes.New().set("StorageTime"              , storage_time.text());
      nodes.New().set("SupportedOrientationsTime", supported_orientations_time.text());
      nodes.New().set("LocationUsageReasonTime"  , location_usage_reason_time.text());
      nodes.New().set("AndroidExpansionTime"     , android_expansion_time.text());
      nodes.New().set("AndroidLicenseKeyTime"    , android_license_key_time.text());

      nodes.New().set("FacebookAppIDTime", fb_app_id_time.text());
   }
   void ElmApp::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="Build"                       )n.getValue(build  );else
         if(n.name=="Package"                     )n.getValue(package);else

         if(n.name=="Icon"                        )n.getValue(icon             );else
         if(n.name=="NotificationIcon"            )n.getValue(notification_icon);else
         if(n.name=="SplashPortrait"              )n.getValue(image_portrait   );else
         if(n.name=="SplashLandscape"             )n.getValue(image_landscape  );else
         if(n.name=="GuiSkin"                     )n.getValue(gui_skin         );else

         if(n.name=="EmbedEngineData"             )embedEngineData (n.value.is() ? n.asInt() : true);else
         if(n.name=="PublishProjectData"          )publishProjData (n.asBool1());else
         if(n.name=="PublishDataAsPAK"            )publishDataAsPak(n.asBool1());else
         if(n.name=="PublishPhysXDLL"             )publishPhysxDll (n.asBool1());else
         if(n.name=="PublishSteamDLL"             )publishSteamDll (n.asBool1());else
         if(n.name=="PublishOpenVRDLL"            )publishOpenVRDll(n.asBool1());else

         if(n.name=="IncludePathsWindows"         )n.getValue(   dirs_windows   );else
         if(n.name=="IncludePathsNonWindows"      )n.getValue(   dirs_nonwindows);else
         if(n.name=="IncludeHeadersWindows"       )n.getValue(headers_windows   );else
         if(n.name=="IncludeHeadersMac"           )n.getValue(headers_mac       );else
         if(n.name=="IncludeHeadersLinux"         )n.getValue(headers_linux     );else
         if(n.name=="IncludeHeadersAndroid"       )n.getValue(headers_android   );else
         if(n.name=="IncludeHeadersiOS"           )n.getValue(headers_ios       );else
         if(n.name=="IncludeLibrariesWindows"     )n.getValue(   libs_windows   );else
         if(n.name=="IncludeLibrariesMac"         )n.getValue(   libs_mac       );else
         if(n.name=="IncludeLibrariesLinux"       )n.getValue(   libs_linux     );else
         if(n.name=="IncludeLibrariesAndroid"     )n.getValue(   libs_android   );else
         if(n.name=="IncludeLibrariesiOS"         )n.getValue(   libs_ios       );else

         if(n.name=="Storage"                     ){REPA(StorageModes)if(n.value==StorageModes[i].name){storage=StorageModes[i].mode; break;}}else
         if(n.name=="SupportedOrientations"       )supported_orientations=n.asInt();else
         if(n.name=="LocationUsageReason"         )n.getValue(location_usage_reason);else
         if(n.name=="AndroidExpansion"            )androidExpansion(n.asBool1());else
         if(n.name=="AndroidLicenseKey"           )n.getValue(android_license_key);else

         if(n.name=="FacebookAppID"               )n.getValue(fb_app_id);else
         if(n.name=="Chartboost"                  )
         {
            REPA(n.nodes)
            {
             C TextNode &cb=n.nodes[i];
               if(cb.name=="AppIDiOS"              )cb.getValue(cb_app_id_ios          );else
               if(cb.name=="AppSignatureiOS"       )cb.getValue(cb_app_signature_ios   );else
               if(cb.name=="AppIDGoogle"           )cb.getValue(cb_app_id_google       );else
               if(cb.name=="AppSignatureGoogle"    )cb.getValue(cb_app_signature_google);else
               if(cb.name=="AppIDiOSTime"          )cb_app_id_ios_time          =cb.asText();else
               if(cb.name=="AppSignatureiOSTime"   )cb_app_signature_ios_time   =cb.asText();else
               if(cb.name=="AppIDGoogleTime"       )cb_app_id_google_time       =cb.asText();else
               if(cb.name=="AppSignatureGoogleTime")cb_app_signature_google_time=cb.asText();
            }
         }else
         if(n.name=="BuildTime"  )build_time=n.asText();else
         if(n.name=="PackageTime")package_time=n.asText();else

         if(n.name=="IconTime"            )icon_time=n.asText();else
         if(n.name=="NotificationIconTime")notification_icon_time=n.asText();else
         if(n.name=="SplashPortraitTime"  )image_portrait_time=n.asText();else
         if(n.name=="SplashLandscapeTime" )image_landscape_time=n.asText();else
         if(n.name=="GuiSkinTime"         )gui_skin_time=n.asText();else

         if(n.name=="EmbedEngineDataTime"   )embed_engine_data_time  =n.asText();else
         if(n.name=="PublishProjectDataTime")publish_proj_data_time  =n.asText();else
         if(n.name=="PublishDataAsPAKTime"  )publish_data_as_pak_time=n.asText();else
         if(n.name=="PublishPhysXDLLTime"   )publish_physx_dll_time  =n.asText();else
         if(n.name=="PublishSteamDLLTime"   )publish_steam_dll_time  =n.asText();else
         if(n.name=="PublishOpenVRDLLTime"  )publish_open_vr_dll_time=n.asText();else

         if(n.name=="IncludePathsWindowsTime"     )dirs_windows_time=n.asText();else
         if(n.name=="IncludePathsNonWindowsTime"  )dirs_nonwindows_time=n.asText();else
         if(n.name=="IncludeHeadersWindowsTime"   )headers_windows_time=n.asText();else
         if(n.name=="IncludeHeadersMacTime"       )headers_mac_time=n.asText();else
         if(n.name=="IncludeHeadersLinuxTime"     )headers_linux_time=n.asText();else
         if(n.name=="IncludeHeadersAndroidTime"   )headers_android_time=n.asText();else
         if(n.name=="IncludeHeadersiOSTime"       )headers_ios_time=n.asText();else
         if(n.name=="IncludeLibrariesWindowsTime" )libs_windows_time=n.asText();else
         if(n.name=="IncludeLibrariesMacTime"     )libs_mac_time=n.asText();else
         if(n.name=="IncludeLibrariesLinuxTime"   )libs_linux_time=n.asText();else
         if(n.name=="IncludeLibrariesAndroidTime" )libs_android_time=n.asText();else
         if(n.name=="IncludeLibrariesiOSTime"     )libs_ios_time=n.asText();else

         if(n.name=="StorageTime"              )storage_time=n.asText();else
         if(n.name=="SupportedOrientationsTime")supported_orientations_time=n.asText();else
         if(n.name=="LocationUsageReasonTime"  )location_usage_reason_time=n.asText();else
         if(n.name=="AndroidExpansionTime"     )android_expansion_time=n.asText();else
         if(n.name=="AndroidLicenseKeyTime"    )android_license_key_time=n.asText();else

         if(n.name=="FacebookAppIDTime")fb_app_id_time=n.asText();
      }
   }
   bool ElmMiniMap::equal(C ElmMiniMap &src)C {return ::ElmData::equal(src) && areas_per_image_time==src.areas_per_image_time && image_size_time==src.image_size_time && world_time==src.world_time && env_time==src.env_time;}
   bool ElmMiniMap::newer(C ElmMiniMap &src)C {return ::ElmData::newer(src) || areas_per_image_time> src.areas_per_image_time || image_size_time> src.image_size_time || world_time> src.world_time || env_time> src.env_time;}
   bool ElmMiniMap::mayContain(C UID &id)C {return id==world_id || id==env_id;}
   void ElmMiniMap::newData(){::ElmData::newData(); areas_per_image_time++; image_size_time++; world_time++; env_time++;}
   uint ElmMiniMap::undo(C ElmMiniMap &src)
   {
      uint changed=::ElmData::undo(src);

      changed|=Undo(areas_per_image_time, src.areas_per_image_time, areas_per_image, src.areas_per_image)*CHANGE_AFFECT_FILE;
      changed|=Undo(     image_size_time, src.     image_size_time,      image_size, src.     image_size)*CHANGE_AFFECT_FILE;
      changed|=Undo(          world_time, src.          world_time,        world_id, src.       world_id)*CHANGE_AFFECT_FILE;
      changed|=Undo(            env_time, src.            env_time,          env_id, src.         env_id)*CHANGE_AFFECT_FILE;

      if(changed)newVer();
      return changed;
   }
   uint ElmMiniMap::sync(C ElmMiniMap &src)
   {
      uint changed=::ElmData::sync(src);

      changed|=Sync(areas_per_image_time, src.areas_per_image_time, areas_per_image, src.areas_per_image)*CHANGE_AFFECT_FILE;
      changed|=Sync(     image_size_time, src.     image_size_time,      image_size, src.     image_size)*CHANGE_AFFECT_FILE;
      changed|=Sync(          world_time, src.          world_time,        world_id, src.       world_id)*CHANGE_AFFECT_FILE;
      changed|=Sync(            env_time, src.            env_time,          env_id, src.         env_id)*CHANGE_AFFECT_FILE;

      if(equal(src))ver=src.ver;else if(changed)newVer();
      return changed;
   }
   void ElmMiniMap::copyTo(Game::MiniMap::Settings &settings)C
   {
      settings.image_size     =image_size;
      settings.areas_per_image=areas_per_image;
   }
   bool ElmMiniMap::save(File &f)C 
{
      ::ElmData::save(f);
      f.cmpUIntV(0);
      f<<areas_per_image<<image_size<<world_id<<env_id<<areas_per_image_time<<image_size_time<<world_time<<env_time;
      return f.ok();
   }
   bool ElmMiniMap::load(File &f)
{
      if(::ElmData::load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>areas_per_image>>image_size>>world_id>>env_id>>areas_per_image_time>>image_size_time>>world_time>>env_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void ElmMiniMap::save(MemPtr<TextNode> nodes)C 
{
      ::ElmData::save(nodes);
                          nodes.New().set  ("AreasPerImage"    , areas_per_image);
                          nodes.New().set  ("ImageSize"        , image_size);
      if(world_id.valid())nodes.New().setFN("World"            , world_id);
      if(  env_id.valid())nodes.New().setFN("Environment"      ,   env_id);
                          nodes.New().set  ("AreasPerImageTime", areas_per_image_time.text());
                          nodes.New().set  ("ImageSizeTime"    ,      image_size_time.text());
                          nodes.New().set  ("WorldTime"        ,           world_time.text());
                          nodes.New().set  ("EnvironmentTime"  ,             env_time.text());
   }
   void ElmMiniMap::load(C MemPtr<TextNode> &nodes)
{
      ::ElmData::load(nodes);
      REPA(nodes)
      {
       C TextNode &n=nodes[i];
         if(n.name=="AreasPerImage"    )n.getValue(areas_per_image);else
         if(n.name=="ImageSize"        )n.getValue(image_size);else
         if(n.name=="World"            )n.getValue(world_id);else
         if(n.name=="Environment"      )n.getValue(  env_id);else
         if(n.name=="AreasPerImageTime")areas_per_image_time=n.asText();else
         if(n.name=="ImageSizeTime"    )     image_size_time=n.asText();else
         if(n.name=="WorldTime"        )          world_time=n.asText();else
         if(n.name=="EnvironmentTime"  )            env_time=n.asText();
      }
   }
  Elm::~Elm() {Delete(data);}
   Elm& Elm::copyParams(C Elm &src, bool set_parent) // copy parameters from 'src', 'type' must be already specified, doesn't copy ID
   {
      if(this!=&src && type==src.type)
      {
         Delete(data);
         flag=src.flag;
         name=src.name;
         name_time=src.      name_time;
      removed_time=src.   removed_time;
   no_publish_time=src.no_publish_time;
         if(set_parent)
         {
            parent_id  =src.parent_id;
            parent_time=src.parent_time;
         }

         // copy data
         if(!src.data)Delete(data);else // if source has no data then delete it
         if(ElmData *data=Data()) // if created data then copy it
         {
            switch(type)
            {
               case ELM_OBJ_CLASS  : *  objClassData()=*src.  objClassData(); break;
               case ELM_OBJ        : *       objData()=*src.       objData(); break;
               case ELM_MESH       : *      meshData()=*src.      meshData(); break;
               case ELM_MTRL       : *      mtrlData()=*src.      mtrlData(); break;
               case ELM_WATER_MTRL : * waterMtrlData()=*src. waterMtrlData(); break;
               case ELM_PHYS_MTRL  : *  physMtrlData()=*src.  physMtrlData(); break;
               case ELM_SKEL       : *      skelData()=*src.      skelData(); break;
               case ELM_PHYS       : *      physData()=*src.      physData(); break;
               case ELM_ANIM       : *      animData()=*src.      animData(); break;
               case ELM_PANEL_IMAGE: *panelImageData()=*src.panelImageData(); break;
               case ELM_ENV        : *       envData()=*src.       envData(); break;
               case ELM_WORLD      : *     worldData()=*src.     worldData(); break;
               case ELM_MINI_MAP   : *   miniMapData()=*src.   miniMapData(); break;
               case ELM_ENUM       : *      enumData()=*src.      enumData(); break;
               case ELM_IMAGE      : *     imageData()=*src.     imageData(); break;
               case ELM_IMAGE_ATLAS: *imageAtlasData()=*src.imageAtlasData(); break;
               case ELM_ICON_SETTS : * iconSettsData()=*src. iconSettsData(); break;
               case ELM_ICON       : *      iconData()=*src.      iconData(); break;
               case ELM_FONT       : *      fontData()=*src.      fontData(); break;
               case ELM_TEXT_STYLE : * textStyleData()=*src. textStyleData(); break;
               case ELM_PANEL      : *     panelData()=*src.     panelData(); break;
               case ELM_GUI_SKIN   : *   guiSkinData()=*src.   guiSkinData(); break;
               case ELM_GUI        : *       guiData()=*src.       guiData(); break;
               case ELM_SOUND      : *     soundData()=*src.     soundData(); break;
               case ELM_VIDEO      : *     videoData()=*src.     videoData(); break;
               case ELM_FILE       : *      fileData()=*src.      fileData(); break;
               case ELM_CODE       : *      codeData()=*src.      codeData(); break;
               case ELM_APP        : *       appData()=*src.       appData(); break;
            }
            if(id!=src.id)data->clearLinked(); // if we're copying as a different object (duplicating) then we need to clear linked
         }
      }
      return T;
   }
   void Elm::operator=(C Elm &src)
   {
      // set values before calling 'copyParams'
      id  =src.id;
      type=src.type;
      copyParams(src);
   }
   Elm::Elm(C Elm &src) : type(ELM_NONE), flag(0), file_size(-1), id(UID().randomizeValid()), parent_id(UIDZero), data(null) {T=src;}
   bool Elm::importing()C {return  FlagTest(flag, IMPORTING       );}
   Elm& Elm::importing(bool on) {FlagSet(flag, IMPORTING       ,  on); return T;}
   bool Elm::opened()C {return  FlagTest(flag, OPENED          );}
   Elm& Elm::opened(bool on) {FlagSet(flag, OPENED          ,  on); return T;}
   bool Elm::removed()C {return  FlagTest(flag, REMOVED         );}
   Elm& Elm::removed(bool on) {FlagSet(flag, REMOVED         ,  on); return T;}
   bool Elm::publish()C {return !FlagTest(flag, NO_PUBLISH      );}
   Elm& Elm::publish(bool on) {FlagSet(flag, NO_PUBLISH      , !on); return T;}
   bool Elm::noPublish()C {return  FlagTest(flag, NO_PUBLISH      );}
   Elm& Elm::noPublish(bool on) {FlagSet(flag, NO_PUBLISH      ,  on); return T;}
   bool Elm::finalRemoved()C {return  FlagTest(flag, FINAL_REMOVED   );}
   Elm& Elm::finalRemoved(bool on) {FlagSet(flag, FINAL_REMOVED   ,  on); return T;}
   bool Elm::finalExists()C {return !FlagTest(flag, FINAL_REMOVED   );}
   Elm& Elm::finalExists(bool on) {FlagSet(flag, FINAL_REMOVED   , !on); return T;}
   bool Elm::finalPublish()C {return !FlagTest(flag, FINAL_NO_PUBLISH);}
   Elm& Elm::finalPublish(bool on) {FlagSet(flag, FINAL_NO_PUBLISH, !on); return T;}
   bool Elm::finalNoPublish()C {return  FlagTest(flag, FINAL_NO_PUBLISH);}
   Elm& Elm::finalNoPublish(bool on) {FlagSet(flag, FINAL_NO_PUBLISH,  on); return T;}
 C Str& Elm::srcFile()C {return data ?  data->src_file : S;}
   bool Elm::initialized()C {return data && data->ver;}
   void Elm::resetFinal() {FlagEnable(flag, FINAL_REMOVED|FINAL_NO_PUBLISH);}
   Elm& Elm::setRemoved(  bool removed   , C TimeStamp &time) {T.removed  (removed   ); T.   removed_time=time; return T;}
   Elm& Elm::setNoPublish(  bool no_publish, C TimeStamp &time) {T.noPublish(no_publish); T.no_publish_time=time; return T;}
   Elm& Elm::setName(C Str &name      , C TimeStamp &time) {T.name     =name       ; T.      name_time=time; return T;}
   Elm& Elm::setParent(C UID &parent_id , C TimeStamp &time) {T.parent_id=parent_id  ; T.    parent_time=time; return T;}
   Elm& Elm::setParent(C Elm *parent    , C TimeStamp &time) {return setParent(parent ? parent->id : UIDZero, time);}
   Elm& Elm::setSrcFile(C Str &src_file  , C TimeStamp &time) {if(ElmData *data=Data()){data->setSrcFile(src_file, time); data->newVer();} return T;}
   ElmObjClass  *   Elm::objClassData() {if(type==ELM_OBJ_CLASS  ){if(!data)data=new ElmObjClass  ; return CAST(ElmObjClass  , data);} return null;}
   C ElmObjClass  *   Elm::objClassData()C {return (type==ELM_OBJ_CLASS  ) ? CAST(ElmObjClass  , data) : null;}
   ElmObj       *        Elm::objData() {if(type==ELM_OBJ        ){if(!data)data=new ElmObj       ; return CAST(ElmObj       , data);} return null;}
   C ElmObj       *        Elm::objData()C {return (type==ELM_OBJ        ) ? CAST(ElmObj       , data) : null;}
   ElmMesh      *       Elm::meshData() {if(type==ELM_MESH       ){if(!data)data=new ElmMesh      ; return CAST(ElmMesh      , data);} return null;}
   C ElmMesh      *       Elm::meshData()C {return (type==ELM_MESH       ) ? CAST(ElmMesh      , data) : null;}
   ElmMaterial  *       Elm::mtrlData() {if(type==ELM_MTRL       ){if(!data)data=new ElmMaterial  ; return CAST(ElmMaterial  , data);} return null;}
   C ElmMaterial  *       Elm::mtrlData()C {return (type==ELM_MTRL       ) ? CAST(ElmMaterial  , data) : null;}
   ElmWaterMtrl *  Elm::waterMtrlData() {if(type==ELM_WATER_MTRL ){if(!data)data=new ElmWaterMtrl ; return CAST(ElmWaterMtrl , data);} return null;}
   C ElmWaterMtrl *  Elm::waterMtrlData()C {return (type==ELM_WATER_MTRL ) ? CAST(ElmWaterMtrl , data) : null;}
   ElmPhysMtrl  *   Elm::physMtrlData() {if(type==ELM_PHYS_MTRL  ){if(!data)data=new ElmPhysMtrl  ; return CAST(ElmPhysMtrl  , data);} return null;}
   C ElmPhysMtrl  *   Elm::physMtrlData()C {return (type==ELM_PHYS_MTRL  ) ? CAST(ElmPhysMtrl  , data) : null;}
   ElmSkel      *       Elm::skelData() {if(type==ELM_SKEL       ){if(!data)data=new ElmSkel      ; return CAST(ElmSkel      , data);} return null;}
   C ElmSkel      *       Elm::skelData()C {return (type==ELM_SKEL       ) ? CAST(ElmSkel      , data) : null;}
   ElmPhys      *       Elm::physData() {if(type==ELM_PHYS       ){if(!data)data=new ElmPhys      ; return CAST(ElmPhys      , data);} return null;}
   C ElmPhys      *       Elm::physData()C {return (type==ELM_PHYS       ) ? CAST(ElmPhys      , data) : null;}
   ElmAnim      *       Elm::animData() {if(type==ELM_ANIM       ){if(!data)data=new ElmAnim      ; return CAST(ElmAnim      , data);} return null;}
   C ElmAnim      *       Elm::animData()C {return (type==ELM_ANIM       ) ? CAST(ElmAnim      , data) : null;}
   ElmPanelImage* Elm::panelImageData() {if(type==ELM_PANEL_IMAGE){if(!data)data=new ElmPanelImage; return CAST(ElmPanelImage, data);} return null;}
   C ElmPanelImage* Elm::panelImageData()C {return (type==ELM_PANEL_IMAGE) ? CAST(ElmPanelImage, data) : null;}
   ElmEnv       *        Elm::envData() {if(type==ELM_ENV        ){if(!data)data=new ElmEnv       ; return CAST(ElmEnv       , data);} return null;}
   C ElmEnv       *        Elm::envData()C {return (type==ELM_ENV        ) ? CAST(ElmEnv       , data) : null;}
   ElmWorld     *      Elm::worldData() {if(type==ELM_WORLD      ){if(!data)data=new ElmWorld     ; return CAST(ElmWorld     , data);} return null;}
   C ElmWorld     *      Elm::worldData()C {return (type==ELM_WORLD      ) ? CAST(ElmWorld     , data) : null;}
   ElmMiniMap   *    Elm::miniMapData() {if(type==ELM_MINI_MAP   ){if(!data)data=new ElmMiniMap   ; return CAST(ElmMiniMap   , data);} return null;}
   C ElmMiniMap   *    Elm::miniMapData()C {return (type==ELM_MINI_MAP   ) ? CAST(ElmMiniMap   , data) : null;}
   ElmEnum      *       Elm::enumData() {if(type==ELM_ENUM       ){if(!data)data=new ElmEnum      ; return CAST(ElmEnum      , data);} return null;}
   C ElmEnum      *       Elm::enumData()C {return (type==ELM_ENUM       ) ? CAST(ElmEnum      , data) : null;}
   ElmImage     *      Elm::imageData() {if(type==ELM_IMAGE      ){if(!data)data=new ElmImage     ; return CAST(ElmImage     , data);} return null;}
   C ElmImage     *      Elm::imageData()C {return (type==ELM_IMAGE      ) ? CAST(ElmImage     , data) : null;}
   ElmImageAtlas* Elm::imageAtlasData() {if(type==ELM_IMAGE_ATLAS){if(!data)data=new ElmImageAtlas; return CAST(ElmImageAtlas, data);} return null;}
   C ElmImageAtlas* Elm::imageAtlasData()C {return (type==ELM_IMAGE_ATLAS) ? CAST(ElmImageAtlas, data) : null;}
   ElmIconSetts *  Elm::iconSettsData() {if(type==ELM_ICON_SETTS ){if(!data)data=new ElmIconSetts ; return CAST(ElmIconSetts , data);} return null;}
   C ElmIconSetts *  Elm::iconSettsData()C {return (type==ELM_ICON_SETTS ) ? CAST(ElmIconSetts , data) : null;}
   ElmIcon      *       Elm::iconData() {if(type==ELM_ICON       ){if(!data)data=new ElmIcon      ; return CAST(ElmIcon      , data);} return null;}
   C ElmIcon      *       Elm::iconData()C {return (type==ELM_ICON       ) ? CAST(ElmIcon      , data) : null;}
   ElmFont      *       Elm::fontData() {if(type==ELM_FONT       ){if(!data)data=new ElmFont      ; return CAST(ElmFont      , data);} return null;}
   C ElmFont      *       Elm::fontData()C {return (type==ELM_FONT       ) ? CAST(ElmFont      , data) : null;}
   ElmTextStyle *  Elm::textStyleData() {if(type==ELM_TEXT_STYLE ){if(!data)data=new ElmTextStyle ; return CAST(ElmTextStyle , data);} return null;}
   C ElmTextStyle *  Elm::textStyleData()C {return (type==ELM_TEXT_STYLE ) ? CAST(ElmTextStyle , data) : null;}
   ElmPanel     *      Elm::panelData() {if(type==ELM_PANEL      ){if(!data)data=new ElmPanel     ; return CAST(ElmPanel     , data);} return null;}
   C ElmPanel     *      Elm::panelData()C {return (type==ELM_PANEL      ) ? CAST(ElmPanel     , data) : null;}
   ElmGuiSkin   *    Elm::guiSkinData() {if(type==ELM_GUI_SKIN   ){if(!data)data=new ElmGuiSkin   ; return CAST(ElmGuiSkin   , data);} return null;}
   C ElmGuiSkin   *    Elm::guiSkinData()C {return (type==ELM_GUI_SKIN   ) ? CAST(ElmGuiSkin   , data) : null;}
   ElmGui       *        Elm::guiData() {if(type==ELM_GUI        ){if(!data)data=new ElmGui       ; return CAST(ElmGui       , data);} return null;}
   C ElmGui       *        Elm::guiData()C {return (type==ELM_GUI        ) ? CAST(ElmGui       , data) : null;}
   ElmSound     *      Elm::soundData() {if(type==ELM_SOUND      ){if(!data)data=new ElmSound     ; return CAST(ElmSound     , data);} return null;}
   C ElmSound     *      Elm::soundData()C {return (type==ELM_SOUND      ) ? CAST(ElmSound     , data) : null;}
   ElmVideo     *      Elm::videoData() {if(type==ELM_VIDEO      ){if(!data)data=new ElmVideo     ; return CAST(ElmVideo     , data);} return null;}
   C ElmVideo     *      Elm::videoData()C {return (type==ELM_VIDEO      ) ? CAST(ElmVideo     , data) : null;}
   ElmFile      *       Elm::fileData() {if(type==ELM_FILE       ){if(!data)data=new ElmFile      ; return CAST(ElmFile      , data);} return null;}
   C ElmFile      *       Elm::fileData()C {return (type==ELM_FILE       ) ? CAST(ElmFile      , data) : null;}
   ElmCode      *       Elm::codeData() {if(type==ELM_CODE       ){if(!data)data=new ElmCode      ; return CAST(ElmCode      , data);} return null;}
   C ElmCode      *       Elm::codeData()C {return (type==ELM_CODE       ) ? CAST(ElmCode      , data) : null;}
   ElmApp       *        Elm::appData() {if(type==ELM_APP        ){if(!data)data=new ElmApp       ; return CAST(ElmApp       , data);} return null;}
   C ElmApp       *        Elm::appData()C {return (type==ELM_APP        ) ? CAST(ElmApp       , data) : null;}
   ElmData      *           Elm::Data()
   {
      if(!data)switch(type)
      {
         case ELM_OBJ_CLASS  : data=new ElmObjClass  ; break;
         case ELM_OBJ        : data=new ElmObj       ; break;
         case ELM_MESH       : data=new ElmMesh      ; break;
         case ELM_MTRL       : data=new ElmMaterial  ; break;
         case ELM_WATER_MTRL : data=new ElmWaterMtrl ; break;
         case ELM_PHYS_MTRL  : data=new ElmPhysMtrl  ; break;
         case ELM_SKEL       : data=new ElmSkel      ; break;
         case ELM_PHYS       : data=new ElmPhys      ; break;
         case ELM_ANIM       : data=new ElmAnim      ; break;
         case ELM_PANEL_IMAGE: data=new ElmPanelImage; break;
         case ELM_ENV        : data=new ElmEnv       ; break;
         case ELM_WORLD      : data=new ElmWorld     ; break;
         case ELM_MINI_MAP   : data=new ElmMiniMap   ; break;
         case ELM_ENUM       : data=new ElmEnum      ; break;
         case ELM_IMAGE      : data=new ElmImage     ; break;
         case ELM_IMAGE_ATLAS: data=new ElmImageAtlas; break;
         case ELM_ICON_SETTS : data=new ElmIconSetts ; break;
         case ELM_ICON       : data=new ElmIcon      ; break;
         case ELM_FONT       : data=new ElmFont      ; break;
         case ELM_TEXT_STYLE : data=new ElmTextStyle ; break;
         case ELM_PANEL      : data=new ElmPanel     ; break;
         case ELM_GUI_SKIN   : data=new ElmGuiSkin   ; break;
         case ELM_GUI        : data=new ElmGui       ; break;
         case ELM_SOUND      : data=new ElmSound     ; break;
         case ELM_VIDEO      : data=new ElmVideo     ; break;
         case ELM_FILE       : data=new ElmFile      ; break;
         case ELM_CODE       : data=new ElmCode      ; break;
         case ELM_APP        : data=new ElmApp       ; break;
      }
      return data;
   }
   bool Elm::newerFile(C Elm &src)C // if 'this' has newer file than 'src'
   {
      if(type==src.type)switch(type)
      {
         case ELM_MESH       : return       meshData()->file_time>src.      meshData()->file_time;
         case ELM_SKEL       : return       skelData()->file_time>src.      skelData()->file_time;
         case ELM_PHYS       : return       physData()->file_time>src.      physData()->file_time;
         case ELM_ANIM       : return       animData()->file_time>src.      animData()->file_time;
         case ELM_GUI        : return        guiData()->file_time>src.       guiData()->file_time;
         case ELM_IMAGE      : return      imageData()->file_time>src.     imageData()->file_time;
         case ELM_IMAGE_ATLAS: return imageAtlasData()->file_time>src.imageAtlasData()->file_time;
         case ELM_SOUND      : return      soundData()->file_time>src.     soundData()->file_time;
         case ELM_VIDEO      : return      videoData()->file_time>src.     videoData()->file_time;
         case ELM_FILE       : return       fileData()->file_time>src.      fileData()->file_time;
         case ELM_ICON       : return       iconData()->file_time>src.      iconData()->file_time;
      }
      return false;
   }
   bool Elm::newerData(C Elm &src)C // if 'this' has any data newer than 'src'
   {
      if(type==src.type)switch(type)
      {
         case ELM_ENUM       : return       enumData()->newer(*src.      enumData());
         case ELM_IMAGE      : return      imageData()->newer(*src.     imageData());
         case ELM_IMAGE_ATLAS: return imageAtlasData()->newer(*src.imageAtlasData());
         case ELM_FONT       : return       fontData()->newer(*src.      fontData());
         case ELM_TEXT_STYLE : return  textStyleData()->newer(*src. textStyleData());
         case ELM_PANEL      : return      panelData()->newer(*src.     panelData());
         case ELM_GUI        : return        guiData()->newer(*src.       guiData());
         case ELM_MTRL       : return       mtrlData()->newer(*src.      mtrlData());
         case ELM_WATER_MTRL : return  waterMtrlData()->newer(*src. waterMtrlData());
         case ELM_PHYS_MTRL  : return   physMtrlData()->newer(*src.  physMtrlData());
         case ELM_OBJ_CLASS  : return   objClassData()->newer(*src.  objClassData());
         case ELM_OBJ        : return        objData()->newer(*src.       objData());
         case ELM_MESH       : return       meshData()->newer(*src.      meshData());
         case ELM_SKEL       : return       skelData()->newer(*src.      skelData());
         case ELM_PHYS       : return       physData()->newer(*src.      physData());
         case ELM_ANIM       : return       animData()->newer(*src.      animData());
         case ELM_PANEL_IMAGE: return panelImageData()->newer(*src.panelImageData());
         case ELM_ICON       : return       iconData()->newer(*src.      iconData());
         case ELM_ENV        : return        envData()->newer(*src.       envData());
         case ELM_WORLD      : return      worldData()->newer(*src.     worldData());
         case ELM_SOUND      : return      soundData()->newer(*src.     soundData());
         case ELM_VIDEO      : return      videoData()->newer(*src.     videoData());
         case ELM_FILE       : return       fileData()->newer(*src.      fileData());
         case ELM_CODE       : return       codeData()->newer(*src.      codeData());
         case ELM_APP        : return        appData()->newer(*src.       appData());
         case ELM_ICON_SETTS : return  iconSettsData()->newer(*src. iconSettsData());
         case ELM_MINI_MAP   : return    miniMapData()->newer(*src.   miniMapData());
         case ELM_GUI_SKIN   : return    guiSkinData()->newer(*src.   guiSkinData());
      }
      return false;
   }
   uint Elm::syncData(C Elm &src)
   {
      if(type==src.type)switch(type)
      {
         case ELM_ENUM       : return       enumData()->sync(*src.      enumData());
         case ELM_IMAGE      : return      imageData()->sync(*src.     imageData());
         case ELM_IMAGE_ATLAS: return imageAtlasData()->sync(*src.imageAtlasData());
         case ELM_FONT       : return       fontData()->sync(*src.      fontData());
         case ELM_TEXT_STYLE : return  textStyleData()->sync(*src. textStyleData());
         case ELM_PANEL      : return      panelData()->sync(*src.     panelData());
         case ELM_GUI        : return        guiData()->sync(*src.       guiData());
         case ELM_MTRL       : return       mtrlData()->sync(*src.      mtrlData());
         case ELM_WATER_MTRL : return  waterMtrlData()->sync(*src. waterMtrlData());
         case ELM_PHYS_MTRL  : return   physMtrlData()->sync(*src.  physMtrlData());
         case ELM_OBJ_CLASS  : return   objClassData()->sync(*src.  objClassData());
         case ELM_OBJ        : return        objData()->sync(*src.       objData());
         case ELM_MESH       : return       meshData()->sync(*src.      meshData());
         case ELM_SKEL       : return       skelData()->sync(*src.      skelData());
         case ELM_PHYS       : return       physData()->sync(*src.      physData());
         case ELM_ANIM       : return       animData()->sync(*src.      animData());
         case ELM_PANEL_IMAGE: return panelImageData()->sync(*src.panelImageData());
         case ELM_ICON       : return       iconData()->sync(*src.      iconData());
         case ELM_ENV        : return        envData()->sync(*src.       envData());
         case ELM_WORLD      : return      worldData()->sync(*src.     worldData());
         case ELM_SOUND      : return      soundData()->sync(*src.     soundData());
         case ELM_VIDEO      : return      videoData()->sync(*src.     videoData());
         case ELM_FILE       : return       fileData()->sync(*src.      fileData());
         case ELM_CODE       : return       codeData()->sync(*src.      codeData());
         case ELM_APP        : return        appData()->sync(*src.       appData(), false);
         case ELM_ICON_SETTS : return  iconSettsData()->sync(*src. iconSettsData());
         case ELM_MINI_MAP   : return    miniMapData()->sync(*src.   miniMapData());
         case ELM_GUI_SKIN   : return    guiSkinData()->sync(*src.   guiSkinData());
      }
      return 0;
   }
   bool Elm::save(File &f, bool network, bool skip_name_data)C
   {
      byte flag=T.flag;
      if(data   )flag|=DATA;
      if(network)FlagDisable(flag, IMPORTING); // don't transmit IMPORTING status over network, to make reload requests local only, so one reload doesn't trigger reload on all connected computers
      f.cmpUIntV(3);
      f<<id<<parent_id<<type<<flag;
      f<<name_time<<parent_time<<removed_time<<no_publish_time;
      if(!skip_name_data)f<<name;

      // data
      if(data)if(skip_name_data)f<<data->ver;
              else                 data->save(f);
      return f.ok();
   }
   bool Elm::load(File &f, bool network, bool skip_name_data)
   {
      switch(f.decUIntV())
      {
         case 3:
         {
            f>>id>>parent_id>>type>>flag; if(type>=ELM_NUM)return false;
            f>>name_time>>parent_time>>removed_time>>no_publish_time;
            if(!skip_name_data)f>>name;

            // data
            Delete(data); if(flag&DATA) // if has data
            {
               FlagDisable(flag, DATA);
               ElmData *data=Data(); if(!data)return false;
               if(skip_name_data)f>>data->ver;
               else             if(!data->load(f))return false;
            }
            if(f.ok())return true;
         }break;

         case 2:
         {
            f>>id>>parent_id>>type>>flag; if(type>=ELM_NUM)return false;
            f>>name_time>>parent_time>>removed_time>>no_publish_time;
            if(!skip_name_data)GetStr2(f, name);

            // data
            Delete(data); if(flag&DATA) // if has data
            {
               FlagDisable(flag, DATA);
               ElmData *data=Data(); if(!data)return false;
               if(skip_name_data)f>>data->ver;
               else             if(!data->load(f))return false;
            }
            if(f.ok())return true;
         }break;

         case 1:
         {
            f>>id>>parent_id>>type>>flag; if(type>=ELM_NUM)return false;
            f>>name_time>>parent_time>>removed_time>>no_publish_time;
            if(!skip_name_data)GetStr(f, name);

            // data
            Delete(data); if(flag&DATA) // if has data
            {
               FlagDisable(flag, DATA);
               ElmData *data=Data(); if(!data)return false;
               if(skip_name_data)f>>data->ver;
               else             if(!data->load(f))return false;
            }
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>id>>parent_id>>type>>flag; if(type>=ELM_NUM)return false;
            f>>name_time>>parent_time>>removed_time; no_publish_time.zero();
            if(!skip_name_data)GetStr(f, name);

            // data
            Delete(data); if(flag&DATA) // if has data
            {
               FlagDisable(flag, DATA);
               ElmData *data=Data(); if(!data)return false;
               if(skip_name_data)f>>data->ver;
               else             if(!data->load(f))return false;
            }
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   void Elm::save(TextNode &node)C
   {
                           node.set(id.asFileName());
                           node.nodes.New().set  ("Type"   , ElmTypeNameNoSpaceDummy.names[type]);
                           node.nodes.New().set  ("Name"   , name);
      if(parent_id.valid())node.nodes.New().setFN("Parent" , parent_id);
      if(removed        ())node.nodes.New().set  ("Removed");
      if(noPublish      ())node.nodes.New().set  ("Publish", publish());

                              node.nodes.New().set("NameTime"   , name_time.text());
                              node.nodes.New().set("ParentTime" , parent_time.text());
      if(   removed_time.is())node.nodes.New().set("RemovedTime", removed_time.text());
      if(no_publish_time.is())node.nodes.New().set("PublishTime", no_publish_time.text());
      // IMPORTING OPENED flags are not saved, because this text format is used for SVN synchronization, and we don't want to send these flags to other computers
      // FINAL_REMOVED FINAL_NO_PUBLISH flags are not saved because they are calculated based on other flags and parents
      if(data)data->save(node.nodes.New().setName("Data").nodes);
   }
   bool Elm::load(C TextNode &node, Str &error) // assumes that 'error' doesn't need to be cleared at start, and 'T.id' was already set
   {
    C TextNode *data_node=null;
      REPA(node.nodes)
      {
       C TextNode &n=node.nodes[i];
         if(n.name=="Type"       ){REP(ELM_NUM)if(n.value==ElmTypeNameNoSpaceDummy.names[i]){type=ELM_TYPE(i); break;}}else
         if(n.name=="Name"       )n.getValue(name     );else
         if(n.name=="Parent"     )n.getValue(parent_id);else
         if(n.name=="Removed"    )removed        (n.asBool1());else
         if(n.name=="Publish"    )publish        (n.asBool1());else
         if(n.name=="NameTime"   )name_time      =n.asText () ;else
         if(n.name=="ParentTime" )parent_time    =n.asText () ;else
         if(n.name=="RemovedTime")removed_time   =n.asText () ;else
         if(n.name=="PublishTime")no_publish_time=n.asText () ;else
         if(n.name=="Data"       )data_node=&n; // remember for later, because to load data, first we must know the type
      }
      if(!type){error=S+"Element \""+node.name+"\" has no type"; return false;}
      if(data_node)if(ElmData *data=Data())data->load(ConstCast(data_node->nodes));/*else return false; we can silently ignore this*/
      return true;
   }
   void Elm::compressNew(File &f)C
   {
      f<<id<<parent_id<<type<<name; //<<create_time;
   }
   void Elm::decompressNew(File &f)
   {
      f>>id>>parent_id>>type>>name; //>>create_time;
      name_time=parent_time=NewElmTime; // new elements always have those times set, so instead of sending we can assume their value here
   }
   void Elm::compressData(File &f)C
   {
      f<<id<<type;
      f.putBool(data!=null); if(data)data->save(f);
   }
   bool Elm::decompressData(File &f)
   {
      f>>id>>type;
      Delete(data); if(f.getBool())
      {
         ElmData *data=Data(); if(!data)return false;
         return   data->load(f);
      }
      return false;
   }
   bool TextureInfo::knownFileSize()C {return file_size>=0;}
   bool TextureInfo::knownSharpness()C {return sharpness< 2;}
ElmObjClass::ElmObjClass() : flag(0) {}

ElmObj::ElmObj() : mesh_id(UIDZero), base_id(UIDZero) {}

ElmMesh::ElmMesh() : obj_id(UIDZero), skel_id(UIDZero), phys_id(UIDZero), body_id(UIDZero), draw_group_id(UIDZero), box(Vec(0), Vec(-1)) {}

ElmMaterial::ElmMaterial() : base_0_tex(UIDZero), base_1_tex(UIDZero), detail_tex(UIDZero), macro_tex(UIDZero), reflection_tex(UIDZero), light_tex(UIDZero), downsize_tex_mobile(0), flag(0) {}

ElmWaterMtrl::ElmWaterMtrl() : base_0_tex(UIDZero), base_1_tex(UIDZero), reflection_tex(UIDZero), flag(0) {}

ElmSkel::ElmSkel() : mesh_id(UIDZero) {}

ElmPhys::ElmPhys() : mesh_id(UIDZero), mtrl_id(UIDZero), density(1), box(Vec(0), Vec(-1)) {}

ElmAnim::ElmAnim() : skel_id(UIDZero), root_move(VecZero), root_rot(VecZero), flag(LOOP) {}

ElmWorld::ElmWorld() : area_size(0), hm_res(0), ctrl_r(0.33f), ctrl_h(2.0f), max_climb(0.7f), max_slope(PI_4), cell_size(1.0f/3), cell_h(0.1f), env_id(UIDZero) {}

ElmEnum::ElmEnum() : type(EditEnums::DEFAULT) {}

ElmImage::ElmImage() : flag(MIP_MAPS), type(COMPRESSED), mode(IMAGE_2D), size(0) {}

ElmImageAtlas::ElmImageAtlas() : mip_maps(true) {}

ElmImageAtlas::Img::Img() : removed(false), id(UIDZero) {}

ElmIconSetts::ElmIconSetts() : type(ElmImage::COMPRESSED) {}

ElmIcon::ElmIcon() : flag(0), icon_settings_id(UIDZero), obj_id(UIDZero), anim_id(UIDZero), variation_id(InheritVariation), anim_pos(0) {}

ElmTextStyle::ElmTextStyle() : font_id(UIDZero) {}

ElmPanelImage::ElmPanelImage() : compressed(false) {}

ElmApp::ElmApp() : build(1), fb_app_id(0), storage(Edit::STORAGE_INTERNAL), supported_orientations(DIRF_X|DIRF_Y), flag(PUBLISH_PROJ_DATA|PUBLISH_PHYSX_DLL|PUBLISH_DATA_AS_PAK), icon(UIDZero), notification_icon(UIDZero), image_portrait(UIDZero), image_landscape(UIDZero), gui_skin(UIDZero) {}

ElmMiniMap::ElmMiniMap() : areas_per_image(4), image_size(256), world_id(UIDZero), env_id(UIDZero) {}

Elm::Elm() : type(ELM_NONE), flag(0), file_size(-1), id(UID().randomizeValid()), parent_id(UIDZero), data(null) {}

TextureInfo::TextureInfo() : file_size(-1), sharpness( 3) {}

/******************************************************************************/
