/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
      void MeshParts::RenamePart::Hide(RenamePart &rp) {rp.hide(); ObjEdit.mesh_parts.activate();}
      void MeshParts::RenamePart::create()
      {
         Gui+=::EE::Window::create(Rect_C(0, 0, 1, 0.14f), "Rename Part").hide(); button[2].func(Hide, T).show();
         T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01f));
      }
      void MeshParts::RenamePart::activate(int part)
      {
         MeshLod &lod=ObjEdit.getLod();
         if(InRange(part, lod))
         {
            name =lod.parts[part].name;
            index=0; FREP(part)if(name==lod.parts[i].name)index++;
            textline.set(name).selectAll().activate();
            ::EE::GuiObj::activate();
         }
      }
      void MeshParts::RenamePart::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);

         if(Gui.window()==this)
         {
            if(Kb.k(KB_ENTER)){Kb.eatKey(); ObjEdit.mesh_parts.renamePart(index, name, textline()); button[2].push();}
         }
      }
      void MeshParts::Part::setColor() {color=GuiListTextColor(); if(removed)color.a/=2;}
      void MeshParts::WeldVtxPos::create()
      {
         Gui+=::EE::Window::create(Rect_C(0, 0, 1, 0.24f), "Weld Vertex Positions").hide(); button[2].show();
         T  +=text    .create(Vec2(clientWidth()/2, -0.05f), "Please Enter Position Tolerance");
         T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), -0.10f).extend(-0.01f)).set("0.001");
      }
      void MeshParts::WeldVtxPos::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);

         if(Gui.window()==this)
         {
            if(Kb.k(KB_ENTER))
            {
               ObjEdit.meshWeldPos(TextFlt(textline()));
               Kb.eatKey();
               button[2].push();
            }
         }
      }
   Str MeshParts::Removed(C Part &p) {if(MeshPart *part=ObjEdit.getPart(p.index))return FlagTest(part->part_flag, MSHP_HIDDEN); return S;}
   Str MeshParts::Name(C Part &p) {if(MeshPart *part=ObjEdit.getPart(p.index))return part->name       ; return S;}
   Str MeshParts::Vtxs(C Part &p) {if(MeshPart *part=ObjEdit.getPart(p.index))return part->vtxs     (); return S;}
   Str MeshParts::Tris(C Part &p) {if(MeshPart *part=ObjEdit.getPart(p.index))return part->trisTotal(); return S;}
   Str MeshParts::Mtrl(C Part &p) {if(MeshPart *part=ObjEdit.getPart(p.index))return Proj.elmFullName(part->variation(ObjEdit.selVariation()).id()); return S;}
   Str MeshParts::VtxsN(C Part &p) {if(MeshPart *part=ObjEdit.getPart(p.index))return TextInt(part->vtxs     (), 9); return S;}
   Str MeshParts::TrisN(C Part &p) {if(MeshPart *part=ObjEdit.getPart(p.index))return TextInt(part->trisTotal(), 9); return S;}
   void MeshParts::Refresh(MeshParts &parts) {parts.refresh();}
   void MeshParts::Locate(MeshParts &parts) {if(parts.list.sel.elms())parts.list.scrollTo(parts.list.absToVis(parts.list.sel[0]), false, 1);}
   void MeshParts::ListChanging(MeshParts &parts) {ObjEdit.trans_mesh.apply();}
   void MeshParts::ListChanged(MeshParts &parts) {ObjEdit.trans_mesh.setAnchorPos();}
   void MeshParts::EditChanged(MeshParts &parts)
   {
      // apply transformation according to previous mode
      {
         parts.edit_selected.set(!parts.edit_selected(), QUIET); ObjEdit.trans_mesh.apply();
         parts.edit_selected.set(!parts.edit_selected(), QUIET);
      }
      ObjEdit.vtx_face_sel_text.visible(parts.edit_selected());
      ObjEdit.vtx_face_sel_mode.visible(parts.edit_selected());
      ObjEdit.trans_mesh.setAnchorPos();
   }
   void MeshParts::HiddenToggle(Part &p)
   {
      if(MeshPart *part=ObjEdit.getPart(p.index))
      {
         ObjEdit.mesh_undos.set("toggle");
         FlagToggle(part->part_flag, MSHP_HIDDEN);
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::Rename()
   {
      MeshLod &lod=ObjEdit.getLod();
      REPA(ObjEdit.mesh_parts.list.sel)
      {
         int p=ObjEdit.mesh_parts.list.sel[i]; if(InRange(p, lod)){ObjEdit.mesh_parts.rename.activate(p); break;}
      }
   }
   void MeshParts::renamePart(int index, C Str &old_name, C Str &new_name)
   {
      if(!Equal(old_name, new_name, true))
      {
         ObjEdit.mesh_undos.set("rename");
         MeshLod &lod=ObjEdit.getLod();
         FREPA(lod)if(old_name==lod.parts[i].name)if(!index--)
         {
            Set(lod.parts[i].name, new_name);
            ObjEdit.setChangedMesh(true, false);
            break;
         }
      }
   }
   void MeshParts::Erase()
   {
      ObjEdit.mesh_undos.set("erase");
      MeshLod &lod=ObjEdit.getLod();
      REPA(lod)if(ObjEdit.mesh_parts.partSel(i)){lod.parts.remove(i, true); ObjEdit.mesh_parts.erasedPart(i);}
      ObjEdit.mesh.setBox();
      ObjEdit.setChangedMesh(true);
   }
   void MeshParts::Remove()
   {
      ObjEdit.mesh_undos.set("remove");
      REPA(ObjEdit.mesh_parts.list.sel)if(MeshPart *part=ObjEdit.getPart(ObjEdit.mesh_parts.list.sel[i]))FlagEnable(part->part_flag, MSHP_HIDDEN);
      if(!ObjEdit.mesh_parts.show_removed())ObjEdit.mesh_parts.list.sel.clear(); // if we won't see those parts (showing removed is not enabled) then deselect them
      ObjEdit.mesh.setBox();
      ObjEdit.setChangedMesh(true);
   }
   void MeshParts::Restore()
   {
      ObjEdit.mesh_undos.set("restore");
      REPA(ObjEdit.mesh_parts.list.sel)if(MeshPart *part=ObjEdit.getPart(ObjEdit.mesh_parts.list.sel[i]))FlagDisable(part->part_flag, MSHP_HIDDEN);
      ObjEdit.mesh.setBox();
      ObjEdit.setChangedMesh(true);
   }
   void MeshParts::Hide()
   {
      ObjEdit.mesh_undos.set("hide");
      REPA(ObjEdit.mesh_parts.list.sel)if(MeshPart *part=ObjEdit.getPart(ObjEdit.mesh_parts.list.sel[i]))part->variations(Max(part->variations(), ObjEdit.selVariation()+1)) // make room for variation first
                                                                                                             .variation (                       ObjEdit.selVariation(), null);
      ObjEdit.setChangedMesh(true, false);
   }
   void MeshParts::Focus() {ObjEdit.v4.moveTo(ObjEdit.selMeshCenter());}
   void MeshParts::Duplicate()
   {
      ObjEdit.mesh_undos.set("copy");
      MeshLod &lod=ObjEdit.getLod();
      REPA(lod)if(ObjEdit.mesh_parts.partSel(i))
      {
         MeshPart &copy=lod.parts.NewAt(i+1); copy.create(lod.parts[i]); // watch out because creating new elements invalidates old references
         ObjEdit.mesh_parts.addedPart(i+1);
      }
      ObjEdit.setChangedMesh(true, false);
   }
   void MeshParts::CopyMem()
   {
      Mesh &dest=Proj.mesh_mem; C MeshLod &src=ObjEdit.getLod(); dest.create(ObjEdit.mesh).setLods(1).parts.del(); // use 'Mesh.create' to copy 'BoneMap'
      FREPA(src)if(ObjEdit.mesh_parts.partSel(i))dest.parts.New().create(src.parts[i]);
   }
   void MeshParts::NewLod()
   {
      ObjEdit.mesh_undos.set("moveNewLod");
      int lod_i=ObjEdit.selLod(); // get at the start because creating new lod may change this index
      MeshLod &last=ObjEdit.mesh.newLod(); // create last first, because it will change memory address of other LOD's
      MeshLod &prev=ObjEdit.mesh.lod(ObjEdit.mesh.lods()-2),
              &lod =ObjEdit.mesh.lod(lod_i);
      ObjEdit.lodDist(last, NewLodClass::NextLodDist(ObjEdit.lodDist(prev)));
      FREPA(lod)if(ObjEdit.mesh_parts.partSel(i))Swap(lod.parts[i], last.parts.New());
       REPA(lod)if(ObjEdit.mesh_parts.partSel(i)){lod.parts.remove(i, true); ObjEdit.mesh_parts.erasedPart(i);}
      ObjEdit.mesh_parts.list.sel.clear();
      ObjEdit.setChangedMesh(true, false);
   }
   void MeshParts::Merge()
   {
      ObjEdit.mesh_undos.set("merge");
      if(ObjEdit.mesh_parts.list.sel.elms()>=2)
      {
         flt pos_eps=EPS*ObjEdit.posScale();
         MeshLod &lod=ObjEdit.getLod();
         for(ObjEdit.mesh_parts.list.sel.sort(Compare); ObjEdit.mesh_parts.list.sel.elms()>=2; )
         {
            int b=ObjEdit.mesh_parts.list.sel.pop(), a=ObjEdit.mesh_parts.list.sel.last();
            lod.join(a, b, pos_eps);
            ObjEdit.mesh_parts.erasedPart(b);
         }
         ObjEdit.setChangedMesh(true, false);
      }
   }
   Rect MeshParts::sizeLimit()C 
{
      Rect r;
      r.min=0.2f;
      r.max.set(D.w(), D.h()*1.5f);
      return r;
   }
              C Rect& MeshParts::rect() {return ::EE::Window::rect();}
   MeshParts& MeshParts::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      flt h=0.05f;
      edit_selected.rect(Rect_LU(0.01f, -0.01f, 0.23f, h));
      add          .rect(Rect_LU(edit_selected.rect().ru()+Vec2(0.01f, 0), h*2.6f, h));
      locate       .rect(Rect_LU(add          .rect().ru()+Vec2(0.01f, 0), h*2.5f, h));
      show_removed .rect(Rect_LU(locate       .rect().ru()+Vec2(0.01f, 0), h    , h));
      region       .rect(Rect(0, -clientHeight(), clientWidth(), show_removed.rect().min.y).extend(-0.01f));
      return T;
   }
   bool MeshParts::partSel(int part)C {return  list.sel.has (part)                 ;}
   bool MeshParts::partOp(int part)C {return !list.sel.elms(    ) || partSel(part);}
   int  MeshParts::visToPart(int vis )C {return list.visToAbs(vis );}
   int  MeshParts::partToVis(int part)C {return list.absToVis(part);}
   void MeshParts::highlight(int part)  {  list.lit=partToVis(part);}
   void MeshParts::clicked(int part)  {  list.cur=partToVis(part); list.processSel(part);}
   void MeshParts::erasedPart(int part)
   {
      if(ObjEdit.lit_vf_part==part){ObjEdit.lit_vf_part=ObjEdit.lit_vtx=ObjEdit.lit_face=-1;}else if(ObjEdit.lit_vf_part>=part)ObjEdit.lit_vf_part--;
      REPA(ObjEdit.sel_vtx ){int &p=ObjEdit.sel_vtx [i].x; if(p==part)ObjEdit.sel_vtx .remove(i, true);else if(p>=part)p--;}
      REPA(ObjEdit.sel_face){int &p=ObjEdit.sel_face[i].x; if(p==part)ObjEdit.sel_face.remove(i, true);else if(p>=part)p--;}
      REPA(list   .sel     ){int &p=list   .sel     [i]  ; if(p==part)list   .sel     .remove(i, true);else if(p>=part)p--;}
   }
   void MeshParts::addedPart(int part)
   {
      if(ObjEdit.lit_vf_part>=part)ObjEdit.lit_vf_part++;
      REPA(ObjEdit.sel_vtx ){int &p=ObjEdit.sel_vtx [i].x; if(p>=part)p++;}
      REPA(ObjEdit.sel_face){int &p=ObjEdit.sel_face[i].x; if(p>=part)p++;}
      REPA(list   .sel     ){int &p=list   .sel     [i]  ; if(p>=part)p++;}
   }
   void MeshParts::selParts(C MemPtr<int> &parts)
   {
      ListChanging(T); list.sel=parts;
      ListChanged (T);
   }
   void MeshParts::newMesh() {rename.hide(); weld_vtx.hide();}
   void MeshParts::modeChanged()
   {
      edit_selected.desc((ObjEdit.mode()==ObjView::MESH) ? S+"This option allows editing of vertexes and faces of selected parts\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+E" :
                         (ObjEdit.mode()==ObjView::SKIN) ? S+"This option hides all parts which are not selected\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+E" : S);
   }
   void MeshParts::skinChanged() {REPAO(data).setColor();}
   void MeshParts::showMenu(C Vec2 &pos)
   {
      if(list.sel.elms())
      {
       C MeshLod &lod=ObjEdit.getLod();
         bool remove=false, restore=false, hide=false;
         REPA(list.sel)
         {
            int p=list.sel[i]; if(InRange(p, lod))
            {
             C MeshPart &part=lod.parts[p];
               if(part.part_flag&MSHP_HIDDEN)restore=true;else remove=true;
               if(part.variation(ObjEdit.selVariation()))hide=true;
            }
         }
         hide&=(ObjEdit.mesh.variations()>1);
         Node<MenuElm> node;
         if(list.sel.elms()==1)node.New().create("Rename"           , Rename   ).desc("Rename part");
         if(remove            )node.New().create("Remove"           , Remove   ).desc("Mark selected parts as removed, these parts won't be available in the game");
         if(restore           )node.New().create("Restore"          , Restore  ).desc("Unmark selected parts as removed");
         if(hide              )node.New().create("Hide in Variation", Hide     ).desc("Mesh Part will be hidden in current variation.\nThis works by clearing the material to null.");
                               node.New().create("Focus Camera"     , Focus    ).desc("Focus camera on selected parts");
                               node.New().create("Copy Here"        , Duplicate).desc("Duplicated selected parts");
                               node.New().create("Copy to Memory"   , CopyMem  ).desc("Copy selected parts into memory which can be later copied using \"New\" option");
                               node.New().create("Move to New LOD"  , NewLod   ).desc("Move selected parts into a new LOD");
         if(list.sel.elms()> 1)node.New().create("Merge"            , Merge    ).desc("Merge selected parts together");
                               node++;                                   
                               node.New().create("Erase"            , Erase    ).desc("Completely erase selected parts from the Mesh");
         Gui+=menu.create(node).posRU(pos); menu.activate();
      }
   }
   void MeshParts::AddBox()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Box"); part.base.create(Box(0.5f), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddBall()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Ball"); part.base.create(Ball(0.5f), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddBall2()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Ball"); part.base.create2(Ball(0.5f), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddBallIco()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Ball"); part.base.createIco(Ball(0.5f), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddCapsule()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Capsule"); part.base.create(Capsule(0.3f, 1), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddTube()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Tube"); part.base.create(Tube(0.3f, 1), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddCone()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Cone"); part.base.create(Cone(0.5f, 0, 1), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddTorus()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         MeshPart &part=ObjEdit.getLod().parts.New(); Set(part.name, "Torus"); part.base.create(Torus(0.5f, 0.2f), ~0); part.setRender();
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   void MeshParts::AddMemory()
   {
      ObjEdit.mesh_undos.set("add");
      if(ObjEdit.getMeshElm())
      {
         Mesh temp; temp.create(Proj.mesh_mem); 
         if(ObjEdit.mesh_skel && ObjEdit.mesh_skel->is())temp.skeleton(ObjEdit.mesh_skel).skeleton(null);else temp.clearSkeleton().exclude(VTX_SKIN); // call 'skeleton' to reassign bone mapping or remove it if not present
         if(!ObjEdit.mesh.is() && ObjEdit.mesh.lods()<=1)Swap(ObjEdit.mesh, temp);else // if this mesh is empty, then just create from 'temp', this will copy all variations
         {
            temp.variations(Min(temp.variations(), ObjEdit.mesh.variations())); // remove variations that don't fit in dest
            MeshLod &dest=ObjEdit.getLod(); FREPA(temp)dest.parts.New().create(temp.parts[i]); // add parts in order as they're listed
         }
         ObjEdit.mesh.setBox();
         ObjEdit.setChangedMesh(true);
      }
   }
   MeshParts& MeshParts::create()
   {
      Node<MenuElm> add_elms;
      add_elms.New().create("Box"     , AddBox);
      add_elms.New().create("Ball"    , AddBall);
      add_elms.New().create("Ball 2"  , AddBall2);
      add_elms.New().create("Ball Ico", AddBallIco);
      add_elms.New().create("Capsule" , AddCapsule);
      add_elms.New().create("Tube"    , AddTube);
      add_elms.New().create("Cone"    , AddCone);
      add_elms.New().create("Torus"   , AddTorus);
      add_elms.New();
      add_elms.New().create("From Memory", AddMemory).desc("Add parts that were earlier copied to memory using \"Copy to Memory\" RightClick option");
      flt height=0.037f;
      ListColumn lc[]=
      {
         ListColumn(Removed            , height  , S), // 0 remove/restore
         ListColumn(MEMBER(Part, index), LCW_DATA, "#"), // 1
         ListColumn(Name               , LCW_DATA, "Name"), // 2
         ListColumn(Vtxs               , LCW_DATA, "Vtxs"), // 3
         ListColumn(Tris               , LCW_DATA, "Tris"), // 4
         ListColumn(Mtrl               , LCW_DATA, "Material"), // 5
      }; lc[3].text_align=lc[4].text_align=-1; lc[3].sort=&md_vtxs; lc[4].sort=&md_tris; md_vtxs.setDataToTextFunc(VtxsN); md_tris.setDataToTextFunc(TrisN);
      ::EE::Window::create("Parts"); flag|=WIN_RESIZABLE;
      T+=region.create().slidebarSize(0.04f);
      T+=edit_selected.create("Edit selected").func(EditChanged, T).focusable(false); edit_selected.mode=BUTTON_TOGGLE;
      T+=add          .create(add_elms).focusable(false).desc("Add new shape to the mesh"); add.text="New"; add.flag|=COMBOBOX_CONST_TEXT;
      T+=locate       .create("Locate").func(Locate, T).focusable(false).desc("Find selected parts in the list");
      T+=show_removed .create().func(Refresh, T).focusable(false).desc("Show removed parts"); show_removed.image="Gui/Misc/trash.img"; show_removed.mode=BUTTON_TOGGLE;
      region+=list.create(lc, Elms(lc)).elmHeight(height).textSize(0, 1).columnHeight(0.05f).selChanging(ListChanging, T).selChanged(ListChanged, T); list.flag|=LIST_MULTI_SEL;
      list.cur_mode=LCM_ALWAYS; list.setElmTextColor(MEMBER(Part, color));
      rect(Rect_LU(0, -0.6f, 0.6f, 0.5f));
      rename.create();
      weld_vtx.create();
      return T;
   }
   void MeshParts::refresh()
   {
      if(ObjEdit.obj_elm) // don't refresh when there's no object at all, so we don't lose list selection when unloading
      {
       C MeshLod &lod=ObjEdit.getLod();
         data.setNum(lod.parts.elms());
         Memt<bool> visible;
         FREPA(data)
         {
          C MeshPart &part=lod.parts[i];
            Part     &p   =data[i];
            p.index=i;
            p.remove.create(Rect_LU(0, list.elmHeight())).func(HiddenToggle, p).focusable(false);
            if(p.removed=FlagTest(part.part_flag, MSHP_HIDDEN))p.remove.setText ("R"            ).desc("Restore this part");
            else                                               p.remove.setImage("Gui/close.img").desc( "Remove this part");
            p.setColor();
            visible.add(!p.removed || show_removed());
            list.addChild(p.remove, i, 0);
         }
         list.setData(data, visible, true);
      }
      ObjEdit.trans_mesh.setAnchorPos();
   }
   void MeshParts::update(C GuiPC &gpc)
{
      if(gpc.visible && visible())
      {
         ::EE::Window::update(gpc);
       C MeshLod &lod=ObjEdit.getLod();
         if(lod.parts.elms()!=data.elms())refresh();
         if(Ms.bp(1) && Gui.ms()==&list)
         {
            if(!partSel(list.visToAbs(list.lit)) || list.sel.elms()<2)
            {
               list.cur=list.lit;
               list.processSel(list.visToAbs(list.cur), LSM_SET);
            }
            showMenu(Ms.pos());
         }
         if(Ms.bd(0) && Gui.ms()==&list)
         {
            rename.activate(list.visToAbs(list.cur));
         }
      }
   }
   void MeshParts::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj)) // assign materials to parts
      {
         REPA(elms)if(Elm *mtrl=Proj.findElm(elms[i], ELM_MTRL))
         {
            ObjEdit.setMaterial(visToPart(list.lit), Proj.gamePath(mtrl->id));
            break;
         }
      }
   }
MeshParts::RenamePart::RenamePart() : index(-1) {}

MeshParts::Part::Part() : removed(false), color(BLACK) {}

/******************************************************************************/
