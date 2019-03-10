/******************************************************************************/
#include "stdafx.h"

#define ALLOW_VTX_MOVE    2 // 0=off, 1=based on edge center, 2=edge center and custom quadric calculation
#define ALLOW_PLANES      1 // 0=off, 1=store multiple planes per each vertex
#define ADJUST_REFS       1 // if adjust references for all vertexes when deleting a triangle (if not then 'tri.exists' checks are performed)
#define TEST_ORIGINAL_POS 1 // if test original position against the new triangle (or if false, test the new position against the old triangle)
#define DEBUG_CHECK_REFS  0 // if perform debug checks that the references have been properly adjusted
#define PROFILE           0 // if perform profiling, measuring which functions take how much time to complete

#define VTX_NORMAL_QM 2.0f // value of 2 was chosen based on test results (it gave better quality)
#define VTX_BORDER_QM 1.0f

#if PROFILE
#pragma message("!! Warning: This will slow down mesh simplification, use only for testing !!")
enum PROFILE_TYPE
{
   ADD,
   SIMPLIFY,
   INIT,
   FLIPPED,
   CALC_ERROR,
   RESET_ERROR,
   SET_VTX_DATA,
   ADD_VTX_DATA,
   TEST_TRIS,
   SET_NRM_TAN_BIN,
   STORE,
   PROFILE_NUM,
};
static Dbl Times[PROFILE_NUM];
static CChar8 *Names[]=
{
   "Add",
   "Simplify",
   " Init",
   " Flipped",
   " Calc Error",
   " Reset Error",
   " Set Vtx Data",
   " Add Vtx Data",
   " Test Tris",
   " Set Nrm Tan Bin",
   "Store",
};
struct Profile
{
   Int type;
   Dbl time;

   void   end() {Times[type]+=Time.curTime()-time;}
   void start() {time=Time.curTime();}

   Profile(Int type) {T.type=type; start();}
  ~Profile() {end();}
};
void ProfileEnd()
{
   Str s;
   FREPA(Times)s.line()+=S+Names[i]+": "+Times[i]+'s';
   Exit(s);
}
#define PROF(x) Profile p(x);
#define PROFILE_END ProfileEnd();
#else
   #define PROF(x)
   #define PROFILE_END
#endif
/******************************************************************************

   Part of the algorithm is based on "Quadric Mesh Simplification" by Sven Forstmann:
      http://voxels.blogspot.com.au/2014/05/quadric-mesh-simplification-with-source.html

/******************************************************************************/
namespace EE{
/******************************************************************************/
typedef Dbl Real; // 'QuadricMatrix' requires high precision
/******************************************************************************/
struct QuadricMatrix
{
   // http://www.bmsc.washington.edu/people/merritt/graphics/quadrics.html
   // F(x, y, z) = Ax2 + By2 + Cz2 + Dxy + Exz + Fyz + Gx + Hy + Iz + J = 0
   union
   {
      struct{Real m[10];};
      struct{Real a, d, e, g, b, f, h, c, i, j;}; // list in this order to match the above formula and 'error2' method
   };

   QuadricMatrix() {}
   QuadricMatrix(Real m11, Real m12, Real m13, Real m14,
                           Real m22, Real m23, Real m24,
                                     Real m33, Real m34,
                                               Real m44)
   {
      m[0]=m11; m[1]=m12; m[2]=m13; m[3]=m14;
                m[4]=m22; m[5]=m23; m[6]=m24;
                          m[7]=m33; m[8]=m34;
                                    m[9]=m44;
   }
   QuadricMatrix(C Vec &normal, Real plane_dist) // QuadricMatrix(n, d) == QuadricMatrix(-n, -d), because each component is multiplied by 1 other, and because a*b == (-a)*(-b), which means that plane normal sign is not important for 'QuadricMatrix'
   {
      Real x=normal.x, y=normal.y, z=normal.z, w=plane_dist;
      m[0]=x*x; m[1]=x*y; m[2]=x*z; m[3]=x*w;
                m[4]=y*y; m[5]=y*z; m[6]=y*w;
                          m[7]=z*z; m[8]=z*w;
                                    m[9]=w*w;
   }
   QuadricMatrix(C Vec &normal, C Vec &pos) : QuadricMatrix(normal, -Dot(normal, pos)) {} // QuadricMatrix(n, p) == QuadricMatrix(-n, p)

   // get
   Real operator[](Int i)C {return m[i];}

   Real det(Int a11, Int a12, Int a13,
            Int a21, Int a22, Int a23,
            Int a31, Int a32, Int a33)C // Determinant
   {
      Real det=m[a11]*m[a22]*m[a33] + m[a13]*m[a21]*m[a32] + m[a12]*m[a23]*m[a31] 
              -m[a13]*m[a22]*m[a31] - m[a11]*m[a23]*m[a32] - m[a12]*m[a21]*m[a33];
      return det;
   }

   Real error (C Vec &pos)C {return SqrtS(error2(pos));} // get         error between position and Quadric (can be negative)
   Real error2(C Vec &pos)C                              // get squared error between position and Quadric (can be negative)
   {
      Real x=pos.x, y=pos.y, z=pos.z;
      return m[0]*x*x + 2*m[1]*x*y + 2*m[2]*x*z + 2*m[3]*x
                      +   m[4]*y*y + 2*m[5]*y*z + 2*m[6]*y
                                     + m[7]*z*z + 2*m[8]*z
                                                +   m[9]  ;
   }
   Vec normal (C Vec &pos)C {return !normalU(pos);} // calculate quadric surface              normal at 'pos' position
   Vec normalU(C Vec &pos)C                         // calculate quadric surface unnormalized normal at 'pos' position
   {
   /* equal to partial derivatives of F with respect to x, y, z, which is (dF/dx, dF/dy, dF/dz)
      something like:
      Real d=0.000001, f=error(pos);
      return Vec((error(pos+Vec(d, 0, 0))-f)/d,
                 (error(pos+Vec(0, d, 0))-f)/d,
                 (error(pos+Vec(0, 0, d))-f)/d); */

      Real x=pos.x, y=pos.y, z=pos.z;
   #if 0 // formula assuming we don't use 2* in 'error2'
      return Vec(2*a*x + d*y + e*z + g,
                 2*b*y + d*x + f*z + h,
                 2*c*z + e*x + f*y + i);
   #elif 0 // formula assuming we use 2* in 'error2'
      return Vec(2*(a*x + d*y + e*z + g),
                 2*(b*y + d*x + f*z + h),
                 2*(c*z + e*x + f*y + i));
   #else // formula assuming we use 2* in 'error2' and with "2*" removed here, because this normal will be normalized
      return Vec(a*x + d*y + e*z + g,
                 b*y + d*x + f*z + h,
                 c*z + e*x + f*y + i);
   #endif
   }

   // operations
   void reset() {REPAO(m)=0;}

   QuadricMatrix operator+(C QuadricMatrix &n)C
   { 
      return QuadricMatrix(m[0]+n[0], m[1]+n[1], m[2]+n[2], m[3]+n[3],
                                      m[4]+n[4], m[5]+n[5], m[6]+n[6],
                                                 m[7]+n[7], m[8]+n[8],
                                                            m[9]+n[9]);
   }
   QuadricMatrix operator*(Real r)C
   { 
      return QuadricMatrix(m[0]*r, m[1]*r, m[2]*r, m[3]*r,
                                   m[4]*r, m[5]*r, m[6]*r,
                                           m[7]*r, m[8]*r,
                                                   m[9]*r);
   }
   QuadricMatrix& operator+=(C QuadricMatrix &n)
   {
      m[0]+=n[0]; m[1]+=n[1]; m[2]+=n[2]; m[3]+=n[3];
                  m[4]+=n[4]; m[5]+=n[5]; m[6]+=n[6];
                              m[7]+=n[7]; m[8]+=n[8];
                                          m[9]+=n[9];
      return T;
   }
   QuadricMatrix& operator*=(Real r) {REPAO(m)*=r; return T;}
   QuadricMatrix& operator/=(Real r) {REPAO(m)/=r; return T;}
};
/******************************************************************************/
struct Simplify // must be used for a single 'simplify', after that it cannot be used (because the members aren't cleared)
{
   struct Weights
   {
      Flt hlp, tex0, tex1, tex2, color, material, size; // no need to keep weight for pos (managed separately), nrm (will be normalized), skin (weights stored per bone/matrix)

      Weights() {}
      Weights(Flt weight)
      {
         hlp     =weight;
         tex0    =weight;
         tex1    =weight;
         tex2    =weight;
         color   =weight;
         material=weight;
         size    =weight;
      }
   };
   struct VtxData
   {
      Vec   hlp, nrm;
      Vec2  tex0, tex1, tex2;
      Vec4  color, material;
      VecB4 matrix, blend;
      Flt   size;

      void from(C MeshBase &mshb, Int i)
      {
         VtxFull vtx; vtx.from(mshb, i);
         hlp=vtx.hlp;
         nrm=vtx.nrm;
         tex0=vtx.tex0;
         tex1=vtx.tex1;
         tex2=vtx.tex2;
         color=vtx.color.asVec4();
         material=vtx.material; material/=255;
         matrix=vtx.matrix;
         blend=vtx.blend;
         size=vtx.size;
      }
      void to(MeshBase &mshb, Int i)
      {
         VtxFull vtx;
         vtx.hlp=hlp;
         vtx.nrm=nrm;
         vtx.tex0=tex0;
         vtx.tex1=tex1;
         vtx.tex2=tex2;
         vtx.color=color;
         if(Flt sum=material.sum())material/=sum; Color mc=material; vtx.material.set(mc.r, mc.g, mc.b, mc.a);
         vtx.matrix=matrix;
         vtx.blend=blend;
         vtx.size=size;
         vtx.to(mshb, i);
      }
      void lerp(C VtxData &a, C VtxData &b, Flt step, UInt flag)
      {
         Flt step1=1-step;
         if(flag&VTX_TEX0    )tex0    =  a.tex0    *step1 + b.tex0    *step;
         if(flag&VTX_COLOR   )color   =  a.color   *step1 + b.color   *step;
         if(flag&VTX_MATERIAL)material=  a.material*step1 + b.material*step;
         if(flag&VTX_NRM     )nrm     =!(a.nrm     *step1 + b.nrm     *step);
         if(flag&VTX_SKIN    )
         {
            MemtN<IndexWeight, 256> skin;
            FREPA(a.matrix)skin.New().set(a.matrix.c[i], a.blend.c[i]*step1);
            FREPA(b.matrix)skin.New().set(b.matrix.c[i], b.blend.c[i]*step );
            SetSkin(skin, matrix, blend, null);
         }
         if(flag&(VTX_HLP|VTX_TEX1|VTX_TEX2|VTX_SIZE)) // uncommon
         {
            if(flag&VTX_HLP )hlp =a.hlp *step1 + b.hlp *step;
            if(flag&VTX_SIZE)size=a.size*step1 + b.size*step;
            if(flag&VTX_TEX1)tex1=a.tex1*step1 + b.tex1*step;
            if(flag&VTX_TEX2)tex2=a.tex2*step1 + b.tex2*step;
         }
      }
      void lerp(C VtxData &a, C VtxData &b, C VtxData &c, C Vec &blend, UInt flag)
      {
         if(flag&VTX_TEX0    )tex0    =  a.tex0    *blend.x + b.tex0    *blend.y + c.tex0    *blend.z;
         if(flag&VTX_COLOR   )color   =  a.color   *blend.x + b.color   *blend.y + c.color   *blend.z;
         if(flag&VTX_MATERIAL)material=  a.material*blend.x + b.material*blend.y + c.material*blend.z;
         if(flag&VTX_NRM     )nrm     =!(a.nrm     *blend.x + b.nrm     *blend.y + c.nrm     *blend.z);
         if(flag&VTX_SKIN    )
         {
            MemtN<IndexWeight, 256> skin;
            FREPA(a.matrix)skin.New().set(a.matrix.c[i], a.blend.c[i]*blend.x);
            FREPA(b.matrix)skin.New().set(b.matrix.c[i], b.blend.c[i]*blend.y);
            FREPA(c.matrix)skin.New().set(c.matrix.c[i], c.blend.c[i]*blend.z);
            SetSkin(skin, T.matrix, T.blend, null);
         }
         if(flag&(VTX_HLP|VTX_TEX1|VTX_TEX2|VTX_SIZE)) // uncommon
         {
            if(flag&VTX_HLP )hlp =a.hlp *blend.x + b.hlp *blend.y + c.hlp *blend.z;
            if(flag&VTX_SIZE)size=a.size*blend.x + b.size*blend.y + c.size*blend.z;
            if(flag&VTX_TEX1)tex1=a.tex1*blend.x + b.tex1*blend.y + c.tex1*blend.z;
            if(flag&VTX_TEX2)tex2=a.tex2*blend.x + b.tex2*blend.y + c.tex2*blend.z;
         }
      }
      void normalize(C Weights &weight, UInt flag)
      {
         // skin not handled here
         if(flag&VTX_NRM     )nrm.normalize();
         if(flag&VTX_TEX0    )tex0    /=weight.tex0;
         if(flag&VTX_COLOR   )color   /=weight.color;
         if(flag&VTX_MATERIAL)material/=weight.material;
         if(flag&(VTX_HLP|VTX_TEX1|VTX_TEX2|VTX_SIZE)) // uncommon
         {
            if(flag&VTX_HLP )hlp /=weight.hlp;
            if(flag&VTX_TEX1)tex1/=weight.tex1;
            if(flag&VTX_TEX2)tex2/=weight.tex2;
            if(flag&VTX_SIZE)size/=weight.size;
         }
      }
   };
   struct Triangle
   {
      Real    edge_error[3], error_min;
      VecI    ind; // vertex index
      Vec     nrm, tan, bin; // for simplification tangent/binormal are computed per triangle (not per vertex), tangent/binormal are not normalized
      Flt     weight;
      VtxData vtxs[3], // data for each vertex
              vtx_new; // weighted average 'vtx_mid' for all tris, this is done per-component only if in 2 tris the shared vertexes data is the same
      Int     part, // MeshPart index
              mtrl_group;
      UInt    flag; // MSHB_FLAG, this is obtained from the MeshPart.base of that triangle
      Bool    middle;
   #if !ADJUST_REFS
      Bool    exists; // if references are not adjusted, then we need to keep information about which triangle exists and which got removed
      Triangle() {exists=true ;}
     ~Triangle() {exists=false;}
   #endif
   };
   struct TrianglePtr
   {
      Triangle *tri;
      VtxData   vtx_mid, // data interpolated for the middle position and then multiplied by 'weight'
                // following 'vtx_*' members specify source triangle to copy from
               *vtx_hlp,
               *vtx_nrm,
               *vtx_tex0,
               *vtx_tex1,
               *vtx_tex2,
               *vtx_skin,
               *vtx_color,
               *vtx_material,
               *vtx_size;

      TrianglePtr& set(Triangle &tri) {T.tri=&tri; return T;}

      void clear() {vtx_hlp=vtx_nrm=vtx_tex0=vtx_tex1=vtx_tex2=vtx_skin=vtx_color=vtx_material=vtx_size=null;}
   };
   enum BORDER
   {
      BORDER_POS=1<<0,
      BORDER_TEX=1<<1,
   };
   struct Vertex
   {
      Vec           pos;
      Int           ref_start, tri_num;
      Byte          border;
      QuadricMatrix qm;
      Real          weight;
   #if ALLOW_PLANES
      struct Plane
      {
         Vec normal;
         Flt plane ;

         void scale(Flt scale) {normal*=scale; plane*=scale;}

         Flt error(C Vec &pos)C {return Abs(Dot(pos, normal)+plane);}

         #define EPS_SIMPLIFY_PLANE 0.001f
         Bool operator==(C Plane &src)C {return Abs(plane-src.plane)<=EPS_SIMPLIFY_PLANE && Dot(normal, src.normal)>=EPS_COL_COS;} // we could also check for T==-src (if they are co-planar), because normal sign of these planes is not important, however most likely planes will have the same sign, if they don't, then in worst case they will be listed twice, but it's better to don't check to avoid extra CPU overhead, because most likely they won't be
      };
      Memc<Plane> planes;

      void includePlane(C Plane &plane)
      {
         REPA(planes)if(planes[i]==plane)return; // if plane is similar, then don't add it
         planes.add(plane);
      }
   #endif
      void eat(Vertex &vtx)
      {
         border|=vtx.border;
         qm    +=vtx.qm;
         weight+=vtx.weight;

      #if ALLOW_PLANES
         if(vtx.planes.elms()>planes.elms())Swap(vtx.planes, planes); // if 'vtx' uses more planes, then reuse its memory container, to avoid memory allocations
         REPA(vtx.planes)includePlane(vtx.planes[i]);
         vtx.planes.del(); // will no longer be needed
      #endif
      }
   };
   struct Ref
   {
      Int tri_abs      , // absolute index of the triangle
          tri_vtx_index; // 0..2 (1 of 3 vertexes in triangle)
   };
   struct VtxDupMap : VtxDup
   {
      Int  vtxs_index;
      Vec2 tex0;
      Int  mtrl_group;
   };
   struct MtrlGroup
   {
      UInt flag;
      Ptr  multi_mtrls[4];

      MtrlGroup(UInt flag=0, C MeshPart *part=null)
      {
         T.flag=flag;
         REPAO(multi_mtrls)=(part ? part->multiMaterial(i)() : null);
      }
      Bool operator==(C MtrlGroup &mg)C
      {
         const UInt mask=(VTX_POS|VTX_HLP|VTX_NRM|VTX_TEX_ALL|VTX_SIZE); // tan/bin not needed because they are recalculated, color not needed because defaulted to WHITE, material blend weight not needed because defaulted to (1,0,0,0), skin not needed because defaulted to bone=0 weight=1
         if((flag&mask)!=(mg.flag&mask))return false;
         REPA(multi_mtrls)if(multi_mtrls[i]!=mg.multi_mtrls[i])return false;
         return true;
      }
      Bool testAndMerge(C MtrlGroup &mg)
      {
         if(T==mg)
         {
            flag|=mg.flag;
            return true;
         }
         return false;
      }
   };

   Bool               keep_border;
   MESH_SIMPLIFY      mode;
   Int                processed_tris, max_skin;
   UInt               test_flag; // this is set based on the max tolerance parameters to the simplify function
   Flt                max_uv2, max_color, max_material, max_normal;
   Box                box;
   Memx<Triangle    > tris;
   Memc<Vertex      > vtxs;
   Memc<Ref         > refs;
   Memc<VtxDupMap   > vtx_dups;
   Memb<TrianglePtr >    all_tris; // need 'Memb' because we're adding new elements and storing pointers
   Memc<TrianglePtr*> middle_tris; // usually this will be just 2 triangles, however it may be more
   Memc<MtrlGroup   > mtrl_groups;
   Bool             *_stop;

   Simplify(Bool *stop) : _stop(stop) {}
   Bool stop()C {return _stop && *_stop;}

   static Int CompareError(C Triangle &a, C Triangle &b) {return Compare(b.error_min, a.error_min);} // swap order to sort from highest to lowest

   // error for one edge
#if ALLOW_PLANES
   static Flt PlanesError(C Vertex &vtx0, C Vertex &vtx1, C Vec &pos) // this is linear
   {
      Flt error=0;
      REPA(vtx0.planes)MAX(error, vtx0.planes[i].error(pos));
      REPA(vtx1.planes)MAX(error, vtx1.planes[i].error(pos));
      return error;
   }
#endif
   Real calculateError(Int v0, Int v1, Vec &mid_pos) // TODO: make error calculation optional?
   {
      PROF(CALC_ERROR);
      // compute interpolated vertex
    C Vertex &vtx0=vtxs[v0],
             &vtx1=vtxs[v1];
      QuadricMatrix qm=vtx0.qm    +vtx1.qm    ;
      Real      weight=vtx0.weight+vtx1.weight, error, det;
      if(keep_border) // if we want to keep border, and one of the vertexes is at the border, then we can merge only to that vtx (this method won't be called if both vertexes are at the border)
      {
         if(vtx0.border&BORDER_POS){mid_pos=vtx0.pos; goto calc_error;}
         if(vtx1.border&BORDER_POS){mid_pos=vtx1.pos; goto calc_error;}
      }
      det=qm.det(0, 1, 2, 1, 4, 5, 2, 5, 7);
      if(det && ALLOW_VTX_MOVE>=2) // q_delta is invertible, calculate a new mid position
      {
         mid_pos.x=-1/det*qm.det(1, 2, 3, 4, 5, 6, 5, 7, 8); // x=A41/det(q_delta)
         mid_pos.y= 1/det*qm.det(0, 2, 3, 1, 5, 6, 2, 7, 8); // y=A42/det(q_delta)
         mid_pos.z=-1/det*qm.det(0, 1, 3, 1, 4, 6, 2, 5, 8); // z=A43/det(q_delta)

      #if 0 // currently this is disabled, because we already adjust 'qm' and 'planes' for border vtx's, however if restored, then it needs to be analyzed, if there's no better way to do this (for example, case when vtx0 is BORDER_TEX, but vtx1 not, meaning that some other triangle with vtx0 but not with this edge will have tex surface stretched), and should it check for BORDER_POS/BORDER_TEX only/both?
         if(vtx0.border&vtx1.border) // if this is a border (tex or pos) then set the mid_pos on the line between the vertexes, to avoid moving them too much, which could introduce UV artifacts, do this also for BORDER_POS because edge middle pos based on quadrics could go completely outside of polygon range - having a /\ triangle, it could turn into /-----/
            mid_pos=NearestPointOnEdge(mid_pos, vtx0.pos, vtx1.pos); // this gave better results than "NearestPointOnStr(mid_pos, vtx0.pos, !(vtx1.pos-vtx0.pos))"
      #endif

      calc_error:
      #if ALLOW_PLANES
         if(mode==SIMPLIFY_PLANES)error=PlanesError(vtx0, vtx1, mid_pos);else
      #endif
            error=qm.error2(mid_pos)/weight; // since the 'qm' is scaled by triangle surface area, we need to restore it back to get normal scale
      }else // det=0, reuse existing position, find best result (one with smallest error)
      {
         Int error_min;
         mid_pos=Avg(vtx0.pos, vtx1.pos);
      #if ALLOW_PLANES
         if(mode==SIMPLIFY_PLANES)
         {
            Flt errors[]={PlanesError(vtx0, vtx1, vtx0.pos),
                          PlanesError(vtx0, vtx1, vtx1.pos),
                          PlanesError(vtx0, vtx1,  mid_pos)};
            error_min=(ALLOW_VTX_MOVE ? MinI(errors[0], errors[1], errors[2]) : MinI(errors[0], errors[1]));
            error    =errors[error_min];
         }else
      #endif
         {
            Real errors[]={qm.error2(vtx0.pos),
                           qm.error2(vtx1.pos),
                           qm.error2( mid_pos)};
            error_min=(ALLOW_VTX_MOVE ? MinI(errors[0], errors[1], errors[2]) : MinI(errors[0], errors[1]));
            error    =errors[error_min]/weight; // since the 'qm' is scaled by triangle surface area, we need to restore it back to get normal scale
         }
         switch(error_min)
         {
            case 0: mid_pos=vtx0.pos; break;
            case 1: mid_pos=vtx1.pos; break;
          //case 2: mid_pos= mid_pos; break; this is NOP
         }
      }
      return error;
   }

   // check if triangle flips when this edge is removed
   Bool flipped(C Vec &mid_pos, Int edge_vtx1i, C Vertex &edge_vtx0, Bool add)
   {
      PROF(FLIPPED);
      REP(edge_vtx0.tri_num)
      {
       C Ref      &ref=refs[edge_vtx0.ref_start+i];
         Triangle &tri=tris.absElm(ref.tri_abs);
      #if !ADJUST_REFS
         if(tri.exists)
      #endif
         {
            Int tvi   =ref.tri_vtx_index,
              //tri_v0=tri.ind.c[ tvi     ], this is always 'edge_vtx0i'
                tri_v1=tri.ind.c[(tvi+1)%3],
                tri_v2=tri.ind.c[(tvi+2)%3];

            if(tri.middle=(tri_v1==edge_vtx1i || tri_v2==edge_vtx1i)) // this is a middle tri
            {
               if(add)middle_tris.add(&all_tris.New().set(tri)); // we're adding middle tris only for the first vertex, to avoid adding middle triangles twice, from both vertexes
            }else
            {
             C Vec //&v0=   edge_vtx0.pos,
                     &v1=vtxs[tri_v1].pos,
                     &v2=vtxs[tri_v2].pos;
               if(Dot(tri.nrm, GetNormalU(mid_pos, v1, v2))<=0)return true; // if the new triangle normal is in the opposite direction of the original normal
               all_tris.New().set(tri);
            }
         }
      }
      return false;
   }

   static inline void AddSkin(MemtN<IndexWeight, 256> &skin, C VecB4 &matrix, C VecB4 &blend, Flt weight)
   {
      REPA(matrix){Flt w=blend.c[i]*weight; if(w>0)skin.New().set(matrix.c[i], w);}
   }
   static Bool EqualSkin(C VecB4 &m0, C VecB4 &w0, C VecB4 &m1, C VecB4 &w1)
   {
      return m0==m1 && w0==w1; // Warning: normally we should check every matrix index separately, in case for example m0.x==m1.y (different order, and that would be much slower), however 'SetSkin' was designed to sort matrix/weight so they can be compared in this fast way
   }
   static inline Int GetBlend(Byte index, VecB4 matrix, VecB4 blend) // find blend value for 'index' in 'matrix' 'blend' skinning
   {
      Int    b=0; REPA(matrix)if(matrix.c[i]==index)b+=blend.c[i];
      return b;
   }
   static Int SkinDifference(VecB4 matrix_a, VecB4 blend_a, VecB4 matrix_b, VecB4 blend_b)
   {
      Int dif=0;
      REP(4)
      {
         if(Byte b=blend_a.c[i])MAX(dif, Abs(b-GetBlend(matrix_a.c[i], matrix_b, blend_b))); // get difference between i-th A bone
         if(Byte b=blend_b.c[i])MAX(dif, Abs(b-GetBlend(matrix_b.c[i], matrix_a, blend_a))); // get difference between i-th B bone
      }
      return dif;
   }
   void addVtxData(TrianglePtr &trip, Int v0, Int v1)
   {
      PROF(ADD_VTX_DATA);

      Triangle &tri=*trip.tri;

      // start with self
      Weights weight(tri.weight); MemtN<IndexWeight, 256> skin;
      tri.vtx_new=trip.vtx_mid; // 'vtx_mid' is already multiplied by 'tri.weight'
      if(tri.flag&VTX_SKIN)AddSkin(skin, trip.vtx_mid.matrix, trip.vtx_mid.blend, tri.weight);

      // check all tris
      REPA(all_tris) // iterate all tris
      {
         TrianglePtr &trip2=all_tris[i]; if(&trip!=&trip2) // skip self, remember that 'tri2' can be middle too, just like 'tri'
         {
            Triangle &tri2=*trip2.tri;
            UInt add_flag=(tri.flag&tri2.flag);
            if(  add_flag&VTX_TEX_ALL) // if we want to merge tex coordinates
            {
               if(tri.mtrl_group!=tri2.mtrl_group // we can merge tex only if materials match
               || Dot(tri.tan, tri2.tan)<=0  // and triangles don't have mirrored tex
               || Dot(tri.bin, tri2.bin)<=0) // and triangles don't have mirrored tex
                  FlagDisable(add_flag, VTX_TEX_ALL); // can't merge tex
            }
            if(tri.mtrl_group!=tri2.mtrl_group)FlagDisable(add_flag, VTX_MATERIAL); // we can merge material only if materials match
            // check if shared vertexes match, remember that both 'tri' and 'tri2' can be middle, in which case, they will share 2 vertexes and not 1, so check all shared
            REPAD(tvi, tri.ind)
            {
               const Int tv=tri.ind.c[tvi];
               if(tv==v0 || tv==v1) // test only edge vertexes, this is to ignore other shared vertexes between the tris which are not on the edge to be removed, this will allow to simplify the mesh more
                  REPAD(tvi2, tri2.ind)
               {
                  const Int tv2=tri2.ind.c[tvi2];
                  if(tv==tv2) // if the same vtx
                  {
                   C VtxData &v=tri.vtxs[tvi], &v2=tri2.vtxs[tvi2];
                     // if any of the vtx components isn't equal in both triangles, then we can't merge
                     if((add_flag&VTX_NRM     ) && !Equal    (v.nrm     , v2.nrm                    ))FlagDisable(add_flag, VTX_NRM     );
                     if((add_flag&VTX_TEX0    ) && !Equal    (v.tex0    , v2.tex0                   ))FlagDisable(add_flag, VTX_TEX0    );
                     if((add_flag&VTX_COLOR   ) && !Equal    (v.color   , v2.color                  ))FlagDisable(add_flag, VTX_COLOR   );
                     if((add_flag&VTX_MATERIAL) && !Equal    (v.material, v2.material               ))FlagDisable(add_flag, VTX_MATERIAL);
                     if((add_flag&VTX_SKIN    ) && !EqualSkin(v.matrix, v.blend, v2.matrix, v2.blend))FlagDisable(add_flag, VTX_SKIN    );
                     if( add_flag&(VTX_HLP|VTX_TEX1|VTX_TEX2|VTX_SIZE)) // uncommon
                     {
                        if((add_flag&VTX_HLP ) && !Equal(v.hlp , v2.hlp ))FlagDisable(add_flag, VTX_HLP );
                        if((add_flag&VTX_TEX1) && !Equal(v.tex1, v2.tex1))FlagDisable(add_flag, VTX_TEX1);
                        if((add_flag&VTX_TEX2) && !Equal(v.tex2, v2.tex2))FlagDisable(add_flag, VTX_TEX2);
                        if((add_flag&VTX_SIZE) && !Equal(v.size, v2.size))FlagDisable(add_flag, VTX_SIZE);
                     }
                  }
               }
            }
            // merge
            if(add_flag&VTX_NRM     ){tri.vtx_new.nrm     +=trip2.vtx_mid.nrm     ; trip2.vtx_nrm     =&tri.vtx_new;}
            if(add_flag&VTX_TEX0    ){tri.vtx_new.tex0    +=trip2.vtx_mid.tex0    ; trip2.vtx_tex0    =&tri.vtx_new; weight.tex0    +=tri2.weight;}
            if(add_flag&VTX_COLOR   ){tri.vtx_new.color   +=trip2.vtx_mid.color   ; trip2.vtx_color   =&tri.vtx_new; weight.color   +=tri2.weight;}
            if(add_flag&VTX_MATERIAL){tri.vtx_new.material+=trip2.vtx_mid.material; trip2.vtx_material=&tri.vtx_new; weight.material+=tri2.weight;}
            if(add_flag&VTX_SKIN    ){AddSkin(skin, trip2.vtx_mid.matrix, trip2.vtx_mid.blend, tri2.weight); trip2.vtx_skin=&tri.vtx_new;}
            if(add_flag&(VTX_HLP|VTX_TEX1|VTX_TEX2|VTX_SIZE)) // uncommon
            {
               if(add_flag&VTX_HLP ){tri.vtx_new.hlp +=trip2.vtx_mid.hlp ; trip2.vtx_hlp =&tri.vtx_new; weight.hlp +=tri2.weight;}
               if(add_flag&VTX_TEX1){tri.vtx_new.tex1+=trip2.vtx_mid.tex1; trip2.vtx_tex1=&tri.vtx_new; weight.tex1+=tri2.weight;}
               if(add_flag&VTX_TEX2){tri.vtx_new.tex2+=trip2.vtx_mid.tex2; trip2.vtx_tex2=&tri.vtx_new; weight.tex2+=tri2.weight;}
               if(add_flag&VTX_SIZE){tri.vtx_new.size+=trip2.vtx_mid.size; trip2.vtx_size=&tri.vtx_new; weight.size+=tri2.weight;}
            }
         }
      }
      // normalize
      tri.vtx_new.normalize(weight, tri.flag);
      SetSkin(skin, tri.vtx_new.matrix, tri.vtx_new.blend, null);
   }
   Bool setVtxData(TrianglePtr &trip, Int v0, Int v1)
   {
      PROF(SET_VTX_DATA);
      Triangle &tri=*trip.tri;
      if(tri.middle)return true; // middle tris have already been calculated
      UInt set_flag=tri.flag; // get what we need to calculate
      if( (set_flag&VTX_TEX0) && !trip.vtx_tex0)return false; // if this triangle has TEX0 but we haven't found any middle triangle match, then we can't proceed
      // try getting from middle triangle if we've got a connection (prefer middle because this is already calculated, and from all triangles, not just one side)
      if(trip.vtx_hlp     ){tri.vtx_new.hlp     =trip.vtx_hlp     ->hlp     ;                                         FlagDisable(set_flag, VTX_HLP     );}
      if(trip.vtx_nrm     ){tri.vtx_new.nrm     =trip.vtx_nrm     ->nrm     ;                                         FlagDisable(set_flag, VTX_NRM     );}
      if(trip.vtx_tex0    ){tri.vtx_new.tex0    =trip.vtx_tex0    ->tex0    ;                                         FlagDisable(set_flag, VTX_TEX0    );}
      if(trip.vtx_tex1    ){tri.vtx_new.tex1    =trip.vtx_tex1    ->tex1    ;                                         FlagDisable(set_flag, VTX_TEX1    );}
      if(trip.vtx_tex2    ){tri.vtx_new.tex2    =trip.vtx_tex2    ->tex2    ;                                         FlagDisable(set_flag, VTX_TEX2    );}
      if(trip.vtx_skin    ){tri.vtx_new.matrix  =trip.vtx_skin    ->matrix  ; tri.vtx_new.blend=trip.vtx_skin->blend; FlagDisable(set_flag, VTX_SKIN    );}
      if(trip.vtx_color   ){tri.vtx_new.color   =trip.vtx_color   ->color   ;                                         FlagDisable(set_flag, VTX_COLOR   );}
      if(trip.vtx_material){tri.vtx_new.material=trip.vtx_material->material;                                         FlagDisable(set_flag, VTX_MATERIAL);}
      if(trip.vtx_size    ){tri.vtx_new.size    =trip.vtx_size    ->size    ;                                         FlagDisable(set_flag, VTX_SIZE    );}
      if(set_flag&(VTX_HLP|VTX_NRM|VTX_TEX_ALL|VTX_SKIN|VTX_COLOR|VTX_MATERIAL|VTX_SIZE)) // if we still have something to process
      {
         Weights weight; MemtN<IndexWeight, 256> skin;
         // start with self
         if(set_flag&VTX_HLP     ){tri.vtx_new.hlp     =trip.vtx_mid.hlp     ; weight.hlp     =tri.weight;}
         if(set_flag&VTX_NRM     ){tri.vtx_new.nrm     =trip.vtx_mid.nrm     ;}
         if(set_flag&VTX_TEX0    ){tri.vtx_new.tex0    =trip.vtx_mid.tex0    ; weight.tex0    =tri.weight;}
         if(set_flag&VTX_TEX1    ){tri.vtx_new.tex1    =trip.vtx_mid.tex1    ; weight.tex1    =tri.weight;}
         if(set_flag&VTX_TEX2    ){tri.vtx_new.tex2    =trip.vtx_mid.tex2    ; weight.tex2    =tri.weight;}
         if(set_flag&VTX_COLOR   ){tri.vtx_new.color   =trip.vtx_mid.color   ; weight.color   =tri.weight;}
         if(set_flag&VTX_MATERIAL){tri.vtx_new.material=trip.vtx_mid.material; weight.material=tri.weight;}
         if(set_flag&VTX_SIZE    ){tri.vtx_new.size    =trip.vtx_mid.size    ; weight.size    =tri.weight;}
         if(set_flag&VTX_SKIN    )AddSkin(skin, trip.vtx_mid.matrix, trip.vtx_mid.blend, tri.weight);
         REPAD(tvi, tri.ind)
         {
            const Int tv=tri.ind.c[tvi];
            if(tv==v0 || tv==v1) // test only edge vertexes, this is to ignore other shared vertexes between the tris which are not on the edge to be removed, this will allow to simplify the mesh more
            {
               VtxData &v=tri.vtxs[tvi];
               REPA(all_tris)
               {
                C TrianglePtr &trip2=all_tris[i]; if(&trip!=&trip2)
                  {
                   C Triangle &tri2=*trip2.tri; REPAD(tvi2, tri2.ind)
                     {
                        const Int tv2=tri2.ind.c[tvi2];
                        if(tv==tv2) // if the same vtx
                        {
                           UInt add_flag=(set_flag&tri2.flag);
                           if(  add_flag&VTX_TEX_ALL) // if we want to merge tex coordinates
                           {
                              if(tri.mtrl_group!=tri2.mtrl_group // we can merge tex only if materials match
                              || Dot(tri.tan, tri2.tan)<=0  // and triangles don't have mirrored tex
                              || Dot(tri.bin, tri2.bin)<=0) // and triangles don't have mirrored tex
                                 FlagDisable(add_flag, VTX_TEX_ALL); // can't merge tex
                           }
                         C VtxData &v2=tri2.vtxs[tvi2];
                           // merge
                           if((add_flag&VTX_NRM     ) &&                                    Equal    (v.nrm     , v2.nrm                    )){tri.vtx_new.nrm     +=trip2.vtx_mid.nrm     ;}
                           if((add_flag&VTX_TEX0    ) &&                                    Equal    (v.tex0    , v2.tex0                   )){tri.vtx_new.tex0    +=trip2.vtx_mid.tex0    ; weight.tex0    +=tri2.weight;}
                           if((add_flag&VTX_COLOR   ) &&                                    Equal    (v.color   , v2.color                  )){tri.vtx_new.color   +=trip2.vtx_mid.color   ; weight.color   +=tri2.weight;}
                           if((add_flag&VTX_MATERIAL) && tri.mtrl_group==tri2.mtrl_group && Equal    (v.material, v2.material               )){tri.vtx_new.material+=trip2.vtx_mid.material; weight.material+=tri2.weight;}
                           if((add_flag&VTX_SKIN    ) &&                                    EqualSkin(v.matrix, v.blend, v2.matrix, v2.blend))AddSkin(skin, trip2.vtx_mid.matrix, trip2.vtx_mid.blend, tri2.weight);
                           if( add_flag&(VTX_HLP|VTX_TEX1|VTX_TEX2|VTX_SIZE)) // uncommon
                           {
                              if((add_flag&VTX_HLP ) && Equal(v.hlp , v2.hlp )){tri.vtx_new.hlp +=trip2.vtx_mid.hlp ; weight.hlp +=tri2.weight;}
                              if((add_flag&VTX_TEX1) && Equal(v.tex1, v2.tex1)){tri.vtx_new.tex1+=trip2.vtx_mid.tex1; weight.tex1+=tri2.weight;}
                              if((add_flag&VTX_TEX2) && Equal(v.tex2, v2.tex2)){tri.vtx_new.tex2+=trip2.vtx_mid.tex2; weight.tex2+=tri2.weight;}
                              if((add_flag&VTX_SIZE) && Equal(v.size, v2.size)){tri.vtx_new.size+=trip2.vtx_mid.size; weight.size+=tri2.weight;}
                           }
                           break;
                        }
                     }
                  }
               }
               break;
            }
         }
         // normalize
         tri.vtx_new.normalize(weight, set_flag);
         if(set_flag&VTX_SKIN)SetSkin(skin, tri.vtx_new.matrix, tri.vtx_new.blend, null);
      }
      return true;
   }
   Bool testTriangles(C Vertex &vtx, C Vec &mid_pos) // test if the new triangle data will not be too far from the original, specifically if moving the old vtx into new location will not introduce too much noise
   {
      PROF(TEST_TRIS);
      REP(vtx.tri_num)
      {
       C Ref      &ref=refs[vtx.ref_start+i];
         Triangle &tri=tris.absElm(ref.tri_abs);
      #if !ADJUST_REFS
         if(tri.exists)
      #endif
         if(!tri.middle) // we don't need to check middle triangles
         {
            UInt  test_flag=(tri.flag&T.test_flag);
          C VtxData  &vtx_n= tri.vtx_new;

            if(TEST_ORIGINAL_POS) // calculate original position in the new triangle
            {
               const Int tvi   =ref.tri_vtx_index,
                         tvi1  =(tvi+1)%3,
                         tvi2  =(tvi+2)%3,
                         tri_v1=tri.ind.c[tvi1],
                         tri_v2=tri.ind.c[tvi2];
             C VtxData  &vtx0  =tri.vtxs[tvi ],
                        &vtx1  =tri.vtxs[tvi1],
                        &vtx2  =tri.vtxs[tvi2];
             C Tri       new_tri(mid_pos, vtxs[tri_v1].pos, vtxs[tri_v2].pos); // this is what's going to be after merging
             C Vec       blend=TriBlend(vtx.pos, new_tri, false);
               if(test_flag&VTX_TEX0)
               {
                  Vec2 got_tex=vtx_n.tex0*blend.x + vtx1.tex0*blend.y + vtx2.tex0*blend.z;
                  if(Dist2(got_tex, vtx0.tex0)>max_uv2)return false;
               }
               if(test_flag&(VTX_TEX1|VTX_TEX2))
               {
                  if(test_flag&VTX_TEX1)
                  {
                     Vec2 got_tex=vtx_n.tex1*blend.x + vtx1.tex1*blend.y + vtx2.tex1*blend.z;
                     if(Dist2(got_tex, vtx0.tex1)>max_uv2)return false;
                  }
                  if(test_flag&VTX_TEX2)
                  {
                     Vec2 got_tex=vtx_n.tex2*blend.x + vtx1.tex2*blend.y + vtx2.tex2*blend.z;
                     if(Dist2(got_tex, vtx0.tex2)>max_uv2)return false;
                  }
               }
               if(test_flag&VTX_COLOR)
               {
                  Vec4 got_col=vtx_n.color*blend.x + vtx1.color*blend.y + vtx2.color*blend.z;
                  if((vtx0.color-got_col).abs().max()>max_color)return false;
               }
               if(test_flag&VTX_MATERIAL)
               {
                  Vec4 got_material=vtx_n.material*blend.x + vtx1.material*blend.y + vtx2.material*blend.z;
                  if((vtx0.material-got_material).abs().max()>max_material)return false;
               }
               if(test_flag&VTX_NRM)
               {
                  Vec got_nrm=vtx_n.nrm*blend.x + vtx1.nrm*blend.y + vtx2.nrm*blend.z;
                  if(CosBetween(got_nrm, vtx0.nrm)<max_normal)return false;
               }
               if(test_flag&VTX_SKIN)
               {
                  MemtN<IndexWeight, 256> got_skin;
                  AddSkin(got_skin, vtx_n.matrix, vtx_n.blend, blend.x);
                  AddSkin(got_skin, vtx1 .matrix, vtx1 .blend, blend.y);
                  AddSkin(got_skin, vtx2 .matrix, vtx2 .blend, blend.z);
                  VecB4 bone_index, bone_weight; SetSkin(got_skin, bone_index, bone_weight, null);
                  if(SkinDifference(bone_index, bone_weight, vtx0.matrix, vtx0.blend)>max_skin)return false;
               }
            }else // calculate new position in the old triangle
            {
               Tri old_tri(vtxs[tri.ind.x].pos, vtxs[tri.ind.y].pos, vtxs[tri.ind.z].pos, &tri.nrm);
               Vec blend=TriBlend(mid_pos, old_tri, false);
               if(test_flag&VTX_TEX0)
               {
                  Vec2 expected_tex=tri.vtxs[0].tex0*blend.x + tri.vtxs[1].tex0*blend.y + tri.vtxs[2].tex0*blend.z;
                  if(Dist2(expected_tex, vtx_n.tex0)>max_uv2)return false;
               }
               if(test_flag&(VTX_TEX1|VTX_TEX2))
               {
                  if(test_flag&VTX_TEX1)
                  {
                     Vec2 expected_tex=tri.vtxs[0].tex1*blend.x + tri.vtxs[1].tex1*blend.y + tri.vtxs[2].tex1*blend.z;
                     if(Dist2(expected_tex, vtx_n.tex1)>max_uv2)return false;
                  }
                  if(test_flag&VTX_TEX2)
                  {
                     Vec2 expected_tex=tri.vtxs[0].tex2*blend.x + tri.vtxs[1].tex2*blend.y + tri.vtxs[2].tex2*blend.z;
                     if(Dist2(expected_tex, vtx_n.tex2)>max_uv2)return false;
                  }
               }
               if(test_flag&VTX_COLOR)
               {
                  Vec4 expected_col=tri.vtxs[0].color*blend.x + tri.vtxs[1].color*blend.y + tri.vtxs[2].color*blend.z;
                  if((vtx_n.color-expected_col).abs().max()>max_color)return false;
               }
               if(test_flag&VTX_MATERIAL)
               {
                  Vec4 expected_material=tri.vtxs[0].material*blend.x + tri.vtxs[1].material*blend.y + tri.vtxs[2].material*blend.z;
                  if((vtx_n.material-expected_material).abs().max()>max_material)return false;
               }
               if(test_flag&VTX_NRM)
               {
                  Vec expected_nrm=tri.vtxs[0].nrm*blend.x + tri.vtxs[1].nrm*blend.y + tri.vtxs[2].nrm*blend.z;
                  if(CosBetween(expected_nrm, vtx_n.nrm)<max_normal)return false;
               }
               if(test_flag&VTX_SKIN)
               {
                  MemtN<IndexWeight, 256> expected_skin;
                  AddSkin(expected_skin, tri.vtxs[0].matrix, tri.vtxs[0].blend, blend.x);
                  AddSkin(expected_skin, tri.vtxs[1].matrix, tri.vtxs[1].blend, blend.y);
                  AddSkin(expected_skin, tri.vtxs[2].matrix, tri.vtxs[2].blend, blend.z);
                  VecB4 bone_index, bone_weight; SetSkin(expected_skin, bone_index, bone_weight, null);
                  if(SkinDifference(bone_index, bone_weight, vtx_n.matrix, vtx_n.blend)>max_skin)return false;
               }
            }
         }
      }
      return true;
   }
   void adjustRefs(Triangle &tri, Int v0, Int v1) // this removes the 'tri' reference from its vertexes
   {
      Int  tri_abs=tris.absIndex(&tri);
      REPA(tri.ind) // iterate all vtxs of the tri
      {
         Int v=tri.ind.c[i]; if(v!=v0 && v!=v1) // we only need to process the 3rd vtx, because 'v0' will be adjusted, and 'v1' will be removed
         {
            Vertex &vtx=vtxs[v]; REP(vtx.tri_num) // iterate all triangles of this vtx
            {
               Ref &ref=refs[vtx.ref_start+i]; if(ref.tri_abs==tri_abs) // if the tri is this one
               {
                  if(InRange(i+1, vtx.tri_num)) // if there is an element after the one being removed
                  {
                  #if 1 // faster
                     ref=refs[vtx.ref_start+vtx.tri_num-1]; // move the last one to the removed one
                  #else
                     refs.moveElm(vtx.ref_start+i, vtx.ref_start+vtx.tri_num-1); // move ref to the last position
                  #endif
                  }
                  vtx.tri_num--;
                  break; // we have found this triangle, finish
               }
            }
            break; // if we've found that vtx, then the job is done
         }
      }
   }

   void resetError(Triangle &tri, Int valid)
   {
      PROF(RESET_ERROR);
      Real error_min=tri.error_min=Min(tri.edge_error[0], tri.edge_error[1], tri.edge_error[2]);
      // reposition this triangle in the 'tris' list to preserve order by 'error_min'
   #if 1 // binary search
      Int elms=Min(tris.elms(), processed_tris), // we can ignore moving to the right of what we've already processed
          l=0, r=elms-1;

      if(InRange(valid-1, elms) && tris[valid-1].error_min>=error_min)l=valid; // left  ok
      if(InRange(valid+1, elms) && tris[valid+1].error_min<=error_min)r=valid; // right ok
      if(l>=r)return;
      for(; l<=r; )
      {
         Int mid=UInt(l+r)/2;
         if( mid==valid) // ignore this element
         {
            if(mid+1<=r)mid++;else
            if(mid-1>=l)mid--;else
               return; // we can't move to the right, and can't move to the left, this means that we've limited the range to the original valid index
         }
         Real e=tris[mid].error_min;
         if(e<error_min)r=mid-1;else
         if(e>error_min)l=mid+1;else
         { // values are the same, however since there can be many same values, then let's move towards the original 'valid' index, so that 'moveElm' can be shortest
            if(valid>mid)l=mid+1; // valid is after  mid, so try moving right (adjust left  boundary)
            else         r=mid-1; // valid is before mid, so try moving left  (adjust right boundary)
         }
      }
      tris.moveElm(valid, (valid<l) ? r : l);
   #else // iterative method
      Int target=valid;
      for(Int range=Min(tris.elms(), processed_tris); ; ) // we can ignore moving to the right of what we've already processed
      {
         Int next_i=target+1; if(!InRange(next_i, range))break;
       C Triangle &next=tris[next_i]; if(next.error_min<=error_min)break; // if OK then stop
         target=next_i;
      }
      for(;;)
      {
         Int next_i=target-1; if(!InRange(next_i, tris))break;
       C Triangle &next=tris[next_i]; if(next.error_min>=error_min)break; // if OK then stop
         target=next_i;
      }
      tris.moveElm(valid, target);
   #endif
   }

   void setNrmTanBin(Triangle &tri)
   {
      PROF(SET_NRM_TAN_BIN);

    C Vec &v0 =vtxs[tri.ind.c[0]].pos,
           v10=vtxs[tri.ind.c[1]].pos-v0,
           v20=vtxs[tri.ind.c[2]].pos-v0;

      // calculate normal
      tri.nrm   =GetNormalU(v10, v20);
      tri.weight=Max(tri.nrm.normalize(), FLT_MIN); // maximize because later there is division by weight, and this will avoid div by zero, this affects both the 'tri.weight' and 'vtx.weight' (which is a sum of all vtx triangle weights)

      // calculate tangent and binormal
      if(tri.flag&VTX_TEX0)
      {
         Vec2 tx    =tri.vtxs[1].tex0-tri.vtxs[0].tex0,
              ty    =tri.vtxs[2].tex0-tri.vtxs[0].tex0;
         Flt  tx_len=tx.normalize(),
              ty_len=ty.normalize();
         Vec2 ty_n  =Perp(ty);
         Flt  tx_v  =-Dot(tx, ty_n);
         if(Abs(tx_v)>EPS)
         {
                tx_v=ty_n.x/tx_v;
            Flt ty_v=Dot(tx, ty)*tx_v+ty.x;

            tx_v=-tx_v;
            if(tx_len)tx_v/=tx_len;
            if(ty_len)ty_v/=ty_len;

            tri.tan=v10*tx_v + v20*ty_v;
         }else tri.tan.zero();

         Vec2 tx_n=Perp(tx);
         Flt  ty_v=-Dot(ty, tx_n);
         if(Abs(ty_v)>EPS)
         {
                ty_v=tx_n.y/ty_v;
            Flt tx_v=Dot(ty, tx)*ty_v+tx.y;

            ty_v=-ty_v;
            if(tx_len)tx_v/=tx_len;
            if(ty_len)ty_v/=ty_len;

            tri.bin=v10*tx_v + v20*ty_v;
         }else tri.bin.zero();
      }
   }

   // update triangle connections and edge error after an edge is collapsed
   void updateTriangles(Int vtx_collapse, C Vertex &vtx) // 'vtx_collapse'=vertex to which we are collapsing
   {
      REP(vtx.tri_num)
      {
       C Ref      &ref=refs[vtx.ref_start+i];
         Triangle &tri=tris.absElm(ref.tri_abs);
      #if !ADJUST_REFS
         if(tri.exists)
      #endif
         if(!tri.middle) // we don't need to update middle triangles as they are removed
         {
            Vec mid_pos;
            tri.ind.c[ref.tri_vtx_index]=vtx_collapse;
            tri.vtxs [ref.tri_vtx_index]=tri.vtx_new;
            setNrmTanBin(tri);
            tri.edge_error[0]=calculateError(tri.ind.c[0], tri.ind.c[1], mid_pos);
            tri.edge_error[1]=calculateError(tri.ind.c[1], tri.ind.c[2], mid_pos);
            tri.edge_error[2]=calculateError(tri.ind.c[2], tri.ind.c[0], mid_pos);
            resetError(tri, tris.absToValidIndex(ref.tri_abs));
            refs.add(Ref(ref)); // !! add as temp variable, because 'add' can cause 'ref' mem address to be invalid
         }
      }
   }

   struct VtxConnection
   {
      Int       vtx   , // index of the other vertex
                tris  ; // how many tris reference this edge
    C Triangle *tri   ; // pointer to the first triangle that has this edge, TODO: this could be a list of all triangles, however we would need a memory container, and in most cases it's not needed
      Bool      mirror; // mirrored tex between 2 triangles

      void init(Int vtx, C Triangle &tri) {T.vtx=vtx; T.tris=1; T.tri=&tri; mirror=false;}
   };
   void addEdge(Memt<VtxConnection> &edges, Int vtx_index, C Triangle &tri)
   {
      REPA(edges) // check if this edge was already created by another tri
      {
         VtxConnection &edge=edges[i]; if(edge.vtx==vtx_index) // found
         {
            // mark that this edge is referenced by 1 more triangle
          C Triangle &edge_tri=*edge.tri;
            if(Dot(edge_tri.nrm, tri.nrm)>-EPS_COL_COS) // but only if the other triangle is not mirror co-planar (can happen for meshes which have 2 sided triangles), if it is mirror co-planar, then we don't want to increase the counter, but keep at 1, so the edge is detected as a border
            {
               edge.tris++;
               if(edge_tri.flag&tri.flag&VTX_TEX_ALL)
                  if(Dot(edge_tri.tan, tri.tan)<=0  // and triangles don't have mirrored tex
                  || Dot(edge_tri.bin, tri.bin)<=0) // and triangles don't have mirrored tex
                     edge.mirror=true;
            }
            return;
         }
      }
      edges.New().init(vtx_index, tri); // create a new edge
   }
   struct TriLink
   {
    C Triangle *tri;
      VecI2     p  ; // indexes of other vertexes

      void set(C Triangle &tri, Int v1, Int v2) {T.tri=&tri; p.set(v1, v2);}
   };
   void processVtxs() // detect which vertexes are on the border
   {
      Memt<VtxConnection> edges; // list of all 'vtx' neighbors, making a list of edges from 'vtx' to its 'neighbor'
      Memt<TriLink      > tris_left; // list of triangles

      REPAD(vi, vtxs)
      {
         Vertex &vtx=vtxs[vi];
         edges.clear(); tris_left.clear();
         REP(vtx.tri_num) // iterate all tris touching this vertex
         {
          C Ref      &ref=refs[vtx.ref_start+i];
          C Triangle &tri=tris.absElm(ref.tri_abs);
            Int tvi   =ref.tri_vtx_index,
              //tri_v0=tri.ind.c[ tvi     ], this is always 'vi', and we don't need to process it, because we're not interested in 'vi'<->'vi' connection
                tri_v1=tri.ind.c[(tvi+1)%3],
                tri_v2=tri.ind.c[(tvi+2)%3];
            addEdge(edges, tri_v1, tri); // add connection from 'vi' to 'tri_v1'
            addEdge(edges, tri_v2, tri); // add connection from 'vi' to 'tri_v2'
            tris_left.New().set(tri, tri_v1, tri_v2);
         }

         // check which edges are referenced by only 1 tri
         REPA(edges)
         {
          C VtxConnection &edge=edges[i];
            if(edge.tris==1)vtx.border|=BORDER_POS; // if edge is referenced by only 1 tri
            if(edge.mirror )vtx.border|=BORDER_TEX;

            // normally vtxs will be repositioned only on the triangle surface, however for border vertexes, we don't want them to move away from the border too, so adjust 'qm' and 'planes'
            if(VTX_BORDER_QM>0)
               if(edge.tris==1 // if BORDER_POS (check "edge.tris==1" again, instead of "vtx.border&BORDER_POS", because BORDER_POS could have been enabled due to other edge)
               || edge.mirror  // if BORDER_TEX (check "edge.mirror"  again, because check below only checks if both have BORDER_TEX)
               || (vtx.border&BORDER_TEX) && (vtxs[edge.vtx].border&BORDER_TEX)) // also check for BORDER_TEX, currently there is no fast check to check this edge, so for simplicity, just check if both vertexes have this enabled
            {
               Vec cross=Cross(vtxs[edge.vtx].pos-vtx.pos, edge.tri->nrm); // this vector points outside (or inside) of the triangle (perpendicular to edge direction and triangle normal), if we use "cross" or "-cross" it's not important, because of how 'QuadricMatrix' works
               cross.normalize();
               vtx.qm+=QuadricMatrix(cross, vtx.pos)*(vtx.weight*VTX_BORDER_QM); // always set this, because it affects mid position
            #if ALLOW_PLANES
               if(mode==SIMPLIFY_PLANES)
               {
                  Vertex::Plane plane;
                  plane.normal=cross;
                  plane.plane =-Dot(plane.normal, vtx.pos);
                  if(VTX_BORDER_QM!=1)plane.scale(VTX_BORDER_QM);
                  vtx.includePlane(plane);
               }
            #endif
            }
         }

         // add a plane along vertex normal, to preserve details, for example for spike mesh parts /\ where the vertex normal is far away from the triangle normals
         // this algorithm correctly handles cases with double sided faces (when processing a single face, it will remove all its copies including reversed faces, and proceed to next in the vtx triangle fan)
         if(VTX_NORMAL_QM>0)
         {
            Vec tris_n=0;
            for(; tris_left.elms(); )
            {
               TriLink   tri_link=tris_left.pop();
             C Triangle *tri     =tri_link.tri;
               Bool      chs     =(Dot(tri->nrm, tris_n)<0); // if this triangle direction points in the opposite of what we've already gathered, then change its (and its neighbors) sign
            again:
               Vec       tri_nw  =tri->nrm*tri->weight; // tri normal weighted
               if(chs)tris_n-=tri_nw;else tris_n+=tri_nw; // add to gathered normal
               TriLink   next; next.tri=null;
               // remove all the same and reversed tris, and find next one
               REPA(tris_left)
               {
                  VecI2 p=tris_left[i].p;
                  if(tri_link.p==p || (tri_link.p.x==p.y && tri_link.p.y==p.x))tris_left.remove(i);else
                  if(tri_link.p.y==p.x)next=tris_left[i]; // if this wasn't removed, and it's the next triangle, then remember it
               }
               if(next.tri)
               {
                  tri_link=next;
                  tri     =tri_link.tri;
                  goto again; // proceed to next one, without adjusting 'chs', because we are processing next triangle in fan connected to the previous, so we treat them as continuous
               }
            }
            tris_n.normalize();
            vtx.qm+=QuadricMatrix(tris_n, vtx.pos)*(vtx.weight*VTX_NORMAL_QM); // the best results is when this qm is proportional to 'vtx.weight'
         }
      }
   }

   void add(C MeshBase &mesh, Int part, C MeshPart *mesh_part)
   {
      UInt mesh_flag=mesh.flag();

      // set material group
      MtrlGroup mtrl_group(mesh_flag, mesh_part);
      Int mtrl_group_i=-1; REPA(mtrl_groups)if(mtrl_groups[i].testAndMerge(mtrl_group)){mtrl_group_i=i; break;} // find existing one
      if( mtrl_group_i< 0){MtrlGroup &mg=mtrl_groups[mtrl_group_i=mtrl_groups.addNum(1)]; Swap(mg, mtrl_group);} // create new one

      Int    vtx_ofs=vtx_dups.addNum(mesh.vtxs     ()),
             tri_ofs=tris    .addNum(mesh.trisTotal());
      Box   mesh_box=mesh; if(vtx_ofs)T.box|=mesh_box;else T.box=mesh_box;
      FREPA(mesh.vtx)
      {
         VtxDupMap &vtx_dup=vtx_dups[vtx_ofs+i];
                            vtx_dup.pos       =mesh.vtx.pos (i);
         if(mesh.vtx.tex0())vtx_dup.tex0      =mesh.vtx.tex0(i);else vtx_dup.tex0.zero(); // if not available then zero so BORDER_TEX can be detected properly
                            vtx_dup.mtrl_group=mtrl_group_i;
      }
      FREPA(mesh.tri)
      {
         Triangle &tri=tris[tri_ofs+i];
       C VecI     &src=mesh.tri.ind(i);
         tri.part      =part;
         tri.mtrl_group=mtrl_group_i;
       //tri.flag      =mesh_flag; this will be set in 'init'
         tri.ind       =src+vtx_ofs;
         REPAO(tri.vtxs).from(mesh, src.c[i]);
      }
      Int quad_ofs=tri_ofs+mesh.tris();
      FREPA(mesh.quad)
      {
       C VecI4 &src=mesh.quad.ind(i);
         VecI    t0=src.tri0(), t1=src.tri1();
         {
            Triangle &tri=tris[quad_ofs++];
            tri.part      =part;
            tri.mtrl_group=mtrl_group_i;
          //tri.flag      =mesh_flag; this will be set in 'init'
            tri.ind       =t0+vtx_ofs;
            REPAO(tri.vtxs).from(mesh, t0.c[i]);
         }
         {
            Triangle &tri=tris[quad_ofs++];
            tri.part      =part;
            tri.mtrl_group=mtrl_group_i;
          //tri.flag      =mesh_flag; this will be set in 'init'
            tri.ind       =t1+vtx_ofs;
            REPAO(tri.vtxs).from(mesh, t1.c[i]);
         }
      }
   }

   void checkRefs()C
   {
      REPAD(t, tris)
      {
       C Triangle &tri=tris[t]; REPAD(v, tri.ind)
         {
          C Vertex &vtx=vtxs[tri.ind.c[v]];
            Bool    has=false;
            REP(vtx.tri_num)
            {
             C Ref      &ref=refs[vtx.ref_start+i];
             C Triangle &vtx_tri=tris.absElm(ref.tri_abs); if(tris.absToValidIndex(ref.tri_abs)<0)Exit("vtx tri doesn't exist");
               if(&tri==&vtx_tri)has=true;
            }
            if(!has)Exit("vtx doesn't have tri");
         }
      }
   }
   void init(Flt pos_eps)
   {
      PROF(INIT);
      Int  mtrl_groups_elms=mtrl_groups.elms(); // how many material groups are there, remember this because 'mtrl_groups' will be deleted
      UInt mtrl_groups_flag=0; REPA(mtrl_groups)mtrl_groups_flag|=mtrl_groups[i].flag; // what vtx components are we processing
      REPA(tris) // set actual triangle flag after adding all meshes, because they may modify the mtrl group flag
      {
         Triangle &tri=tris[i]; tri.flag=mtrl_groups[tri.mtrl_group].flag;
      }
      mtrl_groups.del(); // release to free memory as it's no longer needed

      // create unique vertexes
      vtxs.setNum(SetVtxDup(SCAST(Memc<VtxDup>, vtx_dups), box, pos_eps)); Int vs=0;
      REPA(vtx_dups)
      {
         VtxDupMap &vd=vtx_dups[i]; if(vd.dup==i)
         {
            vd.vtxs_index=vs; // set mapping from all points to 'vtxs'
            Vertex &vtx=vtxs[vs++];
            vtx.pos=vd.pos;
            vtx.qm.reset();
            vtx.weight=0;
            vtx.border=0;
            vtx.ref_start=vtx.tri_num=0;
         }
      }

      // now that all 'vtxs' have been set, we need to check for BORDER_TEX
      if(vtxs.elms()!=vtx_dups.elms()) // if number of unique vtxs is different than all vtxs, then it means there are some duplicates (some share the same position), if all are unique then we don't need to do this
         if(mtrl_groups_elms>1 || (mtrl_groups_flag&VTX_TEX0)) // if there are total of more than 1 mtrl groups, or the vtxs have tex, then we need to check if the shared vtxs have different values
            REPA(vtx_dups)
      {
         VtxDupMap &vd=vtx_dups[i]; // get i-th vtx
         if(vd.dup!=i) // if this is not unique vertex
         {
            VtxDupMap &unique=vtx_dups[vd.dup]; // get unique vtx
            if(vd.mtrl_group!=unique.mtrl_group || !EqualWrap(vd.tex0, unique.tex0)) // if this vtx tex is not the same as the unique vtx tex
               vtxs[unique.vtxs_index].border|=BORDER_TEX; // mark it as being at the border
         }
      }

      // init quadrics by plane and edge errors
      REPA(tris)
      {
         Triangle &tri=tris[i];
         REPA(tri.ind)
         {
            Int    &tv =tri.ind.c[i]; tv=vtx_dups[vtx_dups[tv].dup].vtxs_index; // remap triangle vertex index to unique
            Vertex &vtx=vtxs[tv]    ; vtx.tri_num++;
         }
         tri.tan.zero(); tri.bin.zero(); setNrmTanBin(tri); // zero in init, in case 'setNrmTanBin' doesn't modify the values

         Real plane_dist=-Dot(tri.nrm, vtxs[tri.ind.x].pos);
         QuadricMatrix qm(tri.nrm, plane_dist);
         qm*=tri.weight; // make weighted by triangle surface area

      #if ALLOW_PLANES
         Vertex::Plane plane; plane.normal=tri.nrm; plane.plane=plane_dist;
      #endif
         REP(3)
         {
            Vertex &vtx=vtxs[tri.ind.c[i]];
            vtx.qm    +=qm        ;
            vtx.weight+=tri.weight;
         #if ALLOW_PLANES
            if(mode==SIMPLIFY_PLANES)vtx.includePlane(plane);
         #endif
         }
      }
      vtx_dups.del(); // release to free memory as it's no longer needed

      Int ref_start=0; REPA(vtxs) // prepare for a single continuous array of unique triangle-vertex pairs
      {
         Vertex &vtx=vtxs[i];
         vtx.ref_start=ref_start; ref_start+=vtx.tri_num;
         vtx.tri_num  =0;
      }

      // set references
      refs.setNum(tris.elms()*3);
      REPAD(t, tris)
      {
         Triangle &tri=tris[t];
         Int       tri_abs=tris.validToAbsIndex(t);
         REPAD(v, tri.ind)
         {
            Vertex &vtx=vtxs[tri.ind.c[v]];
            Ref    &ref=refs[vtx.ref_start + vtx.tri_num++];
            ref.tri_abs      =tri_abs;
            ref.tri_vtx_index=v;
         }
      }

      processVtxs(); // call after refs are available

      // once 'vtx.qm' and 'vtx.border' are initialized for all vertexes, calculate edge error
      REPAD(t, tris)
      {
         Triangle &tri=tris[t]; REPAD(v, tri.ind)
         {
            Vec mid_pos; tri.edge_error[v]=calculateError(tri.ind.c[v], tri.ind.c[(v+1)%3], mid_pos);
         }
         tri.error_min=Min(tri.edge_error[0], tri.edge_error[1], tri.edge_error[2]);
      }
      tris.sort(CompareError); // tris are now sorted from highest to lowest error
   }

   void simplify(Flt intensity, Flt max_distance, Flt max_uv, Flt max_color, Flt max_material, Flt max_skin, Flt max_normal, Bool keep_border, MESH_SIMPLIFY mode, Flt pos_eps)
   {
   #if !ALLOW_PLANES
      if(mode==SIMPLIFY_PLANES)mode=SIMPLIFY_QUADRIC;
   #endif
      T.mode        =mode;
      T.max_uv2     =Sqr(Sat(max_uv));
      T.max_color   =Sat(max_color);
      T.max_material=Sat(max_material);
      T.max_skin    =FltToByte(max_skin);
      T.max_normal  =Cos(Mid(max_normal, 0.0f, PI));
      T.keep_border =keep_border;
      T.test_flag   =((T.max_uv2<1-EPS) ? VTX_TEX_ALL : 0)|((T.max_color<1-EPS) ? VTX_COLOR : 0)|((T.max_material<1-EPS) ? VTX_MATERIAL : 0)|((T.max_skin<255) ? VTX_SKIN : 0)|((T.max_normal>-1+EPS) ? VTX_NRM : 0);
      Real max_error  =Max(0.0f, max_distance);
      Int  target_tris=Mid(tris.elms()-RoundPos(tris.elms()*intensity), 0, tris.elms());
      if(mode==SIMPLIFY_QUADRIC) // quadric method operates on squared errors
      {
         max_error*=max_error;
      }

      init(pos_eps);
      if(tris.elms()<=target_tris)return; // must be checked after 'init'

      // main iteration loop
      Vec mid_pos;
      REPA(tris)
         if(InRange(i, tris)) // check in case it got deleted
      {
         processed_tris=i;
      again:
         Triangle &tri=tris[i]; if(tri.error_min>max_error || stop())break; // tris are sorted by their error, so if we've reached the one above the limit, then stop

         Int i=MinI(tri.edge_error[0], tri.edge_error[1], tri.edge_error[2]);
         Int edge_vtx0i=tri.ind.c[ i     ]; Vertex &edge_vtx0=vtxs[edge_vtx0i]; 
         Int edge_vtx1i=tri.ind.c[(i+1)%3]; Vertex &edge_vtx1=vtxs[edge_vtx1i];

         // border check
         if(keep_border && (edge_vtx0.border&edge_vtx1.border&BORDER_POS))goto cant; // if both are on the border, then we can't move, if only one is on border, then we can move but only to the border

         // compute vertex to collapse to
         calculateError(edge_vtx0i, edge_vtx1i, mid_pos);

      /* there are 3 types of processed triangles:
            -have only edge_vtx0                                      , let's call them "left"   triangles
            -have both edge_vtx0 and edge_vtx1 (these will be removed), let's call them "middle" triangles
            -have only edge_vtx1                                      , let's call them "right"  triangles */

         // don't remove if flipped
         middle_tris.clear(); all_tris.clear(); // clear before 'flipped' because that modifies this
         if(flipped(mid_pos, edge_vtx1i, edge_vtx0, true ) // middle tris will be collected only from the first vertex
         || flipped(mid_pos, edge_vtx0i, edge_vtx1, false))goto cant;

         // calculate vertex data for all triangles at the middle position
         REPA(all_tris)
         {
            TrianglePtr &triangle_ptr=all_tris[i];
            Triangle    &triangle    =*triangle_ptr.tri;
            Tri          tri(vtxs[triangle.ind.x].pos, vtxs[triangle.ind.y].pos, vtxs[triangle.ind.z].pos, &triangle.nrm);
            Vec          blend=TriBlend(mid_pos, tri, false);
            blend*=triangle.weight; // make 'vtx_mid' vertex data weighted by 'triangle.weight'
            triangle_ptr.vtx_mid.lerp(triangle.vtxs[0], triangle.vtxs[1], triangle.vtxs[2], blend, triangle.flag);
            triangle_ptr.vtx_mid.nrm*=triangle.weight; // mul 'nrm' because 'lerp' normalizes the normal
            triangle_ptr.clear();
         }

      /* In most cases, vtx data will be a weighted average of all 3 triangle types
         However, since left/right triangles are not in contact, then first the middle triangles will have their vtx data set from all triangles
         And then left/right (outer) triangles will have this vtx data copied from the middle triangles. */

         REPA(middle_tris)    addVtxData(*middle_tris[i], edge_vtx0i, edge_vtx1i);
         REPA(   all_tris)if(!setVtxData(    all_tris[i], edge_vtx0i, edge_vtx1i))goto cant;
         if(test_flag)
         {
            if(!testTriangles(edge_vtx0, mid_pos)
            || !testTriangles(edge_vtx1, mid_pos))goto cant;
         }

         // remove edge
         {
         #if ADJUST_REFS
            REPA(middle_tris)adjustRefs(*middle_tris[i]->tri, edge_vtx0i, edge_vtx1i);
         #endif
         
            REPA(middle_tris)tris.removeData(middle_tris[i]->tri, true);
            edge_vtx0.pos=mid_pos;
            edge_vtx0.eat(edge_vtx1);
            Int ref_start=refs.elms(); // remember current number of references

            updateTriangles(edge_vtx0i, edge_vtx0);
            updateTriangles(edge_vtx0i, edge_vtx1);

            if(tris.elms()<=target_tris)break; // if reached the limit

            Int tri_num =refs.elms()-ref_start; // get how many references were added
            if( tri_num<=edge_vtx0.tri_num) // if the new number fits in the previous range, then just replace it, to reduce memory usage
            {
               CopyN(refs.addr(edge_vtx0.ref_start), refs.addr(ref_start), tri_num); // copy to existing location
               refs.setNum(ref_start); // added elements will not be used, so release them
            }else edge_vtx0.ref_start=ref_start; // set start to what we've added
                  edge_vtx0.tri_num  =tri_num  ; // set number of tris
         #if DEBUG_CHECK_REFS && ADJUST_REFS // we can check refs only if they are adjusted
            #pragma message("!! Warning: This will slow down mesh simplification, use only for testing !!")
            checkRefs();
         #endif
            continue;
         }

      cant:
         if(tri.edge_error[i]<FLT_MAX) // this was the min, and if the min was doable, then
         {
            tri.edge_error[i]=FLT_MAX; // set it as not-doable
            resetError(tri, processed_tris); // re-position it in the 'tris' list based on new 'error_min'
            goto again; // try again with the same index
         }
      }
   }

   void store(MeshBase &mesh, UInt flag_and=~0)
   {
      UInt flags=0; REPA(tris)flags|=tris[i].flag; flags&=flag_and;
      mesh.create(tris.elms()*3, 0, tris.elms(), 0, flags&(VTX_ALL&~(VTX_TAN_BIN|VTX_DUP)));
      REPA(tris)
      {
         Triangle &tri=tris[i];
         Int       tri_index=i, vtx_index=i*3;
         mesh.tri.ind(tri_index).set(vtx_index, vtx_index+1, vtx_index+2);
         tri.vtxs[0].to(mesh, vtx_index  );
         tri.vtxs[1].to(mesh, vtx_index+1);
         tri.vtxs[2].to(mesh, vtx_index+2);
         mesh.vtx.pos(vtx_index  )=vtxs[tri.ind.x].pos;
         mesh.vtx.pos(vtx_index+1)=vtxs[tri.ind.y].pos;
         mesh.vtx.pos(vtx_index+2)=vtxs[tri.ind.z].pos;
      }
      mesh.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, ignore degenerate faces
      // recalculate precisely
      if(flags&VTX_TAN)mesh.setTangents ();
      if(flags&VTX_BIN)mesh.setBinormals();
   }

   void store(MemPtr<MeshPart> parts)
   {
      struct PartInfo
      {
         Int  tris;
         UInt flag;

         PartInfo() {tris=0; flag=0;}
      };
      MemtN<PartInfo, 256> part_infos;

      REPA(tris)
      {
       C Triangle &tri =tris[i];
         PartInfo &part=part_infos(tri.part);
         part.tris++;
         part.flag|=tri.flag;
      }
      parts.setNum(part_infos.elms());
      REPA(parts)
      {
         MeshBase &mesh=parts[i].base;
       C PartInfo &part=part_infos[i];
         mesh.create(part.tris*3, 0, part.tris, 0, part.flag&(VTX_ALL&~(VTX_TAN_BIN|VTX_DUP)));
      }
      REPAO(part_infos).tris=0;
      REPA (tris)
      {
         Triangle &tri=tris[i];
         Int part=tri.part;
         Int tri_index=(part_infos[part].tris++), vtx_index=tri_index*3;
         MeshBase &mesh=parts[part].base;
         mesh.tri.ind(tri_index).set(vtx_index, vtx_index+1, vtx_index+2);
         tri.vtxs[0].to(mesh, vtx_index  );
         tri.vtxs[1].to(mesh, vtx_index+1);
         tri.vtxs[2].to(mesh, vtx_index+2);
         mesh.vtx.pos(vtx_index  )=vtxs[tri.ind.x].pos;
         mesh.vtx.pos(vtx_index+1)=vtxs[tri.ind.y].pos;
         mesh.vtx.pos(vtx_index+2)=vtxs[tri.ind.z].pos;
      }
      REPA(parts) // go from the end so we can remove if needed
      {
         MeshBase &mesh=parts[i].base;
         mesh.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, ignore degenerate faces
         if(stop())return;
         if(!mesh.is())parts.remove(i, true);else // if became empty, then just remove it
         {
            // recalculate precisely
            UInt flag=part_infos[i].flag;
            if(flag&VTX_TAN)mesh.setTangents ();
            if(flag&VTX_BIN)mesh.setBinormals();
         }
      }
   }
};
/******************************************************************************/
MeshBase& MeshBase::simplify(Flt intensity, Flt max_distance, Flt max_uv, Flt max_color, Flt max_material, Flt max_skin, Flt max_normal, Bool keep_border, MESH_SIMPLIFY mode, Flt pos_eps, MeshBase *dest)
{
   if(!dest)dest=this;
   if(intensity<=0)dest->create(T);else
   {
      Simplify s(null);
      s.add(T, 0, null);
      s.simplify(intensity, max_distance, max_uv, max_color, max_material, max_skin, max_normal, keep_border, mode, pos_eps);
      s.store(*dest);
   }
   return *dest;
}
MeshLod& MeshLod::simplify(Flt intensity, Flt max_distance, Flt max_uv, Flt max_color, Flt max_material, Flt max_skin, Flt max_normal, Bool keep_border, MESH_SIMPLIFY mode, Flt pos_eps, MeshLod *dest, Bool *stop)
{
   if(!dest)dest=this;
   if(intensity<=0)dest->create(T);else
   {
      Simplify s(stop);
      if(s.stop())goto stop;
      if(dest!=this)
      {
         dest->copyParams(T);
       //dest->delRender(); don't do this because that would require D._lock, and it's highly possible that we're going to be calling this method on other threads, which would have to wait for the lock
         dest->parts.setNum(parts.elms());
         REPAO(dest->parts).copyParams(parts[i]);
      }
      if(s.stop())goto stop;
      {
         PROF(ADD);
         FREPA(T)
         {
            s.add(parts[i].base, i, &parts[i]);
            if(s.stop())goto stop;
         }
      }
      {
         PROF(SIMPLIFY);
         s.simplify(intensity, max_distance, max_uv, max_color, max_material, max_skin, max_normal, keep_border, mode, pos_eps);
      }
      if(s.stop())goto stop;
      {
         PROF(STORE);
         s.store(dest->parts);
      }
    //if(s.stop())goto stop;
      PROFILE_END;
   }
stop:
   return *dest;
}
/******************************************************************************/
}
/******************************************************************************/
