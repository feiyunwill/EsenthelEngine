/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void Progress::zero()
{
   mode    =PROGRESS_NONE;
  _progress=0;
}
Progress::Progress() {zero();}
Progress& Progress::del()
{
   skin.clear();
   super::del(); zero(); return T;
}
Progress& Progress::create(PROGRESS_MODE mode)
{
   del();

   T._type      =GO_PROGRESS;
   T._visible   =true;
   T. mode      =mode;
   T._rect.max.x= 0.40f;
   T._rect.min.y=-0.04f;

   return T;
}
Progress& Progress::create(C Progress &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type    =GO_PROGRESS;
         mode    =src. mode;
         skin    =src. skin;
        _progress=src._progress;
      }
   }
   return T;
}
/******************************************************************************/
Progress& Progress::clear(                ) {T._progress=0                                   ; return T;}
Progress& Progress::set  (Int  x, Int  max) {T._progress=((max>0) ? Sat(    Flt(x)/max ) : 0); return T;}
Progress& Progress::set  (Long x, Long max) {T._progress=((max>0) ? Sat(Flt(Dbl(x)/max)) : 0); return T;}
Progress& Progress::set  (Flt  progress   ) {T._progress=Sat(progress)                       ; return T;}
/******************************************************************************/
void Progress::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
      if(GuiSkin *skin=getSkin())
   {
      Rect rect=T.rect()+gpc.offset, ext_rect, prog_ext_rect;
      if(skin->progress.background)  skin->progress.background->extendedRect(rect,      ext_rect);else ext_rect =rect;
      if(skin->progress.progress  && skin->progress.progress  ->extendedRect(rect, prog_ext_rect))     ext_rect|=prog_ext_rect;
      if(Cuts(ext_rect, gpc.clip))
      {
         Bool vertical=rect.vertical();
         D.clip(gpc.clip);
         D.alignScreenToPixel(rect);

         // background
         if(skin->progress.background)
         {
            if(vertical)skin->progress.background->drawVertical(skin->progress.background_color, TRANSPARENT, rect);
            else        skin->progress.background->draw        (skin->progress.background_color, TRANSPARENT, rect);
         }else
         if(skin->progress.background_color.a)rect.draw(skin->progress.background_color);

         // progress
         if(skin->progress.progress)
         {
            if(skin->progress.draw_progress_partial)
            {
               if(vertical)skin->progress.progress->drawVerticalFrac(skin->progress.progress_color, TRANSPARENT, rect, T(), true);
               else        skin->progress.progress->drawFrac        (skin->progress.progress_color, TRANSPARENT, rect, T(), true);
            }else
            {
               Rect prog_rect=rect; if(vertical)prog_rect.max.y=rect.lerpY(T());else prog_rect.max.x=rect.lerpX(T());

               // shrink in case of shadow/glow
               if(skin->progress.progress->_padd_any)
               {
                  if(vertical)
                  {
                     Flt h=prog_rect.h(), size=skin->progress.progress->_side_size.y, scale; if(skin->progress.progress->getSideScale(rect, scale))size*=scale;
                     if(size>h)
                     {
                        scale=h/size; scale=(1-scale)*prog_rect.w()*0.5f;
                        prog_rect.min.x+=scale;
                        prog_rect.max.x-=scale;
                     }
                  }else
                  {
                     Flt w=prog_rect.w(), size=skin->progress.progress->_side_size.x, scale; if(skin->progress.progress->getSideScale(rect, scale))size*=scale;
                     if(size>w)
                     {
                        Flt scale=w/size; scale=(1-scale)*prog_rect.h()*0.5f;
                        prog_rect.min.y+=scale;
                        prog_rect.max.y-=scale;
                     }
                  }
               }

               if(vertical)skin->progress.progress->drawVertical(skin->progress.progress_color, TRANSPARENT, prog_rect);
               else        skin->progress.progress->draw        (skin->progress.progress_color, TRANSPARENT, prog_rect);
            }
         }else
         if(skin->progress.progress_color.a)
         {
            Rect prog_rect=rect; if(vertical)prog_rect.max.y=rect.lerpY(T());else prog_rect.max.x=rect.lerpX(T());
            prog_rect.draw(skin->progress.progress_color);
         }

         // text
         if(mode)
            if(TextStyle *text_style=skin->progress.text_style())
         {
            TextStyleParams ts=*text_style; ts.size=(vertical ? rect.w()*0.5f : rect.h())*skin->progress.text_size;
            D.text(ts, rect.center(), (mode==PROGRESS_PERCENT) ? S+RoundPos(T()*100)+'%' : S+T());
         }
      }
   }
}
/******************************************************************************/
Bool Progress::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.cmpUIntV(4); // version

      f<<_progress<<mode;
      f._putAsset(skin.name(path));
      return f.ok();
   }
   return false;
}
Bool Progress::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 4:
      {
        _type=GO_PROGRESS;

         f>>_progress>>mode;
         skin.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;

      case 3:
      {
        _type=GO_PROGRESS;

         f>>mode; f.skip(13); f>>_progress; f._getStr(); f._getStr();
         if(f.ok())return true;
      }break;

      case 2:
      {
        _type=GO_PROGRESS;

         f>>mode; f.skip(13); f>>_progress; f._getStr(); f._getStr();
         if(f.ok())return true;
      }break;

      case 1:
      {
        _type=GO_PROGRESS;

         f>>mode; f.skip(12); f>>_progress; f._getStr8(); f._getStr8();
         if(f.ok())return true;
      }break;

      case 0:
      {
        _type=GO_PROGRESS;

         f>>mode; f.skip(8); f>>_progress; f._getStr8(); 
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
