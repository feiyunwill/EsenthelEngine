/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_IMGA CC4('I','M','G','A')
/******************************************************************************/
DEFINE_CACHE(ImageAtlas, ImageAtlases, ImageAtlasPtr, "ImageAtlas");
/******************************************************************************/
static Bool TestCol(Image &image, Int x) {REPD(y, image.h())if(image.color(x, y).a)return true; return false;}
static Bool TestRow(Image &image, Int y) {REPD(x, image.w())if(image.color(x, y).a)return true; return false;}
/******************************************************************************/
Int               ImageAtlas::findPartI(C Str &name) {if(name.is())FREPA(parts)if(EqualPath(parts[i].name, name))return i; return -1;}
Int               ImageAtlas:: getPartI(C Str &name) {if(name.is()){Int i=findPartI(name); if(i<0)Exit(S+"Part \""+name+"\" not found in ImageAtlas."); return i;} return -1;}
ImageAtlas::Part* ImageAtlas::findPart (C Str &name) {Int i=findPartI(name); return (i<0) ? null : &parts[i];}
ImageAtlas::Part* ImageAtlas:: getPart (C Str &name) {Int i= getPartI(name); return (i<0) ? null : &parts[i];}
/******************************************************************************/
void ImageAtlas::del()
{
   parts .del();
   images.del();
}
struct ImageAtlasSrc : RectSizeAnchor // trimmed size
{
   Image *image;
 C Str   *name;
   RectI  opaque;
   VecI2  original_size;

   ImageAtlasSrc() {size.zero(); image=null; name=null; opaque.set(0, 0, -1, -1); original_size.zero();}
};
/******************************************************************************/
Bool ImageAtlas::create(C MemPtr<Source> &images, IMAGE_TYPE image_type, Int mip_maps, Bool allow_rotate, Int border, Bool align_for_compression, Bool only_square, Int max_tex_size, Bool trim_transparent, Bool transparent_to_neighbors)
{
   del();

   Clamp(max_tex_size, 1, 16384);
   Clamp(border, 0, max_tex_size);

   // get rectangles of opaque parts in images
   Memb<Image        > image_decompressed; // use 'Memb' because we're storing pointer to element
   Memc<ImageAtlasSrc> ias; ias.setNum(images.elms());
   Memc<RectIndex    > packed;
   Memc<VecI2        > image_sizes;
   FREPA(images)
   {
    C Source        &image=images[i];
      ImageAtlasSrc &ia   =ias   [i];
      ia.name=&image.name;
      if(Image *src=image.image())if(src->is())
      {
         ia.original_size.set(src->w(), src->h());
         if( src->compressed()){Image &image_temp=image_decompressed.New(); if(!src->copyTry(image_temp, -1, -1, -1, -1, IMAGE_SOFT, 1))goto error; src=&image_temp;}
         if(!src->lock(LOCK_READ))goto error;
         RectI opaque; // this is inclusive
         if(!trim_transparent)opaque.set(0, 0, src->w()-1, src->h()-1);else
         {
            opaque.set(0, 0, -1, -1); // set as invalid on start
            FREPD(x, src->w())if(TestCol(*src, x)){opaque.min.x=x; break;} REPD(x, src->w())if(TestCol(*src, x)){opaque.max.x=x; break;}
            FREPD(y, src->h())if(TestRow(*src, y)){opaque.min.y=y; break;} REPD(y, src->h())if(TestRow(*src, y)){opaque.max.y=y; break;}
         }
         if(opaque.valid())
         {
            ia.size  .set(opaque.w()+1, opaque.h()+1); // trimmed size, +1 because 'opaque' is inclusive
            ia.anchor.set(ia.original_size.x/2-opaque.min.x, ia.original_size.y/2-opaque.min.y);
            ia.image =src;
            ia.opaque=opaque;
         }
      }
   }

   if(!PackRectsMultiLimit(SCAST(Memc<RectSizeAnchor>, ias), packed, image_sizes, allow_rotate, border, align_for_compression, false, only_square, max_tex_size))goto error; // set false for 'compact_arrangement' because it's better to have some more spacing between parts

   // write parts to images
   T.parts .setNum(ias        .elms());
   T.images.setNum(image_sizes.elms()); REPA(T.images)if(T.images[i].createTry(image_sizes[i].x, image_sizes[i].y, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))T.images[i].clear();else goto error;
   REPA(ias)
   {
    C RectIndex    &pack=packed[i];
      ImageAtlasSrc  &ia=ias   [i];
      Part         &part=parts [i];
      part.original_size= ia.original_size;
      part.name         =*ia.name;
      if(ia.image)
      {
         const Flt e=0.0f; // 0.5f; don't apply LINEAR FILTER offset because it destroys animated images (jump of offset/scale is visible between 2 frames)
         Image &image=T.images[pack.index];
         part.image_index  =   pack.index;
         part.rotated      =((pack.w()>pack.h())!=(ia.size.x>ia.size.y));
         part.tex_rect     .set(Flt(pack.min.x-e)/image.w(), Flt(pack.min.y-e)/image.h(), Flt(pack.max.x+e)/image.w(), Flt(pack.max.y+e)/image.h());
         part.trimmed_size =ia.size;
         part.trim_pos     =ia.opaque.min;
         part.center_offset.set(part.trim_pos.x-part.original_size.x*0.5f, -part.trim_pos.y+part.original_size.y*0.5f);

         // set texture data from the image
         if(part.rotated)
         {
            REPD(y, part.trimmed_size.y)
            REPD(x, part.trimmed_size.x)image.color(pack.max.x-1-y, x+pack.min.y, ia.image->color(x+part.trim_pos.x, y+part.trim_pos.y));
         }else
         {
            REPD(y, part.trimmed_size.y)
            REPD(x, part.trimmed_size.x)image.color(x+pack.min.x, y+pack.min.y, ia.image->color(x+part.trim_pos.x, y+part.trim_pos.y));
         }
      }else
      {
         part.rotated      =false;
         part.image_index  =0xFF;
         part.tex_rect     .zero();
         part.center_offset.zero();
         part.trimmed_size .zero();
         part.trim_pos     .zero();
      }
   }
   REPA(T.images)
   {
      Image &image=T.images[i];
      if(transparent_to_neighbors)image.transparentToNeighbor();
      if(!image.copyTry(image, -1, -1, -1, image_type, IMAGE_2D, mip_maps, FILTER_BEST, true, true))goto error;
   }

   // success
   {
      REPA(images)if(images[i].image)images[i].image->unlock();
      return true;
   }

error:
   REPA(images)if(images[i].image)images[i].image->unlock(); del(); return false;
}
/******************************************************************************/
void ImageAtlas::draw(Int part_index, C Vec2 &pos, Flt pixel_size)C
{
   if(InRange(part_index, parts))
   {
    C Part &part=parts[part_index];
      if(C Image *image=images.addr(part.image_index))
      {
         Rect_LU screen_rect(part.center_offset*pixel_size+pos,  // position
                             part.trimmed_size *pixel_size    ); // size
         if(part.rotated)image->drawPartVertical(screen_rect, part.tex_rect);
         else            image->drawPart        (screen_rect, part.tex_rect);
      }
   }
}
/******************************************************************************/
Bool ImageAtlas::save(File &f)C
{
   f.putUInt(CC4_IMGA).cmpUIntV(2);
   f.cmpUIntV(parts.elms());
   FREPA(parts)
   {
    C Part &part=parts[i];
      f.putMulti(part.image_index, part.rotated, part.tex_rect, part.center_offset, part.original_size, part.trimmed_size, part.trim_pos)<<part.name;
   }
   if(images.save(f))
      return f.ok();
   return false;
}
Bool ImageAtlas::load(File &f)
{
   del();
   if(f.getUInt()==CC4_IMGA)switch(f.decUIntV())
   {
      case 2:
      {
         parts.setNum(f.decUIntV());
         FREPA(parts)
         {
            Part &part=parts[i];
            f.getMulti(part.image_index, part.rotated, part.tex_rect, part.center_offset, part.original_size, part.trimmed_size, part.trim_pos)>>part.name;
         }
         if(images.load(f))
            if(f.ok())return true;
      }break;

      case 1:
      {
         parts.setNum(f.decUIntV());
         FREPA(parts)
         {
            Part &part=parts[i];
            f.getMulti(part.image_index, part.rotated, part.tex_rect, part.center_offset, part.original_size, part.trimmed_size, part.trim_pos)._getStr2(part.name);
         }
         if(images.load(f))
            if(f.ok())return true;
      }break;

      case 0:
      {
         parts.setNum(f.decUIntV());
         FREPA(parts)
         {
            Part &part=parts[i];
            f>>part.image_index>>part.tex_rect>>part.center_offset>>part.original_size>>part.trimmed_size>>part.trim_pos;
            f._getStr(part.name);
            part.rotated=false;
         }
         if(images._load(f))
            if(f.ok())return true;
      }break;
   }
   del(); return false;
}
Bool ImageAtlas::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool ImageAtlas::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}
void ImageAtlas::operator=(C Str &name) {if(!load(name))Exit(S+"Can't load ImageAtlas \""+name+ "\".");}
/******************************************************************************/
}
/******************************************************************************/
