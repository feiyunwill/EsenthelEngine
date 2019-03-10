/******************************************************************************/
class EnvEditor : ClosableWindow
{
   static void PreChanged(C Property &prop) {EnvEdit.undos.set(&prop);}
   static void    Changed(C Property &prop) {EnvEdit.setChanged();}

   class PropWin : .PropWin
   {
      static void Hide(PropWin &pw)
      {
         if(&pw==&EnvEdit.ambient)EnvEdit.bambient.push();
         if(&pw==&EnvEdit.bloom  )EnvEdit.bbloom  .push();
         if(&pw==&EnvEdit.clouds )EnvEdit.bclouds .push();
         if(&pw==&EnvEdit.fog    )EnvEdit.bfog    .push();
         if(&pw==&EnvEdit.sky    )EnvEdit.bsky    .push();
         if(&pw==&EnvEdit.sun    )EnvEdit.bsun    .push();
      }
      PropWin& create(C Str &name, flt value_width)
      {
         super.create(name, Vec2(0.02, -0.02), 0.036, 0.043, value_width); changed(Changed, PreChanged); button[2].func(Hide, T).show();
         return T;
      }
   }
   class Change : Edit._Undo.Change
   {
      EditEnv env;

      virtual void create(ptr user)override
      {
         env=EnvEdit.edit;
         EnvEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         EnvEdit.edit.undo(env);
         EnvEdit.setChanged();
         EnvEdit.toGui();
         EnvEdit.undoVis();
      }
   }

   UID               elm_id=UIDZero;
   Elm              *elm=null;
   bool              changed=false;
   EditEnv           edit;
   Environment       temp;
   EnvironmentPtr    game=&temp;
   Button            undo, redo, locate;
   PropWin           ambient,  bloom,  clouds,  fog,  sky,  sun;
   CheckBox         cambient, cbloom, cclouds, cfog, csky, csun;
   Button           bambient, bbloom, bclouds, bfog, bsky, bsun;
   Vec2              sun_angle=0;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   static void AmbientColor(  EditEnv &env, C Str &text) {env.ambient.color=TextVec(text); env.ambient_color_time.getUTC();}
   static Str  AmbientColor(C EditEnv &env             ) {return Vec4(env.ambient.color, 1);}

   static void NightShadeColor(  EditEnv &env, C Str &text) {env.ambient.night_shade_color=TextVec(text); env.night_shade_color_time.getUTC();}
   static Str  NightShadeColor(C EditEnv &env             ) {return Vec4(env.ambient.night_shade_color, 1);}

   static void BloomOriginal(  EditEnv &env, C Str &text) {env.bloom.original=TextFlt(text); env.bloom_original_time.getUTC();}
   static Str  BloomOriginal(C EditEnv &env             ) {return env.bloom.original;}
   static void BloomScale   (  EditEnv &env, C Str &text) {env.bloom.scale=TextFlt(text); env.bloom_scale_time.getUTC();}
   static Str  BloomScale   (C EditEnv &env             ) {return env.bloom.scale;}
   static void BloomCut     (  EditEnv &env, C Str &text) {env.bloom.cut=TextFlt(text); env.bloom_cut_time.getUTC();}
   static Str  BloomCut     (C EditEnv &env             ) {return env.bloom.cut;}
   static void BloomSaturate(  EditEnv &env, C Str &text) {env.bloom.saturate=TextBool(text); env.bloom_saturate_time.getUTC();}
   static Str  BloomSaturate(C EditEnv &env             ) {return env.bloom.saturate;}
   static void BloomMaximum (  EditEnv &env, C Str &text) {env.bloom.maximum=TextBool(text); env.bloom_maximum_time.getUTC();}
   static Str  BloomMaximum (C EditEnv &env             ) {return env.bloom.maximum;}
   static void BloomHalf    (  EditEnv &env, C Str &text) {env.bloom.half=TextBool(text); env.bloom_half_time.getUTC();}
   static Str  BloomHalf    (C EditEnv &env             ) {return env.bloom.half;}
   static void BloomBlurs   (  EditEnv &env, C Str &text) {env.bloom.blurs=TextInt(text); env.bloom_blurs_time.getUTC();}
   static Str  BloomBlurs   (C EditEnv &env             ) {return env.bloom.blurs;}

   static void CloudsScaleY (  EditEnv &env, C Str &text) {env.clouds.vertical_scale=TextFlt(text); env.clouds_vertical_scale_time.getUTC();}
   static Str  CloudsScaleY (C EditEnv &env             ) {return env.clouds.vertical_scale;}
   static void CloudsRayMask(  EditEnv &env, C Str &text) {env.clouds.ray_mask_contrast=TextFlt(text); env.clouds_ray_mask_contrast_time.getUTC();}
   static Str  CloudsRayMask(C EditEnv &env             ) {return env.clouds.ray_mask_contrast;}

   template<int i>   static void CloudsColor(  EditEnv &env, C Str &text) {env.clouds.layers[i].color=TextVec4(text); env.clouds_color_time[i].getUTC();}
   template<int i>   static Str  CloudsColor(C EditEnv &env             ) {return env.clouds.layers[i].color;}
   template<int i>   static void CloudsScale(  EditEnv &env, C Str &text) {env.clouds.layers[i].scale=TextFlt(text); env.clouds_scale_time[i].getUTC();}
   template<int i>   static Str  CloudsScale(C EditEnv &env             ) {return env.clouds.layers[i].scale;}
   template<int i>   static void CloudsVel  (  EditEnv &env, C Str &text) {env.clouds.layers[i].velocity=TextVec2(text); env.clouds_velocity_time[i].getUTC();}
   template<int i>   static Str  CloudsVel  (C EditEnv &env             ) {return env.clouds.layers[i].velocity;}
   template<int i>   static void CloudsImage(  EditEnv &env, C Str &text) {env.cloud_id[i]=Proj.findElmImageID(text); env.clouds_image_time[i].getUTC();}
   template<int i>   static Str  CloudsImage(C EditEnv &env             ) {return Proj.elmFullName(env.cloud_id[i]);}

   static void FogSky    (  EditEnv &env, C Str &text) {env.fog.affect_sky=TextBool(text); env.fog_affect_sky_time.getUTC();}
   static Str  FogSky    (C EditEnv &env             ) {return env.fog.affect_sky;}
   static void FogDensity(  EditEnv &env, C Str &text) {env.fog.density=TextFlt(text); env.fog_density_time.getUTC();}
   static Str  FogDensity(C EditEnv &env             ) {return (dbl)env.fog.density;}
   static void FogColor  (  EditEnv &env, C Str &text) {env.fog.color=TextVec(text); env.fog_color_time.getUTC();}
   static Str  FogColor  (C EditEnv &env             ) {return Vec4(env.fog.color, 1);}

   static void SkyFrac  (  EditEnv &env, C Str &text) {env.sky.frac=TextFlt(text); env.sky_frac_time.getUTC();}
   static Str  SkyFrac  (C EditEnv &env             ) {return env.sky.frac;}
   static void SkyDnsExp(  EditEnv &env, C Str &text) {env.sky.atmospheric_density_exponent=TextFlt(text); env.sky_atmospheric_density_exponent_time.getUTC();}
   static Str  SkyDnsExp(C EditEnv &env             ) {return (dbl)env.sky.atmospheric_density_exponent;}
   static void SkyHorExp(  EditEnv &env, C Str &text) {env.sky.atmospheric_horizon_exponent=TextFlt(text); env.sky_atmospheric_horizon_exponent_time.getUTC();}
   static Str  SkyHorExp(C EditEnv &env             ) {return env.sky.atmospheric_horizon_exponent;}
   static void SkyHorCol(  EditEnv &env, C Str &text) {env.sky.atmospheric_horizon_color=TextVec4(text); env.sky_atmospheric_horizon_color_time.getUTC();}
   static Str  SkyHorCol(C EditEnv &env             ) {return env.sky.atmospheric_horizon_color;}
   static void SkySkyCol(  EditEnv &env, C Str &text) {env.sky.atmospheric_sky_color=TextVec4(text); env.sky_atmospheric_sky_color_time.getUTC();}
   static Str  SkySkyCol(C EditEnv &env             ) {return env.sky.atmospheric_sky_color;}
   static void SkyStars (  EditEnv &env, C Str &text) {env.star_id=Proj.findElmImageID(text); env.sky_atmospheric_stars_time.getUTC();}
   static Str  SkyStars (C EditEnv &env             ) {return Proj.elmFullName(env.star_id);}
   static void SkyBox   (  EditEnv &env, C Str &text) {env.skybox_id=Proj.findElmImageID(text); env.sky_skybox_time.getUTC();}
   static Str  SkyBox   (C EditEnv &env             ) {return Proj.elmFullName(env.skybox_id);}

   static void SunBlend (  EditEnv &env, C Str &text) {env.sun.blend=TextBool(text); env.sun_blend_time.getUTC();}
   static Str  SunBlend (C EditEnv &env             ) {return env.sun.blend;}
   static void SunGlow  (  EditEnv &env, C Str &text) {env.sun.glow=TextInt(text); env.sun_glow_time.getUTC();}
   static Str  SunGlow  (C EditEnv &env             ) {return env.sun.glow;}
   static void SunSize  (  EditEnv &env, C Str &text) {env.sun.size=TextFlt(text); env.sun_size_time.getUTC();}
   static Str  SunSize  (C EditEnv &env             ) {return env.sun.size;}
   static void SunImage (  EditEnv &env, C Str &text) {env.sun_id=Proj.findElmImageID(text); env.sun_image_time.getUTC();}
   static Str  SunImage (C EditEnv &env             ) {return Proj.elmFullName(env.sun_id);}
   static void SunImageC(  EditEnv &env, C Str &text) {env.sun.image_color=TextVec4(text); env.sun_image_color_time.getUTC();}
   static Str  SunImageC(C EditEnv &env             ) {return env.sun.image_color;}
   static void SunLight (  EditEnv &env, C Str &text) {env.sun.light_color=TextVec(text); env.sun_light_color_time.getUTC();}
   static Str  SunLight (C EditEnv &env             ) {return Vec4(env.sun.light_color, 1);}
   static void SunLitF  (  EditEnv &env, C Str &text) {env.sun.highlight_front=TextFlt(text); env.sun_highlight_front_time.getUTC();}
   static Str  SunLitF  (C EditEnv &env             ) {return env.sun.highlight_front;}
   static void SunLitB  (  EditEnv &env, C Str &text) {env.sun.highlight_back=TextFlt(text); env.sun_highlight_back_time.getUTC();}
   static Str  SunLitB  (C EditEnv &env             ) {return env.sun.highlight_back;}
   static void SunRays  (  EditEnv &env, C Str &text) {env.sun.rays_color=TextVec(text); env.sun_rays_color_time.getUTC();}
   static Str  SunRays  (C EditEnv &env             ) {return Vec4(env.sun.rays_color, 1);}
   static void SunAngleX(  EditEnv &env, C Str &text) {EnvEdit.setSunAngle(DegToRad(TextFlt(text)), EnvEdit.sun_angle.y);}
   static Str  SunAngleX(C EditEnv &env             ) {return TextReal(RadToDeg(EnvEdit.sun_angle.x), 1);}
   static void SunAngleY(  EditEnv &env, C Str &text) {EnvEdit.setSunAngle(EnvEdit.sun_angle.x, DegToRad(TextFlt(text)));}
   static Str  SunAngleY(C EditEnv &env             ) {return TextReal(RadToDeg(EnvEdit.sun_angle.y), 1);}

   static void CAmbient(EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.ambient.on^=1; ee.edit.ambient_on_time.getUTC(); ee.setChanged();}
   static void CBloom  (EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.bloom  .on^=1; ee.edit.  bloom_on_time.getUTC(); ee.setChanged();}
   static void CClouds (EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.clouds .on^=1; ee.edit. clouds_on_time.getUTC(); ee.setChanged();}
   static void CFog    (EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.fog    .on^=1; ee.edit.    fog_on_time.getUTC(); ee.setChanged();}
   static void CSun    (EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.sun    .on^=1; ee.edit.    sun_on_time.getUTC(); ee.setChanged();}
   static void CSky    (EnvEditor &ee) {EnvEdit.undos.set(null, true); ee.edit.sky    .on^=1; ee.edit.    sky_on_time.getUTC(); ee.setChanged();}

   static void BAmbient(EnvEditor &ee) {ee.toggleWin(ee.ambient);}
   static void BBloom  (EnvEditor &ee) {ee.toggleWin(ee.bloom  );}
   static void BClouds (EnvEditor &ee) {ee.toggleWin(ee.clouds );}
   static void BFog    (EnvEditor &ee) {ee.toggleWin(ee.fog    );}
   static void BSun    (EnvEditor &ee) {ee.toggleWin(ee.sun    );}
   static void BSky    (EnvEditor &ee) {ee.toggleWin(ee.sky    );}

   static void Undo  (EnvEditor &editor) {editor.undos.undo();}
   static void Redo  (EnvEditor &editor) {editor.undos.redo();}
   static void Locate(EnvEditor &editor) {Proj.elmLocate(editor.elm_id);}

   Environment* cur() {return (visible() && elm) ? game() : null;}

   Rect getRect(C Rect &rect)
   {
      Memt<Rect> rects;    rects.add(T      .rect());
      if(ambient.visible())rects.add(ambient.rect());
      if(bloom  .visible())rects.add(bloom  .rect());
      if(clouds .visible())rects.add(clouds .rect());
      if(fog    .visible())rects.add(fog    .rect());
      if(sun    .visible())rects.add(sun    .rect());
      if(sky    .visible())rects.add(sky    .rect());

      REPAO(rects).extend(0.05); // ~ shadow border

      return GetRect(rect, rects);
   }
   void toggleWin(GuiObj &obj)
   {
      if(obj.hidden())obj.rect(getRect(obj.rect()));
      obj.visibleToggleActivate();
   }
   void setSunAngle(flt x, flt y)
   {
      sun_angle.set(x, y); edit.sun.pos=Matrix3().setRotateXY(-y, x).z; edit.sun_pos_time.getUTC();
   }

   void create()
   {
      Gui+=super.create("Environment").hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      rect(Rect_RU(D.w(), D.h(), defaultBarFullWidth(), 0.45));
      flt y=-0.095, h=0.052, c=0.044, p=0.02, x=0.07, w=clientWidth()-x-p;
      T+=undo  .create(Rect_LU(0.02, -0.01     , 0.05, 0.05)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05, 0.05)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01, 0), 0.14, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");

      T+=cambient.create(Rect_L(p, y, c, c)).func(CAmbient, T); T+=bambient.create(Rect_L(x, y, w, h), "Ambient").func(BAmbient, T).focusable(false); bambient.mode=BUTTON_TOGGLE; y-=h;
      T+=cbloom  .create(Rect_L(p, y, c, c)).func(CBloom  , T); T+=bbloom  .create(Rect_L(x, y, w, h), "Bloom"  ).func(BBloom  , T).focusable(false); bbloom  .mode=BUTTON_TOGGLE; y-=h;
      T+=cclouds .create(Rect_L(p, y, c, c)).func(CClouds , T); T+=bclouds .create(Rect_L(x, y, w, h), "Clouds" ).func(BClouds , T).focusable(false); bclouds .mode=BUTTON_TOGGLE; y-=h;
      T+=cfog    .create(Rect_L(p, y, c, c)).func(CFog    , T); T+=bfog    .create(Rect_L(x, y, w, h), "Fog"    ).func(BFog    , T).focusable(false); bfog    .mode=BUTTON_TOGGLE; y-=h;
      T+=csun    .create(Rect_L(p, y, c, c)).func(CSun    , T); T+=bsun    .create(Rect_L(x, y, w, h), "Sun"    ).func(BSun    , T).focusable(false); bsun    .mode=BUTTON_TOGGLE; y-=h;
      T+=csky    .create(Rect_L(p, y, c, c)).func(CSky    , T); T+=bsky    .create(Rect_L(x, y, w, h), "Sky"    ).func(BSky    , T).focusable(false); bsky    .mode=BUTTON_TOGGLE; y-=h;
      
      ambient.add("Color"      , MemberDesc(DATA_VEC).setFunc(   AmbientColor,    AmbientColor)).setColor();
      ambient.add("Night Shade", MemberDesc(DATA_VEC).setFunc(NightShadeColor, NightShadeColor)).setColor();
      ambient.autoData(&edit); ambient.create("Ambient", 0.2);

      bloom.add("Original", MemberDesc(DATA_REAL).setFunc(BloomOriginal, BloomOriginal)).range(0, 2).mouseEditSpeed(0.5);
      bloom.add("Scale"   , MemberDesc(DATA_REAL).setFunc(BloomScale   , BloomScale   )).range(0, 2).mouseEditSpeed(0.5);
      bloom.add("Cut"     , MemberDesc(DATA_REAL).setFunc(BloomCut     , BloomCut     )).range(0, 1).mouseEditSpeed(0.4);
      bloom.add("Saturate", MemberDesc(DATA_BOOL).setFunc(BloomSaturate, BloomSaturate));
      bloom.add("Maximum" , MemberDesc(DATA_BOOL).setFunc(BloomMaximum , BloomMaximum ));
      bloom.add("Half Res", MemberDesc(DATA_BOOL).setFunc(BloomHalf    , BloomHalf    ));
      bloom.add("Blurs"   , MemberDesc(DATA_INT ).setFunc(BloomBlurs   , BloomBlurs   )).range(0, 4).mouseEditSpeed(3);
      bloom.autoData(&edit); bloom.create("Bloom", 0.14);

      clouds.add("Vertical Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScaleY , CloudsScaleY )).range(1, 2);
      clouds.add("Ray Mask Contrast", MemberDesc(DATA_REAL).setFunc(CloudsRayMask, CloudsRayMask)).min(1);
      clouds.add();
      clouds.add("Layer 1 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<0>, CloudsColor<0>)).setColor();
      clouds.add("Layer 1 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<0>, CloudsScale<0>)).mouseEditSpeed(0.2);
      clouds.add("Layer 1 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <0>, CloudsVel  <0>)).mouseEditSpeed(0.2);
      clouds.add("Layer 1 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<0>, CloudsImage<0>)).elmType(ELM_IMAGE);
      clouds.add();
      clouds.add("Layer 2 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<1>, CloudsColor<1>)).setColor();
      clouds.add("Layer 2 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<1>, CloudsScale<1>)).mouseEditSpeed(0.2);
      clouds.add("Layer 2 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <1>, CloudsVel  <1>)).mouseEditSpeed(0.2);
      clouds.add("Layer 2 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<1>, CloudsImage<1>)).elmType(ELM_IMAGE);
      clouds.add();
      clouds.add("Layer 3 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<2>, CloudsColor<2>)).setColor();
      clouds.add("Layer 3 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<2>, CloudsScale<2>)).mouseEditSpeed(0.2);
      clouds.add("Layer 3 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <2>, CloudsVel  <2>)).mouseEditSpeed(0.2);
      clouds.add("Layer 3 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<2>, CloudsImage<2>)).elmType(ELM_IMAGE);
      clouds.add();
      clouds.add("Layer 4 Color"   , MemberDesc(DATA_VEC4).setFunc(CloudsColor<3>, CloudsColor<3>)).setColor();
      clouds.add("Layer 4 Scale"   , MemberDesc(DATA_REAL).setFunc(CloudsScale<3>, CloudsScale<3>)).mouseEditSpeed(0.2);
      clouds.add("Layer 4 Velocity", MemberDesc(DATA_VEC2).setFunc(CloudsVel  <3>, CloudsVel  <3>)).mouseEditSpeed(0.2);
      clouds.add("Layer 4 Image"   , MemberDesc(DATA_STR ).setFunc(CloudsImage<3>, CloudsImage<3>)).elmType(ELM_IMAGE);
      clouds.autoData(&edit); clouds.create("Clouds", PropElmNameWidth);

      fog.add("Affect Sky", MemberDesc(DATA_BOOL              ).setFunc(FogSky    , FogSky    ));
      fog.add("Density"   , MemberDesc(DATA_REAL, 0, SIZE(dbl)).setFunc(FogDensity, FogDensity)).range(0, 1).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      fog.add("Color"     , MemberDesc(DATA_VEC               ).setFunc(FogColor  , FogColor  )).setColor();
      fog.autoData(&edit); fog.create("Fog", 0.23);

      sky.add("Fraction"        , MemberDesc(DATA_REAL              ).setFunc(SkyFrac  , SkyFrac  )).range(0, 1).mouseEditSpeed(0.4);
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
   void toGame() {edit.copyTo(*game, Proj);}
   void toGui ()
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

   virtual EnvEditor& hide()override {set(null); ambient.hide(); bloom.hide(); clouds.hide(); fog.hide(); sun.hide(); sky.hide(); super.hide(); return T;}
   virtual EnvEditor& show()override {if(bambient())ambient.show(); if(bbloom())bloom.show(); if(bclouds())clouds.show(); if(bfog())fog.show(); if(bsun())sun.show(); if(bsky())sky.show(); super.show(); return T;}

   void flush()
   {
      if(elm && changed)
      {
         if(ElmEnv *data=elm.envData()){data.newVer(); data.from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(*elm)); // edit
         Save(*game, Proj.gamePath(*elm)); Proj.savedGame(*elm); // game
         Server.setElmLong(elm.id);
      }
      changed=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmEnv *data=elm.envData()){data.newVer(); data.from(edit);}
         toGame();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_ENV)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm)edit.load(Proj.editPath(*elm   ));else edit.reset();
         if(elm)game=     Proj.gamePath( elm.id) ;else game=&temp;
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void toggle  (Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
         EditEnv temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp))toGui();
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
}
EnvEditor EnvEdit;
/******************************************************************************/
