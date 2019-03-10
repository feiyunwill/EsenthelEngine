/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
  Camera       Cam;
C Camera ActiveCam;
/******************************************************************************/
Camera::Camera()
{
   yaw  =0;
   pitch=0;
   roll =0;
   dist =1;
   at     .zero();
   matrix .setPos(0, 0, -1);
       vel.zero();
   ang_vel.zero();

  _matrix_prev=matrix;
}
/******************************************************************************/
Camera& Camera::set(C MatrixM &matrix)
{
   T.dist  =1;
   T.at    =matrix.pos+matrix.z;
   T.matrix=matrix;
   T.yaw   =-Angle(matrix.z.zx());
   T.pitch = Asin (matrix.z.y   );
   T.roll  =0;
   return T;
}
Camera& Camera::setAngle(C VecD &pos, Flt yaw, Flt pitch, Flt roll)
{
   T.yaw  =yaw;
   T.pitch=pitch;
   T.roll =roll;
   T.dist =1;
   T.matrix.orn().setRotateZ(-roll).rotateXY(-pitch, -yaw);
   T.matrix.pos=pos;
   T.at=matrix.pos+matrix.z;
   return T;
}
Camera& Camera::setSpherical(C VecD &at, Flt yaw, Flt pitch, Flt roll, Flt dist)
{
   T.at   =at;
   T.yaw  =yaw;
   T.pitch=pitch;
   T.roll =roll;
   T.dist =dist;
   return setSpherical();
}
Camera& Camera::setSpherical()
{
   matrix.setPos(0, 0, -dist).rotateZ(-roll).rotateXY(-pitch, -yaw).move(at);
   return T;
}
Camera& Camera::setFromAt(C VecD &from, C VecD &at, Flt roll)
{
   matrix.z=at-from; dist=matrix.z.normalize();
   matrix.x=Cross(Vec(0, 1, 0), matrix.z);
   if(!dist || !matrix.x.normalize())matrix.identity();else
   {
      matrix.x*=Matrix3().setRotate(matrix.z, -roll);
      matrix.y=Cross(matrix.z, matrix.x);
   }
   matrix.pos=from;
   T.at   =at;
   T.yaw  =-Angle(matrix.z.zx());
   T.pitch= Asin (matrix.z.y   );
   T.roll =roll;
   return T;
}
Camera& Camera::setPosDir(C VecD &pos, C Vec &dir, C Vec &up)
{
   Vec dir_n=                 dir   ; dist=dir_n.normalize(); if(!dist)dir_n.set(0, 0, 1);
   Vec  up_f=PointOnPlane(up, dir_n);   if(!up_f.normalize())           up_f=PerpN(dir_n);
   T.matrix.setPosDir(pos, dir_n, up_f);
   T.at   =pos+dir;
   T.yaw  =-Angle(matrix.z.zx());
   T.pitch= Asin (matrix.z.y   );
   T.roll =0;
   return T;
}
/******************************************************************************/
Camera& Camera::teleported()
{
  _matrix_prev=matrix; // prevents velocity jump
   return T;
}
Camera& Camera::updateVelocities(CAM_ATTACHMENT attachment)
{
   Flt dt=Time.d();
   if(Physics.created())switch(attachment)
   {
      case CAM_ATTACH_CTRL:
   #if !PHYSX // background thread processing (PhysX should always jump to 'CAM_ATTACH_ACTOR')
      {
         if( Game::WorldManager::update_objects_after_physics)goto attach_actor;
         if(!Physics._last_updated)return T;
          dt=Physics._last_updated_time;
      }break;

      attach_actor:
   #endif
      case CAM_ATTACH_ACTOR:
      {
         if(!Physics.updated())return T;
          dt=Physics.updatedTime();
      }break;
   }

   GetVel(vel, ang_vel, _matrix_prev, matrix, dt);
  _matrix_prev=matrix;
   return T;
}
/******************************************************************************/
void SetCam(C MatrixM &matrix, Bool set_frustum)
{
   CamMatrix=matrix;
   CamMatrix.inverse(CamMatrixInv, true);
   Sh.h_CamMatrix->set(CamMatrix);
   if(set_frustum)Frustum.set();
}
void Camera::set()C // this should be called only outside of 'Renderer' rendering
{
   ConstCast(ActiveCam)=T; // always backup to 'ActiveCam' even if display not yet created, so we can activate it later
   if(D.created()) // do the rest only if display is created because we need some shader handles which could've been null
   {
      Vec eye_ofs=matrix.x*(D.eyeDistance()*0.5f);
      EyeMatrix[0]=matrix; EyeMatrix[0].pos-=eye_ofs;
      EyeMatrix[1]=matrix; EyeMatrix[1].pos+=eye_ofs;
      SetCam(matrix);

      // set velocity related things !! the same must be done below in 'MotionScaleChanged' !!
      CamMatrixInvMotionScale=CamMatrixInv.orn(); CamMatrixInvMotionScale.scale(D.motionScale());
      Sh.h_CamAngVel->set(ang_vel*CamMatrixInvMotionScale);
   }
}
void MotionScaleChanged() // !! this must match codes above !!
{
   Flt l=D.motionScale();
   CamMatrixInvMotionScale.x.setLength(l);
   CamMatrixInvMotionScale.y.setLength(l);
   CamMatrixInvMotionScale.z.setLength(l);
   if(Sh.h_CamAngVel)Sh.h_CamAngVel->set(ActiveCam.ang_vel*CamMatrixInvMotionScale);
}
/******************************************************************************/
Bool Camera::save(File &f)C
{
   f.putMulti(Byte(0), yaw, pitch, roll, dist, at, matrix, vel, ang_vel); // version
   return f.ok();
}
Bool Camera::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f.getMulti(yaw, pitch, roll, dist, at, matrix, vel, ang_vel); // version
        _matrix_prev=matrix;
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
static Vec ScreenToPosD(C Vec2 &screen_d, Flt z, C Matrix3 &cam_matrix)
{
   if(FovPerspective(D.viewFovMode()))
   {
      return (screen_d.x*z*D.viewFovTanGui().x)*cam_matrix.x
            +(screen_d.y*z*D.viewFovTanGui().y)*cam_matrix.y;
   }else
   {
      return (screen_d.x*D.viewFovTanGui().x)*cam_matrix.x
            +(screen_d.y*D.viewFovTanGui().y)*cam_matrix.y;
   }
}
Camera& Camera::transformByMouse(Flt dist_min, Flt dist_max, UInt flag)
{
   if(flag&CAMH_ZOOM   )Clamp(dist*=ScaleFactor(Ms.wheel()*-0.3f), dist_min, dist_max);
   if(flag&CAMH_ROT_X  )yaw  -=Ms.d().x;
   if(flag&CAMH_ROT_Y  )pitch+=Ms.d().y;
   if(flag&CAMH_MOVE   )at   -=ScreenToPosD(Ms.d()/D.scale(), dist, matrix);
   if(flag&CAMH_MOVE_XZ)
   {
      Vec x, z;
                              CosSin(x.x, x.z, yaw     ); x.y=0;
      z.set(-x.z, 0, x.x); // CosSin(z.x, z.z, yaw+PI_2); z.y=0;
      Vec2 mul=D.viewFovTanGui()/D.scale(); if(FovPerspective(D.viewFovMode()))mul*=dist;
      at-=x*(Ms.d().x*mul.x)
         +z*(Ms.d().y*mul.y);
   }
   setSpherical();
   if(!(flag&CAMH_NO_VEL_UPDATE))updateVelocities(CAM_ATTACH_FREE);
   if(!(flag&CAMH_NO_SET       ))set();
   return T;
}
/******************************************************************************/
Bool PosToScreen(C Vec &pos, Vec2 &screen)
{
   Vec v=pos*CamMatrixInv; // no need for 'VecD'
   if(FovPerspective(D.viewFovMode()))
   {
      screen.x=v.x/(v.z*D.viewFovTanGui().x);
      screen.y=v.y/(v.z*D.viewFovTanGui().y);
   }else
   {
      screen.x=v.x/D.viewFovTanGui().x;
      screen.y=v.y/D.viewFovTanGui().y;
   }
      screen+=D.viewCenter();
   return v.z>D.viewFromActual();
}
Bool PosToScreen(C VecD &pos, Vec2 &screen)
{
   Vec v=pos*CamMatrixInv; // no need for 'VecD'
   if(FovPerspective(D.viewFovMode()))
   {
      screen.x=v.x/(v.z*D.viewFovTanGui().x);
      screen.y=v.y/(v.z*D.viewFovTanGui().y);
   }else
   {
      screen.x=v.x/D.viewFovTanGui().x;
      screen.y=v.y/D.viewFovTanGui().y;
   }
      screen+=D.viewCenter();
   return v.z>D.viewFromActual();
}
Bool PosToFullScreen(C Vec &pos, Vec2 &screen)
{
   Vec v=pos*CamMatrixInv; // no need for 'VecD'
   if(FovPerspective(D.viewFovMode()))
   {
      screen.x=v.x/(v.z*D.viewFovTanFull().x);
      screen.y=v.y/(v.z*D.viewFovTanFull().y);
   }else
   {
      screen.x=v.x/D.viewFovTanFull().x;
      screen.y=v.y/D.viewFovTanFull().y;
   }
      screen+=D.viewCenter();
   return v.z>D.viewFromActual();
}
Bool PosToFullScreen(C VecD &pos, Vec2 &screen)
{
   Vec v=pos*CamMatrixInv; // no need for 'VecD'
   if(FovPerspective(D.viewFovMode()))
   {
      screen.x=v.x/(v.z*D.viewFovTanFull().x);
      screen.y=v.y/(v.z*D.viewFovTanFull().y);
   }else
   {
      screen.x=v.x/D.viewFovTanFull().x;
      screen.y=v.y/D.viewFovTanFull().y;
   }
      screen+=D.viewCenter();
   return v.z>D.viewFromActual();
}

Bool PosToScreenM(C Vec &pos, Vec2 &screen)
{
   Vec v=(pos*ObjMatrix)*CamMatrixInv; // no need for 'VecD'
   if(FovPerspective(D.viewFovMode()))
   {
      screen.x=v.x/(v.z*D.viewFovTanGui().x);
      screen.y=v.y/(v.z*D.viewFovTanGui().y);
   }else
   {
      screen.x=v.x/D.viewFovTanGui().x;
      screen.y=v.y/D.viewFovTanGui().y;
   }
      screen+=D.viewCenter();
   return v.z>D.viewFromActual();
}
Bool PosToScreenM(C VecD &pos, Vec2 &screen)
{
   Vec v=(pos*ObjMatrix)*CamMatrixInv; // no need for 'VecD'
   if(FovPerspective(D.viewFovMode()))
   {
      screen.x=v.x/(v.z*D.viewFovTanGui().x);
      screen.y=v.y/(v.z*D.viewFovTanGui().y);
   }else
   {
      screen.x=v.x/D.viewFovTanGui().x;
      screen.y=v.y/D.viewFovTanGui().y;
   }
      screen+=D.viewCenter();
   return v.z>D.viewFromActual();
}

Vec ScreenToPosDM(C Vec2 &screen_d, Flt z) {return ScreenToPosD(screen_d, z)/ObjMatrix.orn();}
Vec ScreenToPosD (C Vec2 &screen_d, Flt z)
{
   if(FovPerspective(D.viewFovMode()))
   {
      return (screen_d.x*z*D.viewFovTanGui().x)*CamMatrix.x
            +(screen_d.y*z*D.viewFovTanGui().y)*CamMatrix.y;
   }else
   {
      return (screen_d.x*D.viewFovTanGui().x)*CamMatrix.x
            +(screen_d.y*D.viewFovTanGui().y)*CamMatrix.y;
   }
}

static INLINE Vec _ScreenToViewPos(C Vec2 &screen, Flt z)
{
   Vec2 v=screen-D.viewCenter();
   if(FovPerspective(D.viewFovMode()))
   {
      return Vec(v.x*z*D.viewFovTanGui().x,
                 v.y*z*D.viewFovTanGui().y,
                                         z);
   }else
   {
      return Vec(v.x*D.viewFovTanGui().x,
                 v.y*D.viewFovTanGui().y,
                                       z);
   }
}
Vec  ScreenToViewPos(C Vec2 &screen, Flt z) {return _ScreenToViewPos(screen, z);}
VecD ScreenToPos    (C Vec2 &screen, Flt z) {return _ScreenToViewPos(screen, z)*CamMatrix;}
VecD ScreenToPosM   (C Vec2 &screen, Flt z) {return  ScreenToPos    (screen, z)/ObjMatrix;}

static INLINE Vec _FullScreenToViewPos(C Vec2 &screen, Flt z)
{
   Vec2 v=screen-D.viewCenter();
   if(FovPerspective(D.viewFovMode()))
   {
      return Vec(v.x*z*D.viewFovTanFull().x,
                 v.y*z*D.viewFovTanFull().y,
                                          z);
   }else
   {
      return Vec(v.x*D.viewFovTanFull().x,
                 v.y*D.viewFovTanFull().y,
                                        z);
   }
}
Vec  FullScreenToViewPos(C Vec2 &screen, Flt z) {return _FullScreenToViewPos(screen, z);}
VecD FullScreenToPos    (C Vec2 &screen, Flt z) {return _FullScreenToViewPos(screen, z)*CamMatrix;}

Vec2 PosToScreen    (C Vec  &pos) {Vec2 screen; PosToScreen    (pos, screen); return screen;}
Vec2 PosToScreen    (C VecD &pos) {Vec2 screen; PosToScreen    (pos, screen); return screen;}
Vec2 PosToScreenM   (C Vec  &pos) {Vec2 screen; PosToScreenM   (pos, screen); return screen;}
Vec2 PosToScreenM   (C VecD &pos) {Vec2 screen; PosToScreenM   (pos, screen); return screen;}
Vec2 PosToFullScreen(C Vec  &pos) {Vec2 screen; PosToFullScreen(pos, screen); return screen;}
Vec2 PosToFullScreen(C VecD &pos) {Vec2 screen; PosToFullScreen(pos, screen); return screen;}
/******************************************************************************/
Vec ScreenToDir(C Vec2 &screen)
{
   if(FovPerspective(D.viewFovMode()))
   {
      Vec2 v=screen-D.viewCenter();
      return !((v.x*D.viewFovTanGui().x)*CamMatrix.x
              +(v.y*D.viewFovTanGui().y)*CamMatrix.y
              +                          CamMatrix.z);
   }else
   {
      return CamMatrix.z;
   }
}
void ScreenToPosDir(C Vec2 &screen, Vec &pos, Vec &dir)
{
   Vec2 v=screen-D.viewCenter();
   if(FovPerspective(D.viewFovMode()))
   {
      dir=!((v.x*D.viewFovTanGui().x)*CamMatrix.x
           +(v.y*D.viewFovTanGui().y)*CamMatrix.y
           +                          CamMatrix.z);
      pos=CamMatrix.pos;
   }else
   {
      pos=(v.x*D.viewFovTanGui().x)*CamMatrix.x
         +(v.y*D.viewFovTanGui().y)*CamMatrix.y
         +CamMatrix.pos;
      dir=CamMatrix.z;
   }
}
void ScreenToPosDir(C Vec2 &screen, VecD &pos, Vec &dir)
{
   Vec2 v=screen-D.viewCenter();
   if(FovPerspective(D.viewFovMode()))
   {
      dir=!((v.x*D.viewFovTanGui().x)*CamMatrix.x
           +(v.y*D.viewFovTanGui().y)*CamMatrix.y
           +                          CamMatrix.z);
      pos=CamMatrix.pos;
   }else
   {
      pos=(v.x*D.viewFovTanGui().x)*CamMatrix.x
         +(v.y*D.viewFovTanGui().y)*CamMatrix.y
         +CamMatrix.pos;
      dir=CamMatrix.z;
   }
}
/******************************************************************************/
static Bool ToScreenRect(C Vec *point, Int points, Rect &rect)
{
   Bool in=false;
   Vec2 screen;
   REP(points)if(PosToScreen(point[i], screen))
   {
      if(in)rect|=screen;
      else  rect =screen;
      in=true;
   }
   return in;
}
static Bool ToScreenRect(C Vec *point, C VecI2 *edge, Int edges, Rect &rect)
{
   Bool in  =false;
   Vec  from=CamMatrix.pos+CamMatrix.z*D.viewFromActual();
   REP(edges)
   {
      Edge e(point[edge[i].x], point[edge[i].y]);
      Flt  d0=DistPointPlane(e.p[0], from, CamMatrix.z),
           d1=DistPointPlane(e.p[1], from, CamMatrix.z);
      Int  s0=Sign(d0),
           s1=Sign(d1);
      if(s0>0 || s1>0)
      {
         if(s0<0)e.p[0]=PointOnPlane(e.p[0], e.p[1], d0, d1);else
         if(s1<0)e.p[1]=PointOnPlane(e.p[0], e.p[1], d0, d1);
         Vec2 screen;
         PosToScreen(e.p[0], screen); if(in)rect|=screen;else rect=screen;
         PosToScreen(e.p[1], screen);       rect|=screen;
         in=true;
      }
   }
   return in;
}
static Bool ToFullScreenRect(C Vec *point, C VecI2 *edge, Int edges, Rect &rect)
{
   Bool in  =false;
   Vec  from=CamMatrix.pos+CamMatrix.z*D.viewFromActual();
   REP(edges)
   {
      Edge e(point[edge[i].x], point[edge[i].y]);
      Flt  d0=DistPointPlane(e.p[0], from, CamMatrix.z),
           d1=DistPointPlane(e.p[1], from, CamMatrix.z);
      Int  s0=Sign(d0),
           s1=Sign(d1);
      if(s0>0 || s1>0)
      {
         if(s0<0)e.p[0]=PointOnPlane(e.p[0], e.p[1], d0, d1);else
         if(s1<0)e.p[1]=PointOnPlane(e.p[0], e.p[1], d0, d1);
         Vec2 screen;
         PosToFullScreen(e.p[0], screen); if(in)rect|=screen;else rect=screen;
         PosToFullScreen(e.p[1], screen);       rect|=screen;
         in=true;
      }
   }
   return in;
}
static Bool ToFullScreenRect(C VecD *point, C VecI2 *edge, Int edges, Rect &rect)
{
   Bool in  =false;
   VecD from=CamMatrix.pos+CamMatrix.z*D.viewFromActual();
   REP(edges)
   {
      EdgeD e(point[edge[i].x], point[edge[i].y]);
      Dbl   d0=DistPointPlane(e.p[0], from, CamMatrix.z),
            d1=DistPointPlane(e.p[1], from, CamMatrix.z);
      Int   s0=Sign(d0),
            s1=Sign(d1);
      if(s0>0 || s1>0)
      {
         if(s0<0)e.p[0]=PointOnPlane(e.p[0], e.p[1], d0, d1);else
         if(s1<0)e.p[1]=PointOnPlane(e.p[0], e.p[1], d0, d1);
         Vec2 screen;
         PosToFullScreen(e.p[0], screen); if(in)rect|=screen;else rect=screen;
         PosToFullScreen(e.p[1], screen);       rect|=screen;
         in=true;
      }
   }
   return in;
}
static const VecI2 BoxEdges[]=
{
   VecI2(0|2|4, 1|2|4),
   VecI2(1|2|4, 1|2|0),
   VecI2(1|2|0, 0|2|0),
   VecI2(0|2|0, 0|2|4),

   VecI2(0|0|4, 1|0|4),
   VecI2(1|0|4, 1|0|0),
   VecI2(1|0|0, 0|0|0),
   VecI2(0|0|0, 0|0|4),

   VecI2(1|0|4, 1|2|4),
   VecI2(1|0|0, 1|2|0),
   VecI2(0|0|4, 0|2|4),
   VecI2(0|0|0, 0|2|0),
};
Bool ToScreenRect(C Box &box, Rect &rect)
{
   Vec point[8]; box.toCorners(point);
#if 0
   return ToScreenRect(point, Elms(point), rect);
#else
   return ToScreenRect(point, BoxEdges, Elms(BoxEdges), rect);
#endif
}
Bool ToScreenRect(C OBox &obox, Rect &rect)
{
   Vec point[8]; obox.toCorners(point);
#if 0
   return ToScreenRect(point, Elms(point), rect);
#else
   return ToScreenRect(point, BoxEdges, Elms(BoxEdges), rect);
#endif
}
Bool ToScreenRect(C Ball &ball, Rect &rect)
{
   if(!Frustum(ball))return false;
   if(Cuts(CamMatrix.pos, ball)){rect=D.viewRect(); return true;}
#if 1
   Flt  l, s, c;
   Vec2 screen;
   Vec  zd, d, z=ball.pos-CamMatrix.pos; // no need for 'VecD'

   zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
   {
      c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
      rect.setX((PosToScreen(zd-d, screen) ? screen.x : D.viewRect().min.x),
                (PosToScreen(zd+d, screen) ? screen.x : D.viewRect().max.x));
      if(!rect.validX())return false;
   }

   zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
   {
      c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
      rect.setY((PosToScreen(zd+d, screen) ? screen.y : D.viewRect().min.y),
                (PosToScreen(zd-d, screen) ? screen.y : D.viewRect().max.y));
      if(!rect.validY())return false;
   }
   return true;
#else
   return ToScreenRect(Box(ball), rect);
#endif
}
Bool ToScreenRect(C BallM &ball, Rect &rect)
{
   if(!Frustum(ball))return false;
   if(Cuts(CamMatrix.pos, ball)){rect=D.viewRect(); return true;}
#if 1
   Flt  l, s, c;
   Vec2 screen;
   Vec  zd, d, z=ball.pos-CamMatrix.pos; // no need for 'VecD'
   VecD zp;

   zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
   {
      c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
      rect.setX((PosToScreen(zp-d, screen) ? screen.x : D.viewRect().min.x),
                (PosToScreen(zp+d, screen) ? screen.x : D.viewRect().max.x));
      if(!rect.validX())return false;
   }

   zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
   {
      c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
      rect.setY((PosToScreen(zp+d, screen) ? screen.y : D.viewRect().min.y),
                (PosToScreen(zp-d, screen) ? screen.y : D.viewRect().max.y));
      if(!rect.validY())return false;
   }
   return true;
#else
   return ToScreenRect(BoxD(ball), rect);
#endif
}
Bool ToFullScreenRect(C Ball &ball, Rect &rect)
{
   if(!Frustum(ball))return false;
   if(Cuts(CamMatrix.pos, ball)){rect=D.viewRect(); return true;}
#if 1
   Flt  l, s, c;
   Vec2 screen;
   Vec  zd, d, z=ball.pos-CamMatrix.pos; // no need for 'VecD'

   zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
   {
      c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
      rect.setX((PosToFullScreen(zd-d, screen) ? screen.x : D.viewRect().min.x),
                (PosToFullScreen(zd+d, screen) ? screen.x : D.viewRect().max.x));
      if(!rect.validX())return false;
   }

   zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
   {
      c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
      rect.setY((PosToFullScreen(zd+d, screen) ? screen.y : D.viewRect().min.y),
                (PosToFullScreen(zd-d, screen) ? screen.y : D.viewRect().max.y));
      if(!rect.validY())return false;
   }
   return true;
#else
   return ToFullScreenRect(Box(ball), rect);
#endif
}
Bool ToFullScreenRect(C BallM &ball, Rect &rect)
{
   if(!Frustum(ball))return false;
   if(Cuts(CamMatrix.pos, ball)){rect=D.viewRect(); return true;}
#if 1
   Flt  l, s, c;
   Vec2 screen;
   Vec  zd, d, z=ball.pos-CamMatrix.pos; // no need for 'VecD'
   VecD zp;

   zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
   {
      c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
      rect.setX((PosToFullScreen(zp-d, screen) ? screen.x : D.viewRect().min.x),
                (PosToFullScreen(zp+d, screen) ? screen.x : D.viewRect().max.x));
      if(!rect.validX())return false;
   }

   zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
   if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
   {
      c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
      rect.setY((PosToFullScreen(zp+d, screen) ? screen.y : D.viewRect().min.y),
                (PosToFullScreen(zp-d, screen) ? screen.y : D.viewRect().max.y));
      if(!rect.validY())return false;
   }
   return true;
#else
   return ToFullScreenRect(BoxD(ball), rect);
#endif
}
Bool ToScreenRect(C Capsule &capsule, Rect &rect)
{
   if(!Frustum(capsule))return false;
   if(Cuts(CamMatrix.pos, capsule)){rect=D.viewRect(); return true;}
#if 1
   Flt  l, s, c;
   Vec2 screen;
   Vec  z, zd, d;
   Ball ball;

   // upper ball
   {
      ball=capsule.ballU();
      z   =ball.pos-CamMatrix.pos; // no need for 'VecD'

      zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
      {
         c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
         rect.setX((PosToScreen(zd-d, screen) ? screen.x : D.viewRect().min.x),
                   (PosToScreen(zd+d, screen) ? screen.x : D.viewRect().max.x));
      }

      zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
      {
         c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
         rect.setY((PosToScreen(zd+d, screen) ? screen.y : D.viewRect().min.y),
                   (PosToScreen(zd-d, screen) ? screen.y : D.viewRect().max.y));
      }
   }

   // lower ball
   {
      ball=capsule.ballD();
      z   =ball.pos-CamMatrix.pos; // no need for 'VecD'

      zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
      {
         c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
         rect.includeX((PosToScreen(zd-d, screen) ? screen.x : D.viewRect().min.x),
                       (PosToScreen(zd+d, screen) ? screen.x : D.viewRect().max.x));
      }

      zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
      {
         c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zd+=ball.pos; if(Renderer.mirror())d.chs();
         rect.includeY((PosToScreen(zd+d, screen) ? screen.y : D.viewRect().min.y),
                       (PosToScreen(zd-d, screen) ? screen.y : D.viewRect().max.y));
      }
   }

   return rect.valid();
#else
   Matrix matrix; matrix.setPosUp(capsule.pos, capsule.up);
   matrix.y*=capsule.h*0.5f;
   matrix.x*=capsule.r;
   matrix.z*=capsule.r;
   return ToScreenRect(OBox(Box(1), matrix));
#endif
}
Bool ToScreenRect(C CapsuleM &capsule, Rect &rect)
{
   if(!Frustum(capsule))return false;
   if(Cuts(CamMatrix.pos, capsule)){rect=D.viewRect(); return true;}
   Flt   l, s, c;
   Vec2  screen;
   Vec   z, zd, d;
   VecD  zp;
   BallM ball;

   // upper ball
   {
      ball=capsule.ballU();
      z   =ball.pos-CamMatrix.pos; // no need for 'VecD'

      zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
      {
         c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
         rect.setX((PosToScreen(zp-d, screen) ? screen.x : D.viewRect().min.x),
                   (PosToScreen(zp+d, screen) ? screen.x : D.viewRect().max.x));
      }

      zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
      {
         c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
         rect.setY((PosToScreen(zp+d, screen) ? screen.y : D.viewRect().min.y),
                   (PosToScreen(zp-d, screen) ? screen.y : D.viewRect().max.y));
      }
   }

   // lower ball
   {
      ball=capsule.ballD();
      z   =ball.pos-CamMatrix.pos; // no need for 'VecD'

      zd=PointOnPlane(z, CamMatrix.y); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setX(D.viewRect().min.x, D.viewRect().max.x);else
      {
         c=CosSin(s); d=Cross(CamMatrix.y, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
         rect.includeX((PosToScreen(zp-d, screen) ? screen.x : D.viewRect().min.x),
                       (PosToScreen(zp+d, screen) ? screen.x : D.viewRect().max.x));
      }

      zd=PointOnPlane(z, CamMatrix.x); l=zd.normalize(); s=ball.r/l;
      if(s>=1)rect.setY(D.viewRect().min.y, D.viewRect().max.y);else
      {
         c=CosSin(s); d=Cross(CamMatrix.x, zd); d.setLength(c*ball.r); zd*=-s*ball.r; zp=zd+ball.pos; if(Renderer.mirror())d.chs();
         rect.includeY((PosToScreen(zp+d, screen) ? screen.y : D.viewRect().min.y),
                       (PosToScreen(zp-d, screen) ? screen.y : D.viewRect().max.y));
      }
   }

   return rect.valid();
}
static const VecI2 PyramidEdges[]=
{
   VecI2(0, 1),
   VecI2(0, 2),
   VecI2(0, 3),
   VecI2(0, 4),
   VecI2(1, 2),
   VecI2(2, 3),
   VecI2(3, 4),
   VecI2(4, 1),
};
Bool ToScreenRect(C Pyramid &pyramid, Rect &rect)
{
   if(Cuts(CamMatrix.pos, pyramid)){rect=D.viewRect(); return true;}

   Vec point[5], x=pyramid.cross();
   point[0]=pyramid.pos;
   point[1]=pyramid.pos+(pyramid.dir+(-x+pyramid.perp)*pyramid.scale)*pyramid.h;
   point[2]=pyramid.pos+(pyramid.dir+( x+pyramid.perp)*pyramid.scale)*pyramid.h;
   point[3]=pyramid.pos+(pyramid.dir+( x-pyramid.perp)*pyramid.scale)*pyramid.h;
   point[4]=pyramid.pos+(pyramid.dir+(-x-pyramid.perp)*pyramid.scale)*pyramid.h;
   return ToScreenRect(point, PyramidEdges, Elms(PyramidEdges), rect);
}
Bool ToFullScreenRect(C Pyramid &pyramid, Rect &rect)
{
   if(Cuts(CamMatrix.pos, pyramid)){rect=D.viewRect(); return true;}

   Vec point[5], x=pyramid.cross();
   point[0]=pyramid.pos;
   point[1]=pyramid.pos+(pyramid.dir+(-x+pyramid.perp)*pyramid.scale)*pyramid.h;
   point[2]=pyramid.pos+(pyramid.dir+( x+pyramid.perp)*pyramid.scale)*pyramid.h;
   point[3]=pyramid.pos+(pyramid.dir+( x-pyramid.perp)*pyramid.scale)*pyramid.h;
   point[4]=pyramid.pos+(pyramid.dir+(-x-pyramid.perp)*pyramid.scale)*pyramid.h;
   return ToFullScreenRect(point, PyramidEdges, Elms(PyramidEdges), rect);
}
Bool ToFullScreenRect(C PyramidM &pyramid, Rect &rect)
{
   if(Cuts(CamMatrix.pos, pyramid)){rect=D.viewRect(); return true;}

   VecD point[5]; Vec x=pyramid.cross();
   point[0]=pyramid.pos;
   point[1]=pyramid.pos+(pyramid.dir+(-x+pyramid.perp)*pyramid.scale)*pyramid.h;
   point[2]=pyramid.pos+(pyramid.dir+( x+pyramid.perp)*pyramid.scale)*pyramid.h;
   point[3]=pyramid.pos+(pyramid.dir+( x-pyramid.perp)*pyramid.scale)*pyramid.h;
   point[4]=pyramid.pos+(pyramid.dir+(-x-pyramid.perp)*pyramid.scale)*pyramid.h;
   return ToFullScreenRect(point, PyramidEdges, Elms(PyramidEdges), rect);
}
Bool ToScreenRect(C Shape &shape, Rect &rect)
{
   switch(shape.type)
   {
      case SHAPE_BOX    : return ToScreenRect(shape.box    , rect);
      case SHAPE_OBOX   : return ToScreenRect(shape.obox   , rect);
      case SHAPE_BALL   : return ToScreenRect(shape.ball   , rect);
      case SHAPE_CAPSULE: return ToScreenRect(shape.capsule, rect);
      case SHAPE_PYRAMID: return ToScreenRect(shape.pyramid, rect);
      default           : return false;
   }
}
Bool ToScreenRect(C Shape *shape, Int shapes, Rect &rect)
{
   Bool in=false;
   REP(shapes)
   {
      Rect r; if(ToScreenRect(shape[i], r))
      {
         if(!in){in=true; rect=r;}else rect|=r;
      }
   }
   return in;
}
/******************************************************************************/
Int CompareTransparencyOrderDepth(C Vec &pos_a, C Vec &pos_b)
{
   return Sign(Dot(pos_a-pos_b, ActiveCam.matrix.z));
}
Int CompareTransparencyOrderDepth(C VecD &pos_a, C VecD &pos_b)
{
   return Sign(Dot(pos_a-pos_b, ActiveCam.matrix.z));
}
Int CompareTransparencyOrderDist(C Vec &pos_a, C Vec &pos_b)
{
   return Sign(Dist2(pos_a, ActiveCam.matrix.pos)-Dist2(pos_b, ActiveCam.matrix.pos));
}
Int CompareTransparencyOrderDist(C VecD &pos_a, C VecD &pos_b)
{
   return Sign(Dist2(pos_a, ActiveCam.matrix.pos)-Dist2(pos_b, ActiveCam.matrix.pos));
}
/******************************************************************************/
void InitCamera()
{
   ActiveCam.set(); // put values to shaders
}
/******************************************************************************/
}
/******************************************************************************/
