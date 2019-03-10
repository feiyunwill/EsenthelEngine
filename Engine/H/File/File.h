/******************************************************************************

   Use 'File' to handle binary files management.

/******************************************************************************/
#if EE_PRIVATE
enum FILE_TYPE : Byte // File Type
{
   FILE_NONE     , // none
   FILE_STD_READ , // stdio in read  mode, '_handle' is a system handle, can be switched to FILE_STD_WRITE if '_writable'
   FILE_STD_WRITE, // stdio in write mode, '_handle' is a system handle, can be switched to FILE_STD_READ
   FILE_MEM      , // memory, always readable, writable if '_writable'
   FILE_MEMB     , // Memb  , always readable+writable
};
#endif
enum FILE_PATH : Byte // File Path Type
{
   FILE_CUR , // relative to 'CurDir'
   FILE_DATA, // relative to 'DataPath'
#if EE_PRIVATE
   FILE_ANDROID_ASSET, // Android Asset, can be accessed by memory ('_aasset' is "AAsset*") or by stdio ('_handle' is a system handle)
#endif
};
struct File
{
   // manage
#if EE_PRIVATE
   void zeroNoBuf(        );
   void zero     (        );
   void delBuf   (        );
   Bool setBuf   (Int size); // set the buffer to be at least of 'size', false on fail
   void close    (        ); // close the file without releasing helper memory
#endif
   File& del(); // manually delete the file object (this does not delete the file on the disk, it only closes the handle of the C++ file object and releases used memory)

   File& append     (C Str     &name, const_mem_addr Cipher *cipher=null); // append      stdio file, reading is     allowed in this mode, Exit on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), if a file doesn't exist then a new file is created, if a file already exists then that file is opened, its contents are preserved, position           is set to the end
   File& write      (C Str     &name, const_mem_addr Cipher *cipher=null); // write to    stdio file, reading is     allowed in this mode, Exit on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), if a file doesn't exist then a new file is created, if a file already exists then that file is opened, its contents are destroyed, position and size are set to 0
   File& readStd    (C Str     &name, const_mem_addr Cipher *cipher=null); // read        stdio file, writing is not allowed in this mode, Exit on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   File& read       (C Str     &name, const_mem_addr Cipher *cipher=null); // read Pak or stdio file, writing is not allowed in this mode, Exit on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   File& read       (C UID     &id  , const_mem_addr Cipher *cipher=null); // read Pak or stdio file, writing is not allowed in this mode, Exit on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   File& read       (C Str     &name, C Pak    &pak                     ); // read Pak          file, writing is not allowed in this mode, Exit on fail
   File& read       (C UID     &id  , C Pak    &pak                     ); // read Pak          file, writing is not allowed in this mode, Exit on fail
   File& read       (C Str     &name, C PakSet &paks                    ); // read Pak          file, writing is not allowed in this mode, Exit on fail
   File& read       (C UID     &id  , C PakSet &paks                    ); // read Pak          file, writing is not allowed in this mode, Exit on fail
   File& read       (C PakFile &file, C Pak    &pak                     ); // read Pak          file, writing is not allowed in this mode, Exit on fail

   Bool appendTry   (C Str     &name, const_mem_addr Cipher *cipher=null); // try to append      stdio file, reading is     allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), if a file doesn't exist then a new file is created, if a file already exists then that file is opened, its contents are preserved, position           is set to the end
   Bool  writeTry   (C Str     &name, const_mem_addr Cipher *cipher=null); // try to write to    stdio file, reading is     allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), if a file doesn't exist then a new file is created, if a file already exists then that file is opened, its contents are destroyed, position and size are set to 0
   Bool   readStdTry(C Str     &name, const_mem_addr Cipher *cipher=null); // try to read        stdio file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool   readTry   (C Str     &name, const_mem_addr Cipher *cipher=null); // try to read Pak or stdio file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool   readTry   (C UID     &id  , const_mem_addr Cipher *cipher=null); // try to read Pak or stdio file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool   readTry   (C Str     &name, C Pak    &pak                     ); // try to read Pak          file, writing is not allowed in this mode, false on fail
   Bool   readTry   (C UID     &id  , C Pak    &pak                     ); // try to read Pak          file, writing is not allowed in this mode, false on fail
   Bool   readTry   (C Str     &name, C PakSet &paks                    ); // try to read Pak          file, writing is not allowed in this mode, false on fail
   Bool   readTry   (C UID     &id  , C PakSet &paks                    ); // try to read Pak          file, writing is not allowed in this mode, false on fail
   Bool   readTry   (C PakFile &file, C Pak    &pak                     ); // try to read Pak          file, writing is not allowed in this mode, false on fail
#if EE_PRIVATE
#if ANDROID
   Bool   readStdTryEx(C Str     &name,                 const_mem_addr Cipher *cipher, UInt max_buf_size=UINT_MAX, Bool *processed=null); // try to read stdio file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
#else
   Bool   readStdTryEx(C Str     &name,                 const_mem_addr Cipher *cipher, UInt max_buf_size=UINT_MAX); // try to read        stdio file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
#endif
   Bool   readTryEx   (C Str     &name,                 const_mem_addr Cipher *cipher, Bool *processed           ); // try to read Pak or stdio file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'processed'=if original file had to be processed
   Bool   readTryEx   (C UID     &id  ,                 const_mem_addr Cipher *cipher, Bool *processed           ); // try to read Pak or stdio file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'processed'=if original file had to be processed
   Bool   readTryEx   (C Str     &name, C PakSet &paks, const_mem_addr Cipher *cipher, Bool *processed           ); // try to read Pak          file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'processed'=if original file had to be processed
   Bool   readTryEx   (C UID     &id  , C PakSet &paks, const_mem_addr Cipher *cipher, Bool *processed           ); // try to read Pak          file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'processed'=if original file had to be processed
   Bool   readTryEx   (C PakFile &file, C Pak    &pak , const_mem_addr Cipher *cipher, Bool *processed           ); // try to read Pak          file, writing is not allowed in this mode, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'processed'=if original file had to be processed
   Bool   readTryRaw  (C PakFile &file, C Pak    &pak                                                            ); // try to read Pak          file, writing is not allowed in this mode, false on fail, this reads file in raw mode (does not decompress files)
#endif

   File& writeMemFixed( Int size              , const_mem_addr Cipher *cipher=null); // start writing to   fixed     memory file   , reading is     allowed in this mode, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   File& writeMem     (UInt block_elms=64*1024, const_mem_addr Cipher *cipher=null); // start writing to   resizable memory file   , reading is     allowed in this mode, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   File&  readMem     (CPtr data, Int size    , const_mem_addr Cipher *cipher=null); // start reading from fixed     memory address, writing is not allowed in this mode, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)

   // get / set
#if EE_PRIVATE
   Long posFile  ()C;                              // get actual   position in file (takes into account buffering)
   Long posAbs   ()C {return _pos+       _offset;} // get absolute position in file
   Int  posCipher()C {return _pos+_cipher_offset;} // get offset to be used in cipher, can be Int (and not Long) because Cipher operates on Int offset only
   Ptr  memFast  ()  {return (Byte*)_mem+posAbs();}
   Ptr  mem      () ; // get raw memory pointer for FILE_MEM
   UInt memUsage ()C; // get memory usage
   void cipher           (Cipher *cipher);                        // adjust file cipher
   void cipherOffset     (Int     offset);                        // adjust file cipher offset
   void cipherOffsetClear(              ) {cipherOffset(-pos());} // adjust file cipher offset so that "posCipher()==0 -> pos+cipher_offset==0 -> cipher_offset=-pos", this will result in encryption being always the same, regardless of current location
#endif
   Bool is  (        )C {return _type!=0    ;} // if  file is opened
   Bool pos (Long pos);                        // set position, false on fail
   Long pos (        )C {return _pos        ;} // get position
   Long size(        )C {return _size       ;} // get size
   Long left(        )C {return _size-_pos  ;} // get size left (number of bytes from current position to the end of the file)
   Bool end (        )C {return _pos>=_size ;} // if  current position is at the end of the file
   Bool skip(Long n  )  {return  pos(_pos+n);} // skip 'n' bytes going forward
   Bool ok  (        )C {return _ok         ;} // check if no errors occurred during reading/writing. When a new file is opened this will be set to true by default, if any 'put' or 'get' call will fail then this will be set to false

   // put / get
#if EE_PRIVATE
            Int  putReturnSize(CPtr  data, Int size); // write from 'data' memory to file, returns number of bytes written (can be less than 'size'), this method doesn't set 'ok' to false on error
            Int  getReturnSize( Ptr  data, Int size); // read  from file to 'data' memory, returns number of bytes read    (can be less than 'size'), this method doesn't set 'ok' to false on error
            Bool getFast      ( Ptr  data, Int size); // read  from file to 'data' memory, false on fail without zeroing the memory like 'get' does, if error occurs then 'ok' will be set to false
   T1(TYPE) Bool getFast      (TYPE &t             ) {return getFast(&t, SIZE(TYPE)  );} // read raw memory              to 't' object             , if error occurs then 'ok' will be set to false
   T1(TYPE) Bool getFastN     (TYPE *t, Int n      ) {return getFast( t, SIZE(TYPE)*n);} // read raw array to 'n' number of 't' objects            , if error occurs then 'ok' will be set to false
#endif
   Bool get( Ptr data, Int size); // read  from file to 'data' memory, false on fail, if error occurs then 'ok' will be set to false
   Bool put(CPtr data, Int size); // write from 'data' memory to file, false on fail, if error occurs then 'ok' will be set to false

   T1(TYPE) Bool  put       (C TYPE &t       ) {return put(&t, SIZE(TYPE)  );} // write raw memory              of 't' object
   T1(TYPE) Bool  get       (  TYPE &t       ) {return get(&t, SIZE(TYPE)  );} // read  raw memory              to 't' object
   T1(TYPE) Bool  putN      (C TYPE *t, Int n) {return put( t, SIZE(TYPE)*n);} // write raw array of 'n' number of 't' objects
   T1(TYPE) Bool  getN      (  TYPE *t, Int n) {return get( t, SIZE(TYPE)*n);} // read  raw array to 'n' number of 't' objects
   T1(TYPE) File& operator<<(C TYPE &t       ) {       put(t);      return T;} // write raw memory              of 't' object
   T1(TYPE) File& operator>>(  TYPE &t       ) {       get(t);      return T;} // read  raw memory              to 't' object
            File& operator<<(C Str8 &s       ) {return putStr(s);            } // write string
            File& operator<<(C Str  &s       ) {return putStr(s);            } // write string
            File& operator>>(  Str8 &s       ) {return getStr(s);            } // read  string
            File& operator>>(  Str  &s       ) {return getStr(s);            } // read  string

   File& putBool  (  Bool  b) {T<<b; return T;}   Bool  getBool  () {Bool   b; T>>b; return b;} // write/read  Bool
   File& putSByte ( SByte  b) {T<<b; return T;}  SByte  getSByte () {SByte  b; T>>b; return b;} // write/read SByte
   File& putByte  (  Byte  b) {T<<b; return T;}   Byte  getByte  () {Byte   b; T>>b; return b;} // write/read  Byte
   File& putShort (  Short i) {T<<i; return T;}   Short getShort () {Short  i; T>>i; return i;} // write/read  Short
   File& putUShort( UShort u) {T<<u; return T;}  UShort getUShort() {UShort u; T>>u; return u;} // write/read UShort
   File& putInt   (  Int   i) {T<<i; return T;}   Int   getInt   () {Int    i; T>>i; return i;} // write/read  Int
   File& putUInt  ( UInt   u) {T<<u; return T;}  UInt   getUInt  () {UInt   u; T>>u; return u;} // write/read UInt
   File& putLong  (  Long  i) {T<<i; return T;}   Long  getLong  () {Long   i; T>>i; return i;} // write/read  Long
   File& putULong ( ULong  u) {T<<u; return T;}  ULong  getULong () {ULong  u; T>>u; return u;} // write/read ULong
   File& putFlt   (  Flt   f) {T<<f; return T;}   Flt   getFlt   () {Flt    f; T>>f; return f;} // write/read  Float
   File& putDbl   (  Dbl   d) {T<<d; return T;}   Dbl   getDbl   () {Dbl    d; T>>d; return d;} // write/read  Double
   File& putUID   (C UID  &i) {T<<i; return T;}   UID   getUID   () {UID    i; T>>i; return i;} // write/read  UID
#if EE_PRIVATE
    Short getBEShort (); // read BigEndian  Short
   UShort getBEUShort(); // read BigEndian UShort
    Int   getBEInt24 (); // read BigEndian  Int24
   UInt   getBEUInt24(); // read BigEndian UInt24
    Int   getBEInt   (); // read BigEndian  Int
   UInt   getBEUInt  (); // read BigEndian UInt
   Long   getBELong  (); // read BigEndian  Long
  ULong   getBEULong (); // read BigEndian ULong
#endif

   File&  putStr(CChar8 *t); // write string
   File&  putStr(CChar  *t); // write string
   File&  putStr(C Str8 &s); // write string
   File&  putStr(C Str  &s); // write string
   Str    getStr(         ); // read  string, this method can be used to read previously written strings using all 'putStr' methods
   File&  getStr(  Str8 &s); // read  string, this method can be used to read previously written strings using all 'putStr' methods
   File&  getStr(  Str  &s); // read  string, this method can be used to read previously written strings using all 'putStr' methods
   File& skipStr(         ); // skip  string, this method can be used to skip previously written strings using all 'putStr' methods, this method works by reading a string without storing it

                       File& getStr(Char8  *t, Int t_elms);                          // read string into 't' array, 't_elms'=number of elements in the array, this method can be used to read previously written strings using all 'putStr' methods
                       File& getStr(Char   *t, Int t_elms);                          // read string into 't' array, 't_elms'=number of elements in the array, this method can be used to read previously written strings using all 'putStr' methods
   template<Int elms>  File& getStr(Char8 (&t)[elms]     ) {return getStr(t, elms);} // read string into 't' array,                                           this method can be used to read previously written strings using all 'putStr' methods
   template<Int elms>  File& getStr(Char  (&t)[elms]     ) {return getStr(t, elms);} // read string into 't' array,                                           this method can be used to read previously written strings using all 'putStr' methods

   File& putAsset  (CChar *name); // write asset location, this method is optimized for saving asset locations, typically assets are stored with file name based on UID, in such case this method can detect that and save the location using fewer bytes
   File& putAsset  (C UID &id  ); // write asset location, this method is optimized for saving asset locations, typically assets are stored with file name based on UID, in such case this method can detect that and save the location using fewer bytes
   File& getAsset  (  Str &s   ); // read  asset location, this method can be used to read previously written asset location using all 'putAsset' methods, returns ""        on fail
   Str   getAsset  (           ); // read  asset location, this method can be used to read previously written asset location using all 'putAsset' methods, returns ""        on fail
   UID   getAssetID(           ); // read  asset location, this method can be used to read previously written asset location using all 'putAsset' methods, returns 'UIDZero' on fail

   // compress / decompress
   File& cmpIntV   (  Int      i);                                                              //   compress any Int to   variable number of bytes (1..5) depending on the value of the Int (values -64..63 will require only 1 byte, -8192..8191 will require 2 bytes, -1048576..1048575 will require 3 bytes, bigger values will require more bytes)
   File& decIntV   (  Int     &i);   Int     decIntV   () {Int     i; decIntV   (i); return i;} // decompress any Int from variable number of bytes (1..5)

   File& cmpUIntV  ( UInt      u);                                                              //   compress any UInt to   variable number of bytes (1..5) depending on the value of the UInt (values 0..127 will require only 1 byte, 128..16383 will require 2 bytes, 16384..2097151 will require 3 bytes, bigger values will require more bytes)
   File& decUIntV  ( UInt     &u);   UInt    decUIntV  () {UInt    u; decUIntV  (u); return u;} // decompress any UInt from variable number of bytes (1..5)

   File& cmpLongV  (  Long     l);                                                              //   compress any Long to   variable number of bytes (1..9) depending on the value of the Long (values -64..63 will require only 1 byte, -8192..8191 will require 2 bytes, -1048576..1048575 will require 3 bytes, bigger values will require more bytes)
   File& decLongV  (  Long    &l);   Long    decLongV  () {Long    l; decLongV  (l); return l;} // decompress any Long from variable number of bytes (1..9)

#if EE_PRIVATE
   File& cmpULongVMax(ULong u); // compress any ULong to 9 bytes, this is useful if data is expected to be loaded using 'decULongV' and the value was not yet known at the moment when writing data, in which case space could have been allocated using "cmpULongV(-1)" which would use max possible size, after that 'cmpULongVMax' can be called in place of 'cmpULongV' which will fill all bytes to correct value
#endif
   File& cmpULongV ( ULong     u);                                                              //   compress any ULong to   variable number of bytes (1..9) depending on the value of the ULong (values 0..127 will require only 1 byte, 128..16383 will require 2 bytes, 16384..2097151 will require 3 bytes, bigger values will require more bytes)
   File& decULongV ( ULong    &u);   ULong   decULongV () {ULong   u; decULongV (u); return u;} // decompress any ULong from variable number of bytes (1..9)

   File& cmpFlt3cm (C Flt     &r);                                                              //   compress Float in range -83,800 .. 83,800 to   3 bytes with precision of 0.01 (this covers range of -83..83 kilometers with 1 centimeter precision)
   File& decFlt3cm (  Flt     &r);   Flt     decFlt3cm () {Flt     r; decFlt3cm (r); return r;} // decompress Float in range -83,800 .. 83,800 from 3 bytes with precision of 0.01 (this covers range of -83..83 kilometers with 1 centimeter precision)

   File& cmpSatFlt1(C Flt     &r);                                                              //   compress Saturated Float in range 0..1 to   1 byte  (having   256 possible values, and precision of ~0.00400)
   File& cmpSatFlt2(C Flt     &r);                                                              //   compress Saturated Float in range 0..1 to   2 bytes (having 65536 possible values, and precision of ~0.00001)
   File& decSatFlt1(  Flt     &r);   Flt     decSatFlt1() {Flt     r; decSatFlt1(r); return r;} // decompress Saturated Float in range 0..1 from 1 byte
   File& decSatFlt2(  Flt     &r);   Flt     decSatFlt2() {Flt     r; decSatFlt2(r); return r;} // decompress Saturated Float in range 0..1 from 2 bytes

   File& cmpAngle1 (C Flt     &r);                                                              //   compress Angle Float (0..PI2) to   1 byte  (having   256 possible values, and precision of ~0.02000)
   File& cmpAngle2 (C Flt     &r);                                                              //   compress Angle Float (0..PI2) to   2 bytes (having 65536 possible values, and precision of ~0.00009)
   File& decAngle1 (  Flt     &r);   Flt     decAngle1 () {Flt     r; decAngle1 (r); return r;} // decompress Angle Float          from 1 byte
   File& decAngle2 (  Flt     &r);   Flt     decAngle2 () {Flt     r; decAngle2 (r); return r;} // decompress Angle Float          from 2 bytes

   File& cmpDir2   (C Vec     &v);                                                              //   compress Normalized Direction Vector to   2 bytes
   File& cmpDir3   (C Vec     &v);                                                              //   compress Normalized Direction Vector to   3 bytes
   File& decDir2   (  Vec     &v);   Vec     decDir2   () {Vec     v; decDir2   (v); return v;} // decompress Normalized Direction Vector from 2 bytes
   File& decDir3   (  Vec     &v);   Vec     decDir3   () {Vec     v; decDir3   (v); return v;} // decompress Normalized Direction Vector from 3 bytes

   File& cmpOrient2(C Matrix3 &m);                                                              //   compress Orientation to   2 bytes (its scale is ignored, only rotation is saved)
   File& cmpOrient3(C Matrix3 &m);                                                              //   compress Orientation to   3 bytes (its scale is ignored, only rotation is saved)
   File& cmpOrient4(C Matrix3 &m);                                                              //   compress Orientation to   4 bytes (its scale is ignored, only rotation is saved)
   File& decOrient2(  Matrix3 &m);   Matrix3 decOrient2() {Matrix3 m; decOrient2(m); return m;} // decompress Orientation from 2 bytes
   File& decOrient3(  Matrix3 &m);   Matrix3 decOrient3() {Matrix3 m; decOrient3(m); return m;} // decompress Orientation from 3 bytes
   File& decOrient4(  Matrix3 &m);   Matrix3 decOrient4() {Matrix3 m; decOrient4(m); return m;} // decompress Orientation from 4 bytes

   // hashes
   UInt       crc32          (Long max_size=-1); // get file CRC32            from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   UInt           xxHash32   (Long max_size=-1); // get file      xxHash32    from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   UInt           xxHash64_32(Long max_size=-1); // get file      xxHash64-32 from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   ULong          xxHash64   (Long max_size=-1); // get file      xxHash64    from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   UInt       spookyHash32   (Long max_size=-1); // get file Spooky Hash32    from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   ULong      spookyHash64   (Long max_size=-1); // get file Spooky Hash64    from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   UID        spookyHash128  (Long max_size=-1); // get file Spooky Hash128   from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   ULong       metroHash64   (Long max_size=-1); // get file Metro  Hash64    from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   UID         metroHash128  (Long max_size=-1); // get file Metro  Hash128   from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   UID        md5            (Long max_size=-1); // get file MD5              from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   SHA1::Hash sha1           (Long max_size=-1); // get file SHA1             from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)
   SHA2::Hash sha2           (Long max_size=-1); // get file SHA2             from current position to the end, this reads the data and modifies file position, 'max_size'=number of bytes to process (-1=all remaining)

   // operations
   File& reset  (                            );   // reset all written  data to the memory, this method affects only files which have been created using 'writeMem', this method sets file position and size to zero, but keeps the file in mode available for writing new data
   Bool  flush  (                            );   // flush all buffered data to the disk  , false on fail, this method doesn't set 'ok' to false on error
   Bool  flushOK(                            );   // flush all buffered data to the disk and check if no other errors occurred before - "flush() && ok()", false on fail
   Bool  equal  (File &f   , Long max_size=-1);   // if has the same data as 'f' file   (only data from current positions to the end are compared, and not from the start to the end), 'max_size'=number of bytes to compare (-1=all remaining)
   Bool  copy   (File &dest, Long max_size=-1);   // copy file to 'dest', false on fail (only data from current position  to the end are copied  , and not from the start to the end), 'max_size'=number of bytes to copy    (-1=all remaining)
   Bool  size   (Long size);                      // set file size                    , false on fail
   Bool  trim   (         ) {return size(pos());} // set file size to current position, false on fail
#if EE_PRIVATE
   Bool  copy       (File &dest, DataCallback &callback, Long max_size=-1); // copy with callback file to 'dest', false on fail (only data from the current position to the end are copied, and not from the start to the end), 'max_size'=number of bytes to copy (-1=all remaining)
   Bool  copyEncrypt(File &dest, Cipher       &cipher  , Long max_size=-1); // copy and  encrypt  file to 'dest', false on fail (only data from the current position to the end are copied, and not from the start to the end), 'max_size'=number of bytes to copy (-1=all remaining)

   Bool copyToAndDiscard(Mems<Byte> &dest); // copy remaining contents of this file to 'dest' and discard this file data after, false on fail
   Bool       sync();
   Bool   flushDo ();
   void   clearBuf();
   Bool discardBuf(Bool flush);
   void   limit(ULong &total_size, ULong &applied_offset, Long new_size); // temporarily limit current file to '0..new_size' size, current position will be mapped to 0
   void unlimit(ULong &total_size, ULong &applied_offset               ); // unlimit previously limited file

   T2(TA, TB                            )  File& putMulti(C TA &a, C TB &b);
   T2(TA, TB                            )  File& getMulti(  TA &a,   TB &b);
   T3(TA, TB, TC                        )  File& putMulti(C TA &a, C TB &b, C TC &c);
   T3(TA, TB, TC                        )  File& getMulti(  TA &a,   TB &b,   TC &c);
   T4(TA, TB, TC, TD                    )  File& putMulti(C TA &a, C TB &b, C TC &c, C TD &d);
   T4(TA, TB, TC, TD                    )  File& getMulti(  TA &a,   TB &b,   TC &c,   TD &d);
   T5(TA, TB, TC, TD, TE                )  File& putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e);
   T5(TA, TB, TC, TD, TE                )  File& getMulti(  TA &a,   TB &b,   TC &c,   TD &d,   TE &e);
   T6(TA, TB, TC, TD, TE, TF            )  File& putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f);
   T6(TA, TB, TC, TD, TE, TF            )  File& getMulti(  TA &a,   TB &b,   TC &c,   TD &d,   TE &e,   TF &f);
   T7(TA, TB, TC, TD, TE, TF, TG        )  File& putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g);
   T7(TA, TB, TC, TD, TE, TF, TG        )  File& getMulti(  TA &a,   TB &b,   TC &c,   TD &d,   TE &e,   TF &f,   TG &g);
   T8(TA, TB, TC, TD, TE, TF, TG, TH    )  File& putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h);
   T8(TA, TB, TC, TD, TE, TF, TG, TH    )  File& getMulti(  TA &a,   TB &b,   TC &c,   TD &d,   TE &e,   TF &f,   TG &g,   TH &h);
   T9(TA, TB, TC, TD, TE, TF, TG, TH, TI)  File& putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h, C TI &i);
   T9(TA, TB, TC, TD, TE, TF, TG, TH, TI)  File& getMulti(  TA &a,   TB &b,   TC &c,   TD &d,   TE &e,   TF &f,   TG &g,   TH &h,   TI &i);

   // fast but unsafe save/load methods for multiple elements at the same time
   T2(TYPE_FROM, TYPE_TO)  File& putRange(C TYPE_FROM &from, C TYPE_TO &to) {put(&from, UIntPtr(&to)-UIntPtr(&from)+SIZE(to)); return T;}
   T2(TYPE_FROM, TYPE_TO)  File& getRange(  TYPE_FROM &from,   TYPE_TO &to) {get(&from, UIntPtr(&to)-UIntPtr(&from)+SIZE(to)); return T;}
#endif

           ~File() {del();}
            File();
   explicit File(C Str     &name,              const_mem_addr Cipher *cipher=null); // read Pak or stdio file, Exit on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   explicit File(C UID     &id  ,              const_mem_addr Cipher *cipher=null); // read Pak or stdio file, Exit on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   explicit File(C Str     &name, C Pak &pak                                     ); // read Pak          file, Exit on fail
   explicit File(C PakFile &file, C Pak &pak                                     ); // read Pak          file, Exit on fail
   explicit File( CPtr      data,   Int  size, const_mem_addr Cipher *cipher=null); // start reading from memory address (writing is not allowed in this mode), 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)

#if !EE_PRIVATE
private:
#endif
   Byte       _type;
   Bool       _writable, _ok;
   FILE_PATH  _path;
   Int        _buf_pos, _buf_len, _buf_size, _cipher_offset;
   ULong      _offset, _pos, _size;
 C Pak       *_pak;
   Cipher    *_cipher;
   Ptr        _buf;
union
{
   Int        _handle;
   Ptr        _mem;
   Memb<Byte> _memb;
};
#if ANDROID
   Ptr        _aasset;
#endif
   NO_COPY_CONSTRUCTOR(File);
#if EE_PRIVATE
   #define FILE_MEMB_UNION 1 // if 'File._memb' is stored in an union and can be accessed only when 'File._type' matches FILE_MEMB, union will give smaller 'File' class size, no union will allow to reuse previously allocated MEMB memory

   File& _decIntV (  Int  &i); // deprecated - do not use
   Str   _getStr2 (         ); // deprecated - do not use
   File& _getStr2 (  Str  &s); // deprecated - do not use
   File& _getStr2 (  Str8 &s); // deprecated - do not use
   File& _putStr  (C Str8 &s);   File& _putStr(CChar8 *t) {return _putStr(Str8(t));} // write string, deprecated - do not use
   File& _putStr  (C Str  &s);   File& _putStr(CChar  *t) {return _putStr(Str (t));} // write string, deprecated - do not use
   Str   _getStr  (         ); // read  string        , deprecated - do not use
   File& _getStr  (  Str8 &s); // read  string        , deprecated - do not use
   File& _getStr  (  Str  &s); // read  string        , deprecated - do not use
   Str8  _getStr8 (         ); // read  string        , deprecated - do not use, with  8-bit per character
   Str   _getStr16(         ); // read  string        , deprecated - do not use, with 16-bit per character
   File& _putAsset(CChar  *t); // write asset location, deprecated - do not use
   Str   _getAsset(         ); // read  asset location, deprecated - do not use

                       File& _getStr2(Char8  *t, Int t_elms);                            // read string into 't' array, 't_elms'=number of elements in the array, deprecated - do not use
                       File& _getStr2(Char   *t, Int t_elms);                            // read string into 't' array, 't_elms'=number of elements in the array, deprecated - do not use
   template<Int elms>  File& _getStr2(Char8 (&t)[elms]     ) {return _getStr2(t, elms);} // read string into 't' array,                                         , deprecated - do not use
   template<Int elms>  File& _getStr2(Char  (&t)[elms]     ) {return _getStr2(t, elms);} // read string into 't' array,                                         , deprecated - do not use

                       File& _getStr(Char  *t, Int t_elms);                           // read string into 't' array, 't_elms'=number of elements in the array, deprecated - do not use
   template<Int elms>  File& _getStr(Char (&t)[elms]     ) {return _getStr(t, elms);} // read string into 't' array,                                         , deprecated - do not use
#endif
};
/******************************************************************************/
