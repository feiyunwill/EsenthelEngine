/******************************************************************************/
class MiniMapEditor : PropWin
{
   class Change : Edit._Undo.Change
   {
      ElmMiniMap data;

      virtual void create(ptr user)override
      {
         if(ElmMiniMap *data=MiniMapEdit.data())
         {
            T.data=*data;
            MiniMapEdit.undoVis();
         }
      }
      virtual void apply(ptr user)override
      {
         if(ElmMiniMap *data=MiniMapEdit.data())
         {
            UID world=data.world_id; int areas=data.areas_per_image;
            data.undo(T.data);
            MiniMapEdit.setChanged(data.world_id!=world, data.areas_per_image!=areas);
            MiniMapEdit.toGui();
            MiniMapEdit.undoVis();
         }
      }
   }

   const int         pixel_border=32; // additional number of pixels to draw while creating mini maps (helps in making bloom/glow objects more smooth at the edges of mini map images)
   flt               prop_max_x=0;
   UID               elm_id=UIDZero;
   Elm              *elm=null;
   bool              changed=false;
   int               progress=0;
   VecI2             image_pos=0;
   RectI             images=0;
   ViewportSkin      viewport;
   EnvironmentPtr    env;
   Button            undo, redo, locate, make;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   static void Render() {MiniMapEdit.render();}
          void render() {Game.World.draw();}

   int areasPerImage() {ElmMiniMap *data=T.data(); return data ? data.areas_per_image : 4;}
   UID       worldID() {ElmMiniMap *data=T.data(); return data ? data.world_id : UIDZero;}
   flt  miniMapWidth() {return Game.World.areaSize()*areasPerImage();} // required width in meters for loaded areas
   int   totalImages() {return (images.w()+1)*(images.h()+1);}

   static flt ShadowStep(int i, int num)
   {
      flt f=flt(i)/num; // uniform fraction
      return Lerp(f, 0.5, 0.97); // because we'll set the camera so that most objects/heightmaps will be in the center of the viewport range, then move shadow split focus into the center (0.5)
   }

   static void Draw(Viewport &viewport) {MiniMapEdit.draw();}
          void draw(bool final=false)
   {
      if(ElmMiniMap *data=T.data())
      {
         // environment
         UID env_id=data.env_id; if(!env_id.valid())if(Elm *world=Proj.findElm(data.world_id))if(ElmWorld *world_data=world.worldData())env_id=world_data.env_id;
         if(!env_id.valid())CurrentEnvironment().set();else{env=Proj.gamePath(env_id); env->set();}

         // graphics settings
         MOTION_MODE      motion     =D.   motionMode(); D. motionMode  (MOTION_NONE     );
         AMBIENT_MODE     ambient    =D.  ambientMode(); D.ambientMode  (AMBIENT_ULTRA   );
         int              ambient_res=D.   ambientRes(); D.ambientRes   (0               );
         bool             bloom_half =D.    bloomHalf(); D.bloomHalf    (true            );
         EDGE_SOFTEN_MODE edge       =D.   edgeSoften(); D.edgeSoften   (EDGE_SOFTEN_SMAA);
         DOF_MODE         dof        =D.      dofMode(); D.    dofMode  (DOF_NONE        );
         flt              lod_fac    =D.    lodFactor(); D.  lodFactor  (0               );
         byte             shd_soft   =D.   shadowSoft(); D.shadowSoft   (1);
         byte             shd_num    =D. shadowMapNum(); D.shadowMapNum (6);
         bool             shd_jitter =D. shadowJitter(); D.shadowJitter (false);
         bool             eye_adapt  =D.eyeAdaptation(); D.eyeAdaptation(false);
         bool             ocean      =Water.draw       ; Water.draw     =false;
         flt              view_from  =D.viewFrom(), view_range=D.viewRange(), view_fov=D.viewFov();
         flt            (*shd_step)(int i, int num)=D.shadow_step; D.shadow_step=ShadowStep;

         RectI viewport; if(final)viewport.set(0, 0, D.resW(), D.resH());else viewport=D.screenToPixelI(D.viewRect());
         int border    =pixel_border,
             res       =data.image_size,         // image resolution
             res_border=res+border*2;            // resolution with borders applied
         MIN(res_border, viewport.size().min()); // do not exceed available viewport resolution
             res       =res_border-border*2;     // recalculate resolution without borders
    if(final)res       =FloorPow2(res);          // adjust it to be a power of 2
             res_border=res+border*2;            // recalculate resolution with borders

         bool downsample=false;
         if(viewport.w()>=res_border*2
         && viewport.h()>=res_border*2 && final){res*=2; res_border*=2; border*=2; downsample=true;} // if viewport resolution is twice as big as rendering resolution, then render everything in 2x bigger size and later downsample for better quality

         if(final)D.viewForceSquarePixel(true);
         D.viewFov (DegToRad(0.4), FOV_XY); // set low 'fov' to simulate orthogonal rendering, use 'FOV_XY' to set equal horizontal and vertical fov values to generate uniform scale textures
         D.viewRect(final ? &RectI(0, 0, res_border, res_border) : &RectI(viewport.centerI()).extend(res_border/2)); // set viewport to cover image map resolution with borders

         // set camera
         Cam.at   = ((Vec2(image_pos)+0.5)*miniMapWidth()).x0y(); // set camera at center of current map image
         Cam.yaw  = 0;
         Cam.pitch=-PI_2;
         Cam.roll = 0;
         Cam.dist = miniMapWidth()*0.5/Tan(D.viewFovX()/2); // set distance to cover the selected areas
         Cam.dist*=flt(res_border)/res; // adjust for borders
         Cam.setSpherical().set();

         // update viewport
         D.viewFrom (ActiveCam.dist*0.01)  // set high 'from' to reduce depth buffer precision issues
          .viewRange(ActiveCam.dist*2   ); // set viewport range to cover the range from sky to ground and ground to underground

         Renderer(MiniMapEditor.Render);

         if(!final)D.pixelToScreen(RectI(viewport.centerI()).extend(res/2)).draw(RED, false);

         D.  lodFactor  (lod_fac);
         D.    dofMode  (dof    );
         D. motionMode  (motion );
         D.ambientMode  (ambient).ambientRes(ambient_res);
         D.bloomHalf    (bloom_half);
         D.shadowSoft   (shd_soft).shadowMapNum(shd_num).shadowJitter(shd_jitter).shadow_step=shd_step;
         D.edgeSoften   (edge   );
         D.eyeAdaptation(eye_adapt);
         D.viewForceSquarePixel(false).viewFrom(view_from).viewRange(view_range).viewFov(view_fov);
         Water.draw      =ocean;

         if(final)
         {
            Image image; if(!Renderer.capture(image, -1, -1, IMAGE_R8G8B8, IMAGE_SOFT)){Gui.msgBox(S, "Error capturing screen data"); SetProjectState();}else
            {
               image.crop(image, border, border, res, res);
               if(downsample)image.downSample();
               image.copyTry(image, -1, -1, -1, IMAGE_BC1, IMAGE_2D, 1);
               image.save(Proj.gamePath(MiniMapEdit.elm_id).tailSlash(true)+image_pos);
               if(MiniMapVer *ver=T.ver())
               {
                  ver.images.binaryInclude(image_pos, Compare);
                  ver.changed=true;
               }
            }
         }
      }
   }

   static void Make(MiniMapEditor &mme) {mme.makeDo();}

   static cchar8 *ImageSizes[]=
   {
        "32",
        "64",
       "128",
       "256",
       "512",
      "1024",
   };
   static cchar8 *AreasPerImage[]=
   {
       "1",
       "2",
       "3",
       "4",
       "5",
       "6",
       "7",
       "8",
      "12",
      "16",
      "24",
   };

   static void PreChanged(C Property &prop) {MiniMapEdit.undos.set(&prop);}
   static void    Changed(C Property &prop) {MiniMapEdit.setChanged();}

   static void World   (  MiniMapEditor &mme, C Str &text) {if(ElmMiniMap *data=mme.data()){data.world_id=Proj.findElmID(text, ELM_WORLD); data.world_time.getUTC(); mme.reloadWorld();}}
   static Str  World   (C MiniMapEditor &mme             ) {if(ElmMiniMap *data=mme.data())return Proj.elmFullName(data.world_id); return S;}
   static void Env     (  MiniMapEditor &mme, C Str &text) {if(ElmMiniMap *data=mme.data()){data.env_id=Proj.findElmID(text, ELM_ENV); data.env_time.getUTC();}}
   static Str  Env     (C MiniMapEditor &mme             ) {if(ElmMiniMap *data=mme.data())return Proj.elmFullName(data.env_id); return S;}
   static void ArPerImg(  MiniMapEditor &mme, C Str &text) {if(ElmMiniMap *data=mme.data()){int i=TextInt(text); data.areas_per_image=(InRange(i, AreasPerImage) ? TextInt(AreasPerImage[i]) : 4); data.areas_per_image_time.getUTC(); mme.reloadAreas();}}
   static Str  ArPerImg(C MiniMapEditor &mme             ) {int nearest=-1, dist; if(ElmMiniMap *data=mme.data())REPA(AreasPerImage){int d=Abs(data.areas_per_image-TextInt(AreasPerImage[i])); if(nearest<0 || d<dist){nearest=i; dist=d;}} return nearest;}
   static void Size    (  MiniMapEditor &mme, C Str &text) {if(ElmMiniMap *data=mme.data()){int i=TextInt(text); data.image_size=(InRange(i, ImageSizes) ? TextInt(ImageSizes[i]) : 256); data.image_size_time.getUTC();}}
   static Str  Size    (C MiniMapEditor &mme             ) {int nearest=-1, dist; if(ElmMiniMap *data=mme.data())REPA(ImageSizes){int d=Abs(data.image_size-TextInt(ImageSizes[i])); if(nearest<0 || d<dist){nearest=i; dist=d;}} return nearest;}

   static void Undo  (MiniMapEditor &editor) {editor.undos.undo();}
   static void Redo  (MiniMapEditor &editor) {editor.undos.redo();}
   static void Locate(MiniMapEditor &editor) {Proj.elmLocate(editor.elm_id);}

   ElmMiniMap* data()C {return elm ?  elm.miniMapData      (      ) : null;}
   MiniMapVer*  ver()C {return elm ? Proj.miniMapVerRequire(elm.id) : null;}

   void makeDo()
   {
      if(elm)if(ElmMiniMap *data=elm.miniMapData())StateMiniMap.set(StateFadeTime);
   }
   void init(bool center=false)
   {
      ElmMiniMap *data       =T.data();
      RectI       world_areas=Proj.getWorldAreas(worldID()); if(center)world_areas=world_areas.centerI();
      int         api        =areasPerImage();
      progress=0;
      images.set(DivFloor(world_areas.min.x, api),
                 DivFloor(world_areas.min.y, api),
                 DivFloor(world_areas.max.x, api),
                 DivFloor(world_areas.max.y, api));
      image_pos=images.min;
      image_pos.x--; // go back 1 step
   }
   bool step()
   {
      if(image_pos.y>images.max.y)return false; // if out of Y range then it means that we've processed all mini maps
      // update mini map positions
         progress++;
         image_pos.x++; // move 1 step forward
      if(image_pos.x>images.max.x) // if out of X range, then reset X position and proceed to the next Y position
      {
            image_pos.x=images.min.x; // reset X
            image_pos.y++;            // proceed to next Y
         if(image_pos.y>images.max.y)return false; // if out of Y range then it means that we've processed all mini maps
      }
      return true;
   }
   void create()
   {
      add("World"          , MemberDesc(DATA_STR).setFunc(World   , World   )).elmType(ELM_WORLD);
      add("Environment"    , MemberDesc(DATA_STR).setFunc(Env     , Env     )).elmType(ELM_ENV  );
      add("Areas Per Image", MemberDesc(        ).setFunc(ArPerImg, ArPerImg)).setEnum(AreasPerImage, Elms(AreasPerImage));
      add("Image Size"     , MemberDesc(        ).setFunc(Size    , Size    )).setEnum(ImageSizes   , Elms(ImageSizes   ));
      autoData(this);

      flt h=0.043;
      Rect r=super.create("Mini Map Editor", Vec2(0.02, -0.07), 0.036, h, PropElmNameWidth); super.changed(Changed, PreChanged);
      T+=undo  .create(Rect_LU(0.02, -0.01     , 0.05, 0.05)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05, 0.05)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01, 0), 0.14, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      prop_max_x=r.max.x; button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=make.create(Rect_U(r.down()-Vec2(0, 0.04), 0.3, 0.055), "Create").func(Make, T).desc("Create mini map using current settings");
      T+=viewport.create(Draw);
      rect(Rect_C(0, 0, Min(1.7, D.w()*2), Min(1.12, D.h()*2)));
   }
   virtual Rect sizeLimit()C override {Rect r=super.sizeLimit(); r.min.set(0.9, 0.5); return r;}
   virtual MiniMapEditor& rect(C Rect &rect)override
   {
      super.rect(rect);
      viewport.rect(Rect(prop_max_x, -clientHeight(), clientWidth(), 0).extend(-0.02));
      return T;
   }

   void flush()
   {
      if(elm && changed)
      {
         if(ElmMiniMap *data=elm.miniMapData())data.newVer(); // modify just before saving/sending in case we've received data from server after edit
         Server.setElmShort(elm.id);
      }
      changed=false;
   }
   void setAreas(int border=4)
   {
      Memc<Game.WorldManager.AreaState> areas;
      RectI recti=image_pos; recti.max++; recti*=areasPerImage(); recti.max--;
      recti.extend(border); // for nearby shadow casting objects
      for(int y=recti.min.y; y<=recti.max.y; y++)
      for(int x=recti.min.x; x<=recti.max.x; x++)areas.New().set(VecI2(x, y), Game.AREA_ACTIVE);
      DataPath(Proj.game_path);
      Game.World.areaSetState(areas, true);
      DataPath(S);
   }
   void reloadAreas()
   {
      if(ElmMiniMap *data=T.data())
      {
         if(data.world_id==WorldEdit.elm_id)WorldEdit.flush();
         init(true); step();
         setAreas(1); // use low border because this is just a preview
         Time.skipUpdate();
      }
   }
   void reloadWorld()
   {
      if(elm)
      {
         if(ElmMiniMap *data=T.data())
         {
            InitGameObjContainers();
            Game.World.use_background_loading=false; // this must be disabled because we may write to world on the main thread
            DataPath(Proj.game_path);
            Game.World.mode(Game.WORLD_MANUAL).NewTry(Proj.gamePath(data.world_id));
            DataPath(S);
            reloadAreas();
         }
      }else
      {
         Game.World.del();
         Game.World.use_background_loading=true;
      }
   }
   void setChanged(bool world=false, bool areas=false)
   {
      if(elm)
      {
         changed=true;
         if(ElmMiniMap *data=elm.miniMapData())data.newVer();
         if(world)reloadWorld();else if(areas)reloadAreas();
         toGui();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_MINI_MAP)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
         reloadWorld();
      }
   }
           void           activate(Elm *elm)         {set(elm); if(T.elm)super.activate();}
           void           toggle  (Elm *elm)         {if(elm==T.elm)elm=null; set(elm);}
   virtual MiniMapEditor& hide    (        )override {if(visible()){super.hide(); set(null);} return T;}

   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
         reloadWorld();
         toGui();
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
}
MiniMapEditor MiniMapEdit;
bool          MiniMapBuilding, MiniMapOk;
int           MiniMapAreasLeft;
State         StateMiniMap(UpdateMiniMap, DrawMiniMap, InitMiniMap, ShutMiniMap);
/******************************************************************************/
bool InitMiniMap()
{
   SetKbExclusive();
 //Proj.pause(); we're not pausing so the 'MiniMapEdit' doesn't close and unload mini map element/data/game world
   Proj.pauseServer(); // instead just pause the server so no changes are made during mini map generation
   MiniMapEdit.init();
   UpdateProgress.create(Rect_C(0, -0.05, 1, 0.045));
   MiniMapBuilding=true;
   MiniMapOk=false;
   MiniMapAreasLeft=Proj.worldAreasToRebuild(&MiniMapEdit.worldID());
   return true;
}
void ShutMiniMap()
{
   UpdateProgress.del();
 //Proj.resume(); we're not resuming
   Proj.resumeServer(); // resume server
   WindowSetNormal();
   WindowFlash();
   MiniMapEdit.reloadAreas();
   /*if(MiniMapOk)if(MiniMapVer *ver=MiniMapEdit.ver()) // send new mini map to the server only after full completion (this is not needed since we're resuming the server above)
   {
      Server.setMiniMapSettings(MiniMapEdit.elm_id, ver.settings, ver.time);
      REPA(ver.images)Synchronizer.setMiniMapImage(MiniMapEdit.elm_id, ver.images[i]);
   }*/
}
/******************************************************************************/
bool UpdateMiniMap()
{
   if(Kb.bp(KB_ESC)){SetProjectState(); Gui.msgBox(S, "Mini Map generation breaked on user request");}

   Builder.update(!MiniMapBuilding, &MiniMapEdit.worldID());
   if(MiniMapBuilding)
   {
      UpdateProgress.set(MiniMapAreasLeft-Proj.worldAreasToRebuild(&MiniMapEdit.worldID()), MiniMapAreasLeft);
      if(Builder.finished(&MiniMapEdit.worldID()))
      {
         MiniMapBuilding=false;
         WorldEdit.flush(); // flush any world areas that were builded
         if(ElmMiniMap *data=MiniMapEdit.data()) // start creating mini maps
         {
            FDelInside(Proj.gamePath(MiniMapEdit.elm_id));
             Proj.createMiniMapPaths(MiniMapEdit.elm_id);

            Preview.elmChanged(MiniMapEdit.elm_id);
            if(MiniMapVer *ver=MiniMapEdit.ver())
            {
               data.copyTo(ver.settings);
               ver.changed =true;
               ver.time    .getUTC();
               ver.settings.area_size=Game.World.areaSize();
               ver.settings.save(Proj.gamePath(MiniMapEdit.elm_id).tailSlash(true)+"Settings");
               ver.images  .clear();
            }
         }
         Memc<Game.WorldManager.AreaState> areas; Game.World.areaSetState(areas, true); // unload all areas in case they were changed during building
      }
      Time.wait(1000/30);
   }

   WindowSetProgress(UpdateProgress());
     //Gui.update(); do not update gui as it's not hidden
    Server.update(null, true);
   if(Ms.bp(3))WindowToggle();
   return true;
}
/******************************************************************************/
void DrawMiniMap()
{
   D.clear(BackgroundColor());
   if(MiniMapBuilding)
   {
      D.text(0, 0.05, S+"Waiting for "+Proj.worldAreasToRebuild(&MiniMapEdit.worldID())+" world areas to finish building");
   }else
   {
      for(uint start=Time.curTimeMs(); ; )
      {
         UpdateProgress.set(MiniMapEdit.progress, MiniMapEdit.totalImages());
         WindowSetProgress(UpdateProgress());

         if(MiniMapEdit.step())
         {
            MiniMapEdit.setAreas();
            MiniMapEdit.draw(true);
            if((Time.curTimeMs()-start)>=1000/30)break;
         }else
         {
            MiniMapOk=true;
            SetProjectState();
            break;
         }
      }
      D.viewRect(null);

      D.text(0, 0.05, "Creating Mini Map Images");
   }
   GuiPC gpc;
   gpc.visible=gpc.enabled=true; 
   gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
   gpc.offset.zero();
   UpdateProgress.draw(gpc);
   D.clip();
 //Gui.draw(); do not draw gui as it's not hidden
}
/******************************************************************************/
