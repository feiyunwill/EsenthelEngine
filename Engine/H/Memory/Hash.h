/******************************************************************************/
struct CRC32 // Cyclic Redundancy Check
{
   void reset     (                   ) ; // reset status
   void update    (CPtr data, Int size) ; // update with next portion of data
   UInt operator()(                   )C; // get current hash value

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   CRC32() {reset();}

private:
   UInt _crc;
};
/******************************************************************************/
struct xxHash32 // xxHash 32-bit
{
   void reset     (                   ); // reset status
   void update    (CPtr data, Int size); // update with next portion of data
   UInt operator()(                   ); // finalize and get hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   xxHash32() {reset();}

private:
   ULong _buffer[6];
   Bool  _finalized;
   UInt  _hash;
#if EE_PRIVATE
   void finalize();
#endif
};
/******************************************************************************/
struct xxHash64 // xxHash 64-bit
{
   void  reset     (                   );                   // reset status
   void  update    (CPtr data, Int size);                   // update with next portion of data
   UInt  hash32    (                   );                   // finalize and get 32-bit hash result
   ULong hash64    (                   );                   // finalize and get 64-bit hash result
   ULong operator()(                   ) {return hash64();} // finalize and get 64-bit hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   xxHash64() {reset();}

private:
   ULong _buffer[11];
   Bool  _finalized;
   ULong _hash;
#if EE_PRIVATE
   void finalize();
#endif
};
/******************************************************************************/
struct SpookyHash // Spooky Hash 32/64/128-bit
{
   void  reset  (                   ); // reset status
   void  update (CPtr data, Int size); // update with next portion of data
   UInt  hash32 (                   ); // finalize and get  32-bit hash result
   ULong hash64 (                   ); // finalize and get  64-bit hash result
 C UID&  hash128(                   ); // finalize and get 128-bit hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   SpookyHash() {reset();}

private:
   ULong _data[X64 ? 38 : 37];
   Bool  _finalized;
   UID   _hash;
#if EE_PRIVATE
   void finalize();
#endif
};
/******************************************************************************/
struct MetroHash64 // Metro Hash 64-bit
{
   void  reset     (                   ); // reset status
   void  update    (CPtr data, Int size); // update with next portion of data
   ULong operator()(                   ); // finalize and get hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   MetroHash64() {reset();}

private:
   ULong _data[10];
   Bool  _finalized;
   ULong _hash;
};
/******************************************************************************/
struct MetroHash128 // Metro Hash 128-bit
{
   void reset     (                   ); // reset status
   void update    (CPtr data, Int size); // update with next portion of data
 C UID& operator()(                   ); // finalize and get hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   MetroHash128() {reset();}

private:
   ULong _data[9];
   Bool  _finalized;
   UID   _hash;
};
/******************************************************************************/
struct MD5 // Message-Digest Algorithm
{
   void reset     (                   ); // reset status
   void update    (CPtr data, Int size); // update with next portion of data
 C UID& operator()(                   ); // finalize and get hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   MD5() {reset();}

private:
   Byte _buffer[64];
   UInt _count [2];
   Bool _finalized;
   UID  _hash;
#if EE_PRIVATE
   void finalize ();
   void transform(const Byte block[64]);
#endif
};
/******************************************************************************/
struct SHA1 // Secure Hash Algorithm-1, with 160-bits
{
   union Hash
   {
      struct{Byte b[20];};
      struct{UInt i[ 5];};

      void zero() {Zero(T);}
   };

   void  reset     (                   ); // reset status
   void  update    (CPtr data, Int size); // update with next portion of data
 C Hash& operator()(                   ); // finalize and get hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   SHA1() {reset();}

#if !EE_PRIVATE
private:
#endif
   Byte _buffer[64];
   UInt _count [2];
   Bool _finalized;
   Hash _hash;
#if EE_PRIVATE
   void finalize ();
   void transform(const Byte block[64]);
#endif
};
/******************************************************************************/
struct SHA2 // Secure Hash Algorithm-2, with 256-bits
{
   union Hash
   {
      struct{Byte  b[32];};
      struct{UInt  i[ 8];};
      struct{ULong l[ 4];};

      void zero() {Zero(T);}
   };

   void  reset     (                   ); // reset status
   void  update    (CPtr data, Int size); // update with next portion of data
 C Hash& operator()(                   ); // finalize and get hash result

   void update(C Str8 &s) {update(s(), s.length()*SIZE(Char8));}
   void update(C Str  &s) {update(s(), s.length()*SIZE(Char ));}

   SHA2() {reset();}

#if !EE_PRIVATE
private:
#endif
   Byte _buffer[64];
   UInt _count [2];
   Bool _finalized;
   Hash _hash;
#if EE_PRIVATE
   void finalize ();
   void transform(const Byte block[64]);
#endif
};
/******************************************************************************/
UInt          CRC32Mem(CPtr data, Int size); // calculate CRC32         for given memory
UInt       xxHash32Mem(CPtr data, Int size); // calculate xxHash32      for given memory
UInt    xxHash64_32Mem(CPtr data, Int size); // calculate xxHash64-32   for given memory
ULong      xxHash64Mem(CPtr data, Int size); // calculate xxHash64      for given memory
UInt   SpookyHash32Mem(CPtr data, Int size); // calculate SpookyHash32  for given memory
ULong  SpookyHash64Mem(CPtr data, Int size); // calculate SpookyHash64  for given memory
UID   SpookyHash128Mem(CPtr data, Int size); // calculate SpookyHash128 for given memory
ULong   MetroHash64Mem(CPtr data, Int size); // calculate MetroHash64   for given memory
UID    MetroHash128Mem(CPtr data, Int size); // calculate MetroHash128  for given memory
UID             MD5Mem(CPtr data, Int size); // calculate MD5           for given memory
SHA1::Hash     SHA1Mem(CPtr data, Int size); // calculate SHA1          for given memory
SHA2::Hash     SHA2Mem(CPtr data, Int size); // calculate SHA2          for given memory

SHA2::Hash HMAC_SHA2(CPtr key, Int key_size, CPtr data, Int data_size); // calculate HMAC-SHA2 for given key and data
/******************************************************************************/
#if EE_PRIVATE
void InitHash();
#endif
/******************************************************************************/
