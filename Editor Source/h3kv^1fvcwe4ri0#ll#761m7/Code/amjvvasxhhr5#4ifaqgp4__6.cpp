/******************************************************************************/
class Obj : ObjData
{
   static int ComparePtr(Obj*C &a, Obj*C &b) {return Compare(uintptr(a), uintptr(b));}

   Area         *area=null;
   bool          selected=false, highlighted=false,
                 invalid_refs=false,
                 visible=false; // this should be reflection of the 'World.obj_visible'
   EDIT_OBJ_TYPE edit_type=EDIT_OBJ_MESH;
   MeshPtr       mesh, mesh_proper; // 'mesh_proper' is set to proper MeshPtr to avoid reloading/unloading meshes in case they are empty because 'mesh' will be forced to 'WorldEdit.dummy_mesh' if empty
   int           mesh_variation=0;
   PhysBodyPtr   phys;
   ImagePtr      icon;
   Particles     particles;
   Vec           light_col=1;
   flt           light_angle=1, light_falloff=0.5;
   bool          light_cast_shadows=true;

  ~Obj() {detach(); Selection.removed(T); WorldEdit.removed(T); ObjList.removed(T);}

   // get
   bool setInvalidRefs() {return invalid_refs=invalidRefs();}
   bool    invalidRefs()C
   {
      if(!removed)
      {
         if(Proj.invalidRef(params.base.id()) || Proj.invalidRef(params.type))return true;
         REPA(params)
         {
          C EditParam &param=params[i];
            if(!param.removed)
            {
               if(param.type==PARAM_ENUM)
               {
                  if(Proj.invalidRef(Enums.id(param.enum_type)))return true;
               }else
               if(ParamTypeID(param.type))
               {
                  // currently these are ignored
               }
            }
         }
         REPA(params.sub_objs)
         {
          C EditObject.SubObj &sub_obj=params.sub_objs[i];
            if(!sub_obj.removed)
            {
               if(Proj.invalidRef(sub_obj.elm_obj_id))return true;
            }
         }
      }
      return false;
   }
   cchar8* variationName()C {if(mesh_proper)return mesh_proper->variationName(mesh_variation); return null;}
   Matrix drawMatrix()
   {
      Matrix m=matrix;
      if(selected)m.pos+=WorldEdit.obj_pos.delta;
      return m;
   }
   Vec pos()
   {
      Vec pos=matrix.pos;
      if(selected)pos+=WorldEdit.obj_pos.delta;
      return pos;
   }
   Vec center()
   {
      Vec pos=(mesh ? mesh->ext.center*matrix : matrix.pos);
      if(selected)pos+=WorldEdit.obj_pos.delta;
      return pos;
   }
   bool getBox(Box &box)
   {
      bool have=false;
      if(mesh && mesh->is()){if(!have){have=true; box=mesh->ext;}else box|=mesh->ext;}
      if(phys && phys->is()){if(!have){have=true; box=phys->box;}else box|=phys->box;}
      return have;
   }
   bool onGround()
   {
      return area ? Abs(matrix.pos.y-area->hmHeight(matrix.pos))<=0.1 : false;
   }
   bool embedded()
   {
      return (params.access==OBJ_ACCESS_TERRAIN && area && area.world && area.world.ver) ? area.world.ver.embedded(T) : false;
   }
   void getCol(Color &col_lit, Color &col_shape)
   {
      col_shape=GetLitSelCol(highlighted, selected, invalid_refs ? InvalidColor : DefColor);
      col_lit  =((highlighted || selected) ? ColorBrightness(col_shape, 0.33) : TRANSPARENT);
   }
   PhysPath physPath()C {return removed ? PhysPath() : PhysPath(params.path, phys);}

   // operations
   void attach(WorldData &world, Area *a=null)
   {
      if(!a)
      {
         VecI2 xy=world.worldToArea(matrix.pos);
                a=world.getArea(xy);
      }
      if(a!=area)
      {
         if(!area && &world==&WorldEdit)removeChanged();
         detach();
         a.load();
         area=a; area.objs.add(this);
      }
   }
   void detach()
   {
      if(area)
      {
         area.objs.exclude(this);
         area=null;
      }
   }

   void setUpdatability()
   {
      if(visible && particles.is())
      {
         WorldEdit.obj_update.binaryInclude(this, ComparePtr);
      }else
      {
         WorldEdit.obj_update.binaryExclude(this, ComparePtr);
      }
   }
   void setVisibility()
   {
      bool visible=(!removed && WorldEdit.show_objs);
      if(visible)
      {
         if(InRange(params.access, WorldEdit.show_obj_access))visible=WorldEdit.show_obj_access[params.access];
         if(visible)if(params.access==OBJ_ACCESS_CUSTOM && WorldEdit.hide_obj_classes.binaryHas(params.type, Compare))visible=false;
      }
      if(!visible)
      {
         Selection.deselect   (T);
         Selection.unhighlight(T);
         WorldEdit.obj_visible.binaryExclude(this, ComparePtr);
      }else
      {
         WorldEdit.obj_visible.binaryInclude(this, ComparePtr);
      }
      T.visible=visible;
      setUpdatability();
   }
   void removeChanged()
   {
      params.updateBase(Proj.edit_path); // before 'setMeshPhys'
      setMeshPhys(); // set this in case 1) object is loaded and needs to have mesh set 2) object is removed/restored in which mesh needs to be set to null or correct pointer
      setVisibility();
   }
   void remove()
   {
      if(!removed)
      {
         setUndo();
         if(physPath())setChangedPaths(); // call before 'setChanged' because it may set path areas to non-existing
         setRemoved(true);
         setChanged().setChangedEmbed();
         removeChanged();
         ObjList.setChanged();
      }
   }
   void create(C Obj &src)
   {
      super.create(src);
      mesh=src.mesh; mesh_proper=src.mesh_proper; mesh_variation=src.mesh_variation;
      phys=src.phys;
   }
   bool sync(C ObjData &src, cchar *edit_path)
   {
      EditObject *base             =  params.base();
      bool        removed          =T.removed;
      uint        mesh_variation_id=  params.mesh_variation_id;
      if(super.sync(src, edit_path))
      {
         if(base   !=params.base())setMeshPhys  ();else if(mesh_variation_id!=params.mesh_variation_id)setMeshVariation(); // 'setMeshPhys' already calls 'setMeshVariation'
         if(removed!=T.removed    )removeChanged();
         return true;
      }
      return false;
   }
   bool undo(C ObjData &src, cchar *edit_path)
   {
      EditObject *base             =  params.base();
      bool        removed          =T.removed;
      uint        mesh_variation_id=  params.mesh_variation_id;
      if(super.undo(src, edit_path))
      {
         if(base   !=params.base())setMeshPhys  ();else if(mesh_variation_id!=params.mesh_variation_id)setMeshVariation(); // 'setMeshPhys' already calls 'setMeshVariation'
         if(removed!=T.removed    )removeChanged();
         return true;
      }
      return false;
   }

   Obj& setChangedEmbed(C VecI2 *area_xy=null) // must be called after 'changedObj' and before 'setChangedPaths'
   {
      if(area)if(WorldData *world=area.world)if(WorldVer *world_ver=world.ver)Proj.rebuildEmbedObj(id, area_xy ? *area_xy : area.xy, *world_ver, false); // 'rebuild_game_area_objs=false' this is Obj which belongs to Area, and Area will be saved to game ver on unload
      return T;
   }
   Obj& setChangedPaths() // must be called after 'setChangedEmbed'
   {
      if(area && area.world && area.world.ver)area.world.ver.rebuildPaths(id, area.xy);
      return T;
   }
   Obj& setChanged(bool matrix=false) // this needs to be called after making all changes
   {
      if(area  )area.setChangedObj(T); // this will set ObjVer, call this before all other things
      if(matrix){matrix_time.getUTC(); setChangedEmbed(); if(physPath())setChangedPaths();} // call 'setChangedEmbed' before 'setChangedPaths'
      setDraw();
      return T;
   }
   void setUndo(cptr type=WorldUndo.ObjType, bool as_new=false)
   {
      if(area && area.world)WorldEdit.undos.set(T, type, as_new);
   }
   void moveTo(C Vec &pos, cptr undo_type=WorldUndo.ObjType)
   {
      if(area && area.world)
      {
         setUndo(undo_type);
         setChanged(true);
         matrix.pos=pos; attach(*area.world);
         setChanged(true);
      }
   }
   void moveTo(C Matrix3 &orn, cptr undo_type=WorldUndo.ObjType)
   {
      setUndo(undo_type);
      matrix.orn()=orn;
      setChanged(true);
      // attach not needed since we're not changing position
   }
   void moveTo(C Matrix &matrix, cptr undo_type=WorldUndo.ObjType)
   {
      if(area && area.world)
      {
         setUndo(undo_type);
         setChanged(true);
         T.matrix=matrix; attach(*area.world);
         setChanged(true);
      }
   }

   void scaleBy (flt f) {setUndo(); matrix.orn()*=f; setChanged(true);}
   void scaleXBy(flt f) {setUndo(); matrix.x    *=f; setChanged(true);}
   void scaleYBy(flt f) {setUndo(); matrix.y    *=f; setChanged(true);}
   void scaleZBy(flt f) {setUndo(); matrix.z    *=f; setChanged(true);}

   void randomRot() {setUndo(); matrix.rotateYL(Random.f(PI2));        setChanged(true);}
   void  resetRot() {setUndo(); matrix.orn().setScale(matrix.scale()); setChanged(true);}

   void alignGrid()
   {
      Vec pos=matrix.pos;
      if(WorldEdit.gridAlignSize ()>0){pos.x=AlignRound(pos.x, WorldEdit.gridAlignSize ()); pos.z=AlignRound(pos.z, WorldEdit.gridAlignSize());}
      if(WorldEdit.gridAlignSizeY()>0) pos.y=AlignRound(pos.y, WorldEdit.gridAlignSizeY());
      moveTo(pos);
   }
   void alignTerrain(bool force, cptr undo_type=WorldUndo.ObjType)
   {
      if(area && area.hm)moveTo(Vec(matrix.pos.x, area.hmHeight(pos())      , matrix.pos.z), undo_type);else
      if(force          )moveTo(Vec(matrix.pos.x, WorldEdit.grid_plane_level, matrix.pos.z), undo_type);
   }
   void alignNormal(int axis=-1, flt blend=1)
   {
      if(blend>EPS && onGround() && area)
      {
         Vec axis_dir, hm_nrm=area.hmNormalAvg(matrix, mesh ? mesh->ext : Extent(0.5));
         switch((axis<0) ? GetNearestAxis(matrix, hm_nrm) : axis)
         {
            case  0: axis_dir=matrix.x; break;
            case  1: axis_dir=matrix.y; break;
            default: axis_dir=matrix.z; break;
         }
         moveTo(matrix.orn()*Matrix3().setRotation(!axis_dir, hm_nrm, Sat(blend)));
      }
   }

   // update
   void update()
   {
      particles.matrix=drawMatrix();
      if(!particles.update())particles.resetFull();
   }

   // draw
   bool skipDraw()
   {
      if(params.access==OBJ_ACCESS_GRASS) // if grass
         if(Renderer()==RM_SHADOW && !D.grassShadow() // shadows disabled
         || Dist2(pos(), ActiveCam.matrix.pos)>WorldEdit.grass_range2) // out of range
            return true;
      return false;
   }
   void draw()
   {
      if(skipDraw())return;
      Matrix m=drawMatrix();
      if(mesh && Frustum(*mesh, m))
      {
         if(WorldEdit.mode()==WorldView.OBJECT)
         {
            Color col_lit, col_shape; getCol(col_lit, col_shape);
            SetHighlight(col_lit);
         }
         SetVariation(mesh_variation);
         mesh->draw(m);
         SetHighlight();
         SetVariation();
      }
      switch(edit_type)
      {
         case EDIT_OBJ_LIGHT_POINT: {LightSqr  l(m.x.length(), m.pos,       light_col);                                                       l.add(light_cast_shadows, this);} break;
         case EDIT_OBJ_LIGHT_CONE : {LightCone l(m.x.length(), m.pos, !m.z, light_col); l.pyramid.scale=light_angle; l.falloff=light_falloff; l.add(light_cast_shadows, this);} break;
         case EDIT_OBJ_PARTICLES  :
         {
            switch(particles.renderMode()) // use include because this can be called multiple times per single rendering in RT_FORWARD
            {
               case RM_BLEND   : WorldEdit.obj_blend     .binaryInclude(this, ComparePtr); break;
               case RM_PALETTE : WorldEdit.obj_palette[0].binaryInclude(this, ComparePtr); break;
               case RM_PALETTE1: WorldEdit.obj_palette[1].binaryInclude(this, ComparePtr); break;
            }
         }break;
      }
   }
   void drawShadow()
   {
      if(skipDraw())return;
      if(mesh && CurrentLight.src!=this)
      {
         Matrix m=drawMatrix();
         if(Frustum(*mesh, m))
         {
            SetVariation(mesh_variation);
            mesh->drawShadow(m);
            SetVariation();
         }
      }
   }
   void drawParticle()
   {
      particles.draw();
   }
   void drawSelected()
   {
      Extent ext; if(mesh && mesh->is())ext=mesh->ext;else ext=0.5;
      Matrix m=drawMatrix();
      if(Frustum(ext, m))
      {
         Color col_lit, col_shape; getCol(col_lit, col_shape);
         SetMatrix(m); ext.draw(col_shape);
         if(selected)
         {
            SetMatrix();
            D.depthLock(false);
            DrawMatrix(m, WorldEdit.obj_axis);
            D.depthUnlock();
         }
      }
   }
   void drawHelper(bool box, bool phys)
   {
      Extent ext; if(mesh && mesh->is())ext=mesh->ext;else ext=0.5;
      Matrix m=drawMatrix();
      if(Frustum(ext, m))
      {
         Color col_lit, col_shape; getCol(col_lit, col_shape);
         SetMatrix(m); if(box)ext.draw(col_shape); if(phys && T.phys)T.phys->draw(col_shape);
      }
   }

   // io
   void setDraw()
   {
      switch(edit_type)
      {
         case EDIT_OBJ_LIGHT_POINT:
         {
            if(C EditParam *p=params.findParam("color"       ))light_col         =p.asVec ();else light_col         =1;
            if(C EditParam *p=params.findParam("cast shadows"))light_cast_shadows=p.asBool();else light_cast_shadows=true;
         }break;

         case EDIT_OBJ_LIGHT_CONE:
         {
            if(C EditParam *p=params.findParam("color"       ))light_col         =    p.asVec ()    ;else light_col         =1;
            if(C EditParam *p=params.findParam("cast shadows"))light_cast_shadows=    p.asBool()    ;else light_cast_shadows=true;
            if(C EditParam *p=params.findParam("angle"       ))light_angle       =Max(p.asFlt (), 0);else light_angle       =1;
            if(C EditParam *p=params.findParam("falloff"     ))light_falloff     =Sat(p.asFlt ()   );else light_falloff     =0.5;
         }break;
      }
      Adjust(particles, params, drawMatrix(), Proj);
      setUpdatability();
   }
   void setMeshVariation()
   {
      mesh_variation=(mesh_proper ? mesh_proper->variationFind(params.mesh_variation_id) : 0);
   }
   void setMeshPhys()
   {
      if(removed){edit_type=EDIT_OBJ_MESH; icon=null; mesh=null; mesh_proper=null; phys=null; particles.del(); setUpdatability();}else
      {
         edit_type=EDIT_OBJ_TYPE(Max(0, EditObjType.find(params.editor_type))); // default to 0 when not found
         switch(edit_type)
         {
            default                  : icon=null               ; break;
            case EDIT_OBJ_PARTICLES  : icon=Proj.icon_particles; break;
            case EDIT_OBJ_LIGHT_POINT:
            case EDIT_OBJ_LIGHT_CONE : icon=Proj.icon_light    ; break;
         }
         setDraw();

         MeshPtr     mesh; // set temp vars first to avoid unloading resources in case we assign the same values
         PhysBodyPtr phys;
         UID base_id=params.base.id();
      #if 1 // use 'ObjectPtr' in case it inherits mesh/phys from base
         if(ObjectPtr obj=Proj.gamePath(base_id))
         {
            mesh=obj->mesh();
            phys=obj->phys();
         }
      #else
         if(Elm *obj_elm=Proj.findElm(base_id))
            if(ElmObj *obj_data=obj_elm.objData())
               if(Elm *mesh_elm=Proj.findElm(obj_data.mesh_id))
         {
            mesh=Proj.gamePath(mesh_elm.id);
            if(ElmMesh *mesh_data=mesh_elm.meshData())
               if(mesh_data.phys_id.valid())
                  phys=Proj.gamePath(mesh_data.phys_id);
         }
      #endif
         T.mesh_proper=mesh;
         if(edit_type==EDIT_OBJ_MESH)if(!mesh || !mesh->is())mesh=&WorldEdit.dummy_mesh;
         T.mesh=mesh;
         T.phys=phys;
         setMeshVariation();
      }
   }
   bool save(File &f) {return super.save(f, Proj.edit_path);}
   bool load(File &f) // 'setMeshPhys' doesn't need to be called because it will be called in 'attach->removeChanged'
   {
      if(super.load(f, Proj.edit_path))
      {
         return true;
      }
      return false;
   }
}
/******************************************************************************/
void NewObjs(C Vec &pos)
{
   Vec        center=0;
   Memt<Obj*> selected;
   if(Selection.elms())
   {
      REPA(Selection){Obj &obj=Selection[i]; center+=obj.pos(); selected.add(&obj);} center/=Selection.elms();
      Matrix delta(-center); if(WorldEdit.obj_random_angle)delta.rotateY(Random.f(PI2)); delta.move(pos);
      if(WorldEdit.grid_align)
      {
         delta.pos.x=AlignRound(delta.pos.x, WorldEdit.grid_align_size_xz);
         delta.pos.z=AlignRound(delta.pos.z, WorldEdit.grid_align_size_xz);
      }
      REPA(selected)
      {
         Obj &src=*selected[i], &obj=WorldEdit.objs.New();
         obj.create(src);
         obj.matrix*=delta;
         obj.attach(WorldEdit);
         obj.setChanged(true); // call after attaching
         obj.setUndo(WorldUndo.NewObjType, true); // call set Undo at the end with 'as_new'
         Selection.  select(obj);
         Selection.deselect(src);
      }
      ObjList.setChanged();
      WorldEdit.objTransChanged();
   }
}
Obj* NewObj(C Vec &pos, Elm &elm)
{
   if(elm.type==ELM_OBJ)
   {
      Obj &obj=WorldEdit.objs.New();
      obj.params.setBase(Proj.editPath(elm.id), Proj.edit_path);
      obj.matrix.setPos(pos);
      obj.setMeshPhys();
      obj.attach(WorldEdit);
      obj.setChanged(true); // call after attaching
      obj.setUndo(WorldUndo.NewObjType, true); // call set Undo at the end with 'as_new'
      ObjList.setChanged();
      WorldEdit.objTransChanged();
      return &obj;
   }
   return null;
}
void DeleteObj()
{
   if(Selection.elms())
   {
      REPAO(Selection).remove();
      WorldEdit.objTransChanged();
   }
}
void DeleteObj(Obj &obj)
{
   bool sel=obj.selected;
   obj.remove();
   if(sel)WorldEdit.objTransChanged();
}
void ScaleOrgObj()
{
   if(Selection.elms())
   {
      Map<UID, flt> scaled(Compare);
      REPA(Selection)
      {
         Obj &obj=Selection[i];
         UID  base_id=obj.params.base.id();
         if(!scaled.find(base_id)) // if ELM_OBJ was not yet scaled due to world object
         {
            flt scale=obj.matrix.avgScale(); if(!Equal(scale, 1))
            {
               obj.matrix.orn()/=scale; obj.setChanged(true);
               Proj.transformApply(base_id, Matrix(scale)); *scaled(base_id)=scale;
            }
         }
      }
      if(scaled.elms())WorldEdit.objTransChanged();
   }
}
void ObjAlignTer () {WorldEdit.obj_pos.apply(); REPAO(Selection).alignTerrain(true); WorldEdit.objTransChanged();}
void ObjAlignNrm () {WorldEdit.obj_pos.apply(); REPAO(Selection).alignNormal (    ); WorldEdit.objTransChanged();}
void ObjAlignGrid() {WorldEdit.obj_pos.apply(); REPA (Selection){Obj &obj=Selection[i]; bool ground=obj.onGround(); obj.alignGrid(); if(ground && WorldEdit.obj_hm_align())obj.alignTerrain(false); if(ground && WorldEdit.hm_align_nrm)obj.alignNormal();} WorldEdit.objTransChanged();}

void ObjRandomRot() {REPAO(Selection).randomRot(); WorldEdit.objTransChanged();}
void ObjResetRot () {REPAO(Selection). resetRot(); WorldEdit.objTransChanged();}

void ObjRotX() {Matrix3 m; m.setRotateX(PI_2); REPA(Selection){Obj &obj=Selection[i]; obj.moveTo(obj.matrix.orn()*m);} WorldEdit.objTransChanged();}
void ObjRotY() {Matrix3 m; m.setRotateY(PI_2); REPA(Selection){Obj &obj=Selection[i]; obj.moveTo(obj.matrix.orn()*m);} WorldEdit.objTransChanged();}
void ObjRotZ() {Matrix3 m; m.setRotateZ(PI_2); REPA(Selection){Obj &obj=Selection[i]; obj.moveTo(obj.matrix.orn()*m);} WorldEdit.objTransChanged();}

void EditObj          () {WorldEdit.param_edit.b_class.push();}
void CopyObjInstanceID() {Str s; FREPA(Selection){s+=Selection[i].id.asCString(); if(Selection.elms()>1)s+=",\n";} ClipSet(s);}
void OpenObjMaterial  ()
{
   Memt<MaterialPtr> mtrls;
   FREPA(Selection)
   {
      Obj &obj=Selection[i]; if(C MeshPtr &mesh=obj.mesh)
      {
         FREPD (v, mesh->variations()) // iterate variations first
         FREPAD(p, mesh->parts       ) // iterate all parts
            if(C MaterialPtr &mtrl=mesh->parts[p].variation(v))mtrls.include(mtrl); // use 'include' and not 'binaryInclude' to preserve order
      }
   }
   MtrlEdit.set(mtrls);
}
/******************************************************************************/
