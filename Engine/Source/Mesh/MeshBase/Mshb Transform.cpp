/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
MeshBase& MeshBase::move     (              C Vec &move) {if(Vec *pos=vtx.pos())REPA(vtx)       (*pos++)+=           move;  if(Vec *pos=vtx.hlp())REPA(vtx)       (*pos++)+=           move;  return T;}
MeshBase& MeshBase::scale    (C Vec &scale             ) {if(Vec *pos=vtx.pos())REPA(vtx)       (*pos++)*=   scale       ;  if(Vec *pos=vtx.hlp())REPA(vtx)       (*pos++)*=   scale       ;  return T;}
MeshBase& MeshBase::scaleMove(C Vec &scale, C Vec &move) {if(Vec *pos=vtx.pos())REPA(vtx){Vec &p=*pos++; p=p*scale + move;} if(Vec *pos=vtx.hlp())REPA(vtx){Vec &p=*pos++; p=p*scale + move;} return T;}
MeshBase& MeshBase::setSize  (C Box &box               )
{
   Box b=T;
   Vec d=b.size(),
       scale((d.x>EPS) ? box.w()/d.x : 0,
             (d.y>EPS) ? box.h()/d.y : 0,
             (d.z>EPS) ? box.d()/d.z : 0),
       move=box.min-b.min*scale;
   return scaleMove(scale, move);
}
MeshBase& MeshBase::transform(C Matrix3 &matrix)
{
   Int     n;
   Matrix3 matrix_n=matrix; matrix_n.inverseScale();

   n=vtxs (); Transform(vtx .pos(), matrix  , n);
              Transform(vtx .hlp(), matrix  , n);
              Transform(vtx .nrm(), matrix_n, n); Normalize(vtx .nrm(), n);
              Transform(vtx .tan(), matrix_n, n); Normalize(vtx .tan(), n);
              Transform(vtx .bin(), matrix_n, n); Normalize(vtx .bin(), n);

   n=edges(); Transform(edge.nrm(), matrix_n, n); Normalize(edge.nrm(), n);
   n=tris (); Transform(tri .nrm(), matrix_n, n); Normalize(tri .nrm(), n);
   n=quads(); Transform(quad.nrm(), matrix_n, n); Normalize(quad.nrm(), n);

   return T;
}
MeshBase& MeshBase::transform(C Matrix &matrix)
{
   Int     n;
   Matrix3 matrix_n=matrix; matrix_n.inverseScale();

   n=vtxs (); Transform(vtx .pos(), matrix  , n);
              Transform(vtx .hlp(), matrix  , n);
              Transform(vtx .nrm(), matrix_n, n); Normalize(vtx .nrm(), n);
              Transform(vtx .tan(), matrix_n, n); Normalize(vtx .tan(), n);
              Transform(vtx .bin(), matrix_n, n); Normalize(vtx .bin(), n);

   n=edges(); Transform(edge.nrm(), matrix_n, n); Normalize(edge.nrm(), n);
   n=tris (); Transform(tri .nrm(), matrix_n, n); Normalize(tri .nrm(), n);
   n=quads(); Transform(quad.nrm(), matrix_n, n); Normalize(quad.nrm(), n);

   return T;
}
MeshBase& MeshBase::animate(C MemPtrN<Matrix, 256> &matrixes)
{
   if(matrixes.elms())
   {
      Vec   *pos  =vtx.pos   (),
            *nrm  =vtx.nrm   (),
            *tan  =vtx.tan   (),
            *bin  =vtx.bin   (),
            *hlp  =vtx.hlp   (), v;
    C VecB4 *blend=vtx.blend (),
            *mtrx =vtx.matrix();

      if(blend && mtrx)REPA(vtx)
      {
       C VecB4  &bi=*blend++; Vec4 b=bi; b/=255.0f; b.w=1-b.xyz.sum();
       C Matrix &m0=(InRange(mtrx->c[0], matrixes) ? matrixes[mtrx->c[0]] : matrixes[0]),
                &m1=(InRange(mtrx->c[1], matrixes) ? matrixes[mtrx->c[1]] : matrixes[0]),
                &m2=(InRange(mtrx->c[2], matrixes) ? matrixes[mtrx->c[2]] : matrixes[0]),
                &m3=(InRange(mtrx->c[3], matrixes) ? matrixes[mtrx->c[3]] : matrixes[0]); mtrx++;
         if(pos){v=*pos; *pos=b.x*(v*m0      ) + b.y*(v*m1      ) + b.z*(v*m2      ) + b.w*(v*m3      );                   pos++;}
         if(hlp){v=*hlp; *hlp=b.x*(v*m0      ) + b.y*(v*m1      ) + b.z*(v*m2      ) + b.w*(v*m3      );                   hlp++;}
         if(nrm){v=*nrm; *nrm=b.x*(v*m0.orn()) + b.y*(v*m1.orn()) + b.z*(v*m2.orn()) + b.w*(v*m3.orn()); nrm->normalize(); nrm++;}
         if(tan){v=*tan; *tan=b.x*(v*m0.orn()) + b.y*(v*m1.orn()) + b.z*(v*m2.orn()) + b.w*(v*m3.orn()); tan->normalize(); tan++;}
         if(bin){v=*bin; *bin=b.x*(v*m0.orn()) + b.y*(v*m1.orn()) + b.z*(v*m2.orn()) + b.w*(v*m3.orn()); bin->normalize(); bin++;}
      }
   }
   return T;
}
MeshBase& MeshBase::animate(C AnimatedSkeleton &anim_skel)
{
   MemtN<Matrix, 256> matrixes;
   anim_skel.getMatrixes(matrixes);
   animate(matrixes);
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::mirrorX()
{
   ChsX(vtx .pos(), vtxs ());
   ChsX(vtx .hlp(), vtxs ());
   ChsX(vtx .nrm(), vtxs ());
   ChsX(vtx .tan(), vtxs ());
   ChsX(vtx .bin(), vtxs ());
   ChsX(edge.nrm(), edges());
   ChsX(tri .nrm(), tris ());
   ChsX(quad.nrm(), quads());

   Reverse(edge.ind(), edges());
   Reverse(tri .ind(), tris ());
   SwapXZ (quad.ind(), quads()); // use 'SwapXZ' to preserve the same triangles being generated, but flipped, this is important, because 'Reverse' would cause different triangle combination

   if(vtx.tan() && !vtx.bin())setBinormals(); // binormals are needed if tangents are present
   return T;
}
MeshBase& MeshBase::mirrorY()
{
   ChsY(vtx .pos(), vtxs ());
   ChsY(vtx .hlp(), vtxs ());
   ChsY(vtx .nrm(), vtxs ());
   ChsY(vtx .tan(), vtxs ());
   ChsY(vtx .bin(), vtxs ());
   ChsY(edge.nrm(), edges());
   ChsY(tri .nrm(), tris ());
   ChsY(quad.nrm(), quads());

   Reverse(edge.ind(), edges());
   Reverse(tri .ind(), tris ());
   SwapXZ (quad.ind(), quads()); // use 'SwapXZ' to preserve the same triangles being generated, but flipped, this is important, because 'Reverse' would cause different triangle combination

   if(vtx.tan() && !vtx.bin())setBinormals(); // binormals are needed if tangents are present
   return T;
}
MeshBase& MeshBase::mirrorZ()
{
   ChsZ(vtx .pos(), vtxs ());
   ChsZ(vtx .hlp(), vtxs ());
   ChsZ(vtx .nrm(), vtxs ());
   ChsZ(vtx .tan(), vtxs ());
   ChsZ(vtx .bin(), vtxs ());
   ChsZ(edge.nrm(), edges());
   ChsZ(tri .nrm(), tris ());
   ChsZ(quad.nrm(), quads());

   Reverse(edge.ind(), edges());
   Reverse(tri .ind(), tris ());
   SwapXZ (quad.ind(), quads()); // use 'SwapXZ' to preserve the same triangles being generated, but flipped, this is important, because 'Reverse' would cause different triangle combination

   if(vtx.tan() && !vtx.bin())setBinormals(); // binormals are needed if tangents are present
   return T;
}
MeshBase& MeshBase::rightToLeft()
{
   RightToLeft(vtx .pos(), vtxs ());
   RightToLeft(vtx .hlp(), vtxs ());
   RightToLeft(vtx .nrm(), vtxs ());
   RightToLeft(vtx .tan(), vtxs ());
   RightToLeft(vtx .bin(), vtxs ());
   RightToLeft(edge.nrm(), edges());
   RightToLeft(tri .nrm(), tris ());
   RightToLeft(quad.nrm(), quads());

   Reverse(edge.ind(), edges());
   Reverse(tri .ind(), tris ());
   SwapXZ (quad.ind(), quads()); // use 'SwapXZ' to preserve the same triangles being generated, but flipped, this is important, because 'Reverse' would cause different triangle combination

   if(vtx.tan() && !vtx.bin())setBinormals(); // binormals are needed if tangents are present
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::reverse()
{
                   Chs    (vtx .nrm(), vtxs ());
   if(!edge.flag())Chs    (edge.nrm(), edges());
                   Chs    (tri .nrm(), tris ());
                   Chs    (quad.nrm(), quads());
                   Reverse(edge.ind(), edges());
                   Reverse(tri .ind(), tris ());
                   SwapXZ (quad.ind(), quads()); // use 'SwapXZ' to preserve the same triangles being generated, but flipped, this is important, because 'Reverse' would cause different triangle combination

   if(vtx.tan() && !vtx.bin())setBinormals(); // binormals are needed if tangents are present
   return T;
}
MeshBase& MeshBase::reverse(Int face)
{
   if(face&SIGN_BIT)
   {
      face^=SIGN_BIT;
      if(InRange(face, quad))
      {
                       quad.ind(face).swapXZ(); // use 'swapXZ' to preserve the same triangles being generated, but flipped, this is important, because 'reverse' would cause different triangle combination
         if(quad.nrm())quad.nrm(face).chs   ();
      }
   }else
   {
      if(InRange(face, tri))
      {
                      tri.ind(face).reverse();
         if(tri.nrm())tri.nrm(face).chs    ();
      }
   }
   return T;
}
MeshBase& MeshBase::reverse(C MemPtr<Int> &faces)
{
   if(faces.elms())
   {
      Memt<Bool> tri_is, quad_is; CreateFaceIs(tri_is, quad_is, faces, tris(), quads());
      MeshBase temp; splitFaces(temp, null, tri_is, quad_is);
      if(temp.is())
      {
         T+=temp.reverse();
         weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down
      }
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
