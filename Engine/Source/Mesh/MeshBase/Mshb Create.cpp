/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
MeshBase& MeshBase::createPlane(Int x, Int y, UInt flag)
{
   MAX(x, 2);
   MAX(y, 2);
   create(x*y, 0, 0, (x-1)*(y-1), flag&(VTX_NRM_TAN_BIN|VTX_TEX0));

   // face
   if(VecI4 *quad=T.quad.ind())
   FREPD(sy, y-1)
   FREPD(sx, x-1)
   {
      Int p=sx+sy*x;
      (quad++)->set(p+x, p+1+x, p+1, p);
   }

   // vtx
   if(Vec *pos=vtx.pos())FREPD(sy, y)
   {
      Flt fy=sy/Flt(y-1);
      FREPD(sx, x)(pos++)->set(sx/Flt(x-1), fy, 0);
   }
   if(Vec *nrm=vtx.nrm())REPA(vtx)(nrm++)->set(0,  0, -1);
   if(Vec *tan=vtx.tan())REPA(vtx)(tan++)->set(1,  0,  0);
   if(Vec *bin=vtx.bin())REPA(vtx)(bin++)->set(0, -1,  0);
   if(vtx.tex0())texMap();

   return T;
}
MeshBase& MeshBase::createGrid(Int x, Int y, Bool fast)
{
   MAX(x, 2);
   MAX(y, 2);
   if(fast)create(x*2+(y-2)*2, x+y            , 0, 0);
   else    create(x*y        , y*(x-1)+x*(y-1), 0, 0);

   // vtx
   Vec *pos=vtx.pos();
   if(fast)
   {
      FREPD(sx, x)               (pos++)->set(   sx/Flt(x-1), 1, 0);
      FREPD(sx, x)               (pos++)->set(   sx/Flt(x-1), 0, 0);
      for(Int sy=1; sy<y-1; sy++)(pos++)->set(0, sy/Flt(y-1), 0   );
      for(Int sy=1; sy<y-1; sy++)(pos++)->set(1, sy/Flt(y-1), 0   );
   }else
   FREPD(sy, y)
   {
      Flt fy=sy/Flt(y-1);
      FREPD(sx, x)(pos++)->set(sx/Flt(x-1), fy, 0);
   }

   // edge
   VecI2 *edge=T.edge.ind();
   if(fast)
   {
      FREPD(sx, x  )(edge++)->set(    sx,       sx+x);
                    (edge++)->set(     x,      x+x-1);
      FREPD(sy, y-2)(edge++)->set(x+x+sy, x+x+sy+y-2);
                    (edge++)->set(     0,        x-1);
   }else
   {
      FREPD(sy, y)FREPD(sx, x-1)(edge++)->set(sx+sy*x, sx+1+(sy  )*x);
      FREPD(sx, x)FREPD(sy, y-1)(edge++)->set(sx+sy*x, sx  +(sy+1)*x);
   }
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::createFast(C Box &box)
{
   create(8, 0, 0, 6);

   // vtx
      FREP(8)vtx.pos(i).set(( i    &1) ? box.max.x : box.min.x,
                            ((i>>1)&1) ? box.max.y : box.min.y,
                            ((i>>2)&1) ? box.max.z : box.min.z);
   // face
      // x
      quad.ind(0).set(1|2|0, 1|2|4, 1|0|4, 1|0|0);
      quad.ind(1).set(0|0|0, 0|0|4, 0|2|4, 0|2|0);
      // y
      quad.ind(2).set(2|4|0, 2|4|1, 2|0|1, 2|0|0);
      quad.ind(3).set(0|0|0, 0|0|1, 0|4|1, 0|4|0);
      // z
      quad.ind(4).set(4|1|2, 4|0|2, 4|0|0, 4|1|0);
      quad.ind(5).set(0|1|0, 0|0|0, 0|0|2, 0|1|2);

   return T;
}
MeshBase& MeshBase::createBox(C Matrix &matrix, UInt flag, Int resolution)
{
   Matrix m; MeshBase mesh[6]; REPA(mesh)
   {
      m.setOrient(DIR_ENUM(i));
      m.anchor(0.5f);
      m*=matrix;
      mesh[i].createPlane(resolution, resolution, flag).transform(m);
   }
   create(mesh, Elms(mesh));
   if(!(flag&(VTX_TAN_BIN|VTX_TEX0)))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   return T;
}
MeshBase& MeshBase::create(C  Box & box, UInt flag, Int resolution) {return createBox(Matrix(     box.size(),      box.min)            , flag, resolution);}
MeshBase& MeshBase::create(C OBox &obox, UInt flag, Int resolution) {return createBox(Matrix(obox.box.size(), obox.box.min)*obox.matrix, flag, resolution);}
/******************************************************************************/
MeshBase& MeshBase::create(C Ball &ball, UInt flag, Int resolution)
{
   if(resolution<0)resolution=7;else MAX(resolution, 2); // default size else min size
   Int face_vtxs=resolution*resolution;
   create(face_vtxs*6, 0, 0, (resolution-1)*(resolution-1)*6, flag&VTX_TEX0);
   Vec   *pos =  vtx .pos ();
   Vec2  *tex =  vtx .tex0();
   VecI4 *quad=T.quad.ind ();

   // vtx
   FREPD(y, resolution)
   {
      Flt fy=y/Flt(resolution-1), sy=-Tan(fy*PI_2-PI_4);
      FREPD(x, resolution)
      {
         Flt fx=x/Flt(resolution-1), sx=-Tan(fx*PI_2-PI_4);
         pos[face_vtxs*0].set( sx, sy,  1 );
         pos[face_vtxs*1].set( 1 , sy, -sx);
         pos[face_vtxs*2].set(-sx, sy, -1 );
         pos[face_vtxs*3].set(-1 , sy,  sx);
         pos[face_vtxs*4].set(-sx,  1,  sy);
         pos[face_vtxs*5].set(-sx, -1, -sy);
         pos++;
         if(tex)
         {
            tex[face_vtxs*0].set(fx, fy);
            tex[face_vtxs*1].set(fx, fy);
            tex[face_vtxs*2].set(fx, fy);
            tex[face_vtxs*3].set(fx, fy);
            tex[face_vtxs*4].set(fx, fy);
            tex[face_vtxs*5].set(fx, fy);
            tex++;
         }
      }
   }

   // face
   FREPD(f, 6)
   FREPD(y, resolution-1)
   FREPD(x, resolution-1)
   {
      Int p=x + y*resolution + f*face_vtxs;
      (quad++)->set(p, p+1, p+1+resolution, p+resolution);
   }

   if(!(flag&VTX_TEX0))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   Normalize(vtx.pos(), vtxs());
   if(flag&VTX_NRM)
   {
      include(VTX_NRM);
      CopyN  (vtx.nrm(), vtx.pos(), vtxs());
   }
   transform(Matrix(ball.r, ball.pos));
   if(flag&VTX_TAN)setTangents ();
   if(flag&VTX_BIN)setBinormals();
   return T;
}
MeshBase& MeshBase::createHalf(C Ball &ball, UInt flag, Int resolution)
{
   if(resolution<0)resolution=7;else MAX(resolution, 2); // default size else min size
#if 1 // floor
   Int  res_half=Max((resolution+1)/2, 2),
#else // ceil
   Int  res_half=resolution/2+1,
#endif
       face_vtxs=resolution*resolution,
       half_vtxs=resolution*res_half;
   create(half_vtxs*4 + face_vtxs, 0, 0, (resolution-1)*(res_half-1)*4 + (resolution-1)*(resolution-1), flag&VTX_TEX0);
   Vec   *pos =  vtx .pos ();
   Vec2  *tex =  vtx .tex0();
   VecI4 *quad=T.quad.ind ();

   // vtx
   FREPD(y, res_half)
   {
      Flt fy=y/Flt((res_half-1)*2), sy=-Tan(fy*PI_2-PI_4);
      FREPD(x, resolution)
      {
         Flt fx=x/Flt(resolution-1), sx=-Tan(fx*PI_2-PI_4);
         pos[half_vtxs*0].set( sx, sy,  1 );
         pos[half_vtxs*1].set( 1 , sy, -sx);
         pos[half_vtxs*2].set(-sx, sy, -1 );
         pos[half_vtxs*3].set(-1 , sy,  sx);
         pos++;
         if(tex)
         {
            tex[half_vtxs*0].set(fx, fy);
            tex[half_vtxs*1].set(fx, fy);
            tex[half_vtxs*2].set(fx, fy);
            tex[half_vtxs*3].set(fx, fy);
            tex++;
         }
      }
   }
          pos+=resolution*res_half*3;
   if(tex)tex+=resolution*res_half*3;
   FREPD(y, resolution)
   {
      Flt fy=y/Flt(resolution-1), sy=-Tan(fy*PI_2-PI_4);
      FREPD(x, resolution)
      {
         Flt fx=x/Flt(resolution-1), sx=-Tan(fx*PI_2-PI_4);
                (pos++)->set(-sx,  1,  sy);
         if(tex)(tex++)->set( fx, fy);
      }
   }

   // face
   FREPD(f, 4)
   FREPD(y, res_half  -1)
   FREPD(x, resolution-1)
   {
      Int p=x + y*resolution + f*half_vtxs;
      (quad++)->set(p, p+1, p+1+resolution, p+resolution);
   }
   FREPD(y, resolution-1)
   FREPD(x, resolution-1)
   {
      Int p=x + y*resolution + 4*half_vtxs;
      (quad++)->set(p, p+1, p+1+resolution, p+resolution);
   }

   if(!(flag&VTX_TEX0))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   Normalize(vtx.pos(), vtxs());
   if(flag&VTX_NRM)
   {
      include(VTX_NRM);
      CopyN  (vtx.nrm(), vtx.pos(), vtxs());
   }
   transform(Matrix(ball.r, ball.pos));
   if(flag&VTX_TAN)setTangents ();
   if(flag&VTX_BIN)setBinormals();
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::create2(C Ball &ball, UInt flag, Int resolution, Int resolution2)
{
   if(resolution <0)resolution =24;else MAX(resolution , 3); // default size else min size
   if(resolution2<0)resolution2=13;else MAX(resolution2, 3); // default size else min size
   create((resolution+1)*resolution2, 0, resolution*2, resolution*(resolution2-3), flag&VTX_TEX0);

   // vertexes
   REPD(y, resolution2)
   {
      Flt cy, sy; CosSin(cy, sy, y/Flt(resolution2-1)*PI-PI_2);
      REPD(x, resolution+1)
      {
         Flt cx, sx; CosSin(cx, sx, x/Flt(resolution)*PI2);
                       vtx.pos (x+y*(resolution+1)).set(cx*cy, sy, sx*cy);
         if(vtx.tex0())vtx.tex0(x+y*(resolution+1)).set(x/Flt(resolution), y/Flt(resolution2-1));
      }
   }

   // triangles
   REPD(x, resolution)
   {
      Int i=x+             0 *(resolution+1); tri.ind(x           ).set(i+resolution+1, i+resolution+1+1, i);
          i=x+(resolution2-2)*(resolution+1); tri.ind(x+resolution).set(i+resolution+1, i+1             , i);
   }

   // quads
   REPD(y, resolution2-3)
   REPD(x, resolution)
   {
      Int i=x+(y+1)*(resolution+1); quad.ind(x+y*resolution).set(i+resolution+1, i+resolution+1+1, i+1, i);
   }

   if(!(flag&VTX_TEX0))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   Normalize(vtx.pos(), vtxs());
   if(flag&VTX_NRM)
   {
      include(VTX_NRM);
      CopyN  (vtx.nrm(), vtx.pos(), vtxs());
   }
   transform(Matrix(ball.r, ball.pos));
   if(flag&VTX_TAN)setTangents ();
   if(flag&VTX_BIN)setBinormals();
   return T;
}
/******************************************************************************/
//VecD2 p((1+Sqrt(5.0))/2, 1); p.normalize();
#define X 0.85065080835203999f // p.x
#define Y 0.52573111211913359f // p.y
static const Vec IcoVtxs[]=
{
  Vec( Y, 0,-X), Vec(-Y, 0,-X), Vec( Y, 0, X), Vec(-Y, 0, X),
  Vec( 0,-X,-Y), Vec( 0,-X, Y), Vec( 0, X,-Y), Vec( 0, X, Y),
  Vec(-X,-Y, 0), Vec( X,-Y, 0), Vec(-X, Y, 0), Vec( X, Y, 0),
};
static const VecI IcoTris[]=
{
  VecI(0, 4, 1), VecI(0,9, 4), VecI(9, 5,4), VecI( 4,5,8), VecI(4,8, 1),
  VecI(8,10, 1), VecI(8,3,10), VecI(5, 3,8), VecI( 5,2,3), VecI(2,7, 3),
  VecI(7,10, 3), VecI(7,6,10), VecI(7,11,6), VecI(11,0,6), VecI(0,1, 6),
  VecI(6, 1,10), VecI(9,0,11), VecI(9,11,2), VecI( 9,2,5), VecI(7,2,11),
};
#undef X
#undef Y
MeshBase& MeshBase::createIco(C Ball &ball, UInt flag, Int resolution)
{
   if(resolution< 0)resolution=3; // default
   if(resolution<=0) // simple
   {
      create(Elms(IcoVtxs), 0, Elms(IcoTris), 0);
      CopyN(vtx.pos(), IcoVtxs, vtxs());
      CopyN(tri.ind(), IcoTris, tris());
   }else // subdivision
   {
      resolution++;
      const Int vtxs=(resolution+1)*(resolution+2)/2, tris=resolution*resolution, sides=Elms(IcoTris);
      create(vtxs*sides, 0, tris*sides, 0);
      Int   vtx_offset=0;
      Vec  *p=vtx.pos();
      VecI *t=tri.ind();
      REP(sides)
      {
       C VecI &tri_ind=IcoTris[i];
       C Vec  &v0     =IcoVtxs[tri_ind.x],
              &v1     =IcoVtxs[tri_ind.y],
              &v2     =IcoVtxs[tri_ind.z];
         Vec   v01    =(v1-v0)/resolution,
               v02    =(v2-v0)/resolution,
               vy     =v0;
         for(Int y=0; y<=resolution; y++)
         {
            Vec vx=vy;
            Int xs=resolution-y;
            for(Int x=0; x<=xs; x++)
            {
               *p++=vx;
               vx+=v01;
               if(y!=resolution && x!=xs)
               {
                            (t++)->set(vtx_offset     , vtx_offset+1, vtx_offset+xs+1);
                  if(x+1<xs)(t++)->set(vtx_offset+xs+1, vtx_offset+1, vtx_offset+xs+2);
               }
               vtx_offset++;
            }
            vy+=v02;
         }
      }
      weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
      Normalize(vtx.pos(), T.vtxs());
   }
   if(flag&VTX_NRM)
   {
      include(VTX_NRM);
      CopyN  (vtx.nrm(), vtx.pos(), T.vtxs());
   }
   transform(Matrix(ball.r, ball.pos));
   if(flag&VTX_TAN)setTangents ();
   if(flag&VTX_BIN)setBinormals();
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::createIcoHalf(C Ball &ball, UInt flag, Int resolution)
{
   createIco(Ball(1), 0, resolution);
   Memt<Bool> is; is.setNumZero(vtxs());
   REPA(tri)
   {
    C VecI &t=tri.ind(i);
      if(vtx.pos(t.x).y>EPS
      || vtx.pos(t.y).y>EPS
      || vtx.pos(t.z).y>EPS)is[t.x]=is[t.y]=is[t.z]=true;
   }
   keepVtxs(is);
   REPA(vtx){Vec &p=vtx.pos(i); if(p.y<0){p.y=0; p.normalize();}}
   if(flag&VTX_NRM)
   {
      include(VTX_NRM);
      CopyN  (vtx.nrm(), vtx.pos(), T.vtxs());
   }
   transform(Matrix(ball.r, ball.pos));
   if(flag&VTX_TAN)setTangents ();
   if(flag&VTX_BIN)setBinormals();
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::create(C Capsule &capsule, UInt flag, Int resolution, Int resolution2)
{
   if(resolution <0)resolution =12;else MAX(resolution , 3);
   if(resolution2<0)resolution2= 5;else MAX(resolution2, 2);
   Flt r=Min(capsule.r, capsule.h*0.5f);
   create((resolution+1)*(resolution2-1)*2+2, 0, resolution*2, resolution+(resolution2-2)*resolution*2, flag&VTX_TEX0);
   REPD(y, resolution2-1)
   {
      Flt cy, sy; CosSin(cy, sy, y/Flt(resolution2-1)*PI_2);
      REPD(x, resolution+1)
      {
         Flt cx, sx; CosSin(cx, sx, x/Flt(resolution)*PI2);
         Int i0=x+ y               *(resolution+1),
             i1=x+(y+resolution2-1)*(resolution+1);
              vtx.pos(i0).set(cy*cx*r, capsule.h*0.5f-r+sy*r, cy*sx*r);
         CHS((vtx.pos(i1)=vtx.pos(i0)).y);
         if(vtx.tex0())
         {
            vtx.tex0(i0).set(x/Flt(resolution), (y/Flt(resolution2-1)*r + capsule.h*0.5f-r)/(capsule.h*0.5f)*0.5f+0.5f);
            vtx.tex0(i1).set(vtx.tex0(i0).x, 1-vtx.tex0(i0).y);
         }
      }
   }
   vtx.pos(vtxs()-2).set(0,  capsule.h*0.5f, 0);
   vtx.pos(vtxs()-1).set(0, -capsule.h*0.5f, 0);
   if(vtx.tex0())
   {
      vtx.tex0(vtxs()-2).set(0.5f, 1);
      vtx.tex0(vtxs()-1).set(0.5f, 0);
   }
   REP(resolution)
   {
      Int lo=(resolution2-1)*(resolution+1),
          o =lo-(resolution+1);
      tri .ind(i           ).set(o+ i+1, o+     i, vtxs()-2);
      tri .ind(i+resolution).set(o+lo+i, o+lo+i+1, vtxs()-1);
      quad.ind(i           ).set(lo+i+1, lo+i, i, i+1);
      REPD(y, resolution2-2)
      {
         o=y*(resolution+1);
         quad.ind(resolution+resolution* y               +i).set(o+i+1, o+i, o+(resolution+1)+i, o+(resolution+1)+i+1); o+=lo;
         quad.ind(resolution+resolution*(y+resolution2-2)+i).set(o+(resolution+1)+i+1, o+(resolution+1)+i, o+i, o+i+1);
      }
   }

   if(!(flag&VTX_TEX0))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   transform(Matrix().setPosUp(capsule.pos, capsule.up));
   if(flag&VTX_NRM)setVtxDup   ().setNormals().exclude(VTX_DUP);
   if(flag&VTX_TAN)setTangents ();
   if(flag&VTX_BIN)setBinormals();
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::createFast(C Tube &tube, Int resolution)
{
   if(resolution<0)resolution=12;else MAX(resolution, 3);
   Flt  h_2=tube.h/2;
   create(resolution*2, 0, (resolution-2)*2+resolution*2, 0);
   VecI *tri=T.tri.ind();
   REP(resolution)
   {
      Flt c, s; CosSin(c, s, i*PI2/resolution);
      vtx.pos(           i).set(c*tube.r,  h_2, s*tube.r);
      vtx.pos(resolution+i).set(c*tube.r, -h_2, s*tube.r);
      (tri++)->set(i, (i+1)%resolution, resolution+i);
      (tri++)->set((i+1)%resolution, (i+1)%resolution+resolution, resolution+i);
   }
   REP(resolution-2)
   {
      tri[             i].set(         0,            i+2,            i+1);
      tri[resolution-2+i].set(resolution, resolution+i+1, resolution+i+2);
   }
   transform(Matrix().setPosUp(tube.pos, tube.up));
   return T;
}
MeshBase& MeshBase::create(C Tube &tube, UInt flag, Int resolution)
{
   if(resolution<0)resolution=12;else MAX(resolution, 3);
   Int offset,
       resolution1=resolution+1;
   Flt h_2=tube.h/2;
   create(resolution*2 + resolution1*2, 0, (resolution-2)*2, resolution, flag&(VTX_NRM_TAN_BIN|VTX_TEX0));
   VecI  *tri =T.tri .ind();
   VecI4 *quad=T.quad.ind();

   // vtxs
   REP(resolution)
   {
      Flt c, s; CosSin(c, s, i*PI2/resolution);
      vtx.pos(           i).set(c*tube.r,  h_2, s*tube.r);
      vtx.pos(resolution+i).set(c*tube.r, -h_2, s*tube.r);
      if(vtx.nrm())
      {
         vtx.nrm(           i).set(0,  1, 0);
         vtx.nrm(resolution+i).set(0, -1, 0);
      }
      if(vtx.tan())
      {
         vtx.tan(           i).set(1, 0, 0);
         vtx.tan(resolution+i).set(1, 0, 0);
      }
      if(vtx.bin())
      {
         vtx.bin(           i).set(0, 0, -1);
         vtx.bin(resolution+i).set(0, 0,  1);
      }
      if(vtx.tex0())
      {
         vtx.tex0(           i).set(c*0.5f+0.5f, -s*0.5f+0.5f);
         vtx.tex0(resolution+i).set(c*0.5f+0.5f,  s*0.5f+0.5f);
      }
   }
   offset=resolution*2; REP(resolution1)
   {
      Flt c, s; CosSin(c, s, i*PI2/resolution);
      vtx.pos(offset+            i).set(c*tube.r,  h_2, s*tube.r);
      vtx.pos(offset+resolution1+i).set(c*tube.r, -h_2, s*tube.r);
      if(vtx.nrm())
      {
         vtx.nrm(offset+            i).set(c, 0, s);
         vtx.nrm(offset+resolution1+i).set(c, 0, s);
      }
      if(vtx.tan())
      {
         vtx.tan(offset+            i).set(-s, 0, c);
         vtx.tan(offset+resolution1+i).set(-s, 0, c);
      }
      if(vtx.bin())
      {
         vtx.bin(offset+            i).set(0, -1, 0);
         vtx.bin(offset+resolution1+i).set(0, -1, 0);
      }
      if(vtx.tex0())
      {
         vtx.tex0(offset+            i).set(Flt(i)/resolution, 0);
         vtx.tex0(offset+resolution1+i).set(Flt(i)/resolution, 1);
      }
   }

   // tris
   REP(resolution-2)
   {
      tri[             i].set(         0,            i+2,            i+1);
      tri[resolution-2+i].set(resolution, resolution+i+1, resolution+i+2);
   }

   // quads
   REP(resolution)(quad++)->set(offset+i, offset+i+1, offset+resolution1+i+1, offset+resolution1+i); // sides

   if(!(flag&VTX_TEX0))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   transform(Matrix().setPosUp(tube.pos, tube.up));
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::create(C Cone &cone, UInt flag, Int resolution)
{
   if( resolution<0)resolution=12;else MAX(resolution, 3);
   Int resolution1 =resolution+1,
       offset;

   if(cone.r_low >EPS
   && cone.r_high>EPS)
   {
      offset=resolution*2;
      create(resolution*2+resolution1*2, 0, (resolution-2)*2, resolution, flag&(VTX_NRM_TAN_BIN|VTX_TEX0));
      VecI  *tri =T.tri .ind();
      VecI4 *quad=T.quad.ind();

      // vtxs
      REP(resolution)
      {
         Flt c, s; CosSin(c, s, i*PI2/resolution);
         vtx.pos(           i).set(c*cone.r_high, cone.h, s*cone.r_high);
         vtx.pos(resolution+i).set(c*cone.r_low ,      0, s*cone.r_low );
         if(vtx.nrm())
         {
            vtx.nrm(           i).set(0,  1, 0);
            vtx.nrm(resolution+i).set(0, -1, 0);
         }
         if(vtx.tan())
         {
            vtx.tan(           i).set(1, 0, 0);
            vtx.tan(resolution+i).set(1, 0, 0);
         }
         if(vtx.bin())
         {
            vtx.bin(           i).set(0, 0, -1);
            vtx.bin(resolution+i).set(0, 0,  1);
         }
         if(vtx.tex0())
         {
            vtx.tex0(           i).set(c, 1-s);
            vtx.tex0(resolution+i).set(c,   s);
         }
      }
      Vec2 N(cone.r_high-cone.r_low, cone.h); N=PerpN(N);
      REP(resolution1)
      {
         Flt c, s; CosSin(c, s, i*PI2/resolution);
         vtx.pos(offset+            i).set(c*cone.r_high, cone.h, s*cone.r_high);
         vtx.pos(offset+resolution1+i).set(c*cone.r_low ,      0, s*cone.r_low );
         if(vtx.nrm())
         {
            vtx.nrm(offset+            i).set(N.x*c, N.y, N.x*s);
            vtx.nrm(offset+resolution1+i).set(N.x*c, N.y, N.x*s);
         }
         if(vtx.tan())
         {
            vtx.tan(offset+            i).set(-s,0,c);
            vtx.tan(offset+resolution1+i).set(-s,0,c);
         }
         if(vtx.bin())
         {
            vtx.bin(offset+            i).set(N.y*c, -N.x, N.y*s);
            vtx.bin(offset+resolution1+i).set(N.y*c, -N.x, N.y*s);
         }
         if(vtx.tex0())
         {
            vtx.tex0(offset+            i).set(Flt(i)/resolution, 0);
            vtx.tex0(offset+resolution1+i).set(Flt(i)/resolution, 1);
         }
      }

      // tris
      REP(resolution-2)
      {
         tri[             i].set(         0,            i+2,            i+1);
         tri[resolution-2+i].set(resolution, resolution+i+1, resolution+i+2);
      }

      // quads
      REP(resolution)(quad++)->set(offset+i, offset+i+1, offset+resolution1+i+1, offset+resolution1+i); // sides

      transform(Matrix().setPosUp(cone.pos, cone.up));
   }else
   if(cone.r_low>EPS)
   {
      offset=resolution;
      create(resolution+resolution1*2,  0, resolution-2+resolution, 0, flag&(VTX_NRM_TAN_BIN|VTX_TEX0));
      VecI *tri=T.tri.ind();

      // vtxs
      REP(resolution)
      {
         Flt c, s; CosSin(c, s, i*PI2/resolution);
                       vtx.pos (i).set(c*cone.r_low, 0, s*cone.r_low);
         if(vtx.nrm ())vtx.nrm (i).set(0, -1, 0);
         if(vtx.tan ())vtx.tan (i).set(1,  0, 0);
         if(vtx.bin ())vtx.bin (i).set(0,  0, 1);
         if(vtx.tex0())vtx.tex0(i).set(c,  s   );
      }
      Vec2 N(cone.r_high-cone.r_low, cone.h); N=PerpN(N);
      REP(resolution1)
      {
         Flt c, s; CosSin(c, s, i*PI2/resolution);
         vtx.pos(offset+            i).set(c*cone.r_high, cone.h, s*cone.r_high);
         vtx.pos(offset+resolution1+i).set(c*cone.r_low ,      0, s*cone.r_low );
         if(vtx.nrm())
         {
            vtx.nrm(offset+            i).set(    0,   1,     0);
            vtx.nrm(offset+resolution1+i).set(N.x*c, N.y, N.x*s);
         }
         if(vtx.tan())
         {
            vtx.tan(offset+            i).set(-s, 0, c);
            vtx.tan(offset+resolution1+i).set(-s, 0, c);
         }
         if(vtx.bin())
         {
            vtx.bin(offset+            i).set(N.y*c, -N.x, N.y*s);
            vtx.bin(offset+resolution1+i).set(N.y*c, -N.x, N.y*s);
         }
         if(vtx.tex0())
         {
            vtx.tex0(offset+            i).set(             0.5f, 0);
            vtx.tex0(offset+resolution1+i).set(Flt(i)/resolution, 1);
         }
      }

      // bottom
      REP(resolution-2)(tri++)->set(0, i+1, i+2);

      // sides
      REP(resolution)(tri++)->set(offset+i, offset+resolution1+i+1, offset+resolution1+i);

      transform(Matrix().setPosUp(cone.pos, cone.up));
   }else
   if(cone.r_high>EPS)
   {
      offset=resolution;
      create(resolution+resolution1*2, 0, resolution-2+resolution, 0, flag&(VTX_NRM_TAN_BIN|VTX_TEX0));
      VecI *tri=T.tri.ind();

      // vtxs
      REP(resolution)
      {
         Flt c, s; CosSin(c, s, i*PI2/resolution);
                       vtx.pos (i).set(c*cone.r_high, cone.h, s*cone.r_high);
         if(vtx.nrm ())vtx.nrm (i).set(0, 1,  0);
         if(vtx.tan ())vtx.tan (i).set(1, 0,  0);
         if(vtx.bin ())vtx.bin (i).set(0, 0, -1);
         if(vtx.tex0())vtx.tex0(i).set(c, 1-s  );
      }
      Vec2 N(cone.r_high-cone.r_low, cone.h); N=PerpN(N);
      REP(resolution1)
      {
         Flt c, s; CosSin(c, s, i*PI2/resolution);
         vtx.pos(offset+            i).set(c*cone.r_high, cone.h, s*cone.r_high);
         vtx.pos(offset+resolution1+i).set(c*cone.r_low ,      0, s*cone.r_low );
         if(vtx.nrm())
         {
            vtx.nrm(offset+            i).set(N.x*c, N.y, N.x*s);
            vtx.nrm(offset+resolution1+i).set(    0,  -1,     0);
         }
         if(vtx.tan())
         {
            vtx.tan(offset+            i).set(-s, 0, c);
            vtx.tan(offset+resolution1+i).set(-s, 0, c);
         }
         if(vtx.bin())
         {
            vtx.bin(offset+            i).set(N.y*c, -N.x, N.y*s);
            vtx.bin(offset+resolution1+i).set(N.y*c, -N.x, N.y*s);
         }
         if(vtx.tex0())
         {
            vtx.tex0(offset+            i).set(Flt(i)/resolution, 0);
            vtx.tex0(offset+resolution1+i).set(             0.5f, 1);
         }
      }

      // bottom
      REP(resolution-2)(tri++)->set(0, i+2, i+1);

      // sides
      REP(resolution)(tri++)->set(offset+i, offset+i+1, offset+resolution1+i);

      transform(Matrix().setPosUp(cone.pos, cone.up));
   }else
   {
      del();
   }
   if(!(flag&VTX_TEX0))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::create(C Torus &torus, UInt flag, Int resolution, Int resolution2)
{
   if(resolution <0)resolution =12;else MAX(resolution , 3);
   if(resolution2<0)resolution2=12;else MAX(resolution2, 3);
   create((resolution+1)*(resolution2+1), 0, 0, resolution*resolution2, flag&(VTX_NRM_TAN_BIN|VTX_TEX0));
   VecI4 *quad=T.quad.ind();
   Matrix3 m; m.setUp(torus.up);

   // vtxs
   REP(resolution+1)
   {
      Vec2 cs; CosSin(cs.x, cs.y, i*PI2/resolution);
      Vec  d=cs.x*m.x + cs.y*m.z;
      REPD(i2, resolution2+1)
      {
         Vec2 cs2; CosSin(cs2.x, cs2.y, i2*PI2/resolution2);
         Vec  d2=cs2.x*d + cs2.y*m.y;
         Int  j=i*(resolution2+1)+i2;
                       vtx.pos (j)=torus.pos + d*torus.R + d2*torus.r;
         if(vtx.nrm ())vtx.nrm (j)=d2;
         if(vtx.tan ())vtx.tan (j)=cs.y*m.x - cs.x*m.z;
         if(vtx.bin ())vtx.bin (j)=cs2.x*m.y - cs2.y*d;
         if(vtx.tex0())vtx.tex0(j).set((1-Flt(i)/resolution)*4, Flt(i2)/resolution2);
      }
   }

   // quads
   REPD(i , resolution )
   REPD(i2, resolution2)(quad++)->set( i   *(resolution2+1) + i2  ,  i   *(resolution2+1) + i2+1,
                                      (i+1)*(resolution2+1) + i2+1, (i+1)*(resolution2+1) + i2  );

   if(!(flag&VTX_TEX0))weldVtx(VTX_ALL, EPS, EPS_COL_COS, -1);
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::createEdge(C Rect &rect, Bool solid)
{
   create(4, 4, 0, 0, solid ? EDGE_FLAG : 0);
   vtx .pos(0).set(rect.min.x, rect.max.y, 0);
   vtx .pos(1).set(rect.max.x, rect.max.y, 0);
   vtx .pos(2).set(rect.max.x, rect.min.y, 0);
   vtx .pos(3).set(rect.min.x, rect.min.y, 0);
   edge.ind(0).set(0, 1);
   edge.ind(1).set(1, 2);
   edge.ind(2).set(2, 3);
   edge.ind(3).set(3, 0);
   if(edge.flag())edge.flag(0)=edge.flag(1)=edge.flag(2)=edge.flag(3)=ETQ_R;
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::create(C Shape &shape, UInt flag, Int resolution, Int resolution2)
{
   switch(shape.type)
   {
      case SHAPE_BOX    : return create(shape.box    , flag, resolution);
      case SHAPE_OBOX   : return create(shape.obox   , flag, resolution);
      case SHAPE_BALL   : return create(shape.ball   , flag, resolution);
      case SHAPE_CAPSULE: return create(shape.capsule, flag, resolution, resolution2);
      case SHAPE_TUBE   : return create(shape.tube   , flag, resolution);
      case SHAPE_CONE   : return create(shape.cone   , flag, resolution);
      case SHAPE_TORUS  : return create(shape.torus  , flag, resolution, resolution2);
      default           : return del();
   }
}
/******************************************************************************/
MeshBase& MeshBase::createEdge(C Circle &circle, Bool solid, Int resolution)
{
   if(resolution<0)resolution=24;else MAX(resolution, 3);
   create(resolution, resolution, 0, 0, solid ? EDGE_FLAG : 0);
   FREP(resolution)
   {
      Vec2 v; CosSin(v.x, v.y, i*-PI2/resolution);
                     vtx .pos (i).set(circle.pos+v*circle.r, 0);
                     edge.ind (i).set(i, (i+1)%resolution);
      if(edge.flag())edge.flag(i)=ETQ_R;
   }
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::createEdgeStar(Flt r1, Flt r2, Bool solid, Int resolution)
{
   MAX(resolution, 3); resolution*=2;
   create(resolution, resolution, 0, 0, solid ? EDGE_FLAG : 0);
   FREP(resolution)
   {
      Flt x, y; CosSin(x, y, PI_2-i*PI2/resolution);
      if(i&1)        vtx .pos (i).set(x*r1, y*r1, 0);
      else           vtx .pos (i).set(x*r2, y*r2, 0);
                     edge.ind (i).set(i, (i+1)%resolution);
      if(edge.flag())edge.flag(i)=ETQ_R;
   }
   return T;
}
/******************************************************************************/
// CONVEX
/******************************************************************************/
struct Face
{
   VecI  tri  ;
   Plane plane;
};
struct Convex
{
   Memb<Vec > pos ;
   Memb<Face> face;

   void newFace(Int p0, Int p1, Int p2)
   {
      Face &f=face.New();
      f.tri  .set(p0, p1, p2);
      f.plane.set(pos[p0], GetNormal(pos[p0], pos[p1], pos[p2]));
   }
   Bool cutsEps(Vec &pos)
   {
      if(face.elms())
      {
         REPA(face)if(Dist(pos, face[i].plane)>EPS)return false;
         return true;
      }
      return false;
   }
   Flt distPlanar(Vec &pos)
   {
      Flt    d=0; REPA(face)MAX(d, Dist(pos, face[i].plane));
      return d;
   }
   void include(C Vec &pos)
   {
      Int         pos_index=-1;
      Memc<VecI2> edge;
      REPA(face) // order is important
      {
         Face &f=face[i];
         Flt   d=Dist(pos, f.plane);
         if(   d>EPS) // face requires removing
         {
            if(pos_index<0){pos_index=T.pos.elms(); T.pos.add(pos);} // add point to container
            VecI2 test;
            Int   t;
            test.set(f.tri.c[0], f.tri.c[1]); t=edge.find(test); if(t>=0)edge.remove(t);else{test.reverse(); edge.add(test);}
            test.set(f.tri.c[1], f.tri.c[2]); t=edge.find(test); if(t>=0)edge.remove(t);else{test.reverse(); edge.add(test);}
            test.set(f.tri.c[2], f.tri.c[0]); t=edge.find(test); if(t>=0)edge.remove(t);else{test.reverse(); edge.add(test);}
            face.remove(i);
         }
      }

      // add faces
      REPA(edge)newFace(pos_index, edge[i].y, edge[i].x);
   }
};
MeshBase& MeshBase::createConvex(C Vec *point, Int points, Int max_points)
{
   MeshBase temp; // use 'temp' in case the 'point' comes from 'this'

   if(points>=4)
   {
      Int bound[8]={-1,-1,-1,-1, -1,-1,-1,-1};
      Flt dist [8];

      // get center
      Vec center=0; REP(points)center+=point[i]; center/=points;

      // get boundaries
      REP(points)
      {
         Flt d;
         Int o=0;
         Vec D=point[i]-center;
         d= D.x+D.y+D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
         d= D.x+D.y-D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
         d= D.x-D.y+D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
         d= D.x-D.y-D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
         d=-D.x+D.y+D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
         d=-D.x+D.y-D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
         d=-D.x-D.y+D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
         d=-D.x-D.y-D.z; if(bound[o]<0 || d>dist[o]){bound[o]=i; dist[o]=d;} o++;
      }

      // move indexes left and remove duplicates
      Int bounds=0;
      {
         Int bound_new[Elms(bound)];
         FREPA(bound) // preserve order
         {
            Int b=bound[i]; if(b>=0)
            {
               REP(bounds)if(bound_new[i]==b)goto skip;
               bound_new[bounds++]=b;
            skip:;
            }
         }
         Swap(bound_new, bound);
      }

      Convex convex;

      /*TODO: optimize this?
      if(bounds==8) special case of box
      {
      }else*/
      if(bounds>=4)
      {
         // take 3 first
         VecI trii(bound[0], bound[1], bound[2]);
         Tri  tri (point[trii.x], point[trii.y], point[trii.z]);

         // from the rest take the most distant
         Int disti=-1;
         Flt dist;
         for(Int i=3; i<bounds; i++)
         {
            Flt d=DistPointPlane(point[bound[i]], tri);
            if(disti<0 || Abs(d)>Abs(dist)){disti=i; dist=d;}
         }
         if(dist<=EPS) // if found point is coplanar to the triangle
            REP(points) // find a point that is most distant from the triangle plane
         {
            Flt d=DistPointPlane(point[i], tri);
            if(Abs(d)>Abs(dist)){bound[disti]=i; dist=d;}
         }

         // adjust
         if(dist>0)trii.reverse();
         Swap(bound[disti], bound[3]);

         // build convex from the 4 points
         convex.pos.New()=point[trii.x];
         convex.pos.New()=point[trii.y];
         convex.pos.New()=point[trii.z];
         convex.pos.New()=point[bound[3]];
         convex.newFace(0, 1, 2);
         convex.newFace(1, 0, 3);
         convex.newFace(2, 1, 3);
         convex.newFace(0, 2, 3);

         for(Int i=4; i<bounds; i++)convex.include(point[bound[i]]);
      }

      if(convex.pos.elms())
      {
         Memc<Vec> left; REP(points)
         {
            REPD(b, bounds)if(bound[b]==i)goto already_added;
            left.add(point[i]);
         already_added:;
         }

         if(max_points<=0) // insert all
         {
            REPA(left)if(convex.cutsEps(left[i]))left.remove(i); // early out
            REPA(left)   convex.include(left[i]);
         }
         else // limit the vtx number
         {
            for(; left.elms() && convex.pos.elms()<max_points; )
            {
               // find the most distant
               Int disti=-1;
               Flt dist;
               for(Int i=0; i<Elms(left); ) // order is important
               {
                  Flt d=convex.distPlanar(left[i]);
                  if( d>EPS) // potentially to be added
                  {
                     if(disti<0 || d>dist){disti=i; dist=d;}
                     i++;
                  }
                  else // it's too close so it can be thrown out
                  {
                     left.remove(i);
                  }
               }
               if(disti<0)break;
               convex.include(left[disti]);
               left  .remove (     disti );
            }
         }

         temp.create(convex.pos.elms(), 0, convex.face.elms(), 0);
         REPA(temp.vtx)temp.vtx.pos(i)=convex.pos [i];
         REPA(temp.tri)temp.tri.ind(i)=convex.face[i].tri;
      }
   }

   Swap(T, temp);
   return T;
}
/******************************************************************************
   MeshBase& createBlade(                                                          Int resolution=2, Flt width=0.06f, Flt angle=1.0f, Bool center=false, Flt normal_x_add=2, Flt normal_y_add=3); // create mesh as grass blade,                                                                                    'resolution'=resolution, 'width'=blade width, 'angle'=blade bend angle, 'center'=if create center vertexes, 'normal_x_add normal_y_add' offsets to vertex normal
   MeshBase& createGrass(Int num, C Shape &shape=Shape(Circle(1)), Flt pitch=0.3f, Int resolution=2, Flt width=0.06f, Flt angle=1.0f, Bool center=false, Flt normal_x_add=2, Flt normal_y_add=3); // create mesh as grass      , 'num'=number of blades, 'shape'=grass positions, 'pitch'=max angle of blade pitch, 'resolution'=resolution, 'width'=blade width, 'angle'=blade bend angle, 'center'=if create center vertexes, 'normal_x_add normal_y_add' offsets to vertex normal
   MeshBase& createRock (Int resolution=3, Int subdivisions=2                                                                                                                               ); // create mesh as rock

enum MESH_TREE_FLAG // Mesh Tree Creation Flags
{
   MESH_TREE_LEAF_FLAT  =0x1, // leafs will be created flat instead of geometrical
   MESH_TREE_LEAF_CENTER=0x2, // leafs will be created at center of the branch (valid only with MESH_TREE_LEAF_FLAT flag)
};

   Mesh& createTree(MaterialPtr bark_material, Memb<MaterialPtr> &leaf_materials, UInt tree_flag=0, Flt leaf_scale=1, Flt leaf_intensity=1); // create a random tree mesh, 'tree_flag'=MESH_TREE_FLAG
/******************************************************************************/
// GRASS
/******************************************************************************
MeshBase& MeshBase::createBlade(Int resolution, Flt width, Flt angle, Bool center, Flt nrm_x_add, Flt nrm_y_add)
{
   if(center)create(1 + (resolution+1)*3 + 1,0,4,resolution*2,VTX_TEX0);
   else      create(1 + (resolution+1)*2 + 1,0,2,resolution  ,VTX_TEX0);
   Vec   *pos =T.vtx .pos ();
   Vec2  *tex =T.vtx .tex0();
   VecI  *tri =T.tri .ind ();
   VecI4 *quad=T.quad.ind ();

   // vtxs
   (pos++)->zero();
   (tex++)->set (0.5f,1.0f);
   for(Int i=0; i<=resolution; i++)
   {
      Flt s=(i+1)/Flt(resolution+2),
          w=Sin(s*PI)*width,
          y=  Sin(s*angle),
          z=1-Cos(s*angle);
      if(center)
      {
         (pos++)->set(-w,y-z*w,z+(1-y)*w); (tex++)->set(0   ,1-s);
         (pos++)->set( 0,y    ,z        ); (tex++)->set(0.5f,1-s);
         (pos++)->set( w,y-z*w,z+(1-y)*w); (tex++)->set(1   ,1-s);
      }else
      {
         (pos++)->set(-w,y,z); (tex++)->set(0,1-s);
         (pos++)->set( w,y,z); (tex++)->set(1,1-s);
      }
   }
   pos->set(0,Sin(angle),1-Cos(angle));
   tex->set(0.5f,0.0f);

   // tris
   if(center)
   {
      tri[0].set(0,1,2);
      tri[1].set(0,2,3); Int a=vtxs()-1, b=a-1, c=a-2, d=a-3;
      tri[2].set(a,b,c);
      tri[3].set(a,c,d);
   }else
   {
      tri[0].set(0,1,2); Int a=vtxs()-1, b=a-1, c=a-2;
      tri[1].set(a,b,c);
   }

   // quads
   FREP(resolution)
   {
      if(center)
      {
         Int i3=i*3;
         (quad++)->set(1+i3  , 1+i3+3  , 1+i3+3+1, 1+i3+1);
         (quad++)->set(1+i3+1, 1+i3+3+1, 1+i3+3+2, 1+i3+2);
      }else
      {
         Int i2=i*2;
         (quad++)->set(1+i2, 1+i2+2, 1+i2+2+1, 1+i2+1);
      }
   }

   // nrm
   setNormals();
   if(nrm_x_add || nrm_y_add)
   {
      Vec *pos=T.vtx.pos();
      Vec *nrm=T.vtx.nrm();
      REPA(vtx)
      {
         nrm[i].x+=nrm_x_add*Sign(pos[i].x);
         nrm[i].y+=nrm_y_add;
         nrm[i].normalize();
      }
   }
   return T;
}
MeshBase& MeshBase::createGrass(Int num, C Shape &shape, Flt pitch, Int resolution, Flt width, Flt angle, Bool center, Flt nrm_x_add, Flt nrm_y_add)
{
   MeshBase *mshb; AllocZero(mshb,num);
   REP(num)
   {
      Vec offset=Random(shape);
      if(ShapeType2D(shape.type))Swap(offset.y,offset.z);
      else                       offset.y=0;
      mshb[i].createBlade(resolution, Random.f(0.9f,1.1f)*width, Random.f(0.9f,1.1f)*angle,center,nrm_x_add,nrm_y_add).transform(Matrix().setRotateXY(Random.f(-pitch,pitch), Random.f(PI2)).move(offset));
   }
   create(mshb,num);
   REP(num)mshb[i].del(); Free(mshb);
   return T;
}
/******************************************************************************/
// ROCK
/******************************************************************************
MeshBase& MeshBase::createRock(Int resolution, Int subdivisions)
{
   create(Ball(0.5),VTX_TEX0,resolution).quadToTri();

   // vtxs
   setVtxDup();
   Vec *pos=vtx.pos();
   Int *dup=vtx.dup();
   REPA(vtx)if(dup[i]==i)pos[i]+=Random(Ball(0.15f));
   REPA(vtx)pos[i]=pos[dup[i]];

   // scale
   Flt f=0.6f,t=1/f;
   scale(Vec(Random.f(f,t), Random.f(f,t), Random.f(f,t)));

   // smooth
   REP(subdivisions)subdivide().setVtxDup();

   // finish
   setTangents();
   return T;
}
/******************************************************************************/
// TREE
/******************************************************************************/
#define RES  7
#define STEP 2
/******************************************************************************
struct Branch
{
   Vec pos[RES+1];
   Flt r  [RES+1];
   Flt length;
   Int num,parent;

   Flt step(Flt frac, Vec &pos, Vec &dir)
   {
      Flt length=0;
      FREP(num)
      {
         Vec &p0=T.pos[i],
             &p1=T.pos[i+1],
             d  =p1-p0;
         Flt l  =d.normalize()/T.length;
         if(frac>=length && frac<=length+l)
         {
            frac=(frac-length)/l;
               dir=d;
               pos=Lerp(p0  ,p1    ,frac);
            return Lerp(r[i],r[i+1],frac);
         }
         length+=l;
      }
      pos=T.pos[0];
      return 0;
   }
};
struct Leaf
{
   Matrix m;
};
/******************************************************************************
static void BranchAdd(Memb<Branch> &branch, C Vec &pos, C Vec &dir, Flt r, Flt length, Int parent, Int step)
{
   Int   num=Mid(Round((length/20)*2*RES),1,RES),
         cur=branch.elms();
   Branch &b=branch.New();
   b.length =length;
   b.parent =parent;
   b.num    =num;
   Vec  p=pos,
        d=dir;
   FREP(num)
   {
      b.pos[i]=p;
      b.r  [i]=r;

      p+=d*length/num;
      d =Random.dir(d,step ? 0.5f : 0.15f);
      r*=Random.f(0.75f,0.85f);
   }
   b.pos[num]=p;
   b.r  [num]=0;
   if(step<=STEP)REPD(c,Round(Random.f(0.2f,1.1f)*length))
   {
      Vec pos,dir;
      Flt f=Random.f(0.3f,0.95f);
      Flt r=b.step(f,pos,dir);
      BranchAdd(branch, pos, Random.dir(dir,0.5f,1.2f), Random.f(0.75f,0.85f)*r, Random.f(0.8f,1.2f)*(1-f)*length,cur,step+1);
   }
}
static void BranchCreate(Memb<Branch> &branch)
{
   BranchAdd(branch, Vec(0, 0, 0), Vec(0, 1, 0), Random.f(0.25f, 0.5f), Random.f(15, 20), -1, 0);
}
static void BranchDraw(Memb<Branch> &branch)
{
   REPA(branch)
   {
      Branch &b=branch[i];
      Int     r=Max(3,Round(Sqrt(b.r[0])*16));
      REP(b.num)
      {
         Vec &p0=b.pos[i  ],
             &p1=b.pos[i+1];
         Cone(b.r[i],b.r[i+1],Dist(p0,p1),p0,!(p1-p0)).draw(WHITE,false,r);
      }
   }
}
static void CreateMesh(MeshBase &mshb, Branch &b, MeshBase *inside)
{
   if(!b.num)mshb.del();else
   {
      Int r =Max(3,Round(Sqrt(b.r[0])*16)),
          r1=r+1;
      mshb.create(r1*b.num+1,0,r,r*(b.num-1), VTX_NRM|VTX_TEX0);
      Vec   *pos =mshb.vtx .pos ();
      Vec   *nrm =mshb.vtx .nrm ();
      Vec2  *tex =mshb.vtx .tex0();
      VecI  *tri =mshb.tri .ind ();
      VecI4 *quad=mshb.quad.ind ();
      Matrix matrix=MatrixIdentity;
      Int    v=0;
      Flt    t=Random.f();
      FREP(b.num)
      {
         Vec dir  =b.pos[i+1]-b.pos[i]; Flt length=dir.normalize();
         matrix.orn()*=Matrix3().setRotation(matrix.y, dir);
         matrix.pos   =b.pos[i];
         REPD(j,r1)
         {
            Flt c, s; CosSin(c, s, j*PI2/r);
            pos[v+j].set(c*b.r[i],0,s*b.r[i]);
            nrm[v+j].set(c       ,0,s       );
            tex[v+j].set(Flt(j)/r,t);
         }
         Transform(pos+v,matrix      ,r1);
         Transform(nrm+v,matrix.orn(),r1); Normalize(nrm+v,r1);
         if(i==b.num-1)
         {
            Int last=mshb.vtxs()-1;
            REPD(j,r)(tri++)->set(last,v+j+1,v+j);
         }else
         {
            REPD(j,r)(quad++)->set(v+r1+j,v+r1+j+1,v+j+1,v+j);
         }
         if(inside && i==2)
         {
            inside->create(r,0,r-2,0, VTX_TEX0);
            REPD(j,r)
            {
               Flt c, s; CosSin(c, s, j*PI2/r);
               inside->vtx.pos (j)=pos[v+j];
               inside->vtx.tex0(j).set(c*0.3f+0.5f,s*-0.3f+0.5f);
            }
            REPD(j,r-2)inside->tri.ind(j).set(0,j+2,j+1);
         }
         t-=length;
         v+=r1;
      }
      pos[v]=b.pos[b.num];
      nrm[v]=matrix.y;
      tex[v].set(0.5f,t);
      mshb.texScale(Vec2(1,(b.parent==-1) ? 0.35f : 0.1f/b.r[0]))
          .texMove (Random.vec2());
   }
}
static void LeafCreate(Memb<Leaf> &leaf, Memb<Branch> &branch, UInt tree_flag, Flt leaf_scale, Flt leaf_intensity)
{
   FREPA(branch)
   {
      Branch &b=branch[i];
      REPD(j,b.num)
      {
         Flt r=Avg(b.r[j],b.r[j+1]);
         if( r<=0.1f)
         {
            Vec &p0    =b.pos[j],
                &p1    =b.pos[j+1],
                 dir   =p1-p0;
            Flt  length=dir.normalize();
            REPD(cn,Round(Random.f(0.75f,1.25f)*10*length*leaf_intensity))
            {
               Leaf   &l=leaf.New();
               Matrix &m=l.m;

               m.setRotateX(Random.f(DegToRad(45),(tree_flag&MESH_TREE_LEAF_FLAT) ? DegToRad(180) : DegToRad(90)))
                .   rotateY(Random.f(PI2))
                .   scale  (Random.f(0.75f,1.3f)*((tree_flag&MESH_TREE_LEAF_FLAT) ? 0.4f : 0.5f)*leaf_scale);

               Flt c, s; CosSin(c, s, Random.f(PI2));
               Flt f=Random.f();
               Vec y=PerpN   (  dir),
                   x=CrossN  (y,dir);
                   r=Lerp    (b.r[j],b.r[j+1],f);
               m+=p0
                 +dir*(f*length)
                 +x  *(r*     c)
                 +y  *(r*     s);
            }
         }
      }
   }
}
static void CreateMesh(MeshBase &mshb,Leaf &leaf,UInt tree_flag)
{
   if(tree_flag&MESH_TREE_LEAF_FLAT)
   {
      mshb.createPlane(2,2,VTX_TEX0|VTX_NRM).move(Vec(-0.5,0,0));
      REPA(mshb.vtx)
      {
         mshb.vtx.nrm(i).x+=Sign(mshb.vtx.pos(i).x);
         mshb.vtx.nrm(i)  +=Random.vec(-0.2f,0.2f);
         mshb.vtx.nrm(i).normalize();
      }
      if(tree_flag&MESH_TREE_LEAF_CENTER)mshb.move(Vec(0,-0.5,0));
   }else
   {
      mshb.createBlade(1,0.2f,1.5f,false,1,0);
   }
   mshb*=leaf.m;
   mshb.include(VTX_HLP); REPA(mshb.vtx)mshb.vtx.hlp(i)=leaf.m.pos; // set helper positions to leaf start
}
static Mesh& setTree(Mesh &mesh, MaterialPtr bark_material, Memb<MaterialPtr> &leaf_materials, Memb<Branch> &branch, UInt tree_flag, Flt leaf_scale, Flt leaf_intensity)
{
   Bool           inside=false;
   Memc<MeshPart> p;

   // inside
   if(inside)
   {
      MeshPart &part=p.New(); // create 0-th MeshPart for 'inside'
                part.flag=MSHP_NO_PHYS_BODY;
   }

   // branches
   REPA(branch)
   {
      MeshPart  &part=p.New();
                 part.material(bark_material);
      CreateMesh(part.base,branch[i],(!inside || i) ? null : &p(0).base);
   }

   // leafs
   if(leaf_materials.elms())
   {
      Memb<Leaf> leaf; LeafCreate(leaf,branch,tree_flag,leaf_scale,leaf_intensity);
      REPA(leaf)
      {
         MeshPart  &part=p.New();
         CreateMesh(part.base,leaf[i],tree_flag);
                    part.material(leaf_materials[i%leaf_materials.elms()]);
                    part.flag=MSHP_NO_PHYS_BODY;
      }
   }

   // finish
   {
      mesh.create (p.elms()            ); FREPA(mesh)Swap(mesh.part(i),p[i]);
      mesh.joinAll(true, true, false, 0);

      mesh.setTangents()
          .setBox     ();
   }

   return mesh;
}
Mesh& Mesh::createTree(MaterialPtr bark_material, Memb<MaterialPtr> &leaf_materials, UInt tree_flag, Flt leaf_scale, Flt leaf_intensity)
{
   Memb<Branch> branch;
   BranchCreate(branch);
   setTree(T,bark_material,leaf_materials,branch,tree_flag,leaf_scale,leaf_intensity);
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
