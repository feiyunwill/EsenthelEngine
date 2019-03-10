/******************************************************************************/
#ifdef _WIN32 // WINDOWS
   #define LOCK_WRITE WindowsLOCK_WRITE
#elif defined __APPLE__ // APPLE
   #define Ptr       ApplePtr
   #define Point     ApplePoint
   #define Cell      AppleCell
   #define Rect      AppleRect
   #define Button    AppleButton
   #define Cursor    AppleCursor
   #define FileInfo  AppleFileInfo
   #define TextStyle AppleTextStyle
#elif (defined __linux__ && !defined ANDROID) || defined EMSCRIPTEN // LINUX or WEB
   #define Time   LinuxTime
   #define Font   LinuxFont
   #define Region LinuxRegion
   #define Window XWindow
   #define Cursor XCursor
#elif defined ANDROID // ANDROID
#endif
/******************************************************************************/
