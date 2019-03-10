/******************************************************************************/
void ObjView.meshDelete()
{
   if(mesh_parts.edit_selected())
   {
      mesh_undos.set("delete");
      MeshLod &lod=getLod();
      Memc<Memc<int>> vtxs ; REPA(sel_vtx )vtxs (sel_vtx [i].x).add(sel_vtx [i].y);
      Memc<Memc<int>> faces; REPA(sel_face)faces(sel_face[i].x).add(sel_face[i].y);
      bool changed=false;
      REPA(lod)
      {
         MeshPart  &part=lod.parts[i];
         Memc<int> *v=vtxs .addr(i);
         Memc<int> *f=faces.addr(i);
         if(v && v.elms())part.base.removeVtxs (*v);else
         if(f && f.elms())part.base.removeFaces(*f);else continue;
         changed=true;
         if(part.base.is())part.setRender();else
         {
            lod.parts.remove(i, true);
            mesh_parts.erasedPart(i);
         }
      }
      litSelVFClear();
      if(changed)
      {
         mesh.setBox();
         setChangedMesh(true);
      }
   }else
   {
      MeshParts.Remove();
   }
}
void ObjView.meshSplit()
{
   if(mesh_parts.edit_selected())
   {
      mesh_undos.set("split");
      MeshLod &lod=getLod();
      Memc<Memc<int>> vtxs ; REPA(sel_vtx )vtxs (sel_vtx [i].x).add(sel_vtx [i].y);
      Memc<Memc<int>> faces; REPA(sel_face)faces(sel_face[i].x).add(sel_face[i].y);
      bool changed=false;
      REPA(lod)
      {
         MeshPart  *part=null;
         Memc<int> *v=vtxs .addr(i);
         Memc<int> *f=faces.addr(i);
         if(v && v.elms())part=lod.splitVtxs (i, *v);else
         if(f && f.elms())part=lod.splitFaces(i, *f);
         if(part)
         {
            changed=true;
            part.setRender(); if(InRange(i, lod))lod.parts[i].setRender();
            mesh_parts.addedPart(lod.parts.index(part));
         }
      }
      litSelVFClear();
      if(changed)setChangedMesh(true);
   }
}
void ObjView.meshAlign(bool xz)
{
   if(lit_vtx<0)Gui.msgBox(S, "No highlighted vertex");else
   if(C MeshPart *part=getPart(lit_vf_part))if(InRange(lit_vtx, part.base.vtx))
   {
      Vec pos=part.base.vtx.pos(lit_vtx)*mesh_matrix,
         frac=Frac(pos);
      if(frac.x>0.5)frac.x--;
      if(frac.y>0.5)frac.y--;
      if(frac.z>0.5)frac.z--;
      if(xz)frac.y=0;
      ObjEdit.applyTransform(Matrix().setPos(-frac));
   }
}
void ObjView.meshWeldPos()
{
   if(mesh_parts.edit_selected())
   {
      if(!sel_vtx.elms())Gui.msgBox(S, "No vertexes selected");else
      {
         mesh_undos.set("weldPos");
         Memt<Vec> poss; Vec center=0;
         REPA(sel_vtx)
         {
          C VecI2 &v=sel_vtx[i]; if(C MeshPart *part=getPart(v.x))if(InRange(v.y, part.base.vtx))
            {
             C Vec &pos=part.base.vtx.pos(v.y); REPA(poss)if(Equal(pos, poss[i]))goto has; poss.add(pos); center+=pos; has:;
            }
         }
         if(poss.elms())
         {
            center/=poss.elms();
            REPA(sel_vtx)
            {
             C VecI2 &v=sel_vtx[i]; if(MeshPart *part=getPart(v.x))if(InRange(v.y, part.base.vtx))part.base.vtx.pos(v.y)=center;
            }
            getLod().setRender();
            mesh.setBox();
            setChangedMesh(true);
         }
      }
   }else mesh_parts.weld_vtx.activate();
}
void ObjView.meshWeldPos(flt pos_eps)
{
   bool changed=false;
   pos_eps*=posScale();
   flt remove_degenerate_faces_eps=EPS*posScale();
   mesh_undos.set("weldPos");
   MeshLod &lod=getLod();
   REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
   {
           part.base.weldVtxValues(VTX_POS, pos_eps, EPS_COL_COS, remove_degenerate_faces_eps); // only weld vtx positions, but keep as separate vtx's
      if(1)part.base.weldVtx      (VTX_ALL, pos_eps, EPS_COL_COS, -1); // weld vtxs as one, don't remove degen faces because we've already done it above
      part.setRender();
      changed=true;
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshSetPos()
{
   if(!sel_vtx.elms() || !mesh_parts.edit_selected())Gui.msgBox(S, "No vertexes selected");else
   if(lit_vtx<0)Gui.msgBox(S, "No highlighted vertex");else
   if(C MeshPart *part=getPart(lit_vf_part))if(InRange(lit_vtx, part.base.vtx))
   {
      Vec center=part.base.vtx.pos(lit_vtx);
      mesh_undos.set("setPos");
      REPA(sel_vtx)
      {
       C VecI2 &v=sel_vtx[i]; if(MeshPart *part=getPart(v.x))if(InRange(v.y, part.base.vtx))part.base.vtx.pos(v.y)=center;
      }
      getLod().setRender();
      mesh.setBox();
      setChangedMesh(true);
   }
}
void ObjView.meshReverse()
{
   bool changed=false;
   MeshLod &lod=getLod();
   if(mesh_parts.edit_selected())
   {
      if(!sel_face.elms())Gui.msgBox(S, "No faces selected");else
      {
         mesh_undos.set("reverse");
         Memc<Memc<int>> faces; REPA(sel_face)faces(sel_face[i].x).add(sel_face[i].y);
         litSelVFClear();
         REPAD(p, lod)if(Memc<int> *f=faces.addr(p))if(f.elms())
         {
            MeshPart &part=lod.parts[p]; MeshBase &base=part.base;
            int tris=0, quads=0;
            REPA(*f)
            {
               int face=(*f)[i];
               if( face&SIGN_BIT){face^=SIGN_BIT; if(InRange(face, base.quad))quads++;}
               else              {                if(InRange(face, base.tri )) tris++;}
            }
            base.reverse(*f);
            part.setRender();
            // when reversing selected faces, the mesh must be split, making the selected faces to be located at the end, therefore we need to adjust the selection
            REP(Min( tris, base. tris()))sel_face.binaryInclude(VecI2(p,  base. tris()-1-i          ), Compare);
            REP(Min(quads, base.quads()))sel_face.binaryInclude(VecI2(p, (base.quads()-1-i)^SIGN_BIT), Compare);
            changed=true;
         }
      }
   }else
   {
      mesh_undos.set("reverse");
      REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
      {
         part.base.reverse();
         part.setRender();
         changed=true;
      }
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshReverseN()
{
   bool changed=false;
   MeshLod &lod=getLod();
      mesh_undos.set("reverseN");
   if(mesh_parts.edit_selected())
   {
      Memt<int  > changed_parts;
      Memt<VecI2> vtxs; getSelectedVtxs(vtxs);
      REPA(vtxs)
      {
         C VecI2 &v=vtxs[i]; if(MeshPart *part=getPart(v.x))
         {
            changed_parts.binaryInclude(v.x, Compare);
            MeshBase &base=part.base; if(InRange(v.y, base.vtx))
            {
               if(base.vtx.nrm())base.vtx.nrm(v.y).chs();
            }
         }
      }
      REPA(changed_parts)
      {
         MeshPart &part=lod.parts[changed_parts[i]];
         part.base.setTangents().setBinormals(); part.setRender();
         changed=true;
      }
   }else
   {
      REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
      {
         Chs(part.base.vtx.nrm(), part.base.vtxs());
         part.base.setTangents().setBinormals(); part.setRender();
         changed=true;
      }
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshSetNrm(uint vtx_test)
{
   const bool avg=false;
   bool changed=false;
   MeshLod &lod=getLod();
   flt  pos_eps=vtxDupPosEps();
      mesh_undos.set("setNrm");
   if(mesh_parts.edit_selected())
   {
      Memt<VecI2> vtxs; getSelectedVtxs(vtxs); if(vtxs.elms())
      {
         // set normals on entire 'temp' copy
         MeshLod temp; temp.create(lod);
         if(vtx_test)temp.setVtxDup(vtx_test, pos_eps);else temp.exclude(VTX_DUP);
         temp.setNormals();

         // copy normals on selected vertexes
         REPA(vtxs)
         {
          C VecI2 &v=vtxs[i];
            if(C MeshPart *src =temp.parts.addr(v.x))if(InRange(v.y,  src.base.vtx) && src.base.vtx.nrm())
            if(  MeshPart *dest=  getPart      (v.x))if(InRange(v.y, dest.base.vtx))
            {
               if(!dest.base.vtx.nrm()){dest.base.include(VTX_NRM); CopyN(dest.base.vtx.nrm(), src.base.vtx.nrm(), dest.base.vtxs());}
               dest.base.vtx.nrm(v.y)=src.base.vtx.nrm(v.y);
               changed=true;
            }
         }
         lod.setTangents().setBinormals().setRender().exclude(VTX_DUP);
      }
   }else
   {
      REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
      {
         MeshBase &base=part.base, temp; if(avg)temp.create(base);
         if(vtx_test)base.setVtxDup(vtx_test, pos_eps);else base.exclude(VTX_DUP);
         base.setNormals();
         if(avg)REPA(base.vtx)base.vtx.nrm(i)=!Avg(base.vtx.nrm(i), temp.vtx.nrm(i));
         base.setTangents().setBinormals(); part.setRender();
         changed=true;
      }
      lod.exclude(VTX_DUP);
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshSetNrmH()
{
   bool changed=false;
   MeshLod &lod=getLod();
   flt  pos_eps=vtxDupPosEps();
      mesh_undos.set("setNrm");
   if(mesh_parts.edit_selected())
   {
   }else
   {
      if(C MeshPart *part_lit=getPart(lit_part))
      {
       C MeshBase &lit=part_lit.base; if(lit.vtx.pos() && lit.vtx.nrm())REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
         {
            MeshBase &base=part.base; if(base.vtx.pos() && base.vtx.nrm())
            {
               bool changed_part=false;
               REPA(base.vtx)
               {
                C Vec &pos=base.vtx.pos(i);
                  Vec &nrm=base.vtx.nrm(i);
                  Vec  best_nrm; flt best_dot; bool found_nrm=false;
                  REPA(lit.vtx)
                  {
                   C Vec &lit_pos=lit.vtx.pos(i);
                     if(Equal(pos, lit_pos, pos_eps))
                     {
                      C Vec &lit_nrm=lit.vtx.nrm(i);
                        flt dot=Dot(nrm, lit_nrm);
                        if(!found_nrm || dot>best_dot){found_nrm=true; best_dot=dot; best_nrm=lit_nrm;}
                     }
                  }
                  if(found_nrm)
                  {
                     nrm=best_nrm;
                     changed_part=true;
                  }
               }
               if(changed_part){base.setTangents().setBinormals(); part.setRender(); changed=true;}
            }
         }
      }
   }
   if(changed)setChangedMesh(true, false);
}
void AlignVtxNormal(Vec &nrm, C Vec &dest)
{
   Vec cross=CrossN(nrm, dest);
   flt angle=AbsAngleBetween(nrm, dest);
   nrm*=Matrix3().setRotate(cross, Min(angle, PI/10)); // 10 steps
   nrm.normalize();
}
void ObjView.meshNrmY()
{
   mesh_undos.set("nrmY");
   bool changed=false;
   MeshLod &lod=getLod();
   Vec dest(0, 1, 0); dest.div(mesh_matrix.orn()).normalize();
   if(mesh_parts.edit_selected())
   {
      Memt<VecI2> vtxs; getSelectedVtxs(vtxs); if(vtxs.elms())
      {
         // align normals on selected vertexes
         REPA(vtxs)
         {
          C VecI2 &v=vtxs[i];
            if(MeshPart *part=getPart(v.x))
            if(InRange(v.y, part.base.vtx) && part.base.vtx.nrm())
            {
               AlignVtxNormal(part.base.vtx.nrm(v.y), dest);
               changed=true;
            }
         }
         lod.setTangents().setBinormals().setRender();
      }
   }else
   {
      REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
      {
         MeshBase &base=part.base; if(base.vtx.nrm())
         {
            REPA(base.vtx)AlignVtxNormal(base.vtx.nrm(i), dest);
            base.setTangents().setBinormals(); part.setRender();
            changed=true;
         }
      }
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshCreateFace()
{
   REPA(sel_vtx)if(sel_vtx[i].x!=sel_vtx[0].x){Gui.msgBox(S, "Selected Vertexes must be in the same mesh part"); return;}
   if(!sel_vtx.elms()                         )Gui.msgBox(S, "No Vertexes Selected");else
   if(MeshPart *part=getPart(sel_vtx[0].x))
   {
      MeshBase &base=part.base;
      MemtN<Vec, 4> unique;
      FREPA(sel_vtx) // add in order
      {
       C Vec &pos=base.vtx.pos(sel_vtx[i].y);
         REPA(unique)if(Equal(unique[i], pos))goto has;
         if(unique.elms()>=4)goto invalid_vtx_number;
         unique.add(pos);
      has:;
      }
      if(unique.elms()>=3 && unique.elms()<=4)
      {
         mesh_undos.set("meshCreateFace");
         VecI4 ind;
         bool  added_vtxs=false; Vec face_nrm;
         if(sel_vtx.elms()==unique.elms()) // if all selected vertexes are unique, then we can re-use the vertexes and just add face index
         {
            REPA(sel_vtx)ind.c[i]=sel_vtx[i].y;
         }else
         {
            added_vtxs=true;
            FREPA(unique) // add in order
            {
               int v=base.vtxs();
               ind.c[i]=v;
               base.addVtx(unique[i]);
            }
         }
         if(unique.elms()==3)
         {
            VecI &tri_ind=ind.xyz;
            Tri   tri(unique[0], unique[1], unique[2]);
            REPAO(tri.p)*=mesh_matrix; tri.setNormal();
            if(Edit.Viewport4.View *last=v4.last())if(!frontFace(tri.center(), &tri.n, last.camera.matrix)){tri_ind.reverse(); tri.n.chs();}
            base.addTri(tri_ind);
            if(added_vtxs)face_nrm=tri.n;
         }else
         {
            VecI4 &quad_ind=ind;
            Quad   quad(unique[0], unique[1], unique[2], unique[3]);
            if(Dot(quad.tri013().n, quad.tri123().n)<0)
            {
               Swap(quad_ind.x, quad_ind.y);
               Swap(quad.p[0] , quad.p[1] );
               if(Dot(quad.tri013().n, quad.tri123().n)<0) // if still not OK
               {
                  Swap(quad_ind.x, quad_ind.z);
                  Swap(quad.p[0] , quad.p[2] );
                  if(Dot(quad.tri013().n, quad.tri123().n)<0) // if still not OK
                  {
                     Swap(quad_ind.x, quad_ind.y);
                     Swap(quad.p[0] , quad.p[1] );
                  }
               }
             //quad.setNormal(); not needed since it's called below
            }
            REPAO(quad.p)*=mesh_matrix; quad.setNormal(); // 'setNormal' also needed because of above
            if(Edit.Viewport4.View *last=v4.last())if(!frontFace(quad.center(), &quad.n, last.camera.matrix)){quad_ind.reverse(); quad.n.chs();}
            base.addQuad(quad_ind);
            if(added_vtxs)face_nrm=quad.n;
         }
         if(added_vtxs)
         {
            face_nrm.div(mesh_matrix.orn()).normalize();
            Matrix3 tex_matrix; tex_matrix.setDir(face_nrm);
            REPA(unique)
            {
               int v=ind.c[i];
               if(base.vtx.nrm     ())base.vtx.nrm     (v)=face_nrm;
               if(base.vtx.color   ())base.vtx.color   (v)=WHITE;
               if(base.vtx.material())base.vtx.material(v).set(255, 0, 0, 0);
               if(base.vtx.blend   ())base.vtx.blend   (v).set(255, 0, 0, 0);
               if(base.vtx.tex0    ()){C Vec &pos=base.vtx.pos(v); base.vtx.tex0(v).set(Dot(pos, tex_matrix.x), Dot(pos, tex_matrix.y));}
            }
            if(base.vtx.tan())base.setTangents ();
            if(base.vtx.bin())base.setBinormals();
            base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, do not remove degenerate faces because they're not needed because we're only adding new faces
         }
         part.setRender();
         setChangedMesh(true, false);
         litSelVFClear();
      }else
      {
      invalid_vtx_number:
         Gui.msgBox(S, "Face can be created only from 3 or 4 Vertexes");
      }
   }
}
void ObjView.meshMergeFaces()
{
   if(!sel_face.elms()  )Gui.msgBox(S, "No Faces Selected");else
   if( sel_face.elms()>2)Gui.msgBox(S, "Too many Faces Selected");else
   {
      Memt<VecI2> faces; faces=sel_face;
      if(faces.elms()==1 && lit_vf_part>=0 && lit_face!=-1)faces.New().set(lit_vf_part, lit_face);
      if(faces.elms()!=2)Gui.msgBox(S, "2 Faces must be Selected");else
      if(faces[0].x!=faces[1].x)Gui.msgBox(S, "Selected faces are of different mesh parts");else
      if(MeshPart *part=getPart(faces[0].x))
      {
         mesh_undos.set("meshMergeFace");
         part.base.mergeFaces(faces[0].y, faces[1].y);
         part.setRender();
         setChangedMesh(true, false);
         litSelVFClear();
      }else Gui.msgBox(S, "Invalid Mesh Part");
   }
}
void ObjView.meshRotQuads()
{
   bool changed=false;
   mesh_undos.set("meshRotQuad");
   REPA(sel_face)
   {
    C VecI2 &sf=sel_face[i]; if(sf.y&SIGN_BIT) // only quads
      {
         int q=sf.y^SIGN_BIT;
         if(MeshPart *part=getPart(sf.x))if(InRange(q, part.base.quad))
         {
            part.base.quad.ind(q).rotateOrder();
            changed=true;
         }
      }
   }
   if(changed)
   {
      mesh.setRender();
      setChangedMesh(true, false);
   }
}
void ObjView.meshQuadToTri()
{
   bool changed=false;
   mesh_undos.set("meshQuadToTri");
   MeshLod &lod=getLod();
   Mems<int> part_tris; part_tris.setNum(lod.parts.elms()); REPAO(part_tris)=lod.parts[i].tris(); // number of tris in each part before conversion
   if(mesh_parts.edit_selected())
   {
      Memc<Memc<int>> part_quads;
      REPA(sel_face)
      {
       C VecI2 &sf=sel_face[i]; if(sf.y&SIGN_BIT) // only quads
         {
            int q=sf.y^SIGN_BIT;
            if(MeshPart *part=getPart(sf.x))if(InRange(q, part.base.quad))part_quads(sf.x).add(q);
         }
      }
      if(part_quads.elms())
      {
         REPA(part_quads)if(MeshPart *part=getPart(i)){part.base.quadToTri(part_quads[i]); part.setRender();}
         changed=true;
      }
   }else
   {
      REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i)){part.base.quadToTri(); part.setRender(); changed=true;}
   }
   if(changed)
   {
       REPA(sel_face)if(sel_face[i].y&SIGN_BIT)sel_face.remove(i, true); // remove all quads from selection
      FREPA(part_tris)for(int j=part_tris[i], end=lod.parts[i].tris(); j<end; j++)sel_face.binaryInclude(VecI2(i, j), Compare); // add all tris to selection that were created in this operation
      setChangedMesh(true, false);
   }
}
void ObjView.meshSubdivide()
{
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("meshSub");
   REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base;
      base.setVtxDup();
      base.subdivide();
      base.exclude(VTX_DUP);
      part.setRender();
      changed=true;
   }
   if(changed)
   {
      mesh.setBox();
      setChangedMesh(true);
   }
}
void ObjView.meshTesselate()
{
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("meshTess");
   REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base;
    //base.setVtxDup();
      base.tesselate();
    //base.exclude(VTX_DUP);
      part.setRender();
      changed=true;
   }
   if(changed)
   {
      mesh.setBox();
      setChangedMesh(true);
   }
}
void ObjView.meshColorBrght()
{
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("vtxCol");
   if(mesh_parts.edit_selected())
   {
      if(!sel_vtx.elms())Gui.msgBox(S, "No vertexes selected");else
      {
         REPA(sel_vtx)
         {
          C VecI2 &v=sel_vtx[i]; if(MeshPart *part=getPart(v.x))if(InRange(v.y, part.base.vtx) && part.base.vtx.color())
            {
               Color &c=part.base.vtx.color(v.y);
               c=Lerp(c, WHITE, 0.1);
               changed=true;
            }
         }
         if(changed)lod.setRender();
      }
   }else
   REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base; if(base.vtx.color())
      {
         REPA(base.vtx)base.vtx.color(i)=Lerp(base.vtx.color(i), WHITE, 0.1);
         part.setRender();
         changed=true;
      }
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshDelDblSide()
{
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("delDblSide");
   litSelVFClear();
   flt pos_eps=vtxDupPosEps();
   REPA(lod)if(partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base, temp; temp.create(base).triToQuad();
      int faces =temp.faces(); temp.setVtxDup(0, pos_eps).removeDoubleSideFaces().exclude(VTX_DUP);
      if( faces!=temp.faces())
      {
         Swap(base, temp);
         part.setRender();
         changed=true;
      }
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshSeparate1()
{
   mesh_undos.set("sep1");
   Proj.clearListSel();
   MeshLod &lod=getLod();
   Str      name;
   bool     all_lods=true;
   Memc<int> parts; FREPA(lod)if(partOp(i)){parts.add(i); MeshPart &part=lod.parts[i]; if(!name.is())name=part.name; all_lods&=SamePartInAllLods(mesh, i);} // add in order
   if(parts.elms() && parts.elms()!=lod.parts.elms())
   {
      if(obj_elm)
      {
         obj_elm.opened(true);
         if(!name.is())name=obj_elm.name;
      }
      Elm        & obj_elm =Proj.Project.newElm(name, obj_id, ELM_OBJ); Proj.list_sel.add(obj_elm.id);
      EditObject   obj_edit; obj_edit.newData(); Save(obj_edit, Proj.editPath(obj_elm.id));
      if(ElmObj  * obj_data=obj_elm.objData()){obj_data.newData(); obj_data.from(obj_edit);}
      if(Elm     *mesh_elm =Proj.getObjMeshElm(obj_elm.id, false, false)) // don't send to server yet, it will be sent below
      if(ElmMesh *mesh_data=mesh_elm.meshData())
      {
         if(T.skel_elm && mesh_skel)
         if(Elm     *skel_elm =Proj.getObjSkelElm(obj_elm.id, false, false)) // don't send to server yet, it will be sent below
         if(ElmSkel *skel_data=skel_elm.skelData())
         {
            if(ElmSkel *src_skel_data=T.skel_elm.skelData())skel_data.transform=src_skel_data.transform;
            Save( edit_skel, Proj.editPath(skel_elm.id));
            Save(*mesh_skel, Proj.gamePath(skel_elm.id)); Proj.savedGame(*skel_elm);
            Server.setElmFull(skel_elm.id);
         }
         mesh_data.transform=mesh_matrix;
         if(T.mesh_elm)if(ElmMesh *src_mesh_data=T.mesh_elm.meshData())
         {
            mesh_data.      body_id=T.mesh_elm.id; //src_mesh_data.body_id;
            mesh_data.draw_group_id=src_mesh_data.draw_group_id;
         }
         Mesh mesh_edit; mesh_edit.copyParams(mesh);
         mesh_edit.setLods(all_lods ? T.mesh.lods() : 1); REPD(l, mesh_edit.lods())
         {
            MeshLod &src=(all_lods ? T.mesh.lod(l) : lod), &dest=mesh_edit.lod(l); dest.copyParams(src);
            dest.parts.setNum(parts.elms()); REPAD(p, dest)
            {
               dest.parts[p].create(src.parts[parts[p]]);
               src.parts.remove(parts[p], true); mesh_parts.erasedPart(parts[p]);
            }
         }
         mesh_edit.delRender().keepOnly(EditMeshFlagAnd).setBox();
         T.mesh.setBox();
         Save(mesh_edit, Proj.editPath(mesh_elm.id), Proj.game_path);
         Proj.makeGameVer(*mesh_elm); Server.setElmFull(mesh_elm.id);
      }
      Proj.makeGameVer(obj_elm); Server.setElmFull(obj_elm.id);
      Proj.setList(); // call before 'setChangedMesh' because we need hierarchy
      setChangedMesh(true);
   }
}
void ObjView.meshSeparateN()
{
   mesh_undos.set("sepN");
   if(obj_elm)
   {
      Proj.clearListSel();
      obj_elm.opened(true);
      MeshLod  &lod=getLod();
      bool      all_lods=true;
      Memc<int> parts; FREPA(lod)if(partOp(i)){parts.add(i); all_lods&=SamePartInAllLods(mesh, i);} bool remove=(parts.elms()!=lod.parts.elms()); // don't remove if there will be no parts left
      REPAD(p, parts) // process in remove order
      {
       C MeshPart   &     part=lod.parts[parts[p]];
         Elm        & obj_elm =Proj.Project.newElm(Is(part.name) ? Str(part.name) : T.obj_elm.name, obj_id, ELM_OBJ); Proj.list_sel.add(obj_elm.id);
         EditObject   obj_edit; obj_edit.newData(); Save(obj_edit, Proj.editPath(obj_elm.id));
         if(ElmObj  * obj_data=obj_elm.objData()){obj_data.newData(); obj_data.from(obj_edit);}
         if(Elm     *mesh_elm =Proj.getObjMeshElm(obj_elm.id, false, false)) // don't send to server yet, it will be sent below
         if(ElmMesh *mesh_data=mesh_elm.meshData())
         {
            if(T.skel_elm && mesh_skel)
            if(Elm     *skel_elm =Proj.getObjSkelElm(obj_elm.id, false, false)) // don't send to server yet, it will be sent below
            if(ElmSkel *skel_data=skel_elm.skelData())
            {
               if(ElmSkel *src_skel_data=T.skel_elm.skelData())skel_data.transform=src_skel_data.transform;
               Save( edit_skel, Proj.editPath(skel_elm.id));
               Save(*mesh_skel, Proj.gamePath(skel_elm.id)); Proj.savedGame(*skel_elm);
               Server.setElmFull(skel_elm.id);
            }
            mesh_data.transform=mesh_matrix;
            if(T.mesh_elm)if(ElmMesh *src_mesh_data=T.mesh_elm.meshData())
            {
               mesh_data.      body_id=T.mesh_elm.id; //src_mesh_data.body_id;
               mesh_data.draw_group_id=src_mesh_data.draw_group_id;
            }
            Mesh mesh_edit; mesh_edit.copyParams(mesh);
            mesh_edit.setLods(all_lods ? T.mesh.lods() : 1); REPD(l, mesh_edit.lods())
            {
               MeshLod &src=(all_lods ? T.mesh.lod(l) : lod), &dest=mesh_edit.lod(l); dest.copyParams(src);
               dest.parts.New().create(src.parts[parts[p]]);
               if(remove){src.parts.remove(parts[p], true); mesh_parts.erasedPart(parts[p]);}
            }
            if(remove)T.mesh.setBox();
            mesh_edit.delRender().keepOnly(EditMeshFlagAnd).setBox();
            Save(mesh_edit, Proj.editPath(mesh_elm.id), Proj.game_path);
            Proj.makeGameVer(*mesh_elm); Server.setElmFull(mesh_elm.id);
         }
         Proj.makeGameVer(obj_elm); Server.setElmFull(obj_elm.id);
      }
      Proj.list_sel.reverseOrder(); // reverse order because we've processed list from the back, but it's better to keep selected elements in normal order
      Proj.setList(); // call before 'setChangedMesh' because we need hierarchy
      if(remove)setChangedMesh(true);
   }
}
void ObjView.meshCopyParts()
{
   mesh_undos.set("copyParts");
   if(Elm *dest_mesh_elm=getMeshElm())if(ElmMesh *dest_mesh_data=dest_mesh_elm.meshData())
   {
      bool changed=false;
      FREPA(menu_ids)
         if(Elm *src_obj_elm =Proj.findElm(         menu_ids[i]))if(ElmObj  *src_obj_data =src_obj_elm . objData())
         if(Elm *src_mesh_elm=Proj.findElm(src_obj_data.mesh_id))if(ElmMesh *src_mesh_data=src_mesh_elm.meshData())
      {
         Mesh src_mesh; if(Load(src_mesh, Proj.editPath(src_mesh_elm.id), Proj.game_path))
         {
            int old_parts=getLod().parts.elms();
            src_mesh.drawGroupEnum(null);
            src_mesh.transform(src_mesh_data.transform() * ~dest_mesh_data.transform());
            if(mesh_skel && mesh_skel.bones.elms())src_mesh.skeleton(mesh_skel); // adjust to the new skeleton
            else                                   src_mesh.exclude(VTX_SKIN).clearSkeleton(); // remove completely
            if(mode()==LOD && (mesh.lods()>1 || mesh.parts.elms())) // if in LOD mode, and mesh has anything (lods or parts), then add to current LOD only
            {
               mesh.variationInclude(src_mesh);
               getLod().add(src_mesh, &src_mesh, &mesh);
            }else  mesh.add(src_mesh); // otherwise copy to entire mesh
            mesh.setTangents().setBinormals().setRender().setBox();
            Memc<int> sel_parts; REP(getLod().parts.elms()-old_parts)sel_parts.add(old_parts+i);
            mesh_parts.selParts(sel_parts);
            changed=true;
         }
      }
      if(changed)setChangedMesh(true);
   }
}
void ObjView.meshSkinFull()
{
   const byte bone=sel_bone+1;
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("skinFull");
   REPA(lod)if(mesh_parts.partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base;
      base.include(VTX_SKIN); // for "!bone" case we could "exclude(VTX_SKIN)" however in order to reduce draw calls we keep consistency for all mesh parts (to have the same draw call settings)
      REPA(base.vtx)
      {
         base.vtx.matrix(i).set(bone, 0, 0, 0);
         base.vtx.blend (i).set( 255, 0, 0, 0); // !! sum must be equal to 255 !!
      }
      part.setRender();
      changed=true;
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshSkinFullP()
{
   const byte bone=sel_bone+1, bone_parent=(mesh_skel ? mesh_skel.boneParent(sel_bone) : -1)+1;
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("skinFull");
   REPA(lod)if(mesh_parts.partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base;
      base.include(VTX_SKIN); // for "!bone" case we could "exclude(VTX_SKIN)" however in order to reduce draw calls we keep consistency for all mesh parts (to have the same draw call settings)
      REPA(base.vtx)
      {
         base.vtx.matrix(i).set(bone, bone_parent, 0, 0);
         base.vtx.blend (i).set( 128,         127, 0, 0); // !! sum must be equal to 255 !!
      }
      part.setRender();
      changed=true;
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshSkinFullU()
{
   const byte bone=sel_bone+1;
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("skinFullU");
   REPA(lod)if(mesh_parts.partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base;
      bool part_changed=false;
      if(base.vtx.matrix() && base.vtx.blend())REPA(base.vtx)
      {
         VecB4 &matrix=base.vtx.matrix(i);
         VecB4 &blend =base.vtx.blend (i);
         REPA(matrix)if(matrix.c[i]==bone && blend.c[i]>0)
         {
            IndexWeight skin[]=
            {
               IndexWeight(matrix.x, blend.x),
               IndexWeight(matrix.y, blend.y),
               IndexWeight(matrix.z, blend.z),
               IndexWeight(matrix.w, blend.w),
            };
            skin[i].weight=0; // unassign from bone
            SetSkin(skin, matrix, blend, mesh_skel);
            part_changed=true;
            break;
         }
      }
      if(part_changed){part.setRender(); changed=true;}
   }
   if(changed)setChangedMesh(true, false);
}
void ObjView.meshSkinAuto()
{
   const byte bone=sel_bone+1;
   bool changed=false;
   MeshLod &lod=getLod();
   mesh_undos.set("skinAuto");
   if(mesh_skel)REPA(lod)if(mesh_parts.partOp(i))if(MeshPart *part=getPart(i))
   {
      MeshBase &base=part.base;
      bool part_changed=false;
      if(base.vtx.pos())REPA(base.vtx)
      {
         VecB4 matrix;
         VecB4 blend ;
         mesh_skel.getSkin(base.vtx.pos(i)*mesh_matrix, blend, matrix);
         if(sel_bone<0 || matrix.x==bone || matrix.y==bone || matrix.z==bone || matrix.w==bone)
         {
            if(!base.vtx.matrix()){base.include(VTX_MATRIX); REPA(base.vtx)base.vtx.matrix(i).zero();}
            if(!base.vtx.blend ()){base.include(VTX_BLEND ); REPA(base.vtx)base.vtx.blend (i).set (255, 0, 0, 0);}
            base.vtx.matrix(i)=matrix;
            base.vtx.blend (i)=blend ;
            part_changed=true;
         }
      }
      if(part_changed){part.setRender(); changed=true;}
   }
   if(changed)setChangedMesh(true, false);
}
/******************************************************************************/
