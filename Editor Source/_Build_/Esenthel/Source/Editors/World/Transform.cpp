/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
void WorldView::editObj()
{
   if(Selection.elms())
   {
      if(cur.onViewport() && Kb.ctrlCmd() && Ms.wheel()) // scale objects matrix
      {
         obj_pos.apply();
         flt mul=ScaleFactor(Ms.wheel()*0.1f*(Kb.shift() ? 0.2f : 1.0f));
         REPAO(Selection).scaleBy(mul);
         objTransChanged();
      }

      int op=obj_op();
      if(Kb.ctrlCmd() && Kb.shift() && !Kb.alt())op=OP_SEPARATE;else
      if(Kb.ctrlCmd()               &&  Kb.alt())op=OP_ROT_GROUP;else
      if(                               Kb.alt())op=OP_ROT_Y;else
      if(Kb.ctrlCmd()                           )op=OP_MOVE_Y;else
      if(Kb.bp(KB_INS)                          )op=OP_INS;
      if(op==OP_ROT_GROUP && Selection.elms()<=1)op=OP_ROT_Y;
      if(op==OP_INS)
      {
         obj_pos.apply();
         if(cur.valid() && (Ms.bp(1) || Kb.kf(KB_INS)))NewObjs(cur.pos());
         REPA(cur_touch)if(cur_touch[i].valid() && cur_touch[i].pd())NewObjs(cur_touch[i].pos());
      }

      Vec2 delta=0;
      bool touch_edit=(obj_op()==OP_MOVE || obj_op()==OP_SCALE || obj_op()==OP_ROT || obj_op()==OP_SEPARATE || obj_op()==OP_MOVE_Y || obj_op()==OP_ROT_Y || obj_op()==OP_ROT_GROUP);
      bool view_on=(cur.onViewport() && Ms.b(1)); if(view_on){delta=Ms.d(); Ms.freeze();} if(touch_edit)REPA(cur_touch)if(cur_touch[i].onViewport() && cur_touch[i].on()){view_on=true; if(Touch *touch=cur_touch[i].touch())delta+=touch->ad();}
      if(  view_on && op!=OP_INS) // edit objects matrix
      {
         bool first_push=true; REP(curTotal())if(curAll(i).notFirstEdit()){first_push=false; break;}

         bool align_angle= gridAlign(),
              align_xz   =(gridAlign() && gridAlignSize ()>0),
              align_y    =(gridAlign() && gridAlignSizeY()>0),
              align_hm   =(obj_hm_align() && op!=OP_MOVE_Y),
              align_nrm  =(hm_align_nrm   && op!=OP_MOVE_Y);
         flt  multiplier =((Kb.shift() && !Kb.ctrlCmd()) ? 0.1f : 1.0f);

         // setup operating values
         Vec center=0; if(Selection.elms()){REPA(Selection)center+=Selection[i].center(); center/=Selection.elms();}
         if(!v4.perspective() || !v4.fpp())obj_edit_speed=CamMoveScale(v4.perspective());else if(first_push)obj_edit_speed=Dist(center, ActiveCam.matrix.pos)*0.75f; flt mul=obj_edit_speed*multiplier; // in FPP mode automatically detect the speed according to average object distance to camera, but only at the moment of first button push

         if(first_push || Kb.bp(KB_LALT) || Kb.bp(KB_RALT))obj_edit_angle=0; obj_edit_angle+=delta.sum()*multiplier;
         if(first_push)obj_edit_vec.zero();

         if(op==OP_MOVE_Y)
         {
            obj_edit_vec.y+=delta.y*mul;
         }else
         REP(curTotal())
         {
            Cursor &cur=curAll(i); Touch *touch=cur.touch();
            if(cur.onViewport() && cur.on() && (touch ? touch_edit : Ms.b(1)))
            {
               cur.view()->setViewportCamera();
               Vec2 cur_delta=(touch ? touch->ad() : Ms.d()); //*MoveScale(*cur.view());
               Vec  x=ActiveCam.matrix.x; x.y=0;     x.normalize();
               Vec  z=ActiveCam.matrix.z; z.y=0; if(!z.normalize())z=ActiveCam.matrix.y;else if(Dot(z, ActiveCam.matrix.y)<0)z.chs();
               obj_edit_vec+=x*(cur_delta.x*mul)
                            +z*(cur_delta.y*mul);
            }
         }

         flt a=0;
         Vec d=0;
         {
            a=(align_angle ? ((Abs(obj_edit_angle)>=PI/12) ? AlignTrunc(obj_edit_angle, PI/12) : 0) : obj_edit_angle);

            if(op==OP_MOVE && obj_axis>=0 && Selection.elms())
            {
               Matrix m=Selection[0].matrix;
               switch(obj_axis)
               {
                  case 0: d=AlignDirToCam(m.x, delta)*mul/3; break;
                  case 1: d=AlignDirToCam(m.y, delta)*mul/3; break;
                  case 2: d=AlignDirToCam(m.z, delta)*mul/3; break;
               }
            }else
            {
               if(!align_xz)
               {
                  d.x=obj_edit_vec.x;
                  d.z=obj_edit_vec.z;
               }else
               {
                  if(Abs(obj_edit_vec.x)>=gridAlignSize()
                  || Abs(obj_edit_vec.z)>=gridAlignSize())
                  {
                     d.x=AlignTrunc(obj_edit_vec.x, gridAlignSize());
                     d.z=AlignTrunc(obj_edit_vec.z, gridAlignSize());
                  }
               }

               if(!align_y)d.y=obj_edit_vec.y;else
               {
                  if(Abs(obj_edit_vec.y)>=gridAlignSizeY())d.y=AlignTrunc(obj_edit_vec.y, gridAlignSizeY());
               }
            }
         }
         if(Abs(d.y)>EPS && d.xz().length()<=EPS)align_hm=false; // if moving only vertically, then disable aligning to heightmap (for example, if we're moving along Y axis, then we want to ignore aligning)

         // edit objects
         {
            obj_pos.apply(); // if we start different mode, then first apply any changes made in 'obj_pos'
            REPA(Selection) // for each object
            {
               Obj &obj=Selection[i];

               if(op==OP_SCALE)switch(obj_axis)
               {
                  case  0: obj.scaleXBy(ScaleFactor(AlignDirToCamEx(obj.matrix.x, delta)     *multiplier)); break;
                  case  1: obj.scaleYBy(ScaleFactor(AlignDirToCamEx(obj.matrix.y, delta)     *multiplier)); break;
                  case  2: obj.scaleZBy(ScaleFactor(AlignDirToCamEx(obj.matrix.z, delta)     *multiplier)); break;
                  default: obj.scaleBy (ScaleFactor(                              delta.sum()*multiplier)); break;
               }else
               {
                  Matrix matrix=obj.matrix;
                  switch(op)
                  {
                     case OP_SEPARATE:
                     {
                        Vec2 d=matrix.pos.xz()-center.xz(); d*=ScaleFactor(delta.sum()*multiplier);
                        matrix.pos.x=center.x+d.x;
                        matrix.pos.z=center.z+d.y;
                     }break;

                     case OP_ROT_GROUP:
                     {
                        matrix.pos-=center; matrix.rotateY(a);
                        matrix.pos+=center;
                     }break;

                     case OP_ROT_Y:
                     {
                        if(align_nrm && obj.area)
                        {
                           Vec axis, nrm=obj.area->hmNormalAvg(matrix, obj.mesh ? obj.mesh->ext : Extent(0.5f));
                           switch(GetNearestAxis(matrix, nrm))
                           {
                              case  0: axis=matrix.x; break;
                              case  1: axis=matrix.y; break;
                              default: axis=matrix.z; break;
                           }
                           matrix.orn().rotate(!axis, a);
                        }else
                        {
                           matrix.orn().rotateY(a);
                        }
                     }break;

                     case OP_ROT:
                     {
                        switch(obj_axis)
                        {
                           case  0: matrix.orn().rotateXL(a); break;
                           case  1: matrix.orn().rotateYL(a); break;
                           case  2: matrix.orn().rotateZL(a); break;
                           default: matrix.orn().rotate  (ActiveCam.matrix.z, -a); break;
                        }
                     }break;

                     default: matrix.pos+=d; break; // move freely
                  }
                  if(matrix.pos!=obj.matrix.pos)
                  {
                     Area *new_area=getArea(worldToArea(matrix.pos));
                     if(align_hm  && obj.onGround() && new_area->hm)matrix.pos.y=new_area->hmHeight(matrix.pos);
                     if(align_nrm && obj.area)
                     {
                        Vec axis, nrm=obj.area->hmNormalAvg(obj.matrix, obj.mesh ? obj.mesh->ext : Extent(0.5f)); // get the old normal
                        switch(GetNearestAxis(obj.matrix, nrm)) // old matrix
                        {
                           case  0: axis=matrix.x; break; // new matrixes
                           case  1: axis=matrix.y; break;
                           default: axis=matrix.z; break;
                        }
                        nrm=new_area->hmNormalAvg(matrix, obj.mesh ? obj.mesh->ext : Extent(0.5f)); // get new normal
                        matrix.orn()*=Matrix3().setRotation(!axis, nrm);
                     }
                  }
                  obj.moveTo(matrix);
               }
            }
            objTransChanged();
         }

         // finalize operating values
         obj_edit_angle=(align_angle ? FracS(obj_edit_angle,            PI/12) : 0);
         obj_edit_vec.x=(align_xz    ? FracS(obj_edit_vec.x, gridAlignSize ()) : 0);
         obj_edit_vec.y=(align_y     ? FracS(obj_edit_vec.y, gridAlignSizeY()) : 0);
         obj_edit_vec.z=(align_xz    ? FracS(obj_edit_vec.z, gridAlignSize ()) : 0);
      }
   }
}
/******************************************************************************/

/******************************************************************************/
   void ObjPos::Cancel(ObjPos &op) {op.reset(); WorldEdit.obj_op.set(WorldView::OP_SEL);}
   void ObjPos::Changed(ObjPos &op)
   {
      if(op.props.elms()>=3)
      {
         Vec center=0; if(Selection.elms())center=Selection[0].matrix.pos;
         Vec target(op.props[0].asFlt(), op.props[1].asFlt(), op.props[2].asFlt());
         op.delta=target-center;
      }
   }
   ObjPos& ObjPos::create(C Vec2 &up)
   {
      ts.reset().size=0.04f; ts.align.set(1, 0);
      ::EE::Region::create(Rect_U(up, 0.255f, 0.22f)).skin(&TransparentSkin, false).hide(); kb_lit=false;
      props.New().create("X", MemberDesc(DATA_REAL));
      props.New().create("Y", MemberDesc(DATA_REAL));
      props.New().create("Z", MemberDesc(DATA_REAL));
      flt x=AddProperties(props, T, Vec2(0.01f, -0.01f), 0.043f, 0.20f, &ts).centerX();
      REPAO(props).textline.func(Changed, T);

      T+=cancel.create(Rect_C(x, -0.18f, 0.2f, 0.05f), "Cancel").func(Cancel, T);
      return T;
   }
   void ObjPos::apply()
   {
      if(delta.any())
      {
         applying=true ; REPAO(Selection).moveTo(Selection[i].matrix.pos+delta); delta=0;
         applying=false;
      }
   }
   void ObjPos::move(C Vec &delta)
   {
      T.delta+=delta;
      if(props.elms()>=3)
      {
         Vec center=0; if(Selection.elms())center=Selection[0].matrix.pos;
         props[0].set(center.x+T.delta.x, QUIET);
         props[1].set(center.y+T.delta.y, QUIET);
         props[2].set(center.z+T.delta.z, QUIET);
      }
   }
   void ObjPos::reset()
   {
      if(!applying)
      {
         delta=0;
         Vec center=0; if(Selection.elms())center=Selection[0].matrix.pos;
         if(props.elms()>=3)
         {
            props[0].set(center.x, QUIET);
            props[1].set(center.y, QUIET);
            props[2].set(center.z, QUIET);
         }
      }
   }
   void ObjPos::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      if(gpc.visible && visible())
      {
         if(Ms.bp(2) && contains(Gui.ms()))WorldEdit.obj_op.set(-1);
         REPA(props)if(props[i].button()) // if at least one property is being edited then adjust scale of editing
         {
            flt speed;
            if(!WorldEdit.v4.perspective() || !WorldEdit.v4.fpp())speed=CamMoveScale(WorldEdit.v4.perspective());else
            {
               Vec center=0; if(Selection.elms()){REPA(Selection)center+=Selection[i].matrix.pos; center/=Selection.elms();}
               speed=Dist(center, ActiveCam.matrix.pos)*0.75f;
            }
            speed*=0.5f*(Kb.shift() ? 0.1f : 1.0f);
            REPAO(props).mouseEditSpeed(speed);
            break;
         }
      }
   }
   void ObjScale::ChangedX(ObjScale &os)
   {
      if(os.props.elms()>=4)if(flt scale=os.props[0].asFlt())
      {
         Selection.setUndo();
         if(os.props[0].button() && os.multi_val[0] && os.scale.x) // relative scale
         {
            flt mul=scale/os.scale.x; REPA(Selection){Obj &obj=Selection[i]; obj.matrix.x*=mul; obj.setChanged(true);}
         }else // changing with textline
         {
            REPA(Selection){Obj &obj=Selection[i]; obj.matrix.x.setLength(scale); obj.setChanged(true);}
         }
         os.reset(0);
      }
   }
   void ObjScale::ChangedY(ObjScale &os)
   {
      if(os.props.elms()>=4)if(flt scale=os.props[1].asFlt())
      {
         Selection.setUndo();
         if(os.props[1].button() && os.multi_val[1] && os.scale.y) // relative scale
         {
            flt mul=scale/os.scale.y; REPA(Selection){Obj &obj=Selection[i]; obj.matrix.y*=mul; obj.setChanged(true);}
         }else // changing with textline
         {
            REPA(Selection){Obj &obj=Selection[i]; obj.matrix.y.setLength(scale); obj.setChanged(true);}
         }
         os.reset(1);
      }
   }
   void ObjScale::ChangedZ(ObjScale &os)
   {
      if(os.props.elms()>=4)if(flt scale=os.props[2].asFlt())
      {
         Selection.setUndo();
         if(os.props[2].button() && os.multi_val[2] && os.scale.z) // relative scale
         {
            flt mul=scale/os.scale.z; REPA(Selection){Obj &obj=Selection[i]; obj.matrix.z*=mul; obj.setChanged(true);}
         }else // changing with textline
         {
            REPA(Selection){Obj &obj=Selection[i]; obj.matrix.z.setLength(scale); obj.setChanged(true);}
         }
         os.reset(2);
      }
   }
   void ObjScale::ChangedXYZ(ObjScale &os)
   {
      if(os.props.elms()>=4)if(flt scale=os.props[3].asFlt())
      {
         Selection.setUndo();
         if(os.props[3].button() && os.multi_val[3] && os.scale_all) // relative scale
         {
            flt mul=scale/os.scale_all; REPA(Selection){Obj &obj=Selection[i]; obj.matrix.orn()*=mul; obj.setChanged(true);}
         }else // changing with textline
         {
            REPA(Selection){Obj &obj=Selection[i]; obj.matrix.x.setLength(scale); obj.matrix.y.setLength(scale); obj.matrix.z.setLength(scale); obj.setChanged(true);}
         }
         os.reset(3);
      }
   }
   ObjScale& ObjScale::create(C Vec2 &up)
   {
      ts.reset().size=0.04f; ts.align.set(1, 0);
      ::EE::Region::create(Rect_U(up, 0.30f, 0.19f)).skin(&TransparentSkin, false).hide(); kb_lit=false;
      props.New().create("X"  , MEMBER(ObjScale, scale.x  )).precision(4).textline.func(ChangedX  , T, true);
      props.New().create("Y"  , MEMBER(ObjScale, scale.y  )).precision(4).textline.func(ChangedY  , T, true);
      props.New().create("Z"  , MEMBER(ObjScale, scale.z  )).precision(4).textline.func(ChangedZ  , T, true);
      props.New().create("XYZ", MEMBER(ObjScale, scale_all)).precision(4).textline.func(ChangedXYZ, T, true);
      AddProperties(props, T, Vec2(0.01f, -0.01f), 0.043f, 0.20f, &ts); REPAO(props).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).autoData(this);
      Zero(multi_val);
      return T;
   }
   void ObjScale::reset(int skip)
   {
      Zero(multi_val);
      scale=1;
      FREPA(Selection)
      {
       C Matrix3 &m=Selection[i].matrix; Vec s=m.scale();
         if(!i)scale=s;else
         {
            multi_val[0]|=!Equal(scale.x, s.x, 0.001f);
            multi_val[1]|=!Equal(scale.y, s.y, 0.001f);
            multi_val[2]|=!Equal(scale.z, s.z, 0.001f);
         }
      }
      scale_all=scale.max();
      multi_val[3]=(multi_val[0] || multi_val[1] || multi_val[2] || !Equal(scale.x, scale.y, 0.001f) || !Equal(scale.x, scale.z, 0.001f));
      if(props.elms()>=4)
      {
         if(multi_val[0])props[0].textline.set(MultipleName, QUIET);else if(skip!=0)props[0].set(scale.x, QUIET);
         if(multi_val[1])props[1].textline.set(MultipleName, QUIET);else if(skip!=1)props[1].set(scale.y, QUIET);
         if(multi_val[2])props[2].textline.set(MultipleName, QUIET);else if(skip!=2)props[2].set(scale.z, QUIET);
         if(multi_val[3])props[3].textline.set(MultipleName, QUIET);else if(skip!=3)props[3].set(scale.x, QUIET);
      }
   }
   void ObjScale::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      if(gpc.visible && visible())
      {
         if(Ms.bp(2) && contains(Gui.ms()))WorldEdit.obj_op.set(-1);
      }
   }
   Vec ObjRot::Angles(C Matrix3 &m)
   {
      Vec a=m.angles();
      if(Abs(a.z)>=PI_2)
      {
         a.x=PI-a.x;
         a.y=a.y-PI;
         a.z=a.z-PI;
      }
      return a;
   }
      ObjRot::Rot::operator Matrix3() {Matrix3 m; m.setRotateZ(angles.z).rotateXY(angles.x, angles.y); m.x*=scale.x; m.y*=scale.y; m.z*=scale.z; return m;}
       ObjRot::Rot::Rot(C Matrix3 &m) {angles=Angles(m); scale=m.scale();}
   void ObjRot::ChangedX(ObjRot &os)
   {
      if(os.props.elms()>=3)
      {
         Selection.setUndo();
         flt angle=os.props[0].asFlt();
         if(os.props[0].button()) // relative rotation
         {
            flt d=DegToRad(angle-os.angles.x); REPA(Selection){Obj &obj=Selection[i]; Rot r=obj.matrix; r.angles.x+=d; obj.matrix.orn()=r; obj.setChanged(true);}
            os.angles.x=angle;
            if(os.multi_val)os.props[0].textline.set(MultipleName, QUIET);
         }else // set rotation
         {
            os.angles.x=angle;
            Vec a=DegToRad(os.angles);
            REPA(Selection){Obj &obj=Selection[i]; Rot r=obj.matrix; r.angles=a; obj.matrix.orn()=r; obj.setChanged(true);}
            if(os.multi_val)os.reset();
         }
      }
   }
   void ObjRot::ChangedY(ObjRot &os)
   {
      if(os.props.elms()>=3)
      {
         Selection.setUndo();
         flt angle=os.props[1].asFlt();
         if(os.props[1].button()) // relative rotation
         {
            flt d=DegToRad(angle-os.angles.y); REPA(Selection){Obj &obj=Selection[i]; Rot r=obj.matrix; r.angles.y+=d; obj.matrix.orn()=r; obj.setChanged(true);}
            os.angles.y=angle;
            if(os.multi_val)os.props[1].textline.set(MultipleName, QUIET);
         }else // set rotation
         {
            os.angles.y=angle;
            Vec a=DegToRad(os.angles);
            REPA(Selection){Obj &obj=Selection[i]; Rot r=obj.matrix; r.angles=a; obj.matrix.orn()=r; obj.setChanged(true);}
            if(os.multi_val)os.reset();
         }
      }
   }
   void ObjRot::ChangedZ(ObjRot &os)
   {
      if(os.props.elms()>=3)
      {
         Selection.setUndo();
         flt angle=os.props[2].asFlt();
         if(os.props[2].button()) // relative rotation
         {
            flt d=DegToRad(angle-os.angles.z); REPA(Selection){Obj &obj=Selection[i]; Rot r=obj.matrix; r.angles.z+=d; obj.matrix.orn()=r; obj.setChanged(true);}
            os.angles.z=angle;
            if(os.multi_val)os.props[2].textline.set(MultipleName, QUIET);
         }else // set rotation
         {
            os.angles.z=angle;
            Vec a=DegToRad(os.angles);
            REPA(Selection){Obj &obj=Selection[i]; Rot r=obj.matrix; r.angles=a; obj.matrix.orn()=r; obj.setChanged(true);}
            if(os.multi_val)os.reset();
         }
      }
   }
   ObjRot& ObjRot::create(C Vec2 &up)
   {
      ts.reset().size=0.04f; ts.align.set(1, 0);
      ::EE::Region::create(Rect_U(up, 0.255f, 0.147f)).skin(&TransparentSkin, false).hide(); kb_lit=false;
      props.New().create("X", MEMBER(ObjRot, angles.x)).textline.func(ChangedX, T);
      props.New().create("Y", MEMBER(ObjRot, angles.y)).textline.func(ChangedY, T);
      props.New().create("Z", MEMBER(ObjRot, angles.z)).textline.func(ChangedZ, T);
      AddProperties(props, T, Vec2(0.01f, -0.01f), 0.043f, 0.20f, &ts); REPAO(props).autoData(this).mouseEditSpeed(20);
      return T;
   }
   void ObjRot::reset(int skip)
   {
      multi_val=false;
      angles=0;
      FREPA(Selection)
      {
         Vec a=Angles(Selection[i].matrix);
         if(!i)angles=a;else if(!Equal(angles, a, EPS_ANIM_ANGLE))multi_val=true;
      }
      angles=RadToDeg(angles);
      if(props.elms()>=3)
      {
         if(multi_val)props[0].textline.set(MultipleName, QUIET);else if(skip!=0)props[0].set(angles.x, QUIET);
         if(multi_val)props[1].textline.set(MultipleName, QUIET);else if(skip!=1)props[1].set(angles.y, QUIET);
         if(multi_val)props[2].textline.set(MultipleName, QUIET);else if(skip!=2)props[2].set(angles.z, QUIET);
      }
   }
   void ObjRot::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      if(gpc.visible && visible())
      {
         if(Ms.bp(2) && contains(Gui.ms()))WorldEdit.obj_op.set(-1);
      }
   }
   ObjGrid& ObjGrid::create()
   {
      ::EE::Region::create(Rect_LU(0, 0, 0.315f, 0.125f)).skin(&TransparentSkin, false); kb_lit=false;
      T+=bxz.create(Rect_L(0.01f, -0.035f, 0.17f, 0.045f), "Horizontal").focusable(false).desc("Enable Horizontal Grid Aligning"); bxz.mode=BUTTON_TOGGLE; bxz.set(true);
      T+=by .create(Rect_L(0.01f, -0.090f, 0.17f, 0.045f), "Vertical"  ).focusable(false).desc("Enable Vertical Grid Aligning"  ); by .mode=BUTTON_TOGGLE;
      T+= xz.create(Rect_L(0.19f, -0.035f, 0.11f, 0.045f), "1").desc(MLTC(u"Grid size (in meters)", PL, u"Rozmiar Siatki (w metrach)", DE, u"Gittergröße (in Meter)", RU, u"Размер сетки (в метрах)", PO, u"Tamanho da grade (em metros)"));
      T+= y .create(Rect_L(0.19f, -0.090f, 0.11f, 0.045f), "1").desc(MLTC(u"Grid size (in meters)", PL, u"Rozmiar Siatki (w metrach)", DE, u"Gittergröße (in Meter)", RU, u"Размер сетки (в метрах)", PO, u"Tamanho da grade (em metros)"));
      return T;
   }
   void ObjGrid::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      if(gpc.visible && visible())
      {
         if(Ms.bp(2) && contains(Gui.ms()))WorldEdit.show_obj_grid.toggle(0);
      }
   }
   void WaypointPos::Changed(WaypointPos &wp)
   {
      if(wp.props.elms()>=3)
         if(WorldEdit.sel_waypoint && !WorldEdit.sel_waypoint->removed)
            if(EditWaypointPoint *point=WorldEdit.sel_waypoint->find(WorldEdit.sel_waypoint_point))
               if(!point->removed)
      {
         WorldEdit.undos.set(*WorldEdit.sel_waypoint);
         Vec target(wp.props[0].asFlt(), wp.props[1].asFlt(), wp.props[2].asFlt());
         point->setPos(target);
         WorldEdit.setChangedWaypoint(WorldEdit.sel_waypoint, false);
      }
   }
   WaypointPos& WaypointPos::create(C Vec2 &up)
   {
      ts.reset().size=0.04f; ts.align.set(1, 0);
      ::EE::Region::create(Rect_U(up, 0.255f, 0.15f)).skin(&TransparentSkin, false).hide(); kb_lit=false;
      props.New().create("X", MemberDesc(DATA_REAL));
      props.New().create("Y", MemberDesc(DATA_REAL));
      props.New().create("Z", MemberDesc(DATA_REAL));
      AddProperties(props, T, Vec2(0.01f, -0.01f), 0.043f, 0.20f, &ts);
      REPAO(props).textline.func(Changed, T);
      return T;
   }
   void WaypointPos::toGui()
   {
      if(props.elms()>=3)
         if(WorldEdit.sel_waypoint)
            if(EditWaypointPoint *point=WorldEdit.sel_waypoint->find(WorldEdit.sel_waypoint_point))
      {
         props[0].set(point->pos.x, QUIET);
         props[1].set(point->pos.y, QUIET);
         props[2].set(point->pos.z, QUIET);
      }
   }
   void WaypointPos::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      if(gpc.visible && visible())
      {
         if(Ms.bp(2) && contains(Gui.ms()))WorldEdit.waypoint_op.set(-1); // close
         REPA(props)if(props[i].button()) // if at least one property is being edited then adjust scale of editing
         {
            flt speed=-1;
            if(!WorldEdit.v4.perspective() || !WorldEdit.v4.fpp())speed=CamMoveScale(WorldEdit.v4.perspective());else
            if(WorldEdit.sel_waypoint)
               if(EditWaypointPoint *point=WorldEdit.sel_waypoint->find(WorldEdit.sel_waypoint_point))
                  speed=Dist(point->pos, ActiveCam.matrix.pos)*0.75f;
            if(speed>0)
            {
               speed*=0.5f*(Kb.shift() ? 0.1f : 1.0f);
               REPAO(props).mouseEditSpeed(speed);
            }
            break;
         }
      }
   }
ObjPos::ObjPos() : applying(false), delta(0) {}

ObjScale::ObjScale() : scale(1), scale_all(1) {}

ObjRot::ObjRot() : angles(0), multi_val(false) {}

/******************************************************************************/
