/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
CutsCache::~CutsCache()
{
   DeleteN(box_face);
}
CutsCache::CutsCache(C MeshBase &mshb)
{
   box=mshb;
   boxes.set(box, mshb.faces());
   New(box_face, boxes.num  ());

   // add faces to boxes
 C Vec *pos=mshb.vtx.pos();
   REPA(mshb.tri) // triangles
   {
    C Int *p   =mshb.tri.ind(i).c;
      Box  box =pos[p[0]];
           box|=pos[p[1]];
           box|=pos[p[2]];
      BoxI boxi=boxes.coords(box.extend(EPS));

      for(Int z=boxi.min.z; z<=boxi.max.z; z++)
      for(Int y=boxi.min.y; y<=boxi.max.y; y++)
      for(Int x=boxi.min.x; x<=boxi.max.x; x++)box_face[boxes.index(VecI(x, y, z))].add(i);
   }
   REPA(mshb.quad) // quads
   {
    C Int *p   =mshb.quad.ind(i).c;
      Box  box =pos[p[0]];
           box|=pos[p[1]];
           box|=pos[p[2]];
           box|=pos[p[3]];
      BoxI boxi=boxes.coords(box.extend(EPS));

      for(Int z=boxi.min.z; z<=boxi.max.z; z++)
      for(Int y=boxi.min.y; y<=boxi.max.y; y++)
      for(Int x=boxi.min.x; x<=boxi.max.x; x++)box_face[boxes.index(VecI(x, y, z))].add(i^SIGN_BIT);
   }
}
/******************************************************************************/
// DIST
/******************************************************************************/
DIST_TYPE DistPointMesh(C Vec2 &point, C MeshBase &mshb, UInt flag, Flt *distance, Int *index, UInt test_flag)
{
   DIST_TYPE type=DIST_NONE;
   Int       j=-1;
   Flt       d, dist=0;
 C Int      *p;
 C Vec      *pos =mshb.vtx .pos ();
 C VecI2    *edge=mshb.edge.ind ();
 C VecI     *tri =mshb.tri .ind ();
 C VecI4    *quad=mshb.quad.ind ();
 C Byte     *efl =mshb.edge.flag();
//C Int     *eid =mshb.edge.id  ();
   if(flag& VTX_POS)REPA(mshb.vtx ){             d=Dist(point,       pos[  i ].xy                                           ); if(!type || d<dist){dist=d; type=DIST_POINT; j=i;}}
   if(flag& TRI_IND)REPA(mshb.tri ){p=tri [i].c; d=Dist(point,  Tri2(pos[p[0]].xy, pos[p[1]].xy, pos[p[2]].xy              )); if(!type || d<dist){dist=d; type=DIST_TRI  ; j=i;}}
   if(flag&QUAD_IND)REPA(mshb.quad){p=quad[i].c; d=Dist(point, Quad2(pos[p[0]].xy, pos[p[1]].xy, pos[p[2]].xy, pos[p[3]].xy)); if(!type || d<dist){dist=d; type=DIST_QUAD ; j=i;}}
   if(flag&EDGE_IND)REPA(mshb.edge)
   {
      if(test_flag)
      {
         if(test_flag&TEST_NO_PHYS     && efl &&  efl[i]&ETQ_NO_PHYS           )continue;
       //if(test_flag&TEST_DOUBLE_SIDE && eid && (eid[i].x==-1)==(eid[i].y==-1))continue;
      }
      p=edge[i].c; d=DistPointEdge(point, pos[p[0]].xy, pos[p[1]].xy); if(!type || d<dist){dist=d; type=DIST_EDGE; j=i;}
   }
   if(distance)*distance=dist;
   if(index   )*index   =j   ;
   return type;
}
DIST_TYPE DistPointMesh(C Vec &point, C MeshBase &mshb, UInt flag, Flt *distance, Int *index)
{
   DIST_TYPE type=DIST_NONE;
   Int       j=-1;
   Flt       d, dist=0;
 C Int      *p;
 C Vec      *pos =mshb.vtx .pos();
 C VecI2    *edge=mshb.edge.ind();
 C VecI     *tri =mshb.tri .ind();
 C VecI4    *quad=mshb.quad.ind();
 C Vec      *tn  =mshb.tri .nrm();
 C Vec      *qn  =mshb.quad.nrm();
   if(flag& VTX_POS)REPA(mshb.vtx ){             d=Dist         (point,      pos[  i ]                                                      ); if(!type || d<dist){dist=d; type=DIST_POINT; j=i;}}
   if(flag&EDGE_IND)REPA(mshb.edge){p=edge[i].c; d=DistPointEdge(point,      pos[p[0]], pos[p[1]]                                           ); if(!type || d<dist){dist=d; type=DIST_EDGE ; j=i;}}
   if(flag& TRI_IND)REPA(mshb.tri ){p=tri [i].c; d=Dist         (point,  Tri(pos[p[0]], pos[p[1]], pos[p[2]]           , tn ? &tn[i] : null)); if(!type || d<dist){dist=d; type=DIST_TRI  ; j=i;}}
   if(flag&QUAD_IND)REPA(mshb.quad){p=quad[i].c; d=Dist         (point, Quad(pos[p[0]], pos[p[1]], pos[p[2]], pos[p[3]], qn ? &qn[i] : null)); if(!type || d<dist){dist=d; type=DIST_QUAD ; j=i;}}
   if(distance)*distance=dist;
   if(index   )*index   =j   ;
   return type;
}
/******************************************************************************/
// CUTS
/******************************************************************************/
struct DIST
{
   DIST_TYPE type;
   Int       ind, part;
   Flt       dist;
   Vec       nrm;
};
static void DISTAdd(Memt<DIST> &memc, DIST_TYPE type, Flt d, Int ind, Int part, C Vec *nrm=null)
{
   REPA(memc)
   {
      DIST &dist=memc[i];
      if(dist.dist>d+EPS)memc.remove(i);
   }
   DIST  &dist=memc.New();
          dist.type= type;
          dist.ind = ind ;
          dist.part= part;
          dist.dist= d   ;
   if(nrm)dist.nrm =*nrm ;
}
/******************************************************************************/
// CUTS 2D
/******************************************************************************/
static Int FindClosest(C Vec2 &point, C MeshBase &mshb, Int part, UInt test_flag, Flt &dist, Memt<DIST> &dists)
{
   // later codes assume that we'll add 'DIST' only when there's mshb.edge.flag
   if(mshb.edge.flag())FREPA(mshb.edge)
   {
      if(test_flag)
      {
         if(test_flag&TEST_NO_PHYS && mshb.edge.flag(i)&ETQ_NO_PHYS)continue;
      }
      DIST_TYPE type;
    C Int *p=mshb.edge.ind(i).c;
      Flt  d=DistPointEdge(point, mshb.vtx.pos(p[0]).xy, mshb.vtx.pos(p[1]).xy, &type);
      if(  d<=EPS){dist=0; return (mshb.edge.flag(i)&ETQ_LR) ? +1 : -1;}
      if( !dists.elms() || d<=dist+EPS){if(!dists.elms() || d<dist)dist=d; DISTAdd(dists, type, d, i, part);}
   }
   return 0;
}
Bool CutsPointMesh(C Vec2 &point, C MeshBase &mshb, Flt *distance, UInt test_flag)
{
   Bool       ret=false;
   Flt        dist;
   Memt<DIST> dists;

   // we assume that FindClosest will add 'DIST' only when there's mshb.edge.flag

   // get distances
   if(Int fc=FindClosest(point, mshb, 0, test_flag, dist, dists)){ret=(fc==1); goto end;}

   // check for closest edge
   FREPA(dists)
   {
      DIST &dist=dists[i];
      if(   dist.type==DIST_EDGE)
      {
       C Int  *p =mshb.edge.ind(dist.ind).c;
       C Vec2 &p0=mshb.vtx .pos(p[0]    ).xy,
              &p1=mshb.vtx .pos(p[1]    ).xy;
         if(mshb.edge.flag(dist.ind)&((DistPointPlane(point, p0, Perp(p1-p0))>0) ? ETQ_R : ETQ_L))ret=true;
         goto end;
      }
   }

   // check for closest point
   if(dists.elms()>=2)
   {
      DIST &center    =dists.first();
    C Vec2 &center_pos=mshb.vtx.pos(mshb.edge.ind(center.ind).c[center.type-DIST_POINT0]).xy;
      Flt  point_angle=AngleFast(point-center_pos),
             max_angle;
      Int    max_ind=-1;
      Bool point_loose;
      for(Int i=1; i<dists.elms(); i++) // for each other point that contact's center
      {
         DIST &dist=dists[i];
         Bool pl; C Int *p=mshb.edge.ind(dist.ind).c;
         if(Equal(center_pos, mshb.vtx.pos(p[0]).xy))pl=1;else
         if(Equal(center_pos, mshb.vtx.pos(p[1]).xy))pl=0;else continue;
         Flt angle=AngleFast(mshb.vtx.pos(p[pl]).xy-center_pos); if(angle>point_angle)angle-=PI2;
         if(max_ind<0 || angle>max_angle){max_ind=i; max_angle=angle; point_loose=pl;}
      }
      if(max_ind>=0)
      {
         DIST &dist=dists[max_ind];
         if(mshb.edge.flag(dist.ind)&(point_loose ? ETQ_L : ETQ_R))ret=true;
      }
   }

end:;
   if(distance)*distance=dist;
   return ret;
}
Bool CutsPointMesh(C Vec2 &point, C MeshLod &mesh, Flt *distance, UInt test_flag)
{
   Bool       ret=false;
   Flt        dist;
   Memt<DIST> dists;

   // we assume that FindClosest will add 'DIST' only when there's mshb.edge.flag

   // get distances
   FREPA(mesh)if(Int fc=FindClosest(point, mesh.parts[i].base, i, test_flag, dist, dists)){ret=(fc==1); goto end;}

   // check for closest edge
   FREPA(dists)
   {
      DIST &dist=dists[i];
      if(   dist.type==DIST_EDGE)
      {
       C MeshBase &mshb=mesh.parts[dist.part].base;
       C Int      *p   =mshb.edge.ind(dist.ind).c;
       C Vec2     &p0  =mshb.vtx .pos(p[0]    ).xy,
                  &p1  =mshb.vtx .pos(p[1]    ).xy;
         if(mshb.edge.flag(dist.ind)&((DistPointPlane(point, p0, Perp(p1-p0))>0) ? ETQ_R : ETQ_L))ret=true;
         goto end;
      }
   }

   // check for closest point
   if(dists.elms()>=2)
   {
      DIST     &center    =dists.first();
    C MeshBase &mshb      =mesh.parts[center.part].base;
    C Vec2     &center_pos=mshb.vtx.pos(mshb.edge.ind(center.ind).c[center.type-DIST_POINT0]).xy;
      Flt      point_angle=AngleFast(point-center_pos),
                 max_angle;
      Int        max_ind=-1;
      Bool     point_loose;
      for(Int i=1; i<dists.elms(); i++) // for each other point that contact's center
      {
         DIST     &dist=dists[i];
       C MeshBase &mshb=mesh.parts[dist.part].base;
         Bool pl; C Int *p=mshb.edge.ind(dist.ind).c;
         if(Equal(center_pos, mshb.vtx.pos(p[0]).xy))pl=1;else
         if(Equal(center_pos, mshb.vtx.pos(p[1]).xy))pl=0;else continue;
         Flt angle=AngleFast(mshb.vtx.pos(p[pl]).xy-center_pos); if(angle>point_angle)angle-=PI2;
         if(max_ind<0 || angle>max_angle){max_ind=i; max_angle=angle; point_loose=pl;}
      }
      if(max_ind>=0)
      {
         DIST     &dist=dists[max_ind];
       C MeshBase &mshb=mesh.parts[dist.part].base;
         if(mshb.edge.flag(dist.ind)&(point_loose ? ETQ_L : ETQ_R))ret=true;
      }
   }

end:;
   if(distance)*distance=dist;
   return ret;
}
/******************************************************************************/
// CUTS 3D
/******************************************************************************/
struct CutsAdd
{
   Vec         point; // testing point
 C Vec        *pos  ; // mshb.vtx.pos
 C MeshBase   *mshb ;
   CutsCache  *cache;
   Flt        *dist ;
   Memt<DIST> *dists;
};
static Bool Add(C VecI &coords, CutsAdd &ca)
{
   Memc<UInt> &faces=ca.cache->box_face[ca.cache->boxes.index(coords)];
   REPA(faces)
   {
      UInt face=faces[i];
      if(  face&SIGN_BIT) // quad
      {
       C Int      *p=ca.mshb->quad.ind(face^SIGN_BIT).c;
         Quad      quad(ca.pos[p[0]], ca.pos[p[1]], ca.pos[p[2]], ca.pos[p[3]], ca.mshb->quad.nrm() ? &ca.mshb->quad.nrm(face^SIGN_BIT) : null);
         DIST_TYPE type; Flt d=Dist(ca.point, quad, &type);
         if(!ca.dists->elms() || d<=*ca.dist+EPS)
         {
            if(!ca.dists->elms() || d<=*ca.dist)*ca.dist=d;
            DISTAdd(*ca.dists, type, d, face, 0, &quad.n);
         }
      }
      else // triangle
      {
       C Int      *p=ca.mshb->tri.ind(face).c;
         Tri       tri(ca.pos[p[0]], ca.pos[p[1]], ca.pos[p[2]], ca.mshb->tri.nrm() ? &ca.mshb->tri.nrm(face) : null);
         DIST_TYPE type; Flt d=Dist(ca.point, tri, &type);
         if(!ca.dists->elms() || d<=*ca.dist+EPS)
         {
            if(!ca.dists->elms() || d<=*ca.dist)*ca.dist=d;
            DISTAdd(*ca.dists, type, d, face, 0, &tri.n);
         }
      }
   }
   return faces.elms()!=0;
}
static void FindClosest(C Vec &point, C MeshBase &mshb, Int part, Flt &dist, Memt<DIST> &dists)
{
   FREPA(mshb.tri)
   {
    C Int      *p=mshb.tri.ind(i).c; Tri tri(mshb.vtx.pos(p[0]), mshb.vtx.pos(p[1]), mshb.vtx.pos(p[2]), mshb.tri.nrm() ? &mshb.tri.nrm(i) : null);
      DIST_TYPE type; Flt d=Dist(point, tri, &type);
      if(!dists.elms() || d<=dist+EPS)
      {
         if(!dists.elms() || d<dist)dist=d;
         DISTAdd(dists, type, d, i, part, &tri.n);
      }
   }
   FREPA(mshb.quad)
   {
    C Int      *p=mshb.quad.ind(i).c; Quad quad(mshb.vtx.pos(p[0]), mshb.vtx.pos(p[1]), mshb.vtx.pos(p[2]), mshb.vtx.pos(p[3]), mshb.quad.nrm() ? &mshb.quad.nrm(i) : null);
      DIST_TYPE type; Flt d=Dist(point, quad, &type);
      if(!dists.elms() || d<=dist+EPS)
      {
         if(!dists.elms() || d<dist)dist=d;
         DISTAdd(dists, type, d, i^SIGN_BIT, part, &quad.n);
      }
   }
}
Bool CutsPointMesh(C Vec &point, C MeshBase &mshb, Flt *distance, CutsCache *cache)
{
   Bool       ret=false;
   Flt        dist;
   Memt<DIST> dists;

   // get distances
   if(cache)
   {
      CutsAdd ca;
      ca.point= point;
      ca.pos  = mshb.vtx.pos();
      ca.mshb =&mshb ;
      ca.cache= cache;
      ca.dist =&dist ;
      ca.dists=&dists;
      VecI    coords=cache->boxes.coords(point);
      if(!Add(coords, ca)) // try center first, if not, then try testing box surface starting with radius=1 and increasing each step
      {
         VecI min =coords,
              max =coords,
              size=cache->boxes.cells;
         for(;;)
         {
            min--;
            max++;
            if(min.x>=0     || min.y>=0     || min.z>=0
            || max.x<size.x || max.y<size.y || max.z<size.z)
            {
               VecI minc=min,
                    maxc=max;
               MAX( minc.x, 0); MIN(maxc.x, size.x-1);
               MAX( minc.y, 0); MIN(maxc.y, size.y-1);
               MAX( minc.z, 0); MIN(maxc.z, size.z-1);

               // all sides need to be checked (can't stop on first found because there were errors)
               if(min.x==minc.x)for(Int y=minc.y  ; y<=maxc.y; y++)for(Int z=minc.z  ; z<=maxc.z; z++)Add(VecI(min.x, y, z), ca); // left
               if(max.x==maxc.x)for(Int y=minc.y  ; y<=maxc.y; y++)for(Int z=minc.z  ; z<=maxc.z; z++)Add(VecI(max.x, y, z), ca); // right
               if(min.y==minc.y)for(Int x=minc.x+1; x< maxc.x; x++)for(Int z=minc.z  ; z<=maxc.z; z++)Add(VecI(x, min.y, z), ca); // bottom , without left and right                end
               if(max.y==maxc.y)for(Int x=minc.x+1; x< maxc.x; x++)for(Int z=minc.z  ; z<=maxc.z; z++)Add(VecI(x, max.y, z), ca); // top    , without left and right                end
               if(min.z==minc.z)for(Int x=minc.x+1; x< maxc.x; x++)for(Int y=minc.y+1; y< maxc.y; y++)Add(VecI(x, y, min.z), ca); // back   , without left     right bottom and top end
               if(max.z==maxc.z)for(Int x=minc.x+1; x< maxc.x; x++)for(Int y=minc.y+1; y< maxc.y; y++)Add(VecI(x, y, max.z), ca); // forward, without left     right bottom and top end
               if(ca.dists->elms())break;
            }else break;
         }
      }
   }else
   {
      FindClosest(point, mshb, 0, dist, dists);
   }

   // check for closest plane
   FREPA(dists)
   {
      DIST &dist=dists[i];
      if(   dist.type==DIST_PLANE)
      {
         if(dist.ind&SIGN_BIT) // quad
         {
          C Int *p =mshb.quad.ind(dist.ind^SIGN_BIT).c;
          C Vec &p0=mshb.vtx .pos(p[0]),
                &p1=mshb.vtx .pos(p[1]),
                &p2=mshb.vtx .pos(p[2]),
                &p3=mshb.vtx .pos(p[3]);
            Tri tri0(p0, p1, p3),
                tri1(p1, p2, p3);
            if(DistPointPlane(p2, tri0)<=0) // convex
            {
               ret=DistPointPlane(point, tri0)<=0
                && DistPointPlane(point, tri1)<=0;
            }
            else // concave
            {
               ret=DistPointPlane(point, tri0)<=0
                || DistPointPlane(point, tri1)<=0;
            }
         }
         else // tri
         {
            ret=(DistPointPlane(point, mshb.vtx.pos(mshb.tri.ind(dist.ind).c[0]), dist.nrm)<=0);
         }
         goto end;
      }
   }

   // check for closest edge
   {
      Bool edge_found=false;
      Flt  edge_angle_nearest,
           edge_angle_delta;
      FREPA(dists) // find the edge which 3rd point (loose) is within the smallest angle to the tested one
      {
         DIST &dist=dists[i];
         if(   dist.type>=DIST_EDGE0 && dist.type<=DIST_EDGE3)
         {
          C Int *p;
            Int vtxs, ei=dist.type-DIST_EDGE0; // edge index
            if(dist.ind&SIGN_BIT){p=mshb.quad.ind(dist.ind^SIGN_BIT).c; vtxs=4;}
            else                 {p=mshb.tri .ind(dist.ind         ).c; vtxs=3;}

          C Vec &edge_p0=mshb.vtx.pos(p[ ei        ]),
                &edge_p1=mshb.vtx.pos(p[(ei+1)%vtxs]);

            Matrix m; m.setPosDir(edge_p0, !(edge_p1-edge_p0));

            Flt point_angle=AngleFast (point                       , m),
                 edge_angle=AngleFast (mshb.vtx.pos(p[(ei+2)%vtxs]), m),
                delta_angle=AngleDelta( edge_angle, point_angle),
            abs_delta_angle=Abs       (delta_angle);

            if(!edge_found || abs_delta_angle<edge_angle_nearest)
            {
               edge_found        =true;
               edge_angle_nearest=abs_delta_angle;
               edge_angle_delta  =    delta_angle;
            }
         }
      }
      if(edge_found)
      {
         if(edge_angle_delta<=0)ret=true;
         goto end;
      }
   }

   // check for closest vertex
   FREPA(dists)
   {
      DIST &dist=dists[i];
      if(   dist.type>=DIST_POINT0 && dist.type<=DIST_POINT3) // get first, set as center
      {
       C Int *p;
         Int  pi=dist.type-DIST_POINT0; // point index
         if(dist.ind&SIGN_BIT)p=mshb.quad.ind(dist.ind^SIGN_BIT).c;
         else                 p=mshb.tri .ind(dist.ind         ).c;

       C Vec &center_pos=mshb.vtx.pos(p[pi]);
         Vec  center_nrm=0;

         for(Int j=i; j<dists.elms(); j++) // calculate average triangle normal from all vertexes that share same position with 'center'
         {
            DIST &dist=dists[j];
            if(   dist.type>=DIST_POINT0 && dist.type<=DIST_POINT3)
            {
               Int vtxs, pi=dist.type-DIST_POINT0;
               if(dist.ind&SIGN_BIT){p=mshb.quad.ind(dist.ind^SIGN_BIT).c; vtxs=4;}
               else                 {p=mshb.tri .ind(dist.ind         ).c; vtxs=3;}

               if(Equal(mshb.vtx.pos(p[pi]), center_pos)) // only for those which share the same position
               {
                  center_nrm+=AbsAngleBetween(mshb.vtx.pos(p[(pi+vtxs-1)%vtxs]), mshb.vtx.pos(p[pi]), mshb.vtx.pos(p[(pi+1)%vtxs]))*dist.nrm;
               }
            }
         }
         if(DistPointPlane(point, center_pos, center_nrm)<=0)ret=true;
         goto end;
      }
   }

end:;
   if(distance)*distance=dist;
   return ret;
}
Bool CutsPointMesh(C Vec &point, C MeshLod &mesh, Flt *distance)
{
   Bool       ret=false;
   Flt        dist;
   Memt<DIST> dists;

   // get distances
   FREPA(mesh)FindClosest(point, mesh.parts[i].base, i, dist, dists);

   // check for closest plane
   FREPA(dists)
   {
      DIST &dist=dists[i];
      if(   dist.type==DIST_PLANE)
      {
         C MeshBase &mshb=mesh.parts[dist.part].base;
         if(dist.ind&SIGN_BIT) // quad
         {
          C Int *p =mshb.quad.ind(dist.ind^SIGN_BIT).c;
          C Vec &p0=mshb.vtx .pos(p[0]),
                &p1=mshb.vtx .pos(p[1]),
                &p2=mshb.vtx .pos(p[2]),
                &p3=mshb.vtx .pos(p[3]);
            Tri tri0(p0, p1, p3),
                tri1(p1, p2, p3);
            if(DistPointPlane(p2, tri0)<=0) // convex
            {
               ret=DistPointPlane(point, tri0)<=0
                && DistPointPlane(point, tri1)<=0;
            }
            else // concave
            {
               ret=DistPointPlane(point, tri0)<=0
                || DistPointPlane(point, tri1)<=0;
            }
         }
         else // tri
         {
            ret=(DistPointPlane(point, mshb.vtx.pos(mshb.tri.ind(dist.ind).c[0]), dist.nrm)<=0);
         }
         goto end;
      }
   }

   // check for closest edge
   {
      Bool edge_found=false;
      Flt  edge_angle_nearest,
           edge_angle_delta;
      FREPA(dists) // find the edge which 3rd point (loose) is within the smallest angle to the tested one
      {
         DIST &dist=dists[i];
         if(   dist.type>=DIST_EDGE0 && dist.type<=DIST_EDGE3)
         {
          C MeshBase &mshb=mesh.parts[dist.part].base;

          C Int *p;
            Int vtxs, ei=dist.type-DIST_EDGE0; // edge index
            if(dist.ind&SIGN_BIT){p=mshb.quad.ind(dist.ind^SIGN_BIT).c; vtxs=4;}
            else                 {p=mshb.tri .ind(dist.ind         ).c; vtxs=3;}

          C Vec &edge_p0=mshb.vtx.pos(p[ ei        ]),
                &edge_p1=mshb.vtx.pos(p[(ei+1)%vtxs]);

            Matrix m; m.setPosDir(edge_p0, !(edge_p1-edge_p0));

            Flt point_angle=AngleFast (point                       , m),
                 edge_angle=AngleFast (mshb.vtx.pos(p[(ei+2)%vtxs]), m),
                delta_angle=AngleDelta( edge_angle, point_angle),
            abs_delta_angle=Abs       (delta_angle);

            if(!edge_found || abs_delta_angle<edge_angle_nearest)
            {
               edge_found        =true;
               edge_angle_nearest=abs_delta_angle;
               edge_angle_delta  =    delta_angle;
            }
         }
      }
      if(edge_found)
      {
         if(edge_angle_delta<=0)ret=true;
         goto end;
      }
   }

   // check for closest vertex
   FREPA(dists)
   {
      DIST &dist=dists[i];
      if(   dist.type>=DIST_POINT0 && dist.type<=DIST_POINT3) // get first, set as center
      {
       C MeshBase &mshb=mesh.parts[dist.part].base;

       C Int *p;
         Int  pi=dist.type-DIST_POINT0; // point index
         if(dist.ind&SIGN_BIT)p=mshb.quad.ind(dist.ind^SIGN_BIT).c;
         else                 p=mshb.tri .ind(dist.ind         ).c;

       C Vec &center_pos=mshb.vtx.pos(p[pi]);
         Vec  center_nrm=0;

         for(Int j=i; j<dists.elms(); j++) // calculate average triangle normal from all vertexes that share same position with 'center'
         {
            DIST &dist=dists[j];
            if(   dist.type>=DIST_POINT0 && dist.type<=DIST_POINT3)
            {
               Int vtxs, pi=dist.type-DIST_POINT0;
               if(dist.ind&SIGN_BIT){p=mshb.quad.ind(dist.ind^SIGN_BIT).c; vtxs=4;}
               else                 {p=mshb.tri .ind(dist.ind         ).c; vtxs=3;}

               if(Equal(mshb.vtx.pos(p[pi]), center_pos)) // only for those which share the same position
               {
                  center_nrm+=AbsAngleBetween(mshb.vtx.pos(p[(pi+vtxs-1)%vtxs]), mshb.vtx.pos(p[pi]), mshb.vtx.pos(p[(pi+1)%vtxs]))*dist.nrm;
               }
            }
         }
         if(DistPointPlane(point, center_pos, center_nrm)<=0)ret=true;
         goto end;
      }
   }

end:;
   if(distance)*distance=dist;
   return ret;
}
/******************************************************************************/
Bool CutsPointMesh(C Vec2 &point, C Mesh &mesh, Flt *dist, UInt test_flag)
{
   if(Cuts(point, Rect(mesh.ext))){C MeshLod &lod=mesh; return CutsPointMesh(point, lod, dist, test_flag);}
   return false;
}
Bool CutsPointMesh(C Vec &point, C Mesh &mesh, Flt *dist)
{
   if(Cuts(point, mesh.ext)){C MeshLod &lod=mesh; return CutsPointMesh(point, lod, dist);}
   return false;
}
/******************************************************************************/
Bool CutsPointMesh(C Vec2 &point, C MeshGroup &mshg, UInt test_flag)
{
   if(Cuts(point, Rect(mshg.ext)))REPA(mshg)if(CutsPointMesh(point, mshg.meshes[i], null, test_flag))return true;
   return false;
}
Bool CutsPointMesh(C Vec &point, C MeshGroup &mshg)
{
   if(Cuts(point, mshg.ext))REPA(mshg)if(CutsPointMesh(point, mshg.meshes[i]))return true;
   return false;
}
/******************************************************************************/
// SWEEP
/******************************************************************************/
Bool Sweep(C Vec2 &point, C Vec2 &move, C MeshBase &mshb, Flt *hit_frac, Vec2 *hit_pos, Int *hit_edge)
{
 C Vec   *pos =mshb.vtx .pos();
 C Vec   *nrm =mshb.edge.nrm();
 C VecI2 *edge=mshb.edge.ind();
   if(pos && edge)
   {
      Bool hit=false;
      Int  edge_i;
      Flt  f , frac;
      Vec2 hp, hitp;
      FREPA(mshb.edge)
      {
       C Int *p=edge[i].c;
         if(SweepPointEdge(point, move, Edge2_I(pos[p[0]].xy, pos[p[1]].xy, nrm ? &nrm[i].xy : null), &f, null, &hp))
            if(!hit || f<frac){hit=true; frac=f; hitp=hp; edge_i=i;}
      }
      if(hit)
      {
         if(hit_frac)*hit_frac=frac;
         if(hit_pos )*hit_pos =hitp;
         if(hit_edge)*hit_edge=edge_i;
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Bool Sweep(C Vec2 &point, C Vec2 &move, C MeshBase &mshb, C Rects &rects, Index &rect_edge, Flt *hit_frac, Vec2 *hit_pos, Int *hit_edge)
{
 C Vec   *pos =mshb.vtx .pos();
 C Vec   *nrm =mshb.edge.nrm();
 C VecI2 *edge=mshb.edge.ind();
   if(pos && edge)
   {
      Byte  hit=0;
      Int   hit_dist2, edge_i;
      Flt   f , frac;
      Vec2  hp, hitp;
      Vec2  start =rects.coords(point     ),
            end   =rects.coords(point+move);
      VecI2 starti=Floor(start);

      for(PixelWalkerMask walker(start, end, RectI(0, 0, rects.cells.x-1, rects.cells.y-1)); walker.active(); walker.step())
      {
         if(hit && hit_dist2<Dist2(walker.pos(), starti))break;
       C IndexGroup &ig=rect_edge.group[walker.pos().x + walker.pos().y*rects.cells.x];
         REPA(ig)
         {
          C Int e=ig[i], *p=edge[e].c;
            if(SweepPointEdge(point, move, Edge2_I(pos[p[0]].xy, pos[p[1]].xy, nrm ? &nrm[e].xy : null), &f, null, &hp))
               if(!hit || f<frac){hit=2; frac=f; hitp=hp; edge_i=e;}
         }
         if(hit==2){hit=1; hit_dist2=Dist2(rects.coords(hitp), starti);}
      }
      if(hit)
      {
         if(hit_frac)*hit_frac=frac;
         if(hit_pos )*hit_pos =hitp;
         if(hit_edge)*hit_edge=edge_i;
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Bool Sweep(C Vec &point, C Vec &move, C MeshBase &mshb, C Matrix *mesh_matrix, Flt *hit_frac, Vec *hit_pos, Int *hit_face, Bool test_quads_as_2_tris, Bool two_sided)
{
   Bool hit=false;
   Int  face_index;
   Flt  f , frac;
   Vec  hp, hitp, Point=point, Move=move; if(mesh_matrix){Point/=*mesh_matrix; Move/=mesh_matrix->orn();}

 C Int   *p;
 C VecI  * tri_ind=mshb.tri .ind();
 C VecI4 *quad_ind=mshb.quad.ind();
 C Vec   * tri_nrm=mshb.tri .nrm();
 C Vec   *quad_nrm=mshb.quad.nrm();
 C Vec   * vtx_pos=mshb.vtx .pos();

   if(vtx_pos &&  tri_ind)REPA(mshb.tri ){p= tri_ind[i].c; if(SweepPointTriEps (Point, Move, Tri (vtx_pos[p[0]], vtx_pos[p[1]], vtx_pos[p[2]],                 tri_nrm ? & tri_nrm[i] : null), &f, &hp                      , two_sided))if(!hit || f<frac){hit=true; frac=f; face_index=i         ; hitp=hp;}}
   if(vtx_pos && quad_ind)REPA(mshb.quad){p=quad_ind[i].c; if(SweepPointQuadEps(Point, Move, Quad(vtx_pos[p[0]], vtx_pos[p[1]], vtx_pos[p[2]], vtx_pos[p[3]], quad_nrm ? &quad_nrm[i] : null), &f, &hp, test_quads_as_2_tris, two_sided))if(!hit || f<frac){hit=true; frac=f; face_index=i^SIGN_BIT; hitp=hp;}}

   if(hit)
   {
      if(hit_frac)*hit_frac=frac;
      if(hit_pos )*hit_pos =(mesh_matrix ? point+frac*move : hitp);
      if(hit_face)*hit_face=face_index;
      return true;
   }
   return false;
}
/******************************************************************************/
Bool Sweep(C Vec &point, C Vec &move, C MeshRender &mshr, C Matrix *mesh_matrix, Flt *hit_frac, Vec *hit_pos, Int *hit_face, Bool two_sided)
{
   Bool hit=false;
   Int  face_index;
   Flt  f , frac;
   Vec  hp, hitp, Point=point, Move=move; if(mesh_matrix){Point/=*mesh_matrix; Move/=mesh_matrix->orn();}

   Int pos_ofs=mshr.vtxOfs(VTX_POS);
   if( pos_ofs>=0)
      if(C Byte *vtx=mshr.vtxLockRead())
   {
      vtx+=pos_ofs;
      if(CPtr index=mshr.indLockRead())
      {
         Int p0, p1, p2;
         REP(mshr.tris())
         {
            if(mshr._ib.bit16()){U16 *ind=(U16*)index; p0=ind[i*3+0]; p1=ind[i*3+1]; p2=ind[i*3+2];}
            else                {U32 *ind=(U32*)index; p0=ind[i*3+0]; p1=ind[i*3+1]; p2=ind[i*3+2];}
            if(SweepPointTriEps(Point, Move, Tri(*(Vec*)(vtx+p0*mshr.vtxSize()), *(Vec*)(vtx+p1*mshr.vtxSize()), *(Vec*)(vtx+p2*mshr.vtxSize())), &f, &hp, two_sided))if(!hit || f<frac){hit=true; frac=f; face_index=i; hitp=hp;}
         }
         mshr.indUnlock();
      }
      mshr.vtxUnlock();
   }

   if(hit)
   {
      if(hit_frac)*hit_frac=frac;
      if(hit_pos )*hit_pos =(mesh_matrix ? point+frac*move : hitp);
      if(hit_face)*hit_face=face_index;
      return true;
   }
   return false;
}
/******************************************************************************/
Bool Sweep(C Vec &point, C Vec &move, C MeshPart &part, C Matrix *mesh_matrix, Flt *hit_frac, Vec *hit_pos, Int *hit_face, Bool test_quads_as_2_tris, Int two_sided, Bool only_visible)
{
   if(only_visible && (part.part_flag&MSHP_HIDDEN))return false;
   Bool ts=((two_sided<0) ? (part.material() ? !part.material()->cull : false) : (two_sided!=0));
   return part.base  .is() ? Sweep(point, move, part.base  , mesh_matrix, hit_frac, hit_pos, hit_face, test_quads_as_2_tris, ts) :
          part.render.is() ? Sweep(point, move, part.render, mesh_matrix, hit_frac, hit_pos, hit_face                      , ts) : false;
}
/******************************************************************************/
Bool Sweep(C Vec &point, C Vec &move, C MeshLod &mesh, C Matrix *mesh_matrix, Flt *hit_frac, Vec *hit_pos, Int *hit_face, Int *hit_part, Bool test_quads_as_2_tris, Int two_sided, Bool only_visible)
{
   Int fi, face_index, part_index;
   Flt f , frac;
   Vec hp, hitp, Point=point, Move=move; if(mesh_matrix){Point/=*mesh_matrix; Move/=mesh_matrix->orn();}

   Bool hit=false;
   REPA(mesh)
   {
    C MeshPart &part=mesh.parts[i];
      if(!(only_visible && (part.part_flag&MSHP_HIDDEN)))
      {
         Bool ts=((two_sided<0) ? (part.material() ? !part.material()->cull : false) : (two_sided!=0));
         if(part.base  .is() ? Sweep(Point, Move, part.base  , null, &f, &hp, &fi, test_quads_as_2_tris, ts) :
            part.render.is() ? Sweep(Point, Move, part.render, null, &f, &hp, &fi                      , ts) : false)
            if(!hit || f<frac){hit=true; frac=f; hitp=hp; face_index=fi; part_index=i;}
      }
   }
   if(hit)
   {
      if(hit_frac)*hit_frac=frac ;
      if(hit_pos )*hit_pos =(mesh_matrix ? point+frac*move : hitp);
      if(hit_face)*hit_face=face_index;
      if(hit_part)*hit_part=part_index;
      return true;
   }
   return false;
}
/******************************************************************************/
Bool Sweep(C Vec &point, C Vec &move, C Mesh &mesh, C Matrix *mesh_matrix, Flt *hit_frac, Vec *hit_pos, Int *hit_face, Int *hit_part, Bool test_quads_as_2_tris, Int two_sided, Bool only_visible)
{
   Vec Point=point, Move=move; if(mesh_matrix){Point/=*mesh_matrix; Move/=mesh_matrix->orn();}

   if(SweepPointBox(Point, Move, mesh.ext))
   {
      Flt frac; if(hit_pos && mesh_matrix && !hit_frac)hit_frac=&frac; // we will need it for 'hit_pos' calculation
      if(Sweep(Point, Move, mesh.lod(0), null, hit_frac, hit_pos, hit_face, hit_part, test_quads_as_2_tris, two_sided, only_visible))
      {
         if(hit_pos && mesh_matrix)*hit_pos=point+*hit_frac*move;
         return true;
      }
   }
   return false;
}
Bool Sweep(C VecD &point, C VecD &move, C Mesh &mesh, C MatrixM *mesh_matrix, Flt *hit_frac, VecD *hit_pos, Int *hit_face, Int *hit_part, Bool test_quads_as_2_tris, Int two_sided, Bool only_visible)
{
   Vec Point, Move; if(mesh_matrix){Point.fromDiv(point, *mesh_matrix); Move.fromDiv(move, mesh_matrix->orn());}else{Point=point; Move=move;}

   if(SweepPointBox(Point, Move, mesh.ext))
   {
      Flt frac; Vec hp;
      if(hit_pos && mesh_matrix && !hit_frac)hit_frac=&frac; // we will need it for 'hit_pos' calculation
      if(Sweep(Point, Move, mesh.lod(0), null, hit_frac, (hit_pos && !mesh_matrix) ? &hp : null, hit_face, hit_part, test_quads_as_2_tris, two_sided, only_visible))
      {
         if(hit_pos){if(mesh_matrix)*hit_pos=point+*hit_frac*move;else *hit_pos=hp;} // can't do "*hit_pos=mesh_matrix?:" because of different types and compile fail on Clang
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Bool Sweep(C Vec &point, C Vec &move, C MeshGroup &mshg, C Matrix *mesh_matrix, Flt *hit_frac, Vec *hit_pos, Int *hit_face, Int *hit_part, Int *hit_mesh, Bool test_quads_as_2_tris, Int two_sided, Bool only_visible)
{
   Int fi, face_index, part_index, mesh_index;
   Flt f , frac;
   Vec hp, hitp, Point=point, Move=move; if(mesh_matrix){Point/=*mesh_matrix; Move/=mesh_matrix->orn();}

   if(SweepPointBox(Point, Move, mshg.ext))
   {
      Bool hit=false;
      REPA(mshg)
      {
       C Mesh &mesh=mshg.meshes[i];
         if(SweepPointBox(Point, Move, mesh.ext))REPAD(j, mesh)
         {
          C MeshPart &part=mesh.parts[j];
            if(!(only_visible && (part.part_flag&MSHP_HIDDEN)))
            {
               Bool ts=((two_sided<0) ? (part.material() ? !part.material()->cull : false) : (two_sided!=0));
               if(part.base  .is() ? Sweep(Point, Move, part.base  , null, &f, &hp, &fi, test_quads_as_2_tris, ts) :
                  part.render.is() ? Sweep(Point, Move, part.render, null, &f, &hp, &fi                      , ts) : false)
                  if(!hit || f<frac){hit=true; frac=f; hitp=hp; face_index=fi; part_index=j; mesh_index=i;}
            }
         }
      }
      if(hit)
      {
         if(hit_frac)*hit_frac=frac;
         if(hit_pos )*hit_pos =(mesh_matrix ? point+frac*move : hitp);
         if(hit_face)*hit_face=face_index;
         if(hit_part)*hit_part=part_index;
         if(hit_mesh)*hit_mesh=mesh_index;
         return true;
      }
   }
   return false;
}
/******************************************************************************/
// POS
/******************************************************************************/
Bool PosPointMeshXL(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos, Int *hit_face,                   C Box *box, Bool test_quads_as_2_tris) {Box b=(box ? *box : mshb); return Sweep(Vec(     b  .max.x +1, point.y, point.x), Vec(-     b  .w()-2, 0, 0), mshb, null, null, hit_pos, hit_face                    , test_quads_as_2_tris);}
Bool PosPointMeshXL(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos, Int *hit_face, Int *hit_part               , Bool test_quads_as_2_tris) {                           return Sweep(Vec(mesh.ext.maxX()+1, point.y, point.x), Vec(-mesh.ext.w()-2, 0, 0), mesh, null, null, hit_pos, hit_face, hit_part          , test_quads_as_2_tris);}
Bool PosPointMeshXL(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos, Int *hit_face, Int *hit_part, Int *hit_mesh, Bool test_quads_as_2_tris) {                           return Sweep(Vec(mshg.ext.maxX()+1, point.y, point.x), Vec(-mshg.ext.w()-2, 0, 0), mshg, null, null, hit_pos, hit_face, hit_part, hit_mesh, test_quads_as_2_tris);}

Bool PosPointMeshXR(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos, Int *hit_face,                   C Box *box, Bool test_quads_as_2_tris) {Box b=(box ? *box : mshb); return Sweep(Vec(     b  .min.x -1, point.y, point.x), Vec(      b  .w()+2, 0, 0), mshb, null, null, hit_pos, hit_face                    , test_quads_as_2_tris);}
Bool PosPointMeshXR(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos, Int *hit_face, Int *hit_part               , Bool test_quads_as_2_tris) {                           return Sweep(Vec(mesh.ext.minX()-1, point.y, point.x), Vec( mesh.ext.w()+2, 0, 0), mesh, null, null, hit_pos, hit_face, hit_part          , test_quads_as_2_tris);}
Bool PosPointMeshXR(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos, Int *hit_face, Int *hit_part, Int *hit_mesh, Bool test_quads_as_2_tris) {                           return Sweep(Vec(mshg.ext.minX()-1, point.y, point.x), Vec( mshg.ext.w()+2, 0, 0), mshg, null, null, hit_pos, hit_face, hit_part, hit_mesh, test_quads_as_2_tris);}

Bool PosPointMeshY (C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos, Int *hit_face,                   C Box *box, Bool test_quads_as_2_tris) {Box b=(box ? *box : mshb); return Sweep(Vec(point.x,      b  .max.y +1, point.y), Vec(0, -     b  .h()-2, 0), mshb, null, null, hit_pos, hit_face                    , test_quads_as_2_tris);}
Bool PosPointMeshY (C Vec2 &point, C Mesh      &mesh, Vec *hit_pos, Int *hit_face, Int *hit_part               , Bool test_quads_as_2_tris) {                           return Sweep(Vec(point.x, mesh.ext.maxY()+1, point.y), Vec(0, -mesh.ext.h()-2, 0), mesh, null, null, hit_pos, hit_face, hit_part          , test_quads_as_2_tris);}
Bool PosPointMeshY (C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos, Int *hit_face, Int *hit_part, Int *hit_mesh, Bool test_quads_as_2_tris) {                           return Sweep(Vec(point.x, mshg.ext.maxY()+1, point.y), Vec(0, -mshg.ext.h()-2, 0), mshg, null, null, hit_pos, hit_face, hit_part, hit_mesh, test_quads_as_2_tris);}

Bool PosPointMeshZF(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos, Int *hit_face,                   C Box *box, Bool test_quads_as_2_tris) {Box b=(box ? *box : mshb); return Sweep(Vec(point.x, point.y,      b  .min.z -1), Vec(0, 0,       b  .d()+2), mshb, null, null, hit_pos, hit_face                    , test_quads_as_2_tris);}
Bool PosPointMeshZF(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos, Int *hit_face, Int *hit_part               , Bool test_quads_as_2_tris) {                           return Sweep(Vec(point.x, point.y, mesh.ext.minZ()-1), Vec(0, 0,  mesh.ext.d()+2), mesh, null, null, hit_pos, hit_face, hit_part          , test_quads_as_2_tris);}
Bool PosPointMeshZF(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos, Int *hit_face, Int *hit_part, Int *hit_mesh, Bool test_quads_as_2_tris) {                           return Sweep(Vec(point.x, point.y, mshg.ext.minZ()-1), Vec(0, 0,  mshg.ext.d()+2), mshg, null, null, hit_pos, hit_face, hit_part, hit_mesh, test_quads_as_2_tris);}

Bool PosPointMeshZB(C Vec2 &point, C MeshBase  &mshb, Vec *hit_pos, Int *hit_face,                   C Box *box, Bool test_quads_as_2_tris) {Box b=(box ? *box : mshb); return Sweep(Vec(point.x, point.y,      b  .max.z +1), Vec(0, 0, -     b  .d()-2), mshb, null, null, hit_pos, hit_face                    , test_quads_as_2_tris);}
Bool PosPointMeshZB(C Vec2 &point, C Mesh      &mesh, Vec *hit_pos, Int *hit_face, Int *hit_part               , Bool test_quads_as_2_tris) {                           return Sweep(Vec(point.x, point.y, mesh.ext.maxZ()+1), Vec(0, 0, -mesh.ext.d()-2), mesh, null, null, hit_pos, hit_face, hit_part          , test_quads_as_2_tris);}
Bool PosPointMeshZB(C Vec2 &point, C MeshGroup &mshg, Vec *hit_pos, Int *hit_face, Int *hit_part, Int *hit_mesh, Bool test_quads_as_2_tris) {                           return Sweep(Vec(point.x, point.y, mshg.ext.maxZ()+1), Vec(0, 0, -mshg.ext.d()-2), mshg, null, null, hit_pos, hit_face, hit_part, hit_mesh, test_quads_as_2_tris);}
/******************************************************************************/
// REST
/******************************************************************************/
Matrix GetRestMatrix(C Memt<C MeshBase*> &meshes, C Memt<C MeshRender*> &gpu_meshes, C Matrix *initial_matrix, C Vec *mass_center, Flt min_dist, Box *rest_box, Int max_steps)
{
   Matrix matrix; if(initial_matrix)matrix=*initial_matrix;else matrix.identity();
   MAX(min_dist, EPS);

   REPA(gpu_meshes)if(C MeshRender *mesh=gpu_meshes[i])mesh->vtxLockRead();

   {
      // find first contact point 'a' located at the bottom
      Bool found=false; Vec a_org, a;
      REPA(meshes)
         if(C MeshBase *mesh=meshes[i])
           REPA(mesh->vtx)
      {
       C Vec &p_org=mesh->vtx.pos(i); Vec p=p_org*matrix;
         if(!found || p.y<a.y)
         {
            found=true;
            a_org=p_org;
            a    =p;
         }
      }
      REPA(gpu_meshes)
         if(C MeshRender *mesh=gpu_meshes[i])
            if(C Byte *vtx_pos=mesh->vtxLockedElm(VTX_POS))
               REP(mesh->vtxs())
      {
       C Vec &p_org=*(Vec*)vtx_pos; vtx_pos+=mesh->vtxSize(); Vec p=p_org*matrix;
         if(!found || p.y<a.y)
         {
            found=true;
            a_org=p_org;
            a    =p;
         }
      }
      if(!found)goto end;

      matrix.pos.y-=a.y; a.y=0; // align to bottom
      if(!max_steps--)goto end;

      // find mass center
      Vec com_org; if(mass_center)com_org=*mass_center;else
      {
         Box  box;
         Bool found=false;
         REPA(meshes)if(C MeshBase *mesh=meshes[i])
         {
            Box mesh_box; if(mesh->getBox(mesh_box))
            {
               if(found)box|=mesh_box;else{found=true; box=mesh_box;}
            }
         }
         REPA(gpu_meshes)if(C MeshRender *mesh=gpu_meshes[i])
         {
            Box mesh_box; if(mesh->getBox(mesh_box))
            {
               if(found)box|=mesh_box;else{found=true; box=mesh_box;}
            }
         }
         if(!found)goto end;
         com_org=box.center();
      }
      Vec com=com_org*matrix; // get transformed center of mass

   find_b_contact:
      // get direction from contact point towards center of mass on XZ plane
      Vec axis=com-a; axis.y=0; if(!axis.normalize())axis.set(1, 0, 0);
      
      // get smallest slope from 'a' along the 'axis' which will be the second contact point 'b'
      found=false; Vec b_org; Flt slope_val;
      REPA(meshes)
         if(C MeshBase *mesh=meshes[i])
           REPA(mesh->vtx)
      {
       C Vec &p_org=mesh->vtx.pos(i); Vec p=p_org*matrix;
         if(!Equal(p, a, min_dist)) // ignore the original point or those close to it. Operate on 'p' and 'a' instead of 'p_org' and 'a_org' so we get correct scale for 'min_dist' in case 'matrix' scales
         {
            Vec d=p-a;
            Flt x=Dot(d, axis); if(x>0)
            {
               Flt y=d.y, slope=y/x;
               if(!found || slope<slope_val)
               {
                  found    =true;
                  slope_val=slope;
                  b_org    =p_org;
               }
            }
         }
      }
      REPA(gpu_meshes)
         if(C MeshRender *mesh=gpu_meshes[i])
            if(C Byte *vtx_pos=mesh->vtxLockedElm(VTX_POS))
               REP(mesh->vtxs())
      {
       C Vec &p_org=*(Vec*)vtx_pos; vtx_pos+=mesh->vtxSize(); Vec p=p_org*matrix;
         if(!Equal(p, a, min_dist)) // ignore the original point or those close to it. Operate on 'p' and 'a' instead of 'p_org' and 'a_org' so we get correct scale for 'min_dist' in case 'matrix' scales
         {
            Vec d=p-a;
            Flt x=Dot(d, axis); if(x>0)
            {
               Flt y=d.y, slope=y/x;
               if(!found || slope<slope_val)
               {
                  found    =true;
                  slope_val=slope;
                  b_org    =p_org;
               }
            }
         }
      }
      if(!found)goto end;

      Vec right=Perp(axis.xz()).x0y();
      matrix-=a; matrix.rotate(right, Atan(slope_val));
      matrix+=a;
        //a  =  a_org*matrix; // this matrix change does not affect 'a'
      Vec b  =  b_org*matrix; // calculate contact point
          com=com_org*matrix; // adjust com according to new matrix
      if(max_steps) // if want to keep looking
         if(slope_val>EPS) // if we have rotated (if not then it means that 2 contact points are coplanar and we can proceed with them)
            if(Dot(com, axis)>Dot(b, axis)+min_dist) // if mass center is further, then it means the body can still rotate. Operate on 'com' and 'b' instead of 'com_org' and 'b_org' so we get correct scale for 'min_dist' in case 'matrix' scales
      {
         max_steps--;
         a=b; a_org=b_org; // treat second contact point as the first one now
         goto find_b_contact;
      }

      // find third contact point
   find_c_contact:
       axis=b-a; Vec axis3=axis; axis3.normalize(); axis.y=0; if(!axis.normalize())axis.set(1, 0, 0); // we will rotate along the 'axis' now
      right=Perp(axis.xz()).x0y(); if(DistPointPlane(com, a, right)<0){axis.chs(); /*axis3.chs();*/ right.chs(); Swap(a, b); Swap(a_org, b_org);} // if center of mass is on the other side, 'axis3' sign is not important
      found=false; Vec c_org;
      REPA(meshes)
         if(C MeshBase *mesh=meshes[i])
           REPA(mesh->vtx)
      {
       C Vec &p_org=mesh->vtx.pos(i); Vec p=p_org*matrix;
       //if(!Equal(p, a, min_dist) && !Equal(p, b, min_dist)) // ignore the original points or those close to them, this check is not needed because of the one below
         if(DistPointStr(p, a, axis3)>min_dist) // ignore points on the same line. Operate on 'p' and 'a' instead of 'p_org' and 'a_org' so we get correct scale for 'min_dist' in case 'matrix' scales
         {
            Vec d=p-a;
            Flt x=Dot(d, right); if(x>0) // if point is on the same side as center of mass
            {
               Flt y=d.y, slope=y/x;
               if(!found || slope<slope_val)
               {
                  found    =true;
                  slope_val=slope;
                  c_org    =p_org;
               }
            }
         }
      }
      REPA(gpu_meshes)
         if(C MeshRender *mesh=gpu_meshes[i])
            if(C Byte *vtx_pos=mesh->vtxLockedElm(VTX_POS))
               REP(mesh->vtxs())
      {
       C Vec &p_org=*(Vec*)vtx_pos; vtx_pos+=mesh->vtxSize(); Vec p=p_org*matrix;
       //if(!Equal(p, a, min_dist) && !Equal(p, b, min_dist)) // ignore the original points or those close to them, this check is not needed because of the one below
         if(DistPointStr(p, a, axis3)>min_dist) // ignore points on the same line. Operate on 'p' and 'a' instead of 'p_org' and 'a_org' so we get correct scale for 'min_dist' in case 'matrix' scales
         {
            Vec d=p-a;
            Flt x=Dot(d, right); if(x>0) // if point is on the same side as center of mass
            {
               Flt y=d.y, slope=y/x;
               if(!found || slope<slope_val)
               {
                  found    =true;
                  slope_val=slope;
                  c_org    =p_org;
               }
            }
         }
      }
      if(!found)goto end;
      matrix-=a; matrix.rotate(axis, -Atan(slope_val));
      matrix+=a;
      if(max_steps) // want to keep looking further
      {
         max_steps--;

           //a=  a_org*matrix; // this matrix change does not affect 'a'
             b=  b_org*matrix;
         Vec c=  c_org*matrix;
           com=com_org*matrix;

         // we now have 3 contact points forming a triangle, and center of mass
         // check if the center of mass is outside of the triangle, which means the mesh will keep rotating
         DIST_TYPE dt; Flt dist=Dist(com.xz(), Tri2(a.xz(), b.xz(), c.xz()), &dt); if(dist>min_dist)
         {
            if(dt==DIST_EDGE1) // BC edge
            {
               a_org=c_org;
               a    =c    ;
               goto find_c_contact;
            }else
            if(dt==DIST_POINT0 || dt==DIST_EDGE2 || dt==DIST_POINT2) // A point || CA edge || C point
            {
               b_org=c_org;
               b    =c    ;
               goto find_c_contact;
            }
         }
      }
   }

end:
   if(rest_box)
   {
      Bool found=false;
      REPA(meshes)
         if(C MeshBase *mesh=meshes[i])
           REPA(mesh->vtx)
      {
       C Vec &p_org=mesh->vtx.pos(i); Vec p=p_org*matrix;
         if(found)(*rest_box)|=p;else{found=true; (*rest_box)=p;}
      }
      REPA(gpu_meshes)
         if(C MeshRender *mesh=gpu_meshes[i])
            if(C Byte *vtx_pos=mesh->vtxLockedElm(VTX_POS))
               REP(mesh->vtxs())
      {
       C Vec &p_org=*(Vec*)vtx_pos; vtx_pos+=mesh->vtxSize(); Vec p=p_org*matrix;
         if(found)(*rest_box)|=p;else{found=true; (*rest_box)=p;}
      }
      if(!found)rest_box->zero();
   }

   REPA(gpu_meshes)if(C MeshRender *mesh=gpu_meshes[i])mesh->vtxUnlock();

   return matrix;
}
Matrix GetRestMatrix(C MeshBase &mesh, C Matrix *initial_matrix, C Vec *mass_center, Flt min_dist, Box *rest_box, Int max_steps)
{
   Memt<C MeshBase  *>     meshes; meshes.add(&mesh);
   Memt<C MeshRender*> gpu_meshes;
   return GetRestMatrix(meshes, gpu_meshes, initial_matrix, mass_center, min_dist, rest_box, max_steps);
}
Matrix GetRestMatrix(C MemPtr<C MeshPart*> &meshes, C Matrix *initial_matrix, C Vec *mass_center, Flt min_dist, Box *rest_box, Int max_steps, Bool only_visible, Bool only_phys)
{
   Memt<C MeshBase  *>     temp;
   Memt<C MeshRender*> gpu_temp;
   REPA(meshes)
      if(C MeshPart *part=meshes[i])
   {
      if(!only_visible || !FlagTest(part->part_flag, MSHP_HIDDEN))
      if(!only_phys    || !FlagTest(part->part_flag, MSHP_NO_PHYS_BODY))
         if(part->base  .is())    temp.add(&part->base  );else
         if(part->render.is())gpu_temp.add(&part->render);
   }
   return GetRestMatrix(temp, gpu_temp, initial_matrix, mass_center, min_dist, rest_box, max_steps);
}
Matrix GetRestMatrix(C MemPtr<C MeshLod*> &meshes, C Matrix *initial_matrix, C Vec *mass_center, Flt min_dist, Box *rest_box, Int max_steps, Bool only_visible, Bool only_phys)
{
   Memt<C MeshPart*> parts;
   REPA(meshes)
      if(C MeshLod *mesh=meshes[i])
         REPA(*mesh)parts.add(&mesh->parts[i]);
   return GetRestMatrix(parts, initial_matrix, mass_center, min_dist, rest_box, max_steps, only_visible, only_phys);
}
Matrix GetRestMatrix(C MeshLod &mesh, C Matrix *initial_matrix, C Vec *mass_center, Flt min_dist, Box *rest_box, Int max_steps, Bool only_visible, Bool only_phys)
{
 C MeshLod *mesh_ptr=&mesh;
   return GetRestMatrix(mesh_ptr, initial_matrix, mass_center, min_dist, rest_box, max_steps, only_visible, only_phys);
}
Matrix GetRestMatrix(C Mesh &mesh, C Matrix *initial_matrix, C Vec *mass_center, Flt min_dist, Box *rest_box, Int max_steps, Bool only_visible, Bool only_phys)
{
   return GetRestMatrix(mesh.lod(0), initial_matrix, mass_center ? mass_center : &mesh.ext.pos, min_dist, rest_box, max_steps, only_visible, only_phys);
}
/******************************************************************************/
}
/******************************************************************************/
