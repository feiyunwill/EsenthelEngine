/******************************************************************************/
#pragma once // needed for android "fake PCH" compilation
#if PHYSX
/******************************************************************************/
struct OutputStream : PxOutputStream
{
   Int   pos, max;
   Byte *data;

	virtual PxU32 write(const void* src, PxU32 count);

   OutputStream() {pos=max=0; data=null;}
  ~OutputStream() {pos=max=0; Free(data);}
};
/******************************************************************************/
struct InputStream : PxInputStream
{
         Int   pos, size;
   const Byte *data;

	virtual PxU32 read(void* dest, PxU32 count);

   InputStream(CPtr data, Int size) {T.pos=0; T.size=size; T.data=(const Byte*)data;}
};
/******************************************************************************/
#endif
/******************************************************************************/
