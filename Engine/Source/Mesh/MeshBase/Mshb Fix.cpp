/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************
MeshBase& fixSolid      (Bool swapped           ); // fix edge winding according to ETQ_FLAG
MeshBase& MeshBase::fixSolid(Bool swapped)
{
   if(edge.ind() && edge.flag())
   {
      exclude(ADJ_ALL);
      Memt<Bool> is; is.setNum(edges());
      FREPA(edge)
      {
         Byte &flag=edge.flag(i);
         if(flag&ETQ_NO_PHYS || (flag&ETQ_LR)==ETQ_LR || !(flag&ETQ_LR))is[i]=false;else
         {
            is[i]=true;
            if(swapped){if(flag&ETQ_L) edge.ind(i).reverse();else flag=(flag&(~ETQ_LR))|ETQ_L; }
            else       {if(flag&ETQ_R){edge.ind(i).reverse();     flag=(flag&(~ETQ_LR))|ETQ_L;}}
         }
      }
      keepEdges(is);
   }
   return T;
}
/******************************************************************************
enum WINDING_TYPE
{
   WINDING_DUNNO,
   WINDING_OK   ,
   WINDING_WRONG,
};
struct FixWindingClass
{
   Byte  * tri_flag;
   Byte  *quad_flag;
   VecI  * tri_ind ;
   VecI4 *quad_ind ;
   Index * vtx_face;
};
static void FixWinding(FixWindingClass &fw, Int face, Bool ok)
{
   Int *p, *tp, vrts, tvrts, tface;
   if(face&SIGN_BIT)
   {
        fw.quad_flag[face^SIGN_BIT]=(ok ? WINDING_OK : WINDING_WRONG);
      p=fw.quad_ind [face^SIGN_BIT].c;
      vrts=4;
   }else
   {
        fw.tri_flag[face]=(ok ? WINDING_OK : WINDING_WRONG);
      p=fw.tri_ind [face].c;
      vrts=3;
   }
   REP(vrts)
   {
      Int v   =p[ i        ],
          next=p[(i+1)%vrts];
      IndexGroup &ig=fw.vtx_face->group[v];
      REPAD(f, ig)
      {
         tface=ig[f];
         if(tface&SIGN_BIT){if(fw.quad_flag[tface^SIGN_BIT])continue; tp=fw.quad_ind[tface^SIGN_BIT].c; tvrts=4;}else
                           {if(fw. tri_flag[tface         ])continue; tp=fw. tri_ind[tface         ].c; tvrts=3;}
         REPD(tn, tvrts)if(tp[tn]==v)
         {
            if(tp[(tn+tvrts-1)%tvrts]==next)FixWinding(fw, tface,  ok);else // tprev==next
            if(tp[(tn      +1)%tvrts]==next)FixWinding(fw, tface, !ok);     // tnext==next
            break;
         }
      }
   }
}
MeshBase& fixWinding    (Int  face, Bool ok=true); // recursive fix of adjacent faces winding, according to given 'face' index and if it's winded correctly
MeshBase& MeshBase::fixWinding(Int face, Bool ok)
{
   if((face&SIGN_BIT) ? InRange(face^SIGN_BIT, quad) : InRange(face, tri))
   {
      Byte * tri_flag=AllocZero<Byte>(tris ()),
           *quad_flag=AllocZero<Byte>(quads());

      // link vtx->face
      Index vtx_face; linkVtxFace(vtx_face);

      // fix winding
      FixWindingClass fw;
      fw. tri_flag= tri_flag;
      fw.quad_flag=quad_flag;
      fw. tri_ind=  tri.ind();
      fw.quad_ind= quad.ind();
      fw.vtx_face= &vtx_face;
      FixWinding(fw, face, ok);
      FREPA(tri )if( tri_flag[i]==WINDING_WRONG)tri .ind(i).reverse();
      FREPA(quad)if(quad_flag[i]==WINDING_WRONG)quad.ind(i).reverse();

      // free
      Free( tri_flag);
      Free(quad_flag);
   }
   return T;
}
/******************************************************************************/
struct VTX
{
   Int dup;
   Flt tex_min, tex_max;

   void init(Flt tex)
   {
      dup=0;
      tex_min=tex_max=tex;
   }
};
MeshBase& MeshBase::fixTexWrapping(Byte tex_index)
{
   if(InRange(tex_index, 3))
      if(Vec2 *tex=((tex_index==0) ? vtx.tex0() : (tex_index==1) ? vtx.tex1() : vtx.tex2()))
   {
      Int *p, p0, p1, p2, p3, j;
      Memt<VTX> vs; vs.setNum(vtxs());

      // get tex range
      Flt x;
      FREPA(vtx)vs[i].init(tex[i].x);
      FREPA(tri)
      {
         p=tri.ind(i).c; p0=p[0]; p1=p[1]; p2=p[2];
         x=tex[p0].x; if(x<vs[p1].tex_min)vs[p1].tex_min=x;else if(x>vs[p1].tex_max)vs[p1].tex_max=x;
                      if(x<vs[p2].tex_min)vs[p2].tex_min=x;else if(x>vs[p2].tex_max)vs[p2].tex_max=x;
         x=tex[p1].x; if(x<vs[p0].tex_min)vs[p0].tex_min=x;else if(x>vs[p0].tex_max)vs[p0].tex_max=x;
                      if(x<vs[p2].tex_min)vs[p2].tex_min=x;else if(x>vs[p2].tex_max)vs[p2].tex_max=x;
         x=tex[p2].x; if(x<vs[p0].tex_min)vs[p0].tex_min=x;else if(x>vs[p0].tex_max)vs[p0].tex_max=x;
                      if(x<vs[p1].tex_min)vs[p1].tex_min=x;else if(x>vs[p1].tex_max)vs[p1].tex_max=x;
      }
      FREPA(quad)
      {
         p=quad.ind(i).c; p0=p[0]; p1=p[1]; p2=p[2]; p3=p[3];
         x=tex[p0].x; if(x<vs[p1].tex_min)vs[p1].tex_min=x;else if(x>vs[p1].tex_max)vs[p1].tex_max=x;
                      if(x<vs[p2].tex_min)vs[p2].tex_min=x;else if(x>vs[p2].tex_max)vs[p2].tex_max=x;
                      if(x<vs[p3].tex_min)vs[p3].tex_min=x;else if(x>vs[p3].tex_max)vs[p3].tex_max=x;
         x=tex[p1].x; if(x<vs[p0].tex_min)vs[p0].tex_min=x;else if(x>vs[p0].tex_max)vs[p0].tex_max=x;
                      if(x<vs[p2].tex_min)vs[p2].tex_min=x;else if(x>vs[p2].tex_max)vs[p2].tex_max=x;
                      if(x<vs[p3].tex_min)vs[p3].tex_min=x;else if(x>vs[p3].tex_max)vs[p3].tex_max=x;
         x=tex[p2].x; if(x<vs[p0].tex_min)vs[p0].tex_min=x;else if(x>vs[p0].tex_max)vs[p0].tex_max=x;
                      if(x<vs[p1].tex_min)vs[p1].tex_min=x;else if(x>vs[p1].tex_max)vs[p1].tex_max=x;
                      if(x<vs[p3].tex_min)vs[p3].tex_min=x;else if(x>vs[p3].tex_max)vs[p3].tex_max=x;
         x=tex[p3].x; if(x<vs[p0].tex_min)vs[p0].tex_min=x;else if(x>vs[p0].tex_max)vs[p0].tex_max=x;
                      if(x<vs[p1].tex_min)vs[p1].tex_min=x;else if(x>vs[p1].tex_max)vs[p1].tex_max=x;
                      if(x<vs[p2].tex_min)vs[p2].tex_min=x;else if(x>vs[p2].tex_max)vs[p2].tex_max=x;
      }
      // get odd vtxs (tex.range >= 2/3)
      Int dups=0;
      FREPA(vtx)if(vs[i].tex_max-vs[i].tex_min>=2.0f/3)vs[i].dup=vtxs()+(dups++);

      // copy mshb
      {
         MeshBase temp(vtxs()+dups, 0, 0, 0, flag()); temp.copyVtxs(T);
         tex=((tex_index==0) ? temp.vtx.tex0() : (tex_index==1) ? temp.vtx.tex1() : temp.vtx.tex2());
         FREPA(vtx)if(vs[i].dup)
         {
            j=vs[i].dup; copyVtx(i, temp, j);
            Int d=Trunc(vs[i].tex_max-vs[i].tex_min+2.0f/3);
            if(Abs(tex[j].x-vs[i].tex_max)<Abs(tex[i].x-vs[i].tex_min))tex[i].x-=d;else tex[j].x+=d;
         }
         Swap(vtx, temp.vtx);
      }

      // correct indexing to nearest tex.coord
      FREPA(tri)
      {
         p=tri.ind(i).c; p0=p[0]; p1=p[1]; p2=p[2];
         if(j=vs[p0].dup)
         {
            if(!vs[p1].dup){if(Abs(tex[p1].x-tex[j].x)<Abs(tex[p1].x-tex[p0].x))p[0]=j;}else
            if(!vs[p2].dup){if(Abs(tex[p2].x-tex[j].x)<Abs(tex[p2].x-tex[p0].x))p[0]=j;}
         }
         if(j=vs[p1].dup)
         {
            if(!vs[p0].dup){if(Abs(tex[p0].x-tex[j].x)<Abs(tex[p0].x-tex[p1].x))p[1]=j;}else
            if(!vs[p2].dup){if(Abs(tex[p2].x-tex[j].x)<Abs(tex[p2].x-tex[p1].x))p[1]=j;}
         }
         if(j=vs[p2].dup)
         {
            if(!vs[p0].dup){if(Abs(tex[p0].x-tex[j].x)<Abs(tex[p0].x-tex[p2].x))p[2]=j;}else
            if(!vs[p1].dup){if(Abs(tex[p1].x-tex[j].x)<Abs(tex[p1].x-tex[p2].x))p[2]=j;}
         }
      }
      FREPA(quad)
      {
         p=quad.ind(i).c; p0=p[0]; p1=p[1]; p2=p[2]; p3=p[3];
         if(j=vs[p0].dup)
         {
            if(!vs[p1].dup){if(Abs(tex[p1].x-tex[j].x)<Abs(tex[p1].x-tex[p0].x))p[0]=j;}else
            if(!vs[p2].dup){if(Abs(tex[p2].x-tex[j].x)<Abs(tex[p2].x-tex[p0].x))p[0]=j;}else
            if(!vs[p3].dup){if(Abs(tex[p3].x-tex[j].x)<Abs(tex[p3].x-tex[p0].x))p[0]=j;}
         }
         if(j=vs[p1].dup)
         {
            if(!vs[p0].dup){if(Abs(tex[p0].x-tex[j].x)<Abs(tex[p0].x-tex[p1].x))p[1]=j;}else
            if(!vs[p2].dup){if(Abs(tex[p2].x-tex[j].x)<Abs(tex[p2].x-tex[p1].x))p[1]=j;}else
            if(!vs[p3].dup){if(Abs(tex[p3].x-tex[j].x)<Abs(tex[p3].x-tex[p1].x))p[1]=j;}
         }
         if(j=vs[p2].dup)
         {
            if(!vs[p0].dup){if(Abs(tex[p0].x-tex[j].x)<Abs(tex[p0].x-tex[p2].x))p[2]=j;}else
            if(!vs[p1].dup){if(Abs(tex[p1].x-tex[j].x)<Abs(tex[p1].x-tex[p2].x))p[2]=j;}else
            if(!vs[p3].dup){if(Abs(tex[p3].x-tex[j].x)<Abs(tex[p3].x-tex[p2].x))p[2]=j;}
         }
         if(j=vs[p3].dup)
         {
            if(!vs[p0].dup){if(Abs(tex[p0].x-tex[j].x)<Abs(tex[p0].x-tex[p3].x))p[3]=j;}else
            if(!vs[p1].dup){if(Abs(tex[p1].x-tex[j].x)<Abs(tex[p1].x-tex[p3].x))p[3]=j;}else
            if(!vs[p2].dup){if(Abs(tex[p2].x-tex[j].x)<Abs(tex[p2].x-tex[p3].x))p[3]=j;}
         }
      }
   }
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::fixTexOffset(Byte tex_index)
{
   if(InRange(tex_index, 3))
      if(Vec2 *tex=((tex_index==0) ? vtx.tex0() : (tex_index==1) ? vtx.tex1() : vtx.tex2()))
   {
      exclude(VTX_DUP);

      Index vtx_vtx; linkVtxVtxOnFace(vtx_vtx); // link vertexes together
      Memt<Int , 16384> mvtx;
      Memt<Int , 16384> madd;
      Memt<Bool, 32768> vtx_is; vtx_is.setNumZero(vtxs());

      REPA(vtx)if(!vtx_is[i]) // process all vertexes
      {
         // get list of all vertexes that are connected together
         mvtx.clear();
         madd.add(i); vtx_is[i]=true;
         for(; madd.elms(); )
         {
            Int vtx=madd.pop();
            mvtx.add(vtx);
            IndexGroup &ig=vtx_vtx.group[vtx];
            REPA(ig)
            {
               Int v=ig[i]; if(!vtx_is[v])
               {
                  madd.add(v);
                  vtx_is[v]=true;
               }
            }
         }
         // calculate their average texture coordinates
         if(mvtx.elms())
         {
            Vec2 min, max; min=max=tex[mvtx.last()];
            REP(mvtx.elms()-1)
            {
             C Vec2 &t=tex[mvtx[i]];
               if(t.x<min.x)min.x=t.x;else if(t.x>max.x)max.x=t.x;
               if(t.y<min.y)min.y=t.y;else if(t.y>max.y)max.y=t.y;
            }
            VecI2 center=Floor(Avg(min, max)); // always try to fit UV's in 0..1, even if the Average is -0.1 which translates to -1..0 range, because material atlas generation tests for 0..1 range
            if(center.any())
            {
               Vec2 offset=-center; REPA(mvtx)tex[mvtx[i]]+=offset; // apply offset
            }
         }
      }
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
