/******************************************************************************/
#define FBX_LINK_NONE 0
#define FBX_LINK_LIB  1
#define FBX_LINK_DLL  2

#ifndef FBX_LINK_TYPE
   #if WINDOWS
      #define FBX_LINK_TYPE FBX_LINK_DLL
   #elif MAC || LINUX
      #define FBX_LINK_TYPE FBX_LINK_LIB
   #else
      #define FBX_LINK_TYPE FBX_LINK_NONE
   #endif
#endif
/******************************************************************************/
#ifndef _FBX_H
#define _FBX_H

#define FBX_MESH               0x01
#define FBX_SKEL               0x02
#define FBX_ANIM               0x04
#define FBX_MTRL               0x08
#define FBX_PMI                0x10
#define FBX_BONE_NAMES         0x20
#define FBX_ALL_NODES_AS_BONES 0x40

inline Bool SaveFBXData(File &f, C Mesh &mesh, C Skeleton &skeleton, C MemPtr<XAnimation> &animations, C MemPtr<XMaterial> &materials, C MemPtr<Int> &part_material_index, C MemPtr<Str> &bone_names)
{
   f.cmpUIntV(0);
   mesh               .save   (f);
   skeleton           .save   (f);
   animations         .save   (f);
   materials          .save   (f);
   bone_names         .save   (f);
   part_material_index.saveRaw(f);
   return f.ok();
}
inline Bool LoadFBXData(File &f, Mesh &mesh, Skeleton &skeleton, MemPtr<XAnimation> animations, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index, MemPtr<Str> bone_names)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         if(mesh               .load   (f))
         if(skeleton           .load   (f))
         if(animations         .load   (f))
         if(materials          .load   (f))
         if(bone_names         .load   (f))
         if(part_material_index.loadRaw(f))
            if(f.ok())return true;
      }break;
   }
   return false;
}
#endif
/******************************************************************************/
