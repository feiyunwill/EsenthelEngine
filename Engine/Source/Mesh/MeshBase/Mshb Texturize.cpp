/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
MeshBase& MeshBase::texMap(Flt scale, Byte tex_index)
{
   if(InRange(tex_index, 3))
   if(C Vec *pos=vtx.pos())
   {
      Vec2 *tex; switch(tex_index){default: include(VTX_TEX0); tex=vtx.tex0(); break; case 1: include(VTX_TEX1); tex=vtx.tex1(); break; case 2: include(VTX_TEX2); tex=vtx.tex2(); break;}
      REPA(vtx)
      {
         (tex++)->set(pos->x*scale, 1-pos->y*scale);
          pos++;
      }
   }
   return T;
}
MeshBase& MeshBase::texMapXZ(Flt scale, Byte tex_index)
{
   if(InRange(tex_index, 3))
   if(C Vec *pos=vtx.pos())
   {
      Vec2 *tex; switch(tex_index){default: include(VTX_TEX0); tex=vtx.tex0(); break; case 1: include(VTX_TEX1); tex=vtx.tex1(); break; case 2: include(VTX_TEX2); tex=vtx.tex2(); break;}
      REPA(vtx)
      {
         (tex++)->set(pos->x*scale, 1-pos->z*scale);
          pos++;
      }
   }
   return T;
}
MeshBase& MeshBase::texMap(C Matrix &matrix, Byte tex_index)
{
   if(InRange(tex_index, 3))
   if(C Vec *pos=vtx.pos())
   {
      Vec2 *tex; switch(tex_index){default: include(VTX_TEX0); tex=vtx.tex0(); break; case 1: include(VTX_TEX1); tex=vtx.tex1(); break; case 2: include(VTX_TEX2); tex=vtx.tex2(); break;}
      REPA(vtx)(*tex++)=((*pos++)*matrix).xy;
   }
   return T;
}
MeshBase& MeshBase::texMap(C Plane &plane, Byte tex_index)
{
   if(InRange(tex_index, 3))
   if(C Vec *pos=vtx.pos())
   {
      Vec2   *tex; switch(tex_index){default: include(VTX_TEX0); tex=vtx.tex0(); break; case 1: include(VTX_TEX1); tex=vtx.tex1(); break; case 2: include(VTX_TEX2); tex=vtx.tex2(); break;}
      Matrix3 matrix; matrix.setDir(plane.normal);
      REPA(vtx)
      {
         Vec vec=(*pos++)-plane.pos;
         (tex++)->set(Dot(vec, matrix.x),
                      Dot(vec, matrix.y));
      }
   }
   return T;
}
MeshBase& MeshBase::texMap(C Ball &ball, Byte tex_index)
{
   if(InRange(tex_index, 3))
   if(C Vec *pos=vtx.pos())
   {
      Vec2 *tex; switch(tex_index){default: include(VTX_TEX0); tex=vtx.tex0(); break; case 1: include(VTX_TEX1); tex=vtx.tex1(); break; case 2: include(VTX_TEX2); tex=vtx.tex2(); break;}
      REPA(vtx)
      {
         Vec v=(*pos++)-ball.pos;
         (tex++)->set(Angle       ( Vec2(v.x, v.z))/PI2,
                   AbsAngleBetween(v, Vec(0, 1, 0))/PI );
      }
   }
   return T;
}
MeshBase& MeshBase::texMap(C Tube &tube, Byte tex_index)
{
   if(InRange(tex_index, 3))
   if(C Vec *pos=vtx.pos())
   {
      Vec2   *tex; switch(tex_index){default: include(VTX_TEX0); tex=vtx.tex0(); break; case 1: include(VTX_TEX1); tex=vtx.tex1(); break; case 2: include(VTX_TEX2); tex=vtx.tex2(); break;}
      Matrix3 matrix; matrix.setDir(tube.up); matrix.z.chs();
      REPA(vtx)
      {
         Vec v=(*pos++)-tube.pos;
         (tex++)->set(-Angle(v, matrix  )/PI2,
                         Dot(v, matrix.z)/tube.h/2+0.5f);
      }
   }
   return T;
}
MeshBase& MeshBase::texMove  (C Vec2 &move , Byte tex_index) {if(InRange(tex_index, 3) && move.any()){Vec2 *tex=(tex_index==0 ? vtx.tex0() : tex_index==1 ? vtx.tex1() : vtx.tex2());                                        if(tex)REPA(vtx)(*tex++)+=move                 ;} return T;}
MeshBase& MeshBase::texScale (C Vec2 &scale, Byte tex_index) {if(InRange(tex_index, 3) && scale!=1  ){Vec2 *tex=(tex_index==0 ? vtx.tex0() : tex_index==1 ? vtx.tex1() : vtx.tex2());                                        if(tex)REPA(vtx)(*tex++)*=scale                ;} return T;}
MeshBase& MeshBase::texRotate(  Flt   angle, Byte tex_index) {if(InRange(tex_index, 3) && angle     ){Vec2 *tex=(tex_index==0 ? vtx.tex0() : tex_index==1 ? vtx.tex1() : vtx.tex2()); Flt cos, sin; CosSin(cos, sin, angle); if(tex)REPA(vtx)(*tex++).rotateCosSin(cos, sin);} return T;}
/******************************************************************************/
}
/******************************************************************************/
