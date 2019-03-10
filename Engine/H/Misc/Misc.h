/******************************************************************************

   Here is a list of miscellaneous helper functions.

/******************************************************************************/
enum ALIGN_TYPE // Align Type
{
   ALIGN_NONE  , // no align
   ALIGN_MIN   , // align to minimum
   ALIGN_CENTER, // align to center
   ALIGN_MAX   , // align to maximum
};
#if EE_PRIVATE
enum FADE_TYPE : Byte // Fade Type
{
   FADE_NONE, // no fading
   FADE_IN  , // fading in
   FADE_OUT , // fading out
};
#endif
/******************************************************************************/
template<typename TYPE, Int elms>   constexpr Int Elms(C TYPE (&Array)[elms]) {return elms;} // get number of elements in array

inline Bool InRange(Int   i, Byte  elms) {return UInt (i)<UInt (elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"
inline Bool InRange(Int   i, Int   elms) {return UInt (i)<UInt (elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"
inline Bool InRange(Int   i, UInt  elms) {return UInt (i)<UInt (elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"
inline Bool InRange(UInt  i, Int   elms) {return UInt (i)<UInt (elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"
inline Bool InRange(UInt  i, UInt  elms) {return UInt (i)<UInt (elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"
inline Bool InRange(Long  i, Long  elms) {return ULong(i)<ULong(elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"
inline Bool InRange(Long  i, ULong elms) {return ULong(i)<ULong(elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"
inline Bool InRange(ULong i, ULong elms) {return ULong(i)<ULong(elms);} // if 'i' index is in range "0..elms-1", this assumes that "elms>=0"

T1(TYPE) ENABLE_IF_ENUM(TYPE, Bool) InRange(Int  i, TYPE enum_value) {return UInt(i)<UInt(enum_value);} // template specialization for enum's
T1(TYPE) ENABLE_IF_ENUM(TYPE, Bool) InRange(TYPE i, TYPE enum_value) {return UInt(i)<UInt(enum_value);} // template specialization for enum's

T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Int   i, C TYPE &container); // if 'i' index is in range of container, 'container' can be of many types, for example a C++ array (x[]), memory container ('Memc', 'Memb', ..) or any other type for which 'Elms' function was defined
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(UInt  i, C TYPE &container); // if 'i' index is in range of container, 'container' can be of many types, for example a C++ array (x[]), memory container ('Memc', 'Memb', ..) or any other type for which 'Elms' function was defined
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Long  i, C TYPE &container); // if 'i' index is in range of container, 'container' can be of many types, for example a C++ array (x[]), memory container ('Memc', 'Memb', ..) or any other type for which 'Elms' function was defined
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(ULong i, C TYPE &container); // if 'i' index is in range of container, 'container' can be of many types, for example a C++ array (x[]), memory container ('Memc', 'Memb', ..) or any other type for which 'Elms' function was defined

Str GetBase     (C Str &name); // get     base name                  , sample usage: GetBase     ("C:/Folder/file.ext") -> "file.ext"
Str GetBaseNoExt(C Str &name); // get     base name without extension, sample usage: GetBaseNoExt("C:/Folder/file.ext") -> "file"
Str GetExt      (C Str &name); // get     extension                  , sample usage: GetExt      ("C:/Folder/file.ext") -> "ext"
Str GetExtNot   (C Str &name); // get not extension                  , sample usage: GetExtNot   ("C:/Folder/file.ext") -> "C:/Folder/file"
Str GetPath     (C Str &name); // get     path                       , sample usage: GetPath     ("C:/Folder/file.ext") -> "C:/Folder"
Str GetStart    (C Str &name); // get     path start                 , sample usage: GetStart    ("C:/Folder/file.ext") -> "C:"
Str GetStartNot (C Str &name); // get not path start                 , sample usage: GetStartNot ("C:/Folder/file.ext") -> "Folder/file.ext"

Str GetRelativePath(Str src, Str dest); // get relative path from 'src' location to 'dest' file, sample usage: GetRelativePath("C:/Folder", "C:/dest.txt") -> "../dest.txt"

#if EE_PRIVATE
INLINE Bool IsSlash(Char c) {return c=='/' || c=='\\';}

CChar * _GetBase     (CChar  *name, Bool tail_slash= 0, Char  (&dest)[MAX_LONG_PATH]=ConstCast(TempChar <MAX_LONG_PATH>()).c); // 'tail_slash'=if keep   tail slash
CChar * _GetBaseNoExt(CChar  *name,                     Char  (&dest)[MAX_LONG_PATH]=ConstCast(TempChar <MAX_LONG_PATH>()).c);
CChar * _GetExt      (CChar  *name,                     Char  (&dest)[MAX_LONG_PATH]=ConstCast(TempChar <MAX_LONG_PATH>()).c);
CChar * _GetExtNot   (CChar  *name,                     Char  (&dest)[MAX_LONG_PATH]=ConstCast(TempChar <MAX_LONG_PATH>()).c);
CChar * _GetPath     (CChar  *name, Int  tail_slash=-1, Char  (&dest)[MAX_LONG_PATH]=ConstCast(TempChar <MAX_LONG_PATH>()).c); // 'tail_slash'=if insert tail slash (1=always, 0=never, -1=if name had a tail slash)
 Char8* _GetStart    (CChar8 *name,                     Char8 (&dest)[MAX_LONG_PATH]=ConstCast(TempChar8<MAX_LONG_PATH>()).c);
 Char * _GetStart    (CChar  *name,                     Char  (&dest)[MAX_LONG_PATH]=ConstCast(TempChar <MAX_LONG_PATH>()).c);
CChar * _GetStartNot (CChar  *name);

Str  WindowsPath    (C Str &path); // replace '/'  with '\\'
Str     UnixPath    (C Str &path); // replace '\\' with '/'
Str8    UnixPathUTF8(C Str &path); // return UnixPath in UTF-8 format
#endif

Bool  IsDrive (CChar *path);   Bool  IsDrive (CChar8 *path); // if path is a     drive  path, sample usage:  IsDrive ("x:/"     ) -> true,  IsDrive("x:/data/0.bmp") -> false,  IsDrive("data/0.bmp") -> false
Bool HasDrive (CChar *path);   Bool HasDrive (CChar8 *path); // if path contains drive  path, sample usage: HasDrive ("x:/"     ) -> true, HasDrive("x:/data/0.bmp") -> true , HasDrive("data/0.bmp") -> false
Bool HasRemote(CChar *path);   Bool HasRemote(CChar8 *path); // if path contains remote path, sample usage: HasRemote("\\\\comp") -> true
Bool FullPath (CChar *path);   Bool FullPath (CChar8 *path); // if path is a full path (has a drive or is remote)

T3(TA,TB,TC) inline TA& Clamp(TA &x, TB min, TC max) {if(x<min)x=min;else if(x>max)x=max; return x;} // clamp 'x' to "min..max" range

T2(TA,TB   ) inline Bool FlagTest   (TA  flags, TB f           ) {return (flags&f)!=0;}                                   // check if 'f' flag is enabled                in 'flags', in case 'f' contains multiple options then this will succeed if any of them are enabled
T2(TA,TB   ) inline Bool FlagAll    (TA  flags, TB f           ) {return (flags&f)==f;}                                   // check if 'f' flag is enabled                in 'flags', in case 'f' contains multiple options then this will succeed if all of them are enabled
T2(TA,TB   ) inline void FlagEnable (TA &flags, TB f           ) {flags|= f;}                                             // enable   'f' flag                           in 'flags'
T2(TA,TB   ) inline void FlagDisable(TA &flags, TB f           ) {flags&=~f;}                                             // disable  'f' flag                           in 'flags'
T2(TA,TB   ) inline void FlagToggle (TA &flags, TB f           ) {flags^= f;}                                             // toggle   'f' flag                           in 'flags'
T2(TA,TB   ) inline void FlagSet    (TA &flags, TB f, Bool on  ) {if(on)FlagEnable(flags, f);else FlagDisable(flags, f);} // set      'f' flag to be enabled or disabled in 'flags'
T3(TA,TB,TC) inline void FlagCopy   (TA &flags, TB f, TC   mask) {flags=(flags&~mask)|(f&mask);}                          // copy     'f' flags                          to 'flags' using specified mask

inline UInt IndexToFlag(Int i) {return 1<<i;} // convert index to flag

inline Int   DivFloor(Int   x, Int   y) {return (x>=0) ?  x     /y : (x-y+1)/y;} // integer divide with floor
inline Long  DivFloor(Long  x, Long  y) {return (x>=0) ?  x     /y : (x-y+1)/y;} // integer divide with floor
inline UInt  DivFloor(UInt  x, UInt  y) {return           x     /y            ;} // integer divide with floor
inline ULong DivFloor(ULong x, ULong y) {return           x     /y            ;} // integer divide with floor
inline Int   DivCeil (Int   x, Int   y) {return (x<=0) ?  x     /y : (x+y-1)/y;} // integer divide with ceil
inline Long  DivCeil (Long  x, Long  y) {return (x<=0) ?  x     /y : (x+y-1)/y;} // integer divide with ceil
inline UInt  DivCeil (UInt  x, UInt  y) {return                      (x+y-1)/y;} // integer divide with ceil
inline ULong DivCeil (ULong x, ULong y) {return                      (x+y-1)/y;} // integer divide with ceil
inline Int   DivRound(Int   x, Int   y) {return (x>=0) ? (x+y/2)/y : (x-y/2)/y;} // integer divide with round
inline Long  DivRound(Long  x, Long  y) {return (x>=0) ? (x+y/2)/y : (x-y/2)/y;} // integer divide with round
inline UInt  DivRound(UInt  x, UInt  y) {return          (x+y/2)/y            ;} // integer divide with round
inline ULong DivRound(ULong x, ULong y) {return          (x+y/2)/y            ;} // integer divide with round

inline Int     Mod(Int  x, Int  y          ) {if(!y)return 0; Int  z=x%y; return (z>=0) ? z : z+y;} // safe modulo "x%y" , returns always a positive number between   "0..y-1"
inline Long    Mod(Long x, Long y          ) {if(!y)return 0; Long z=x%y; return (z>=0) ? z : z+y;} // safe modulo "x%y" , returns always a positive number between   "0..y-1"
       Int  MidMod(Int  x, Int min, Int max);                                                       // safe middle modulo, returns always a          number between "min..max"

UInt Ceil2      (UInt  x); // rounds 'x' to the nearest multiple of 2  , which is equal or greater than 'x'
UInt Ceil4      (UInt  x); // rounds 'x' to the nearest multiple of 4  , which is equal or greater than 'x'
UInt Ceil8      (UInt  x); // rounds 'x' to the nearest multiple of 8  , which is equal or greater than 'x'
UInt Ceil16     (UInt  x); // rounds 'x' to the nearest multiple of 16 , which is equal or greater than 'x'
UInt Ceil32     (UInt  x); // rounds 'x' to the nearest multiple of 32 , which is equal or greater than 'x'
UInt Ceil64     (UInt  x); // rounds 'x' to the nearest multiple of 64 , which is equal or greater than 'x'
UInt Ceil128    (UInt  x); // rounds 'x' to the nearest multiple of 128, which is equal or greater than 'x'
UInt CeilPow2   (UInt  x); // rounds 'x' to the nearest power    of 2  , which is equal or greater than 'x'
UInt FloorPow2  (UInt  x); // rounds 'x' to the nearest power    of 2  , which is equal or smaller than 'x'
UInt NearestPow2(UInt  x); // rounds 'x' to the nearest power    of 2
Bool IsPow2     (UInt  x); // if     'x' is     a       power    of 2
Int  BitLo      (UInt  x); // get index of lowest  non-zero bit  in 'x' (31 if none)
Int  BitLo      (ULong x); // get index of lowest  non-zero bit  in 'x' (63 if none)
Int  BitHi      (UInt  x); // get index of highest non-zero bit  in 'x' ( 0 if none)
Int  BitHi      (ULong x); // get index of highest non-zero bit  in 'x' ( 0 if none)
Int  ByteHi     (UInt  x); // get index of highest non-zero byte in 'x' ( 0 if none)
Int  ByteHi     (ULong x); // get index of highest non-zero byte in 'x' ( 0 if none)
#if EE_PRIVATE
       Int Log2Ceil(UInt  x); // returns Ceil(Log2(x))
       Int Log2Ceil(ULong x); // returns Ceil(Log2(x))
inline Int DivCeil2(UInt  x) {return DivCeil(x, 2u);}
inline Int DivCeil4(UInt  x) {return DivCeil(x, 4u);}
inline Int DivCeil8(UInt  x) {return DivCeil(x, 8u);}

inline UInt CeilGL(UInt x) {return Ceil128(x);} // use 'Ceil128' because of crash when setting/getting data due to internal system memmove which reads ahead

INLINE SByte Signed(Byte   x) {return x;}
INLINE Short Signed(UShort x) {return x;}
INLINE Int   Signed(UInt   x) {return x;}
INLINE Long  Signed(ULong  x) {return x;}

INLINE Byte   Unsigned(Char8 x) {return x;}
INLINE UShort Unsigned(Char  x) {return x;}
INLINE Byte   Unsigned(SByte x) {return x;}
INLINE UShort Unsigned(Short x) {return x;}
INLINE UInt   Unsigned(Int   x) {return x;}
INLINE ULong  Unsigned(Long  x) {return x;}

Byte FltToByteScale (Flt  x);
Byte FltToByteScale2(Flt  x);
Flt  ByteScaleToFlt (Byte x);
Flt  ByteScale2ToFlt(Byte x);

Int   ByteScaleRes(  Int    res, Byte byte_scale);
VecI2 ByteScaleRes(C VecI2 &res, Byte byte_scale);

Int   ByteScale2Res(  Int    res, Byte byte_scale);
VecI2 ByteScale2Res(C VecI2 &res, Byte byte_scale);
#endif

UInt Shl(UInt x, Int i); // safe "x<<i", works ok on negative 'i', and 'i' greater than 32
UInt Shr(UInt x, Int i); // safe "x>>i", works ok on negative 'i', and 'i' greater than 32

UInt Rol(UInt x, Int i); // safe "x ROL i" (Rotate Left ), works ok on negative 'i', and 'i' greater than 32
UInt Ror(UInt x, Int i); // safe "x ROR i" (Rotate Right), works ok on negative 'i', and 'i' greater than 32

void LogConsole(  Bool on=true);                  // open a console window which will include all messages that were passed to 'Log' and 'LogN' functions, if 'on' is set to false then the console window will be closed instead of opened, this function does not prevent from outputting messages to the log file, if you wish to output only to the console the please open the console using 'LogConsole' and clear the log file name using "LogName(S);" (this function works only on Windows)
void LogName   (C Str &name   );   Str LogName(); // set/get name for the log file, default="log.txt" (null on Mobile platforms), specifying an empty file name prevents any writing to the file
void LogDel    (              );                  // delete           the log file
void Log       (C Str &text   );                  // write text              to the log (this will output the message to the log file if 'LogName' is specified, and to the console window if it was opened with 'LogConsole')
void LogN      (C Str &text=S );                  // write text and new-line to the log (this will output the message to the log file if 'LogName' is specified, and to the console window if it was opened with 'LogConsole')
void LogShow   (Bool thread_id, Bool date, Bool time, Bool cur_time); // set which additional information should be displayed during logging, 'thread_id'=ID of the thread which is writing to log, 'date'=year-month-day at the moment of the log, 'time'=hour-minute-second at the moment of the log, 'cur_time'=time in seconds obtained using 'Time.curTime()'

Bool ClipSet(C Str &text, Bool fix_new_line=true); // set system clipboard value to 'text', 'fix_new_line'=if replace "\n" characters with "\r\n" which are required in Windows operating system
Str  ClipGet(             Bool fix_new_line=true); // get system clipboard value          , 'fix_new_line'=if remove  '\r' characters

enum OS_VER // Operating System Version
{
   OS_UNKNOWN,

   WINDOWS_UNKNOWN,
   WINDOWS_2000,
   WINDOWS_XP,
   WINDOWS_XP_64,
   WINDOWS_VISTA,
   WINDOWS_7,
   WINDOWS_8,
   WINDOWS_10,
   WINDOWS_SERVER_2003,
   WINDOWS_SERVER_2003_R2,
   WINDOWS_SERVER_2008,
   WINDOWS_SERVER_2008_R2,
   WINDOWS_SERVER_2012,
   WINDOWS_SERVER_2012_R2,
   WINDOWS_SERVER_2016,

   OS_MAC,

   OS_LINUX,

   ANDROID_UNKNOWN,
   ANDROID_GINGERBREAD,
   ANDROID_HONEYCOMB,
   ANDROID_ICE_CREAM_SANDWICH,
   ANDROID_JELLY_BEAN,
   ANDROID_KIT_KAT,
   ANDROID_LOLLIPOP,
   ANDROID_MARSHMALLOW,
   ANDROID_NOUGAT,
   ANDROID_OREO,
   ANDROID_PIE,

   OS_IOS,
};
VecI4   OSVerNumber(                  ); // get Operating System version number
OS_VER  OSVer      (                  ); // get Operating System version
OS_VER  OSGroup    (OS_VER ver=OSVer()); // get Operating System group, this ignores specific versions and returns just the main groups, such as WINDOWS_UNKNOWN, OS_MAC, OS_LINUX, ANDROID_UNKNOWN, OS_IOS
CChar8* OSName     (OS_VER ver=OSVer()); // get Operating System name
Bool    OSWindows  (OS_VER ver=OSVer()); // if  Operating System is Windows
Bool    OSMac      (OS_VER ver=OSVer()); // if  Operating System is Mac
Bool    OSLinux    (OS_VER ver=OSVer()); // if  Operating System is Linux
Bool    OSAndroid  (OS_VER ver=OSVer()); // if  Operating System is Android
Bool    OSiOS      (OS_VER ver=OSVer()); // if  Operating System is iOS

Str OSUserName(Bool short_name=false); // get the user name of currently logged in user in the Operating System (for Android platform this will be the main email address associated with the device, 'GET_ACCOUNTS' permission which is by default enabled, needs to be specified in the "AndroidManifest.xml" file in order to access this value)

Bool Explore(C Str &name, Bool select=false                                      ); // explore selected 'name' location, 'select'=if explore the parent location instead, and inside it select desired element. This function will only open folders, drives, URL links using the System File Explorer or Browser, it will never run any programs. If 'name' points to a file, then its parent folder will be opened and the file will always be selected regardless of 'select', false on fail.
Bool Run    (C Str &name, C Str &params=S, Bool hidden=false, Bool as_admin=false); // run     selected 'name' command/application/file/folder/drive/URL link, 'as_admin'=if run as administrator, Sample Usage: Run("C:/esenthel.exe"), Run("http://www.esenthel.com"), false on fail

Bool OpenAppSettings(); // open application settings, false on fail, this is used only on iOS and Android, after calling this function the OS will open the Application Settings Screen in the System Settings menu

void    JavaScriptRun (CChar8 *code); // execute custom java script command                                 , valid only for WEB platform
void    JavaScriptRun (CChar  *code); // execute custom java script command                                 , valid only for WEB platform
Int     JavaScriptRunI(CChar8 *code); // execute custom java script command and return its result as integer, valid only for WEB platform
Int     JavaScriptRunI(CChar  *code); // execute custom java script command and return its result as integer, valid only for WEB platform
CChar8* JavaScriptRunS(CChar8 *code); // execute custom java script command and return its result as string , valid only for WEB platform
CChar8* JavaScriptRunS(CChar  *code); // execute custom java script command and return its result as string , valid only for WEB platform

Bool CreateShortcut(C Str &file, C Str &shortcut, C Str &desc=S, C Str &icon=S); // create a shortcut to 'file' program, shortcut will be placed in 'shortcut' location, with 'desc' description, 'icon'=custom icon file path (set empty to use default icon), Sample Usage: CreateShortcut("C:/Games/Esenthel/Esenthel.exe", SystemPath(SP_MENU_PROG)+"/Esenthel/Esenthel", "Esenthel - 3D Action RPG");

Bool AssociateFileType(Str extension, Str application_path, Str application_id, Str extension_desc=S, Str custom_icon=S); // associate file type with specified application, 'extension'=file extension, 'application_path'=path to application, 'application_id'=unique string identyfing the application (recommended format is "CompanyNamy.AppName"), 'extension_desc'=description of the extension (can be left empty), 'custom_icon'=path to icon file which will be used as the icon for the files of specified extension (if left empty, icon will be taken from the application), false on fail, Sample Usage: AssociateFileType("dat", "c:/program.exe", "Company.Program", "Data File")

Str NormalizePath(C Str &path); // return normalized path, Sample Usage: NormalizePath("C:/Folder/../test") -> "C:/test"

Str CleanFileName(C Str &name); // removes all characters which are disallowed in file names, these include \ / : * ? " < > |

Str  EncodeFileName(             CPtr src , Int size); // encode 'src' binary data of 'size' size, into        string which can be used as a file name
void EncodeFileName(  Str &dest, CPtr src , Int size); // encode 'src' binary data of 'size' size, into 'dest' string which can be used as a file name
Bool DecodeFileName(C Str &src ,  Ptr dest, Int size); // decode       binary data                 from 'src'  string, false on fail

T1(TYPE) Str  EncodeFileName(             C TYPE &elm) {return EncodeFileName(      &elm, SIZE(elm));}
T1(TYPE) void EncodeFileName(  Str &dest, C TYPE &elm) {return EncodeFileName(dest, &elm, SIZE(elm));}
T1(TYPE) Bool DecodeFileName(C Str &src ,   TYPE &elm) {return DecodeFileName(src , &elm, SIZE(elm));}
         Bool DecodeFileName(CChar *src ,   UID  &elm);                                     // UID optimized version
  inline Bool DecodeFileName(C Str &src ,   UID  &elm) {return DecodeFileName(src(), elm);} // UID optimized version

UID FileNameID(C Str &name); // convert base of 'name' (obtained using 'GetBase') to ID using 'DecodeFileName', 'UIDZero' on fail, this works like "UID id; DecodeFileName(GetBase(name), id); return id;"

Str  EncodeRaw(              CPtr src , Int size); // encode 'src' binary data of 'size' size, into        string, this is the most efficient encoding (2-bytes per character), warning: string may contain '\0' null characters
void EncodeRaw(  Str  &dest, CPtr src , Int size); // encode 'src' binary data of 'size' size, into 'dest' string, this is the most efficient encoding (2-bytes per character), warning: string may contain '\0' null characters
void EncodeRaw(  Str8 &dest, CPtr src , Int size); // encode 'src' binary data of 'size' size, into 'dest' string, this is the most efficient encoding (2-bytes per character), warning: string may contain '\0' null characters
Bool DecodeRaw(C Str  &src ,  Ptr dest, Int size); // decode       binary data                 from 'src'  string, this is the most efficient encoding (2-bytes per character), false on fail
Bool DecodeRaw(C Str8 &src ,  Ptr dest, Int size); // decode       binary data                 from 'src'  string, this is the most efficient encoding (2-bytes per character), false on fail

T1(TYPE) Str  EncodeRaw(              C TYPE &elm) {return EncodeRaw(      &elm, SIZE(elm));}
T1(TYPE) void EncodeRaw(  Str  &dest, C TYPE &elm) {return EncodeRaw(dest, &elm, SIZE(elm));}
T1(TYPE) void EncodeRaw(  Str8 &dest, C TYPE &elm) {return EncodeRaw(dest, &elm, SIZE(elm));}
T1(TYPE) Bool DecodeRaw(C Str  &src ,   TYPE &elm) {return DecodeRaw(src , &elm, SIZE(elm));}
T1(TYPE) Bool DecodeRaw(C Str8 &src ,   TYPE &elm) {return DecodeRaw(src , &elm, SIZE(elm));}

VecI4 FileVersion(C Str &name); // get exe/dll file version, (-1, -1, -1, -1) on fail

Bool ValidEmail     (C Str &email); // test if 'email' is in correct email       format - "user@domain.com"
Bool ValidURL       (C Str &url  ); // test if 'url'   is in correct url         format - "http://domain.com"
Bool ValidLicenseKey(C Str &key  ); // test if 'key'   is in correct license key format - "XXXXX-XXXXX-XXXXX-XXXXX-XXXXX"

Str         CString(C Str &str); // get string as if it would be stored for the C++ language (preceeding '\' and '"' characters with '\'                                ), for example:         CString("abc"def\ghi"     ) -> ("abc\"def\\ghi")
Str       XmlString(C Str &str); // get string as if it would be stored for the XML text     (replacing   & -> &amp;   < -> &lt;   > -> &gt;   ' -> &apos;   " -> &quot;), for example:       XmlString("abc"def\ghi"     ) -> ("abc&quot;def\ghi")
Str DecodeXmlString(C Str &str); // decode string back from the                 XML text     (replacing   &amp; -> &   &lt; -> <   &gt; -> >   &apos; -> '   &quot; -> "), for example: DecodeXmlString("abc&quot;def\ghi") -> ("abc"def\ghi")

UID   DeviceID          (Bool per_user); // get a unique ID of this device, 'per_user'=if generate a different ID depending on which user is logged in
Str   DeviceManufacturer(); // get Device Manufacturer , available only on Android and Apple
Str   DeviceModel       (); // get Device Model        , available only on Android
Str8  DeviceSerialNumber(); // get Device Serial Number, available only on Android
ULong AndroidID         (); // get Android ID - https://developer.android.com/reference/android/provider/Settings.Secure.html#ANDROID_ID

Str MicrosoftWindowsStoreLink(C Str &app_id); // return a website link to Microsoft Windows Store page for the specified App ID, 'app_id' example = "9NBLGGH4QC8G"
Str         AppleAppStoreLink(C Str &app_id); // return a website link to Apple     App     Store page for the specified App ID, 'app_id' example = "828638910"
Str       GooglePlayStoreLink(C Str &app_id); // return a website link to Google    Play    Store page for the specified App ID, 'app_id' example = "com.esenthel.dm"

void Break(              ); // calling this function will force breakpoint exception, use for debugging
void Exit (C Str &error=S); // immediately exit the application with 'error' message
#if EE_PRIVATE
void ExitNow         (                 ); // exit now without reporting any messages
void ExitEx          (CChar *error=null); // does not require memory allocation for error message, however does not check for call stack
Bool GetCallStack    (Str   &stack     );
Bool GetCallStackFast(Str   &stack     );
void InitMisc        (                 );

// display a message box and always return false
Bool Error     (C Str &msg             );
Bool ErrorDel  (C Str &file            );
Bool ErrorRead (C Str &file            );
Bool ErrorWrite(C Str &file            );
Bool ErrorCopy (C Str &src, C Str &dest);
Bool ErrorMove (C Str &src, C Str &dest);

CChar* _EncodeFileName(C UID &id, Char (&name)[24+1]=ConstCast(TempChar<24+1>()).c); // have to use 'CChar' instead of 'CChar8' because this function is passed to _Cache._find,_get,_require
#endif
/******************************************************************************/
struct ExeSection
{
   enum TYPE : Byte
   {
      CONSTANT     , // section memory data should always be constant                          ,     have the same  data in the process memory as in the exe file
      CONST_PROCESS, // section memory data should        be constant after loading the process, may have different data in the process memory from  the exe file
      VARIABLE     , // section memory data may change
      HASH         , // section used to store the hash
   };

   TYPE  type;
   Char8 name[9];
   UInt  size;
   union
   {
      UIntPtr offset;
         CPtr offset_p;
   };

   Bool contains(UIntPtr offset)C {return offset>=T.offset && offset<T.offset+T.size;} // check if section contains specified offset
   Bool contains(   CPtr offset)C {return                  contains(UIntPtr(offset));} // check if section contains specified offset
};
Bool ParseProcess(             MemPtr<ExeSection> sections); // parse current process and list its sections, false on fail
Bool ParseExe    ( File &f   , MemPtr<ExeSection> sections); // parse EXE file        and list its sections, false on fail
Bool ParseExe    (C Str &name, MemPtr<ExeSection> sections); // parse EXE file        and list its sections, false on fail

  Int         FindSectionNameI  (C MemPtr<ExeSection> &sections, CChar8 *name  ); // find section index by its name                  ,   -1 on fail
  Int         FindSectionOffsetI(C MemPtr<ExeSection> &sections, CPtr    offset); // find section index by offset it should belong to,   -1 on fail
C ExeSection* FindSectionName   (C MemPtr<ExeSection> &sections, CChar8 *name  ); // find section index by its name                  , null on fail
C ExeSection* FindSectionOffset (C MemPtr<ExeSection> &sections, CPtr    offset); // find section index by offset it should belong to, null on fail
/******************************************************************************/
enum SYSTEM_PATH // System Path Type
{
   SP_NONE           , // none
   SP_DESKTOP        , // Desktop             (typically "C:/Users/*/Desktop")
   SP_PROG_FILES     , // Program Files       (typically "C:/Program Files")
   SP_SYSTEM         , // System              (typically "C:/Windows/System32")
   SP_MENU           , // Start Menu          (typically "C:/Users/*/AppData/Roaming/Microsoft/Windows/Start Menu")
   SP_MENU_PROG      , // Start Menu/Programs (typically "C:/Users/*/AppData/Roaming/Microsoft/Windows/Start Menu/Programs")
   SP_STARTUP        , // Start Menu/Startup  (typically "C:/Users/*/AppData/Roaming/Microsoft/Windows/Start Menu/Programs/Startup")
   SP_FAVORITES      , // User/Favorites      (typically "C:/Users/*/Favorites")
   SP_ONE_DRIVE      , // User/One Drive      (typically "C:/Users/*/OneDrive")
   SP_DOCUMENTS      , // User/Documents      (typically "C:/Users/*/Documents"      , for Mobile platforms this is the Application's private folder, where it can save its data that no other App  can access)
   SP_SAVED_GAMES    , // User/Saved Games    (typically "C:/Users/*/Saved Games"    , for Mobile platforms this is the Application's public  folder, or if it's not available, then private folder is selected)
   SP_APP_DATA       , // User/App Data       (typically "C:/Users/*/AppData/Roaming", for Mobile platforms this is the Application's private folder, where it can save its data that no other App  can access)
   SP_APP_DATA_PUBLIC, // User/App Data       (typically "C:/Users/*/AppData/Roaming", for Mobile platforms this is the Application's public  folder, where it can save its data that    other Apps can access)
   SP_ALL_APP_DATA   , // All Users/App Data  (typically "C:/ProgramData")
   SP_PUBLIC         , // for Android this is the path to the public folder which is visible when connected to a computer
   SP_SD_CARD        , // for Android this is the path to SD Card
#if EE_PRIVATE
   SP_FRAMEWORKS     , // User/Frameworks     (typically "/Users/*/Library/Frameworks") used only on Mac
   SP_TRASH          , // User/Trash          (typically "/home/*/.local/share/Trash" ) used only on Linux
#endif
};
Str SystemPath(SYSTEM_PATH type); // get system path, Sample Usage: SystemPath(SP_PROG_FILES) -> "C:/Program Files"

Str AndroidExpansionFileName(Int version, Bool main=true); // get Android Expansion File Name, 'version'=app build version associated with the expansion file, 'main'=if this is the main or patch expansion file
/******************************************************************************/
enum PERMISSION // Permissions
{
   PERMISSION_EXTERNAL_STORAGE, // allow accessing files outside of application folders
   PERMISSION_LOCATION        , // allow accessing device location  using 'Location*' functions
   PERMISSION_SOUND_RECORD    , // allow recording sounds           using 'SoundRecord'
   PERMISSION_USER_NAME       , // allow accessing system user name using 'OSUserName'
#if EE_PRIVATE
   PERMISSION_NUM             , // number of permissions
#endif
};
Bool HasPermission(PERMISSION permission); // check if Application has specified 'permission', this function is only intended for Android, on other platforms it   always returns true
void GetPermission(PERMISSION permission); // request                  specified 'permission', this function is only intended for Android, on other platforms it's always ignored
#if EE_PRIVATE
void RequirePermission(PERMISSION permission); // if(!HasPermission(permission))GetPermission(permission)
#endif
/******************************************************************************/
enum EXTENSION_TYPE // Extension Type
{
   EXT_NONE , // none
   EXT_TEXT , // text
   EXT_IMAGE, // image
   EXT_SOUND, // sound
   EXT_MESH , // mesh
   EXT_VIDEO, // video
};
EXTENSION_TYPE ExtType(C Str &ext); // get extension type from given extension name, Sample Usage : ExtType("bmp") -> EXT_IMAGE

#define SUPPORTED_IMAGE_EXT "bmp|png|jpg|jpeg|webp|tga|tif|tiff|dds|psd|ico|cur|img"
#define SUPPORTED_SOUND_EXT "wav|flac|ogg|opus|weba|webm|mp3|mp4|m4a"
#define SUPPORTED_MESH_EXT  "fbx|dae|ase|obj|3ds|b3d|ms3d|psk|mesh"
/******************************************************************************/
enum REG_KEY_GROUP // registry key group
{
   RKG_CLASSES_ROOT , // this will be translated to HKEY_CLASSES_ROOT
   RKG_CURRENT_USER , // this will be translated to HKEY_CURRENT_USER
   RKG_LOCAL_MACHINE, // this will be translated to HKEY_LOCAL_MACHINE
   RKG_USERS        , // this will be translated to HKEY_USERS
};
enum REG_KEY_TYPE // registry key type
{
   REG_KEY_NONE  ,
   REG_KEY_STRING,
   REG_KEY_U32   ,
   REG_KEY_U64   ,
   REG_KEY_DATA  ,
};
REG_KEY_TYPE GetReg    (REG_KEY_GROUP reg_key_group, C Str &name, Memc<Byte> *data   =null  ); // get registry key type and optionally data, REG_KEY_NONE on fail
Str          GetRegStr (REG_KEY_GROUP reg_key_group, C Str &name, Bool       *success=null  ); // get registry key value as String         , ""           on fail, 'success'=optional parameter which will be set to true if the key exists
UInt         GetRegUInt(REG_KEY_GROUP reg_key_group, C Str &name, Bool       *success=null  ); // get registry key value as UInt           , 0            on fail, 'success'=optional parameter which will be set to true if the key exists
Bool         SetRegStr (REG_KEY_GROUP reg_key_group, C Str &name, C Str      &value         ); // set registry key value as String         , false        on fail
Bool         SetRegUInt(REG_KEY_GROUP reg_key_group, C Str &name, UInt        value         ); // set registry key value as UInt           , false        on fail
Bool         SetRegData(REG_KEY_GROUP reg_key_group, C Str &name, CPtr        data, Int size); // set registry key value as binary data    , false        on fail
#if EE_PRIVATE
#if APPLE
Boolean GetDictionaryBoolean(CFDictionaryRef dict, const void *key);
long    GetDictionaryLong   (CFDictionaryRef dict, const void *key);
#endif
#endif
/******************************************************************************/
struct CyclicUShort
{
   UShort v; // value

   Bool operator==(C CyclicUShort &c)C {return v==c.v;}
   Bool operator!=(C CyclicUShort &c)C {return v!=c.v;}

   Bool operator>=(C CyclicUShort &c)C {return UShort(v-c.v) <USHORT_MAX/2;}
   Bool operator> (C CyclicUShort &c)C {return UShort(c.v-v)>=USHORT_MAX/2;}

   Bool operator<=(C CyclicUShort &c)C {return UShort(c.v-v)< USHORT_MAX/2;}
   Bool operator< (C CyclicUShort &c)C {return UShort(v-c.v)>=USHORT_MAX/2;}

   CyclicUShort& operator++(   ) {++v; return T;}
   void          operator++(int) {++v;}

   void zero() {v=0;}

   CyclicUShort(        )        {}
   CyclicUShort(UShort v) : v(v) {}
};
struct CyclicUInt
{
   UInt v; // value

   Bool operator==(C CyclicUInt &c)C {return v==c.v;}
   Bool operator!=(C CyclicUInt &c)C {return v!=c.v;}

   Bool operator>=(C CyclicUInt &c)C {return UInt(v-c.v) <UINT_MAX/2;}
   Bool operator> (C CyclicUInt &c)C {return UInt(c.v-v)>=UINT_MAX/2;}

   Bool operator<=(C CyclicUInt &c)C {return UInt(c.v-v)< UINT_MAX/2;}
   Bool operator< (C CyclicUInt &c)C {return UInt(v-c.v)>=UINT_MAX/2;}

   CyclicUInt& operator++(   ) {++v; return T;}
   void        operator++(int) {++v;}

   void zero() {v=0;}

   CyclicUInt(      )        {}
   CyclicUInt(UInt v) : v(v) {}
};
/******************************************************************************/
struct IndexWeight
{
   Int index;
   Flt weight;

   void set(Int index, Flt weight) {T.index=index; T.weight=weight;}

   IndexWeight() {}
   IndexWeight(Int index, Flt weight) {set(index, weight);}
};
/******************************************************************************/
struct TextPatch
{
   enum MODE
   {
      ADD  ,
      DEL  ,
      EQUAL,
   };

   struct Diff
   {
      MODE mode;
      Str  text;
   };

   Bool       ok; // if patch succeeded
   Int        base_offset, a_offset,
              base_length, a_length;
   Mems<Diff> diffs;

   Int diffLength()C; // get sum of all ADD/DEL Diff text lengths
};
Str Merge(C Str &base, C Str &a, C Str &b, MemPtr<TextPatch> patches=null, Int timeout=-1); // merge 3 texts, 'base'=original text, 'a'=modification of original text, 'b'=another modification of original text, 'patches'=optional paramter that will receive information about patches applied, 'timeout'=approximatelly how long to search for best merge (-1=unlimited), returns merged text

Int Difference(C Str &a, C Str &b); // get number of characters that need to be added/removed to make both strings equal
/******************************************************************************/
T2(BASE, EXTENDED)   void ASSERT_BASE_EXTENDED_EX(BASE&, EXTENDED&)   {DYNAMIC_ASSERT(static_cast<BASE*>((EXTENDED*)256)==(BASE*)256, "Selected class is not the base class of extended one");}
T2(BASE, EXTENDED)   void ASSERT_BASE_EXTENDED_EX(BASE*, EXTENDED*)   {DYNAMIC_ASSERT(static_cast<BASE*>((EXTENDED*)256)==(BASE*)256, "Selected class is not the base class of extended one");}

T2(BASE, EXTENDED)   void ASSERT_BASE_EXTENDED   (                )   {int i=0; ASSERT_BASE_EXTENDED_EX((BASE&)i, (EXTENDED&)i);} // asserts that 'BASE' class is the main base class of 'EXTENDED', this also works for pointers

T1(TYPE) CPtr CType(       ) {return (CPtr)&typeid(TYPE);} // convert C++ type into pointer
T1(TYPE) CPtr CType(TYPE &x) {return (CPtr)&typeid(x   );} // convert C++ type into pointer

#if EE_PRIVATE
#if WINDOWS
T1(TYPE) Bool IsVirtual(         ) {return std::is_polymorphic<TYPE>();}
T1(TYPE) Bool IsVirtual(C TYPE &x) {return std::is_polymorphic<TYPE>();}
#endif
#endif
/******************************************************************************/
enum LANG_TYPE : Byte
{
#ifndef _WINNT_
   LANG_NEUTRAL      =0x00,
   LANG_INVARIANT    =0x7F,

   LANG_AFRIKAANS    =0x36,
   LANG_ALBANIAN     =0x1C,
   LANG_ALSATIAN     =0x84,
   LANG_AMHARIC      =0x5E,
   LANG_ARABIC       =0x01,
   LANG_ARMENIAN     =0x2B,
   LANG_ASSAMESE     =0x4D,
   LANG_AZERI        =0x2C,
   LANG_BASHKIR      =0x6D,
   LANG_BASQUE       =0x2D,
   LANG_BELARUSIAN   =0x23,
   LANG_BENGALI      =0x45,
   LANG_BRETON       =0x7E,
   LANG_BOSNIAN      =0x1A,
   LANG_BULGARIAN    =0x02,
   LANG_CATALAN      =0x03,
   LANG_CHINESE      =0x04,
   LANG_CORSICAN     =0x83,
   LANG_CROATIAN     =0x1A,
   LANG_CZECH        =0x05,
   LANG_DANISH       =0x06,
   LANG_DARI         =0x8C,
   LANG_DIVEHI       =0x65,
   LANG_DUTCH        =0x13,
   LANG_ENGLISH      =0x09,
   LANG_ESTONIAN     =0x25,
   LANG_FAEROESE     =0x38,
   LANG_FILIPINO     =0x64,
   LANG_FINNISH      =0x0B,
   LANG_FRENCH       =0x0C,
   LANG_FRISIAN      =0x62,
   LANG_GALICIAN     =0x56,
   LANG_GEORGIAN     =0x37,
   LANG_GERMAN       =0x07,
   LANG_GREEK        =0x08,
   LANG_GREENLANDIC  =0x6F,
   LANG_GUJARATI     =0x47,
   LANG_HAUSA        =0x68,
   LANG_HEBREW       =0x0D,
   LANG_HINDI        =0x39,
   LANG_HUNGARIAN    =0x0E,
   LANG_ICELANDIC    =0x0F,
   LANG_IGBO         =0x70,
   LANG_INDONESIAN   =0x21,
   LANG_INUKTITUT    =0x5D,
   LANG_IRISH        =0x3C,
   LANG_ITALIAN      =0x10,
   LANG_JAPANESE     =0x11,
   LANG_KANNADA      =0x4B,
   LANG_KASHMIRI     =0x60,
   LANG_KAZAK        =0x3F,
   LANG_KHMER        =0x53,
   LANG_KICHE        =0x86,
   LANG_KINYARWANDA  =0x87,
   LANG_KONKANI      =0x57,
   LANG_KOREAN       =0x12,
   LANG_KYRGYZ       =0x40,
   LANG_LAO          =0x54,
   LANG_LATVIAN      =0x26,
   LANG_LITHUANIAN   =0x27,
   LANG_LOWER_SORBIAN=0x2E,
   LANG_LUXEMBOURGISH=0x6E,
   LANG_MACEDONIAN   =0x2F,
   LANG_MALAY        =0x3E,
   LANG_MALAYALAM    =0x4C,
   LANG_MALTESE      =0x3A,
   LANG_MANIPURI     =0x58,
   LANG_MAORI        =0x81,
   LANG_MAPUDUNGUN   =0x7A,
   LANG_MARATHI      =0x4E,
   LANG_MOHAWK       =0x7C,
   LANG_MONGOLIAN    =0x50,
   LANG_NEPALI       =0x61,
   LANG_NORWEGIAN    =0x14,
   LANG_OCCITAN      =0x82,
   LANG_ORIYA        =0x48,
   LANG_PASHTO       =0x63,
   LANG_PERSIAN      =0x29,
   LANG_POLISH       =0x15,
   LANG_PORTUGUESE   =0x16,
   LANG_PUNJABI      =0x46,
   LANG_QUECHUA      =0x6B,
   LANG_ROMANIAN     =0x18,
   LANG_ROMANSH      =0x17,
   LANG_RUSSIAN      =0x19,
   LANG_SAMI         =0x3B,
   LANG_SANSKRIT     =0x4F,
   LANG_SERBIAN      =0x1A,
   LANG_SINDHI       =0x59,
   LANG_SINHALESE    =0x5B,
   LANG_SLOVAK       =0x1B,
   LANG_SLOVENIAN    =0x24,
   LANG_SOTHO        =0x6C,
   LANG_SPANISH      =0x0A,
   LANG_SWAHILI      =0x41,
   LANG_SWEDISH      =0x1D,
   LANG_SYRIAC       =0x5A,
   LANG_TAJIK        =0x28,
   LANG_TAMAZIGHT    =0x5F,
   LANG_TAMIL        =0x49,
   LANG_TATAR        =0x44,
   LANG_TELUGU       =0x4A,
   LANG_THAI         =0x1E,
   LANG_TIBETAN      =0x51,
   LANG_TIGRIGNA     =0x73,
   LANG_TSWANA       =0x32,
   LANG_TURKISH      =0x1F,
   LANG_TURKMEN      =0x42,
   LANG_UIGHUR       =0x80,
   LANG_UKRAINIAN    =0x22,
   LANG_UPPER_SORBIAN=0x2E,
   LANG_URDU         =0x20,
   LANG_UZBEK        =0x43,
   LANG_VIETNAMESE   =0x2A,
   LANG_WELSH        =0x52,
   LANG_WOLOF        =0x88,
   LANG_XHOSA        =0x34,
   LANG_YAKUT        =0x85,
   LANG_YI           =0x78,
   LANG_YORUBA       =0x6A,
   LANG_ZULU         =0x35,
#endif
   LANG_UNKNOWN=LANG_NEUTRAL,

   EN=LANG_ENGLISH   ,
   CN=LANG_CHINESE   ,
   JP=LANG_JAPANESE  ,
   KO=LANG_KOREAN    ,
   DE=LANG_GERMAN    ,
   FR=LANG_FRENCH    ,
   PL=LANG_POLISH    ,
   RU=LANG_RUSSIAN   ,
   IT=LANG_ITALIAN   ,
   SP=LANG_SPANISH   ,
   PO=LANG_PORTUGUESE,
   TH=LANG_THAI      ,
};
LANG_TYPE OSLanguage        (              ); // get Operating System Language
LANG_TYPE   LanguageCode    (   C Str &lang); // get language      from language code
CChar8*     LanguageCode    (LANG_TYPE lang); // get language code from language
Str         LanguageSpecific(LANG_TYPE lang); // get specific alphabet characters for the selected language
/******************************************************************************/
Int Compare(C CyclicUShort &a, C CyclicUShort &b);
Int Compare(C CyclicUInt   &a, C CyclicUInt   &b);
/******************************************************************************/
struct Notification
{
   Ptr user; // user data, default=null

   void set(C Str &title, C Str &text, Bool dismissable=true); // set notification parameters, 'dismissable'=if user can dismiss this notification. If this notification already exists, then calling this method will update its properties.

 C Str& title      ()C {return _title      ;} // get title
 C Str& text       ()C {return _text       ;} // get text
   Bool dismissable()C {return _dismissable;} // get if dismissable

   void hide  (); // remove this notification from the status bar
   void remove(); // remove this notification from the status bar and 'Notifications' container, after making this call you may no longer operate on this object as it will point to invalid memory

#if !EE_PRIVATE // make constructors private to prevent from manually creating 'Notification' objects as they should be created only through 'Notifications.New'
private:
#endif
   Bool _dismissable, _visible;
   Str  _title, _text;

  ~Notification();
   Notification();
};
extern Memx<Notification> Notifications; // list of active notifications
#if EE_PRIVATE
void HideNotifications();
#endif
/******************************************************************************/
