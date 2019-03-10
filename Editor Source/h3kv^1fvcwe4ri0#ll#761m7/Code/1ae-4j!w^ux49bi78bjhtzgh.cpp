/******************************************************************************/
class TransformRegion : Region
{
   enum ANCHOR
   {
      SEL_CENTER,
      SEL_LEFT,
      SEL_RIGHT,
      SEL_DOWN,
      SEL_UP,
      SEL_BACK,
      SEL_FRONT,
      WORLD_CENTER,
      CAM_TARGET,
   }
   static cchar8 *AnchorName[]=
   {
      "Selection Center",
      "Selection Left",
      "Selection Right",
      "Selection Bottom",
      "Selection Top",
      "Selection Back",
      "Selection Front",
      "World Center",
      "Camera Target",
   };

   bool           full=false;
   Property      *move_p[3];
   Memx<Property> props;
   TextWhite      ts;
   Button         rescale_height, rescale_depth, move_bottom, move_back, move_center, move_center_xz, rest_bottom, rot_y_min_box, ok, reset, cancel, close, original,
                  rot[3][2]; // [xyz][dec/inc]
   TextLine       rescale_height_value, rescale_depth_value;
   Pose           trans;
   flt            trans_normal=0;
   Vec            trans_scale=1, anchor_pos=0;
   Vec2           move_uv=0, scale_uv=1;
   Matrix         matrix(1);
   ANCHOR         anchor=SEL_CENTER;

   static void RescaleHeight(TransformRegion &tr)
   {
      if(flt h=ObjEdit.mesh_box.h())
      if(flt t=TextFlt(tr.rescale_height_value()))
      {
         tr.trans.scale=t/h;
         tr.toGui();
      }
   }
   static void RescaleDepth(TransformRegion &tr)
   {
      if(flt d=ObjEdit.mesh_box.d())
      if(flt t=TextFlt(tr.rescale_depth_value()))
      {
         tr.trans.scale=t/d;
         tr.toGui();
      }
   }
   static void MoveBottom  (TransformRegion &tr) {tr.moveBottom  ();}
   static void MoveBack    (TransformRegion &tr) {tr.moveBack    ();}
   static void MoveCenter  (TransformRegion &tr) {tr.moveCenter  ();}
   static void MoveCenterXZ(TransformRegion &tr) {tr.moveCenterXZ();}
   static void RestBottom  (TransformRegion &tr) {tr.restBottom  ();}
   static void RotYMinBox  (TransformRegion &tr) {tr.rotYMinBox  ();}
   static void OK          (TransformRegion &tr) {tr.   apply(); tr.hideDo();}
   static void Reset       (TransformRegion &tr) {tr. resetDo();}
   static void Cancel      (TransformRegion &tr) {tr.cancelDo();}
   static void Original    (TransformRegion &tr) {tr.cancelDo(); ObjEdit.resetTransform();}

   static void RotX(  TransformRegion &tr, C Str &text) {tr.trans.rot.x=DegToRad(TextFlt(text));}
   static void RotY(  TransformRegion &tr, C Str &text) {tr.trans.rot.y=DegToRad(TextFlt(text));}
   static void RotZ(  TransformRegion &tr, C Str &text) {tr.trans.rot.z=DegToRad(TextFlt(text));}
   static Str  RotX(C TransformRegion &tr             ) {return RadToDeg(tr.trans.rot.x);}
   static Str  RotY(C TransformRegion &tr             ) {return RadToDeg(tr.trans.rot.y);}
   static Str  RotZ(C TransformRegion &tr             ) {return RadToDeg(tr.trans.rot.z);}

   static void Dec90(Property &prop) {prop.set(FracS(prop.asFlt()-90, 360));}
   static void Inc90(Property &prop) {prop.set(FracS(prop.asFlt()+90, 360));}

   void moveBottom  () {trans.pos.y=-ObjEdit.mesh_box.min.y    ;                                          toGui();}
   void moveBack    () {trans.pos.z=-ObjEdit.mesh_box.min.z    ;                                          toGui();}
   void moveCenter  () {trans.pos  =-ObjEdit.mesh_box.center ();                                          toGui();}
   void moveCenterXZ() {trans.pos.x=-ObjEdit.mesh_box.centerX(); trans.pos.z=-ObjEdit.mesh_box.centerZ(); toGui();}
   void restBottom  ()
   {
      Matrix matrix=GetRestMatrix(ObjEdit.mesh, &ObjEdit.mesh_matrix); // no need to adjust with 'ObjEdit.posScale()' because 'ObjEdit.mesh_matrix' already does that
      matrix=GetTransform(ObjEdit.mesh_matrix, matrix); // we need the transform from current matrix to calculated matrix
      trans.rot=matrix.angles();
      toGui();
   }
   void rotYMinBox()
   {
      Memt<Vec2> vtxs;
    C MeshLod &lod=ObjEdit.mesh; vtxs.reserve(lod.vtxs()); REPA(lod)
      {
       C MeshPart &part=lod.parts[i]; if(!(part.part_flag&MSHP_HIDDEN))if(C Vec *src=part.base.vtx.pos())
         {
            int vs=part.base.vtxs();
            Vec2 *dest=&vtxs[vtxs.addNum(vs)];
            REP(vs)dest[i]=(src[i]*ObjEdit.mesh_matrix.orn()).xz();
         }
      }
      trans.rot.zero();
      Vec2 axis; if(BestFit(vtxs.data(), vtxs.elms(), axis))
      {
         flt a=Angle(axis); a=Frac(a, PI_2); if(a>EPS && a<PI_2-EPS)
         {
            if(a>PI_4)a-=PI_2; trans.rot.y=a;
         }
      }
      toGui();
   }

   void toGui() {REPAO(props).toGui(); setMatrix();}

   static void Changed  (C Property &prop) {((TransformRegion*)prop.autoData()).setMatrix();}
          void setMatrix()
   {
      if(!full)matrix=trans();else
      {
         setAnchorPos();
         matrix.setPos(-anchor_pos);
         matrix.rotateZ(trans.rot.z).rotateXY(trans.rot.x, trans.rot.y);
         matrix.scale(trans.scale).scaleL(trans_scale);
         matrix.move(anchor_pos+trans.pos);
      }
   }

   Matrix drawMatrix()C {return Matrix(anchor_pos)*matrix;}
   void setAnchorPos() {anchor_pos=anchorPos();}
   Vec anchorPos()C
   {
      switch(anchor)
      {
         case CAM_TARGET  : return ActiveCam.at;
         case WORLD_CENTER: return VecZero;
         default          :
         {
            Box box; bool is=false; Matrix matrix=ObjEdit.mesh_matrix;
            MeshLod &lod=ObjEdit.getLod();
            if(ObjEdit.mesh_parts.edit_selected())
            {
               REPA(ObjEdit.sel_vtx)
               {
                C VecI2 &v=ObjEdit.sel_vtx[i]; if(MeshPart *part=ObjEdit.getPart(v.x))
                  {
                     MeshBase &base=part.base;
                     if(base.vtx.pos() && InRange(v.y, base.vtx))Include(box, is, base.vtx.pos(v.y)*matrix);
                  }
               }
               REPA(ObjEdit.sel_face)
               {
                C VecI2 &v=ObjEdit.sel_face[i]; if(MeshPart *part=ObjEdit.getPart(v.x))
                  {
                     MeshBase &base=part.base; if(C Vec *pos=base.vtx.pos())
                     {
                        if(v.y&SIGN_BIT){int f=v.y^SIGN_BIT; if(InRange(f, base.quad)){C VecI4 &q=base.quad.ind(f); REPA(q)Include(box, is, pos[q.c[i]]*matrix);}}
                        else            {int f=v.y         ; if(InRange(f, base.tri )){C VecI  &t=base.tri .ind(f); REPA(t)Include(box, is, pos[t.c[i]]*matrix);}}
                     }
                  }
               }
            }else
            {
               REPA(lod)if(ObjEdit.partOp(i))
               {
                  MeshPart &part=lod.parts[i]; if(ObjEdit.partVisible(i, part, false))
                  {
                     MeshBase &base=part.base; if(C Vec *pos=base.vtx.pos())REPA(base.vtx)Include(box, is, pos[i]*matrix);
                  }
               }
            }
            if(is)switch(anchor)
            {
               case SEL_CENTER: return box.center ();
               case SEL_LEFT  : return box.left   ();
               case SEL_RIGHT : return box.right  ();
               case SEL_DOWN  : return box.down   ();
               case SEL_UP    : return box.up     ();
               case SEL_BACK  : return box.back   ();
               case SEL_FRONT : return box.forward();
            }
         }break;
      }
      return VecZero;
   }
   void resetDo()
   {
      trans.reset();
      trans_normal=0;
      trans_scale=1;
      move_uv=0; scale_uv=1;
      toGui();
   }
   void hideDo()
   {
      if(full)hide();else ObjEdit.mode.set(-1);
   }
   void cancelDo()
   {
      resetDo();
       hideDo();
   }
   void apply()
   {
      if(trans!=PoseIdentity || trans_scale!=Vec(1) || trans_normal || move_uv.any() || scale_uv!=1)
      {
         if(full)
         {
            ObjEdit.mesh_undos.set("transform");
            MeshLod &lod=ObjEdit.getLod();
            Matrix matrix=ObjEdit.mesh_matrix*T.matrix*~ObjEdit.mesh_matrix; // "edit mesh" is in 'MatrixIdentity' however we're expecting it to be in 'mesh_matrix'

            if(ObjEdit.mesh_parts.edit_selected())
            {
               Matrix3     matrix_n=matrix; matrix_n.inverseScale();
               Memt<int  > parts;
               Memt<VecI2> vtxs;
               if(ObjEdit.sel_face.elms())
               {
                  REPA(ObjEdit.sel_face)
                  {
                   C VecI2 &v=ObjEdit.sel_face[i]; if(MeshPart *part=ObjEdit.getPart(v.x))
                     {
                        MeshBase &base=part.base;
                        if(v.y&SIGN_BIT){int f=v.y^SIGN_BIT; if(InRange(f, base.quad)){C VecI4 &q=base.quad.ind(f); REPA(q)vtxs.binaryInclude(VecI2(v.x, q.c[i]), Compare);}}
                        else            {int f=v.y         ; if(InRange(f, base.tri )){C VecI  &t=base.tri .ind(f); REPA(t)vtxs.binaryInclude(VecI2(v.x, t.c[i]), Compare);}}
                     }
                  }
                  // add touching vertexes from other parts
                  if(1)
                  {
                     // at this point we're sure that 'vtxs' are valid (they point to existing parts and vertex indexes)
                     REPA(lod)if(ObjEdit.partVisible(i, lod.parts[i], false))ObjEdit.includeSamePos(i, vtxs);
                  }
               }else
               {
                  vtxs=ObjEdit.sel_vtx;
               }
               REPA(vtxs)
               {
                C VecI2 &v=vtxs[i]; if(MeshPart *part=ObjEdit.getPart(v.x))
                  {
                     parts.binaryInclude(v.x, Compare);
                     MeshBase &base=part.base; if(InRange(v.y, base.vtx))
                     {
                        if(trans_normal && base.vtx.pos() && base.vtx.nrm())base.vtx.pos(v.y)+=base.vtx.nrm(v.y)*trans_normal; // transform before transforming by matrix
                        if(base.vtx.pos())base.vtx.pos(v.y)*=matrix;
                        if(base.vtx.hlp())base.vtx.hlp(v.y)*=matrix;
                        if(base.vtx.nrm())base.vtx.nrm(v.y)*=matrix_n;
                        if(base.vtx.tan())base.vtx.tan(v.y)*=matrix_n;
                        if(base.vtx.bin())base.vtx.bin(v.y)*=matrix_n;
                        if(base.vtx.tex0()){Vec2 &t=base.vtx.tex0(v.y); t=t*scale_uv+move_uv;}
                     }
                  }
               }
               REPA(parts)
               {
                  MeshPart &part=lod.parts[parts[i]];
                  Normalize(part.base.vtx.nrm(), part.base.vtxs());
                  Normalize(part.base.vtx.tan(), part.base.vtxs());
                  Normalize(part.base.vtx.bin(), part.base.vtxs());
                  part.base.setTangents().setBinormals(); part.setRender();
               }
            }else
            REPA(lod)if(ObjEdit.partOp(i))
            {
               MeshPart &part=lod.parts[i];
               part.transform(matrix);
               part.texScale(scale_uv);
               part.texMove ( move_uv);
            }

            ObjEdit.mesh.setBox();
            ObjEdit.setChangedMesh(true);
         }else
         {
            ObjEdit.applyTransform(matrix);
         }
         resetDo();
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(gpc.visible && visible()) // set speed based on cam distance and FOV
      {
         if(anchor==CAM_TARGET)setMatrix(); // this will already call 'setAnchorPos'
         flt speed=CamMoveScale(ObjEdit.v4.perspective())*(Kb.shift() ? 0.1 : 1)*0.5;
         if(!full && trans.scale)speed/=trans.scale;
         REPA(move_p)if(move_p[i])move_p[i].mouseEditSpeed(speed);
      }
   }

   TransformRegion& create(bool full)
   {
      super.create().skin(&TransparentSkin, false); kb_lit=false; T.full=full; if(full)hide();
      Property *anchor_p=null, *rot_p[3];
                  props.New().create("Scale"   , MemberDesc(MEMBER(TransformRegion, trans.scale))).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).real_precision=4;
               if(full)
               {
                  props.New().create("Scale X" , MemberDesc(MEMBER(TransformRegion, trans_scale.x))).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).real_precision=4;
                  props.New().create("Scale Y" , MemberDesc(MEMBER(TransformRegion, trans_scale.y))).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).real_precision=4;
                  props.New().create("Scale Z" , MemberDesc(MEMBER(TransformRegion, trans_scale.z))).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).real_precision=4;
               }
       move_p[0]=&props.New().create("Move X"  , MemberDesc(MEMBER(TransformRegion, trans.pos.x)));
       move_p[1]=&props.New().create("Move Y"  , MemberDesc(MEMBER(TransformRegion, trans.pos.y)));
       move_p[2]=&props.New().create("Move Z"  , MemberDesc(MEMBER(TransformRegion, trans.pos.z)));
        rot_p[0]=&props.New().create("Rotate X", MemberDesc(MEMBER(TransformRegion, trans.rot.x)).setFunc(RotX, RotX)).mouseEditSpeed(40);
        rot_p[1]=&props.New().create("Rotate Y", MemberDesc(MEMBER(TransformRegion, trans.rot.y)).setFunc(RotY, RotY)).mouseEditSpeed(40);
        rot_p[2]=&props.New().create("Rotate Z", MemberDesc(MEMBER(TransformRegion, trans.rot.z)).setFunc(RotZ, RotZ)).mouseEditSpeed(40);
if(full)anchor_p=&props.New().create("Anchor"  , MemberDesc(MEMBER(TransformRegion, anchor     ))).setEnum(AnchorName, Elms(AnchorName));
if(full)
{
      props.New().create("Move U" , MemberDesc(MEMBER(TransformRegion,  move_uv.x))).mouseEditSpeed(0.1).real_precision=4;
      props.New().create("Move V" , MemberDesc(MEMBER(TransformRegion,  move_uv.y))).mouseEditSpeed(0.1).real_precision=4;
      props.New().create("Scale U", MemberDesc(MEMBER(TransformRegion, scale_uv.x))).mouseEditSpeed(1.0);
      props.New().create("Scale V", MemberDesc(MEMBER(TransformRegion, scale_uv.y))).mouseEditSpeed(1.0);
}
      REPAO(move_p).real_precision=4;
      ts.reset(); ts.size=0.038; ts.align.set(1, 0);
      flt  prop_height=0.045, elm_height=prop_height*0.95, e=0.01;
      Rect prop_rect=AddProperties(props, T, Vec2(0.01, -0.01), prop_height, 0.18, &ts); REPAO(props).autoData(this).changed(Changed); prop_rect.min.y-=0.01;
      if(anchor_p)anchor_p.combobox.resize(Vec2(0.11, 0));
      REPD(a, 3)REPD(i, 2)T+=rot[a][i].create(Rect_LU(rot_p[a].button.rect().ru()+Vec2(0.01+prop_height*i, 0), prop_height)).setImage(i ? "Gui/arrow_right_big.img" : "Gui/arrow_left_big.img").func(i ? Inc90 : Dec90, *rot_p[a]);
      if(!full)
      {
         T+=rescale_height   .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.29, elm_height), "Rescale Height to:").func(RescaleHeight, T); T+=rescale_height_value.create(Rect_LU(rescale_height.rect().max.x+e, prop_rect.min.y, 0.09, elm_height), "1"); prop_rect.min.y-=prop_height;
         T+=rescale_depth    .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.29, elm_height), "Rescale Depth to:" ).func(RescaleDepth , T); T+=rescale_depth_value .create(Rect_LU(rescale_depth .rect().max.x+e, prop_rect.min.y, 0.09, elm_height), "1"); prop_rect.min.y-=prop_height;
         T+=   move_bottom   .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.33, elm_height), "Move Bottom to Y=0").func(MoveBottom, T); prop_rect.min.y-=prop_height;
         T+=   move_back     .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.29, elm_height), "Move Back to Z=0"  ).func(MoveBack  , T); prop_rect.min.y-=prop_height;
         T+=   move_center   .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.37, elm_height), "Move Center to (0, 0, 0)").func(MoveCenter, T); prop_rect.min.y-=prop_height;
         T+=   move_center_xz.create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.39, elm_height), "Move Center XZ to (0, 0)").func(MoveCenterXZ, T); prop_rect.min.y-=prop_height;
         T+=   rest_bottom   .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.26, elm_height), "Rest on Bottom"          ).func(RestBottom, T).desc("This will transform the object as if it fell on the ground"); prop_rect.min.y-=prop_height;
         T+=   rot_y_min_box .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.38, elm_height), "Rotate Y to Minimize Box").func(RotYMinBox, T).desc("This will rotate the object along Y axis to minimize its bounding box"); prop_rect.min.y-=prop_height*1.5;
      }
      if(!full)T+=ok      .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.20, elm_height*1.2), "OK"   ).func(OK   , T).desc("Apply transformation");
      else     T+=reset   .create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.20, elm_height*1.2), "Reset").func(Reset, T).desc("Reset transformation values to default");
               T+=cancel  .create(Rect_LU(ok.is() ? ok.rect().max.x+elm_height : reset.is() ? reset.rect().max.x+elm_height : prop_rect.min.x, prop_rect.min.y, 0.20, elm_height*1.2), "Cancel").func(Cancel, T).desc("Cancel applying transformation"); prop_rect.min.y-=prop_height*2.0;
      if(!full)T+=original.create(Rect_LU(prop_rect.min.x, prop_rect.min.y, 0.45, elm_height*1.2), "Reset Object Transform").func(Original, T).desc("Restore object original transformation as it was at the moment of object import");
      T.size(Vec2(slidebar[0].lengthTotal(), slidebar[1].lengthTotal())+0.01);
      if(full)T+=close.create(Rect_RU(clientWidth(), 0, elm_height, elm_height)).func(OK, T); close.image="Gui/close.img"; close.skin=&EmptyGuiSkin;
      return T;
   }
}
/******************************************************************************/
