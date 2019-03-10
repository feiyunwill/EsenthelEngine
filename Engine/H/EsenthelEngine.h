/******************************************************************************/
#pragma once // include the header only once
/******************************************************************************/
#define ENGINE_BUILD 59 // this is an integer value specifying the build iteration of Esenthel Engine, this gets incremented with each new release
/******************************************************************************

   Following defines are available, they are always set to 0 (disabled) or 1 (enabled)
      WINDOWS         - Windows                       platform
      WINDOWS_DESKTOP - Windows Desktop               platform
      WINDOWS_PHONE   - Windows Phone                 platform
      MAC             - Mac OS X                      platform
      IOS             - iOS                           platform
      LINUX           - Linux                         platform
      ANDROID         - Android                       platform
      WEB             - Web                           platform
      DESKTOP         - Windows Desktop, Mac, Linux   platform
      MOBILE          - Windows Phone  , iOS, Android platform
      APPLE           - Mac, iOS                      platform
      X64             - 64-bit                        platform
      DEBUG           - Debug mode

   Sample Usage:

      #if WINDOWS
         This line will compile only on Windows platform!
      #endif

/******************************************************************************/
// DETECT PLATFORM
/******************************************************************************/
// 32/64-bit
#if defined _WIN64 || defined __LP64__
   #define X64 1 // 64-bit
#else
   #define X64 0 // 32-bit
#endif

// ARM
#if (defined _M_ARM || defined __arm__) || (defined _M_ARM64 || defined __aarch64__)
   #define ARM 1
#else
   #define ARM 0
#endif

// Windows
#ifdef _WIN32
   #ifdef WINAPI_FAMILY
      #include <winapifamily.h>
      #define WINDOWS_OLD (WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP)
   #else
      #define WINDOWS_OLD 1
   #endif
   #define WINDOWS_NEW (!WINDOWS_OLD)
   #define PLATFORM(windows, unix) windows
#else
   #define WINDOWS_OLD 0
   #define WINDOWS_NEW 0
   #define PLATFORM(windows, unix) unix
#endif

// Apple - Mac or iOS
#ifdef __APPLE__
   #include <TargetConditionals.h>
   #if TARGET_OS_IPHONE // iOS
      #define MAC 0
      #define IOS 1
   #else // Mac
      #define MAC 1
      #define IOS 0
   #endif
   #if TARGET_IPHONE_SIMULATOR // iOS Simulator
      #define IOS_SIMULATOR 1
   #endif
#else
   #define MAC 0
   #define IOS 0
#endif

// Android
#ifdef ANDROID
   #undef  ANDROID
   #define ANDROID 1
#else
   #define ANDROID 0
#endif

// Linux
#if defined __linux__ && !ANDROID // Android also has '__linux__' defined
   #define LINUX 1
#else
   #define LINUX 0
#endif

// Web
#ifdef EMSCRIPTEN
   #define WEB 1
#else
   #define WEB 0
#endif

// Other
#define WINDOWS         (WINDOWS_OLD || WINDOWS_NEW)
#define WINDOWS_PHONE   (WINDOWS     &&  ARM       ) // can't use WINAPI_FAMILY_PHONE_APP because that's only for Win8
#define WINDOWS_DESKTOP (WINDOWS     && !ARM       )

#define APPLE (MAC || IOS)

#define DESKTOP (WINDOWS_DESKTOP || MAC || LINUX  )
#define MOBILE  (WINDOWS_PHONE   || IOS || ANDROID)

#if (WINDOWS+MAC+IOS+LINUX+ANDROID+WEB)!=1
   #error Unsupported platform detected
#endif
/******************************************************************************/
#if EE_PRIVATE>=2
   #include "Esenthel Config.h" // include engine configuration created by "Esenthel Builder" or the default one if not available, this needs to be done after platform detection
#endif
#include "_/headers.h"
#include "_/types.h"
/******************************************************************************/
namespace EE // Esenthel Engine namespace
{
   /******************************************************************************/
   #pragma pack(push, 4)
   #include "_/defines.h"
   #include "_/structs.h"
   /******************************************************************************/
   // MISC
   /******************************************************************************/
   #include "Misc/Character.h"
   #include "Memory/Mem Simple.h"
   #include "Misc/String8.h"
   #include "Misc/String.h"
   #include "Misc/String Functions.h"
   #include "Misc/Cpu.h"
   #include "Memory/Mem Pointer.h"
   #include "Misc/Misc.h"
   #include "Memory/_Memc.h"
   #include "Memory/Mem Continuous.h"
   #include "Misc/UID.h"
   #include "Memory/Memory.h"
   #include "Memory/Counted Pointer.h"
   #include "Memory/Cipher.h"
   #include "Memory/Hash.h"
   #include "Memory/Mem Temporary.h"
   #include "Memory/Fixed Array.h"
   #include "Memory/_Memb.h"
   #include "Memory/Mem Block.h"
   #include "Memory/_Memx.h"
   #include "Memory/Mem Extended.h"
   #include "Memory/_Meml.h"
   #include "Memory/Mem List.h"
   #include "Misc/String Borrowed.h"
   #include "Memory/Node.h"
   #include "Misc/Date Time.h"
   #include "Memory/Index.h"
   //
   #include "Math/Vector.h"
   #include "Graphics/Color.h"
   #include "Misc/Timer.h"
   #include "Math/Matrix.h"
   #include "Math/Math.h"
   #include "Math/Orientation.h"
   #include "Math/Shapes/Rectangle.h"
   //
   #include "Misc/Sort.h"
   #include "Misc/Thread.h"
   #include "Misc/Reference.h"
   #include "Memory/_Memc Thread Safe.h"
   #include "Memory/Mem Continuous Thread Safe.h"
   #include "Memory/_Cache.h"
   #include "Memory/Cache.h"
   #include "Memory/_Map.h"
   #include "Memory/Map.h"
   #include "Misc/States.h"
   #include "File/File.h"
   #include "File/File Text.h"
   #include "File/IO.h"
   #include "File/Chunk.h"
   #include "Misc/Compress.h"
   #include "File/Pak.h"
   #include "Misc/Enum.h"
   #include "Misc/Param.h"
   #include "File/Xml.h"
   #include "Platforms/Android.h"
   #include "Misc/Callback.h"
   #include "Misc/DLL.h"
   #include "Misc/Printer.h"
   /******************************************************************************/
   // MATH
   /******************************************************************************/
   #include "Math/Shapes/Plane.h"
   #include "Math/Shapes/Edge.h"
   #include "Math/Shapes/Triangle.h"
   #include "Math/Shapes/Quad.h"
   #include "Math/Shapes/Box.h"
   #include "Math/Shapes/Circle.h"
   #include "Math/Shapes/Ball.h"
   #include "Math/Shapes/Capsule.h"
   #include "Math/Shapes/Tube.h"
   #include "Math/Shapes/Cone.h"
   #include "Math/Shapes/Pyramid.h"
   #include "Math/Shapes/Torus.h"
   #include "Math/Shapes/Polygon.h"
   #include "Math/Shape.h"
   #include "Math/Number.h"
   #include "Math/Quaternion.h"
   #include "Math/Camera.h"
   #include "Math/Calculate.h"
   #include "Math/Random.h"
   /******************************************************************************/
   // ANIMATION
   /******************************************************************************/
   #include "Animation/Skeleton.h"
   #include "Animation/Animation Keys.h"
   #include "Animation/Animation.h"
   #include "Animation/Motion.h"
   /******************************************************************************/
   // GRAPHICS
   /******************************************************************************/
   #include "Graphics/Display State.h"
   #include "Graphics/Image.h"
   #include "Graphics/Image Atlas.h"
   #include "Graphics/Image RT.h"
   #include "Gui/Font.h"
   #include "Gui/Text Style.h"
   #include "Graphics/Material.h"
   #include "Graphics/Display Draw.h"
   #include "Graphics/Renderer.h"
   #include "Graphics/Display.h"
   #include "Graphics/Light.h"
   #include "Graphics/Shader.h"
   #include "Graphics/Shader Main.h"
   #include "Graphics/Vertex Index Buffer Ex.h"
   #include "Graphics/Vertex Index Buffer.h"
   #include "Graphics/Sun.h"
   #include "Graphics/Fur.h"
   #include "Graphics/Effects 2D.h"
   #include "Graphics/Environment.h"
   /******************************************************************************/
   // SOUND
   /******************************************************************************/
   #include "Sound/Sound Extra.h"
   #include "Sound/Sound Stream.h"
   #include "Sound/Sound Buffer.h"
   #include "Sound/Sound.h"
   #include "Sound/Music.h"
   /******************************************************************************/
   // MESH
   /******************************************************************************/
   #include "Mesh/Mesh Csg.h"
   #include "Mesh/Mesh Render.h"
   #include "Mesh/Mesh Base.h"
   #include "Mesh/Mesh Part.h"
   #include "Mesh/Mesh Lod.h"
   #include "Mesh/Mesh Mesh.h"
   #include "Mesh/Mesh Group.h"
   #include "Mesh/Mesh Cuts.h"
   #include "Mesh/Mesh Import.h"
   #include "Mesh/Mesh Overlay.h"
   #include "Mesh/Material Palette.h"
   /******************************************************************************/
   // PHYSICS
   /******************************************************************************/
   #include "Physics/Phys Material.h"
   #include "Physics/Phys Body.h"
   #include "Physics/Joint.h"
   #include "Physics/Actor.h"
   #include "Physics/Vehicle.h"
   #include "Physics/Physics.h"
   #include "Physics/Aggregate.h"
   #include "Physics/Controller.h"
   #include "Physics/Ragdoll.h"
   #include "Physics/Grab.h"
   #include "Physics/Cloth.h"
   #include "Physics/Destruct Mesh.h"
   /******************************************************************************/
   //
   /******************************************************************************/
   #include "Math/Frustum.h"
   #include "Misc/Application.h"
   #include "Misc/Windows.h"
   #include "Input/Input Buttons.h"
   #include "Input/Input.h"
   #include "Input/Keyboard.h"
   #include "Input/Mouse.h"
   #include "Input/Joypad.h"
   #include "Memory/_Grid.h"
   #include "Memory/Grid.h"
   #include "Misc/Pathfind 2D.h"
   #include "Misc/Pathfind.h"
   #include "Misc/Video.h"
   #include "Misc/Member.h"
   #include "Graphics/Water.h"
   #include "Graphics/Sky.h"
   #include "Graphics/Fog.h"
   #include "Graphics/Clouds.h"
   #include "Graphics/Particle.h"
   #include "Graphics/Effects 3D.h"
   #include "Misc/Object.h"
   #include "Mesh/Heightmap.h"
   #include "Mesh/Blocks.h"
   #include "Mesh/Voxel.h"
   /******************************************************************************/
   // GUI
   /******************************************************************************/
   #include "Gui/Panel Image.h"
   #include "Gui/Panel.h"
   #include "Gui/Gui Skin.h"
   #include "Gui/Gui Object.h"
   #include "Gui/Gui Objects.h"
   #include "Gui/Gui.h"
   #include "Gui/Objects/Text.h"
   #include "Gui/Objects/Gui Image.h"
   #include "Gui/Objects/Button.h"
   #include "Gui/Objects/Tabs.h"
   #include "Gui/Objects/CheckBox.h"
   #include "Gui/Objects/Progress.h"
   #include "Gui/Objects/Slider.h"
   #include "Gui/Objects/SlideBar.h"
   #include "Gui/Objects/TextLine.h"
   #include "Gui/Objects/TextBox.h"
   #include "Gui/Objects/Region.h"
   #include "Gui/Objects/Viewport.h"
   #include "Gui/Objects/Gui Custom.h"
   #include "Gui/Objects/List.h"
   #include "Gui/Objects/Menu.h"
   #include "Gui/Objects/MenuBar.h"
   #include "Gui/Objects/ComboBox.h"
   #include "Gui/Objects/Window.h"
   #include "Gui/Objects/Window IO.h"
   #include "Gui/Objects/Desktop.h"
   #include "Gui/Objects/Property.h"
   #include "Gui/Objects/Color Picker.h"
   /******************************************************************************/
   // GAME
   /******************************************************************************/
   #include "Game/Obj.h"
   #include "Game/Objects/Static.h"
   #include "Game/Objects/Kinematic.h"
   #include "Game/Objects/Animatable.h"
   #include "Game/Objects/Destructible.h"
   #include "Game/Objects/Door.h"
   #include "Game/Objects/Item.h"
   #include "Game/Objects/Character.h"
   #include "Game/Objects/Obj Light Point.h"
   #include "Game/Objects/Obj Light Cone.h"
   #include "Game/Objects/Obj Particles.h"
   #include "Game/Objects/Obj Decal.h"
   #include "Game/Waypoint.h"
   #include "Game/Area.h"
   #include "Game/Mini Map.h"
   #include "Game/World.h"
   /******************************************************************************/
   // NET
   /******************************************************************************/
   #include "Net/Interpolator.h"
   #include "Net/Socket.h"
   #include "Net/Download.h"
   #include "Net/Patcher.h"
   #include "Net/Ftp.h"
   #include "Net/Connection.h"
   #include "Net/Net Object.h"
   #include "Net/Net World.h"
   #include "Net/Sql.h"
   #include "Net/Esenthel Store.h"
   #include "Net/Advertisement.h"
   #include "Net/Facebook.h"
   #include "Net/Store.h"
   #include "Net/Steam.h"
   #include "Net/Internet Cache.h"
   #include "Net/Game Analytics.h"
   /******************************************************************************/
   // EDIT
   /******************************************************************************/
   #include "Edit/Save Changes.h"
   #include "Edit/Viewport4.h"
   #include "Edit/Version.h"
   #include "Edit/Undo.h"
   /******************************************************************************/
   // CODE
   /******************************************************************************/
   #include "Code/CE Shared.h"
   #include "Code/CE Symbol.h"
   #include "Code/CE Token.h"
   #include "Code/CE Macro.h"
   #include "Code/CE Parser.h"
   #include "Code/CE Code Col Line.h"
   #include "Code/CE Source.h"
   #include "Code/CE Environment.h"
   #include "Code/CE Expression.h"
   #include "Code/CE Compiler.h"
   #include "Code/CE Item.h"
   #include "Code/CE Find.h"
   #include "Code/CE.h"
   /******************************************************************************/
   #include "Edit/Editor Interface.h"
   #include "Graphics/Renderer Instancing.h"
   #include "Input/Touch.h"
   #include "_/inline.h"
   /******************************************************************************/
   #pragma pack(pop)
   /******************************************************************************/
} // namespace
/******************************************************************************/
using namespace EE;
/******************************************************************************/
// DECLARATIONS FOR USER PROVIDED MAIN FUNCTIONS
/******************************************************************************/
Bool Preload(); // this method is called only on Web platform, it is called before InitPre, it will be called continuously as long as you're returning 'true' inside it. You should use this method to optionally download necessary files, like "Engine.pak". Once you've finished downloading files, return 'false' so that the application can continue, and proceed to 'InitPre'. After that, 'Preload' will never be called again.
void InitPre(); // init before engine, in this function you can modify things like: applications settings (App), Display options (D), Renderer options (Renderer), camera parameters (Cam), file path settings (DataPath), loading data paks (Paks.add), etc. In this stage the engine is not yet fully initialized, so creating GPU (Images, Meshes, ..) and Sound data will fail, this data should be processed later, for example in Init().
Bool Init   (); // init after  engine
void Shut   (); // shut at exit
Bool Update (); // update
void Draw   (); // draw
/******************************************************************************/
