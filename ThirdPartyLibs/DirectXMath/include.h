/******************************************************************************/
#if !WINDOWS
   typedef U32 DWORD;
   #define _In_range_(a, b)
   #define _In_
   #define _Out_
   #define _Inout_
   #define _Analysis_assume_(x)
   #define _Out_opt_
   #define _Out_writes_(x)
   #define _Out_writes_opt_(x)
   #define _Out_writes_all_(x)
   #define _Out_writes_bytes_(x)
   #define _In_reads_(x)
   #define _In_reads_bytes_(x)
   #define _In_reads_opt_(x)
   #define _Inout_updates_all_(x)
   #define _Inout_updates_all_opt_(x)
   #define _Inout_updates_bytes_all_(x)
   #define _Use_decl_annotations_
   #define _When_(x, y)
   #define _Success_(x)
   #define XMGLOBALCONST const
   #undef  XM_DEPRECATED
   #define XM_DEPRECATED
   #define ARRAYSIZE(A) Elms(A)
   #define UNREFERENCED_PARAMETER(P)
   #define _XM_NO_INTRINSICS_
   #define XM_CALLCONV
#endif

#if WINDOWS_NEW // WINDOWS_NEW already has these headers included always, and including our custom ones would introduce conflict, so in this case always use system headers
   #include <DirectXMath.h>
   #include <DirectXPackedVector.h>
#else
   #include "DirectXMath.h"
   #include "DirectXPackedVector.h"
#endif
/******************************************************************************/
