/******************************************************************************/
#include "stdafx.h"
namespace EE{
#if 0
   #define TEX_ZERO HalfZero
   #define TEX_ONE  HalfOne
   #define SET_TEX(t, x, y) t.set(x, y)
#else
   #define TEX_ZERO 0
   #define TEX_ONE  255
   #define SET_TEX(t, x, y) t.set(x, y, 0, 0)
#endif
/******************************************************************************/
DisplayDraw::DisplayDraw()
{
  _text_depth=false;
}
/******************************************************************************
   static Vec2 posToScreen(Vec2 &pos   ); // convert from world  position to screen position, according to 'scale offset'
   static Vec2 screenToPos(Vec2 &screen); // convert from screen position to world  position, according to 'scale offset'
Vec2 DisplayDraw::posToScreen(Vec2 &pos   ){return D._scale * (pos+D._offset);}
Vec2 DisplayDraw::screenToPos(Vec2 &screen){return screen/D._scale-D._offset ;}
/******************************************************************************/
void DisplayDraw::lineX(C Color &color, Flt y, Flt x0, Flt x1)
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, VI_LINE);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(2))
   {
      v[0].pos.set(x0, y);
      v[1].pos.set(x1, y);
   }
   VI.end();
}
void DisplayDraw::lineY(C Color &color, Flt x, Flt y0, Flt y1)
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, VI_LINE);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(2))
   {
      v[0].pos.set(x, y0);
      v[1].pos.set(x, y1);
   }
   VI.end();
}
void DisplayDraw::lines(C Color &color, C Vec2 *point, Int points)
{
   if(point && points>1)
   {
      VI.color(color);
      if(points*SIZE(*point)>VI._mem_max)
      {
         VI.setType(VI_2D_FLAT, VI_LINE);
         Int  lines=points-1;
         FREP(lines)if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(2))
         {
            v[0].pos=point[i  ];
            v[1].pos=point[i+1];
         }
      }else
      {
         VI.setType(VI_2D_FLAT, VI_LINE|VI_STRIP);
         if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(points))CopyFast(v, point, points*SIZE(*v));
      }
      VI.end();
   }
}
/******************************************************************************/
void DisplayDraw::drawShadowBorders(Byte alpha, C Rect &rect, Flt shadow_radius)
{
   if(Gui.image_shadow)Gui.image_shadow->draw3x3Borders(Color(0, 0, 0, alpha), TRANSPARENT, Rect(rect.min.x-shadow_radius, rect.min.y-shadow_radius, rect.max.x+shadow_radius, rect.max.y+shadow_radius), shadow_radius, 0.33f);
}
void DisplayDraw::drawShadow(Byte alpha, C Rect &rect, Flt shadow_radius)
{
   if(Gui.image_shadow)Gui.image_shadow->draw3x3(Color(0, 0, 0, alpha), TRANSPARENT, Rect(rect.min.x-shadow_radius, rect.min.y-shadow_radius, rect.max.x+shadow_radius, rect.max.y+shadow_radius), shadow_radius, 0.33f);
}
/******************************************************************************/
void Image::draw(C Rect &rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::draw(C Color &color, C Color &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawVertical(C Rect &rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(_part.y,       0);
      v[1].tex.set(_part.y, _part.x);
      v[2].tex.set(      0,       0);
      v[3].tex.set(      0, _part.x);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawVertical(C Color &color, C Color &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(_part.y,       0);
      v[1].tex.set(_part.y, _part.x);
      v[2].tex.set(      0,       0);
      v[3].tex.set(      0, _part.x);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawFilter(C Rect &rect, FILTER_TYPE filter)C
{
   VecI2 pixel=Round(Renderer.screenToPixelSize(rect.size())).abs(); // get target pixel size
   if(pixel.x>w() || pixel.y>h())switch(filter) // if that size is bigger than the image resolution
   {
    //case FILTER_LINEAR: VI.shader(null); break;

      case FILTER_NONE:
      #if DX9
         Sh.h_ImageCol[0]->_sampler=&SamplerPoint;
      #elif DX11
         VI.shader(Sh.h_DrawTexPoint);
       //SamplerPoint.setPS(SSI_DEFAULT);
      #elif GL // in GL 'ShaderImage.Sampler' does not affect filtering, so modify it manually
         D.texBind(GL_TEXTURE_2D, _txtr); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      #endif
      break;

      case FILTER_CUBIC_FAST       :
      case FILTER_CUBIC_FAST_SMOOTH:
      case FILTER_CUBIC_FAST_SHARP : VI.shader(Sh.h_DrawTexCubicFast); break;

      case FILTER_BEST       :
      case FILTER_CUBIC      :
      case FILTER_CUBIC_SHARP: Sh.loadCubicShaders(); VI.shader(Sh.h_DrawTexCubic); break;
   }
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
      Sh.h_ColSize->set(Vec4(1.0f/hwSize(), hwSize()));
   }
   VI.end();
   if(filter==FILTER_NONE)
   {
   #if DX9
      Sh.h_ImageCol[0]->_sampler=null;
   #elif DX11
    //SamplerLinearClamp.setPS(SSI_DEFAULT);
   #elif GL
      if(!GL_ES || ImageTI[hwType()].precision<IMAGE_PRECISION_32) // GLES2/3 don't support filtering F32 textures, without this check reading from F32 textures will fail - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml
         {D.texBind(GL_TEXTURE_2D, _txtr); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);}
   #endif
   }
}
void Image::drawFilter(C Color &color, C Color &color_add, C Rect &rect, FILTER_TYPE filter)C
{
   VecI2 pixel=Round(Renderer.screenToPixelSize(rect.size())).abs(); // get target pixel size
   if(pixel.x>w() || pixel.y>h())switch(filter) // if that size is bigger than the image resolution
   {
    //case FILTER_LINEAR: VI.shader(null); break;

      case FILTER_NONE:
      #if DX9
         Sh.h_ImageCol[0]->_sampler=&SamplerPoint;
      #elif DX11
         VI.shader(Sh.h_DrawTexPointC);
       //SamplerPoint.setPS(SSI_DEFAULT);
      #elif GL // in GL 'ShaderImage.Sampler' does not affect filtering, so modify it manually
         D.texBind(GL_TEXTURE_2D, _txtr); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      #endif
      break;

      case FILTER_CUBIC_FAST       :
      case FILTER_CUBIC_FAST_SMOOTH:
      case FILTER_CUBIC_FAST_SHARP : VI.shader(Sh.h_DrawTexCubicFastC); break;

      case FILTER_BEST       :
      case FILTER_CUBIC      :
      case FILTER_CUBIC_SHARP: Sh.loadCubicShaders(); VI.shader(Sh.h_DrawTexCubicC); break;
   }
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
      Sh.h_ColSize->set(Vec4(1.0f/hwSize(), hwSize()));
   }
   VI.end();
   if(filter==FILTER_NONE)
   {
   #if DX9
      Sh.h_ImageCol[0]->_sampler=null;
   #elif DX11
    //SamplerLinearClamp.setPS(SSI_DEFAULT);
   #elif GL
      if(!GL_ES || ImageTI[hwType()].precision<IMAGE_PRECISION_32) // GLES2/3 don't support filtering F32 textures, without this check reading from F32 textures will fail - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml
         {D.texBind(GL_TEXTURE_2D, _txtr); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);}
   #endif
   }
}
/******************************************************************************
void Image::drawOutline(C Color &color, C Rect &rect, Flt tex_range)
{
   if(color.a)
   {
      VI.shader (Sh.h_OutlineI);
      VI.color  (color    );
      VI.image  (this     );
      VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
      {
         Vec2 tr=tex_range;
         v[0].pos.set(rect.min.x, rect.max.y);
         v[1].pos.set(rect.max.x, rect.max.y);
         v[2].pos.set(rect.min.x, rect.min.y);
         v[3].pos.set(rect.max.x, rect.min.y);
         v[0].tex.set(      0,       0);
         v[1].tex.set(_part.x,       0);
         v[2].tex.set(      0, _part.y);
         v[3].tex.set(_part.x, _part.y);
         Sh.h_ImgSize->set();
      }
      VI.end();
   }
}
/******************************************************************************/
#define DEFAULT_FILTER (MOBILE ? FILTER_LINEAR : DX9 ? FILTER_CUBIC_FAST : FILTER_CUBIC)
void Image::drawFs(                                    FIT_MODE fit, Int filter)C {drawFilter(                  T.fit(D.rect(), fit), (filter>=0) ? FILTER_TYPE(filter) : DEFAULT_FILTER);}
void Image::drawFs(C Color &color, C Color &color_add, FIT_MODE fit, Int filter)C {drawFilter(color, color_add, T.fit(D.rect(), fit), (filter>=0) ? FILTER_TYPE(filter) : DEFAULT_FILTER);}
/******************************************************************************/
void Image::drawPart(C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.min.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawPart(C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.min.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawPartVertical(C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[1].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.max.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.max.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawPartVertical(C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[1].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.max.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.max.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
/******************************************************************************/
void Image::drawRotate(C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      Vec2   c=(rotation_center ? *rotation_center : 0.5f);
             c.x*= size.x;
             c.y*=-size.y;
      Matrix m; m.orn().setRotateZ(angle); m.pos.set(center.x, center.y, 0).xy-=c*m.orn();
      v[0].pos.set(     0,       0)*=m;
      v[1].pos.set(size.x,       0)*=m;
      v[2].pos.set(     0, -size.y)*=m;
      v[3].pos.set(size.x, -size.y)*=m;
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end();
}
void Image::drawRotate(C Color &color, C Color &color_add, C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      Vec2   c=(rotation_center ? *rotation_center : 0.5f);
             c.x*= size.x;
             c.y*=-size.y;
      Matrix m; m.orn().setRotateZ(angle); m.pos.set(center.x, center.y, 0).xy-=c*m.orn();
      v[0].pos.set(     0,       0)*=m;
      v[1].pos.set(size.x,       0)*=m;
      v[2].pos.set(     0, -size.y)*=m;
      v[3].pos.set(size.x, -size.y)*=m;
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end();
}
/******************************************************************************/
void Image::drawMask(C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect)C
{
   Rect r=rect&mask_rect;
   if(  r.valid())
   {
      VI.color (color    );
      VI.color2(color_add);
      VI.image (this     );
      Sh.h_ImageCol[1]->set(mask); MaterialClear();
      VI.setType(VI_2D_TEX2, VI_STRIP);
      if(Vtx2DTex2 *v=(Vtx2DTex2*)VI.addVtx(4))
      {
         v[0].pos.set(r.min.x, r.max.y);
         v[1].pos.set(r.max.x, r.max.y);
         v[2].pos.set(r.min.x, r.min.y);
         v[3].pos.set(r.max.x, r.min.y);

         v[0].tex[0].x=v[2].tex[0].x=LerpR(rect.min.x, rect.max.x, r.min.x); // min x
         v[1].tex[0].x=v[3].tex[0].x=LerpR(rect.min.x, rect.max.x, r.max.x); // max x
         v[0].tex[0].y=v[1].tex[0].y=LerpR(rect.max.y, rect.min.y, r.max.y); // min y
         v[2].tex[0].y=v[3].tex[0].y=LerpR(rect.max.y, rect.min.y, r.min.y); // max y

         v[0].tex[1].x=v[2].tex[1].x=LerpR(mask_rect.min.x, mask_rect.max.x, r.min.x); // min x
         v[1].tex[1].x=v[3].tex[1].x=LerpR(mask_rect.min.x, mask_rect.max.x, r.max.x); // max x
         v[0].tex[1].y=v[1].tex[1].y=LerpR(mask_rect.max.y, mask_rect.min.y, r.max.y); // min y
         v[2].tex[1].y=v[3].tex[1].y=LerpR(mask_rect.max.y, mask_rect.min.y, r.min.y); // max y

         if(partial())
         {
            v[0].tex[0]*=_part.xy;
            v[1].tex[0]*=_part.xy;
            v[2].tex[0]*=_part.xy;
            v[3].tex[0]*=_part.xy;
         }
         if(mask.partial())
         {
            v[0].tex[1]*=mask._part.xy;
            v[1].tex[1]*=mask._part.xy;
            v[2].tex[1]*=mask._part.xy;
            v[3].tex[1]*=mask._part.xy;
         }
      }
      VI.end();
   }
}
/******************************************************************************/
void Image::drawTile(C Rect &rect, Flt tex_scale)C
{
   VI.image  (this);
   VI.wrap   (    );
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      Flt w=rect.w()*tex_scale,
          h=rect.h()*tex_scale*aspect();
      v[0].tex.set(0, 0);
      v[1].tex.set(w, 0);
      v[2].tex.set(0, h);
      v[3].tex.set(w, h);
   }
   VI.end();
}
void Image::drawTile(C Color &color, C Color &color_add, C Rect &rect, Flt tex_scale)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.wrap   (         );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      Flt w=rect.w()*tex_scale,
          h=rect.h()*tex_scale*aspect();
      v[0].tex.set(0, 0);
      v[1].tex.set(w, 0);
      v[2].tex.set(0, h);
      v[3].tex.set(w, h);
   }
   VI.end();
}
/******************************************************************************/
void Image::drawBorder(C Rect &rect, Flt b, Flt tex_scale, Flt tex_offset, Bool wrap_mode)C
{
   VI.image  (this);
   VI.wrapX  (    );
   VI.setType(VI_2D_TEX, wrap_mode ? VI_STRIP : 0);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(10))
   {
      Flt x0=rect.min.x, x1=rect.min.x+b, x2=rect.max.x-b, x3=rect.max.x,
          y0=rect.max.y, y1=rect.max.y-b, y2=rect.min.y+b, y3=rect.min.y;

      Flt scale=b*aspect(),
          w    =rect.w()/scale,
          h    =rect.h()/scale;
          b   /=         scale;

      if(wrap_mode)
      {
         v[0].pos.set(x0, y0);
         v[1].pos.set(x1, y1);

         v[2].pos.set(x3, y0);
         v[3].pos.set(x2, y1);

         v[4].pos.set(x3, y3);
         v[5].pos.set(x2, y2);

         v[6].pos.set(x0, y3);
         v[7].pos.set(x1, y2);

         v[8].pos.set(x0, y0);
         v[9].pos.set(x1, y1);

         v[0].tex.set(      0,       0);
         v[1].tex.set(      0, _part.y);
         v[2].tex.set(w      ,       0);
         v[3].tex.set(w      , _part.y);
         v[4].tex.set(w+h    ,       0);
         v[5].tex.set(w+h    , _part.y);
         v[6].tex.set(w+h+w  ,       0);
         v[7].tex.set(w+h+w  , _part.y);
         v[8].tex.set(w+h+w+h,       0);
         v[9].tex.set(w+h+w+h, _part.y);
      }else
      {
         v[0].pos.set(x0, y0);
         v[1].pos.set(x3, y0);
         v[2].pos.set(x3, y3);
         v[3].pos.set(x0, y3);
         v[4].pos.set(x1, y1);
         v[5].pos.set(x2, y1);
         v[6].pos.set(x2, y2);
         v[7].pos.set(x1, y2);
         v[8].pos.set(x3, y3);
         v[9].pos.set(x2, y2);

         v[0].tex.set(    0,       0);
         v[1].tex.set(w    ,       0);
         v[2].tex.set(w-h  ,       0);
         v[3].tex.set(  h  ,       0);
         v[4].tex.set(    b, _part.y);
         v[5].tex.set(w  -b, _part.y);
         v[6].tex.set(w-h+b, _part.y);
         v[7].tex.set(  h-b, _part.y);
         v[8].tex.set(h-w  ,       0);
         v[9].tex.set(h-w+b, _part.y);
      }

      if(tex_scale!=1 || tex_offset!=0)REP(10)v[i].tex.x=v[i].tex.x*tex_scale+tex_offset;

      if(wrap_mode)VI.end();else VI.flushIndexed(IndBufBorder, 4*2*3);
   }
   VI.clear();
}
void Image::drawBorder(C Color &color, C Color &color_add, C Rect &rect, Flt b, Flt tex_scale, Flt tex_offset, Bool wrap_mode)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.wrapX  (         );
   VI.setType(VI_2D_TEX, (wrap_mode ? VI_STRIP : 0)|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(10))
   {
      Flt x0=rect.min.x, x1=rect.min.x+b, x2=rect.max.x-b, x3=rect.max.x,
          y0=rect.max.y, y1=rect.max.y-b, y2=rect.min.y+b, y3=rect.min.y;

      Flt scale=b*aspect(),
          w    =rect.w()/scale,
          h    =rect.h()/scale;
          b   /=         scale;

      if(wrap_mode)
      {
         v[0].pos.set(x0, y0);
         v[1].pos.set(x1, y1);

         v[2].pos.set(x3, y0);
         v[3].pos.set(x2, y1);

         v[4].pos.set(x3, y3);
         v[5].pos.set(x2, y2);

         v[6].pos.set(x0, y3);
         v[7].pos.set(x1, y2);

         v[8].pos.set(x0, y0);
         v[9].pos.set(x1, y1);

         v[0].tex.set(      0,       0);
         v[1].tex.set(      0, _part.y);
         v[2].tex.set(w      ,       0);
         v[3].tex.set(w      , _part.y);
         v[4].tex.set(w+h    ,       0);
         v[5].tex.set(w+h    , _part.y);
         v[6].tex.set(w+h+w  ,       0);
         v[7].tex.set(w+h+w  , _part.y);
         v[8].tex.set(w+h+w+h,       0);
         v[9].tex.set(w+h+w+h, _part.y);
      }else
      {
         v[0].pos.set(x0, y0);
         v[1].pos.set(x3, y0);
         v[2].pos.set(x3, y3);
         v[3].pos.set(x0, y3);
         v[4].pos.set(x1, y1);
         v[5].pos.set(x2, y1);
         v[6].pos.set(x2, y2);
         v[7].pos.set(x1, y2);
         v[8].pos.set(x3, y3);
         v[9].pos.set(x2, y2);

         v[0].tex.set(    0,       0);
         v[1].tex.set(w    ,       0);
         v[2].tex.set(w-h  ,       0);
         v[3].tex.set(  h  ,       0);
         v[4].tex.set(    b, _part.y);
         v[5].tex.set(w  -b, _part.y);
         v[6].tex.set(w-h+b, _part.y);
         v[7].tex.set(  h-b, _part.y);
         v[8].tex.set(h-w  ,       0);
         v[9].tex.set(h-w+b, _part.y);
      }

      if(tex_scale!=1 || tex_offset!=0)REP(10)v[i].tex.x=v[i].tex.x*tex_scale+tex_offset;

      if(wrap_mode)VI.end();else VI.flushIndexed(IndBufBorder, 4*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void Image::draw3x3Borders(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(16))
   {
      Flt w=rect.w(), W=((w>=0) ? Min(w/2, border_size) : Max(w/2, -border_size)),
          h=rect.h(), H=((h>=0) ? Min(h/2, border_size) : Max(h/2, -border_size)),
          x0=rect.min.x, x3=rect.max.x, x1=x0+W, x2=x3-W,
          y0=rect.max.y, y3=rect.min.y, y1=y0-H, y2=y3+H,
          tex_frac1=1-tex_frac;

      v[ 0].pos.set(x0, y0);
      v[ 1].pos.set(x1, y0);
      v[ 2].pos.set(x2, y0);
      v[ 3].pos.set(x3, y0);
      v[ 4].pos.set(x0, y1);
      v[ 5].pos.set(x1, y1);
      v[ 6].pos.set(x2, y1);
      v[ 7].pos.set(x3, y1);
      v[ 8].pos.set(x0, y2);
      v[ 9].pos.set(x1, y2);
      v[10].pos.set(x2, y2);
      v[11].pos.set(x3, y2);
      v[12].pos.set(x0, y3);
      v[13].pos.set(x1, y3);
      v[14].pos.set(x2, y3);
      v[15].pos.set(x3, y3);

      v[ 0].tex.set(        0,         0);
      v[ 1].tex.set(tex_frac ,         0);
      v[ 2].tex.set(tex_frac1,         0);
      v[ 3].tex.set(        1,         0);
      v[ 4].tex.set(        0, tex_frac );
      v[ 5].tex.set(tex_frac , tex_frac );
      v[ 6].tex.set(tex_frac1, tex_frac );
      v[ 7].tex.set(        1, tex_frac );
      v[ 8].tex.set(        0, tex_frac1);
      v[ 9].tex.set(tex_frac , tex_frac1);
      v[10].tex.set(tex_frac1, tex_frac1);
      v[11].tex.set(        1, tex_frac1);
      v[12].tex.set(        0,         1);
      v[13].tex.set(tex_frac ,         1);
      v[14].tex.set(tex_frac1,         1);
      v[15].tex.set(        1,         1);

      if(partial())REP(16)v[i].tex*=_part.xy;

      VI.flushIndexed(IndBufPanel, (3*3-1)*2*3);
   }
   VI.clear();
}
void Image::draw3x3(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(16))
   {
      Flt w=rect.w(), W=((w>=0) ? Min(w/2, border_size) : Max(w/2, -border_size)),
          h=rect.h(), H=((h>=0) ? Min(h/2, border_size) : Max(h/2, -border_size)),
          x0=rect.min.x, x3=rect.max.x, x1=x0+W, x2=x3-W,
          y0=rect.max.y, y3=rect.min.y, y1=y0-H, y2=y3+H,
          tex_frac1=1-tex_frac;

      v[ 0].pos.set(x0, y0);
      v[ 1].pos.set(x1, y0);
      v[ 2].pos.set(x2, y0);
      v[ 3].pos.set(x3, y0);
      v[ 4].pos.set(x0, y1);
      v[ 5].pos.set(x1, y1);
      v[ 6].pos.set(x2, y1);
      v[ 7].pos.set(x3, y1);
      v[ 8].pos.set(x0, y2);
      v[ 9].pos.set(x1, y2);
      v[10].pos.set(x2, y2);
      v[11].pos.set(x3, y2);
      v[12].pos.set(x0, y3);
      v[13].pos.set(x1, y3);
      v[14].pos.set(x2, y3);
      v[15].pos.set(x3, y3);

      v[ 0].tex.set(        0,         0);
      v[ 1].tex.set(tex_frac ,         0);
      v[ 2].tex.set(tex_frac1,         0);
      v[ 3].tex.set(        1,         0);
      v[ 4].tex.set(        0, tex_frac );
      v[ 5].tex.set(tex_frac , tex_frac );
      v[ 6].tex.set(tex_frac1, tex_frac );
      v[ 7].tex.set(        1, tex_frac );
      v[ 8].tex.set(        0, tex_frac1);
      v[ 9].tex.set(tex_frac , tex_frac1);
      v[10].tex.set(tex_frac1, tex_frac1);
      v[11].tex.set(        1, tex_frac1);
      v[12].tex.set(        0,         1);
      v[13].tex.set(tex_frac ,         1);
      v[14].tex.set(tex_frac1,         1);
      v[15].tex.set(        1,         1);

      if(partial())REP(16)v[i].tex*=_part.xy;

      VI.flushIndexed(IndBufPanel, 3*3*2*3);
   }
   VI.clear();
}
void Image::draw3x3Vertical(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac)C
{
   VI.color  (color    );
   VI.color2 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(16))
   {
      Flt w=rect.w(), W=((w>=0) ? Min(w/2, border_size) : Max(w/2, -border_size)),
          h=rect.h(), H=((h>=0) ? Min(h/2, border_size) : Max(h/2, -border_size)),
          x0=rect.min.x, x3=rect.max.x, x1=x0+W, x2=x3-W,
          y0=rect.max.y, y3=rect.min.y, y1=y0-H, y2=y3+H,
          tex_frac1=1-tex_frac;

      v[ 0].pos.set(x0, y0);
      v[ 1].pos.set(x1, y0);
      v[ 2].pos.set(x2, y0);
      v[ 3].pos.set(x3, y0);
      v[ 4].pos.set(x0, y1);
      v[ 5].pos.set(x1, y1);
      v[ 6].pos.set(x2, y1);
      v[ 7].pos.set(x3, y1);
      v[ 8].pos.set(x0, y2);
      v[ 9].pos.set(x1, y2);
      v[10].pos.set(x2, y2);
      v[11].pos.set(x3, y2);
      v[12].pos.set(x0, y3);
      v[13].pos.set(x1, y3);
      v[14].pos.set(x2, y3);
      v[15].pos.set(x3, y3);

      v[ 0].tex.set(        1,         0);
      v[ 1].tex.set(        1, tex_frac );
      v[ 2].tex.set(        1, tex_frac1);
      v[ 3].tex.set(        1,         1);
      v[ 4].tex.set(tex_frac1,         0);
      v[ 5].tex.set(tex_frac1, tex_frac );
      v[ 6].tex.set(tex_frac1, tex_frac1);
      v[ 7].tex.set(tex_frac1,         1);
      v[ 8].tex.set(tex_frac ,         0);
      v[ 9].tex.set(tex_frac , tex_frac );
      v[10].tex.set(tex_frac , tex_frac1);
      v[11].tex.set(tex_frac ,         1);
      v[12].tex.set(        0,         0);
      v[13].tex.set(        0, tex_frac );
      v[14].tex.set(        0, tex_frac1);
      v[15].tex.set(        0,         1);

      if(partial())REP(16)v[i].tex*=_part.xy;

      VI.flushIndexed(IndBufPanel, 3*3*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void Image::drawCubeFace(C Color &color, C Color &color_add, C Rect &rect, DIR_ENUM face)C
{
   if(InRange(face, 6))
   {
      if(!Sh.h_DrawCubeFace)Sh.h_DrawCubeFace=Sh.get("DrawCubeFace");
      VI.shader (Sh.h_DrawCubeFace);
      VI.color  (color    );
      VI.color2 (color_add);
      VI.setType(VI_2D_FONT, VI_STRIP);
      Sh.h_ImageRfl[0]->set(this);
      if(Vtx2DFont *v=(Vtx2DFont*)VI.addVtx(4))
      {
         v[0].pos.set(rect.min.x, rect.max.y);
         v[1].pos.set(rect.max.x, rect.max.y);
         v[2].pos.set(rect.min.x, rect.min.y);
         v[3].pos.set(rect.max.x, rect.min.y);
         switch(face)
         { // here 'shade' is used as the 3rd tex Z coordinate
            case DIR_RIGHT  : v[0].tex.set( 1,  1); v[0].shade= 1;
                              v[1].tex.set( 1,  1); v[1].shade=-1;
                              v[2].tex.set( 1, -1); v[2].shade= 1;
                              v[3].tex.set( 1, -1); v[3].shade=-1; break;
            case DIR_LEFT   : v[0].tex.set(-1,  1); v[0].shade=-1;
                              v[1].tex.set(-1,  1); v[1].shade= 1;
                              v[2].tex.set(-1, -1); v[2].shade=-1;
                              v[3].tex.set(-1, -1); v[3].shade= 1; break;
            case DIR_UP     : v[0].tex.set(-1,  1); v[0].shade=-1;
                              v[1].tex.set( 1,  1); v[1].shade=-1;
                              v[2].tex.set(-1,  1); v[2].shade= 1;
                              v[3].tex.set( 1,  1); v[3].shade= 1; break;
            case DIR_DOWN   : v[0].tex.set(-1, -1); v[0].shade= 1;
                              v[1].tex.set( 1, -1); v[1].shade= 1;
                              v[2].tex.set(-1, -1); v[2].shade=-1;
                              v[3].tex.set( 1, -1); v[3].shade=-1; break;
            case DIR_FORWARD: v[0].tex.set(-1,  1); v[0].shade= 1;
                              v[1].tex.set( 1,  1); v[1].shade= 1;
                              v[2].tex.set(-1, -1); v[2].shade= 1;
                              v[3].tex.set( 1, -1); v[3].shade= 1; break;
            case DIR_BACK   : v[0].tex.set( 1,  1); v[0].shade=-1;
                              v[1].tex.set(-1,  1); v[1].shade=-1;
                              v[2].tex.set( 1, -1); v[2].shade=-1;
                              v[3].tex.set(-1, -1); v[3].shade=-1; break;
         }
      }
      VI.end();
   }
}
/******************************************************************************/
void Image::draw3D(C Color &color, Flt size, Flt angle, C Vec &pos, ALPHA_MODE alpha)C
{
#if DX9 || GL // DX10+ should support all sizes
   Sh.h_ColSize->set(_part.xy);
#endif
   D .alpha  (alpha);
   VI.image  (this );
   VI.setType(VI_3D_BILB, VI_STRIP);
   if(Vtx3DBilb *v=(Vtx3DBilb*)VI.addVtx(4))
   {
      v[0].pos      =v[1].pos      =v[2].pos      =v[3].pos      =pos;
   #if GPU_HALF_SUPPORTED
      v[0].vel_angle=v[1].vel_angle=v[2].vel_angle=v[3].vel_angle.set(HalfZero, HalfZero, HalfZero, angle);
   #else
      v[0].vel_angle=v[1].vel_angle=v[2].vel_angle=v[3].vel_angle.set(       0,        0,        0, angle);
   #endif
      v[0].color    =v[1].color    =v[2].color    =v[3].color    =color;
      v[0].size     =v[1].size     =v[2].size     =v[3].size     =size;
      SET_TEX(v[0].tex, TEX_ZERO, TEX_ZERO);
      SET_TEX(v[1].tex, TEX_ONE , TEX_ZERO);
      SET_TEX(v[2].tex, TEX_ZERO, TEX_ONE );
      SET_TEX(v[3].tex, TEX_ONE , TEX_ONE );
   }
   VI.end();
}
/******************************************************************************/
#pragma pack(push, 4)
struct GpuVolume
{
   Flt min_steps,
       max_steps,
       density_factor,
       precision     ;
   Vec size  ,
       pixels,
       inside;
};
#pragma pack(pop)
void Image::drawVolume(C Color &color, C Color &color_add, C OBox &obox, Flt voxel_density_factor, Flt precision, Int min_steps, Int max_steps)C
{
   if(Frustum(obox) && Renderer.canReadDepth())
   {
      if(!Sh.h_Volume)
      {
         Sh.h_Volume=GetShaderParam("Volume");
         Sh.h_Volume0[0]=Sh.get("Volume0"); Sh.h_Volume0[1]=Sh.get("Volume0LA");
         Sh.h_Volume1[0]=Sh.get("Volume1"); Sh.h_Volume1[1]=Sh.get("Volume1LA");
         Sh.h_Volume2[0]=Sh.get("Volume2"); Sh.h_Volume2[1]=Sh.get("Volume2LA");
      }
      GpuVolume v;
      Bool LA=(type()==IMAGE_R8G8); // check 'type' instead of 'hwType' because volumetric clouds may want to be set as RG, but got RGBA, however only RG was set

      v.min_steps     =Mid(Flt(min_steps),        2.0f, 1024.0f);
      v.max_steps     =Mid(Flt(max_steps), v.min_steps, 1024.0f);
      v.density_factor=Mid(voxel_density_factor, EPS_GPU, 1-EPS_GPU);
      v.precision     =precision;
      v.size          =obox.box.size()*0.5f;
      v.pixels        =size3();

      Matrix temp;
      temp.pos=obox.center();
      temp.x  =obox.matrix.x*v.size.x;
      temp.y  =obox.matrix.y*v.size.y;
      temp.z  =obox.matrix.z*v.size.z;
      SetOneMatrix(temp);

      Vec delta=CamMatrix.pos-temp.pos;
      v.inside.x=Dot(delta, obox.matrix.x);
      v.inside.y=Dot(delta, obox.matrix.y);
      v.inside.z=Dot(delta, obox.matrix.z);

      D .alphaFactor(TRANSPARENT);
      Sh.h_ImageVol[0]->set(T        ); Sh.h_ImageVol[0]->_sampler=&SamplerLinearClamp;
      Sh.h_Color   [0]->set(color    );
      Sh.h_Color   [1]->set(color_add);
      Sh.h_Volume     ->set(v);

      D.cull (true);
      D.alpha(ALPHA_BLEND_DEC);
      Renderer.needDepthRead();

      Flt e=Frustum.view_quad_max_dist;
      if(v.inside.x>=-v.size.x-e && v.inside.x<=v.size.x+e
      && v.inside.y>=-v.size.y-e && v.inside.y<=v.size.y+e
      && v.inside.z>=-v.size.z-e && v.inside.z<=v.size.z+e)
      {
         D.depth(false);

         if(v.inside.x>=-v.size.x+e && v.inside.x<=v.size.x-e
         && v.inside.y>=-v.size.y+e && v.inside.y<=v.size.y-e
         && v.inside.z>=-v.size.z+e && v.inside.z<=v.size.z-e)Sh.h_Volume2[LA]->begin();
         else                                                 Sh.h_Volume1[LA]->begin();
         MshrBoxR.set().drawFull();
         ShaderEnd();
      }else
      {
         D .depth     (true );
         D .depthWrite(false);
         Sh.h_Volume0[LA]->begin(); MshrBox.set().drawFull();
         ShaderEnd();
      }

      Sh.h_ImageVol[0]->_sampler=null;
      MaterialClear(); // because D.alphaFactor and ImageCol
   }
}
/******************************************************************************/
}
/******************************************************************************/
