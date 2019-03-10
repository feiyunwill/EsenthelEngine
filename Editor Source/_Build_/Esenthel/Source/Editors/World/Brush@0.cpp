/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
WorldBrushClass Brush;
/******************************************************************************/

/******************************************************************************/
   WorldBrushClass& WorldBrushClass::create(GuiObj &parent)
   {
      ::BrushClass::create(parent, Vec2(WorldEdit.rect().w(), -WorldEdit.rect().h()));
      ssize .set(0.1f);
      sspeed.set(0.2f);
      return T;
   }
   bool WorldBrushClass::hasMsWheelFocus()C {return WorldEdit.cur.onViewport();}
   bool WorldBrushClass::affects(C Edge2 &edge, C VecI2 &xz)C
   {
      return Cuts(edge, (Rect_LD(xz.x, xz.y, 1, 1)*WorldEdit.areaSize()).extend(size*rotate_scale));
   }
   RectI WorldBrushClass::affectedAreas(C Edge2 &edge)C
   {
      return WorldEdit.worldToArea(Rect(edge).extend(size*rotate_scale))&WorldEdit.visible_area_1;
   }
   RectI WorldBrushClass::affectedAreas(int mouse_button)C // always calculate on demand, and for example not in 'update' because that method may get called later in frame than when other codes need it (resulting in 1 frame delay which could result in wrong behavior)
   {
      Rect rect(0, -1); // invalid at start

      // touches
      REPA(WorldEdit.cur_touch)
      {
         Cursor &cur=WorldEdit.cur_touch[i]; if(cur.valid())Include(rect, Rect(WorldEdit.cur_touch[i].edge()));
      }

      // mouse
      if(WorldEdit.cur.valid() && WorldEdit.cur.on() && (mouse_button<0 || Ms.b(mouse_button)))Include(rect, Rect(WorldEdit.cur.edge())); // if mouse cursor valid and pressed

      if(rect.valid())return WorldEdit.worldToArea(rect.extend(size*rotate_scale))&WorldEdit.visible_area_1;
      return RectI(0, -1); // return invalid
   }
   bool WorldBrushClass::affected(C VecI2 &xz)C // if area of 'xz' coordinates is affected by brush, this is more precise than the 'affectedAreas'
   {
      bool touch_on_viewport=false;

      // first the touches
      REPA(WorldEdit.cur_touch)if(WorldEdit.cur_touch[i].onViewport())
      {
         touch_on_viewport=true;
         if(WorldEdit.cur_touch[i].valid())if(affects(WorldEdit.cur_touch[i].edge(), xz))return true;
      }

      // now the mouse
      if(WorldEdit.cur.valid()) // if mouse cursor valid
         if(touch_on_viewport ? WorldEdit.cur.on() : true) // if there is already some touch on the viewport then add from mouse only if it's pressed
            if(affects(WorldEdit.cur.edge(), xz))return true;

      return false;
   }
   flt WorldBrushClass::power(C Vec2 &pos, bool texture, Cursor &cur)C // get brush power (0..1) at world XZ position, 'texture'=if allow applying image coverage, 'cur'=get power from specified cursor
   {
      flt f;

      // distance from shape
      switch(shape())
      {
         default       :
         case BS_CIRCLE: f=Dist(pos, cur.edge()); break;
         case BS_SQUARE: f=Abs ((NearestPointOnEdge(pos, cur.edge())-pos).rotateCosSin(rotate_cos, rotate_sin)).max(); break;
      }

      if(f<=size)
      {
         f/=size;

         // soften
         f=Sat((f-(1-soft))/soft); // soften
         f=BlendSmoothCube(f);     // blend

         // fade
         flt d=1, v;
         v=pos.x/WorldEdit.areaSize(); MIN(d, v-WorldEdit.visible_area_1.min.x  );
                                       MIN(d, 1+WorldEdit.visible_area_1.max.x-v);
         v=pos.y/WorldEdit.areaSize(); MIN(d, v-WorldEdit.visible_area_1.min.y  );
                                       MIN(d, 1+WorldEdit.visible_area_1.max.y-v);
         f*=SmoothCube(Max(0, d));

         // texture
         if(texture && image() && image_sw.is())
         {
            if(pattern())
            {
               Vec2 d=pos*pattern_scale;
                    d.y=1-d.y;
               f*=image_sw.pixelFCubic(d.x*(image_sw.w()-1), d.y*(image_sw.h()-1), false);
            }else
            {
               Vec2 d=(pos-cur.pos().xz())/size;
                    d.rotateCosSin(rotate_cos, rotate_sin);
                    d=d*0.5f+0.5f;
                    d.y=1-d.y;
               f*=image_sw.pixelFCubic(d.x*(image_sw.w()-1), d.y*(image_sw.h()-1), true);
            }
         }
         return f;
      }
      return 0;
   }
   flt WorldBrushClass::powerTotal(C Vec2 &pos, bool texture)C // get brush power (0..1) at world XZ position, 'texture'=if allow applying image coverage, get power from all cursors
   {
      flt    p=0; REP(WorldEdit.curTotal()){Cursor &cur=WorldEdit.curAll(i); if(cur.valid() && cur.on())p+=power(pos, texture, cur);}
      return p;
   }
   Vec WorldBrushClass::randomPos(bool align_heightmap)C
   {
      // TODO: brush random position including image
      if(image() && image_sw.is())
      {
      }
      
      Vec2 pos=0;
      switch(shape())
      {
         default       :
         case BS_CIRCLE:
         {
            flt full=1-soft,
                soft_range=soft*0.5f;
            pos=Random(Circle(full + soft_range));
            flt d=pos.length();
            if( d>full)
            {
               d-=full;
               d/=soft_range;
               d =Lerp(Pow(d, 1.2f), Pow(d, 3.0f), d);
               pos.setLength(full+d*soft);
            }
         }break;

         case BS_SQUARE:
         {
            flt full=1-soft,
                soft_range=soft*0.5f;
            pos=Random(Rect_C(Vec2(0,0), full + soft_range));
            flt d=Abs(pos.x);
            if( d>full)
            {
               d-=full;
               d/=soft_range;
               d =Lerp(Pow(d, 1.2f), Pow(d, 3.0f), d);
               pos.x=(full+d*soft)*SignBool(pos.x>0);
            }
                d=Abs(pos.y);
            if( d>full)
            {
               d-=full;
               d/=soft_range;
               d =Lerp(Pow(d, 1.2f), Pow(d, 3.0f), d);
               pos.y=(full+d*soft)*SignBool(pos.y>0);
            }
         }break;
      }

      pos*=size;
      pos+=WorldEdit.cur.pos().xz();
      return Vec(pos.x, align_heightmap ? WorldEdit.hmHeightNearest(pos) : WorldEdit.cur.pos().y, pos.y);
   }
   void WorldBrushClass::setVisibility() {visible(WorldEdit.mode()!=WorldView::OBJECT || ObjPaint.available());}
   void WorldBrushClass::update(C GuiPC &gpc)
{
      ::BrushClass::update(gpc);
      setVisibility();
      if(visible() && gpc.visible)
      {
         // parameters
         flt cds=(bsize() ? CamMoveScale(WorldEdit.v4.perspective())/25 : 1); // camera distance scale
         size =Lerp(1.00f, 25.0f, ssize ()) * cds;
         soft =Lerp(EPS ,  1.0f, ssoft ());
         speed=Lerp(0.05f,  0.8f, sspeed());

         flt ass=32.0f/WorldEdit.areaSize(); // area size scale
         pattern_scale*=ass;

         switch(WorldEdit.mode())
         {
            case WorldView::HEIGHTMAP:
            {
               if(HeightBrush.mode()>=0)switch(HeightBrush.mode())
               {
               #if MOBILE
                  case HB_ADD    :
                  case HB_SUB    : speed*=cds*Time.d()*(bsize() ? 0.5f : 1)*ass; break;
               #else
                  case HB_ADD_SUB: speed*=cds*Time.d()*(bsize() ? 0.5f : 1)*ass; break;
               #endif
                  case HB_LEVEL  : speed*=    Time.d()*3.5f ; break;
                  case HB_FLAT   : speed*=cds*Time.d()     ; break;
                  case HB_AVG    : speed*=    Time.d()* 6  ; break;
                  case HB_SOFT   : speed*=    Time.d()*16  ; break;
                  case HB_NOISE  : speed*=ass*Time.d()* 0.3f; break;
                  default        : speed*=              0  ; break;
               }else
               if(MtrlBrush.sel!=-1 || MtrlBrush.max1() || MtrlBrush.max2())
               {
                  speed*=Time.d()*15;
               }
            }break;
            
            case WorldView::OBJECT: // object paint
            {
               speed*=1.5f;
            }break;
         }
      }
   }
/******************************************************************************/
