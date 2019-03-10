/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
IconSettsEditor IconSettsEdit;
/******************************************************************************/
IconEditor IconEdit;
/******************************************************************************/

/******************************************************************************/
   cchar8 *IconSettsEditor::IconSizes[]=
   {
       "32",
       "64",
      "128",
      "192",
      "256",
      "320",
      "384",
      "512",
   };
/******************************************************************************/
      void IconSettsEditor::Change::create(ptr user)
{
         data=IconSettsEdit.edit;
         IconSettsEdit.undoVis();
      }
      void IconSettsEditor::Change::apply(ptr user)
{
         IconSettsEdit.edit.undo(data);
         IconSettsEdit.setChanged();
         IconSettsEdit.toGui();
         IconSettsEdit.undoVis();
      }
   void IconSettsEditor::undoVis() {SetUndo(undos, undo, redo);}
   ElmIconSetts* IconSettsEditor::data()C {return elm ? elm->iconSettsData() : null;}
   void IconSettsEditor::PreChanged(C Property &prop) {IconSettsEdit.undos.set(&prop);}
   void    IconSettsEditor::Changed(C Property &prop) {IconSettsEdit.setChanged();}
   void IconSettsEditor::Width(  IconSettsEditor &ie, C Str &t) {int i=TextInt(t); ie.edit.width=(InRange(i, IconSizes) ? TextInt(IconSizes[i]) : 128); ie.edit.width_time.getUTC();}
   Str  IconSettsEditor::Width(C IconSettsEditor &ie          ) {int nearest=-1, dist; REPA(IconSizes){int d=Abs(ie.edit.width-TextInt(IconSizes[i])); if(nearest<0 || d<dist){nearest=i; dist=d;}} return nearest;}
   void IconSettsEditor::Height(  IconSettsEditor &ie, C Str &t) {int i=TextInt(t); ie.edit.height=(InRange(i, IconSizes) ? TextInt(IconSizes[i]) : 128); ie.edit.height_time.getUTC();}
   Str  IconSettsEditor::Height(C IconSettsEditor &ie          ) {int nearest=-1, dist; REPA(IconSizes){int d=Abs(ie.edit.height-TextInt(IconSizes[i])); if(nearest<0 || d<dist){nearest=i; dist=d;}} return nearest;}
   void IconSettsEditor::Type(  IconSettsEditor &ie, C Str &t) {int i=TextInt(t); if(InRange(i, ElmImage::NUM))ie.edit.type=ElmImage::TYPE(i); ie.edit.type_time.getUTC();}
   Str  IconSettsEditor::Type(C IconSettsEditor &ie          ) {return ie.edit.type;}
   void IconSettsEditor::MipMaps(  IconSettsEditor &ie, C Str &t) {ie.edit.mip_maps=TextBool(t); ie.edit.mip_maps_time.getUTC();}
   Str  IconSettsEditor::MipMaps(C IconSettsEditor &ie          ) {return ie.edit.mip_maps;}
   void IconSettsEditor::AutoCenter(  IconSettsEditor &ie, C Str &t) {ie.edit.auto_center=TextBool(t); ie.edit.auto_center_time.getUTC();}
   Str  IconSettsEditor::AutoCenter(C IconSettsEditor &ie          ) {return ie.edit.auto_center;}
   void IconSettsEditor::Scale(  IconSettsEditor &ie, C Str &t) {ie.edit.scale=TextFlt(t); ie.edit.scale_time.getUTC();}
   Str  IconSettsEditor::Scale(C IconSettsEditor &ie          ) {return ie.edit.scale;}
   void IconSettsEditor::CamYaw(  IconSettsEditor &ie, C Str &t) {ie.edit.cam_angle.x=DegToRad(TextFlt(t)); ie.edit.cam_angle_time.getUTC();}
   Str  IconSettsEditor::CamYaw(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.cam_angle.x);}
   void IconSettsEditor::CamPitch(  IconSettsEditor &ie, C Str &t) {ie.edit.cam_angle.y=DegToRad(TextFlt(t)); ie.edit.cam_angle_time.getUTC();}
   Str  IconSettsEditor::CamPitch(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.cam_angle.y);}
   void IconSettsEditor::CamRoll(  IconSettsEditor &ie, C Str &t) {ie.edit.cam_angle.z=DegToRad(TextFlt(t)); ie.edit.cam_angle_time.getUTC();}
   Str  IconSettsEditor::CamRoll(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.cam_angle.z);}
   void IconSettsEditor::CamFocusX(  IconSettsEditor &ie, C Str &t) {ie.edit.cam_focus.x=TextFlt(t); ie.edit.cam_focus_time.getUTC();}
   Str  IconSettsEditor::CamFocusX(C IconSettsEditor &ie          ) {return ie.edit.cam_focus.x;}
   void IconSettsEditor::CamFocusY(  IconSettsEditor &ie, C Str &t) {ie.edit.cam_focus.y=TextFlt(t); ie.edit.cam_focus_time.getUTC();}
   Str  IconSettsEditor::CamFocusY(C IconSettsEditor &ie          ) {return ie.edit.cam_focus.y;}
   void IconSettsEditor::CamFocusZ(  IconSettsEditor &ie, C Str &t) {ie.edit.cam_focus.z=TextFlt(t); ie.edit.cam_focus_time.getUTC();}
   Str  IconSettsEditor::CamFocusZ(C IconSettsEditor &ie          ) {return ie.edit.cam_focus.z;}
   void IconSettsEditor::BloomOriginal(  IconSettsEditor &ie, C Str &t) {ie.edit.bloom_original=TextFlt(t); ie.edit.bloom_original_time.getUTC();}
   Str  IconSettsEditor::BloomOriginal(C IconSettsEditor &ie          ) {return ie.edit.bloom_original;}
   void IconSettsEditor::BloomScale(  IconSettsEditor &ie, C Str &t) {ie.edit.bloom_scale=TextFlt(t); ie.edit.bloom_scale_time.getUTC();}
   Str  IconSettsEditor::BloomScale(C IconSettsEditor &ie          ) {return ie.edit.bloom_scale;}
   void IconSettsEditor::BloomCut(  IconSettsEditor &ie, C Str &t) {ie.edit.bloom_cut=TextFlt(t); ie.edit.bloom_cut_time.getUTC();}
   Str  IconSettsEditor::BloomCut(C IconSettsEditor &ie          ) {return ie.edit.bloom_cut;}
   void IconSettsEditor::Light0Shadow(  IconSettsEditor &ie, C Str &t) {ie.edit.light0_shadow=TextBool(t); ie.edit.light0_shadow_time.getUTC();}
   Str  IconSettsEditor::Light0Shadow(C IconSettsEditor &ie          ) {return ie.edit.light0_shadow;}
   void IconSettsEditor::Light1Shadow(  IconSettsEditor &ie, C Str &t) {ie.edit.light1_shadow=TextBool(t); ie.edit.light1_shadow_time.getUTC();}
   Str  IconSettsEditor::Light1Shadow(C IconSettsEditor &ie          ) {return ie.edit.light1_shadow;}
   void IconSettsEditor::AmbientCol(  IconSettsEditor &ie, C Str &t) {ie.edit.ambient_col=TextVec(t); ie.edit.ambient_col_time.getUTC();}
   Str  IconSettsEditor::AmbientCol(C IconSettsEditor &ie          ) {return ie.edit.ambient_col;}
   void IconSettsEditor::AmbientOccl(  IconSettsEditor &ie, C Str &t) {ie.edit.ambient_occl=TextFlt(t); ie.edit.ambient_occl_time.getUTC();}
   Str  IconSettsEditor::AmbientOccl(C IconSettsEditor &ie          ) {return ie.edit.ambient_occl;}
   void IconSettsEditor::AmbientRange(  IconSettsEditor &ie, C Str &t) {ie.edit.ambient_range=TextFlt(t); ie.edit.ambient_range_time.getUTC();}
   Str  IconSettsEditor::AmbientRange(C IconSettsEditor &ie          ) {return ie.edit.ambient_range;}
   void IconSettsEditor::Light0Col(  IconSettsEditor &ie, C Str &t) {ie.edit.light0_col=TextVec(t); ie.edit.light0_col_time.getUTC();}
   Str  IconSettsEditor::Light0Col(C IconSettsEditor &ie          ) {return ie.edit.light0_col;}
   void IconSettsEditor::Light0Yaw(  IconSettsEditor &ie, C Str &t) {ie.edit.light0_angle.x=DegToRad(TextFlt(t)); ie.edit.light0_angle_time.getUTC();}
   Str  IconSettsEditor::Light0Yaw(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.light0_angle.x);}
   void IconSettsEditor::Light0Pitch(  IconSettsEditor &ie, C Str &t) {ie.edit.light0_angle.y=DegToRad(TextFlt(t)); ie.edit.light0_angle_time.getUTC();}
   Str  IconSettsEditor::Light0Pitch(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.light0_angle.y);}
   void IconSettsEditor::Light1Col(  IconSettsEditor &ie, C Str &t) {ie.edit.light1_col=TextVec(t); ie.edit.light1_col_time.getUTC();}
   Str  IconSettsEditor::Light1Col(C IconSettsEditor &ie          ) {return ie.edit.light1_col;}
   void IconSettsEditor::Light1Yaw(  IconSettsEditor &ie, C Str &t) {ie.edit.light1_angle.x=DegToRad(TextFlt(t)); ie.edit.light1_angle_time.getUTC();}
   Str  IconSettsEditor::Light1Yaw(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.light1_angle.x);}
   void IconSettsEditor::Light1Pitch(  IconSettsEditor &ie, C Str &t) {ie.edit.light1_angle.y=DegToRad(TextFlt(t)); ie.edit.light1_angle_time.getUTC();}
   Str  IconSettsEditor::Light1Pitch(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.light1_angle.y);}
   void IconSettsEditor::Fov(  IconSettsEditor &ie, C Str &t) {ie.edit.fov=DegToRad(TextFlt(t)); ie.edit.fov_time.getUTC();}
   Str  IconSettsEditor::Fov(C IconSettsEditor &ie          ) {return RadToDeg(ie.edit.fov);}
   void IconSettsEditor::Undo(IconSettsEditor &editor) {editor.undos.undo();}
   void IconSettsEditor::Redo(IconSettsEditor &editor) {editor.undos.redo();}
   void IconSettsEditor::Locate(IconSettsEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void IconSettsEditor::create()
   {
      ListColumn lct[]=
      {
         ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      add("Width"         , MemberDesc(         ).setFunc(Width        , Width        )).setEnum(IconSizes, Elms(IconSizes));
      add("Height"        , MemberDesc(         ).setFunc(Height       , Height       )).setEnum(IconSizes, Elms(IconSizes));
      add("Type"          , MemberDesc(         ).setFunc(Type         , Type         )).setEnum().combobox.setColumns(lct, Elms(lct)).setData(ElmImage::ImageTypes, ElmImage::ImageTypesElms).menu.list.setElmDesc(MEMBER(NameDesc, desc));
      add("Mip Maps"      , MemberDesc(DATA_BOOL).setFunc(MipMaps      , MipMaps      ));
      add();
      add("Auto Center"   , MemberDesc(DATA_BOOL).setFunc(AutoCenter   , AutoCenter   ));
      add("Scale"         , MemberDesc(DATA_REAL).setFunc(Scale        , Scale        )).min(0.001f).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add();
      add("Camera Yaw"    , MemberDesc(DATA_INT).setFunc(CamYaw       , CamYaw       ));
      add("Camera Pitch"  , MemberDesc(DATA_INT).setFunc(CamPitch     , CamPitch     ));
      add("Camera Roll"   , MemberDesc(DATA_INT).setFunc(CamRoll      , CamRoll      ));
      add();
      add("Camera Focus X", MemberDesc(DATA_REAL).setFunc(CamFocusX    , CamFocusX    ));
      add("Camera Focus Y", MemberDesc(DATA_REAL).setFunc(CamFocusY    , CamFocusY    ));
      add("Camera Focus Z", MemberDesc(DATA_REAL).setFunc(CamFocusZ    , CamFocusZ    ));
      add();
      add("Bloom Original", MemberDesc(DATA_REAL).setFunc(BloomOriginal, BloomOriginal)).range(0, 2);
      add("Bloom Scale"   , MemberDesc(DATA_REAL).setFunc(BloomScale   , BloomScale   )).range(0, 2);
      add("Bloom Cut"     , MemberDesc(DATA_REAL).setFunc(BloomCut     , BloomCut     )).range(0, 1);
      add();
      add("Ambient"          , MemberDesc(DATA_VEC ).setFunc(AmbientCol   , AmbientCol   )).setColor();
      add("Ambient Occlusion", MemberDesc(DATA_REAL).setFunc(AmbientOccl  , AmbientOccl  )).range(0, 2);
      add("Ambient Range"    , MemberDesc(DATA_REAL).setFunc(AmbientRange , AmbientRange )).range(0, 1).mouseEditSpeed(0.01f);
      add();
      add("Light 1"          , MemberDesc(DATA_VEC ).setFunc(Light0Col   , Light0Col   )).setColor();
      add("Light 1 Yaw"      , MemberDesc(DATA_INT ).setFunc(Light0Yaw   , Light0Yaw   ));
      add("Light 1 Pitch"    , MemberDesc(DATA_INT ).setFunc(Light0Pitch , Light0Pitch ));
      add("Light 1 Shadows"  , MemberDesc(DATA_BOOL).setFunc(Light0Shadow, Light0Shadow));
      add();
      add("Light 2"          , MemberDesc(DATA_VEC ).setFunc(Light1Col   , Light1Col   )).setColor();
      add("Light 2 Yaw"      , MemberDesc(DATA_INT ).setFunc(Light1Yaw   , Light1Yaw   ));
      add("Light 2 Pitch"    , MemberDesc(DATA_INT ).setFunc(Light1Pitch , Light1Pitch ));
      add("Light 2 Shadows"  , MemberDesc(DATA_BOOL).setFunc(Light1Shadow, Light1Shadow));
      add();
      add("Field of View"    , MemberDesc(DATA_REAL).setFunc(Fov, Fov)).range(1, 120).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      autoData(this);

      ::PropWin::create("Icon Settings Editor", Vec2(0.02f, -0.07f), 0.036f, 0.043f, 0.28f); ::PropWin::changed(Changed, PreChanged); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      T+=undo  .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      move(Vec2(D.w(), 0)-rect().right());
   }
   IconSettsEditor& IconSettsEditor::hide(){set(null); ::PropWin::hide(); return T;}
   void IconSettsEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmIconSetts *data=T.data())
         {
            data->newVer(); // modify just before saving/sending in case we've received data from server after edit
            data->from(edit);
         }
         Save(edit, Proj.basePath(*elm));
         Server.setElmLong(elm->id);
      }
      changed=false;
   }
   void IconSettsEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmIconSetts *data=T.data())
         {
            data->newVer();
            data->from(edit); // call this here too, because data can be used for icon creation which can be called before flushing
         }
         if(ElmIcon *icon=IconEdit.data())if(icon->icon_settings_id==elm_id)IconEdit.settings=edit;
      }
   }
   void IconSettsEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_ICON_SETTS)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm)edit.load(Proj.basePath(*elm));else edit.reset();
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void IconSettsEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void IconSettsEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void IconSettsEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
   void IconSettsEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         undos.set(null, true);
         IconSettings temp; if(temp.load(Proj.basePath(*elm)))if(edit.sync(temp))toGui();
      }
   }
      void IconEditor::Change::create(ptr user)
{
         if(ElmIcon *data=IconEdit.data())T.data=*data;
         IconEdit.undoVis();
      }
      void IconEditor::Change::apply(ptr user)
{
         if(ElmIcon *data=IconEdit.data())
         {
            data->undo(T.data);
            IconEdit.setChanged();
            IconEdit.reloadSettings();
            IconEdit.toGui();
            IconEdit.undoVis();
         }
      }
   void IconEditor::Render() {IconEdit.render();}
          void IconEditor::render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            if(obj && obj->mesh())
            {
               SetVariation(mesh_variation); obj->mesh()->draw(skel);
               SetVariation();
            }

            LightDir(Matrix3().setRotateXY(settings.light0_angle.y, settings.light0_angle.x+PI).z, settings.light0_col).add(settings.light0_shadow);
            LightDir(Matrix3().setRotateXY(settings.light1_angle.y, settings.light1_angle.x+PI).z, settings.light1_col).add(settings.light1_shadow);
         }break;

         case RM_SHADOW:
         {
            if(obj && obj->mesh())
            {
               SetVariation(mesh_variation); obj->mesh()->drawShadow(skel);
               SetVariation();
            }
         }break;
      }
   }
   Matrix IconEditor::matrix() {return Matrix().setScale(settings.scale);}
   Box    IconEditor::box() {return (obj && obj->mesh()) ? obj->mesh()->ext : 0;}
   flt IconEditor::GetDist(C Box &box) // avoid changing this func as it will break old icon settings
   {
      return Max(box.w()/(2*Tan(D.viewFovX()/2)), box.h()/(2*Tan(D.viewFovY()/2))) + box.d()/2;
   }
   flt IconEditor::getDist() // !! assumes that viewport and obj are already set !!
   {
      if(!settings.auto_center)return 1; // we need constant camera distance when auto centering is disabled
      return GetDist(box());
   }
   void IconEditor::Draw(Viewport &viewport) {IconEdit.draw();}
          void IconEditor::draw(bool final)
   {
      MOTION_MODE      motion       =D.motionMode     (); D. motionMode    (MOTION_NONE);
      AMBIENT_MODE     ambient      =D.ambientMode    (); D.ambientMode    (AMBIENT_ULTRA);
      BUMP_MODE        bump         =D.bumpMode       (); if(final && bump<BUMP_PARALLAX)D.bumpMode(BUMP_PARALLAX);
      EDGE_SOFTEN_MODE edge         =D.edgeSoften     (); D.edgeSoften     (EDGE_SOFTEN_SMAA);
      SHADOW_MODE      shadow       =D.shadowMode     (); D.shadowMode     (SHADOW_MAP);
      Vec              ambient_col  =D.ambientColor   (); D.ambientColor   (settings.ambient_col);
      flt              ambient_contr=D.ambientContrast(); D.ambientContrast(settings.ambient_occl);
      Vec2             ambient_range=D.ambientRange   (); D.ambientRange   (settings.ambient_range);
      int              ambient_res  =D.ambientRes     (); D.ambientRes     (0);
      flt              lod_fac      =D.lodFactor      (); D.lodFactor      (0);
      DOF_MODE         dof          =D.dofMode        (); D.dofMode        (DOF_NONE);
      bool             bloom_max    =D.bloomMaximum   ();
      bool             bloom_half   =D.bloomHalf      (), shadow_jitter=D.shadowJitter();
      byte             bloom_blurs  =D.bloomBlurs     (), shadow_soft  =D.shadowSoft  (), shd_map_num=D.shadowMapNum();
      bool             eye_adapt    =D.eyeAdaptation  (); D.eyeAdaptation(false);
      bool             astros       =AstrosDraw         ; AstrosDraw     =false;
      bool             ocean        =Water.draw         ; Water.draw     =false;
      flt              view_from=D.viewFrom(), view_range=D.viewRange(), view_fov=D.viewFov();
      VecI2            view_size;
      Sky.clear();

      D.bloomOriginal(settings.bloom_original).bloomScale(settings.bloom_scale).bloomCut(settings.bloom_cut);
      D.bloomBlurs(0).bloomHalf(true).bloomMaximum(false);
      D.shadowSoft(1).shadowJitter(true).shadowMapNum(6);

      // set object first
      setObj();
      
      // set variation, skeleton & anim
      mesh_variation=0;
      ElmIcon  *data=T.data();
      Skeleton *skel=null; if(T.obj)if(C MeshPtr &mesh=T.obj->mesh())
      {
         skel=mesh->skeleton();
         if(data)mesh_variation=((data->variation_id==ElmIcon::InheritVariation) ? obj->meshVariationIndex() : mesh->variationFind(data->variation_id));
      }
      if(T.skel.skeleton()!=skel || !T.skel.scale() || skel && skel->bones.elms()!=T.skel.bones.elms()){T.skel.create(skel); T.skel_anim.del();}
     ::Animation *anim=null; if(data)if(data->anim_id.valid())anim=Animations(Proj.gamePath(data->anim_id));
      if(skel_anim.animation()!=anim)
         if(skel && anim)skel_anim.create(*skel, *anim);else skel_anim.del();

      flt time=(data ? data->anim_pos : 0);
      T.skel.clear().animate(skel_anim, time).animateRoot(anim, time).updateMatrix(matrix()).updateVelocities(false, false); T.skel.updateVelocities(false, false); // 'updateVelocities' twice to remove motion blur

      // set viewport
      VecI2 size(settings.width, settings.height);
      Rect  rect=D.viewRect();
      Vec2  rect_size=rect.size();
      if(final)
      {
         D.viewForceSquarePixel(true);
         view_size=size; VecI2 screen_size(D.resW(), D.resH());
         if(view_size.x>screen_size.x)view_size=Round(view_size*(flt(screen_size.x)/view_size.x)); // fit X
         if(view_size.y>screen_size.y)view_size=Round(view_size*(flt(screen_size.y)/view_size.y)); // fit Y
         REP(8) // super sample
         {
            VecI2 super_size=view_size*2;
            if(   super_size.x<=screen_size.x && super_size.y<=screen_size.y)view_size=super_size;else break;
         }
         D.viewRect(&RectI(VecI2(0), view_size));
      }else
      if(no_scale)
      {
         Vec2 view_size=D.pixelToScreenSize(size);
         if(view_size.x>rect_size.x)view_size*=rect_size.x/view_size.x; // fit X
         if(view_size.y>rect_size.y)view_size*=rect_size.y/view_size.y; // fit Y
         D.viewRect(Rect_C(rect.center(), view_size));
      }else
      {
         Vec2 view_size=size*rect_size.y/size.y; // scale to fit Y
         if(view_size.x>rect_size.x)view_size*=rect_size.x/view_size.x; // fit X
         D.viewRect(Rect_C(rect.center(), view_size));
      }
      D.viewFov  (settings.fov); // set fov first
      D.viewRange(Max(0.1f, getDist()*5)); // 5 is needed because of 64x320 icons with low FOV
      D.viewFrom (D.viewRange()*0.01f);

      // now after obj and viewport
      Cam.at=settings.cam_focus;
      if(settings.auto_center)Cam.at+=(box()*matrix()).center();
      Cam.setSpherical(Cam.at, settings.cam_angle.x+PI, settings.cam_angle.y, settings.cam_angle.z, getDist()).set();

      if(!final)Renderer(IconEditor::Render);else
      if(data)
      {
         Image col, soft;
         D.edgeSoften(EDGE_SOFTEN_NONE); Renderer(IconEditor::Render); Renderer.capture(col , -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1, true );
         D.edgeSoften(EDGE_SOFTEN_SMAA); Renderer(IconEditor::Render); Renderer.capture(soft, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1, false); // skip alpha for 'soft' to improve performance
         col .crop(col , 0, 0, view_size.x, view_size.y);
         soft.crop(soft, 0, 0, view_size.x, view_size.y);

         // copy alpha from 'col' to 'soft
         REPD(y, col.h())
         REPD(x, col.w())
         {
            Color c=soft.color(x, y); c.a=col.color(x, y).a; soft.color(x, y, c);
         }

         // !! following code assumes that background is black !!
         // setup anti-aliasing according to soft (soft will be darker on edges because it will be softened with black background)
         REPD(y, col.h())
         REPD(x, col.w())
         {
            Color c=col.color(x, y), s=soft.color(x, y);
            if(s.a) // fully opaque
            {
               if(c.r!=s.r || c.g!=s.g || c.b!=s.b) // if different (was affected by softening)
               {
                  for(int sy=y-1; sy<=y+1; sy++)if(InRange(sy, col.h()))
                  for(int sx=x-1; sx<=x+1; sx++)if(InRange(sx, col.w()))if(sx || sy)if(!soft.color(sx, sy).a) // if at least one neighbor is transparent (so we won't adjust opacity of softened pixels which are "inside" the object)
                  {
                     int c_lum=c.r+c.g+c.b, s_lum=s.r+s.g+s.b;
                     if( c_lum)
                     {
                        c.a=Min(DivRound(c.a*s_lum, c_lum), 255);
                        col.color(x, y, c);
                     }
                     goto found_transparent;
                  }
                  col.color(x, y, s); // copy color from soft (because this is softened surrounded by object)
               found_transparent:;
               }
            }else // transparent
            {
               int c_lum=c.lum(), // generated from bloom blur or blend material
                   s_lum=s.lum(); // generated from softening
               if( s_lum>c_lum+3) // softening
               {
                  // find max luminance of nearby opaque colors
                  c_lum=0;
                  for(int sy=y-1; sy<=y+1; sy++)if(InRange(sy, col.h()))
                  for(int sx=x-1; sx<=x+1; sx++)if(InRange(sx, col.w()))if(sx || sy)
                  {
                     Color n=soft.color(sx, sy); if(n.a) // fully opaque neighbor
                     {
                        MAX(c_lum, n.lum());
                     }
                  }
                  if(c_lum)
                  {
                     s=ColorBrightness(s, Min(8, flt(c_lum)/s_lum));
                     s.a=Min(DivRound(255*s_lum, c_lum), 255);
                     col.color(x, y, s);
                  }
               }else
               if(c_lum) // bloom blur / blend material
               {
                  c.a=c_lum;
                  MAX(c_lum, 32); // limit brightness adjustment because without this, artifacts could occur (especially visible when drawing stretched and with compression enabled)
                  c=ColorBrightness(c, 255.0f/c_lum); // increase brightness
                  col.color(x, y, c);
               }
            }
         }
         col.resize(settings.width, settings.height, FILTER_BEST, true, true);
         data->hasAlpha(HasAlpha(col)).hasColor(HasColor(col));
         EditToGameImage(col, col, false, false, settings.type, IMAGE_2D, settings.mip_maps ? 0 : 1, data->hasColor(), data->hasAlpha(), false);
         setChanged(true);
         Save(col, Proj.gamePath(*elm)); Proj.savedGame(*elm);
      }

      D.viewForceSquarePixel(false).viewFrom(view_from).viewRange(view_range).viewFov(view_fov);
      D.   bloomBlurs(bloom_blurs).bloomHalf(bloom_half).bloomMaximum(bloom_max);
      D.   shadowMode(shadow ).shadowSoft(shadow_soft).shadowJitter(shadow_jitter).shadowMapNum(shd_map_num);
      D.    lodFactor(lod_fac);
      D.      dofMode(dof    );
      D.   edgeSoften(edge   );
      D.     bumpMode(bump   );
      D.  ambientMode(ambient).ambientColor(ambient_col).ambientContrast(ambient_contr).ambientRange(ambient_range).ambientRes(ambient_res);
      D.   motionMode(motion );
      D.eyeAdaptation(eye_adapt);
      AstrosDraw      =astros;
      Water.draw      =ocean;
   }
   void IconEditor::makeIcon()
   {
      if(make_do)
      {
         make_do=false;
         if(elm){draw(true); D.viewRect(null);}
      }
   }
   void IconEditor::undoVis() {SetUndo(undos, undo, redo);}
   ElmIcon* IconEditor::data()C {return elm ? elm->iconData() : null;}
   void IconEditor::PreChanged(C Property &prop) {IconEdit.undos.set(&prop);}
   void    IconEditor::Changed(C Property &prop) {IconEdit.setChanged();}
   void IconEditor::Object(  IconEditor &ie, C Str &text) {if(ElmIcon *data=ie.data()){data->obj_id=Proj.findElmID(text, ELM_OBJ); data->obj_time.getUTC();}}
   Str  IconEditor::Object(C IconEditor &ie             ) {if(ElmIcon *data=ie.data())return Proj.elmFullName(data->obj_id); return S;}
   void IconEditor::Variation(  IconEditor &ie, C Str &text) {if(ElmIcon *data=ie.data())if(ie.var){data->variation_id=(InRange(ie.var->combobox(), ie.mesh_variations) ? ie.mesh_variations[ie.var->combobox()].id : ElmIcon::InheritVariation); data->variation_time.getUTC();}}
   Str  IconEditor::Variation(C IconEditor &ie             ) {if(ElmIcon *data=ie.data())REPA(ie.mesh_variations)if(data->variation_id==ie.mesh_variations[i].id)return i; return S;}
   void IconEditor::Settings(  IconEditor &ie, C Str &text) {if(ElmIcon *data=ie.data()){data->icon_settings_id=Proj.findElmID(text, ELM_ICON_SETTS); data->icon_settings_time.getUTC(); ie.reloadSettings();}}
   Str  IconEditor::Settings(C IconEditor &ie             ) {if(ElmIcon *data=ie.data())return Proj.elmFullName(data->icon_settings_id); return S;}
   void IconEditor::Animation(  IconEditor &ie, C Str &text) {if(ElmIcon *data=ie.data()){data->anim_id=Proj.findElmID(text, ELM_ANIM); data->anim_id_time.getUTC();}}
   Str  IconEditor::Animation(C IconEditor &ie             ) {if(ElmIcon *data=ie.data())return Proj.elmFullName(data->anim_id); return S;}
   void IconEditor::AnimPos(  IconEditor &ie, C Str &text) {if(ElmIcon *data=ie.data()){data->anim_pos=TextFlt(text); data->anim_pos_time.getUTC();}}
   Str  IconEditor::AnimPos(C IconEditor &ie             ) {if(ElmIcon *data=ie.data())return data->anim_pos; return S;}
   void IconEditor::Make(IconEditor &editor) {editor.make_do=true;}
   void IconEditor::Undo(IconEditor &editor) {editor.undos.undo();}
   void IconEditor::Redo(IconEditor &editor) {editor.undos.redo();}
   void IconEditor::Locate(IconEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void IconEditor::setObj()
   {
      ElmIcon *data=T.data();
      UID obj_id=UIDZero; if(data)obj_id=data->obj_id; obj=Proj.gamePath(obj_id);
   }
   void IconEditor::meshVariationChanged()
   {
      if(var)
      {
         setObj();
         mesh_variations.setNum(2);
         mesh_variations[0].set("Inherit", ElmIcon::InheritVariation);
         mesh_variations[1].set("Default", 0);
         if(obj)if(C MeshPtr &mesh=obj->mesh())for(int i=1; i<mesh->variations(); i++)mesh_variations.New().set(mesh->variationName(i), mesh->variationID(i));
         var->combobox.setData(mesh_variations, null, true);
      }
   }
   void IconEditor::create()
   {
      add("Object"        , MemberDesc(DATA_STR ).setFunc(Object   , Object   )).elmType(ELM_OBJ);
 var=&add("Variation"     , MemberDesc(         ).setFunc(Variation, Variation)).setEnum();
      add("Icon Settings" , MemberDesc(DATA_STR ).setFunc(Settings , Settings )).elmType(ELM_ICON_SETTS);
      add("Animation"     , MemberDesc(DATA_STR ).setFunc(Animation, Animation)).elmType(ELM_ANIM);
      add("Animation Time", MemberDesc(DATA_REAL).setFunc(AnimPos  , AnimPos  ));
      add();
      add("Display:");
 nos=&add("No Scale", MemberDesc(MEMBER(IconEditor, no_scale)));
      autoData(this);

      flt  h=0.043f;
      Rect r=::PropWin::create("Icon Editor", Vec2(0.02f, -0.07f), 0.036f, h, PropElmNameWidth); ::PropWin::changed(Changed, PreChanged); nos->changed(null, null);
      T+=undo  .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      prop_max_x=r.max.x; button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=make.create(Rect_U(r.down()-Vec2(0, 0.04f), 0.3f, 0.055f), "Create").func(Make, T).desc("Create and save icon using current settings");
      T+=viewport.create(Draw);
      rect(Rect_C(0, 0, Min(1.7f, D.w()*2), Min(1.12f, D.h()*2)));

      ListColumn lc[]=
      {
         ListColumn(MEMBER(ParamEditor::MeshVariation, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      var->combobox.setColumns(lc, Elms(lc), true);
   }
   void IconEditor::toGui() {meshVariationChanged(); ::PropWin::toGui();}
   IconEditor& IconEditor::hide(            )  {set(null); ::PropWin::hide(); return T;}
   Rect        IconEditor::sizeLimit(            )C {Rect r=::EE::Window::sizeLimit(); r.min.set(0.75f, 0.45f); return r;}
   IconEditor& IconEditor::rect(C Rect &rect)  
{
      ::EE::Window::rect(rect);
      viewport.rect(Rect(prop_max_x, -clientHeight(), clientWidth(), 0).extend(-0.02f));
      return T;
   }
   void IconEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmIcon *data=T.data())data->newVer(); // modify just before saving/sending in case we've received data from server after edit
         if(changed_file)Server.setElmLong(elm->id);else Server.setElmShort(elm->id);
      }
      changed=changed_file=false;
   }
   void IconEditor::setChanged(bool file)
   {
      if(elm)
      {
         changed=true; 
         if(ElmIcon *data=T.data())
         {
            data->newVer();
            if(file){data->file_time.getUTC(); changed_file=true;}
         }
      }
   }
   void IconEditor::reloadSettings()
   {
      UID settings_id=UIDZero; if(ElmIcon *data=T.data())settings_id=data->icon_settings_id;
      if(!settings_id.valid())settings.reset();else
      if(IconSettsEdit.elm_id==settings_id)settings=IconSettsEdit.edit;else
                                           settings.load(Proj.editPath(settings_id));
   }
   void IconEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_ICON)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         reloadSettings();
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void IconEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void IconEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void IconEditor::elmChanged(C UID &elm_id)
   {
      if(elm)
      {
         if(elm->id==elm_id){undos.set(null, true); reloadSettings(); toGui();}
         if(ElmIcon *data=T.data())
         {
            if(data->icon_settings_id==elm_id)reloadSettings();
            if(data->         anim_id==elm_id)skel_anim.del();
         }
      }
   }
   void IconEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
IconSettsEditor::IconSettsEditor() : elm_id(UIDZero), elm(null), changed(false), undos(true) {}

IconEditor::IconEditor() : make_do(false), elm_id(UIDZero), elm(null), changed(false), changed_file(false), no_scale(false), prop_max_x(0), var(null), nos(null), mesh_variation(0), undos(true) {}

/******************************************************************************/
