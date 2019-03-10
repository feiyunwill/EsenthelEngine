/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
TextData Settings;
/******************************************************************************/
bool SaveSettings(C Str &name)
{
   TextData data;

   TextNode &main=data.getNode("Main");
   {
      main.getNode("ProjectsPath"    ).value=RelativePath(ProjectsPath);
      main.getNode("ServerAddress"   ).value=Projs.server();
      main.getNode("Email"           ).value=Projs.email ();
      main.getNode("NetworkInterface").value=Projs.editor_network_interface();
      TextNode &pl=main.getNode("ProjectList");
      {
         pl.getNode(   "ChildrenFileSize"        ).value=Proj.list.ics;
         pl.getNode(    "TextureFileSize"        ).value=Proj.list.its;
         pl.getNode(    "TextureFileSizeInOjects").value=Proj.list.include_texture_size_in_object;
         pl.getNode("UnpublishedFileSize"        ).value=Proj.list.include_unpublished_elm_size;
      }
      TextNode &fp=main.getNode("FilePaths");
      {
         TextNode &favorite=fp.getNode("Favorite"); FREPA(WindowIOFavorites)favorite.nodes.New().set("Path", WindowIOFavorites[i]);
         TextNode &recent  =fp.getNode("Recent"  ); FREPA(WindowIORecents  )recent  .nodes.New().set("Path", WindowIORecents  [i]);
      }
   }

   TextNode &store=data.getNode("Store");
   {
      store.getNode("Email").value=AppStore.login.email();
   }

   TextNode &object=data.getNode("Object");
   {
      TextNode &general=object.getNode("General");
      general.nodes.New().set("ActiveViewport"   , ObjEdit.v4.sel());
      general.nodes.New().set("CameraMoveMode"   , ObjEdit.v4.  moveMode());
      general.nodes.New().set("CameraOrientation", ObjEdit.v4.orientMode());
      general.nodes.New().set("CameraZoom"       , ObjEdit.v4.  drawZoom());
   }

   TextNode &anim=data.getNode("Animation");
   {
      TextNode &general=anim.getNode("General");
      general.nodes.New().set("ActiveViewport"   , AnimEdit.v4.sel());
      general.nodes.New().set("CameraMoveMode"   , AnimEdit.v4.  moveMode());
      general.nodes.New().set("CameraOrientation", AnimEdit.v4.orientMode());
      general.nodes.New().set("CameraZoom"       , AnimEdit.v4.  drawZoom());
   }

   TextNode &world=data.getNode("World");
   {
      TextNode &brush=world.getNode("Brush");
      brush.nodes.New().set("SizeProportional", Brush.bsize());
      brush.nodes.New().set("Image"           , RelativePath(Brush.image_name));

      TextNode &colors=world.getNode("Colors");
      FREPA(ColorBrush.col)colors.nodes.New().set("Color", ColorBrush.col[i].col);

      TextNode &general=world.getNode("General");
    //general.nodes.New().set("CursorCollidesWithHeightmaps", MainMenu("WE Edit/Cursor Collides with Heightmaps"));
    //general.nodes.New().set("CursorCollidesWithObjects"   , MainMenu("WE Edit/Cursor Collides with Objects"));
      general.nodes.New().set("VisibleRadius"               , WorldEdit.visibleRadius());
      general.nodes.New().set("ActiveViewport"              , WorldEdit.v4.sel());
      general.nodes.New().set("CameraMoveMode"              , WorldEdit.v4.  moveMode());
      general.nodes.New().set("CameraOrientation"           , WorldEdit.v4.orientMode());
      general.nodes.New().set("CameraZoom"                  , WorldEdit.v4.  drawZoom());
      general.nodes.New().set("CameraSpherical"             , WorldEdit.cam_spherical());
      general.nodes.New().set("CameraFppSpeed"              , WorldEdit.fpp_speed());
   }

   TextNode &code=data.getNode("Code");
   CodeEdit.saveSettings(code);

   TextNode &video=data.getNode("Video");
   {
      video.nodes.New().set("ScreenX"                 , D.resW());
      video.nodes.New().set("ScreenY"                 , D.resH());
      video.nodes.New().set("Fullscreen"              , D.full());
      video.nodes.New().set("Exclusive"               , D.exclusive());
      video.nodes.New().set("Synchronization"         , D.sync());
      video.nodes.New().set("Renderer"                , Renderer.type());
      video.nodes.New().set("EdgeSoftening"           , D.edgeSoften());
      video.nodes.New().set("Shadows"                 , D.shadowMode()==SHADOW_MAP);
      video.nodes.New().set("ShadowMapSize"           , D.shadowMapSize());
      video.nodes.New().set("ShadowMapNum"            , D.shadowMapNum());
      video.nodes.New().set("ShadowSoft"              , D.shadowSoft());
      video.nodes.New().set("ShadowJitter"            , D.shadowJitter());
      video.nodes.New().set("ShadowReduceFlicker"     , D.shadowReduceFlicker());
      video.nodes.New().set("ShadowRangeFraction"     , D.shadowFrac());
      video.nodes.New().set("ShadowFadeFraction"      , D.shadowFade());
      video.nodes.New().set("BumpMapping"             , D.bumpMode());
      video.nodes.New().set("MotionBlur"              , D.motionMode());
      video.nodes.New().set("BloomSaturate"           , DefaultEnvironment.bloom.saturate);
      video.nodes.New().set("AmbientLight"            , DefaultEnvironment.ambient.color.max());
      video.nodes.New().set("AmbientOcclusion"        , D.ambientMode());
      video.nodes.New().set("AmbientOcclusionRange"   , D.ambientRange().x);
      video.nodes.New().set("AmbientOcclusionContrast", D.ambientContrast());
      video.nodes.New().set("EyeAdaptation"           , D.eyeAdaptation());
      video.nodes.New().set("EyeAdaptationBrightness" , D.eyeAdaptationBrightness());
      video.nodes.New().set("MonitorPrecision"        , D.monitorPrecision());
      video.nodes.New().set("Dither"                  , D.dither());
      video.nodes.New().set("ColRTPrecision"          , D.highPrecColRT    ());
      video.nodes.New().set("NrmCalcPrecision"        , D.highPrecNrmCalc  ());
      video.nodes.New().set("NrmRTPrecision"          , D.highPrecNrmRT    ());
      video.nodes.New().set("LumRTPrecision"          , D.highPrecLumRT    ());
      video.nodes.New().set("LitColRTPrecision"       , D.litColRTPrecision());
      video.nodes.New().set("GuiScale"                , VidOpt.scale);
      video.nodes.New().set("GuiScaleWin"             , VidOpt.scale_win);
      video.nodes.New().set("GuiSkin"                 , VidOpt.skinName());
      video.nodes.New().set("FieldOfView"             , RadToDeg(VidOpt.advanced.fov));
      video.nodes.New().set("TextureFiltering"        , D.texFilter());
      video.nodes.New().set("TextureMipFiltering"     , D.texMipFilter());
      video.nodes.New().set("DetailTexture"           , D.texDetail());
      video.nodes.New().set("Samples"                 , D.samples());
      video.nodes.New().set("Density"                 , D.density());
      video.nodes.New().set("DensityFilter"           , VidOpt.advanced.DensityFilter(VidOpt.advanced));
      video.nodes.New().set("SoftParticles"           , D.particlesSoft());
      video.nodes.New().set("GrassRange"              , D.grassRange());
      video.nodes.New().set("MaxLights"               , D.maxLights());
      video.nodes.New().set("EdgeDetect"              , D.edgeDetect());
      video.nodes.New().set("AllowGlow"               , D.glowAllow());
      video.nodes.New().set("SimpleRendererPerPixel"  , Renderer.simplePrecision());
      video.nodes.New().set("MaterialBlendPerPixel"   , D.materialBlend());
   }

   TextNode &theater=data.getNode("Theater");
   SaveProperties(Theater.options.props, theater.nodes);
   if(TextNode *env=theater.findNode("Environment"))theater.nodes.removeData(env, true); // don't store project element
   theater.nodes.New().set("Mode", Theater.mode());

   return data.save(name);
}
/******************************************************************************/
bool LoadSettings(C Str &name)
{
   return Settings.load(name);
}
void ApplyVideoSettings(C TextData &data)
{
   if(C TextNode *video=data.findNode("Video"))
   {
      int x=-1, y=-1, full=-1;
      if(C TextParam *p=video->findNode("ScreenX"                 ))x=p->asInt();
      if(C TextParam *p=video->findNode("ScreenY"                 ))y=p->asInt();
      if(C TextParam *p=video->findNode("Fullscreen"              ))full=p->asBool();
      if(C TextParam *p=video->findNode("Exclusive"               ))D.exclusive(p->asBool());
      if(C TextParam *p=video->findNode("Synchronization"         ))D.sync(p->asBool());
      if(C TextParam *p=video->findNode("Renderer"                ))Renderer.type(RENDER_TYPE(p->asInt()));
      if(C TextParam *p=video->findNode("EdgeSoftening"           ))D.edgeSoften(EDGE_SOFTEN_MODE(p->asInt()));
      if(C TextParam *p=video->findNode("Shadows"                 ))D.shadowMode(p->asBool() ? SHADOW_MAP : SHADOW_NONE);
      if(C TextParam *p=video->findNode("ShadowMapSize"           ))D.shadowMapSize(p->asInt());
      if(C TextParam *p=video->findNode("ShadowMapNum"            ))D.shadowMapNum(p->asInt());
      if(C TextParam *p=video->findNode("ShadowSoft"              ))D.shadowSoft(p->asInt());
      if(C TextParam *p=video->findNode("ShadowJitter"            ))D.shadowJitter(p->asBool());
      if(C TextParam *p=video->findNode("ShadowReduceFlicker"     ))D.shadowReduceFlicker(p->asBool());
      if(C TextParam *p=video->findNode("ShadowRangeFraction"     ))D.shadowFrac(p->asFlt());
      if(C TextParam *p=video->findNode("ShadowFadeFraction"      ))D.shadowFade(p->asFlt());
      if(C TextParam *p=video->findNode("BumpMapping"             ))D.bumpMode(BUMP_MODE(p->asInt()));
      if(C TextParam *p=video->findNode("MotionBlur"              ))D.motionMode(MOTION_MODE(p->asInt()));
      if(C TextParam *p=video->findNode("AmbientOcclusion"        ))D.ambientMode    (AMBIENT_MODE(p->asInt()));
      if(C TextParam *p=video->findNode("AmbientOcclusionRange"   ))D.ambientRange   (p->asFlt());
      if(C TextParam *p=video->findNode("AmbientOcclusionContrast"))D.ambientContrast(p->asFlt());
      if(C TextParam *p=video->findNode("EyeAdaptation"           ))D.eyeAdaptation(p->asBool());
      if(C TextParam *p=video->findNode("EyeAdaptationBrightness" ))D.eyeAdaptationBrightness(p->asFlt());
      if(C TextParam *p=video->findNode("MonitorPrecision"        ))D.monitorPrecision(IMAGE_PRECISION(p->asInt()));
      if(C TextParam *p=video->findNode("Dither"                  ))D.dither(p->asBool());
      if(C TextParam *p=video->findNode("ColRTPrecision"          ))D.highPrecColRT    (p->asBool());
      if(C TextParam *p=video->findNode("NrmCalcPrecision"        ))D.highPrecNrmCalc  (p->asBool());
      if(C TextParam *p=video->findNode("NrmRTPrecision"          ))D.highPrecNrmRT    (p->asBool());
      if(C TextParam *p=video->findNode("LumRTPrecision"          ))D.highPrecLumRT    (p->asBool());
      if(C TextParam *p=video->findNode("LitColRTPrecision"       ))D.litColRTPrecision(IMAGE_PRECISION(p->asInt()));
      if(C TextParam *p=video->findNode("FieldOfView"             ))VidOpt.advanced.setFov(DegToRad(p->asFlt()));
      if(C TextParam *p=video->findNode("TextureFiltering"        ))D.texFilter(p->asInt());
      if(C TextParam *p=video->findNode("TextureMipFiltering"     ))D.texMipFilter(p->asBool());
      if(C TextParam *p=video->findNode("DetailTexture"           ))D.texDetail(TEXTURE_USAGE(p->asInt()));
      if(C TextParam *p=video->findNode("Samples"                 ))D.samples(p->asInt());
      if(C TextParam *p=video->findNode("Density"                 ))D.density(p->asFlt());
      if(C TextParam *p=video->findNode("DensityFilter"           ))VidOpt.advanced.DensityFilter(VidOpt.advanced, p->asText());
      if(C TextParam *p=video->findNode("SoftParticles"           ))D.particlesSoft(p->asBool());
      if(C TextParam *p=video->findNode("GrassRange"              ))D.grassRange(p->asFlt());
      if(C TextParam *p=video->findNode("MaxLights"               ))D.maxLights(p->asInt());
      if(C TextParam *p=video->findNode("EdgeDetect"              ))D.edgeDetect(EDGE_DETECT_MODE(p->asInt()));
      if(C TextParam *p=video->findNode("AllowGlow"               ))D.glowAllow(p->asBool());
      if(C TextParam *p=video->findNode("SimpleRendererPerPixel"  ))Renderer.simplePrecision(p->asBool());
      if(C TextParam *p=video->findNode("MaterialBlendPerPixel"   ))D.materialBlend(p->asBool());
      D.mode(x, y, full);

      // following needs to be called after setting resolution
      if(C TextParam *p=video->findNode("GuiScale"   ))VidOpt.setScale   (p->asFlt ());
      if(C TextParam *p=video->findNode("GuiScaleWin"))VidOpt.setScaleWin(p->asBool());
   }
}
void ApplySettings(C TextData &data)
{
   if(C TextNode *main=data.findNode("Main"))
   {
      if(C TextParam *p=main->findNode("ProjectsPath"        ))Projs.proj_path.set(MakeFullPath(p->value));
      if(C TextParam *p=main->findNode("ServerAddress"       ))Projs.server   .set(p->value);
      if(C TextParam *p=main->findNode("Email"               ))Projs.email    .set(p->value);
      if(C TextParam *p=main->findNode("NetworkInterface"    ))Projs.editor_network_interface.set(p->asBool(), QUIET);

      if(C TextNode *pl=main->findNode("ProjectList"))
      {
         if(C TextParam *p=pl->findNode(   "ChildrenFileSize"        ))Proj.includeChildrenSize       (ProjectEx::ElmList::INCLUDE_CHILDREN_SIZE(p->asInt()));
         if(C TextParam *p=pl->findNode(    "TextureFileSize"        ))Proj.includeTextureSize        (ProjectEx::ElmList::INCLUDE_TEXTURE_SIZE (p->asInt()));
         if(C TextParam *p=pl->findNode(    "TextureFileSizeInOjects"))Proj.includeTextureSizeInObject(p->asBool());
         if(C TextParam *p=pl->findNode("UnpublishedFileSize"        ))Proj.includeUnpublishedElmSize (p->asBool());
      }
      if(C TextNode *fp=main->findNode("FilePaths"))
      {
         if(C TextNode *favorite=fp->findNode("Favorite"))for(int i=0; C TextParam *path=favorite->findNode("Path", i); i++)WindowIOFavorites.include(path->value);
         if(C TextNode *recent  =fp->findNode("Recent"  ))for(int i=0; C TextParam *path=recent  ->findNode("Path", i); i++)WindowIORecents  .include(path->value);
      }
   }
   if(C TextNode *video=data.findNode("Video"))
   {
      if(C TextParam *p=video->findNode("BloomSaturate"))DefaultEnvironment.bloom.saturate=p->asBool();
      if(C TextParam *p=video->findNode("AmbientLight" ))DefaultEnvironment.ambient.color =p->asFlt ();
   }
   if(C TextNode *store=data.findNode("Store"))
   {
      if(C TextParam *p=store->findNode("Email"))AppStore.login.email.set(p->value);
   }
   if(C TextNode *object=data.findNode("Object"))
   {
      if(C TextNode *general=object->findNode("General"))
      {
         if(C TextParam *p=general->findNode("ActiveViewport"   ))ObjEdit.   setView(p->asInt());
         if(C TextParam *p=general->findNode("CameraMoveMode"   ))ObjEdit.  moveMode(Edit::Viewport4::  MOVE_MODE(p->asInt ()));
         if(C TextParam *p=general->findNode("CameraOrientation"))ObjEdit.orientMode(Edit::Viewport4::ORIENT_MODE(p->asInt ()));
         if(C TextParam *p=general->findNode("CameraZoom"       ))ObjEdit.      zoom(                           p->asBool() );
      }
   }
   if(C TextNode *anim=data.findNode("Animation"))
   {
      if(C TextNode *general=anim->findNode("General"))
      {
         if(C TextParam *p=general->findNode("ActiveViewport"   ))AnimEdit.   setView(p->asInt());
         if(C TextParam *p=general->findNode("CameraMoveMode"   ))AnimEdit.  moveMode(Edit::Viewport4::  MOVE_MODE(p->asInt ()));
         if(C TextParam *p=general->findNode("CameraOrientation"))AnimEdit.orientMode(Edit::Viewport4::ORIENT_MODE(p->asInt ()));
         if(C TextParam *p=general->findNode("CameraZoom"       ))AnimEdit.      zoom(                           p->asBool() );
      }
   }
   if(C TextNode *world=data.findNode("World"))
   {
      if(C TextNode *brush=world->findNode("Brush"))
      {
         if(C TextParam *p=brush->findNode("SizeProportional"))Brush.bsize.set(p->asBool());
         if(C TextParam *p=brush->findNode("Image"           ))Brush.setImage (p->value);
      }
      if(C TextNode *colors=world->findNode("Colors"))
      {
         FREPA(ColorBrush.col)if(C TextParam *p=colors->findNode("Color", i))ColorBrush.col[i].col=TextVec(p->value);
      }
      if(C TextNode *general=world->findNode("General"))
      {
       //if(C TextParam *p=general.findNode("CursorCollidesWithHeightmaps"))MainMenu("WE Edit/Cursor Collides with Heightmaps", p.asBool());
       //if(C TextParam *p=general.findNode("CursorCollidesWithObjects"   ))MainMenu("WE Edit/Cursor Collides with Objects"   , p.asBool());
         if(C TextParam *p=general->findNode("VisibleRadius"               ))WorldEdit.visibleRadius(p->asInt());
         if(C TextParam *p=general->findNode("ActiveViewport"              ))WorldEdit.setView      (p->asInt());
         if(C TextParam *p=general->findNode("CameraMoveMode"              ))WorldEdit.  moveMode   (Edit::Viewport4::  MOVE_MODE(p->asInt ()));
         if(C TextParam *p=general->findNode("CameraOrientation"           ))WorldEdit.orientMode   (Edit::Viewport4::ORIENT_MODE(p->asInt ()));
         if(C TextParam *p=general->findNode("CameraZoom"                  ))WorldEdit.      zoom   (                           p->asBool() );
         if(C TextParam *p=general->findNode("CameraSpherical"             ))WorldEdit.cam_spherical.set(p->asBool());
         if(C TextParam *p=general->findNode("CameraFppSpeed"              ))WorldEdit.fpp_speed    .set(p->asFlt ());
      }
   }
   if(C TextNode *theater=data.findNode("Theater"))
   {
      LoadProperties(Theater.options.props, ConstCast(theater->nodes));
      if(C TextNode *mode=theater->findNode("Mode"))Theater.mode.set(mode->asInt());
   }
}
/******************************************************************************/

/******************************************************************************/
