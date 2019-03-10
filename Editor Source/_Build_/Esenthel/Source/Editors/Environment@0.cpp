/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
EnvEditor EnvEdit;
/******************************************************************************/

/******************************************************************************/
   void EnvEditor::PreChanged(C Property &prop) {EnvEdit.undos.set(&prop);}
   void    EnvEditor::Changed(C Property &prop) {EnvEdit.setChanged();}
      void EnvEditor::PropWin::Hide(PropWin &pw)
      {
         if(&pw==&EnvEdit.ambient)EnvEdit.bambient.push();
         if(&pw==&EnvEdit.bloom  )EnvEdit.bbloom  .push();
         if(&pw==&EnvEdit.clouds )EnvEdit.bclouds .push();
         if(&pw==&EnvEdit.fog    )EnvEdit.bfog    .push();
         if(&pw==&EnvEdit.sky    )EnvEdit.bsky    .push();
         if(&pw==&EnvEdit.sun    )EnvEdit.bsun    .push();
      }
      ::EnvEditor::PropWin& EnvEditor::PropWin::create(C Str &name, flt value_width)
      {
         ::PropWin::create(name, Vec2(0.02f, -0.02f), 0.036f, 0.043f, value_width); changed(Changed, PreChanged); button[2].func(Hide, T).show();
         return T;
      }
      void EnvEditor::Change::create(ptr user)
{
         env=EnvEdit.edit;
         EnvEdit.undoVis();
      }
      void EnvEditor::Change::apply(ptr user)
{
         EnvEdit.edit.undo(env);
         EnvEdit.setChanged();
         EnvEdit.toGui();
         EnvEdit.undoVis();
      }
   void EnvEditor::undoVis() {SetUndo(undos, undo, redo);}
   void EnvEditor::AmbientColor(  EditEnv &env, C Str &text) {env.ambient.color=TextVec(text); env.ambient_color_time.getUTC();}
   Str  EnvEditor::AmbientColor(C EditEnv &env             ) {return Vec4(env.ambient.color, 1);}
   void EnvEditor::NightShadeColor(  EditEnv &env, C Str &text) {env.ambient.night_shade_color=TextVec(text); env.night_shade_color_time.getUTC();}
   Str  EnvEditor::NightShadeColor(C EditEnv &env             ) {return Vec4(env.ambient.night_shade_color, 1);}
   void EnvEditor::BloomOriginal(  EditEnv &env, C Str &text) {env.bloom.original=TextFlt(text); env.bloom_original_time.getUTC();}
   Str  EnvEditor::BloomOriginal(C EditEnv &env             ) {return env.bloom.original;}
   void EnvEditor::BloomScale(  EditEnv &env, C Str &text) {env.bloom.scale=TextFlt(text); env.bloom_scale_time.getUTC();}
   Str  EnvEditor::BloomScale(C EditEnv &env             ) {return env.bloom.scale;}
   void EnvEditor::BloomCut(  EditEnv &env, C Str &text) {env.bloom.cut=TextFlt(text); env.bloom_cut_time.getUTC();}
   Str  EnvEditor::BloomCut(C EditEnv &env             ) {return env.bloom.cut;}
   void EnvEditor::BloomSaturate(  EditEnv &env, C Str &text) {env.bloom.saturate=TextBool(text); env.bloom_saturate_time.getUTC();}
   Str  EnvEditor::BloomSaturate(C EditEnv &env             ) {return env.bloom.saturate;}
   void EnvEditor::BloomMaximum(  EditEnv &env, C Str &text) {env.bloom.maximum=TextBool(text); env.bloom_maximum_time.getUTC();}
   Str  EnvEditor::BloomMaximum(C EditEnv &env             ) {return env.bloom.maximum;}
   void EnvEditor::BloomHalf(  EditEnv &env, C Str &text) {env.bloom.half=TextBool(text); env.bloom_half_time.getUTC();}
   Str  EnvEditor::BloomHalf(C EditEnv &env             ) {return env.bloom.half;}
   void EnvEditor::BloomBlurs(  EditEnv &env, C Str &text) {env.bloom.blurs=TextInt(text); env.bloom_blurs_time.getUTC();}
   Str  EnvEditor::BloomBlurs(C EditEnv &env             ) {return env.bloom.blurs;}
   void EnvEditor::CloudsScaleY(  EditEnv &env, C Str &text) {env.clouds.vertical_scale=TextFlt(text); env.clouds_vertical_scale_time.getUTC();}
   Str  EnvEditor::CloudsScaleY(C EditEnv &env             ) {return env.clouds.vertical_scale;}
   void EnvEditor::CloudsRayMask(  EditEnv &env, C Str &text) {env.clouds.ray_mask_contrast=TextFlt(text); env.clouds_ray_mask_contrast_time.getUTC();}
   Str  EnvEditor::CloudsRayMask(C EditEnv &env             ) {return env.clouds.ray_mask_contrast;}
   void EnvEditor::FogSky(  EditEnv &env, C Str &text) {env.fog.affect_sky=TextBool(text); env.fog_affect_sky_time.getUTC();}
   Str  EnvEditor::FogSky(C EditEnv &env             ) {return env.fog.affect_sky;}
   void EnvEditor::FogDensity(  EditEnv &env, C Str &text) {env.fog.density=TextFlt(text); env.fog_density_time.getUTC();}
   Str  EnvEditor::FogDensity(C EditEnv &env             ) {return (dbl)env.fog.density;}
   void EnvEditor::FogColor(  EditEnv &env, C Str &text) {env.fog.color=TextVec(text); env.fog_color_time.getUTC();}
   Str  EnvEditor::FogColor(C EditEnv &env             ) {return Vec4(env.fog.color, 1);}
   void EnvEditor::SkyFrac(  EditEnv &env, C Str &text) {env.sky.frac=TextFlt(text); env.sky_frac_time.getUTC();}
   Str  EnvEditor::SkyFrac(C EditEnv &env             ) {return env.sky.frac;}
   void EnvEditor::SkyDnsExp(  EditEnv &env, C Str &text) {env.sky.atmospheric_density_exponent=TextFlt(text); env.sky_atmospheric_density_exponent_time.getUTC();}
   Str  EnvEditor::SkyDnsExp(C EditEnv &env             ) {return (dbl)env.sky.atmospheric_density_exponent;}
   void EnvEditor::SkyHorExp(  EditEnv &env, C Str &text) {env.sky.atmospheric_horizon_exponent=TextFlt(text); env.sky_atmospheric_horizon_exponent_time.getUTC();}
   Str  EnvEditor::SkyHorExp(C EditEnv &env             ) {return env.sky.atmospheric_horizon_exponent;}
   void EnvEditor::SkyHorCol(  EditEnv &env, C Str &text) {env.sky.atmospheric_horizon_color=TextVec4(text); env.sky_atmospheric_horizon_color_time.getUTC();}
   Str  EnvEditor::SkyHorCol(C EditEnv &env             ) {return env.sky.atmospheric_horizon_color;}
   void EnvEditor::SkySkyCol(  EditEnv &env, C Str &text) {env.sky.atmospheric_sky_color=TextVec4(text); env.sky_atmospheric_sky_color_time.getUTC();}
   Str  EnvEditor::SkySkyCol(C EditEnv &env             ) {return env.sky.atmospheric_sky_color;}
   void EnvEditor::SkyStars(  EditEnv &env, C Str &text) {env.star_id=Proj.findElmImageID(text); env.sky_atmospheric_stars_time.getUTC();}
   Str  EnvEditor::SkyStars(C EditEnv &env             ) {return Proj.elmFullName(env.star_id);}
   void EnvEditor::SkyBox(  EditEnv &env, C Str &text) {env.skybox_id=Proj.findElmImageID(text); env.sky_skybox_time.getUTC();}
   Str  EnvEditor::SkyBox(C EditEnv &env             ) {return Proj.elmFullName(env.skybox_id);}
   void EnvEditor::SunBlend(  EditEnv &env, C Str &text) {env.sun.blend=TextBool(text); env.sun_blend_time.getUTC();}
   Str  EnvEditor::SunBlend(C EditEnv &env             ) {return env.sun.blend;}
   void EnvEditor::SunGlow(  EditEnv &env, C Str &text) {env.sun.glow=TextInt(text); env.sun_glow_time.getUTC();}
   Str  EnvEditor::SunGlow(C EditEnv &env             ) {return env.sun.glow;}
   void EnvEditor::SunSize(  EditEnv &env, C Str &text) {env.sun.size=TextFlt(text); env.sun_size_time.getUTC();}
   Str  EnvEditor::SunSize(C EditEnv &env             ) {return env.sun.size;}
   void EnvEditor::SunImage(  EditEnv &env, C Str &text) {env.sun_id=Proj.findElmImageID(text); env.sun_image_time.getUTC();}
   Str  EnvEditor::SunImage(C EditEnv &env             ) {return Proj.elmFullName(env.sun_id);}
   void EnvEditor::SunImageC(  EditEnv &env, C Str &text) {env.sun.image_color=TextVec4(text); env.sun_image_color_time.getUTC();}
   Str  EnvEditor::SunImageC(C EditEnv &env             ) {return env.sun.image_color;}
   void EnvEditor::SunLight(  EditEnv &env, C Str &text) {env.sun.light_color=TextVec(text); env.sun_light_color_time.getUTC();}
   Str  EnvEditor::SunLight(C EditEnv &env             ) {return Vec4(env.sun.light_color, 1);}
   void EnvEditor::SunLitF(  EditEnv &env, C Str &text) {env.sun.highlight_front=TextFlt(text); env.sun_highlight_front_time.getUTC();}
   Str  EnvEditor::SunLitF(C EditEnv &env             ) {return env.sun.highlight_front;}
   void EnvEditor::SunLitB(  EditEnv &env, C Str &text) {env.sun.highlight_back=TextFlt(text); env.sun_highlight_back_time.getUTC();}
   Str  EnvEditor::SunLitB(C EditEnv &env             ) {return env.sun.highlight_back;}
   void EnvEditor::SunRays(  EditEnv &env, C Str &text) {env.sun.rays_color=TextVec(text); env.sun_rays_color_time.getUTC();}
   Str  EnvEditor::SunRays(C EditEnv &env             ) {return Vec4(env.sun.rays_color, 1);}
   void EnvEditor::SunAngleX(  EditEnv &env, C Str &text) {EnvEdit.setSunAngle(DegToRad(TextFlt(text)), EnvEdit.sun_angle.y);}
   Str  EnvEditor::SunAngleX(C EditEnv &env             ) {return TextReal(RadToDeg(EnvEdit.sun_angle.x), 1);}
   void EnvEditor::SunAngleY(  EditEnv &env, C Str &text) {EnvEdit.setSunAngle(EnvEdit.sun_angle.x, DegToRad(TextFlt(text)));}
   Str  EnvEditor::SunAngleY(C EditEnv &env             ) {return TextReal(RadToDeg(EnvEdit.sun_angle.y), 1);}
   void EnvEditor::CAmbient(EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.ambient.on^=1; ee.edit.ambient_on_time.getUTC(); ee.setChanged();}
   void EnvEditor::CBloom(EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.bloom  .on^=1; ee.edit.  bloom_on_time.getUTC(); ee.setChanged();}
   void EnvEditor::CClouds(EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.clouds .on^=1; ee.edit. clouds_on_time.getUTC(); ee.setChanged();}
   void EnvEditor::CFog(EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.fog    .on^=1; ee.edit.    fog_on_time.getUTC(); ee.setChanged();}
   void EnvEditor::CSun(EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.sun    .on^=1; ee.edit.    sun_on_time.getUTC(); ee.setChanged();}
   void EnvEditor::CSky(EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.sky    .on^=1; ee.edit.    sky_on_time.getUTC(); ee.setChanged();}
   void EnvEditor::BAmbient(EnvEditor &ee) {ee.toggleWin(ee.ambient);}
   void EnvEditor::BBloom(EnvEditor &ee) {ee.toggleWin(ee.bloom  );}
   void EnvEditor::BClouds(EnvEditor &ee) {ee.toggleWin(ee.clouds );}
   void EnvEditor::BFog(EnvEditor &ee) {ee.toggleWin(ee.fog    );}
   void EnvEditor::BSun(EnvEditor &ee) {ee.toggleWin(ee.sun    );}
   void EnvEditor::BSky(EnvEditor &ee) {ee.toggleWin(ee.sky    );}
   void EnvEditor::Undo(EnvEditor &editor) {editor.undos.undo();}
   void EnvEditor::Redo(EnvEditor &editor) {editor.undos.redo();}
   void EnvEditor::Locate(EnvEditor &editor) {Proj.elmLocate(editor.elm_id);}
   Environment* EnvEditor::cur() {return (visible() && elm) ? game() : null;}
   Rect EnvEditor::getRect(C Rect &rect)
   {
      Memt<Rect> rects;    rects.add(T      .rect());
      if(ambient.visible())rects.add(ambient.rect());
      if(bloom  .visible())rects.add(bloom  .rect());
      if(clouds .visible())rects.add(clouds .rect());
      if(fog    .visible())rects.add(fog    .rect());
      if(sun    .visible())rects.add(sun    .rect());
      if(sky    .visible())rects.add(sky    .rect());

      REPAO(rects).extend(0.05f); // ~ shadow border

      return GetRect(rect, rects);
   }
   void EnvEditor::toggleWin(GuiObj &obj)
   {
      if(obj.hidden())obj.rect(getRect(obj.rect()));
      obj.visibleToggleActivate();
   }
   void EnvEditor::setSunAngle(flt x, flt y)
   {
      sun_angle.set(x, y); edit.sun.pos=Matrix3().setRotateXY(-y, x).z; edit.sun_pos_time.getUTC();
   }
   void EnvEditor::create()
   {
      Gui+=::EE::Window::create("Environment").hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      rect(Rect_RU(D.w(), D.h(), defaultBarFullWidth(), 0.45f));
      flt y=-0.095f, h=0.052f, c=0.044f, p=0.02f, x=0.07f, w=clientWidth()-x-p;
      T+=undo  .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");

      T+=cambient.create(Rect_L(p, y, c, c)).func(CAmbient, T); T+=bambient.create(Rect_L(x, y, w, h), "Ambient").func(BAmbient, T).focusable(false); bambient.mode=BUTTON_TOGGLE; y-=h;
      T+=cbloom  .create(Rect_L(p, y, c, c)).func(CBloom  , T); T+=bbloom  .create(Rect_L(x, y, w, h), "Bloom"  ).func(BBloom  , T).focusable(false); bbloom  .mode=BUTTON_TOGGLE; y-=h;
      T+=cclouds .create(Rect_L(p, y, c, c)).func(CClouds , T); T+=bclouds .create(Rect_L(x, y, w, h), "Clouds" ).func(BClouds , T).focusable(false); bclouds .mode=BUTTON_TOGGLE; y-=h;
      T+=cfog    .create(Rect_L(p, y, c, c)).func(CFog    , T); T+=bfog    .create(Rect_L(x, y, w, h), "Fog"    ).func(BFog    , T).focusable(false); bfog    .mode=BUTTON_TOGGLE; y-=h;
      T+=csun    .create(Rect_L(p, y, c, c)).func(CSun    , T); T+=bsun    .create(Rect_L(x, y, w, h), "Sun"    ).func(BSun    , T).focusable(false); bsun    .mode=BUTTON_TOGGLE; y-=h;
      T+=csky    .create(Rect_L(p, y, c, c)).func(CSky    , T); T+=bsky    .create(Rect_L(x, y, w, h), "Sky"    ).func(BSky    , T).focusable(false); bsky    .mode=BUTTON_TOGGLE; y-=h;
      
      ambient.add("Color"      , MemberDesc(DATA_VEC).setFunc(   AmbientColor,    AmbientColor)).setColor();
      ambient.add("Night Shade", MemberDesc(DATA_VEC).setFunc(NightShadeColor, NightShadeColor)).setColor();
      ambient.autoData(&edit); ambient.create("Ambient", 0.2f);

      bloom.add("Original", MemberDesc(DATA_REAL).setFunc(BloomOriginal, BloomOriginal)).range(0, 2).mouseEditSpeed(0.5f);
      bloom.add("Scale"   , MemberDesc(DATA_REAL).setFunc(BloomScale   , BloomScale   )).range(0, 2).mouseEditSpeed(0.5f);
      bloom.add("Cut"     , MemberDesc(DATA_REAL).setFunc(BloomCut     , BloomCut     )).range(0, 1).mouseEditSpeed(0.4f);
      bloom.add("Saturate", MemberDesc(DATA_BOOL).setFunc(BloomSaturate, BloomSaturate));
      bloom.add("Maximum" , MemberDesc(DATA_BOOL).setFunc(BloomMaximum , BloomMaximum ));
      bloom.add("Half Res", MemberDesc(DATA_BOOL).setFunc(BloomHalf    , BloomHalf    ));
      bloom.add("Blurs"   , MemberDesc(DATA_INT ).setFunc(BloomBlurs   , BloomBlurs   )).range(0, 4).mouseEditSpeed(3);
      bloom.autoData(&edit); bloom.create("Bloom", 0.14f);

      clouds.add("Vertical Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScaleY , CloudsScaleY )).range(1, 2);
      clouds.add("Ray Mask Contrast", MemberDesc(DATA_REAL).setFunc(CloudsRayMask, CloudsRayMask)).min(1);
      clouds.add();
      clouds.add("Layer 1 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<0>, CloudsColor<0>)).setColor();
      clouds.add("Layer 1 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<0>, CloudsScale<0>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 1 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <0>, CloudsVel  <0>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 1 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<0>, CloudsImage<0>)).elmType(ELM_IMAGE);
      clouds.add();
      clouds.add("Layer 2 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<1>, CloudsColor<1>)).setColor();
      clouds.add("Layer 2 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<1>, CloudsScale<1>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 2 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <1>, CloudsVel  <1>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 2 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<1>, CloudsImage<1>)).elmType(ELM_IMAGE);
      clouds.add();
      clouds.add("Layer 3 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<2>, CloudsColor<2>)).setColor();
      clouds.add("Layer 3 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<2>, CloudsScale<2>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 3 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <2>, CloudsVel  <2>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 3 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<2>, CloudsImage<2>)).elmType(ELM_IMAGE);
      clouds.add();
      clouds.add("Layer 4 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<3>, CloudsColor<3>)).setColor();
      clouds.add("Layer 4 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<3>, CloudsScale<3>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 4 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <3>, CloudsVel  <3>)).mouseEditSpeed(0.2f);
      clouds.add("Layer 4 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<3>, CloudsImage<3>)).elmType(ELM_IMAGE);
      clouds.autoData(&edit); clouds.create("Clouds", PropElmNameWidth);

      fog.add("Affect Sky", MemberDesc(DATA_BOOL              ).setFunc(FogSky    , FogSky    ));
      fog.add("Density"   , MemberDesc(DATA_REAL, 0, SIZE(dbl)).setFunc(FogDensity, FogDensity)).range(0, 1).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      fog.add("Color"     , MemberDesc(DATA_VEC               ).setFunc(FogColor  , FogColor  )).setColor();
      fog.autoData(&edit); fog.create("Fog", 0.23f);

      sky.add("Fraction"        , MemberDesc(DATA_REAL              ).setFunc(SkyFrac  , SkyFrac  )).range(0, 1).mouseEditSpeed(0.4f);
      sky.add();
      sky.add("Density Exponent", MemberDesc(DATA_REAL, 0, SIZE(dbl)).setFunc(SkyDnsExp, SkyDnsExp)).range(0, 1).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      sky.add("Horizon Exponent", MemberDesc(DATA_REAL              ).setFunc(SkyHorExp, SkyHorExp)).min  (0   ).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      sky.add("Horizon Color"   , MemberDesc(DATA_VEC4              ).setFunc(SkyHorCol, SkyHorCol)).setColor();
      sky.add("Sky Color"       , MemberDesc(DATA_VEC4              ).setFunc(SkySkyCol, SkySkyCol)).setColor();
      sky.add("Stars"           , MemberDesc(DATA_STR               ).setFunc(SkyStars , SkyStars )).elmType(ELM_IMAGE);
      sky.add();
      sky.add("Skybox"          , MemberDesc(DATA_STR               ).setFunc(SkyBox   , SkyBox   )).elmType(ELM_IMAGE);
      sky.autoData(&edit); sky.create("Sky", PropElmNameWidth);

      sun.add("Blend"          , MemberDesc(DATA_BOOL).setFunc(SunBlend , SunBlend )).desc("If use blending for image.\nIf true then image will be applied using alpha blending,\nif false then image will be added onto the screen.");
      sun.add("Glow"           , MemberDesc(DATA_INT ).setFunc(SunGlow  , SunGlow  )).range(0, 255);
      sun.add("Size"           , MemberDesc(DATA_REAL).setFunc(SunSize  , SunSize  )).range(0, 1);
      sun.add("Image"          , MemberDesc(DATA_STR ).setFunc(SunImage , SunImage )).elmType(ELM_IMAGE);
      sun.add("Image Color"    , MemberDesc(DATA_VEC4).setFunc(SunImageC, SunImageC)).setColor();
      sun.add("Light Color"    , MemberDesc(DATA_VEC4).setFunc(SunLight , SunLight )).setColor();
      sun.add("Highlight Front", MemberDesc(DATA_REAL).setFunc(SunLitF  , SunLitF  )).range(0, 1);
      sun.add("Highlight Back" , MemberDesc(DATA_REAL).setFunc(SunLitB  , SunLitB  )).range(0, 1);
      sun.add("Rays Color"     , MemberDesc(DATA_VEC4).setFunc(SunRays  , SunRays  )).setColor();
      sun.add("Angle X"        , MemberDesc(DATA_REAL).setFunc(SunAngleX, SunAngleX)).mouseEditSpeed(20);
      sun.add("Angle Y"        , MemberDesc(DATA_REAL).setFunc(SunAngleY, SunAngleY)).mouseEditSpeed(20);
      sun.autoData(&edit); sun.create("Sun", PropElmNameWidth);
   }
   void EnvEditor::toGame() {edit.copyTo(*game, Proj);}
   void EnvEditor::toGui()
   {
      sun_angle=Matrix3().setDir(edit.sun.pos).angles().yx(); CHS(sun_angle.y);
      cambient.set(edit.ambient.on, QUIET); ambient.toGui();
      cbloom  .set(edit.bloom  .on, QUIET); bloom  .toGui();
      cclouds .set(edit.clouds .on, QUIET); clouds .toGui();
      cfog    .set(edit.fog    .on, QUIET); fog    .toGui();
      csky    .set(edit.sky    .on, QUIET); sky    .toGui();
      csun    .set(edit.sun    .on, QUIET); sun    .toGui();
      toGame();
   }
   EnvEditor& EnvEditor::hide(){set(null); ambient.hide(); bloom.hide(); clouds.hide(); fog.hide(); sun.hide(); sky.hide(); ::EE::Window::hide(); return T;}
   EnvEditor& EnvEditor::show(){if(bambient())ambient.show(); if(bbloom())bloom.show(); if(bclouds())clouds.show(); if(bfog())fog.show(); if(bsun())sun.show(); if(bsky())sky.show(); ::EE::Window::show(); return T;}
   void EnvEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmEnv *data=elm->envData()){data->newVer(); data->from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(*elm)); // edit
         Save(*game, Proj.gamePath(*elm)); Proj.savedGame(*elm); // game
         Server.setElmLong(elm->id);
      }
      changed=false;
   }
   void EnvEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmEnv *data=elm->envData()){data->newVer(); data->from(edit);}
         toGame();
      }
   }
   void EnvEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_ENV)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm)edit.load(Proj.editPath(*elm   ));else edit.reset();
         if(elm)game=     Proj.gamePath( elm->id) ;else game=&temp;
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void EnvEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void EnvEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void EnvEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         undos.set(null, true);
         EditEnv temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp))toGui();
      }
   }
   void EnvEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
EnvEditor::EnvEditor() : elm_id(UIDZero), elm(null), changed(false), game(&temp), sun_angle(0), undos(true) {}

/******************************************************************************/
