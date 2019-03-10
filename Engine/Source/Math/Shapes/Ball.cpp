/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define SCALE 1.5f // max assumed stretch (for example if the original box is set to a character in rest pose, but due to animation it moves its arms around, then the box after animation might be bigger than original non-animated box, this allows to specify max allowed scale of the original box)
/******************************************************************************
Ball& Ball::setAnimated(C Box &box, C Matrix &matrix)
{
   r  =box.size  ().length()*matrix.x.length()*(0.5f*SCALE);
   pos=box.center()         *matrix;
   return T;
}
Ball& Ball::setAnimated(C Box &box, C AnimatedSkeleton &anim_skel)
{
   r  =box.size  ().length()*anim_skel.scale ()*(0.5f*SCALE);
   pos=box.center()         *anim_skel.matrix();
   return T;
}
/******************************************************************************/
Ball& Ball::setAnimated(C Extent &ext, C Matrix &matrix)
{
   r  =ext.ext.length()*matrix.x.length()*SCALE;
   pos=ext.pos         *matrix;
   return T;
}
Ball& Ball::setAnimated(C Extent &ext, C AnimatedSkeleton &anim_skel)
{
   r  =ext.ext.length()*anim_skel.scale ()*SCALE;
   pos=ext.pos         *anim_skel.matrix();
   return T;
}
/******************************************************************************/
Ball::Ball(C Extent  &ext    ) {set(     ext.ext   .length()     , ext    .pos     );}
Ball::Ball(C Box     &box    ) {set(     box.size().length()*0.5f, box    .center());}
Ball::Ball(C OBox    &obox   ) {set(obox.box.size().length()*0.5f, obox   .center());}
Ball::Ball(C Capsule &capsule) {set(               capsule.h*0.5f, capsule.pos     );}
Ball::Ball(C Shape   &shape  )
{
   switch(shape.type)
   {
      case SHAPE_BOX    : T=shape.box    ; break;
      case SHAPE_OBOX   : T=shape.obox   ; break;
      case SHAPE_BALL   : T=shape.ball   ; break;
      case SHAPE_CAPSULE: T=shape.capsule; break;
   }
}
Ball::Ball(C MeshBase &mesh) {from(mesh.vtx.pos(), mesh.vtxs());}

Ball& Ball::operator*=(  Flt      f) {pos*=f; r*=f; return T;}
Ball& Ball::operator/=(  Flt      f) {pos/=f; r/=f; return T;}
Ball& Ball::operator*=(C Matrix3 &m)
{
   pos*=m;
   r  *=m.maxScale();
   return T;
}
Ball& Ball::operator*=(C Matrix &m)
{
   pos*=m;
   r  *=m.maxScale();
   return T;
}
Ball& Ball::operator/=(C Matrix3 &m)
{
   pos/=m;
   r  /=m.maxScale();
   return T;
}
Ball& Ball::operator/=(C Matrix &m)
{
   pos/=m;
   r  /=m.maxScale();
   return T;
}
/******************************************************************************/
Bool Ball::from(C Vec *point, Int points)
{
   Box box; if(box.from(point, points))
   {
      pos=box.center();
      Flt dist2=0; REP(points)MAX(dist2, Dist2(point[i], pos));
      r=SqrtFast(dist2);
      return true;
   }
   zero(); return false;
}
Bool Ball::from(C Vec *point, Int points, C Matrix &matrix)
{
   Box box; if(box.from(point, points))
   {
      pos=box.center()*matrix;
      Flt dist2=0; REP(points)MAX(dist2, Dist2(point[i]*matrix, pos));
      r=SqrtFast(dist2);
      return true;
   }
   zero(); return false;
}
/******************************************************************************/
Vec Ball::nearest(C Vec &normal)C {return pos-normal*r;}
/******************************************************************************/
void Ball::drawVI(Bool fill, C VecI2 &resolution)C
{
   Int res_x=((resolution.x<0) ? 24 : ((resolution.x<3) ? 3 : resolution.x)),
       res_y=((resolution.y<0) ? 24 : ((resolution.y<3) ? 3 : resolution.y));
   Flt y1_sr=r,
       y1_cr=0;

   REPD(y, res_y-1)
   {
      Flt y2_cr, y2_sr; CosSin(y2_cr, y2_sr, -PI_2+PI*y/(res_y-1)); y2_sr*=r; y2_cr*=r;
      Flt x1_s=0,
          x1_c=1;
      REPD(x, res_x)
      {
         Flt x2_c, x2_s; CosSin(x2_c, x2_s, (PI2*x)/res_x);
         if(fill)
         {
            VI.quad(pos+Vec(x2_c*y1_cr, y1_sr, x2_s*y1_cr),
                    pos+Vec(x1_c*y1_cr, y1_sr, x1_s*y1_cr),
                    pos+Vec(x1_c*y2_cr, y2_sr, x1_s*y2_cr),
                    pos+Vec(x2_c*y2_cr, y2_sr, x2_s*y2_cr));
         }else
         {
                 VI.line(pos+Vec(x1_c*y1_cr, y1_sr, x1_s*y1_cr), pos+Vec(x1_c*y2_cr, y2_sr, x1_s*y2_cr));
            if(y)VI.line(pos+Vec(x1_c*y2_cr, y2_sr, x1_s*y2_cr), pos+Vec(x2_c*y2_cr, y2_sr, x2_s*y2_cr));
         }
         x1_s=x2_s;
         x1_c=x2_c;
      }
      y1_sr=y2_sr;
      y1_cr=y2_cr;
   }
}
void Ball::draw(C Color &color, Bool fill, C VecI2 &resolution)C
{
   VI.color(color); drawVI(fill, resolution);
   VI.end  (     );
}
void Ball::drawAngle(C Color &color, Flt from, Flt to, C Vec &up, Bool fill, C VecI2 &resolution)C
{
   to-=from;
   Matrix m; m.setPosUp(VecZero, up).scale(r).move(pos);
   Int    res_x=((resolution.x<0) ? 24 : ((resolution.x<3) ? 3 : resolution.x)),
          res_y=((resolution.y<0) ? 24 : ((resolution.y<3) ? 3 : resolution.y));
   Flt    y1_cr, y1_sr; CosSin(y1_cr, y1_sr, from+to);

   VI.color(color);
   REPD(y, res_y-1)
   {
      Flt y2_cr, y2_sr; CosSin(y2_cr, y2_sr, from+to*y/(res_y-1));
      Flt x1_s=0,
          x1_c=1;
      REPD(x, res_x)
      {
         Flt x2_c, x2_s; CosSin(x2_c, x2_s, (PI2*x)/res_x);
         if(fill)
         {
            VI.quad(Vec(x2_c*y1_cr, y1_sr, x2_s*y1_cr)*m,
                    Vec(x1_c*y1_cr, y1_sr, x1_s*y1_cr)*m,
                    Vec(x1_c*y2_cr, y2_sr, x1_s*y2_cr)*m,
                    Vec(x2_c*y2_cr, y2_sr, x2_s*y2_cr)*m);
         }else
         {
                 VI.line(Vec(x1_c*y1_cr, y1_sr, x1_s*y1_cr)*m, Vec(x1_c*y2_cr, y2_sr, x1_s*y2_cr)*m);
            if(y)VI.line(Vec(x1_c*y2_cr, y2_sr, x1_s*y2_cr)*m, Vec(x2_c*y2_cr, y2_sr, x2_s*y2_cr)*m);
         }
         x1_s=x2_s;
         x1_c=x2_c;
      }
      y1_sr=y2_sr;
      y1_cr=y2_cr;
   }
   VI.end();
}
void Ball::draw2(C Color &color, Bool fill, Int resolution)C
{
   Matrix m[6]; REP(6)m[i].setPosOrient(pos, DIR_ENUM(i));
   m[DIR_BACK].y.chs();
   m[DIR_LEFT].y.chs();
   m[DIR_DOWN].x.set(0, 0,-1);
   m[DIR_DOWN].y.set(1, 0, 0);
   m[DIR_DOWN].z.set(0,-1, 0);

   if(resolution<0)resolution=12;else if(resolution<1)resolution=1;
   Vec t(-resolution/2.0f, -resolution/2.0f, resolution/2.0f);

   VI.color(color);
   REPD(y, resolution)
   REPD(x, resolution)
   {
      Vec bp[4]=
      {
         Vec(x  , y+1, 0),
         Vec(x+1, y+1, 0),
         Vec(x+1, y  , 0),
         Vec(x  , y  , 0),
      };
      REP(4){bp[i]+=t; bp[i].setLength(r);}
      REPD(s, 6)
      {
         Vec p[4]; Copy(p, bp); Transform(p, m[s], 4);
         if(fill)VI.quad(p[0], p[1], p[2], p[3]);else
         {
            VI.line(p[0], p[1]);
            VI.line(p[0], p[3]);
         }
      }
   }
   VI.end();
}
/******************************************************************************/
Ball Avg(C Ball &a, C Ball &b) {return Ball(Avg(a.r, b.r), Avg(a.pos, b.pos));}
/******************************************************************************/
Flt Dist(C Vec &point, C Ball &ball)
{
   return Max(0, Dist(point, ball.pos)-ball.r);
}
Flt Dist(C Edge &edge, C Ball &ball)
{
   return Max(0, Dist(ball.pos, edge)-ball.r);
}
Flt Dist(C Tri &tri, C Ball &ball)
{
   return Max(0, Dist(ball.pos, tri)-ball.r);
}
Flt Dist(C Box &box, C Ball &ball)
{
   Flt dist2=0;

	if(ball.pos.x<box.min.x)dist2+=Sqr(ball.pos.x-box.min.x);else
	if(ball.pos.x>box.max.x)dist2+=Sqr(ball.pos.x-box.max.x);

	if(ball.pos.y<box.min.y)dist2+=Sqr(ball.pos.y-box.min.y);else
	if(ball.pos.y>box.max.y)dist2+=Sqr(ball.pos.y-box.max.y);

	if(ball.pos.z<box.min.z)dist2+=Sqr(ball.pos.z-box.min.z);else
	if(ball.pos.z>box.max.z)dist2+=Sqr(ball.pos.z-box.max.z);

   return Max(0, SqrtFast(dist2)-ball.r);
}
Flt Dist(C OBox &obox, C Ball &ball)
{
   Ball   temp=ball; temp.pos.divNormalized(obox.matrix);
   return Dist(obox.box, temp);
}
Flt Dist(C Ball &a, C Ball &b)
{
   return Max(0, Dist(a.pos, b.pos)-a.r-b.r);
}
Flt DistBallPlane(C Ball &ball, C Vec &plane, C Vec &normal)
{
   return DistPointPlane(ball.pos, plane, normal)-ball.r;
}
Dbl DistBallPlane(C Ball &ball, C VecD &plane, C Vec &normal)
{
   return DistPointPlane(ball.pos, plane, normal)-ball.r;
}
Dbl DistBallPlane(C BallM &ball, C VecD &plane, C Vec &normal)
{
   return DistPointPlane(ball.pos, plane, normal)-ball.r;
}
/******************************************************************************/
Bool Cuts(C Vec &point, C Ball &ball)
{
   return Dist2(point, ball.pos)<=Sqr(ball.r);
}
Bool Cuts(C VecD &point, C Ball &ball)
{
   return Dist2(point, ball.pos)<=Sqr(ball.r);
}
Bool Cuts(C VecD &point, C BallM &ball)
{
   return Dist2(point, ball.pos)<=Sqr(ball.r);
}
Bool Cuts(C Edge &edge, C Ball &ball)
{
   return Dist(ball.pos, edge)<=ball.r;
}
Bool Cuts(C Tri &tri, C Ball &ball)
{
   return Dist(ball.pos, tri)<=ball.r;
}
Bool Cuts(C Ball &a, C Ball &b)
{
   return Dist2(a.pos, b.pos)<=Sqr(a.r+b.r);
}
Bool Cuts(C Box &box, C Ball &ball)
{
#if 1 // fastest
	if(ball.pos.x<box.min.x-ball.r || ball.pos.x>box.max.x+ball.r)return false;
	if(ball.pos.y<box.min.y-ball.r || ball.pos.y>box.max.y+ball.r)return false;
	if(ball.pos.z<box.min.z-ball.r || ball.pos.z>box.max.z+ball.r)return false;

   Flt dist2=0;

	if(ball.pos.x<box.min.x)dist2 =Sqr(ball.pos.x-box.min.x);else
	if(ball.pos.x>box.max.x)dist2 =Sqr(ball.pos.x-box.max.x);

	if(ball.pos.y<box.min.y)dist2+=Sqr(ball.pos.y-box.min.y);else
	if(ball.pos.y>box.max.y)dist2+=Sqr(ball.pos.y-box.max.y);

	if(ball.pos.z<box.min.z)dist2+=Sqr(ball.pos.z-box.min.z);else
	if(ball.pos.z>box.max.z)dist2+=Sqr(ball.pos.z-box.max.z);

   return dist2<=Sqr(ball.r);
#elif 1 // medium
   Flt dist2=0, r2=Sqr(ball.r);

	if(ball.pos.x<box.min.x){dist2 =Sqr(box.min.x-ball.pos.x); if(dist2>r2)return false;}else
	if(ball.pos.x>box.max.x){dist2 =Sqr(ball.pos.x-box.max.x); if(dist2>r2)return false;}

	if(ball.pos.y<box.min.y){dist2+=Sqr(box.min.y-ball.pos.y); if(dist2>r2)return false;}else
	if(ball.pos.y>box.max.y){dist2+=Sqr(ball.pos.y-box.max.y); if(dist2>r2)return false;}

	if(ball.pos.z<box.min.z){dist2+=Sqr(box.min.z-ball.pos.z); if(dist2>r2)return false;}else
	if(ball.pos.z>box.max.z){dist2+=Sqr(ball.pos.z-box.max.z); if(dist2>r2)return false;}

   return true;
#else // slowest
   Flt dist2=0, d;

	if(ball.pos.x<box.min.x){d=box.min.x-ball.pos.x; if(d>ball.r)return false; dist2 =Sqr(d);}else
	if(ball.pos.x>box.max.x){d=ball.pos.x-box.max.x; if(d>ball.r)return false; dist2 =Sqr(d);}

	if(ball.pos.y<box.min.y){d=box.min.y-ball.pos.y; if(d>ball.r)return false; dist2+=Sqr(d);}else
	if(ball.pos.y>box.max.y){d=ball.pos.y-box.max.y; if(d>ball.r)return false; dist2+=Sqr(d);}

	if(ball.pos.z<box.min.z){d=box.min.z-ball.pos.z; if(d>ball.r)return false; dist2+=Sqr(d);}else
	if(ball.pos.z>box.max.z){d=ball.pos.z-box.max.z; if(d>ball.r)return false; dist2+=Sqr(d);}

   return dist2<=Sqr(ball.r);
#endif
}
Bool Cuts(C Extent &ext, C Ball &ball)
{
#if 1 // faster
   return Dist2(Max(0, Abs(ext.pos.x-ball.pos.x)-ext.ext.x),
                Max(0, Abs(ext.pos.y-ball.pos.y)-ext.ext.y),
                Max(0, Abs(ext.pos.z-ball.pos.z)-ext.ext.z))<=Sqr(ball.r);
#else // slower
   Flt dist2=0, d;
   d=Abs(ext.pos.x-ball.pos.x)-ext.ext.x; if(d>0)dist2 =Sqr(d);
   d=Abs(ext.pos.y-ball.pos.y)-ext.ext.y; if(d>0)dist2+=Sqr(d);
   d=Abs(ext.pos.z-ball.pos.z)-ext.ext.z; if(d>0)dist2+=Sqr(d);
   return dist2<=Sqr(ball.r);
#endif
}
Bool Cuts(C OBox &obox, C Ball &ball)
{
   Ball temp; // 'ball' in 'obox' space
   temp.r=ball.r;
   temp.pos.fromDivNormalized(ball.pos, obox.matrix);
   return Cuts(obox.box, temp);
}
/******************************************************************************/
Int CutsStrBall(C Vec &str_pos, C Vec &str_dir, C Ball &ball, Vec *contact_a, Vec *contact_b)
{
   Vec p=PointOnPlane(str_pos, ball.pos, str_dir);
   Flt s=Dist        (p      , ball.pos         )/ball.r;
   if(s> 1)return 0;
   if(s==1){if(contact_a)*contact_a=p; return 1;}
   if(contact_a || contact_b)
   {
      s=Sqrt(1-s*s)*ball.r;
      if(contact_a)*contact_a=p-s*str_dir;
      if(contact_b)*contact_b=p+s*str_dir;
   }
   return 2;
}
Int CutsEdgeBall(C Vec &edge_start, C Vec &edge_end, C Ball &ball, Vec *contact_a, Vec *contact_b)
{
   Vec c[2], dir=!(edge_end-edge_start);
   if(Int hit=CutsStrBall(edge_start, dir, ball, &c[0], &c[1]))
   {
      Bool in[2]={(hit>=1), (hit>=2)};
      REP(2)if(in[i])if(DistPointPlane(c[i], edge_start, dir)<0 || DistPointPlane(c[i], edge_end, dir)>0)in[i]=false;
      if(in[0] && in[1]){if(contact_a)*contact_a=c[    0]; if(contact_b)*contact_b=c[1]; return 2;}
      if(in[0] || in[1]){if(contact_a)*contact_a=c[in[1]];                               return 1;}
   }
   return 0;
}
/******************************************************************************/
Bool Inside(C Box &a, C Ball &b)
{
#if 0 // this is slower
   return Dist2(Max(Abs(a.max.x-b.pos.x), Abs(a.min.x-b.pos.x)),
                Max(Abs(a.max.y-b.pos.y), Abs(a.min.y-b.pos.y)),
                Max(Abs(a.max.z-b.pos.z), Abs(a.min.z-b.pos.z)))<=Sqr(b.r);
#else // this is faster
   return Max(Sqr(a.max.x-b.pos.x), Sqr(a.min.x-b.pos.x))
         +Max(Sqr(a.max.y-b.pos.y), Sqr(a.min.y-b.pos.y))
         +Max(Sqr(a.max.z-b.pos.z), Sqr(a.min.z-b.pos.z))<=Sqr(b.r);
#endif
}
Bool Inside(C Extent &a, C Ball &b)
{
   return Dist2(((a.pos.x>b.pos.x) ? a.maxX() : a.minX())-b.pos.x,
                ((a.pos.y>b.pos.y) ? a.maxY() : a.minY())-b.pos.y,
                ((a.pos.z>b.pos.z) ? a.maxZ() : a.minZ())-b.pos.z)<=Sqr(b.r);
}
/******************************************************************************/
Bool SweepPointBall(C Vec &point, C Vec &move, C Ball &ball, Flt *hit_frac, Vec *hit_normal)
{
   Vec dir =move; Flt length=dir.normalize();
   Vec p   =PointOnPlane(point, ball.pos, dir);
   Flt s   =Dist(p, ball.pos)/ball.r; if(s>1)return false;
       p  -=dir*(CosSin(s)*ball.r);
   Flt dist=DistPointPlane(p, point, dir); if(dist<0 || dist>length)return false;
   if(hit_frac  )*hit_frac  =dist/length;
   if(hit_normal)*hit_normal=(p-ball.pos)/ball.r;
   return true;
}
Bool SweepPointBall(C VecD &point, C VecD &move, C BallD &ball, Dbl *hit_frac, VecD *hit_normal)
{
   VecD dir =move; Dbl length=dir.normalize();
   VecD p   =PointOnPlane(point, ball.pos, dir);
   Dbl  s   =Dist(p, ball.pos)/ball.r; if(s>1)return false;
        p  -=dir*(CosSin(s)*ball.r);
   Dbl  dist=DistPointPlane(p, point, dir); if(dist<0 || dist>length)return false;
   if(hit_frac  )*hit_frac  =dist/length;
   if(hit_normal)*hit_normal=(p-ball.pos)/ball.r;
   return true;
}
/******************************************************************************/
Bool SweepBallPoint(C Ball &ball, C Vec &move, C Vec &point, Flt *hit_frac, Vec *hit_normal)
{
   Vec dir =move; Flt length=dir.normalize();
   Vec p   =PointOnPlane(ball.pos, point, dir);
   Flt s   =Dist(p, point)/ball.r; if(s>1)return false;
       p  -=dir*(CosSin(s)*ball.r);
   Flt dist=DistPointPlane(p, ball.pos, dir); if(dist<0 || dist>length)return false;
   if(hit_frac  )*hit_frac  =dist/length;
   if(hit_normal)*hit_normal=(p-point)/ball.r;
   return true;
}
Bool SweepBallPoint(C BallD &ball, C VecD &move, C VecD &point, Dbl *hit_frac, VecD *hit_normal)
{
   VecD dir =move; Dbl length=dir.normalize();
   VecD p   =PointOnPlane(ball.pos, point, dir);
   Dbl  s   =Dist(p, point)/ball.r; if(s>1)return false;
        p  -=dir*(CosSin(s)*ball.r);
   Dbl  dist=DistPointPlane(p, ball.pos, dir); if(dist<0 || dist>length)return false;
   if(hit_frac  )*hit_frac  =dist/length;
   if(hit_normal)*hit_normal=(p-point)/ball.r;
   return true;
}
/******************************************************************************/
Bool SweepBallEdge(C Ball &ball, C Vec &move, C Edge &edge, Flt *hit_frac, Vec *hit_normal) // safe in case 'edge' is zero length
{
   Int point_test;
   Vec dir=edge.delta(); if(dir.normalize()) // check if 'edge' has length
   {
      Matrix matrix;        matrix.    setPosDir(edge.p[0], dir  );
      Circle circle(ball.r, matrix.      convert(ball.pos , true));
      Vec2   move2 =        matrix.orn().convert(move     , true) ;
      Vec2   normal; Flt frac;
      if(SweepCirclePoint(circle, move2, Vec2(0), &frac, &normal))
      {
         Vec point=ball.pos+frac*move;
         if(DistPointPlane(point, edge.p[0], matrix.z)<0)point_test=0;else
         if(DistPointPlane(point, edge.p[1], matrix.z)>0)point_test=1;else
         {
            if(hit_frac  )*hit_frac  =frac;
            if(hit_normal)*hit_normal=matrix.orn().convert(normal);
            return true;
         }
      }else point_test=Closer(ball.pos, edge.p[0], edge.p[1]);
   }else point_test=0; // if 'edge' is zero length then check first point
   return SweepBallPoint(ball, move, edge.p[point_test], hit_frac, hit_normal);
}
Bool SweepBallEdge(C BallD &ball, C VecD &move, C EdgeD &edge, Dbl *hit_frac, VecD *hit_normal) // safe in case 'edge' is zero length
{
   Int  point_test;
   VecD dir=edge.delta(); if(dir.normalize()) // check if 'edge' has length
   {
      MatrixD matrix;        matrix.    setPosDir(edge.p[0], dir  );
      CircleD circle(ball.r, matrix.      convert(ball.pos , true));
      VecD2   move2 =        matrix.orn().convert(move     , true) ;
      VecD2   normal; Dbl frac;
      if(SweepCirclePoint(circle, move2, VecD2(0), &frac, &normal))
      {
         VecD point=ball.pos+frac*move;
         if(DistPointPlane(point, edge.p[0], matrix.z)<0)point_test=0;else
         if(DistPointPlane(point, edge.p[1], matrix.z)>0)point_test=1;else
         {
            if(hit_frac  )*hit_frac  =frac;
            if(hit_normal)*hit_normal=matrix.orn().convert(normal);
            return true;
         }
      }else point_test=Closer(ball.pos, edge.p[0], edge.p[1]);
   }else point_test=0; // if 'edge' is zero length then check first point
   return SweepBallPoint(ball, move, edge.p[point_test], hit_frac, hit_normal);
}
/******************************************************************************/
Bool SweepBallBall(C Ball &ball, C Vec &move, C Ball &ball2, Flt *hit_frac, Vec *hit_normal)
{
   return SweepPointBall(ball.pos, move, Ball(ball.r+ball2.r, ball2.pos), hit_frac, hit_normal); // 'hit_normal' is OK when merging 2 balls, this was tested, it will always be in the line of ball positions when they are in contact ('ball' 'move'd by 'hit_frac')
}
Bool SweepBallBall(C BallD &ball, C VecD &move, C BallD &ball2, Dbl *hit_frac, VecD *hit_normal)
{
   return SweepPointBall(ball.pos, move, BallD(ball.r+ball2.r, ball2.pos), hit_frac, hit_normal); // 'hit_normal' is OK when merging 2 balls, this was tested, it will always be in the line of ball positions when they are in contact ('ball' 'move'd by 'hit_frac')
}
/******************************************************************************/
}
/******************************************************************************/
