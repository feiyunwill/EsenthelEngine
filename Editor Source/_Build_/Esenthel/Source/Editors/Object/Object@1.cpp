/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
/******************************************************************************

   Meshes are stored in:
      "Edit" (untransformed, skeleton not set but has BoneMap, parts not merged                          , only MeshBase   available)
      "Game" (  transformed, skeleton     set with    BoneMap, parts     merged by material and DrawGroup, only MeshRender available)

   Upon opening a mesh in Object Editor, "Edit" version is loaded (and MeshRender created)
      every usage of the mesh must be combined with transforming by its current "Game" matrix.

   Upon saving a mesh, it's saved as both "Edit" and "Game" versions.

/******************************************************************************/
ObjView ObjEdit;
/******************************************************************************/

/******************************************************************************/
   cchar8 *ObjView::mode_t[]=
   {
      "Transform",
      "Params",
      "LOD",
      "Mesh",
      "Variations",
      "Slots",
      "Bones",
      "Skin",
      "Physics",
      "Groups",
      "Remove",
      "Ragdoll",
      "Body",
      "Background",
   };
   cchar8 *ObjView::skin_mode_t[]=
   {
      "Select Mesh Part",
      "Select Bone",
      "Change Skin",
   };
   cchar8 *ObjView::skin_mode_desc[]=
   {
      "Select which mesh parts should be editable\nKeyboard Shortcut: Shift+F1",
      "Select target bone\nKeyboard Shortcut: Shift+F2",
      "Apply skin changes\nUse left/right mouse button on mesh vertexes to decrease/increase blend weight for currently selected bone.\nOptionally hold Alt to make instant changes.\n\nKeyboard Shortcut: Shift+F3",
   };
   cchar8 *ObjView::slot_desc[]=
   {
      "+1",
      "-1",
      "  ", // move
      "  ", // rot
      "  ", // scale
      "Rename",
      "Set Parent",
   };
   cchar8 *ObjView::del_root_bone_dialog_id="delRootBone";
   cchar8 *ObjView::bone_desc[]=
   {
      "+1",
      "-1",
      "  ", // move
      "  ", // rot
      "  ", // scale
      "Rename",
      "Set Parent",
   };
   cchar8 *ObjView::bone_move_desc[]=
   {
      "Start",
      "End",
   };
   cchar8 *ObjView::bone_child_desc[]=
   {
      "Don't transform children",
      "Transform nearest children",
      "Transform all children",
   };
   cchar8 *ObjView::bone_child_rot_desc[]=
   {
      "Don't transform children",
      "Transform all children",
   };
   cchar8 *ObjView::phys_desc[]=
   {
      "-1",
      "  ",     // move
      "  ",     // rot
      "  ",     // scale
      "Toggle Parts", // toggle
   };
   cchar8 *ObjView::ragdoll_desc[]=
   {
      "Toggle Bone",
      "  ", // scale
      "  ", // move
   };
/******************************************************************************/
      void ObjView::BackMesh::draw()
      {
         if(mesh)mesh->draw(matrix);
      }
      void ObjView::BackMesh::drawBlend()
      {
         if(mesh)mesh->drawBlend(matrix, VecZero, VecZero, &Vec4(1, 1, 1, ObjEdit.background_alpha()));
      }
      void ObjView::SlotMesh::set(C MeshPtr &mesh, C Str &name) {T.mesh=mesh; T.name=name;}
      void ObjView::SlotMesh::draw(Skeleton &skel)
      {
         if(mesh)if(SkelSlot *slot=skel.findSlot(name))
         {
            Matrix m=*slot; m.scaleOrn(scale); mesh->draw(m);
         }
      }
      void ObjView::Info::draw(C GuiPC &gpc){ObjEdit.drawInfo(gpc);}
      void ObjView::MeshChange::removeBone(C Str &name)
      {
         bone_removals.add(name); // add to removal list
         if(C EditSkeleton::Bone *bone_remove=ObjEdit.cur_skel_to_saved_skel.findBone(name)) // add to restore list
         {
            BoneWeight &bone_restore=bone_restores.New();
            bone_restore.name=bone_remove->name;
            bone_restore.setNum(bone_remove->elms());
            REPA(bone_restore)
            {
             C IndexWeight &src =(*bone_remove)[i];
               NameWeight  &dest=  bone_restore[i];
               if(InRange(src.index, ObjEdit.cur_skel_to_saved_skel.nodes))dest.name=ObjEdit.cur_skel_to_saved_skel.nodes[src.index].name;
               dest.weight=src.weight;
            }
         }
      }
      void ObjView::MeshChange::selOnly()
      {
                 data.del();
                 skel.del();
            edit_skel.del(); cur_skel_to_saved_skel.del(); can_use_cur_skel_to_saved_skel=false;
       //bone_renames.del(); bone_removals.del(); bone_restores.del(); these are added manually and not in 'create'
         sel_only=true;
      }
      uint ObjView::MeshChange::memUsage()C {return data.memUsage()+skel.memUsage()+edit_skel.memUsage()+sel_vtx.memUsage()+sel_face.memUsage();}
      void ObjView::MeshChange::create(ptr user)
{
         if(ObjEdit.mesh_elm ){data.create(ObjEdit.mesh); if(ElmMesh *mesh_data=ObjEdit.mesh_elm->meshData())T.mesh_data=*mesh_data;}
         if(ObjEdit.mesh_skel)
         {
cur_skel_to_saved_skel= ObjEdit.cur_skel_to_saved_skel;
             edit_skel= ObjEdit.             edit_skel;
                  skel=*ObjEdit.             mesh_skel;
         }
         mesh_matrix=ObjEdit.mesh_matrix;
         sel_vtx =ObjEdit.sel_vtx;
         sel_face=ObjEdit.sel_face;
         ObjEdit.undoVis();
      }
      void ObjView::MeshChange::apply(bool undo)
      {
         // renames
         if(undo)REPA(bone_renames)ObjEdit.cur_skel_to_saved_skel.renameBone(bone_renames[i].dest, bone_renames[i].src ); // rename back in reversed order
         else   FREPA(bone_renames)ObjEdit.cur_skel_to_saved_skel.renameBone(bone_renames[i].src , bone_renames[i].dest); // rename      in normal   order

         // removals
         if(undo)REPA(bone_restores) // restore in reversed order
         {
          C BoneWeight &bone_restore=bone_restores[i]; if(EditSkeleton::Bone *bone=ObjEdit.cur_skel_to_saved_skel.getBone(bone_restore.name))FREPA(bone_restore)
            {
             C NameWeight &src=bone_restore[i]; bone->addWeight(ObjEdit.cur_skel_to_saved_skel.findNodeI(src.name), src.weight);
            }
         }else FREPA(bone_removals)ObjEdit.cur_skel_to_saved_skel.removeBone(bone_removals[i]); // remove in normal order

         // bone mapping changes through adjust bone orientations FIXME
      }
      void ObjView::MeshChange::apply(ptr user)
{
         if(ObjEdit.mesh_elm)
         {
            ObjEdit.sel_vtx =sel_vtx;
            ObjEdit.sel_face=sel_face;
            if(!sel_only)
            {
               {
                  CacheLock cl(Meshes);
                  ObjEdit.mesh.create(data).setShader();
                  ObjEdit.setChangedMesh(true);
                  ObjEdit.trans_mesh.resetDo();
               }
               if(ObjEdit.mesh_elm)if(ElmMesh *mesh_data=ObjEdit.mesh_elm->meshData())
               {
                  mesh_data->undo(T.mesh_data); // remember that transform is not undone in this method
                  Proj.getMeshSkels(mesh_data, null, &ObjEdit.body_skel);
               }
                  ObjEdit.edit_skel=edit_skel;
               if(ObjEdit.mesh_skel)
               {
                  CacheLock cl(Skeletons);
                 *ObjEdit.mesh_skel=skel;
                  ObjEdit.mesh_skel->transform(GetTransform(mesh_matrix, ObjEdit.mesh_matrix));
                  ObjEdit.setChangedSkel(true);
                //if(ElmSkel *skel_data=ObjEdit.skel_elm.skelData())skel_data.undo(T.skel_data);
               }

               if(can_use_cur_skel_to_saved_skel)ObjEdit.cur_skel_to_saved_skel=cur_skel_to_saved_skel;else // use if we can
               if(ObjEdit.mesh_undos_undo                                                     )       apply(true );else // if we can't use, then we need to manually perform changes
               if(MeshChange *change=ObjEdit.mesh_undos.addr(ObjEdit.mesh_undos.index(this)-1))change->apply(false);     // we have to redo the previous change, and not this one
               ObjEdit.lod.toGui();
            }
            ObjEdit.undoVis();
         }
      }
      void ObjView::PhysChange::create(ptr user)
{
         if(ObjEdit.phys)
         {
            data=*ObjEdit.phys;
            if(ObjEdit.phys_elm)if(ElmPhys *phys_data=ObjEdit.phys_elm->physData())T.phys_data=*phys_data;
         }
         ObjEdit.undoVis();
      }
      void ObjView::PhysChange::apply(ptr user)
{
         if(ObjEdit.phys && ObjEdit.phys_elm)if(ElmPhys *phys_data=ObjEdit.phys_elm->physData())
         {
            CacheLock cl(PhysBodies);
           *ObjEdit.phys=data;
            ObjEdit.phys->transform(GetTransform(T.phys_data.transform(), ObjEdit.mesh_matrix)); // transform from undo matrix to current matrix
            phys_data->undo(T.phys_data);
            phys_data->transform=ObjEdit.mesh_matrix;
            phys_data->from(*ObjEdit.phys);
            ObjEdit.setPhysPartMatrix();
            ObjEdit.setChangedPhys();
            ObjEdit.toGuiPhys();
            ObjEdit.undoVis();
         }
      }
      ::ObjView::SkinBrush& ObjView::SkinBrush::create(GuiObj &parent, C Vec2 &rd)
      {
         ::BrushClass::create(parent, rd);
         alt_rotate=false;
         ssize.set(0.3f);
         return T;
      }
      bool ObjView::SkinBrush::hasMsWheelFocus()C {return ObjEdit.v4.getView(Gui.ms())!=null;}
   void ObjView::undoVis() {SetUndo(mesh_undos, mesh_undo, mesh_redo); SetUndo(phys_undos, phys_undo, phys_redo);}
   bool ObjView::selected()C {return Mode()==MODE_OBJ;}
   bool ObjView::lodEditDist()C {return mode()==LOD && lod.edit_dist.visible() && lod.edit_dist()==0;}
   bool ObjView::lodDrawAtDist()C {return mode()==LOD && NewLod.visible() && NewLod.draw_at_distance;}
   bool ObjView::customDrawMatrix()C {return lodEditDist() || lodDrawAtDist();}
   bool ObjView::partVisible(int p, C MeshPart &part, bool allow_lit)C
   {
      if(mode()==REMOVE || (mode()==PHYS && phys_tabs()==PHYS_TOGGLE))return true;
      if(mode()==MESH || mode()==SKIN)
      {
         if(allow_lit && p==lit_part)return true;
         if(mesh_parts.edit_selected() && !mesh_parts.partOp(p))return false;
      }
      return !FlagTest(part.part_flag, MSHP_HIDDEN);
   }
   bool ObjView::partOp(int p)C {return                                   !mesh_parts.edit_selected() && mesh_parts.partOp (p);}
   bool ObjView::partSel(int p)C {return (mode()==MESH || mode()==SKIN) && !mesh_parts.edit_selected() && mesh_parts.partSel(p);}
   bool ObjView::frontFace(C Vec &pos, C Vec *nrm, C Matrix &cam)C
   {
      if(!nrm)return true;
      return v4.perspective() ? DistPointPlane(cam.pos, pos, *nrm)>=0
                              : Dot           (cam.z  ,      *nrm)<=0;
   }
   bool ObjView::editMeshParts()C {return mode()==MESH && mesh_parts.edit_selected();}
   bool ObjView::transMesh()C {return mode()==MESH && trans_mesh.visible();}
   bool ObjView::showBoxes()C {return box() && !customDrawMatrix();}
   bool ObjView::showMainBox()C {return showBoxes() && !editMeshParts();}
   Matrix ObjView::transformMatrix()C {return mesh_matrix*trans.matrix;}
   Matrix ObjView::transformMatrix(bool sel)C {Matrix m=transformMatrix(); if(sel)m*=trans_mesh.matrix; return m;}
   Circle   ObjView::vtxSelCircle()C {return Circle(vtx_sel_r, Ms.pos());}
   bool ObjView::showVtxSelCircle()
   {
      return editMeshParts() && !transMesh() && (lit_vtx>=0 || sel_vtx.elms()) && (mesh_parts.list.selMode()==LSM_INCLUDE || mesh_parts.list.selMode()==LSM_EXCLUDE) && vtx_face_sel_mode()==0 && v4.getView(Gui.ms())!=null;
   }
   bool ObjView::showChangeSkin()C {return mode()==SKIN && skin_tabs()==SKIN_CHANGE_SKIN;}
   bool ObjView::showChangeSkinShape()  {return showChangeSkin() && v4.getView(Gui.ms())!=null;}
   Shape ObjView::changeSkinShape()C
   {
      flt r=Lerp(0.01f, D.h()*0.5f, Sqr(skin_brush.ssize()));
      switch(skin_brush.shape())
      {
         case BS_SQUARE: return Rect_C(Ms.pos(), r);
         case BS_CIRCLE: return Circle(r, Ms.pos());
         default       : return Shape();
      }
   }
   int ObjView::boneTabs()C
   {
      if(Kb.alt())return BONE_ROT;
      int v=bone_tabs();
      if(v<0)v=BONE_MOVE;
      return v;
   }
   int ObjView::boneAxis()C
   {
      if(Kb.alt())return 2;
      return bone_axis;
   }
   flt ObjView::posScale()C
   {
      if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())return mesh_data->posScale(); // because we're operating on meshes in original import matrix, we need to adjust by the transform scale
      return 1;
   }
   flt ObjView::vtxDupPosEps()C {return VtxDupPosEps*posScale();}
   Vec ObjView::selMeshCenter()C
   {
      bool is =false;
      Box  box=mesh.ext;
      if(mode()==MESH)REPA(mesh_parts.list.sel)if(C MeshPart *part=getPart(mesh_parts.list.sel[i]))Include(box, is, part->base);
      return box.center()*transformMatrix(true);
   }
   Skeleton* ObjView::getVisSkel()
   {
      return adjust_bone_orns.visibleFull() ? &adjust_bone_orns.getSkel() : mesh_skel;
   }
   void ObjView::setMatrixAtDist(Matrix &matrix, flt dist)
   {
      dist/=Tan(D.viewFov()/2); // we're actually interested in the visible size, so make it dependent on current FOV
      matrix=mesh_matrix; matrix.move(ActiveCam.matrix.pos + ActiveCam.matrix.z*dist - mesh.lod_center*mesh_matrix);
   }
   void ObjView::Render() {ObjEdit.render();}
          void ObjView::render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            bool interval=(Trunc(Time.appTime()*2.5f)&1);
            if(lodEditDist()) // LOD compare models to edit distance
            {
               int lod=selLod();
               if(InRange(lod-1, mesh.lods()) && InRange(lod, mesh.lods()))
               {
                  MeshLod &a=mesh.lod(lod-1),
                          &b=mesh.lod(lod  ), &draw_lod=(interval ? a : b);
                  Matrix matrix; setMatrixAtDist(matrix, absLodDist(b));
                  FREPA(draw_lod)if(!(draw_lod.parts[i].part_flag&MSHP_HIDDEN))draw_lod.parts[i].draw(matrix);
               }
            }else // default
            if(!showChangeSkin())
            {
               SetVariation(visibleVariation());
               if(group.groups_l.lit>=0)SetDrawMask(IndexToFlag(group.groups_l.lit));
               bool     allow_lit=!(Kb.win() || Kb.b(KB_SPACE) || Ms.b(2) || Ms.b(3) || Ms.b(4));
               MeshLod &lod=getDrawLod();
               bool     custom_matrix=customDrawMatrix();
               Matrix   matrix; if(custom_matrix)setMatrixAtDist(matrix, NewLod.draw_distance);
               FREPA(lod)
               {
                  MeshPart &part=lod.parts[i]; if(partVisible(i, part))
                  {
                     bool invalid=false; if(interval)
                     {
                        bool optional=(obj_elm && obj_elm->finalNoPublish()); // if the object itself is not published, then set reference as optional
                        REP(4                )if(Proj.invalidRef(part.multiMaterial(i).id(), optional)){invalid=true; break;}
                        REP(part.variations())if(Proj.invalidRef(part.variation    (i).id(), optional)){invalid=true; break;}
                     }
                     bool lit=(allow_lit && i==lit_part), sel=partSel(i);
                     if(mode()==PHYS && phys_tabs()==PHYS_TOGGLE)SetHighlight(Color((FlagTest(part.part_flag, MSHP_NO_PHYS_BODY) && interval) ? 85 : 0, lit ? 85 : 0, lit ? 85 : 0, 0));else
                     if(mode()==REMOVE                          )SetHighlight(Color((FlagTest(part.part_flag, MSHP_HIDDEN      ) && interval) ? 85 : 0, lit ? 85 : 0, lit ? 85 : 0, 0));else
                     if(invalid   )SetHighlight(Color(85,  0,  0, 0));else
                     if(sel && lit)SetHighlight(Color(40,  0,  0, 0));else
                     if(sel       )SetHighlight(Color(40, 40,  0, 0));else
                     if(       lit)SetHighlight(Color( 0, 85, 85, 0));

                     if(!custom_matrix)matrix=transformMatrix(partOp(i));
                     part.draw(matrix);

                     SetHighlight(TRANSPARENT);
                  }
               }
               SetDrawMask();
               SetVariation();

               if(background_alpha()>=1)REPAO(back_meshes).draw();
            }

            // meshes in skeleton slots
            if(mode()==SLOTS && mesh_skel)REPAO(slot_meshes).draw(*mesh_skel);

            if(mode()==BODY && body)body->draw(MatrixIdentity); // body is game mesh which is already in correct transform

            LightDir(light_dir() ? !Vec(-1, -1, -1) : ActiveCam.matrix.z, 1-D.ambientColor()).add(false);
         }break;

         case RM_BLEND:
         {
            if(showChangeSkin())
            {
               SetMatrix(transformMatrix());
               const int bone=((lit_bone>=0) ? lit_bone : sel_bone);
               MeshLod &lod=getLod(); FREPA(lod)
               {
                  MeshPart &part=lod.parts[i];
                  if(partVisible(i, lod.parts[i]))part.drawBoneHighlight(bone);
               }
            }

            // draw preview
            if(ListElm *list_elm=Proj.list.visToData(Proj.list.lit))
               if(Elm *obj=list_elm->elm)if(ElmObj *obj_data=obj->objData())if(obj_data->mesh_id.valid())
                  if(C MeshPtr &mesh=Proj.gamePath(obj_data->mesh_id))
                     mesh->drawBlend(MatrixIdentity, VecZero, VecZero, &Vec4(1, 1, 1, 0.5f));

            if(background_alpha()<1)REPAO(back_meshes).drawBlend();
         }break;
      }
      particles.draw();
   }
   void ObjView::Draw(Viewport &viewport) {if(Edit::Viewport4::View *view=ObjEdit.v4.getView(&viewport))ObjEdit.draw(*view);}
          void ObjView::draw(Edit::Viewport4::View &view)
   {
      view.camera.set();
      if(mode()==BODY)
      {
         UID body_id=UIDZero; if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())body_id=mesh_data->body_id;
         body=Proj.gamePath(body_id);
      }

      CurrentEnvironment().set();
      bool astros=AstrosDraw; AstrosDraw=false;
      bool ocean =Water.draw; Water.draw=false;
      Renderer.wire=wire();

      Renderer(ObjView::Render);

      Renderer.wire=false;
      AstrosDraw=astros;
      Water.draw=ocean;

      // helpers using depth buffer
      bool line_smooth=D.lineSmooth(true); // this can be very slow, so don't use it everywhere
      Renderer.setDepthForDebugDrawing();
      if(axis() && !customDrawMatrix()){SetMatrix(); MatrixIdentity.draw();}
      if(mode()==TRANSFORM){SetMatrix(); D.depthLock(false); DrawMatrix(trans     .drawMatrix(), trans_axis); D.depthUnlock();}
      if(transMesh()      ){SetMatrix(); D.depthLock(false); DrawMatrix(trans_mesh.drawMatrix(), trans_axis); D.depthUnlock();}

      // draw boxes
      if(showBoxes())
      {
         if(showMainBox() && mesh.is()){SetMatrix(); mesh_box.draw(); if(mode()==TRANSFORM){SetMatrix(trans.matrix); mesh_box.draw(YELLOW);}}
         MeshLod &lod=getLod(); REPA(lod)
         {
            MeshPart &part=lod.parts[i]; if(partVisible(i, part))
            {
               bool lit=(i==lit_part), sel=partSel(i); if(lit || sel)
               {
                  Box box; if(part.base.getBox(box, transformMatrix(partOp(i))))
                  {
                     Color col=GetLitSelCol(lit, sel, TRANSPARENT);
                     /*if(col.a)*/{SetMatrix(); box.draw(col);}
                  }
               }
            }
         }
      }

      // draw vertexes
      if(vtxs() && !customDrawMatrix())
      {
         // points
         const bool draw_skin=(mode()==SKIN || mode()==BONES && lit_bone>=0);
         const byte bone=((lit_bone>=0) ? lit_bone : sel_bone)+1;
         const flt  r=0.0045f;
         D.depthLock(false);
       C MeshLod &lod=getDrawLod(); REPAD(p, lod)
         {
          C MeshPart &part=lod.parts[p]; if(partVisible(p, part))
            {
               bool lit=(p==lit_part), sel=partSel(p);
               Matrix m=transformMatrix(partOp(p)), cam=ActiveCam.matrix*~m; SetMatrix(m);
               VI.color(GetLitSelCol(lit, sel));
               REPA(part.base.vtx)
               {
                C Vec &pos=part.base.vtx.pos(i);
                  if(!vtxs_front() || frontFace(pos, part.base.vtx.nrm() ? &part.base.vtx.nrm(i) : null, cam))
                  {
                     if(draw_skin)
                     {
                        int sum=0;
                        if(part.base.vtx.matrix() && part.base.vtx.blend())
                        {
                         C VecB4 &m=part.base.vtx.matrix(i);
                         C VecB4 &b=part.base.vtx.blend (i);
                           REPA(m)if(m.c[i]==bone)sum+=b.c[i];
                        }
                        VI.dot(sum ? Lerp(BLUE, RED, Min(sum/255.0f, 1)) : WHITE, pos, r);
                     }else
                     {
                        VI.dot(pos, r);
                     }
                  }
               }
               VI.flush();
            }
         }
         VI.end();
         D.depthUnlock();

         // normals
         if(vtxs_normals())
         {
            flt length=Min(mesh_box.size().max()*0.07f, ActiveCam.dist*0.3f/Tan(D.viewFov()/2));
            VI.color(ColorAlpha(0.5f));
          C MeshLod &lod=getDrawLod(); REPAD(p, lod)
            {
             C MeshPart &part=lod.parts[p]; if(partVisible(p, part))
               {
                C MeshBase &base=part.base;
                  if(C Vec *pos=base.vtx.pos())
                  if(C Vec *nrm=base.vtx.nrm())
                  {
                     Matrix m=transformMatrix(partOp(p)), cam=ActiveCam.matrix*~m; SetMatrix(m);
                     flt    l=length/m.avgScale();
                     REPA(base.vtx)
                     {
                      C Vec &p=pos[i], &n=nrm[i];
                        if(!vtxs_front() || frontFace(p, &n, cam))VI.line(p, p+n*l);
                     }
                     VI.flush();
                  }
               }
            }
            VI.end();
         }
      }

      // draw vertex face selection
      if(editMeshParts())
         if(sel_vtx.elms() || sel_face.elms())
      {
         D.depthLock(false);
         Matrix matrix=transformMatrix(true), cam=ActiveCam.matrix*~matrix;
         SetMatrix(matrix);
       C MeshLod &lod=getLod();

         // draw faces first
         VI.color(ColorAlpha(LitSelColor, 0.3f));
         REPA(sel_face)
         {
          C VecI2 &v=sel_face[i]; if(InRange(v.x, lod))
            {
             C MeshPart &part=lod.parts[v.x]; if(partVisible(v.x, part))if(C Vec *pos=part.base.vtx.pos())
               {
                  if(v.y&SIGN_BIT)
                  {
                     int f=(v.y^SIGN_BIT); if(InRange(f, part.base.quad))
                     {
                      C VecI4 &v=part.base.quad.ind(f); // draw as 2 tris so that VI uses tris everywhere (mixed 'tri' and 'quad' calls at the same time may not be supported)
                        Tri a(pos[v.x], pos[v.y], pos[v.w], &VecZero),
                            b(pos[v.y], pos[v.z], pos[v.w], &VecZero);
                        if(trans_mesh.trans_normal)if(C Vec *nrm=part.base.vtx.nrm())
                        {
                           a.p[0]+=nrm[v.x]*trans_mesh.trans_normal; a.p[1]+=nrm[v.y]*trans_mesh.trans_normal; a.p[2]+=nrm[v.w]*trans_mesh.trans_normal;
                           b.p[1]+=nrm[v.z]*trans_mesh.trans_normal; b.p[0]=a.p[1]; b.p[2]=a.p[2];
                        }
                        VI.tri(a);
                        VI.tri(b);
                     }
                  }else
                  {
                     int f=v.y; if(InRange(f, part.base.tri))
                     {
                      C VecI &v=part.base.tri.ind(f);
                        Tri a(pos[v.x], pos[v.y], pos[v.z], &VecZero);
                        if(trans_mesh.trans_normal)if(C Vec *nrm=part.base.vtx.nrm())
                        {
                           a.p[0]+=nrm[v.x]*trans_mesh.trans_normal; a.p[1]+=nrm[v.y]*trans_mesh.trans_normal; a.p[2]+=nrm[v.z]*trans_mesh.trans_normal;
                        }
                        VI.tri(a);
                     }
                  }
               }
            }
         }
         VI.end();

         // vtxs
         VI.color(LitSelColor);
         REPA(sel_vtx)
         {
          C VecI2 &v=sel_vtx[i]; if(InRange(v.x, lod))
            {
             C MeshPart &part=lod.parts[v.x]; if(partVisible(v.x, part))if(InRange(v.y, part.base.vtx))
               {
                C Vec &pos=part.base.vtx.pos(v.y);
                  if(!vtxs_front() || frontFace(pos, part.base.vtx.nrm() ? &part.base.vtx.nrm(v.y) : null, cam))
                     VI.dot((trans_mesh.trans_normal && part.base.vtx.nrm()) ? pos+trans_mesh.trans_normal*part.base.vtx.nrm(v.y) : pos, 0.005f);
               }
            }
         }
         VI.end();

         D.depthUnlock();
      }

      // draw vertex face lit
      if(lit_vtx>=0 || lit_face!=-1)if(C MeshPart *part=getPart(lit_vf_part))if(C Vec *pos=part->base.vtx.pos())
      {
         D.depthLock(false);
         Matrix matrix=transformMatrix(partOp(lit_vf_part)), cam=ActiveCam.matrix*~matrix; SetMatrix(matrix);
         Memt<int> lit;

         // vertexes
         getVtxNeighbors(lit, lit_vtx, lit_vf_part);
         if(lit.elms())
         {
            REPA(lit){int vtx=lit[i]; VI.dot(sel_vtx.binaryHas(VecI2(lit_vf_part, vtx), Compare) ? SelColor : LitColor, pos[vtx], 0.005f);}
            VI.end();
         }
         if(InRange(lit_vtx, part->base.vtx))part->base.vtx.pos(lit_vtx).draw(sel_vtx.binaryHas(VecI2(lit_vf_part, lit_vtx), Compare) ? SelColor : LitColor, 0.01f);

         // faces
         getFaceNeighbors(lit, lit_face, lit_vf_part);
         if(lit.elms())
         {
            Color sel_color=ColorAlpha(SelColor, 0.3f),
                  lit_color=ColorAlpha(LitColor, 0.3f);
            REPA(lit)
            {
               int    face=lit[i];
             C Color &col =(sel_face.binaryHas(VecI2(lit_vf_part, face), Compare) ? sel_color : lit_color);
               if(face&SIGN_BIT)
               {
                  int f=(face^SIGN_BIT); if(InRange(f, part->base.quad))
                  {
                     C VecI4 &v=part->base.quad.ind(f); // draw as 2 tris so that VI uses tris everywhere (mixed 'tri' and 'quad' calls at the same time may not be supported)
                     VI.tri(col, pos[v.x], pos[v.y], pos[v.w]);
                     VI.tri(col, pos[v.y], pos[v.z], pos[v.w]);
                  }
               }else
               {
                  int f=face; if(InRange(f, part->base.tri))
                  {
                     C VecI &v=part->base.tri.ind(f);
                     VI.tri(col, pos[v.x], pos[v.y], pos[v.z]);
                  }
               }
            }
            VI.end();
         }
         D.depthUnlock();
      }

      // skeleton (slots/bones)
      if(mode()==SLOTS || mode()==SKIN || mode()==BONES)
         if(Skeleton *skel=getVisSkel())
      {
         D.depthLock(false);
         SetMatrix(trans.matrix);
         int  bone_tabs=boneTabs(), bone_axis=boneAxis();
         byte alpha=((mode()==SKIN && skin_tabs()==SKIN_SEL_BONE || mode()==BONES) ? 153 : 100);
         bool bone =(mode()==BONES && (bone_tabs==BONE_ADD || bone_tabs==BONE_MOVE || bone_tabs==BONE_ROT || bone_tabs==BONE_SCALE)
                  || mode()==SKIN),
              slot =(mode()==SLOTS && (slot_tabs()<0 || slot_tabs()==SLOT_MOVE || slot_tabs()==SLOT_ROT || slot_tabs()==SLOT_SCALE));
         int  lit_bone=        T.lit_bone_vis      ,
              sel_bone=(bone ? T.sel_bone_vis : -1),
              sel_slot=(slot ? T.sel_slot     : -1);
                          FREPAO(skel->bones).draw((sel_bone==i && lit_bone==i) ? LitSelColor : (sel_bone==i               ) ? SelColor : (lit_bone==i) ? LitColor : Color(0, 255, 255, alpha));
         if(mode()==SLOTS)FREPAO(skel->slots).draw((sel_slot==i && lit_slot==i) ? LitSelColor : (sel_slot==i || lit_slot==i) ? SelColor :                            Color(255, 128, 0, 255), SkelSlotSize);
         if(InRange(sel_bone, skel->bones) && mode()==BONES && (bone_tabs==BONE_MOVE || bone_tabs==BONE_ROT || bone_tabs==BONE_SCALE)){Matrix m=skel->bones[sel_bone]; m.scaleOrn(SkelSlotSize); DrawMatrix(m, bone_axis);}
         if(InRange(sel_slot, skel->slots)                                                                                           ){Matrix m=skel->slots[sel_slot]; m.scaleOrn(SkelSlotSize); DrawMatrix(m, slot_axis);}
         // draw parent<->children line
         if(C SkelBone *bone=skel->bones.addr(lit_bone))if(C SkelBone *parent=skel->bones.addr(bone->parent))D.line(PURPLE, bone->pos, parent->center());
         if(C SkelSlot *slot=skel->slots.addr(lit_slot))
         {
                                     if(C SkelBone *parent=skel->bones.addr(slot->bone ))D.line(PURPLE, slot->pos, parent->center());
            if(slot->bone!=slot->bone1)if(C SkelBone *parent=skel->bones.addr(slot->bone1))D.line(PURPLE, slot->pos, parent->center());
         }
         REPA(skel->bones)if(skel->bones[i].parent==lit_bone)D.line(WHITE, skel->bones[i].pos, skel->bones[lit_bone].to());
         D.depthUnlock();
      }
      
      // ragdoll
      if(mode()==RAGDOLL && mesh_skel)
      {
         D.depthLock(false);
         SetMatrix(trans.matrix);
         FREPA (mesh_skel->bones)if(mesh_skel->bones[i].flag&BONE_RAGDOLL)mesh_skel->bones[i].shape.draw(ColorAlpha(                                                  ORANGE    , (lit_bone==i) ? 1 : 0.55f), false, 16);
         FREPAO(mesh_skel->bones)                                                                .draw(ColorAlpha(FlagTest(mesh_skel->bones[i].flag, BONE_RAGDOLL) ? RED : CYAN, (lit_bone==i) ? 1 : 0.60f));
         if(ragdoll_tabs()!=RAGDOLL_TOGGLE)if(InRange(sel_bone, mesh_skel->bones)){Matrix m=mesh_skel->bones[sel_bone]; m.scaleOrn(SkelSlotSize); DrawMatrix(m, bone_axis);}
         D.depthUnlock();
      }
      
      // phys
      if(mode()==PHYS && phys)
      {
         D.depthLock(false);
         SetMatrix(trans.matrix);
         FREPAO(phys->parts).draw((sel_phys==i) ? LitSelColor : (lit_phys==i) ? SelColor : LitColor);
         if(InRange(sel_phys, phys->parts))DrawMatrix(phys_part_matrix, phys_axis);
         D.depthUnlock();
      }

      // cursor position
      if(show_cur_pos() && has_cur_pos && lit_vtx<0){SetMatrix(); cur_pos.draw(Color(255, 112), 0.005f);} // don't draw when we're highlighting a vertex because it's the same position

      D.lineSmooth(line_smooth);
   }
   void       ObjView::LodChanged(ObjView &editor) {editor.selLod      (editor.      lod_tabs());}
   void ObjView::VariationChanged(ObjView &editor) {editor.selVariation(editor.variation_tabs());}
   void      ObjView::ModeChanged(ObjView &editor)
   {
      editor.trans_mesh.apply();
      editor.setMenu();
      editor.mesh_parts.modeChanged();
      if(editor.mode()!=LOD )NewLod.hide();
      if(editor.mode()!=MESH)MeshAO.hide();
      if((editor.mode()==TRANSFORM || editor.mode()==SLOTS /*|| editor.mode()==BONES*/ || editor.mode()==RAGDOLL) && editor.mesh_elm)
         if(ElmMesh *mesh_data=editor.mesh_elm->meshData())
            if(mesh_data->body_id.valid())
      {
         if(editor.mode()==TRANSFORM                                              )Gui.msgBox(S, "This object is interpreted as cloth/armor to a different object (body).\nTherefore you cannot transform it.\nPlease transform the body object for this cloth.");
         if(editor.mode()==SLOTS || editor.mode()==BONES || editor.mode()==RAGDOLL)Gui.msgBox(S, "This object is interpreted as cloth/armor to a different object (body).\nTherefore it doesn't have its own skeleton.\nPlease edit skeleton on the body object for this cloth.");
         editor.mode.set(-1);
      }
      if(editor.mode()==TRANSFORM
      || editor.mode()==MESH     )editor.mode.tab(editor.mode())+=editor.trans_tabs;

      if(editor.mode()==MESH
      || editor.mode()==SKIN)editor.mode.tab(editor.mode())+=editor.mesh_parts;

      if(editor.mode()==GROUP 
      || editor.mode()==REMOVE
      || editor.mode()==MESH  
      || editor.mode()==VARIATIONS
      || editor.mode()==SKIN  )editor.mode.tab(editor.mode())+=editor.lod_tabs;

      if(editor.mode()==GROUP     ){editor.lod_tabs.move(editor.goto_group.rect().down()-Vec2(0, 0.01f)-editor.lod_tabs.rect().up());}
      if(editor.mode()==VARIATIONS){editor.lod_tabs.move(editor.mode      .rect().down()-Vec2(0, 0.01f)-editor.lod_tabs.rect().up());}
      if(editor.mode()==REMOVE
      || editor.mode()==MESH  
      || editor.mode()==SKIN      ){editor.lod_tabs.move(editor.mode      .rect().down()-Vec2(0, 0.01f)-editor.lod_tabs.rect().up()); SetObjOp(OP_OBJ_NONE);}

      if(editor.mode()==MESH
      || editor.mode()==VARIATIONS)
      {
         editor.mode.tab(editor.mode())+=editor.variation_tabs;
         editor.variation_tabs.move(editor.lod_tabs.rect().down()-Vec2(0, 0.01f)-editor.variation_tabs.rect().up());
      }
   }
   void ObjView::Mode1(ObjView &editor) {editor.mode.toggle(0);}
   void ObjView::Mode2(ObjView &editor) {editor.mode.toggle(1);}
   void ObjView::Mode3(ObjView &editor) {editor.mode.toggle(2);}
   void ObjView::Mode4(ObjView &editor) {editor.mode.toggle(3);}
   void ObjView::Mode5(ObjView &editor) {editor.mode.toggle(4);}
   void ObjView::Mode6(ObjView &editor) {editor.mode.toggle(5);}
   void ObjView::Mode7(ObjView &editor) {editor.mode.toggle(6);}
   void ObjView::Mode8(ObjView &editor) {editor.mode.toggle(7);}
   void ObjView::Mode9(ObjView &editor) {editor.mode.toggle(8);}
   void ObjView::Mode0(ObjView &editor) {editor.mode.toggle(9);}
   void ObjView::ModeS0(ObjView &editor) {editor.modeS(0);}
   void ObjView::ModeS1(ObjView &editor) {editor.modeS(1);}
   void ObjView::ModeS2(ObjView &editor) {editor.modeS(2);}
   void ObjView::ModeS3(ObjView &editor) {editor.modeS(3);}
   void ObjView::ModeS4(ObjView &editor) {editor.modeS(4);}
   void ObjView::ModeS5(ObjView &editor) {editor.modeS(5);}
   void ObjView::ModeS6(ObjView &editor) {editor.modeS(6);}
   void ObjView::ModeS7(ObjView &editor) {editor.modeS(7);}
   void ObjView::ModeS8(ObjView &editor) {editor.modeS(8);}
   void ObjView::ModeS9(ObjView &editor) {editor.modeS(9);}
   void ObjView::ModeS10(ObjView &editor) {editor.modeS(10);}
   void ObjView::ModeS11(ObjView &editor) {editor.modeS(11);}
   void ObjView::Identity(ObjView &editor) {editor.axis.push();}
   void ObjView::LightMode(ObjView &editor) {editor.light_dir.push();}
   void ObjView::PrevObj(ObjView &editor) {Proj.elmNext(editor.obj_id, -1);}
   void ObjView::NextObj(ObjView &editor) {Proj.elmNext(editor.obj_id);}
   void ObjView::ShowBox(ObjView &editor) {editor.box .push();}
   void ObjView::ShowCur(ObjView &editor) {editor.show_cur_pos.push();}
   void ObjView::VtxsChanged(ObjView &editor) {editor.vtxs_front.visible(editor.vtxs()); editor.vtxs_normals.visible(editor.vtxs());}
   void ObjView::MeshDelete(ObjView &editor) {editor.meshDelete    ();}
   void ObjView::MeshSplit(ObjView &editor) {editor.meshSplit     ();}
   void ObjView::MeshAlignXZ(ObjView &editor) {editor.meshAlign     (true );}
   void ObjView::MeshAlign(ObjView &editor) {editor.meshAlign     (false);}
   void ObjView::MeshWeldPos(ObjView &editor) {editor.meshWeldPos   ();}
   void ObjView::MeshSetPos(ObjView &editor) {editor.meshSetPos    ();}
   void ObjView::MeshReverse(ObjView &editor) {editor.meshReverse   ();}
   void ObjView::MeshReverseN(ObjView &editor) {editor.meshReverseN  ();}
   void ObjView::MeshSetNormalN(ObjView &editor) {editor.meshSetNrm    (VTX_NRM );}
   void ObjView::MeshSetNormalT(ObjView &editor) {editor.meshSetNrm    (VTX_TEX0);}
   void ObjView::MeshSetNormalP(ObjView &editor) {editor.meshSetNrm    (VTX_POS );}
   void ObjView::MeshSetNormal(ObjView &editor) {editor.meshSetNrm    (       0);}
   void ObjView::MeshSetNormalH(ObjView &editor) {editor.meshSetNrmH   (        );}
   void ObjView::MeshNormalY(ObjView &editor) {editor.meshNrmY      ();}
   void ObjView::MeshSetVtxAO(ObjView &editor) {MeshAO.activate();}
   void ObjView::MeshCreateFace(ObjView &editor) {editor.meshCreateFace();}
   void ObjView::MeshMergeFaces(ObjView &editor) {editor.meshMergeFaces();}
   void ObjView::MeshRotQuads(ObjView &editor) {editor.meshRotQuads  ();}
   void ObjView::MeshQuadToTri(ObjView &editor) {editor.meshQuadToTri ();}
   void ObjView::MeshTesselate(ObjView &editor) {editor.meshTesselate ();}
   void ObjView::MeshSubdivide(ObjView &editor) {editor.meshSubdivide ();}
   void ObjView::MeshColorBrght(ObjView &editor) {editor.meshColorBrght();}
   void ObjView::MeshDelDblSide(ObjView &editor) {editor.meshDelDblSide();}
   void ObjView::MeshSeparate1(ObjView &editor) {editor.meshSeparate1 ();}
   void ObjView::MeshSeparateN(ObjView &editor) {editor.meshSeparateN ();}
   void ObjView::MeshCopyParts(ObjView &editor) {editor.meshCopyParts ();}
   void ObjView::SetBody(ObjView &editor) {Proj.objSetBody(editor.menu_ids, ObjEdit.mesh_elm ? ObjEdit.mesh_elm->id : UIDZero);}
   void ObjView::AnimTargetObj(ObjView &editor) {editor.animTargetObj ();}
   void ObjView::MeshSkinFull(ObjView &editor) {editor.meshSkinFull  ();}
   void ObjView::MeshSkinFullP(ObjView &editor) {editor.meshSkinFullP ();}
   void ObjView::MeshSkinFullU(ObjView &editor) {editor.meshSkinFullU ();}
   void ObjView::MeshSkinAuto(ObjView &editor) {editor.meshSkinAuto  ();}
   void ObjView::MeshEditSel(ObjView &editor) {editor.mesh_parts.edit_selected.push();}
   void ObjView::MeshVFS0(ObjView &editor) {editor.vtx_face_sel_mode.toggle(0);}
   void ObjView::MeshVFS1(ObjView &editor) {editor.vtx_face_sel_mode.toggle(1);}
   void ObjView::MeshVFS2(ObjView &editor) {editor.vtx_face_sel_mode.toggle(2);}
   void ObjView::MeshVFS3(ObjView &editor) {editor.vtx_face_sel_mode.toggle(3);}
   void ObjView::ShowVtxs(ObjView &editor) {editor.vtxs        .push();}
   void ObjView::ShowVtxsF(ObjView &editor) {editor.vtxs_front  .push();}
   void ObjView::ShowVtxsN(ObjView &editor) {editor.vtxs_normals.push();}
   void  ObjView::GotoPhMtrl(ObjView &editor) {editor.gotoPhysMtrl();}
   void ObjView::ClearPhMtrl(ObjView &editor) {editor. setPhysMtrl(UIDZero);}
   void  ObjView::GotoBody(ObjView &editor) {editor.gotoBody    ();}
   void ObjView::ClearBody(ObjView &editor) {editor. setBody    (UIDZero);}
   void  ObjView::GotoGroups(ObjView &editor) {editor.gotoDrawGroupEnum();}
   void ObjView::ClearGroups(ObjView &editor) {editor. setDrawGroupEnum(UIDZero);}
   void ObjView::ClearBack(ObjView &editor) {editor.back_meshes.clear();}
   void ObjView::MeshRemVtxTex1(ObjView &editor) {editor.remVtx(VTX_TEX1         );}
   void ObjView::MeshRemVtxTex2(ObjView &editor) {editor.remVtx(         VTX_TEX2);}
   void ObjView::MeshRemVtxTex12(ObjView &editor) {editor.remVtx(VTX_TEX1|VTX_TEX2);}
   void ObjView::MeshRemVtxColor(ObjView &editor) {editor.remVtx(VTX_COLOR, null, ObjEdit.mesh_parts.list.sel.elms()>0);}
   void ObjView::MeshRemVtxSkin(ObjView &editor) {editor.remVtx(VTX_SKIN );}
   void ObjView::modeS(int i)
   {
      switch(mode())
      {
         case SKIN      :    skin_tabs.toggle(i); break;
         case SLOTS     :    slot_tabs.toggle(i); break;
         case BONES     :    bone_tabs.toggle(i); break;
         case PHYS      :    phys_tabs.toggle(i); break;
         case RAGDOLL   : ragdoll_tabs.toggle(i); break;
         case VARIATIONS:        selVariation(i); break;

         case GROUP:
         case LOD  : selLod(i); break;

         case TRANSFORM:
         case MESH     : trans_tabs.toggle(i); break;
      }
   }
   void ObjView::remVtx(uint flag, C MaterialPtr &mtrl, bool only_selected)
   {
      mesh_undos.set("remove");
      bool changed=false;
      REPD(i, mesh.lods())if(!only_selected || ObjEdit.selLod()==i || !ObjEdit.lod_tabs.visibleFull())
      {
         MeshLod &lod=mesh.lod(i); REPA(lod.parts)if(!only_selected || ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleFull())
         {
            MeshPart &part=lod.parts[i]; if(!mtrl || HasMaterial(part, mtrl))if(FlagTest(part.base.flag()|part.render.flag(), flag)){part.exclude(flag); changed=true;}
         }
      }
      if(changed)
      {
         if(flag&VTX_SKIN)mesh.clearSkeleton();
         setChangedMesh(true, false);
      }
   }
   void ObjView::MeshUndo(ObjView &editor) {editor.mesh_undos_undo= true; editor.mesh_undos.undo();}
   void ObjView::MeshRedo(ObjView &editor) {editor.mesh_undos_undo=false; editor.mesh_undos.redo();}
   void ObjView::Locate(ObjView &editor) {Proj.elmLocate(editor.obj_id);}
   flt  ObjView::absLodDist(C MeshLod &lod          ) {return   Abs(lodDist(lod));}
   flt     ObjView::lodDist(C MeshLod &lod          ) {return   lod.dist()*mesh_matrix.maxScale() ;}
   void    ObjView::lodDist(  MeshLod &lod, flt dist) {lod.dist(    dist  /mesh_matrix.maxScale());}
   void ObjView::setLodDist(  MeshLod &lod, flt dist) {lodDist(lod, dist); setChangedMesh(true, false);}
 C MeshLod& ObjView::getLod()C {return ConstCast(T).getLod();}
   MeshLod& ObjView::getLod()
   {
      if(mode()==GROUP || mode()==REMOVE || mode()==MESH || mode()==SKIN || mode()==LOD || mode()==VARIATIONS)return mesh.lod(selLod());
      return mesh;
   }
 C MeshLod& ObjView::getDrawLod()C {return ConstCast(T).getDrawLod();}
   MeshLod& ObjView::getDrawLod()
   {
      if(NewLod.visible() && NewLod.preview && NewLod.simplified_valid)return NewLod.simplified;
      if(MeshAO.visible() && MeshAO.preview && MeshAO.     baked_valid)return MeshAO.baked;
      if(InRange(lit_lod, mesh.lods()))return mesh.lod(lit_lod);
      return getLod();
   }
 C MeshPart* ObjView::getPart(int i)C {return getLod().parts.addr(i);}
   MeshPart* ObjView::getPart(int i)  {return getLod().parts.addr(i);}
   int       ObjView::getPart(GuiObj *go, C Vec2 &screen_pos, Vec *hit_pos)
   {
      int hit_part=-1;
      if(Edit::Viewport4::View *view=v4.getView(go))
      {
         MeshLod &lod=getLod();
         view->setViewportCamera();
         Vec pos, dir; ScreenToPosDir(screen_pos, pos, dir);
         pos+=(D.viewFrom ()/Dot(dir, ActiveCam.matrix.z))*dir;
         dir*= D.viewRange();
         flt frac, f; Vec hp;
         REPA(lod)if(partVisible(i, lod.parts[i]))if(Sweep(pos, dir, lod.parts[i], &transformMatrix(partOp(i)), &f, &hp, null, true, -1, false))if(hit_part<0 || f<frac)
         {
            hit_part=i; frac=f; if(hit_pos)*hit_pos=hp;
         }
      }
      return hit_part;
   }
   int ObjView::getVtxFace(GuiObj *go, C Vec2 &screen_pos, int *hit_vtx, int *hit_face, Vec *hit_pos)
   {
      if(hit_vtx )*hit_vtx =-1;
      if(hit_face)*hit_face=-1;
      int hit_part=-1;
      if(Edit::Viewport4::View *view=v4.getView(go))
      {
         MeshLod &lod=getLod();
         view->setViewportCamera();
         Matrix matrix=transformMatrix(), cam=ActiveCam.matrix*~matrix;
         flt dist=0, d;
         if(hit_vtx)
         {
            REPAD(p, lod)
            {
               C MeshPart &part=lod.parts[p]; if(partVisible(p, part))if(part.base.vtx.pos())REPA(part.base.vtx)
               {
                  C Vec &pos=part.base.vtx.pos(i); if(!vtxs_front() || frontFace(pos, part.base.vtx.nrm() ? &part.base.vtx.nrm(i) : null, cam))
                  {
                     Vec world_pos=pos*matrix; Vec2 s; if(PosToScreen(world_pos, s))
                     {
                        d=Dist2(s, screen_pos);
                        if(hit_part<0 || d<dist){*hit_vtx=i; hit_part=p; if(hit_pos)*hit_pos=world_pos; dist=d;}
                     }
                  }
               }
            }
            if(dist>Sqr(0.03f))*hit_vtx=hit_part=-1;
         }
         if(hit_face && hit_part<0)
         {
            Vec pos, dir; ScreenToPosDir(screen_pos, pos, dir);
            pos+=(D.viewFrom ()/Dot(dir, ActiveCam.matrix.z))*dir;
            dir*= D.viewRange();
            int f; Vec hp;
            REPAD(p, lod)
            {
               C MeshPart &part=lod.parts[p]; if(partVisible(p, part))if(Sweep(pos, dir, part, &matrix, &d, &hp, &f, true, -1, false))if(hit_part<0 || d<dist)
               {
                  *hit_face=f; hit_part=p; if(hit_pos)*hit_pos=hp; dist=d;
               }
            }
         }
      }
      return hit_part;
   }
   void ObjView::getVtxs(GuiObj *go, C Shape &shape, MemPtr<VecI2> vtxs) // 'vtxs' will be sorted
   {
      vtxs.clear();
      if(Edit::Viewport4::View *view=v4.getView(go))
      {
         MeshLod &lod=getLod();
         view->setViewportCamera();
         Matrix matrix=transformMatrix(), cam=ActiveCam.matrix*~matrix;
         REPAD(p, lod)
         {
          C MeshPart &part=lod.parts[p]; if(partVisible(p, part))if(part.base.vtx.pos())REPA(part.base.vtx)
            {
             C Vec &pos=part.base.vtx.pos(i); if(!vtxs_front() || frontFace(pos, part.base.vtx.nrm() ? &part.base.vtx.nrm(i) : null, cam))
               {
                  Vec world_pos=pos*matrix; Vec2 s; if(PosToScreen(world_pos, s))if(Cuts(s, shape))vtxs.binaryInclude(VecI2(p, i), Compare);
               }
            }
         }
      }
   }
   void ObjView::getSkel(GuiObj *go, C Vec2 &screen_pos, int *bone_i, int *slot_i)
   {
      if(bone_i)*bone_i=-1;
      if(slot_i)*slot_i=-1;
      flt dist;
      if(C Skeleton *skel=getVisSkel())if(Edit::Viewport4::View *view=v4.getView(go))
      {
         view->setViewportCamera();
         if(bone_i)
         {
            REPA(skel->bones)
            {
             C SkelBone &bone=skel->bones[i];
               flt d; if(Distance2D(screen_pos, Edge(bone.pos, bone.to())*trans.matrix, d, 0))
                  if(*bone_i<0 || d<dist){*bone_i=i; dist=d;}
            }
         }
         if(slot_i)
         {
            REPA(skel->slots)
            {
             C SkelSlot &slot=skel->slots[i];
               flt d; if(Distance2D(screen_pos, Edge(slot.pos, slot.pos+slot.perp*SkelSlotSize)*trans.matrix, d, 0))
                  if(*slot_i<0 && (!bone_i || *bone_i<0) || d<dist){*slot_i=i; dist=d; if(bone_i)*bone_i=-1;}
            }
         }
         if(bone_i && *bone_i>=0
         || slot_i && *slot_i>=0)
            if(dist>0.03f){if(bone_i)*bone_i=-1; if(slot_i)*slot_i=-1;}
      }
   }
   void ObjView::getSkelSlot(C Vec2 &screen_pos)
   {
      getSkel(Gui.objAtPos(screen_pos), screen_pos, (slot_tabs()==SLOT_ADD || slot_tabs()==SLOT_PARENT       ) ? &lit_bone_vis : null,
                                                    (slot_tabs()<0 || slot_tabs()>=SLOT_DEL || Gui.dragging()) ? &lit_slot     : null);
      lit_bone=visToBone(lit_bone_vis);
   }
   int ObjView::getPhys(GuiObj *go, C Vec2 &screen_pos)
   {
      int part_hit=-1;
      if(phys)if(Edit::Viewport4::View *view=v4.getView(go))
      {
         view->setViewportCamera();
         flt  d, dist;
         Vec  pos, dir; ScreenToPosDir(screen_pos, pos, dir);
         Edge edge(pos, pos+dir*D.viewRange());

         REPA(*phys)if(phys->parts[i].type()==PHYS_SHAPE)
         {
            Shape shape=phys->parts[i].shape*trans.matrix;
            Vec2  screen;
            if(PosToScreen(shape.pos(), screen))
            {
               d=Dist(screen_pos, screen);
               if((part_hit==-1 || d<dist) && Cuts(Shape(edge), shape)){part_hit=i; dist=d;}
            }
         }
      }
      return part_hit;
   }
   MeshPart* ObjView::validateDup(int part)
   {
      MeshPart *p=getPart(part);
      if(p)
      {
         if(!(p->flag()&VTX_DUP) || vtx_dup_mode!=vtx_face_sel_mode())
         {
            mesh.exclude(VTX_DUP); // need to remove for all lods & parts
            uint flag=0;
            switch(vtx_dup_mode=vtx_face_sel_mode())
            {
               case 1: flag=VTX_NRM ; break; // must share the same normal
               case 2: flag=VTX_TEX0; break; // must share the same UV
               case 3: flag=       0; break; // no requirements
            }
            p->base.setVtxDup(flag, vtxDupPosEps(), 0.99f);
         }
      }
      return p;
   }
   void ObjView::getVtxNeighbors(MemPtr<int> vtxs, int vtx, int part)
   {
      vtxs.clear();
      if(vtx_face_sel_mode()==0){if(vtx>=0)vtxs.add(vtx);}else if(MeshPart *p=validateDup(part))p->base.getVtxNeighbors(vtx, vtxs);
   }
   void ObjView::getFaceNeighbors(MemPtr<int> faces, int face, int part)
   {
      faces.clear();
      if(vtx_face_sel_mode()==0){if(face!=-1)faces.add(face);}else if(MeshPart *p=validateDup(part))p->base.getFaceNeighbors(face, faces);
   }
   void ObjView::getSelectedVtxs(MemPtr<VecI2> vtxs, bool from_vtxs, bool from_faces)
   {
      vtxs.clear();
      if(from_vtxs )vtxs=sel_vtx;
      if(from_faces)REPA(sel_face)
      {
       C VecI2 &v=sel_face[i]; if(MeshPart *part=getPart(v.x))
         {
          C MeshBase &base=part->base;
            if(v.y&SIGN_BIT){int f=v.y^SIGN_BIT; if(InRange(f, base.quad)){C VecI4 &q=base.quad.ind(f); REPA(q)vtxs.binaryInclude(VecI2(v.x, q.c[i]), Compare);}}
            else            {int f=v.y         ; if(InRange(f, base.tri )){C VecI  &t=base.tri .ind(f); REPA(t)vtxs.binaryInclude(VecI2(v.x, t.c[i]), Compare);}}
         }
      }
   }
   void ObjView::getSamePos(int part, int vtx, MemPtr<VecI2> vtxs)
   {
      vtxs.clear();
    C MeshLod &lod=getLod(); if(C MeshPart *p=getPart(part))if(InRange(vtx, p->base.vtx))
      {
       C Vec &pos=p->base.vtx.pos(vtx); REPAD(p, lod)
         {
          C MeshPart &part=lod.parts[p]; if(partVisible(p, part))REPA(part.base.vtx)if(Equal(part.base.vtx.pos(i), pos))vtxs.add(VecI2(p, i));
         }
      }
   }
   void ObjView::includeSamePos(int part, MemPtr<VecI2> vtxs) // !! assumes that 'vtxs' is sorted and valid (point to valid indexes) !!
   {
    C MeshLod &lod=getLod(); if(C MeshPart *p=getPart(part))REPA(p->base.vtx)
      {
       C Vec &pos=p->base.vtx.pos(i); REPAD(v, vtxs)
         {
          C VecI2 &vtx=vtxs[v]; if(Equal(pos, lod.parts[vtx.x].base.vtx.pos(vtx.y))){vtxs.binaryInclude(VecI2(part, i), Compare); break;} // if share position then process this one too
         }
      }
   }
   void ObjView::selVFClear()
   {
      sel_vtx .clear();
      sel_face.clear();
   }
   void ObjView::litSelVFClear()
   {
      selVFClear();
      lit_vf_part=lit_vtx=lit_face=-1;
   }
   void ObjView::selVFSet(int part, int vtx, int face, bool same_pos)
   {
      selVFClear();
      if(part>=0)
      {
         if(vtx >= 0)if(!same_pos)sel_vtx .add(VecI2(part, vtx ));else getSamePos(part, vtx, sel_vtx);
         if(face!=-1)             sel_face.add(VecI2(part, face));
      }
   }
   void ObjView::selVFToggle(int part, int vtx, int face, bool same_pos)
   {
      if(part>=0)
      {
         if(vtx >= 0)if(!same_pos)sel_vtx .binaryToggle(VecI2(part, vtx ), Compare);else{Memt<VecI2> vtxs; getSamePos(part, vtx, vtxs); REPA(vtxs)sel_vtx.binaryToggle(vtxs[i], Compare);}
         if(face!=-1)             sel_face.binaryToggle(VecI2(part, face), Compare);
      }
   }
   void ObjView::selVFInclude(int part, int vtx, int face, bool same_pos)
   {
      if(part>=0)
      {
         if(vtx >= 0)if(!same_pos)sel_vtx .binaryInclude(VecI2(part, vtx ), Compare);else{Memt<VecI2> vtxs; getSamePos(part, vtx, vtxs); REPA(vtxs)sel_vtx.binaryInclude(vtxs[i], Compare);}
         if(face!=-1)             sel_face.binaryInclude(VecI2(part, face), Compare);
      }
   }
   void ObjView::selVFExclude(int part, int vtx, int face, bool same_pos)
   {
      if(part>=0)
      {
         if(vtx >= 0)if(!same_pos)sel_vtx .binaryExclude(VecI2(part, vtx ), Compare);else{Memt<VecI2> vtxs; getSamePos(part, vtx, vtxs); REPA(vtxs)sel_vtx.binaryExclude(vtxs[i], Compare);}
         if(face!=-1)             sel_face.binaryExclude(VecI2(part, face), Compare);
      }
   }
   void ObjView::selVFDo()
   {
      Memt<int>  vtxs; getVtxNeighbors ( vtxs, lit_vtx , lit_vf_part);
      Memt<int> faces; getFaceNeighbors(faces, lit_face, lit_vf_part);
      bool      same_pos=(vtx_face_sel_mode()==0);
      if(    mesh_parts.list.selMode()==LSM_SET)selVFClear();
      switch(mesh_parts.list.selMode())
      {
         case LSM_SET    :
         case LSM_INCLUDE: REPA(vtxs)selVFInclude(lit_vf_part, vtxs[i], -1, same_pos); REPA(faces)selVFInclude(lit_vf_part, -1, faces[i], same_pos); break;
         case LSM_EXCLUDE: REPA(vtxs)selVFExclude(lit_vf_part, vtxs[i], -1, same_pos); REPA(faces)selVFExclude(lit_vf_part, -1, faces[i], same_pos); break;
         case LSM_TOGGLE : REPA(vtxs)selVFToggle (lit_vf_part, vtxs[i], -1, same_pos); REPA(faces)selVFToggle (lit_vf_part, -1, faces[i], same_pos); break;
      }
   }
   void ObjView::selVFDo(bool include, bool same_pos)
   {
      if(showVtxSelCircle())
      {
         Memt<VecI2> vtxs; getVtxs(Gui.ms(), vtxSelCircle(), vtxs);
         if(same_pos) // iterate all mesh vertexes and check if they have the same position as any of the 'vtxs'
         {
          C MeshLod &lod=getLod(); REPA(lod)if(partVisible(i, lod.parts[i]))includeSamePos(i, vtxs);
         }
         if(include)REPA(vtxs)sel_vtx.binaryInclude(vtxs[i], Compare);
         else       REPA(vtxs)sel_vtx.binaryExclude(vtxs[i], Compare);
      }
   }
   void ObjView::selUndo()
   {
      cptr change_type="sel";
      bool same=(mesh_undos.lastChangeType()==change_type);
      if(MeshChange *change=mesh_undos.set(change_type, false, 1))if(same)change->selOnly();
   }
   void ObjView::setMenu()
   {
      ::Viewport4Region::setMenu(selected());
      mesh_ops.menu.enabled(selected() && mode()==MESH );
      skin_ops.menu.enabled(selected() && mode()==SKIN );
      slot_ops.menu.enabled(selected() && mode()==SLOTS);
      bone_ops.menu.enabled(selected() && mode()==BONES);
      phys_ops.menu.enabled(selected() && mode()==PHYS );
      lod     .menu.enabled(selected() && mode()==LOD  );
   }
   void ObjView::setMenu(Node<MenuElm> &menu, C Str &prefix)
   {
      ::Viewport4Region::setMenu(menu, prefix);
      FREPA(menu.children)if(menu.children[i].name==prefix+"View")
      {
         Node<MenuElm> &v=menu.children[i];
         v.New().create("Mode 1", Mode1, T, true).kbsc(KbSc(KB_F1));
         v.New().create("Mode 2", Mode2, T, true).kbsc(KbSc(KB_F2));
         v.New().create("Mode 3", Mode3, T, true).kbsc(KbSc(KB_F3));
         v.New().create("Mode 4", Mode4, T, true).kbsc(KbSc(KB_F4));
         v.New().create("Mode 5", Mode5, T, true).kbsc(KbSc(KB_F5));
         v.New().create("Mode 6", Mode6, T, true).kbsc(KbSc(KB_F6));
         v.New().create("Mode 7", Mode7, T, true).kbsc(KbSc(KB_F7));
         v.New().create("Mode 8", Mode8, T, true).kbsc(KbSc(KB_F8));
         v.New().create("Mode 9", Mode9, T, true).kbsc(KbSc(KB_F9));
         v.New().create("Mode 0", Mode0, T, true).kbsc(KbSc(KB_F10));
         v.New().create("ModeS 0" , ModeS0 , T).kbsc(KbSc(KB_F1 , KBSC_SHIFT));
         v.New().create("ModeS 1" , ModeS1 , T).kbsc(KbSc(KB_F2 , KBSC_SHIFT));
         v.New().create("ModeS 2" , ModeS2 , T).kbsc(KbSc(KB_F3 , KBSC_SHIFT));
         v.New().create("ModeS 3" , ModeS3 , T).kbsc(KbSc(KB_F4 , KBSC_SHIFT));
         v.New().create("ModeS 4" , ModeS4 , T).kbsc(KbSc(KB_F5 , KBSC_SHIFT));
         v.New().create("ModeS 5" , ModeS5 , T).kbsc(KbSc(KB_F6 , KBSC_SHIFT));
         v.New().create("ModeS 6" , ModeS6 , T).kbsc(KbSc(KB_F7 , KBSC_SHIFT));
         v.New().create("ModeS 7" , ModeS7 , T).kbsc(KbSc(KB_F8 , KBSC_SHIFT));
         v.New().create("ModeS 8" , ModeS8 , T).kbsc(KbSc(KB_F9 , KBSC_SHIFT));
         v.New().create("ModeS 9" , ModeS9 , T).kbsc(KbSc(KB_F10, KBSC_SHIFT));
         v.New().create("ModeS 10", ModeS10, T).kbsc(KbSc(KB_F11, KBSC_SHIFT));
         v.New().create("ModeS 11", ModeS11, T).kbsc(KbSc(KB_F12, KBSC_SHIFT));
         v.New().create("Box"            , ShowBox  , T).kbsc(KbSc(KB_B, KBSC_ALT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("ShowCur"        , ShowCur  , T).kbsc(KbSc(KB_C, KBSC_ALT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("ShowVtxs"       , ShowVtxs , T).kbsc(KbSc(KB_V, KBSC_ALT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("ShowVtxsF"      , ShowVtxsF, T).kbsc(KbSc(KB_V, KBSC_ALT|KBSC_SHIFT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("ShowVtxsN"      , ShowVtxsN, T).kbsc(KbSc(KB_N, KBSC_ALT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Identity Matrix", Identity , T).kbsc(KbSc(KB_A, KBSC_ALT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Light Direction", LightMode, T).kbsc(KbSc(KB_L, KBSC_ALT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Previous Object", PrevObj  , T).kbsc(KbSc(KB_PGUP, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Next Object"    , NextObj  , T).kbsc(KbSc(KB_PGDN, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         break;
      }
   }
   ObjView& ObjView::create()
   {
      mesh_undos.maxMemUsage(100*1024*1024); // 100 MB
      ts.reset().size=0.045f; ts.align.set(1, 0);
      ::Viewport4Region::create(Draw, false, 0, PI, 1, 0.01f, 1000); v4.toggleHorizontal();
      flt h=0.05f;

      T+=info.create(Vec2(0)); // create first to display text on bottom

      T+=axis        .create(Rect_LU(ctrls       .rect().ld(), h)).focusable(false).desc(S+"Display identity matrix axes, where:\nRed = right (x vector)\nGreen = up (y vector)\nBlue = forward (z vector)\nLength of each vector is 1 unit\nPlease note that the camera in Object Editor is by default faced from forward to backward.\n\nKeyboard Shortcut: Alt+A"); axis.mode=BUTTON_TOGGLE; axis.set(true); axis.image="Gui/Misc/axis.img";
      T+=box         .create(Rect_LU(axis        .rect().ru(), h)).focusable(false).desc("Display object bounding box\nKeyboard Shortcut: Alt+B"); box.mode=BUTTON_TOGGLE; box.image="Gui/Misc/box.img";
         wire        .pos   (        box         .rect().ru());
      T+=show_cur_pos.create(Rect_LU(wire        .rect().ru(), h)).focusable(false).desc("Display mouse cursor position information\nKeyboard Shortcut: Alt+C"); show_cur_pos.mode=BUTTON_TOGGLE; show_cur_pos.image="Gui/Misc/select_info.img";
      T+=vtxs        .create(Rect_LU(axis        .rect().ld(), h)).focusable(false).func(VtxsChanged, T).desc("Display mesh vertexes\nKeyboard Shortcut: Alt+V"); vtxs.mode=BUTTON_TOGGLE; vtxs.image="Gui/Misc/vertexes.img";
      T+=vtxs_front  .create(Rect_LU(vtxs        .rect().ru(), h), "F").focusable(false).desc("Display only front facing vertexes\nKeyboard Shortcut: Shift+Alt+V").hide(); vtxs_front.mode=BUTTON_TOGGLE; vtxs_front.set(true);
      T+=vtxs_normals.create(Rect_LU(vtxs_front  .rect().ru(), h), "N").focusable(false).desc("Display vertex normals\nKeyboard Shortcut: Alt+N").hide(); vtxs_normals.mode=BUTTON_TOGGLE;
      T+=light_dir   .create(Rect_LU(vtxs_normals.rect().ru(), h)).setImage(Proj.icon_env).focusable(false).desc("Set Vertical Light Direction\nKeyboard Shortcut: Alt+L"); light_dir.mode=BUTTON_TOGGLE;
      cam_spherical.hide(); cam_lock.pos(cam_spherical.pos());

      T+=mesh_undo.create(Rect_LU(ctrls    .rect().ru()+Vec2(h, 0), h, h)     ).func(MeshUndo, T).focusable(false).desc("Undo"); mesh_undo.image="Gui/Misc/undo.img";
      T+=mesh_redo.create(Rect_LU(mesh_undo.rect().ru()           , h, h)     ).func(MeshRedo, T).focusable(false).desc("Redo"); mesh_redo.image="Gui/Misc/redo.img";
      T+=locate   .create(Rect_LU(mesh_redo.rect().ru()           , h, h), "L").func(Locate  , T).focusable(false).desc("Locate this element in the Project");

      T+=mode.create(Rect_LU(locate.rect().ru()+Vec2(h, 0), 1.84f, h), 0, mode_t, Elms(mode_t), true).func(ModeChanged, T);
      mode.tab(TRANSFORM ).desc("Apply transformation to the object\nKeyboard Shortcut: F1");
      mode.tab(PARAM     ).desc("Edit object parameters\nKeyboard Shortcut: F2");
      mode.tab(LOD       ).desc("Edit object levels of detail\nKeyboard Shortcut: F3");
      mode.tab(MESH      ).desc("Edit Mesh for this object\nKeyboard Shortcut: F4");
      mode.tab(VARIATIONS).desc("Edit Mesh Material Variations\nVariations allow to specify different combinations of Materials for Mesh Parts.\nKeyboard Shortcut: F5");
      mode.tab(SLOTS     ).desc("Edit object slots\n\nSlots are places for attachable items.\nFor example you can create a slot named \"hand\" in which later you can put a weapon like sword or a gun.\n\nSlots should be created with correct skeleton bone as their parent,\nwhich will allow the slot to get animated along with the bone.\n\nKeyboard Shortcut: F6");
      mode.tab(BONES     ).desc("Edit Mesh Skeleton Bones for this object\nKeyboard Shortcut: F7");
      mode.tab(SKIN      ).desc("Edit Mesh Skinning for this object\nKeyboard Shortcut: F8");
      mode.tab(PHYS      ).desc("Edit object physical body used for collision detection\nKeyboard Shortcut: F9");
      mode.tab(GROUP     ).desc("Assign \"Draw Groups\" to object parts\nDraw groups allow dynamic control of which object parts should be displayed during the game.\nAll object parts are by default assigned to first group (index=0).\nUp to 32 groups are supported.\n\nKeyboard Shortcut: F10");
      mode.tab(REMOVE    ).desc("Set which object parts should be removed\nClick on object parts to toggle them.\n\nParts which are removed using this function will not be available in the game at all.\nIf you wish to dynamically control visibility of a part, then please use \"Draw Groups\" instead.");
      mode.tab(RAGDOLL   ).desc("Edit Ragdoll for this object");
      mode.tab(BODY      ).desc("Set body for this object, making it a cloth/armor\nThis option will make the objects share the same skeleton,\nallowing them to be animated and drawn using one skeleton.");
      mode.tab(BACKGROUND).desc("Drag and drop objects onto the viewport to display them as background.\nThis can be useful when wanting to adjust scale of items according to existing objects.\nYou can drag and drop existing objects onto the viewport to keep displaying them, then adjust the new object scale to match the other ones.");

      mode.tab(TRANSFORM)+=trans.create(false).pos(mode.tab(TRANSFORM).rect().ld()-Vec2(0, 0.01f));
      mode.tab(LOD      )+=lod  .create(     ).pos(mode.tab(0        ).rect().ld()-Vec2(0, 0.01f));
      mode.tab(GROUP    )+=group.create(     );

      T+=leaf.create();
      param_edit.create(mode.tab(PARAM));

      Property &body=body_props.New().create("Body Object", MemberDesc(DATA_STR)); body.button.create("C").func(ClearBody, T).desc("Remove any current body this object is assigned to");
      AddProperties(body_props, mode.tab(BODY), mode.rect().ld()-Vec2(0, 0.01f), 0.05f, 0.90f, &ts);
      mode.tab(BODY)+=goto_body.create(body.textline.rect()).func(GotoBody, T).focusable(false).desc("Click to open body in the editor.\nDrag and drop an object here to set it as body."); goto_body.text_align=1; body.textline.del();

                                  lod_tabs.create((cchar**)null, 0).valid(true).func(      LodChanged, T, true);
      mode.tab(VARIATIONS)+=variation_tabs.create((cchar**)null, 0).valid(true).func(VariationChanged, T, true);

      Property &groups=group_props.New().create("Draw Groups Enum", MemberDesc(DATA_STR)); groups.button.create("C").func(ClearGroups, T).desc("Remove draw group enum assigned to this object");
      AddProperties(group_props, mode.tab(GROUP), mode.rect().ld()-Vec2(0, 0.01f), 0.05f, 0.90f, &ts);
      mode.tab(GROUP)+=goto_group.create(groups.textline.rect()).func(GotoGroups, T).focusable(false).desc("Click to open enum in the editor.\nDrag and drop an enum here to set it.\nElements from this enum can be assigned to object parts.\nEach enum element represents a different \"draw group\".\nUp to 32 groups are supported."); goto_group.text_align=1; groups.textline.del();

      mode.tab(BACKGROUND)+=background_alpha.create(Rect_U(mode.rect().down()-Vec2(0, 0.01f), 0.2f, 0.055f), 0.5f); mode.tab(BACKGROUND)+=background_alpha_t.create(background_alpha.rect().right()+Vec2(0.01f, 0), "Alpha", &ts);
      mode.tab(BACKGROUND)+=clear_background.create(Rect_R(background_alpha.rect().left()-Vec2(0.055f, 0), 0.2f, 0.055f), "Clear").focusable(false).func(ClearBack, T).desc("Clear all background objects");

      createSlots  ();
      createBones  ();
      createPhys   ();
      createRagdoll();
      createMesh   ();

      return T;
   }
   void ObjView::createMesh()
   {
      Gui+=MeshAO.create();
                            mesh_parts     .create();
      mode.tab(VARIATIONS)+=mesh_variations.create();

      flt h=mode.rect().h();
      trans_tabs.create(Rect_LU(mode.rect().ru()+Vec2(h, 0), h*3, h), 0, (cchar**)null, 3).valid(true).set(0);
      trans_tabs.tab(TRANS_MOVE ).setImage("Gui/Misc/move.img"  ).desc("Move\nRightClick while moving the mouse on the Viewport\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F1");
      trans_tabs.tab(TRANS_ROT  ).setImage("Gui/Misc/rotate.img").desc("Rotate\nRightClick while moving the mouse on the Viewport\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F2");
      trans_tabs.tab(TRANS_SCALE).setImage("Gui/Misc/scale.img" ).desc("Scale\nRightClick while moving the mouse on the Viewport\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F3");

      mode.tab(MESH)+=trans_mesh.create(true); trans_mesh.move(trans_tabs.rect().down()-Vec2(0, 0.01f)-trans_mesh.rect().up());

      // keep to the left because LOD tabs may be visible
      cchar8 *vfs_modes[]=
      {
         "Single",
         "Normal",
         "UV",
         "All",
      };
      cchar8 *vfs_desc[]=
      {
         "This mode selects only single face/vertexes\n\nKeyboard Shortcut: Shift+Alt+1",
         "This mode selects a group of face/vertexes that are connected to each other and have the same normal vectors\n\nKeyboard Shortcut: Shift+Alt+2",
         "This mode selects a group of face/vertexes that are connected to each other and have the same texture coordinates\n\nKeyboard Shortcut: Shift+Alt+3",
         "This mode selects a group of all face/vertexes that are connected to each other\n\nKeyboard Shortcut: Shift+Alt+4",
      };
      mode.tab(MESH)+=vtx_face_sel_text.create(Rect(mesh_undo.rect().ld()-Vec2(0, 0.01f+ts.size.y/2)), "Vertex/Face Selection:", &ts).visible(mesh_parts.edit_selected());
      mode.tab(MESH)+=vtx_face_sel_mode.create(Rect_L(vtx_face_sel_text.rect().ld()-Vec2(0, ts.size.y+0.005f), 0.40f, 0.055f), 0, vfs_modes, Elms(vfs_modes), true).valid(true).set(0).visible(vtx_face_sel_text.visible());
      REPA(vfs_desc)vtx_face_sel_mode.tab(i).desc(vfs_desc[i]);

      {
         Node<MenuElm> n;
         n.New().create("Delete"                        , MeshDelete    , T).kbsc(KbSc(KB_DEL                                 )).desc("This option will delete selected vertexes/faces");
         n.New().create("Split"                         , MeshSplit     , T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD                    )).desc("This option will split selected vertexes/faces into new Mesh Parts");
         n.New().create("Weld Vertex Positions"         , MeshWeldPos   , T).kbsc(KbSc(KB_W, KBSC_CTRL_CMD                    )).desc("This option will weld positions of selected vertexes making them share one position");
         n.New().create("Set Vertex Positions"          , MeshSetPos    , T).kbsc(KbSc(KB_W, KBSC_CTRL_CMD|KBSC_SHIFT         )).desc("This option will set positions of selected vertexes to the same position as the highlighted vertex.\nTo use:\n-Select vertexes\n-Highlight target vertex\n-Press Keyboard shortcut for this option");
         n.New().create("Reverse"                       , MeshReverse   , T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD                    )).desc("This option will reverse the selected faces");
         n.New().create("Reverse Normals"               , MeshReverseN  , T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_SHIFT         )).desc("This option will reverse normals of selected vertexes/faces");
         n.New().create("Set Normals (Normal)"          , MeshSetNormalN, T).kbsc(KbSc(KB_N, KBSC_CTRL_CMD                    )).desc("This option will set normals of selected vertexes/faces\nNormals will be smoothened based on existing normal vertex connections");
         n.New().create("Set Normals (UV)"              , MeshSetNormalT, T).kbsc(KbSc(KB_N, KBSC_CTRL_CMD|KBSC_SHIFT         )).desc("This option will set normals of selected vertexes/faces\nNormals will be smoothened based on UV vertex connections");
         n.New().create("Set Normals (Position)"        , MeshSetNormalP, T).kbsc(KbSc(KB_N, KBSC_CTRL_CMD|KBSC_WIN_CTRL      )).desc("This option will set normals of selected vertexes/faces\nNormals will be smoothened based on vertex connections");
         n.New().create("Set Normals (None)"            , MeshSetNormal , T).kbsc(KbSc(KB_N, KBSC_CTRL_CMD|KBSC_ALT           )).desc("This option will set normals of selected vertexes/faces");
         n.New().create("Set Normals (Highlight)"       , MeshSetNormalH, T).kbsc(KbSc(KB_N, KBSC_WIN     |KBSC_ALT           )).desc("This option will set normals of selected vertexes/faces\nNormals will be taken from highlighted element.\nTo use:\n-Select parts\n-Highlight target part\n-Press Keyboard shortcut for this option");
         n.New().create("Align Normals Up"              , MeshNormalY   , T).kbsc(KbSc(KB_N, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_ALT)).desc("This option will align normals towards up direction by a bit");
         n.New().create("Align To Vertex Round XZ"      , MeshAlignXZ   , T).kbsc(KbSc(KB_A, KBSC_CTRL_CMD|KBSC_SHIFT         )).desc("This option will align the object so that the highlighted vertex XZ position will be an integer.");
         n.New().create("Align To Vertex Round"         , MeshAlign     , T).kbsc(KbSc(KB_A, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_ALT)).desc("This option will align the object so that the highlighted vertex position will be an integer.");
         n.New().create("Set Vertex Ambient Occlusion"  , MeshSetVtxAO  , T).kbsc(KbSc(KB_A, KBSC_CTRL_CMD|KBSC_WIN           )).desc("This option will calculate Ambient Occlusion for each vertex and store it as vertex color.");
         n.New().create("Create Face"                   , MeshCreateFace, T).kbsc(KbSc(KB_F, KBSC_CTRL_CMD                    )).desc("This option will create 1 face from selected vertexes");
         n.New().create("Merge Faces"                   , MeshMergeFaces, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD                    )).desc("This option will merge 2 selected/highlighted faces if they share 2 vertexes");
         n.New().create("Rotate Quads"                  , MeshRotQuads  , T);
         n.New().create("Convert Quads To Tris"         , MeshQuadToTri , T);
         n.New().create("Tesselate"                     , MeshTesselate , T).kbsc(KbSc(KB_T, KBSC_CTRL_CMD|           KBSC_ALT)).desc("This option will smoothen the mesh, keeping original vertexes in place");
         n.New().create("Subdivide"                     , MeshSubdivide , T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD|           KBSC_ALT)).desc("This option will smoothen the mesh, repositioning original vertexes");
         n.New().create("Brighten Vertex Colors"        , MeshColorBrght, T).kbsc(KbSc(KB_C, KBSC_CTRL_CMD|KBSC_SHIFT         )).desc("This option will brighten vertex colors");
         n.New().create("Delete Double Side Faces"      , MeshDelDblSide, T).desc("This option will remove double sided faces");
         n++;
         n.New().create("Separate into 1 Object"        , MeshSeparate1, T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD|KBSC_SHIFT         )).desc("This option will separate selected parts into 1 new object");
         n.New().create("Separate into Multiple Objects", MeshSeparateN, T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_ALT)).desc("This option will separate selected parts into multiple new objects (1 per part)");
         n.New().create("Edit Selected"                 , MeshEditSel  , T).kbsc(KbSc(KB_E, KBSC_CTRL_CMD      )).flag(MENU_HIDDEN);
         n.New().create("VFS0"                          , MeshVFS0     , T).kbsc(KbSc(KB_1, KBSC_SHIFT|KBSC_ALT)).flag(MENU_HIDDEN);
         n.New().create("VFS1"                          , MeshVFS1     , T).kbsc(KbSc(KB_2, KBSC_SHIFT|KBSC_ALT)).flag(MENU_HIDDEN);
         n.New().create("VFS2"                          , MeshVFS2     , T).kbsc(KbSc(KB_3, KBSC_SHIFT|KBSC_ALT)).flag(MENU_HIDDEN);
         n.New().create("VFS3"                          , MeshVFS3     , T).kbsc(KbSc(KB_4, KBSC_SHIFT|KBSC_ALT)).flag(MENU_HIDDEN);
         n.New().create("MeshAO Preview"                , MeshAOClass::PreviewToggle, MeshAO).kbsc(KbSc(KB_P, KBSC_ALT)).flag(MENU_HIDDEN);
         n++;
         {
            Node<MenuElm> &rem=(n+="Remove");
            rem.New().create("Vertex TexCoord1"  , MeshRemVtxTex1 , T);
            rem.New().create("Vertex TexCoord2"  , MeshRemVtxTex2 , T);
            rem.New().create("Vertex TexCoord1&2", MeshRemVtxTex12, T);
            rem.New().create("Vertex Color"      , MeshRemVtxColor, T);
            rem.New().create("Vertex Skin"       , MeshRemVtxSkin , T);
         }
         mode.tab(MESH)+=mesh_ops.create(Rect_LU(vtx_face_sel_mode.rect().max.x+h, mode.rect().min.y-0.01f, 0.25f, 0.055f), n).focusable(false); mesh_ops.text="Operations"; mesh_ops.flag|=COMBOBOX_CONST_TEXT;
      }

      // SKIN
      // keep to the left because LOD tabs may be visible
      mode.tab(SKIN)+=skin_tabs.create(Rect_LU(mesh_undo.rect().ld()-Vec2(0, 0.01f), 0.33f, 0.055f*3), 0, skin_mode_t, Elms(skin_mode_t), true).layout(TABS_VERTICAL).valid(true).set(SKIN_SEL_BONE);
      REPA(skin_mode_desc)skin_tabs.tab(i).desc(skin_mode_desc[i]);
      {
         Node<MenuElm> n;
         n.New().create("Assign fully to bone"               , MeshSkinFull , T).kbsc(KbSc(KB_F, KBSC_CTRL_CMD              )).desc("This option will assign selected mesh parts fully to selected bone");
         n.New().create("Assign fully to bone and its parent", MeshSkinFullP, T).kbsc(KbSc(KB_F, KBSC_CTRL_CMD|KBSC_WIN_CTRL)).desc("This option will assign selected mesh parts fully to selected bone and its parent (half-half)");
         n.New().create("Unassign fully from bone"           , MeshSkinFullU, T).kbsc(KbSc(KB_F, KBSC_CTRL_CMD|KBSC_ALT     )).desc("This option will unassign selected mesh parts fully from selected bone"); // Ctrl+Shift+F is reserved for project find
         n.New().create("Auto Skin"                          , MeshSkinAuto , T).kbsc(KbSc(KB_A, KBSC_CTRL_CMD|KBSC_ALT     )).desc("This option will automatically set skinning for selected bone, or entire skeleton if no bone is selected");
         n.New().create("Edit Selected"                      , MeshEditSel  , T).kbsc(KbSc(KB_E, KBSC_CTRL_CMD              )).flag(MENU_HIDDEN);
         mode.tab(SKIN)+=skin_ops.create(Rect_LU(skin_tabs.rect().max.x+h, mode.rect().min.y-0.01f, 0.25f, 0.055f), n); skin_ops.text="Operations"; skin_ops.flag|=COMBOBOX_CONST_TEXT;
      }
      skin_brush.create(skin_tabs.tab(SKIN_CHANGE_SKIN), Vec2(clientWidth(), -clientHeight()));
      skin_brush.bsize  .del();
      skin_brush.image  .del();
      skin_brush.slope_b.del();
   }
   void ObjView::resize()
{
      ::Viewport4Region::resize();
      flt h=0.05f;
      lod.resize();
      leaf.move(Vec2(rect().w(), rect().h()/-2)-leaf.rect().right());
      group.move(Vec2(0, rect().h()/-2)-group.rect().left());
      param_edit.move(Vec2(rect().w(), mode.rect().min.y-0.01f)-param_edit.rect().ru());
      adjust_bone_orns.move(Vec2(rect().w(), rect().h()/-2)-adjust_bone_orns.rect().right());
   }
   void ObjView::setShader() {mesh.setShader();}
   void ObjView::flushObj()
   {
      if(obj_elm && (changed_obj || param_edit.changed))
      {
         ObjectPtr   old_obj    =Proj.gamePath(obj_id); // get data of the previous version from the file
         TerrainObj2 old_terrain=*old_obj;
         PhysPath    old_phys   =*old_obj;
         Object      obj;
         ElmObj    * obj_data=( obj_elm ?  obj_elm-> objData() : null);
         ElmMesh   *mesh_data=(mesh_elm ? mesh_elm->meshData() : null);
         
         // set mesh/phys only if they're not empty
         bool override_mesh=( obj_data &&  obj_data->mesh_id.valid() && OverrideMeshSkel(&mesh, mesh_skel)),
              override_phys=(mesh_data && mesh_data->phys_id.valid() && OverridePhys    (phys()));

         param_edit.p->copyTo(obj, Proj, true, override_mesh ? &obj_data->mesh_id : null, override_phys ? &mesh_data->phys_id : null);
         if(obj_data){obj_data->newVer(); obj_data->from(*param_edit.p);} // modify just before saving/sending in case we've received data from server after edit
         Save(*param_edit.p, Proj.editPath(obj_id)); // save edit
         Save( obj         , Proj.gamePath(obj_id)); Proj.savedGame(*obj_elm); // save game
         Proj.objChanged(*obj_elm); // call 'Proj.objChanged' instead of 'Proj.elmChanged' so 'ObjEdit.elmChanged' won't get called
         if(old_terrain!=TerrainObj2(obj))Proj.rebuildEmbedForObj(obj_id      ); // if saving changed 'terrainObj'
         if(old_phys   !=PhysPath   (obj))Proj.rebuildPathsForObj(obj_id, true); // if saving changed 'physPath', rebuild only for objects that don't override paths (if they override then it means that changing the base doesn't affect their path mode), we must rebuild this also for objects with final path mode set to ignore, in case we've just disabled paths
         Server.setElmLong(obj_elm->id);
      }
      changed_obj=param_edit.changed=false;
   }
   void ObjView::flushMeshSkel(SAVE_MODE save_mode)
   {
      if(save_mode!=SAVE_AUTO) // can't flush the skeleton for auto-save because if we're in the middle of editing it, and accidentally removed a bone, then auto-save would trigger animation adjustment and remove that bone animations
      {
         if(save_mode==SAVE_DEFAULT)trans_mesh.apply();

         // flush skeleton first because: 1) when flushing mesh we need skeleton to be already available 2) when sent to server/other clients they will receive skel first, and mesh later
         if(skel_elm && changed_skel && mesh_skel)
         {
            if(ElmSkel *skel_data=skel_elm->skelData())
            {
               skel_data->newVer(); // modify just before saving/sending in case we've received data from server after edit
               skel_data->file_time=skel_file_time; // we're operating on temporary memory 'mesh_skel' (not on file) so adjust modification time only at saving (so background send will not use different time+file)

               // transform 'saved_skel' into the same matrix as 'mesh_skel'
               Matrix skel_matrix=skel_data->transform();
               saved_skel.transform(GetTransform(saved_skel_matrix, skel_matrix));
               saved_skel_matrix=skel_matrix;
            }
            Save( edit_skel, Proj.editPath(skel_elm->id));
            Save(*mesh_skel, Proj.gamePath(skel_elm->id)); Proj.savedGame(*skel_elm);
            Server.setElmLong(skel_elm->id);

            Mems<Mems<IndexWeight> > bone_weights; cur_skel_to_saved_skel.set(bone_weights, saved_skel, *mesh_skel, EditSkeleton::BONE_NAME_IS_NODE_NAME); // 'cur_skel_to_saved_skel' is always created from 'saved_skel' so node name matches bone name
            int old_bone_as_root=-1; if(InRange(edit_skel.root, edit_skel.nodes)) // if we want root
            {
             C EditSkeleton::Node &new_root=edit_skel.nodes[edit_skel.root];
               if(!InRange(saved_edit_skel.root, saved_edit_skel.nodes) || saved_edit_skel.nodes[saved_edit_skel.root].name!=new_root.name) // saved didn't have root or had different one
               {
                  int old_root_i=saved_edit_skel.findNodeI(new_root.name); // find same root in old skeleton
                  if( old_root_i>=0)FREPA(saved_edit_skel.bones) // iterate all old bones, it's important to start from 0 to process parents first, so we will stop on the first parent linked to 'old_root_i'
                  {
                   C EditSkeleton::Bone &saved_bone=saved_edit_skel.bones[i];
                     REPA(saved_bone)if(saved_bone[i].index==old_root_i) // if found any bone linked to 'old_root_i'
                     {
                        if(saved_bone.elms()==1)old_bone_as_root=saved_skel.findBoneI(saved_bone.name); // we can accept it as root only if it has only 1 link
                        goto old_bone_as_root_set; // always stop after first bone was found, because even if it didn't meet the criteria, then we still can't accept children
                     }
                  }
               }
            }
         old_bone_as_root_set:
            Proj.adjustAnimations(skel_elm->id, saved_edit_skel, saved_skel, *mesh_skel, bone_weights, old_bone_as_root);
            saved_skel=*mesh_skel; saved_edit_skel=edit_skel; cur_skel_to_saved_skel.create(saved_skel); // we've saved, so 'saved_skel' should now be set to 'mesh_skel'
            REPAO(mesh_undos).can_use_cur_skel_to_saved_skel=false;

            Proj.skelChanged(*skel_elm); // call after skeleton was already saved, call 'Proj.skelChanged' instead of 'Proj.elmChanged' so 'ObjEdit.elmChanged' won't get called
         }
         changed_skel=false;

         // flush mesh second
         if(mesh_elm && changed_mesh)
         {
            ElmMesh *mesh_data = mesh_elm->meshData();
            Enum    *draw_group=(mesh_data ? Proj.getEnum(mesh_data->draw_group_id) : null);
            // save mesh
            Mesh edit; edit.create(mesh).keepOnly(EditMeshFlagAnd).delRender().drawGroupEnum(draw_group); Save(edit, Proj.editPath(mesh_elm->id), Proj.game_path); // edit
            Mesh game; EditToGameMesh(edit, game, body_skel, draw_group, &mesh_matrix);                   Save(game, Proj.gamePath(mesh_elm->id)); Proj.savedGame(*mesh_elm); // game
            bool changed_mesh_file=false;
            if(mesh_data)
            {
               if(Sync(mesh_data->file_time, mesh_file_time))changed_mesh_file=true; // we're operating on temporary memory 'mesh' (not on file) so adjust modification time only at saving (so background send will not use different time+file)
               mesh_data->newVer(); mesh_data->from(game); // modify just before saving/sending in case we've received data from server after edit
            }
            Proj.meshChanged(*mesh_elm); // call 'Proj.meshChanged' instead of 'Proj.elmChanged' so 'ObjEdit.elmChanged' won't get called
            if(changed_mesh_file)Server.setElmLong(mesh_elm->id);else Server.setElmShort(mesh_elm->id);
         }
         changed_mesh=false;
      }
   }
   void ObjView::flushPhys()
   {
      if(phys_elm && changed_phys && phys)
      {
         Save(*phys, Proj.gamePath(phys_elm->id)); Proj.savedGame(*phys_elm);
         if(ElmPhys *ep=phys_elm->physData())
         {
            ep->file_time.getUTC(); // we're operating on temporary memory 'phys' (not on file) so adjust modification time only at saving (so background send will not use different time+file)
            ep->newVer(); ep->from(*phys); // modify just before saving/sending in case we've received data from server after edit
         }
         Proj.physChanged(*phys_elm); // call 'Proj.physChanged' instead of 'Proj.elmChanged' so 'ObjEdit.elmChanged' won't get called
         Server.setElmLong(phys_elm->id);
      }
      changed_phys=false;
   }
   void ObjView::flush(SAVE_MODE save_mode)
   {
      flushPhys(); flushMeshSkel(save_mode); flushObj(); // flush starting from base-most element because parents rely on them existing
   }
   void ObjView::setChangedObj(                        ) {if( obj_elm){changed_obj =true; if(ElmObj  * obj_data= obj_elm-> objData()) obj_data->newVer();}}
   void ObjView::setChangedMesh(bool file, bool box) {if(mesh_elm){changed_mesh=true; if(ElmMesh *mesh_data=mesh_elm->meshData())mesh_data->fromMtrl(mesh); if(file){mesh_file_time.getUTC(); mesh_parts.refresh(); mesh_variations.refresh();}} if(box)setBox();}
   void ObjView::setChangedSkel(bool bone               ) {if(skel_elm){changed_skel=true; skel_file_time.getUTC(); if(bone && mesh_skel)mesh_skel->setBoneShapes(); if(bone){toGuiSkel(); adjust_bone_orns.refresh();}}}
   void ObjView::setChangedPhys(                        ) {if(phys_elm){changed_phys=true; if(ElmPhys *ep=phys_elm->physData()){ep->newVer(); ep->file_time.getUTC();}}}
   void ObjView::setBox(                        ) {MeshBase temp; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))temp+=mesh.parts[i]; if(!temp.transform(mesh_matrix).getBox(mesh_box))mesh_box.zero();}
   void ObjView::gotoPhysMtrl()
   {
      if(phys_elm)if(ElmPhys *phys_data=phys_elm->physData())if(Elm *phys_mtrl=Proj.findElm(phys_data->mtrl_id))PhysMtrlEdit.set(phys_mtrl);
   }
   void ObjView::gotoBody()
   {
      if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())if(Elm *body_obj=Proj.meshToObjElm(mesh_data->body_id))set(body_obj);
   }
   void ObjView::gotoDrawGroupEnum()
   {
      if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())if(Elm *draw_group=Proj.findElm(mesh_data->draw_group_id))EnumEdit.set(draw_group);
   }
   void ObjView::setPhysMtrl(C UID &mtrl_id)
   {
      Elm *mtrl_elm=Proj.findElm(mtrl_id, ELM_PHYS_MTRL);
      if( !mtrl_id.valid() || mtrl_elm)if(phys_elm)if(ElmPhys *phys_data=phys_elm->physData())if(phys_data->mtrl_id!=mtrl_id)
      {
         phys_undos.set("mtrl");
         phys_data->mtrl_id=mtrl_id;
         phys_data->mtrl_time.getUTC();
         if(phys)
         {
            PhysMtrl *mtrl=PhysMtrls(Proj.gamePath(mtrl_id));
            {CacheLock cl(PhysBodies); phys->material=mtrl;}
         }
         setChangedPhys();
      }
   }
   void ObjView::setBody(C UID &body_id)
   {
      if(mesh_elm)
      {
       C UID &actual_body_id=((mesh_elm->id==body_id) ? UIDZero : body_id); // if trying to set body to self, then clear it instead
         Elm *body=Proj.findElm(actual_body_id, ELM_MESH);
         if( !actual_body_id.valid() || body)if(ElmMesh *mesh_data=mesh_elm->meshData())if(mesh_data->body_id!=actual_body_id)
         {
            mesh_undos.set("body");
            mesh_data->body_id=actual_body_id;
            mesh_data->body_time.getUTC();
            Proj.getMeshSkels(mesh_data, null, &body_skel);
            setChangedMesh(false, false);

            // transform to body mesh
            Proj.meshTransformChanged(*mesh_elm, true);
            mesh_matrix=mesh_data->transform(); setBox();
            setPhysPartMatrix();
         }
      }
   }
   void ObjView::setDrawGroupEnum(C UID &enum_id)
   {
      Elm *enum_elm=Proj.findElm(enum_id, ELM_ENUM);
      if( !enum_id.valid() || enum_elm)if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())if(mesh_data->draw_group_id!=enum_id)
      {
         mesh_undos.set("drawGroup");
         mesh_data->draw_group_id=enum_id;
         mesh_data->draw_group_time.getUTC();
         setChangedMesh(false, false);
      }
   }
   void ObjView::animTargetObj()
   {
      if(obj_elm)
      {
         UID skel_id=Proj.objToSkel(obj_elm);
         if( skel_id.valid())Proj.animSetTargetSkel(menu_ids, skel_id);
         else                Gui.msgBox(S, "Can't set target object because it doesn't have a mesh skeleton.");
      }
   }
   void ObjView::resetTransform()
   {
      Matrix transform=~mesh_matrix;
      if(mesh_elm)
         if(ElmMesh *mesh_data=mesh_elm->meshData())
      {
         if(!mesh_data->canHaveCustomTransform())return;

         mesh_matrix.identity();
         mesh_data->transform.reset(); mesh_data->transform_time.now();
         setChangedMesh(false, false);
         Mesh game; EditToGameMesh(mesh, game, body_skel, Proj.getEnum(mesh_data->draw_group_id), &mesh_matrix); Saved(game, Proj.gamePath(mesh_elm->id)); // here 'Saved' is used on purpose instead of 'Save', to copy mesh to cache, actual save will happen later
         mesh_box=game.ext; // we can set from 'game' because it will have parts removed and correct transform set
         mesh_data->from(game);
         Proj.meshTransformChanged(*mesh_elm);
      }
      phys_part_matrix*=transform;
      mesh_skel_temp  *=transform; // transform the temp copy
   }
   void ObjView::setTransform(C Matrix &matrix)
   {
      Matrix transform=GetTransform(mesh_matrix, matrix);
      if(mesh_elm)
         if(ElmMesh *mesh_data=mesh_elm->meshData())
      {
         if(!mesh_data->canHaveCustomTransform())return;

         mesh_matrix=matrix;
         mesh_data->transform=mesh_matrix; mesh_data->transform_time.now();
         setChangedMesh(false, false);
         Mesh game; EditToGameMesh(mesh, game, body_skel, Proj.getEnum(mesh_data->draw_group_id), &mesh_matrix); Saved(game, Proj.gamePath(mesh_elm->id)); // here 'Saved' is used on purpose instead of 'Save', to copy mesh to cache, actual save will happen later
         mesh_box=game.ext; // we can set from 'game' because it will have parts removed and correct transform set
         mesh_data->from(game);
         Proj.meshTransformChanged(*mesh_elm);
      }
      phys_part_matrix*=transform;
      mesh_skel_temp  *=transform; // transform the temp copy
   }
   void ObjView::applyTransform(C Matrix &matrix)
   {
      if(mesh_elm)
         if(ElmMesh *mesh_data=mesh_elm->meshData())
      {
         if(!mesh_data->canHaveCustomTransform())return;

         mesh_matrix*=matrix;
         mesh_data->transform=mesh_matrix; mesh_data->transform_time.now();
         setChangedMesh(false, false);
         Mesh game; EditToGameMesh(mesh, game, body_skel, Proj.getEnum(mesh_data->draw_group_id), &mesh_matrix); Saved(game, Proj.gamePath(mesh_elm->id)); // here 'Saved' is used on purpose instead of 'Save', to copy mesh to cache, actual save will happen later
         mesh_box=game.ext; // we can set from 'game' because it will have parts removed and correct transform set
         mesh_data->from(game);
         Proj.meshTransformChanged(*mesh_elm);
      }
      phys_part_matrix*=matrix;
      mesh_skel_temp  *=matrix; // transform the temp copy
   }
   void ObjView::skinChanged()
   {
      mesh_parts.skinChanged();
   }
   void ObjView::selectedChanged()
   {
      setMenu();
      flush();
   }
   bool ObjView::selectionZoom(flt &dist)
   {
      flt size=mesh_box.size().avg();
      if( size>0)
      {
         dist=size/Tan(v4.perspFov()/2);
         return true;
      }
      return false;
   }
   void ObjView::camCenter(bool zoom)
{
      Vec hit_pos; bool hit=(getPart(Gui.ms(), Ms.pos(), &hit_pos)>=0); flt dist;
      v4.moveTo(hit ? hit_pos : selMeshCenter()); if(zoom && selectionZoom(dist))v4.dist(dist);
   }
   void ObjView::reloadObj()
   {
    //changed_obj=param_edit.changed=false; don't use for same reason as explained for 'changed_mesh'
      if(obj_elm)param_edit.p->load(Proj.editPath(obj_id));else param_edit.p->del();
      param_edit.toGui();
      Adjust(particles.del(), *param_edit.p, MatrixIdentity, Proj);
   }
   void ObjView::syncObj()
   {
      EditObject temp; if(temp.load(Proj.editPath(obj_id)))if(param_edit.p->sync(temp, Proj.edit_path))
      {
         if(!param_edit.param_window.contains(Gui.kb()))param_edit.toGui(); // if we're editing param, then don't refresh gui
      }
   }
   void ObjView::reloadMeshSkel()
   {
      mesh_undos.del(); undoVis(); // for now disable undos because of the difficulty with mapping between old skeleton and new skeleton in order to adjust animations
    //changed_mesh=changed_skel=false; this can't be cleared because for example 'changed_mesh' is set when changing body (skeleton) and after making that change, when using 'reload' on the object would trigger 'reloadMeshSkel' and body change would not be saved
                                                                     ElmObj  * obj_data=( obj_elm ?  obj_elm-> objData() : null);
      mesh_elm=( obj_data ? Proj.findElm( obj_data->mesh_id) : null); ElmMesh *mesh_data=(mesh_elm ? mesh_elm->meshData() : null);
      skel_elm=(mesh_data ? Proj.findElm(mesh_data->skel_id) : null); ElmSkel *skel_data=(skel_elm ? skel_elm->skelData() : null);
      if(mesh_data)mesh_file_time=mesh_data->file_time;else mesh_file_time.zero();
      if(skel_data)skel_file_time=skel_data->file_time;else skel_file_time.zero();

      Proj.getMeshSkels(mesh_data, &mesh_skel, &body_skel);
      if(mesh_skel)
      {
          mesh_skel_temp=*mesh_skel; mesh_skel=&mesh_skel_temp;
         saved_skel=*mesh_skel;
      }else
      {
          mesh_skel_temp.del();
         saved_skel.del();
      }
      if(skel_elm)edit_skel.load(Proj.editPath(skel_elm->id));else edit_skel.del(); saved_edit_skel=edit_skel;
      cur_skel_to_saved_skel.create(saved_skel);

      if(mesh_elm)
      {
         Load(mesh, Proj.editPath(mesh_elm->id), Proj.game_path);
         mesh.skeleton(mesh_skel).skeleton(null) // adjust mapping to skeleton in case that one got changed in the meantime
             .setTangents().setBinormals().setRender();
         mesh_ptr=Proj.gamePath(mesh_elm->id);
      }else
      {
         mesh.del(); mesh_ptr=null;
      }

      if(mesh_data)mesh_matrix=mesh_data->transform();else mesh_matrix.identity();
      if(skel_data)saved_skel_matrix=skel_data->transform();else saved_skel_matrix=mesh_matrix;
      if(mesh_data && mesh_data->body_id.valid() && (mode()==TRANSFORM || mode()==SLOTS /*|| mode()==BONES*/ || mode()==RAGDOLL))mode.set(-1); // disable unavailable modes if it's a cloth object
      setBox();
      lod.toGui();
      toGuiSkel();
      mesh_parts      .refresh();
      mesh_variations .refresh();
      adjust_bone_orns.refresh();
      meshVariationChanged();
   }
   void ObjView::reloadPhys()
   {
    //changed_phys=false; don't use for same reason as explained for 'changed_mesh'
      ElmMesh *mesh_data=(mesh_elm ? mesh_elm->meshData() : null);
      phys_elm=(mesh_data ? Proj.findElm(mesh_data->phys_id) : null);
      if(phys_elm)phys=Proj.gamePath(phys_elm->id);else phys=null;
      toGuiPhys();
   }
   void ObjView::reload()
   {
      reloadObj     ();
      reloadMeshSkel();
      reloadPhys    (); // after mesh
   }
   Str ObjView::nodeDisplayName(int node_i)C
   {
      Str name;
      if(C EditSkeleton::Node *node=edit_skel.nodes.addr(node_i))name+=node->name;
      FREPA(edit_skel.bones) // go from the start to find the first parent linked with this node
      {
       C EditSkeleton::Bone &bone=edit_skel.bones[i];
         FREPA(bone)if(bone[i].index==node_i) // if bone is linked to this node
         {
            if(bone.elms()==1 && name!=bone.name)name+=S+" ("+bone.name+")"; // add bone name only if it's a direct link and is different than node name
            goto finish; // always stop
         }
      }
   finish:
      return name;
   }
   void ObjView::addBoneRootData(int parent, int max_depth)
   {
      FREPA(edit_skel.nodes) // add in order
      {
       C EditSkeleton::Node &node=edit_skel.nodes[i]; if(node.parent==parent)
         {
            BoneRoot &bone_root=bone_root_data.New();
            bone_root.display=nodeDisplayName(i);
            bone_root.node.set(i, edit_skel);
         }
      }
      if(--max_depth>0)FREPA(edit_skel.nodes)if(edit_skel.nodes[i].parent==parent)addBoneRootData(i, max_depth); // add in order
   }
   void ObjView::setBoneRootTextSize()
   {
      bone_root.text_size=1; flt text_size, text_padd; if(bone_root.textParams(text_size, text_padd))
      {
         flt text_width=bone_root.textWidth(), rect_width=bone_root.rect().w()-text_padd*2; if(text_width>rect_width)bone_root.text_size=rect_width/text_width;
      }
   }
   void ObjView::toGuiSkel()
   {
      bone_root_data.clear();
      bone_root_data.New().display=NullName;
      addBoneRootData(-1, 3);
      bone_root.setData(bone_root_data);
      if(InRange(edit_skel.root, edit_skel.nodes))bone_root.setText(nodeDisplayName(edit_skel.root), true, QUIET);
      else                                        bone_root.set    (0                                    , QUIET);
      setBoneRootTextSize();
   }
   void ObjView::toGuiPhys()
   {
      REPAO(phys_props).toGui();
   }
   void ObjView::toGui()
   {
      toGuiPhys();
   }
   void ObjView::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_OBJ)elm=null;
      if(obj_elm!=elm)
      {
         Gui.closeMsgBox(del_root_bone_dialog_id);

         flush(); // flush previous data

         param_edit.undos.del(); param_edit.undoVis();
         mesh_undos.del(); phys_undos.del(); undoVis();

         // clear helpers
         if(mesh_skel)
         {
            sel_bone_name=(InRange(sel_bone, mesh_skel->bones) ? mesh_skel->bones[sel_bone].name : null);
            sel_slot_name=(InRange(sel_slot, mesh_skel->slots) ? mesh_skel->slots[sel_slot].name : null);
         }
         setPhys(-1);
         slot_meshes.clear();
         litSelVFClear();

         // setup elements
         obj_elm=elm; obj_id=(obj_elm ? obj_elm->id : UIDZero);
         reload();

         adjust_bone_orns.hide();

         sel_bone=(mesh_skel ? mesh_skel->findBoneI(sel_bone_name) : -1); sel_bone_vis=boneToVis(sel_bone);
         sel_slot=(mesh_skel ? mesh_skel->findSlotI(sel_slot_name) : -1);

         Proj.refresh(false, false);
         Mode.tabAvailable(MODE_OBJ, obj_elm!=null);
         mesh_parts     .newMesh();
         mesh_variations.newMesh();
         RenameSlot.hide();
         RenameBone.hide();
             NewLod.hide();
             MeshAO.hide();
      }
   }
   void ObjView::activate(Elm *elm)
   {
      set(elm); if(T.obj_elm){Mode.set(MODE_OBJ); HideBig();}
   }
   void ObjView::toggle(Elm *elm)
   {
      if(elm==T.obj_elm && selected())elm=null;
      activate(elm);
   }
   void ObjView::resetDrawGroupEnum()
   {
      if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())mesh.drawGroupEnum(Proj.getEnum(mesh_data->draw_group_id));
   }
   void ObjView::enumChanged(C UID &enum_id)
   {
      param_edit.enumChanged();
      if(enum_id.valid() && mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())if(enum_id==mesh_data->draw_group_id)resetDrawGroupEnum();
   }
   void ObjView::meshVariationChanged() {param_edit.meshVariationChanged();}
   void ObjView::elmChanged(C UID &elm_id)
   {
      if(obj_elm && obj_elm->id==elm_id)
      {
         syncObj();
         if(!changed_mesh && !changed_skel)
         {
            ElmObj *obj_data=(obj_elm ? obj_elm->objData() : null);
            if(mesh_elm!=(obj_data ? Proj.findElm(obj_data->mesh_id) : null)) // if new obj.mesh pointer
               reloadMeshSkel();
         }
         if(!changed_phys)reloadPhys();
      }
      if(mesh_elm && mesh_elm->id==elm_id)
      {
         ElmMesh *mesh_data=(mesh_elm ? mesh_elm->meshData() : null);
         if(!changed_mesh && !changed_skel || mesh_data->file_time>mesh_file_time) // if didn't change anything or new file time is newer than last edited file time
            reloadMeshSkel();
         if(!changed_phys)reloadPhys();
      }
      if(skel_elm && skel_elm->id==elm_id)
      {
         ElmSkel *skel_data=(skel_elm ? skel_elm->skelData() : null);
         if(!changed_mesh && !changed_skel || skel_data->file_time>skel_file_time) // if didn't change anything or new file time is newer than last edited file time
            reloadMeshSkel();
      }
      if(phys_elm && phys_elm->id==elm_id)
      {
         toGuiPhys();
      }
   }
   void ObjView::meshChanged()
   {
      if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())
      {
         Matrix matrix=mesh_data->transform(), transform=GetTransform(mesh_matrix, matrix);
         mesh_matrix=matrix;
         mesh_skel_temp*=transform; // transform skeleton too because it's expected to be in 'mesh_matrix'
      }
      trans_mesh.setAnchorPos();
      setBox();
   }
   void ObjView::skelTransformChanged()
   {
      adjust_bone_orns.refresh(); // skeleton is already transformed in 'meshChanged'
   }
   void ObjView::erasing(C UID &elm_id)
   {
      if( obj_elm &&  obj_elm->id==elm_id)set(null);
      if(mesh_elm && mesh_elm->id==elm_id)set(null);
      if(skel_elm && skel_elm->id==elm_id)set(null);
      if(phys_elm && phys_elm->id==elm_id)set(null);
   }
   Elm* ObjView::getMeshElm()
   {
      if(!mesh_elm) // if doesn't exist yet
         if(mesh_elm=Proj.getObjMeshElm(obj_id)) // if was just created
            mesh_ptr=Proj.gamePath(mesh_elm->id);
      return mesh_elm;
   }
   Elm* ObjView::getSkelElm()
   {
      if(!skel_elm) // if doesn't exist yet
         if(Elm *mesh_elm=getMeshElm()) // we need to have a mesh to have a skeleton
            if(skel_elm=Proj.getObjSkelElm(obj_id)) // if was just created
      {
         Proj.getMeshSkels(mesh_elm->meshData(), &mesh_skel, &body_skel);
         if(mesh_skel)mesh_skel=&mesh_skel_temp.del();
         setChangedMesh(false, false); // set mesh as changed so it will be resaved with the new skeleton
      }
      return skel_elm;
   }
   void ObjView::dragPhysMtrl(Memc<UID> &elms)
   {
      FREPA(elms)if(Elm *phys_mtrl=Proj.findElm(elms[i], ELM_PHYS_MTRL))
      {
         setPhysMtrl(phys_mtrl->id);
         break;
      }
   }
   void ObjView::dragDrawGroup(Memc<UID> &elms)
   {
      FREPA(elms)if(Elm *draw_group_enum=Proj.findElm(elms[i], ELM_ENUM))
      {
         setDrawGroupEnum(draw_group_enum->id);
         break;
      }
   }
   void ObjView::dragBody(Memc<UID> &elms)
   {
      FREPA(elms)if(Elm *obj=Proj.findElm(elms[i]))if(ElmObj *obj_data=obj->objData())
         if(obj_data->mesh_id.valid() && obj->id!=obj_id && obj_data->mesh_id!=mesh_elm->id) // don't apply self as body
      {
         setBody(obj_data->mesh_id);
         elms.clear();
         break;
      }
   }
   void ObjView::setAutoTanBin(C MaterialPtr &material)
   {
      bool changed=false;
      REPD(l, mesh.lods())
      {
         MeshLod &lod=mesh.lod(l); REPA(lod)
         {
            MeshPart &p=lod.parts[i]; if(!material || HasMaterial(p, material))
            {
               uint flag =p.flag(); p.setAutoTanBin();
               if(  flag!=p.flag())changed=true;
            }
         }
      }
      if(changed)setChangedMesh(true, false);
   }
   void ObjView::setMaterial(int part_i, C MaterialPtr &material)
   {
      MeshLod &lod=getLod();
      if(InRange(part_i, lod))
      {
         mesh_undos.set("mtrl");
         // set the material of other LOD's if they're the same (have same number of parts with same materials and names)
         int variation=visibleVariation();
         REPD(l, mesh.lods())
         {
            MeshLod &lod2=mesh.lod(l); if(&lod2!=&lod && lod.parts.elms()==lod2.parts.elms())
            {
               REPA(lod2)
               {
                C MeshPart &p=lod.parts[i], &p2=lod2.parts[i];
                  if(p.variation(variation)!=p2.variation(variation) || !Equal(p.name, p2.name))goto different;
               }
               MeshPart &part=lod2.parts[part_i];
               part.variations(Max(part.variations(), variation+1)) // make room first for specified variation
                   .variation (variation, material).setAutoTanBin();
            }
            different:;
         }
         MeshPart &part=lod.parts[part_i];
         part.variations(Max(part.variations(), variation+1)) // make room first for specified variation
             .variation (variation, material).setAutoTanBin();
         setChangedMesh(true, false);
         Proj.refresh(false, false); // refresh in case the mesh had invalid refs and now it won't
      }
   }
   void ObjView::drag(Memc<UID> &elms, GuiObj* &focus_obj, C Vec2 &screen_pos)
   {
      REPA(v4.view)if(focus_obj==&v4.view[i].viewport) // apply on model
      {
         MeshLod &lod=getLod();
         if(InRange(lit_part, lod)) // apply material on part
            FREPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_MTRL))
         {
            setMaterial(lit_part, Proj.gamePath(elm->id));
            break;
         }

         if(mode()==SLOTS && mesh_skel && InRange(lit_slot, mesh_skel->slots)) // put mesh in slot
            FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(ElmObj *obj_data=elm->objData())if(obj_data->mesh_id.valid())
         {
            MeshPtr mesh=Proj.gamePath(obj_data->mesh_id);
            putMeshToSlot(mesh, lit_slot);
            elms.clear(); // processed
            break;
         }

         if(mode()==PHYS  && phys_elm)dragPhysMtrl (elms);
         if(mode()==GROUP && mesh_elm)dragDrawGroup(elms);
         if(mode()==BODY  && mesh_elm)dragBody     (elms);

         if(mode()==BACKGROUND)
         {
            FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(ElmObj *obj_data=elm->objData())if(obj_data->mesh_id.valid())
            {
               back_meshes.New().mesh=Proj.gamePath(obj_data->mesh_id);
            }
         }else
         {
            bool mesh=false, anim=false;
            FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))switch(elm->type)
            {
               case ELM_OBJ : if(ElmObj *obj_data=elm->objData())mesh|=obj_data->mesh_id.valid(); break;
               case ELM_ANIM: anim=true; break;
            }
            Node<MenuElm> n;
            if(mesh)
            {
               n.New().create("Copy Mesh Parts here"                          , MeshCopyParts, T);
               n.New().create("Set opened Object as Body for selected Objects", SetBody      , T).desc("This option will set the \"Body\" of all selected objects to the Object that is currently opened in the Object Editor");
            }
            if(anim)n.New().create("Link Animations with this Object", AnimTargetObj, T);
            if(n.children.elms())
            {
               menu_ids=elms; Gui+=menu.create(n); menu.activate().posRU(Ms.pos());
            }
         }

         break;
      }

      if(focus_obj==&goto_phys_mtrl                         )dragPhysMtrl (elms);else
      if(focus_obj==&goto_body                              )dragBody     (elms);else
      if(focus_obj==&goto_group || group.contains(focus_obj))dragDrawGroup(elms);

      lod            .drag(elms, focus_obj, screen_pos);
      param_edit     .drag(elms, focus_obj, screen_pos);
      mesh_parts     .drag(elms, focus_obj, screen_pos);
      mesh_variations.drag(elms, focus_obj, screen_pos);
   }
   void ObjView::drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      lod.drop(names, focus_obj, screen_pos);
      if(obj_elm)REPA(v4.view)if(focus_obj==&v4.view[i].viewport)Importer.import(*obj_elm, names, screen_pos);
   }
   void ObjView::animate(C AnimSkel &anim_skel)
   {
      if(mesh_skel && skel_elm)
      {
         flushMeshSkel(); // save any unsaved changes, after this call 'saved_skel' is the same as 'mesh_skel'
         mesh_undos.del(); undoVis(); // we're modifying 'saved_skel' so we can't go back

         MemtN<Matrix, 256> matrixes; anim_skel.getMatrixes(matrixes); REPAO(matrixes)=mesh_matrix*matrixes[i]/mesh_matrix;
         mesh     .animate(matrixes ); // 'mesh'      is in identity matrix
         mesh_skel->animate(anim_skel); // 'mesh_skel' is in 'mesh_matrix'
         edit_skel.animate(anim_skel, matrixes); // need to modify 'edit_skel' so new animations will import correctly

         Proj.offsetAnimations(saved_skel, *mesh_skel, skel_elm->id); // we've transformed mesh skeleton, and since animations can transform bones on top of skeleton, then we need to offset them back

         // need to modify 'saved_skel' so existing animations won't be modified
         saved_skel=*mesh_skel; saved_edit_skel=edit_skel; saved_skel_matrix=mesh_matrix; cur_skel_to_saved_skel.create(saved_skel);
       //REPAO(mesh_undos).can_use_cur_skel_to_saved_skel=false; no need to do this because mapping doesn't change

         setChangedMesh(true);
         setChangedSkel(true);
      }
   }
   void ObjView::update(C GuiPC &gpc)
{
      has_cur_pos=false;
      lit_lod=lit_part=lit_vf_part=lit_vtx=lit_face=-1;
      lit_bone=lit_bone_vis=lit_slot=-1;
      lit_phys=-1;
      selLod(sel_lod);
      ::Viewport4Region::update(gpc);
      if(visible() && gpc.visible)
      {
         // get lit
         if(mode()==SLOTS)
         {
                    REPA(MT)if(MT.b(i)){getSkelSlot(MT.pos(i)); if(lit_bone>=0 || lit_slot>=0)break;}
            if(lit_bone<0 && lit_slot<0)getSkelSlot(Ms.pos( ));
         }else
         if((mode()==SKIN && skin_tabs()==SKIN_SEL_BONE) || mode()==BONES)
         {
            if(!Ms.b(1)) // don't highlight when operating (like transforming)
            {
               getSkel(Gui.msLit(), Ms.pos(), &lit_bone_vis, null); lit_bone=visToBone(lit_bone_vis);
            }
         }else
         if(mode()==RAGDOLL)
         {
            REPA(MT)if(MT.b(i))
            {
               getSkel(Gui.objAtPos(MT.pos(i)), MT.pos(i), &lit_bone_vis, null);
               if(lit_bone_vis>=0)break;
            }
            if(lit_bone_vis<0)getSkel(Gui.msLit(), Ms.pos(), &lit_bone_vis, null);
            lit_bone=visToBone(lit_bone_vis);
         }else
         if(mode()==PHYS && phys_tabs()!=PHYS_TOGGLE)
         {
            REPA(MT)if(MT.b(i))
            {
               lit_phys=getPhys(Gui.objAtPos(MT.pos(i)), MT.pos(i));
               if(lit_phys>=0)break;
            }
            if(lit_phys<0)lit_phys=getPhys(Gui.msLit(), Ms.pos());
         }else
         if(NewLod.visible())
         {
         }else
         if(MeshAO.visible())
         {
         }else
         if(mode()==BACKGROUND)
         {
         }else
         {
            if(editMeshParts())
            {
               if(!transMesh())
               {
                  bool ignore_vtxs=(sel_face.elms() || vtx_face_sel_mode() && !sel_vtx.elms()); // ignore vertexes if we have faces selected, or we're in multi-sel mode with no vtxs
                  lit_vf_part=getVtxFace(Gui.msLit(), Ms.pos(), ignore_vtxs ? null : &lit_vtx, sel_vtx.elms() ? null : &lit_face, &cur_pos); if(lit_vf_part>=0)has_cur_pos=true;
               }
            }else
            if(mode()!=SKIN || (mode()==SKIN && skin_tabs()==SKIN_SEL_MESH_PART))
            {
               REPA(MT)if(MT.b(i))
               {
                  lit_part=getPart(Gui.objAtPos(MT.pos(i)), MT.pos(i), &cur_pos); if(lit_part>=0){has_cur_pos=true; break;}
               }
               if(lit_part<0 && !Ms.b(1)) // don't highlight when operating (like transforming)
               {
                  lit_part=getPart(Gui.msLit(), Ms.pos(), &cur_pos); if(lit_part>=0)has_cur_pos=true;
               }
            }
            if(mode()==GROUP)
            {
               if(group.parts_l.lit>=0)lit_part=GroupRegion::VisiblePartI(getLod(), group.parts_l.lit);else group.highlight(lit_part);
            }else
            if(mode()==MESH || mode()==SKIN)
            {
               if(mesh_parts.list.lit>=0)lit_part=mesh_parts.visToPart(mesh_parts.list.lit);else mesh_parts.highlight(lit_part);
            }
         }
         if(show_cur_pos() && !has_cur_pos)has_cur_pos=(getPart(Gui.msLit(), Ms.pos(), &cur_pos)>=0);

         // update
         UID body_id=UIDZero, draw_group_id=UIDZero, phys_mtrl_id=UIDZero;
         if(mesh_elm)if(ElmMesh *mesh_data=mesh_elm->meshData())
         {
                  body_id=mesh_data->      body_id;
            draw_group_id=mesh_data->draw_group_id;
         }
         if(phys_elm)if(ElmPhys *phys_data=phys_elm->physData())phys_mtrl_id=phys_data->mtrl_id;
         goto_body     .text=Proj.elmFullName(      body_id);
         goto_group    .text=Proj.elmFullName(draw_group_id);
         goto_phys_mtrl.text=Proj.elmFullName( phys_mtrl_id);

         Adjust(particles, *param_edit.p, MatrixIdentity, Proj);
         if(!particles.update())particles.resetFull();
         updateMesh   ();
         updateSlots  ();
         updateBones  ();
         updateRagdoll();
         updatePhys   ();

         if(Ms.bd(0)) // maximize on double click
         {
            if(Gui.ms()==&param_edit.param_window)param_edit.param_window.maximize();
         }
         if(Ms.bp(2)) // close on middle click
         {
            if(trans_mesh             .contains(Gui.ms()))trans_mesh.close.push();else
            if(param_edit             .contains(Gui.ms())
            || param_edit.param_window.contains(Gui.ms())
            || trans                  .contains(Gui.ms())
            || group                  .contains(Gui.ms())
            || mesh_parts             .contains(Gui.ms())
            || mesh_variations        .contains(Gui.ms())
            )mode.set(-1);
         }
      }
   }
   void ObjView::Add(Str &s, cchar8 *t) {if(s.last()!=' ')s+=','; s.space()+=t;}
   void ObjView::drawInfo(C GuiPC &gpc)
   {
      if(gpc.visible)
      {
         D.clip(gpc.clip);

         Memt<Str> a, b, c;

       C MeshLod &lod=getDrawLod();
         if(VisibleQuads(lod))a.add(S+VisibleVtxs(lod)+" vtx"+CountS(VisibleVtxs(lod))+", "+VisibleTris     (lod)+" tri"+CountS(VisibleTris     (lod))+", "+VisibleQuads(lod)+" quad"+CountS(VisibleQuads(lod)));
         else                 a.add(S+VisibleVtxs(lod)+" vtx"+CountS(VisibleVtxs(lod))+", "+VisibleTrisTotal(lod)+" tri"+CountS(VisibleTrisTotal(lod)));
         int size=0; REP(mesh.lods())size+=VisibleSize(mesh.lod(i)); a.add(S+"Mesh size "+FileSize(size));
         if(mode()==LOD)a.add(S+mesh.lods()+" LOD"+CountS(mesh.lods()));
         if(mesh.is())
         {
            Str  s="Vertex Data: ";
            uint flag=mesh.flag();
            if(flag&VTX_POS     )Add(s, "Position"); if(flag&VTX_HLP )Add(s, "Helper"); if(flag&VTX_NRM)Add(s, "Normal");
          //if(flag&VTX_TAN     )Add(s, "Tangent" ); if(flag&VTX_BIN )Add(s, "Binormal"); ignore these as they're always set in the Editor
            if(flag&VTX_TEX0    )Add(s, "TexCoord"); if(flag&VTX_TEX1)Add(s, "TexCoord1"); if(flag&VTX_TEX2)Add(s, "TexCoord2");
            if(flag&VTX_COLOR   )Add(s, "Color");
            if(flag&VTX_MATERIAL)Add(s, "Material");
            if(flag&VTX_SKIN    )Add(s, "Skin");
            c.add(s);
         }
         if(mode()==PHYS && phys)a.add(S+phys->parts.elms()+" physical body part"+CountS(phys->parts.elms()));
         if(Skeleton *skel=getVisSkel())
         {
            if(mode()==SLOTS)a.add(S+skel->slots.elms()+" slot"+CountS(skel->slots.elms()));
            if(mode()==BONES)a.add(S+skel->bones.elms()+" bone"+CountS(skel->bones.elms()));
            if(InRange(lit_bone_vis, skel->bones))
            {
             C SkelBone &bone=skel->bones[lit_bone_vis];
               Str s=S+"Bone \""+bone.name+"\"";
               s+=S+", Parent: "+(InRange(bone.parent, skel->bones) ? S+'"'+skel->bones[bone.parent].name+'"' : S+"none");
               s+=S+", Children: "+bone.children_num;
               if(bone.type)
               {
                  s+=S+", Type: "+BoneName(bone.type);
                  bool index=false, sub=false;
                  REPA(skel->bones)
                  {
                   C SkelBone &test=skel->bones[i]; if(test.type==bone.type)
                     {
                        if(test.type_index)index=true;
                        if(test.type_sub  )sub  =true;
                     }
                  }
                  if(index || sub)
                  {
                            s.space()+=      bone.type_index;
                     if(sub)s        +=S+':'+bone.type_sub;
                  }
               }
               a.add(s);
            }
            if(InRange(lit_slot, skel->slots))
            {
             C SkelSlot &skel_slot=skel->slots[lit_slot];
               Str s=S+"Slot \""+skel_slot.name+"\", Bone Parent: "+(InRange(skel_slot.bone, skel->bones) ? S+'"'+skel->bones[skel_slot.bone].name+'"' : S+"none");
               if(skel_slot.bone!=skel_slot.bone1)s+=S+", Bone Parent1: "+(InRange(skel_slot.bone1, skel->bones) ? S+'"'+skel->bones[skel_slot.bone1].name+'"' : S+"none");
               a.add(s);
            }
         }
         if(show_cur_pos()){Str s="Cursor: "; if(has_cur_pos)s+=cur_pos.asText(4); a.add(s);}
         if(showMainBox())
         {
            Box box=mesh_box;
            a.add(S+"Min     ("+box.min     +')');
            a.add(S+"Max    ("+box.max     +')');
            a.add(S+"Center ("+box.center()+')');
            a.add(S+"Size    ("+box.size  ()+')');
            if(mode()==TRANSFORM)
            {
               box*=trans.matrix;
               b.add(S+"Min     ("+box.min     +')');
               b.add(S+"Max    ("+box.max     +')');
               b.add(S+"Center ("+box.center()+')');
               b.add(S+"Size    ("+box.size  ()+')');
            }
         }
         if(a.elms() || b.elms() || c.elms())
         {
            flt x=gpc.offset.x+0.01f, y=gpc.offset.y-0.25f;
            TextStyleParams ts(false); ts.size=0.045f; ts.align.set(1, -1);
            D.clip();
                             FREPA(a){D.text(ts, x, y, a[i]); y-=ts.size.y;}
            ts.color=YELLOW; FREPA(b){D.text(ts, x, y, b[i]); y-=ts.size.y;}
            if(c.elms())
            {
               if(!Proj.visible())x+=Misc.rect().w();
               y=-rect().h()+gpc.offset.y+ts.size.y*c.elms();
               ts.color=WHITE; FREPA(c){D.text(ts, x, y, c[i]); y-=ts.size.y;}
            }
         }
         if(showVtxSelCircle   ())vtxSelCircle   ().draw(LitColor, false);
         if(showChangeSkinShape())changeSkinShape().draw(LitColor, false);
      }
   }
   int ObjView::visibleVariation()C
   {
      if(mesh_variations.list.lit>=0)return mesh_variations.list.lit;
      if(variation_tabs.visibleFull())return selVariation();
      return param_edit.v_mesh_var();
   }
   int  ObjView::selVariation()C {return Mid(sel_variation, 0, mesh.variations()-1);}
   void ObjView::selVariation(int variation, SET_MODE mode)
   {
      if(variation!=sel_variation)
      {
         sel_variation=variation; variation_tabs.set(selVariation(), QUIET);
         if(mode!=QUIET)mesh_variations.list.setCur(selVariation());
      }
   }
   int  ObjView::selLod()C {return Mid(sel_lod, 0, mesh.lods()-1);}
   void ObjView::selLod(int lod)
   {
      if(sel_lod!=lod){sel_lod=lod; T.lod.toGui(); mesh_parts.refresh(); lod_tabs.set(selLod(), QUIET);}
      T.lod.edit_dist.visible(lod>0 && mesh.lods()>1);
   }
   void ObjView::eraseLOD(int i)
   {
      mesh_undos.set("lod");
      mesh.removeLod(i).setBox();
      setChangedMesh(true); lod.toGui();
   }
   void ObjView::disableLOD(int i)
   {
      mesh_undos.set("lod");
      if(InRange(i, mesh.lods()))CHSSB(mesh.lod(i).dist2);
      setChangedMesh(true); lod.toGui();
   }
   void ObjView::updateMesh()
   {
      // get trans_axis
      if(mode()==TRANSFORM){if(trans_tabs()!=TRANS_SCALE)MatrixAxis(v4, trans     .drawMatrix(), trans_axis);else trans_axis=-1;}else // SCALE doesn't support per-axis setting
      if(transMesh()      )                              MatrixAxis(v4, trans_mesh.drawMatrix(), trans_axis);else
         trans_axis=-1;

      // transform object
      if(mode()==TRANSFORM)if(Ms.b(1))if(Edit::Viewport4::View *view=v4.getView(Gui.ms()))
      {
         flt multiplier=(Kb.shift() ? 0.1f : 1)*0.5f;
         Ms.freeze();
         view->setViewportCamera();
         switch(trans_tabs())
         {
            case TRANS_MOVE:
            {
               if(trans.move_p[0])multiplier=trans.move_p[0]->mouse_edit_speed;
               Vec2 mul=multiplier*MoveScale(*view);
               switch(trans_axis)
               {
                  case  0: trans.trans.pos.x+=AlignDirToCamEx(    trans.matrix.x, Ms.d()*mul); break;
                  case  1: trans.trans.pos.y+=AlignDirToCamEx(    trans.matrix.y, Ms.d()*mul); break;
                  case  2: trans.trans.pos.z+=AlignDirToCamEx(    trans.matrix.z, Ms.d()*mul); break;
                  default: trans.trans.pos  +=               (ActiveCam.matrix.x*Ms.d().x*mul.x
                                                             +ActiveCam.matrix.y*Ms.d().y*mul.y)*~trans.matrix.orn(); break;
               }
            }break;

            case TRANS_ROT:
            {
               switch(trans_axis)
               {
                  case  0: trans.matrix.rotateXL(                     Ms.d().sum()*multiplier); break;
                  case  1: trans.matrix.rotateYL(                     Ms.d().sum()*multiplier); break;
                  case  2: trans.matrix.rotateZL(                     Ms.d().sum()*multiplier); break;
                  default: trans.matrix.rotate  (ActiveCam.matrix.z, -Ms.d().sum()*multiplier); break;
               }
               trans.trans.rot=     trans.matrix.angles();
               trans.trans.pos=Pose(trans.matrix).pos;
            }break;

            case TRANS_SCALE:
            {
             /*switch(trans_axis)
               {
                  case  0: trans.trans_scale.x*=ScaleFactor(AlignDirToCamEx(trans.matrix.x, Ms.d()*multiplier)); break;
                  case  1: trans.trans_scale.y*=ScaleFactor(AlignDirToCamEx(trans.matrix.y, Ms.d()*multiplier)); break;
                  case  2: trans.trans_scale.z*=ScaleFactor(AlignDirToCamEx(trans.matrix.z, Ms.d()*multiplier)); break;
                  default: */trans.trans.scale  *=ScaleFactor(                                Ms.d().sum()*multiplier); /*break;
               }*/
            }break;
         }
         trans.toGui();
      }

      // transform mesh
      if(mode()==MESH)if(Ms.b(1))if(Edit::Viewport4::View *view=v4.getView(Gui.ms()))
      {
         flt multiplier=(Kb.shift() ? 0.1f : 1)*0.5f;
         Ms.freeze();
         view->setViewportCamera();
         trans_mesh.activate();
         if(!Kb.alt ()) // don't transform when alt pressed, so we can hold alt at the start, to just show the GUI
         if( Kb.ctrl())
         {
            if(trans_mesh.move_p[0])multiplier=trans_mesh.move_p[0]->mouse_edit_speed;
            trans_mesh.trans_normal+=Ms.d().sum()*multiplier*posScale()*0.5f;
         }else
         switch(trans_tabs())
         {
            case TRANS_MOVE:
            {
               if(trans_mesh.move_p[0])multiplier=trans_mesh.move_p[0]->mouse_edit_speed;
               Vec2 mul=multiplier*MoveScale(*view);
               switch(trans_axis)
               {
                  case  0: trans_mesh.trans.pos+=AlignDirToCam(trans_mesh.matrix.x, Ms.d()  *mul); break;
                  case  1: trans_mesh.trans.pos+=AlignDirToCam(trans_mesh.matrix.y, Ms.d()  *mul); break;
                  case  2: trans_mesh.trans.pos+=AlignDirToCam(trans_mesh.matrix.z, Ms.d()  *mul); break;
                  default: trans_mesh.trans.pos+=               ActiveCam.matrix.x* Ms.d().x*mul.x
                                                               +ActiveCam.matrix.y* Ms.d().y*mul.y; break;
               }
            }break;

            case TRANS_ROT:
            {
               switch(trans_axis)
               {
                  case  0: trans_mesh.matrix.rotateXL(                     Ms.d().sum()*multiplier); break;
                  case  1: trans_mesh.matrix.rotateYL(                     Ms.d().sum()*multiplier); break;
                  case  2: trans_mesh.matrix.rotateZL(                     Ms.d().sum()*multiplier); break;
                  default: trans_mesh.matrix.rotate  (ActiveCam.matrix.z, -Ms.d().sum()*multiplier); break;
               }
               trans_mesh.trans.rot=trans_mesh.matrix.angles();
            }break;

            case TRANS_SCALE:
            {
               switch(trans_axis)
               {
                  case  0: trans_mesh.trans_scale.x*=ScaleFactor(AlignDirToCamEx(trans_mesh.matrix.x, Ms.d()*multiplier)); break;
                  case  1: trans_mesh.trans_scale.y*=ScaleFactor(AlignDirToCamEx(trans_mesh.matrix.y, Ms.d()*multiplier)); break;
                  case  2: trans_mesh.trans_scale.z*=ScaleFactor(AlignDirToCamEx(trans_mesh.matrix.z, Ms.d()*multiplier)); break;
                  default: trans_mesh.trans.scale  *=ScaleFactor(                                     Ms.d().sum()*multiplier); break;
               }
            }break;
         }
         trans_mesh.toGui();
      }

      if(showVtxSelCircle() && Ms.wheel())Clamp(vtx_sel_r*=ScaleFactor(Ms.wheel()*0.3f), 0.01f, D.h()*0.5f);

      // operate on mesh vtx faces
      if(editMeshParts())
      {
         if(Ms.b(0) && v4.getView(Gui.ms()))
         {
            if(transMesh())trans_mesh.close.push();else switch(mesh_parts.list.selMode())
            {
               case LSM_SET    : if(Ms.bp(0)){selUndo(); selVFDo();} break;
               case LSM_TOGGLE : if(Ms.bp(0)){selUndo(); selVFDo();} break;
               case LSM_INCLUDE:             {selUndo(); selVFDo(); selVFDo(true );} break;
               case LSM_EXCLUDE:             {selUndo(); selVFDo(); selVFDo(false);} break;
            }
         }
      }

      // operate on mesh parts
      if(mode()!=SLOTS && mode()!=RAGDOLL && mode()!=PHYS && mode()!=BONES)
         REPA(MT)if(MT.bp(i) && v4.getView(MT.guiObj(i)))
      {
         MeshLod &lod=getLod();
         int   part_i=getPart(MT.guiObj(i), MT.pos(i));
         if(OpObj==OP_OBJ_SET_MTRL && MtrlEdit.visible()) // set material
         {
            setMaterial(part_i, MtrlEdit.game);
         }else
         if(mode()==MESH)
         {
            if(!mesh_parts.edit_selected())
            {
               trans_mesh.apply();
               mesh_parts.clicked(part_i);
            }
         }else
         if(mode()==SKIN)
         {
            if(skin_tabs()==SKIN_SEL_MESH_PART)
            {
               mesh_parts.clicked(part_i);
            }else
            if(skin_tabs()==SKIN_SEL_BONE)
            {
               getSkel(MT.guiObj(i), MT.pos(i), &sel_bone_vis, null); sel_bone=visToBone(sel_bone_vis);
            }
         }else
         if(mode()==BACKGROUND)
         {
            
         }else
         {
            if(InRange(part_i, lod))
            {
               MeshPart &part=lod.parts[part_i];
               if(mode()==REMOVE) // toggle visibility
               {
                  mesh_undos.set("remove");
                  FlagToggle(part.part_flag, MSHP_HIDDEN);
                  mesh.setBox();
                  setChangedMesh(true);
               }else
               if(OpObj==OP_OBJ_SET_GROUP && group.getSetGroup()>=0)
               {
                  mesh_undos.set("drawGroup");
                  SetDrawGroup(mesh, lod, part_i, group.getSetGroup(), mesh.drawGroupEnum());
                  setChangedMesh(true, false);
               }else // get material
               {
                  Memt<MaterialPtr> mtrls;
                  if(int v=visibleVariation())if(C MaterialPtr &var=part.variation(v))mtrls.add(var); // first try selecting variation if it exists
                  if(!mtrls.elms())REP(4)mtrls.include(part.multiMaterial(i)); // if doesn't exist, then add multi materials
                  MtrlEdit.set(mtrls);
               }
            }
         }
      }

      // change skin
      if(mode()==SKIN && skin_tabs()==SKIN_CHANGE_SKIN)if(Ms.b(0) || Ms.b(1))if(Edit::Viewport4::View *view=v4.getView(Gui.ms()))
      {
         mesh_undos.set("skin", false, 1);
         const flt min_time=2.99f/60; // (3 fps) min amount of time needed to make a change
         flt time =Time.appTime()-edit_time;
         if( time>=min_time) // enough time passed
         {
            edit_time=Time.appTime();
            if(Ms.bp(0) || Ms.bp(1))time=min_time; // on first click don't change more than minimum
            const byte   bone=sel_bone+1, parent=(mesh_skel ? mesh_skel->boneParent(sel_bone)+1 : 0);
            const bool    add=Ms.b(1), // if adding weight
                      instant=Kb.alt();
            const byte adelta=(instant ? 255 : Min(Round(time*Lerp(32, 640-32, skin_brush.sspeed())), 255)); // 0..255
            if(adelta)
            {
               int         delta=adelta*SignBool(add), // -255 .. 255
                           delta_soft=(instant ? delta : Round(Lerp(delta, 0, skin_brush.ssoft()))); // softness=0 (hard) we have 'delta', softness=1 (soft) have '0'
               Shape       shape=changeSkinShape();
               Memt<VecI2> vtxs; getVtxs(Gui.ms(), shape, vtxs);
               Matrix      matrix=transformMatrix();
               MeshLod    &lod=getLod(); REPA(lod)if(mesh_parts.partOp(i))includeSamePos(i, vtxs);
               Memt<int, SIZE(int)*128> parts;
               REPA(vtxs)
               {
                C VecI2 &vtx=vtxs[i]; if(mesh_parts.partOp(vtx.x))if(MeshPart *part=getPart(vtx.x))
                  {
                     parts.binaryInclude(vtx.x, Compare);
                     MeshBase &base=part->base;
                     if(add) // if adding then create and setup vertex data if needed
                     {
                        if(!base.vtx.matrix()){base.include(VTX_MATRIX); REPA(base.vtx)base.vtx.matrix(i).zero();}
                        if(!base.vtx.blend ()){base.include(VTX_BLEND ); REPA(base.vtx)base.vtx.blend (i).set(255, 0, 0, 0);}
                     }
                     if(base.vtx.matrix() && base.vtx.blend())
                     {
                        int d; 
                        if(delta==delta_soft)d=delta;else // hard
                        {
                           // soft
                           Vec2 pos=PosToScreen(base.vtx.pos(vtx.y)*matrix);
                           flt  dist=0;
                           switch(shape.type)
                           {
                              case SHAPE_CIRCLE: dist=Dist(pos, shape.circle.pos)/shape.circle.r; break;
                              case SHAPE_RECT  : dist=(pos-shape.rect.center()).abs().max()/shape.rect.w()*2; break;
                           }
                           d=Round(Lerp(delta, delta_soft, dist)); // distance=0 (center) we have 'delta', distance=1 (border) we have 'delta_soft'
                        }

                        VecB4 &matrix=base.vtx.matrix(vtx.y);
                        VecB4 &blend =base.vtx.blend (vtx.y);
                        IndexWeight skin[3]; Int skins=0;
                        if(add)skin[skins++].set(bone, 0); // when adding, reserve room for 'bone' as it's required to be present
                        int sum=0; // actual weight of bones other than 'bone'
                        FREPA(blend)if(blend.c[i]) // start with most important bones
                        {
                           if(matrix.c[i]==bone)Clamp(d+=blend.c[i], 0, 255);else
                           {
                              sum+=blend.c[i];
                              if(InRange(skins, skin))skin[skins++].set(matrix.c[i], blend.c[i]);
                           }
                        }
                        int rest=255-d; // total weight of bones other than 'bone'
                        if(sum && rest!=sum){flt mul=flt(rest)/sum; REP(skins)skin[i].weight*=mul;} // rescale from 'sum' to 'rest'
                        if(add)skin[0].weight=d;else // if increasing, readjust the weight that may have changed during the loop
                        { // decreasing
                           if(!skins)skin[skins++].set(parent, rest); // if there are no other bones, then we need to set root bone
                           if(d && InRange(skins, skin))skin[skins++].set(bone, d); // if still have room for 'bone'
                        }
                        SetSkin(MemPtrN<IndexWeight, 256>(skin, skins), matrix, blend, mesh_skel);
                     }
                  }
               }
               REPA(parts)lod.parts[parts[i]].setRender();
               if(parts.elms())setChangedMesh(true, false);
            }
         }
      }

      // update LOD tabs
      if(mesh.lods()!=lod_tabs.tabs())
      {
         for(; lod_tabs.tabs()>mesh.lods(); )lod_tabs.remove(lod_tabs.tabs()-1);
         for(; lod_tabs.tabs()<mesh.lods(); )lod_tabs.New(S+"Lod "+lod_tabs.tabs());
         lod_tabs.rect(Rect_U(lod_tabs.rect().up(), lod_tabs.tabs()*0.13f, 0.055f));
         lod_tabs.set(selLod(), QUIET).visible(lod_tabs.tabs()>=2);
      }

      // update variation tabs
      if(mesh.variations()!=variation_tabs.tabs())
      {
         for(; variation_tabs.tabs()>mesh.variations(); )variation_tabs.remove(variation_tabs.tabs()-1);
         for(; variation_tabs.tabs()<mesh.variations(); )variation_tabs.New(S);
         variation_tabs.set(selVariation(), QUIET).visible(variation_tabs.tabs()>=2);
      }
      if(variation_tabs.visible())
      {
         flt w=0, h=0.055f;
         FREPA(variation_tabs)
         {
            Tab &tab=variation_tabs.tab(i); 
            tab.text(i ? mesh.variationName(i) : "Default");
            w+=tab.textWidth(&h)+h*0.5f;
         }
         variation_tabs.rect(Rect_U(variation_tabs.rect().up(), w, h), 0, true);
      }
   }
   void ObjView::SlotModeChanged(ObjView &editor) {}
   void ObjView::SlotCopy(ObjView &editor)
   {
      Skeleton &dest=Proj.slot_skel_mem; dest.del(); if(C Skeleton *src=editor.mesh_skel)
      {
         dest.bones=src->bones;
         if(InRange(editor.sel_slot, src->slots))dest.slots.New()=src->slots[editor.sel_slot];else dest.slots=src->slots; // copy selected or all
      }
   }
   void ObjView::SlotPaste(ObjView &editor)
   {
      if(editor.getSkelElm()) // create skel if needed
         if(Skeleton *dest=editor.mesh_skel)
      {
       C Skeleton &src=Proj.slot_skel_mem;
         editor.mesh_undos.set("slot");
         dest->addSlots(src);
         editor.setChangedSkel(true);
      }
   }
   void ObjView::SlotReplace(ObjView &editor)
   {
      if(editor.getSkelElm()) // create skel if needed
         if(Skeleton *dest=editor.mesh_skel)
      {
       C Skeleton &src=Proj.slot_skel_mem;
         editor.mesh_undos.set("slot");
         dest->slots.del();
         dest->addSlots(src);
         editor.setChangedSkel(true);
         editor.sel_slot=editor.lit_slot=-1;
      }
   }
   void ObjView::slotRot(flt x, flt y, flt z)
   {
      if(Skeleton *skel=mesh_skel)
      {
         mesh_undos.set("slotRot");
         SkelSlot *slot=skel->slots.addr(sel_slot);
         if( !slot)slot=skel->slots.addr(lit_slot);
         if(  slot)
         {
            if(x)slot->rotateCross(x);
            if(y)slot->rotatePerp (y);
            if(z)slot->rotateDir  (z);
            setChangedSkel(true);
         }
      }
   }
   void ObjView::SlotRotX(ObjView &editor) {editor.slotRot( PI_2, 0, 0);}
   void ObjView::SlotRotXN(ObjView &editor) {editor.slotRot(-PI_2, 0, 0);}
   void ObjView::SlotRotY(ObjView &editor) {editor.slotRot(0,  PI_2, 0);}
   void ObjView::SlotRotYN(ObjView &editor) {editor.slotRot(0, -PI_2, 0);}
   void ObjView::SlotRotZ(ObjView &editor) {editor.slotRot(0, 0,  PI_2);}
   void ObjView::SlotRotZN(ObjView &editor) {editor.slotRot(0, 0, -PI_2);}
   void ObjView::SlotSetSelMirror(ObjView &editor) {editor.slotSetSelMirror(false);}
   void ObjView::SlotSetMirrorSel(ObjView &editor) {editor.slotSetSelMirror(true );}
          void ObjView::slotSetSelMirror(bool set_other)
   {
      if(mesh_skel)
      {
         mesh_undos.set("slotMirror", true);
         int slot_i=(sel_slot>=0 ? sel_slot : lit_slot);
         if(SkelSlot *slot=mesh_skel->slots.addr(slot_i))
         {
            Str slot_name=BoneNeutralName(slot->name);
            REPA(mesh_skel->slots)if(i!=slot_i && slot_name==BoneNeutralName(mesh_skel->slots[i].name))
            {
               SkelSlot *other=&mesh_skel->slots[i];
               if(set_other)Swap(slot, other);
               SCAST(OrientP, *slot)=*other;
               slot->mirrorX();
               setChangedSkel(true);
               break;
            }
         }
      }
   }
   void ObjView::createSlots()
   {
      mode.tab(SLOTS)+=slot_tabs.create(Rect_U(mode.tab(SLOTS).rect().down()-Vec2(0, 0.01f), 0.73f, 0.06f), 0, slot_desc, Elms(slot_desc), true).func(SlotModeChanged, T);
      slot_tabs.tab(SLOT_MOVE  ).setText(S).setImage("Gui/Misc/move.img"  ).desc("Move slot\nSelect slot with LeftClick\nMove slot with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F3");
      slot_tabs.tab(SLOT_ROT   ).setText(S).setImage("Gui/Misc/rotate.img").desc("Rotate slot\nSelect slot with LeftClick\nRotate slot with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F4");
      slot_tabs.tab(SLOT_SCALE ).setText(S).setImage("Gui/Misc/scale.img" ).desc("Scale slot\nSelect slot with LeftClick\nScale slot with RightClick\n\nThis option is for scaling meshes which have been drag and dropped on the slot\nIt is used only for preview of the mesh - it is not saved.\n\nKeyboard Shortcut: Shift+F5");
      slot_tabs.tab(SLOT_ADD   ).desc("Create new slot:\nClick on the screen to create with no parent.\nClick on a bone to create with bone as the slot parent.\n\nKeyboard Shortcut: Shift+F1");
      slot_tabs.tab(SLOT_DEL   ).desc("Delete slot\nKeyboard Shortcut: Shift+F2");
      slot_tabs.tab(SLOT_RENAME).desc("Rename slot\nKeyboard Shortcut: Shift+F6");
      slot_tabs.tab(SLOT_PARENT).desc("Set slot parent:\nClick on a slot and drag it to a bone to set it as the parent.\nAlternatively drag the bone to the slot.\nHold Control to set a secondary bone parent.\n\nKeyboard Shortcut: Shift+F7");
      {
         Node<MenuElm> n;
         n.New().create("Copy to Memory"     , SlotCopy   , T).desc(   "Copy Skeleton Slots into memory, so they can be pasted into another object.\nIf a slot is selected then only that slot will be copied, otherwise all slots are copied.");
         n.New().create("Paste from Memory"  , SlotPaste  , T).desc(  "Paste Skeleton Slots from memory");
         n.New().create("Replace from Memory", SlotReplace, T).desc("Replace Skeleton Slots from memory");
         n++;
         n.New().create("Set Mirrored from Selection", SlotSetMirrorSel, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD           )).desc("This option will set slot transformation from the other side as mirrored version of the selected slot");
         n.New().create("Set Selection from Mirrored", SlotSetSelMirror, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("This option will set selected slot transformation as mirrored version of the slot from the other side");
         n++;
         n.New().create("Rotate +X", SlotRotX , T).kbsc(KbSc(KB_X, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT           )).desc("Rotate selected slot along its X axis");
         n.New().create("Rotate -X", SlotRotXN, T).kbsc(KbSc(KB_X, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT|KBSC_SHIFT)).desc("Rotate selected slot along its -X axis");
         n.New().create("Rotate +Y", SlotRotY , T).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT           )).desc("Rotate selected slot along its Y axis");
         n.New().create("Rotate -Y", SlotRotYN, T).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT|KBSC_SHIFT)).desc("Rotate selected slot along its -Y axis");
         n.New().create("Rotate +Z", SlotRotZ , T).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT           )).desc("Rotate selected slot along its Z axis");
         n.New().create("Rotate -Z", SlotRotZN, T).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT|KBSC_SHIFT)).desc("Rotate selected slot along its -Z axis");
         mode.tab(SLOTS)+=slot_ops.create(Rect_LU(slot_tabs.rect().ru(), slot_tabs.rect().h()), n); slot_ops.flag|=COMBOBOX_CONST_TEXT;
      }
   }
   void ObjView::DragSlotsSlot(ptr slot_index, GuiObj *go, C Vec2 &screen_pos)
   {
      int slot_i=intptr(slot_index);
      if(ObjEdit.mesh_skel && InRange(slot_i, ObjEdit.mesh_skel->slots))
      {
         ObjEdit.mesh_undos.set("slotBone", true);
         SkelSlot &slot=ObjEdit.mesh_skel->slots[slot_i];
         slot.bone1=(InRange(ObjEdit.lit_bone, ObjEdit.mesh_skel->bones) ? ObjEdit.lit_bone : 0xFF);
         if(!Kb.ctrlCmd())slot.bone=slot.bone1;
         ObjEdit.setChangedSkel(false);
      }
   }
   void ObjView::DragSlotsBone(ptr bone_index, GuiObj *go, C Vec2 &screen_pos)
   {
      int bone=intptr(bone_index);
      if(ObjEdit.mesh_skel && InRange(bone, ObjEdit.mesh_skel->bones) && InRange(ObjEdit.lit_slot, ObjEdit.mesh_skel->slots))
      {
         ObjEdit.mesh_undos.set("slotBone", true);
         SkelSlot &slot=ObjEdit.mesh_skel->slots[ObjEdit.lit_slot];
         slot.bone1=bone;
         if(!Kb.ctrlCmd())slot.bone=slot.bone1;
         ObjEdit.setChangedSkel(false);
      }
   }
   void ObjView::updateSlots()
   {
      if(mode()==SLOTS)
      {
         // get 'slot_axis'
         if(!mesh_skel || !InRange(sel_slot, mesh_skel->slots) || slot_tabs()!=SLOT_MOVE && slot_tabs()!=SLOT_ROT)slot_axis=-1;else
         {
            Matrix m=mesh_skel->slots[sel_slot]; m.scaleOrn(SkelSlotSize); m*=trans.matrix;
            MatrixAxis(v4, m, slot_axis);
         }

         // operate on slots
         REPA(MT)if(Edit::Viewport4::View *view=v4.getView(MT.guiObj(i)))switch(slot_tabs())
         {
            case -1: if(MT.bp(i))sel_slot=lit_slot; break;

            case SLOT_ADD: if(MT.bp(i))
            {
               getSkelElm(); // create new skeleton if doesn't exist yet
               if(mesh_skel)
               {
                  mesh_undos.set("slotNew");
                  SkelSlot &slot=mesh_skel->slots.New();
                  SkelBone *bone=mesh_skel->bones.addr(lit_bone);
                  slot.setParent(bone ? lit_bone : 0xFF);
                  Str8 name; FREP(1000){name=S+"Slot "+i; if(!mesh_skel->findSlot(name))break;} Set(slot.name, bone ? bone->name : name);
                  if(!bone)
                  {
                     view->setViewportCamera();
                     slot.pos=ScreenToPos(MT.pos(i));
                     slot/=trans.matrix;
                  }else
                  {
                     slot.pos =bone->pos;
                     slot.dir =bone->dir;
                     slot.perp=bone->perp;
                  }
                  setChangedSkel(false);
               }
            }break;

            case SLOT_DEL: if(MT.bp(i) && mesh_skel && InRange(lit_slot, mesh_skel->slots))
            {
               mesh_undos.set("slotDel");
               mesh_skel->slots.remove(lit_slot); sel_slot=lit_slot=-1;
               setChangedSkel(false);
            }break;

            case SLOT_RENAME: if(MT.bp(i))
            {
               RenameSlot.activate(lit_slot);
            }break;

            case SLOT_PARENT: if(MT.bp(i) && mesh_skel)
            {
               if(InRange(lit_slot, mesh_skel->slots))Gui.drag(DragSlotsSlot, ptr(lit_slot), MT.touch(i));else
               if(InRange(lit_bone, mesh_skel->bones))Gui.drag(DragSlotsBone, ptr(lit_bone), MT.touch(i));
            }break;

            case SLOT_MOVE:
            case SLOT_ROT:
            case SLOT_SCALE:
            {
               if(MT.bp(i) && !MT.touch(i))sel_slot=lit_slot;
               if(mesh_skel && InRange(sel_slot, mesh_skel->slots))if(MT.touch(i) ? MT.b(i) : MT.b(i, 1))
               {
                  mesh_undos.set("slotEdit");
                  view->setViewportCamera();
                  SkelSlot &slot=mesh_skel->slots[sel_slot];
                  switch(slot_tabs())
                  {
                     case SLOT_MOVE:
                     {
                        Vec2 mul=(Kb.shift() ? 0.1f : 1.0f)*0.5f*CamMoveScale(v4.perspective())*MoveScale(*view);
                        switch(slot_axis)
                        {
                           case  0: slot.pos+=AlignDirToCam(slot.cross(), MT.ad(i)  *mul); break;
                           case  1: slot.pos+=AlignDirToCam(slot.perp   , MT.ad(i)  *mul); break;
                           case  2: slot.pos+=AlignDirToCam(slot.dir    , MT.ad(i)  *mul); break;
                           default: slot.pos+=         ActiveCam.matrix.x*MT.ad(i).x*mul.x
                                                      +ActiveCam.matrix.y*MT.ad(i).y*mul.y; break;
                        }
                        setChangedSkel(false);
                     }break;

                     case SLOT_ROT:
                     {
                        flt mul=(Kb.shift() ? 0.1f : 1.0f)*0.5f;
                        switch(slot_axis)
                        {
                           case  0: slot.rotateCross(MT.ad(i).sum()*mul); break;
                           case  1: slot.rotatePerp (MT.ad(i).sum()*mul); break;
                           case  2: slot.rotateDir  (MT.ad(i).sum()*mul); break;
                           default: slot.Orient::mul(Matrix3().setRotate(ActiveCam.matrix.z, MT.ad(i).sum()*mul)); break;
                        }
                        setChangedSkel(false);
                     }break;

                     case SLOT_SCALE:
                     {
                        REPA(slot_meshes)if(slot_meshes[i].name==mesh_skel->slots[sel_slot].name)slot_meshes[i].scale*=ScaleFactor(MT.ad(i).sum());
                      //setChangedSkel(); this doesn't change the skeleton but items put in slots
                     }break;
                  }
               }
            }break;
         }
      }
   }
   void ObjView::renameSlot(int index, C Str &old_name, C Str &new_name)
   {
      if(mesh_skel)FREPA(mesh_skel->slots)if(old_name==mesh_skel->slots[i].name)if(!index--)
      {
         mesh_undos.set("slotRename");
         Set(mesh_skel->slots[i].name, new_name);
         setChangedSkel(false);
         break;
      }
   }
   void ObjView::putMeshToSlot(C MeshPtr &mesh, int slot_index)
   {
      if(mesh_skel && InRange(slot_index, mesh_skel->slots))
      {
         SkelSlot &slot=mesh_skel->slots[slot_index];
         SlotMesh *sm=null; REPA(slot_meshes)if(slot_meshes[i].name==slot.name)
         {
            if(!mesh){slot_meshes.remove(i); return;}
            sm=&slot_meshes[i]; break;
         }
         if(mesh)
         {
            if(!sm)sm=&slot_meshes.New();
            sm->set(mesh, slot.name);
         }
      }
   }
   void ObjView::DelRootBone(ObjView &editor) {editor.delRootBone();}
          void ObjView::delRootBone()
   {
      if(mesh_skel)
      {
         int bone_index=edit_skel.nodeToBone(edit_skel.root);
         if(C EditSkeleton::Bone *bone=edit_skel.bones.addr(bone_index))delBone(mesh_skel->findBoneI(bone->name));
      }
      Gui.closeMsgBox(del_root_bone_dialog_id);
   }
   void ObjView::delBone(int bone_i)
   {
      if(mesh_skel)
         if(C SkelBone *bone=mesh_skel->bones.addr(bone_i))
      {
         Memt<byte, 256> remap;
         mesh_undos.set("boneDel"); if(MeshChange *change=mesh_undos.getNextUndo())change->removeBone(bone->name); // use 'getNextUndo' instead of 'set' because that may return null
cur_skel_to_saved_skel.removeBone(bone->name);
             edit_skel.removeBone(bone->name);
         if( mesh_skel->removeBone(bone_i, remap))
         {
            clearBones();
            mesh.skeleton(mesh_skel, true).skeleton(null);
            setChangedMesh(true, false);
            setChangedSkel(true);
         }
      }
   }
   void ObjView::BoneModeChanged(ObjView &editor) {}
   void ObjView::BoneRootChanged(ObjView &editor)
   {
      editor.mesh_undos.set("root");
      if(BoneRoot *root=editor.bone_root_data.addr(editor.bone_root()))editor.edit_skel.root=(root->node.is ? editor.edit_skel.findNodeI(root->node.name) : -1);
      editor.setBoneRootTextSize();
      editor.setChangedSkel(false);

      int bone_index=editor.edit_skel.nodeToBone(editor.edit_skel.root);
      if(editor.mesh_skel && InRange(bone_index, editor.mesh_skel->bones) && editor.mesh_skel->bones[bone_index].parent!=0xFF)bone_index=-1; // don't ask to delete if has a parent
      if(bone_index>=0)
      {
         bool used_bones[256]; if(InRange(bone_index, used_bones))
         {
            editor.mesh.setUsedBones(used_bones);
            if(used_bones[bone_index])bone_index=-1; // don't ask to delete if used by mesh
         }
      }
      if(bone_index>=0)
      {
         Dialog &dialog=Gui.getMsgBox(del_root_bone_dialog_id);
         dialog.set("Delete Root Bone", "Would you like to delete this bone from the Skeleton?", Memt<Str>().add("Delete").add("No")); dialog.button[2].show();
         dialog.activate();
         dialog.buttons[0].func(DelRootBone, editor).kbSet();
         dialog.buttons[1].func(Hide, SCAST(GuiObj, dialog));
      }else Gui.closeMsgBox(del_root_bone_dialog_id);
   }
   void ObjView::SkelCopy(ObjView &editor) {Skeleton &dest=Proj.skel_mem; dest.del(); if(C Skeleton *src=editor.mesh_skel)dest.bones=src->bones; Proj.edit_skel_mem=editor.edit_skel;}
   void ObjView::SkelPaste(ObjView &editor)
   {
      if(editor.getSkelElm()) // create skel if needed
         if(Skeleton *dest=editor.mesh_skel)
      {
       C Skeleton &src=Proj.skel_mem;

         // check if can paste
         if(src.bones.elms()+dest->bones.elms()+1>=256){Gui.msgBox(S, "Can't paste skeleton bones because the total number will exceed the 256 bone limit."); return;}
         REPA(src.bones)if(dest->findBoneI(src.bones[i].name)>=0){Gui.msgBox(S, S+"Can't paste skeleton bones because both source and target contain a bone with the same name \""+src.bones[i].name+"\". Skeleton Bone names must be unique."); return;}

         // paste
         editor.mesh_undos.set("skel");
                     dest->add(src);
         editor.edit_skel.add(Proj.edit_skel_mem, true);
         editor.clearBones();
         editor.mesh.skeleton(dest).skeleton(null);
         editor.setChangedMesh(true, false);
         editor.setChangedSkel(true);
      }
   }
   void ObjView::SkelReplace(ObjView &editor)
   {
      if(editor.getSkelElm()) // create skel if needed
         if(Skeleton *dest=editor.mesh_skel)
      {
       C Skeleton &src=Proj.skel_mem;
         editor.mesh_undos.set("skel");
         dest->bones=src.bones;
         editor.edit_skel=Proj.edit_skel_mem;
         editor.clearBones();
         editor.mesh.skeleton(dest).skeleton(null);
         editor.setChangedMesh(true, false);
         editor.setChangedSkel(true);
      }
   }
   void ObjView::SkelSetSelMirror(ObjView &editor) {editor.skelSetSelMirror(false);}
   void ObjView::SkelSetMirrorSel(ObjView &editor) {editor.skelSetSelMirror(true );}
          void ObjView::skelSetSelMirror(bool set_other)
   {
      if(mesh_skel)
      {
         mesh_undos.set("boneMirror", true);
         int bone_i=((sel_bone>=0) ? sel_bone : lit_bone);
         if(SkelBone *bone=mesh_skel->bones.addr(bone_i))
         {
            Str bone_name=BoneNeutralName(bone->name);
            REPA(mesh_skel->bones)if(i!=bone_i && bone_name==BoneNeutralName(mesh_skel->bones[i].name))
            {
               SkelBone *other=&mesh_skel->bones[i];
               if(set_other)Swap(bone, other);
               SCAST(OrientP, *bone)=*other;
               bone->length=other->length;
               bone->width =other->width ;
             //bone.frac  =other.frac  ;
               bone->offset=other->offset;
               bone->shape =other->shape ;
               bone->mirrorX();
               setChangedSkel(true);
               break;
            }
         }
      }
   }
   void ObjView::RotBoneRoll(ObjView &editor)
   {
      if(editor.mesh_skel)
      if(SkelBone *bone=editor.mesh_skel->bones.addr(editor.sel_bone>=0 ? editor.sel_bone : editor.lit_bone))
      {
         editor.mesh_undos.set("boneEdit");
         bone->rotateDir(PI_2);
         editor.mesh_skel->setBoneTypes(); // bone orientation may affect bone type indexes
         editor.setChangedSkel(true);
         editor.mesh.skeleton(editor.mesh_skel, true).skeleton(null);
         editor.setChangedMesh(true, false);
      }
   }
   void ObjView::AdjustBoneOrnsDo(ObjView &editor) {editor.adjust_bone_orns.activate();}
   void ObjView::SkelDelBones(ObjView &editor) {editor.skelDelBones();}
          void ObjView::skelDelBones()
   {
      if(mesh_skel)
      {
         remVtx(VTX_SKIN); // this already calls undo
         if(MeshChange *change=mesh_undos.getNextUndo())REPA(mesh_skel->bones)change->removeBone(mesh_skel->bones[i].name);
cur_skel_to_saved_skel.bones.del();
             edit_skel.del();
             mesh_skel->bones.del();
         setChangedSkel(true);
      }
   }
   void ObjView::SkelDelLeafBones(ObjView &editor) {editor.skelDelLeafBones();}
          void ObjView::skelDelLeafBones()
   {
      if(skel_elm && mesh_skel)
      {
         mesh_undos.set("delLeafBones");
         bool changed=false;

         Memt<byte, 256> remap;
         bool used_bones[256]; mesh.setUsedBones(used_bones);
         REPA(mesh_skel->bones) // go from the end, so if bone is removed, then we can process later its parent, without having that child anymore
            if(InRange(i, used_bones) && !used_bones[i] && !mesh_skel->bones[i].children_num) // not used and don't have children
         {
            if(MeshChange *change=mesh_undos.getNextUndo())change->removeBone(mesh_skel->bones[i].name);
  cur_skel_to_saved_skel.removeBone(mesh_skel->bones[i].name);
               edit_skel.removeBone(mesh_skel->bones[i].name);
            if(mesh_skel->removeBone(i, remap))
            {
               changed=true;
             //mesh.boneRemap(remap); not needed since we have to do full assignment anyway
               bool used_bones2[256]; REP(Min(remap.elms(), Elms(used_bones)))used_bones2[remap[i]]=used_bones[i]; Swap(used_bones, used_bones2);
            }
         }
         if(changed)
         {
            mesh.skeleton(mesh_skel, true).skeleton(null);
            clearBones();
            setChangedSkel(true);
            setChangedMesh(true, false);
         }
      }
   }
   void ObjView::clearBones() {sel_bone=sel_bone_vis=lit_bone=lit_bone_vis=-1;}
   void ObjView::selectLit() {sel_bone=lit_bone; sel_bone_vis=lit_bone_vis;}
   int ObjView::boneToVis(int i) {if(Skeleton *vis_skel=getVisSkel())if(mesh_skel && InRange(i, mesh_skel->bones))return  vis_skel->findBoneI(mesh_skel->bones[i].name); return -1;}
   int ObjView::visToBone(int i) {if(Skeleton *vis_skel=getVisSkel())if(mesh_skel && InRange(i,  vis_skel->bones))return mesh_skel->findBoneI( vis_skel->bones[i].name); return -1;}
   void  ObjView::selBone(int i) {sel_bone=i; sel_bone_vis=boneToVis(i);}
   void ObjView::createBones()
   {
      mode.tab(BONES)+=adjust_bone_orns.create();
      mode.tab(BONES)+=bone_tabs.create(Rect_U(mode.tab(BONES).rect().down()-Vec2(0.16f, 0.01f), 0.73f, 0.06f), 0, bone_desc, Elms(bone_desc), true).func(BoneModeChanged, T);
      bone_tabs.tab(BONE_MOVE  ).setText(S).setImage("Gui/Misc/move.img").desc(S+"Move bone\nSelect bone with LeftClick\nMove bone with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F3");
      bone_tabs.tab(BONE_ROT   ).setText(S).setImage("Gui/Misc/rotate.img").desc(S+"Rotate bone\nSelect bone with LeftClick\nRotate bone with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F4");
      bone_tabs.tab(BONE_SCALE ).setText(S).setImage("Gui/Misc/scale.img").desc(S+"Scale bone\nSelect bone with LeftClick\nScale bone with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F5");
      bone_tabs.tab(BONE_ADD   ).desc(S+"Create new bone\nSelect bone with LeftClick\nAdd bone with RightClick\nHaving some bone selected while creating a new bone will set it as its parent.\nOptionally hold "+Kb.ctrlCmdName()+" to set Bone origin at mouse position facing forward.\n\nKeyboard Shortcut: Shift+F1");
      bone_tabs.tab(BONE_DEL   ).desc(S+"Delete bone\nKeyboard Shortcut: Shift+F2");
      bone_tabs.tab(BONE_RENAME).desc(S+"Rename bone\nKeyboard Shortcut: Shift+F6");
      bone_tabs.tab(BONE_PARENT).desc(S+"Set bone parent:\nClick on a child bone and drag it to a desired parent bone (or nothing) to set it as the parent.\n\nKeyboard Shortcut: Shift+F7");

      bone_tabs.tab(BONE_MOVE)+=bone_move_tabs.create(Rect_U(bone_tabs.tab(BONE_MOVE).rect().down()-Vec2(0, 0.01f), 0.28f, 0.055f), 0, bone_move_desc, Elms(bone_move_desc)).set(BONE_MOVE_END);
      bone_move_tabs.tab(BONE_MOVE_START).desc("Move only start of the bone");
      bone_move_tabs.tab(BONE_MOVE_END  ).desc("Move only end of the bone");

      bone_tabs.tab(BONE_MOVE)+=bone_children    .create(Rect_L(bone_move_tabs.rect().right()+Vec2(0.02f, 0), 0.54f, bone_move_tabs.rect().h()), bone_child_desc, Elms(bone_child_desc)).set(BONE_CHILDREN_NEAREST);
      bone_tabs.tab(BONE_ROT )+=bone_children_rot.create(Rect_U(bone_tabs.tab(BONE_ROT).rect().down()-Vec2(0, 0.01f), 0.54f, 0.055f), bone_child_rot_desc, Elms(bone_child_rot_desc)).set(0);

      {
         Node<MenuElm> n;
         n.New().create("Copy to Memory"     , SkelCopy   , T).desc(   "Copy Skeleton Bones into memory, so they can be pasted into another object");
         n.New().create("Paste from Memory"  , SkelPaste  , T).desc(  "Paste Skeleton Bones from memory");
         n.New().create("Replace from Memory", SkelReplace, T).desc("Replace Skeleton Bones from memory").kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_SHIFT));
         n++;
         n.New().create("Set Mirrored from Selection", SkelSetMirrorSel, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD           )).desc("This option will set bone transformation from the other side as mirrored version of the selected bone");
         n.New().create("Set Selection from Mirrored", SkelSetSelMirror, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("This option will set selected bone transformation as mirrored version of the bone from the other side");
         n++;
         n.New().create("Rotate Bone Roll", RotBoneRoll, T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_ALT)).desc("Rotate bone along its direction");
         n.New().create("Adjust Bone Orientations", AdjustBoneOrnsDo, T).kbsc(KbSc(KB_O, KBSC_CTRL_CMD)).desc("With this option you can automatically adjust bone orientations, in case it was not done by the Artist");
         n++;
         n.New().create("Delete Bones", SkelDelBones, T).desc("Delete all Skeleton Bones");
         n.New().create("Delete Leaf Bones without Skin", SkelDelLeafBones, T).desc("Delete all Skeleton Bones that have no children and no mesh connection");
         mode.tab(BONES)+=bone_ops.create(Rect_LU(bone_tabs.rect().ru(), bone_tabs.rect().h()), n); bone_ops.flag|=COMBOBOX_CONST_TEXT;
      }
      {
         mode.tab(BONES)+=bone_root_t.create(bone_ops.rect().right()+Vec2(0.05f, 0), "Root", &ts);
         mode.tab(BONES)+=bone_root  .create(Rect_L(bone_root_t.rect().right()+Vec2(0.09f, 0), 0.60f, 0.055f)).func(BoneRootChanged, T); FlagDisable(bone_root.flag, COMBOBOX_MOUSE_WHEEL);
         ListColumn lc[]=
         {
            ListColumn(MEMBER(BoneRoot, display), LCW_MAX_DATA_PARENT, S),
         };
         bone_root.setColumns(lc, Elms(lc));
      }
   }
   void ObjView::DragBonesBone(ptr bone_index, GuiObj *go, C Vec2 &screen_pos)
   {
      int bone=intptr(bone_index);
      if(ObjEdit.mesh_skel && InRange(bone, ObjEdit.mesh_skel->bones))
      {
         Memt<byte, 256> remap;
         ObjEdit.mesh_undos.set("boneParent", true);
         if(ObjEdit.mesh_skel->setBoneParent(bone, ObjEdit.lit_bone, remap))
         {
            ObjEdit.mesh.skeleton(ObjEdit.mesh_skel, true).skeleton(null);
            ObjEdit.setChangedMesh(true, false);
            ObjEdit.setChangedSkel(true);
            ObjEdit.clearBones();
         }
      }
   }
   void ObjView::updateBones()
   {
      if(mode()==BONES)
      {
         int bone_tabs=boneTabs();

         // get 'bone_axis'
         if(!mesh_skel || !InRange(sel_bone, mesh_skel->bones) || bone_tabs!=BONE_MOVE && bone_tabs!=BONE_ROT && bone_tabs!=BONE_SCALE)bone_axis=-1;else
         {
            Matrix m=mesh_skel->bones[sel_bone]; m.scaleOrn(SkelSlotSize); m*=trans.matrix;
            MatrixAxis(v4, m, bone_axis, &axis_vec);
         }
         int bone_axis=boneAxis();

         // operate on bones
         REPA(MT)if(Edit::Viewport4::View *view=v4.getView(MT.guiObj(i)))switch(bone_tabs)
         {
            case -1: if(MT.bp(i) && !MT.touch(i))selectLit(); break;

            case BONE_ADD:
            {
               if(MT.bp(i) && !MT.touch(i))selectLit();
               if(MT.bp(i, 1))
               {
                  getSkelElm(); // create new skeleton if doesn't exist yet
                  if(mesh_skel)
                  {
                     mesh_undos.set("boneNew");
                     int  bone_index=mesh_skel->bones.elms(), parent_index=sel_bone;
                     Str8 bone_name;
                     {
                        SkelBone &bone  =mesh_skel->bones.New(),
                                 *parent=mesh_skel->bones.addr(parent_index);
                        FREP(1000){bone_name=S+"Bone "+i; if(!mesh_skel->findBone(bone_name))break;} Set(bone.name, bone_name); // generate a unique name
                        view->setViewportCamera();
                        Vec mt_pos=ScreenToPos(MT.pos(i)),
                            from=(parent ? parent->to()*trans.matrix : VecZero), to=mt_pos;
                        if(Kb.ctrlCmd())
                        {
                           from=mt_pos;
                           to  =mt_pos+Vec(0, 0, 1)*SkelSlotSize;
                        }
                        bone.perp=(parent ? parent->perp*trans.matrix.orn() : Vec(0, 1, 0));
                        bone.setFromTo(from, to);
                        bone/=trans.matrix;
                     }
                     if(!mesh_skel->setBoneParent(bone_index, parent_index)) // set parent at the very end, because it may change the order of bones
                         mesh_skel->sortBones().setBoneTypes(); // if 'setBoneParent' didn't do anything then we still need to sort bones, because 'bone' was created as last, but we need to make sure that it's at the right position, 'setBoneTypes' needs to be called as well
                     lit_bone    =sel_bone    = mesh_skel  ->findBoneI(bone_name);
                     lit_bone_vis=sel_bone_vis=getVisSkel()->findBoneI(bone_name);
                     mesh.skeleton(mesh_skel, true).skeleton(null); // reset skeleton because we've added a new bone and need to reset the BoneMap (also bone order may have changed)
                     setChangedMesh(true, false);
                     setChangedSkel(true);
                  }
               }
            }break;

            case BONE_DEL: if(MT.bp(i))delBone(lit_bone); break;

            case BONE_RENAME: if(MT.bp(i))RenameBone.activate(lit_bone); break;

            case BONE_PARENT: if(MT.bp(i) && mesh_skel)
            {
               if(InRange(lit_bone, mesh_skel->bones))Gui.drag(DragBonesBone, ptr(lit_bone), MT.touch(i));
            }break;

            case BONE_MOVE:
            case BONE_ROT:
            case BONE_SCALE:
            {
               if(MT.bp(i) && !MT.touch(i))selectLit();
               if(mesh_skel)if(MT.touch(i) ? MT.b(i) : MT.b(i, 1))
               {
                  mesh_undos.set("boneEdit");
                  view->setViewportCamera();
                  SkelBone *bone=mesh_skel->bones.addr(sel_bone);
                  Vec2      mul =(Kb.shift() ? 0.1f : 1.0f)*0.5f;
                  switch(bone_tabs)
                  {
                     case BONE_MOVE:
                     {
                        mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                        Vec d; switch(bone_axis)
                        {
                           case  0:
                           case  1:
                           case  2: d=AlignDirToCam(axis_vec, MT.ad(i)  *mul ); break;
                           default: d=     ActiveCam.matrix.x*MT.ad(i).x*mul.x
                                          +ActiveCam.matrix.y*MT.ad(i).y*mul.y; break;
                        }
                        if(bone)
                        {
                           switch(bone_move_tabs())
                           {
                              case BONE_MOVE_START: bone->setFromTo(bone->pos+d, bone->to()  ); break;
                              case BONE_MOVE_END  : bone->setFromTo(bone->pos  , bone->to()+d); break;
                              default             : bone->pos+=d                            ; break;
                           }
                           if(bone_children()!=BONE_CHILDREN_NONE && bone_move_tabs()!=BONE_MOVE_START)
                              REPA(mesh_skel->bones)if(i!=sel_bone)
                           {
                              SkelBone &bone=mesh_skel->bones[i];
                              if(bone_children()==BONE_CHILDREN_NEAREST)
                              {
                                 if(bone.parent==sel_bone)bone.setFromTo(bone.pos+d, bone.to());
                              }else
                              {
                                 if(mesh_skel->contains(sel_bone, i))bone+=d;
                              }
                           }
                        }else mesh_skel->move(d);
                     }break;

                     case BONE_ROT:
                     {
                        flt angle=MT.ad(i).sum()*mul.x;
                        if(bone)
                        {
                           Vec axis;
                           switch(bone_axis)
                           {
                              case  0: axis=bone->cross()      ; break;
                              case  1: axis=bone->perp         ; break;
                              case  2: axis=bone->dir          ; break;
                              default: axis=ActiveCam.matrix.z; break;
                           }
                           Matrix m; m.setTransformAtPos(bone->pos, Matrix3().setRotate(axis, angle));
                           bone->transform(m);
                           if(bone_children_rot())REPA(mesh_skel->bones)if(sel_bone!=i && mesh_skel->contains(sel_bone, i))mesh_skel->bones[i].transform(m);
                        }else
                        {
                           mesh_skel->transform(Matrix3().setRotate(ActiveCam.matrix.z, angle));
                        }
                     }break;

                     case BONE_SCALE:
                     {
                        if(bone)switch(bone_axis)
                        {
                           case  0: {flt scale=ScaleFactor(AlignDirToCamEx(bone->cross(), MT.ad(i)*mul)      ); bone->width *=scale;                   } break; // width
                           case  1: {flt scale=ScaleFactor(AlignDirToCamEx(bone->perp   , MT.ad(i)*mul)      ); bone->width *=scale;                   } break; // width
                           case  2: {flt scale=ScaleFactor(AlignDirToCamEx(bone->dir    , MT.ad(i)*mul)      ); bone->length*=scale; bone->width/=scale;} break; // length
                           default: {flt scale=ScaleFactor(                             (MT.ad(i)*mul).sum()); bone->length*=scale;                   } break; // length & width
                        }else
                        {
                           mesh_skel->scale(ScaleFactor((MT.ad(i)*mul).sum()));
                        }
                     }break;
                  }
                  setChangedSkel(true);
               }
            }break;
         }
      }
   }
   bool ObjView::renameBone(C Str &old_name, C Str &new_name)
   {
      if(!mesh_skel
      || Equal(old_name, new_name, true))return true; // if it's exactly the same then close with success
      if(!new_name.is()){Gui.msgBox(S, "Name can't be empty."); return false;}
      if( new_name.length()>=MEMBER_ELMS(SkelBone, name)-1){Gui.msgBox(S, "Name too long."); return false;}
      int bone_i=mesh_skel->findBoneI((Str8)old_name); if(bone_i>=0)
      {
         REPA(mesh_skel->bones)if(i!=bone_i && new_name==mesh_skel->bones[i].name){Gui.msgBox(S, S+"Can't rename bone to \""+new_name+"\" because there already exists a bone with that name. Bone names must be unique."); return false;} // if there already exists a bone with new name the fail with error (but accept renaming the same bone in case just upper/lower case is being changed)

         mesh_undos.set("boneRename"); if(MeshChange *change=mesh_undos.getNextUndo())change->bone_renames.New().set(old_name, new_name); // use 'getNextUndo' instead of 'set' because that may return null
cur_skel_to_saved_skel.renameBone(old_name, new_name);
             edit_skel.renameBone(old_name, new_name);
         Set(mesh_skel->bones [bone_i].name, new_name); // this must not change the bone order because of simple BoneMap assignment below
         mesh_skel->setBoneTypes(); // bone names affect bone types
         mesh.clearSkeleton().skeleton(mesh_skel).skeleton(null); // first completely clear any skeleton info so when setting new, it will not try to adjust mapping, then set new to remember mapping, and clear pointer
         setChangedMesh(true, false);
         setChangedSkel(true);
      }
      return true;
   }
   void ObjView::PhysUndo(ObjView &editor) {editor.phys_undos.undo();}
   void ObjView::PhysRedo(ObjView &editor) {editor.phys_undos.redo();}
   void ObjView::PhysChanged(ObjView &editor) {if(editor.phys_tabs()==PHYS_DEL)editor.setPhys(-1);}
   void ObjView::NewBox(ObjView &editor) {if(editor.getPhysElm() && editor.phys){editor.phys_undos.set("box"    ); if(editor.hasPhysMeshOrConvex())editor.physDel(); MeshBase m; editor.physMesh(m); Box  box =m;                           {CacheLock cl(PhysBodies); editor.phys->parts.New().create(OBox(Box(Max(box.w(), 0.1f), Max(box.h(), 0.1f), Max(box.d(), 0.1f), box.center()))); editor.phys->setBox();} editor.setChangedPhys();}}
   void ObjView::NewBall(ObjView &editor) {if(editor.getPhysElm() && editor.phys){editor.phys_undos.set("ball"   ); if(editor.hasPhysMeshOrConvex())editor.physDel(); MeshBase m; editor.physMesh(m); Ball ball=m;       MAX(ball.r , 0.05f); {CacheLock cl(PhysBodies); editor.phys->parts.New().create(ball                                                                            ); editor.phys->setBox();} editor.setChangedPhys();}}
   void ObjView::NewCapsule(ObjView &editor) {if(editor.getPhysElm() && editor.phys){editor.phys_undos.set("capsule"); if(editor.hasPhysMeshOrConvex())editor.physDel(); MeshBase m; editor.physMesh(m); Box  box =m; flt h=Max(box.h(), 0.1f ); {CacheLock cl(PhysBodies); editor.phys->parts.New().create(Capsule(h*0.25f, h, box.center())                                                ); editor.phys->setBox();} editor.setChangedPhys();}}
   void ObjView::NewTube(ObjView &editor) {if(editor.getPhysElm() && editor.phys){editor.phys_undos.set("tube"   ); if(editor.hasPhysMeshOrConvex())editor.physDel(); MeshBase m; editor.physMesh(m); Box  box =m; flt h=Max(box.h(), 0.1f ); {CacheLock cl(PhysBodies); editor.phys->parts.New().create(Tube   (h*0.25f, h, box.center())                                                ); editor.phys->setBox();} editor.setChangedPhys();}}
   void ObjView::Convex8(ObjView &editor) {editor.physSetConvex( 8);}
   void ObjView::Convex16(ObjView &editor) {editor.physSetConvex(16);}
   void ObjView::Convex24(ObjView &editor) {editor.physSetConvex(24);}
   void ObjView::Convex32(ObjView &editor) {editor.physSetConvex(32);}
   void ObjView::Convex64(ObjView &editor) {editor.physSetConvex(64);}
   void ObjView::PhysMesh(ObjView &editor) {editor.physSetMesh  ();}
   void ObjView::PhysDel(ObjView &editor) {editor.physDel      ();}
   void ObjView::PhysCopy(ObjView &editor) {editor.physCopy     ();}
   Str  ObjView::PhysDensity(C ObjView &editor             ) {return editor.phys ? editor.phys->density : 1;}
   void ObjView::PhysDensity(  ObjView &editor, C Str &text)
   {
      if(editor.phys_elm && editor.phys)if(ElmPhys *ep=editor.phys_elm->physData())
      {
         editor.phys_undos.set("density");
         {CacheLock cl(PhysBodies); ep->density=editor.phys->density=TextFlt(text);}
         ep->density_time.getUTC();
         editor.setChangedPhys();
      }
   }
   Elm* ObjView::getPhysElm()
   {
      if(!phys_elm) // if doesn't exist yet
         if(Elm *mesh_elm=getMeshElm()) // we need to have a mesh to have a phys body
            if(phys_elm=Proj.getObjPhysElm(obj_id)) // if was just created
      {
         phys=Proj.gamePath(phys_elm->id);
         setChangedObj(); // we store game information about the phys in the object so we need to recreate it
      }
      return phys_elm;
   }
   bool ObjView::hasPhysMeshOrConvex()
   {
      if(phys)REPA(*phys)if(phys->parts[i].type()==PHYS_CONVEX || phys->parts[i].type()==PHYS_MESH)return true;
      return false;
   }
   void ObjView::physMesh(MeshBase &mesh)
   {
      mesh.createPhys(T.mesh).transform(mesh_matrix);
   }
   void ObjView::physSetConvex(int max_vtxs)
   {
      if(getPhysElm() && phys)
      {
         phys_undos.set("convex");
         MeshBase m; physMesh(m); m.createConvex(m.vtx.pos(), m.vtx.elms(), max_vtxs);
         {CacheLock cl(PhysBodies); if(phys->del().parts.New().createConvexTry(m, 1, true))phys->setBox();else phys->del();}
         setChangedPhys(); setPhys(-1);
      }
   }
   void ObjView::physSetMesh()
   {
      if(getPhysElm() && phys)
      {
         phys_undos.set("mesh");
         MeshBase m; physMesh(m); m.weldVtx(0, 0.005f).simplify(1, 0.01f, 1, 1, 1, 1, PI, true, SIMPLIFY_PLANES);
         {CacheLock cl(PhysBodies); if(phys->del().parts.New().createMeshTry(m))phys->setBox();else phys->del();}
         setChangedPhys(); setPhys(-1);
      }
   }
   void ObjView::physCopy()
   {
      if(phys && InRange(sel_phys, phys->parts) && phys->parts[sel_phys].type()==PHYS_SHAPE)
      {
         phys_undos.set("copy");
         {CacheLock cl(PhysBodies); PhysPart &part=phys->parts.New(); part=phys->parts[sel_phys];} // !! watch out for memory address change on create new part !!
         setChangedPhys();
      }
   }
   void ObjView::physDel() {if(phys){phys_undos.set("del"); CacheLock cl(PhysBodies); phys->del(); setChangedPhys(); setPhys(-1);}}
   void ObjView::createPhys()
   {
      GuiObj &parent=mode.tab(PHYS);
      Vec2    pos   =parent.rect().ld()-Vec2(0.05f, 0.01f);
      flt     d=0.06f, h=0.05f, w=d*4;

      parent+=phys_tabs.create(Rect_U(parent.rect().down()-Vec2(0, 0.01f), 0.50f, d), 0, phys_desc, Elms(phys_desc), true).func(PhysChanged, T);
      phys_tabs.tab(PHYS_MOVE  ).setText(S).setImage("Gui/Misc/move.img"  ).desc("Move seleted geometric shape\nSelect shape with LeftClick\nMove shape with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F2");
      phys_tabs.tab(PHYS_ROT   ).setText(S).setImage("Gui/Misc/rotate.img").desc("Rotate selected geometric shape\nSelect shape with LeftClick\nRotate shape with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F3");
      phys_tabs.tab(PHYS_SCALE ).setText(S).setImage("Gui/Misc/scale.img" ).desc("Scale selected geometric shape\nSelect shape with LeftClick\nScale shape with RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: Shift+F4");
      phys_tabs.tab(PHYS_DEL   ).desc("Delete geometric shape\nKeyboard Shortcut: Shift+F1");
      phys_tabs.tab(PHYS_TOGGLE).desc("Toggle which object parts should be included in physial body generation.\nClick on object parts to toggle them.\n\nKeyboard Shortcut: Shift+F5");

      {
         Node<MenuElm> n;
         n.New().create("Copy", PhysCopy, T).kbsc(KbSc(KB_D, KBSC_CTRL_CMD)).desc("Copy selected physical parts");
         parent+=phys_ops.create(Rect_LU(phys_tabs.rect().ru(), d), n).focusable(false); phys_ops.flag|=COMBOBOX_CONST_TEXT;
      }

      parent+=phys_redo.create(Rect_RU(phys_tabs.rect().lu()-Vec2(h, 0), d, d)).func(PhysRedo, T).focusable(false).desc("Physics Redo"); phys_redo.image="Gui/Misc/redo.img";
      parent+=phys_undo.create(Rect_RU(phys_redo.rect().lu()           , d, d)).func(PhysUndo, T).focusable(false).desc("Physics Undo"); phys_undo.image="Gui/Misc/undo.img";

      pos=phys_ops.rect().ru()+Vec2(h, 0);
      parent+=phys_box      .create(Rect_LU(pos                       , d, d)               ).func(NewBox    , T).focusable(false).desc("Add box to the physical body"    ); phys_box    .image="Gui/Misc/box.img";
      parent+=phys_ball     .create(Rect_LU(phys_box      .rect().ru(), d, d)               ).func(NewBall   , T).focusable(false).desc("Add ball to the physical body"   ); phys_ball   .image="Gui/Misc/circle.img";
      parent+=phys_capsule  .create(Rect_LU(phys_ball     .rect().ru(), d, d)               ).func(NewCapsule, T).focusable(false).desc("Add capsule to the physical body"); phys_capsule.image="Gui/Misc/capsule.img";
      parent+=phys_tube     .create(Rect_LU(phys_capsule  .rect().ru(), d, d)               ).func(NewTube   , T).focusable(false).desc("Add tube to the physical body"   ); phys_tube   .image="Gui/Misc/tube.img";
      parent+=phys_convex[0].create(Rect_LU(phys_box      .rect().ld(), w, h), "Convex 8"   ).func(Convex8   , T).focusable(false).desc("Create physical body as 8 vertex convex");
      parent+=phys_convex[1].create(Rect_LU(phys_convex[0].rect().ld(), w, h), "Convex 16"  ).func(Convex16  , T).focusable(false).desc("Create physical body as 16 vertex convex");
      parent+=phys_convex[2].create(Rect_LU(phys_convex[1].rect().ld(), w, h), "Convex 24"  ).func(Convex24  , T).focusable(false).desc("Create physical body as 24 vertex convex");
      parent+=phys_convex[3].create(Rect_LU(phys_convex[2].rect().ld(), w, h), "Convex 32"  ).func(Convex32  , T).focusable(false).desc("Create physical body as 32 vertex convex");
      parent+=phys_convex[4].create(Rect_LU(phys_convex[3].rect().ld(), w, h), "Convex 64"  ).func(Convex64  , T).focusable(false).desc("Create physical body as 64 vertex convex");
      parent+=phys_mesh     .create(Rect_LU(phys_convex[4].rect().ld(), w, h), "Static Mesh").func(PhysMesh  , T).focusable(false).desc("Create physical body as static triangle mesh\nPhysical bodies made from triangles can be only static");
      parent+=phys_del      .create(Rect_LU(phys_mesh     .rect().ld(), w, h), "No Physics" ).func(PhysDel   , T).focusable(false).desc("Delete physical body");

                     phys_props.New().create("Density" , MemberDesc(DATA_REAL).setFunc(PhysDensity, PhysDensity)).min(0);
      Property &mtrl=phys_props.New().create("Material", MemberDesc(DATA_STR )                                  ); mtrl.button.create("C").func(ClearPhMtrl, T).desc("Remove any current physics material this object has");
      AddProperties(phys_props, parent, phys_tabs.rect().ld()+Vec2(-0.38f, -0.01f), 0.05f, 0.81f, &ts); REPAO(phys_props).autoData(this);
      parent+=goto_phys_mtrl.create(mtrl.textline.rect()).func(GotoPhMtrl, T).focusable(false).desc("Click to open physics material in the editor.\nDrag and drop physics material here to set it for the physical body."); goto_phys_mtrl.text_align=1; mtrl.textline.del();
   }
   void ObjView::setPhysPartMatrix()
   {
      phys_part_matrix.identity();
      if(phys && InRange(sel_phys, phys->parts) && phys->parts[sel_phys].type()==PHYS_SHAPE)
      {
         phys_part_matrix=phys->parts[sel_phys].shape.asMatrixScaled();

         // set vectors to max scale
         flt l=phys_part_matrix.maxScale();
         phys_part_matrix.x.setLength(l);
         phys_part_matrix.y.setLength(l);
         phys_part_matrix.z.setLength(l);
      }
   }
   void ObjView::setPhys(int part)
   {
      sel_phys=part;
      setPhysPartMatrix();
   }
   void ObjView::updatePhys()
   {
      REPAO(phys_props).visible(phys_elm!=null);
         goto_phys_mtrl.visible(phys_elm!=null);

      if(mode()==PHYS)
      {
         // get phys_axis
         if(!phys || !InRange(sel_phys, phys->parts) || phys_tabs()!=PHYS_MOVE && phys_tabs()!=PHYS_ROT && phys_tabs()!=PHYS_SCALE)phys_axis=-1;else
         {
            MatrixAxis(v4, phys_part_matrix*trans.matrix, phys_axis);
         }

         REPA(MT)if(Edit::Viewport4::View *view=v4.getView(MT.guiObj(i)))
         {
            if(MT.bp(i) && !MT.touch(i))setPhys(lit_phys);
            switch(phys_tabs())
            {
               case PHYS_DEL:
               {
                  if(MT.bp(i) && phys && InRange(lit_phys, phys->parts)){phys_undos.set("del"); CacheLock cl(PhysBodies); phys->parts.remove(lit_phys); phys->setBox(); setPhys(lit_phys=-1); setChangedPhys();}
               }break;

               case PHYS_MOVE:
               case PHYS_ROT:
               case PHYS_SCALE:
               {
                  if(phys && InRange(sel_phys, phys->parts) && phys->parts[sel_phys].type()==PHYS_SHAPE)
                     if(MT.touch(i) ? MT.b(i) : MT.b(i, 1))
                  {
                     CacheLock cl(PhysBodies);
                     Shape &shape=phys->parts[sel_phys].shape;
                     view->setViewportCamera();
                     Vec2 mul=(Kb.shift() ? 0.1f : 1.0f)*0.5f;
                     switch(phys_tabs())
                     {
                        case PHYS_MOVE:
                        {
                           phys_undos.set("move");
                           mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                           Vec move;
                           switch(phys_axis)
                           {
                              case  0: move=AlignDirToCam(phys_part_matrix.x, MT.ad(i)  *mul); break;
                              case  1: move=AlignDirToCam(phys_part_matrix.y, MT.ad(i)  *mul); break;
                              case  2: move=AlignDirToCam(phys_part_matrix.z, MT.ad(i)  *mul); break;
                              default: move=              ActiveCam.matrix.x*MT.ad(i).x*mul.x
                                                         +ActiveCam.matrix.y*MT.ad(i).y*mul.y; break;
                           }
                           phys_part_matrix+=move;
                           shape           +=move;
                           phys->setBox();
                           setChangedPhys();
                        }break;

                        case PHYS_ROT:
                        {
                           phys_undos.set("rot");
                           Matrix m;
                           switch(phys_axis)
                           {
                              case  0: m.setTransformAtPos(phys_part_matrix.pos, Matrix3().setRotate(!phys_part_matrix.x, (MT.ad(i)*mul).sum())); break;
                              case  1: m.setTransformAtPos(phys_part_matrix.pos, Matrix3().setRotate(!phys_part_matrix.y, (MT.ad(i)*mul).sum())); break;
                              case  2: m.setTransformAtPos(phys_part_matrix.pos, Matrix3().setRotate(!phys_part_matrix.z, (MT.ad(i)*mul).sum())); break;
                              default: m.setTransformAtPos(phys_part_matrix.pos, Matrix3().setRotate( ActiveCam.matrix.z, (MT.ad(i)*mul).sum())); break;
                           }
                           phys_part_matrix*=m;
                           shape           *=m;
                           phys->setBox();
                           setChangedPhys();
                        }break;

                        case PHYS_SCALE:
                        {
                           phys_undos.set("scale");
                           switch(phys_axis)
                           {
                              case 0:
                              {
                                 flt scale=ScaleFactor(AlignDirToCamEx(phys_part_matrix.x, MT.ad(i)*mul));
                                 switch(shape.type)
                                 {
                                    case SHAPE_BALL   : shape.ball   .r*=scale; phys_part_matrix.scaleOrn(scale); break;
                                    case SHAPE_CAPSULE: shape.capsule.r*=scale; phys_part_matrix.x*=scale; phys_part_matrix.z*=scale; break;
                                    case SHAPE_TUBE   : shape.tube   .r*=scale; phys_part_matrix.x*=scale; phys_part_matrix.z*=scale; break;
                                    case SHAPE_OBOX   : {flt c=shape.obox.box.centerX(); shape.obox.box.setX((shape.obox.box.min.x-c)*scale+c, (shape.obox.box.max.x-c)*scale+c); phys_part_matrix.x*=scale;} break;
                                 }
                              }break;

                              case 1:
                              {
                                 flt scale=ScaleFactor(AlignDirToCamEx(phys_part_matrix.y, MT.ad(i)*mul));
                                 switch(shape.type)
                                 {
                                    case SHAPE_BALL   : shape.ball   .r*=scale; phys_part_matrix.scaleOrn(scale); break;
                                    case SHAPE_CAPSULE: shape.capsule.h*=scale; phys_part_matrix.y*=scale; break;
                                    case SHAPE_TUBE   : shape.tube   .h*=scale; phys_part_matrix.y*=scale; break;
                                    case SHAPE_OBOX   : {flt c=shape.obox.box.centerY(); shape.obox.box.setY((shape.obox.box.min.y-c)*scale+c, (shape.obox.box.max.y-c)*scale+c); phys_part_matrix.y*=scale;} break;
                                 }
                              }break;

                              case 2:
                              {
                                 flt scale=ScaleFactor(AlignDirToCamEx(phys_part_matrix.z, MT.ad(i)*mul));
                                 switch(shape.type)
                                 {
                                    case SHAPE_BALL   : shape.ball   .r*=scale; phys_part_matrix.scaleOrn(scale); break;
                                    case SHAPE_CAPSULE: shape.capsule.r*=scale; phys_part_matrix.x*=scale; phys_part_matrix.z*=scale; break;
                                    case SHAPE_TUBE   : shape.tube   .r*=scale; phys_part_matrix.x*=scale; phys_part_matrix.z*=scale; break;
                                    case SHAPE_OBOX   : {flt c=shape.obox.box.centerZ(); shape.obox.box.setZ((shape.obox.box.min.z-c)*scale+c, (shape.obox.box.max.z-c)*scale+c); phys_part_matrix.z*=scale;} break;
                                 }
                              }break;

                              default:
                              {
                                 Matrix m; m.setTransformAtPos(phys_part_matrix.pos, Matrix3(ScaleFactor((MT.ad(i)*mul).sum())));
                                 phys_part_matrix*=m;
                                 shape           *=m;
                              }break;
                           }
                           phys->setBox();
                           setChangedPhys();
                           setPhysPartMatrix(); // because matrix vectors length are set to maxScale, we always need to call this
                        }break;
                     }
                  }
               }break;

               case PHYS_TOGGLE:
               {
                  if(MT.bp(i) && InRange(lit_part, mesh)){FlagToggle(mesh.parts[lit_part].part_flag, MSHP_NO_PHYS_BODY); setChangedMesh(true, false);}
               }break;
            }
         }
      }
   }
   void ObjView::createRagdoll()
   {
      mode.tab(RAGDOLL)+=ragdoll_tabs.create(Rect_RU(mode.tab(RAGDOLL).rect().rd()-Vec2(0, 0.01f), 0.40f, 0.06f), 0, ragdoll_desc, Elms(ragdoll_desc), true).valid(true).set(RAGDOLL_TOGGLE);
      ragdoll_tabs.tab(RAGDOLL_SCALE ).setText(S).setImage("Gui/Misc/scale.img").desc("Change size of a bone\nLeftClick to select a bone\nRightClick to change its size\n\nKeyboard Shortcut: Shift+F2");
      ragdoll_tabs.tab(RAGDOLL_MOVE  ).setText(S).setImage("Gui/Misc/move.img").desc("Change offset of a bone\nLeftClick to select a bone\nRightClick to change its offset\n\nKeyboard Shortcut: Shift+F3");
      ragdoll_tabs.tab(RAGDOLL_TOGGLE).desc("Click on a skeleton bone to toggle it as a ragdoll bone\nKeyboard Shortcut: Shift+F1");
   }
   void ObjView::updateRagdoll()
   {
      if(mode()==RAGDOLL)
      {
         // get 'bone_axis'
         if(!mesh_skel || !InRange(sel_bone, mesh_skel->bones) || ragdoll_tabs()==RAGDOLL_TOGGLE)bone_axis=-1;else
         {
            Matrix m=mesh_skel->bones[sel_bone]; m.scaleOrn(SkelSlotSize); m*=trans.matrix;
            MatrixAxis(v4, m, bone_axis);
         }

         if(skel_elm && mesh_skel)REPA(MT)if(Edit::Viewport4::View *view=v4.getView(MT.guiObj(i)))switch(ragdoll_tabs())
         {
            case RAGDOLL_TOGGLE: if(MT.bp(i))
            {
               int bone; getSkel(Gui.objAtPos(MT.pos(i)), MT.pos(i), &bone, null);
               if(InRange(bone, mesh_skel->bones))
               {
                  mesh_undos.set("boneRagdoll");
                     mesh_skel->bones[bone].flag^=BONE_RAGDOLL;
                  if(mesh_skel->bones[bone].flag& BONE_RAGDOLL)selBone(bone);
                  setChangedSkel(true);
               }
            }break;

            case RAGDOLL_MOVE :
            case RAGDOLL_SCALE:
            {
               if(MT.bp(i) && !MT.touch(i))selectLit();
               if(InRange(sel_bone, mesh_skel->bones))if(MT.touch(i) ? MT.b(i) : MT.b(i, 1))
               {
                  mesh_undos.set("boneEdit");
                  view->setViewportCamera();
                  SkelBone &bone=mesh_skel->bones[sel_bone];
                      Vec2  mul =(Kb.shift() ? 0.1f : 1.0f)*0.5f;
                  switch(ragdoll_tabs())
                  {
                     case RAGDOLL_MOVE:
                     {
                        mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                        switch(bone_axis)
                        {
                           case  0: bone.offset+=AlignDirToCam(bone.cross(), MT.ad(i)  *mul); break;
                           case  1: bone.offset+=AlignDirToCam(bone.perp   , MT.ad(i)  *mul); break;
                           case  2: bone.offset+=AlignDirToCam(bone.dir    , MT.ad(i)  *mul); break;
                           default: bone.offset+=         ActiveCam.matrix.x*MT.ad(i).x*mul.x
                                                         +ActiveCam.matrix.y*MT.ad(i).y*mul.y; break;
                        }
                     }break;

                     case RAGDOLL_SCALE:
                     {
                        switch(bone_axis)
                        {
                           case  0: {flt scale=ScaleFactor(AlignDirToCamEx(bone.cross(), MT.ad(i)*mul)      ); bone.width *=scale;                   } break; // width
                           case  1: {flt scale=ScaleFactor(AlignDirToCamEx(bone.perp   , MT.ad(i)*mul)      ); bone.width *=scale;                   } break; // width
                           case  2: {flt scale=ScaleFactor(AlignDirToCamEx(bone.dir    , MT.ad(i)*mul)      ); bone.length*=scale; bone.width/=scale;} break; // length
                           default: {flt scale=ScaleFactor(                             (MT.ad(i)*mul).sum()); bone.width *=scale;                   } break; // width
                        }
                     }break;
                  }
                  setChangedSkel(true);
               }
            }break;
         }
      }
   }
ObjView::ObjView() : mesh_box(0), mesh_matrix(1), mesh_skel(null), body_skel(null), obj_id(UIDZero), obj_elm(null), mesh_elm(null), skel_elm(null), phys_elm(null), has_cur_pos(false), cur_pos(0), axis_vec(0), lit_lod(-1), sel_lod(0), sel_variation(0), lit_part(-1), lit_vf_part(-1), lit_vtx(-1), lit_face(-1), trans_axis(-1), lit_bone(-1), lit_bone_vis(-1), lit_slot(-1), sel_bone(-1), sel_bone_vis(-1), sel_slot(-1), slot_axis(-1), bone_axis(-1), lit_phys(-1), sel_phys(-1), phys_axis(-1), vtx_dup_mode(-1), vtx_sel_r(0.06f), edit_time(0), changed_obj(false), changed_mesh(false), changed_skel(false), changed_phys(false), mesh_undos(true), phys_undos(true) {}

ObjView::BackMesh::BackMesh() : matrix(1) {}

ObjView::SlotMesh::SlotMesh() : scale(1) {}

ObjView::MeshChange::MeshChange() : sel_only(false), can_use_cur_skel_to_saved_skel(true) {}

/******************************************************************************/
