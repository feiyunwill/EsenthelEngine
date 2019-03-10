/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Edge& Edge::divNormalized(C Matrix3 &m)
{
   p[0].divNormalized(m);
   p[1].divNormalized(m);
   return T;
}
Edge& Edge::divNormalized(C Matrix &m)
{
   p[0].divNormalized(m);
   p[1].divNormalized(m);
   return T;
}
Edge& Edge::div(C Matrix3 &m)
{
   p[0].divNormalized(m);
   p[1].divNormalized(m);
   Flt x=m.x.length2(); p[0].x/=x; p[1].x/=x;
   Flt y=m.y.length2(); p[0].y/=y; p[1].y/=y;
   Flt z=m.z.length2(); p[0].z/=z; p[1].z/=z;
   return T;
}
Edge& Edge::div(C Matrix &m)
{
   p[0].divNormalized(m);
   p[1].divNormalized(m);
   Flt x=m.x.length2(); p[0].x/=x; p[1].x/=x;
   Flt y=m.y.length2(); p[0].y/=y; p[1].y/=y;
   Flt z=m.z.length2(); p[0].z/=z; p[1].z/=z;
   return T;
}
/******************************************************************************/
void Edge2::draw(C Color &color)C
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, VI_LINE);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(2))
   {
      v[0].pos=p[0];
      v[1].pos=p[1];
   }
   VI.end();
}
void Edge2::draw(C Color &color0, C Color &color1)C
{
   VI.setType(VI_2D_COL, VI_LINE);
   if(Vtx2DCol *v=(Vtx2DCol*)VI.addVtx(2))
   {
      v[0].pos=p[0]; v[0].color=color0;
      v[1].pos=p[1]; v[1].color=color1;
   }
   VI.end();
}
void Edge2::draw(C Color &color, Flt width)C
{
   Vec2 perp=T.perp(); perp.setLength(width);
   VI.color  (color);
   VI.setType(VI_2D_FLAT, VI_STRIP);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(4))
   {
      v[0].pos.set(p[0].x-perp.x, p[0].y-perp.y);
      v[1].pos.set(p[1].x-perp.x, p[1].y-perp.y);
      v[3].pos.set(p[1].x+perp.x, p[1].y+perp.y);
      v[2].pos.set(p[0].x+perp.x, p[0].y+perp.y);
   }
   VI.end();
}
/******************************************************************************/
void Edge::draw(C Color &color)C
{
   VI.color  (color);
   VI.setType(VI_3D_FLAT, VI_LINE);
   if(Vtx3DFlat *v=(Vtx3DFlat*)VI.addVtx(2))
   {
      v[0].pos=p[0];
      v[1].pos=p[1];
   }
   VI.end();
}
void Edge::draw(C Color &color0, C Color &color1)C
{
   VI.setType(VI_3D_COL, VI_LINE);
   if(Vtx3DCol *v=(Vtx3DCol*)VI.addVtx(2))
   {
      v[0].pos=p[0]; v[0].color=color0;
      v[1].pos=p[1]; v[1].color=color1;
   }
   VI.end();
}
void Edge::draw(C Color &color, Flt width)C
{
   Flt z0=((p[0]*ObjMatrix)*CamMatrixInv).z-D.viewFromActual(),
       z1=((p[1]*ObjMatrix)*CamMatrixInv).z-D.viewFromActual();
   if( z0>=0 && z1>=0)Edge2(PosToScreenM(p[0]), PosToScreenM(p[1]                            )).draw(color, width);else
   if( z0> 0         )Edge2(PosToScreenM(p[0]), PosToScreenM(PointOnPlane(p[0], p[1], z0, z1))).draw(color, width);else
   if( z1> 0         )Edge2(PosToScreenM(p[1]), PosToScreenM(PointOnPlane(p[0], p[1], z0, z1))).draw(color, width);
}
/******************************************************************************/
void EdgeD2::draw(C Color &color                  )C {return Edge2(T).draw(color         );}
void EdgeD2::draw(C Color &color ,   Flt    width )C {return Edge2(T).draw(color , width );}
void EdgeD2::draw(C Color &color0, C Color &color1)C {return Edge2(T).draw(color0, color1);}
void EdgeD ::draw(C Color &color                  )C {return Edge (T).draw(color         );}
void EdgeD ::draw(C Color &color ,   Flt    width )C {return Edge (T).draw(color , width );}
void EdgeD ::draw(C Color &color0, C Color &color1)C {return Edge (T).draw(color0, color1);}
/******************************************************************************/
void Edge2_I::set(C Vec2 &a, C Vec2 &b, C Vec2 *normal)
{
   p[0]=a;
   p[1]=b;
   if(normal)
   {
      T.normal=*normal;
        dir.x =-T.normal.y;
        dir.y = T.normal.x;
        length= DistPointPlane(b, a, dir);
   }else
   {
        dir     = delta();
        length  = dir.normalize();
      T.normal.x= dir.y;
      T.normal.y=-dir.x;
   }
}
void EdgeD2_I::set(C VecD2 &a, C VecD2 &b, C VecD2 *normal)
{
   p[0]=a;
   p[1]=b;
   if(normal)
   {
      T.normal=*normal;
        dir.x =-T.normal.y;
        dir.y = T.normal.x;
        length= DistPointPlane(b, a, dir);
   }else
   {
        dir     = delta();
        length  = dir.normalize();
      T.normal.x= dir.y;
      T.normal.y=-dir.x;
   }
}
void Edge_I::set(C Vec &a, C Vec &b)
{
   p[0]  =a;
   p[1]  =b;
   dir   =b-a;
   length=dir.normalize();
}
void EdgeD_I::set(C VecD &a, C VecD &b)
{
   p[0]  =a;
   p[1]  =b;
   dir   =b-a;
   length=dir.normalize();
}
/******************************************************************************/
// PIXEL WALKER
/******************************************************************************/
void PixelWalker::start(C VecI2 &start, C VecI2 &end)
{
   VecI2 delta=end-start, adelta=Abs(delta);

  _active   =true;
  _side_step=false;
  _axis     =(adelta.maxI()==1);
  _steps    = adelta.max ();
  _main_sign=Sign(delta.c[_axis]);
  _pos      =start;
  _posr     =start+0.5f;
  _step     =delta; if(_steps)_step/=_steps;
  _pos_temp =_pos;
}
void PixelWalker::start(C Vec2 &start, C Vec2 &end)
{
  _pos =Floor(start);
  _posr=      start ;

   VecI2 delta=Floor(end)-_pos, adelta=Abs(delta);

  _active   =true;
  _side_step=false;
  _axis     =(adelta.maxI()==1);
  _steps    = adelta.max (); // Abs(delta.c[_axis]);
  _main_sign=Sign(delta.c[_axis]);
  _step     =end-start; if(_steps)_step/=_steps;
  _pos_temp =_pos;
}
void PixelWalker::step()
{
   if(_active)
   {
      if(_side_step)
      {
        _pos_temp =_pos;
        _side_step= false;
      }else
      if(_steps-- <= 0)
      {
        _active=false;
      }else
      {
         Vec2  posr_next=_posr+_step;
         VecI2  pos_next;
         if(_axis==0) // horizontal
         {
            pos_next.set(_pos.x+_main_sign, Floor(posr_next.y));
            if(_pos.y!=pos_next.y) // we've changed Y, we need to check if additional step is required
            {
               Clamp(pos_next.y, _pos.y-1, _pos.y+1); // due to numerical precision issues the position can move more than 1, if for example the end is located at 0.0 coordinates and we would slightly cross the border, this is be cause in each step we accumulate '_step' which introduces some error
               // check which pixel was crossed first
               Flt dest;
               dest=Max(_pos.x, pos_next.x); Flt d0=(dest-_posr.x)/_step.x;
               dest=Max(_pos.y, pos_next.y); Flt d1=(dest-_posr.y)/_step.y;
               if(d0<d1){_pos_temp.x=pos_next.x; _side_step=true;}else // move in  _axis first
               if(d0>d1){_pos_temp.y=pos_next.y; _side_step=true;}     // move in !_axis first
            }
         }else // vertical
         {
            pos_next.set(Floor(posr_next.x), _pos.y+_main_sign);
            if(_pos.x!=pos_next.x) // we've changed X, we need to check if additional step is required
            {
               Clamp(pos_next.x, _pos.x-1, _pos.x+1); // due to numerical precision issues the position can move more than 1, if for example the end is located at 0.0 coordinates and we would slightly cross the border, this is be cause in each step we accumulate '_step' which introduces some error
               // check which pixel was crossed first
               Flt dest;
               dest=Max(_pos.y, pos_next.y); Flt d0=(dest-_posr.y)/_step.y;
               dest=Max(_pos.x, pos_next.x); Flt d1=(dest-_posr.x)/_step.x;
               if(d0<d1){_pos_temp.y=pos_next.y; _side_step=true;}else // move in  _axis first
               if(d0>d1){_pos_temp.x=pos_next.x; _side_step=true;}     // move in !_axis first
            }
         }
        _posr=posr_next;
        _pos = pos_next;
         if(!_side_step)_pos_temp=_pos;
      }
   }
}
/******************************************************************************/
void PixelWalkerMask::start(C VecI2 &start, C VecI2 &end, C RectI &mask) {T.start(start+0.5f, end+0.5f, mask);}
void PixelWalkerMask::start(C Vec2  &start, C Vec2  &end, C RectI &mask)
{
 //Edge2 edge(start, end); if(Clip(edge, Rect(mask.min, mask.max+1))) no need to clip 'end' because we have to do it anyway in 'step'
   Vec2 start_clamp; if(SweepPointRect(start, end-start, Rect(mask.min, mask.max+1), null, null, &start_clamp))
   {
    //super::start(edge.p[0], edge.p[1]);
      super::start(start_clamp, end);

      T._mask=mask;
      for(; !mask.includes(T.pos()) && _active; )super::step(); // if not at the start, have to check multiple times in case we hit the mask rect at the edge, but it needs multiple steps to cross that border
   }else _active=false;
}
void PixelWalkerMask::step()
{
   super::step();
   if(!_mask.includes(T.pos()))_active=false; // have to check mask in each step, because there's no easy way to prevent the end from going out of range, can't just decrease '_steps' because last valid pixel could be a side step before a position that is out of range
}
/******************************************************************************/
void PixelWalkerEdge::start(C Vec2 &start, C Vec2 &end)
{
   VecI2 pos=Floor(start), endi=Floor(end), delta=endi-pos;

   Bool frac_start_x=(start.x-pos.x!=0), frac_end_x=(end.x-endi.x!=0),
        frac_start_y=(start.y-pos.y!=0), frac_end_y=(end.y-endi.y!=0);

   if(end.x<start.x && !frac_start_x &&  frac_end_x)delta.x++;else
   if(end.x>start.x &&  frac_start_x && !frac_end_x)delta.x--;

   if(end.y<start.y && !frac_start_y &&  frac_end_y)delta.y++;else
   if(end.y>start.y &&  frac_start_y && !frac_end_y)delta.y--;

   VecI2 adelta=Abs(delta);

  _active   =true;
  _side_step=false;
  _axis     =((adelta.x>adelta.y) ? 0 : (adelta.y>adelta.x) ? 1 : (Abs(end-start).maxI()==1)); // _axis=(adelta.maxI()==1);
  _steps    =adelta.max(); // Abs(delta.c[_axis]);
  _posr     =    start;
  _pos_end  =end      ;
  _step     =end-start;
   if(_axis==0) // horizontal
   {
      if(frac_start_x)
      {
        Flt dest=pos.x+(end.x>start.x);
       _pos_next.set(dest, start.y+(dest-start.x)/_step.x*_step.y);
      }else
      {
        _pos_next=start;
      }
     _pos=pos.y;
     _step/=Max(1, Abs(_step.x)); // "Max(1" solves problem for start(-0.5, -0.5) end(0.0, 0.0) generating 2 points, because of this, when calculating if side-step is needed, 'd' will get 1.0 value and not qualify
      if(!frac_start_x || !frac_end_x)_steps++;
   }else // vertical
   {
      if(frac_start_y)
      {
         Flt dest=pos.y+(end.y>start.y);
        _pos_next.set(start.x+(dest-start.y)/_step.y*_step.x, dest);
      }else
      {
        _pos_next=start;
      }
     _pos=pos.x;
     _step/=Max(1, Abs(_step.y)); // "Max(1" solves problem for start(-0.5, -0.5) end(0.0, 0.0) generating 2 points, because of this, when calculating if side-step is needed, 'd' will get 1.0 value and not qualify
      if(!frac_start_y || !frac_end_y)_steps++;
   }
   step();
}
void PixelWalkerEdge::step()
{
   if(_active)
   {
      if(_steps<=0)
      {
         if(_steps==0) // if this is the last step, then check if we have to perform a side-step
         {
           _pos_next=_pos_end;
           _steps= 1; step();
           _steps=-1; // make sure we will not make any more attempts
            if(sideStep())return;
         }
        _active=false;
      }else
      if(_side_step)
      {
        _side_step= false;
        _posr     =_pos_next; _pos_next+=_step;
        _steps--;
      }else
      {
         if(_axis==0) // horizontal
         {
            Int pos_y=Floor(_pos_next.y);
            if(_pos!=pos_y) // we've changed Y, we need to check if additional step is required
            {
               Flt dest=Max(_pos, pos_y), d=(dest-_posr.y)/_step.y; if(d>EPS && d<1-EPS) // EPS needed to avoid generating duplicate points at the same position due to numerical precision issues
               {
                 _posr.x+=d*_step.x;
                 _posr.y =dest; _side_step=true;
               }
              _pos=pos_y;
            }
         }else // vertical
         {
            Int pos_x=Floor(_pos_next.x);
            if(_pos!=pos_x) // we've changed X, we need to check if additional step is required
            {
               Flt dest=Max(_pos, pos_x), d=(dest-_posr.x)/_step.x; if(d>EPS && d<1-EPS) // EPS needed to avoid generating duplicate points at the same position due to numerical precision issues
               {
                 _posr.y+=d*_step.y;
                 _posr.x =dest; _side_step=true;
               }
              _pos=pos_x;
            }
         }
         if(!_side_step)
         {
           _posr=_pos_next; _pos_next+=_step;
           _steps--;
         }
      }
   }
}
/******************************************************************************/
void PixelWalkerEdgeMask::start(C Vec2 &start, C Vec2 &end, C RectI &mask)
{
//_mask.set(mask.min, mask.max+1).extend(EPS); // extend to make sure we will process positions at the borders, due to numerical precision issues
  _mask.set(mask.min-EPS, mask.max+(1+EPS)); // extend to make sure we will process positions at the borders, due to numerical precision issues

 //Edge2 edge(start, end); if(Clip(edge, _mask)) no need to clip 'end' because we have to do it anyway in 'step'
   Vec2 start_clamp; if(SweepPointRect(start, end-start, _mask, null, null, &start_clamp))
   {
    //super::start(edge.p[0], edge.p[1]);
      super::start(start_clamp, end);

      for(; !_mask.includes(T.pos()) && _active; )super::step(); // if not at the start, have to check multiple times in case we hit the mask rect at the edge, but it needs multiple steps to cross that border
   }else _active=false;
}
void PixelWalkerEdgeMask::step()
{
   super::step();
   if(!_mask.includes(T.pos()))_active=false; // have to check mask in each step, because there's no easy way to prevent the end from going out of range, can't just decrease '_steps' because last valid pixel could be a side step before a position that is out of range
}
/******************************************************************************/
// VOXEL WALKER
/******************************************************************************/
void VoxelWalker::start(C VecI &start, C VecI &end)
{
   VecI delta=end-start, adelta=Abs(delta); _axis.set(0, 1, 2);
   if(adelta.y<adelta.z){Swap(_axis.y, _axis.z); Swap(adelta.y, adelta.z);}
   if(adelta.x<adelta.y){Swap(_axis.x, _axis.y); Swap(adelta.x, adelta.y);
   if(adelta.y<adelta.z){Swap(_axis.y, _axis.z); Swap(adelta.y, adelta.z);}} // this can happen only if XY were swapped

  _active   =true;
  _side_step=false;
  _steps    =adelta.max();
  _main_sign=Sign(delta.c[_axis.x]);
  _pos      =start;
  _posr     =start+0.5f;
  _step     =delta; if(_steps)_step/=_steps;
  _pos_temp =_pos;
}
void VoxelWalker::start(C Vec &start, C Vec &end)
{
  _pos =Floor(start);
  _posr=      start ;

   VecI delta=Floor(end)-_pos, adelta=Abs(delta); _axis.set(0, 1, 2);
   if(adelta.y<adelta.z){Swap(_axis.y, _axis.z); Swap(adelta.y, adelta.z);}
   if(adelta.x<adelta.y){Swap(_axis.x, _axis.y); Swap(adelta.x, adelta.y);
   if(adelta.y<adelta.z){Swap(_axis.y, _axis.z); Swap(adelta.y, adelta.z);}} // this can happen only if XY were swapped

  _active   =true;
  _side_step=false;
  _steps    =adelta.max();
  _main_sign=Sign(delta.c[_axis.x]);
  _step     =end-start; if(_steps)_step/=_steps;
  _pos_temp =_pos;
}
void VoxelWalker::step()
{
   if(_active)
   {
      if(_side_step)
      {
         if(_side_step>=2)
         {
            Int axis=_side_step-2;
           _pos_temp.c[axis]=_pos.c[axis];
           _side_step=true;
         }else
         {
           _pos_temp =_pos;
           _side_step= false;
         }
      }else
      if(_steps-- <= 0)
      {
        _active=false;
      }else
      {
         Vec posr_next=_posr+_step;
         VecI pos_next;
         pos_next.c[_axis.x]=     _pos      .c[_axis.x]+_main_sign;
         pos_next.c[_axis.y]=Floor(posr_next.c[_axis.y]);
         pos_next.c[_axis.z]=Floor(posr_next.c[_axis.z]);
         VecI2 pos_changed(_pos.c[_axis.y]!=pos_next.c[_axis.y],  // if we've changed second axis
                           _pos.c[_axis.z]!=pos_next.c[_axis.z]); // if we've changed third  axis
         if(pos_changed.all()) // both secondary axes changed
         {
            Clamp(pos_next.c[_axis.y], _pos.c[_axis.y]-1, _pos.c[_axis.y]+1); // due to numerical precision issues the position can move more than 1, if for example the end is located at 0.0 coordinates and we would slightly cross the border, this is be cause in each step we accumulate '_step' which introduces some error
            Clamp(pos_next.c[_axis.z], _pos.c[_axis.z]-1, _pos.c[_axis.z]+1); // due to numerical precision issues the position can move more than 1, if for example the end is located at 0.0 coordinates and we would slightly cross the border, this is be cause in each step we accumulate '_step' which introduces some error

            // check which voxel was crossed first
            Flt dest;
            dest=Max(_pos.c[_axis.x], pos_next.c[_axis.x]); Flt d0=(dest-_posr.c[_axis.x])/_step.c[_axis.x];
            dest=Max(_pos.c[_axis.y], pos_next.c[_axis.y]); Flt d1=(dest-_posr.c[_axis.y])/_step.c[_axis.y];
            dest=Max(_pos.c[_axis.z], pos_next.c[_axis.z]); Flt d2=(dest-_posr.c[_axis.z])/_step.c[_axis.z];

            VecI order=_axis;
            if(d1>d2){Swap(order.y, order.z); Swap(d1, d2);}
            if(d0>d1){Swap(order.x, order.y); Swap(d0, d1);
            if(d1>d2){Swap(order.y, order.z); Swap(d1, d2);}} // this can happen only if XY were swapped

            // following code is for ignoring voxels when crossing at the same time 01 instead of 01
            //                                                                      10            11
            Int same=0;
            if(d0==d1) // if both were crossed at the same time
            {
               if(d0==d2)same=2;else // all the same, then don't move at all
               {
                 _pos_temp.c[order.y]=pos_next.c[order.y]; // move on order.y axis (this will be together with order.x)
                  order.y=order.z; d1=d2; // remove d1, replace with d2
                  same=1;
               }
            }else
            if(d1==d2)same=1;

           _pos_temp.c[order.x]=pos_next.c[order.x]; // move on order.x axis first
           _side_step=((same==0) ? 2+order.y : (same==1) ? true : false); // move on order.y axis next
         }else
         if(pos_changed.any()) // only one secondary axis was changed
         {
            Int main_axis=                 _axis.x           ;
            Int  sec_axis=(pos_changed.x ? _axis.y : _axis.z);

            Clamp(pos_next.c[sec_axis], _pos.c[sec_axis]-1, _pos.c[sec_axis]+1); // due to numerical precision issues the position can move more than 1, if for example the end is located at 0.0 coordinates and we would slightly cross the border, this is be cause in each step we accumulate '_step' which introduces some error

            // check which voxel was crossed first
            Flt dest;
            dest=Max(_pos.c[main_axis], pos_next.c[main_axis]); Flt d0=(dest-_posr.c[main_axis])/_step.c[main_axis];
            dest=Max(_pos.c[ sec_axis], pos_next.c[ sec_axis]); Flt d1=(dest-_posr.c[ sec_axis])/_step.c[ sec_axis];
            if(d0<d1){_pos_temp.c[main_axis]=pos_next.c[main_axis]; _side_step=true;}else // move in main_axis first
            if(d0>d1){_pos_temp.c[ sec_axis]=pos_next.c[ sec_axis]; _side_step=true;}     // move in  sec_axis first
         }
        _posr=posr_next;
        _pos = pos_next;
         if(!_side_step)_pos_temp=_pos;
      }
   }
}
/******************************************************************************/
Flt DistPointStr(C Vec2  &point, C Vec2  &str, C Vec2  &dir) {return  Abs(DistPointPlane(point, str, Perp(dir))     );}
Dbl DistPointStr(C VecD2 &point, C VecD2 &str, C VecD2 &dir) {return  Abs(DistPointPlane(point, str, Perp(dir))     );}
Flt DistPointStr(C Vec   &point, C Vec   &str, C Vec   &dir) {return Dist(  PointOnPlane(point, str,      dir ), str);}
Dbl DistPointStr(C VecD  &point, C Vec   &str, C Vec   &dir) {return Dist(  PointOnPlane(point, str,      dir ), str);}
Dbl DistPointStr(C VecD  &point, C VecD  &str, C VecD  &dir) {return Dist(  PointOnPlane(point, str,      dir ), str);}

Flt DistStrStr(C Vec &pos_a, C Vec &dir_a, C Vec &pos_b, C Vec &dir_b)
{
   Vec normal=Cross(dir_a, dir_b);
   if( normal.normalize())return Abs(DistPointPlane(pos_b, pos_a, normal));
                          return     Dist          (pos_a, NearestPointOnStr(pos_a, pos_b, dir_b)); // if they're parallel, find closest point on straight 'pos_b' towards point 'pos_a' and return distance between them
}

Flt DistPointEdge(C Vec2 &point, C Vec2 &edge_a, C Vec2 &edge_b, DIST_TYPE *type) // safe in case 'edge' is zero length because 'd' would be zero, and 'DistPointPlane' would be zero
{
   Vec2 d=edge_b-edge_a;
   if(DistPointPlane(point, edge_a, d)<=0){if(type)*type=DIST_POINT0; return Dist        (point, edge_a    );}
   if(DistPointPlane(point, edge_b, d)>=0){if(type)*type=DIST_POINT1; return Dist        (point, edge_b    );}
                                           if(type)*type=DIST_EDGE  ; return DistPointStr(point, edge_a, !d);
}
Dbl DistPointEdge(C VecD2 &point, C VecD2 &edge_a, C VecD2 &edge_b, DIST_TYPE *type) // safe in case 'edge' is zero length because 'd' would be zero, and 'DistPointPlane' would be zero
{
   VecD2 d=edge_b-edge_a;
   if(DistPointPlane(point, edge_a, d)<=0){if(type)*type=DIST_POINT0; return Dist        (point, edge_a    );}
   if(DistPointPlane(point, edge_b, d)>=0){if(type)*type=DIST_POINT1; return Dist        (point, edge_b    );}
                                           if(type)*type=DIST_EDGE  ; return DistPointStr(point, edge_a, !d);
}
Flt DistPointEdge(C Vec &point, C Vec &edge_a, C Vec &edge_b, DIST_TYPE *type) // safe in case 'edge' is zero length because 'd' would be zero, and 'DistPointPlane' would be zero
{
   Vec d=edge_b-edge_a;
   if(DistPointPlane(point, edge_a, d)<=0){if(type)*type=DIST_POINT0; return Dist        (point, edge_a    );}
   if(DistPointPlane(point, edge_b, d)>=0){if(type)*type=DIST_POINT1; return Dist        (point, edge_b    );}
                                           if(type)*type=DIST_EDGE  ; return DistPointStr(point, edge_a, !d);
}
Dbl DistPointEdge(C VecD &point, C Vec &edge_a, C Vec &edge_b, DIST_TYPE *type) // safe in case 'edge' is zero length because 'd' would be zero, and 'DistPointPlane' would be zero
{
   Vec d=edge_b-edge_a;
   if(DistPointPlane(point, edge_a, d)<=0){if(type)*type=DIST_POINT0; return Dist        (point, edge_a    );}
   if(DistPointPlane(point, edge_b, d)>=0){if(type)*type=DIST_POINT1; return Dist        (point, edge_b    );}
                                           if(type)*type=DIST_EDGE  ; return DistPointStr(point, edge_a, !d);
}
Dbl DistPointEdge(C VecD &point, C VecD &edge_a, C VecD &edge_b, DIST_TYPE *type) // safe in case 'edge' is zero length because 'd' would be zero, and 'DistPointPlane' would be zero
{
   VecD d=edge_b-edge_a;
   if(DistPointPlane(point, edge_a, d)<=0){if(type)*type=DIST_POINT0; return Dist        (point, edge_a    );}
   if(DistPointPlane(point, edge_b, d)>=0){if(type)*type=DIST_POINT1; return Dist        (point, edge_b    );}
                                           if(type)*type=DIST_EDGE  ; return DistPointStr(point, edge_a, !d);
}
/******************************************************************************/
Flt Dist(C Edge2 &a, C Edge2 &b)
{
   // check if they're cutting each other
   Vec2 a_dir =a.delta(); Flt a_length=a_dir.normalize();
   Vec2 b_dir =b.dir  (),
        a_perp=Perp(a_dir),
        b_perp=Perp(b_dir);
   Flt d0=DistPointPlane(b.p[0], a.p[0], a_perp); Int s0=Sign(d0);
   Flt d1=DistPointPlane(b.p[1], a.p[0], a_perp); Int s1=Sign(d1);
   if( s0==-s1 && s0)
   {
      Vec2 p=PointOnPlane  (b.p[0], b.p[1], d0, d1);
      Flt  d=DistPointPlane(  p   , a.p[0], a_dir );
      if(d>=0 && d<=a_length)return 0;
   }

   // they don't cut
   return Min(Dist(a.p[0], b),
              Dist(a.p[1], b),
              Dist(b.p[0], a),
              Dist(b.p[1], a));
}
Flt Dist(C Edge &a, C Edge &b) // safe in case 'a' or 'b' are zero length
{
   // check if they're cutting each other
   Edge c;
   Vec  a_dir=a.dir(),
        b_dir=b.dir();
   if(NearestPointOnStr(a.p[0], a_dir, b.p[0], b_dir, c)) // safe in case 'a' or 'b' are zero length
   {
      if(DistPointPlane(c.p[0], a.p[0], a_dir)>=0 && DistPointPlane(c.p[0], a.p[1], a_dir)<=0
      && DistPointPlane(c.p[1], b.p[0], b_dir)>=0 && DistPointPlane(c.p[1], b.p[1], b_dir)<=0)return c.length();
   }

   // they don't cut
   return Min(Dist(a.p[0], b),
              Dist(a.p[1], b),
              Dist(b.p[0], a),
              Dist(b.p[1], a));
}
/******************************************************************************/
Flt Dist(C Edge &edge, C Plane &plane) // safe in case 'a' or 'b' are zero length
{
   return Min(Dist(edge.p[0], plane),
              Dist(edge.p[1], plane));
}
/******************************************************************************/
Vec2 NearestPointOnEdge(C Vec2 &point, C Vec2 &edge_a, C Vec2 &edge_b)
{
   Vec2 dir=edge_b-edge_a;
   if(DistPointPlane(point, edge_a, dir)<=0)return edge_a;
   if(DistPointPlane(point, edge_b, dir)>=0)return edge_b;
   {
      dir.normalize();
      return edge_a + dir*DistPointPlane(point, edge_a, dir);
   }
}
Vec NearestPointOnEdge(C Vec &point, C Vec &edge_a, C Vec &edge_b)
{
   Vec dir=edge_b-edge_a;
   if(DistPointPlane(point, edge_a, dir)<=0)return edge_a;
   if(DistPointPlane(point, edge_b, dir)>=0)return edge_b;
   {
      dir.normalize();
      return edge_a + dir*DistPointPlane(point, edge_a, dir);
   }
}
Vec NearestPointOnStr(C Vec &point, C Vec &str, C Vec &dir)
{
   return str + dir*DistPointPlane(point, str, dir);
}
Bool NearestPointOnStr(C Vec &pos_a, C Vec &dir_a, C Vec &pos_b, C Vec &dir_b, Edge &out)
{
   Vec normal=Cross(dir_a, dir_b);
   if( normal.normalize())
   {
      out.p[0]=PointOnPlaneRay(pos_a, pos_b, CrossN(normal, dir_b), dir_a);
      out.p[1]=PointOnPlaneRay(pos_b, pos_a, CrossN(normal, dir_a), dir_b);
      return true;
   }
   return false;
}
/******************************************************************************/
Bool CutsPointEdge(C Vec2 &point, C Edge2_I &ei, Vec2 *cuts)
{
   if(Abs(DistPointPlane(point, ei.p[0], ei.normal))<=EPS)
   {
      const Flt eps=EPS;
      Flt l=DistPointPlane(point, ei.p[0], ei.dir);
      if( l>=-eps && l<=ei.length+eps)
      {
         if(cuts)
         {
            const Flt eps2=EPS;
            if(l<=          eps2)*cuts=ei.p[0];else
            if(l>=ei.length-eps2)*cuts=ei.p[1];else
                                 *cuts=point;
         }
         return true;
      }
   }
   return false;
}
Bool CutsPointEdge(C VecD2 &point, C EdgeD2_I &ei, VecD2 *cuts)
{
   if(Abs(DistPointPlane(point, ei.p[0], ei.normal))<=EPSD)
   {
      const Dbl eps=EPSD;
      Dbl l=DistPointPlane(point, ei.p[0], ei.dir);
      if( l>=-eps && l<=ei.length+eps)
      {
         if(cuts)
         {
            const Dbl eps2=EPSD;
            if(l<=          eps2)*cuts=ei.p[0];else
            if(l>=ei.length-eps2)*cuts=ei.p[1];else
                                 *cuts=point;
         }
         return true;
      }
   }
   return false;
}
Bool CutsPointEdge(C Vec &point, C Edge_I &ei, Vec *cuts)
{
   if(DistPointStr(point, ei.p[0], ei.dir)<=EPS)
   {
      const Flt eps=EPS;
      Flt l=DistPointPlane(point, ei.p[0], ei.dir);
      if( l>=-eps && l<=ei.length+eps)
      {
         if(cuts)
         {
            const Flt eps2=EPS;
            if(l<=          eps2)*cuts=ei.p[0];else
            if(l>=ei.length-eps2)*cuts=ei.p[1];else
                                 *cuts=point;
         }
         return true;
      }
   }
   return false;
}
Bool CutsPointEdge(C VecD &point, C EdgeD_I &ei, VecD *cuts)
{
   if(DistPointStr(point, ei.p[0], ei.dir)<=EPSD)
   {
      const Dbl eps=EPSD;
      Dbl l=DistPointPlane(point, ei.p[0], ei.dir);
      if( l>=-eps && l<=ei.length+eps)
      {
         if(cuts)
         {
            const Dbl eps2=EPSD;
            if(l<=          eps2)*cuts=ei.p[0];else
            if(l>=ei.length-eps2)*cuts=ei.p[1];else
                                 *cuts=point;
         }
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Int CutsEdgeEdge(C Edge2_I &a, C Edge2_I &b, Edge2 *cuts)
{
   if(cuts)
   {
      Vec2 point[2];
      Int  points=0;
      if(CutsPointEdge(b.p[0], a, &point[     0]))points++;
      if(CutsPointEdge(b.p[1], a, &point[points]))points++;
      if(points==0)
      {
         if(CutsPointEdge(a.p[0], b, &point[     0]))points++;
         if(CutsPointEdge(a.p[1], b, &point[points]))points++;
      }else
      if(points==1)
      {
         if(CutsPointEdge(a.p[0], b, &point[1]) && !Equal(point[0], point[1]))points++;else
         if(CutsPointEdge(a.p[1], b, &point[1]) && !Equal(point[0], point[1]))points++;
      }
      if(points)
      {
         cuts->p[0]=point[0];
         cuts->p[1]=point[1];
         return points;
      }
      Flt d0=DistPointPlane(b.p[0], a.p[0], a.normal); Int s0=Sign(d0);
      Flt d1=DistPointPlane(b.p[1], a.p[0], a.normal); Int s1=Sign(d1);
      if( s0==-s1 && s0)
      {
         Vec2 p=PointOnPlane  (b.p[0], b.p[1], d0, d1);
         Flt  d=DistPointPlane(  p   , a.p[0], a.dir );
         if(d>=0 && d<=a.length){cuts->p[0]=p; return 1;} // EPS doesn't need to be included because those cases have already been checked when testing points
      }
   }else
   {
      if(CutsPointEdge(a.p[0], b)
      || CutsPointEdge(a.p[1], b)
      || CutsPointEdge(b.p[0], a)
      || CutsPointEdge(b.p[1], a))return 1;
      Flt d0=DistPointPlane(b.p[0], a.p[0], a.normal); Int s0=Sign(d0);
      Flt d1=DistPointPlane(b.p[1], a.p[0], a.normal); Int s1=Sign(d1);
      if( s0==-s1 && s0)
      {
         Flt d=DistPointPlane(PointOnPlane(b.p[0], b.p[1], d0, d1), a.p[0], a.dir);
         if(d>=0 && d<=a.length)return 1; // EPS doesn't need to be included because those cases have already been checked when testing points
      }
   }
   return 0;
}
Int CutsEdgeEdge(C EdgeD2_I &a, C EdgeD2_I &b, EdgeD2 *cuts)
{
   if(cuts)
   {
      VecD2 point[2];
      Int   points=0;
      if(CutsPointEdge(b.p[0], a, &point[     0]))points++;
      if(CutsPointEdge(b.p[1], a, &point[points]))points++;
      if(points==0)
      {
         if(CutsPointEdge(a.p[0], b, &point[     0]))points++;
         if(CutsPointEdge(a.p[1], b, &point[points]))points++;
      }else
      if(points==1)
      {
         if(CutsPointEdge(a.p[0], b, &point[1]) && !Equal(point[0], point[1]))points++;else
         if(CutsPointEdge(a.p[1], b, &point[1]) && !Equal(point[0], point[1]))points++;
      }
      if(points)
      {
         cuts->p[0]=point[0];
         cuts->p[1]=point[1];
         return points;
      }
      Dbl d0=DistPointPlane(b.p[0], a.p[0], a.normal); Int s0=Sign(d0);
      Dbl d1=DistPointPlane(b.p[1], a.p[0], a.normal); Int s1=Sign(d1);
      if( s0==-s1 && s0)
      {
         VecD2 p=PointOnPlane  (b.p[0], b.p[1], d0, d1);
         Dbl   d=DistPointPlane(  p   , a.p[0], a.dir );
         if(d>=0 && d<=a.length){cuts->p[0]=p; return 1;} // EPS doesn't need to be included because those cases have already been checked when testing points
      }
   }else
   {
      if(CutsPointEdge(a.p[0], b)
      || CutsPointEdge(a.p[1], b)
      || CutsPointEdge(b.p[0], a)
      || CutsPointEdge(b.p[1], a))return 1;
      Dbl d0=DistPointPlane(b.p[0], a.p[0], a.normal); Int s0=Sign(d0);
      Dbl d1=DistPointPlane(b.p[1], a.p[0], a.normal); Int s1=Sign(d1);
      if( s0==-s1 && s0)
      {
         Dbl d=DistPointPlane(PointOnPlane(b.p[0], b.p[1], d0, d1), a.p[0], a.dir);
         if(d>=0 && d<=a.length)return 1; // EPS doesn't need to be included because those cases have already been checked when testing points
      }
   }
   return 0;
}
Int CutsEdgeEdge(C Edge_I &a, C Edge_I &b, Edge *cuts)
{
   if(cuts)
   {
      Vec point[2];
      Int points=0;
      if(CutsPointEdge(b.p[0], a, &point[     0]))points++;
      if(CutsPointEdge(b.p[1], a, &point[points]))points++;
      if(points==0)
      {
         if(CutsPointEdge(a.p[0], b, &point[     0]))points++;
         if(CutsPointEdge(a.p[1], b, &point[points]))points++;
      }else
      if(points==1)
      {
         if(CutsPointEdge(a.p[0], b, &point[1]) && !Equal(point[0], point[1]))points++;else
         if(CutsPointEdge(a.p[1], b, &point[1]) && !Equal(point[0], point[1]))points++;
      }
      if(points)
      {
         cuts->p[0]=point[0];
         cuts->p[1]=point[1];
         return points;
      }
      Vec normal=Cross(a.dir, b.dir);
      if( normal.normalize() && Abs(DistPointPlane(b.p[0], a.p[0], normal))<=EPS)
      {
         Vec p=PointOnPlaneRay(a.p[0], b.p[0], CrossN(normal, b.dir), a.dir);
         Flt d=DistPointPlane (p     , b.p[0],                        b.dir);
         if( d>=0 && d<=b.length) // EPS doesn't need to be included because those cases have already been checked when testing points
         {
            d=DistPointPlane(p, a.p[0], a.dir);
            if(d>=0 && d<=a.length){cuts->p[0]=p; return 1;} // EPS doesn't need to be included because those cases have already been checked when testing points
         }
      }
   }else
   {
      if(CutsPointEdge(a.p[0], b)
      || CutsPointEdge(a.p[1], b)
      || CutsPointEdge(b.p[0], a)
      || CutsPointEdge(b.p[1], a))return 1;
      Vec normal=Cross(a.dir, b.dir);
      if( normal.normalize() && Abs(DistPointPlane(b.p[0], a.p[0], normal))<=EPS)
      {
         Vec p=PointOnPlaneRay(a.p[0], b.p[0], CrossN(normal, b.dir), a.dir);
         Flt d=DistPointPlane (p     , b.p[0],                        b.dir);
         if( d>=0 && d<=b.length) // EPS doesn't need to be included because those cases have already been checked when testing points
         {
            d=DistPointPlane(p, a.p[0], a.dir);
            if(d>=0 && d<=a.length)return 1; // EPS doesn't need to be included because those cases have already been checked when testing points
         }
      }
   }
   return 0;
}
Int CutsEdgeEdge(C EdgeD_I &a, C EdgeD_I &b, EdgeD *cuts)
{
   if(cuts)
   {
      VecD point[2];
      Int  points=0;
      if(CutsPointEdge(b.p[0], a, &point[     0]))points++;
      if(CutsPointEdge(b.p[1], a, &point[points]))points++;
      if(points==0)
      {
         if(CutsPointEdge(a.p[0], b, &point[     0]))points++;
         if(CutsPointEdge(a.p[1], b, &point[points]))points++;
      }else
      if(points==1)
      {
         if(CutsPointEdge(a.p[0], b, &point[1]) && !Equal(point[0], point[1]))points++;else
         if(CutsPointEdge(a.p[1], b, &point[1]) && !Equal(point[0], point[1]))points++;
      }
      if(points)
      {
         cuts->p[0]=point[0];
         cuts->p[1]=point[1];
         return points;
      }
      VecD normal=Cross(a.dir, b.dir);
      if(  normal.normalize() && Abs(DistPointPlane(b.p[0], a.p[0], normal))<=EPSD)
      {
         VecD p=PointOnPlaneRay(a.p[0], b.p[0], CrossN(normal, b.dir), a.dir);
         Dbl  d=DistPointPlane (p     , b.p[0],                        b.dir);
         if(  d>=0 && d<=b.length) // EPS doesn't need to be included because those cases have already been checked when testing points
         {
            d=DistPointPlane(p, a.p[0], a.dir);
            if(d>=0 && d<=a.length){cuts->p[0]=p; return 1;} // EPS doesn't need to be included because those cases have already been checked when testing points
         }
      }
   }else
   {
      if(CutsPointEdge(a.p[0], b)
      || CutsPointEdge(a.p[1], b)
      || CutsPointEdge(b.p[0], a)
      || CutsPointEdge(b.p[1], a))return 1;
      VecD normal=Cross(a.dir, b.dir);
      if(  normal.normalize() && Abs(DistPointPlane(b.p[0], a.p[0], normal))<=EPSD)
      {
         VecD p=PointOnPlaneRay(a.p[0], b.p[0], CrossN(normal, b.dir), a.dir);
         Dbl  d=DistPointPlane (p     , b.p[0],                        b.dir);
         if(  d>=0 && d<=b.length) // EPS doesn't need to be included because those cases have already been checked when testing points
         {
            d=DistPointPlane(p, a.p[0], a.dir);
            if(d>=0 && d<=a.length)return 1; // EPS doesn't need to be included because those cases have already been checked when testing points
         }
      }
   }
   return 0;
}
/******************************************************************************/
Bool Cuts(C Edge &edge, C Plane &plane, Vec *cuts)
{
   Flt d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   if(Sign(d0)!=Sign(d1)){if(cuts)*cuts=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   if(Abs (d0)<=EPS     ){if(cuts)*cuts=             edge.p[0]                    ; return true;}
   if(Abs (d1)<=EPS     ){if(cuts)*cuts=             edge.p[1]                    ; return true;}
   return false;
}
Bool Cuts(C EdgeD &edge, C PlaneM &plane, VecD *cuts)
{
   Dbl d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   if(Sign(d0)!=Sign(d1)){if(cuts)*cuts=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   if(Abs (d0)<=EPSD    ){if(cuts)*cuts=             edge.p[0]                    ; return true;}
   if(Abs (d1)<=EPSD    ){if(cuts)*cuts=             edge.p[1]                    ; return true;}
   return false;
}
/******************************************************************************/
Bool Clip(Edge2 &edge, C Plane2 &plane)
{
   Flt d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   if( d0>0 && d1>0)return false; // both outside
   if( d0>0        )edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);else
   if( d1>0        )edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);
   return true;
}
Bool Clip(EdgeD2 &edge, C PlaneD2 &plane)
{
   Dbl d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   if( d0>0 && d1>0)return false; // both outside
   if( d0>0        )edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);else
   if( d1>0        )edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);
   return true;
}
Bool Clip(Edge &edge, C Plane &plane)
{
   Flt d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   if( d0>0 && d1>0)return false; // both outside
   if( d0>0        )edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);else
   if( d1>0        )edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);
   return true;
}
Bool Clip(EdgeD &edge, C PlaneD &plane)
{
   Dbl d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   if( d0>0 && d1>0)return false; // both outside
   if( d0>0        )edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);else
   if( d1>0        )edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1);
   return true;
}
/******************************************************************************/
Bool ClipEps(Edge2 &edge, C Plane2 &plane)
{
   Flt d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   Int s0=SignEps(d0),
       s1=SignEps(d1);
   if( s0<=0 && s1<=0)return true;
   if( d0< 0         ){edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   if( d1< 0         ){edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   return false;
}
Bool ClipEps(EdgeD2 &edge, C PlaneD2 &plane)
{
   Dbl d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   Int s0=SignEps(d0),
       s1=SignEps(d1);
   if( s0<=0 && s1<=0)return true;
   if( d0< 0         ){edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   if( d1< 0         ){edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   return false;
}
Bool ClipEps(Edge &edge, C Plane &plane)
{
   Flt d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   Int s0=SignEps(d0),
       s1=SignEps(d1);
   if( s0<=0 && s1<=0)return true;
   if( d0< 0         ){edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   if( d1< 0         ){edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   return false;
}
Bool ClipEps(EdgeD &edge, C PlaneD &plane)
{
   Dbl d0=Dist(edge.p[0], plane),
       d1=Dist(edge.p[1], plane);
   Int s0=SignEps(d0),
       s1=SignEps(d1);
   if( s0<=0 && s1<=0)return true;
   if( d0< 0         ){edge.p[1]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   if( d1< 0         ){edge.p[0]=PointOnPlane(edge.p[0], edge.p[1], d0, d1); return true;}
   return false;
}
/******************************************************************************/
Bool Clip(Edge2 &edge, C Rect &rect)
{
   Flt d0, d1;

   // left
   d0=rect.min.x-edge.p[0].x;
   d1=rect.min.x-edge.p[1].x;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(rect.min.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));else
   if(d1>0        )edge.p[1].set(rect.min.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));

   // right
   d0=edge.p[0].x-rect.max.x;
   d1=edge.p[1].x-rect.max.x;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(rect.max.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));else
   if(d1>0        )edge.p[1].set(rect.max.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));

   // bottom
   d0=rect.min.y-edge.p[0].y;
   d1=rect.min.y-edge.p[1].y;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.min.y);else
   if(d1>0        )edge.p[1].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.min.y);

   // top
   d0=edge.p[0].y-rect.max.y;
   d1=edge.p[1].y-rect.max.y;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.max.y);else
   if(d1>0        )edge.p[1].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.max.y);

   return true;
}
Bool Clip(EdgeD2 &edge, C RectD &rect)
{
   Dbl d0, d1;

   // left
   d0=rect.min.x-edge.p[0].x;
   d1=rect.min.x-edge.p[1].x;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(rect.min.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));else
   if(d1>0        )edge.p[1].set(rect.min.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));

   // right
   d0=edge.p[0].x-rect.max.x;
   d1=edge.p[1].x-rect.max.x;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(rect.max.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));else
   if(d1>0        )edge.p[1].set(rect.max.x, PointOnPlane(edge.p[0].y, edge.p[1].y, d0, d1));

   // bottom
   d0=rect.min.y-edge.p[0].y;
   d1=rect.min.y-edge.p[1].y;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.min.y);else
   if(d1>0        )edge.p[1].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.min.y);

   // top
   d0=edge.p[0].y-rect.max.y;
   d1=edge.p[1].y-rect.max.y;
   if(d0>0 && d1>0)return false;
   if(d0>0        )edge.p[0].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.max.y);else
   if(d1>0        )edge.p[1].set(PointOnPlane(edge.p[0].x, edge.p[1].x, d0, d1), rect.max.y);

   return true;
}
/******************************************************************************/
Bool Clip(Edge &edge, C Box &box)
{
   Flt d0, d1;

   // left
   d0=box.min.x-edge.p[0].x;
   d1=box.min.x-edge.p[1].x;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(box.min.x, edge.lerpY(s), edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(box.min.x, edge.lerpY(s), edge.lerpZ(s));}

   // right
   d0=edge.p[0].x-box.max.x;
   d1=edge.p[1].x-box.max.x;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(box.max.x, edge.lerpY(s), edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(box.max.x, edge.lerpY(s), edge.lerpZ(s));}

   // bottom
   d0=box.min.y-edge.p[0].y;
   d1=box.min.y-edge.p[1].y;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), box.min.y, edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), box.min.y, edge.lerpZ(s));}

   // top
   d0=edge.p[0].y-box.max.y;
   d1=edge.p[1].y-box.max.y;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), box.max.y, edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), box.max.y, edge.lerpZ(s));}

   // back
   d0=box.min.z-edge.p[0].z;
   d1=box.min.z-edge.p[1].z;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), edge.lerpY(s), box.min.z);}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), edge.lerpY(s), box.min.z);}

   // forward
   d0=edge.p[0].z-box.max.z;
   d1=edge.p[1].z-box.max.z;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), edge.lerpY(s), box.max.z);}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), edge.lerpY(s), box.max.z);}

   return true;
}
Bool Clip(EdgeD &edge, C BoxD &box)
{
   Dbl d0, d1;

   // left
   d0=box.min.x-edge.p[0].x;
   d1=box.min.x-edge.p[1].x;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(box.min.x, edge.lerpY(s), edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(box.min.x, edge.lerpY(s), edge.lerpZ(s));}

   // right
   d0=edge.p[0].x-box.max.x;
   d1=edge.p[1].x-box.max.x;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(box.max.x, edge.lerpY(s), edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(box.max.x, edge.lerpY(s), edge.lerpZ(s));}

   // bottom
   d0=box.min.y-edge.p[0].y;
   d1=box.min.y-edge.p[1].y;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), box.min.y, edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), box.min.y, edge.lerpZ(s));}

   // top
   d0=edge.p[0].y-box.max.y;
   d1=edge.p[1].y-box.max.y;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), box.max.y, edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), box.max.y, edge.lerpZ(s));}

   // back
   d0=box.min.z-edge.p[0].z;
   d1=box.min.z-edge.p[1].z;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), edge.lerpY(s), box.min.z);}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), edge.lerpY(s), box.min.z);}

   // forward
   d0=edge.p[0].z-box.max.z;
   d1=edge.p[1].z-box.max.z;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), edge.lerpY(s), box.max.z);}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), edge.lerpY(s), box.max.z);}

   return true;
}
Bool Clip(Edge &edge, C Extent &ext)
{
   Flt d0, d1, border;

   // left
   border=ext.minX();
   d0=border-edge.p[0].x;
   d1=border-edge.p[1].x;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(border, edge.lerpY(s), edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(border, edge.lerpY(s), edge.lerpZ(s));}

   // right
   border=ext.maxX();
   d0=edge.p[0].x-border;
   d1=edge.p[1].x-border;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(border, edge.lerpY(s), edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(border, edge.lerpY(s), edge.lerpZ(s));}

   // bottom
   border=ext.minY();
   d0=border-edge.p[0].y;
   d1=border-edge.p[1].y;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), border, edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), border, edge.lerpZ(s));}

   // top
   border=ext.maxY();
   d0=edge.p[0].y-border;
   d1=edge.p[1].y-border;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), border, edge.lerpZ(s));}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), border, edge.lerpZ(s));}

   // back
   border=ext.minZ();
   d0=border-edge.p[0].z;
   d1=border-edge.p[1].z;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), edge.lerpY(s), border);}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), edge.lerpY(s), border);}

   // forward
   border=ext.maxZ();
   d0=edge.p[0].z-border;
   d1=edge.p[1].z-border;
   if(d0>0 && d1>0)return false;
   if(d0>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[0].set(edge.lerpX(s), edge.lerpY(s), border);}else
   if(d1>0        ){Flt s=PointOnPlaneStep(d0, d1); edge.p[1].set(edge.lerpX(s), edge.lerpY(s), border);}

   return true;
}
/******************************************************************************/
Bool SweepPointEdge(C Vec2 &point, C Vec2 &move, C Edge2_I &ei, Flt *hit_frac, Vec2 *hit_normal, Vec2 *hit_pos)
{
   Vec2 hitp; if(SweepPointPlane(point, move, Plane2(ei.p[0], ei.normal), hit_frac, hit_normal, &hitp))
   {
      Flt d=DistPointPlane(hitp, ei.p[0], ei.dir);
      if( d>=0 && d<=ei.length)
      {
         if(hit_pos)*hit_pos=hitp;
         return true;
      }
   }
   return false;
}
Bool SweepPointEdge(C VecD2 &point, C VecD2 &move, C EdgeD2_I &ei, Dbl *hit_frac, VecD2 *hit_normal, VecD2 *hit_pos)
{
   VecD2 hitp; if(SweepPointPlane(point, move, PlaneD2(ei.p[0], ei.normal), hit_frac, hit_normal, &hitp))
   {
      Dbl d=DistPointPlane(hitp, ei.p[0], ei.dir);
      if( d>=0 && d<=ei.length)
      {
         if(hit_pos)*hit_pos=hitp;
         return true;
      }
   }
   return false;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void DrawArrow(C Color &color, C Vec &start, C Vec &end, Flt tip_radius, Flt tip_length)
{
   Matrix3 m;
   Vec     dir=end-start;
   Flt     len=dir.normalize();
   m.setDir(dir);
   m.x*= tip_radius*len;
   m.y*= tip_radius*len;
   m.z*=-tip_length*len;

   VI.color(color);
   VI.line (start, end);
   Vec end2=end+m.z;
   REP(3)
   {
      Flt c, s; CosSin(c, s, i*PI2/3);
      VI.line(end, end2 + m.x*c + m.y*s);
   }
   VI.end();
}
void DrawArrow2(C Color &color, C Vec &start, C Vec &end, Flt width, Flt tip_radius, Flt tip_length)
{
   Matrix3 m;
   Vec     dir=end-start;
   Flt     len=dir.normalize();
   m.setDir(dir);
   m.x*= tip_radius*len;
   m.y*= tip_radius*len;
   m.z*=-tip_length*len;

   Edge(start, end).draw(color, width);
   Vec end2=end+m.z;
   REP(3)
   {
      Flt c, s; CosSin(c, s, i*PI2/3);
      Edge(end, end2 + m.x*c + m.y*s).draw(color, width);
   }
}
/******************************************************************************/
void SubdivideEdges(C MemPtr<Vec> &src, MemPtr<Vec> dest)
{
   dest.setNum(Max(0, src.elms() + (src.elms()-1))); // original + edge points

   // edge center points
   REP(src.elms()-1)dest[i*2+1]=Avg(src[i], src[i+1]);

   // original points
   REPA(src)
   {
      Vec &d=dest[i*2];
      if(i==0 || i==src.elms()-1)d=src[i];
      else                       d=Avg(src[i], Avg(dest[i*2-1], dest[i*2+1]));
   }
}
void SubdivideEdges(C MemPtr<VtxFull> &src, MemPtr<VtxFull> dest)
{
   dest.setNum(Max(0, src.elms() + (src.elms()-1))); // original + edge points

   // edge center points
   REP(src.elms()-1)dest[i*2+1].avg(src[i], src[i+1]);

   // original points
   REPA(src)
   {
      VtxFull &d=dest[i*2];
      if(i==0 || i==src.elms()-1)d=src[i];
      else                       d.avg(src[i], d.avg(dest[i*2-1], dest[i*2+1]));
   }
}
/******************************************************************************/
}
/******************************************************************************/
