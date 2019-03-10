/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void GuiImage::zero()
{
   fit=false;
   alpha_mode=ALPHA_BLEND;
   rect_color.zero();
   color    =WHITE;
   color_add.zero();
}
GuiImage::GuiImage() {zero();}
GuiImage& GuiImage::del()
{
   super::del(); zero(); return T;
}
GuiImage& GuiImage::create(C ImagePtr &image)
{
   del();

  _type      =GO_IMAGE;
  _visible   =true;
  _rect.max.x= 0.3f;
  _rect.min.y=-0.3f;
 T.image     =image;
   rect_color=Gui.borderColor();

   return T;
}
GuiImage& GuiImage::create(C GuiImage &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type      =GO_IMAGE;
         fit       =src.fit       ;
         alpha_mode=src.alpha_mode;
         rect_color=src.rect_color;
         color     =src.color     ;
         color_add =src.color_add ;   
         image     =src.image     ;
      }
   }
   return T;
}
/******************************************************************************/
GuiImage& GuiImage::set(C ImagePtr &image) {T.image=image; return T;}
/******************************************************************************/
void GuiImage::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      Rect rect=T.rect()+gpc.offset;
      if(Cuts(rect, gpc.clip))
      {
         D.alignScreenToPixel(rect);
         D.clip(gpc.clip);
         if(image)
         {
            ALPHA_MODE alpha=D.alpha(alpha_mode); if(fit)image->drawFit(color, color_add, rect);else image->draw(color, color_add, rect);
                             D.alpha(alpha     );
         }
         if(rect_color.a)rect.draw(rect_color, false);
      }
   }
}
/******************************************************************************/
Bool GuiImage::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.putMulti(Byte(5), fit, alpha_mode, rect_color, color, color_add); // version
      f.putAsset(image.id());
      return f.ok();
   }
   return false;
}
Bool GuiImage::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 5:
      {
        _type=GO_IMAGE;

         f.getMulti(fit, alpha_mode, rect_color, color, color_add);
         image.require(f.getAssetID(), path);
         if(f.ok())return true;
      }break;

      case 4:
      {
        _type=GO_IMAGE;

         f>>alpha_mode>>fit>>rect_color>>color>>color_add;
         image.require(f._getStr(), path);
         if(f.ok())return true;
      }break;

      case 3:
      {
        _type=GO_IMAGE;

         f>>alpha_mode>>rect_color>>color>>color_add;
         image.require(f._getStr(), path);
         if(f.ok())return true;
      }break;

      case 2:
      {
        _type=GO_IMAGE;

         f>>alpha_mode>>rect_color>>color>>color_add; Swap(rect_color.r, rect_color.b); Swap(color.r, color.b); Swap(color_add.r, color_add.b);
         image.require(f._getStr(), path);
         if(f.ok())return true;
      }break;

      case 1:
      {
        _type=GO_IMAGE;

         f>>alpha_mode>>rect_color>>color>>color_add; Swap(rect_color.r, rect_color.b); Swap(color.r, color.b); Swap(color_add.r, color_add.b);
         image.require(f._getStr8(), path);
         if(f.ok())return true;
      }break;

      case 0:
      {
        _type=GO_IMAGE;

         f>>alpha_mode>>rect_color; Swap(rect_color.r, rect_color.b); color=WHITE; color_add.zero();
         image.require(f._getStr8(), path);
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
