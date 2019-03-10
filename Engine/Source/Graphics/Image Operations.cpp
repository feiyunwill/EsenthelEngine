/******************************************************************************/
#include "stdafx.h"
namespace EE{
#include "Import/BC.h"
#include "Import/ETC.h"
/******************************************************************************/
static Bool Decompress(Image &image, IMAGE_TYPE &type, IMAGE_MODE &mode, Int &mip_maps) // returns if image exists
{
   type    =image.type   ();
   mode    =image.mode   ();
   mip_maps=image.mipMaps();
   if(image.is())
   {
      if(image.compressed())return image.copyTry(image, -1, -1, -1, IMAGE_R8G8B8A8, image.cube() ? IMAGE_SOFT_CUBE : IMAGE_SOFT, 1);
      return true;
   }
   return false;
}
static void Compress(Image &image, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps)
{
   image.copyTry(image, -1, -1, -1, type, mode, mip_maps);
}
/******************************************************************************/
Bool Image::extractMipMap(Image &dest, Int type, Int mode, Int mip_map, DIR_ENUM cube_face)C
{
   Bool  ok=false;
   Image temp;
   if(InRange(mip_map, mipMaps()))
   {
      if(type<=0                 )type=T.type();
      if(mode< 0                 )mode=T.mode();
      if(IsCube(IMAGE_MODE(mode)))mode=IMAGE_SOFT;

      if(temp.createTry(Max(1, w()>>mip_map), Max(1, h()>>mip_map), Max(1, d()>>mip_map), hwType(), IMAGE_SOFT, 1, false))
      if(temp.lock(LOCK_WRITE))
      {
         if(lockRead(mip_map, cube_face))
         {
            ok=true;
            Int blocks_y=Min(ImageBlocksY(hwW(), hwH(), mip_map, hwType()), ImageBlocksY(temp.hwW(), temp.hwH(), 0, temp.hwType()));
            REPD(z, temp.d())
            {
             C Byte *src =     data() + z*     pitch2();
               Byte *dest=temp.data() + z*temp.pitch2();
               REPD(y, blocks_y)CopyFast(dest + y*temp.pitch(), src + y*pitch(), Min(temp.pitch(), pitch()));
            }
            unlock();
         }
         temp.unlock();
      }
      if(ok)ok=temp.copyTry(temp, -1, -1, -1, type, mode, 1);
   }
   Swap(dest, temp);
   return ok;
}
/******************************************************************************/
Bool Image::injectMipMap(C Image &src, Int mip_map, DIR_ENUM cube_face, FILTER_TYPE filter, Bool clamp, Bool mtrl_base_1)
{
   Bool ok=false;
   if(InRange(mip_map, mipMaps()))
   {
    C Image *s=&src;
      Image  temp;
      Int w=Max(1, T.w()>>mip_map), h=Max(1, T.h()>>mip_map), d=Max(1, T.d()>>mip_map);
      if(s->w()!=w || s->h()!=h || s->d()!=d || s->hwType()!=hwType() || s==this)
         if(s->copyTry(temp, w, h, d, hwType(), IMAGE_SOFT, 1, filter, clamp, false, false, mtrl_base_1, false))s=&temp;else return false; // resize to mip size, use target type
      if(lock(LOCK_WRITE, mip_map, cube_face))
      {
         if(s->lockRead())
         {
            ok=true;
            Int blocks_y=Min(ImageBlocksY(hwW(), hwH(), mip_map, hwType()), ImageBlocksY(s->hwW(), s->hwH(), 0, s->hwType()));
            REPD(z, Min(ld(), s->d()))
            {
             C Byte *src =s->data() + z*s->pitch2();
               Byte *dest=   data() + z*   pitch2();
               REPD(y, blocks_y)CopyFast(dest + y*pitch(), src + y*s->pitch(), Min(pitch(), s->pitch()));
            }
            s->unlock();
         }
         unlock();
      }
   }
   return ok;
}
/******************************************************************************/
Image& Image::clear()
{
   if(soft())Zero(_data_all, memUsage());else
   {
      Int faces=T.faces();
      REPD(m, mipMaps())
      REPD(f, faces)
         if(lock(LOCK_WRITE, m, DIR_ENUM(f)))
      {
         Zero(data(), ld()*pitch2());
         unlock();
      }
   }
   return T;
}
/******************************************************************************/
Image& Image::normalize(Bool red, Bool green, Bool blue, Bool alpha, C BoxI *box)
{
   if(red || green || blue || alpha)
   {
      IMAGE_TYPE type;
      IMAGE_MODE mode;
      Int        mip_maps;
      if(Decompress(T, type, mode, mip_maps))
      {
         Vec4 min, max; if(stats(&min, &max, null, null, null, null, box))
         {
            Flt d; Vec4 mul, add;
            if(red   && (d=max.x-min.x)){mul.x=1.0f/d; add.x=-min.x*mul.x;}else{mul.x=1; add.x=0;}
            if(green && (d=max.y-min.y)){mul.y=1.0f/d; add.y=-min.y*mul.y;}else{mul.y=1; add.y=0;}
            if(blue  && (d=max.z-min.z)){mul.z=1.0f/d; add.z=-min.z*mul.z;}else{mul.z=1; add.z=0;}
            if(alpha && (d=max.w-min.w)){mul.w=1.0f/d; add.w=-min.w*mul.w;}else{mul.w=1; add.w=0;}
            mulAdd(mul, add, box);
         }
         Compress(T, type, mode, mip_maps);
      }
   }
   return T;
}
/******************************************************************************/
Image& Image::mulAdd(C Vec4 &mul, C Vec4 &add, C BoxI *box)
{
   if(mul!=Vec4(1) || add!=Vec4Zero)
   {
      IMAGE_TYPE type;
      IMAGE_MODE mode;
      Int        mip_maps;
      if(Decompress(T, type, mode, mip_maps))
      {
         if(lock())
         {
            BoxI b(0, size3()); if(box)b&=*box;
            for(Int z=b.min.z; z<b.max.z; z++)
            for(Int y=b.min.y; y<b.max.y; y++)
            for(Int x=b.min.x; x<b.max.x; x++)color3DF(x, y, z, color3DF(x, y, z)*mul+add);
            unlock().updateMipMaps();
         }
         Compress(T, type, mode, mip_maps);
      }
   }
   return T;
}
/******************************************************************************/
void Image::bumpToNormal(Image &dest, Flt scale, Bool high_precision)C
{
 C Image *src=this;
   Image temp;
   if(compressed())if(copyTry(temp, -1, -1, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return;
   if(src->lockRead())
   {
      Image normal;
      if(   normal.createTry(src->w(), src->h(), 1, high_precision ? IMAGE_F32_4 : IMAGE_R8G8B8A8, IMAGE_SOFT, 1) && normal.lock(LOCK_WRITE))
      {
         high_precision=(normal.hwType()==IMAGE_F32_4); // verify in case it was created as different type
         Bool src_hp=src->highPrecision(),
              src_1c=(ImageTI[src->type()].channels==1);
         if( !src_hp)scale/=(src_1c ? (1<<(8*src->bytePP()))-1 : 255);
         Flt z=2/scale;
         REPD(y, src->h())
         REPD(x, src->w())
         {
            Vec4 nrm_bump;
            if(src_hp)
            {
               nrm_bump.w=src->pixelF( x                     ,  y                     );
               Flt      l=src->pixelF((x+src->w()-1)%src->w(),  y                     ),
                        r=src->pixelF((x+         1)%src->w(),  y                     ),
                        u=src->pixelF( x                     , (y+src->h()-1)%src->h()),
                        d=src->pixelF( x                     , (y+         1)%src->h());
               nrm_bump.x=l-r;
               nrm_bump.y=u-d;
            }else
            if(src_1c)
            {
               nrm_bump.w=src->pixel( x                     ,  y                     )*scale;
               Int      l=src->pixel((x+src->w()-1)%src->w(),  y                     ),
                        r=src->pixel((x+         1)%src->w(),  y                     ),
                        u=src->pixel( x                     , (y+src->h()-1)%src->h()),
                        d=src->pixel( x                     , (y+         1)%src->h());
               nrm_bump.x=l-r;
               nrm_bump.y=u-d;
            }else
            {
               nrm_bump.w=src->color( x                     ,  y                     ).r*scale;
               Byte     l=src->color((x+src->w()-1)%src->w(),  y                     ).r,
                        r=src->color((x+         1)%src->w(),  y                     ).r,
                        u=src->color( x                     , (y+src->h()-1)%src->h()).r,
                        d=src->color( x                     , (y+         1)%src->h()).r;
               nrm_bump.x=l-r;
               nrm_bump.y=u-d;
            }
         #if 0
            nrm_bump.x*=scale;
            nrm_bump.y*=scale;
            nrm_bump.z =2;
         #else
            nrm_bump.z=z;
         #endif
            nrm_bump.xyz.normalize();
            if(high_precision)normal.pixF4(x, y)=nrm_bump;else
            {
               Color color;
               color.r=    Round(nrm_bump.x*127)+128;
               color.g=    Round(nrm_bump.y*127)+128;
               color.b=    Round(nrm_bump.z*127)+128;
               color.a=FltToByte(nrm_bump.w);
               normal.color(x, y, color);
            }
         }
         src  ->unlock();
         normal.unlock().updateMipMaps(FILTER_BEST, false); // normal maps are usually wrapped
         Swap(dest, normal);
      }else
      {
         src->unlock();
      }
   }
}
/******************************************************************************
Image& Image::dither(IMAGE_TYPE type)
{
   if(!compressed())switch(type)
   {
      case IMAGE_BC1:
      case IMAGE_BC2:
      case IMAGE_BC3: if(lock()) // "x<<1, y<<2" was tested as best combination (from x=0..3 and y=0..3, XOR was also tested instead of ADD but it made no difference)
      {
         if(hwType()==IMAGE_B8G8R8A8 || hwType()==IMAGE_R8G8B8A8)
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               VecB4 &v=pixB4(x, y);
               Int    d=(((x<<1)+(y<<2))&7)-4;
               v.x=Mid(v.x+ d    , 0, 255); // red   (8-bit original, 5-bit on DXT, 3-bit dither)
               v.y=Mid(v.y+(d>>1), 0, 255); // green (8-bit original, 6-bit on DXT, 2-bit dither)
               v.z=Mid(v.z+ d    , 0, 255); // blue  (8-bit original, 5-bit on DXT, 3-bit dither)
            }
         }else
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color(x, y);
               Int   d=(((x<<1)+(y<<2))&7)-4;
               c.r=Mid(c.r+ d    , 0, 255); // red   (8-bit original, 5-bit on DXT, 3-bit dither)
               c.g=Mid(c.g+(d>>1), 0, 255); // green (8-bit original, 6-bit on DXT, 2-bit dither)
               c.b=Mid(c.b+ d    , 0, 255); // blue  (8-bit original, 5-bit on DXT, 3-bit dither)
               color(x, y, c);
            }
         }
         unlock();
      }break;

      case IMAGE_ETC1: if(lock()) // ETC has 4/4/4 bits or 5/5/5 bits per pixel (set 3-bit dither because 4 is visually too big, set smaller dither for green because it has bigger perceptual value in ETC encoder)
      {
         if(hwType()==IMAGE_B8G8R8A8 || hwType()==IMAGE_R8G8B8A8)
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               VecB4 &v=pixB4(x, y);
               Int    d=(((x<<1)+(y<<2))&7)-4;
               v.x=Mid(v.x+ d    , 0, 255);
               v.y=Mid(v.y+(d>>1), 0, 255);
               v.z=Mid(v.z+ d    , 0, 255);
            }
         }else
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color(x, y);
               Int   d=(((x<<1)+(y<<2))&7)-4;
               c.r=Mid(c.r+ d    , 0, 255);
               c.g=Mid(c.g+(d>>1), 0, 255);
               c.b=Mid(c.b+ d    , 0, 255);
               color(x, y, c);
            }
         }
         unlock();
      }break;

      case IMAGE_PVRTC1_2:
      case IMAGE_PVRTC1_4: if(lock()) // 2-bit dithering gave best results
      {
         if(hwType()==IMAGE_B8G8R8A8 || hwType()==IMAGE_R8G8B8A8)
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               VecB4 &v=pixB4(x, y);
               Int    d=(((x<<0)+(y<<1))&3)-2;
               v.x=Mid(v.x+d, 0, 255);
               v.y=Mid(v.y+d, 0, 255);
               v.z=Mid(v.z+d, 0, 255);
            }
         }else
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color(x, y);
               Int   d=(((x<<0)+(y<<1))&3)-2;
               c.r=Mid(c.r+d, 0, 255);
               c.g=Mid(c.g+d, 0, 255);
               c.b=Mid(c.b+d, 0, 255);
               color(x, y, c);
            }
         }
         unlock();
      }break;
   }
   return T;
}
/******************************************************************************/
Image& Image::fastCrop(Int w, Int h, Int d)
{
   if(is() && mipMaps()==1)
   {
     _size.x=Min(Max(1, w), hwW());
     _size.y=Min(Max(1, h), hwH());
     _size.z=Min(Max(1, d), hwD());
      if(soft())_lock_size=_size;
      setPartial();
   }
   return T;
}
/******************************************************************************/
void Image::crop(Image &dest, Int x, Int y, Int w, Int h)C
{
   crop3D(dest, x, y, 0, w, h, d());
}
void Image::crop3D(Image &dest, Int x, Int y, Int z, Int w, Int h, Int d)C
{
   if(!is() || w<=0 || h<=0 || d<=0){dest.del(); return;}
   if(&dest==this && x==0 && y==0 && z==0 && w==T.w() && h==T.h() && d==T.d())return; // no change needed

 C Image *src=this;
   Image  temp;
   if(compressed())if(copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return;

   if(src->lockRead())
   {
      Int   mip_maps=((mipMaps()>1) ? 0 : 1);
      Image temp;
      if(   temp.createTry(w, h, d, src->type(), src->mode(), mip_maps) && temp.lock(LOCK_WRITE))
      {
         if(src->bytePP()<=4)
         {
            REPD(sz, d)
            REPD(sy, h)
            REPD(sx, w)temp.pixel3D(sx, sy, sz, src->pixel3D(x+sx, y+sy, z+sz));
         }else
         if(ImageTI[src->type()].channels<=1)
         {
            REPD(sz, d)
            REPD(sy, h)
            REPD(sx, w)temp.pixel3DF(sx, sy, sz, src->pixel3DF(x+sx, y+sy, z+sz));
         }else
         {
            REPD(sz, d)
            REPD(sy, h)
            REPD(sx, w)temp.color3DF(sx, sy, sz, src->color3DF(x+sx, y+sy, z+sz));
         }

         temp.unlock().updateMipMaps();
         temp.copyTry(temp, -1, -1, -1, T.type(), T.mode(), mip_maps);
         Swap(dest, temp);
      }
      src->unlock();
   }
}
/******************************************************************************/
Image& Image::resize(Int w, Int h, FILTER_TYPE filter, Bool clamp, Bool alpha_weight, Bool keep_edges)
{
   MAX(w, 1);
   MAX(h, 1);
   if(is() && (w!=T.w() || h!=T.h()))copyTry(T, w, h, -1, -1, -1, -1, filter, clamp, alpha_weight, keep_edges);
   return T;
}
/******************************************************************************/
Image& Image::resize3D(Int w, Int h, Int d, FILTER_TYPE filter, Bool clamp, Bool alpha_weight, Bool keep_edges)
{
   MAX(w, 1);
   MAX(h, 1);
   MAX(d, 1);
   if(is() && (w!=T.w() || h!=T.h() || d!=T.d()))copyTry(T, w, h, d, -1, -1, -1, filter, clamp, alpha_weight, keep_edges);
   return T;
}
/******************************************************************************/
// MIRROR
/******************************************************************************/
Image& Image::mirrorX()
{
   if(is())
   {
    C Image *src=this;
      Image  temp;
      if(compressed())
         if(copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return T;
      if(src->lockRead())
      {
         Bool  ok=false;
         Image mirror; if(mirror.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
            if(mirror.lock(LOCK_WRITE))
         {
            if(mirror.highPrecision())
            {
               REPD(z, mirror.d())
               REPD(y, mirror.h())
               REPD(x, mirror.w())mirror.color3DF(x, y, z, src->color3DF(mirror.w()-1-x, y, z));
            }else
            {
               REPD(z, mirror.d())
               REPD(y, mirror.h())
               REPD(x, mirror.w())mirror.color3D(x, y, z, src->color3D(mirror.w()-1-x, y, z));
            }
            mirror.unlock();
            ok=mirror.copyTry(mirror, w(), h(), d(), type(), mode(), mipMaps());
         }
         src->unlock();
         if(ok)Swap(T, mirror.updateMipMaps());
      }
   }
   return T;
}
/******************************************************************************/
Image& Image::mirrorY()
{
   if(is())
   {
    C Image *src=this;
      Image  temp;
      if(compressed())
         if(copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return T;
      if(src->lockRead())
      {
         Bool  ok=false;
         Image mirror; if(mirror.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
            if(mirror.lock(LOCK_WRITE))
         {
            if(mirror.highPrecision())
            {
               REPD(z, mirror.d())
               REPD(y, mirror.h())
               REPD(x, mirror.w())mirror.color3DF(x, y, z, src->color3DF(x, mirror.h()-1-y, z));
            }else
            {
               REPD(z, mirror.d())
               REPD(y, mirror.h())
               REPD(x, mirror.w())mirror.color3D(x, y, z, src->color3D(x, mirror.h()-1-y, z));
            }
            mirror.unlock();
            ok=mirror.copyTry(mirror, w(), h(), d(), type(), mode(), mipMaps());
         }
         src->unlock();
         if(ok)Swap(T, mirror.updateMipMaps());
      }
   }
   return T;
}
/******************************************************************************/
// ALPHA
/******************************************************************************/
Image& Image::alphaFromKey(C Color &key)
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;
   if(Decompress(T, type, mode, mip_maps))
   {
      if(!ImageTI[type].a){copy(T, -1, -1, -1, IMAGE_R8G8B8A8, mode, mip_maps); type=T.type();} // if image doesn't have alpha channel then convert to R8G8B8A8, set 'type' to value after conversion so it won't be converted back
      if(lock())
      {
         REPD(z, T.d())
         REPD(y, T.h())
         REPD(x, T.w()){Color c=color3D(x, y, z); color3D(x, y, z, (c==key) ? TRANSPARENT : Color(c.r, c.g, c.b));}
         unlock().updateMipMaps();
         Compress(T, type, mode, mip_maps);
      }
   }
   return T;
}
/******************************************************************************/
Image& Image::alphaFromBrightness()
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;
   if(Decompress(T, type, mode, mip_maps))
   {
      if(!ImageTI[T.type()].a){copy(T, -1, -1, -1, ImageTypeIncludeAlpha(T.type()), mode, mip_maps); type=T.type();} // if image doesn't have alpha channel then include it, set 'type' to value after conversion so it won't be converted back
      if(lock())
      {
         if(highPrecision())
         {
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w()){Vec4 c=color3DF(x, y, z); c.w=c.xyz.max(); color3DF(x, y, z, c);}
         }else
         {
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w()){Color c=color3D(x, y, z); c.a=c.lum(); color3D(x, y, z, c);}
         }
         unlock().updateMipMaps();
         Compress(T, type, mode, mip_maps);
      }
   }
   return T;
}
/******************************************************************************/
Image& Image::divRgbByAlpha()
{
   if(ImageTI[type()].a)
   {
      IMAGE_TYPE type;
      IMAGE_MODE mode;
      Int        mip_maps;
      if(Decompress(T, type, mode, mip_maps) && lock())
      {
         if(highPrecision())
         {
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w()){Vec4 c=color3DF(x, y, z); if(c.w){c.xyz/=c.w; color3DF(x, y, z, c);}}
         }else
         {
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w()){Color c=color3D(x, y, z); if(c.a){c.r=Min(c.r*255/c.a, 255); c.g=Min(c.g*255/c.a, 255); c.b=Min(c.b*255/c.a, 255); color3D(x, y, z, c);}}
         }
         unlock().updateMipMaps();
         Compress(T, type, mode, mip_maps);
      }
   }
   return T;
}
/******************************************************************************/
// DOWNSAMPLE
/******************************************************************************/
Image& Image::downSample(FILTER_TYPE filter, Bool clamp, Bool alpha_weight)
{
   if(w()>1 || h()>1 || d()>1)copyTry(T, Max(1, w()>>1), Max(1, h()>>1), Max(1, d()>>1), -1, -1, -1, filter, clamp, alpha_weight);
   return T;
}
/******************************************************************************
Image& Image::downSampleNormal()
{
   if((w()>1 || h()>1) && d()==1)
   {
      IMAGE_TYPE type;
      IMAGE_MODE mode;
      Int        mip_maps;
      if(Decompress(T, type, mode, mip_maps)) // try to preserve number of mip-maps
      {
         if(lockRead())
         {
            Image temp(Max(1, w()>>1), Max(1, h()>>1), Max(1, d()>>1), T.type(), T.mode(), ImageTI[type].compressed ? 1 : mip_maps);
            if(   temp.lock(LOCK_WRITE))
            {
               REPD(y, temp.h())
               REPD(x, temp.w())
               {
                  UInt  x2=x*2,
                        y2=y*2;
                  Color lu=color(x2+0, y2+0),
                        ru=color(x2+1, y2+0),
                        ld=color(x2+0, y2+1),
                        rd=color(x2+1, y2+1);

                  Vec nrm;
                  nrm.x = lu.r+ru.r+ld.r+rd.r - 4*128;
                  nrm.y = lu.g+ru.g+ld.g+rd.g - 4*128;
                  nrm.z = lu.b+ru.b+ld.b+rd.b - 4*128;
                  nrm.normalize();

                  Color color;
                  color.r=Round(nrm.x*127+128);
                  color.g=Round(nrm.y*127+128);
                  color.b=Round(nrm.z*127+128);
                  color.a=((lu.a+ru.a+ld.a+rd.a+2)>>2);

                  temp.color(x, y, color);
               }
               Swap(unlock(), temp.unlock().updateMipMaps());
               Compress(T, type, mode, mip_maps);
            }else
            {
               unlock();
            }
         }
      }
   }
   return T;
}
/******************************************************************************/
// BLUR
/******************************************************************************/
struct BlurContext
{
   typedef void(BlurContext::*Blur)(Int x, Int y, Int z)C; // pointer to BlurContext method

   Bool             high_prec, clamp;
   Byte             func;
   Int              rangei, rangei_2_1;
   Flt              range;
   VecI             size;
 C Image           *src;
   Image           *dest;
   Threads         *threads;
   MemtN<Byte, 256> weight_b;
   MemtN<Flt , 256> weight_f;
   Blur             blur_ptr;

   BlurContext(C Image &src, Bool clamp, Threads *threads)
   {
      if(T.high_prec=src.highPrecision())
      {
         if(src.hwType()==IMAGE_F32        )func=0;else // HP 1F, here check 'hwType' because we will access memory directly using 'pixF' method
         if(ImageTI[src.type()].channels==1)func=1;else // HP 1C
                                            func=2;     // HP MC
      }else
      {
       C ImageTypeInfo &ti=ImageTI[src.hwType()];  // here check 'hwType' because we will access memory directly using 'pixB' method
         if(ti.bit_pp==8 && ti.channels==1)func=3; // LP 1byte
         else                              func=4; // LP MC
      }
      T.clamp=clamp; T.size=src.size3(); T.rangei=0; T.rangei_2_1=1; T.range=0; T.threads=threads; T.src=&src;
   }
   void setRange(Flt range) // this is used for blur
   {
      MAX(range, 0); if(T.range!=range)
      {
         T.range     =     range;
         T.rangei    =Ceil(range);
         T.rangei_2_1=rangei*2+1;
         Flt range_1=range+1;
         if(high_prec){weight_f.setNum(rangei_2_1); REPAO(weight_f)=           BlendSmoothCube(Flt(i-rangei)/range_1) ;}
         else         {weight_b.setNum(rangei_2_1); REPAO(weight_b)=RoundU(255*BlendSmoothCube(Flt(i-rangei)/range_1));}
      }
   }
   void setRange(Int range) // this is used for average
   {
      T.rangei=range;
      T.rangei_2_1=rangei*2+1;
   }

   void setX()
   {
      switch(func)
      {
         case 0: blur_ptr=clamp ? &BlurContext::blur_X_HP_1F_CLAMP : &BlurContext::blur_X_HP_1F_WRAP; break;
         case 1: blur_ptr=clamp ? &BlurContext::blur_X_HP_1C_CLAMP : &BlurContext::blur_X_HP_1C_WRAP; break;
         case 2: blur_ptr=clamp ? &BlurContext::blur_X_HP_MC_CLAMP : &BlurContext::blur_X_HP_MC_WRAP; break;
         case 3: blur_ptr=clamp ? &BlurContext::blur_X_LP_1B_CLAMP : &BlurContext::blur_X_LP_1B_WRAP; break;
         case 4: blur_ptr=clamp ? &BlurContext::blur_X_LP_MC_CLAMP : &BlurContext::blur_X_LP_MC_WRAP; break;
      }
   }
   void setY()
   {
      switch(func)
      {
         case 0: blur_ptr=clamp ? &BlurContext::blur_Y_HP_1F_CLAMP : &BlurContext::blur_Y_HP_1F_WRAP; break;
         case 1: blur_ptr=clamp ? &BlurContext::blur_Y_HP_1C_CLAMP : &BlurContext::blur_Y_HP_1C_WRAP; break;
         case 2: blur_ptr=clamp ? &BlurContext::blur_Y_HP_MC_CLAMP : &BlurContext::blur_Y_HP_MC_WRAP; break;
         case 3: blur_ptr=clamp ? &BlurContext::blur_Y_LP_1B_CLAMP : &BlurContext::blur_Y_LP_1B_WRAP; break;
         case 4: blur_ptr=clamp ? &BlurContext::blur_Y_LP_MC_CLAMP : &BlurContext::blur_Y_LP_MC_WRAP; break;
      }
   }
   void setZ()
   {
      switch(func)
      {
         case 0: blur_ptr=clamp ? &BlurContext::blur_Z_HP_1F_CLAMP : &BlurContext::blur_Z_HP_1F_WRAP; break;
         case 1: blur_ptr=clamp ? &BlurContext::blur_Z_HP_1C_CLAMP : &BlurContext::blur_Z_HP_1C_WRAP; break;
         case 2: blur_ptr=clamp ? &BlurContext::blur_Z_HP_MC_CLAMP : &BlurContext::blur_Z_HP_MC_WRAP; break;
         case 3: blur_ptr=clamp ? &BlurContext::blur_Z_LP_1B_CLAMP : &BlurContext::blur_Z_LP_1B_WRAP; break;
         case 4: blur_ptr=clamp ? &BlurContext::blur_Z_LP_MC_CLAMP : &BlurContext::blur_Z_LP_MC_WRAP; break;
      }
   }

   void setAvgX()
   {
      switch(func)
      {
         case 0: blur_ptr=clamp ? &BlurContext::avg_X_HP_1F_CLAMP : &BlurContext::avg_X_HP_1F_WRAP; break;
         case 1: blur_ptr=clamp ? &BlurContext::avg_X_HP_1C_CLAMP : &BlurContext::avg_X_HP_1C_WRAP; break;
         case 2: blur_ptr=clamp ? &BlurContext::avg_X_HP_MC_CLAMP : &BlurContext::avg_X_HP_MC_WRAP; break;
         case 3: blur_ptr=clamp ? &BlurContext::avg_X_LP_1B_CLAMP : &BlurContext::avg_X_LP_1B_WRAP; break;
         case 4: blur_ptr=clamp ? &BlurContext::avg_X_LP_MC_CLAMP : &BlurContext::avg_X_LP_MC_WRAP; break;
      }
   }
   void setAvgY()
   {
      switch(func)
      {
         case 0: blur_ptr=clamp ? &BlurContext::avg_Y_HP_1F_CLAMP : &BlurContext::avg_Y_HP_1F_WRAP; break;
         case 1: blur_ptr=clamp ? &BlurContext::avg_Y_HP_1C_CLAMP : &BlurContext::avg_Y_HP_1C_WRAP; break;
         case 2: blur_ptr=clamp ? &BlurContext::avg_Y_HP_MC_CLAMP : &BlurContext::avg_Y_HP_MC_WRAP; break;
         case 3: blur_ptr=clamp ? &BlurContext::avg_Y_LP_1B_CLAMP : &BlurContext::avg_Y_LP_1B_WRAP; break;
         case 4: blur_ptr=clamp ? &BlurContext::avg_Y_LP_MC_CLAMP : &BlurContext::avg_Y_LP_MC_WRAP; break;
      }
   }
   void setAvgZ()
   {
      switch(func)
      {
         case 0: blur_ptr=clamp ? &BlurContext::avg_Z_HP_1F_CLAMP : &BlurContext::avg_Z_HP_1F_WRAP; break;
         case 1: blur_ptr=clamp ? &BlurContext::avg_Z_HP_1C_CLAMP : &BlurContext::avg_Z_HP_1C_WRAP; break;
         case 2: blur_ptr=clamp ? &BlurContext::avg_Z_HP_MC_CLAMP : &BlurContext::avg_Z_HP_MC_WRAP; break;
         case 3: blur_ptr=clamp ? &BlurContext::avg_Z_LP_1B_CLAMP : &BlurContext::avg_Z_LP_1B_WRAP; break;
         case 4: blur_ptr=clamp ? &BlurContext::avg_Z_LP_MC_CLAMP : &BlurContext::avg_Z_LP_MC_WRAP; break;
      }
   }

   inline void blur(Int x, Int y, Int z)C {(T.*blur_ptr)(x, y, z);}

   static void BlurFunc(IntPtr elm_index, BlurContext &bc, Int thread_index)
   {
      Int  z=Unsigned(elm_index)/Unsigned(bc.size.y),
           y=Unsigned(elm_index)%Unsigned(bc.size.y);
      REPD(x, bc.size.x)bc.blur(x, y, z);
   }

   void blur()
   {
      if(threads)threads->process1(size.z*size.y, BlurFunc, T);else
      REPD(z, size.z)
      REPD(y, size.y)
      REPD(x, size.x)blur(x, y, z);
   }

   // X
   void blur_X_HP_1F_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;}
    C Flt *data=&src->pixF(0, y, z);
      for(; min<=max; min++, i++)
      {
         Flt c=data[min], w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixF(x, y, z)=color;
   }
   void blur_X_HP_1F_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
    C Flt *data=&src->pixF(0, y, z);
      REP(rangei_2_1)
      {
         Flt c=data[Mod(x+i-rangei, size.x)], w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixF(x, y, z)=color;
   }
   void blur_X_HP_1C_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Flt c=src->pixel3DF(min, y, z), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixel3DF(x, y, z, color);
   }
   void blur_X_HP_1C_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
      REP(rangei_2_1)
      {
         Flt c=src->pixel3DF(Mod(x+i-rangei, size.x), y, z), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixel3DF(x, y, z, color);
   }
   void blur_X_HP_MC_CLAMP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Flt  power=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Vec4 c=src->color3DF(min, y, z);
         Flt  w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->color3DF(x, y, z, color);
   }
   void blur_X_HP_MC_WRAP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Flt  power=0;
      REP(rangei_2_1)
      {
         Vec4 c=src->color3DF(Mod(x+i-rangei, size.x), y, z);
         Flt  w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->color3DF(x, y, z, color);
   }
   void blur_X_LP_1B_CLAMP(Int x, Int y, Int z)C
   {
      UInt value=0, power=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;}
    C Byte *data=&src->pixB(0, y, z);
      for(; min<=max; min++, i++)
      {
         Byte c=data[min], w=weight_b[i];
         value+=w*c;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); value=(value+p_2)/power;}
      dest->pixB(x, y, z)=value;
   }
   void blur_X_LP_1B_WRAP(Int x, Int y, Int z)C
   {
      UInt value=0, power=0;
    C Byte *data=&src->pixB(0, y, z);
      REP(rangei_2_1)
      {
         Byte c=data[Mod(x+i-rangei, size.x)], w=weight_b[i];
         value+=w*c;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); value=(value+p_2)/power;}
      dest->pixB(x, y, z)=value;
   }
   void blur_X_LP_MC_CLAMP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0, power=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Color c=src->color3D(min, y, z);
         Byte  w=weight_b[i];
         r+=c.r*w; g+=c.g*w; b+=c.b*w; a+=c.a*w;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power;}
      dest->color3D(x, y, z, Color(r, g, b, a));
   }
   void blur_X_LP_MC_WRAP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0, power=0;
      REP(rangei_2_1)
      {
         Color c=src->color3D(Mod(x+i-rangei, size.x), y, z);
         Byte  w=weight_b[i];
         r+=c.r*w; g+=c.g*w; b+=c.b*w; a+=c.a*w;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power;}
      dest->color3D(x, y, z, Color(r, g, b, a));
   }

   // Y
   void blur_Y_HP_1F_CLAMP(Int x, Int y, Int z)C
   {
      Flt  color=0, power=0;
      Int  i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;}
      UInt pitch=src->pitch();
    C Flt *data=&src->pixF(x, 0, z);
      for(; min<=max; min++, i++)
      {
         Flt c=*(Flt*)((Byte*)data+min*pitch), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixF(x, y, z)=color;
   }
   void blur_Y_HP_1F_WRAP(Int x, Int y, Int z)C
   {
      Flt  color=0, power=0;
      UInt pitch=src->pitch();
    C Flt *data=&src->pixF(x, 0, z);
      REP(rangei_2_1)
      {
         Flt c=*(Flt*)((Byte*)data+Mod(y+i-rangei, size.y)*pitch), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixF(x, y, z)=color;
   }
   void blur_Y_HP_1C_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Flt c=src->pixel3DF(x, min, z), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixel3DF(x, y, z, color);
   }
   void blur_Y_HP_1C_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
      REP(rangei_2_1)
      {
         Flt c=src->pixel3DF(x, Mod(y+i-rangei, size.y), z), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixel3DF(x, y, z, color);
   }
   void blur_Y_HP_MC_CLAMP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Flt  power=0;
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Vec4 c=src->color3DF(x, min, z);
         Flt  w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->color3DF(x, y, z, color);
   }
   void blur_Y_HP_MC_WRAP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Flt  power=0;
      REP(rangei_2_1)
      {
         Vec4 c=src->color3DF(x, Mod(y+i-rangei, size.y), z);
         Flt  w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->color3DF(x, y, z, color);
   }
   void blur_Y_LP_1B_CLAMP(Int x, Int y, Int z)C
   {
      UInt value=0, power=0, pitch=src->pitch();
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;}
    C Byte *data=&src->pixB(x, 0, z);
      for(; min<=max; min++, i++)
      {
         Byte c=data[min*pitch], w=weight_b[i];
         value+=w*c;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); value=(value+p_2)/power;}
      dest->pixB(x, y, z)=value;
   }
   void blur_Y_LP_1B_WRAP(Int x, Int y, Int z)C
   {
      UInt value=0, power=0, pitch=src->pitch();
    C Byte *data=&src->pixB(x, 0, z);
      REP(rangei_2_1)
      {
         Byte c=data[Mod(y+i-rangei, size.y)*pitch], w=weight_b[i];
         value+=w*c;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); value=(value+p_2)/power;}
      dest->pixB(x, y, z)=value;
   }
   void blur_Y_LP_MC_CLAMP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0, power=0;
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Color c=src->color3D(x, min, z);
         Byte  w=weight_b[i];
         r+=c.r*w; g+=c.g*w; b+=c.b*w; a+=c.a*w;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power;}
      dest->color3D(x, y, z, Color(r, g, b, a));
   }
   void blur_Y_LP_MC_WRAP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0, power=0;
      REP(rangei_2_1)
      {
         Color c=src->color3D(x, Mod(y+i-rangei, size.y), z);
         Byte  w=weight_b[i];
         r+=c.r*w; g+=c.g*w; b+=c.b*w; a+=c.a*w;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power;}
      dest->color3D(x, y, z, Color(r, g, b, a));
   }

   // Z
   void blur_Z_HP_1F_CLAMP(Int x, Int y, Int z)C
   {
      Flt  color=0, power=0;
      Int  i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;}
      UInt pitch=src->pitch2();
    C Flt *data=&src->pixF(x, y, 0);
      for(; min<=max; min++, i++)
      {
         Flt c=*(Flt*)((Byte*)data+min*pitch), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixF(x, y, z)=color;
   }
   void blur_Z_HP_1F_WRAP(Int x, Int y, Int z)C
   {
      Flt  color=0, power=0;
      UInt pitch=src->pitch2();
    C Flt *data=&src->pixF(x, y, 0);
      REP(rangei_2_1)
      {
         Flt c=*(Flt*)((Byte*)data+Mod(z+i-rangei, size.z)*pitch), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixF(x, y, z)=color;
   }
   void blur_Z_HP_1C_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Flt c=src->pixel3DF(x, y, min), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixel3DF(x, y, z, color);
   }
   void blur_Z_HP_1C_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0, power=0;
      REP(rangei_2_1)
      {
         Flt c=src->pixel3DF(x, y, Mod(z+i-rangei, size.z)), w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->pixel3DF(x, y, z, color);
   }
   void blur_Z_HP_MC_CLAMP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Flt  power=0;
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Vec4 c=src->color3DF(x, y, min);
         Flt  w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->color3DF(x, y, z, color);
   }
   void blur_Z_HP_MC_WRAP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Flt  power=0;
      REP(rangei_2_1)
      {
         Vec4 c=src->color3DF(x, y, Mod(z+i-rangei, size.z));
         Flt  w=weight_f[i];
         color+=w*c;
         power+=w;
      }
      if(power)color/=power;
      dest->color3DF(x, y, z, color);
   }
   void blur_Z_LP_1B_CLAMP(Int x, Int y, Int z)C
   {
      UInt value=0, power=0, pitch=src->pitch2();
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;}
    C Byte *data=&src->pixB(x, y, 0);
      for(; min<=max; min++, i++)
      {
         Byte c=data[min*pitch], w=weight_b[i];
         value+=w*c;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); value=(value+p_2)/power;}
      dest->pixB(x, y, z)=value;
   }
   void blur_Z_LP_1B_WRAP(Int x, Int y, Int z)C
   {
      UInt value=0, power=0, pitch=src->pitch2();
    C Byte *data=&src->pixB(x, y, 0);
      REP(rangei_2_1)
      {
         Byte c=data[Mod(z+i-rangei, size.z)*pitch], w=weight_b[i];
         value+=w*c;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); value=(value+p_2)/power;}
      dest->pixB(x, y, z)=value;
   }
   void blur_Z_LP_MC_CLAMP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0, power=0;
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;}
      for(; min<=max; min++, i++)
      {
         Color c=src->color3D(x, y, min);
         Byte  w=weight_b[i];
         r+=c.r*w; g+=c.g*w; b+=c.b*w; a+=c.a*w;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power;}
      dest->color3D(x, y, z, Color(r, g, b, a));
   }
   void blur_Z_LP_MC_WRAP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0, power=0;
      REP(rangei_2_1)
      {
         Color c=src->color3D(x, y, Mod(z+i-rangei, size.z));
         Byte  w=weight_b[i];
         r+=c.r*w; g+=c.g*w; b+=c.b*w; a+=c.a*w;
         power+=w;
      }
      if(power){UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power;}
      dest->color3D(x, y, z, Color(r, g, b, a));
   }

   // AVERAGE

   // X
   void avg_X_HP_1F_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
    C Flt *data=&src->pixF(0, y, z);
      for(; min<=max; min++, i++)color+=data[min];
      color/=power; // always non-zero
      dest->pixF(x, y, z)=color;
   }
   void avg_X_HP_1F_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0;
    C Flt *data=&src->pixF(0, y, z);
      REP(rangei_2_1)color+=data[Mod(x+i-rangei, size.x)];
      color/=rangei_2_1; // always non-zero
      dest->pixF(x, y, z)=color;
   }
   void avg_X_HP_1C_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++)color+=src->pixel3DF(min, y, z);
      color/=power; // always non-zero
      dest->pixel3DF(x, y, z, color);
   }
   void avg_X_HP_1C_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0;
      REP(rangei_2_1)color+=src->pixel3DF(Mod(x+i-rangei, size.x), y, z);
      color/=rangei_2_1; // always non-zero
      dest->pixel3DF(x, y, z, color);
   }
   void avg_X_HP_MC_CLAMP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++)color+=src->color3DF(min, y, z);
      color/=power; // always non-zero
      dest->color3DF(x, y, z, color);
   }
   void avg_X_HP_MC_WRAP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      REP(rangei_2_1)color+=src->color3DF(Mod(x+i-rangei, size.x), y, z);
      color/=rangei_2_1; // always non-zero
      dest->color3DF(x, y, z, color);
   }
   void avg_X_LP_1B_CLAMP(Int x, Int y, Int z)C
   {
      UInt value=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
    C Byte *data=&src->pixB(0, y, z);
      for(; min<=max; min++, i++)value+=data[min];
      UInt p_2=(power>>1); value=(value+p_2)/power; // always non-zero
      dest->pixB(x, y, z)=value;
   }
   void avg_X_LP_1B_WRAP(Int x, Int y, Int z)C
   {
      UInt value=0;
    C Byte *data=&src->pixB(0, y, z);
      REP(rangei_2_1)value+=data[Mod(x+i-rangei, size.x)];
      UInt p_2=(rangei_2_1>>1); value=(value+p_2)/rangei_2_1; // always non-zero
      dest->pixB(x, y, z)=value;
   }
   void avg_X_LP_MC_CLAMP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0;
      Int i=0, min=x-rangei, max=Min(x+rangei, size.x-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++){Color c=src->color3D(min, y, z); r+=c.r; g+=c.g; b+=c.b; a+=c.a;}
      UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power; // always non-zero
      dest->color3D(x, y, z, Color(r, g, b, a));
   }
   void avg_X_LP_MC_WRAP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0;
      REP(rangei_2_1){Color c=src->color3D(Mod(x+i-rangei, size.x), y, z); r+=c.r; g+=c.g; b+=c.b; a+=c.a;}
      UInt p_2=(rangei_2_1>>1); r=(r+p_2)/rangei_2_1; g=(g+p_2)/rangei_2_1; b=(b+p_2)/rangei_2_1; a=(a+p_2)/rangei_2_1; // always non-zero
      dest->color3D(x, y, z, Color(r, g, b, a));
   }

   // Y
   void avg_Y_HP_1F_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0;
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;} UInt power=max-min+1, pitch=src->pitch();
    C Flt *data=&src->pixF(x, 0, z);
      for(; min<=max; min++, i++)color+=*(Flt*)((Byte*)data+min*pitch);
      color/=power; // always non-zero
      dest->pixF(x, y, z)=color;
   }
   void avg_Y_HP_1F_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0; UInt pitch=src->pitch();
    C Flt *data=&src->pixF(x, 0, z);
      REP(rangei_2_1)color+=*(Flt*)((Byte*)data+Mod(y+i-rangei, size.y)*pitch);
      color/=rangei_2_1; // always non-zero
      dest->pixF(x, y, z)=color;
   }
   void avg_Y_HP_1C_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0;
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++)color+=src->pixel3DF(x, min, z);
      color/=power; // always non-zero
      dest->pixel3DF(x, y, z, color);
   }
   void avg_Y_HP_1C_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0;
      REP(rangei_2_1)color+=src->pixel3DF(x, Mod(y+i-rangei, size.y), z);
      color/=rangei_2_1; // always non-zero
      dest->pixel3DF(x, y, z, color);
   }
   void avg_Y_HP_MC_CLAMP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++)color+=src->color3DF(x, min, z);
      color/=power; // always non-zero
      dest->color3DF(x, y, z, color);
   }
   void avg_Y_HP_MC_WRAP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      REP(rangei_2_1)color+=src->color3DF(x, Mod(y+i-rangei, size.y), z);
      color/=rangei_2_1; // always non-zero
      dest->color3DF(x, y, z, color);
   }
   void avg_Y_LP_1B_CLAMP(Int x, Int y, Int z)C
   {
      UInt value=0, pitch=src->pitch();
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
    C Byte *data=&src->pixB(x, 0, z);
      for(; min<=max; min++, i++)value+=data[min*pitch];
      UInt p_2=(power>>1); value=(value+p_2)/power; // always non-zero
      dest->pixB(x, y, z)=value;
   }
   void avg_Y_LP_1B_WRAP(Int x, Int y, Int z)C
   {
      UInt value=0, pitch=src->pitch();
    C Byte *data=&src->pixB(x, 0, z);
      REP(rangei_2_1)value+=data[Mod(y+i-rangei, size.y)*pitch];
      UInt p_2=(rangei_2_1>>1); value=(value+p_2)/rangei_2_1; // always non-zero
      dest->pixB(x, y, z)=value;
   }
   void avg_Y_LP_MC_CLAMP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0;
      Int i=0, min=y-rangei, max=Min(y+rangei, size.y-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++){Color c=src->color3D(x, min, z); r+=c.r; g+=c.g; b+=c.b; a+=c.a;}
      UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power; // always non-zero
      dest->color3D(x, y, z, Color(r, g, b, a));
   }
   void avg_Y_LP_MC_WRAP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0;
      REP(rangei_2_1){Color c=src->color3D(x, Mod(y+i-rangei, size.y), z); r+=c.r; g+=c.g; b+=c.b; a+=c.a;}
      UInt p_2=(rangei_2_1>>1); r=(r+p_2)/rangei_2_1; g=(g+p_2)/rangei_2_1; b=(b+p_2)/rangei_2_1; a=(a+p_2)/rangei_2_1; // always non-zero
      dest->color3D(x, y, z, Color(r, g, b, a));
   }

   // Z
   void avg_Z_HP_1F_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0;
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;} UInt power=max-min+1, pitch=src->pitch2();
    C Flt *data=&src->pixF(x, y, 0);
      for(; min<=max; min++, i++)color+=*(Flt*)((Byte*)data+min*pitch);
      color/=power; // always non-zero
      dest->pixF(x, y, z)=color;
   }
   void avg_Z_HP_1F_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0; UInt pitch=src->pitch2();
    C Flt *data=&src->pixF(x, y, 0);
      REP(rangei_2_1)color+=*(Flt*)((Byte*)data+Mod(z+i-rangei, size.z)*pitch);
      color/=rangei_2_1; // always non-zero
      dest->pixF(x, y, z)=color;
   }
   void avg_Z_HP_1C_CLAMP(Int x, Int y, Int z)C
   {
      Flt color=0;
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++)color+=src->pixel3DF(x, y, min);
      color/=power; // always non-zero
      dest->pixel3DF(x, y, z, color);
   }
   void avg_Z_HP_1C_WRAP(Int x, Int y, Int z)C
   {
      Flt color=0;
      REP(rangei_2_1)color+=src->pixel3DF(x, y, Mod(z+i-rangei, size.z));
      color/=rangei_2_1; // always non-zero
      dest->pixel3DF(x, y, z, color);
   }
   void avg_Z_HP_MC_CLAMP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++)color+=src->color3DF(x, y, min);
      color/=power; // always non-zero
      dest->color3DF(x, y, z, color);
   }
   void avg_Z_HP_MC_WRAP(Int x, Int y, Int z)C
   {
      Vec4 color=0;
      REP(rangei_2_1)color+=src->color3DF(x, y, Mod(z+i-rangei, size.z));
      color/=rangei_2_1; // always non-zero
      dest->color3DF(x, y, z, color);
   }
   void avg_Z_LP_1B_CLAMP(Int x, Int y, Int z)C
   {
      UInt value=0, pitch=src->pitch2();
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
    C Byte *data=&src->pixB(x, y, 0);
      for(; min<=max; min++, i++)value+=data[min*pitch];
      UInt p_2=(power>>1); value=(value+p_2)/power; // always non-zero
      dest->pixB(x, y, z)=value;
   }
   void avg_Z_LP_1B_WRAP(Int x, Int y, Int z)C
   {
      UInt value=0, pitch=src->pitch2();
    C Byte *data=&src->pixB(x, y, 0);
      REP(rangei_2_1)value+=data[Mod(z+i-rangei, size.z)*pitch];
      UInt p_2=(rangei_2_1>>1); value=(value+p_2)/rangei_2_1; // always non-zero
      dest->pixB(x, y, z)=value;
   }
   void avg_Z_LP_MC_CLAMP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0;
      Int i=0, min=z-rangei, max=Min(z+rangei, size.z-1); if(min<0){i=-min; min=0;} UInt power=max-min+1;
      for(; min<=max; min++, i++){Color c=src->color3D(x, y, min); r+=c.r; g+=c.g; b+=c.b; a+=c.a;}
      UInt p_2=(power>>1); r=(r+p_2)/power; g=(g+p_2)/power; b=(b+p_2)/power; a=(a+p_2)/power; // always non-zero
      dest->color3D(x, y, z, Color(r, g, b, a));
   }
   void avg_Z_LP_MC_WRAP(Int x, Int y, Int z)C
   {
      UInt r=0, g=0, b=0, a=0;
      REP(rangei_2_1){Color c=src->color3D(x, y, Mod(z+i-rangei, size.z)); r+=c.r; g+=c.g; b+=c.b; a+=c.a;}
      UInt p_2=(rangei_2_1>>1); r=(r+p_2)/rangei_2_1; g=(g+p_2)/rangei_2_1; b=(b+p_2)/rangei_2_1; a=(a+p_2)/rangei_2_1; // always non-zero
      dest->color3D(x, y, z, Color(r, g, b, a));
   }
};
/******************************************************************************/
Bool Image::averageX(Image &dest, Int range, Bool clamp, Threads *threads)C
{
   if(range<=0 || w()<=1)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   Image &work=((src==&dest) ? temp : dest); // this makes &work!=src
   if(!work.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copyTry'), and if not compressed then we will create the target as what we want the dest to be
   if(!src->lockRead())return false;
   if(!work.lock    (LOCK_WRITE)){src->unlock(); return false;}

   BlurContext bc(*src, clamp, threads); bc.setRange(range); bc.setAvgX(); bc.dest=&work; bc.blur();

   work.unlock(); src->unlock();
   if(work.type()==type && work.mode()==mode && work.mipMaps()==mip_maps){work.updateMipMaps(FILTER_BEST, clamp); if(&work!=&dest)Swap(work, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
   return work.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
Bool Image::averageY(Image &dest, Int range, Bool clamp, Threads *threads)C
{
   if(range<=0 || h()<=1)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   Image &work=((src==&dest) ? temp : dest); // this makes &work!=src
   if(!work.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copyTry'), and if not compressed then we will create the target as what we want the dest to be
   if(!src->lockRead())return false;
   if(!work.lock    (LOCK_WRITE)){src->unlock(); return false;}

   BlurContext bc(*src, clamp, threads); bc.setRange(range); bc.setAvgY(); bc.dest=&work; bc.blur();

   work.unlock(); src->unlock();
   if(work.type()==type && work.mode()==mode && work.mipMaps()==mip_maps){work.updateMipMaps(FILTER_BEST, clamp); if(&work!=&dest)Swap(work, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
   return work.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
Bool Image::averageZ(Image &dest, Int range, Bool clamp, Threads *threads)C
{
   if(range<=0 || d()<=1)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   Image &work=((src==&dest) ? temp : dest); // this makes &work!=src
   if(!work.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copyTry'), and if not compressed then we will create the target as what we want the dest to be
   if(!src->lockRead())return false;
   if(!work.lock    (LOCK_WRITE)){src->unlock(); return false;}

   BlurContext bc(*src, clamp, threads); bc.setRange(range); bc.setAvgZ(); bc.dest=&work; bc.blur();

   work.unlock(); src->unlock();
   if(work.type()==type && work.mode()==mode && work.mipMaps()==mip_maps){work.updateMipMaps(FILTER_BEST, clamp); if(&work!=&dest)Swap(work, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
   return work.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
/******************************************************************************/
Bool Image::blurX(Image &dest, Flt range, Bool clamp, Threads *threads)C
{
   if(range<=0 || w()<=1)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   Image &work=((src==&dest) ? temp : dest); // this makes &work!=src
   if(!work.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copyTry'), and if not compressed then we will create the target as what we want the dest to be
   if(!src->lockRead())return false;
   if(!work.lock    (LOCK_WRITE)){src->unlock(); return false;}

   BlurContext bc(*src, clamp, threads); bc.setRange(range); bc.setX(); bc.dest=&work; bc.blur();

   work.unlock(); src->unlock();
   if(work.type()==type && work.mode()==mode && work.mipMaps()==mip_maps){work.updateMipMaps(FILTER_BEST, clamp); if(&work!=&dest)Swap(work, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
   return work.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
Bool Image::blurY(Image &dest, Flt range, Bool clamp, Threads *threads)C
{
   if(range<=0 || h()<=1)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   Image &work=((src==&dest) ? temp : dest); // this makes &work!=src
   if(!work.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copyTry'), and if not compressed then we will create the target as what we want the dest to be
   if(!src->lockRead())return false;
   if(!work.lock    (LOCK_WRITE)){src->unlock(); return false;}

   BlurContext bc(*src, clamp, threads); bc.setRange(range); bc.setY(); bc.dest=&work; bc.blur();

   work.unlock(); src->unlock();
   if(work.type()==type && work.mode()==mode && work.mipMaps()==mip_maps){work.updateMipMaps(FILTER_BEST, clamp); if(&work!=&dest)Swap(work, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
   return work.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
Bool Image::blurZ(Image &dest, Flt range, Bool clamp, Threads *threads)C
{
   if(range<=0 || d()<=1)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   Image &work=((src==&dest) ? temp : dest); // this makes &work!=src
   if(!work.createTry(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copyTry'), and if not compressed then we will create the target as what we want the dest to be
   if(!src->lockRead())return false;
   if(!work.lock    (LOCK_WRITE)){src->unlock(); return false;}

   BlurContext bc(*src, clamp, threads); bc.setRange(range); bc.setZ(); bc.dest=&work; bc.blur();

   work.unlock(); src->unlock();
   if(work.type()==type && work.mode()==mode && work.mipMaps()==mip_maps){work.updateMipMaps(FILTER_BEST, clamp); if(&work!=&dest)Swap(work, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
   return work.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
/******************************************************************************/
Image& Image::average(             C VecI &range, Bool clamp, Threads *threads) {average(T, range, clamp, threads); return T;}
Bool   Image::average(Image &dest, C VecI &range, Bool clamp, Threads *threads)C
{
   Bool blur[]={range.x>0 && w()>1, range.y>0 && h()>1, range.z>0 && d()>1};
   Int  blurs =blur[0]+blur[1]+blur[2];
   if( !blurs)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image     *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   BlurContext bc(*src, clamp, threads);
   if(src==&dest) // this also means !T.compressed && 'temp' is empty
   {
      if(blurs&1)temp.createTry    (w(), h(), d(), type, mode, mip_maps); // we will end up in 'temp' so create 'temp' as target mode and mip maps
      else       temp.createSoftTry(w(), h(), d(), type,              1); // we will end up in 'dest' so create 'temp' as SOFT
      if(!temp.is())return false;
      bc.dest=&temp;
      if(!temp.lock(LOCK_WRITE     ))return false;
      if(!dest.lock(LOCK_READ_WRITE))return false; // we read from src and write to dest, and src==dest
      // we already locked 'src' by locking 'dest'
   }else
   if(src==&temp) // this also means T.compressed && 'temp' is created
   {
      if(!dest.createSoftTry(w(), h(), d(), IMAGE_R8G8B8A8, 1))return false; // always create as soft, because T.compressed so we will have to compress it anyway
      bc.dest=&dest;
      // no need to lock, because src==temp (which is SOFT) and dest is SOFT
   }else // src!=&dest && src!=&temp && !T.compressed
   {
      if(           !dest.createTry    (w(), h(), d(), type, mode, mip_maps))return false; // we will end up in 'dest' so create it as target mode and mip maps
      if(blurs>1 && !temp.createSoftTry(w(), h(), d(), type,              1))return false; // create temp as SOFT
      bc.dest=((blurs&1) ? &dest : &temp);
      if(!src->lockRead()      )return false;
      if(!dest.lock(LOCK_WRITE))return false;
      // 'temp' is SOFT so doesn't need lock
   }

   if(blur[0]){bc.setRange(range.x); bc.setAvgX(); bc.blur(); bc.src=bc.dest; bc.dest=((bc.src==&dest) ? &temp : &dest);}
   if(blur[1]){bc.setRange(range.y); bc.setAvgY(); bc.blur(); bc.src=bc.dest; bc.dest=((bc.src==&dest) ? &temp : &dest);}
   if(blur[2]){bc.setRange(range.z); bc.setAvgZ(); bc.blur(); bc.src=bc.dest; bc.dest=((bc.src==&dest) ? &temp : &dest);}

   if(src==&dest)
   {
      temp.unlock(); // unlock in case we will Swap it
      dest.unlock();
   }else
   if(src==&temp)
   {
   }else
   {
      src->unlock();
      dest.unlock();
   }

   Image &img=ConstCast(*bc.src); // at this point 'bc.src' points to non const image ('temp' or 'dest'), so we can use 'ConstCast'
   if(img.type()==type && img.mode()==mode && img.mipMaps()==mip_maps){img.updateMipMaps(FILTER_BEST, clamp); if(&img!=&dest)Swap(img, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'img'
   return img.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
Image& Image::blur(             C Vec &range, Bool clamp, Threads *threads) {blur(T, range, clamp, threads); return T;}
Bool   Image::blur(Image &dest, C Vec &range, Bool clamp, Threads *threads)C
{
   Bool blur[]={range.x>0 && w()>1, range.y>0 && h()>1, range.z>0 && d()>1};
   Int  blurs =blur[0]+blur[1]+blur[2];
   if( !blurs)return copyTry(dest);
   IMAGE_TYPE type    =T.type   ();
   IMAGE_MODE mode    =T.mode   ();
   Int        mip_maps=T.mipMaps();
 C Image     *src=this; Image temp; if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   BlurContext bc(*src, clamp, threads);
   if(src==&dest) // this also means !T.compressed && 'temp' is empty
   {
      if(blurs&1)temp.createTry    (w(), h(), d(), type, mode, mip_maps); // we will end up in 'temp' so create 'temp' as target mode and mip maps
      else       temp.createSoftTry(w(), h(), d(), type,              1); // we will end up in 'dest' so create 'temp' as SOFT
      if(!temp.is())return false;
      bc.dest=&temp;
      if(!temp.lock(LOCK_WRITE     ))return false;
      if(!dest.lock(LOCK_READ_WRITE))return false; // we read from src and write to dest, and src==dest
      // we already locked 'src' by locking 'dest'
   }else
   if(src==&temp) // this also means T.compressed && 'temp' is created
   {
      if(!dest.createSoftTry(w(), h(), d(), IMAGE_R8G8B8A8, 1))return false; // always create as soft, because T.compressed so we will have to compress it anyway
      bc.dest=&dest;
      // no need to lock, because src==temp (which is SOFT) and dest is SOFT
   }else // src!=&dest && src!=&temp && !T.compressed
   {
      if(           !dest.createTry    (w(), h(), d(), type, mode, mip_maps))return false; // we will end up in 'dest' so create it as target mode and mip maps
      if(blurs>1 && !temp.createSoftTry(w(), h(), d(), type,              1))return false; // create temp as SOFT
      bc.dest=((blurs&1) ? &dest : &temp);
      if(!src->lockRead()      )return false;
      if(!dest.lock(LOCK_WRITE))return false;
      // 'temp' is SOFT so doesn't need lock
   }

   if(blur[0]){bc.setRange(range.x); bc.setX(); bc.blur(); bc.src=bc.dest; bc.dest=((bc.src==&dest) ? &temp : &dest);}
   if(blur[1]){bc.setRange(range.y); bc.setY(); bc.blur(); bc.src=bc.dest; bc.dest=((bc.src==&dest) ? &temp : &dest);}
   if(blur[2]){bc.setRange(range.z); bc.setZ(); bc.blur(); bc.src=bc.dest; bc.dest=((bc.src==&dest) ? &temp : &dest);}

   if(src==&dest)
   {
      temp.unlock(); // unlock in case we will Swap it
      dest.unlock();
   }else
   if(src==&temp)
   {
   }else
   {
      src->unlock();
      dest.unlock();
   }

   Image &img=ConstCast(*bc.src); // at this point 'bc.src' points to non const image ('temp' or 'dest'), so we can use 'ConstCast'
   if(img.type()==type && img.mode()==mode && img.mipMaps()==mip_maps){img.updateMipMaps(FILTER_BEST, clamp); if(&img!=&dest)Swap(img, dest); return true;} // if we have desired type mode and mip maps, then all we need is to update mip maps and possibly Swap if needed, remember that after Swap we should operate on 'dest' and not 'img'
   return img.copyTry(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, clamp);
}
/******************************************************************************/
Image& Image::sharpen(Flt power, Byte range, Bool clamp, Bool blur)
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;
   if(range && Decompress(T, type, mode, mip_maps) && lock())
   {
      Image temp(w(), h(), d(), T.type(), IMAGE_SOFT, 1); if(copySoft(temp))
      {
         if(blur)temp.blur   (range, clamp);
         else    temp.average(range, clamp);

         REPD(z, T.d())
         REPD(y, T.h())
         REPD(x, T.w())
         {
            Color col_src =     color3D(x, y, z),
                  col_blur=temp.color3D(x, y, z), c;
            c.a=Mid(col_src.a+Round((col_src.a-col_blur.a)*power), 0, 255);
            c.r=Mid(col_src.r+Round((col_src.r-col_blur.r)*power), 0, 255);
            c.g=Mid(col_src.g+Round((col_src.g-col_blur.g)*power), 0, 255);
            c.b=Mid(col_src.b+Round((col_src.b-col_blur.b)*power), 0, 255);
            color3D(x, y, z, c);
         }
      }
      unlock().updateMipMaps(FILTER_BEST, clamp);
      Compress(T, type, mode, mip_maps);
   }
   return T;
}
/******************************************************************************/
Image& Image::noise(Byte red, Byte green, Byte blue, Byte alpha)
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;
   if(red || green || blue || alpha)
      if(Decompress(T, type, mode, mip_maps) && lock())
   {
      switch(T.type())
      {
         case IMAGE_L8  :
         case IMAGE_L8A8:
         {
            Byte noise=Max(red, green, blue);
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color3D(x, y, z);
               c.r=c.g=c.b=Mid(c.r+Random(-noise, noise), 0, 255);
                       c.a=Mid(c.a+Random(-alpha, alpha), 0, 255);
               color3D(x, y, z, c);
            }
         }break;

         default:
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color3D(x, y, z);
               c.r=Mid(c.r+Random(-red  , red  ), 0, 255);
               c.g=Mid(c.g+Random(-green, green), 0, 255);
               c.b=Mid(c.b+Random(-blue , blue ), 0, 255);
               c.a=Mid(c.a+Random(-alpha, alpha), 0, 255);
               color3D(x, y, z, c);
            }
         break;
      }
      unlock().updateMipMaps();
      Compress(T, type, mode, mip_maps);
   }
   return T;
}
/******************************************************************************/
Image& Image::RGBToHSB()
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;
   if(Decompress(T, type, mode, mip_maps) && lock())
   {
      REPD(z, T.d())
      REPD(y, T.h())
      REPD(x, T.w())
      {
         Vec4 c=color3DF(x, y, z);
         c.xyz=RgbToHsb(c.xyz);
         color3DF(x, y, z, c);
      }
      unlock().updateMipMaps();
      Compress(T, type, mode, mip_maps);
   }
   return T;
}
Image& Image::HSBToRGB()
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;
   if(Decompress(T, type, mode, mip_maps) && lock())
   {
      REPD(z, T.d())
      REPD(y, T.h())
      REPD(x, T.w())
      {
         Vec4 c=color3DF(x, y, z);
         c.xyz=HsbToRgb(c.xyz);
         color3DF(x, y, z, c);
      }
      unlock().updateMipMaps();
      Compress(T, type, mode, mip_maps);
   }
   return T;
}
/******************************************************************************
Image& Image::lumFromAlphaAndLight(UInt light_dir, Flt density_factor)
{
   IMAGE_MODE mode=T.mode();
   if(type()!=IMAGE_L8A8)
   {
      Image temp;
      if(temp.createTry(w(), h(), d(), IMAGE_L8A8, mode      , mipMaps(), false)
      || temp.createTry(w(), h(), d(), IMAGE_L8A8, IMAGE_SOFT, 1        , false)) // DX10+ doesn't support L8A8 so create in software
         if(temp.lock(LOCK_WRITE))
            if(lockRead())
      {
         REPD(z, T.d())
         REPD(y, T.h())
         REPD(x, T.w())temp.color3D(x, y, z, Color(0, 0, 0, color3D(x, y, z).a));
         Swap(unlock(), temp.unlock());
      }
   }
   if(hwType()==IMAGE_L8A8)
   {
      if(lock())
      {
         Int   X=w(),
               Y=h(),
               Z=d();
         UInt _Y=pitch (),
              _Z=pitch2();
         Flt   a_mul=density_factor/255;

         if(!light_dir)
         {
            REPD(z, Z)
            REPD(y, Y)
            REPD(x, X)_data[(x*2) + y*_Y + z*_Z]=0;
         }else
         if(light_dir==(DIRF_LEFT|DIRF_RIGHT|DIRF_DOWN|DIRF_UP|DIRF_BACK|DIRF_FORWARD))
         {
            REPD(z, Z)
            REPD(y, Y)
            REPD(x, X)_data[(x*2) + y*_Y + z*_Z]=255;
         }else
         if(IsPow2(X) && IsPow2(Y) && IsPow2(Z))
         {
            UInt X1=X-1,
                 Y1=Y-1,
                 Z1=Z-1;
            switch(light_dir)
            {
               #define A(x, y, z) (((x)<<1) + (y)*_Y + (z)*_Z)
               // down
               case DIRF_LEFT |DIRF_DOWN|DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A((x+y)&X1, y, (z-y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_DOWN|DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A( x      , y, (z-y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_DOWN|DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A((x-y)&X1, y, (z-y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_DOWN             : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A((x+y)&X1, y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_DOWN             : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A( x      , y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_DOWN             : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A((x-y)&X1, y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_DOWN|DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A((x+y)&X1, y, (z+y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_DOWN|DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A( x      , y, (z+y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_DOWN|DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A((x-y)&X1, y, (z+y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               // up
               case DIRF_LEFT |DIRF_UP  |DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A((x-y)&X1, y, (z+y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_UP  |DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A( x      , y, (z+y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_UP  |DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A((x+y)&X1, y, (z+y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_UP               : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A((x-y)&X1, y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_UP               : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A( x      , y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_UP               : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A((x+y)&X1, y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_UP  |DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A((x-y)&X1, y, (z-y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_UP  |DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A( x      , y, (z-y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_UP  |DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A((x+y)&X1, y, (z-y)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               // horizontal
               case DIRF_LEFT                        : REPD(z,Z)REPD(y,Y){Flt l=255;  REPD(x,X){UInt i=A( x      , y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT           |DIRF_FORWARD: REPD(z,Z)REPD(y,Y){Flt l=255;  REPD(x,X){UInt i=A( x      , y, (z-x)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT           |DIRF_BACK   : REPD(z,Z)REPD(y,Y){Flt l=255;  REPD(x,X){UInt i=A( x      , y, (z+x)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT                       : REPD(z,Z)REPD(y,Y){Flt l=255; FREPD(x,X){UInt i=A( x      , y,  z      ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT          |DIRF_FORWARD: REPD(z,Z)REPD(y,Y){Flt l=255; FREPD(x,X){UInt i=A( x      , y, (z+x)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT          |DIRF_BACK   : REPD(z,Z)REPD(y,Y){Flt l=255; FREPD(x,X){UInt i=A( x      , y, (z-x)&Z1); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               #undef A
            }
         }else
         {
            switch(light_dir)
            {
               #define A(x, y, z) ((Mod(x,X)<<1) + (y)*_Y + Mod(z,Z)*_Z)
               case DIRF_LEFT |DIRF_DOWN|DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x+y, y, z-y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_DOWN|DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x  , y, z-y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_DOWN|DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x-y, y, z-y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_DOWN             : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x+y, y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_DOWN             : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x  , y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_DOWN             : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x-y, y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_DOWN|DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x+y, y, z+y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_DOWN|DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x  , y, z+y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_DOWN|DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255;  REPD(y,Y){UInt i=A(x-y, y, z+y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               // up
               case DIRF_LEFT |DIRF_UP  |DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x-y, y, z+y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_UP  |DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x  , y, z+y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_UP  |DIRF_FORWARD: REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x+y, y, z+y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_UP               : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x-y, y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_UP               : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x  , y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_UP               : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x+y, y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT |DIRF_UP  |DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x-y, y, z-y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case            DIRF_UP  |DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x  , y, z-y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT|DIRF_UP  |DIRF_BACK   : REPD(z,Z)REPD(x,X){Flt l=255; FREPD(y,Y){UInt i=A(x+y, y, z-y); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               // horizontal
               case DIRF_LEFT                        : REPD(z,Z)REPD(y,Y){Flt l=255;  REPD(x,X){UInt i=A(x  , y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT           |DIRF_FORWARD: REPD(z,Z)REPD(y,Y){Flt l=255;  REPD(x,X){UInt i=A(x  , y, z-x); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_LEFT           |DIRF_BACK   : REPD(z,Z)REPD(y,Y){Flt l=255;  REPD(x,X){UInt i=A(x  , y, z+x); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT                       : REPD(z,Z)REPD(y,Y){Flt l=255; FREPD(x,X){UInt i=A(x  , y, z  ); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT          |DIRF_FORWARD: REPD(z,Z)REPD(y,Y){Flt l=255; FREPD(x,X){UInt i=A(x  , y, z+x); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               case DIRF_RIGHT          |DIRF_BACK   : REPD(z,Z)REPD(y,Y){Flt l=255; FREPD(x,X){UInt i=A(x  , y, z-x); _data[i]=Round(l); l*=1-Sat(_data[i+1]*a_mul);}} break;
               #undef A
            }
         }
         unlock().updateMipMaps();
      }
      if(T.mode()!=mode)
      {
         Image temp;
         if(temp.createTry(w(), h(), d(), type(), mode, 1))
            if(temp.lock(LOCK_WRITE))
               if(lockRead())
         {
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w())temp.color3D(x, y, z, color3D(x, y, z));
            Swap(unlock(), temp.unlock());
         }
      }
   }
   return T;
}
/******************************************************************************
Image& fadeMipMaps(); // fade out mip maps, the smaller the mip map the more faded it will be
Image& Image::fadeMipMaps()
{
   if(mode()==IMAGE_2D)
   {
      Image temp;
      FREP(mipMaps())
      {
         Flt s;
         Int mip_size=Max(w()>>i, h()>>i);
         if( mip_size>=256)s=1.00f;else
         if( mip_size>=128)s=0.66f;else
         if( mip_size>= 64)s=0.33f;else
                           s=0.00f;
         if(s<1)
         {
            Bool copy=compressed(); // if the image is compressed then operate on mip extracted to copy image
            if(  copy)extractMipMap(temp, IMAGE_R8G8B8A8, IMAGE_SOFT, i);
            Image &image=(copy ? temp : T);

            if(image.lock(LOCK_READ_WRITE, copy ? 0 : i))
            {
               REPD(y, image.lh())
               REPD(x, image.lw())image.color(x, y, Lerp(Color(128, 128, 128, 128), image.color(x, y), s));
               image.unlock();

               if(copy)injectMipMap(temp, i);
            }
         }
      }
   }
   return T;
}
/******************************************************************************/
Image& Image::tile(Int range, Bool horizontally, Bool vertically)
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;

   Clamp(range, 0, Min(w(), h()));
   if(range && (horizontally || vertically) && Decompress(T, type, mode, mip_maps) && lock())
   {
   #if 1
      if(highPrecision())
      {
         const Flt mul=1.0f/(range+1);
         if(horizontally)
         FREPD(y, T.h())
         FREPD(x, range)
         {
            Vec4 c0=colorF(        x, y),
                 c1=colorF(T.w()-x-1, y);
            colorF(T.w()-x-1, y, Lerp(c0, c1, (x+1)*mul));
         }

         if(vertically)
         FREPD(x, T.w())
         FREPD(y, range)
         {
            Vec4 c0=colorF(x,         y),
                 c1=colorF(x, T.h()-y-1);
            colorF(x, T.h()-y-1, Lerp(c0, c1, (y+1)*mul));
         }
      }else
      {
         const UInt div=range+1, div_2=(div+1)/2;
         if(horizontally)
         FREPD(y, T.h())
         FREPD(x, range)
         {
            UInt  s0=range-x,
                  s1=x+1;
            Color c0=color(        x, y),
                  c1=color(T.w()-x-1, y), c;
            c.r=(c0.r*s0 + c1.r*s1 + div_2)/div;
            c.g=(c0.g*s0 + c1.g*s1 + div_2)/div;
            c.b=(c0.b*s0 + c1.b*s1 + div_2)/div;
            c.a=(c0.a*s0 + c1.a*s1 + div_2)/div;
            color(T.w()-x-1, y, c);
         }

         if(vertically)
         FREPD(x, T.w())
         FREPD(y, range)
         {
            UInt  s0=range-y,
                  s1=y+1;
            Color c0=color(x,         y),
                  c1=color(x, T.h()-y-1), c;
            c.r=(c0.r*s0 + c1.r*s1 + div_2)/div;
            c.g=(c0.g*s0 + c1.g*s1 + div_2)/div;
            c.b=(c0.b*s0 + c1.b*s1 + div_2)/div;
            c.a=(c0.a*s0 + c1.a*s1 + div_2)/div;
            color(x, T.h()-y-1, c);
         }
      }
   #else
      if(horizontally)
      FREPD(y, T.h())
      FREPD(x, range)
      {
         UInt  s0   =x+1,
               s1   =range-x,
               sum  =s0+s1,
               sum_2=(sum+1)/2;
         Color c0   =color(            x, y),
               c1   =color(T.w()-range+x, y), c;
         c.a=(c0.a*s0 + c1.a*s1 + sum_2)/sum;
         c.r=(c0.r*s0 + c1.r*s1 + sum_2)/sum;
         c.g=(c0.g*s0 + c1.g*s1 + sum_2)/sum;
         c.b=(c0.b*s0 + c1.b*s1 + sum_2)/sum;
         color(T.w()-range+x, y, c);
      }

      if(vertically)
      FREPD(x, T.w())
      FREPD(y, range)
      {
         UInt  s0   =y+1,
               s1   =range-y,
               sum  =s0+s1,
               sum_2=(sum+1)/2;
         Color c0   =color(x,             y),
               c1   =color(x, T.h()-range+y), c;
         c.a=(c0.a*s0 + c1.a*s1 + sum_2)/sum;
         c.r=(c0.r*s0 + c1.r*s1 + sum_2)/sum;
         c.g=(c0.g*s0 + c1.g*s1 + sum_2)/sum;
         c.b=(c0.b*s0 + c1.b*s1 + sum_2)/sum;
         color(x, T.h()-range+y, c);
      }
   #endif
      unlock().updateMipMaps(FILTER_BEST, false);
      Compress(T, type, mode, mip_maps);
   }
   return T;
}
/******************************************************************************/
// MIN / MAX
/******************************************************************************/
static Flt Median(Long (&v)[256], Long p, Bool exact)
{
   Long n=0; FREPA(v)
   {
      n+=v[i]; if(n>p)
      {
         if(exact || n>p+1)return i/255.0f; // if exact, or the next value is still the same value (we use histogram, so we can just check if from "v[i]" we've accumulated 1 more than needed, this is OK)
         // find next value
         Int j=i+1; for(; j<255 && !v[j]; j++); // 255 instead of 256, because we always want to stop on the last element
         return (i+j)/(255.0f*2); // return average of 'i' and 'j'
      }
   }
   return 1;
}
Bool Image::stats(Vec4 *min, Vec4 *max, Vec4 *avg, Vec4 *med, Vec4 *mod, Vec *avg_alpha_weight, C BoxI *box_ptr)C
{
   if(!min && !max && !avg && !med && !mod && !avg_alpha_weight)return true; // nothing to calculate
   if(is())
   {
    C Image *src=this; Image temp;
      if(compressed())
         if(copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else goto error;
      if(src->lockRead())
      {
         VecD4 sum=0, sum_alpha_weight=0;
         Vec4 _avg, _med;
         Long  r[256], g[256], b[256], a[256]; // channel 8-bit histogram
         Flt  *rf, *gf, *bf, *af;
         Memt<Flt> hist; // channels are listed separately, all reds, followed by all greens, followed by all blues, followed by all alphas
         if(mod) // to calculate mode, we need to have average and median
         { // if not specified then operate on temp
            if(!avg)avg=&_avg;
            if(!med)med=&_med;
         }
         if(min)*min= FLT_MAX;
         if(max)*max=-FLT_MAX;
         Bool med_fast=!src->highPrecision(); // calculate median using fast method (8-bit histogram)
         BoxI box(0, src->size3()); if(box_ptr)box&=*box_ptr;
         Long pixels=box.volumeL();
         if(med)
         {
            if(med_fast)
            {
               Zero(r); Zero(g); Zero(b); Zero(a);
            }else
            {
               hist.setNum(pixels*4); // 4 channels
               rf=hist.data();
               gf=rf+pixels;
               bf=gf+pixels;
               af=bf+pixels;
            }
         }

         for(Int z=box.min.z; z<box.max.z; z++)
         for(Int y=box.min.y; y<box.max.y; y++)
         for(Int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 color=src->color3DF(x, y, z);
            if(min)*min =Min(*min, color);
            if(max)*max =Max(*max, color);
            if(avg) sum+=VecD4(color);
            if(avg_alpha_weight){sum_alpha_weight.xyz+=color.xyz*color.w; sum_alpha_weight.w+=color.w;}
            if(med)
            {
               if(med_fast)
               {
                  r[FltToByte(color.x)]++;
                  g[FltToByte(color.y)]++;
                  b[FltToByte(color.z)]++;
                  a[FltToByte(color.w)]++;
               }else
               {
                  *rf++=color.x;
                  *gf++=color.y;
                  *bf++=color.z;
                  *af++=color.w;
               }
            }
         }

         src->unlock();

         if(avg)*avg=sum/Dbl(pixels);
         if(avg_alpha_weight){if(sum_alpha_weight.w)sum_alpha_weight.xyz/=sum_alpha_weight.w; *avg_alpha_weight=sum_alpha_weight.xyz;}
         if(med)
         {
            Long p=Unsigned(pixels)/2;
            Bool exact=pixels&1;
            if(med_fast)
            {
               p-=!exact; // for not exact (average) we need to stop at 1 before, and average it with the next one, alternatively we could change 'Median' to find previous and not next value, however that would require 1 more iteration, so it's faster to just do this 1 time, instead of 1 iteration in each call
               med->set(Median(r, p, exact), Median(g, p, exact), Median(b, p, exact), Median(a, p, exact));
            }else
            {
               // go back at the start of the array, and sort
               rf-=pixels; Sort(rf, pixels);
               gf-=pixels; Sort(gf, pixels);
               bf-=pixels; Sort(bf, pixels);
               af-=pixels; Sort(af, pixels);
               if(exact)med->set(rf[p], gf[p], bf[p], af[p]);else
               {
                  Long q=p-1; med->set(Avg(rf[q], rf[p]), Avg(gf[q], gf[p]), Avg(bf[q], bf[p]), Avg(af[q], af[p]));
               }
            }
         }
         if(mod)*mod=3*(*med) - 2*(*avg); // Mode = 3*Median - 2*Mean

         return true;
      }
   }

error:
   if(min)min->zero();
   if(max)max->zero();
   if(avg)avg->zero();
   if(med)med->zero();
   if(mod)mod->zero();
   if(avg_alpha_weight)avg_alpha_weight->zero();
   return false;
}
Bool Image::statsSat(Flt *min, Flt *max, Flt *avg, Flt *med, Flt *mod, Flt *avg_alpha_weight, C BoxI *box_ptr)C
{
   if(!min && !max && !avg && !med && !mod && !avg_alpha_weight)return true; // nothing to calculate
   if(is())
   {
    C Image *src=this; Image temp;
      if(compressed())
         if(copyTry(temp, -1, -1, -1, IMAGE_R8G8B8, IMAGE_SOFT, 1))src=&temp;else goto error;
      if(src->lockRead())
      {
         Dbl  sum=0; Vec2 sum_alpha_weight=0;
         Flt _avg, _med;
         Long v[256]; // 8-bit histogram
         Flt *vf;
         Memt<Flt> hist;
         if(mod) // to calculate mode, we need to have average and median
         { // if not specified then operate on temp
            if(!avg)avg=&_avg;
            if(!med)med=&_med;
         }
         if(min)*min= FLT_MAX;
         if(max)*max=-FLT_MAX;
         Bool med_fast=!src->highPrecision(); // calculate median using fast method (8-bit histogram)
         BoxI box(0, src->size3()); if(box_ptr)box&=*box_ptr;
         Long pixels=box.volumeL();
         if(med)
         {
            if(med_fast)Zero(v);else vf=hist.setNum(pixels).data();
         }

         for(Int z=box.min.z; z<box.max.z; z++)
         for(Int y=box.min.y; y<box.max.y; y++)
         for(Int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=src->color3DF(x, y, z);
            Flt  s=RgbToHsb(c.xyz).y;
            if(min)MIN(*min, s);
            if(max)MAX(*max, s);
            if(avg)sum+=s;
            if(avg_alpha_weight){sum_alpha_weight.x+=s*c.w; sum_alpha_weight.y+=c.w;}
            if(med)
            {
               if(med_fast)v[FltToByte(s)]++;
               else       *vf++=s;
            }
         }

         src->unlock();

         if(avg)*avg=sum/Dbl(pixels);
         if(avg_alpha_weight){if(sum_alpha_weight.y)sum_alpha_weight.x/=sum_alpha_weight.y; *avg_alpha_weight=sum_alpha_weight.x;}
         if(med)
         {
            Long p=Unsigned(pixels)/2;
            Bool exact=pixels&1;
            if(med_fast)
            {
               p-=!exact; // for not exact (average) we need to stop at 1 before, and average it with the next one, alternatively we could change 'Median' to find previous and not next value, however that would require 1 more iteration, so it's faster to just do this 1 time, instead of 1 iteration in each call
              *med=Median(v, p, exact);
            }else
            {
               // go back at the start of the array, and sort
               vf-=pixels; Sort(vf, pixels);
              *med=(exact ? vf[p] : Avg(vf[p-1], vf[p]));
            }
         }
         if(mod)*mod=3*(*med) - 2*(*avg); // Mode = 3*Median - 2*Mean

         return true;
      }
   }

error:
   if(min)*min=0;
   if(max)*max=0;
   if(avg)*avg=0;
   if(med)*med=0;
   if(mod)*mod=0;
   if(avg_alpha_weight)*avg_alpha_weight=0;
   return false;
}
/******************************************************************************/
Bool Image::monochromatic()C
{
   if(ImageTI[type()].channels==1 || type()==IMAGE_L8A8)return true;

 C Image *src =this; Image temp; if(compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return false;
   Bool   mono=true;
   if(src->lockRead())
   {
      REPD(z, src->ld())
      REPD(y, src->lh())
      REPD(x, src->lw())
      {
         Color c=src->color3D(x, y, z); if(c.r!=c.g || c.r!=c.b){mono=false; goto stop;}
      }
   stop:
      src->unlock();
   }
   return mono;
}
/******************************************************************************/
Image& Image::minimum(Flt distance)
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;

   SAT(distance);
   if (distance>EPS && Decompress(T, type, mode, mip_maps))
   {
      Image temp(w(), h(), 1, T.type(), T.mode(), (T.type()!=type || T.mode()!=mode) ? 1 : mip_maps);
      if(temp.lock(LOCK_WRITE))
      {
         if(lockRead())
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Vec4 c=T.colorF(x, y);
               for(Int sy=-1; sy<=1; sy++)if(InRange(y+sy, T.h()))
               for(Int sx=-1; sx<=1; sx++)if(InRange(x+sx, T.w()) && (sx || sy))
               {
                  Vec2 o(sx, sy); o.setLength(distance);
                  Vec4 t=colorFCubicFast(x+o.x, y+o.y, true);
                  MIN(c.x, t.x);
                  MIN(c.y, t.y);
                  MIN(c.z, t.z);
                  MIN(c.w, t.w);
               }
               temp.colorF(x, y, c);
            }
            unlock();
         }
         temp.unlock();
      }
      Swap(temp.updateMipMaps(), T);
      Compress(T, type, mode, mip_maps);
   }
   return T;
}
/******************************************************************************/
Image& Image::maximum(Flt distance)
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;

   SAT(distance);
   if (distance>EPS && Decompress(T, type, mode, mip_maps))
   {
      Image temp(w(), h(), 1, T.type(), T.mode(), (T.type()!=type || T.mode()!=mode) ? 1 : mip_maps);
      if(temp.lock(LOCK_WRITE))
      {
         if(lockRead())
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Vec4 c=T.colorF(x, y);
               for(Int sy=-1; sy<=1; sy++)if(InRange(y+sy, T.h()))
               for(Int sx=-1; sx<=1; sx++)if(InRange(x+sx, T.w()) && (sx || sy))
               {
                  Vec2 o(sx, sy); o.setLength(distance);
                  Vec4 t=T.colorFLinear(x+o.x, y+o.y, true);
                  MAX(c.x, t.x);
                  MAX(c.y, t.y);
                  MAX(c.z, t.z);
                  MAX(c.w, t.w);
               }
               temp.colorF(x, y, c);
            }
            unlock();
         }
         temp.unlock();
      }
      Swap(temp.updateMipMaps(), T);
      Compress(T, type, mode, mip_maps);
   }
   return T;
}
/******************************************************************************/
Image& Image::transparentToNeighbor(Bool clamp, Flt step)
{
#if 1 // new method
   Int    mips=TotalMipMaps(w(), h(), d(), IMAGE_F32_4); if(mips<=1)return T;//true;
   Image *src =this, temp; if(!src->highPrecision() || src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_F32_4, IMAGE_SOFT, 1))src=&temp;else return T;//false; // first we have to copy to IMAGE_F32_4 to make sure we have floating point, so that downsizing will not use ALPHA_LIMIT, this is absolutely critical
   Bool   ok  =false;
   if(src->lock())
   {
      Memt<Image> mip; mip.setNum(mips-1);
      Image *s=src; VecI size=s->size3();
      SAT(step); Bool lerp=(step!=1);
      FREPA(mip)
      {
         size>>=1;
         if(!s->copyTry(mip[i], size.x, size.y, size.z, IMAGE_F32_4, IMAGE_SOFT, 1, FILTER_CUBIC_FAST_SMOOTH, clamp, true))goto error; // we need a non-sharpening filter and one that spreads in all directions (more than 2x2 samples)
         s=&mip[i];
      }

      REPD(y, h())
      REPD(x, w())
      {
         Vec4 s=src->colorF(x, y); if(!s.w)FREPA(mip) // if transparent, then iterate all mip maps starting from the biggest
         {
            Image &m=mip[i];
            Vec2 x_mul_add; x_mul_add.x=Flt(m.w())/w(); x_mul_add.y=x_mul_add.x*0.5f-0.5f;
            Vec2 y_mul_add; y_mul_add.x=Flt(m.h())/h(); y_mul_add.y=y_mul_add.x*0.5f-0.5f;
          //Vec2 z_mul_add; z_mul_add.x=Flt(m.d())/d(); z_mul_add.y=z_mul_add.x*0.5f-0.5f;

            Vec4 c=m.colorFLinearTTNF32_4(x*x_mul_add.x + x_mul_add.y, y*y_mul_add.x + y_mul_add.y, clamp);
          //Vec4 c=m.colorFCubicFastSmooth(x*x_mul_add.x + x_mul_add.y, y*y_mul_add.x + y_mul_add.y, clamp);
            if(c.w) // if we've found some valid color value
            {
               c.w=0; // remember to clear alpha to zero to preserve original transparency, but we keep the new RGB values
               if(lerp)c.xyz=Lerp(s.xyz, c.xyz, step);
               src->colorF(x, y, c);
               break; // finished looking for a sample
            }
         }
      }
      ok=true;
   error:;
      src->unlock();
      if(ok)if(src==this)src->updateMipMaps(FILTER_BEST, clamp, true);else ok=src->copyTry(T, -1, -1, -1, type(), mode(), mipMaps(), FILTER_BEST, clamp, true);
   }
   return T;//ok;
#else // old method
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;

   if(Decompress(T, type, mode, mip_maps))
   {
      if(lock())
      {
         Image used(w(), h(), 1, IMAGE_I8, IMAGE_SOFT, 1); used.clear();
         Memt<VecI2> opn, opn_next; // coordinates of transparent pixel which has opaque neighbor
         // iterate all pixels
         REPD(y, T.h())
         REPD(x, T.w())if(color(x, y).a)used.pixB(x, y)=1;else
         {
            // iterate all neighbors
            for(Int sy=y-1; sy<=y+1; sy++)if(InRange(sy, T.h()))
            for(Int sx=x-1; sx<=x+1; sx++)if(InRange(sx, T.w()))if(color(sx, sy).a) // if at least one neighbor has alpha
            {
               used.pixB(x, y)=2; opn.New().set(x, y); goto added;
            }
            added:;
         }
         for(Byte prev_step=1, cur_step=2; opn.elms(); )
         {
            Byte next_step=(cur_step+1)&0xFF; if(!next_step)next_step=1; // set next step, and make sure to skip '0' which is used for "not yet set"
            REPA(opn)
            {
               VecI2 p=opn[i]; opn.removeLast();
               VecI4 sum=0;
               for(Int sy=p.y-1; sy<=p.y+1; sy++)if(InRange(sy, T.h()))
               for(Int sx=p.x-1; sx<=p.x+1; sx++)if(InRange(sx, T.w()))
               {
                  Byte &u=used.pixB(sx, sy);
                  if(u==prev_step) // was set in previous frame
                  {
                     Color c=T.color(sx, sy);
                     Int   a=c.a+1; if(sy==p.y || sx==p.x)a*=2; // make horizontal/vertical neighbors 2x more significant, and thus making corner neighbors 2x less
                     sum.x+=c.r*a;
                     sum.y+=c.g*a;
                     sum.z+=c.b*a;
                     sum.w+=    a;
                  }else
                  if(!u) // if not yet added
                  {
                     u=next_step; // set as already added
                     opn_next.New().set(sx, sy); // add to next list of pixels
                  }
               }
               Int h=sum.w/2; // half
               T.color(p.x, p.y, sum.w ? Color((sum.x+h)/sum.w, (sum.y+h)/sum.w, (sum.z+h)/sum.w, 0) : TRANSPARENT);
            }
            prev_step=cur_step; cur_step=next_step;
            Swap(opn, opn_next);
         }
         unlock().updateMipMaps();
      }
      Compress(T, type, mode, mip_maps);
   }
   return T;
#endif
}
static VecI2 move[8]=
{
   VecI2( 0, 1),
   VecI2( 0,-1),
   VecI2( 1, 0),
   VecI2(-1, 0),

   VecI2( 1, 1),
   VecI2( 1,-1),
   VecI2(-1, 1),
   VecI2(-1,-1),
};
Bool Image::getSameColorNeighbors(Int x, Int y, MemPtr<VecI2> pixels, Bool diagonal)C
{
   pixels.clear();
   if(InRange(x, w())
   && InRange(y, h()))
   {
    C Image *src=this;
      Image  temp;
      if(compressed()){if(!copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))return false; src=&temp;}
      if(src->lockRead())
      {
         const Int moves=(diagonal ? 8 : 4);
         Image used(w(), h(), 1, IMAGE_I8, IMAGE_SOFT, 1); used.clear().pixel(x, y, 1);
         Color color=src->color(x, y);
         pixels.add(VecI2(x, y));
         FREPAD(processed, pixels)
         {
            VecI2 pos=pixels[processed]; // don't use reference because 'pixels' may get modified later
            REP(moves)
            {
               VecI2 next=pos+move[i];
               if(InRange(next.x, w())
               && InRange(next.y, h()))
               {
                  Byte &b=used.pixB(next.x, next.y); if(!b)
                  {
                     b=1;
                     if(src->color(next.x, next.y)==color)pixels.add(next);
                  }
               }
            }
         }
         src->unlock();
         return true;
      }
   }
   return false;
}
Image& Image::fill(Int x, Int y, C Color &color, Bool diagonal)
{
   if(InRange(x, w())
   && InRange(y, h()))
   {
      IMAGE_TYPE type;
      IMAGE_MODE mode;
      Int        mip_maps;

      if(Decompress(T, type, mode, mip_maps))
      {
         if(lock())
         {
            const Int moves=(diagonal ? 8 : 4);
            Image used(w(), h(), 1, IMAGE_I8, IMAGE_SOFT, 1); used.clear().pixel(x, y, 1);
            Color src=T.color(x, y);
            Memt<VecI2> pixels; for(pixels.add(VecI2(x, y)); pixels.elms(); )
            {
               VecI2 pos=pixels.pop();
               T.color(pos.x, pos.y, color);
               REP(moves)
               {
                  VecI2 next=pos+move[i];
                  if(InRange(next.x, w())
                  && InRange(next.y, h()))
                  {
                     Byte &b=used.pixB(next.x, next.y); if(!b)
                     {
                        b=1;
                        if(T.color(next.x, next.y)==src)pixels.add(next);
                     }
                  }
               }
            }
            unlock().updateMipMaps();
         }
         Compress(T, type, mode, mip_maps);
      }
   }
   return T;
}
/******************************************************************************/
// SHADOWS
/******************************************************************************/
Image& Image::createShadow(C Image &src, Int blur, Flt shadow_opacity, Flt shadow_spread, Bool border_padd)
{
   MAX(blur, 0);
   Clamp(shadow_opacity, 0, 1);
   Clamp(shadow_spread , 0, 1); shadow_spread=1-shadow_spread; if(shadow_spread)shadow_spread=1/shadow_spread;
   Int padd=blur+border_padd;

 C Image *s=&src;
   Image decompressed; if(s->compressed())if(s->copyTry(decompressed, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))s=&decompressed;else return T;
   Image temp(s->w()+padd*2, s->h()+padd*2, 1, IMAGE_A8, IMAGE_SOFT, 1);

   // copy
   if(s->lockRead())
   {
      FREPD(y, temp.h())
      FREPD(x, temp.w())temp.pixB(x, y)=s->color(x-padd, y-padd).a;
      s->unlock();
   }

   // blur
   temp.blur(blur, true);

   // normalize
   Int max=0; REPD(y, temp.h())REPD(x, temp.w())MAX(max, temp.pixB(x, y));
   if( max   )REPD(y, temp.h())REPD(x, temp.w())
   {
      Flt s=Flt(temp.pixB(x, y))/max*shadow_opacity;
      if(shadow_spread)s*=shadow_spread;else if(s)s=1;
      MIN(s, shadow_opacity);
      temp.pixB(x, y)=RoundPos(s*255);
   }

   Swap(T, temp); return T;
}
Image& Image::applyShadow(C Image &shadow, C Color &shadow_color, C VecI2 &offset, Int image_type, Bool combine)
{
   IMAGE_TYPE type;
   IMAGE_MODE mode;
   Int        mip_maps;

   if(shadow_color.a && Decompress(T, type, mode, mip_maps))
   {
      Int l=Max(0, -offset.x),
          u=Max(0, -offset.y);
      if( image_type<0)image_type=type; // get original
      if(!image_type  ) // auto-detect
      {
         if(ImageTI[type].a)image_type=type;else // if original type has alpha channel then use it
            switch(type) // if not then manually specify the type
         {
            case IMAGE_A8 :
            case IMAGE_L8 :
            case IMAGE_I8 :
            case IMAGE_I16:
            case IMAGE_I24:
            case IMAGE_I32: image_type=IMAGE_L8A8    ; break;
            default       : image_type=IMAGE_R8G8B8A8; break;
         }
      }
      if(!ImageTI[image_type].a)return T; // no alpha available

      Image temp(Max(w(), shadow.w()+offset.x)-Min(0, offset.x), Max(h(), shadow.h()+offset.y)-Min(0, offset.y), 1, ImageTI[image_type].compressed ? IMAGE_R8G8B8A8 : IMAGE_TYPE(image_type), IMAGE_SOFT, 1);
      if(shadow.lockRead())
      {
         if(lockRead())
         {
            Color sc=shadow_color;
            FREPD(y, temp.h())
            FREPD(x, temp.w())
            {
               Color col=       color(x-l         , y-u         );
               Byte  shd=shadow.pixel(x-l-offset.x, y-u-offset.y); sc.a=(shadow_color.a*shd+128)/255;
               if(combine)temp.color(x, y, Blend(sc, col));else
               {
                  col.a=sc.a;
                  temp.color(x, y, col);
               }
            }
            unlock();
         }
         shadow.unlock();
      }
      temp.copyTry(temp, -1, -1, -1, IMAGE_TYPE(image_type), mode, mip_maps);
      Swap(T, temp);
   }
   return T;
}
Image& Image::setShadow(Int blur, Flt shadow_opacity, Flt shadow_spread, Bool border_padd, C VecI2 &offset, Int image_type, Bool combine)
{
   if(shadow_opacity>0)applyShadow(Image().createShadow(T, blur, shadow_opacity, shadow_spread, border_padd), BLACK, offset-blur-border_padd, image_type, combine);
   return T;
}
/******************************************************************************/
Bool Image::raycast(C Vec &start, C Vec &move, C Matrix *image_matrix, Flt *hit_frac, Vec *hit_pos, Flt precision)C
{
   Bool hit=false;
   Vec  s=start, m=move;
   if(image_matrix)
   {
      s/=*image_matrix;
      m/= image_matrix->orn();
   }
   Rect rect(0, 0, 1, 1);
   Flt  frac_start, frac_end;
   if(SweepPointRect( s.xy      ,  m.xy, rect, &frac_start)
   && SweepPointRect((s.xy+m.xy), -m.xy, rect, &frac_end  )) // 'frac_end' is frac from end position towards start, "1-frac_end" is frac from start towards end
      if(lockRead())
   {
      Int w1=w()-1, h1=h()-1;
      s.x*=w1; m.x*=w1;
      s.y*=h1; m.y*=h1;

      Vec pos   =s+frac_start*m           , // start clipped
          m_clip=m*(1-frac_end-frac_start); // move  clipped
      Int steps =Max(1, RoundPos(m_clip.length()*precision)); m_clip/=steps;
      Flt image_pos=pixelFLinear(pos.x, pos.y);
      REP(steps)
      {
         Vec next=pos+m_clip;
         Flt image_next=pixelFLinear(next.x, next.y);
         if(pos.z>=image_pos && next.z<=image_next  // current is above and next is below
         || pos.z<=image_pos && next.z>=image_next) // current is below and next is above
         {
            hit=true;
            if(hit_frac || hit_pos)
            {
               // X    Y1        Y2
               // 0  pos .z  image_pos
               // 1  next.z  image_next

               // Y1(x) = pos.z     +  m_clip.z             *x
               // Y2(x) = image_pos + (image_next-image_pos)*x

               // Y1(x) = Y2(x)
               // pos.z + m_clip.z*x = image_pos + (image_next-image_pos)*x
               // (m_clip.z-(image_next-image_pos))*x = image_pos - pos.z
               // (m_clip.z+image_pos-image_next)*x = image_pos - pos.z
               // x = image_pos - pos.z / (m_clip.z+image_pos-image_next)
               if(Flt div=m_clip.z+image_pos-image_next)
               {
                  Flt x=(image_pos-pos.z)/div;
                  pos+=m_clip*x;
               }

               Int maxi=Abs(m).maxI();
               Flt frac=m.c[maxi]; if(frac)frac=(pos.c[maxi]-s.c[maxi])/frac;
               if(hit_frac)*hit_frac=frac;
               if(hit_pos )*hit_pos =start+move*frac;
            }
            break;
         }
         pos=next;
         image_pos=image_next;
      }
      unlock();
   }
   return hit;
}
/******************************************************************************
void normalToBump(Image &dest, Bool high_quality); // convert normal map to bump map

static inline Flt DX(Vec nrm, Bool rescale)
{
   if(rescale)nrm=nrm*(255.0f/127)-128.0f/127.0f; // (nrm*255-128)/127
   else       nrm=nrm*2-1;
   Vec2 n2(nrm.z, -nrm.x);
   n2.y/=Max(0.1f, Abs(n2.x));
   return n2.y;
}
static inline Flt DY(Vec nrm, Bool rescale)
{
   if(rescale)nrm=nrm*(255.0f/127)-128.0f/127.0f; // (nrm*255-128)/127
   else       nrm=nrm*2-1;
   Vec2 n2(nrm.z, -nrm.y);
   n2.y/=Max(0.1f, Abs(n2.x));
   return n2.y;
}
void Image::normalToBump(Image &dest, Bool high_quality)
{
   Bool high=true;
   // obliczaj lokalnie w blokach (przykladowo dla kazdego 8x8 obliczaj lokalna heightmape)
   // roznice w poziomach pomiedzy blokami obliczaj na podstawie sredniej roznicy wspolnych pikseli (krawedzi)
   // roznice pomiedzy blokami nalezy obliczac od srodka obrazka, promieniscie na zewnatrz (jaka rozchodzaca sie fala)
   if(high_quality)
   {
      if(lockRead())
      {
         Image delta, bump, temp;
         if(delta.createTry(_x, _y, 1, IMAGE_F32_2, IMAGE_SOFT, 1))
         if(bump .createTry(_x, _y, 1, IMAGE_F32_2, IMAGE_SOFT, 1))
         if(temp .createTry(_x, _y, 1, IMAGE_F32  , IMAGE_SOFT, 1))
         {
            // set bump delta's from normals
            Bool rescale=(bytePP()<=4); // rescale imprecisions of integer types resulting in 128/255 != 0.5
            REPD(y, T._y)
            REPD(x, T._x)
            {
               Vec nrm=colorF(x, y);
               delta.pixF2(x, y).set(DX(nrm, rescale), DY(nrm, rescale));
            }

            // create blended temp
            temp.clear();
            Int res  =Max(1, Min(64, T._x, T._y)),
                blend=(res>>1);
            for(Int my=0; my<T._y; my+=res)
            for(Int mx=0; mx<T._x; mx+=res)
            {
               bump.pixF2(mx, my)=0; // set starting heights to zero

               // fill horizontal middle line
               for(Int x=mx-1; x>=0; x--)bump.pixF2(x, my).x=bump.pixF2(x+1, my).x-delta.pixF2(x, my).x;
               for(Int x=mx+1; x<_x; x++)bump.pixF2(x, my).x=bump.pixF2(x-1, my).x+delta.pixF2(x, my).x;

               // fill vertical lines from source horizontal middle line
               REPD(x, T._x)
               {
                  for(Int y=my-1; y>=0; y--)bump.pixF2(x, y).x=bump.pixF2(x, y+1).x-delta.pixF2(x, y).y;
                  for(Int y=my+1; y<_y; y++)bump.pixF2(x, y).x=bump.pixF2(x, y-1).x+delta.pixF2(x, y).y;
               }
               
               // copy vertical middle line to Y channel
               REPD(y, T._y){Vec2 &b=bump.pixF2(mx, y); b.y=b.x;}
               
               // fill horizontal lines from source vertical middle line
               REPD(y, T._y)
               {
                  for(Int x=mx-1; x>=0; x--)bump.pixF2(x, y).y=bump.pixF2(x+1, y).y-delta.pixF2(x, y).x;
                  for(Int x=mx+1; x<_x; x++)bump.pixF2(x, y).y=bump.pixF2(x-1, y).y+delta.pixF2(x, y).x;
               }

               // merge X and Y channels
               REPD(y, T._y)
               REPD(x, T._x){Vec2 &b=bump.pixF2(x, y); b.x+=b.y;}
               
               Vec4 min, max; bump.getMinMax(min, max);
               Flt  mul, add; if(min.x==max.x){mul=1; add=0;}else{mul=1.0f/(max.x-min.x); add=-min.x*mul;}

               Int y_from=Max(0, my-blend), y_to=Min(my+res+blend, T._y), y_min_blend=(my ? (my+blend) : 0), y_max_blend=((my+res<T._y) ? (my+res-blend-1) : _y),
                   x_from=Max(0, mx-blend), x_to=Min(mx+res+blend, T._x), x_min_blend=(mx ? (mx+blend) : 0), x_max_blend=((mx+res<T._x) ? (mx+res-blend-1) : _x);
               for(Int y=y_from; y<y_to; y++)
               for(Int x=x_from; x<x_to; x++)
               {
                  Flt b=1;
                  if(blend)
                  {
                     if(x<x_min_blend)b*=Flt(blend*2+1 + x-x_min_blend)/(blend*2+1);else
                     if(x>x_max_blend)b*=Flt(blend*2+1 + x_max_blend-x)/(blend*2+1);

                     if(y<y_min_blend)b*=Flt(blend*2+1 + y-y_min_blend)/(blend*2+1);else
                     if(y>y_max_blend)b*=Flt(blend*2+1 + y_max_blend-y)/(blend*2+1);
                  }
                  temp.pixF(x, y)+=(bump.pixF2(x, y).x*mul + add)*b;
               }
            }

            // create destination
            if(dest.createTry(_x, _y, 1, IMAGE_L8, mode()))
            if(dest.lock())
            {
               REPD(y, _y)
               REPD(x, _x)dest.pixB(x, y)=Round(Sat(temp.pixF(x, y))*255);
               dest.unlock();
            }
         }
         unlock();
      }
   }else
   {
      if(lockRead())
      {
         Image bump;
         if(   bump.createTry(_x, _y, 1, IMAGE_F32_2, IMAGE_SOFT, 1))
         {
            Bool rescale=(bytePP()<=4); // rescale imprecisions of integer types resulting in 128/255 != 0.5
            Int  mx     =(T._x>>1), // middle x
                 my     =(T._y>>1); // middle y
            bump.pixF2(mx, my)=0;    // set starting heights to zero

            // fill horizontal middle line
            for(Int x=mx-1; x>=0; x--)bump.pixF2(x, my).x=bump.pixF2(x+1, my).x-DX(colorF(x, my), rescale);
            for(Int x=mx+1; x<_x; x++)bump.pixF2(x, my).x=bump.pixF2(x-1, my).x+DX(colorF(x, my), rescale);

            // fill vertical lines from source horizontal middle line
            REPD(x, T._x)
            {
               for(Int y=my-1; y>=0; y--)bump.pixF2(x, y).x=bump.pixF2(x, y+1).x-DY(colorF(x, y), rescale);
               for(Int y=my+1; y<_y; y++)bump.pixF2(x, y).x=bump.pixF2(x, y-1).x+DY(colorF(x, y), rescale);
            }
            
            // copy vertical middle line to Y channel
            REPD(y, T._y){Vec2 &b=bump.pixF2(mx, y); b.y=b.x;}
            
            // fill horizontal lines from source vertical middle line
            REPD(y, T._y)
            {
               for(Int x=mx-1; x>=0; x--)bump.pixF2(x, y).y=bump.pixF2(x+1, y).y-DX(colorF(x, y), rescale);
               for(Int x=mx+1; x<_x; x++)bump.pixF2(x, y).y=bump.pixF2(x-1, y).y+DX(colorF(x, y), rescale);
            }

            // merge X and Y channels
            REPD(y, T._y)
            REPD(x, T._x){Vec2 &b=bump.pixF2(x, y); b.x+=b.y;}

            bump.normalize();

            // create dest
            dest.create(T._x, T._y, 1, IMAGE_L8, mode(), 1);
            if(dest.lock())
            {
               REPD(y, T._y)
               REPD(x, T._x)dest.pixB(x, y)=Round(bump.pixF2(x, y).x*255);
               dest.unlock();
            }
         }
         unlock();
      }
   }
}
/******************************************************************************/
static Bool CanDecompress(IMAGE_TYPE type)
{
   return type!=IMAGE_PVRTC1_2 && type!=IMAGE_PVRTC1_4;
}
void (*DecompressBlock(IMAGE_TYPE type))(C Byte *b, Color (&block)[4][4])
{
   switch(type)
   {
      default           : return null;
      case IMAGE_BC1    : return DecompressBlockBC1   ; break;
      case IMAGE_BC2    : return DecompressBlockBC2   ; break;
      case IMAGE_BC3    : return DecompressBlockBC3   ; break;
      case IMAGE_BC7    : return DecompressBlockBC7   ; break;
      case IMAGE_ETC1   : return DecompressBlockETC1  ; break;
      case IMAGE_ETC2   : return DecompressBlockETC2  ; break;
      case IMAGE_ETC2_A1: return DecompressBlockETC2A1; break;
      case IMAGE_ETC2_A8: return DecompressBlockETC2A8; break;
   }
}
Bool ImageCompare::compare(C Image &a, C Image &b, Flt similar_dif, Bool alpha_weight, Int a_mip, Flt skip_dif)
{
   // clear
   skipped =false;
   max_dif =0;
   avg_dif =0;
   avg_dif2=0;
   similar =0;
   psnr    =0;

   Bool ok=false;
   if(InRange(a_mip, a.mipMaps())) // if 'a' has requested mip map
   {
      // get dimensions of mip map
      Int aw=Max(1, a.w()>>a_mip),
          ah=Max(1, a.h()>>a_mip);
      // if 'b' has the same size
      FREPD(b_mip, b.mipMaps())
      {
         // get dimensions of mip map
         Int bw=Max(1, b.w()>>b_mip),
             bh=Max(1, b.h()>>b_mip);
         if(aw==bw && ah==bh) // if match
         {
            Image temp_a, temp_b;
          C Image *sa=&a, *sb=&b;
            if(!CanDecompress(sa->hwType())){if(!sa->extractMipMap(temp_a, IMAGE_R8G8B8A8, IMAGE_SOFT, a_mip))return false; sa=&temp_a; a_mip=0;}
            if(!CanDecompress(sb->hwType())){if(!sb->extractMipMap(temp_b, IMAGE_R8G8B8A8, IMAGE_SOFT, b_mip))return false; sb=&temp_b; b_mip=0;}
            if(sa->lockRead(a_mip))
            {
               if(sb->lockRead(b_mip))
               {
                  if(sa->lockSize()==sb->lockSize())
                  {
                     ok=true;

               const Bool per_channel =false; // false=faster
               const UInt     channels=4,
                             max_value=255,
                                 scale=channels*max_value;
                     UInt  _skip_dif =Max(0, RoundPos(   skip_dif*scale)),
                        _similar_dif =Max(0, RoundPos(similar_dif*scale)),
                             max_dif =0;
                     ULong total_dif =0,
                           total_dif2=0,
                       similar_pixels=0,
                     processed_pixels=0;

                     Int   a_x_mul=ImageTI[sa->hwType()].bit_pp*2, // *2 because (4*4 colors / 8 bits)
                           b_x_mul=ImageTI[sb->hwType()].bit_pp*2; // *2 because (4*4 colors / 8 bits)
                     Color a_block[4][4], b_block[4][4];
                     void (*decompress_a)(C Byte *b, Color (&block)[4][4])=DecompressBlock(sa->hwType());
                     void (*decompress_b)(C Byte *b, Color (&block)[4][4])=DecompressBlock(sb->hwType());

                     REPD(by, DivCeil4(sa->lh()))
                     {
                        const Int py=by*4, ys=Min(4, sa->lh()-py); Int yo[4]; REP(ys)yo[i]=py+i;
                        REPD(bx, DivCeil4(sa->lw()))
                        {
                           const Int px=bx*4, xs=Min(4, sa->lw()-px); Int xo[4]; REPAO(xo)=Min(px+i, sa->lw()-1); // set all 'xo' and clamp, because we may read more, read below why

                           // it's okay to call decompress on partial blocks, because if source is compressed then its size will always fit the entire block, and we're decompressing to temporary memory
                           if(decompress_a)decompress_a(sa->data() + bx*a_x_mul + by*sa->pitch(), a_block);else sa->gather(a_block[0], xo, 4, yo, ys); // always read 4 xs because we need correct alignment for y's (for example if we would read only 2, then the next row would not be set for block[1][0] but for block[0][2])
                           if(decompress_b)decompress_b(sb->data() + bx*b_x_mul + by*sb->pitch(), b_block);else sb->gather(b_block[0], xo, 4, yo, ys); // always read 4 xs because we need correct alignment for y's (for example if we would read only 2, then the next row would not be set for block[1][0] but for block[0][2])

                           REPD(y, ys)
                           REPD(x, xs)
                           {
                            C Color &ca=a_block[y][x],
                                    &cb=b_block[y][x];
                              UInt avg_a  =AvgI(UInt(ca.a), UInt(cb.a)),
                                   dif_r  =Abs (ca.r-cb.r),
                                   dif_g  =Abs (ca.g-cb.g),
                                   dif_b  =Abs (ca.b-cb.b),
                                   dif_a  =Abs (ca.a-cb.a),
                                   dif_rgb=dif_r+dif_g+dif_b,
                                   dif    =(alpha_weight ? DivRound(dif_rgb*avg_a, 255u) : dif_rgb)+dif_a;

                              MAX(max_dif, dif);
                              total_dif+=dif;

                              if(per_channel)
                                 total_dif2+=(alpha_weight ? Sqr(DivRound(dif_r*avg_a, 255u)) + Sqr(DivRound(dif_g*avg_a, 255u)) + Sqr(DivRound(dif_b*avg_a, 255u))
                                                           : Sqr(         dif_r             ) + Sqr(         dif_g             ) + Sqr(         dif_b            )) + Sqr(dif_a);
                              else total_dif2+=dif*dif;

                              if(dif<=_similar_dif)similar_pixels++;
                              processed_pixels++;

                              if(dif>_skip_dif){skipped=true; goto finish;} // skip after setting other parameters, especially 'max_diff'
                           }
                        }
                     }

                  finish:
                     Dbl   MSE=  total_dif2  /Dbl(processed_pixels*(per_channel ? channels*max_value*max_value : scale*scale));
                     T.max_dif=    max_dif   /Flt(                 scale);
                     T.avg_dif=  total_dif   /Dbl(processed_pixels*scale); T.avg_dif2=Sqrt(MSE);
                     T.similar=similar_pixels/Dbl(processed_pixels      ); T.psnr    =10*log10(1.0/MSE);
                  }
                  sb->unlock();
               }
               sa->unlock();
            }
            break;
         }
      }
   }
   return ok;
}
/******************************************************************************/
#if WINDOWS_OLD
HICON CreateIcon(C Image &image, C VecI2 *cursor_hot_spot)
{
   HICON icon=null;
   Image temp; C Image *src=&image;
   if(src->compressed())if(src->copyTry(temp, -1, -1, 1, IMAGE_B8G8R8A8, IMAGE_SOFT, 1))src=&temp;else src=null;
   if(src && src->is() && src->lockRead())
   {
      BITMAPV5HEADER bi; Zero(bi);
      bi.bV5Size       =SIZE(bi);
      bi.bV5Width      =src->w();
      bi.bV5Height     =src->h();
      bi.bV5Planes     =1;
      bi.bV5BitCount   =32;
      bi.bV5Compression=BI_BITFIELDS;
      bi.bV5RedMask    =0x00FF0000;
      bi.bV5GreenMask  =0x0000FF00;
      bi.bV5BlueMask   =0x000000FF;
      bi.bV5AlphaMask  =0xFF000000; 

      VecB4  *data=null;
      HBITMAP hBitmap    =CreateDIBSection(null, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (Ptr*)&data, null, 0),
              hMonoBitmap=CreateBitmap    (src->w(), src->h(), 1, 1, null);

      if(data)
      {
          REPD(y, src->h())
         FREPD(x, src->w())
         {
            Color c=src->color(x, y);
            (data++)->set(c.b, c.g, c.r, c.a);
         }

         ICONINFO ii;
         if(cursor_hot_spot)
         {
            ii.fIcon   =false;
            ii.xHotspot=cursor_hot_spot->x;
            ii.yHotspot=cursor_hot_spot->y;
         }else
         {
            ii.fIcon   =true;
            ii.xHotspot=0; // this is ignored for icons
            ii.yHotspot=0; // this is ignored for icons
         }
         ii.hbmMask =hMonoBitmap;
         ii.hbmColor=hBitmap;

         icon=CreateIconIndirect(&ii);
      }

      DeleteObject(hBitmap);
      DeleteObject(hMonoBitmap);

      src->unlock();
   }
   return icon;
}
#endif
/******************************************************************************/
}
/******************************************************************************/
