/******************************************************************************/
#include "stdafx.h"

#define ZERO_BUFFERS 0 // if zero helper buffers after hash calculation, to clear sensitive information

#include "../../../ThirdPartyLibs/begin.h"

#define XXH_STATIC_LINKING_ONLY
#include "../../../ThirdPartyLibs/LZ4/xxhash.h"

#define SpookyHash _SpookyHash
#include "../../../ThirdPartyLibs/Spooky Hash/SpookyV2.cpp"
#undef SpookyHash

#define MetroHash64  _MetroHash64
#define MetroHash128 _MetroHash128
#include "../../../ThirdPartyLibs/Metro Hash/metrohash64.cpp"
#include "../../../ThirdPartyLibs/Metro Hash/metrohash128.cpp"
#undef MetroHash64
#undef MetroHash128

#include "../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
static const Byte HashPadding[64]=
{
0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
/******************************************************************************/
// CRC32
/******************************************************************************/
static UInt crc32[256];
static UInt CRC32Reflect(UInt reflect, Int bits)
{
   UInt value=0;
   for(Int i=1; i<=bits; i++)
   {
      if(reflect&1)value|=(1<<(bits-i));
      reflect>>=1; 
   }
   return value; 
}
static void CRC32Init()
{
   const UInt polynominal=0x04C11DB7; // this is compatible with Zip, Png - https://en.wikipedia.org/wiki/Cyclic_redundancy_check#Standards_and_common_use
   FREP(256)
   {
      UInt c=(CRC32Reflect(i, 8)<<24);
      FREPD(j, 8)c=((c<<1)^((c&(1<<31)) ? polynominal : 0));
      crc32[i]=CRC32Reflect(c, 32);
   }
}
void CRC32::reset     (                   )  {       _crc=0xFFFFFFFF;}
UInt CRC32::operator()(                   )C {return _crc^0xFFFFFFFF;}
void CRC32::update    (CPtr data, Int size)
{
#if 0 // this version is correct, but it is slower
   UInt *u=(UInt*)data;
   REP(size/4)
   {
     _crc^=*u++;
     _crc=((_crc>>8)^crc32[_crc&0xFF]);
     _crc=((_crc>>8)^crc32[_crc&0xFF]);
     _crc=((_crc>>8)^crc32[_crc&0xFF]);
     _crc=((_crc>>8)^crc32[_crc&0xFF]);
   }
   Byte *b=(Byte*)u;
#else
   Byte *b=(Byte*)data;
   REP(size/4)
   {
     _crc=((_crc>>8)^crc32[(_crc&0xFF) ^ (*b++)]);
     _crc=((_crc>>8)^crc32[(_crc&0xFF) ^ (*b++)]);
     _crc=((_crc>>8)^crc32[(_crc&0xFF) ^ (*b++)]);
     _crc=((_crc>>8)^crc32[(_crc&0xFF) ^ (*b++)]);
   }
#endif
   switch(size%4)
   {
      case 3: _crc=((_crc>>8)^crc32[(_crc&0xFF) ^ (*b++)]); // !! no break on purpose !!
      case 2: _crc=((_crc>>8)^crc32[(_crc&0xFF) ^ (*b++)]); // !! no break on purpose !!
      case 1: _crc=((_crc>>8)^crc32[(_crc&0xFF) ^ (*b++)]); // !! no break on purpose !!
   }
}
/******************************************************************************/
// xxHash
/******************************************************************************/
void xxHash32::reset()
{
  _finalized=false;
   ASSERT(SIZE(_buffer)==SIZE(XXH32_state_t));
   XXH32_reset((XXH32_state_t*)_buffer, 0);
}
void xxHash64::reset()
{
  _finalized=false;
#if IOS || ANDROID
   ASSERT(SIZE(_buffer)>=SIZE(XXH64_state_t)); // needed for iOS and Android
#else
   ASSERT(SIZE(_buffer)==SIZE(XXH64_state_t));
#endif
   XXH64_reset((XXH64_state_t*)_buffer, 0);
}
void xxHash32::update(CPtr data, Int size) {XXH32_update((XXH32_state_t*)_buffer, data, size);}
void xxHash64::update(CPtr data, Int size) {XXH64_update((XXH64_state_t*)_buffer, data, size);}

INLINE void xxHash32::finalize() {if(!_finalized){_finalized=true; _hash=XXH32_digest((XXH32_state_t*)_buffer); if(ZERO_BUFFERS)Zero(_buffer);}}
INLINE void xxHash64::finalize() {if(!_finalized){_finalized=true; _hash=XXH64_digest((XXH64_state_t*)_buffer); if(ZERO_BUFFERS)Zero(_buffer);}}

UInt  xxHash32::operator()() {finalize(); return _hash;}
UInt  xxHash64::hash32    () {finalize(); return _hash;}
ULong xxHash64::hash64    () {finalize(); return _hash;}
/******************************************************************************/
// SPOOKY HASH
/******************************************************************************/
void SpookyHash::reset()
{
  _finalized=false;
   ASSERT(SIZE(_data)==SIZE(_SpookyHash));
   ((_SpookyHash&)_data).Init(0, 0);
}
void SpookyHash::update(CPtr data, Int size) {((_SpookyHash&)_data).Update(data, size);}

INLINE void SpookyHash::finalize()
{
   if(!_finalized)
   {
     _finalized=true;
      ((_SpookyHash&)_data).Final(&_hash.l[0], &_hash.l[1]);
      if(ZERO_BUFFERS)Zero(_data);
   }
}
  UInt  SpookyHash::hash32 () {finalize(); return _hash.i[0];}
  ULong SpookyHash::hash64 () {finalize(); return _hash.l[0];}
C UID&  SpookyHash::hash128() {finalize(); return _hash     ;}
/******************************************************************************/
// METRO HASH
/******************************************************************************/
void MetroHash64 ::reset() {_finalized=false; ASSERT(SIZE(_data)==SIZE(_MetroHash64 )); ((_MetroHash64 &)_data).Initialize();}
void MetroHash128::reset() {_finalized=false; ASSERT(SIZE(_data)==SIZE(_MetroHash128)); ((_MetroHash128&)_data).Initialize();}

void MetroHash64 ::update(CPtr data, Int size) {((_MetroHash64 &)_data).Update((Byte*)data, size);}
void MetroHash128::update(CPtr data, Int size) {((_MetroHash128&)_data).Update((Byte*)data, size);}

  ULong MetroHash64 ::operator()() {if(!_finalized){_finalized=true; ((_MetroHash64 &)_data).Finalize((Byte*)&_hash  ); if(ZERO_BUFFERS)Zero(_data);} return _hash;}
C UID&  MetroHash128::operator()() {if(!_finalized){_finalized=true; ((_MetroHash128&)_data).Finalize(        _hash.b); if(ZERO_BUFFERS)Zero(_data);} return _hash;}
/******************************************************************************/
// MD5
/******************************************************************************/
void MD5::reset()
{
  _finalized=false;

  _count[0]=0;
  _count[1]=0;

  _hash.i[0]=0x67452301;
  _hash.i[1]=0xefcdab89;
  _hash.i[2]=0x98badcfe;
  _hash.i[3]=0x10325476;
}
/******************************************************************************/
static void MD5Decode(UInt output[], const Byte input[], Int len) // decodes input (Byte) into output (UInt). Assumes 'len' is a multiple of 4
{
   for(Int i=0, j=0; j<len; i++, j+=4)
      output[i]=((UInt)input[j]) | (((UInt)input[j+1])<<8) | (((UInt)input[j+2])<<16) | (((UInt)input[j+3])<<24);
}
static void MD5Encode(Byte output[], const UInt input[], Int len) // encodes input (UInt) into output (Byte). Assumes 'len' is a multiple of 4
{
   for(Int i=0, j=0; j<len; i++, j+=4)
   {
      output[j  ]= input[i]     &0xFF;
      output[j+1]=(input[i]>> 8)&0xFF;
      output[j+2]=(input[i]>>16)&0xFF;
      output[j+3]=(input[i]>>24)&0xFF;
   }
}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static INLINE UInt F(UInt x, UInt y, UInt z) {return x&y | ~x&z;}
static INLINE UInt G(UInt x, UInt y, UInt z) {return x&z | y&~z;}
static INLINE UInt H(UInt x, UInt y, UInt z) {return x^y^z;}
static INLINE UInt I(UInt x, UInt y, UInt z) {return y ^ (x | ~z);}
static INLINE UInt ROL(UInt x, int n) {return (x<<n) | (x>>(32-n));}
static INLINE void FF(UInt &a, UInt b, UInt c, UInt d, UInt x, UInt s, UInt ac) {a=ROL(a + F(b,c,d) + x + ac, s) + b;}
static INLINE void GG(UInt &a, UInt b, UInt c, UInt d, UInt x, UInt s, UInt ac) {a=ROL(a + G(b,c,d) + x + ac, s) + b;}
static INLINE void HH(UInt &a, UInt b, UInt c, UInt d, UInt x, UInt s, UInt ac) {a=ROL(a + H(b,c,d) + x + ac, s) + b;}
static INLINE void II(UInt &a, UInt b, UInt c, UInt d, UInt x, UInt s, UInt ac) {a=ROL(a + I(b,c,d) + x + ac, s) + b;}

void MD5::transform(const Byte block[64])
{
   UInt a=_hash.i[0], b=_hash.i[1], c=_hash.i[2], d=_hash.i[3];
#if 1
   UInt *x=(UInt*)block;
#else // big-endian
   UInt x[16]; MD5Decode(x, block, 64);
#endif

   // Round 1
   FF(a, b, c, d, x[ 0], S11, 0xd76aa478); // 1
   FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); // 2
   FF(c, d, a, b, x[ 2], S13, 0x242070db); // 3
   FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); // 4
   FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); // 5
   FF(d, a, b, c, x[ 5], S12, 0x4787c62a); // 6
   FF(c, d, a, b, x[ 6], S13, 0xa8304613); // 7
   FF(b, c, d, a, x[ 7], S14, 0xfd469501); // 8
   FF(a, b, c, d, x[ 8], S11, 0x698098d8); // 9
   FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); // 10
   FF(c, d, a, b, x[10], S13, 0xffff5bb1); // 11
   FF(b, c, d, a, x[11], S14, 0x895cd7be); // 12
   FF(a, b, c, d, x[12], S11, 0x6b901122); // 13
   FF(d, a, b, c, x[13], S12, 0xfd987193); // 14
   FF(c, d, a, b, x[14], S13, 0xa679438e); // 15
   FF(b, c, d, a, x[15], S14, 0x49b40821); // 16

   // Round 2
   GG(a, b, c, d, x[ 1], S21, 0xf61e2562); // 17
   GG(d, a, b, c, x[ 6], S22, 0xc040b340); // 18
   GG(c, d, a, b, x[11], S23, 0x265e5a51); // 19
   GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); // 20
   GG(a, b, c, d, x[ 5], S21, 0xd62f105d); // 21
   GG(d, a, b, c, x[10], S22,  0x2441453); // 22
   GG(c, d, a, b, x[15], S23, 0xd8a1e681); // 23
   GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); // 24
   GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); // 25
   GG(d, a, b, c, x[14], S22, 0xc33707d6); // 26
   GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); // 27
   GG(b, c, d, a, x[ 8], S24, 0x455a14ed); // 28
   GG(a, b, c, d, x[13], S21, 0xa9e3e905); // 29
   GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); // 30
   GG(c, d, a, b, x[ 7], S23, 0x676f02d9); // 31
   GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); // 32

   // Round 3
   HH(a, b, c, d, x[ 5], S31, 0xfffa3942); // 33
   HH(d, a, b, c, x[ 8], S32, 0x8771f681); // 34
   HH(c, d, a, b, x[11], S33, 0x6d9d6122); // 35
   HH(b, c, d, a, x[14], S34, 0xfde5380c); // 36
   HH(a, b, c, d, x[ 1], S31, 0xa4beea44); // 37
   HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); // 38
   HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); // 39
   HH(b, c, d, a, x[10], S34, 0xbebfbc70); // 40
   HH(a, b, c, d, x[13], S31, 0x289b7ec6); // 41
   HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); // 42
   HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); // 43
   HH(b, c, d, a, x[ 6], S34,  0x4881d05); // 44
   HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); // 45
   HH(d, a, b, c, x[12], S32, 0xe6db99e5); // 46
   HH(c, d, a, b, x[15], S33, 0x1fa27cf8); // 47
   HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); // 48

   // Round 4
   II(a, b, c, d, x[ 0], S41, 0xf4292244); // 49
   II(d, a, b, c, x[ 7], S42, 0x432aff97); // 50
   II(c, d, a, b, x[14], S43, 0xab9423a7); // 51
   II(b, c, d, a, x[ 5], S44, 0xfc93a039); // 52
   II(a, b, c, d, x[12], S41, 0x655b59c3); // 53
   II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); // 54
   II(c, d, a, b, x[10], S43, 0xffeff47d); // 55
   II(b, c, d, a, x[ 1], S44, 0x85845dd1); // 56
   II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); // 57
   II(d, a, b, c, x[15], S42, 0xfe2ce6e0); // 58
   II(c, d, a, b, x[ 6], S43, 0xa3014314); // 59
   II(b, c, d, a, x[13], S44, 0x4e0811a1); // 60
   II(a, b, c, d, x[ 4], S41, 0xf7537e82); // 61
   II(d, a, b, c, x[11], S42, 0xbd3af235); // 62
   II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); // 63
   II(b, c, d, a, x[ 9], S44, 0xeb86d391); // 64

  _hash.i[0]+=a;
  _hash.i[1]+=b;
  _hash.i[2]+=c;
  _hash.i[3]+=d;
}
void MD5::update(CPtr data, Int size)
{
   if(size>0)
   {
      // compute number of bytes mod 64
      Int index=_count[0]/8 % 64;

      // Update number of bits
      if((_count[0]+=(size<<3))<(size<<3))_count[1]++;
     _count[1]+=(size>>29);

      // number of bytes we need to fill in buffer
      Int firstpart=64-index;

      Int i;
      if(size>=firstpart) // transform as many times as possible.
      {
         // fill buffer first, transform
         CopyFast(&_buffer[index], data, firstpart);
         transform(_buffer);

         // transform chunks of 64
         for(i=firstpart; i+64<=size; i+=64)transform((Byte*)data+i);

         index=0;
      }else i=0;

      // buffer remaining input
      CopyFast(&_buffer[index], (Byte*)data+i, size-i);
   }
}
INLINE void MD5::finalize()
{
   if(!_finalized)
   {
      // Save number of bits
      Byte bits[8]; MD5Encode(bits, _count, 8);

      // pad out to 56 mod 64
      Int index=_count[0]/8 % 64, padLen=(index<56) ? 56-index : 120-index;
      update(HashPadding, padLen);

      // Append length
      update(bits, 8);

      // clear sensitive information
      if(ZERO_BUFFERS)
      {
         Zero(_buffer);
         Zero(_count );
      }

     _finalized=true;
   }
}
C UID& MD5::operator()()
{
   finalize(); return _hash;
}
/******************************************************************************/
// SHA1-160
/******************************************************************************/
#define ROL(bits, word) (((word)<<(bits)) | ((word)>>(32-(bits))))
#undef C
void SHA1::reset()
{
  _finalized=false;

  _hash.i[0]=0x67452301;
  _hash.i[1]=0xEFCDAB89;
  _hash.i[2]=0x98BADCFE;
  _hash.i[3]=0x10325476;
  _hash.i[4]=0xC3D2E1F0;

  _count[0]=0;
  _count[1]=0;
}
void SHA1::transform(const Byte block[64])
{
   const UInt K[]=
   {
      0x5A827999,
      0x6ED9EBA1,
      0x8F1BBCDC,
      0xCA62C1D6,
   };
   Int  t;             // Loop counter
   UInt temp,          // Temporary word value
        W[80],         // Word sequence
        A, B, C, D, E; // Word buffers

   for(t=0; t<16; t++)
   {
      W[t] =(block[t*4  ]<<24);
      W[t]|=(block[t*4+1]<<16);
      W[t]|=(block[t*4+2]<< 8);
      W[t]|=(block[t*4+3]    );
   }
   for(t=16; t<80; t++)W[t]=ROL(1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);

   A=_hash.i[0];
   B=_hash.i[1];
   C=_hash.i[2];
   D=_hash.i[3];
   E=_hash.i[4];

   for(t=0; t<20; t++)
   {
      temp=ROL(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
      E=D;
      D=C;
      C=ROL(30, B);
      B=A;
      A=temp;
   }

   for(t=20; t<40; t++)
   {
      temp=ROL(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
      E=D;
      D=C;
      C=ROL(30, B);
      B=A;
      A=temp;
   }

   for(t=40; t<60; t++)
   {
      temp=ROL(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
      E=D;
      D=C;
      C=ROL(30, B);
      B=A;
      A=temp;
   }

   for(t=60; t<80; t++)
   {
      temp=ROL(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
      E=D;
      D=C;
      C=ROL(30, B);
      B=A;
      A=temp;
   }

  _hash.i[0]+=A;
  _hash.i[1]+=B;
  _hash.i[2]+=C;
  _hash.i[3]+=D;
  _hash.i[4]+=E;
}
void SHA1::update(CPtr data, Int size)
{
   if(size>0)
   {
      Int left=_count[0]&0x3F,
          fill=64-left;

     _count[0]+=size; if(_count[0]<size)_count[1]++;
      if(left && size>=fill)
      {
         CopyFast(_buffer+left, data, fill);
         transform(_buffer);
         size-= fill;
         data =(Byte*)data + fill;
         left = 0;
      }

      while(size>=64)
      {
         transform((Byte*)data);
         size-=64;
         data =(Byte*)data+64;
      }

      if(size)CopyFast(_buffer+left, data, size);
   }
}
INLINE void SHA1::finalize()
{
   if(!_finalized)
   {
      UInt size[2]=
      {
         (_count[0]>>29)|(_count[1]<<3),
         (_count[0]<< 3),
      };
      UInt last=(_count[0]&0x3F),
           padn=(last<56) ? 56-last : 120-last;

      SwapEndian(size[0]);
      SwapEndian(size[1]);

      update(HashPadding, padn      );
      update(size       , SIZE(size));

      REPA(_hash.i)SwapEndian(_hash.i[i]);

      // clear sensitive information
      if(ZERO_BUFFERS)
      {
         Zero(_buffer);
         Zero(_count );
      }

     _finalized=true;
   }
}
#define C const
C SHA1::Hash& SHA1::operator()()
{
   finalize(); return _hash;
}
#undef ROL
/******************************************************************************/
// SHA2-256
/******************************************************************************/
#undef C
void SHA2::reset()
{
  _finalized=false;

  _hash.i[0]=0x6A09E667;
  _hash.i[1]=0xBB67AE85;
  _hash.i[2]=0x3C6EF372;
  _hash.i[3]=0xA54FF53A;
  _hash.i[4]=0x510E527F;
  _hash.i[5]=0x9B05688C;
  _hash.i[6]=0x1F83D9AB;
  _hash.i[7]=0x5BE0CD19;

  _count[0]=0;
  _count[1]=0;
}
#define GET_UINT32(n, b, i)   \
{                             \
   (n)=((UInt)(b)[(i)  ]<<24) \
      |((UInt)(b)[(i)+1]<<16) \
      |((UInt)(b)[(i)+2]<< 8) \
      |((UInt)(b)[(i)+3]    );\
}
#define PUT_UINT32(n, b, i)   \
{                             \
   (b)[(i)  ]=(Byte)((n)>>24);\
   (b)[(i)+1]=(Byte)((n)>>16);\
   (b)[(i)+2]=(Byte)((n)>> 8);\
   (b)[(i)+3]=(Byte)((n)    );\
}
void SHA2::transform(const Byte data[64])
{
   UInt temp1, temp2;
   UInt W[64];

#if 1
   GET_UINT32(W[0],  data,  0);
   GET_UINT32(W[1],  data,  4);
   GET_UINT32(W[2],  data,  8);
   GET_UINT32(W[3],  data, 12);
   GET_UINT32(W[4],  data, 16);
   GET_UINT32(W[5],  data, 20);
   GET_UINT32(W[6],  data, 24);
   GET_UINT32(W[7],  data, 28);
   GET_UINT32(W[8],  data, 32);
   GET_UINT32(W[9],  data, 36);
   GET_UINT32(W[10], data, 40);
   GET_UINT32(W[11], data, 44);
   GET_UINT32(W[12], data, 48);
   GET_UINT32(W[13], data, 52);
   GET_UINT32(W[14], data, 56);
   GET_UINT32(W[15], data, 60);
#else
   CopyFast(W, data, 64);
#endif

#define  SHR(x,n) ((x&0xFFFFFFFF)>>n)
#define ROTR(x,n) (SHR(x,n) | (x<<(32-n)))

#define S0(x) (ROTR(x, 7) ^ ROTR(x,18) ^  SHR(x, 3))
#define S1(x) (ROTR(x,17) ^ ROTR(x,19) ^  SHR(x,10))
#define S2(x) (ROTR(x, 2) ^ ROTR(x,13) ^ ROTR(x,22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x,11) ^ ROTR(x,25))

#define F0(x,y,z) ((x&y)|(z&(x|y)))
#define F1(x,y,z) (z^(x&(y^z)))

#define R(t)                \
(                           \
   W[t]=S1(W[t- 2])+W[t- 7]+\
        S0(W[t-15])+W[t-16] \
)

#define P(a,b,c,d,e,f,g,h,x,K) \
{                              \
   temp1=h+S3(e)+F1(e,f,g)+K+x;\
   temp2=  S2(a)+F0(a,b,c)    ;\
   d+=temp1; h=temp1+temp2;    \
}

   UInt A=_hash.i[0],
        B=_hash.i[1],
        C=_hash.i[2],
        D=_hash.i[3],
        E=_hash.i[4],
        F=_hash.i[5],
        G=_hash.i[6],
        H=_hash.i[7];

   P(A, B, C, D, E, F, G, H, W[ 0], 0x428A2F98);
   P(H, A, B, C, D, E, F, G, W[ 1], 0x71374491);
   P(G, H, A, B, C, D, E, F, W[ 2], 0xB5C0FBCF);
   P(F, G, H, A, B, C, D, E, W[ 3], 0xE9B5DBA5);
   P(E, F, G, H, A, B, C, D, W[ 4], 0x3956C25B);
   P(D, E, F, G, H, A, B, C, W[ 5], 0x59F111F1);
   P(C, D, E, F, G, H, A, B, W[ 6], 0x923F82A4);
   P(B, C, D, E, F, G, H, A, W[ 7], 0xAB1C5ED5);
   P(A, B, C, D, E, F, G, H, W[ 8], 0xD807AA98);
   P(H, A, B, C, D, E, F, G, W[ 9], 0x12835B01);
   P(G, H, A, B, C, D, E, F, W[10], 0x243185BE);
   P(F, G, H, A, B, C, D, E, W[11], 0x550C7DC3);
   P(E, F, G, H, A, B, C, D, W[12], 0x72BE5D74);
   P(D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE);
   P(C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7);
   P(B, C, D, E, F, G, H, A, W[15], 0xC19BF174);
   P(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1);
   P(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786);
   P(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6);
   P(F, G, H, A, B, C, D, E, R(19), 0x240CA1CC);
   P(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6F);
   P(D, E, F, G, H, A, B, C, R(21), 0x4A7484AA);
   P(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DC);
   P(B, C, D, E, F, G, H, A, R(23), 0x76F988DA);
   P(A, B, C, D, E, F, G, H, R(24), 0x983E5152);
   P(H, A, B, C, D, E, F, G, R(25), 0xA831C66D);
   P(G, H, A, B, C, D, E, F, R(26), 0xB00327C8);
   P(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7);
   P(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3);
   P(D, E, F, G, H, A, B, C, R(29), 0xD5A79147);
   P(C, D, E, F, G, H, A, B, R(30), 0x06CA6351);
   P(B, C, D, E, F, G, H, A, R(31), 0x14292967);
   P(A, B, C, D, E, F, G, H, R(32), 0x27B70A85);
   P(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138);
   P(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFC);
   P(F, G, H, A, B, C, D, E, R(35), 0x53380D13);
   P(E, F, G, H, A, B, C, D, R(36), 0x650A7354);
   P(D, E, F, G, H, A, B, C, R(37), 0x766A0ABB);
   P(C, D, E, F, G, H, A, B, R(38), 0x81C2C92E);
   P(B, C, D, E, F, G, H, A, R(39), 0x92722C85);
   P(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1);
   P(H, A, B, C, D, E, F, G, R(41), 0xA81A664B);
   P(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70);
   P(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3);
   P(E, F, G, H, A, B, C, D, R(44), 0xD192E819);
   P(D, E, F, G, H, A, B, C, R(45), 0xD6990624);
   P(C, D, E, F, G, H, A, B, R(46), 0xF40E3585);
   P(B, C, D, E, F, G, H, A, R(47), 0x106AA070);
   P(A, B, C, D, E, F, G, H, R(48), 0x19A4C116);
   P(H, A, B, C, D, E, F, G, R(49), 0x1E376C08);
   P(G, H, A, B, C, D, E, F, R(50), 0x2748774C);
   P(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5);
   P(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3);
   P(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4A);
   P(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4F);
   P(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3);
   P(A, B, C, D, E, F, G, H, R(56), 0x748F82EE);
   P(H, A, B, C, D, E, F, G, R(57), 0x78A5636F);
   P(G, H, A, B, C, D, E, F, R(58), 0x84C87814);
   P(F, G, H, A, B, C, D, E, R(59), 0x8CC70208);
   P(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFA);
   P(D, E, F, G, H, A, B, C, R(61), 0xA4506CEB);
   P(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7);
   P(B, C, D, E, F, G, H, A, R(63), 0xC67178F2);

  _hash.i[0]+=A;
  _hash.i[1]+=B;
  _hash.i[2]+=C;
  _hash.i[3]+=D;
  _hash.i[4]+=E;
  _hash.i[5]+=F;
  _hash.i[6]+=G;
  _hash.i[7]+=H;
}
void SHA2::update(CPtr data, Int size)
{
   if(size>0)
   {
      Int left=_count[0]&0x3F,
          fill=64-left;

     _count[0]+=size; if(_count[0]<size)_count[1]++;
      if(left && size>=fill)
      {
         CopyFast(_buffer+left, data, fill);
         transform(_buffer);
         size-= fill;
         data =(Byte*)data + fill;
         left = 0;
      }

      while(size>=64)
      {
         transform((Byte*)data);
         size-=64;
         data =(Byte*)data+64;
      }

      if(size)CopyFast(_buffer+left, data, size);
   }
}
INLINE void SHA2::finalize()
{
   if(!_finalized)
   {
      UInt size[2]=
      {
         (_count[0]>>29)|(_count[1]<<3),
         (_count[0]<< 3),
      };
      UInt last=(_count[0]&0x3F),
           padn=(last<56) ? 56-last : 120-last;

      SwapEndian(size[0]);
      SwapEndian(size[1]);

      update(HashPadding, padn      );
      update(size       , SIZE(size));

      REPA(_hash.i)SwapEndian(_hash.i[i]);

      // clear sensitive information
      if(ZERO_BUFFERS)
      {
         Zero(_buffer);
         Zero(_count );
      }

     _finalized=true;
   }
}
#define C const
C SHA2::Hash& SHA2::operator()()
{
   finalize(); return _hash;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
UInt          CRC32Mem(CPtr data, Int size) {CRC32 hash; hash.update(data, size); return hash();}
UInt       xxHash32Mem(CPtr data, Int size) {return XXH32(data, size, 0);}
UInt    xxHash64_32Mem(CPtr data, Int size) {return XXH64(data, size, 0);}
ULong      xxHash64Mem(CPtr data, Int size) {return XXH64(data, size, 0);}
UInt   SpookyHash32Mem(CPtr data, Int size) {return            _SpookyHash  ::Hash32 (data, size, 0);}
ULong  SpookyHash64Mem(CPtr data, Int size) {return            _SpookyHash  ::Hash64 (data, size, 0);}
UID   SpookyHash128Mem(CPtr data, Int size) {UID   hash(0, 0); _SpookyHash  ::Hash128(data, size, &hash.l[0], &hash.l[1]); return hash;} // !! need to initialize 'hash' to zero, because 'Hash128' treats it as seed !!
ULong   MetroHash64Mem(CPtr data, Int size) {ULong hash;       _MetroHash64 ::Hash   ((Byte*)data, size, (Byte*)&hash   ); return hash;}
UID    MetroHash128Mem(CPtr data, Int size) {UID   hash;       _MetroHash128::Hash   ((Byte*)data, size,         hash.b ); return hash;}
UID             MD5Mem(CPtr data, Int size) {MD5   hash; hash.update(data, size); return hash();}
SHA1::Hash     SHA1Mem(CPtr data, Int size) {SHA1  hash; hash.update(data, size); return hash();}
SHA2::Hash     SHA2Mem(CPtr data, Int size) {SHA2  hash; hash.update(data, size); return hash();}
/******************************************************************************/
SHA2::Hash HMAC_SHA2(CPtr key, Int key_size, CPtr data, Int data_size)
{
   SHA2 sha;
   if(key_size>64) // too long keys need to be converted to hash
   {
      sha.update(key, key_size);
      key=&sha(); // store pointer to hash result
      key_size=SIZE(SHA2::Hash);
   }
   Byte i_key_pad[64], o_key_pad[64]; // set 'i_key_pad/o_key_pad' before resetting 'sha', because they may operate on 'key' which points to 'sha.hash' !!
   Int  i=0;
   for(; i<key_size; i++)
   {
      Byte k=((Byte*)key)[i];
      i_key_pad[i]=k^0x36;
      o_key_pad[i]=k^0x5C;
   }
   for(; i<64; i++)
   {
      i_key_pad[i]=0x36;
      o_key_pad[i]=0x5C;
   }

   sha.reset(); // !! reset 'sha' only after 'i_key_pad/o_key_pad' have been set, because they may operate on 'key' which points to 'sha.hash' !!
   sha.update(i_key_pad, SIZE(i_key_pad));
   sha.update(data     , data_size      );
   SHA2::Hash hash=sha();

   sha.reset();
   sha.update(o_key_pad, SIZE(o_key_pad));
   sha.update(&hash    , SIZE(hash     ));
   return sha();
}
/******************************************************************************/
void InitHash()
{
   CRC32Init();
   DEBUG_ASSERT(_MetroHash64 ::ImplementationVerified(), "MetroHash64 fail" );
   DEBUG_ASSERT(_MetroHash128::ImplementationVerified(), "MetroHash128 fail");
}
/******************************************************************************/
}
/******************************************************************************/
