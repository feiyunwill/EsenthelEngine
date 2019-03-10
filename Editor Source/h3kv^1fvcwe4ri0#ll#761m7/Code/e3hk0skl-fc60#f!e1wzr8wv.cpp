/******************************************************************************/
class RayTest // first all areas are added, then they are sorted depending on distance to camera, to test closer objects first
{
   class Area
   {
      .Area *area=null;
      int    dist=0;

      Area(.Area &area, int dist) {T.area=&area; T.dist=dist;}
   }
   static int Compare(C Area &a, C Area &b) {return .Compare(a.dist, b.dist);}

   bool       hit=false, on_heightmap=false, heightmaps=true, objects=true;
   flt        frac=0;
   VecI2      center=0;
   Vec        from, move;
   Obj       *obj=null;
   SyncLock   lock;
   Memc<Area> areas; // areas to process
   Memc<Obj*> objs2D; // 2d objects

   void clear()
   {
      areas .clear();
      objs2D.clear();
      T.hit       =false;
      T.heightmaps=false;
      T.objects   =false;
   }
   void init(C VecI2 &center, C Vec &from, C Vec &move, bool heightmaps, bool objects)
   {
      T.center=center; T.from=from; T.move=move; T.heightmaps=heightmaps; T.objects=objects;
   }

   static void ListAreas(Cell<.Area> &cell, RayTest &ray_test) // add areas sorted by distance to center
   {
      Area area(cell(), Dist2(cell.x()-ray_test.center.x, cell.y()-ray_test.center.y));
      ray_test.areas.binaryAdd(area, Compare); // can't use 'binaryInclude' because it won't add multiple areas of same distance
   }
   static void Test( Area &area, RayTest &rt, int thread_index) {rt.test(*area.area);}
          void test(.Area &area) // !! this function should be multi-threaded safe !!
   {
      bool gpu_ctx=false;
      flt  frac;

      if(heightmaps && WorldEdit.visible_area.includes(area.xy))if(Heightmap2 *hm=area.hm)
      {
         Matrix matrix=MatrixIdentity;
         Vec    from  =T.from/matrix,
                move  =T.move/matrix.orn();
         if(SweepPointBox(from, move, hm.mesh.ext, &frac))
            if(possibleHit(frac)) // proceed with the mesh testing only if it's possible that the collision will be closer than current collisions
         {
            if(!gpu_ctx){gpu_ctx=true; ThreadMayUseGPUData();} // we're about to test mesh
            if(MtrlBrush.hole()) // in hole mode we can't use mesh testing, because the mesh keeps on being removed as we operate it which generates incorrect results
            {
               if(hm.heightImage().raycast(T.from, T.move, &Matrix((Vec)area.xy.xy0(), WorldEdit.areaSize()).swapYZ(), &frac))setHit(frac, true, null);
            }else
            {
               MeshLod &lod=hm.mesh.lod(Mid(1, 0, hm.mesh.lods()-1));
               if(Sweep(from, move, lod, null, &frac))setHit(frac, true, null);
            }
         }
      }

      if(objects && WorldEdit.visible_area_1.includes(area.xy))
         REPA(area.objs)if(Obj *obj=area.objs[i])if(obj.visible)
      {
         if(C MeshPtr &mesh=obj.mesh)if(mesh->is())
         {
            Matrix matrix=obj.drawMatrix();
            Vec    from  =T.from/matrix,
                   move  =T.move/matrix.orn();
            if(SweepPointBox(from, move, mesh->ext, &frac)) // make additional test of point->box sweep (this is done already in point->mesh sweep, however making it here we can perform next line of testing the mesh only if the closest collision point on the box is closer than already detected collisions)
               if(possibleHit(frac)) // proceed with the mesh testing only if it's possible that the collision will be closer than current collisions
            {
               if(!gpu_ctx){gpu_ctx=true; ThreadMayUseGPUData();} // we're about to test mesh
               if(Sweep(from, move, *mesh, null, &frac))setHit(frac, false, obj);
            }
            continue; // continue to next object
         }
         add2DObj(obj); // if the object has no valid mesh then add it to 2d list
      }
   }

   bool test()C {return heightmaps || objects;}

   bool possibleHit(flt frac)C {SyncLocker locker(lock); return !hit || frac<T.frac;}
   void      setHit(flt frac, bool heightmap, Obj *obj)
   {
      SyncLocker locker(lock);
      if(!hit || frac<T.frac)
      {
         T.hit         =true;
         T.frac        =frac;
         T.on_heightmap=heightmap;
         T.obj         =obj;
      }
   }
   void add2DObj(Obj *obj) {SyncLocker locker(lock); objs2D.add(obj);}

   bool process()
   {
      WorkerThreads.process1(areas, Test, T);
      return hit;
   }
}
/******************************************************************************/
void UpdateButtonState(byte *dest, byte *on, int num)
{
   REP(num)
   {
      bool last=FlagTest(*dest, BS_ON);
      *dest++=(*on++ ? (BS_ON|(last?0:BS_PUSHED)) : (last?BS_RELEASED:0));
   }
}
/******************************************************************************/
class Cursor
{
   bool   onViewport ()C {return _view!=null  ;} // if the cursor is on one of the editing world viewports
   bool   valid      ()C {return _valid       ;} // if the cursor points to a valid location , this should be tested before using the cursor positions
   bool   onHeightmap()C {return _on_heightmap;} // if cursor is on a heightmap
 C Vec2 & screenPos  ()C {return _screen_pos  ;} // get cursor screen position
 C Vec  & pos        ()C {return _pos         ;} // get cursor world  position               , this can be used only after testing the 'valid' method
 C VecI2& area       ()C {return _area        ;} // get area world coordinates               , this can be used only after testing the 'valid' method
 C VecI2& xz         ()C {return _xz          ;} // get coordinates according to current grid, this can be used only after testing the 'valid' method
 C Edge2& edge       ()C {return _edge        ;} // get 2D edge on the ground surface, where one point has the position of the cursor in previous frame, while other point has current cursor position, this can be used only after testing the 'valid' method

   bool on()C {return ButtonOn(_state);} // if cursor is on
   bool pd()C {return ButtonPd(_state);} // if cursor is pushed
   bool rs()C {return ButtonRs(_state);} // if cursor is released
   bool db()C {return ButtonDb(_state);} // if cursor is double clicked

   bool notFirstEdit()C {return on() && !pd() && onViewport();}

   Touch* touch()C {return FindTouch(_touch_id);} // get touch linked with this cursor

   Edit.Viewport4.View* view()C {return _view;}

   void update(C Vec2 &screen_pos, GuiObj *go, bool on, bool db, WorldView &we)
   {
      byte byte_on=on; UpdateButtonState(&_state, &byte_on, 1); FlagSet(_state, BS_DOUBLE, db);
         _valid       =false;
         _on_heightmap=false;
         _obj         =null;
         _screen_pos  =screen_pos;
         _pos_prev    =_pos;
         _ray_test    .clear();
      if(_view        =we.v4.getView(go))
         //if(!(we.mode()==WorldView.OBJECT && we.obj_drag_view)) // don't detect cursor when dragging
      {
        _view->setViewportCamera();

         Vec from, dir; ScreenToPosDir(screen_pos, from, dir); from+=dir*D.viewFrom();

         bool force_plane=false;
         flt  plane_y    =we.grid_plane_level;

         if(we.mode()==WorldView.WATER && we.sel_lake && InRange(we.sel_lake_poly, we.sel_lake.polys) && InRange(we.sel_lake_point, we.sel_lake.polys[we.sel_lake_poly]))
         {
            force_plane=true;
            plane_y    =we.sel_lake.polys[we.sel_lake_poly][we.sel_lake_point].y;
         }

         if(!force_plane)
         {
            Vec move=dir*D.viewRange();
           _ray_test.init(we.worldToArea(ActiveCam.matrix.pos), from, move, we.cur_collides_with_hm, we.cur_collides_with_obj && we.mode()!=WorldView.HEIGHTMAP && (ObjPaint.available() ? false : !Ms.b(1)));
            if(_ray_test.test())
            {
               we.grid.func(we.visible_area_1, RayTest.ListAreas, _ray_test); // add areas to ray test
               if(_ray_test.process()) // because of big embedded objects which can span over many areas, we need to test all areas
               {
                 _valid       =true;
                 _on_heightmap=_ray_test.on_heightmap;
                 _obj         =_ray_test.obj;
                 _pos         =from+move*_ray_test.frac;
               }
            }
         }

         if(!_valid && Sign(from.y-plane_y)!=Sign(dir.y) && dir.y)
         {
           _valid=true;
           _pos  =from+dir*Abs((from.y-plane_y)/dir.y); // cast 'pos' onto ground plane at 'plane_y' height position
         }

         if(_valid)
         {
            Clamp(_pos.x, (we.visible_area.min.x)*we.areaSize()+EPSL, (we.visible_area.max.x+1)*we.areaSize()-EPSL);
            Clamp(_pos.z, (we.visible_area.min.y)*we.areaSize()+EPSL, (we.visible_area.max.y+1)*we.areaSize()-EPSL);
            if(we.gridAlignSize())
            {
               if(we.gridAlignRound())_xz.set(Round(_pos.x/we.gridAlignSize()), Round(_pos.z/we.gridAlignSize()));
               else                   _xz.set(Floor(_pos.x/we.gridAlignSize()), Floor(_pos.z/we.gridAlignSize()));
               if(we.gridAlign())
               {
                 _pos.x=_xz.x*we.gridAlignSize();
                 _pos.z=_xz.y*we.gridAlignSize();
               }
            }else
            {
              _xz=Floor(_pos.xz());
            }
            if(_on_heightmap)_pos.y=we.hmHeight(_pos);
           _area=we.worldToArea(_pos);
         }
         if(pd())_pos_prev=_pos;
      }
     _edge.set(_pos.xz(), _pos_prev.xz());

      // get 2D objects
      if(onViewport() && WorldEdit.mode()==WorldView.OBJECT)
      {
         bool per_face=WorldEdit.cur_collides_with_obj, center=WorldEdit.obj_center_points, matrix=WorldEdit.obj_matrix_points, icon=!_ray_test.objects, obj2d=(_ray_test.objects && _ray_test.objs2D.elms()); // don't test icon if we've already gathered list of 2d objects
         if( !per_face && !center && !matrix)matrix=true;
         if(center || matrix || icon || obj2d)
         {
            Obj *lit=null;
            flt  d, dist; Vec2 p; view().setViewportCamera();
            if(center || matrix || icon)REPA(WorldEdit.obj_visible)
            {
               Obj &obj=*WorldEdit.obj_visible[i];
               if( center              && PosToScreen(obj.center(), p)){d=Dist(p, screenPos()); if(!lit || d<dist){dist=d; lit=&obj;}}
               if((matrix || obj.icon) && PosToScreen(obj.pos   (), p)){d=Dist(p, screenPos()); if(!lit || d<dist){dist=d; lit=&obj;}}
            }
            if(obj2d)REPA(_ray_test.objs2D)
            {
               Obj &obj=*_ray_test.objs2D[i];
               if(PosToScreen(obj.pos(), p)){d=Dist(p, screenPos()); if(!lit || d<dist){dist=d; lit=&obj;}}
            }
            if(lit && dist<=(_obj ? 0.05 : 0.1))_obj=lit; // if an object is already highlighted through per-face, then use smaller max-distance for per-point selection
         }
      }
   }
   void removed(Obj &obj) {if(&obj==T._obj)T._obj=null;}

//private:
   bool                 _valid=false, _on_heightmap=false;
   byte                 _state=0;
   uint                 _touch_id=0;
   flt                  _brush_height=0, _brush_height_help=0;
   Vec2                 _screen_pos=0;
   Vec                  _pos=0, _pos_prev=0;
   VecI2                _area=0, _xz=0;
   Edge2                _edge=0;
   Obj                 *_obj=null;
   Edit.Viewport4.View *_view=null;
   RayTest              _ray_test;
}
/******************************************************************************/
