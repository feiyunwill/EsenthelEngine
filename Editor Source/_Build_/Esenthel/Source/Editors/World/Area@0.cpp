/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
flt HmHeight(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec  &pos, bool smooth) {return HmHeight(hm, area_size, area_xy, pos.xz(), smooth);}
flt HmHeight(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec2 &xz , bool smooth)
{
   flt fx=(xz.x/area_size-area_xy.x)*(hm.resolution()-1),
       fy=(xz.y/area_size-area_xy.y)*(hm.resolution()-1);
   return (smooth ? hm.heightLinear(fx, fy) : hm.height(Round(fx), Round(fy)))*area_size;
}

Vec HmNormal(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec  &pos) {return HmNormal(hm, area_size, area_xy, pos.xz());}
Vec HmNormal(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec2 &xz )
{
   flt fx=(xz.x/area_size-area_xy.x)*(hm.resolution()-1),
       fy=(xz.y/area_size-area_xy.y)*(hm.resolution()-1);

   Vec nrm;

   flt l, r;
   l=hm.heightLinear(fx-1, fy);
   r=hm.heightLinear(fx+1, fy);
   nrm.x=l-r;

   flt b, f;
   b=hm.heightLinear(fx, fy-1);
   f=hm.heightLinear(fx, fy+1);
   nrm.z=b-f;

   nrm.x*=hm.resolution(); nrm.z*=hm.resolution(); nrm.y=2; nrm.normalize();
   return nrm;
}

Vec HmNormalAvg(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec  &pos, flt r) {return HmNormalAvg(hm, area_size, area_xy, pos.xz(), r);}
Vec HmNormalAvg(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec2 &xz , flt r)
{
   flt fx =(xz.x/area_size-area_xy.x)*(hm.resolution()-1),
       fy =(xz.y/area_size-area_xy.y)*(hm.resolution()-1),
       mx =(   r/area_size          )*(hm.resolution()-1),
       my =(   r/area_size          )*(hm.resolution()-1);
   Vec nrm=0;

   for(int y=-3; y<=3; y++)
   for(int x=-3; x<=3; x++)
   {
      Vec n;

      flt px=fx-x*mx,
          py=fy+y*my;

      flt l, r;
      l=hm.heightLinear(px-1, py);
      r=hm.heightLinear(px+1, py);
      n.x=l-r;

      flt b, f;
      b=hm.heightLinear(px, py-1);
      f=hm.heightLinear(px, py+1);
      n.z=b-f;

      n.x*=hm.resolution(); n.z*=hm.resolution(); n.y=2; n.normalize();
      nrm+=n;
   }

   nrm.normalize();
   return nrm;
}

Vec HmNormalAvg(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Matrix &matrix, C Box &box)
{
   Vec      hm_nrm=HmNormal(hm, area_size, area_xy, matrix.pos);
   Matrix3  hm_matrix; hm_matrix.setUp(hm_nrm);
   Matrix3 obj_matrix=matrix; obj_matrix.scaleL(box.size()*0.5f);
   flt     obj_hm_size=Max(MatrixLength(obj_matrix.x, obj_matrix.y, obj_matrix.z, hm_matrix.x),
                           MatrixLength(obj_matrix.x, obj_matrix.y, obj_matrix.z, hm_matrix.z));
   return HmNormalAvg(hm, area_size, area_xy, matrix.pos, obj_hm_size*0.5f); // additional 0.5 because we're more focusing on the center
}
/******************************************************************************/

/******************************************************************************/
   bool Heightmap2::builded()C {return mesh.is();}
   bool Heightmap2::invalidRefs()C
   {
      REP(materials())if(Proj.invalidRef(material(i).id()))return true;
      return false;
   }
   void Heightmap2::setShader()
   {
      mesh.setShader();
      REP(mesh.lods())
      {
         MeshLod &lod=mesh.lod(i); REPA(lod)lod.parts[i].setShader(RM_BLEND, WorldEdit.hm_shader);
      }
   }
  Area::~Area()
   {
      // unlink with neighbors
      if(_l ){_l ->_r =null; _l =null;}
      if(_r ){_r ->_l =null; _r =null;}
      if(_b ){_b ->_f =null; _b =null;}
      if(_f ){_f ->_b =null; _f =null;}
      if(_lb){_lb->_rf=null; _lb=null;}
      if(_lf){_lf->_rb=null; _lf=null;}
      if(_rb){_rb->_lf=null; _rb=null;}
      if(_rf){_rf->_lb=null; _rf=null;}

      Delete(hm);
      path.del();

      if(world)REPA(objs)world->objs.removeData(objs[i]); if(world==&WorldEdit)ObjList.setChanged(); // delete completely (if used then call before detaching)
      REPAO(objs)->detach(); // detach from area
   }
   Area::Area(C VecI2 &xy, ptr grid_user) : xy(xy), loaded(false), changed(false), invalid_refs(false), hm(null), _l(null), _r(null), _b(null), _f(null), _lb(null), _lf(null), _rb(null), _rf(null), world(null), ver(null), validate_refs_time(FLT_MAX)
   {
      world=(WorldData*)grid_user;
      if(world)if(Cell<Area> *cell=world->grid.find(xy))if(cell->data()==this) // if this area belongs to world.grid, then link with neighbors
      {
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x-1, xy.y  ))){cell->data()->_r =this; _l =cell->data();}
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x+1, xy.y  ))){cell->data()->_l =this; _r =cell->data();}
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x  , xy.y-1))){cell->data()->_f =this; _b =cell->data();}
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x  , xy.y+1))){cell->data()->_b =this; _f =cell->data();}
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x-1, xy.y-1))){cell->data()->_rf=this; _lb=cell->data();}
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x-1, xy.y+1))){cell->data()->_rb=this; _lf=cell->data();}
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x+1, xy.y-1))){cell->data()->_lf=this; _rb=cell->data();}
         if(Cell<Area> *cell=world->grid.find(VecI2(xy.x+1, xy.y+1))){cell->data()->_lb=this; _rf=cell->data();}
      }
   }
   Heightmap2* Area::hm_l() {return _l  ? _l ->hm : null;}
   Heightmap2* Area::hm_r() {return _r  ? _r ->hm : null;}
   Heightmap2* Area::hm_b() {return _b  ? _b ->hm : null;}
   Heightmap2* Area::hm_f() {return _f  ? _f ->hm : null;}
   Heightmap2* Area::hm_lb() {return _lb ? _lb->hm : null;}
   Heightmap2* Area::hm_lf() {return _lf ? _lf->hm : null;}
   Heightmap2* Area::hm_rb() {return _rb ? _rb->hm : null;}
   Heightmap2* Area::hm_rf() {return _rf ? _rf->hm : null;}
   bool Area::hasSelectedObj()C {REPA(objs)if(objs[i]->selected)return true; return false;}
   Obj* Area::findobj(C UID &obj_id) {REPA(objs)if(objs[i]->id==obj_id)return objs[i]; return null;}
   Vec2 Area::pos2D()C {return xy*WorldEdit.areaSize();}
   flt Area::hmHeight(C Vec  &pos, bool smooth)C {return      hmHeight(pos.xz(), smooth);}
   flt Area::hmHeight(C Vec2 &xz , bool smooth)C {return hm ? HmHeight(*hm, WorldEdit.area_size, xy, xz, smooth) : 0;}
   Vec Area::hmNormal(C Vec  &pos)C {return      hmNormal(pos.xz());}
   Vec Area::hmNormal(C Vec2 &xz )C {return hm ? HmNormal(*hm, WorldEdit.area_size, xy, xz) : Vec(0, 1, 0);}
   Vec Area::hmNormalAvg(C Vec  &pos, flt r)C {return      hmNormalAvg(pos.xz(), r);}
   Vec Area::hmNormalAvg(C Vec2 &xz , flt r)C {return hm ? HmNormalAvg(*hm, WorldEdit.area_size, xy, xz, r) : Vec(0, 1, 0);}
   Vec Area::hmNormalAvg(C Matrix &matrix, C Box &box)C {return hm ? HmNormalAvg(*hm, WorldEdit.area_size, xy, matrix, box) : Vec(0, 1, 0);}
   Vec Area::hmNormalNeighbor(int x, int y)
   {
      // WARNING: no corner heightmaps are checked
      int res=WorldEdit.hmRes();
      Vec nrm;
      if(x==0    )nrm.x=(hm_l() ? hm_l()->height(res-2, y)-hm    ->height(1, y) : (hm->height(    0, y)-hm->height(    1, y))*2);else
      if(x==res-1)nrm.x=(hm_r() ? hm    ->height(res-2, y)-hm_r()->height(1, y) : (hm->height(res-2, y)-hm->height(res-1, y))*2);else
                  nrm.x=                                                        (hm->height(  x-1, y)-hm->height(  x+1, y))   ;

      if(y==0    )nrm.z=(hm_b() ? hm_b()->height(x, res-2)-hm    ->height(x, 1) : (hm->height(x,     0)-hm->height(x,     1))*2);else
      if(y==res-1)nrm.z=(hm_f() ? hm    ->height(x, res-2)-hm_f()->height(x, 1) : (hm->height(x, res-2)-hm->height(x, res-1))*2);else
                  nrm.z=                                                        (hm->height(x,   y-1)-hm->height(x,   y+1))   ;

      nrm.x*=res; nrm.z*=res; nrm.y=2; nrm.normalize();
      return nrm;
   }
   bool Area::hmColor(C Vec2 &xz, Vec &color)C
   {
      if(hm)
      {
         int x=Floor((xz.x/WorldEdit.areaSize()-T.xy.x)*(hm->resolution()-1)),
             y=Floor((xz.y/WorldEdit.areaSize()-T.xy.y)*(hm->resolution()-1));
         color=hm->colorF(x, y); return true;
      }
      color=1; return false;
   }
   MaterialPtr Area::hmMtrl(C Vec2 &xz, C MaterialPtr &cur_mtrl)C
   {
      if(hm)
      {
         int x=Floor((xz.x/WorldEdit.areaSize()-T.xy.x)*(hm->resolution()-1)),
             y=Floor((xz.y/WorldEdit.areaSize()-T.xy.y)*(hm->resolution()-1));

         MaterialPtr mtrl[4];
         Vec4        blend;
         hm->getMaterial(x, y, mtrl[0], mtrl[1], mtrl[2], mtrl[3], blend);

         if(cur_mtrl) // we already have some material selected
         {
            FREPA(mtrl)if(mtrl[i]==cur_mtrl) // selected is on the list
               FREPD(j, Elms(mtrl)-1)
            {
               int k=(i+j+1)%Elms(mtrl);
               if(blend.c[k]>EPS_COL && mtrl[k])return mtrl[k]; // get the first next one which exists
            }
         }
         return mtrl[blend.maxI()];
      }
      return null;
   }
      AreaVer*          Area::getVer(                      ) {if(!ver && world && world->ver)ver=world->ver->areas.get(xy); return ver;}
    C AreaVer*          Area::getVer(                      )C{return ConstCast(T).getVer();}
      void Area::setTerrainUndo(                      ) {if(world)WorldEdit.undos.setTerrain(T);}
      void Area::setServer(                      ) {if(world)Synchronizer.delayedSetArea(world->elm_id, xy);}
      void Area::setChanged(                      ) {changed=true; if(world && world->ver)world->ver->changed=true;}
      void Area::setChangedHmRemoved(C TimeStamp *time) {if(!time)setTerrainUndo(); setChanged(); if(AreaVer *ver=getVer())if(time)ver->hm_removed_time=*time;else{ver->hm_removed_time=CurTime; setServer();} if(world && world->ver)world->ver->rebuildArea(xy, AREA_SYNC_REMOVED, true     );}
      void Area::setChangedHeight(bool skip_mesh   ) {         setTerrainUndo(); setChanged(); if(AreaVer *ver=getVer())                                      {ver-> hm_height_time=CurTime; setServer();} if(world && world->ver)world->ver->rebuildArea(xy, AREA_SYNC_HEIGHT , skip_mesh);}
      void Area::setChangedMtrl(                      ) {         setTerrainUndo(); setChanged(); if(AreaVer *ver=getVer())                                      {ver->   hm_mtrl_time=CurTime; setServer();} if(world && world->ver)world->ver->rebuildArea(xy, AREA_SYNC_MTRL   , true     );}
      void Area::setChangedColor(                      ) {         setTerrainUndo(); setChanged(); if(AreaVer *ver=getVer())                                      {ver->  hm_color_time=CurTime; setServer();} if(world && world->ver)world->ver->rebuildArea(xy, AREA_SYNC_COLOR  , true     );}
      void Area::setChangedObj(                      ) {                           setChanged(); if(AreaVer *ver=getVer())                                      {ver->    obj_ver.randomize();             }}
      void Area::setChangedObj(Obj &obj              ) {                           setChangedObj(); if(world && world->ver){world->ver->changedObj(obj, xy); Synchronizer.delayedSetObj(world->elm_id, obj.id);} delayedValidateRefs();}
   void Area::setShader() {if(hm)hm->setShader();}
   bool Area::hmDel(C TimeStamp *time)
   {
      if(loaded && hm)
      {
         setChangedHmRemoved(time);
         Delete(hm);
         return true;
      }
      return false;
   }
   bool Area::hmCreate(bool align_height_to_neighbors)
   {
      if(loaded && !hm)
      {
         setChangedHeight();
         setChangedMtrl  ();
         New(hm)->create(WorldEdit.hmRes(), WorldEdit.grid_plane_level/WorldEdit.areaSize(), WorldEdit.hm_mtrl, align_height_to_neighbors, hm_l(), hm_r(), hm_b(), hm_f(), hm_lb(), hm_lf(), hm_rb(), hm_rf());
         return true;
      }
      return false;
   }
   void Area::hmBuild()
   {
      if(loaded && hm)
      {
         Build(*hm, hm->mesh, WorldEdit.areaSize(), xy, hm_l(), hm_r(), hm_b(), hm_f(), hm_lb(), hm_lf(), hm_rb(), hm_rf());
         setShader();
      }
   }
   bool Area::invalidRefs()C
   {
      bool invalid=false;
      REPA(objs)if(Obj *obj=objs[i])invalid|=obj->setInvalidRefs();
      if(hm)invalid|=hm->invalidRefs();
      return invalid;
   }
   void        Area::validateRefs() {invalid_refs=invalidRefs(); validate_refs_time=FLT_MAX;}
   void Area::delayedValidateRefs() {MIN(validate_refs_time, Time.appTime()+2);}
   void  Area::updateValidateRefs() {if(Time.appTime()>=validate_refs_time)validateRefs();}
   void Area::undo(C WorldChange::Terrain &undo, bool skip_mesh)
   {
      if(AreaVer *ver=getVer())
      {
         uint changed=0;

         if(undo.is()) // create heightmap
         {
            if(!hm)New(hm);
            changed|=ver->undo(undo.ver, *hm, undo);
         }else // remove heightmap
         {
            Delete(hm);
         }
         changed|=ver->setHm(undo.is());

         if(changed)
         {
            WorldEdit.ver->rebuildArea(xy, changed, skip_mesh);
            setChanged();
            setServer();
         }
      }
   }
   void Area::draw()
   {
      if(loaded)
      {
         if(hm && Frustum(hm->mesh))
         {
            bool lit=(WorldEdit.mode()==WorldView::HEIGHTMAP && WorldEdit.hm_add_rem() && WorldEdit.cur.valid() && Cuts(xy, RectI(WorldEdit.cur.xz()).extend(WorldEdit.hm_sel_size)));
            SetHighlight(lit ? Color(32, 32, 32, 0) : TRANSPARENT); SetStencilValue(true ); hm->mesh.draw(MatrixIdentity);
            SetHighlight(                             TRANSPARENT); SetStencilValue(false);
            if(WorldEdit.hm_use_shader)scheduleDrawBlend(hm->mesh.ext.center);
         }
      }else
      {
         // TODO: draw as loading
      }
   }
   void Area::drawShadow()
   {
      if(hm && Frustum(hm->mesh))hm->mesh.drawShadow(MatrixIdentity);
   }
   void Area::drawBlend()
{
      if(hm)
      {
         SetBlendAlpha(ALPHA_ADD_KEEP); hm->mesh.drawBlend(MatrixIdentity);
         SetBlendAlpha();
      }
   }
   void Area::draw2D()
   {
      updateValidateRefs();
      if(invalid_refs)
      {
         Vec  pos=(hm ? hm->mesh.ext.center : ((Vec2(xy)+0.5f)*WorldEdit.areaSize()).x0y()+Vec(0, WorldEdit.grid_plane_level, 0));
         Vec2 screen; if(PosToScreen(pos, screen))Proj.exclamation->drawFit(Rect_C(screen, 0.03f));
      }
   }
   bool Area::saveEdit(C Str &name)
   {
      File f; f.writeMem(); ChunkWriter cw(f);
      if(hm)if(File *f=cw.beginChunk("Heightmap", 0)) // must be in sync with 'LoadEditHeightmap'
      {
         hm->clean();
         hm->save(*f, Proj.game_path);
      }
      if(objs.elms())if(File *f=cw.beginChunk("Object", 0)) // must be in sync with 'LoadEditObject'
      {
         f->cmpUIntV(objs.elms()); FREPAO(objs)->save(*f);
      }
      cw.endChunk(); f.pos(0); return SafeOverwriteChunk(f, name, WorldAreaSync);
   }
   void Area::loadEdit(C Str &name)
   {
      Delete(hm);
      ReadLock rl(WorldAreaSync);
      File f; if(f.readTry(name))for(ChunkReader cr(f); File *f=cr(); )
      {
         if(cr.name()=="Heightmap")switch(cr.ver())
         {
            case 0:
            {
               New(hm)->load(*f, Proj.game_path);
            }break;
         }else
         if(cr.name()=="Object")switch(cr.ver())
         {
            case 0: if(world)REP(f->decUIntV())
            {
               Obj &obj=world->objs.New(); if(!obj.load(*f)){world->objs.removeData(&obj); break;} obj.attach(*world, this);
            }break;
         }
      }
   }
   bool Area::saveGame(C Str &name)
   {
      File f; f.writeMem(); ChunkWriter cw(f);
      // !! Chunks must be saved in alphabetic order !! (so hash of file data is always the same)
      if(hm)
      {
         if(hm->height_map.is())if(File *f=cw.beginChunk("Heightmap", 0))
         {
            hm->height_map.save(*f); hm->mtrl_map.save(*f); f->putInt(hm->mtrls.elms()); REPA(hm->mtrls)PutStr(*f, hm->mtrls[i].valid() ? EncodeFileName(hm->mtrls[i]) : S);
         }
         if(hm->mesh.is())if(File *f=cw.beginChunk("HeightmapMesh", 0))
         {
            hm->mesh.save(*f, Proj.game_path);
         }
         if(hm->phys.is())if(File *f=cw.beginChunk("HeightmapPhys", 0))
         {
            hm->phys.save(*f);
         }
      }
      REPA(objs)if(!objs[i]->removed && !objs[i]->embedded()) // if there's at least one existing object
      {
         if(File *f=cw.beginChunk("Object", 1))
         {
            Memt<Game::Area::Data::AreaObj> area_objs; FREPA(objs)if(!objs[i]->removed && !objs[i]->embedded())objs[i]->copyTo(area_objs.New(), Proj);
            area_objs.sort(CompareObj); FREPAO(area_objs).save(*f, Proj.game_path);
         }
         break;
      }
      if(obj_mesh.is())if(File *f=cw.beginChunk("ObjectMesh", 0))
      {
         obj_mesh.save(*f, Proj.game_path);
      }
      if(obj_phys.is())if(File *f=cw.beginChunk("ObjectPhys", 0))
      {
         obj_phys.save(*f);
      }
      if(path.is())if(File *f=cw.beginChunk("PathMesh", 0))
      {
         path.save(*f);
      }
      if(waters.elms())if(File *f=cw.beginChunk("Water", 0))
      {
         Save(*f, waters, Proj.game_path);
      }
      cw.endChunk(); f.pos(0); return SafeOverwriteChunk(f, name, WorldAreaSync);
   }
   void Area::loadGame(C Str &name)
   {
      ReadLock rl(WorldAreaSync);
      File f; if(f.readTry(name))for(ChunkReader cr(f); File *f=cr(); )
      {
         if(cr.name()=="Heightmap")if(hm)switch(cr.ver())
         {
            case 0:
            {
               hm->height_map.load(*f); hm->mtrl_map.load(*f);
               hm->mtrls.setNum(f->getInt()); REPA(hm->mtrls){UID &mtrl=hm->mtrls[i]; if(!DecodeFileName(GetStr(*f), mtrl))mtrl.zero();}
            }break;
         }
         if(cr.name()=="HeightmapMesh")if(hm)switch(cr.ver())
         {
            case 0:
            {
               hm->mesh.load(*f, Proj.game_path); setShader();
            }break;
         }
         if(cr.name()=="HeightmapPhys")if(hm)switch(cr.ver())
         {
            case 0:
            {
               hm->phys.load(*f);
            }break;
         }
         // "Object" is loaded in 'loadEdit'
         if(cr.name()=="ObjectMesh")switch(cr.ver())
         {
            case 0:
            {
               obj_mesh.load(*f, Proj.game_path);
            }break;
         }
         if(cr.name()=="ObjectPhys")switch(cr.ver())
         {
            case 0:
            {
               obj_phys.load(*f, Proj.game_path);
            }break;
         }
         if(cr.name()=="PathMesh")switch(cr.ver())
         {
            case 0:
            {
               path.load(*f); if(world)world->path_world.set(&path, xy);
            }break;
         }
         if(cr.name()=="Water")switch(cr.ver())
         {
            case 0:
            {
               Load(*f, waters, Proj.game_path);
            }break;
         }
      }
   }
   void Area::load()
   {
      if(!loaded)
      {
         loaded=true; // set this at start in case loading objects will cause their area to be loaded
         if(world)
         {
            loadEdit(world->edit_area_path+xy); // load "edit" first (because it creates 'hm'), these are OK to fail in case they were not created yet
            loadGame(world->game_area_path+xy); //                                              these are OK to fail in case they were not created yet
            validateRefs();
            setShader();
            ObjList.setChanged();
         }
      }
   }
   void Area::Flush(Cell<Area> &cell, ptr) {cell().flush();}
          void Area::flush()
   {
      if(loaded && changed)
      {
         if(world)
         {
            saveEdit(world->edit_area_path+xy);
            saveGame(world->game_area_path+xy);
         }
      }
      changed=false;
   }
   void Area::unload()
   {
      flush();
      if(loaded)
      {
         loaded=false;
         if(world)REPA(objs)world->objs.removeData(objs[i]); if(world==&WorldEdit)ObjList.setChanged();
         Delete(hm);
         obj_mesh.del();
         obj_phys.del();
         waters  .del();
         path    .del();
      }
   }
Area::Area() : loaded(false), changed(false), invalid_refs(false), hm(null), _l(null), _r(null), _b(null), _f(null), _lb(null), _lf(null), _rb(null), _rf(null), world(null), ver(null), validate_refs_time(FLT_MAX) {}

/******************************************************************************/
