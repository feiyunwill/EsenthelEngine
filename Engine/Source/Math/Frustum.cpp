/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   Order of planes is set based on performance tests.

/******************************************************************************/
FrustumClass Frustum     ,
             FrustumMain ,
             FrustumGrass;
/******************************************************************************/
void FrustumClass::set(Flt range, C Vec2 &fov, C MatrixM &camera)
{
   T.range =range;
   T.matrix=camera;

   // set planes
   if(persp=FovPerspective(D._view_active.fov_mode))
   {
      Vec2 fov_sin, fov_cos;

      if(VR.active() && D._allow_stereo)
      {
         eye=D.eyeDistance()*0.5f;
         fov_tan.y=D._view_active.fov_tan.y;
         fov_sin.y=D._view_active.fov_sin.y;
         fov_cos.y=D._view_active.fov_cos.y;

      //                   proj_center=desc.DefaultEyeFov[0].LeftTan/(desc.DefaultEyeFov[0].LeftTan+desc.DefaultEyeFov[0].RightTan);
      //                       fov_tan=                         0.5f*(desc.DefaultEyeFov[0].LeftTan+desc.DefaultEyeFov[0].RightTan);
      // desc.DefaultEyeFov[0].LeftTan=proj_center*fov_tan*2;
         Flt proj_center=ProjMatrixEyeOffset[0]*0.5f+0.5f;
               fov_tan.x=proj_center*D._view_active.fov_tan.x*2; // outer tangent, this is correct, because when *0.95 multiplier was applied, then objects were culled near the edge of the screen
         CosSin(fov_cos.x, fov_sin.x, Atan(fov_tan.x));
      }else
      {
         eye=0;
         fov_tan=D._view_active.fov_tan;
         fov_sin=D._view_active.fov_sin;
         fov_cos=D._view_active.fov_cos;
      }
      fov_cos_inv=1.0f/fov_cos; // calculate inverse, because later we'll be able to use "*fov_cos_inv" instead of "/fov_cos", as multiplication is faster than division

      Vec view_quad_max(fov_tan.x*D._view_active.from, fov_tan.y*D._view_active.from, D._view_active.from);
          view_quad_max_dist=view_quad_max.length();

      plane[DIR_RIGHT  ].normal.set(    fov_cos.x, 0, -fov_sin.x); plane[DIR_RIGHT  ].pos.set ( eye, 0, 0                ); // right
      plane[DIR_LEFT   ].normal.set(   -fov_cos.x, 0, -fov_sin.x); plane[DIR_LEFT   ].pos.set (-eye, 0, 0                ); // left
      plane[DIR_UP     ].normal.set(0,  fov_cos.y,    -fov_sin.y); plane[DIR_UP     ].pos.zero(                          ); // up
      plane[DIR_DOWN   ].normal.set(0, -fov_cos.y,    -fov_sin.y); plane[DIR_DOWN   ].pos.zero(                          ); // down
      plane[DIR_FORWARD].normal.set(0,          0,     1        ); plane[DIR_FORWARD].pos.set (0, 0,                range); // front
      plane[DIR_BACK   ].normal.set(0,          0,    -1        ); plane[DIR_BACK   ].pos.set (0, 0, D._view_active.from ); // back
   }else
   {
      view_quad_max_dist=0;
      eye=0;
      size.xy=fov;
      size.z =         range*0.5f; // set as half because we're extending both ways
      matrix.pos+=matrix.z*size.z; // set 'pos' in the center, ortho mode was designed to always have the position in the center so we can use fast frustum culling for Z axis the same way as for XY axes

      plane[DIR_RIGHT  ].normal.set( 1, 0, 0); plane[DIR_RIGHT  ].pos.set(       size.x, 0, 0); // right
      plane[DIR_LEFT   ].normal.set(-1, 0, 0); plane[DIR_LEFT   ].pos.set(      -size.x, 0, 0); // left
      plane[DIR_UP     ].normal.set( 0, 1, 0); plane[DIR_UP     ].pos.set(0,     size.y, 0   ); // up
      plane[DIR_DOWN   ].normal.set( 0,-1, 0); plane[DIR_DOWN   ].pos.set(0,    -size.y, 0   ); // down
      plane[DIR_FORWARD].normal.set( 0, 0, 1); plane[DIR_FORWARD].pos.set(0, 0,  size.z      ); // front
      plane[DIR_BACK   ].normal.set( 0, 0,-1); plane[DIR_BACK   ].pos.set(0, 0, -size.z      ); // back
   }

   REPA(plane)
   {
      plane[i].pos   *=matrix;
      plane[i].normal*=matrix.orn();
      plane_n_abs[i]  =Abs(plane[i].normal);
   }

   use_extra_plane=false;
   if(Renderer.mirror())
   {
      use_extra_plane=true;
      extra_plane=Renderer._mirror_plane;
      extra_plane.normal.chs();
      extra_plane_n_abs=Abs(extra_plane.normal);
   }/*else
   if(Water.draw_plane_surface)
   {
      Flt density=Water.density+Water.density_add;
      if( density>0)
      {
         planes++;
         plane[6]=Water.plane;
         plane[6].normal.chs();
         if(density<1)
         {
            // underwater opacity = Sat(pow(1-WaterDns, x)-WaterDnsAdd);
            // pow(1-WaterDns, x)-WaterDnsAdd = eps
            // pow(1-WaterDns, x) = eps+WaterDnsAdd
            // a**c=b <=> loga(b)=c
            // (1-WaterDns)**x=eps+WaterDnsAdd <=> log 1-WaterDns (eps+WaterDnsAdd)=x

            plane[6]+=plane[6].normal*Max(0,Log(0.015f+Water.density_add,1-Water.density));
         }
         plane[6]+=plane[6].normal*Water.wave_scale;
      }
   }*/

   // points and edges
   if(persp)
   {
      // points
      {
         Flt z=range,
             x=fov_tan.x*z,
             y=fov_tan.y*z;
         point[0]=matrix.pos;
         // set in clock-wise order, do not change the order as it is used in 'ShadowMap'
         point[1].set(-x,  y, z);
         point[2].set( x,  y, z);
         point[3].set( x, -y, z);
         point[4].set(-x, -y, z);
         Transform(point+1, matrix, 4);
      }
      // edges
      {
         edge[0].set(0, 1);
         edge[1].set(0, 2);
         edge[2].set(0, 3);
         edge[3].set(0, 4);

         edge[4].set(1, 2);
         edge[5].set(2, 3);
         edge[6].set(3, 4);
         edge[7].set(4, 1);
      }
      points=5;
      edges =8;
   }else
   {
      // points
      {
         Flt x=size.x,
             y=size.y,
             z=size.z;
         // set in clock-wise order, do not change the order as it is used in 'ShadowMap' and 'ConnectedPoints'
         point[0].set(-x, y,-z);
         point[1].set( x, y,-z);
         point[2].set( x,-y,-z);
         point[3].set(-x,-y,-z);

         point[4].set(-x, y, z);
         point[5].set( x, y, z);
         point[6].set( x,-y, z);
         point[7].set(-x,-y, z);

         Transform(point, matrix, 8);
      }
      // edges
      {
         edge[ 0].set(0, 4);
         edge[ 1].set(1, 5);
         edge[ 2].set(2, 6);
         edge[ 3].set(3, 7);

         edge[ 4].set(0, 1);
         edge[ 5].set(1, 2);
         edge[ 6].set(2, 3);
         edge[ 7].set(3, 0);

         edge[ 8].set(4, 5);
         edge[ 9].set(5, 6);
         edge[10].set(6, 7);
         edge[11].set(7, 4);
      }
      points=8;
      edges =12;
   }
}
void FrustumClass::set()
{
   set(D._view_active.range, D._view_active.fov, CamMatrix);

   if(Renderer()!=RM_SHADOW)
   {
      FrustumMain=T;
      FrustumGrass.set(Min(D._view_active.range, D.grassRange()), D._view_active.fov, CamMatrix);
   }
}
/******************************************************************************/
void FrustumClass::from(C BoxD &box)
{
   // set planes
   plane[DIR_RIGHT  ].normal.set( 1, 0, 0); plane[DIR_RIGHT  ].pos.set(box.max.x, 0, 0); // right
   plane[DIR_LEFT   ].normal.set(-1, 0, 0); plane[DIR_LEFT   ].pos.set(box.min.x, 0, 0); // left
   plane[DIR_UP     ].normal.set( 0, 1, 0); plane[DIR_UP     ].pos.set(0, box.max.y, 0); // up
   plane[DIR_DOWN   ].normal.set( 0,-1, 0); plane[DIR_DOWN   ].pos.set(0, box.min.y, 0); // down
   plane[DIR_FORWARD].normal.set( 0, 0, 1); plane[DIR_FORWARD].pos.set(0, 0, box.max.z); // front
   plane[DIR_BACK   ].normal.set( 0, 0,-1); plane[DIR_BACK   ].pos.set(0, 0, box.min.z); // back

   // set helpers
   use_extra_plane=false;
   view_quad_max_dist=0;
   eye   =0;
   persp =false;
   size  =box.size()*0.5f;
   matrix.setPos(box.center());
   REPA(plane)plane_n_abs[i]=Abs(plane[i].normal);

   // points
   {
      point[0].set(box.min.x, box.max.y, box.min.z);
      point[1].set(box.max.x, box.max.y, box.min.z);
      point[2].set(box.max.x, box.min.y, box.min.z);
      point[3].set(box.min.x, box.min.y, box.min.z);

      point[4].set(box.min.x, box.max.y, box.max.z);
      point[5].set(box.max.x, box.max.y, box.max.z);
      point[6].set(box.max.x, box.min.y, box.max.z);
      point[7].set(box.min.x, box.min.y, box.max.z);
   }
   // edges
   {
      edge[ 0].set(0, 4);
      edge[ 1].set(1, 5);
      edge[ 2].set(2, 6);
      edge[ 3].set(3, 7);

      edge[ 4].set(0, 1);
      edge[ 5].set(1, 2);
      edge[ 6].set(2, 3);
      edge[ 7].set(3, 0);

      edge[ 8].set(4, 5);
      edge[ 9].set(5, 6);
      edge[10].set(6, 7);
      edge[11].set(7, 4);
   }
   points=8;
   edges =12;

   // this method is called only inside 'Light.drawForward', we don't set here FrustumMain
}
/******************************************************************************/
void FrustumClass::from(C PyramidM &pyramid)
{
   // set planes
   Vec2 d(pyramid.scale, 1); d.normalize();

 /*plane[DIR_RIGHT  ].normal.set(    D._view_active.fov_cos.x, 0, -D._view_active.fov_sin.x); plane[DIR_RIGHT  ].pos.zero(                          ); // right
   plane[DIR_LEFT   ].normal.set(   -D._view_active.fov_cos.x, 0, -D._view_active.fov_sin.x); plane[DIR_LEFT   ].pos.zero(                          ); // left
   plane[DIR_UP     ].normal.set(0,  D._view_active.fov_cos.y,    -D._view_active.fov_sin.y); plane[DIR_UP     ].pos.zero(                          ); // up
   plane[DIR_DOWN   ].normal.set(0, -D._view_active.fov_cos.y,    -D._view_active.fov_sin.y); plane[DIR_DOWN   ].pos.zero(                          ); // down
   plane[DIR_FORWARD].normal.set(0,                         0,     1                       ); plane[DIR_FORWARD].pos.set (0, 0, D._view_active.range); // front
   plane[DIR_BACK   ].normal.set(0,                         0,    -1                       ); plane[DIR_BACK   ].pos.set (0, 0, D._view_active.from ); // back*/

   plane[DIR_FORWARD].normal= pyramid.dir; plane[DIR_FORWARD].pos=pyramid.pos+pyramid.dir*pyramid.h; // front
   plane[DIR_BACK   ].normal=-pyramid.dir; plane[DIR_BACK   ].pos=pyramid.pos                      ; // back

   plane[DIR_RIGHT].normal.set(    d.y, 0, -d.x); plane[DIR_RIGHT].pos.zero(); // right
   plane[DIR_LEFT ].normal.set(   -d.y, 0, -d.x); plane[DIR_LEFT ].pos.zero(); // left
   plane[DIR_UP   ].normal.set(0,  d.y,    -d.x); plane[DIR_UP   ].pos.zero(); // up
   plane[DIR_DOWN ].normal.set(0, -d.y,    -d.x); plane[DIR_DOWN ].pos.zero(); // down

   matrix=pyramid;
   DIR_ENUM dir[]={DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN};
   REPA(dir)
   {
      plane[dir[i]].pos   *=matrix;
      plane[dir[i]].normal*=matrix.orn();
   }

   // set helpers
   eye=0;
   view_quad_max_dist=0;
   use_extra_plane=false;
   persp  =true;
   fov_tan=pyramid.scale;
   Vec2 fov_cos=d.y; fov_cos_inv=1.0f/fov_cos;
   range  =pyramid.h;
   REPA(plane)plane_n_abs[i]=Abs(plane[i].normal);

   // points
   {
      point[0]=matrix.pos;
      point[1]=matrix.pos+(matrix.z+(-matrix.x+matrix.y)*pyramid.scale)*pyramid.h;
      point[2]=matrix.pos+(matrix.z+( matrix.x+matrix.y)*pyramid.scale)*pyramid.h;
      point[3]=matrix.pos+(matrix.z+( matrix.x-matrix.y)*pyramid.scale)*pyramid.h;
      point[4]=matrix.pos+(matrix.z+(-matrix.x-matrix.y)*pyramid.scale)*pyramid.h;
   }
   // edges
   {
      edge[0].set(0, 1);
      edge[1].set(0, 2);
      edge[2].set(0, 3);
      edge[3].set(0, 4);

      edge[4].set(1, 2);
      edge[5].set(2, 3);
      edge[6].set(3, 4);
      edge[7].set(4, 1);
   }
   points=5;
   edges =8;

   // this method is called only inside 'Light.drawForward', we don't set here FrustumMain
}
/******************************************************************************/
Bool FrustumClass::operator()(C Vec &point)C
{
   Vec pos=point-matrix.pos; // no need for 'VecD'
   if(persp)
   {
      Flt z=Dot(pos, matrix.z);                if(z<0 || z>range)return false;
      Flt x=Dot(pos, matrix.x), bx=fov_tan.x*z+eye; if(Abs(x)>bx)return false;
      Flt y=Dot(pos, matrix.y), by=fov_tan.y*z    ; if(Abs(y)>by)return false;

      if(use_extra_plane && Dist(point, extra_plane)>0)return false;
   }else
   {
      Flt y=Dot(pos, matrix.y), by=size.y; if(Abs(y)>by)return false;
      Flt x=Dot(pos, matrix.x), bx=size.x; if(Abs(x)>bx)return false;
      Flt z=Dot(pos, matrix.z), bz=size.z; if(Abs(z)>bz)return false;
   }
   return true;
}
Bool FrustumClass::operator()(C VecD &point)C
{
   Vec pos=point-matrix.pos; // no need for 'VecD'
   if(persp)
   {
      Flt z=Dot(pos, matrix.z);                if(z<0 || z>range)return false;
      Flt x=Dot(pos, matrix.x), bx=fov_tan.x*z+eye; if(Abs(x)>bx)return false;
      Flt y=Dot(pos, matrix.y), by=fov_tan.y*z    ; if(Abs(y)>by)return false;

      if(use_extra_plane && Dist(point, extra_plane)>0)return false;
   }else
   {
      Flt y=Dot(pos, matrix.y), by=size.y; if(Abs(y)>by)return false;
      Flt x=Dot(pos, matrix.x), bx=size.x; if(Abs(x)>bx)return false;
      Flt z=Dot(pos, matrix.z), bz=size.z; if(Abs(z)>bz)return false;
   }
   return true;
}
/******************************************************************************/
Bool FrustumClass::operator()(C Ball &ball)C
{
   Vec pos=ball.pos-matrix.pos; // no need for 'VecD'
   if(persp)
   {
      Flt z=Dot(pos, matrix.z);                        if(z<-ball.r || z>range+ball.r)return false; MAX(z, 0);
      Flt x=Dot(pos, matrix.x), bx=fov_tan.x*z+ball.r*fov_cos_inv.x+eye; if(Abs(x)>bx)return false;
      Flt y=Dot(pos, matrix.y), by=fov_tan.y*z+ball.r*fov_cos_inv.y    ; if(Abs(y)>by)return false;

      if(use_extra_plane && Dist(ball, extra_plane)>0)return false;
   }else
   {
      Flt y=Dot(pos, matrix.y), by=size.y+ball.r; if(Abs(y)>by)return false;
      Flt x=Dot(pos, matrix.x), bx=size.x+ball.r; if(Abs(x)>bx)return false;
      Flt z=Dot(pos, matrix.z), bz=size.z+ball.r; if(Abs(z)>bz)return false;
   }
   return true;
}
Bool FrustumClass::operator()(C BallM &ball)C
{
   Vec pos=ball.pos-matrix.pos; // no need for 'VecD'
   if(persp)
   {
      Flt z=Dot(pos, matrix.z);                        if(z<-ball.r || z>range+ball.r)return false; MAX(z, 0);
      Flt x=Dot(pos, matrix.x), bx=fov_tan.x*z+ball.r*fov_cos_inv.x+eye; if(Abs(x)>bx)return false;
      Flt y=Dot(pos, matrix.y), by=fov_tan.y*z+ball.r*fov_cos_inv.y    ; if(Abs(y)>by)return false;

      if(use_extra_plane && Dist(ball, extra_plane)>0)return false;
   }else
   {
      Flt y=Dot(pos, matrix.y), by=size.y+ball.r; if(Abs(y)>by)return false;
      Flt x=Dot(pos, matrix.x), bx=size.x+ball.r; if(Abs(x)>bx)return false;
      Flt z=Dot(pos, matrix.z), bz=size.z+ball.r; if(Abs(z)>bz)return false;
   }
   return true;
}
Bool FrustumClass::operator()(C Capsule &capsule)C
{
   Vec up  =capsule.pos-matrix.pos, // no need for 'VecD'
       down=up,
       d   =capsule.up*(capsule.h*0.5f-capsule.r);
   up  +=d;
   down-=d;
   if(persp)
   {
      Flt zu=Dot(up, matrix.z), zd=Dot(down, matrix.z);     if((zu<-capsule.r && zd<-capsule.r) || (zu>range+capsule.r && zd>range+capsule.r))return false; MAX(zu, 0); MAX(zd, 0);
      Flt xu=Dot(up, matrix.x), xd=Dot(down, matrix.x), bx=capsule.r*fov_cos_inv.x+eye; if(Abs(xu)>bx+fov_tan.x*zu && Abs(xd)>bx+fov_tan.x*zd)return false;
      Flt yu=Dot(up, matrix.y), yd=Dot(down, matrix.y), by=capsule.r*fov_cos_inv.y    ; if(Abs(yu)>by+fov_tan.y*zu && Abs(yd)>by+fov_tan.y*zd)return false;

      if(use_extra_plane && Dist(capsule, extra_plane)>0)return false;
   }else
   {
      Flt yu=Dot(up, matrix.y), yd=Dot(down, matrix.y), by=size.y+capsule.r; if(Abs(yu)>by && Abs(yd)>by)return false;
      Flt xu=Dot(up, matrix.x), xd=Dot(down, matrix.x), bx=size.x+capsule.r; if(Abs(xu)>bx && Abs(xd)>bx)return false;
      Flt zu=Dot(up, matrix.z), zd=Dot(down, matrix.z), bz=size.z+capsule.r; if(Abs(zu)>bz && Abs(zd)>bz)return false;
   }
   return true;
}
Bool FrustumClass::operator()(C CapsuleM &capsule)C
{
   Vec up  =capsule.pos-matrix.pos, // no need for 'VecD'
       down=up,
       d   =capsule.up*(capsule.h*0.5f-capsule.r);
   up  +=d;
   down-=d;
   if(persp)
   {
      Flt zu=Dot(up, matrix.z), zd=Dot(down, matrix.z);     if((zu<-capsule.r && zd<-capsule.r) || (zu>range+capsule.r && zd>range+capsule.r))return false; MAX(zu, 0); MAX(zd, 0);
      Flt xu=Dot(up, matrix.x), xd=Dot(down, matrix.x), bx=capsule.r*fov_cos_inv.x+eye; if(Abs(xu)>bx+fov_tan.x*zu && Abs(xd)>bx+fov_tan.x*zd)return false;
      Flt yu=Dot(up, matrix.y), yd=Dot(down, matrix.y), by=capsule.r*fov_cos_inv.y    ; if(Abs(yu)>by+fov_tan.y*zu && Abs(yd)>by+fov_tan.y*zd)return false;

      if(use_extra_plane && Dist(capsule, extra_plane)>0)return false;
   }else
   {
      Flt yu=Dot(up, matrix.y), yd=Dot(down, matrix.y), by=size.y+capsule.r; if(Abs(yu)>by && Abs(yd)>by)return false;
      Flt xu=Dot(up, matrix.x), xd=Dot(down, matrix.x), bx=size.x+capsule.r; if(Abs(xu)>bx && Abs(xd)>bx)return false;
      Flt zu=Dot(up, matrix.z), zd=Dot(down, matrix.z), bz=size.z+capsule.r; if(Abs(zu)>bz && Abs(zd)>bz)return false;
   }
   return true;
}
/******************************************************************************/
INLINE Flt BoxLength(C Vec &size, C Vec &dir) // box length along direction
{
   return Abs(dir.x)*size.x
        + Abs(dir.y)*size.y
        + Abs(dir.z)*size.z;
}
INLINE Flt BoxLengthAbs(C Vec &size, C Vec &dir_abs) // box length along direction, assuming that direction has absolute components (non-negative)
{
   return dir_abs.x*size.x
        + dir_abs.y*size.y
        + dir_abs.z*size.z;
}
INLINE Flt OBoxLength(C Vec &x, C Vec &y, C Vec &z, C Vec &dir) // obox length along direction
{
   return Abs(Dot(x, dir))
        + Abs(Dot(y, dir))
        + Abs(Dot(z, dir));
}
Bool FrustumClass::operator()(C Extent &ext)C
{
   Vec pos=ext.pos-matrix.pos; // no need for 'VecD'
   if(persp)
   {
      Flt z=Dot(pos, matrix.z);
      if( z<0 || z>range)
      {
         Flt bz=BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]);
         if(z<-bz || z>range+bz)return false; // fb
         MAX(z, 0);
      }

      Flt x=Dot(pos, matrix.x), bx=fov_tan.x*z+eye;
      if(Abs(x)>bx)
      {
         if(x> bx + BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT])*fov_cos_inv.x)return false; // r
         if(x<-bx - BoxLengthAbs(ext.ext, plane_n_abs[DIR_LEFT ])*fov_cos_inv.x)return false; // l
      }

      Flt y=Dot(pos, matrix.y), by=fov_tan.y*z;
      if(Abs(y)>by)
      {
         if(y> by + BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP   ])*fov_cos_inv.y)return false; // u
         if(y<-by - BoxLengthAbs(ext.ext, plane_n_abs[DIR_DOWN ])*fov_cos_inv.y)return false; // d
      }

      if(use_extra_plane)
      {
         Flt e=Dist(pos+matrix.pos, extra_plane);
         if( e>0)if(e>BoxLengthAbs(ext.ext, extra_plane_n_abs))return false;
      }
   }else
   {
      Flt y=Abs(Dot(pos, matrix.y)), by=T.size.y;
      if( y>by)if(y>by+BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP     ]))return false; // ud

      Flt x=Abs(Dot(pos, matrix.x)), bx=T.size.x;
      if( x>bx)if(x>bx+BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT  ]))return false; // rl

      Flt z=Abs(Dot(pos, matrix.z)), bz=T.size.z;
      if( z>bz)if(z>bz+BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]))return false; // fb
   }
   return true;
}
Bool FrustumClass::operator()(C Extent &ext, C Matrix &matrix)C
{
   Vec dx =ext.ext.x*matrix.x,
       dy =ext.ext.y*matrix.y,
       dz =ext.ext.z*matrix.z,
    #if 0
       pos=ext.pos*matrix-T.matrix.pos; // no need for 'VecD'
    #else // faster than above
       pos=ext.pos; pos*=matrix; pos-=T.matrix.pos; // #VecMulMatrix
    #endif

   if(persp)
   {
      Flt z=Dot(pos, T.matrix.z);
      if( z<0 || z>range)
      {
         Flt bz=OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
         if(z<-bz || z>range+bz)return false; // fb
         MAX(z, 0);
      }

      Flt x=Dot(pos, T.matrix.x), bx=fov_tan.x*z+eye;
      if(Abs(x)>bx)
      {
         if(x> bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal)*fov_cos_inv.x)return false; // r
         if(x<-bx - OBoxLength(dx, dy, dz, plane[DIR_LEFT ].normal)*fov_cos_inv.x)return false; // l
      }

      Flt y=Dot(pos, T.matrix.y), by=fov_tan.y*z;
      if(Abs(y)>by)
      {
         if(y> by + OBoxLength(dx, dy, dz, plane[DIR_UP   ].normal)*fov_cos_inv.y)return false; // u
         if(y<-by - OBoxLength(dx, dy, dz, plane[DIR_DOWN ].normal)*fov_cos_inv.y)return false; // d
      }

      if(use_extra_plane)
      {
         Flt e=Dist(pos+T.matrix.pos, extra_plane);
         if( e>0)if(e>OBoxLength(dx, dy, dz, extra_plane.normal))return false;
      }
   }else
   {
      Flt y=Abs(Dot(pos, T.matrix.y)), by=T.size.y;
      if( y>by)if(y>by+OBoxLength(dx, dy, dz, plane[DIR_UP     ].normal))return false; // ud

      Flt x=Abs(Dot(pos, T.matrix.x)), bx=T.size.x;
      if( x>bx)if(x>bx+OBoxLength(dx, dy, dz, plane[DIR_RIGHT  ].normal))return false; // rl

      Flt z=Abs(Dot(pos, T.matrix.z)), bz=T.size.z;
      if( z>bz)if(z>bz+OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal))return false; // fb
   }
   return true;
}
Bool FrustumClass::operator()(C Extent &ext, C MatrixM &matrix)C
{
   Vec dx =ext.ext.x*matrix.x,
       dy =ext.ext.y*matrix.y,
       dz =ext.ext.z*matrix.z,
       pos=ext.pos  *matrix-T.matrix.pos; // no need for 'VecD' if all computations done before setting to 'Vec'

   if(persp)
   {
      Flt z=Dot(pos, T.matrix.z);
      if( z<0 || z>range)
      {
         Flt bz=OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
         if(z<-bz || z>range+bz)return false; // fb
         MAX(z, 0);
      }

      Flt x=Dot(pos, T.matrix.x), bx=fov_tan.x*z+eye;
      if(Abs(x)>bx)
      {
         if(x> bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal)*fov_cos_inv.x)return false; // r
         if(x<-bx - OBoxLength(dx, dy, dz, plane[DIR_LEFT ].normal)*fov_cos_inv.x)return false; // l
      }

      Flt y=Dot(pos, T.matrix.y), by=fov_tan.y*z;
      if(Abs(y)>by)
      {
         if(y> by + OBoxLength(dx, dy, dz, plane[DIR_UP   ].normal)*fov_cos_inv.y)return false; // u
         if(y<-by - OBoxLength(dx, dy, dz, plane[DIR_DOWN ].normal)*fov_cos_inv.y)return false; // d
      }

      if(use_extra_plane)
      {
         Flt e=Dist(pos+T.matrix.pos, extra_plane);
         if( e>0)if(e>OBoxLength(dx, dy, dz, extra_plane.normal))return false;
      }
   }else
   {
      Flt y=Abs(Dot(pos, T.matrix.y)), by=T.size.y;
      if( y>by)if(y>by+OBoxLength(dx, dy, dz, plane[DIR_UP     ].normal))return false; // ud

      Flt x=Abs(Dot(pos, T.matrix.x)), bx=T.size.x;
      if( x>bx)if(x>bx+OBoxLength(dx, dy, dz, plane[DIR_RIGHT  ].normal))return false; // rl

      Flt z=Abs(Dot(pos, T.matrix.z)), bz=T.size.z;
      if( z>bz)if(z>bz+OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal))return false; // fb
   }
   return true;
}
Bool FrustumClass::operator()(C OBox &obox)C // here we assume that 'obox.matrix' can be scaled
{
   Vec dx =(obox.box.w()*0.5f)*obox.matrix.x,
       dy =(obox.box.h()*0.5f)*obox.matrix.y,
       dz =(obox.box.d()*0.5f)*obox.matrix.z,
    #if 0
       pos= obox.center()-matrix.pos; // no need for 'VecD'
    #else // faster than above
       pos= obox.box.center(); pos*=obox.matrix; pos-=matrix.pos; // #VecMulMatrix
    #endif

   if(persp)
   {
      Flt z=Dot(pos, matrix.z);
      if( z<0 || z>range)
      {
         Flt bz=OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal);
         if(z<-bz || z>range+bz)return false; // fb
         MAX(z, 0);
      }

      Flt x=Dot(pos, matrix.x), bx=fov_tan.x*z+eye;
      if(Abs(x)>bx)
      {
         if(x> bx + OBoxLength(dx, dy, dz, plane[DIR_RIGHT].normal)*fov_cos_inv.x)return false; // r
         if(x<-bx - OBoxLength(dx, dy, dz, plane[DIR_LEFT ].normal)*fov_cos_inv.x)return false; // l
      }

      Flt y=Dot(pos, matrix.y), by=fov_tan.y*z;
      if(Abs(y)>by)
      {
         if(y> by + OBoxLength(dx, dy, dz, plane[DIR_UP   ].normal)*fov_cos_inv.y)return false; // u
         if(y<-by - OBoxLength(dx, dy, dz, plane[DIR_DOWN ].normal)*fov_cos_inv.y)return false; // d
      }

      if(use_extra_plane)
      {
         Flt e=Dist(pos+matrix.pos, extra_plane);
         if( e>0)if(e>OBoxLength(dx, dy, dz, extra_plane.normal))return false;
      }
   }else
   {
      Flt y=Abs(Dot(pos, matrix.y)), by=T.size.y;
      if( y>by)if(y>by+OBoxLength(dx, dy, dz, plane[DIR_UP     ].normal))return false; // ud

      Flt x=Abs(Dot(pos, matrix.x)), bx=T.size.x;
      if( x>bx)if(x>bx+OBoxLength(dx, dy, dz, plane[DIR_RIGHT  ].normal))return false; // rl

      Flt z=Abs(Dot(pos, matrix.z)), bz=T.size.z;
      if( z>bz)if(z>bz+OBoxLength(dx, dy, dz, plane[DIR_FORWARD].normal))return false; // fb
   }
   return true;
}
/******************************************************************************/
Bool FrustumClass::operator()(C Extent &ext, Bool &fully_inside)C
{
   fully_inside=true;

   Vec pos=ext.pos-matrix.pos; // no need for 'VecD'
   if(persp)
   {
      Flt z=Dot(pos, matrix.z);
    //if( z<0 || z>range)
      {
         Flt bz=BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]);
         if(z<      bz){if(z<     -bz)return false; fully_inside=false;} // b
         if(z>range-bz){if(z>range+bz)return false; fully_inside=false;} // f
         MAX(z, 0);
      }

      Flt x=Dot(pos, matrix.x), bx=fov_tan.x*z+eye;
    //if(Abs(x)>bx)
      {
         Flt bxr=BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT])*fov_cos_inv.x; if(x> bx-bxr){if(x> bx+bxr)return false; fully_inside=false;} // r
         Flt bxl=BoxLengthAbs(ext.ext, plane_n_abs[DIR_LEFT ])*fov_cos_inv.x; if(x<-bx+bxl){if(x<-bx-bxl)return false; fully_inside=false;} // l
      }

      Flt y=Dot(pos, matrix.y), by=fov_tan.y*z;
    //if(Abs(y)>by)
      {
         Flt bxu=BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP   ])*fov_cos_inv.y; if(y> by-bxu){if(y> by+bxu)return false; fully_inside=false;} // u
         Flt bxd=BoxLengthAbs(ext.ext, plane_n_abs[DIR_DOWN ])*fov_cos_inv.y; if(y<-by+bxd){if(y<-by-bxd)return false; fully_inside=false;} // d
      }

      if(use_extra_plane)
      {
         Flt e=Dist(pos+matrix.pos, extra_plane), be=BoxLengthAbs(ext.ext, extra_plane_n_abs); if(e>-be){if(e>be)return false; fully_inside=false;}
      }
   }else
   {
      Flt y=Abs(Dot(pos, matrix.y))-T.size.y, by=BoxLengthAbs(ext.ext, plane_n_abs[DIR_UP     ]); if(y>-by){if(y>by)return false; fully_inside=false;} // ud
      Flt x=Abs(Dot(pos, matrix.x))-T.size.x, bx=BoxLengthAbs(ext.ext, plane_n_abs[DIR_RIGHT  ]); if(x>-bx){if(x>bx)return false; fully_inside=false;} // rl
      Flt z=Abs(Dot(pos, matrix.z))-T.size.z, bz=BoxLengthAbs(ext.ext, plane_n_abs[DIR_FORWARD]); if(z>-bz){if(z>bz)return false; fully_inside=false;} // fb
   }
   return true;
}
/******************************************************************************/
Bool FrustumClass::operator()(C Box &box                         )C {return T(Extent(box)              );}
Bool FrustumClass::operator()(C Box &box, C Matrix  &matrix      )C {return T(Extent(box), matrix      );}
Bool FrustumClass::operator()(C Box &box, C MatrixM &matrix      )C {return T(Extent(box), matrix      );}
Bool FrustumClass::operator()(C Box &box,   Bool    &fully_inside)C {return T(Extent(box), fully_inside);}
/******************************************************************************/
Bool FrustumClass::operator()(C Shape &shape)C
{
   switch(shape.type)
   {
      case SHAPE_POINT  : return T(shape.point  );
      case SHAPE_BOX    : return T(shape.box    );
      case SHAPE_OBOX   : return T(shape.obox   );
      case SHAPE_BALL   : return T(shape.ball   );
      case SHAPE_CAPSULE: return T(shape.capsule);
   }
   return false;
}
Bool FrustumClass::operator()(C Shape *shape, Int shapes)C
{
   REP(shapes)if(T(shape[i]))return true;
   return false;
}
/******************************************************************************/
Bool FrustumClass::operator()(C FrustumClass &frustum)C // assumes that one frustum is not entirely inside the other frustum (which means that there is contact between edge and faces)
{
   // comparing edges with faces is more precise than comparing points only
   REPD(f, 2)
   {
    C FrustumClass &a=(f ? T : frustum),
                   &b=(f ? frustum : T);
      REPD(e, a.edges)
      {
         VecD  contact;
         EdgeD ed(a.point[a.edge[e].x], a.point[a.edge[e].y]);
         REPA(b.plane)if(Cuts(ed, b.plane[i], &contact))if(b(contact-b.plane[i].normal*EPSL))return true; // use epsilon to make sure that we're under plane surface
      }
   }
   return false;
}
/******************************************************************************/
static INLINE void ProcessPos(C VecI2 &pos, C RectI &rect, Memt<VecI2> &row_min_max_x)
{
   if(rect.includesY(pos.y))
   {
      VecI2 &min_max_x=row_min_max_x[pos.y-rect.min.y];
      if(min_max_x.y<min_max_x.x)min_max_x=pos.x;else // if invalid (not yet set) then set,
      { // extend
         if(pos.x<min_max_x.x)min_max_x.x=pos.x;else
         if(pos.x>min_max_x.y)min_max_x.y=pos.x;
      }
   }
}
void FrustumClass::getIntersectingAreas(MemPtr<VecI2> area_pos, Flt area_size, Bool distance_check, Bool sort_by_distance, Bool extend, C RectI *clamp)C
{
   area_pos.clear();

   Memt<VecD2> convex_points; CreateConvex2Dxz(convex_points, point, points); if(!convex_points.elms())return;

   RectI rect; // inclusive
   Bool  is;

   // set min_y..max_y visibility
   is=false; REPA(convex_points)
   {
      VecD2 &p=convex_points[i]; p/=area_size;
      Int    y=Floor(extend ? p.y-0.5f : p.y);
      if(is)rect.includeY(y);else{rect.setY(y); is=true;}
   }
   if(extend)rect.max.y++;
   if(clamp )rect&=*clamp;
   if(!rect.validY())return;

   // set min_x..max_x per row in 'row_min_max_x'
   Memt<VecI2> row_min_max_x; row_min_max_x.setNum(rect.h()+1); // +1 because it's inclusive
   REPAO(row_min_max_x).set(0, -1); // on start set invalid range ("max<min")

   REPA(convex_points) // warning: this needs to work as well for "convex_points.elms()==1"
   {
      Vec2 start=convex_points[i], end=convex_points[(i+1)%convex_points.elms()];
      if(extend)
      {
         // add corner point first as a 2x2 block (needs to process 2x2 because just 1 corner didn't cover all areas, the same was for Avg of 2 Perps)
         RectI corner; corner.min=Floor(start); VecI2 pos;
         if(start.x-corner.min.x<0.5f)corner.max.x=corner.min.x--;else corner.max.x=corner.min.x+1; // if point is on the left   side (frac<0.5), then process from pos-1..pos, otherwise from pos..pos+1
         if(start.y-corner.min.y<0.5f)corner.max.y=corner.min.y--;else corner.max.y=corner.min.y+1; // if point is on the bottom side (frac<0.5), then process from pos-1..pos, otherwise from pos..pos+1
         for(pos.y=corner.min.y; pos.y<=corner.max.y; pos.y++)
         for(pos.x=corner.min.x; pos.x<=corner.max.x; pos.x++)ProcessPos(pos, rect, row_min_max_x);

         Vec2 perp=Perp(start-end); if(Flt max=Abs(perp).max()){perp*=0.5f/max; start+=perp; end+=perp;} // use "Abs(perp).max()" instead of "perp.length()" because we need to extend orthogonally (because we're using extend for the purpose of detecting objects from neighborhood areas that extend over to other areas, and this extend is allowed orthogonally)
      }
      for(PixelWalker walker(start, end); walker.active(); walker.step())ProcessPos(walker.pos(), rect, row_min_max_x);
   }

   // set min_x..max_x visibility (this is more precise than what can be calculated from just 'convex_points', because here, we've clipped the edges to min_y..max_y range)
   is=false; REPA(row_min_max_x)
   {
      VecI2 &min_max_x=row_min_max_x[i];
      if(min_max_x.y>=min_max_x.x) // if valid
      {
         if(is)rect.includeX(min_max_x.x, min_max_x.y);else{rect.setX(min_max_x.x, min_max_x.y); is=true;}
      }
   }
   if(clamp)rect&=*clamp;
   if(!is || !rect.validX())return;

   const Bool fast=true; // if use ~2x faster 'Dist2PointSquare' instead of 'Dist2(Vec2 point, RectI rect)'

   Vec2 distance_pos;
   Flt  distance_range2;
   if(  distance_check&=persp) // can do range tests only in perspective mode (orthogonal mode is used for shadows, and there we need full range, and also in that mode 'matrix.pos' is in the center, so it can't be used as 'distance_pos')
   { // convert to area space
      distance_pos   =matrix.pos.xz()/area_size; if(fast  )distance_pos   -=0.5f; // since in fast mode we're testing against a square of radius 0.5, instead of setting each square as "pos+0.5f", we offset the 'distance_pos' by the negative (this was tested and works OK - the same results as when 'fast'=false)
      distance_range2=          range/area_size; if(extend)distance_range2+=0.5f; distance_range2*=distance_range2;
   }

   // set areas for drawing
   if(sort_by_distance) // in look order (from camera/foreground to background)
   {
      Vec2  look_dir=matrix.z.xz();
      Flt   max     =Abs(look_dir).max();
      VecI2 dir     =(max ? Round(look_dir/max) : VecI2(0, 1)), // (-1, -1) .. (1, 1)
            perp    =Perp(dir);                                 // parallel to direction
      if((dir.x== 1 && dir.y== 1)
      || (dir.x== 1 && dir.y== 0)
      || (dir.x==-1 && dir.y==-1)
      || (dir.x== 0 && dir.y==-1))perp.chs();

      for(VecI2 edge((dir.x<0) ? rect.max.x : rect.min.x, (dir.y<0) ? rect.max.y : rect.min.y); ; )
      {
         for(VecI2 pos=edge; ; )
         {
            VecI2 &min_max_x=row_min_max_x[pos.y-rect.min.y];
            if(pos.x>=min_max_x.x && pos.x<=min_max_x.y)
               if(!distance_check || (fast ? Dist2PointSquare(distance_pos, pos, 0.5f) : Dist2(distance_pos, RectI(pos, pos+1)))<=distance_range2)
                  area_pos.add(pos); // add to array

            pos+=perp; if(!rect.includes(pos))break; // go along the parallel until you can't
         }
         if(dir.x && rect.includesX(edge.x+dir.x))edge.x+=dir.x;else        // first travel on the x-edge until you can't
         if(dir.y && rect.includesY(edge.y+dir.y))edge.y+=dir.y;else break; // then  travel on the y-edge until you can't, after that get out of the loop
      }
   }else
   {
      VecI2 pos; for(pos.y=rect.min.y; pos.y<=rect.max.y; pos.y++)
      {
         VecI2 min_max_x=row_min_max_x[pos.y-rect.min.y];
         MAX(min_max_x.x, rect.min.x);
         MIN(min_max_x.y, rect.max.x);
         for(pos.x=min_max_x.x; pos.x<=min_max_x.y; pos.x++)
            if(!distance_check || (fast ? Dist2PointSquare(distance_pos, pos, 0.5f) : Dist2(distance_pos, RectI(pos, pos+1)))<=distance_range2)
               area_pos.add(pos); // add to array
      }
   }
}
/******************************************************************************/
void FrustumClass::draw(C Color &col)C
{
             VI.color(col);
   REP(edges)VI.line (point[edge[i].x], point[edge[i].y]);
             VI.end  ();

   //REPA(plane)DrawArrow(RED, plane[i].p, plane[i].p+plane[i].normal*3);
}
/******************************************************************************/
}
/******************************************************************************/
