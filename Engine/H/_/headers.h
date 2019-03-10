/******************************************************************************/
#define PHYSX_DLL 0 // if dynamically link PhysX

#if EE_PRIVATE
   // Threads
   #if WEB
      #define HAS_THREADS 0 // WEB has no real threads
   #else
      #define HAS_THREADS 1
   #endif
   /******************************************************************************/
   // SELECT WHICH LIBRARIES TO USE
   /******************************************************************************/
   // Renderer - Define "DX9" for DirectX 9, "DX11" for DirectX 10/11, "DX12" for DirectX 12, "METAL" for Metal, "VULKAN" for Vulkan, "GL" or nothing for OpenGL
   // defines are specified through Project Settings
   #ifdef DX9
      #undef  DX9
      #define DX9 1
   #else
      #define DX9 0
   #endif

   #ifdef DX11
      #undef  DX11
      #define DX11 1
   #else
      #define DX11 0
   #endif

   #ifdef DX12
      #undef  DX12
      #define DX12 1
   #else
      #define DX12 0
   #endif

   #ifdef METAL
      #undef  METAL
      #define METAL 1
   #else
      #define METAL 0
   #endif

   #ifdef VULKAN
      #undef  VULKAN
      #define VULKAN 1
   #else
      #define VULKAN 0
   #endif

   #if defined GL || !(DX9 || DX11 || DX12 || METAL || VULKAN)
      #undef  GL
      #define GL 1
   #else
      #define GL 0
   #endif

   #if (DX9+DX11+DX12+METAL+VULKAN+GL)!=1
      #error Unsupported platform detected
   #endif

   #define GL_ES (GL && (IOS || ANDROID || WEB))

   #define GL_LOCK (GL && 0) // if surround all GL calls with a lock

   #define SLOW_SHADER_LOAD GL // Only OpenGL has slow shader loads because it compiles on the fly from text instead of binary

   #if DX9
      #define GPU_API(dx9, dx11, gl) dx9
   #elif DX11
      #define GPU_API(dx9, dx11, gl) dx11
   #elif GL
      #define GPU_API(dx9, dx11, gl) gl
   #endif

   #if GL_ES
      #define GPU_HALF_SUPPORTED 0 // depends on "GL_OES_vertex_half_float" GLES extension
   #else
      #define GPU_HALF_SUPPORTED 1
   #endif

   #define REVERSE_DEPTH (!GL) // if Depth Buffer is reversed

   #define PHYSX_DLL_ACTUAL (PHYSX && PHYSX_DLL && (WINDOWS_OLD || LINUX))

   #if PHYSX
      #define PHYS_API(physx, bullet) physx
   #else
      #define PHYS_API(physx, bullet) bullet
   #endif

   #if PHYSX && (WINDOWS || LINUX)
      #define PX_FOUNDATION_DLL 0
      #define PX_PHYSX_STATIC_LIB
   #endif

   // Sound
   #define DIRECT_SOUND WINDOWS_OLD             // use DirectSound on Windows Classic
   #define XAUDIO       WINDOWS_NEW             // use XAudio      on Windows Universal
   #define OPEN_AL      (APPLE || LINUX || WEB) // use OpenAL      on Apple, Linux and Web. OpenAL on Windows requires OpenAL DLL file, however it can be enabled just for testing the implementation.
   #define OPEN_SL      ANDROID                 // use OpenSL      on Android
   /******************************************************************************/
   // INCLUDE SYSTEM HEADERS
   /******************************************************************************/
   // this needs to be included first, as some macros may cause conflicts with the headers
   #include "../../../ThirdPartyLibs/begin.h"

   #if WINDOWS // Windows
      #define SUPPORT_WINDOWS_XP (!X64 && DX9) // 0=minor performance improvements in some parts of the engine, but no WindowsXP support, 1=some extra checks in the codes but with WindowsXP support
      #if WINDOWS_OLD
         #if SUPPORT_WINDOWS_XP // https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745.aspx (this can be used for compilation testing if we don't use any functions not available on WindowsXP, however we can use defines and enums)
            #define _WIN32_WINNT 0x0502 // _WIN32_WINNT_WS03 , don't use any API's newer than WindowsXP SP2
         #else
            #define _WIN32_WINNT 0x0600 // _WIN32_WINNT_VISTA, don't use any API's newer than Windows Vista
         #endif
      #endif
      #define NOGDICAPMASKS
      #define NOICONS
      #define NOKEYSTATES
      #define OEMRESOURCE
      #define NOATOM
      #define NOCOLOR
      #define NODRAWTEXT
      #define NOKERNEL
      #define NOMEMMGR
      #define NOMETAFILE
      #define NOMINMAX
      #define NOOPENFILE
      #define NOSCROLL
      #define NOSERVICE
      #define NOSOUND
      #define NOCOMM
      #define NOKANJI
      #define NOHELP
      #define NOPROFILER
      #define NODEFERWINDOWPOS
      #define NOMCX
      #define _ISO646
      #define TokenType    WindowsTokenType
      #define UpdateWindow WindowsUpdateWindow
      #define TimeStamp    WindowsTimeStamp
      #define LOCK_WRITE   WindowsLOCK_WRITE
      #define _ALLOW_RTCc_IN_STL

      #pragma warning(disable:4091) // 'typedef ': ignored on left of '' when no variable is declared
      #include <stdio.h>
      #include <new.h>
      #include <typeinfo>
      #include <type_traits>
      #include <atomic>
      #include <process.h>
      #include <winsock2.h>
      #include <ws2tcpip.h>
      #include <windows.h>
      #include <io.h>
      #include <share.h>
      #include <fcntl.h>
      #include <sys/stat.h>
      #include <intrin.h>
      #include <locale.h>
      #if WINDOWS_OLD
         #include <IPHlpApi.h> // including this fails on WINDOWS_NEW
         #include <shlobj.h>
         #include <psapi.h>
         #include <wbemidl.h>
         #include <tlhelp32.h>
         #define SECURITY_WIN32
         #include <Security.h>
         #include <comdef.h>
      #else
         #include <collection.h>
         #include <ppltasks.h>
         #include <mmdeviceapi.h>
         #include <audioclient.h>
         #include <wrl/implements.h>
      #endif
      #if OPEN_AL
         #include "../../../ThirdPartyLibs/OpenAL for Windows/al.h"
         #include "../../../ThirdPartyLibs/OpenAL for Windows/alc.h"
      #endif
      #include "../../../ThirdPartyLibs/DirectX/xinput.h"
      #if WINDOWS_OLD
         #define DIRECTINPUT_VERSION 0x0800
         #include "../../../ThirdPartyLibs/DirectX/dinput.h"
      #endif
      #if DIRECT_SOUND
         #include "../../../ThirdPartyLibs/DirectX/dsound.h"
      #elif XAUDIO
         #if WINDOWS_OLD
            #include "../../../ThirdPartyLibs/DirectX/old/xaudio2.h"
            #include "../../../ThirdPartyLibs/DirectX/old/x3daudio.h"
         #else
            #include "../../../ThirdPartyLibs/DirectX/xaudio2.h"
            #include "../../../ThirdPartyLibs/DirectX/x3daudio.h"
         #endif
      #endif
   #elif APPLE // Apple
      #define Ptr       ApplePtr
      #define Point     ApplePoint
      #define Cell      AppleCell
      #define Rect      AppleRect
      #define Button    AppleButton
      #define Cursor    AppleCursor
      #define FileInfo  AppleFileInfo
      #define TextStyle AppleTextStyle
      #define gamma     AppleGamma
      #define ok        AppleOk
      #define require   AppleRequire
      #define __STDBOOL_H // to ignore stdbool.h header which defines _Bool which is used by PhysX otherwise
   #elif LINUX || WEB
      #define Time   LinuxTime
      #define Font   LinuxFont
      #define Region LinuxRegion
      #define Window XWindow
      #define Cursor XCursor
   #elif ANDROID // Android
      #include <stdio.h>
      #include <new>
      #include <typeinfo>
      #include <type_traits>
      #include <atomic>
      #include <fcntl.h>
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <locale.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/socket.h>
      #include <sys/wait.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <linux/if.h>
      #include <arpa/inet.h>
      #include <android/log.h>
      #include <android/sensor.h>
      #include <android/asset_manager.h>
      #include <android/api-level.h> // needed for __ANDROID_API__
      #include <android_native_app_glue.h>
      #include <SLES/OpenSLES.h>
      #include <SLES/OpenSLES_Android.h>
   #if __ANDROID_API__>=21
      #include <sys/statvfs.h>
   #endif
      #undef LOCK_READ
      #undef LOCK_WRITE
   #endif

   // Renderer
   #define MAX_FVF_DECL_SIZE (MAXD3DDECLLENGTH+1) // +1 for END
   #if DX9|DX11 // DirectX 9, 11
      #include "../../../ThirdPartyLibs/DirectX/dxgi1_6.h"
      #include "../../../ThirdPartyLibs/DirectX/d3d9.h"
      #include "../../../ThirdPartyLibs/DirectX/d3d11_4.h"
      #if DX11
         #include "../../../ThirdPartyLibs/DirectX/d3dcompiler.h"
         #include "../../../ThirdPartyLibs/DirectX/d3dcommon.h"
      #endif
   #elif GL // OpenGL
      #if WINDOWS
         #define  GLEW_STATIC
         #include "../../../ThirdPartyLibs/GL/glew.h"
         #include "../../../ThirdPartyLibs/GL/wglew.h"
         // if we're compiling OpenGL on windows, then include DirectX headers as well, to support some basic DirectX functions
         #include "../../../ThirdPartyLibs/DirectX/d3d9.h"
         #include "../../../ThirdPartyLibs/DirectX/d3d11_3.h"
      #elif MAC
         #include <OpenGL/gl3.h>
         #include <OpenGL/gl3ext.h>
      #elif LINUX
         #define GL_GLEXT_PROTOTYPES
         #define GLX_GLXEXT_PROTOTYPES
         #include <GL/gl.h>
         #include <GL/glext.h>
         #include <GL/glx.h>
      #elif ANDROID // Android OpenGL ES
      	#include <EGL/egl.h>
         #if __ANDROID_API__>=18 // GL ES 3.0 headers available only on API 18 and newer
            #include <GLES3/gl3.h>
            #include <GLES3/gl3ext.h>
         #else
            #include <GLES2/gl2.h>
            #include <GLES2/gl2ext.h>
         #endif
      #elif IOS // iOS OpenGL ES
         #include <UIKit/UIKit.h>
         #include <QuartzCore/QuartzCore.h>
         #include <OpenGLES/EAGL.h>
         #include <OpenGLES/EAGLDrawable.h>
         #include <OpenGLES/ES3/gl.h>
         #include <OpenGLES/ES3/glext.h>
      #elif WEB // WebGL
         #define  GL_GLEXT_PROTOTYPES
         #include <EGL/egl.h>
         #include <GLES3/gl3.h>
      #endif
   #endif

   #if WINDOWS
      #undef GetComputerName
      #undef THIS
      #undef IGNORE
      #undef TRANSPARENT
      #undef ERROR
      #undef UNIQUE_NAME
      #undef INPUT_MOUSE
      #undef INPUT_KEYBOARD
      #undef near
      #undef min
      #undef max
      #undef TokenType
      #undef UpdateWindow
      #undef TimeStamp
      #undef LOCK_WRITE
      #undef RGB
      #undef ReplaceText
      #undef FindText
   #endif

   #if APPLE // Mac, iOS
      #include <stdio.h>
      #include <new>
      #include <typeinfo>
      #include <type_traits>
      #include <atomic>
      #include <fcntl.h>
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <locale.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/statvfs.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/sysctl.h>
      #include <sys/socket.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <ifaddrs.h>
      #include <net/if.h>
      #include <net/if_dl.h>  
      #include <arpa/inet.h>
      #include <mach/mach.h>
      #include <mach/clock.h>
      #include <mach/mach_time.h>
      #include <libkern/OSAtomic.h>
      #include <OpenAL/al.h>
      #include <OpenAL/alc.h>
      #include <CoreFoundation/CoreFoundation.h>
      #include <StoreKit/StoreKit.h>
      #include <AudioToolbox/AudioToolbox.h>
   #if MAC
      #include <smmintrin.h>
      #include <wmmintrin.h>
      #include <CoreAudio/CoreAudio.h>
      #include <net/if_types.h>
      #include <IOKit/hid/IOHIDLib.h>
      #include <IOKit/pwr_mgt/IOPMLib.h>
      #include <Carbon/Carbon.h>
      #include <OpenGL/OpenGL.h>
      #ifdef __OBJC__
         #include <Cocoa/Cocoa.h>
      #endif
   #elif IOS
      #define IFT_ETHER 0x06 // iOS does not have this in headers
      #include <CoreMotion/CoreMotion.h>
      #include <CoreLocation/CoreLocation.h>
      #include <AVFoundation/AVFoundation.h>
      #include <AdSupport/ASIdentifierManager.h>
      #include <FBSDKCoreKit/FBSDKCoreKit.h>
      #include <FBSDKLoginKit/FBSDKLoginKit.h>
      #include <FBSDKShareKit/FBSDKShareKit.h>
      #include "../../../ThirdPartyLibs/Chartboost/Headers/Chartboost.h"
   #endif
      #undef Ptr
      #undef Point
      #undef Cell
      #undef Rect
      #undef Button
      #undef Cursor
      #undef FileInfo
      #undef TextStyle
      #undef gamma
      #undef ok
      #undef require
      #undef verify
      #undef check
      #undef MIN
      #undef MAX
      #undef ABS
   #endif

   #if LINUX // Linux
      #include <stdio.h>
      #include <new>
      #include <malloc.h>
      #include <typeinfo>
      #include <type_traits>
      #include <atomic>
      #include <fcntl.h>
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <locale.h>
      #include <cpuid.h>
      #include <pwd.h>
      #include <smmintrin.h>
      #include <wmmintrin.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/statvfs.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/socket.h>
      #include <sys/wait.h>
      #include <sys/resource.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <ifaddrs.h>
      #include <linux/if.h>
      #include <arpa/inet.h>
      #include <string.h>
      #include <X11/Xatom.h>
      #include <X11/XKBlib.h>
      #include <X11/Xmu/WinUtil.h>
      #include <X11/Xcursor/Xcursor.h>
      #include <X11/extensions/xf86vmode.h>
      #include <X11/extensions/XInput2.h>
      #include <Xm/MwmUtil.h>
      #if OPEN_AL
         #include "AL/al.h"
         #include "AL/alc.h"
      #endif
      #undef LOCK_READ
      #undef LOCK_WRITE
      #undef PropertyNotify
      #undef Status
      #undef Convex
      #undef Button1
      #undef Button2
      #undef Button3
      #undef Button4
      #undef Button5
      #undef Bool
      #undef Time
      #undef Region
      #undef Window
      #undef Cursor
      #undef Font
      #undef None
      #undef Success
      #undef B32
      #undef B16
   #endif

   #if WEB // Web
      #include <emscripten.h>
      #include <emscripten/html5.h>
      #include <stdio.h>
      #include <new>
      #include <typeinfo>
      #include <type_traits>
      #include <atomic>
      #include <fcntl.h>
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <locale.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/statvfs.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/socket.h>
      #include <sys/wait.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <arpa/inet.h>
      #include <string.h>
      #include <X11/Xatom.h>
      #include <X11/Xlib.h>
      #include <X11/Xutil.h>
      #if OPEN_AL
         #include "AL/al.h"
         #include "AL/alc.h"
      #endif
      #undef LOCK_READ
      #undef LOCK_WRITE
      #undef PropertyNotify
      #undef Status
      #undef Convex
      #undef Button1
      #undef Button2
      #undef Button3
      #undef Button4
      #undef Button5
      #undef Bool
      #undef Time
      #undef Region
      #undef Window
      #undef Cursor
      #undef Font
      #undef None
      #undef Success
   #endif
   /******************************************************************************/
   // INCLUDE THIRD PARTY LIBRARIES
   /******************************************************************************/
   // Physics
   #if PHYSX // use PhysX
      #ifndef NDEBUG
         #define NDEBUG
      #endif
      #if ARM
         #include <arm_neon.h> // TODO: remove this once fixed in PhysX, include ARM NEON before PhysX headers, as workaround for https://github.com/NVIDIAGameWorks/PhysX-3.4/issues/77
      #endif
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxAggregate.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxBatchQuery.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxMaterial.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxPhysics.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxPhysicsVersion.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxRigidDynamic.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxRigidStatic.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxScene.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxShape.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/PxSimulationEventCallback.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxBoxGeometry.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxSphereGeometry.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxCapsuleGeometry.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxPlaneGeometry.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxConvexMeshGeometry.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxTriangleMeshGeometry.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxHeightFieldGeometry.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxConvexMesh.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxTriangleMesh.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxHeightField.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/geometry/PxGeometryQuery.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/cloth/PxCloth.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/cloth/PxClothFabric.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/cloth/PxClothParticleData.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/common/PxTolerancesScale.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/cooking/PxCooking.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxClothFabricCooker.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxDefaultAllocator.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxDefaultErrorCallback.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxDefaultCpuDispatcher.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxDefaultSimulationFilterShader.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxD6Joint.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxDistanceJoint.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxFixedJoint.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxJoint.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxPrismaticJoint.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxRigidBodyExt.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxRevoluteJoint.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/extensions/PxSphericalJoint.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/vehicle/PxVehicleDrive4W.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/vehicle/PxVehicleWheels.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/vehicle/PxVehicleUpdate.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/vehicle/PxVehicleTireFriction.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Include/vehicle/PxVehicleUtilControl.h"
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Source/GeomUtils/src/mesh/GuMeshData.h"     // needed for PX_MESH_VERSION
      #include "../../../ThirdPartyLibs/PhysX/PhysX/Source/GeomUtils/src/convex/GuConvexMesh.h" // needed for PX_CONVEX_VERSION
      #include "../../../ThirdPartyLibs/PhysX/PxShared/include/foundation/PxFoundation.h"
      #include "../../../ThirdPartyLibs/PhysX/PxShared/include/foundation/PxFoundationVersion.h"
      #include "../../../ThirdPartyLibs/PhysX/PxShared/include/foundation/PxIO.h"
      #include "../../../ThirdPartyLibs/PhysX/PxShared/include/task/PxTask.h"
      using namespace physx;
   #endif

   // always include Bullet to generate optimized PhysBody if needed
   #pragma warning(push             )
   #pragma warning(disable:4359     ) // Alignment specifier is less than actual alignment (128), and will be ignored.
   #pragma warning(disable:4311 4302) // 'type cast': pointer truncation from '' to ''
   #include "../../../ThirdPartyLibs/Bullet/lib/src/btBulletDynamicsCommon.h"
   #pragma warning(pop)

   // Recast/Detour path finding
   #include "../../../ThirdPartyLibs/Recast/Recast/Include/Recast.h"
   #include "../../../ThirdPartyLibs/Recast/Recast/Include/RecastAlloc.h"
   #include "../../../ThirdPartyLibs/Recast/Detour/Include/DetourNavMesh.h"
   #include "../../../ThirdPartyLibs/Recast/Detour/Include/DetourNavMeshQuery.h"
   #include "../../../ThirdPartyLibs/Recast/Detour/Include/DetourNavMeshBuilder.h"

   // SSL/TLS/HTTPS
   #define SUPPORT_MBED_TLS (!WEB)
   #if     SUPPORT_MBED_TLS
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/config.h"

      #ifdef MBEDTLS_PLATFORM_C
         #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/platform.h"
      #else
         #define mbedtls_time    time 
         #define mbedtls_time_t  time_t
         #define mbedtls_fprintf fprintf
         #define mbedtls_printf  printf
      #endif

      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/net_sockets.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/debug.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/ssl.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/entropy.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/ctr_drbg.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/error.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/certs.h"
   #endif

   #include <algorithm> // must be after PhysX or compile errors will show on Android
   /******************************************************************************/
   // Finish including headers - this needs to be included after all headers
   #include "../../../ThirdPartyLibs/end.h"
   /******************************************************************************/
#else
#if WINDOWS
   #ifndef __PLACEMENT_NEW_INLINE
      #define __PLACEMENT_NEW_INLINE
      inline void* __cdecl operator new   (size_t, void *where) {return where;}
      inline void  __cdecl operator delete(void *, void *     )throw() {}
   #endif
   #undef GetComputerName
   #undef TRANSPARENT
   #undef ERROR
   #undef INPUT_MOUSE
   #undef INPUT_KEYBOARD
   #undef min
   #undef max
   #define _ALLOW_RTCc_IN_STL
#else
   #include <new>
   #include <stdint.h>
   #include <stddef.h>
#endif
#include <math.h>
#include <typeinfo>
#include <type_traits> // needed for 'std::enable_if', 'std::is_enum'
#if ANDROID
   #include <android/api-level.h> // needed for __ANDROID_API__
#endif
#endif
#if !WINDOWS_NEW
namespace std{typedef decltype(nullptr) nullptr_t;}
#endif
/******************************************************************************/
