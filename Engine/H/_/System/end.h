/******************************************************************************/
#ifdef _WIN32 // WINDOWS
   #undef LOCK_WRITE
   #undef GetComputerName
   #undef TRANSPARENT
#elif defined __APPLE__ // APPLE
   #undef Ptr
   #undef Point
   #undef Cell
   #undef Rect
   #undef Button
   #undef Cursor
   #undef FileInfo
   #undef TextStyle
   #undef MIN
   #undef MAX
   #undef ABS
#elif (defined __linux__ && !defined ANDROID) || defined EMSCRIPTEN // LINUX or WEB
   #undef Time
   #undef Font
   #undef Region
   #undef Window
   #undef Cursor
   #undef Bool
   #undef LOCK_READ
   #undef LOCK_WRITE
#elif defined ANDROID // ANDROID
   #undef LOCK_READ
   #undef LOCK_WRITE
#endif
/******************************************************************************/
