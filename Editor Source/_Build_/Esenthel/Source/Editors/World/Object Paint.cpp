/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ObjPaintClass ObjPaint;
/******************************************************************************/

/******************************************************************************/
      void ObjPaintClass::Mtrl::update(C GuiPC &gpc)
{
         ::EE::GuiObj::update(gpc);
         remove.visible(Gui.ms()==this || Gui.ms()==&remove);
      }
   void ObjPaintClass::Hide(ObjPaintClass &op) {WorldEdit.obj_paint.set(false);}
   Str  ObjPaintClass::ElmName(C UID &elm_id) {return Proj.elmFullName(elm_id);}
   void ObjPaintClass::ClearObjs(ObjPaintClass &op) {op.clearObjs();}
   void ObjPaintClass::RemoveObj(ptr   p   ) {ObjPaint.removeObj (intptr(p));}
   void ObjPaintClass::RemoveMtrl(Mtrl &mtrl) {ObjPaint.removeMtrl(mtrl);}
   void ObjPaintClass::GetMtrlA(ptr) {ObjPaint.disallowed_mtrls_g.set(false, QUIET);}
   void ObjPaintClass::GetMtrlD(ptr) {ObjPaint.   allowed_mtrls_g.set(false, QUIET);}
   void ObjPaintClass::Copy(ObjPaintClass &op) {op. copyDo();}
   void ObjPaintClass::Paste(ObjPaintClass &op) {op.pasteDo();}
   bool ObjPaintClass::ready()C {return !allowed_mtrls_g() && !disallowed_mtrls_g();}
   bool ObjPaintClass::available()C {return WorldEdit.obj_paint() && !Selection.elms() && WorldEdit.mode()==WorldView::OBJECT;}
   void ObjPaintClass::clearObjs()
   {
      objects_remove.clear();
      objects_list  .clear();
      objects_data  .clear();
   }
   void ObjPaintClass::clearProj()
   {
      clearObjs();
         allowed_mtrls.clear();
      disallowed_mtrls.clear();
   }
   void ObjPaintClass::removeObj(int i)
   {
      if(InRange(i, objects_data))
      {
         objects_data.remove(i, true);
         toGuiObjs();
      }
   }
   void ObjPaintClass::removeMtrl(Mtrl &mtrl)
   {
         allowed_mtrls.removeData(&mtrl, true); toGuiMtrls(   allowed_mtrls,    allowed_mtrls_r);
      disallowed_mtrls.removeData(&mtrl, true); toGuiMtrls(disallowed_mtrls, disallowed_mtrls_r);
   }
   void ObjPaintClass::toGuiObjs()
   {
      FREPA(objects_data)
      {
         objects_data[i].obj=Proj.gamePath(objects_data[i].id);
         objects_data[i].elm=Proj.findElm (objects_data[i].id, ELM_OBJ);
      }
      objects_list.setData(objects_data);
      objects_remove.clear().setNum(objects_data.elms()); // allocate all up fron because of const_mem_addr
      FREPA(objects_remove)
      {
         objects_r+=objects_remove[i].create(Rect_LU(0, -i*objects_list.elmHeight(), objects_list.columnWidth(0), objects_list.elmHeight())).func(RemoveObj, ptr(i)).desc("Remove this object");
         objects_remove[i].image="Gui/close.img";
      }
   }
   void ObjPaintClass::toGuiMtrls(Memx<Mtrl> &mtrls, Region &region)
   {
      const int cols=4;
      const flt size=(region.rect().w()-region.slidebarSize())/cols;
      FREPA(mtrls)
      {
         int x=i%cols, y=i/cols;
         mtrls[i].mtrl=Proj.gamePath(mtrls[i].id);
         region+=mtrls[i].create(Rect_LU(x, -y, 1, 1)*size, MaterialImage(mtrls[i].mtrl)).desc(Proj.elmFullName(mtrls[i].id));
         mtrls[i].color=MaterialColor(mtrls[i].mtrl);
         mtrls[i].alpha_mode=ALPHA_NONE;
         region+=mtrls[i].remove.create(Rect_RU(mtrls[i].rect().ru(), 0.04f, 0.04f)).func(RemoveMtrl, mtrls[i]).hide(); mtrls[i].remove.image="Gui/close.img";
      }
   }
   void  ObjPaintClass::copyDo() {TextData t; FileText f; save(t); t.save(f.writeMem()); ClipSet(f.rewind().getAll());}
   void ObjPaintClass::pasteDo() {TextData t; FileText f; f.writeMem().putText(ClipGet()); t.load(f.rewind()); load(t);}
   void ObjPaintClass::save(TextData &data)C
   {
      SaveProperties(props, data.nodes);
      TextNode &objects=data.getNode("Objects");
      FREPA(objects_data)
      {
         objects.nodes.New().setValue(objects_data[i].id.asCString());
      }
      TextNode &allowed_mtrls=data.getNode("Allowed Materials");
      FREPA(T.allowed_mtrls)
      {
         allowed_mtrls.nodes.New().setValue(T.allowed_mtrls[i].id.asCString());
      }
      TextNode &disallowed_mtrls=data.getNode("Disallowed Materials");
      FREPA(T.disallowed_mtrls)
      {
         disallowed_mtrls.nodes.New().setValue(T.disallowed_mtrls[i].id.asCString());
      }
   }
   void ObjPaintClass::load(C TextData &data)
   {
      clearProj();
      LoadProperties(props, ConstCast(data.nodes));
      if(C TextNode *objects=data.findNode("Objects"))FREPA(objects->nodes)
      {
         UID id; if(id.fromText(objects->nodes[i].value))includeObjs(id);
      }
      if(C TextNode *allowed_mtrls=data.findNode("Allowed Materials"))FREPA(allowed_mtrls->nodes)
      {
         UID id; if(id.fromText(allowed_mtrls->nodes[i].value))includeMtrls(id, T.allowed_mtrls);
      }
      if(C TextNode *disallowed_mtrls=data.findNode("Disallowed Materials"))FREPA(disallowed_mtrls->nodes)
      {
         UID id; if(id.fromText(disallowed_mtrls->nodes[i].value))includeMtrls(id, T.disallowed_mtrls);
      }
      toGuiMtrls(   allowed_mtrls,    allowed_mtrls_r);
      toGuiMtrls(disallowed_mtrls, disallowed_mtrls_r);
      toGuiObjs();
   }
   ObjPaintClass& ObjPaintClass::create()
   {
      ListColumn lc_obj[]=
      {
         ListColumn(DATA_NONE, 0, 0, 0.045f   , "remove"),
         ListColumn(ElmName        , LCW_DATA, "name"  ),
      };
      ts.reset().size=0.042f; ts.align.set(1, 1);
      add("Collision Radius"    , MEMBER(ObjPaintClass, collision   )).range(0  , 20).desc("Collision radius factor used when testing if a new object should be painted next to another one");
      add("Random Scale"        , MEMBER(ObjPaintClass, random_scale)).range(0  ,  5).desc("Random scale applied to objects when painting");
      add("Scale Multiplier"    , MEMBER(ObjPaintClass, scale_mul   )).range(0.1f, 10).desc("Constant scale factor applied to objects when painting").mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Terrain Normal Align", MEMBER(ObjPaintClass, align_normal)).range(0  ,  1).desc("How much align the object to terrain normal vector\n0 = don't align\n1 = fully align").mouseEditSpeed(0.3f);
      autoData(this);
      Rect r=::PropWin::create("Object Paint", Vec2(0.02f, -0.01f), 0.036f, 0.043f, 0.15f); hide(); button[2].show().func(Hide, T);
      flt  h=ts.lineHeight(), w=0.46f, p=h*1.3f, y=r.min.y-p;
      T+=        objects_t    .create(Vec2(0.02f, y), "Objects", &ts);
      T+=        objects_r    .create(Rect_LU(0.02f, y, w, 0.25f)); y=objects_r.rect().min.y-p;
      T+=        objects_clear.create(Rect_RD(objects_r.rect().ru(), 0.12f, 0.045f), "Clear").func(ClearObjs, T).desc("Remove all objects from the list");
      objects_r+=objects_list .create(lc_obj, Elms(lc_obj), true).elmHeight(0.036f).textSize(0, 1).desc("Drag and drop objects here so they can be used during painting");
      T+=   allowed_mtrls_t.create(Vec2(0.02f, y), "Allowed Materials", &ts);
      T+=   allowed_mtrls_r.create(Rect_LU(0.02f, y, w, 0.16f)).desc("Drag and drop materials here to specify on which terrain materials painting is allowed\nSet empty to allow painting on all materials"); y=allowed_mtrls_r.rect().min.y-p;
      T+=   allowed_mtrls_g.create(Rect_RD(allowed_mtrls_r.rect().ru(), h)).func(GetMtrlA).focusable(false).desc("Enable this button and then:\nLeftClick on the terrain to add its material to the list\nRightClick on the terrain to remove its material from the list"); allowed_mtrls_g.image="Gui/Misc/eye_drop.img"; allowed_mtrls_g.mode=BUTTON_TOGGLE;
      T+=disallowed_mtrls_t.create(Vec2(0.02f, y), "Disallowed Materials", &ts);
      T+=disallowed_mtrls_r.create(Rect_LU(0.02f, y, w, 0.16f)).desc("Drag and drop materials here to specify on which terrain materials painting is disallowed"); y=disallowed_mtrls_r.rect().min.y-p+h;
      T+=disallowed_mtrls_g.create(Rect_RD(disallowed_mtrls_r.rect().ru(), h)).func(GetMtrlD).focusable(false).desc("Enable this button and then:\nLeftClick on the terrain to add its material to the list\nRightClick on the terrain to remove its material from the list"); disallowed_mtrls_g.image="Gui/Misc/eye_drop.img"; disallowed_mtrls_g.mode=BUTTON_TOGGLE;
      T+=copy .create(Rect_U(0.02f+w*0.25f, y, 0.18f, 0.05f), "Copy" ).func(Copy , T);
      T+=paste.create(Rect_U(0.02f+w*0.75f, y, 0.18f, 0.05f), "Paste").func(Paste, T); y=copy.rect().min.y;
      Vec2 padd=defaultInnerPaddingSize()+0.02f; rect(Rect_R(D.w(), 0, w+padd.x+0.02f, -y+padd.y));
      return T;
   }
   void ObjPaintClass::update(C GuiPC &gpc)
{
      visible(StateActive==&StateProject && Mode()==MODE_WORLD && WorldEdit.mode()==WorldView::OBJECT && WorldEdit.obj_paint());
      if(visible() && available())
      {
         if(ready())
         {
            if(objects_data.elms() && WorldEdit.cur.valid())
               if(Ms.b(1) || Ms.b(0))
            {
               bool rotate   =true, // random rotate
                    grid     =WorldEdit.gridAlign(),
                    heightmap=WorldEdit.hm_align;
               flt  collision_radius=T.collision*0.5f;

               flt area=0; int area_count=0;
               REPA(objects_data)if(C ObjectPtr &obj=objects_data[i].obj)if(obj->mesh())
               {
                  Vec2 size=(obj->mesh()->ext.size()*obj->scale3()).xz()*scale_mul; // don't include random scale here
                  area+=size.x*size.y;
                  area_count++;
               }
               if(area_count)area/=area_count;

               flow+=Time.d()*Brush.speed*Brush.size*(area ? Mid(40/Sqrt(area), 0.1f, 10.0f) : 1); // make object insert speed dependent on object size

               for(; flow>=1; flow--)
               {
                  if(Ms.b(1)) // insert
                  {
                     Object &object=objects_data[Random(objects_data.elms())];
                     if(C ObjectPtr &base=object.obj)if(object.elm)
                     {
                        Vec  pos     =Brush.randomPos(heightmap);
                        flt  scale   =scale_mul*ScaleFactor(Random.f(-random_scale, random_scale));
                        bool collides=false;
                        if(base->mesh())
                           if(collision_radius>EPS || allowed_mtrls.elms() || disallowed_mtrls.elms() || Brush.slope_b())
                        {
                           flt src_radius=(base->mesh()->ext.size()*base->scale3()).xz().sum()*0.5f*scale;

                           REP(5) // 5 attempts to insert an object
                           {
                              collides=false;

                              if(WorldEdit.cur.onHeightmap() || heightmap)
                              {
                                 if(allowed_mtrls.elms() || disallowed_mtrls.elms())
                                 {
                                    C MaterialPtr &mtrl=WorldEdit.hmMtrl(pos);
                                    if(allowed_mtrls.elms() && !hasMtrl(   allowed_mtrls, mtrl) // if allowed materials are set but isn't included
                                    ||                          hasMtrl(disallowed_mtrls, mtrl))goto collision_detected; // or is on the disallowed list, then skip
                                 }
                                 if(Brush.slope_b())
                                 {
                                    flt s=1-WorldEdit.hmNormalAvg(Matrix(scale*0.5f, pos), base->mesh() ? base->mesh()->ext : Extent(0.5f)).y, slope=Brush.slope.cos(); // scale/2 is important especially for trees with big boxes
                                    if(Brush.slope.side ? s>slope : s<slope)goto collision_detected;
                                 }
                              }
                              if(collision_radius>EPS)
                              {
                                 Rect  rect =pos.xz(); rect.extend(src_radius*collision_radius + Sqrt(area)*0.5f); // Sqrt(area)*0.5f = average object radius
                                 RectI recti=WorldEdit.worldToArea(rect);

                                 for(int y=recti.min.y; y<=recti.max.y; y++)
                                 for(int x=recti.min.x; x<=recti.max.x; x++)if(Cell<Area> *cell=WorldEdit.grid.find(VecI2(x, y)))REPA(cell->data()->objs)
                                    if(Obj *obj=cell->data()->objs[i])if(obj->mesh && !obj->removed)
                                 {
                                    flt test_radius=(obj->mesh->ext*obj->drawMatrix()).size().xz().sum()*0.5f;
                                    if(Dist2(obj->matrix.pos, pos) <= Sqr((src_radius+test_radius)*collision_radius)) // if colliding
                                       REPA(objects_data)if(objects_data[i].obj && objects_data[i].obj->mesh()==obj->mesh_proper) // if on the list
                                         goto collision_detected;
                                 }
                              }
                              break; // no collision
                           collision_detected:
                              collides=true;
                              pos     =Brush.randomPos(heightmap); // try new position
                           }
                        }

                        if(!collides)if(Obj *obj=NewObj(pos, *object.elm))
                        {
                                        obj->scaleBy     (scale);
                           if(rotate   )obj->randomRot   (     );
                           if(grid     )obj->alignGrid   (     );
                           if(heightmap)obj->alignTerrain(false);
                                        obj->alignNormal (1    , align_normal);
                        }
                     }
                  }else // remove
                  {
                     Vec   pos    =Brush.randomPos(heightmap);
                     RectI rect   =Brush.affectedAreas();
                     Obj  *nearest=null;
                     flt   dist;
                     for(int y=rect.min.y; y<=rect.max.y; y++)
                     for(int x=rect.min.x; x<=rect.max.x; x++)if(Cell<Area> *cell=WorldEdit.grid.find(VecI2(x, y)))REPA(cell->data()->objs)
                        if(Obj *obj=cell->data()->objs[i])if(!obj->removed)
                     {
                        flt d=Dist2(obj->matrix.pos, pos);
                        if(!nearest || d<dist)
                           if(Brush.power(obj->drawMatrix().pos.xz())>EPS_COL)
                              REPA(objects_data)if(objects_data[i].obj && objects_data[i].obj->mesh()==obj->mesh_proper)
                        {
                           nearest=obj;
                           dist   =d;
                           break;
                        }
                     }
                     if(nearest)DeleteObj(*nearest);
                  }
               }
            }
         }else
         if(WorldEdit.cur.onHeightmap() && (Ms.bp(0) || Ms.bp(1))) // add/remove terrain mtrl to/from list
         {
            UID mtrl_id=WorldEdit.hmMtrl(WorldEdit.cur.pos()).id();
            if( mtrl_id.valid())
            {
               if(Ms.bp(0)) // add
               {
                  if(   allowed_mtrls_g() && !hasMtrl(   allowed_mtrls, mtrl_id)){   allowed_mtrls.New().id=mtrl_id; toGuiMtrls(   allowed_mtrls,    allowed_mtrls_r);}
                  if(disallowed_mtrls_g() && !hasMtrl(disallowed_mtrls, mtrl_id)){disallowed_mtrls.New().id=mtrl_id; toGuiMtrls(disallowed_mtrls, disallowed_mtrls_r);}
               }else // remove
               {
                  if(   allowed_mtrls_g())REPA(   allowed_mtrls)if(   allowed_mtrls[i].id==mtrl_id){   allowed_mtrls.removeValid(i, true); toGuiMtrls(   allowed_mtrls,    allowed_mtrls_r);}
                  if(disallowed_mtrls_g())REPA(disallowed_mtrls)if(disallowed_mtrls[i].id==mtrl_id){disallowed_mtrls.removeValid(i, true); toGuiMtrls(disallowed_mtrls, disallowed_mtrls_r);}
               }
            }
         }
      }
      ::EE::ClosableWindow::update(gpc); // now update list
   }
   void ObjPaintClass::erasing(C UID &elm_id)
   {
      REPA(   objects_data )if(   objects_data [i].id==elm_id){   objects_data .remove     (i, true); toGuiObjs ();}
      REPA(   allowed_mtrls)if(   allowed_mtrls[i].id==elm_id){   allowed_mtrls.removeValid(i, true); toGuiMtrls(   allowed_mtrls,    allowed_mtrls_r);}
      REPA(disallowed_mtrls)if(disallowed_mtrls[i].id==elm_id){disallowed_mtrls.removeValid(i, true); toGuiMtrls(disallowed_mtrls, disallowed_mtrls_r);}
   }
   bool     ObjPaintClass::hasMtrl(Memx<Mtrl> &mtrls, C MaterialPtr &mtrl   )C {REPA(mtrls)if(mtrls[i].mtrl==mtrl   )return true; return false;}
   bool     ObjPaintClass::hasMtrl(Memx<Mtrl> &mtrls, C UID         &mtrl_id)C {REPA(mtrls)if(mtrls[i].id  ==mtrl_id)return true; return false;}
   bool ObjPaintClass::includeMtrls(C MemPtr<UID> &elms, Memx<Mtrl> &mtrls)
   {
      bool changed=false;
      FREPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_MTRL))if(!hasMtrl(mtrls, elm->id))
      {
         changed=true; mtrls.New().id=elm->id;
      }
      return changed;
   }
   bool     ObjPaintClass::hasObj(C UID &obj_id)C {REPA(objects_data)if(objects_data[i].id==obj_id)return true; return false;}
   bool ObjPaintClass::includeObjs(C MemPtr<UID> &elms)
   {
      bool changed=false;
      FREPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_OBJ))if(!hasObj(elm->id))
      {
         changed=true; objects_data.New().id=elm->id;
      }
      return changed;
   }
   void ObjPaintClass::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(focus_obj==&objects_list)
      {
         if(includeObjs(elms))toGuiObjs();
      }else
      if(allowed_mtrls_r.contains(focus_obj))
      {
         if(includeMtrls(elms, allowed_mtrls))toGuiMtrls(allowed_mtrls, allowed_mtrls_r);
      }else
      if(disallowed_mtrls_r.contains(focus_obj))
      {
         if(includeMtrls(elms, disallowed_mtrls))toGuiMtrls(disallowed_mtrls, disallowed_mtrls_r);
      }
   }
ObjPaintClass::ObjPaintClass() : collision(1), random_scale(0), scale_mul(1), align_normal(1), flow(0) {}

ObjPaintClass::Object::Object() : id(UIDZero), elm(null) {}

ObjPaintClass::Mtrl::Mtrl() : id(UIDZero) {}

/******************************************************************************/
