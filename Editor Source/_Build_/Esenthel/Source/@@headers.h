/******************************************************************************/
#pragma once

#define bool Bool // boolean value (8-bit)

#define char8 Char8 //  8-bit character
#define char  Char  // 16-bit character

#define sbyte  I8  //  8-bit   signed integer
#define  byte  U8  //  8-bit unsigned integer
#define  short I16 // 16-bit   signed integer
#define ushort U16 // 16-bit unsigned integer
#define  int   I32 // 32-bit   signed integer
#define uint   U32 // 32-bit unsigned integer
#define  long  I64 // 64-bit   signed integer
#define ulong  U64 // 64-bit unsigned integer

#define flt Flt // 32-bit floating point
#define dbl Dbl // 64-bit floating point

#define  ptr  Ptr // universal pointer
#define cptr CPtr // universal pointer to const data

#define cchar8 CChar8 // const Char8
#define cchar  CChar  // const Char16

#define  intptr  IntPtr //   signed integer capable of storing full memory address
#define uintptr UIntPtr // unsigned integer capable of storing full memory address

#define class struct // Esenthel Script "class" is a C++ "struct"
/******************************************************************************/
// DEFINES
#define STEAM   0
#define OPEN_VR 0
/******************************************************************************/
/******************************************************************************/
// CLASSES
/******************************************************************************/
class AdjustBoneOrns;
class AnimEditor;
class AppPropsEditor;
class Area;
class AreaBuild;
class AreaSync;
class AreaVer;
class BitRateQuality;
class BrushClass;
class BuilderClass;
class CalculatorClass;
class ChangePassWin;
class Chunk;
class Chunks;
class ClassNameDesc;
class Code;
class CodeView;
class ColorBrushClass;
class CompareProjects;
class ConvertToAtlasClass;
class ConvertToDeAtlasClass;
class CopyElements;
class CreateMaterials;
class Cursor;
class DetectSimilarTextures;
class EE1EditLake;
class EE1EditRiver;
class EE1EditWaypoint;
class EE1ObjGlobal;
class EE1WorldSettings;
class EEItem;
class EditEnum;
class EditEnums;
class EditEnv;
class EditFont;
class EditGuiSkin;
class EditMaterial;
class EditObjTypeClass;
class EditObject;
class EditPanel;
class EditPanelImage;
class EditParam;
class EditParams;
class EditPhysMtrl;
class EditSkeleton;
class EditTextStyle;
class EditWaterMtrl;
class EditWaypoint;
class EditWaypointPoint;
class EditorServer;
class Elm;
class ElmAnim;
class ElmApp;
class ElmCode;
class ElmCodeBase;
class ElmCodeData;
class ElmData;
class ElmEnum;
class ElmEnv;
class ElmFile;
class ElmFont;
class ElmGui;
class ElmGuiSkin;
class ElmIcon;
class ElmIconSetts;
class ElmImage;
class ElmImageAtlas;
class ElmMaterial;
class ElmMesh;
class ElmMiniMap;
class ElmName;
class ElmNode;
class ElmObj;
class ElmObjClass;
class ElmPanel;
class ElmPanelImage;
class ElmPhys;
class ElmPhysMtrl;
class ElmProperties;
class ElmSkel;
class ElmSound;
class ElmTextStyle;
class ElmTypeNameNoSpace;
class ElmTypeVer;
class ElmVideo;
class ElmWaterMtrl;
class ElmWorld;
class EmbedObj;
class EmbedObjBase;
class EnumEditor;
class EnvEditor;
class EraseRemovedElms;
class ExportWindow;
class FileSizeGetter;
class FontEditor;
class GoToArea;
class GridPlaneLevel;
class GroupRegion;
class GuiEditParam;
class GuiSkinEditor;
class GuiView;
class HeightBrushClass;
class Heightmap2;
class IDReplace;
class IconEditor;
class IconSettings;
class IconSettsEditor;
class ImageAtlasEditor;
class ImageConvert;
class ImageEditor;
class ImageGenerate;
class ImageSkin;
class ImportCodeClass;
class ImportTerrainClass;
class ImportTerrainTask;
class ImporterClass;
class Lake;
class LakeBase;
class LeafRegion;
class ListElm;
class LodRegion;
class LodView;
class MaterialRegion;
class MaterialTech;
class MergeSimilarMaterials;
class MeshAOClass;
class MeshParts;
class MeshVariations;
class MiniMapEditor;
class MiniMapVer;
class MiscRegion;
class ModeTabs;
class MtrlBrushClass;
class MtrlImages;
class MulSoundVolumeClass;
class NameDesc;
class NewLodClass;
class NewProjWin;
class NewWorldClass;
class Obj;
class ObjClassEditor;
class ObjData;
class ObjGrid;
class ObjListClass;
class ObjPaintClass;
class ObjPos;
template<typename TYPE> class ObjPtrs;
class ObjRot;
class ObjScale;
class ObjVer;
class ObjView;
class PanelEditor;
class PanelImageEditor;
class ParamEditor;
class PathProps;
class PhysMtrlEditor;
class PhysPath;
class Player;
class Pose;
class PreviewClass;
class Project;
class ProjectCipher;
class ProjectEx;
class ProjectHierarchy;
class ProjectLocked;
class ProjectSettings;
class ProjectUpdate;
class Projects;
class PropEx;
class PropWin;
class PublishClass;
class PublishPVRTC;
class PublishResult;
class RayTest;
class RegisterWin;
class ReloadElmClass;
class RemoveProjWin;
class Rename;
class RenameBoneClass;
class RenameElmClass;
class RenameEventClass;
class RenameProjWin;
class RenameSlotClass;
class ReplaceNameClass;
class River;
class RiverBase;
class Rotator;
class SelectionClass;
class ServerClass;
class SetMtrlColorClass;
class SizeStatistics;
class SmallMatrix;
class SmallMatrix3;
class SoundEditor;
class SplitAnimation;
class StoreClass;
class StoreFiles;
class SynchronizerClass;
class TerrainObj;
class TerrainObj2;
class TexInfoGetter;
class TextBlack;
class TextNoTest;
class TextStyleEditor;
class TextWhite;
class Texture;
class TextureDownsize;
class TextureInfo;
class TheaterClass;
class TimeStamp;
class TransformRegion;
class UpdateWindowClass;
class UpdaterClass;
class Version;
class VideoEditor;
class VideoOptions;
class Viewport4Region;
class ViewportSkin;
class WaterMtrlRegion;
class WaterVer;
class WaypointListClass;
class WaypointPos;
class WorldBrushClass;
class WorldChange;
class WorldData;
class WorldUndo;
class WorldVer;
class WorldView;
/******************************************************************************/
// CACHES
/******************************************************************************/
extern Cache<EditObject> EditObjects;
/******************************************************************************/
// ENUMS
/******************************************************************************/
enum AREA_LOAD_FLAG
{
   AREA_LOAD_HEIGHTMAP     =1<<0,
   AREA_LOAD_HM_MESH       =1<<1,
   AREA_LOAD_HM_PHYS       =1<<2,
   AREA_LOAD_LOCAL_OBJ_PHYS=1<<3, // if load physical bodies of area objects (that are not embedded)
   AREA_LOAD_LOCAL_WATER   =1<<4, // if load water meshes of an area
   AREA_MAKE_PATH_SRC      =1<<5, // if create 'path_src' member which is used for PathMesh generation in later step (don't put this into 'build' because it should not be stored in save, it's only a temporary helper value)
};
/******************************************************************************/
enum AREA_REBUILD_FLAG // !! these enums are saved !!
{
   AREA_REBUILD_HM_MESH         =1<<0,
   AREA_REBUILD_HM_MESH_SIMPLIFY=1<<1,
   AREA_REBUILD_HM_PHYS         =1<<2,
   AREA_REBUILD_EMBED_OBJ       =1<<3,
   AREA_REBUILD_GAME_AREA_OBJS  =1<<4, // if list of game area objects needs to be rebuilt
   AREA_REBUILD_WATER           =1<<5,
   AREA_REBUILD_PATH            =1<<6,
   AREA_REBUILD_HM              =1<<7, // heightmap lookup images (height, mtrl_map)
// newer flags require changing from byte to U16 (watch out for save/load)
};
/******************************************************************************/
enum AREA_SYNC_FLAG // these enums are not saved
{
   AREA_SYNC_HEIGHT =1<<0,
   AREA_SYNC_MTRL   =1<<1,
   AREA_SYNC_COLOR  =1<<2,
   AREA_SYNC_REMOVED=1<<3,
   AREA_SYNC_OBJ    =1<<4,
   AREA_SYNC_HM     =(AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR|AREA_SYNC_REMOVED),
};
/******************************************************************************/
enum BRUSH_SHAPE
{
   BS_CIRCLE,
   BS_SQUARE,
};
/******************************************************************************/
enum CHANGE_PASS_RESULT
{
   CHANGE_PASS_EMAIL_NOT_FOUND,
   CHANGE_PASS_INVALID_KEY,
   CHANGE_PASS_INVALID_PASS,
   CHANGE_PASS_SUCCESS,
};
/******************************************************************************/
enum CIPHER_TYPE : byte
{
   CIPHER_NONE,
   CIPHER_1,
   CIPHER_2,
   CIPHER_3,
   CIPHER_NUM,
};
/******************************************************************************/
enum CLIENT_SERVER_COMMANDS
{
   CS_VERSION_CHECK,

   CS_LOGIN      ,
   CS_REGISTER   ,
   CS_FORGOT_PASS,
   CS_CHANGE_PASS,
   CS_LICENSE_KEY,

   CS_PROJECTS_LIST   ,
   CS_PROJECT_OPEN    ,
   CS_PROJECT_DATA    ,
   CS_PROJECT_SETTINGS,

   CS_NEW_ELM,
   CS_RENAME_ELM,
   CS_SET_ELM_PARENT,
   CS_REMOVE_ELMS,
   CS_NO_PUBLISH_ELMS,
   CS_GET_ELM_NAMES,

   CS_GET_TEXTURES,
   CS_SET_TEXTURE,

   CS_GET_ELM_SHORT, //       ElmData + optional(ElmFile) if ElmFileInShort
   CS_SET_ELM_SHORT, //       ElmData + optional(ElmFile) if ElmFileInShort
   CS_GET_ELM_LONG , //       ElmData +          ElmFile
   CS_SET_ELM_LONG , //       ElmData +          ElmFile
   CS_SET_ELM_FULL , // Elm + ElmData +          ElmFile

   CS_GET_WORLD_VER,
   CS_GET_WORLD_AREAS,
   CS_SET_WORLD_AREA,
   CS_SET_WORLD_OBJS,
   CS_GET_WORLD_WAYPOINTS,
   CS_SET_WORLD_WAYPOINT,
   CS_GET_WORLD_LAKES,
   CS_SET_WORLD_LAKE,
   CS_GET_WORLD_RIVERS,
   CS_SET_WORLD_RIVER,

   CS_GET_MINI_MAP_VER,
   CS_GET_MINI_MAP_IMAGES,
   CS_SET_MINI_MAP_SETTINGS,
   CS_SET_MINI_MAP_IMAGE,

   CS_GET_CODE_VER ,
   CS_SET_CODE_DATA,
   CS_CODE_SYNC_STATUS,

   CS_NUM,
};
/******************************************************************************/
enum EDIT_OBJ_TYPE : byte
{
   EDIT_OBJ_MESH       ,
   EDIT_OBJ_LIGHT_POINT,
   EDIT_OBJ_LIGHT_CONE ,
   EDIT_OBJ_PARTICLES  ,
   EDIT_OBJ_NUM        ,
};
/******************************************************************************/
enum ELM_FLAG
{
   ELM_CONTAINS_NAME      =1<<0,
   ELM_CONTAINS_NAME_CHILD=1<<1,
   ELM_EDITED             =1<<2,
   ELM_EDITED_CHILD       =1<<3,
};
/******************************************************************************/
enum ELM_OFFSET // offset applied for element ID when creating sub-elements for ELM_OBJ (so that when creating new children on different computers, their ID will be the same, obj_elm.id+ELM_OFFSET_*)
{
   ELM_OFFSET_MESH=1, // mesh first so following can be children of mesh
   ELM_OFFSET_SKEL  ,
   ELM_OFFSET_PHYS  ,
   ELM_OFFSET_NUM   ,
};
/******************************************************************************/
enum ELM_TYPE : byte // !! these enums are saved, also this must be in sync with the Edit.ELM_TYPE !!
{
   ELM_NONE       ,
   ELM_FOLDER     ,
   ELM_ENUM       ,
   ELM_IMAGE      ,
   ELM_IMAGE_ATLAS,
   ELM_FONT       ,
   ELM_TEXT_STYLE ,
   ELM_PANEL      ,
   ELM_GUI        ,
   ELM_SHADER     ,
   ELM_MTRL       ,
   ELM_WATER_MTRL ,
   ELM_PHYS_MTRL  ,
   ELM_OBJ_CLASS  , // this is 'Object' (     params) with 'Object.type' set to elm.id and 'Object.access' set to OBJ_ACCESS_CUSTOM
   ELM_OBJ        , // this is 'Object' (main+params)
   ELM_MESH       ,
   ELM_SKEL       ,
   ELM_PHYS       ,
   ELM_ANIM       ,
   ELM_PANEL_IMAGE,
   ELM_ICON       , // this is 2D image created from 3D object
   ELM_ENV        ,
   ELM_WORLD      ,
   ELM_SOUND      ,
   ELM_VIDEO      ,
   ELM_FILE       ,
   ELM_CODE       , // Code
   ELM_LIB        , // Code Library
   ELM_APP        , // Code Application
   ELM_ICON_SETTS , // Icon Settings
   ELM_MINI_MAP   , // World Mini Map
   ELM_GUI_SKIN   ,
   ELM_NUM        , // number of element types
   ELM_ANY        =ELM_NUM,
   ELM_NUM_ANY    , // number of element types (including ELM_ANY)
};
/******************************************************************************/
enum FORGOT_PASS_RESULT
{
   FORGOT_PASS_EMAIL_NOT_FOUND,
   FORGOT_PASS_KEY_SENT,
   FORGOT_PASS_NO_SEND_MAIL,
};
/******************************************************************************/
enum HB_MODE // Height Brush Mode
{
#if MOBILE
   HB_ADD    ,
   HB_SUB    ,
#else
   HB_ADD_SUB,
#endif
   HB_LEVEL  , 
   HB_FLAT   ,
   HB_AVG    ,
   HB_SOFT   ,
   HB_NOISE  ,
   HB_NUM    ,
};
/******************************************************************************/
enum IMPORT_PHASE
{
   IMPORT_GET_FILES,
   IMPORT_IMPORT   ,
   IMPORT_IMPORT1  ,
   IMPORT_IMPORT2  ,
   IMPORT_IMPORT3  ,
   IMPORT_FINISHED ,
};
/******************************************************************************/
enum LOAD_RESULT
{
   LOAD_OK    , // success
   LOAD_EMPTY , // ok    , but project is empty
   LOAD_NEWER , // failed, project requires newer engine version
   LOAD_LOCKED, // failed, project is currently opened in another instance (or that instance closed without unlocking)
   LOAD_ERROR , // failed, error occured while reading the file
};
/******************************************************************************/
enum LOGIN_RESULT
{
   LOGIN_EMAIL_NOT_FOUND_REGISTRATION_UNAVAILABLE,
   LOGIN_EMAIL_NOT_FOUND_REGISTRATION_AVAILABLE,
   LOGIN_EMAIL_FOUND_INVALID_PASS,
   LOGIN_LICENSE_USED,
   LOGIN_DEMO_USER_NOT_ALLOWED,
   LOGIN_SUCCESS,
};
/******************************************************************************/
enum MATERIAL_SIMPLIFY : byte // !! this enum is saved !!
{
   MS_NEVER , // don't simplify materials
   MS_MOBILE, // simplify only for mobile platforms
   MS_ALWAYS, // always simplify
   MS_NUM   ,
};
/******************************************************************************/
enum MODE
{
   MODE_OBJ     ,
   MODE_ANIM    ,
   MODE_WORLD   ,
   MODE_GUI_SKIN,
   MODE_GUI     ,
   MODE_CODE    ,
   MODE_TEX_DOWN,
   MODE_NUM     ,
};
/******************************************************************************/
enum
{
   FORCE_HQ    =1<<0, // high quality
   IGNORE_ALPHA=1<<1,
};
/******************************************************************************/
enum
{
   CHANGE_NORMAL     =1<<0, // set this first so that it will be equivalent to 'bool', because below some codes use "uint changed|=Sync(..)"
   CHANGE_AFFECT_FILE=1<<1, // this change affects the file
};
/******************************************************************************/
enum OBJ_TYPE // Object Class Types
{
};
/******************************************************************************/
enum OP_HM_MODE
{
   OP_HM_NONE,
   OP_HM_ADD_REM,
   OP_HM_HEIGHT,
   OP_HM_COLOR,
   OP_HM_MTRL,
   OP_HM_MTRL_SCALE,
   OP_HM_MTRL_HOLE,
   OP_HM_MTRL_MAX_1,
   OP_HM_MTRL_MAX_2,
};
/******************************************************************************/
enum OP_OBJ_MODE
{
   OP_OBJ_NONE     ,
   OP_OBJ_SET_MTRL ,
   OP_OBJ_SET_GROUP,
};
/******************************************************************************/
enum PUBLISH_STAGE
{
   PUBLISH_MTRL_SIMPLIFY,
   PUBLISH_TEX_OPTIMIZE ,
   PUBLISH_PUBLISH      ,
};
/******************************************************************************/
enum REGISTER_RESULT
{
   REGISTER_REGISTRATION_UNAVAILABLE,
   REGISTER_INVALID_EMAIL,
   REGISTER_EMAIL_USED,
   REGISTER_SUCCESS,
};
/******************************************************************************/
enum SAVE_MODE
{
   SAVE_DEFAULT, // default save
   SAVE_AUTO   , //    auto-save
};
/******************************************************************************/
enum UNIT_TYPE
{
   UNIT_DEFAULT,
   UNIT_PIXEL  ,
   UNIT_PERCENT, // 1/100
   UNIT_PERMIL , // 1/1000
};
/******************************************************************************/
enum USER_ACCESS : byte
{
   UA_NO_ACCESS,
   UA_READ_ONLY,
   UA_ARTIST   ,
   UA_NORMAL   ,
   UA_ADMIN    ,
   UA_NUM      ,
};
/******************************************************************************/
enum VIEW_MODE
{
   VIEW_FPP,
   VIEW_TPP,
   VIEW_ISO,
   VIEW_NUM,
};
/******************************************************************************/
// CONSTANTS
/******************************************************************************/
const int  EE_APP_BUILD        =59;
const uint EditMeshFlagAnd=~(VTX_DUP|EDGE_ALL|FACE_NRM|ADJ_ALL|VTX_TAN_BIN), // TanBin are not needed in Edit because they're always re-created if needed
           GameMeshFlagAnd=~(VTX_DUP|EDGE_ALL|FACE_NRM|ADJ_ALL);
const int           ForceInstaller=-2, // -2=disable and don't update, -1=disable, 0=auto, 1=enable (this is used only in Debug)
                    HeightBrushNoiseRes=256,
                    MtrlBrushSlots=14,
                    MaxDemoProjElms=64,
                    MaxVisibleRadius=(X64 ? 20 : 15),
                    MaxMaterialDownsize=3, // 0=full, 1=half, 2=quarter
                    ClientSendBufSize=3*1024*1024, // 3 MB
                    ServerSendBufSize=2*1024*1024, // 2 MB
                    InvalidCoordinate=SIGN_BIT,
                    RebuildBlockSize=6,
                    LinearAnimFpsLimit=12, // min number of FPS for an animation to be set as linear
                    EsenthelStoreMaxFileNum   =16, // max num of files for item
                    EsenthelStoreMaxFileSize  =255*1024*1024, // 255 MB max file size
                    EsenthelStoreFileChunked  =100*1024*1024, // 100 MB
                    EsenthelStoreFileChunkSize= 24*1024*1024, //  24 MB
                    EsenthelStoreMaxIconSize  =128*1024, // 128 KB
                    EsenthelStoreMaxImageSize =256*1024, // 256 KB
                    EsenthelStoreEngineLicense[]={1, 4, 5}, // Binary Monthly, Binary Yearly, Source Yearly
                    DefaultVorbisBitRate= 96*1000,
                    DefaultOpusBitRate  = 96*1000,
                    DefaultHQBitRate    =128*1000,
                    MeshSplitMaxSteps=3,    // max steps of splitting a mesh
                    MeshSplitMinVtxs=12000, // min number of vertexes in a mesh to split it
                    MeshSplitMinSize=4;
const uint          MeshJoinAllTestVtxFlag=VTX_HLP|VTX_SIZE;
const uint          NewElmTime=1;
const uint ProjectVersion     =64, // !! increase this by one if any of engine/editor asset formats have changed !!
           ClientServerVersion=59;
const int           ServerNetworkCompressionLevel=9           , ClientNetworkCompressionLevel=5            , EsenthelProjectCompressionLevel=5;
/******************************************************************************/
// TYPEDEFS
/******************************************************************************/
typedef CacheElmPtr<EditObject, EditObjects> EditObjectPtr;
ASSERT(MTECH_DEFAULT==0 && MTECH_ALPHA_TEST==1 && MTECH_FUR==2 && MTECH_GRASS==3 && MTECH_LEAF==4 && MTECH_BLEND==5 && MTECH_BLEND_LIGHT==6 && MTECH_BLEND_LIGHT_GRASS==7 && MTECH_BLEND_LIGHT_LEAF==8 && MTECH_TEST_BLEND_LIGHT==9 && MTECH_TEST_BLEND_LIGHT_GRASS==10 && MTECH_TEST_BLEND_LIGHT_LEAF==11);
ASSERT(CIPHER_NONE==0 && CIPHER_1==1 && CIPHER_2==2 && CIPHER_3==3 && CIPHER_NUM==4);
ASSERT(OBJ_ACCESS_CUSTOM==0 && OBJ_ACCESS_TERRAIN==1 && OBJ_ACCESS_GRASS==2 && OBJ_ACCESS_OVERLAY==3);
/******************************************************************************/
/******************************************************************************/
// CLASSES
/******************************************************************************/
#include "@WorldVer.h"
#include "@WorldUndo.h"
#include "@WorldData.h"
#include "@WaypointListClass.h"
#include "@Viewport4Region.h"
#include "@Version.h"
#include "@UpdateWindowClass.h"
#include "@UpdaterClass.h"
#include "@TimeStamp.h"
#include "@TextureInfo.h"
#include "@Texture.h"
#include "@TextNoTest.h"
#include "@TexInfoGetter.h"
#include "@TerrainObj2.h"
#include "@TerrainObj.h"
#include "@SynchronizerClass.h"
#include "@StoreFiles.h"
#include "@SplitAnimation.h"
#include "@SmallMatrix3.h"
#include "@SmallMatrix.h"
#include "@ServerClass.h"
#include "@SelectionClass.h"
#include "@Rotator.h"
#include "@RiverBase.h"
#include "@River.h"
#include "@ReplaceNameClass.h"
#include "@RenameSlotClass.h"
#include "@RenameEventClass.h"
#include "@RenameElmClass.h"
#include "@RenameBoneClass.h"
#include "@Rename.h"
#include "@ReloadElmClass.h"
#include "@RegisterWin.h"
#include "@RayTest.h"
#include "@PublishResult.h"
#include "@PublishPVRTC.h"
#include "@ProjectUpdate.h"
#include "@ProjectLocked.h"
#include "@ProjectCipher.h"
#include "@PreviewClass.h"
#include "@Pose.h"
#include "@Player.h"
#include "@PhysPath.h"
#include "@ObjVer.h"
#include "@ObjPtrs.h"
#include "@ObjGrid.h"
#include "@NewProjWin.h"
#include "@NameDesc.h"
#include "@MtrlImages.h"
#include "@MtrlBrushClass.h"
#include "@ModeTabs.h"
#include "@MiniMapVer.h"
#include "@MeshVariations.h"
#include "@MeshParts.h"
#include "@MaterialTech.h"
#include "@LodView.h"
#include "@ListElm.h"
#include "@LakeBase.h"
#include "@Lake.h"
#include "@ImportTerrainTask.h"
#include "@ImportCodeClass.h"
#include "@ImageSkin.h"
#include "@ImageGenerate.h"
#include "@ImageConvert.h"
#include "@IDReplace.h"
#include "@Heightmap2.h"
#include "@HeightBrushClass.h"
#include "@GoToArea.h"
#include "@FileSizeGetter.h"
#include "@ExportWindow.h"
#include "@EraseRemovedElms.h"
#include "@EmbedObjBase.h"
#include "@EmbedObj.h"
#include "@ElmTypeVer.h"
#include "@ElmTypeNameNoSpace.h"
#include "@ElmNode.h"
#include "@ElmName.h"
#include "@ElmData.h"
#include "@ElmCodeBase.h"
#include "@ElmCode.h"
#include "@ElmApp.h"
#include "@ElmAnim.h"
#include "@Elm.h"
#include "@EEItem.h"
#include "@EE1WorldSettings.h"
#include "@EE1ObjGlobal.h"
#include "@EE1EditWaypoint.h"
#include "@EE1EditRiver.h"
#include "@EE1EditLake.h"
#include "@EditWaypointPoint.h"
#include "@EditWaypoint.h"
#include "@EditTextStyle.h"
#include "@EditSkeleton.h"
#include "@EditPhysMtrl.h"
#include "@EditParams.h"
#include "@EditParam.h"
#include "@EditPanelImage.h"
#include "@EditPanel.h"
#include "@EditorServer.h"
#include "@EditObjTypeClass.h"
#include "@EditObject.h"
#include "@EditMaterial.h"
#include "@EditGuiSkin.h"
#include "@EditFont.h"
#include "@EditEnv.h"
#include "@EditEnums.h"
#include "@EditEnum.h"
#include "@Cursor.h"
#include "@ColorBrushClass.h"
#include "@CodeView.h"
#include "@Code.h"
#include "@ClassNameDesc.h"
#include "@Chunks.h"
#include "@Chunk.h"
#include "@ChangePassWin.h"
#include "@CalculatorClass.h"
#include "@BuilderClass.h"
#include "@BitRateQuality.h"
#include "@AreaVer.h"
#include "@AreaSync.h"
#include "@AreaBuild.h"
#include "@GuiEditParam.h"
#include "@Project.h"
#include "@TextWhite.h"
#include "@ViewportSkin.h"
#include "@ElmWorld.h"
#include "@ElmWaterMtrl.h"
#include "@ElmVideo.h"
#include "@PathProps.h"
#include "@ElmTextStyle.h"
#include "@ElmSound.h"
#include "@ElmSkel.h"
#include "@ElmPhysMtrl.h"
#include "@ElmPhys.h"
#include "@ElmPanelImage.h"
#include "@ElmPanel.h"
#include "@ElmObjClass.h"
#include "@ElmObj.h"
#include "@ElmMiniMap.h"
#include "@ElmMesh.h"
#include "@ElmMaterial.h"
#include "@ElmImageAtlas.h"
#include "@ElmImage.h"
#include "@ElmIconSetts.h"
#include "@ElmIcon.h"
#include "@ElmGuiSkin.h"
#include "@ElmGui.h"
#include "@ElmFont.h"
#include "@ElmFile.h"
#include "@ElmEnv.h"
#include "@ElmEnum.h"
#include "@ElmCodeData.h"
#include "@WorldChange.h"
#include "@TextBlack.h"
#include "@ObjData.h"
#include "@Obj.h"
#include "@NewWorldClass.h"
#include "@EditWaterMtrl.h"
#include "@NewLodClass.h"
#include "@WaypointPos.h"
#include "@MiscRegion.h"
#include "@StoreClass.h"
#include "@MeshAOClass.h"
#include "@TransformRegion.h"
#include "@MaterialRegion.h"
#include "@LodRegion.h"
#include "@WaterVer.h"
#include "@LeafRegion.h"
#include "@PublishClass.h"
#include "@PropWin.h"
#include "@PropEx.h"
#include "@ImportTerrainClass.h"
#include "@ImporterClass.h"
#include "@TheaterClass.h"
#include "@ProjectSettings.h"
#include "@BrushClass.h"
#include "@Projects.h"
#include "@ImageEditor.h"
#include "@SizeStatistics.h"
#include "@ImageAtlasEditor.h"
#include "@Area.h"
#include "@AppPropsEditor.h"
#include "@AnimEditor.h"
#include "@AdjustBoneOrns.h"
#include "@PanelImageEditor.h"
#include "@RenameProjWin.h"
#include "@PanelEditor.h"
#include "@EnumEditor.h"
#include "@VideoOptions.h"
#include "@MulSoundVolumeClass.h"
#include "@VideoEditor.h"
#include "@PhysMtrlEditor.h"
#include "@RemoveProjWin.h"
#include "@MiniMapEditor.h"
#include "@SetMtrlColorClass.h"
#include "@MergeSimilarMaterials.h"
#include "@GuiView.h"
#include "@GuiSkinEditor.h"
#include "@SoundEditor.h"
#include "@DetectSimilarTextures.h"
#include "@ParamEditor.h"
#include "@CreateMaterials.h"
#include "@ConvertToDeAtlasClass.h"
#include "@ConvertToAtlasClass.h"
#include "@CompareProjects.h"
#include "@ProjectHierarchy.h"
#include "@GroupRegion.h"
#include "@GridPlaneLevel.h"
#include "@ElmProperties.h"
#include "@ObjView.h"
#include "@FontEditor.h"
#include "@TextStyleEditor.h"
#include "@ObjScale.h"
#include "@ObjRot.h"
#include "@IconSettings.h"
#include "@ObjPos.h"
#include "@ObjPaintClass.h"
#include "@ObjListClass.h"
#include "@WaterMtrlRegion.h"
#include "@TextureDownsize.h"
#include "@ObjClassEditor.h"
#include "@EnvEditor.h"
#include "@ProjectEx.h"
#include "@CopyElements.h"
#include "@IconEditor.h"
#include "@IconSettsEditor.h"
#include "@WorldView.h"
#include "@WorldBrushClass.h"
/******************************************************************************/
// CPP
/******************************************************************************/
#include "Auto.h"
#include "Editor Update.h"
#include "Editors/Mini Map.h"
#include "Editors/Object/Mesh.h"
#include "Editors/World/Area@0.h"
#include "Editors/World/Builder.h"
#include "Editors/World/Cursor.h"
#include "Editors/World/Import Terrain.h"
#include "Editors/World/Object@0.h"
#include "Editors/World/Selection.h"
#include "Editors/World/Water@0.h"
#include "Editors/World/Waypoint@0.h"
#include "Elm@0.h"
#include "Game/Controls.h"
#include "Game/Game.h"
#include "Gui/Copy Elements.h"
#include "Gui/Create Materials.h"
#include "Gui/Gui.h"
#include "Gui/Import 1.h"
#include "Gui/Merge Similar Materials.h"
#include "Gui/Project Update@0.h"
#include "Main.h"
#include "Preview.h"
#include "Project@0.h"
#include "Project List.h"
#include "Publish.h"
#include "Settings.h"
#include "Shared/Area.h"
#include "Shared/Commands.h"
#include "Shared/Constants.h"
#include "Shared/Elements/Object.h"
#include "Shared/Elm.h"
#include "Shared/Functions.h"
#include "Shared/Project.h"
#include "Store/License Test.h"
/******************************************************************************/
// INLINE, TEMPLATES
/******************************************************************************/
#include "Editors/Environment@0.inline.h"
#include "Editors/Gui@1.inline.h"
#include "Editors/Panel Image@0.inline.h"
#include "Editors/Param.inline.h"
#include "Game/Game.inline.h"
#include "Gui/Gui.inline.h"
#include "Project@0.inline.h"
#include "Shared/Functions.inline.h"
/******************************************************************************/
