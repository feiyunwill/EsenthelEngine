/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   windows aero colors
   on black background - R  21, G  46, B  71
   on white background - R 199, G 224, B 249
   panel_window.back_color.set(178, 178, 178);
   panel_window.blur_color.set( 21,  46,  71);

/******************************************************************************/
#define CC4_GSTL CC4('G','S','T','L')
#define DEFAULT_SIZE 0.1f
/******************************************************************************/
DEFINE_CACHE(Panel, Panels, PanelPtr, "Panel");
/******************************************************************************/
void Panel::extendedRect(C Rect &rect, Rect &extended)C
{
   Rect r; if(panel_image)panel_image->extendedRect(rect, r);else r=rect;
   if(shadow_opacity)r|=(rect+Vec2(shadow_offset, -shadow_offset)).extend(shadow_radius);
   if(border_color.a)
   {
      Flt border=-border_size;
      if( border>0)r.extend(border);
   }
   if(side_stretch)
   {
      Flt y[]={rect.min.y+bottom_offset, rect.max.y+top_offset};
      Flt x[3][2]= // [y][x]
      {
         {rect.min.x-   top_corner_offset.x, rect.max.x+   top_corner_offset.x},
         {rect.min.x-   left_right_offset.x, rect.max.x+   left_right_offset.x},
         {rect.min.x-bottom_corner_offset.x, rect.max.x+bottom_corner_offset.x},
      };
      if(       top_image)r.includeY(y[1]   +       top_size);
      if(    bottom_image)r.includeY(y[0]   -    bottom_size);
      if(left_right_image)r.includeX(x[1][0]-left_right_size, x[1][1]+left_right_size);
      if(top_corner_image)
      {
         Flt w=top_corner_size*top_corner_image->aspect(),
             h=top_corner_size;
         r.includeY(y[1]+h).includeX(x[0][0]-w, x[0][1]+w);
      }
      if(bottom_corner_image)
      {
         Flt w=bottom_corner_size*bottom_corner_image->aspect(),
             h=bottom_corner_size;
         r.includeY(y[0]-h).includeX(x[2][0]-w, x[2][1]+w);
      }
   }else
   {
      if(       top_image)r|=Rect_C(rect.centerX()/*+   top_offset.x*/, rect.max.y+   top_offset,    top_size*   top_image->aspect(),    top_size);
      if(    bottom_image)r|=Rect_C(rect.centerX()/*+bottom_offset.x*/, rect.min.y+bottom_offset, bottom_size*bottom_image->aspect(), bottom_size);
      if(left_right_image)
      {
         Flt w=left_right_size*left_right_image->aspect(),
             h=left_right_size,
             y=rect.centerY()+left_right_offset.y;
         r|=Rect_C(rect.min.x-left_right_offset.x, y, w, h);
         r|=Rect_C(rect.max.x+left_right_offset.x, y, w, h);
      }
      if(top_corner_image)
      {
         Flt w=top_corner_size*top_corner_image->aspect(),
             h=top_corner_size,
             y=rect.max.y+top_corner_offset.y;
         r|=Rect_C(rect.min.x-top_corner_offset.x, y, w, h);
         r|=Rect_C(rect.max.x+top_corner_offset.x, y, w, h);
      }
      if(bottom_corner_image)
      {
         Flt w=bottom_corner_size*bottom_corner_image->aspect(),
             h=bottom_corner_size,
             y=rect.min.y+bottom_corner_offset.y;
         r|=Rect_C(rect.min.x-bottom_corner_offset.x, y, w, h);
         r|=Rect_C(rect.max.x+bottom_corner_offset.x, y, w, h);
      }
   }
   extended=r; // modify at the end in case 'extended' is 'rect'
}
void Panel::defaultInnerPadding(Rect &padding)C
{
   if(panel_image)padding=panel_image->defaultInnerPadding();else padding.zero();
#if 0 // border is not included
   if(border_color.a && border_size>0)
   {
      MAX(padding.min.x, border_size);
      MAX(padding.min.y, border_size);
      MAX(padding.max.x, border_size);
      MAX(padding.max.y, border_size);
   }
#endif
   if(side_stretch)
   {
      if(       top_image) MAX(padding.max.y,        -top_offset); //    'top_offset' moves up
      if(    bottom_image) MAX(padding.min.y,      bottom_offset); // 'bottom_offset' moves up
      if(left_right_image){MAX(padding.min.x, -left_right_offset.x); MAX(padding.max.x, -left_right_offset.x);} // 'left_right_offset.x' moves right
   }
}
void Panel::innerPadding(C Rect &rect, Rect &padding)C
{
   if(panel_image)panel_image->innerPadding(rect, padding);else padding.zero();
#if 0 // border is not included
   if(border_color.a && border_size>0)
   {
      MAX(padding.min.x, border_size);
      MAX(padding.min.y, border_size);
      MAX(padding.max.x, border_size);
      MAX(padding.max.y, border_size);
   }
#endif
   if(side_stretch)
   {
      if(       top_image) MAX(padding.max.y,        -top_offset); //    'top_offset' moves up
      if(    bottom_image) MAX(padding.min.y,      bottom_offset); // 'bottom_offset' moves up
      if(left_right_image){MAX(padding.min.x, -left_right_offset.x); MAX(padding.max.x, -left_right_offset.x);} // 'left_right_offset.x' moves right
   }
}
void Panel::defaultInnerPaddingSize(Vec2 &padd_size)C
{
   Rect padding; defaultInnerPadding(padding);
   padd_size.set(padding.min.x+padding.max.x, padding.min.y+padding.max.y);
}
/******************************************************************************/
void Panel::reset()
{
   center_stretch=side_stretch=false;
   center_color=WHITE;
   border_color=WHITE;
     side_color=WHITE;
     blur_color=TRANSPARENT;
   center_shadow =false;
   shadow_opacity=170;
   shadow_radius =0.035f;
   shadow_offset =0;
   border_size   =0;
   center_scale  =1;
   top_size=bottom_size=left_right_size=top_corner_size=bottom_corner_size=DEFAULT_SIZE;
   top_offset=bottom_offset=0;
   left_right_offset=top_corner_offset=bottom_corner_offset.zero();
   center_image=border_image=top_image=bottom_image=left_right_image=top_corner_image=bottom_corner_image.clear();
   panel_image.clear();
}
/******************************************************************************/
Bool Panel::pixelBorder()C
{
   return border_color.a && !border_image && !border_size;
}
Bool Panel::getSideScale(C Rect &rect, Flt &scale)C
{
   if(panel_image)return panel_image->getSideScale(rect, scale);
   return false;
}
void Panel::scaleBorder(Flt scale)
{
          border_size*=scale;
             top_size*=scale;           top_offset*=scale;
          bottom_size*=scale;        bottom_offset*=scale;
      left_right_size*=scale;    left_right_offset*=scale;
      top_corner_size*=scale;    top_corner_offset*=scale;
   bottom_corner_size*=scale; bottom_corner_offset*=scale;
}
/******************************************************************************/
void Panel::draw(C Rect &rect)C
{
   Rect r=rect; if(side_stretch)
   {
      r.min.x-=left_right_offset.x; r.min.y+=bottom_offset;
      r.max.x+=left_right_offset.x; r.max.y+=   top_offset;
   }
#if !MOBILE // too slow
   if(blur_color.a)
   {
      const Bool    hi   =true;
      const Int     shift=(hi ? 1 : 2);
      ImageRTPtrRef rt0(hi ? Renderer._h0 : Renderer._q0); rt0.get(ImageRTDesc(Renderer._gui->w()>>shift, Renderer._gui->h()>>shift, IMAGERT_RGB));
      ImageRTPtrRef rt1(hi ? Renderer._h1 : Renderer._q1); rt1.get(ImageRTDesc(Renderer._gui->w()>>shift, Renderer._gui->h()>>shift, IMAGERT_RGB));
      Image        *cur      =Renderer._cur[0], *ds=Renderer._cur_ds;
      Rect          re       =r; re.extend(D.pixelToScreenSize(SHADER_BLUR_RANGE<<shift));
      Bool          secondary=(Renderer._gui!=cur); // required when "window.fade && blur" is used

                   Renderer._gui->copyHw(*rt0, false, re); // use 'Renderer.gui' instead of 'Renderer.cur[0]' in case we're drawing transparent Window and we're inside 'D.fxBegin' but need to access default gui RT
      if(secondary)Renderer._gui->copyHw(*cur, false, r ); // set background to be a copy

      ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Renderer.set(rt1(), null, false); Sh.h_BlurX[true]->draw(rt0(), &re);
                                            Renderer.set(rt0(), null, false); Sh.h_BlurY[true]->draw(rt1(), &re);
                                            Renderer.set(cur  , ds  , true );
      if(shadow_opacity)
      {
         D.alpha(ALPHA_BLEND);
         if(shadow_opacity)
         {
            if(center_shadow)D.drawShadow       (shadow_opacity, rect+Vec2(shadow_offset, -shadow_offset), shadow_radius);
            else             D.drawShadowBorders(shadow_opacity, rect                                    , shadow_radius);
         }
      }
      if(secondary) // for secondary we need to force rt.alpha to 1.0
      {
         D.alphaFactor(Color(blur_color.a, blur_color.a, blur_color.a, 255));
         D.alpha(ALPHA_FACTOR); Sh.h_Color[0]->set(Color(center_color.r, center_color.g, center_color.b, 0)); Sh.h_Color[1]->set(Color(blur_color.r, blur_color.g, blur_color.b, 255)); Sh.h_DrawC->draw(rt0(), &r);
      }else
      {
         D.alpha(ALPHA_BLEND ); Sh.h_Color[0]->set(Color(center_color.r, center_color.g, center_color.b, 0)); Sh.h_Color[1]->set(blur_color); Sh.h_DrawC->draw(rt0(), &r);
      }
         D.alpha(alpha       );
   }else
#endif
   {
      if(shadow_opacity)
      {
         if(center_shadow)D.drawShadow       (shadow_opacity, rect+Vec2(shadow_offset, -shadow_offset), shadow_radius);
         else             D.drawShadowBorders(shadow_opacity, rect                                    , shadow_radius);
      }
      if(center_color.a)
      {
         if(  panel_image  )panel_image ->draw    (center_color, TRANSPARENT, r);else
         if(!center_image  )r           . draw    (center_color                );else
         if(!center_stretch)center_image->drawTile(center_color, TRANSPARENT, r, center_scale);else
                            center_image->draw    (center_color, TRANSPARENT, r);
      }
   }

   if(border_color.a)
   {
      if(border_image)border_image->drawBorder(border_color, TRANSPARENT, r, border_size);else
      if(border_size )r           . drawBorder(border_color, border_size);else
                      r           . draw      (border_color, false);
   }
   if(side_color.a)
   {
      if(side_stretch)
      {
         Flt y[]={r.min.y, r.max.y};
         Flt x[3][2]= // [y][x]
         {
            {rect.min.x-   top_corner_offset.x, rect.max.x+   top_corner_offset.x},
            {r   .min.x                       , r   .max.x                       },
            {rect.min.x-bottom_corner_offset.x, rect.max.x+bottom_corner_offset.x},
         };
         if(   top_image)top_image->draw(side_color, TRANSPARENT, Rect(x[0][0], y[1], x[0][1], y[1]+top_size));
         if(bottom_image)
         {
            Rect r(x[2][0], y[0]-bottom_size, x[2][1], y[0]);
            if(bottom_image==top_image)r.swapY(); // mirror vertically if it's the same as top
            bottom_image->draw(side_color, TRANSPARENT, r);
         }
         if(left_right_image)
         {
            left_right_image->draw(side_color, TRANSPARENT, Rect(x[1][0], y[0], x[1][0]-left_right_size, y[1])); // mirror horizontally
            left_right_image->draw(side_color, TRANSPARENT, Rect(x[1][1], y[0], x[1][1]+left_right_size, y[1]));
         }
         if(top_corner_image)
         {
            Flt w=top_corner_size*top_corner_image->aspect(),
                h=top_corner_size,
                y1=y[1]+h;
            top_corner_image->draw(side_color, TRANSPARENT, Rect(x[0][0], y[1], x[0][0]-w, y1)); // mirror horizontally
            top_corner_image->draw(side_color, TRANSPARENT, Rect(x[0][1], y[1], x[0][1]+w, y1));
         }
         if(bottom_corner_image)
         {
            Flt w=bottom_corner_size*bottom_corner_image->aspect(),
                h=bottom_corner_size,
                y0=y[0]-h, y1=y[0];
            if(bottom_corner_image==top_corner_image)Swap(y0, y1); // mirror vertically if it's the same as top
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect(x[2][0], y0, x[2][0]-w, y1)); // mirror horizontally
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect(x[2][1], y0, x[2][1]+w, y1));
         }
      }else
      {
         if(   top_image)top_image->draw(side_color, TRANSPARENT, Rect_C(rect.centerX()/*+top_offset.x*/, rect.max.y+top_offset, top_size*top_image->aspect(), top_size));
         if(bottom_image)
         {
            Flt h=bottom_size;
            if(bottom_image==top_image)CHS(h); // mirror vertically if it's the same as top
            bottom_image->draw(side_color, TRANSPARENT, Rect_C(rect.centerX()/*+bottom_offset.x*/, rect.min.y+bottom_offset, bottom_size*bottom_image->aspect(), h));
         }
         if(left_right_image)
         {
            Flt w=left_right_size*left_right_image->aspect(),
                h=left_right_size,
                y=rect.centerY()+left_right_offset.y;
            left_right_image->draw(side_color, TRANSPARENT, Rect_C(rect.min.x-left_right_offset.x, y, -w, h));
            left_right_image->draw(side_color, TRANSPARENT, Rect_C(rect.max.x+left_right_offset.x, y,  w, h));
         }
         if(top_corner_image)
         {
            Flt w=top_corner_size*top_corner_image->aspect(),
                h=top_corner_size,
                y=rect.max.y+top_corner_offset.y;
            top_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.min.x-top_corner_offset.x, y, -w, h));
            top_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.max.x+top_corner_offset.x, y,  w, h));
         }
         if(bottom_corner_image)
         {
            Flt w=bottom_corner_size*bottom_corner_image->aspect(),
                h=bottom_corner_size,
                y=rect.min.y+bottom_corner_offset.y;
            if(bottom_corner_image==top_corner_image)CHS(h); // mirror vertically if it's the same as top
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.min.x-bottom_corner_offset.x, y, -w, h));
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.max.x+bottom_corner_offset.x, y,  w, h));
         }
      }
   }
}
void Panel::draw(C Color &color, C Rect &rect)C
{
   Byte  shadow_opacity=        (T.shadow_opacity* color.a+128)/255;
   Color center_color  =ColorMul(T.center_color  , color),
         border_color  =ColorMul(T.border_color  , color),
           side_color  =ColorMul(T.  side_color  , color),
           blur_color  =ColorMul(T.  blur_color  , color);

   Rect r=rect; if(side_stretch)
   {
      r.min.x-=left_right_offset.x; r.min.y+=bottom_offset;
      r.max.x+=left_right_offset.x; r.max.y+=   top_offset;
   }

#if !MOBILE // too slow
   if(blur_color.a)
   {
      const Bool    hi   =true;
      const Int     shift=(hi ? 1 : 2);
      ImageRTPtrRef rt0(hi ? Renderer._h0 : Renderer._q0); rt0.get(ImageRTDesc(Renderer._gui->w()>>shift, Renderer._gui->h()>>shift, IMAGERT_RGB));
      ImageRTPtrRef rt1(hi ? Renderer._h1 : Renderer._q1); rt1.get(ImageRTDesc(Renderer._gui->w()>>shift, Renderer._gui->h()>>shift, IMAGERT_RGB));
      Image        *cur      =Renderer._cur[0], *ds=Renderer._cur_ds;
      Rect          re       =r; re.extend(D.pixelToScreenSize(SHADER_BLUR_RANGE<<shift));
      Bool          secondary=(Renderer._gui!=cur); // required when "window.fade && blur" is used

                   Renderer._gui->copyHw(*rt0, false, re); // use 'Renderer.gui' instead of 'Renderer.cur[0]' in case we're drawing transparent Window and we're inside 'D.fxBegin' but need to access default gui RT
      if(secondary)Renderer._gui->copyHw(*cur, false, r ); // set background to be a copy

      ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Renderer.set(rt1(), null, false); Sh.h_BlurX[true]->draw(rt0(), &re);
                                            Renderer.set(rt0(), null, false); Sh.h_BlurY[true]->draw(rt1(), &re);
                                            Renderer.set(cur  , ds  , true );
      if(shadow_opacity)
      {
         D.alpha(ALPHA_BLEND);
         if(shadow_opacity)
         {
            if(center_shadow)D.drawShadow       (shadow_opacity, rect+Vec2(shadow_offset, -shadow_offset), shadow_radius);
            else             D.drawShadowBorders(shadow_opacity, rect                                    , shadow_radius);
         }
      }
      if(secondary) // for secondary we need to force rt.alpha to 1.0
      {
         D.alphaFactor(Color(blur_color.a, blur_color.a, blur_color.a, 255));
         D.alpha(ALPHA_FACTOR); Sh.h_Color[0]->set(Color(center_color.r, center_color.g, center_color.b, 0)); Sh.h_Color[1]->set(Color(blur_color.r, blur_color.g, blur_color.b, 255)); Sh.h_DrawC->draw(rt0(), &r);
      }else
      {
         D.alpha(ALPHA_BLEND ); Sh.h_Color[0]->set(Color(center_color.r, center_color.g, center_color.b, 0)); Sh.h_Color[1]->set(blur_color); Sh.h_DrawC->draw(rt0(), &r);
      }
         D.alpha(alpha       );
   }else
#endif
   {
      if(shadow_opacity)
      {
         if(center_shadow)D.drawShadow       (shadow_opacity, rect+Vec2(shadow_offset, -shadow_offset), shadow_radius);
         else             D.drawShadowBorders(shadow_opacity, rect                                    , shadow_radius);
      }
      if(center_color.a)
      {
         if(  panel_image  )panel_image ->draw    (center_color, TRANSPARENT, r);else
         if(!center_image  )r           . draw    (center_color                );else
         if(!center_stretch)center_image->drawTile(center_color, TRANSPARENT, r, center_scale);else
                            center_image->draw    (center_color, TRANSPARENT, r);
      }
   }

   if(border_color.a)
   {
      if(border_image)border_image->drawBorder(border_color, TRANSPARENT, r, border_size);else
      if(border_size )r           . drawBorder(border_color, border_size);else
                      r           . draw      (border_color, false);
   }
   if(side_color.a)
   {
      if(side_stretch)
      {
         Flt y[]={r.min.y, r.max.y};
         Flt x[3][2]= // [y][x]
         {
            {rect.min.x-   top_corner_offset.x, rect.max.x+   top_corner_offset.x},
            {r   .min.x                       , r   .max.x                       },
            {rect.min.x-bottom_corner_offset.x, rect.max.x+bottom_corner_offset.x},
         };
         if(   top_image)top_image->draw(side_color, TRANSPARENT, Rect(x[0][0], y[1], x[0][1], y[1]+top_size));
         if(bottom_image)
         {
            Rect r(x[2][0], y[0]-bottom_size, x[2][1], y[0]);
            if(bottom_image==top_image)r.swapY(); // mirror vertically if it's the same as top
            bottom_image->draw(side_color, TRANSPARENT, r);
         }
         if(left_right_image)
         {
            left_right_image->draw(side_color, TRANSPARENT, Rect(x[1][0], y[0], x[1][0]-left_right_size, y[1])); // mirror horizontally
            left_right_image->draw(side_color, TRANSPARENT, Rect(x[1][1], y[0], x[1][1]+left_right_size, y[1]));
         }
         if(top_corner_image)
         {
            Flt w=top_corner_size*top_corner_image->aspect(),
                h=top_corner_size,
                y1=y[1]+h;
            top_corner_image->draw(side_color, TRANSPARENT, Rect(x[0][0], y[1], x[0][0]-w, y1)); // mirror horizontally
            top_corner_image->draw(side_color, TRANSPARENT, Rect(x[0][1], y[1], x[0][1]+w, y1));
         }
         if(bottom_corner_image)
         {
            Flt w=bottom_corner_size*bottom_corner_image->aspect(),
                h=bottom_corner_size,
                y0=y[0]-h, y1=y[0];
            if(bottom_corner_image==top_corner_image)Swap(y0, y1); // mirror vertically if it's the same as top
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect(x[2][0], y0, x[2][0]-w, y1)); // mirror horizontally
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect(x[2][1], y0, x[2][1]+w, y1));
         }
      }else
      {
         if(   top_image)top_image->draw(side_color, TRANSPARENT, Rect_C(rect.centerX()/*+top_offset.x*/, rect.max.y+top_offset, top_size*top_image->aspect(), top_size));
         if(bottom_image)
         {
            Flt h=bottom_size;
            if(bottom_image==top_image)CHS(h); // mirror vertically if it's the same as top
            bottom_image->draw(side_color, TRANSPARENT, Rect_C(rect.centerX()/*+bottom_offset.x*/, rect.min.y+bottom_offset, bottom_size*bottom_image->aspect(), h));
         }
         if(left_right_image)
         {
            Flt w=left_right_size*left_right_image->aspect(),
                h=left_right_size,
                y=rect.centerY()+left_right_offset.y;
            left_right_image->draw(side_color, TRANSPARENT, Rect_C(rect.min.x-left_right_offset.x, y, -w, h));
            left_right_image->draw(side_color, TRANSPARENT, Rect_C(rect.max.x+left_right_offset.x, y,  w, h));
         }
         if(top_corner_image)
         {
            Flt w=top_corner_size*top_corner_image->aspect(),
                h=top_corner_size,
                y=rect.max.y+top_corner_offset.y;
            top_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.min.x-top_corner_offset.x, y, -w, h));
            top_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.max.x+top_corner_offset.x, y,  w, h));
         }
         if(bottom_corner_image)
         {
            Flt w=bottom_corner_size*bottom_corner_image->aspect(),
                h=bottom_corner_size,
                y=rect.min.y+bottom_corner_offset.y;
            if(bottom_corner_image==top_corner_image)CHS(h); // mirror vertically if it's the same as top
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.min.x-bottom_corner_offset.x, y, -w, h));
            bottom_corner_image->draw(side_color, TRANSPARENT, Rect_C(rect.max.x+bottom_corner_offset.x, y,  w, h));
         }
      }
   }
}
/******************************************************************************/
void Panel::drawLines(C Color &line_color, C Rect &rect)C
{
   rect.draw(line_color, false);
   if(side_stretch)
   {
      Flt y[]={rect.min.y+bottom_offset, rect.max.y+top_offset};
      Flt x[3][2]= // [y][x]
      {
         {rect.min.x-   top_corner_offset.x, rect.max.x+   top_corner_offset.x},
         {rect.min.x-   left_right_offset.x, rect.max.x+   left_right_offset.x},
         {rect.min.x-bottom_corner_offset.x, rect.max.x+bottom_corner_offset.x},
      };
      if(   top_image)Rect(x[0][0], y[1]            , x[0][1], y[1]+top_size).draw(line_color, false);
      if(bottom_image)Rect(x[2][0], y[0]-bottom_size, x[2][1], y[0]         ).draw(line_color, false);
      if(left_right_image)
      {
         Rect(x[1][0], y[0], x[1][0]-left_right_size, y[1]).draw(line_color, false);
         Rect(x[1][1], y[0], x[1][1]+left_right_size, y[1]).draw(line_color, false);
      }
      if(top_corner_image)
      {
         Flt w =top_corner_size*top_corner_image->aspect(),
             h =top_corner_size,
             y1=y[1]+h;
         Rect(x[0][0], y[1], x[0][0]-w, y1).draw(line_color, false);
         Rect(x[0][1], y[1], x[0][1]+w, y1).draw(line_color, false);
      }
      if(bottom_corner_image)
      {
         Flt w=bottom_corner_size*bottom_corner_image->aspect(),
             h=bottom_corner_size,
             y0=y[0]-h, y1=y[0];
         Rect(x[2][0], y0, x[2][0]-w, y1).draw(line_color, false);
         Rect(x[2][1], y0, x[2][1]+w, y1).draw(line_color, false);
      }
   }else
   {
      if(   top_image)Rect_C(rect.centerX()/*+top_offset.x*/, rect.max.y+top_offset, top_size*top_image->aspect(), top_size).draw(line_color, false);
      if(bottom_image)
      {
         Flt h=bottom_size;
         Rect_C(rect.centerX()/*+bottom_offset.x*/, rect.min.y+bottom_offset, bottom_size*bottom_image->aspect(), h).draw(line_color, false);
      }
      if(left_right_image)
      {
         Flt w=left_right_size*left_right_image->aspect(),
             h=left_right_size,
             y=rect.centerY()+left_right_offset.y;
         Rect_C(rect.min.x-left_right_offset.x, y, -w, h).draw(line_color, false);
         Rect_C(rect.max.x+left_right_offset.x, y,  w, h).draw(line_color, false);
      }
      if(top_corner_image)
      {
         Flt w=top_corner_size*top_corner_image->aspect(),
             h=top_corner_size,
             y=rect.max.y+top_corner_offset.y;
         Rect_C(rect.min.x-top_corner_offset.x, y, -w, h).draw(line_color, false);
         Rect_C(rect.max.x+top_corner_offset.x, y,  w, h).draw(line_color, false);
      }
      if(bottom_corner_image)
      {
         Flt w=bottom_corner_size*bottom_corner_image->aspect(),
             h=bottom_corner_size,
             y=rect.min.y+bottom_corner_offset.y;
         Rect_C(rect.min.x-bottom_corner_offset.x, y, -w, h).draw(line_color, false);
         Rect_C(rect.max.x+bottom_corner_offset.x, y,  w, h).draw(line_color, false);
      }
   }
}
/******************************************************************************/
#pragma pack(push, 1)
struct PanelDesc
{
   Bool  center_stretch, side_stretch, center_shadow;
   Byte  shadow_opacity;
   Color center_color, border_color, side_color, blur_color;
   Flt   shadow_radius, shadow_offset,
         border_size, center_scale, top_size, bottom_size, left_right_size, top_corner_size, bottom_corner_size,
         top_offset, bottom_offset;
   Vec2  left_right_offset, top_corner_offset, bottom_corner_offset;
};
struct PanelDesc2
{
   Byte  center_stretch, shadow_opacity;
   Color center_color, border_color, blur_color;
   Flt   shadow_offset, shadow_radius, border_size, center_scale, corner_size, top_size;
   Vec2  corner_offset, top_offset;
};
#pragma pack(pop)

Bool Panel::save(File &f, CChar *path)C
{
   f.putUInt (CC4_GSTL);
   f.cmpUIntV(6       ); // version

   PanelDesc desc;

   Unaligned(desc.       center_stretch,        center_stretch);
   Unaligned(desc.         side_stretch,          side_stretch);
   Unaligned(desc.       center_shadow ,        center_shadow );
   Unaligned(desc.       shadow_opacity,        shadow_opacity);
   Unaligned(desc.       center_color  ,        center_color  );
   Unaligned(desc.       border_color  ,        border_color  );
   Unaligned(desc.         side_color  ,          side_color  );
   Unaligned(desc.         blur_color  ,          blur_color  );
   Unaligned(desc.       shadow_radius ,        shadow_radius );
   Unaligned(desc.       shadow_offset ,        shadow_offset );
   Unaligned(desc.       border_size   ,        border_size   );
   Unaligned(desc.       center_scale  ,        center_scale  );
   Unaligned(desc.          top_size   ,           top_size   );
   Unaligned(desc.       bottom_size   ,        bottom_size   );
   Unaligned(desc.   left_right_size   ,    left_right_size   );
   Unaligned(desc.   top_corner_size   ,    top_corner_size   );
   Unaligned(desc.bottom_corner_size   , bottom_corner_size   );
   Unaligned(desc.          top_offset ,           top_offset );
   Unaligned(desc.       bottom_offset ,        bottom_offset );
   Unaligned(desc.   left_right_offset ,    left_right_offset );
   Unaligned(desc.   top_corner_offset ,    top_corner_offset );
   Unaligned(desc.bottom_corner_offset , bottom_corner_offset );
   f<<desc;
   f.putAsset(       center_image.id());
   f.putAsset(       border_image.id());
   f.putAsset(          top_image.id());
   f.putAsset(       bottom_image.id());
   f.putAsset(   left_right_image.id());
   f.putAsset(   top_corner_image.id());
   f.putAsset(bottom_corner_image.id());
   f.putAsset(        panel_image.id());
   return f.ok();
}
Bool Panel::load(File &f, CChar *path)
{
   if(f.getUInt()==CC4_GSTL)switch(f.decUIntV()) // version
   {
      case 6:
      {
         PanelDesc desc; if(f.get(desc))
         {
            Unaligned(       center_stretch, desc.       center_stretch);
            Unaligned(         side_stretch, desc.         side_stretch);
            Unaligned(        center_shadow, desc.       center_shadow );
            Unaligned(       shadow_opacity, desc.       shadow_opacity);
            Unaligned(       center_color  , desc.       center_color  );
            Unaligned(       border_color  , desc.       border_color  );
            Unaligned(         side_color  , desc.         side_color  );
            Unaligned(         blur_color  , desc.         blur_color  );
            Unaligned(       shadow_radius , desc.       shadow_radius );
            Unaligned(       shadow_offset , desc.       shadow_offset );
            Unaligned(       border_size   , desc.       border_size   );
            Unaligned(       center_scale  , desc.       center_scale  );
            Unaligned(          top_size   , desc.          top_size   );
            Unaligned(       bottom_size   , desc.       bottom_size   );
            Unaligned(   left_right_size   , desc.   left_right_size   );
            Unaligned(   top_corner_size   , desc.   top_corner_size   );
            Unaligned(bottom_corner_size   , desc.bottom_corner_size   );
            Unaligned(          top_offset , desc.          top_offset );
            Unaligned(       bottom_offset , desc.       bottom_offset );
            Unaligned(   left_right_offset , desc.   left_right_offset );
            Unaligned(   top_corner_offset , desc.   top_corner_offset );
            Unaligned(bottom_corner_offset , desc.bottom_corner_offset );

            center_image.require(f.getAssetID(), path);
            border_image.require(f.getAssetID(), path);
               top_image.require(f.getAssetID(), path);
            bottom_image.require(f.getAssetID(), path);
        left_right_image.require(f.getAssetID(), path);
        top_corner_image.require(f.getAssetID(), path);
     bottom_corner_image.require(f.getAssetID(), path);
             panel_image.require(f.getAssetID(), path);

            if(f.ok())return true;
         }
      }break;

      case 5:
      {
         PanelDesc desc; if(f.get(desc))
         {
            Unaligned(       center_stretch, desc.       center_stretch);
            Unaligned(         side_stretch, desc.         side_stretch);
            Unaligned(        center_shadow, desc.       center_shadow );
            Unaligned(       shadow_opacity, desc.       shadow_opacity);
            Unaligned(       center_color  , desc.       center_color  );
            Unaligned(       border_color  , desc.       border_color  );
            Unaligned(         side_color  , desc.         side_color  );
            Unaligned(         blur_color  , desc.         blur_color  );
            Unaligned(       shadow_radius , desc.       shadow_radius );
            Unaligned(       shadow_offset , desc.       shadow_offset );
            Unaligned(       border_size   , desc.       border_size   );
            Unaligned(       center_scale  , desc.       center_scale  );
            Unaligned(          top_size   , desc.          top_size   );
            Unaligned(       bottom_size   , desc.       bottom_size   );
            Unaligned(   left_right_size   , desc.   left_right_size   );
            Unaligned(   top_corner_size   , desc.   top_corner_size   );
            Unaligned(bottom_corner_size   , desc.bottom_corner_size   );
            Unaligned(          top_offset , desc.          top_offset );
            Unaligned(       bottom_offset , desc.       bottom_offset );
            Unaligned(   left_right_offset , desc.   left_right_offset );
            Unaligned(   top_corner_offset , desc.   top_corner_offset );
            Unaligned(bottom_corner_offset , desc.bottom_corner_offset );

            center_image.require(f._getAsset(), path);
            border_image.require(f._getAsset(), path);
               top_image.require(f._getAsset(), path);
            bottom_image.require(f._getAsset(), path);
        left_right_image.require(f._getAsset(), path);
        top_corner_image.require(f._getAsset(), path);
     bottom_corner_image.require(f._getAsset(), path);
             panel_image.require(f._getAsset(), path);

            if(f.ok())return true;
         }
      }break;

      case 4:
      {
         #pragma pack(push, 1)
         struct PanelDesc4
         {
            Byte  center_stretch, shadow_opacity;
            Color center_color, border_color, blur_color;
            Flt   shadow_offset, shadow_radius, border_size, center_scale, corner_size, top_size, bottom_size, left_right_size;
            Vec2  corner_offset, top_offset, bottom_offset, left_right_offset;
         }desc;
         #pragma pack(pop)
         if(f.get(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.    shadow_opacity );
            Unaligned(    center_color  , desc.    center_color   );
            Unaligned(    border_color  , desc.    border_color   );
            Unaligned(      blur_color  , desc.      blur_color   );
            Unaligned(    shadow_offset , desc.    shadow_offset  );
            Unaligned(    shadow_radius , desc.    shadow_radius  );
            Unaligned(    border_size   , desc.    border_size    );
            Unaligned(    center_scale  , desc.    center_scale   );
            Unaligned(top_corner_size   , desc.    corner_size    );
            Unaligned(       top_size   , desc.       top_size    );
            Unaligned(    bottom_size   , desc.    bottom_size    );
            Unaligned(left_right_size   , desc.left_right_size    );
            Unaligned(top_corner_offset , desc.    corner_offset  );
            Unaligned(       top_offset , desc.       top_offset.y);
            Unaligned(    bottom_offset , desc.    bottom_offset.y);
            Unaligned(left_right_offset , desc.left_right_offset  );

                   top_image.require(f._getStr2(), path);
                center_image.require(f._getStr2(), path);
                border_image.require(f._getStr2(), path);
            top_corner_image.require(f._getStr2(), path);
                bottom_image.require(f._getStr2(), path);
            left_right_image.require(f._getStr2(), path);

              side_stretch      =false;
              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
             center_shadow=!Equal(shadow_offset, 0);

            if(f.ok())return true;
         }
      }break;

      case 3:
      {
         PanelDesc2 desc; if(f.get(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity);
            Unaligned(    center_color  , desc.center_color  );
            Unaligned(    border_color  , desc.border_color  );
            Unaligned(      blur_color  , desc.  blur_color  );
            Unaligned(    shadow_offset , desc.shadow_offset );
            Unaligned(    shadow_radius , desc.shadow_radius );
            Unaligned(    border_size   , desc.border_size   );
            Unaligned(    center_scale  , desc.center_scale  );
            Unaligned(top_corner_size   , desc.corner_size   );
            Unaligned(       top_size   , desc.   top_size   );
            Unaligned(top_corner_offset , desc.corner_offset );
            Unaligned(       top_offset , desc.   top_offset.y);

                   top_image.require(f._getStr(), path);
                center_image.require(f._getStr(), path);
                border_image.require(f._getStr(), path);
            top_corner_image.require(f._getStr(), path);

              side_stretch      =false;
              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
             center_shadow=!Equal(shadow_offset, 0);

            if(f.ok())return true;
         }
      }break;

      case 2:
      {
         PanelDesc2 desc; if(f.get(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity);
            Unaligned(    center_color  , desc.center_color  ); Swap(center_color.r, center_color.b);
            Unaligned(    border_color  , desc.border_color  ); Swap(border_color.r, border_color.b);
            Unaligned(      blur_color  , desc.  blur_color  ); Swap(  blur_color.r,   blur_color.b);
            Unaligned(    shadow_offset , desc.shadow_offset );
            Unaligned(    shadow_radius , desc.shadow_radius );
            Unaligned(    border_size   , desc.border_size   );
            Unaligned(    center_scale  , desc.center_scale  );
            Unaligned(top_corner_size   , desc.corner_size   );
            Unaligned(       top_size   , desc.   top_size   );
            Unaligned(top_corner_offset , desc.corner_offset );
            Unaligned(       top_offset , desc.   top_offset.y);

                   top_image.require(f._getStr(), path);
                center_image.require(f._getStr(), path);
                border_image.require(f._getStr(), path);
            top_corner_image.require(f._getStr(), path);

              side_stretch      =false;
              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
             center_shadow=!Equal(shadow_offset, 0);

            if(f.ok())return true;
         }
      }break;

      case 1:
      {
         #pragma pack(push, 4)
         struct PanelDesc1
         {
            Byte  shadow_opacity, center_stretch;
            VecB4 center_color, border_color, blur_color;
            Flt   center_scale, border_size, shadow_radius, shadow_offset;
         }desc;
         #pragma pack(pop)
         if(f.get(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity);
                          center_color  .set(Unaligned(desc.center_color.z), Unaligned(desc.center_color.y), Unaligned(desc.center_color.x), Unaligned(desc.center_color.w));
                          border_color  .set(Unaligned(desc.border_color.z), Unaligned(desc.border_color.y), Unaligned(desc.border_color.x), Unaligned(desc.border_color.w));
                            blur_color  .set(Unaligned(desc.  blur_color.z), Unaligned(desc.  blur_color.y), Unaligned(desc.  blur_color.x), Unaligned(desc.  blur_color.w));
            Unaligned(    shadow_offset , desc.shadow_offset);
            Unaligned(    shadow_radius , desc.shadow_radius);
            Unaligned(    border_size   , desc.border_size  );
            Unaligned(    center_scale  , desc.center_scale );
            Unaligned(top_corner_size   , desc.border_size  );
            Unaligned(       top_size   , desc.border_size  );
                      top_corner_offset =0;
                             top_offset =0;

                   top_image.require(f._getStr8(), path);
                center_image.require(f._getStr8(), path);
                border_image.require(f._getStr8(), path);
            top_corner_image.require(f._getStr8(), path);

              side_stretch      =false;
              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
             center_shadow=!Equal(shadow_offset, 0);

            if(f.ok())return true;
         }
      }break;

      case 0:
      {
         #pragma pack(push, 4)
         struct PanelDesc0
         {
            Byte  shadow_opacity, center_stretch;
            VecB4 center_color, border_color;
            Flt   center_scale, border_size, shadow_radius, shadow_offset;
         }desc;
         #pragma pack(pop)
         if(f.get(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity);
                          center_color  .set(Unaligned(desc.center_color.z), Unaligned(desc.center_color.y), Unaligned(desc.center_color.x), Unaligned(desc.center_color.w));
                          border_color  .set(Unaligned(desc.border_color.z), Unaligned(desc.border_color.y), Unaligned(desc.border_color.x), Unaligned(desc.border_color.w));
                            blur_color  .zero();
            Unaligned(    shadow_offset , desc.shadow_offset);
            Unaligned(    shadow_radius , desc.shadow_radius);
            Unaligned(    border_size   , desc.border_size  );
            Unaligned(    center_scale  , desc.center_scale );
            Unaligned(top_corner_size   , desc.border_size  );
            Unaligned(       top_size   , desc.border_size  );
                      top_corner_offset =0;
                             top_offset =0;

                   top_image.require(f._getStr8(), path);
                center_image.require(f._getStr8(), path);
                border_image.require(f._getStr8(), path);
            top_corner_image.require(f._getStr8(), path);

              side_stretch      =false;
              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
            center_shadow=!Equal(shadow_offset, 0);

            if(f.ok())return true;
         }
      }break;
   }
   reset(); return false;
}
Bool Panel::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Panel::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
void Panel::operator=(C Str &name)
{
   if(!load(name))Exit(MLT(S+"Can't load Panel \""       +name+"\"",
                       PL,S+u"Nie można wczytać Panel \""+name+"\""));
}
/******************************************************************************/
}
/******************************************************************************/
