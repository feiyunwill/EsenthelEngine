/******************************************************************************/
class LodView : GuiCustom
{
   int    lod_index=0;
   Button disable, erase;

   static Str TextSimple(flt x) {return x<10 ? TextReal(x, -1) : TextInt(Round(x));}

   virtual void draw(C GuiPC &gpc)override
   {
      if(visible() && gpc.visible)
      {
         D.clip(gpc.clip);
         Rect r=rect()+gpc.offset;
         r.extend(-0.005);
         r.draw((ObjEdit.selLod()==lod_index) ? RED : (ObjEdit.lit_lod==lod_index) ? TURQ : BLACK, false);
         TextStyleParams ts; ts.size=0.039;
         if(InRange(lod_index, ObjEdit.mesh.lods()))
         {
            MeshLod &base=ObjEdit.mesh, &lod=ObjEdit.mesh.lod(lod_index); flt dist=ObjEdit.lodDist(lod);
            D.text(ts, r, S+"Lod "+lod_index+"\nVtxs "+VisibleVtxs(lod)+"\nTris "+VisibleTrisTotal(lod)+"\nSize "+FileSize(VisibleSize(lod))+"\nQuality "+Round(VisibleLodQuality(ObjEdit.mesh, lod_index)*100)+"%\n"+(NegativeSB(dist) ? S+"DISABLED" : lod_index>0 ? S+"Distance "+TextSimple(dist) : S));
         }else
         {
            D.text(ts, r, "New");
         }
      }
   }
}
/******************************************************************************/
class LodRegion : Region
{
   Memx<LodView>  lods;
   Tabs           edit_dist;
   Text           edit_dist_text;
   TextWhite      ts;
   Memx<Property> props;
   Region         props_region;
   Menu           menu;

   static void Erase  (LodView   &lod) {ObjEdit.  eraseLOD(lod.lod_index);}
   static void Disable(LodView   &lod) {ObjEdit.disableLOD(lod.lod_index);}
   static void Disable(LodRegion &lr ) {ObjEdit.disableLOD((ObjEdit.lit_lod>=0) ? ObjEdit.lit_lod :  ObjEdit.selLod());}

   static Str  Dist(C LodRegion &lr          ) {return ObjEdit.absLodDist(ObjEdit.getLod());}
   static void Dist(  LodRegion &lr, C Str &t) {ObjEdit.mesh_undos.set("lodDist"); MeshLod &lod=ObjEdit.getLod(); flt dist=TextFlt(t); if(NegativeSB(lod.dist2))CHSSB(dist); ObjEdit.setLodDist(lod, dist);} // preserve sign because it specifies if LOD is disabled or not

   static void EditDistChanged(LodRegion &lr) {if(lr.edit_dist()>=0)NewLod.hide();}
   static void EditDistToggle (LodRegion &lr) {lr.edit_dist.toggle(0);}

   LodRegion& create()
   {
      Gui+=NewLod.create();
      super.create().skin(&EmptyGuiSkin, false); kb_lit=false;
      cchar8 *t_edit_dist[]=
      {
         "Edit Distance",
      };
      ObjEdit.mode.tab(ObjView.LOD)+=props_region.create().skin(&TransparentSkin, false);
      ObjEdit.mode.tab(ObjView.LOD)+=edit_dist.create(t_edit_dist, Elms(t_edit_dist)).func(EditDistChanged, T).desc(S+"Keyboard Shortcut: "+Kb.ctrlCmdName()+"+E").hide();
      edit_dist.tab(0)+=edit_dist_text.create("Set distance at which LOD's look similar", &ts); ts.reset().size=0.05;
      props.New().create("Distance", MemberDesc(DATA_REAL).setFunc(Dist, Dist)).min(0).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(0.5);
      AddProperties(props, edit_dist.tab(0), edit_dist.rect().ru()+Vec2(0.01, 0), 0.05, 0.2, &ts); REPAO(props).autoData(this);

      {
         Node<MenuElm> n;
         n.New().create("Edit"      ,               EditDistToggle, T     ).kbsc(KbSc(KB_E, KBSC_CTRL_CMD));
         n.New().create("Disable"   ,                      Disable, T     ).kbsc(KbSc(KB_D, KBSC_CTRL_CMD));
         n.New().create("Preview"   , NewLodClass.   PreviewToggle, NewLod).kbsc(KbSc(KB_P, KBSC_ALT     ));
         n.New().create("DrawAtDist", NewLodClass.DrawAtDistToggle, NewLod).kbsc(KbSc(KB_D, KBSC_ALT     ));
         Gui+=menu.create(n);
      }
      return T;
   }
   void resize()
   {
      Vec2 old=edit_dist.posRU();
      edit_dist.rect(Rect_D(ObjEdit.rect().w()/2-0.16, -ObjEdit.rect().h(), 0.32, 0.055));
      edit_dist_text.pos(edit_dist.rect().ru()+Vec2(0, ts.size.y/2+0.01));
      Vec2 delta=edit_dist.posRU()-old; REPAO(props).move(delta);
      props_region.rect(Rect_D(ObjEdit.rect().w()/2, -ObjEdit.rect().h(), 0.8, 0.13));
   }
   void toGui() {REPAO(props).toGui();}

   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(gpc.visible && visible())
      {
         flt s=0.25;
         lods.setNum(ObjEdit.mesh.lods()+1);
         FREPA(lods)
         {
            LodView &lod=lods[i];
            if(Gui.ms()==&lod || Gui.ms()==&lod.erase || Gui.ms()==&lod.disable)ObjEdit.lit_lod=i;
            if(!lod.is())
            {
               T+=lod.create();
               T+=lod.erase  .create().func(Erase  , lod).desc("Completely erase this LOD\nYou will not be able to restore it later."); lod.erase.image="Gui/close.img";
               T+=lod.disable.create().func(Disable, lod); lod.disable.mode=BUTTON_TOGGLE;
            }
            bool in_range=InRange(i, ObjEdit.mesh.lods());
            lod.lod_index=i;
            Rect_LU r(i*s, 0, s, s); lod.rect(r); r.extend(-0.005);
            lod.erase  .rect(Rect_RU(r.ru  (),      0.045, 0.045)).visible(ObjEdit.lit_lod==i && in_range /*&& ObjEdit.mesh.lods()>1 always display to just empty last LOD*/);
            lod.disable.rect(Rect_D (r.down(), r.w()*0.85, 0.049)).visible(ObjEdit.lit_lod==i && in_range);
            if(in_range)
            {
               lod.disable.set(NegativeSB(ObjEdit.mesh.lod(i).dist2), QUIET);
               if(lod.disable())
               {
                  lod.disable.text="DISABLED";
                  lod.disable.desc(S+"This LOD is now disabled, which means that it will NOT exist in the game.\nClick to enable it.\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+D");
               }else
               {
                  lod.disable.text="Disable";
                  lod.disable.desc(S+"This LOD is enabled, which means that it will exist in the game.\nClick to disable it.\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+D");
               }
            }
            lod.desc(in_range ? S+"Keyboard Shortcut: Shift+F"+(i+1) : S);
            REPAD(mt, MT)if(MT.bp(mt, 0) && MT.guiObj(mt)==&lod)
            {
               if(!in_range)NewLod.activate();
               else         ObjEdit.selLod(lod.lod_index);
            }
         }
         int l=lods.elms(), show_l=Min(5, l);
         size(Vec2(show_l*s, s+((l>show_l) ? slidebarSize() : 0)));
         props_region.visible(edit_dist.visible() && edit_dist()==0);
      }
   }

   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(ObjEdit.obj_elm && contains(focus_obj))REPAD(l, lods)if(lods[l].contains(focus_obj))
      {
         REPA(names)if(ExtType(GetExt(names[i]))==EXT_MESH)
         {
            NewLodClass.ImportEx(names[i], l);
            break;
         }
         break;
      }
   }
   void drag(Memc<UID> &elms, GuiObj* &focus_obj, C Vec2 &screen_pos)
   {
      if(ObjEdit.obj_elm && contains(focus_obj))REPAD(l, lods)if(lods[l].contains(focus_obj))
      {
         REPA(elms)
            if(Elm *elm=Proj.findElm(elms[i], ELM_OBJ))
               if(ElmObj *obj_data=elm.objData())
                  if(obj_data.mesh_id.valid())
         {
            Mesh src; if(Load(src, Proj.editPath(obj_data.mesh_id), Proj.game_path))if(src.is())
            {
               ObjEdit.mesh_undos.set("lod");
               ObjEdit.getMeshElm(); // make sure mesh exists
               src.skeleton(ObjEdit.mesh_skel).skeleton(null); // set skeleton to remap bones to match the original mesh
               src.setTangents().setBinormals().setRender();
               // keep original matrix as the 'mesh' matrix is identity
               flt dist_add;
               if(InRange(l, ObjEdit.mesh.lods()))dist_add=                        ObjEdit.absLodDist(ObjEdit.mesh.lod(l));
               else                               dist_add=NewLodClass.NextLodDist(ObjEdit.absLodDist(ObjEdit.mesh.lod(ObjEdit.mesh.lods()-1))); // get distance that we would normally use for the next LOD
               FREP(src.lods()) // store lods in mesh
               {
                  MeshLod &dest=ObjEdit.mesh.newLod(l+i);
                  Swap(dest, src.lod(i));
                  ObjEdit.lodDist(dest, ObjEdit.absLodDist(dest)+dist_add);
               }
               ObjEdit.setChangedMesh(true); ObjEdit.lod.toGui();
            }
            break;
         }
         focus_obj=null; // clear in case got deleted
         break;
      }
   }
}
/******************************************************************************/
class NewLodClass : ClosableWindow
{
   bool           preview=true, simplified_valid=false, processed_ready=false, keep_border=false, draw_at_distance=false, finished=false, stop=false;
   flt            intensity=0.5, max_distance=1, max_uv=1, max_color=1, max_material=1, max_skin=1, max_normal=PI, draw_distance=2, scale=1;
   uint           src_id=0, simplified_src_id=0, change_id=0, simplified_change_id=0;
   Property      *quality=null, *preview_prop=null, *draw_at_distance_prop=null;
   Memx<Property> props;
   TextBlack      ts;
   Button         ok, from_file;
   WindowIO       win_io;
   MeshLod        src, processed, simplified;
   Thread         thread;
   SyncLock       lock;

   static flt NextLodDist(flt dist) {flt next=Max(2, Abs(dist)*2); if(NegativeSB(dist))CHSSB(next); return next;} // make next LOD distance 2x bigger, and at least at 2 meters away

   static void FromFile(NewLodClass &nl)
   {
      Str path=Proj.elmSrcFileFirst(ObjEdit.mesh_elm); if(FileInfoSystem(path).type==FSTD_FILE)path=GetPath(path); if(path.is())nl.win_io.path(S, path);
      nl.win_io.activate(); nl.hide();
   }
   static void    PreviewToggle(NewLodClass &nl) {if(nl.         preview_prop)nl.         preview_prop.set(!nl.         preview_prop.asBool());}
   static void DrawAtDistToggle(NewLodClass &nl) {if(nl.draw_at_distance_prop)nl.draw_at_distance_prop.set(!nl.draw_at_distance_prop.asBool());}
   static void OK(NewLodClass &nl)
   {
      if(!nl.finished)Gui.msgBox(S, "Processing not finished yet");else
      {
         nl.hide();
         if(ObjEdit.mesh_elm)
         {
            ObjEdit.mesh_undos.set("lod");
            Mesh &mesh=ObjEdit.mesh;
            ObjEdit.lodDist(nl.simplified, nl.draw_at_distance ? nl.draw_distance : NextLodDist(ObjEdit.lodDist(mesh.lod(mesh.lods()-1))));
            Swap(mesh.newLod(), nl.simplified);
            ObjEdit.setChangedMesh(true);
            ObjEdit.lod.toGui();
         }
      }
   }
   static void ChangedParams(C Property &prop) {NewLod.changedParams();}
          void changedParams()
   {
      stop=true;
      change_id++;
      finished=false;
      if(quality)quality.name.set(S+"Quality: Calculating..");
   }

   static void Import  (C Str &name, ptr=null) {ImportEx(name, 0xFFFF);}
   static void ImportEx(C Str &name, int lod_index)
   {
      if(ObjEdit.getMeshElm()) // make sure mesh is created
      {
         Mesh                                 &mesh=ObjEdit.mesh, temp;
         Memc<ImporterClass.Import.MaterialEx> mtrls;
         Memc<int                            > pmi;
         if(EE.Import(name, &temp, null, null, SCAST(Memc<XMaterial>, mtrls), pmi))
            if(temp.is())
         {
            ObjEdit.mesh_undos.set("lod");
            Proj.setListCurSel(); // because we may add new elements (materials)

            FixMesh(temp);
            temp.skeleton(ObjEdit.mesh_skel).skeleton(null); // set skeleton to remap bones to match the original mesh
            temp.setTangents().setBinormals();
            // keep original 'temp' matrix as the 'mesh' matrix is identity
            MeshLod &lod=temp;

            // first setup materials
            Memt<MaterialPtr> mtrl_ptrs;
            Str path=GetPath(name);
            Edit.FileParams fp=name;
            FREPA(mtrls)
            {
               // TODO: detect existing similar materials in the source mesh?
               mtrls[i].process(path);
               fp.getParam("name").value=mtrls[i].name;
               Elm &mtrl=Proj.newMtrl(mtrls[i], ObjEdit.obj_id, fp.encode()); Server.setElmFull(mtrl.id);
               mtrl_ptrs.add(Proj.gamePath(mtrl.id));
            }

            // now process mesh
            FREPA(lod)if(InRange(i, pmi)) // set materials
            {
               int mtrl_index=pmi[i];
               if(InRange(mtrl_index, mtrl_ptrs))lod.parts[i].material(mtrl_ptrs[mtrl_index]);
            }
            ObjEdit.lodDist(lod, NextLodDist(ObjEdit.lodDist(mesh.lod(mesh.lods()-1))));
            lod.setRender();

            // store lod in mesh and finalize
            Swap(mesh.newLod(lod_index), lod);
            ObjEdit.setChangedMesh(true);
            ObjEdit.lod.toGui();
            Proj.setList();
         }
      }
   }

   bool needRebuild()C {return src_id!=simplified_src_id || change_id!=simplified_change_id;}

   static bool Simplify(Thread &thread) {return ((NewLodClass*)thread.user).simplify();}
          bool simplify()
   {
      SyncLockerEx locker(lock);
      if(needRebuild())
      {
         simplified_src_id   =   src_id;
         simplified_change_id=change_id;
         flt dist=scale*max_distance;

         MeshLod temp; Swap(src, temp);
         stop=false;
         locker.off();

         ThreadMayUseGPUData();
         MeshLod processed; temp.simplify(intensity, dist, max_uv, max_color, max_material, max_skin, max_normal, keep_border, SIMPLIFY_QUADRIC, EPS*scale, &processed, &stop);
         if(!stop)processed.setRender(false);
         locker.on();
         if(simplified_src_id==src_id)
         {
            Swap(temp, src);
            if(!stop && simplified_change_id==change_id)
            {
               Swap(processed, T.processed);
               processed_ready=true;
            }
         }
      }else
      {
         locker.off();
         Time.wait(1);
      }
      return true;
   }

   void startThread() {if(!thread.active())thread.create(Simplify, this);}
   void createSrc()
   {
      {
         MeshLod temp; temp.create(ObjEdit.mesh.lod(ObjEdit.mesh.lods()-1)).setBase().delRender(); SyncLocker locker(lock); Swap(temp, src); src_id++; scale=ObjEdit.posScale(); changedParams(); simplified.del(); simplified_valid=finished=false;
      }
      startThread();
   }
           NewLodClass& activate()override {if(hidden()){createSrc(); thread.cancelStop(); ObjEdit.lod.edit_dist.set(-1);} super.activate(); return T;}
   virtual NewLodClass& hide    ()override {                          thread.      stop();                                 super.hide    (); return T;}

  ~NewLodClass() {thread.del();} // delete thread before other members
   NewLodClass& del   ()override {thread.del(); super.del(); return T;}
   NewLodClass& create()
   {
      Property *draw_distance_prop;
      super.create("New LOD").hide(); button[2].show();
         preview_prop=&props.New().create("Preview"         , MEMBER(NewLodClass, preview         )).desc("Keyboard Shortcut: Alt+P");
draw_at_distance_prop=&props.New().create("Draw at Distance", MEMBER(NewLodClass, draw_at_distance)).desc("Draw LOD at specified distance\nKeyboard Shortcut: Alt+D");
   draw_distance_prop=&props.New().create("Draw Distance"   , MEMBER(NewLodClass, draw_distance   )).min(0).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(0.5);
                       props.New();
                       props.New().create("Intensity"       , MEMBER(NewLodClass, intensity       )).range(0, 1).mouseEditSpeed(0.4).desc("How much to simplify\n0..1\n0=no simplification\n1=full simplification");
                       props.New().create("Max Distance"    , MEMBER(NewLodClass, max_distance    )).min  (0   ).mouseEditSpeed(0.01).precision(4).desc("Max distance between elements to merge them");
                       props.New().create("Max UV"          , MEMBER(NewLodClass, max_uv          )).range(0, 1).mouseEditSpeed(0.01).precision(4).desc("Max allowed vertex texture UV deviations (0..1)");
                       props.New().create("Max Color"       , MEMBER(NewLodClass, max_color       )).range(0, 1).mouseEditSpeed(0.1).desc("Max allowed vertex color deviations (0..1)");
                     //props.New().create("Max Material"    , MEMBER(NewLodClass, max_material    )).range(0, 1).mouseEditSpeed(0.1).desc("Max allowed vertex material deviations (0..1)");  // this is not used since most likely there won't be any models with per-vertex materials over here
                       props.New().create("Max Skin"        , MEMBER(NewLodClass, max_skin        )).range(0, 1).mouseEditSpeed(0.1).desc("Max allowed vertex skin deviations (0..1)");
                       props.New().create("Max Normal"      , MEMBER(NewLodClass, max_normal      )).range(0, PI).mouseEditSpeed(0.1).precision(4).desc("Max allowed vertex normal angle deviations (0..PI)");
                       props.New().create("Keep Border"     , MEMBER(NewLodClass, keep_border     )).desc("If always keep border edges (edges that have faces only on one side)");
              quality=&props.New().create(S);
      ts.reset().size=0.045; ts.align.set(1, 0); Rect r=AddProperties(props, T, Vec2(0.02, -0.02), 0.05, 0.2, &ts); REPAO(props).autoData(&NewLod).changed(ChangedParams); preview_prop.changed(null, null); draw_at_distance_prop.changed(null, null); draw_distance_prop.changed(null, null);
      T+=       ok.create(Rect_U(r.down()-Vec2(0, 0.05), 0.3, 0.06), "OK").func(OK, T);
      T+=from_file.create(Rect_U(ok.rect().down()-Vec2(0, 0.01), 0.3, 0.06), "From File").func(FromFile, T);
      r|=from_file.rect(); Vec2 size=Vec2(r.max.x, -r.min.y)+0.02+defaultInnerPaddingSize();
      rect(Rect_RU(D.w(), D.h(), size.x, size.y));
      win_io.create(S, S, S, Import, Import).ext(SUPPORTED_MESH_EXT, "mesh");
      return T;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         if(needRebuild())startThread();
         if(processed_ready)
         {
            SyncLocker locker(lock);
            if(processed_ready)
            {
               processed_ready =false;
               simplified_valid=true;
               Swap(processed, simplified);
               if(simplified_change_id==change_id)
               {
                  finished=true;
                  if(quality)
                  {
                     int v=VisibleVtxs     (ObjEdit.mesh),
                         f=VisibleTrisTotal(ObjEdit.mesh);
                     flt q=Avg(v ? flt(VisibleVtxs     (simplified))/v : 1,
                               f ? flt(VisibleTrisTotal(simplified))/f : 1);
                     quality.name.set(S+"Quality: "+Round(q*100)+'%');
                  }
               }
            }
         }
      }
   }
}
NewLodClass NewLod;
/******************************************************************************/
