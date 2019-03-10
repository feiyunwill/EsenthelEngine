/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_GFX CC4('G','F','X',0)
/******************************************************************************/
static IMAGE_TYPE OldImageType1(Byte type)
{
   switch(type)
   {
      default: return IMAGE_NONE;

      case  1: return IMAGE_B8G8R8A8;
      case  2: return IMAGE_R8G8B8A8;
      case  3: return IMAGE_R8G8B8  ;
      case  4: return IMAGE_R8G8    ;
      case  5: return IMAGE_R8      ;

      case  6: return IMAGE_A8  ;
      case  7: return IMAGE_L8  ;
      case  8: return IMAGE_L8A8;

      case  9: return IMAGE_BC2;
      case 10: return IMAGE_BC3;

      case 11: return IMAGE_I8;
      case 12: return IMAGE_I16;
      case 13: return IMAGE_I24;
      case 14: return IMAGE_I32;
      case 15: return IMAGE_F16;
      case 16: return IMAGE_F32;
      case 17: return IMAGE_F16_2;
      case 18: return IMAGE_F32_2;
      case 19: return IMAGE_F16_3;
      case 20: return IMAGE_F32_3;
      case 21: return IMAGE_F16_4;
      case 22: return IMAGE_F32_4;

      case 23: return IMAGE_PVRTC1_2;
      case 24: return IMAGE_PVRTC1_4;

      case 25: return IMAGE_ETC1;
   }
}
static IMAGE_TYPE OldImageType0(Byte type)
{
   switch(type)
   {
      default: return IMAGE_NONE;

      case  1: return IMAGE_B8G8R8A8;
      case  2: return IMAGE_B8G8R8A8; // IMAGE_X8R8G8B8
      case  3: return IMAGE_R8G8B8  ;
      case  4: return IMAGE_NONE    ; // IMAGE_A1R5G5B5
      case  5: return IMAGE_NONE    ; // IMAGE_R5G6B5

      case  6: return IMAGE_NONE; // IMAGE_P8
      case  7: return IMAGE_A8  ;
      case  8: return IMAGE_A8  ; // IMAGE_A8W
      case  9: return IMAGE_L8  ;
      case 10: return IMAGE_I16 ; // IMAGE_L16
      case 11: return IMAGE_NONE; // IMAGE_A4L4
      case 12: return IMAGE_L8A8;

      case 13: return IMAGE_BC2;
      case 14: return IMAGE_BC3;

      case 15: return IMAGE_I8   ;
      case 16: return IMAGE_I16  ;
      case 17: return IMAGE_I24  ;
      case 18: return IMAGE_I32  ;
      case 19: return IMAGE_F16  ;
      case 20: return IMAGE_F32  ;
      case 21: return IMAGE_F16_2;
      case 22: return IMAGE_F32_2;
      case 23: return IMAGE_F16_3;
      case 24: return IMAGE_F32_3;
      case 25: return IMAGE_F16_4;
      case 26: return IMAGE_F32_4;
   }
}
/******************************************************************************/
// SAVE / LOAD
/******************************************************************************/
Bool Image::saveData(File &f)C
{
   if(mode()!=IMAGE_SOFT && mode()!=IMAGE_SOFT_CUBE && mode()!=IMAGE_2D && mode()!=IMAGE_3D && mode()!=IMAGE_CUBE)return false; // verify that mode is correct

   IMAGE_TYPE file_type=T.type(); // set image type as to be stored in the file
   if((file_type==IMAGE_BC7                                                                                      ) && !CompressBC7
   || (file_type==IMAGE_ETC1     || file_type==IMAGE_ETC2 || file_type==IMAGE_ETC2_A1 || file_type==IMAGE_ETC2_A8) && !CompressETC
   || (file_type==IMAGE_PVRTC1_2 || file_type==IMAGE_PVRTC1_4                                                    ) && !CompressPVRTC
   )file_type=T.hwType(); // if compressing to format which isn't supported then store as current 'hwType'

   f.putMulti(Byte(4), size3(), Byte(file_type), Byte(mode()), Byte(mipMaps())); // version

   if(soft() && hwType()==file_type) // software with matching type, we can save without locking
   {
      if(hwSize3()==size3())f.put(softData(), memUsage());else // exact size, then we can save entire memory
      {
       C Byte *data =softData();
         Int   faces=T.faces();
         FREPD(mip, mipMaps()) // iterate all mip maps
         {
            // here no need to use any "Min" because soft HW sizes are guaranteed to be >= file sizes
            Int file_pitch   =ImagePitch  (  w(),   h(), mip, file_type), // use "w(), h()" instead of "hwW(), hwH()" because we want to write only valid pixels
                file_blocks_y=ImageBlocksY(  w(),   h(), mip, file_type), // use "w(), h()" instead of "hwW(), hwH()" because we want to write only valid pixels
               image_pitch   =ImagePitch  (hwW(), hwH(), mip, file_type),
               image_blocks_y=ImageBlocksY(hwW(), hwH(), mip, file_type),
                file_d       =             Max(1,   d()>>mip)           , // use "d()"      instead of "hwD()"        because we want to write only valid pixels
               image_d       =             Max(1, hwD()>>mip)           ,
                write        =  file_blocks_y*file_pitch,
               skip          =(image_blocks_y-file_blocks_y)*image_pitch,
               skip2         =(image_d       -file_d       )*image_pitch*image_blocks_y;
            FREPD(face, faces) // iterate all faces
            {
               FREPD(z, file_d)
               {
                  if(file_pitch==image_pitch) // if file pitch is the same as image pitch
                  {
                     f.put(data, write); data+=write; // we can write both XY in one go
                  }else
                  FREPD(y, file_blocks_y){f.put(data, file_pitch); data+=image_pitch;}
                  data+=skip;
               }
               data+=skip2;
            }
         }
      }
   }else
   {
      Image soft;
      Int   faces=T.faces();
      FREPD(mip, mipMaps()) // iterate all mip maps
      {
         Int file_pitch   =ImagePitch  (w(), h(), mip, file_type), // use "w(), h()" instead of "hwW(), hwH()" because we want to write only valid pixels
             file_blocks_y=ImageBlocksY(w(), h(), mip, file_type); // use "w(), h()" instead of "hwW(), hwH()" because we want to write only valid pixels
         FREPD(face, faces) // iterate all faces
         {
          C Image *src=this;
            Int    src_mip=mip, src_face=face;
            if(hwType()!=file_type){if(!extractMipMap(soft, file_type, IMAGE_SOFT, mip, DIR_ENUM(face)))return false; src=&soft; src_mip=0; src_face=0;} // if 'hwType' is different than of file, then convert to 'file_type' IMAGE_SOFT, after extracting the mip map its Pitch and BlocksY may be different than of calculated from base (for example non-power-of-2 images) so write zeros to file to match the expected size

            if(!src->lockRead(src_mip, DIR_ENUM(src_face)))return false;
            Int write_pitch   =Min(src->pitch()                                            , file_pitch   ),
                write_blocks_y=Min(ImageBlocksY(src->w(), src->h(), src_mip, src->hwType()), file_blocks_y); // use "w(), h()" instead of "hwW(), hwH()" because we want to write only valid pixels
            FREPD(z, src->ld())
            {
             C Byte *src_data_z=src->data() + z*src->pitch2();
               if(file_pitch==src->pitch()) // if file pitch is the same as image pitch !! compare 'src->pitch' and not 'write_pitch' !!
                  f.put(src_data_z, write_blocks_y*write_pitch);else // we can write both XY in one go !! use "write_blocks_y*write_pitch" and not 'pitch2', because 'pitch2' may be bigger !!
               {
                  Int skip=file_pitch-write_pitch;
                  FREPD(y, write_blocks_y){f.put(src_data_z + y*src->pitch(), write_pitch); f.put(null, skip);} // write each line separately
               }
               f.put(null, (file_blocks_y-write_blocks_y)*file_pitch); // unwritten blocksY * pitch
            }
            src->unlock();
         }
      }
   }
   return f.ok();
}
/******************************************************************************/
static INLINE Bool SizeFits (  Int   src,   Int   dest) {return src<dest*2;} // this is OK for src=7, dest=4 (7<4*2), but NOT OK for src=8, dest=4 (8<4*2)
static INLINE Bool SizeFits1(  Int   src,   Int   dest) {return src>1 && SizeFits(src, dest);} // only if 'src>1', if we don't check this, then 1024x1024x1 src will fit into 16x16x1 dest because of Z=1
static        Bool SizeFits (C VecI &src, C VecI &dest) {return SizeFits1(src.x, dest.x) || SizeFits1(src.y, dest.y) || SizeFits1(src.z, dest.z) || (src.x==1 && src.y==1 && src.z==1);}
static Bool Load(Image &image, File &f, C ImageHeader &header, C Str &name, IMAGE_TYPE type_on_fail)
{
   if(!f.ok())return false;

   ImageHeader want=header;
   Int         shrink=0;
   if(Int (*image_load_shrink)(ImageHeader &image_header, C Str &name)=D.image_load_shrink) // copy to temp variable to avoid multi-threading issues
      shrink=image_load_shrink(want, name);

   // shrink
   for(; --shrink>=0 || (IsHW(want.mode) && want.size.max()>D.maxTexSize() && D.maxTexSize()>0); ) // apply 'D.maxTexSize' only for hardware textures (not for software images)
   {
      want.size.x  =Max(1, want.size.x >>1);
      want.size.y  =Max(1, want.size.y >>1);
      want.size.z  =Max(1, want.size.z >>1);
      want.mip_maps=Max(1, want.mip_maps-1);
   }

   const Bool create_from_soft=DX11|GL; // if want to load into SOFT and then create HW from it, to avoid locking 'D._lock', only DX11, GL support this
   if(image.createTryEx(want.size.x, want.size.y, want.size.z, want.type, create_from_soft ? (IsCube(want.mode) ? IMAGE_SOFT_CUBE : IMAGE_SOFT) : want.mode, want.mip_maps, 1, type_on_fail)) // don't use 'want' after this call, instead operate on 'image' members
   {
      const FILTER_TYPE filter=FILTER_BEST;
      Image soft; // store outside the loop to avoid overhead
const Bool   file_cube =IsCube(header.mode), fast_load=(image.soft() && image.hwType()==header.type && image.cube()==file_cube);
const Int    file_faces=(file_cube ? 6 : 1);
      Int   image_mip  =0; // how many mip-maps have already been set in the image
      FREPD( file_mip, header.mip_maps) // iterate all mip maps in the file
      {
         VecI file_mip_size(Max(1, header.size.x>>file_mip), Max(1, header.size.y>>file_mip), Max(1, header.size.z>>file_mip));
         Bool      mip_fits=SizeFits(file_mip_size, image.size3()); // if this mip-map fits into the image
         if(image_mip<image.mipMaps() // if we can set another mip-map, because it fits into 'image' mip map array
         && (mip_fits || file_mip==header.mip_maps-1)) // if fits or this is the last mip-map in the file
         {
            Int  mip_count;
            VecI image_mip_size(Max(1, image.w()>>image_mip), Max(1, image.h()>>image_mip), Max(1, image.d()>>image_mip));
            /*
            Watch out for following cases:
               image.width=497, image.type=BC1
               file.mip0.padded_width=Ceil4(497   )=500
               file.mip1.padded_width=Ceil4(497>>1)=248
              image.mip0.padded_width=Ceil4(497   )=500
              image.mip1.padded_width=Ceil4(500>>1)=252, image mip pitch is calculated based on HW size, so it may be bigger than file mip
            */
            if(fast_load && image_mip_size==file_mip_size) // if this is a software image with same type/cube and mip size matches
            {
               if(image_mip_size.x==Max(1, image.hwW()>>image_mip)
               && image_mip_size.y==Max(1, image.hwH()>>image_mip)
               && image_mip_size.z==Max(1, image.hwD()>>image_mip)) // if image mip size is the same as image HW mip size, then we can read all remaining data in one go
               {
                  mip_count=Min(image.mipMaps()-image_mip, header.mip_maps-file_mip); // how many mip-maps we can read = Min(available in 'image', available in file)
                  f.getFast(image.softData(image_mip), ImageSize(file_mip_size.x, file_mip_size.y, file_mip_size.z, header.type, header.mode, mip_count));
                  file_mip+=mip_count-1; // -1 because 1 is already added at the end of FREPD loop
               }else
               {
                  mip_count=1;
                  // here no need to use any "Min" because soft HW sizes are guaranteed to be >= file sizes
            const Int file_pitch   =ImagePitch  (header.size.x, header.size.y,  file_mip, header.type),
                      file_blocks_y=ImageBlocksY(header.size.x, header.size.y,  file_mip, header.type),
                     image_pitch   =ImagePitch  ( image.hwW() ,  image.hwH() , image_mip, header.type),
                     image_blocks_y=ImageBlocksY( image.hwW() ,  image.hwH() , image_mip, header.type),
                      file_d       =      Max(1, header.size.z>> file_mip),
                     image_d       =      Max(1,  image.hwD() >>image_mip),
                      zero_pitch   = image_pitch   -file_pitch,
                      read         =  file_blocks_y*file_pitch,
                      zero         =(image_blocks_y-file_blocks_y)*image_pitch, // how much to zero = total - what was set
                      zero2        =(image_d       -file_d       )*image_pitch*image_blocks_y;
                  Byte *data=image.softData(image_mip);
                  FREPD(face, file_faces) // iterate all faces
                  {
                     FREPD(z, file_d)
                     {
                        if(file_pitch==image_pitch) // if file pitch is the same as image pitch
                        {  // we can read both XY in one go
                           f.getFast(data, read);
                           data+=read;
                        }else
                        FREPD(y, file_blocks_y) // read each line separately
                        {
                           f.getFast(data, file_pitch);
                           Zero(data+file_pitch, zero_pitch); // zero remaining data to avoid garbage
                           data+=image_pitch;
                        }
                        Zero(data, zero); data+=zero; // zero remaining data to avoid garbage
                     }
                     Zero(data, zero2); data+=zero2; // zero remaining data to avoid garbage
                  }
               }
            }else
            {
               Bool   temp=(image.hwType()!=header.type || file_mip_size!=image_mip_size); // we need to load the mip-map into temporary image first, if the hardware types don't match, or if the mip-map size doesn't match
               Image *dest; Int dest_mip;
               mip_count=1;
               if(temp) // if need to use a temporary image
               {
                  if(!soft.createTry(file_mip_size.x, file_mip_size.y, file_mip_size.z, header.type, IMAGE_SOFT, 1, false))return false;
                  dest=&soft; dest_mip=0;
                  if(!image_mip) // if file is 64x64 but 'image' is 256x256 then we need to write the first file mip map into 256x256, 128x128, 64x64 'image' mip maps, this check is needed only at the start, when we still haven't written any mip-maps
                     REP(image.mipMaps()-1) // -1 because we already have 'mip_count'=1
                  {
                     image_mip_size.set(Max(1, image_mip_size.x>>1), Max(1, image_mip_size.y>>1), Max(1, image_mip_size.z>>1)); // calculate next image mip size
                     if(SizeFits(file_mip_size, image_mip_size))mip_count++;else break; // if file mip still fits into the next image mip size, then increase the counter, else stop
                  }
               }else // we can write directly to 'image'
               {
                  dest=&image; dest_mip=image_mip;
               }

         const Int file_pitch   =ImagePitch  (header.size.x, header.size.y, file_mip,  header.type  ),
                   file_blocks_y=ImageBlocksY(header.size.x, header.size.y, file_mip,  header.type  ),
                   dest_blocks_y=ImageBlocksY(  dest->hwW(),   dest->hwH(), dest_mip, dest->hwType()),
                   read_blocks_y=Min(dest_blocks_y, file_blocks_y),
                   read         = read_blocks_y*file_pitch, // !! use "read_blocks_y*file_pitch" and not 'pitch2', because 'pitch2' may be bigger !!
                   skip         =(file_blocks_y-read_blocks_y)*file_pitch; // unread blocksY * pitch
               FREPD(face, file_faces) // iterate all faces
               {
                  if(!dest->lock(LOCK_WRITE, dest_mip, DIR_ENUM(temp ? 0 : face)))return false;
            const Int   read_pitch=Min(dest->pitch (), file_pitch),
                        zero      =    dest->pitch2()- read_blocks_y*dest->pitch(); // how much to zero = total - what was set
                  Byte *dest_data =    dest->data  ();
                  FREPD(z, dest->ld())
                  {
                     if(file_pitch==dest->pitch()) // if file pitch is the same as image pitch !! compare 'dest->pitch' and not 'read_pitch' !!
                     {  // we can read both XY in one go
                        f.getFast(dest_data, read);
                        dest_data+=read;
                     }else
                     {
                        const Int skip_pitch= file_pitch  -read_pitch, // even though this could be calculated above outside of the loop, most likely this will not be needed because most likely "file_pitch==dest->pitch()" and we can read all in one go
                                  zero_pitch=dest->pitch()-read_pitch;
                        FREPD(y, read_blocks_y) // read each line separately
                        {
                           f.getFast(dest_data, read_pitch); f.skip(skip_pitch);
                           Zero(dest_data+read_pitch, zero_pitch); // zero remaining data to avoid garbage
                           dest_data+=dest->pitch();
                        }
                     }
                     f.skip(skip);
                     Zero(dest_data, zero); // zero remaining data to avoid garbage
                     dest_data+=zero;
                  }
                  dest->unlock();

                  if(temp)REP(mip_count)if(!image.injectMipMap(*dest, image_mip+i, DIR_ENUM(face), filter))return false;
               }
            }
            image_mip+=mip_count;
         }else // skip this mip-map
         {
            f.skip(ImageMipSize(header.size.x, header.size.y, header.size.z, file_mip, header.type)*file_faces);
         }
      }
      if(image_mip)image.updateMipMaps(filter, true, false, false, image_mip-1); // set any missing mip maps, this is needed for example if file had 1 mip map, but we've requested to create more
      else         image.clear(); // or if we didn't load anything, then clear to zero

      if(image.mode()!=want.mode) // if created as SOFT, then convert to HW
      {
         Swap(soft, image); // can't create from self
         if(!image.createTryEx(soft.w(), soft.h(), soft.d(), soft.type(), want.mode, soft.mipMaps(), soft.samples(), IMAGE_NONE, &soft))
         {
            if(!type_on_fail || type_on_fail==soft.type() // no alternative type specified, or is the same one
            || !soft .  copyTry  (soft, -1, -1, -1, type_on_fail)
            || !image.createTryEx(soft.w(), soft.h(), soft.d(), soft.type(), want.mode, soft.mipMaps(), soft.samples(), IMAGE_NONE, &soft))return false;
            image._type=want.type; // adjust wanted type, have to do it here, because there's no other way
         }
      }

      return f.ok();
   }
   return false;
}
Bool Image::loadData(File &f, IMAGE_TYPE type_on_fail, ImageHeader *header, C Str &name)
{
   ImageHeader ih;
   switch(f.decUIntV())
   {
      case 4:
      {
         #pragma pack(push, 1)
         struct ImageHeader4
         {
            VecI       size;
            IMAGE_TYPE type;
            IMAGE_MODE mode;
            Byte       mips;
         }header4;
         #pragma pack(pop)
         f>>header4;
         Unaligned(ih.size    , header4.size);
         Unaligned(ih.type    , header4.type);
         Unaligned(ih.mode    , header4.mode);
        _Unaligned(ih.mip_maps, header4.mips);
         if(header)goto set_header;
         if(Load(T, f, ih, name, type_on_fail))goto ok;
      }break;

      case 3:
      {
         ih.size.x  =f.getInt();
         ih.size.y  =f.getInt();
         ih.size.z  =f.getInt();
         ih.type    =OldImageType1(f.getByte());
         ih.mode    =IMAGE_MODE   (f.getByte());
         ih.mip_maps=f.getByte();
         if(header)goto set_header;
         if(Load(T, f, ih, name, type_on_fail))goto ok;
      }break;

      case 2:
      {
         f.getByte(); // old U16 version part
         ih.size.x  =f.getInt();
         ih.size.y  =f.getInt();
         ih.size.z  =f.getInt();
         Byte byte_pp =f.getByte();
         Byte old_type=f.getByte(); ih.type=OldImageType0(old_type);
         ih.mode    =IMAGE_MODE(f.getByte()); if(ih.mode==IMAGE_CUBE && ih.size.z==6)ih.size.z=1;
         ih.mip_maps=           f.getByte();
         if(header)goto set_header;
         if(old_type==6)f.skip(SIZE(Color)*256); // palette
         if(createTryEx(ih.size.x, ih.size.y, ih.size.z, ih.type, ih.mode, ih.mip_maps, 1, type_on_fail))
         {
            Image soft;
            FREPD(mip ,  ih.mip_maps                 ) // iterate all mip maps
            FREPD(face, (ih.mode==IMAGE_CUBE) ? 6 : 1) // iterate all faces
            {
               Image *dest=this;
               Int    dest_mip=mip, dest_face=face;
               if(hwType()!=ih.type){if(!soft.createTry(Max(1, ih.size.x>>mip), Max(1, ih.size.y>>mip), Max(1, ih.size.z>>mip), ih.type, IMAGE_SOFT, 1, false))return false; dest=&soft; dest_mip=0; dest_face=0;} // if 'hwType' is different than of file, then load into 'file_type' IMAGE_SOFT, after creating the mip map its Pitch and BlocksY may be different than of calculated from base (for example non-power-of-2 images) so skip some data from file to read only created

               if(!dest->lock(LOCK_WRITE, dest_mip, DIR_ENUM(dest_face)))return false;
               Int file_pitch   =ImagePitch  (ih.size.x, ih.size.y, mip, ih.type), dest_pitch   =Min(dest->pitch()                                                   , file_pitch   ),
                   file_blocks_y=ImageBlocksY(ih.size.x, ih.size.y, mip, ih.type), dest_blocks_y=Min(ImageBlocksY(dest->hwW(), dest->hwH(), dest_mip, dest->hwType()), file_blocks_y);
               FREPD(z, dest->ld())
               {
                  FREPD(y, dest_blocks_y){f.getFast(dest->data() + y*dest->pitch() + z*dest->pitch2(), dest_pitch); f.skip(file_pitch-dest_pitch);}
                  f.skip((file_blocks_y-dest_blocks_y)*file_pitch); // unread blocksY * pitch
               }
               if(ih.mode==IMAGE_SOFT && ih.type==IMAGE_R8G8B8){Byte *bgr=dest->data(); REP(dest->lw()*dest->lh()*dest->ld()){Swap(bgr[0], bgr[2]); bgr+=3;}}
               dest->unlock();

               if(hwType()!=ih.type)if(!injectMipMap(*dest, mip, DIR_ENUM(face)))return false;
            }
            if(f.ok())goto ok;
         }
      }break;

      case 1:
      {
         f.getByte(); // old U16 version part
         ih.size.x=f.getInt();
         ih.size.y=f.getInt();
         ih.size.z=f.getInt();
         Byte byte_pp =f.getByte();
         Byte old_type=f.getByte(); ih.type=OldImageType0(old_type);
         ih.mode    =IMAGE_MODE(f.getByte()); if(ih.mode==IMAGE_CUBE && ih.size.z==6)ih.size.z=1;
         ih.mip_maps=           f.getByte();
         if(header)goto set_header;
         if(old_type==6)f.skip(SIZE(Color)*256); // palette
         if(createTryEx(ih.size.x, ih.size.y, ih.size.z, ih.type, ih.mode, ih.mip_maps, 1, type_on_fail))
         {
            Image soft;
            FREPD(mip ,  ih.mip_maps                 ) // iterate all mip maps
            FREPD(face, (ih.mode==IMAGE_CUBE) ? 6 : 1) // iterate all faces
            {
               Image *dest=this;
               Int    dest_mip=mip, dest_face=face;
               if(hwType()!=ih.type){if(!soft.createTry(Max(1, ih.size.x>>mip), Max(1, ih.size.y>>mip), Max(1, ih.size.z>>mip), ih.type, IMAGE_SOFT, 1, false))return false; dest=&soft; dest_mip=0; dest_face=0;} // if 'hwType' is different than of file, then load into 'file_type' IMAGE_SOFT

               if(!dest->lock(LOCK_WRITE, dest_mip, DIR_ENUM(dest_face)))return false;
               Int file_pitch   =dest->lw(),
                   file_blocks_y=dest->lh(); if(ImageTI[ih.type].compressed){file_blocks_y=DivCeil4(file_blocks_y); file_pitch*=4; if(file_pitch<16)file_pitch=16;} file_pitch*=ImageTI[ih.type].bit_pp; file_pitch/=8;
               Int dest_pitch   =Min(dest->pitch()                                                   , file_pitch   ),
                   dest_blocks_y=Min(ImageBlocksY(dest->hwW(), dest->hwH(), dest_mip, dest->hwType()), file_blocks_y);
               FREPD(z, dest->ld())
               {
                  FREPD(y, dest_blocks_y){f.getFast(dest->data() + y*dest->pitch() + z*dest->pitch2(), dest_pitch); f.skip(file_pitch-dest_pitch);}
                  f.skip((file_blocks_y-dest_blocks_y)*file_pitch); // unread blocksY * pitch
               }
               if(ih.mode==IMAGE_SOFT && ih.type==IMAGE_R8G8B8){Byte *bgr=dest->data(); REP(dest->lw()*dest->lh()*dest->ld()){Swap(bgr[0], bgr[2]); bgr+=3;}}
               dest->unlock();

               if(hwType()!=ih.type)if(!injectMipMap(*dest, mip, DIR_ENUM(face)))return false;
            }
            if(f.ok())goto ok;
         }
      }break;

      case 0:
      {
         f.getByte(); // old U16 version part
         ih.size.x=f.getInt();
         ih.size.y=f.getInt();
         ih.size.z=1;
         Byte byte_pp =f.getByte();
         Byte old_type=f.getByte(); ih.type=OldImageType0(old_type);
         ih.mip_maps  =f.getByte();
         ih.mode      =IMAGE_MODE(f.getByte()); if(ih.mode==1)ih.mode=IMAGE_SOFT;else if(ih.mode==0)ih.mode=IMAGE_2D; if(ih.mode==IMAGE_CUBE && ih.size.z==6)ih.size.z=1;
         if(header)goto set_header;
         if(old_type==6)f.skip(SIZE(Color)*256); // palette
         if(createTryEx(ih.size.x, ih.size.y, ih.size.z, ih.type, ih.mode, ih.mip_maps, 1, type_on_fail))
         {
            Image soft;
            FREPD(mip ,  ih.mip_maps                 ) // iterate all mip maps
            FREPD(face, (ih.mode==IMAGE_CUBE) ? 6 : 1) // iterate all faces
            {
               Image *dest=this;
               Int    dest_mip=mip, dest_face=face;
               if(hwType()!=ih.type){if(!soft.createTry(Max(1, ih.size.x>>mip), Max(1, ih.size.y>>mip), Max(1, ih.size.z>>mip), ih.type, IMAGE_SOFT, 1, false))return false; dest=&soft; dest_mip=0; dest_face=0;} // if 'hwType' is different than of file, then load into 'file_type' IMAGE_SOFT

               if(!dest->lock(LOCK_WRITE, dest_mip, DIR_ENUM(dest_face)))return false;
               Int file_pitch   =dest->lw(),
                   file_blocks_y=dest->lh(); if(ImageTI[ih.type].compressed){file_blocks_y=DivCeil4(file_blocks_y); file_pitch*=4; if(file_pitch<16)file_pitch=16;} file_pitch*=ImageTI[ih.type].bit_pp; file_pitch/=8;
               Int dest_pitch   =Min(dest->pitch()                                                   , file_pitch   ),
                   dest_blocks_y=Min(ImageBlocksY(dest->hwW(), dest->hwH(), dest_mip, dest->hwType()), file_blocks_y);
               FREPD(z, dest->ld())
               {
                  FREPD(y, dest_blocks_y){f.getFast(dest->data() + y*dest->pitch() + z*dest->pitch2(), dest_pitch); f.skip(file_pitch-dest_pitch);}
                  f.skip((file_blocks_y-dest_blocks_y)*file_pitch); // unread blocksY * pitch
               }
               if(ih.mode==IMAGE_SOFT && ih.type==IMAGE_R8G8B8){Byte *bgr=dest->data(); REP(dest->lw()*dest->lh()*dest->ld()){Swap(bgr[0], bgr[2]); bgr+=3;}}
               dest->unlock();

               if(hwType()!=ih.type)if(!injectMipMap(*dest, mip, DIR_ENUM(face)))return false;
            }
            if(f.ok())goto ok;
         }
      }break;
   }
error:
   if(header)header->zero();
   del(); return false;

ok:;
   if(App.flag&APP_AUTO_FREE_IMAGE_OPEN_GL_ES_DATA)freeOpenGLESData();
   return true;

set_header:;
   if(f.ok()){*header=ih; return true;}
   goto error;
}
/******************************************************************************/
Bool Image::save(File &f)C
{
   f.putUInt(CC4_GFX);
   return saveData(f);
}
Bool Image::load(File &f)
{
   if(f.getUInt()==CC4_GFX)return loadData(f);
   del(); return false;
}

Bool Image::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Image::load(C Str &name)
{
   File f; if(f.readTry(name) && f.getUInt()==CC4_GFX)return loadData(f, IMAGE_DEFAULT, null, name);
   del(); return false;
}
void Image::operator=(C UID &id  ) {T=_EncodeFileName(id);}
void Image::operator=(C Str &name)
{
   if(!load(name))Exit(MLT(S+"Can't load Image \""         +name+ "\", possible reasons:\n-Video card doesn't support required format\n-File not found\n-Out of memory\n-Engine not yet initialized",
                       PL,S+u"Nie można wczytać Obrazka \""+name+u"\", możliwe przyczyny:\n-Karta graficzna nie obsługuje wczytywanego formatu\n-Nie odnaleziono pliku\n-Skończyła się pamięć\n-Silnik nie został jeszcze zainicjalizowany"));
}
/******************************************************************************/
// IMPORT
/******************************************************************************
Bool Image::ExportDX(C Str &name, GPU_API(D3DXIMAGE_FILEFORMAT, D3DX11_IMAGE_FILE_FORMAT, UInt) format)C
{
#if DX9 || DX11
   if(D.created())
   {
    C Image *src=this;
      Image  temp;
      if(src->cube()){temp.fromCube(*src, -1, IMAGE_SURF_SCRATCH); src=&temp;}

   #if DX9
      if(src->type()==IMAGE_A8)
      {
         Image temp2;
         Bool  ok=false;
         if(!temp2.createTry(src->w(), src->h(), 1, IMAGE_L8, IMAGE_SURF_SCRATCH, 1))return false;
         if(src->lockRead())
         {
            if(temp2.lock(LOCK_WRITE))
            {
               ok=true;
               REPD(y, src->h())
               REPD(x, src->w()){Color c=src->color(x, y); temp2.color(x, y, Color(c.a, c.a, c.a));}
               temp2.unlock();
            }
            src->unlock(); 
         }
         if(!ok)return false;
         Swap(temp, temp2); src=&temp;
      }
      if(!src->_surf)
      {
         if(!src->copyTry(temp, -1, -1, 1, -1, IMAGE_SURF_SCRATCH, 1))return false; src=&temp;
      }

      SyncLocker locker(D._lock);
      return OK(D3DXSaveSurfaceToFile(name, format, src->_surf, null, null));
   #elif DX11
      if(src->type()==IMAGE_A8)
      {
         Image temp2;
         Bool  ok=false;
         if(!temp2.createTry(src->w(), src->h(), 1, IMAGE_R8G8B8A8, IMAGE_SURF_SCRATCH, 1))return false;
         if(src->lockRead())
         {
            if(temp2.lock(LOCK_WRITE))
            {
               ok=true;
               REPD(y, src->h())
               REPD(x, src->w()){Color c=src->color(x, y); temp2.color(x, y, Color(c.a, c.a, c.a));}
               temp2.unlock();
            }
            src->unlock(); 
         }
         if(!ok)return false;
         Swap(temp, temp2); src=&temp;
      }
      if(src->type()!=IMAGE_R8G8B8A8 || !src->_txtr)
      {
         if(!src->copyTry(temp, -1, -1, 1, IMAGE_R8G8B8A8, IMAGE_SURF_SCRATCH, 1))return false; src=&temp;
      }
      SyncLocker locker(D._lock);
      return OK(D3DX11SaveTextureToFile(D3DC, src->_txtr, format, name)); // 'D3DX11SaveTextureToFile' supports only R8G8B8A8
   #endif
   }
#endif
   return false;
}
/******************************************************************************
Bool ImportDX    (C Str &name, Int type=-1, Int mode=-1, Int mip_maps=-1);                             // import using DirectX
Bool Image::ImportDX(C Str &name, Int type, Int mode, Int mip_maps) // 'type, mode, mip_maps' params are only suggestions !!
{
   del();
   if(name.is())
   {
      if(mode<0)mode=IMAGE_SOFT;
      if(mode==IMAGE_SOFT || mode==IMAGE_2D || mode==IMAGE_CUBE || mode==IMAGE_SURF_SCRATCH || mode==IMAGE_SURF_SYSTEM || mode==IMAGE_SURF)
         if(D.created())
      {
      #if DX9 || DX11
         Mems<Byte> file_data;

         Str path=name;
         if(!FullPath(path))
         {
            if(C PaksFile *file=Paks.find(path)){File f; if(f.readTry(*file->file, *file->pak))f.get(file_data.setNum(f.size()).data(), f.size());}else
            if(DataPath().is() && !FExistSystem(path))path=DataPath()+path;
         }

         SyncLockerEx locker(D._lock);
      #if DX9
         D3DXIMAGE_INFO info;

         switch(mode)
         {
            case IMAGE_2D:
               if(file_data.elms() ? OK(D3DXCreateTextureFromFileInMemoryEx(D3D, file_data.data(), file_data.elms(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, (mip_maps<0) ? D3DX_FROM_FILE : mip_maps, 0, ImageTypeToFormat(type), D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 0, &info, null, &_txtr))  // here D3DFMT_UNKNOWN means detect from file but use device compatible if file format is not supported (D3DFMT_FROM_FILE would fail if file format is not supported by device), use 'D3DX_FILTER_NONE' so the image will not get rescaled
                                   : OK(D3DXCreateTextureFromFileEx        (D3D, path                              , D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, (mip_maps<0) ? D3DX_FROM_FILE : mip_maps, 0, ImageTypeToFormat(type), D._no_gpu ? D3DPOOL_SCRATCH : D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 0, &info, null, &_txtr))) // here D3DFMT_UNKNOWN means detect from file but use device compatible if file format is not supported (D3DFMT_FROM_FILE would fail if file format is not supported by device), use 'D3DX_FILTER_NONE' so the image will not get rescaled
               {
                  setInfo(info.Width, info.Height, 0, type, IMAGE_MODE(mode));
                  locker.off();
                  return (T.type()>=IMAGE_TYPES) ? copyTry(T, -1, -1, -1, IMAGE_B8G8R8A8) : true;
               }
            break;

            case IMAGE_SURF_SCRATCH:
            case IMAGE_SURF        :
            case IMAGE_SURF_SYSTEM :
               if(file_data.elms() ? OK(D3DXGetImageInfoFromFileInMemory(file_data.data(), file_data.elms(), &info))
                                   : OK(D3DXGetImageInfoFromFile        (path                              , &info)))
               {
                  if(type<=0)type=ImageFormatToType(info.Format);
                  if(type<=0 || type>=IMAGE_TYPES)type=IMAGE_B8G8R8A8;

                  if(OK(D3D->CreateOffscreenPlainSurface(info.Width, info.Height, ImageTI[type          ].format, (mode==IMAGE_SURF) ? D3DPOOL_DEFAULT : (mode==IMAGE_SURF_SYSTEM) ? D3DPOOL_SYSTEMMEM : D3DPOOL_SCRATCH, &_surf, null))
                  || OK(D3D->CreateOffscreenPlainSurface(info.Width, info.Height, ImageTI[IMAGE_B8G8R8A8].format, (mode==IMAGE_SURF) ? D3DPOOL_DEFAULT : (mode==IMAGE_SURF_SYSTEM) ? D3DPOOL_SYSTEMMEM : D3DPOOL_SCRATCH, &_surf, null))) // try bgra on fail
                  {
                     if(file_data.elms() ? OK(D3DXLoadSurfaceFromFileInMemory(_surf, null, null, file_data.data(), file_data.elms(), null, D3DX_FILTER_NONE, 0, null))  // use 'D3DX_FILTER_NONE' so the image will not get rescaled
                                         : OK(D3DXLoadSurfaceFromFileW       (_surf, null, null, path                              , null, D3DX_FILTER_NONE, 0, null))) // use 'D3DX_FILTER_NONE' so the image will not get rescaled
                     {
                        setInfo(info.Width, info.Height, 0, type, IMAGE_MODE(mode));
                        return true;
                     }
                  }
                  RELEASE(_surf);
               }
            break;

            case IMAGE_SOFT:
               if(file_data.elms() ? OK(D3DXGetImageInfoFromFileInMemory(file_data.data(), file_data.elms(), &info))
                                   : OK(D3DXGetImageInfoFromFile        (path                              , &info)))
               {
                  if(type<=0)type=ImageFormatToType(info.Format);
                  if(type<=0 || type>=IMAGE_TYPES)type=IMAGE_B8G8R8A8;

                  Bool               ok  =false;
                  IDirect3DSurface9 *surf=null;
                  if(OK(D3D->CreateOffscreenPlainSurface(info.Width, info.Height, ImageTI[type          ].format, D3DPOOL_SCRATCH, &surf, null))
                  || OK(D3D->CreateOffscreenPlainSurface(info.Width, info.Height, ImageTI[IMAGE_B8G8R8A8].format, D3DPOOL_SCRATCH, &surf, null))) // try bgra on fail
                  {
                     if(file_data.elms() ? OK(D3DXLoadSurfaceFromFileInMemory(surf, null, null, file_data.data(), file_data.elms(), null, D3DX_FILTER_NONE, 0, null))  // use 'D3DX_FILTER_NONE' so the image will not get rescaled
                                         : OK(D3DXLoadSurfaceFromFileW       (surf, null, null, path                              , null, D3DX_FILTER_NONE, 0, null))) // use 'D3DX_FILTER_NONE' so the image will not get rescaled
                     {
                        D3DLOCKED_RECT  lr;
                        D3DSURFACE_DESC desc;
                        if(OK(surf->GetDesc (&desc)))
                        if(OK(surf->LockRect(&lr, null, 0)))
                        {
                           if(createSoftTry(info.Width, info.Height, 1, ImageFormatToType(desc.Format))) // use format from surface because we're doing raw copy later
                           {
                              ok=true;
                              REPD(y, ImageBlocksY(hwW(), hwH(), 0, hwType()))CopyFast(data() + y*pitch(), (Byte*)lr.pBits + y*lr.Pitch, pitch());
                           }
                           surf->UnlockRect();
                        }
                     }
                  }
                  RELEASE(surf);
                  if(ok)return true;
               }
            break;

            case IMAGE_CUBE:
               if(file_data.elms() ? OK(D3DXGetImageInfoFromFileInMemory(file_data.data(), file_data.elms(), &info))
                                   : OK(D3DXGetImageInfoFromFile        (path                              , &info)))
               {
                  if(type<=0)type=ImageFormatToType(info.Format);
                  if(type<=0 || type>=IMAGE_TYPES)type=IMAGE_B8G8R8A8;

                  if(OK(D3D->CreateOffscreenPlainSurface(info.Width, info.Height, ImageTI[type].compressed ? ImageTI[IMAGE_B8G8R8A8].format : ImageTI[type].format, D3DPOOL_SCRATCH, &_surf, null))
                  || OK(D3D->CreateOffscreenPlainSurface(info.Width, info.Height,                            ImageTI[IMAGE_B8G8R8A8].format                       , D3DPOOL_SCRATCH, &_surf, null))) // try bgra on fail
                  {
                     if(file_data.elms() ? OK(D3DXLoadSurfaceFromFileInMemory(_surf, null, null, file_data.data(), file_data.elms(), null, D3DX_FILTER_NONE, 0, null))  // use 'D3DX_FILTER_NONE' so the image will not get rescaled
                                         : OK(D3DXLoadSurfaceFromFileW       (_surf, null, null, path                              , null, D3DX_FILTER_NONE, 0, null))) // use 'D3DX_FILTER_NONE' so the image will not get rescaled
                     {
                        setInfo(info.Width, info.Height, 0, type, IMAGE_SURF_SCRATCH);
                        locker.off();
                        return toCube(T, -1, type);
                     }
                  }
                  RELEASE(_surf);
               }
            break;
         }
      #elif DX11
         switch(mode)
         {
            case IMAGE_2D:
            {
               D3DX11_IMAGE_LOAD_INFO info;
                              info.Usage    =D3D11_USAGE_DEFAULT;
                              info.BindFlags=D3D11_BIND_SHADER_RESOURCE;
               if(type    > 0)info.Format   =ImageTypeToFormat(type); if(info.Format==DXGI_FORMAT_UNKNOWN)info.Format=DXGI_FORMAT_FROM_FILE;
               if(mip_maps>=0)info.MipLevels=mip_maps;
               if(file_data.elms() ? OK(D3DX11CreateTextureFromMemory(D3D, file_data.data(), file_data.elms(), &info, null, (ID3D11Resource**)&_txtr, null))
                                   : OK(D3DX11CreateTextureFromFile  (D3D, path                              , &info, null, (ID3D11Resource**)&_txtr, null)))
               {
                  setInfo(0, 0, 0, type, IMAGE_MODE(mode));
                  locker.off();
                  return (T.type()>=IMAGE_TYPES) ? copyTry(T, -1, -1, -1, IMAGE_R8G8B8A8) : true;
               }
            }break;

            case IMAGE_SURF        :
            case IMAGE_SURF_SYSTEM :
            case IMAGE_SURF_SCRATCH:
            {
               D3DX11_IMAGE_LOAD_INFO info;
                              info.Usage         =D3D11_USAGE_STAGING;
                              info.BindFlags     =0;
                              info.CpuAccessFlags=D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
               if(type    > 0)info.Format        =ImageTypeToFormat(type); if(info.Format==DXGI_FORMAT_UNKNOWN)info.Format=DXGI_FORMAT_FROM_FILE;
               if(mip_maps>=0)info.MipLevels     =mip_maps;
               if(file_data.elms() ? OK(D3DX11CreateTextureFromMemory(D3D, file_data.data(), file_data.elms(), &info, null, (ID3D11Resource**)&_txtr, null))
                                   : OK(D3DX11CreateTextureFromFile  (D3D, path                              , &info, null, (ID3D11Resource**)&_txtr, null)))
               {
                  setInfo(0, 0, 0, type, IMAGE_MODE(mode));
                  locker.off();
                  return (T.type()>=IMAGE_TYPES) ? copyTry(T, -1, -1, -1, IMAGE_R8G8B8A8) : true;
               }
            }break;

            case IMAGE_SOFT:
            {
               D3DX11_IMAGE_LOAD_INFO info;
                         info.Usage         =D3D11_USAGE_STAGING;
                         info.BindFlags     =0;
                         info.CpuAccessFlags=D3D11_CPU_ACCESS_READ;
               if(type>0)info.Format        =ImageTypeToFormat(type); if(info.Format==DXGI_FORMAT_UNKNOWN)info.Format=DXGI_FORMAT_FROM_FILE;
                         info.MipLevels     =1;
               if(file_data.elms() ? OK(D3DX11CreateTextureFromMemory(D3D, file_data.data(), file_data.elms(), &info, null, (ID3D11Resource**)&_txtr, null))
                                   : OK(D3DX11CreateTextureFromFile  (D3D, path                              , &info, null, (ID3D11Resource**)&_txtr, null)))
               {
                  setInfo(0, 0, 0, type, IMAGE_2D);
                  locker.off();
                  return copyTry(T, -1, -1, -1, (T.type()>=IMAGE_TYPES) ? IMAGE_R8G8B8A8 : -1, IMAGE_SOFT, 1);
               }
            }break;

            case IMAGE_CUBE:
            {
               D3DX11_IMAGE_LOAD_INFO info;
                              info.Usage         =D3D11_USAGE_STAGING;
                              info.BindFlags     =0;
                              info.CpuAccessFlags=D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
               if(type    > 0)info.Format        =ImageTypeToFormat(type); if(info.Format==DXGI_FORMAT_UNKNOWN)info.Format=DXGI_FORMAT_FROM_FILE;
               if(mip_maps>=0)info.MipLevels     =mip_maps;
               if(file_data.elms() ? OK(D3DX11CreateTextureFromMemory(D3D, file_data.data(), file_data.elms(), &info, null, (ID3D11Resource**)&_txtr, null))
                                   : OK(D3DX11CreateTextureFromFile  (D3D, path                              , &info, null, (ID3D11Resource**)&_txtr, null)))
               {
                  setInfo(0, 0, 0, type, IMAGE_SURF_SCRATCH);
                  locker.off();
                  return toCube(T, -1, (T.type()>=IMAGE_TYPES) ? IMAGE_R8G8B8A8 : -1);
               }
            }break;
         }
      #endif
      #endif
      }
      del(); return false;
   }
   return true;
}
/******************************************************************************/
Bool Image::Export(C Str &name, Flt rgb_quality, Flt alpha_quality, Flt compression_level, Int sub_sample)C
{
   CChar   *ext=_GetExt(name);
   if(Equal(ext, "img" ))return save      (name);
   if(Equal(ext, "bmp" ))return ExportBMP (name);
   if(Equal(ext, "png" ))return ExportPNG (name, compression_level);
   if(Equal(ext, "jpg" ))return ExportJPG (name, rgb_quality, sub_sample);
   if(Equal(ext, "webp"))return ExportWEBP(name, rgb_quality, alpha_quality);
   if(Equal(ext, "tga" ))return ExportTGA (name);
   if(Equal(ext, "tif" ))return ExportTIF (name, compression_level);
   if(Equal(ext, "dds" ))return ExportDDS (name);
   if(Equal(ext, "ico" ))return ExportICO (name);
   if(Equal(ext, "icns"))return ExportICNS(name);
                         return false;
}
/******************************************************************************/
Bool Image::ImportTry(File &f, Int type, Int mode, Int mip_maps)
{
   Long pos=f.pos();
               if(load      (f))goto ok;
   f.pos(pos); if(ImportBMP (f))goto ok;
   f.pos(pos); if(ImportPNG (f))goto ok;
   f.pos(pos); if(ImportJPG (f))goto ok;
   f.pos(pos); if(ImportWEBP(f))goto ok;
   f.pos(pos); if(ImportTIF (f))goto ok; // import after PNG/JPG in case LibTIFF tries to decode them too
   f.pos(pos); if(ImportDDS (f, type, mode, mip_maps))goto ok;
   f.pos(pos); if(ImportPSD (f))goto ok;
   f.pos(pos); if(ImportICO (f))goto ok;
   // TGA format doesn't contain any special signatures, so we can't check it
   del(); return false;
ok:;
   return copyTry(T, -1, -1, -1, type, mode, mip_maps);
}
Bool Image::ImportTry(C Str &name, Int type, Int mode, Int mip_maps)
{
   if(!name.is()){del(); return true;}

   File f; if(f.readTry(name))
   {
      if(ImportTry(f, type, mode, mip_maps))return true;
      CChar *ext=_GetExt(name);
      if(Equal(ext, "tga") && f.pos(0) && ImportTGA(f))goto ok; // TGA format doesn't contain any special signatures, so check extension instead
   }
   del(); return false;
ok:;
   return copyTry(T, -1, -1, -1, type, mode, mip_maps);
}
Image& Image::Import(File &f, Int type, Int mode, Int mip_maps)
{
   if(!ImportTry(f, type, mode, mip_maps))Exit(MLT(S+"Can't import image", PL,S+u"Nie można zaimportować obrazka"));
   return T;
}
Image& Image::Import(C Str &name, Int type, Int mode, Int mip_maps)
{
   if(!ImportTry(name, type, mode, mip_maps))Exit(MLT(S+"Can't import image \""+name+'"', PL,S+u"Nie można zaimportować obrazka \""+name+'"'));
   return T;
}
/******************************************************************************/
Bool Image::ImportCubeTry(C Image &right, C Image &left, C Image &up, C Image &down, C Image &forward, C Image &back, Int type, Int mip_maps, Bool resize_to_pow2, FILTER_TYPE filter)
{
   Int size= Max(right  .w(), right  .h(), left.w(), left.h()) ;
   MAX(size, Max(up     .w(), up     .h(), down.w(), down.h()));
   MAX(size, Max(forward.w(), forward.h(), back.w(), back.h()));
   if(createCubeTry(resize_to_pow2 ? NearestPow2(size) : size, IMAGE_TYPE((type<=0) ? right.type() : type), mip_maps))
   {
      injectMipMap(right  , 0, DIR_RIGHT  , filter);
      injectMipMap(left   , 0, DIR_LEFT   , filter);
      injectMipMap(up     , 0, DIR_UP     , filter);
      injectMipMap(down   , 0, DIR_DOWN   , filter);
      injectMipMap(forward, 0, DIR_FORWARD, filter);
      injectMipMap(back   , 0, DIR_BACK   , filter);
      updateMipMaps(); return true;
   }
   del(); return false;
}
Bool Image::ImportCubeTry(C Str &right, C Str &left, C Str &up, C Str &down, C Str &forward, C Str &back, Int type, Int mip_maps, Bool resize_to_pow2, FILTER_TYPE filter)
{
   Image r, l, u, d, f, b;
   if(r.ImportTry(right  , -1, IMAGE_SOFT, 1) // use OR to proceed if at least one was imported
   |  l.ImportTry(left   , -1, IMAGE_SOFT, 1) // use single OR "|" to call import for all images (double OR "||" would continue on first success)
   |  u.ImportTry(up     , -1, IMAGE_SOFT, 1)
   |  d.ImportTry(down   , -1, IMAGE_SOFT, 1)
   |  f.ImportTry(forward, -1, IMAGE_SOFT, 1)
   |  b.ImportTry(back   , -1, IMAGE_SOFT, 1))return ImportCubeTry(r, l, u, d, f, b, type, mip_maps, resize_to_pow2, filter);
   del(); return false;
}
Image& Image::ImportCube(C Str &right, C Str &left, C Str &up, C Str &down, C Str &forward, C Str &back, Int type, Int mip_maps, Bool resize_to_pow2, FILTER_TYPE filter)
{
   if(!ImportCubeTry(right, left, up, down, forward, back, type, mip_maps, resize_to_pow2, filter))Exit(MLT(S+"Can't import images as Cube Texture \""              +right+"\", \""+left+"\", \""+up+"\", \""+down+"\", \""+forward+"\", \""+back+'"',
                                                                                                        PL,S+u"Nie można zaimportować obrazków jako Cube Texture \""+right+"\", \""+left+"\", \""+up+"\", \""+down+"\", \""+forward+"\", \""+back+'"'));
   return T;
}
/******************************************************************************/
Bool ImageLoadHeader(File &f, ImageHeader &header)
{
   Image temp;
   Long  pos=f.pos(); // remember file position
   Bool  ok =(f.getUInt()==CC4_GFX && temp.loadData(f, IMAGE_NONE, &header));
   f.pos(pos); // reset file position
   if(ok)return true;
   header.zero(); return false;
}
Bool ImageLoadHeader(C Str &name, ImageHeader &header)
{
   File f; if(f.readTry(name))return ImageLoadHeader(f, header);
   header.zero(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
