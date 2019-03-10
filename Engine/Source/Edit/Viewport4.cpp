/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************

   'Arrows' uses one GuiObj instead of multiple Buttons so that Mouse/Touch can switch buttons while being continuously pressed.

   Normally MT.guiObj sticks to just one Object when pressed.

/******************************************************************************/
static Bool AdjustToGround(Viewport4 &v4) {return  v4.horizontal() && !v4.fpp();} // World  Editor mode
static Bool AdjustToCamera(Viewport4 &v4) {return !v4.horizontal() && !v4.fpp();} // Object Editor mode
/******************************************************************************/
void Viewport4::View::setViewportCamera()
{
   viewport.setDisplayView();
   camera  .set();
}
/******************************************************************************/
void Viewport4::ViewportEx::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      Color rect_color=T.rect_color; T.rect_color.a=0;

      super::draw(gpc);

      T.rect_color=rect_color;
      if(Viewport4 *v4=(Viewport4*)user)
         if(v4->sel()<0 && rect_color.a)
      {
         Rect r=rect()+gpc.offset;
         D.clip(gpc.clip);
         // down
         if(this==&v4->view[VIEW_FRONT].viewport
         || this==&v4->view[VIEW_LEFT ].viewport)D.lineX(rect_color, r.max.y-D._pixel_size_2.y, r.min.x, r.max.x); // align down = -pixel_size
         // right
         if(this==&v4->view[VIEW_DIAGONAL].viewport
         || this==&v4->view[VIEW_LEFT    ].viewport)D.lineY(rect_color, r.min.x+D._pixel_size_2.x, r.min.y, r.max.y); // align right = +pixel_size
      }
   }
}
/******************************************************************************/
// CUBE
/******************************************************************************/
Viewport4::Cube& Viewport4::Cube::create(Viewport4 &v4, View &view) {super::create(); T.v4=&v4; T.view=&view; mesh="Gui/Viewcube/0.mesh"; return T;}

void Viewport4::Cube::setView(C GuiPC &gpc, Display::ViewportSettings &view, Camera &camera, MatrixM &matrix)
{
   if(T.view)
   {
      // backup
      camera=ActiveCam;
      view.get();

      D.view(rect()+gpc.offset, 0.1f, 100, DegToRad(30), FOV_Y);
      T.view->camera.set();
      matrix.identity(); matrix.pos.z+=3.3f; matrix.pos*=T.view->camera.matrix;
   }
}
void Viewport4::Cube::resetView(C Display::ViewportSettings &view, C Camera &camera)
{
   view  .set();
   camera.set();
}
GuiObj* Viewport4::Cube::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   GuiObj *ret=null;
   if(mesh && super::test(gpc, pos, mouse_wheel))
   {
      Display::ViewportSettings view; Camera camera; MatrixM matrix;
      setView(gpc, view, camera, matrix);
      VecD pos3D; Vec dir; ScreenToPosDir(pos, pos3D, dir);
      if(Sweep(pos3D, dir*D.viewRange(), *mesh, &matrix, null, null, null, &part, false))ret=this;
      resetView(view, camera);
   }
   return ret;
}
void Viewport4::Cube::update(C GuiPC &gpc)
{
   super::update(gpc);
   if(view && v4)
   {
      Touch *touch=null; REPA(Touches)if(Touches[i].guiObj()==this){touch=&Touches[i]; break;}
      if(touch || Gui.ms()==this)
      {
         Bool possible_drag=(touch ? touch->life()>=DragTime+Time.ad() || touch->selecting() : Ms.life()>=DragTime+Time.ad()),
              rot          =(touch ? touch->on() : (Ms.b(0) || Ms.b(2) || Ms.b(4))),
              move         =(touch ? false       : (Ms.b(1) || Ms.b(3)           ));
         if(rot && !move)
         {
            if(!touch)Ms.freeze();
            View &view=(v4->lock() ? v4->view[VIEW_FRONT] : *T.view);
            VecD  pos =view.camera.matrix.pos;
            if(!(!Kb.ctrlCmd() &&  Kb.alt()))view.camera.yaw  -=(touch ? touch->ad().x*3.0f : Ms.d().x); // disable X rotation when only Alt  pressed
            if(!( Kb.ctrlCmd() && !Kb.alt()))view.camera.pitch+=(touch ? touch->ad().y*3.0f : Ms.d().y); // disable Y rotation when only Ctrl pressed
            view.camera.setSpherical();
            if(v4->fpp())view.camera+=pos-view.camera.matrix.pos;
            if(possible_drag)part=-1;
         }
         if(move)
         {
            Ms.freeze();
            Camera &cam=view->camera;
            VecD at=cam.at   ; cam.transformByMouse(0, 0, ((v4->horizontal()^rot) ? CAMH_MOVE_XZ : CAMH_MOVE) | CAMH_NO_VEL_UPDATE | CAMH_NO_SET);
            VecD d =cam.at-at;
            if(v4->lock())REPA(v4->view)if(&v4->view[i]!=view)v4->view[i].camera+=d;
         }
         if((touch ? touch->rs() : Ms.br(0)) && !possible_drag && mesh && InRange(part, *mesh))
         {
            Flt y=0, p=0;
            CChar8  *name=mesh->parts[part].name;
            if(Equal(name, "l" ))y=-PI_2;else
            if(Equal(name, "r" ))y= PI_2;else
            if(Equal(name, "f" ))y= PI  ;else
            if(Equal(name, "b" ))y=  0  ;else
            if(Equal(name, "u" )){y=AlignRound(view->camera.yaw, PI_2); p=-PI_2;}else // v4->_default_yaw
            if(Equal(name, "d" )){y=AlignRound(view->camera.yaw, PI_2); p= PI_2;}else

            if(Equal(name, "lb"))y=-PI_4;else
            if(Equal(name, "lf"))y=-PI_4-PI_2;else
            if(Equal(name, "rf"))y= PI_4+PI_2;else
            if(Equal(name, "rb"))y= PI_4;else

            if(Equal(name, "lu")){y=-PI_2; p=-PI_4;}else
            if(Equal(name, "ru")){y= PI_2; p=-PI_4;}else
            if(Equal(name, "uf")){y= PI  ; p=-PI_4;}else
            if(Equal(name, "ub")){y=  0  ; p=-PI_4;}else

            if(Equal(name, "ld")){y=-PI_2; p= PI_4;}else
            if(Equal(name, "rd")){y= PI_2; p= PI_4;}else
            if(Equal(name, "df")){y= PI  ; p= PI_4;}else
            if(Equal(name, "db")){y=  0  ; p= PI_4;}else

            if(Equal(name, "lub")){y=-PI_4     ; p=-PI_4;}else
            if(Equal(name, "luf")){y=-PI_4-PI_2; p=-PI_4;}else
            if(Equal(name, "ruf")){y= PI_4+PI_2; p=-PI_4;}else
            if(Equal(name, "rub")){y= PI_4     ; p=-PI_4;}else

            if(Equal(name, "ldb")){y=-PI_4     ; p= PI_4;}else
            if(Equal(name, "ldf")){y=-PI_4-PI_2; p= PI_4;}else
            if(Equal(name, "rdf")){y= PI_4+PI_2; p= PI_4;}else
            if(Equal(name, "rdb")){y= PI_4     ; p= PI_4;}

            if(Kb.ctrlCmd() && Kb.shift())p=SignBool(p>0)*(PI_4*2.0f/3);else // 0.66
            if(                Kb.shift())p=SignBool(p>0)*(PI_4*0.75f );else // 0.75
            if(Kb.ctrlCmd()              )p=SignBool(p>0)*(PI_4*0.5f  );     // 0.50

            if(!v4->lock())v4->_rotate_view=view;else
            {
                  v4->_rotate_view=&v4->view[VIEW_FRONT];
               y+=v4->_rotate_view->camera.yaw  -view->camera.yaw  ;
               p+=v4->_rotate_view->camera.pitch-view->camera.pitch;
            }
            v4->_rotate_time=0;
            v4->_rotate_yaw  .set(v4->_rotate_view->camera.yaw  , y);
            v4->_rotate_pitch.set(v4->_rotate_view->camera.pitch, p);
         }
      }
   }
}
void Viewport4::Cube::draw(C GuiPC &gpc)
{
   if(mesh && visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      ALPHA_MODE alpha=D.alpha();
      Display::ViewportSettings view; Camera camera; MatrixM matrix;
      setView(gpc, view, camera, matrix);
      D.sampler3D(); D.depthLock  (false); mesh->drawBlend(matrix); if(Gui.ms()==this && InRange(part, *mesh))mesh->parts[part].drawBlend(&NoTemp(Vec4(2)));
      D.sampler2D(); D.depthUnlock(     ); D.depthWrite(true); D.alpha(alpha); // reset 'depthWrite' and 'alpha' because they can be modified by 'drawBlend'
      resetView(view, camera);
   }
}
/******************************************************************************/
// ZOOM
/******************************************************************************/
Viewport4::Zoom& Viewport4::Zoom::create(Viewport4 *v4, View *view) {super::create(); T.v4=v4; T.view=view; return T;}

void Viewport4::Zoom::update(C GuiPC &gpc)
{
   zoom=0;
   if(gpc.enabled)
   {
      if(Gui.ms()==this && Ms.b(0)){zoom-=Ms.d().y; Ms.freeze();} REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())zoom-=Touches[i].ad().y*2.0f;
      if(zoom && v4 && view)
      {
         Flt scale=ScaleFactor(zoom);
         if(v4->lock())REPA(v4->view)
         {
            if(v4->perspective()){Clamp(v4->view[i].camera.dist*=scale, v4->minDist(), v4->maxDist()); v4->view[i].camera.setSpherical();}else v4->view[i].viewport.fov*=scale;
         }else
         {
            if(v4->perspective()){Clamp(    view  ->camera.dist*=scale, v4->minDist(), v4->maxDist());     view  ->camera.setSpherical();}else     view  ->viewport.fov*=scale;
         }
      }
   }
}
void Viewport4::Zoom::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      Rect rect=T.rect()+gpc.offset;
      rect.draw(Color(192, 200, 232, 64));
      rect.draw(Color(0, 0, 0, 128), false);
      {
         VI.color(Color(0, 0, 0, 64));
         Flt offset=-0.2f*Log2((v4 && view) ? v4->perspective() ? view->camera.dist : view->viewport.fov : ActiveCam.dist),
             x0=rect.lerpX(0.2f),
             x1=rect.lerpX(0.8f);
         REP(3){Flt y=rect.min.y+Frac(offset+(i*rect.h())/3, rect.h()); VI.line(Vec2(x0, y), Vec2(x1, y));}
         VI.end();
      }
   }
}
/******************************************************************************/
// DIGITAL PAD
/******************************************************************************/
#define DPAD_INNER 0.33f

Viewport4::DPad & Viewport4::DPad ::create(Viewport4 *v4, View *view) {super::create(); T.v4=v4; T.view=view; return T;}
Viewport4::DPadY& Viewport4::DPadY::create(Viewport4 *v4, View *view) {super::create(); T.v4=v4; T.view=view; return T;}

GuiObj* Viewport4::DPad::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   return (visible() && gpc.visible && Cuts(pos, Circle(rect().h()*0.5f, rect().center()+gpc.offset))) ? this : null;
}
void Viewport4::DPad::update(C GuiPC &gpc)
{
   touched=false;
   axis   =0;
   if(gpc.enabled)
   {
      C Vec2 *pos=null; if(Gui.ms()==this && Ms.b(0))pos=&Ms.pos(); REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on()){pos=&Touches[i].pos(); break;}
      if(pos)
      {
         Circle inner(rect().h()*0.5f*DPAD_INNER, rect().center()+gpc.offset);
         if(!Cuts(*pos, inner))
         {
            touched=true;
            Vec2 delta=*pos-inner.pos; angle=AlignRound(AngleFast(delta), PI_4);
            CosSin(delta.x, delta.y, angle); axis.set(SignEps(delta.x), SignEps(delta.y));
            if(v4 && view)
            {
               delta*=v4->moveScale(*view);
               Vec move;
               if(AdjustToGround(*v4)){if(view->camera.matrix.y.y<0)CHS(delta.y); move.y=0; CosSin(move.x, move.z, view->camera.yaw+Angle(delta)); move*=delta.length();}else
               if(AdjustToCamera(*v4))move=view->camera.matrix.x*delta.x + view->camera.matrix.y*delta.y;else
                                      move=view->camera.matrix.x*delta.x + view->camera.matrix.z*delta.y;
               if(v4->lock())REPAO(v4->view).camera+=move;else view->camera+=move;
            }
         }
      }
   }
}
void Viewport4::DPadY::update(C GuiPC &gpc)
{
   dir=0;
   if(gpc.enabled)
   {
      C Vec2 *pos=null; if(Gui.ms()==this && Ms.b(0))pos=&Ms.pos(); REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on()){pos=&Touches[i].pos(); break;}
      if(pos)
      {
         dir=Sign(pos->y-(rect().centerY()+gpc.offset.y));
         if(v4 && view)
         {
            Vec move=(AdjustToGround(*v4) ? Vec(0,1,0) : AdjustToCamera(*v4) ? view->camera.matrix.z : view->camera.matrix.y)*(v4->moveScale(*view)*dir);
            if(v4->lock())REPAO(v4->view).camera+=move;else view->camera+=move;
         }
      }
   }
}
void Viewport4::DPad::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      Circle circle(rect().h()*0.5f, rect().center()+gpc.offset);
      Flt r2=circle.r*DPAD_INNER;
      circle.drawPie(Color(0, 192, 255, 64), r2);
      if(touched)circle.drawPie(Color(0, 192, 255, 192), r2, angle-PI_4/2, PI_4);
      // draw lines
      {
         VI.color(Color(0, 0, 0, 64));
         REP(8){Flt c, s; CosSin(c, s, i*PI_4 + PI_4/2); VI.line(Vec2(circle.pos.x+c*circle.r, circle.pos.y+s*circle.r), Vec2(circle.pos.x+c*r2, circle.pos.y+s*r2));}
         VI.end();
      }
      circle.r=r2; circle.draw(Color(0, 0, 0, 64));
   }
}
void Viewport4::DPadY::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      Rect rect=T.rect()+gpc.offset; Vec2 right=rect.lerp(0.66f, 0.5f);
      VI.color(Color(0, 192, 255, 64));
      VI.tri(rect.lu(), right, rect.ld()); // left
      VI.tri(rect.lu(), rect.ru(), right); // top right
      VI.tri(rect.rd(), rect.ld(), right); // bottom right
      VI.end();
      if(dir>0)Quad2(rect.left(), rect.lu(), rect.ru  (), right).draw(Color(0, 192, 255, 192), true);else
      if(dir<0)Quad2(rect.rd  (), rect.ld(), rect.left(), right).draw(Color(0, 192, 255, 192), true);
      D.line(Color(0, 0, 0, 64), rect.left(), right);
   }
}
/******************************************************************************/
// ANALOG PAD
/******************************************************************************/
Viewport4::APad & Viewport4::APad ::create(Viewport4 &v4, View &view) {super::create(); T.v4=&v4; T.view=&view; return T;}
Viewport4::APadY& Viewport4::APadY::create(Viewport4 &v4, View &view) {super::create(); T.v4=&v4; T.view=&view; return T;}

GuiObj* Viewport4::APad::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   return (visible() && gpc.visible && Cuts(pos, Circle(rect().h()*0.5f, rect().center()+gpc.offset))) ? this : null;
}
void Viewport4::APad::update(C GuiPC &gpc)
{
   touched=false;
   if(gpc.enabled)
   {
      C Vec2 *pos=null; Bool pushed=false; if(Gui.ms()==this && Ms.b(0)){pos=&Ms.pos(); pushed=Ms.bp(0);} REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on()){pos=&Touches[i].pos(); pushed=Touches[i].pd(); break;}
      if(pos)
      {
         touched=true; cur=*pos-(rect().center()+gpc.offset); if(pushed)start=cur;
         Vec2 delta=(cur-start)/(rect().h()*0.5f)*v4->moveScale(*view);
         Vec  move;
         if(AdjustToGround(*v4)){if(view->camera.matrix.y.y<0)CHS(delta.y); move.y=0; CosSin(move.x, move.z, view->camera.yaw+Angle(delta)); move*=delta.length();}else
         if(AdjustToCamera(*v4))move=view->camera.matrix.x*delta.x + view->camera.matrix.y*delta.y;else
                                move=view->camera.matrix.x*delta.x + view->camera.matrix.z*delta.y;
         if(v4->lock())REPAO(v4->view).camera+=move;else view->camera+=move;
      }
   }
}
void Viewport4::APadY::update(C GuiPC &gpc)
{
   touched=false;
   if(gpc.enabled)
   {
      C Vec2 *pos=null; Bool pushed=false; if(Gui.ms()==this && Ms.b(0)){pos=&Ms.pos(); pushed=Ms.bp(0);} REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on()){pos=&Touches[i].pos(); pushed=Touches[i].pd(); break;}
      if(pos)
      {
         touched=true; cur=pos->y-(rect().centerY()+gpc.offset.y); if(pushed)start=cur;
         Vec move=(AdjustToGround(*v4) ? Vec(0,1,0) : AdjustToCamera(*v4) ? view->camera.matrix.z : view->camera.matrix.y)*(v4->moveScale(*view)*(cur-start)/rect().h());
         if(v4->lock())REPAO(v4->view).camera+=move;else view->camera+=move;
      }
   }
}
void Viewport4::APad::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      Circle c(rect().h()*0.5f, rect().center()+gpc.offset);
      c.draw(Color(192, 192, 192,  64));
      c.draw(Color(255, 255, 255, 192), false);
      if(touched)
      {
         Vec2 begin=start+rect().center()+gpc.offset,
              end  =cur  +rect().center()+gpc.offset;
         c.r*=0.5f;
         c.pos=end;
         c.draw(Color(192, 192, 192,  64));
         c.draw(Color(255, 255, 255, 192), false);
         D.line(Color(255, 255, 255, 192), begin, end);
      }
   }
}
void Viewport4::APadY::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      Rect rect=T.rect()+gpc.offset;
      rect.draw(Color(192, 192, 192,  64));
      rect.draw(Color(255, 255, 255, 192), false);
      if(touched)
      {
         Flt begin=rect.centerY()+start,
             end  =rect.centerY()+cur  ;
         rect.setC(rect.centerX(), end, 0.06f, 0.03f);
         rect.draw (Color(192, 192, 192,  64));
         rect.draw (Color(255, 255, 255, 192), false);
            D.lineY(Color(255, 255, 255, 192), rect.centerX(), begin, end);
      }
   }
}
/******************************************************************************/
// ZOOM
/******************************************************************************/
Viewport4::Drag & Viewport4::Drag ::create(Viewport4 &v4, View &view) {super::create(); T.v4=&v4; T.view=&view; return T;}
Viewport4::DragY& Viewport4::DragY::create(Viewport4 &v4, View &view) {super::create(); T.v4=&v4; T.view=&view; return T;}

GuiObj* Viewport4::Drag::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   return (visible() && gpc.visible && Cuts(pos, Circle(rect().h()*0.5f, rect().center()+gpc.offset))) ? this : null;
}
void Viewport4::Drag::update(C GuiPC &gpc)
{
   if(gpc.enabled)
   {
      Vec2 delta(0,0); if(Gui.ms()==this && Ms.b(0)){delta-=Ms.d(); Ms.freeze();} REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())delta-=Touches[i].ad()*2.0f;
      if(  delta.any())
      {
         delta*=v4->moveScale(*view, false);
         Vec move;
         if(AdjustToGround(*v4)){if(view->camera.matrix.y.y<0)CHS(delta.y); move.y=0; CosSin(move.x, move.z, view->camera.yaw+Angle(delta)); move*=delta.length();}else
         if(AdjustToCamera(*v4))move=view->camera.matrix.x*delta.x + view->camera.matrix.y*delta.y;else
                                move=view->camera.matrix.x*delta.x + view->camera.matrix.z*delta.y;
         if(v4->lock())REPAO(v4->view).camera+=move;else view->camera+=move;
      }
   }
}
void Viewport4::DragY::update(C GuiPC &gpc)
{
   if(gpc.enabled)
   {
      Flt delta=0; if(Gui.ms()==this && Ms.b(0)){delta-=Ms.d().y; Ms.freeze();} REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())delta-=Touches[i].ad().y*2.0f;
      if( delta)
      {
         Vec move=(AdjustToGround(*v4) ? Vec(0,1,0) : AdjustToCamera(*v4) ? view->camera.matrix.z : view->camera.matrix.y)*(v4->moveScale(*view, false)*delta);
         if(v4->lock())REPAO(v4->view).camera+=move;else view->camera+=move;
      }
   }
}
void Viewport4::Drag::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      Circle c(rect().h()*0.5f, rect().center()+gpc.offset);
      c.draw(Color(192, 200, 232, 64));
      c.draw(Color(0, 0, 0, 128), false);
   }
}
void Viewport4::DragY::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      D.clip(gpc.clip);
      Rect rect=T.rect()+gpc.offset;
      rect.draw(Color(192, 200, 232, 64));
      rect.draw(Color(0, 0, 0, 128), false);
   }
}
/******************************************************************************/
// ARROWS
/******************************************************************************/
Viewport4::Arrows& Viewport4::Arrows::create(Viewport4 &v4, View &view) {super::create(); T.v4=&v4; T.view=&view; return T;}
GuiObj&            Viewport4::Arrows::rect  (C Rect &rect)
{
   Flt x0=rect.lerpX(0.00f), y0=rect.lerpY(0.0f),
       x1=rect.lerpX(0.33f), y1=rect.lerpY(0.5f),
       x2=rect.lerpX(0.66f), y2=rect.lerpY(1.0f),
       x3=rect.lerpX(1.00f);

   arrow_rect[DIR_LEFT   ].set(x0, y0, x1, y1);
   arrow_rect[DIR_BACK   ].set(x1, y0, x2, y1);
   arrow_rect[DIR_RIGHT  ].set(x2, y0, x3, y1);
   arrow_rect[DIR_DOWN   ].set(x0, y1, x1, y2);
   arrow_rect[DIR_FORWARD].set(x1, y1, x2, y2);
   arrow_rect[DIR_UP     ].set(x2, y1, x3, y2);

   return super::rect(rect);
}
void Viewport4::Arrows::update(C GuiPC &gpc)
{
   Zero(arrow_pushed);
   if(gpc.visible && visible() && v4 && view)
   {
                   if(           Gui.ms()==this &&         Ms.b (0)){Vec2 pos=        Ms.pos()-gpc.offset; REPA(arrow_rect)if(Cuts(pos, arrow_rect[i])){arrow_pushed[i]=true; break;}}
      REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on( )){Vec2 pos=Touches[i].pos()-gpc.offset; REPA(arrow_rect)if(Cuts(pos, arrow_rect[i])){arrow_pushed[i]=true; break;}}
      Vec move=0;
      if(AdjustToGround(*v4))
      {
         VecI2 di(arrow_pushed[DIR_RIGHT]-arrow_pushed[DIR_LEFT], arrow_pushed[DIR_FORWARD]-arrow_pushed[DIR_BACK]);
         if(di.any()){if(view->camera.matrix.y.y<0)CHS(di.y); CosSin(move.x, move.z, view->camera.yaw+Angle(Vec2(di)));}
      }else
      if(AdjustToCamera(*v4))
      {
         if(arrow_pushed[DIR_RIGHT  ])move+=view->camera.matrix.x;
         if(arrow_pushed[DIR_LEFT   ])move-=view->camera.matrix.x;
         if(arrow_pushed[DIR_FORWARD])move+=view->camera.matrix.y;
         if(arrow_pushed[DIR_BACK   ])move-=view->camera.matrix.y;
      }else
      {
         if(arrow_pushed[DIR_RIGHT  ])move+=view->camera.matrix.x;
         if(arrow_pushed[DIR_LEFT   ])move-=view->camera.matrix.x;
         if(arrow_pushed[DIR_FORWARD])move+=view->camera.matrix.z;
         if(arrow_pushed[DIR_BACK   ])move-=view->camera.matrix.z;
      }
      if(arrow_pushed[DIR_UP  ])move+=(AdjustToGround(*v4) ? Vec(0, 1, 0) : AdjustToCamera(*v4) ? view->camera.matrix.z : view->camera.matrix.y);
      if(arrow_pushed[DIR_DOWN])move-=(AdjustToGround(*v4) ? Vec(0, 1, 0) : AdjustToCamera(*v4) ? view->camera.matrix.z : view->camera.matrix.y);
      move*=v4->moveScale(*view); if(v4->lock())REPAO(v4->view).camera+=move;else view->camera+=move;
   }
}
void Viewport4::Arrows::draw(C GuiPC &gpc)
{
   if(gpc.visible && visible())
   {
      D.clip(gpc.clip);
      Button b; b.create().disabled(true); b.text_size=0.75f; // set disabled to be half transparent
      REPA(arrow_rect)
      {
         if(i==DIR_UP  )b.text=(AdjustToCamera(*v4) ? "Fw" : "Up");else
         if(i==DIR_DOWN)b.text=(AdjustToCamera(*v4) ? "Bk" : "Dn");else
                        b.text.clear();
         b._on=arrow_pushed[i];
         b.rect(arrow_rect[i]);
         b.draw(gpc);
      }
      if(GuiSkin *skin=b.getSkin())
      {
         Color color=skin->button.disabled_color;
         if(skin->slidebar.left.image)
         {
            skin->slidebar.left.image->drawFit        (color, TRANSPARENT, arrow_rect[DIR_LEFT]+gpc.offset);
            skin->slidebar.left.image->drawFitVertical(color, TRANSPARENT, arrow_rect[DIR_BACK]+gpc.offset);
         }
         if(skin->slidebar.right.image)
         {
            skin->slidebar.right.image->drawFit        (color, TRANSPARENT, arrow_rect[DIR_RIGHT  ]+gpc.offset);
            skin->slidebar.right.image->drawFitVertical(color, TRANSPARENT, arrow_rect[DIR_FORWARD]+gpc.offset);
         }
      }
   }
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
Viewport4& Viewport4::create(void (*draw)(Viewport&), Flt default_pitch, Flt default_yaw, Flt default_dist, Flt from, Flt range)
{
  _sel        =0;
  _perspective=true;
  _horizontal =true;
  _keyboard   =true;
  _fpp        =false;
  _lock       =true;
#if DESKTOP
  _orn_mode   =ORIENT_TOP_LEFT;
  _move_mode  =MOVE_NONE;
#else
  _orn_mode   =ORIENT_RIGHT;
  _move_mode  =MOVE_DPAD;
  _draw_zoom  =true;
#endif
  _rect       =D.rect();
  _focus      =null;
  _last       =&view[0];
  _rotate_view=null;

  _fpp_speed    =5;
  _persp_fov    =DegToRad(70);
  _default_pitch=default_pitch;
  _default_yaw  =default_yaw  ;
  _default_dist =default_dist ;

   REPA(view)
   {
      view[i].viewport.create(draw, this);
      view[i].viewport.rect_color.set(64);
      view[i].viewport.from =from ;
      view[i].viewport.range=range;
      view[i].viewport.fov  =_persp_fov;
   }
   REPA(_cube  )_cube  [i].create( T,  view[i]);
   REPA(_zoom  )_zoom  [i].create(&T, &view[i]);
   REPA(_dpad  )_dpad  [i].create(&T, &view[i]);
   REPA(_dpady )_dpady [i].create(&T, &view[i]);
   REPA(_apad  )_apad  [i].create( T,  view[i]);
   REPA(_apady )_apady [i].create( T,  view[i]);
   REPA(_drag  )_drag  [i].create( T,  view[i]);
   REPA(_dragy )_dragy [i].create( T,  view[i]);
   REPA(_arrows)_arrows[i].create( T,  view[i]);
   resetPosOrn();
   sel(-1);
   return T;
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Viewport4::View* Viewport4::getView(GuiObj *go)
{
   if(go)REPA(view)if(go==&view[i].viewport)return &view[i];
   return null;
}
Viewport4::View* Viewport4::getViewCtrl(GuiObj *go)
{
   if(go)REPA(view)
      if(go==& view  [i].viewport
      || go==&_cube  [i]
      || go==&_zoom  [i]
      || go==&_dpad  [i]
      || go==&_dpady [i]
      || go==&_apad  [i]
      || go==&_apady [i]
      || go==&_drag  [i]
      || go==&_dragy [i]
      || go==&_arrows[i]
      )return &view  [i];
   return null;
}
Flt  Viewport4::minDist  (                     )C {return 0.01f;}
Flt  Viewport4::maxDist  (                     )C {return _last->viewport.range*0.5f;}
Flt  Viewport4::moveScale(View &view, Bool time)C {return (fpp() ? fppSpeed() : 1.3f*(perspective() ? Max(horizontal() ? 2.5f : 0.0f, view.camera.dist)*Tan(view.viewport.fov/2) : view.viewport.fov))*(time ? Time.d() : 1)*(Kb.shift() ? 4 : 1);}
Bool Viewport4::visible  (                     )C {REPA(view)if(view[i].viewport.visible())return true; return false;}
/******************************************************************************/
Viewport4& Viewport4::perspective(Bool on)
{
   if(perspective()!=on)
   {
      T._perspective=on;
      REPA(view)
      {
         if(on)
         {
            view[i].camera  .dist    =view[i].viewport.fov/Tan(perspFov()/2);
            view[i].viewport.fov_mode=FOV_Y;
            view[i].viewport.fov     =perspFov();
         }else
         {
            view[i].viewport.fov_mode=FOV_ORTHO_Y;
            view[i].viewport.fov     =Tan(perspFov()/2)*view[i].camera.dist;
            view[i].camera  .dist    =view[i].viewport.range*0.5f;
         }
      }
   }
   return T;
}
Viewport4& Viewport4::lock(Bool on)
{
   if(on!=lock())toggleLock();
   return T;
}
Viewport4& Viewport4::fpp(Bool on)
{
   T._fpp=on;
   return T;
}
Viewport4& Viewport4::horizontal(Bool on)
{
   T._horizontal=on;
   return T;
}
Viewport4& Viewport4::keyboard(Bool on)
{
   T._keyboard=on;
   return T;
}
Viewport4& Viewport4::sel(Int view)
{
   Clamp(view, -1, 3);
   if(T._sel!=view)
   {
      T._sel=view;
      if(InRange(T._sel, T.view))_last=&T.view[T._sel]; // if we're selecting one view then set it as last to help with setting default view/cam for it without needing the mouse to hover over it
      if(visible())visible(true);
      setRect();
   }
   return T;
}
Viewport4& Viewport4::fppSpeed(Flt speed) {T._fpp_speed=Max(0, speed); return T;}
Viewport4& Viewport4::perspFov(Flt fov  )
{
  _persp_fov=fov;
   if(perspective())REPAO(view).viewport.fov=fov;
   return T;
}
Viewport4& Viewport4::drawZoom(Bool on)
{
   T._draw_zoom=on; if(visible())visible(true); return T;
}
Viewport4& Viewport4::orientMode(ORIENT_MODE mode)
{
   T._orn_mode=mode; if(visible())visible(true); return T;
}
Viewport4& Viewport4::moveMode(MOVE_MODE mode)
{
   T._move_mode=mode; if(visible())visible(true); return T;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
Viewport4& Viewport4::visible(Bool visible)
{
   REPAO(view).viewport.visible(visible                      && (T._sel<0 || T._sel==i));
   REPAO(_cube  ).visible(visible && _orn_mode               && (T._sel<0 || T._sel==i)); if(visible && _orn_mode)setCubeRect();
   REPAO(_zoom  ).visible(visible &&  drawZoom()             && (T._sel<0 || T._sel==i));
   REPAO(_dpad  ).visible(visible && _move_mode==MOVE_DPAD   && (T._sel<0 || T._sel==i));
   REPAO(_dpady ).visible(visible && _move_mode==MOVE_DPAD   && (T._sel<0 || T._sel==i));
   REPAO(_apad  ).visible(visible && _move_mode==MOVE_APAD   && (T._sel<0 || T._sel==i));
   REPAO(_apady ).visible(visible && _move_mode==MOVE_APAD   && (T._sel<0 || T._sel==i));
   REPAO(_drag  ).visible(visible && _move_mode==MOVE_DRAG   && (T._sel<0 || T._sel==i));
   REPAO(_dragy ).visible(visible && _move_mode==MOVE_DRAG   && (T._sel<0 || T._sel==i));
   REPAO(_arrows).visible(visible && _move_mode==MOVE_ARROWS && (T._sel<0 || T._sel==i));
   return T;
}
void Viewport4::setCubeRect()
{
   Flt s=0.20f, // size
       y=0.45f, // y viewport fraction to place touch controls
       p=0.09f; // padding
   if(orientMode()==ORIENT_RIGHT)
   {
      REPAO(_cube).rect(Rect_R(view[i].viewport.rect().lerp(1, y)+Vec2(-p, 0), s, s));
   }else
   {
      REPAO(_cube).rect(Rect_C(view[i].viewport.rect().lu()+Vec2(0.13f, -0.13f), s, s));
   }
}
void Viewport4::setRect()
{
   if(_sel<0)
   {
      view[VIEW_TOP     ].viewport.rect(Rect(rect().min.x    , rect().centerY(), rect().centerX(), rect().max.y    ));
      view[VIEW_DIAGONAL].viewport.rect(Rect(rect().centerX(), rect().centerY(), rect().max.x    , rect().max.y    ));
      view[VIEW_FRONT   ].viewport.rect(Rect(rect().min.x    , rect().min.y    , rect().centerX(), rect().centerY()));
      view[VIEW_LEFT    ].viewport.rect(Rect(rect().centerX(), rect().min.y    , rect().max.x    , rect().centerY()));
   }else
   {
      view[_sel].viewport.rect(rect());
   }

   Flt s =0.15f   , // size
       sc=s *SQRT2, // size of cube (must be Sqrt2, because cube can be rotated)
       sd=sc*1.1f , // size of dpad
       sa=s *1.8f , // size of arrows ('s' is too small for arrows so use bigger)
       sl=0.08f   , // size of sliders
       y =0.45f   , // y viewport fraction to place touch controls
       p =0.09f   ; // padding
   REPAO(_dpad  ).rect(Rect_L(view[i].viewport.rect().lerp(0, y)+Vec2(p, 0), sd     , sd));
   REPAO(_dpady ).rect(Rect_L(view[i].viewport.rect().lerp(0, y)           , sl     , sd));
   REPAO(_apad  ).rect(Rect_L(view[i].viewport.rect().lerp(0, y)+Vec2(p, 0), sc     , sc));
   REPAO(_apady ).rect(Rect_L(view[i].viewport.rect().lerp(0, y)           , sl     , sc));
   REPAO(_drag  ).rect(Rect_L(view[i].viewport.rect().lerp(0, y)+Vec2(p, 0), sc     , sc));
   REPAO(_dragy ).rect(Rect_L(view[i].viewport.rect().lerp(0, y)           , sl     , sc));
   REPAO(_arrows).rect(Rect_L(view[i].viewport.rect().lerp(0, y)+Vec2(0, 0), sa*1.5f, sa));
   REPAO(_zoom  ).rect(Rect_R(view[i].viewport.rect().lerp(1, y)           , sl     , sc));
   setCubeRect();
}
Viewport4& Viewport4::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      T._rect=rect;
      setRect();
   }
   return T;
}
Viewport4& Viewport4::resetPosOrn()
{
   if(lock())
   {
      View &v=view[VIEW_FRONT];
      v.camera.setSpherical(VecZero, _default_yaw, _default_pitch, 0, perspective() ? _default_dist : v.viewport.range*0.5f);
   }else
   {
      if(_last==&view[VIEW_FRONT   ])_last->camera.setSpherical(VecZero, _default_yaw     , _default_pitch     , 0, perspective() ? _default_dist : _last->viewport.range*0.5f);else
      if(_last==&view[VIEW_TOP     ])_last->camera.setSpherical(VecZero, _default_yaw     , _default_pitch-PI_2, 0, perspective() ? _default_dist : _last->viewport.range*0.5f);else
      if(_last==&view[VIEW_DIAGONAL])_last->camera.setSpherical(VecZero, _default_yaw+PI_4, _default_pitch-PI_4, 0, perspective() ? _default_dist : _last->viewport.range*0.5f);else
      if(_last==&view[VIEW_LEFT    ])_last->camera.setSpherical(VecZero, _default_yaw+PI_2, _default_pitch     , 0, perspective() ? _default_dist : _last->viewport.range*0.5f);
   }
   return T;
}
Viewport4& Viewport4::axisAlign()
{
   if(lock())
   {
      Camera &camera=view[VIEW_FRONT].camera;
      camera.setSpherical(camera.at, _default_yaw, 0, 0, camera.dist);
   }else
   {
      if(_last==&view[VIEW_FRONT   ])_last->camera.setSpherical(_last->camera.at, _default_yaw     ,     0, 0, _last->camera.dist);else
      if(_last==&view[VIEW_TOP     ])_last->camera.setSpherical(_last->camera.at, _default_yaw     , -PI_2, 0, _last->camera.dist);else
      if(_last==&view[VIEW_DIAGONAL])_last->camera.setSpherical(_last->camera.at, _default_yaw+PI_4, -PI_4, 0, _last->camera.dist);else
      if(_last==&view[VIEW_LEFT    ])_last->camera.setSpherical(_last->camera.at, _default_yaw+PI_2,     0, 0, _last->camera.dist);
   }
   return T;
}
Viewport4& Viewport4::moveTo(C VecD &pos)
{
   if(lock())
   {
      VecD d=pos-view[VIEW_FRONT].camera.at;
      REPAO(view).camera+=d;
   }else
   {
     _last->camera.at=pos;
     _last->camera.setSpherical();
   }
   return T;
}
Viewport4& Viewport4::dist(Flt dist)
{
   Clamp(dist, minDist(), maxDist());
   if(lock())
   {
      REPAO(view).camera.dist=dist;
      REPAO(view).camera.setSpherical();
   }else
   {
     _last->camera.dist=dist;
     _last->camera.setSpherical();
   }
   return T;
}
Viewport4& Viewport4::toggleView(Int view)
{
   return sel((view==T._sel) ? -1 : view);
}
Viewport4& Viewport4::toggleOrientMode(ORIENT_MODE mode)
{
   return orientMode((orientMode()==mode) ? ORIENT_NONE : mode);
}
Viewport4& Viewport4::toggleMoveMode(MOVE_MODE mode)
{
   return moveMode((moveMode()==mode) ? MOVE_NONE : mode);
}
Viewport4& Viewport4::toggleZoom()
{
   return drawZoom(!drawZoom());
}
Viewport4& Viewport4::togglePerspective()
{
   return perspective(!perspective());
}
Viewport4& Viewport4::toggleHorizontal()
{
   return horizontal(!horizontal());
}
Viewport4& Viewport4::toggleKeyboard()
{
   return keyboard(!keyboard());
}
Viewport4& Viewport4::toggleFpp()
{
  _fpp^=1;
   return T;
}
Viewport4& Viewport4::toggleLock()
{
   if(_lock^=1)
   {
      if(!fpp())view[VIEW_FRONT].camera.at=_last->camera.at;
      else      view[VIEW_FRONT].camera  +=_last->camera.matrix.pos-view[VIEW_FRONT].camera.matrix.pos;
   }
   return T;
}
Viewport4& Viewport4::setViewportCamera()
{
   if(_last)_last->setViewportCamera();
   return T;
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
void Viewport4::update()
{
   // detect highlight
   if(_focus=getView(Gui.ms()))_last=_focus;

   // auto-rotate
   if(_rotate_view)
   {
     _rotate_time+=Time.ad();
      Camera &cam=_rotate_view->camera;
      Flt     s  =Sat(_rotate_time/0.2f);
      VecD    pos=cam.matrix.pos;
      cam.yaw  =LerpAngle(_rotate_yaw  .x, _rotate_yaw  .y, s);
      cam.pitch=LerpAngle(_rotate_pitch.x, _rotate_pitch.y, s);
      cam.setSpherical();
      if(fpp())cam+=pos-cam.matrix.pos;
      if(s==1 )_rotate_view=null;
   }

   // update camera change
   Bool zoom       =(Ms.wheel() && !Kb.ctrlCmd() && !Kb.shift() && !Kb.alt() && !fpp()),
        zoom_manual=(                             (Ms.b(2) &&   Kb.alt() && Kb.ctrlCmd()                 )),
        rotate_ex  =(Ms.b(4) || Kb.winCtrl()   || Kb.b(KB_FIND)),
        rotate     =(rotate_ex                 || (Ms.b(2) && (!Kb.alt() || Kb.ctrlCmd()) && !zoom_manual)),
        move       =(Ms.b(3) || Kb.b(KB_SPACE) || (Ms.b(2) && ( Kb.alt() || Kb.ctrlCmd()) && !zoom_manual)),
        rotate_move=(rotate && move); if(rotate_move)rotate=move=false;

   // set main camera
   View   &v  =(lock() ? view[VIEW_FRONT] : *_last); v.viewport.setDisplayView();
   Camera &cam=v.camera;

   // zoom / rotate
   if(_focus)
   {
      if(rotate || move || rotate_move || zoom_manual)Ms.freeze();

      VecD pos=cam.matrix.pos;
      if(zoom_manual)
      {
         Flt f=ScaleFactor(-Ms.d().sum());
         if(perspective())Clamp(cam.dist*=f, minDist(), maxDist());else REPAO(view).viewport.fov*=f;
      }
      UInt flag=CAMH_NO_VEL_UPDATE|CAMH_NO_SET;
      if(rotate)
      {
         if(rotate_ex)
         {
            if(!(!Kb.ctrlCmd() &&  Kb.alt()))flag|=CAMH_ROT_X; // disable X rotation when only Alt  pressed
            if(!( Kb.ctrlCmd() && !Kb.alt()))flag|=CAMH_ROT_Y; // disable Y rotation when only Ctrl pressed
         }else flag|=CAMH_ROT;
      }
      cam.transformByMouse(minDist(), maxDist(), flag);
      if(fpp())cam+=pos-cam.matrix.pos;
   }
   if(zoom)if(View *view=getViewCtrl(Gui.ms()))
   {
      if(perspective())Clamp(cam.dist*=ScaleFactor(Ms.wheel()*-0.3f), minDist(), maxDist());
      else REPAO(T.view).viewport.fov*=ScaleFactor(Ms.wheel()*-0.3f);
   }

   // set cameras
   if(lock())
   {
      // set secondary basing on the primary
      REPA(view)
      {
         Camera &camera=view[i].camera;
         camera.at   =cam.at;
         camera.dist =cam.dist;
         camera.yaw  =cam.yaw;
         camera.pitch=cam.pitch;
         camera.roll =cam.roll;
      }
      view[VIEW_TOP     ].camera.pitch-=PI_2;
      view[VIEW_DIAGONAL].camera.pitch-=PI_4; view[VIEW_DIAGONAL].camera.yaw+=PI_4;
      view[VIEW_LEFT    ].camera.yaw  +=PI_2;

      // set all in one place
      if(fpp())REPA(view)
      {
         Camera &camera=view[i].camera;
         camera.setSpherical();
         camera+=cam.matrix.pos-camera.matrix.pos;
      }
   }

   // move
   if(_focus && !Gui.window())
   {
                  _focus->viewport.setDisplayView();
      Camera &cam=_focus->camera  .setSpherical  ();
      VecD     at=cam.at;

      // fpp movement
      if(_keyboard && !Kb.ctrlCmd() && !Kb.alt())
         if(!Gui.kb() || Gui.kb()->type()!=GO_TEXTLINE && Gui.kb()->type()!=GO_TEXTBOX && Gui.kb()->type()!=GO_LIST)
      {
         Flt scale=moveScale(*_focus);
         if(Kb.b(Kb.qwerty(KB_A)) || Kb.b(KB_LEFT ))cam.at  -=cam.matrix.x*scale;
         if(Kb.b(Kb.qwerty(KB_D)) || Kb.b(KB_RIGHT))cam.at  +=cam.matrix.x*scale;
         if(Kb.b(Kb.qwerty(KB_S)) || Kb.b(KB_DOWN ))cam.at  -=cam.matrix.z*scale;
         if(Kb.b(Kb.qwerty(KB_W)) || Kb.b(KB_UP   ))cam.at  +=cam.matrix.z*scale;
         if(Kb.b(Kb.qwerty(KB_Q))                  )cam.at.y-=scale;
         if(Kb.b(Kb.qwerty(KB_E))                  )cam.at.y+=scale;
      }
      
      // mouse movement
      if(move || rotate_move)
      {
         if(!horizontal())Swap(move, rotate_move);
         Flt temp; if(fpp()){temp=cam.dist; cam.dist=fppSpeed()*0.5f;} cam.transformByMouse(minDist(), maxDist(), (move?CAMH_MOVE_XZ:0) | (rotate_move?CAMH_MOVE:0) | CAMH_NO_VEL_UPDATE | CAMH_NO_SET);
                   if(fpp())      cam.dist=temp;
      }

      VecD d=cam.at-at;
      if(lock())REPA(view)if(&view[i]!=_focus)view[i].camera+=d;
   }

   // finalize cameras
   REPAO(view).camera.setSpherical().updateVelocities(CAM_ATTACH_FREE);

   // set main view
   setViewportCamera();
}
/******************************************************************************/
} // Edit
/******************************************************************************/
void operator+=(GuiObj &gui_obj, Edit::Viewport4 &child)
{
   REPA(child. view  )gui_obj+=child. view  [i].viewport;
   REPA(child._cube  )gui_obj+=child._cube  [i];
   REPA(child._zoom  )gui_obj+=child._zoom  [i];
   REPA(child._dpad  )gui_obj+=child._dpad  [i];
   REPA(child._dpady )gui_obj+=child._dpady [i];
   REPA(child._apad  )gui_obj+=child._apad  [i];
   REPA(child._apady )gui_obj+=child._apady [i];
   REPA(child._drag  )gui_obj+=child._drag  [i];
   REPA(child._dragy )gui_obj+=child._dragy [i];
   REPA(child._arrows)gui_obj+=child._arrows[i];
}
void operator-=(GuiObj &gui_obj, Edit::Viewport4 &child)
{
   REPA(child. view  )gui_obj-=child. view  [i].viewport;
   REPA(child._cube  )gui_obj-=child._cube  [i];
   REPA(child._zoom  )gui_obj-=child._zoom  [i];
   REPA(child._dpad  )gui_obj-=child._dpad  [i];
   REPA(child._dpady )gui_obj-=child._dpady [i];
   REPA(child._apad  )gui_obj-=child._apad  [i];
   REPA(child._apady )gui_obj-=child._apady [i];
   REPA(child._drag  )gui_obj-=child._drag  [i];
   REPA(child._dragy )gui_obj-=child._dragy [i];
   REPA(child._arrows)gui_obj-=child._arrows[i];
}
/******************************************************************************/
} // EE
/******************************************************************************/
