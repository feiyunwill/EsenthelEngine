/******************************************************************************/
// MATH
/******************************************************************************/
struct Half;
struct VecH2;
struct Vec2;
struct VecD2;
struct VecI2;
struct VecB2;
struct VecSB2;
struct VecUS2;
struct VecH;
struct Vec;
struct VecD;
struct VecI;
struct VecB;
struct VecSB;
struct VecUS;
struct VecH4;
struct Vec4;
struct VecD4;
struct VecI4;
struct VecB4;
struct VecSB4;

struct Plane;
struct PlaneM;
struct PlaneD;

struct Edge2;
struct EdgeD2;
struct Edge;
struct EdgeD;

struct Tri2;
struct TriD2;
struct Tri;
struct TriD;

struct Quad2;
struct QuadD2;
struct Quad;
struct QuadD;

struct Rect;
struct RectD;
struct RectI;
struct Box;
struct BoxD;
struct BoxI;
struct OBox;
struct Extent;
struct Circle;
struct Ball;
struct BallM;
struct BallD;
struct Capsule;
struct Tube;
struct Torus;
struct Cone;
struct Pyramid;
struct Shape;

struct Matrix3;
struct MatrixD3;
struct Matrix;
struct MatrixM;
struct MatrixD;
struct Matrix4;
struct GpuMatrix;
struct RevMatrix3;
struct RevMatrix;

struct Orient;
struct OrientD;
struct OrientP;
struct OrientM;

struct Quaternion;

struct FrustumClass;

struct Randomizer;
extern Randomizer Random;

struct InterpolatorTemp;
/******************************************************************************/
// ANIMATION
/******************************************************************************/
struct SkeletonBone;
struct SkeletonSlot;
struct Skeleton;
struct AnimatedSkeletonBone;
struct AnimatedSkeleton;
struct AnimationKeys;
struct AnimationBone;
struct AnimationEvent;
struct Animation;
struct SkelAnim;
struct Motion;
/******************************************************************************/
// GRAPHICS
/******************************************************************************/
struct Color;
struct Color2;
struct Image;
struct ImageHeader;
struct Video;
struct Font;
struct Display;
struct MainShaderClass;
struct RendererClass;
struct RenderTargets;
struct VtxFormatGL;
struct VtxFormat;
struct VtxBuf;
struct VtxIndBuf;
struct VtxFull;
struct Light;
struct RippleFx;
struct TextStyleParams;
struct TextStyle;
struct TextInput;
struct ShaderParam;
struct ShaderParamChange;
struct ShaderBase;
struct Shader;
struct ShaderFile;
struct FRST;
struct BLST;
/******************************************************************************/
// GUI
/******************************************************************************/
struct  GUI;
struct  GuiObj;
struct  GuiObjs;
struct  GuiObjChildren;
struct  Button;
struct  CheckBox;
struct  ColorPicker;
struct  ComboBox;
struct  Dialog;
struct  Desktop;
struct  GuiCustom;
struct  TextCodeData;
struct  Text;
struct  Viewport;
struct  GuiImage;
struct _List;
struct  ListColumn;
struct  MenuElm;
struct  Menu;
struct  MenuBar;
struct  Progress;
struct  Region;
struct  SlideBar;
struct  Slider;
struct  Tab;
struct  Tabs;
struct  TextBox;
struct  TextLine;
struct  Window;
struct  WindowIO;
/******************************************************************************/
// MESH
/******************************************************************************/
struct Blocks;
struct BlocksMap;
struct MeshBase;
struct MeshBaseIndex;
struct MeshRender;
struct MeshPart;
struct MeshLod;
struct Mesh;
struct MeshGroup;
/******************************************************************************/
// INPUT
/******************************************************************************/
struct KbSc;
struct Keyboard;
struct Mouse;
struct Touch;
struct VirtualRealityApi;
/******************************************************************************/
// MISC
/******************************************************************************/
struct Str8;
struct Str;
struct BStr;
struct File;
struct PakFile;
struct Pak;
struct PakSet;
struct PakProgress;
struct PakNode;
struct PakFileData;
struct TextNode;
struct TextData;
struct XmlNode;
struct XmlData;
struct TextEdit;
struct CalcValue;
struct DateTime;
struct Cipher;
struct PathWorld;
struct UID;
struct _Memc;
struct _Memb;
struct _Memx;
struct  MemlNode;
T1(TYPE) struct Mems;
T1(TYPE) struct FixedMems;
T1(TYPE) struct Memc;
T1(TYPE) struct Memb;
T1(TYPE) struct Memx;
T1(TYPE) struct Meml;
template<typename TYPE, Int size=64*1024>   struct Memt;
template<typename TYPE, Int size=64*1024>   struct MemPtr;
struct _Grid;
struct _Map;
struct _Cache;
T1(TYPE) struct Cache;
template<typename TYPE, Cache<TYPE> &CACHE>   struct CacheElmPtr;
struct Object;
struct DataCallback;
struct Notification;
/******************************************************************************/
// SOUND
/******************************************************************************/
struct _Sound;
struct _SoundRecord;
struct SoundStream;
struct SoundDataCallback;
/******************************************************************************/
// PHYSICS
/******************************************************************************/
struct PhysHitBasic;
struct PhysHit;
struct PhysCutsCallback;
struct PhysHitCallback;
struct Joint;
struct Actor;
struct ActorInfo;
struct PhysMtrl;
struct Ragdoll;
struct Controller;
struct Grab;
struct PhysPart;
struct PhysBody;
struct PhysGroup;
struct PhysicsClass;
struct RigidBody;
/******************************************************************************/
// NET
/******************************************************************************/
struct Socket;
/******************************************************************************/
// GAME
/******************************************************************************/
namespace Game
{
   struct Obj;
   struct Item;
   struct Chr;
   struct Area;
   struct WorldSettings;
   struct WorldManager;
   T1(TYPE) struct ObjMap;
}
/******************************************************************************/
// EDIT
/******************************************************************************/
namespace Edit
{
   struct Symbol;
   struct Line;
   struct Source;
   struct Token;
   struct Macro;
   struct Expr;
   struct Command;
   struct Message;
   struct Compiler;
   struct CompilerContext;
}
/******************************************************************************/
// NET
/******************************************************************************/
namespace Net
{
   struct Obj;
   struct Area;
   struct World;
}
/******************************************************************************/
