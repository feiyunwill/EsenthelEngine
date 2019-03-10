/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// GET
/******************************************************************************/
Int MeshBase::maxId()C
{
   Int max=-1;
   if(edge.id())REPA(edge)MAX(max, edge.id(i));
   if(tri .id())REPA(tri )MAX(max, tri .id(i));
   if(quad.id())REPA(quad)MAX(max, quad.id(i));
   return max;
}
Bool MeshBase::hasId(Int id)C
{
   if(edge.id())REPA(edge)if(edge.id(i)==id)return true;
   if(tri .id())REPA(tri )if(tri .id(i)==id)return true;
   if(quad.id())REPA(quad)if(quad.id(i)==id)return true;
   return false;
}
/******************************************************************************/
Bool MeshBase::getRect(Rect &rect)C
{
   Box box; Bool ret=getBox(box);
   rect.min=box.min.xy;
   rect.max=box.max.xy;
   return ret;
}
Bool MeshBase::getRectXZ(Rect &rect)C
{
   Box box; Bool ret=getBox(box);
   rect.min=box.min.xz();
   rect.max=box.max.xz();
   return ret;
}
Bool MeshBase::getBox (Box  &box                        )C {return box .from(vtx.pos(), vtxs());}
Bool MeshBase::getBox (Box  &box , C Matrix &mesh_matrix)C {return box .from(vtx.pos(), vtxs(), mesh_matrix);}
Bool MeshBase::getBall(Ball &ball                       )C {return ball.from(vtx.pos(), vtxs());}

Flt MeshBase::area(Vec *center)C
{
   if(center)center->zero();
   Flt area=0;
   if(C Vec *vtx=T.vtx.pos())
   {
      REPA(tri)
      {
       C VecI &f=tri.ind(i); Tri t(vtx[f.x], vtx[f.y], vtx[f.z]);
         Flt   a=t.area();
                    area  +=a;
         if(center)*center+=a*t.center();
      }
      REPA(quad)
      {
       C VecI4 &f=quad.ind(i); Quad q(vtx[f.x], vtx[f.y], vtx[f.z], vtx[f.w]);
         Flt    a=q.area();
                    area  +=a;
         if(center)*center+=a*q.center();
      }
   }
   if(center && area)*center/=area;
   return area;
}
/******************************************************************************/
Flt MeshBase::convexVolume()C
{
   Flt vol=0; 
   Box box; if(getBox(box))
   {
    C Vec *pos=vtx.pos(); // 'getBox' implies "vtx.pos()!=null"
      Vec  center=box.center();
      REPA(tri)
      {
       C VecI &ind=tri.ind(i);
         vol+=TetraVolume(pos[ind.x], pos[ind.y], pos[ind.z], center);
      }
      REPA(quad)
      {
       C VecI4 &ind=quad.ind(i);
         vol+=TetraVolume(pos[ind.x], pos[ind.y], pos[ind.w], center);
         vol+=TetraVolume(pos[ind.w], pos[ind.y], pos[ind.z], center);
      }
   }
   return vol;
}
/******************************************************************************/
// SET
/******************************************************************************/
MeshBase& MeshBase::setEdgeNormals(Bool flag)
{
   exclude(FACE_NRM);
   include(EDGE_NRM);
   if(flag && !edge.flag())flag=false;
   REPA(edge)
   {
      Int *p  =edge.ind(i).c;
      Int  dir=1;
      if(flag)switch(edge.flag(i)&ETQ_LR)
      {
         case 0     :
         case ETQ_LR: dir= 0; break;
         case ETQ_L : dir=-1; break;
      }
      switch(dir)
      {
         case  0: edge.nrm(i).zero(); break;
         case -1: edge.nrm(i).set (PerpN(vtx.pos(p[0]).xy-vtx.pos(p[1]).xy), 0); break;
         case +1: edge.nrm(i).set (PerpN(vtx.pos(p[1]).xy-vtx.pos(p[0]).xy), 0); break;
      }
   }
   return T;
}
MeshBase& MeshBase::setFaceNormals()
{
   if(vtx.pos())
   {
      exclude(EDGE_NRM);
      include(FACE_NRM);
      if(VecI  *_tri =tri .ind()){Vec *nrm=tri .nrm(); REPA(tri ){Int *p=(_tri ++)->c; *nrm++=GetNormal(vtx.pos(p[0]), vtx.pos(p[1]), vtx.pos(p[2]));}}
      if(VecI4 *_quad=quad.ind()){Vec *nrm=quad.nrm(); REPA(quad){Int *p=(_quad++)->c; *nrm++=GetNormal(vtx.pos(p[0]), vtx.pos(p[1]), vtx.pos(p[3]));}}
   }
   return T;
}
MeshBase& MeshBase::setNormals2D(Bool flag)
{
   setEdgeNormals(flag);
   include(VTX_NRM); ZeroN(vtx.nrm(), vtxs());

   VecI2 *_edge=edge.ind();
   Vec   * nrm =edge.nrm(); REPA(edge)
   {
      VecI2 f=*_edge++; f.remap(vtx.dup());
      vtx.nrm(f.c[0])+=*nrm;
      vtx.nrm(f.c[1])+=*nrm++;
   }
   Normalize(vtx.nrm(), vtxs());
   if(vtx.dup())REPA(vtx)vtx.nrm(i)=vtx.nrm(vtx.dup(i));
   return T;
}
MeshBase& MeshBase::setNormals()
{
   include(VTX_NRM); ZeroN(vtx.nrm(), vtxs());

#if 1 // weighted normal depending on face surface area (that's why there's 'GetNormalU')
   REPA(tri)
   {
      VecI f  =tri.ind(i); f.remap(vtx.dup());
      Vec  nrm=GetNormalU(vtx.pos(f.c[0]), vtx.pos(f.c[1]), vtx.pos(f.c[2]));
      vtx.nrm(f.c[0])+=nrm;
      vtx.nrm(f.c[1])+=nrm;
      vtx.nrm(f.c[2])+=nrm;
   }

   REPA(quad)
   {
      VecI4 f =quad.ind(i); f.remap(vtx.dup());
      Vec  &v0=vtx.pos(f.c[0]),
           &v1=vtx.pos(f.c[1]),
           &v2=vtx.pos(f.c[2]),
           &v3=vtx.pos(f.c[3]),
           nrm=GetNormalU(v0, v1, v3)+GetNormalU(v1, v2, v3);
      vtx.nrm(f.c[0])+=nrm;
      vtx.nrm(f.c[1])+=nrm;
      vtx.nrm(f.c[2])+=nrm;
      vtx.nrm(f.c[3])+=nrm;
   }
   Normalize(vtx.nrm(), vtxs());
#elif 1 // weighted normal depending on face angle
   REPA(tri)
   {
      VecI f  =tri.ind(i); f.remap(vtx.dup());
      Vec &v0 =vtx.pos(f.c[0]),
          &v1 =vtx.pos(f.c[1]),
          &v2 =vtx.pos(f.c[2]),
           nrm=GetNormal(v0, v1, v2);
      Flt  a0 =AbsAngleBetween(v2, v0, v1),
           a1 =AbsAngleBetween(v0, v1, v2), a2=PI-a0-a1;
      vtx.nrm(f.c[0])+=a0*nrm;
      vtx.nrm(f.c[1])+=a1*nrm;
      vtx.nrm(f.c[2])+=a2*nrm;
   }
   REPA(quad)
   {
      VecI4 f     =quad.ind(i); f.remap(vtx.dup());
      Vec  &v0    =vtx.pos(f.c[0]),
           &v1    =vtx.pos(f.c[1]),
           &v2    =vtx.pos(f.c[2]),
           &v3    =vtx.pos(f.c[3]),
            nrm013=GetNormal(v0, v1, v3),
            nrm123=GetNormal(v1, v2, v3);
      Flt   a301  =AbsAngleBetween(v3, v0, v1),
            a013  =AbsAngleBetween(v0, v1, v3), a130=PI-a301-a013,
            a123  =AbsAngleBetween(v1, v2, v3),
            a231  =AbsAngleBetween(v2, v3, v1), a312=PI-a123-a231;
      vtx.nrm(f.c[0])+=a301*nrm013;
      vtx.nrm(f.c[1])+=a013*nrm013 + a312*nrm123;
      vtx.nrm(f.c[2])+=            + a123*nrm123;
      vtx.nrm(f.c[3])+=a130*nrm013 + a231*nrm123;
   }
   Normalize(vtx.nrm(), vtxs());
#else // calculate using Quadrics (this is only for testing)
   Memt<QuadricMatrix> vtx_qm; vtx_qm.setNumZero(vtxs());
   REPA(tri)
   {
      VecI f     =tri.ind(i); f.remap(vtx.dup());
      Vec &v0    =vtx.pos(f.c[0]),
          &v1    =vtx.pos(f.c[1]),
          &v2    =vtx.pos(f.c[2]),
           nrm   =GetNormalU(v0, v1, v2);
      Flt  weight=nrm.normalize();
      QuadricMatrix qm(nrm, v0); qm*=weight;
      vtx_qm[f.c[0]]+=qm;
      vtx_qm[f.c[1]]+=qm;
      vtx_qm[f.c[2]]+=qm;
   }

   REPA(quad)
   {
      VecI4 f     =quad.ind(i); f.remap(vtx.dup());
      Vec  &v0    =vtx.pos(f.c[0]),
           &v1    =vtx.pos(f.c[1]),
           &v2    =vtx.pos(f.c[2]),
           &v3    =vtx.pos(f.c[3]),
            nrm013=GetNormalU(v0, v1, v3),
            nrm123=GetNormalU(v1, v2, v3);
      Flt   weight013=nrm013.normalize(),
            weight123=nrm123.normalize();
      QuadricMatrix qm013(nrm013, v0); qm013*=weight013;
      QuadricMatrix qm123(nrm123, v1); qm123*=weight123;
      vtx_qm[f.c[0]]+=qm013;
      vtx_qm[f.c[1]]+=qm013+qm123;
      vtx_qm[f.c[2]]+=      qm123;
      vtx_qm[f.c[3]]+=qm013+qm123;
   }

   REPA(vtx)vtx.nrm(i)=vtx_qm[i].normal(vtx.pos(i)+offset); // <- here we would have to use some offset because normal at the surface is not precise, best offset is vtx.nrm which we're trying to calculate
#endif

   if(vtx.dup())REPA(vtx)vtx.nrm(i)=vtx.nrm(vtx.dup(i));
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::setTangents()
{
   if(C Vec  *pos=vtx.pos ())
   if(C Vec2 *tex=vtx.tex0())
   {
      include(VTX_TAN); ZeroN(vtx.tan(), vtxs());

      if(VecI *_tri=tri.ind())REPA(tri)
      {
         VecI f =*_tri++;
         Vec2 t0=tex[f.x],
              ta=tex[f.y]-t0,
              tb=tex[f.z]-t0;
         Flt  u, v;
         // u*ta   + v*tb   = Vec2(1, 0)
         // u*ta.x + v*tb.x = 1
         // u*ta.y + v*tb.y = 0
         if(Solve(ta.x, ta.y, tb.x, tb.y, 1, 0, u, v)==1)
         {
          C Vec &p0 =pos[f.x],
                &p1 =pos[f.y],
                &p2 =pos[f.z],
                 p01=p1-p0,
                 p02=p2-p0;
            Vec  tan=p01*u + p02*v;
              // Flt Tri::area()C {return 0.5f*Cross(p[1]-p[0], p[2]-p[0]).length();}
                 tan.setLength(Cross(p01, p02).length());
            vtx.tan(f.x)+=tan;
            vtx.tan(f.y)+=tan;
            vtx.tan(f.z)+=tan;
         }
      }
      if(VecI4 *_quad=quad.ind())REPA(quad)
      {
         VecI4 f =*_quad++;
         Vec2  t0=tex[f.x],
               ta=tex[f.y]-t0,
               tb=tex[f.w]-t0;
         Flt u, v;
         if(Solve(ta.x, ta.y, tb.x, tb.y, 1, 0, u, v)==1)
         {
          C Vec &p0 =pos[f.x],
                &p1 =pos[f.y],
                &p2 =pos[f.z],
                &p3 =pos[f.w],
                 p01=p1-p0,
                 p03=p3-p0,
                 p12=p2-p1,
                 p13=p3-p1;
             Vec tan=p01*u + p03*v;
              // Flt Quad::area()C {return 0.5f*(Cross(p[1]-p[0], p[3]-p[0]).length()+Cross(p[2]-p[1], p[3]-p[1]).length());}
                 tan.setLength(Cross(p01, p03).length() + Cross(p12, p13).length());
            vtx.tan(f.x)+=tan;
            vtx.tan(f.y)+=tan;
            vtx.tan(f.z)+=tan;
            vtx.tan(f.w)+=tan;
         }
      }
      REPA(vtx)
      {
         Vec &tan=vtx.tan(i); if(!tan.normalize()) // !! valid non-zero tangent must be set because otherwise triangles can become black !!
         {
            if(vtx.nrm())tan=PerpN(vtx.nrm(i));
            else         tan.set(1, 0, 0);
         }
      }
   }
   return T;
}
MeshBase& MeshBase::setBinormals()
{
   if(C Vec  *pos=vtx.pos ())
   if(C Vec2 *tex=vtx.tex0())
   {
      include(VTX_BIN); ZeroN(vtx.bin(), vtxs());

      if(VecI *_tri=tri.ind())REPA(tri)
      {
         VecI f =*_tri++;
         Vec2 t0=tex[f.x],
              ta=tex[f.y]-t0,
              tb=tex[f.z]-t0;
         Flt  u, v;
         // u*ta   + v*tb   = Vec2(0, 1)
         // u*ta.x + v*tb.x = 0
         // u*ta.y + v*tb.y = 1
         if(Solve(ta.x, ta.y, tb.x, tb.y, 0, 1, u, v)==1)
         {
          C Vec &p0 =pos[f.x],
                &p1 =pos[f.y],
                &p2 =pos[f.z],
                 p01=p1-p0,
                 p02=p2-p0;
            Vec  bin=p01*u + p02*v;
              // Flt Tri::area()C {return 0.5f*Cross(p[1]-p[0], p[2]-p[0]).length();}
                 bin.setLength(Cross(p01, p02).length());
            vtx.bin(f.x)+=bin;
            vtx.bin(f.y)+=bin;
            vtx.bin(f.z)+=bin;
         }
      }
      if(VecI4 *_quad=quad.ind())REPA(quad)
      {
         VecI4 f =*_quad++;
         Vec2  t0=tex[f.x],
               ta=tex[f.y]-t0,
               tb=tex[f.w]-t0;
         Flt u, v;
         if(Solve(ta.x, ta.y, tb.x, tb.y, 0, 1, u, v)==1)
         {
          C Vec &p0 =pos[f.x],
                &p1 =pos[f.y],
                &p2 =pos[f.z],
                &p3 =pos[f.w],
                 p01=p1-p0,
                 p03=p3-p0,
                 p12=p2-p1,
                 p13=p3-p1;
             Vec bin=p01*u + p03*v;
              // Flt Quad::area()C {return 0.5f*(Cross(p[1]-p[0], p[3]-p[0]).length()+Cross(p[2]-p[1], p[3]-p[1]).length());}
                 bin.setLength(Cross(p01, p03).length() + Cross(p12, p13).length());
            vtx.bin(f.x)+=bin;
            vtx.bin(f.y)+=bin;
            vtx.bin(f.z)+=bin;
            vtx.bin(f.w)+=bin;
         }
      }
      REPA(vtx)
      {
         Vec &bin=vtx.bin(i); if(!bin.normalize()) // !! valid non-zero binormal must be set because otherwise triangles can become black !!
         {
            if(vtx.nrm() && vtx.tan())bin=Cross(vtx.nrm(i), vtx.tan(i));else
            if(vtx.nrm()             )bin=PerpN(vtx.nrm(i)            );else
            if(             vtx.tan())bin=PerpN(            vtx.tan(i));else
                                      bin.set(0, -1, 0);
         }
      }
   }
   return T;
}
MeshBase& MeshBase::setAutoTanBin()
{
   if(vtx.nrm())
   {
      setTangents(); if(vtx.tan())
      {
         setBinormals(); if(vtx.bin())
         {
            REPA(vtx)
            {
               Vec bin=Cross(vtx.nrm(i), vtx.tan(i));
               if(Dot(bin, vtx.bin(i))<0.99f)return T; // if binormals are different, then it means that binormal is necessary, so keep it and return without deleting it
            }
            exclude(VTX_BIN); // binormal unnecessary
         }
      }
   }else exclude(VTX_TAN_BIN);
   return T;
}
/******************************************************************************
MeshBase& setSolid      (                            ); // fill 'edge.flag' EDGE_FLAG with solid info

MeshBase& MeshBase::setSolid()
{
   if(!edge.adj_face)setAdjacencies();
   if(!edge.flag    )AllocZero     (edge.flag, edge.num);
   Byte  *flag=edge.flag;
   VecI2 *adj =edge.adj_face;
   REP(edge.num)
   {
                       *flag&=~ETQ_LR;
      if(adj->c[0]!=-1)*flag|= ETQ_R;
      if(adj->c[1]!=-1)*flag|= ETQ_L;
      flag++;
      adj ++;
   }
   return T;
}
/******************************************************************************
MeshBase& setID         (Int  solid, Int not_solid=-1                    ); // set face ID values (edge.id, tri.id, quad.id) to 'solid' / 'not_solid'
MeshBase& MeshBase::setID(Int solid, Int not_solid)
{
   include(ID_ALL);
   ;
   VecI2 *eid =edge.id,
         *tid =tri .id,
         *qid =quad.id;
   if(C Byte *flag=edge.flag)
   {
      REPA(edge)
      {
         (eid++)->set(((*flag)&ETQ_R) ? solid : not_solid,
                      ((*flag)&ETQ_L) ? solid : not_solid);
         flag++;
      }
   }else
   {
      REPA(edge)eid[i].set(not_solid);
   }
   REPA(tri )tid[i].set(solid, not_solid);
   REPA(quad)qid[i].set(solid, not_solid);
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::setVtxDup2D(UInt flag, Flt pos_eps, Flt nrm_cos)
{
   include(VTX_DUP); // vtx dup doesn't need to be initialized here, because the algorithm works in a way that only processed vertexes are tested
   flag&=(T.flag()&(VTX_NRM_TAN_BIN|VTX_HLP|VTX_TEX_ALL|VTX_SIZE|VTX_SKIN|VTX_MATERIAL|VTX_COLOR|VTX_FLAG)); // only these are tested

   // link rect->vtx
   Rects rects   (Rect(T)    , vtxs());
   Index rect_vtx(rects.num(), vtxs());
   REPA(vtx)rect_vtx.elmGroup(rects.index(vtx.pos(i).xy), i); rect_vtx.set();

   // get duplicates
   Int xs=rects.cells.x,
       ys=rects.cells.y;

   FREPD(y, ys)
   FREPD(x, xs)
   {
      Int         rect_tests=0;
      IndexGroup *rect_test[2+3], *rect_cur=&rect_vtx.group[x+y*xs];
           rect_test[rect_tests++]=rect_cur  ;
      if(x)rect_test[rect_tests++]=rect_cur-1;
      if(y)
      {
         if(x<xs-1)rect_test[rect_tests++]=rect_cur+1-xs;
                   rect_test[rect_tests++]=rect_cur  -xs;
         if(x     )rect_test[rect_tests++]=rect_cur-1-xs;
      }

      FREPA(*rect_cur)
      {
         Int   num_cur=(*rect_cur)[i];
         Vec2 &pos_cur=vtx.pos(num_cur).xy;
         REPD(c, rect_tests)
         {
            IndexGroup *rt=rect_test[c];
            REPD(j, (rect_cur==rt) ? i : rt->num)
            {
               Int num_test=(*rt)[j]; if(vtx.dup(num_test)==num_test && Equal(pos_cur, vtx.pos(num_test).xy, pos_eps))
               {
                  if(flag)
                  {
                     if(flag&VTX_NRM      &&  Dot  (vtx.nrm     (num_cur), vtx.nrm     (num_test))<nrm_cos    )continue;
                     if(flag&VTX_TAN      &&  Dot  (vtx.tan     (num_cur), vtx.tan     (num_test))<EPS_TAN_COS)continue;
                     if(flag&VTX_BIN      &&  Dot  (vtx.bin     (num_cur), vtx.bin     (num_test))<EPS_TAN_COS)continue;
                     if(flag&VTX_HLP      && !Equal(vtx.hlp     (num_cur), vtx.hlp     (num_test))            )continue;
                     if(flag&VTX_TEX0     && !Equal(vtx.tex0    (num_cur), vtx.tex0    (num_test))            )continue;
                     if(flag&VTX_TEX1     && !Equal(vtx.tex1    (num_cur), vtx.tex1    (num_test))            )continue;
                     if(flag&VTX_TEX2     && !Equal(vtx.tex2    (num_cur), vtx.tex2    (num_test))            )continue;
                     if(flag&VTX_SIZE     && !Equal(vtx.size    (num_cur), vtx.size    (num_test))            )continue;
                     if(flag&VTX_BLEND    &&        vtx.blend   (num_cur)!=vtx.blend   (num_test)             )continue;
                     if(flag&VTX_MATRIX   &&        vtx.matrix  (num_cur)!=vtx.matrix  (num_test)             )continue;
                     if(flag&VTX_MATERIAL &&        vtx.material(num_cur)!=vtx.material(num_test)             )continue;
                     if(flag&VTX_COLOR    &&        vtx.color   (num_cur)!=vtx.color   (num_test)             )continue;
                     if(flag&VTX_FLAG     &&        vtx.flag    (num_cur)!=vtx.flag    (num_test)             )continue;
                  }
                  vtx.dup(num_cur)=num_test; goto next;
               }
            }
         }
         vtx.dup(num_cur)=num_cur;
         next:;
      }
   }
   return T;
}
MeshBase& MeshBase::setVtxDup  (UInt flag, Flt pos_eps, Flt nrm_cos                                    ) {return setVtxDupEx(flag, pos_eps, nrm_cos);}
MeshBase& MeshBase::setVtxDupEx(UInt flag, Flt pos_eps, Flt nrm_cos, Bool smooth_groups_in_vtx_material)
{
   include(VTX_DUP); // vtx dup doesn't need to be initialized here, because the algorithm works in a way that only processed vertexes are tested
   UInt t_flag=T.flag();
   flag                         &=(t_flag&(VTX_NRM_TAN_BIN|VTX_HLP|VTX_TEX_ALL|VTX_SIZE|VTX_SKIN|VTX_MATERIAL|VTX_COLOR|VTX_FLAG)); // only these are tested
   smooth_groups_in_vtx_material&=FlagTest(t_flag, VTX_MATERIAL);

   // link box->vtx
   Boxes boxes  (Box(T)     , vtxs());
   Index box_vtx(boxes.num(), vtxs());
   REPA(vtx)box_vtx.elmGroup(boxes.index(vtx.pos(i)), i); box_vtx.set();

   // get duplicates
   Int xs=boxes.cells.x,
       ys=boxes.cells.y, xys=xs*ys;

   FREPD(z, boxes.cells.z)
   FREPD(y, boxes.cells.y)
   FREPD(x, boxes.cells.x)
   {
      Int         box_tests=0;
      IndexGroup *box_test[2+3+9], *box_cur=&box_vtx.group[x+y*xs+z*xys];
           box_test[box_tests++]=box_cur  ;
      if(x)box_test[box_tests++]=box_cur-1;
      if(y)
      {
         if(x<xs-1)box_test[box_tests++]=box_cur+1-xs;
                   box_test[box_tests++]=box_cur  -xs;
         if(     x)box_test[box_tests++]=box_cur-1-xs;
      }
      if(z)
      {
         if(y<ys-1)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1+xs-xys;
                      box_test[box_tests++]=box_cur  +xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1+xs-xys;
         }
            if(x<xs-1)box_test[box_tests++]=box_cur+1   -xys;
                      box_test[box_tests++]=box_cur     -xys;
            if(     x)box_test[box_tests++]=box_cur-1   -xys;
         if(y)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1-xs-xys;
                      box_test[box_tests++]=box_cur  -xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1-xs-xys;
         }
      }

      FREPA(*box_cur)
      {
         Int  num_cur=(*box_cur)[i];
         Vec &pos_cur=vtx.pos(num_cur);
         REPD(c, box_tests)
         {
            IndexGroup *bt=box_test[c];
            REPD(j, (box_cur==bt) ? i : bt->num)
            {
               Int num_test=(*bt)[j]; if(vtx.dup(num_test)==num_test && Equal(pos_cur, vtx.pos(num_test), pos_eps))
               {
                  if(flag)
                  {
                     if(flag&VTX_NRM      &&  Dot  (vtx.nrm     (num_cur), vtx.nrm     (num_test))<nrm_cos    )continue;
                     if(flag&VTX_TAN      &&  Dot  (vtx.tan     (num_cur), vtx.tan     (num_test))<EPS_TAN_COS)continue;
                     if(flag&VTX_BIN      &&  Dot  (vtx.bin     (num_cur), vtx.bin     (num_test))<EPS_TAN_COS)continue;
                     if(flag&VTX_HLP      && !Equal(vtx.hlp     (num_cur), vtx.hlp     (num_test))            )continue;
                     if(flag&VTX_TEX0     && !Equal(vtx.tex0    (num_cur), vtx.tex0    (num_test))            )continue;
                     if(flag&VTX_TEX1     && !Equal(vtx.tex1    (num_cur), vtx.tex1    (num_test))            )continue;
                     if(flag&VTX_TEX2     && !Equal(vtx.tex2    (num_cur), vtx.tex2    (num_test))            )continue;
                     if(flag&VTX_SIZE     && !Equal(vtx.size    (num_cur), vtx.size    (num_test))            )continue;
                     if(flag&VTX_BLEND    &&        vtx.blend   (num_cur)!=vtx.blend   (num_test)             )continue;
                     if(flag&VTX_MATRIX   &&        vtx.matrix  (num_cur)!=vtx.matrix  (num_test)             )continue;
                     if(flag&VTX_MATERIAL &&        vtx.material(num_cur)!=vtx.material(num_test)             )continue;
                     if(flag&VTX_COLOR    &&        vtx.color   (num_cur)!=vtx.color   (num_test)             )continue;
                     if(flag&VTX_FLAG     &&        vtx.flag    (num_cur)!=vtx.flag    (num_test)             )continue;
                  }
                  if(smooth_groups_in_vtx_material)if(!(vtx.material(num_cur).u&vtx.material(num_test).u))continue; // if there are no shared smooth groups
                  vtx.dup(num_cur)=num_test; goto next;
               }
            }
         }
         vtx.dup(num_cur)=num_cur;
         next:;
      }
   }
   return T;
}
/******************************************************************************/
struct EdgeAdj
{
   VecI2 ind, adj_face;

   EdgeAdj& set(Int p0, Int p1, Int f0, Int f1) {ind.set(p0, p1); adj_face.set(f0, f1); return T;}
};
struct Adj
{
   Int face,           // face index
       vtxi,           // index of vertex place in the face
       face_extra_vtx; // index of vertex which is loose

   void set(Int face, Int vtxi, Int face_extra_vtx) {T.face=face; T.vtxi=vtxi; T.face_extra_vtx=face_extra_vtx;}
};
MeshBase& MeshBase::setAdjacencies(Bool faces, Bool edges)
{
   if(faces || edges)
   {
      include((faces ? FACE_ADJ_FACE : 0)|(edges ? FACE_ADJ_EDGE : 0));
      if(faces)
      {
         SetMemN(tri .adjFace(), 0xFF, tris ());
         SetMemN(quad.adjFace(), 0xFF, quads());
      }
      if(edges)
      {
         SetMemN(tri .adjEdge(), 0xFF, tris ());
         SetMemN(quad.adjEdge(), 0xFF, quads());
      }

      // link vtx->face
      Index vtx_face; linkVtxFace(vtx_face);

      // add double sided edges
      Memb<EdgeAdj> _edge(1024);
   #if 1 // new version, that generates duplicates of edges
      REPAD(f0, tri) // for each triangle
      {
         VecI f0i=tri.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 3) // for each triangle vertex
         {
            Int f0v0=f0i.c[ f0vi     ],
                f0v1=f0i.c[(f0vi+1)%3];
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig) // for each face that the vertex belongs to
            {
               Int f1=ig[vfi]; VecI4 f1i; Int f1vtxs;
               if( f1&SIGN_BIT) // quad
               {
                  f1i=quad.ind(f1^SIGN_BIT); f1i.remap(vtx.dup()); f1vtxs=4;
               }else
               {
                  if(f1>=f0)continue; // if the other triangle has a greater index (or is the same one) then skip, because we will check this again later for the other triangle
                  f1i.xyz=tri.ind(f1); f1i.xyz.remap(vtx.dup()); f1vtxs=3;
               }
               REPD(f1vi, f1vtxs) // for each face vertex
                  if(f0v1==f1i.c[ f1vi          ]  // here f0v1==f1v0 (order of vertexes is swapped)
                  && f0v0==f1i.c[(f1vi+1)%f1vtxs]) // here f0v0==f1v1
               {
                  if(faces){tri.adjFace(f0).c[f0vi]=f1          ; ((f1&SIGN_BIT) ? quad.adjFace(f1^SIGN_BIT).c[f1vi] : tri.adjFace(f1).c[f1vi])=f0;}
                  if(edges){tri.adjEdge(f0).c[f0vi]=_edge.elms(); ((f1&SIGN_BIT) ? quad.adjEdge(f1^SIGN_BIT).c[f1vi] : tri.adjEdge(f1).c[f1vi])=_edge.elms(); _edge.New().set(f0v0, f0v1, f0, f1);}
                  break;
               }
            }
         }
      }

      REPAD(f0, quad) // for each quad
      {
         VecI4 f0i=quad.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 4) // for each quad vertex
         {
            Int f0v0=f0i.c[ f0vi     ],
                f0v1=f0i.c[(f0vi+1)%4];
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig) // for each face that the vertex belongs to
            {
               // in this case we will process only quads, as we've already processed triangle-quad pairs above
               Int f1=ig[vfi];
               if( f1&SIGN_BIT) // quad
               {
                  f1^=SIGN_BIT;
                  if(f1<f0) // here process only quads that have index smaller than 'f0', because we will check this again later for the other quad
                  {
                     VecI4 f1i=quad.ind(f1); f1i.remap(vtx.dup());
                     REPD(f1vi, 4) // for each quad vertex
                        if(f0v1==f1i.c[ f1vi     ]  // here f0v1==f1v0 (order of vertexes is swapped)
                        && f0v0==f1i.c[(f1vi+1)%4]) // here f0v0==f1v1
                     {
                        if(faces){quad.adjFace(f0).c[f0vi]=f1^SIGN_BIT ; quad.adjFace(f1).c[f1vi]=f0^SIGN_BIT;}
                        if(edges){quad.adjEdge(f0).c[f0vi]=_edge.elms(); quad.adjEdge(f1).c[f1vi]=_edge.elms(); _edge.New().set(f0v0, f0v1, f0^SIGN_BIT, f1^SIGN_BIT);}
                        break;
                     }
                  }
               }
            }
         }
      }
   #else
      this version may crash the 'DirectX::OptimizeFaces'
      Memt<Adj> adj;
      REPAD(f0, tri) // for each triangle
      {
         VecI f0i=tri.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 3) // for each triangle vertexes
         {
            Int f0v0=f0i.c[ f0vi     ],
                f0v1=f0i.c[(f0vi+1)%3];
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig) // for each face that the vertex belongs to
            {
               Int f1=ig[vfi];
               if( f1&SIGN_BIT) // quad
               {
                  VecI4 f1i=quad.ind(f1^SIGN_BIT); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0)adj.New().set(f1, 0, f1i.c[2]);else
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0)adj.New().set(f1, 1, f1i.c[3]);else
                  if(f1i.c[2]==f0v1 && f1i.c[3]==f0v0)adj.New().set(f1, 2, f1i.c[0]);else
                  if(f1i.c[3]==f0v1 && f1i.c[0]==f0v0)adj.New().set(f1, 3, f1i.c[1]);
               }else
               {
                  VecI f1i=tri.ind(f1); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0)adj.New().set(f1, 0, f1i.c[2]);else
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0)adj.New().set(f1, 1, f1i.c[0]);else
                  if(f1i.c[2]==f0v1 && f1i.c[0]==f0v0)adj.New().set(f1, 2, f1i.c[1]);
               }
            }
            if(adj.elms())
            {
               if(adj.elms()>1 && vtx.pos()) // if the edge links many faces (for example 3 faces like "T")
               {
                  Matrix m; m.setPosDir(vtx.pos(f0v0), !(vtx.pos(f0v1)-vtx.pos(f0v0)));
                  Flt    angle, a0=AngleFast(vtx.pos(f0i.c[(f0vi+2)%3]), m);
                  Int    max=-1; REPA(adj)
                  {
                     Flt a=AngleFast(vtx.pos(adj[i].face_extra_vtx), m);
                     if( a<a0)a+=PI2;
                     if(max<0 || a>angle){max=i; angle=a;}
                  }
                  adj[0]=adj[max];
               }
               Adj &a   =adj[0];
               Int  f1  =a.face,
                    f1vi=a.vtxi;
               if(  f1&SIGN_BIT || f1>f0)
               {
                  if(faces){tri.adjFace(f0).c[f0vi]=f1          ; ((f1&SIGN_BIT) ? quad.adjFace(f1^SIGN_BIT).c[f1vi] : tri.adjFace(f1).c[f1vi])=f0;}
                  if(edges){tri.adjEdge(f0).c[f0vi]=_edge.elms(); ((f1&SIGN_BIT) ? quad.adjEdge(f1^SIGN_BIT).c[f1vi] : tri.adjEdge(f1).c[f1vi])=_edge.elms(); _edge.New().set(f0v0, f0v1, f0, f1);}
               }
               adj.clear();
            }
         }
      }

      REPAD(f0, quad) // quads
      {
         VecI4 f0i=quad.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 4)
         {
            Int f0v0=f0i.c[ f0vi     ],
                f0v1=f0i.c[(f0vi+1)%4];
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig)
            {
               Int f1=ig[vfi];
               if( f1&SIGN_BIT) // quad
               {
                  VecI4 f1i=quad.ind(f1^SIGN_BIT); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0)adj.New().set(f1, 0, f1i.c[2]);else
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0)adj.New().set(f1, 1, f1i.c[3]);else
                  if(f1i.c[2]==f0v1 && f1i.c[3]==f0v0)adj.New().set(f1, 2, f1i.c[0]);else
                  if(f1i.c[3]==f0v1 && f1i.c[0]==f0v0)adj.New().set(f1, 3, f1i.c[1]);
               }else
               {
                  VecI f1i=tri.ind(f1); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0)adj.New().set(f1, 0, f1i.c[2]);else
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0)adj.New().set(f1, 1, f1i.c[0]);else
                  if(f1i.c[2]==f0v1 && f1i.c[0]==f0v0)adj.New().set(f1, 2, f1i.c[1]);
               }
            }
            if(adj.elms())
            {
               if(adj.elms()>1 && vtx.pos()) // if the edge links many faces (for example 3 faces like "T")
               {
                  Matrix m; m.setPosDir(vtx.pos(f0v0), !(vtx.pos(f0v1)-vtx.pos(f0v0)));
                  Flt    angle, a0=AngleFast(vtx.pos(f0i.c[(f0vi+2)%4]), m);
                  Int    max=-1; REPA(adj)
                  {
                     Flt a=AngleFast(vtx.pos(adj[i].face_extra_vtx), m);
                     if(a<a0)a+=PI2;
                     if(max<0 || a>angle){max=i; angle=a;}
                  }
                  adj[0]=adj[max];
               }
               Adj &a   =adj[0];
               Int  f1  =a.face,
                    f1vi=a.vtxi;
               if((f1&SIGN_BIT) && (f1^SIGN_BIT)>f0)
               {
                  if(faces){quad.adjFace(f0).c[f0vi]=f1          ; quad.adjFace(f1^SIGN_BIT).c[f1vi]=f0^SIGN_BIT;}
                  if(edges){quad.adjEdge(f0).c[f0vi]=_edge.elms(); quad.adjEdge(f1^SIGN_BIT).c[f1vi]=_edge.elms(); _edge.New().set(f0v0, f0v1, f0^SIGN_BIT, f1);}
               }
               adj.clear();
            }
         }
      }
   #endif

      if(edges)
      {
         // add one sided edges
         FREPA(tri)
         {
            Int *p=tri.adjEdge(i).c;
            REPD(j, 3)if(p[j]<0)
            {
               Int *v=tri.ind(i).c;
               p[j]=_edge.elms();
                    _edge.New().set(v[j], v[(j+1)%3], i, -1).ind.remap(vtx.dup());
            }
         }
         FREPA(quad)
         {
            Int *p=quad.adjEdge(i).c;
            REPD(j, 4)if(p[j]<0)
            {
               Int *v=quad.ind(i).c;
               p[j]=_edge.elms();
                    _edge.New().set(v[j], v[(j+1)%4], i^SIGN_BIT, -1).ind.remap(vtx.dup());
            }
         }

         // set edges
         exclude(EDGE_ALL); edge._elms=_edge.elms();
         include(EDGE_IND|EDGE_ADJ_FACE);
         FREPA(edge)
         {
            edge.ind    (i)=_edge[i].ind;
            edge.adjFace(i)=_edge[i].adj_face;
         }
      }
   }
   return T;
}
/******************************************************************************/
struct MeshAO
{
   static Flt Full   (Flt x) {return         0  ;}
   static Flt Quartic(Flt x) {return Sqr(Sqr(x));}
   static Flt Square (Flt x) {return     Sqr(x) ;}
   static Flt Linear (Flt x) {return         x  ;}
   static Flt LinearR(Flt x) {return   2-2/(x+1);}
   static Flt SquareR(Flt x) {return  1-Sqr(1-x);}

 C Vec      *pos, *nrm;
   Color    *col;
   Memt<Vec> ray_dir;
   Byte      max;
   Flt       mul, add, ray_length, pos_eps;
   PhysPart  body;
   Flt     (*func)(Flt x);

   INLINE Bool sweep(C Vec &pos, C Vec &move, Flt *frac)
   {
   #if 0
      return Sweep(pos, move, *mesh, null, frac, null, null, true, two_sided);
   #else
      PhysHitBasic hit;
      if(body.ray(pos, move, null, frac ? &hit : null, true))
      {
         if(frac)*frac=hit.frac;
         return true;
      }
      return false;
   #endif
   }
   static void Set(IntPtr elm_index, MeshAO &ao, Int thread_index) {ao.set(elm_index);}
          void set(Int    vtx)
   {
    C Vec &pos=T.pos[vtx], &nrm=T.nrm[vtx];
      Flt light=0;
      REPA(ray_dir)
      {
       C Vec &ray=ray_dir[i];
         Flt d=Dot(nrm, ray); if(d>0)
         {
            Flt frac; if(sweep(pos+ray*pos_eps, ray, &frac))light+=func(frac)*d;else light+=d;
         }
      }
      Color &c=col[vtx]; c.r=c.g=c.b=Mid(RoundPos(light*mul+add), max, 255); // keep alpha
   }
   void process(Int vtxs, Threads *threads)
   {
      if(threads)threads->process1(vtxs, Set, T);else REP(vtxs)set(i);
   }
   MeshAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps, Int rays, MESH_AO_FUNC func)
   {
      switch(func)
      {
         default            :
         case MAF_FULL      : T.func=Full   ; break;
         case MAF_QUARTIC   : T.func=Quartic; break;
         case MAF_SQUARE    : T.func=Square ; break;
         case MAF_LINEAR    : T.func=Linear ; break;
         case MAF_LINEAR_REV: T.func=LinearR; break;
         case MAF_SQUARE_REV: T.func=SquareR; break;
      }
                      T.ray_length =Max(0, ray_length);
                      T.pos_eps    =Mid(pos_eps, 0.0f, T.ray_length*0.5f);
                      T.ray_length-=T.pos_eps;
      if(T.ray_length)T.pos_eps   /=T.ray_length; // because we pre-multiply 'ray_dir' with 'ray_length' then we need to adjust 'pos_eps'
                      T.max        =FltToByte(1-max);

      Int rays_res=Max(1, Round(Sqrt(rays/6.0f))); // rays resolution in 2D, in one of 6 cube faces
      rays=Sqr(rays_res)*6; // total number of rays
      ray_dir.setNum(rays);
      Int r=0, r_pos=0;
      Flt ray_step=PI_2/rays_res;
      Vec ray_base; ray_base.z=1;
      Dbl ray_sum=0;
      REPD(x, rays_res)
      {
         ray_base.x=Tan(x*ray_step-PI_4);
         REPD(y, rays_res)
         {
            ray_base.y=Tan(y*ray_step-PI_4);
            Vec n=ray_base; n.normalize(); // normalized
            REPD(f, 6) // 6 cube faces
            {
               Vec ray; switch(f)
               {
                  case 0: ray.set( n.x,  n.y,  n.z); break;
                  case 1: ray.set(-n.z,  n.y,  n.x); break;
                  case 2: ray.set(-n.x, -n.y, -n.z); break;
                  case 3: ray.set( n.z, -n.y, -n.x); break;
                  case 4: ray.set( n.x,  n.z, -n.y); break;
                  case 5: ray.set(-n.x, -n.z,  n.y); break;
               }
               ray*=T.ray_length; // pre-multiply so we don't have to do this for each vertex*ray
               ray_dir[r++]=ray;
               if(ray.x>0){r_pos++; ray_sum+=ray.x;} // Dot(ray, Vec(1, 0, 0))
            }
         }
      }

      add=255*(1-strength + strength*bias);
      mul=255*   strength/ray_sum;
    //mul=255*   strength/r_pos  ; // this is for light independent on the Dot product
   }
};
/******************************************************************************/
MeshBase& MeshBase::setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps, Int rays, MESH_AO_FUNC func, Threads *threads)
{
   if(vtx.pos() && vtx.nrm())
   {
      MeshAO ao(strength, bias, max, ray_length, pos_eps, rays, func);
      if(ao.body.createMeshTry(T))
      {
         if(!vtx.color()){include(VTX_COLOR); REPA(vtx)vtx.color(i)=WHITE;} // setup colors, especially needed for Alpha which is kept and not overwritten, but set all components in case some codes below don't get executed
         if(ao.pos=vtx.pos  ())
         if(ao.nrm=vtx.nrm  ())
         if(ao.col=vtx.color())ao.process(vtxs(), threads);
      }
   }
   return T;
}
MeshLod& MeshLod::setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps, Int rays, MESH_AO_FUNC func, Threads *threads)
{
   if(parts.elms())
   {
      MeshAO ao(strength, bias, max, ray_length, pos_eps, rays, func);
      if(ao.body.createMeshTry(T, false, true))REPA(T)
      {
         MeshPart &part=parts[i];
         MeshBase &base=part.base;
         if(!base.vtx.color()){base.include(VTX_COLOR); REPA(base.vtx)base.vtx.color(i)=WHITE;} // setup colors, especially needed for Alpha which is kept and not overwritten, but set all components in case some codes below don't get executed
         if(ao.pos=base.vtx.pos  ())
         if(ao.nrm=base.vtx.nrm  ())
         if(ao.col=base.vtx.color())ao.process(base.vtxs(), threads);
      }
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
