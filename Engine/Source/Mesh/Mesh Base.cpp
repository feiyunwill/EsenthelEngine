/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
UInt EtqFlagSwap(UInt flag)
{
   UInt f=(flag&~ETQ_LR);
   if(flag&ETQ_L)f|=ETQ_R;
   if(flag&ETQ_R)f|=ETQ_L;
   return f;
}
/******************************************************************************/
SIDE_TYPE GetSide(C VecI2 &edge, C VecI &tri)
{
   Int e0=edge.c[0], e1=edge.c[1];
   if((tri.c[0]==e0 && tri.c[1]==e1)
   || (tri.c[1]==e0 && tri.c[2]==e1)
   || (tri.c[2]==e0 && tri.c[0]==e1))return SIDE_R;
   if((tri.c[0]==e1 && tri.c[1]==e0)
   || (tri.c[1]==e1 && tri.c[2]==e0)
   || (tri.c[2]==e1 && tri.c[0]==e0))return SIDE_L;
   return SIDE_NONE;
}
SIDE_TYPE GetSide(C VecI2 &edge, C VecI4 &quad)
{
   Int e0=edge.c[0], e1=edge.c[1];
   if((quad.c[0]==e0 && quad.c[1]==e1)
   || (quad.c[1]==e0 && quad.c[2]==e1)
   || (quad.c[2]==e0 && quad.c[3]==e1) 
   || (quad.c[3]==e0 && quad.c[0]==e1))return SIDE_R;
   if((quad.c[0]==e1 && quad.c[1]==e0)
   || (quad.c[1]==e1 && quad.c[2]==e0)
   || (quad.c[2]==e1 && quad.c[3]==e0) 
   || (quad.c[3]==e1 && quad.c[0]==e0))return SIDE_L;
   return SIDE_NONE;
}
/******************************************************************************/
MeshBase::MeshBase(  Int         vtxs, Int edges, Int tris, Int quads, UInt flag) : MeshBase() {create(vtxs, edges, tris, quads, flag);}
MeshBase::MeshBase(C MeshBase   &src , UInt flag_and                            ) : MeshBase() {create(src , flag_and);}
MeshBase::MeshBase(C MeshRender &src , UInt flag_and                            ) : MeshBase() {create(src , flag_and);}
MeshBase::MeshBase(C MeshPart   &src , UInt flag_and                            ) : MeshBase() {create(src , flag_and);}
MeshBase::MeshBase(C MeshLod    &src , UInt flag_and                            ) : MeshBase() {create(src , flag_and);}
MeshBase::MeshBase(C PhysPart   &src                                            ) : MeshBase() {create(src);}
/******************************************************************************/
void MeshBase::copyVtxs(C MeshBase &src)
{
   Int elms=Min(vtxs(), src.vtxs());
   CopyN(vtx.pos     (), src.vtx.pos     (), elms);
   CopyN(vtx.nrm     (), src.vtx.nrm     (), elms);
   CopyN(vtx.tan     (), src.vtx.tan     (), elms);
   CopyN(vtx.bin     (), src.vtx.bin     (), elms);
   CopyN(vtx.hlp     (), src.vtx.hlp     (), elms);
   CopyN(vtx.tex0    (), src.vtx.tex0    (), elms);
   CopyN(vtx.tex1    (), src.vtx.tex1    (), elms);
   CopyN(vtx.tex2    (), src.vtx.tex2    (), elms);
   CopyN(vtx.matrix  (), src.vtx.matrix  (), elms);
   CopyN(vtx.blend   (), src.vtx.blend   (), elms);
   CopyN(vtx.size    (), src.vtx.size    (), elms);
   CopyN(vtx.material(), src.vtx.material(), elms);
   CopyN(vtx.color   (), src.vtx.color   (), elms);
   CopyN(vtx.flag    (), src.vtx.flag    (), elms);
   CopyN(vtx.dup     (), src.vtx.dup     (), elms);
}
void MeshBase::copyVtxs(C MeshBase &src, C MemPtr<Bool> &is)
{
   DEBUG_ASSERT(is.elms()<=src.vtxs() && CountIs(is)<=vtxs(), "copyVtxs");
   CopyIs(vtx.pos     (), src.vtx.pos     (), is);
   CopyIs(vtx.nrm     (), src.vtx.nrm     (), is);
   CopyIs(vtx.tan     (), src.vtx.tan     (), is);
   CopyIs(vtx.bin     (), src.vtx.bin     (), is);
   CopyIs(vtx.hlp     (), src.vtx.hlp     (), is);
   CopyIs(vtx.tex0    (), src.vtx.tex0    (), is);
   CopyIs(vtx.tex1    (), src.vtx.tex1    (), is);
   CopyIs(vtx.tex2    (), src.vtx.tex2    (), is);
   CopyIs(vtx.matrix  (), src.vtx.matrix  (), is);
   CopyIs(vtx.blend   (), src.vtx.blend   (), is);
   CopyIs(vtx.size    (), src.vtx.size    (), is);
   CopyIs(vtx.material(), src.vtx.material(), is);
   CopyIs(vtx.color   (), src.vtx.color   (), is);
   CopyIs(vtx.flag    (), src.vtx.flag    (), is);
   CopyIs(vtx.dup     (), src.vtx.dup     (), is);
}
void MeshBase::copyEdges(C MeshBase &src)
{
   Int elms=Min(edges(), src.edges());
   CopyN(edge.ind    (), src.edge.ind    (), elms);
   CopyN(edge.adjFace(), src.edge.adjFace(), elms);
   CopyN(edge.nrm    (), src.edge.nrm    (), elms);
   CopyN(edge.flag   (), src.edge.flag   (), elms);
   CopyN(edge.id     (), src.edge.id     (), elms);
}
void MeshBase::copyEdges(C MeshBase &src, C MemPtr<Bool> &is)
{
   DEBUG_ASSERT(is.elms()<=src.edges() && CountIs(is)<=edges(), "copyEdges");
   CopyIs(edge.ind    (), src.edge.ind    (), is);
   CopyIs(edge.adjFace(), src.edge.adjFace(), is);
   CopyIs(edge.nrm    (), src.edge.nrm    (), is);
   CopyIs(edge.flag   (), src.edge.flag   (), is);
   CopyIs(edge.id     (), src.edge.id     (), is);
}
void MeshBase::copyTris(C MeshBase &src)
{
   Int elms=Min(tris(), src.tris());
   CopyN(tri.ind    (), src.tri.ind    (), elms);
   CopyN(tri.adjFace(), src.tri.adjFace(), elms);
   CopyN(tri.adjEdge(), src.tri.adjEdge(), elms);
   CopyN(tri.nrm    (), src.tri.nrm    (), elms);
   CopyN(tri.flag   (), src.tri.flag   (), elms);
   CopyN(tri.id     (), src.tri.id     (), elms);
}
void MeshBase::copyTris(C MeshBase &src, C MemPtr<Bool> &is)
{
   DEBUG_ASSERT(is.elms()<=src.tris() && CountIs(is)<=tris(), "copyTris");
   CopyIs(tri.ind    (), src.tri.ind    (), is);
   CopyIs(tri.adjFace(), src.tri.adjFace(), is);
   CopyIs(tri.adjEdge(), src.tri.adjEdge(), is);
   CopyIs(tri.nrm    (), src.tri.nrm    (), is);
   CopyIs(tri.flag   (), src.tri.flag   (), is);
   CopyIs(tri.id     (), src.tri.id     (), is);
}
void MeshBase::copyQuads(C MeshBase &src)
{
   Int elms=Min(quads(), src.quads());
   CopyN(quad.ind    (), src.quad.ind    (), elms);
   CopyN(quad.adjFace(), src.quad.adjFace(), elms);
   CopyN(quad.adjEdge(), src.quad.adjEdge(), elms);
   CopyN(quad.nrm    (), src.quad.nrm    (), elms);
   CopyN(quad.flag   (), src.quad.flag   (), elms);
   CopyN(quad.id     (), src.quad.id     (), elms);
}
void MeshBase::copyQuads(C MeshBase &src, C MemPtr<Bool> &is)
{
   DEBUG_ASSERT(is.elms()<=src.quads() && CountIs(is)<=quads(), "copyQuads");
   CopyIs(quad.ind    (), src.quad.ind    (), is);
   CopyIs(quad.adjFace(), src.quad.adjFace(), is);
   CopyIs(quad.adjEdge(), src.quad.adjEdge(), is);
   CopyIs(quad.nrm    (), src.quad.nrm    (), is);
   CopyIs(quad.flag   (), src.quad.flag   (), is);
   CopyIs(quad.id     (), src.quad.id     (), is);
}
/******************************************************************************/
Int SetVtxDup(MemPtr<VtxDup> vtxs, C Box &box, Flt pos_eps)
{
   Int unique=0;

   // link box->vtx
   Boxes boxes  (box        , vtxs.elms());
   Index box_vtx(boxes.num(), vtxs.elms());
   REPA(vtxs)box_vtx.elmGroup(boxes.index(vtxs[i].pos), i); box_vtx.set();

   // get duplicates
   Int xs=boxes.cells.x,
       ys=boxes.cells.y, xys=xs*ys;

   // iterate all boxes
   FREPD(z, boxes.cells.z)
   FREPD(y, boxes.cells.y)
   FREPD(x, boxes.cells.x)
   {
      Int         box_tests=0;
      IndexGroup *box_test[2+3+9],
                 *box_cur=&box_vtx.group[x + y*xs + z*xys]; // get current box

      // set which neighbor boxes to test
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

      FREPA(*box_cur) // iterate all vertexes in this box
      {
         Int     vtx_cur_i=  (*box_cur)[i]; // this is i-th vtx in this box
         VtxDup &vtx_cur  =vtxs[vtx_cur_i];
       C Vec    &pos_cur  =vtx_cur.pos; // this is position of that vertex
         REPD(c, box_tests) // iterate all boxes to test
         {
            IndexGroup *bt=box_test[c];
            REPD(m, (box_cur==bt) ? i : bt->num) // iterate all vtxs in the test box (if the test box is the current box, then check only vertexes before the current one)
            {
               Int     vtx_test_i=(*bt)[m]; // this is m-th vtx in the test box
             C VtxDup &vtx_test  =vtxs[vtx_test_i];
               if(vtx_test.dup==vtx_test_i // if this vtx is unique (points to self), this is so that we assign mapping only to uniqe vtxs (and not to vtxs that point to other vtxs)
               && Equal(pos_cur, vtx_test.pos, pos_eps)) // if position is the same
               {
                  // found a duplicate
                  vtx_cur.dup=vtx_test_i; goto next; // set 'dup' index to point to unique 'vtx_test_i'
               }
            }
         }
         // haven't found a duplicate, so set as unique
         vtx_cur.dup=vtx_cur_i; // set 'dup' index to point to self
         unique++; // increase unique counter
      next:;
      }
   }
   return unique;
}
Int SetVtxDup(MemPtr<VtxDupNrm> vtxs, C Box &box, Flt pos_eps, Flt nrm_cos)
{
   Int unique=0;

   // link box->vtx
   Boxes boxes  (box        , vtxs.elms());
   Index box_vtx(boxes.num(), vtxs.elms());
   REPA(vtxs)box_vtx.elmGroup(boxes.index(vtxs[i].pos), i); box_vtx.set();

   // get duplicates
   Int xs=boxes.cells.x,
       ys=boxes.cells.y, xys=xs*ys;

   // iterate all boxes
   FREPD(z, boxes.cells.z)
   FREPD(y, boxes.cells.y)
   FREPD(x, boxes.cells.x)
   {
      Int         box_tests=0;
      IndexGroup *box_test[2+3+9],
                 *box_cur=&box_vtx.group[x + y*xs + z*xys]; // get current box

      // set which neighbor boxes to test
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

      FREPA(*box_cur) // iterate all vertexes in this box
      {
         Int        vtx_cur_i=  (*box_cur)[i]; // this is i-th vtx in this box
         VtxDupNrm &vtx_cur  =vtxs[vtx_cur_i];
       C Vec       &pos_cur  =vtx_cur.pos; // this is position of that vertex
       C Vec       &nrm_cur  =vtx_cur.nrm; // this is normal   of that vertex
         REPD(c, box_tests) // iterate all boxes to test
         {
            IndexGroup *bt=box_test[c];
            REPD(m, (box_cur==bt) ? i : bt->num) // iterate all vtxs in the test box (if the test box is the current box, then check only vertexes before the current one)
            {
               Int        vtx_test_i=(*bt)[m]; // this is m-th vtx in the test box
             C VtxDupNrm &vtx_test  =vtxs[vtx_test_i];
               if(vtx_test.dup==vtx_test_i // if this vtx is unique (points to self), this is so that we assign mapping only to uniqe vtxs (and not to vtxs that point to other vtxs)
               && Equal(pos_cur, vtx_test.pos, pos_eps) // if position is the same
               && Dot  (nrm_cur, vtx_test.nrm)>=nrm_cos)
               {
                  // found a duplicate
                  vtx_cur.dup=vtx_test_i; goto next; // set 'dup' index to point to unique 'vtx_test_i'
               }
            }
         }
         // haven't found a duplicate, so set as unique
         vtx_cur.dup=vtx_cur_i; // set 'dup' index to point to self
         unique++; // increase unique counter
      next:;
      }
   }
   return unique;
}
/******************************************************************************/
static Int CompareMatrixWeight(C VecB2 &matrix_weight_a, C VecB2 &matrix_weight_b)
{
   if(Int c=Compare(matrix_weight_b.y, matrix_weight_a.y))return c; // first compare by weight, reverse order to list most important first
   return   Compare(matrix_weight_b.x, matrix_weight_a.x)         ; // next  compare by matrix, reverse order to list children       first, and parents last (especially zero last)
}
void SetSkin(C MemPtrN<IndexWeight, 256> &skin, VecB4 &matrix, VecB4 &blend, C Skeleton *skeleton)
{
   const Int max_bone_influence=3; // only 3 bones are supported

   MemtN<IndexWeight, 256> temp; temp=skin;

   // remove invalid indexes
   if(skeleton)
   {
      const Int max_index=skeleton->bones.elms()+VIRTUAL_ROOT_BONE;
      REPA(temp)if(!InRange(temp[i].index, max_index))temp.remove(i);
   }

   // merge same bones (do this before removing empty references so we can have both positive and negative weights for the same matrix, used by the editor when changing skinning)
   REPA(temp)
   {
    C IndexWeight &a=temp[i]; REPD(j, i)
      {
         IndexWeight &b=temp[j]; if(a.index==b.index){b.weight+=a.weight; temp.remove(i); break;}
      }
   }

   // remove empty references (do this after merging the same bones)
   REPA(temp)if(temp[i].weight<=0)temp.remove(i);

   // sort from most to least important
   temp.sort(Compare);

   // remove those that won't fit
   if(skeleton) // if skeleton is provided
      for(Int i=temp.elms()-1; i>=max_bone_influence; i--) // iterate all weights that would get removed
   {
      Int bone=temp[i].index-VIRTUAL_ROOT_BONE,
          min_distance=0xFF+1,
          closest=0;
      REPD(j, i-1) // get all closest bones
      {
         Int distance =skeleton->hierarchyDistance(bone, temp[j].index-VIRTUAL_ROOT_BONE);
         if( distance< min_distance){min_distance=distance; closest=1;}else // if found smaller distance than previous then set it, and set closest bones to 1
         if( distance==min_distance){closest++;}                            // if distance found is equal to min then another bone is close, so increase it by 1
      }
      if(closest)
      {
         Flt weight=temp[i].weight/closest; REPD(j, i-1)if(skeleton->hierarchyDistance(bone, temp[j].index-VIRTUAL_ROOT_BONE)==min_distance)temp[j].weight+=weight; // add weight from this bone to all closest bones equally
      }
      temp.removeLast(); if(closest)temp.sort(Compare); // remove last element and re-sort if any weights were modified
   }
   if(temp.elms()>max_bone_influence)temp.setNum(max_bone_influence); // this is needed because we're calculating sum below

   Flt sum=0; REPA(temp)sum+=temp[i].weight;
   if( sum)
   {
      sum=255/sum; REPAO(temp).weight*=sum; // normalize and scale to 255
      switch(temp.elms())
      {
         case 0: goto zero; // shouldn't really happen

         case 1:
         {
            blend .set(255          , 0, 0, 0); // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, 0, 0, 0);
         }break;

         case 2:
         {
            Byte w=RoundPos(temp[0].weight);
            blend .set(w            , 255-w        , 0, 0); // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, temp[1].index, 0, 0);
         }break;

         case 3:
         {
            Byte w0=    RoundPos(temp[0].weight),
                 w1=Mid(RoundPos(temp[1].weight), 0, 255-w0); // limit to "255-w0" because we can't have "w0+w1>255"
            blend .set(w0           , w1           , 255-w0-w1    , 0); // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, temp[1].index, temp[2].index, 0);
         }break;

         default: // 4 or more
         {
            Byte w0=    RoundPos(temp[0].weight),
                 w1=Mid(RoundPos(temp[1].weight), 0, 255-w0   ), // limit to "255-w0   " because we can't have "w0+w1   >255"
                 w2=Mid(RoundPos(temp[2].weight), 0, 255-w0-w1); // limit to "255-w0-w1" because we can't have "w0+w1+w2>255"
            blend .set(w0           , w1           , w2           , 255-w0-w1-w2 ); // 'blend.sum' must be exactly equal to 255 !!
            matrix.set(temp[0].index, temp[1].index, temp[2].index, temp[3].index);
         }break;
      }
      REPA(blend)if(!blend.c[i])matrix.c[i]=0; // clear bones to 0 if they have no weight
      // sort matrix/weight to list most important first, and in case weights are the same, then sort by matrix index (this is needed because even though 'temp' is already sorted, we need to sort again because weights now in byte format can be the same, and in which case we need to sort by matrix index), we do this, so in the future we can compare 2 matrix weights using fast checks like "matrix0==matrix1 && weight0==weight1" instead of checking each matrix index component separately (for cases where they are listed in different order)
      if(temp.elms()>=3) // need to check this only for 3 or more bones, because 1 and 2 will never have this (1 has always 255,0,0,0 weights, and 2 has always w,255-w,0,0 weights, which means they are always different, because Byte w is always different than 255-w)
      {
         VecB2 matrix_weight[]={VecB2(matrix.x, blend.x), VecB2(matrix.y, blend.y), VecB2(matrix.z, blend.z), VecB2(matrix.w, blend.w)};
         Sort( matrix_weight, Elms(matrix_weight), CompareMatrixWeight);
         matrix.set(matrix_weight[0].x, matrix_weight[1].x, matrix_weight[2].x, matrix_weight[3].x);
         blend .set(matrix_weight[0].y, matrix_weight[1].y, matrix_weight[2].y, matrix_weight[3].y);
      }
   }else
   {
   zero:
      blend .set(255, 0, 0, 0); // 'blend.sum' must be exactly equal to 255 !!
      matrix=0;
   }
}
/******************************************************************************/
// VERTEX FULL
/******************************************************************************/
void VtxFull::reset()
{
   Zero(T);
   color=WHITE;
   material.x=255; // 'material.sum' must be exactly equal to 255 !!
   blend   .x=255; //    'blend.sum' must be exactly equal to 255 !!
}
VtxFull& VtxFull::from(C MeshBase &mshb, Int i)
{
   reset();
   if(InRange(i, mshb.vtx))
   {
      if(mshb.vtx.pos     ())pos     =mshb.vtx.pos     (i);
      if(mshb.vtx.nrm     ())nrm     =mshb.vtx.nrm     (i);
      if(mshb.vtx.tan     ())tan     =mshb.vtx.tan     (i);
      if(mshb.vtx.bin     ())bin     =mshb.vtx.bin     (i);
      if(mshb.vtx.hlp     ())hlp     =mshb.vtx.hlp     (i);
      if(mshb.vtx.tex0    ())tex0    =mshb.vtx.tex0    (i);
      if(mshb.vtx.tex1    ())tex1    =mshb.vtx.tex1    (i);
      if(mshb.vtx.tex2    ())tex2    =mshb.vtx.tex2    (i);
      if(mshb.vtx.color   ())color   =mshb.vtx.color   (i);
      if(mshb.vtx.material())material=mshb.vtx.material(i);
      if(mshb.vtx.matrix  ())matrix  =mshb.vtx.matrix  (i);
      if(mshb.vtx.blend   ())blend   =mshb.vtx.blend   (i);
      if(mshb.vtx.size    ())size    =mshb.vtx.size    (i);
   }
   return T;
}
void VtxFull::to(MeshBase &mshb, Int i)C
{
   if(InRange(i, mshb.vtx))
   {
      if(mshb.vtx.pos     ())mshb.vtx.pos     (i)=pos;
      if(mshb.vtx.nrm     ())mshb.vtx.nrm     (i)=nrm;
      if(mshb.vtx.tan     ())mshb.vtx.tan     (i)=tan;
      if(mshb.vtx.bin     ())mshb.vtx.bin     (i)=bin;
      if(mshb.vtx.hlp     ())mshb.vtx.hlp     (i)=hlp;
      if(mshb.vtx.tex0    ())mshb.vtx.tex0    (i)=tex0;
      if(mshb.vtx.tex1    ())mshb.vtx.tex1    (i)=tex1;
      if(mshb.vtx.tex2    ())mshb.vtx.tex2    (i)=tex2;
      if(mshb.vtx.color   ())mshb.vtx.color   (i)=color;
      if(mshb.vtx.material())mshb.vtx.material(i)=material;
      if(mshb.vtx.matrix  ())mshb.vtx.matrix  (i)=matrix;
      if(mshb.vtx.blend   ())mshb.vtx.blend   (i)=blend;
      if(mshb.vtx.size    ())mshb.vtx.size    (i)=size;
      if(mshb.vtx.dup     ())mshb.vtx.dup     (i)=i;
   }
}
VtxFull& VtxFull::avg(C VtxFull &a, C VtxFull &b)
{
   pos     =Avg (a.pos     , b.pos     );
   nrm     =   !(a.nrm     + b.nrm     );
   tan     =   !(a.tan     + b.tan     );
   bin     =   !(a.bin     + b.bin     );
   hlp     =Avg (a.hlp     , b.hlp     );
   tex0    =Avg (a.tex0    , b.tex0    );
   tex1    =Avg (a.tex1    , b.tex1    );
   tex2    =Avg (a.tex2    , b.tex2    );
   size    =Avg (a.size    , b.size    );
   color   =Avg (a.color   , b.color   );
   material=AvgI(a.material, b.material);

   MemtN<IndexWeight, 256> skin;
   FREPA(a.matrix)skin.New().set(a.matrix.c[i], a.blend.c[i]);
   FREPA(b.matrix)skin.New().set(b.matrix.c[i], b.blend.c[i]);
   SetSkin(skin, matrix, blend, null);

   return T;
}
VtxFull& VtxFull::lerp(C VtxFull &a, C VtxFull &b, Flt step)
{
   Flt step1=1-step;
   pos     =     a.pos *step1 + b.pos *step;
   nrm     =   !(a.nrm *step1 + b.nrm *step);
   tan     =   !(a.tan *step1 + b.tan *step);
   bin     =   !(a.bin *step1 + b.bin *step);
   hlp     =     a.hlp *step1 + b.hlp *step;
   tex0    =     a.tex0*step1 + b.tex0*step;
   tex1    =     a.tex1*step1 + b.tex1*step;
   tex2    =     a.tex2*step1 + b.tex2*step;
   size    =     a.size*step1 + b.size*step;
   color   =Lerp(a.color   , b.color   , step);
   material=Lerp(a.material, b.material, step);

   MemtN<IndexWeight, 256> skin;
   FREPA(a.matrix)skin.New().set(a.matrix.c[i], a.blend.c[i]*step1);
   FREPA(b.matrix)skin.New().set(b.matrix.c[i], b.blend.c[i]*step );
   SetSkin(skin, matrix, blend, null);

   return T;
}
VtxFull& VtxFull::lerp(C VtxFull &a, C VtxFull &b, C VtxFull &c, C Vec &blend)
{
   Flt ba=blend.c[0],
       bb=blend.c[1],
       bc=blend.c[2];

   T.pos     =  a.pos *ba + b.pos *bb + c.pos *bc ;
   T.nrm     =!(a.nrm *ba + b.nrm *bb + c.nrm *bc);
   T.tan     =!(a.tan *ba + b.tan *bb + c.tan *bc);
   T.bin     =!(a.bin *ba + b.bin *bb + c.bin *bc);
   T.hlp     =  a.hlp *ba + b.hlp *bb + c.hlp *bc ;
   T.tex0    =  a.tex0*ba + b.tex0*bb + c.tex0*bc ;
   T.tex1    =  a.tex1*ba + b.tex1*bb + c.tex1*bc ;
   T.tex2    =  a.tex2*ba + b.tex2*bb + c.tex2*bc ;
   T.size    =  a.size*ba + b.size*bb + c.size*bc ;
   T.color   =Lerp(a.color   , b.color   , c.color   , blend);
   T.material=Lerp(a.material, b.material, c.material, blend);

   MemtN<IndexWeight, 256> skin;
   FREPA(a.matrix)skin.New().set(a.matrix.c[i], a.blend.c[i]*ba);
   FREPA(b.matrix)skin.New().set(b.matrix.c[i], b.blend.c[i]*bb);
   FREPA(c.matrix)skin.New().set(c.matrix.c[i], c.blend.c[i]*bc);
   SetSkin(skin, T.matrix, T.blend, null);

   return T;
}
VtxFull& VtxFull::mul(C Matrix &matrix, C Matrix3 &matrix3)
{
   pos*=matrix ;
   hlp*=matrix ;
   nrm*=matrix3;
   tan*=matrix3;
   bin*=matrix3;
   return T;
}
/******************************************************************************/
// MESH BASE
/******************************************************************************/
UInt MeshBase::flag()C
{
   UInt f=0;

   if(vtx.pos     ())f|=VTX_POS;
   if(vtx.nrm     ())f|=VTX_NRM;
   if(vtx.tan     ())f|=VTX_TAN;
   if(vtx.bin     ())f|=VTX_BIN;
   if(vtx.hlp     ())f|=VTX_HLP;
   if(vtx.tex0    ())f|=VTX_TEX0;
   if(vtx.tex1    ())f|=VTX_TEX1;
   if(vtx.tex2    ())f|=VTX_TEX2;
   if(vtx.matrix  ())f|=VTX_MATRIX;
   if(vtx.blend   ())f|=VTX_BLEND;
   if(vtx.size    ())f|=VTX_SIZE;
   if(vtx.material())f|=VTX_MATERIAL;
   if(vtx.color   ())f|=VTX_COLOR;
   if(vtx.flag    ())f|=VTX_FLAG;
   if(vtx.dup     ())f|=VTX_DUP;

   if(edge.ind    ())f|=EDGE_IND;
   if(edge.adjFace())f|=EDGE_ADJ_FACE;
   if(edge.nrm    ())f|=EDGE_NRM;
   if(edge.flag   ())f|=EDGE_FLAG;
   if(edge.id     ())f|=EDGE_ID;

   if(tri.ind    ())f|=TRI_IND;
   if(tri.adjFace())f|=TRI_ADJ_FACE;
   if(tri.adjEdge())f|=TRI_ADJ_EDGE;
   if(tri.nrm    ())f|=TRI_NRM;
   if(tri.flag   ())f|=TRI_FLAG;
   if(tri.id     ())f|=TRI_ID;

   if(quad.ind    ())f|=QUAD_IND;
   if(quad.adjFace())f|=QUAD_ADJ_FACE;
   if(quad.adjEdge())f|=QUAD_ADJ_EDGE;
   if(quad.nrm    ())f|=QUAD_NRM;
   if(quad.flag   ())f|=QUAD_FLAG;
   if(quad.id     ())f|=QUAD_ID;

   return f;
}
UInt MeshBase::memUsage()C
{
   UInt size=0;
   if(Int elms=vtx.elms())
   {
      UInt s=0;
      if(vtx.pos     ())s+=SIZE(*vtx.pos     ());
      if(vtx.nrm     ())s+=SIZE(*vtx.nrm     ());
      if(vtx.tan     ())s+=SIZE(*vtx.tan     ());
      if(vtx.bin     ())s+=SIZE(*vtx.bin     ());
      if(vtx.hlp     ())s+=SIZE(*vtx.hlp     ());
      if(vtx.tex0    ())s+=SIZE(*vtx.tex0    ());
      if(vtx.tex1    ())s+=SIZE(*vtx.tex1    ());
      if(vtx.tex2    ())s+=SIZE(*vtx.tex2    ());
      if(vtx.matrix  ())s+=SIZE(*vtx.matrix  ());
      if(vtx.blend   ())s+=SIZE(*vtx.blend   ());
      if(vtx.size    ())s+=SIZE(*vtx.size    ());
      if(vtx.material())s+=SIZE(*vtx.material());
      if(vtx.color   ())s+=SIZE(*vtx.color   ());
      if(vtx.flag    ())s+=SIZE(*vtx.flag    ());
      if(vtx.dup     ())s+=SIZE(*vtx.dup     ());
      size+=s*elms;
   }
   if(Int elms=edge.elms())
   {
      UInt s=0;
      if(edge.ind    ())s+=SIZE(*edge.ind    ());
      if(edge.adjFace())s+=SIZE(*edge.adjFace());
      if(edge.nrm    ())s+=SIZE(*edge.nrm    ());
      if(edge.flag   ())s+=SIZE(*edge.flag   ());
      if(edge.id     ())s+=SIZE(*edge.id     ());
      size+=s*elms;
   }
   if(Int elms=tri.elms())
   {
      UInt s=0;
      if(tri.ind    ())s+=SIZE(*tri.ind    ());
      if(tri.adjFace())s+=SIZE(*tri.adjFace());
      if(tri.adjEdge())s+=SIZE(*tri.adjEdge());
      if(tri.nrm    ())s+=SIZE(*tri.nrm    ());
      if(tri.flag   ())s+=SIZE(*tri.flag   ());
      if(tri.id     ())s+=SIZE(*tri.id     ());
      size+=s*elms;
   }
   if(Int elms=quad.elms())
   {
      UInt s=0;
      if(quad.ind    ())s+=SIZE(*quad.ind    ());
      if(quad.adjFace())s+=SIZE(*quad.adjFace());
      if(quad.adjEdge())s+=SIZE(*quad.adjEdge());
      if(quad.nrm    ())s+=SIZE(*quad.nrm    ());
      if(quad.flag   ())s+=SIZE(*quad.flag   ());
      if(quad.id     ())s+=SIZE(*quad.id     ());
      size+=s*elms;
   }
   return size;
}
MeshBase& MeshBase::include(UInt f)
{
   if(f&VTX_ALL)
   {
      Int elms=vtx.elms();
      if(f&VTX_POS      && !vtx._pos     )Alloc(vtx._pos     , elms);
      if(f&VTX_NRM      && !vtx._nrm     )Alloc(vtx._nrm     , elms);
      if(f&VTX_TAN      && !vtx._tan     )Alloc(vtx._tan     , elms);
      if(f&VTX_BIN      && !vtx._bin     )Alloc(vtx._bin     , elms);
      if(f&VTX_HLP      && !vtx._hlp     )Alloc(vtx._hlp     , elms);
      if(f&VTX_TEX0     && !vtx._tex0    )Alloc(vtx._tex0    , elms);
      if(f&VTX_TEX1     && !vtx._tex1    )Alloc(vtx._tex1    , elms);
      if(f&VTX_TEX2     && !vtx._tex2    )Alloc(vtx._tex2    , elms);
      if(f&VTX_MATRIX   && !vtx._matrix  )Alloc(vtx._matrix  , elms);
      if(f&VTX_BLEND    && !vtx._blend   )Alloc(vtx._blend   , elms);
      if(f&VTX_SIZE     && !vtx._size    )Alloc(vtx._size    , elms);
      if(f&VTX_MATERIAL && !vtx._material)Alloc(vtx._material, elms);
      if(f&VTX_COLOR    && !vtx._color   )Alloc(vtx._color   , elms);
      if(f&VTX_FLAG     && !vtx._flag    )Alloc(vtx._flag    , elms);
      if(f&VTX_DUP      && !vtx._dup     )Alloc(vtx._dup     , elms);
   }
   if(f&EDGE_ALL)
   {
      Int elms=edge.elms();
      if(f&EDGE_IND      && !edge._ind     )Alloc(edge._ind     , elms);
      if(f&EDGE_ADJ_FACE && !edge._adj_face)Alloc(edge._adj_face, elms);
      if(f&EDGE_NRM      && !edge._nrm     )Alloc(edge._nrm     , elms);
      if(f&EDGE_FLAG     && !edge._flag    )Alloc(edge._flag    , elms);
      if(f&EDGE_ID       && !edge._id      )Alloc(edge._id      , elms);
   }
   if(f&TRI_ALL)
   {
      Int elms=tri.elms();
      if(f&TRI_IND      && !tri._ind     )Alloc(tri._ind     , elms);
      if(f&TRI_ADJ_FACE && !tri._adj_face)Alloc(tri._adj_face, elms);
      if(f&TRI_ADJ_EDGE && !tri._adj_edge)Alloc(tri._adj_edge, elms);
      if(f&TRI_NRM      && !tri._nrm     )Alloc(tri._nrm     , elms);
      if(f&TRI_FLAG     && !tri._flag    )Alloc(tri._flag    , elms);
      if(f&TRI_ID       && !tri._id      )Alloc(tri._id      , elms);
   }
   if(f&QUAD_ALL)
   {
      Int elms=quad.elms();
      if(f&QUAD_IND      && !quad._ind     )Alloc(quad._ind     , elms);
      if(f&QUAD_ADJ_FACE && !quad._adj_face)Alloc(quad._adj_face, elms);
      if(f&QUAD_ADJ_EDGE && !quad._adj_edge)Alloc(quad._adj_edge, elms);
      if(f&QUAD_NRM      && !quad._nrm     )Alloc(quad._nrm     , elms);
      if(f&QUAD_FLAG     && !quad._flag    )Alloc(quad._flag    , elms);
      if(f&QUAD_ID       && !quad._id      )Alloc(quad._id      , elms);
   }
   return T;
}
MeshBase& MeshBase::exclude(UInt f)
{
   if(f&VTX_ALL)
   {
      if(f&VTX_POS     )Free(vtx._pos     );
      if(f&VTX_NRM     )Free(vtx._nrm     );
      if(f&VTX_TAN     )Free(vtx._tan     );
      if(f&VTX_BIN     )Free(vtx._bin     );
      if(f&VTX_HLP     )Free(vtx._hlp     );
      if(f&VTX_TEX0    )Free(vtx._tex0    );
      if(f&VTX_TEX1    )Free(vtx._tex1    );
      if(f&VTX_TEX2    )Free(vtx._tex2    );
      if(f&VTX_MATRIX  )Free(vtx._matrix  );
      if(f&VTX_BLEND   )Free(vtx._blend   );
      if(f&VTX_SIZE    )Free(vtx._size    );
      if(f&VTX_MATERIAL)Free(vtx._material);
      if(f&VTX_COLOR   )Free(vtx._color   );
      if(f&VTX_FLAG    )Free(vtx._flag    );
      if(f&VTX_DUP     )Free(vtx._dup     );
   }
   if(f&EDGE_ALL)
   {
      if(f&EDGE_IND     )Free(edge._ind     );
      if(f&EDGE_ADJ_FACE)Free(edge._adj_face);
      if(f&EDGE_NRM     )Free(edge._nrm     );
      if(f&EDGE_FLAG    )Free(edge._flag    );
      if(f&EDGE_ID      )Free(edge._id      );
   }
   if(f&TRI_ALL)
   {
      if(f&TRI_IND     )Free(tri._ind     );
      if(f&TRI_ADJ_FACE)Free(tri._adj_face);
      if(f&TRI_ADJ_EDGE)Free(tri._adj_edge);
      if(f&TRI_NRM     )Free(tri._nrm     );
      if(f&TRI_FLAG    )Free(tri._flag    );
      if(f&TRI_ID      )Free(tri._id      );
   }
   if(f&QUAD_ALL)
   {
      if(f&QUAD_IND     )Free(quad._ind     );
      if(f&QUAD_ADJ_FACE)Free(quad._adj_face);
      if(f&QUAD_ADJ_EDGE)Free(quad._adj_edge);
      if(f&QUAD_NRM     )Free(quad._nrm     );
      if(f&QUAD_FLAG    )Free(quad._flag    );
      if(f&QUAD_ID      )Free(quad._id      );
   }

   f=flag();
   if(!(f& VTX_ALL))vtx ._elms=0;
   if(!(f&EDGE_ALL))edge._elms=0;
   if(!(f& TRI_ALL))tri ._elms=0;
   if(!(f&QUAD_ALL))quad._elms=0;
   return T;
}
MeshBase& MeshBase::keepOnly(UInt f) {return exclude(~f);}
/******************************************************************************/
MeshBase& MeshBase::del()
{
   return keepOnly(0);
}
MeshBase& MeshBase::create(Int vtxs, Int edges, Int tris, Int quads, UInt flag)
{
   del();

   vtx ._elms=vtxs ;
   edge._elms=edges;
   tri ._elms=tris ;
   quad._elms=quads;
   include(flag|VTX_POS|EDGE_IND|FACE_IND);

   return T;
}
/******************************************************************************/
#if WINDOWS_OLD
static void SetWeight(VecB4 &weight, Vec4 src) // 'weight.sum' must be always equal to 255 !!
{
   if(Flt sum=src.sum())src/=sum; // normalize

   if(src.z) // currently there's limit of 3 max bone influences, so ignore src.w and set it to 0
   {
      Byte w0=    RoundPos(src.x*255),
           w1=Mid(RoundPos(src.y*255), 0, 255-w0);
      weight.set(w0, w1, 255-w0-w1, 0);
   }else
   if(src.y)
   {
      Byte w=RoundPos(src.x*255);
      weight.set(w, 255-w, 0, 0);
   }else
   {
      weight.set(255, 0, 0, 0);
   }
}
static Bool Create(MeshBase &mesh, IDirect3DVertexBuffer9 *vtx_buffer, D3DVERTEXELEMENT9 ve[], Int vtx_size, Int vtx_num, Bool allow_binormal)
{
   mesh.exclude(VTX_ALL); if(!vtx_num)return true;

   SyncLocker locker(D._lock);
   Byte *vtx_buf; if(OK(vtx_buffer->Lock(0, 0, (Ptr*)&vtx_buf, D3DLOCK_READONLY)))
   {
      // check elements
      UInt flag=0;
      Bool bin_from_tan=false, bin_from_bin=false;
      FREP(MAX_FVF_DECL_SIZE)
      {
         D3DVERTEXELEMENT9 &v=ve[i];
         if(v.Stream==0xFF)break;else switch(v.Usage)
         {
            case D3DDECLUSAGE_POSITION    : if(v.UsageIndex==0)flag|=VTX_POS;else if(v.UsageIndex==1)flag|=VTX_HLP; break;
            case D3DDECLUSAGE_BLENDWEIGHT : flag|=VTX_BLEND ; break;
            case D3DDECLUSAGE_BLENDINDICES: flag|=VTX_MATRIX; break;
            case D3DDECLUSAGE_NORMAL      : if(v.Type==D3DDECLTYPE_FLOAT3)flag|=VTX_NRM;else if(v.Type==D3DDECLTYPE_UBYTE4N) flag|=VTX_NRM;                     break;
            case D3DDECLUSAGE_TANGENT     : if(v.Type==D3DDECLTYPE_FLOAT3)flag|=VTX_TAN;else if(v.Type==D3DDECLTYPE_UBYTE4N){flag|=VTX_TAN; bin_from_tan=true;} break;
            case D3DDECLUSAGE_BINORMAL    : if(v.Type==D3DDECLTYPE_FLOAT3)flag|=VTX_BIN;                                                    bin_from_bin=true;  break;
            case D3DDECLUSAGE_PSIZE       : flag|=VTX_SIZE; break;
            case D3DDECLUSAGE_TEXCOORD    : if(v.UsageIndex==0)flag|=VTX_TEX0    ;else if(v.UsageIndex==1)flag|=VTX_TEX1 ;else if(v.UsageIndex==2)flag|=VTX_TEX2; break;
            case D3DDECLUSAGE_COLOR       : if(v.UsageIndex==0)flag|=VTX_MATERIAL;else if(v.UsageIndex==1)flag|=VTX_COLOR; break;
         }
      }

      // update binormal info
      if(bin_from_tan)
      {
         if(bin_from_bin || !(flag&VTX_NRM) || !allow_binormal)bin_from_tan=false;else flag|=VTX_BIN;
      }

      // create vertexes
      mesh.vtx._elms=vtx_num;
      mesh.include(flag);

      // copy data
      FREP(MAX_FVF_DECL_SIZE)
      {
         D3DVERTEXELEMENT9 &v=ve[i];
         if(v.Stream==0xFF)break;else
         {
            Byte *vd=vtx_buf+v.Offset; // vtx_data
            switch(v.Usage)
            {
               case D3DDECLUSAGE_POSITION:
                  if(v.UsageIndex==0)REPA(mesh.vtx)mesh.vtx.pos(i)=*(Vec*)(vd+i*vtx_size);else
                  if(v.UsageIndex==1)REPA(mesh.vtx)mesh.vtx.hlp(i)=*(Vec*)(vd+i*vtx_size);
               break;

               case D3DDECLUSAGE_NORMAL:
                  if(v.Type==D3DDECLTYPE_FLOAT3 )REPA(mesh.vtx)mesh.vtx.nrm(i)=            *(Vec  *)(vd+i*vtx_size) ;else
                  if(v.Type==D3DDECLTYPE_UBYTE4N)REPA(mesh.vtx)mesh.vtx.nrm(i)=UByte4ToNrm(*(VecB4*)(vd+i*vtx_size));
               break;

               case D3DDECLUSAGE_TANGENT:
                  if(v.Type==D3DDECLTYPE_FLOAT3 )REPA(mesh.vtx)mesh.vtx.tan(i)=*(Vec*)(vd+i*vtx_size);else
                  if(v.Type==D3DDECLTYPE_UBYTE4N)REPA(mesh.vtx)
                  {
                     VecB4 tan=*(VecB4*)(vd+i*vtx_size);
                                     mesh.vtx.tan(i)  =UByte4ToNrm(tan);
                     if(bin_from_tan)mesh.vtx.bin(i).x=((tan.w>=128) ? 1.0f : -1.0f);
                  }
               break;

               case D3DDECLUSAGE_BINORMAL:
                  if(v.Type==D3DDECLTYPE_FLOAT3)REPA(mesh.vtx)mesh.vtx.bin(i)=*(Vec*)(vd+i*vtx_size); break;
               break;

               case D3DDECLUSAGE_PSIZE       : REPA(mesh.vtx)mesh.vtx.size  (i)=*(Flt  *)(vd+i*vtx_size); break;
               case D3DDECLUSAGE_BLENDINDICES: REPA(mesh.vtx)mesh.vtx.matrix(i)=*(VecB4*)(vd+i*vtx_size); break;

               case D3DDECLUSAGE_BLENDWEIGHT:
                  if(v.UsageIndex==0)REPA(mesh.vtx)
                  {
                     if(v.Type==D3DDECLTYPE_UBYTE4
                     || v.Type==D3DDECLTYPE_UBYTE4N){VecB4 v=*(VecB4*)(vd+i*vtx_size); SetWeight(mesh.vtx.blend(i),      v            );}else
                     if(v.Type==D3DDECLTYPE_FLOAT1 ){Flt   f=*(Flt  *)(vd+i*vtx_size); SetWeight(mesh.vtx.blend(i), Vec4(f, 1-f, 0, 0));}else
                     if(v.Type==D3DDECLTYPE_FLOAT3 ){Vec   v=*(Vec  *)(vd+i*vtx_size); SetWeight(mesh.vtx.blend(i), Vec4(v        , 0));}else
                     if(v.Type==D3DDECLTYPE_FLOAT4 ){Vec4  v=*(Vec4 *)(vd+i*vtx_size); SetWeight(mesh.vtx.blend(i),      v            );}
                  }
               break;

               case D3DDECLUSAGE_TEXCOORD:
                  if(v.UsageIndex==0)REPA(mesh.vtx)mesh.vtx.tex0(i)=*(Vec2*)(vd+i*vtx_size);else
                  if(v.UsageIndex==1)REPA(mesh.vtx)mesh.vtx.tex1(i)=*(Vec2*)(vd+i*vtx_size);else
                  if(v.UsageIndex==2)REPA(mesh.vtx)mesh.vtx.tex2(i)=*(Vec2*)(vd+i*vtx_size);
               break;

               case D3DDECLUSAGE_COLOR:
                  if(v.UsageIndex==0)REPA(mesh.vtx)mesh.vtx.material(i)=*(VecB4*)(vd+i*vtx_size);else
                  if(v.UsageIndex==1)REPA(mesh.vtx)
                  {
                     if(v.Type==D3DDECLTYPE_UBYTE4N ) mesh.vtx.color(i)=*(Color*)(vd+i*vtx_size);else
                     if(v.Type==D3DDECLTYPE_D3DCOLOR){mesh.vtx.color(i)=*(Color*)(vd+i*vtx_size); Swap(mesh.vtx.color(i).r, mesh.vtx.color(i).b);}
                  }
               break;
            }
         }
      }

      // update binormals
      if(bin_from_tan)REPA(mesh.vtx)mesh.vtx.bin(i)=mesh.vtx.bin(i).x*Cross(mesh.vtx.nrm(i), mesh.vtx.tan(i));

      // add missing vertex blends when only matrix indexes provided
      if(mesh.vtx.matrix() && !mesh.vtx.blend())
      {
         mesh.include(VTX_BLEND);
         REPA(mesh.vtx)mesh.vtx.blend(i).set(255, 0, 0, 0);
      }

      vtx_buffer->Unlock();
      return true;
   }
   return false;
}
static Bool Create(MeshBase &mesh, IDirect3DIndexBuffer9 *ind_buffer, Bool bit16, Int tris)
{
   mesh.exclude(FACE_ALL|ADJ_ALL); if(tris<=0)return true;

   SyncLocker locker(D._lock);
   Ptr ind; if(OK(ind_buffer->Lock(0, 0, &ind, D3DLOCK_READONLY)))
   {
      mesh.tri._elms=tris;
      mesh.include(TRI_IND);

      if(bit16)Copy16To32(mesh.tri.ind(), ind, tris*3);
      else     Copy32To32(mesh.tri.ind(), ind, tris*3);

      ind_buffer->Unlock();
      return true;
   }
   return false;
}
#if 0
Bool MeshBase::create(ID3DXMesh *mesh)
{
   Bool ok;
   del(); if(!mesh)return true;
   SyncLocker locker(D._lock);

   // create vertexes
   D3DVERTEXELEMENT9       ve[MAX_FVF_DECL_SIZE]; if(!OK(mesh->GetDeclaration ( ve     )))return false;
   IDirect3DVertexBuffer9 *vtx_buf=null;          if(!OK(mesh->GetVertexBuffer(&vtx_buf)))return false;
   ok=Create(T, vtx_buf, ve, mesh->GetNumBytesPerVertex(), mesh->GetNumVertices(), true); RELEASE(vtx_buf); if(!ok)return false;

   // create indexes
   IDirect3DIndexBuffer9 *ind_buf=null; if(!OK(mesh->GetIndexBuffer(&ind_buf)))return false;
   ok=Create(T, ind_buf, !FlagTest(mesh->GetOptions(), D3DXMESH_32BIT), mesh->GetNumFaces()); RELEASE(ind_buf); if(!ok)return false;

   // set attributes
   D3DXATTRIBUTERANGE attrib[256];
   DWORD              elms=Elms(attrib);
   if(OK(mesh->GetAttributeTable(attrib, &elms)))
   {
      if(elms>1 || attrib[0].AttribId!=0) // required by DirectX .X importer
      {
         include(TRI_ID);
         REP(elms)
         {
            Int id   =attrib[i].AttribId ,
                start=attrib[i].FaceStart;
            REPD(f,   attrib[i].FaceCount)tri.id(start+f)=id;
         }
      }
   }
   return true;
}
#endif
#endif
/******************************************************************************/
T1(TYPE) static void Set(C Byte* &v, Int i, TYPE *data, UInt flag) {if(data)data[i]=*(TYPE*)v; if(flag)v+=SIZE(TYPE);}

Bool MeshBase::createVtx(C VtxBuf &vb, UInt flag, UInt storage, MeshRender::BoneSplit *bone_split, Int bone_splits, UInt flag_and)
{
   exclude(VTX_ALL);

   if(!vb._vtx_num)return true;

   if(C Byte *v=vb.lockRead())
   {
      vtx._elms=vb._vtx_num; include(VTX_ALL&flag&flag_and);
      if(storage&MSHR_COMPRESS)
      {
         Bool sign=FlagTest(storage, MSHR_SIGNED);
         FREPA(vtx)
         {
            Set(v, i, vtx.pos     (), flag&VTX_POS     );
            if(vtx.nrm()             )vtx.nrm(i)=(sign ? SByte4ToNrm : UByte4ToNrm)(*(VecB4*)v); if(flag&VTX_NRM)v+=SIZE(VecB4);
            if(vtx.tan() || vtx.bin())(sign ? SByte4ToTan : UByte4ToTan)(*(VecB4*)v, vtx.tan() ? &vtx.tan(i) : null, vtx.bin() ? &vtx.bin(i) : null, vtx.nrm() ? &vtx.nrm(i) : null); if(flag&VTX_TAN_BIN)v+=SIZE(VecB4);
            Set(v, i, vtx.hlp     (), flag&VTX_HLP     );
            Set(v, i, vtx.tex0    (), flag&VTX_TEX0    );
            Set(v, i, vtx.tex1    (), flag&VTX_TEX1    );
            Set(v, i, vtx.tex2    (), flag&VTX_TEX2    );
            Set(v, i, vtx.matrix  (), flag&VTX_MATRIX  );
            Set(v, i, vtx.blend   (), flag&VTX_BLEND   );
            Set(v, i, vtx.size    (), flag&VTX_SIZE    );
            Set(v, i, vtx.material(), flag&VTX_MATERIAL);
            Set(v, i, vtx.color   (), flag&VTX_COLOR   );
         }
      }else
      {
         FREPA(vtx)
         {
            Set(v, i, vtx.pos     (), flag&VTX_POS     );
            Set(v, i, vtx.nrm     (), flag&VTX_NRM     );
            Set(v, i, vtx.tan     (), flag&VTX_TAN     );
            Set(v, i, vtx.bin     (), flag&VTX_BIN     );
            Set(v, i, vtx.hlp     (), flag&VTX_HLP     );
            Set(v, i, vtx.tex0    (), flag&VTX_TEX0    );
            Set(v, i, vtx.tex1    (), flag&VTX_TEX1    );
            Set(v, i, vtx.tex2    (), flag&VTX_TEX2    );
            Set(v, i, vtx.matrix  (), flag&VTX_MATRIX  );
            Set(v, i, vtx.blend   (), flag&VTX_BLEND   );
            Set(v, i, vtx.size    (), flag&VTX_SIZE    );
            Set(v, i, vtx.material(), flag&VTX_MATERIAL);
            Set(v, i, vtx.color   (), flag&VTX_COLOR   );
         }
      }
      vb.unlock();

      // restore real bone matrix indexes
      if((storage&MSHR_BONE_SPLIT) && bone_split)if(VecB4 *matrix=vtx.matrix())FREP(bone_splits)
      {
       C MeshRender::BoneSplit &split=bone_split[i];
         FREP(split.vtxs)
         {
            matrix->x=split.split_to_real[matrix->x];
            matrix->y=split.split_to_real[matrix->y];
            matrix->z=split.split_to_real[matrix->z];
            matrix->w=split.split_to_real[matrix->w];
            matrix++;
         }
      }
      return true;
   }
   return false;
}
/******************************************************************************/
Bool MeshBase::createInd(C IndBuf &ib)
{
   exclude(FACE_ALL|ADJ_ALL);

   if(ib._ind_num/3<=0)return true;

   if(CPtr ind=ib.lockRead())
   {
      tri._elms=ib._ind_num/3;
      include(TRI_IND);

      if(ib.bit16())Copy16To32(tri.ind(), ind, tri.elms()*3);
      else          Copy32To32(tri.ind(), ind, tri.elms()*3);

      ib.unlock();
      return true;
   }
   return false;
}
/******************************************************************************/
MeshBase& MeshBase::create(C MeshRender &src, UInt flag_and)
{
   del();
   if(createVtx(src._vb, src.flag(), src._storage, src._bone_split, src._bone_splits, flag_and))
      createInd(src._ib);
   return T;
}
/******************************************************************************/
MeshBase& MeshBase::createPhys(C MeshLod &src, UInt flag_and, Bool set_face_id_from_part_index, Bool skip_hidden_parts)
{
   UInt part_flag=MSHP_NO_PHYS_BODY; if(skip_hidden_parts)part_flag|=MSHP_HIDDEN;
   REPA(src)if(src.parts[i].part_flag&part_flag) // if at least one needs to be removed
   {
      Memt<MeshBase> meshes; REPAD(p, src)if(!(src.parts[p].part_flag&part_flag)) // copy all desired MeshBase into container
      {
         MeshBase &mesh=meshes.New();
         mesh.create(src.parts[p], flag_and);
         if(set_face_id_from_part_index)
         {
            mesh.include(FACE_ID);
            REPA(mesh.tri )mesh.tri .id(i)=p;
            REPA(mesh.quad)mesh.quad.id(i)=p;
         }
      }
      create(meshes.data(), meshes.elms()); // create basing on the container
      return T;
   }
   create(src, flag_and, set_face_id_from_part_index); // create basing on the whole 'MeshLod'
   return T;
}
/******************************************************************************/
#if PHYSX
Bool MeshBase::create(PxConvexMesh &convex)
{
   Int           tris=0;
   PxHullPolygon poly;
   const PxU8   *ind=convex.getIndexBuffer();

   // calculate number of tris
   REP(convex.getNbPolygons())
      if(convex.getPolygonData(i, poly))tris+=Max(0, poly.mNbVerts-2);

   // create
   create(convex.getNbVertices(), 0, tris, 0);

   // copy vertexes
   CopyN(vtx.pos(), (Vec*)convex.getVertices(), vtxs());

   // setup tris
   tris=0;
   FREP(convex.getNbPolygons())
      if(convex.getPolygonData(i, poly))
         FREP(poly.mNbVerts-2)
            tri.ind(tris++).set(ind[poly.mIndexBase+0], ind[poly.mIndexBase+i+1], ind[poly.mIndexBase+i+2]);

   return true;
}
Bool MeshBase::create(PxTriangleMesh &mesh)
{
   create(mesh.getNbVertices(), 0, mesh.getNbTriangles(), 0);
                                                                      CopyN     (vtx.pos(), (Vec *)mesh.getVertices (), vtxs()  );
   if(mesh.getTriangleMeshFlags()&PxTriangleMeshFlag::e16_BIT_INDICES)Copy16To32(tri.ind(),        mesh.getTriangles(), tris()*3);
   else                                                               CopyN     (tri.ind(), (VecI*)mesh.getTriangles(), tris()  );
   return true;
}
#else
Bool MeshBase::create(btConvexHullShape &convex)
{
   Memt<Vec> points; points.setNum(convex.getNumPoints()); REPA(points)points[i]=Bullet.vec(convex.getScaledPoint(i));
   createConvex(points.data(), points.elms());
   return true;
}
Bool MeshBase::create(btBvhTriangleMeshShape &mesh)
{
   Bool ok=false;
   if(btStridingMeshInterface *smi=mesh.getMeshInterface())
   {
      Vec           *pos;
      VecI          *ind;
      int            vtxs, tris, vtx_stride, ind_stride;
      PHY_ScalarType vtx_type, ind_type;
      smi->getLockedReadOnlyVertexIndexBase((const unsigned char**)&pos, vtxs, vtx_type, vtx_stride, (const unsigned char**)&ind, ind_stride, tris, ind_type);
      if(vtx_type==PHY_FLOAT && ind_type==PHY_INTEGER)
      {
         create(vtxs, 0, tris, 0);
         CopyN (vtx.pos(), pos, vtxs);
         CopyN (tri.ind(), ind, tris);
         ok=true;
      }
      smi->unLockReadOnlyVertexBase(0);
   }
   if(!ok)del(); return ok;
}
#endif
MeshBase& MeshBase::create(C PhysPart &part)
{
   switch(part.type())
   {
      case PHYS_SHAPE: return create(part.shape, 0, (part.shape.type==SHAPE_BALL) ? 16 : ShapeTypeRound(part.shape.type) ? 32 : -1);

      case PHYS_CONVEX:
      case PHYS_MESH  : if(part._pm)
      {
         if(part._pm->_base  ){create(*part._pm->_base  ); return T;}
         if(part._pm->_convex){create(*part._pm->_convex); return T;}
         if(part._pm->_mesh  ){create(*part._pm->_mesh  ); return T;}
      }break;
   }
   return del();
}
/******************************************************************************/
MeshBase& MeshBase::create(C MeshBase *src[], Int elms, UInt flag_and, Bool set_face_id_from_part_index)
{
   if(!src)elms=0;

   UInt     flag=0;
   Int      vtxs=0, edges=0, tris=0, quads=0;
   MeshBase temp;

   REP(elms)if(C MeshBase *mesh=src[i])
   {
      flag |=mesh->flag ();
      vtxs +=mesh->vtxs ();
      edges+=mesh->edges();
      tris +=mesh->tris ();
      quads+=mesh->quads();
   }

   flag&=flag_and&(~(VTX_DUP|ADJ_ALL)); if(set_face_id_from_part_index)flag|=ID_ALL;
   temp.create(vtxs, edges, tris, quads, flag);

   Vec   *vtx_pos     =temp.vtx.pos     ();
   Vec   *vtx_nrm     =temp.vtx.nrm     ();
   Vec   *vtx_tan     =temp.vtx.tan     ();
   Vec   *vtx_bin     =temp.vtx.bin     ();
   Vec   *vtx_hlp     =temp.vtx.hlp     ();
   Vec2  *vtx_tex0    =temp.vtx.tex0    ();
   Vec2  *vtx_tex1    =temp.vtx.tex1    ();
   Vec2  *vtx_tex2    =temp.vtx.tex2    ();
   VecB4 *vtx_matrix  =temp.vtx.matrix  ();
   VecB4 *vtx_blend   =temp.vtx.blend   ();
   Flt   *vtx_size    =temp.vtx.size    ();
   VecB4 *vtx_material=temp.vtx.material();
   Color *vtx_color   =temp.vtx.color   ();
   Byte  *vtx_flag    =temp.vtx.flag    ();

   VecI2 *edge_ind =temp.edge.ind ();
   Vec   *edge_nrm =temp.edge.nrm ();
   Byte  *edge_flag=temp.edge.flag();
   Int   *edge_id  =temp.edge.id  ();

   VecI  *tri_ind =temp.tri.ind ();
   Vec   *tri_nrm =temp.tri.nrm ();
   Byte  *tri_flag=temp.tri.flag();
   Int   *tri_id  =temp.tri.id  ();

   VecI4 *quad_ind =temp.quad.ind ();
   Vec   *quad_nrm =temp.quad.nrm ();
   Byte  *quad_flag=temp.quad.flag();
   Int   *quad_id  =temp.quad.id  ();

   // vertexes
   FREP(elms)if(C MeshBase *mesh=src[i])if(Int vtxs=mesh->vtxs())
   {
      if(vtx_pos     ){CopyN(vtx_pos     , mesh->vtx.pos     (), vtxs); vtx_pos     +=vtxs;}
      if(vtx_nrm     ){CopyN(vtx_nrm     , mesh->vtx.nrm     (), vtxs); vtx_nrm     +=vtxs;}
      if(vtx_tan     ){CopyN(vtx_tan     , mesh->vtx.tan     (), vtxs); vtx_tan     +=vtxs;}
      if(vtx_bin     ){CopyN(vtx_bin     , mesh->vtx.bin     (), vtxs); vtx_bin     +=vtxs;}
      if(vtx_hlp     ){CopyN(vtx_hlp     , mesh->vtx.hlp     (), vtxs); vtx_hlp     +=vtxs;}
      if(vtx_tex0    ){CopyN(vtx_tex0    , mesh->vtx.tex0    (), vtxs); vtx_tex0    +=vtxs;}
      if(vtx_tex1    ){CopyN(vtx_tex1    , mesh->vtx.tex1    (), vtxs); vtx_tex1    +=vtxs;}
      if(vtx_tex2    ){CopyN(vtx_tex2    , mesh->vtx.tex2    (), vtxs); vtx_tex2    +=vtxs;}
      if(vtx_matrix  ){CopyN(vtx_matrix  , mesh->vtx.matrix  (), vtxs); vtx_matrix  +=vtxs;}
      if(vtx_size    ){CopyN(vtx_size    , mesh->vtx.size    (), vtxs); vtx_size    +=vtxs;}
      if(vtx_flag    ){CopyN(vtx_flag    , mesh->vtx.flag    (), vtxs); vtx_flag    +=vtxs;}
      if(vtx_blend   ){if(mesh->vtx.blend   ())CopyN(vtx_blend   , mesh->vtx.blend   (), vtxs);else REP(vtxs)vtx_blend   [i].set(255, 0, 0, 0); vtx_blend   +=vtxs;}
      if(vtx_material){if(mesh->vtx.material())CopyN(vtx_material, mesh->vtx.material(), vtxs);else REP(vtxs)vtx_material[i].set(255, 0, 0, 0); vtx_material+=vtxs;}
      if(vtx_color   ){if(mesh->vtx.color   ())CopyN(vtx_color   , mesh->vtx.color   (), vtxs);else REP(vtxs)vtx_color   [i]=WHITE            ; vtx_color   +=vtxs;}
   }

   // edges, tris, quads
   if(elms==1)
   {
      if(C MeshBase *mesh=src[0])
      {
         if(edge_ind )CopyN(edge_ind , mesh->edge.ind (), edges);
         if(edge_nrm )CopyN(edge_nrm , mesh->edge.nrm (), edges);
         if(edge_flag)CopyN(edge_flag, mesh->edge.flag(), edges);

         if(tri_ind )CopyN(tri_ind , mesh->tri.ind (), tris);
         if(tri_nrm )CopyN(tri_nrm , mesh->tri.nrm (), tris);
         if(tri_flag)CopyN(tri_flag, mesh->tri.flag(), tris);

         if(quad_ind )CopyN(quad_ind , mesh->quad.ind (), quads);
         if(quad_nrm )CopyN(quad_nrm , mesh->quad.nrm (), quads);
         if(quad_flag)CopyN(quad_flag, mesh->quad.flag(), quads);

         if(set_face_id_from_part_index)
         {
            REP(edges)edge_id[i]=0;
            REP(tris ) tri_id[i]=0;
            REP(quads)quad_id[i]=0;
         }else
         {
            if(edge_id)CopyN(edge_id, mesh->edge.id(), edges);
            if( tri_id)CopyN( tri_id, mesh->tri .id(),  tris);
            if(quad_id)CopyN(quad_id, mesh->quad.id(), quads);
         }
      }
   }else
   {
      Int vtx_ofs=0; FREPD(m, elms)if(C MeshBase *mesh=src[m])
      {
         if(Int edges=mesh->edges())
         {
            if(edge_ind ){C VecI2 *ind=mesh->edge.ind(); if(!ind){ZeroN(edge_ind, edges); edge_ind+=edges;}else REP(edges)*edge_ind++=(*ind++)+vtx_ofs;}
            if(edge_nrm ){CopyN(edge_nrm , mesh->edge.nrm (), edges); edge_nrm +=edges;}
            if(edge_flag){CopyN(edge_flag, mesh->edge.flag(), edges); edge_flag+=edges;}
            if(set_face_id_from_part_index)REP(edges)*edge_id++=m;else if(edge_id){CopyN(edge_id, mesh->edge.id(), edges); edge_id+=edges;}
         }
         if(Int tris=mesh->tris())
         {
            if(tri_ind ){C VecI *ind=mesh->tri.ind(); if(!ind){ZeroN(tri_ind, tris); tri_ind+=tris;}else REP(tris)*tri_ind++=(*ind++)+vtx_ofs;}
            if(tri_nrm ){CopyN(tri_nrm , mesh->tri.nrm (), tris); tri_nrm +=tris;}
            if(tri_flag){CopyN(tri_flag, mesh->tri.flag(), tris); tri_flag+=tris;}
            if(set_face_id_from_part_index)REP(tris)*tri_id++=m;else if(tri_id){CopyN(tri_id, mesh->tri.id(), tris); tri_id+=tris;}
         }
         if(Int quads=mesh->quads())
         {
            if(quad_ind ){C VecI4 *ind=mesh->quad.ind(); if(!ind){ZeroN(quad_ind, quads); quad_ind+=quads;}else REP(quads)*quad_ind++=(*ind++)+vtx_ofs;}
            if(quad_nrm ){CopyN(quad_nrm , mesh->quad.nrm (), quads); quad_nrm +=quads;}
            if(quad_flag){CopyN(quad_flag, mesh->quad.flag(), quads); quad_flag+=quads;}
            if(set_face_id_from_part_index)REP(quads)*quad_id++=m;else if(quad_id){CopyN(quad_id, mesh->quad.id(), quads); quad_id+=quads;}
         }
         vtx_ofs+=mesh->vtxs();
      }
   }
   Swap(temp, T); return T;
}
MeshBase& MeshBase::create(C MeshBase *src, Int elms, UInt flag_and, Bool set_face_id_from_part_index)
{
   if(!src)elms=0;
   Memt<C MeshBase*, 1024> mesh_ptr; mesh_ptr.setNum(elms); REPAO(mesh_ptr)=&src[i];
   return create(mesh_ptr.data(), mesh_ptr.elms(), flag_and, set_face_id_from_part_index);
}
MeshBase& MeshBase::create(C MeshBase &src, UInt flag_and)
{
   if(this==&src)keepOnly(flag_and);else
   {
      create   (src.vtxs(), src.edges(), src.tris(), src.quads(), src.flag()&flag_and);
      copyVtxs (src);
      copyEdges(src);
      copyTris (src);
      copyQuads(src);
   }
   return T;
}
MeshBase& MeshBase::create(C MeshLod &src, UInt flag_and, Bool set_face_id_from_part_index)
{
   Memb<  MeshBase       > temp    ; // use 'Memb' because we're storing pointers to elms
   Memt<C MeshBase*, 1024> mesh_ptr; mesh_ptr.setNum(src.parts.elms());
   REPA(mesh_ptr)
   {
    C MeshPart &part=src.parts[i];
      if(part.render.is() && !part.base.is())mesh_ptr[i]=&temp.New().create(part.render, flag_and); // MeshBase needs to be created
      else                                   mesh_ptr[i]=&                  part.base;              // copy ptr of MeshLod's MeshBase
   }
   return create(mesh_ptr.data(), mesh_ptr.elms(), flag_and, set_face_id_from_part_index);
}
MeshBase& MeshBase::create(C MeshPart &src, UInt flag_and)
{
   if(src.base  .is())return create(src.base  , flag_and);
   if(src.render.is())return create(src.render, flag_and);
                      return del();
}
MeshBase& MeshBase::copyFace(MeshBase &dest, C MemPtr<Bool> &edge_is, C MemPtr<Bool> &tri_is, C MemPtr<Bool> &quad_is, UInt flag_and)C
{
 C Int *p;

   // vtx
   Memt<Bool> vtx_is; vtx_is.setNumZero(vtxs());
   Memt<Int > vtx_remap;
   if(edge_is)FREPA(edge)if(edge_is[i]){p=edge.ind(i).c; REPD(j, 2)vtx_is[p[j]]=true;}
   if( tri_is)FREPA(tri )if( tri_is[i]){p=tri .ind(i).c; REPD(j, 3)vtx_is[p[j]]=true;}
   if(quad_is)FREPA(quad)if(quad_is[i]){p=quad.ind(i).c; REPD(j, 4)vtx_is[p[j]]=true;}

   // create copy
   MeshBase temp(CountIs( vtx_is),
                 CountIs(edge_is),
                 CountIs( tri_is),
                 CountIs(quad_is), flag()&flag_and&(~(VTX_DUP|ADJ_ALL)));
   temp.copyVtxs (T,  vtx_is);
   temp.copyEdges(T, edge_is);
   temp.copyTris (T,  tri_is);
   temp.copyQuads(T, quad_is);
   SetRemap(vtx_remap, vtx_is         ,      vtxs ());
   IndRemap(vtx_remap, temp.edge.ind(), temp.edges());
   IndRemap(vtx_remap, temp.tri .ind(), temp.tris ());
   IndRemap(vtx_remap, temp.quad.ind(), temp.quads());

   Swap(dest, temp); return dest;
}
/******************************************************************************/
static void CopyId(MeshBase &dest, C MeshBase &src, Int id, Memt<Bool> &edge_is, Memt<Bool> &tri_is, Memt<Bool> &quad_is, UInt flag_and)
{
   edge_is.setNum(src.edges());
    tri_is.setNum(src.tris ());
   quad_is.setNum(src.quads());

   // select
 C Int *_id;
  _id=src.edge.id(); if(!_id)ZeroN(edge_is.data(), edge_is.elms());else FREPA(edge_is){edge_is[i]=(*_id==id); _id++;}
  _id=src.tri .id(); if(!_id)ZeroN( tri_is.data(),  tri_is.elms());else FREPA( tri_is){ tri_is[i]=(*_id==id); _id++;}
  _id=src.quad.id(); if(!_id)ZeroN(quad_is.data(), quad_is.elms());else FREPA(quad_is){quad_is[i]=(*_id==id); _id++;}

   // copy
   src.copyFace(dest, edge_is, tri_is, quad_is, flag_and);

   /*// adjust edge flag
   if(Byte  *flag=dest.edge.flag())
   if(VecI2 *e_id=dest.edge.id  ())REPA(dest.edge)
   {
      *flag&=~ETQ_LR;
      if(e_id->x==id)*flag|=ETQ_R;
      if(e_id->y==id)*flag|=ETQ_L;
      flag++; e_id++;
   }*/
}
void MeshBase::copyId(MeshBase &dest, Int id, UInt flag_and)C
{
   Memt<Bool> edge_is, tri_is, quad_is;
   CopyId(dest, T, id, edge_is, tri_is, quad_is, flag_and);
}
void MeshBase::copyId(MeshLod &dest, UInt flag_and)C
{
   Memt<Bool> edge_is, tri_is, quad_is;
   dest.create(maxId()+1); REPA(dest)CopyId(dest.parts[i].base, T, i, edge_is, tri_is, quad_is, flag_and);
}
void MeshBase::copyId(Mesh &dest, UInt flag_and)C
{
   dest.del();
   
   MeshLod &d=dest; copyId(d, flag_and);
   dest.setBox();
}
/******************************************************************************/
}
/******************************************************************************/
