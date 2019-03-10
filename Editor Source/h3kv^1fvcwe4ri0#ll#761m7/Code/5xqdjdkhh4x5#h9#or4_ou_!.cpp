/******************************************************************************/
enum VIEW_MODE
{
   VIEW_FPP,
   VIEW_TPP,
   VIEW_ISO,
   VIEW_NUM,
}
/******************************************************************************/
State     StateGame(UpdateGame, DrawGame, InitGame, ShutGame);
Str       GameWorld;
bool      DrawPhysics, DrawWorldPath;
VIEW_MODE ViewMode=VIEW_TPP;

Memx<Game.ObjMap<Game.Static       >>      Statics;
Memx<Game.ObjMap<Game.ObjLightPoint>>  PointLights;
Memx<Game.ObjMap<Game.ObjLightCone >>   ConeLights;
Memx<Game.ObjMap<Game.ObjParticles >> ObjParticles;
Memx<Game.ObjMap<Player            >>      Players;

<TYPE> Game.ObjMap<TYPE>& GetObjContainer(Memx<Game.ObjMap<TYPE>> &containers, int &counter)
{
   Game.ObjMap<TYPE> &container=(InRange(counter, containers) ? containers[counter] : containers.New());
   counter++; return container;
}
/******************************************************************************/
void StartGame(Elm &elm)
{
   GameWorld=EncodeFileName(elm.id);
   StateGame.set(StateFadeTime);
}
void ExitGame(ptr=null)
{
   SetProjectState();
}
/******************************************************************************/
void InitGameObjContainers()
{
   // set object type enum
   Memt<Enum.Elm> obj_types; REPA(Proj.existing_obj_classes)if(Elm *elm=Proj.findElm(Proj.existing_obj_classes[i]))obj_types.New().set(NameToEnum(elm.name), elm.id);
   ObjType.create("OBJ_TYPE", obj_types);

   // set game object containers
   //Statics.del(); PointLights.del(); ConeLights.del(); ObjParticles.del(); Players.del(); don't delete those objects because Game.World obj containers may point to some of them, instead, just create new ones if needed
   int statics=0, point_lights=0, cone_lights=0, particles=0, players=0;
   FREPA(ObjType)
   {
      EditObjectPtr obj_class=Proj.editPath(ObjType[i].id);
      // must set object container casted directly target type and not Game.Obj because CType will not be detected properly
      if(obj_class->editor_type==EditObjType.elmID(EDIT_OBJ_LIGHT_POINT))Game.World.setObjType(GetObjContainer( PointLights, point_lights), i);else
      if(obj_class->editor_type==EditObjType.elmID(EDIT_OBJ_LIGHT_CONE ))Game.World.setObjType(GetObjContainer(  ConeLights,  cone_lights), i);else
      if(obj_class->editor_type==EditObjType.elmID(EDIT_OBJ_PARTICLES  ))Game.World.setObjType(GetObjContainer(ObjParticles,    particles), i);else
                                                                         Game.World.setObjType(GetObjContainer(     Statics,      statics), i);
   }
}
bool InitGame()
{
   SetKbExclusive();
   Proj.pause();
   VidOpt.hideAll();
   AppStore.hide();
   DataPath(Proj.game_path);
   InitGameButtons();
   InitGameObjContainers();
   Cam=ActiveCam;

   Game.World.activeRange(D.viewRange(300).viewRange()).mode(Game.WORLD_STREAM);
   if(!Game.World.NewTry(GameWorld)){Gui.msgBox(S, "Can't load world"); ExitGame();}
   if(Game.World.settings().environment)Game.World.settings().environment->set();else DefaultEnvironment.set();
   DrawPhysics=DrawWorldPath=false;
   return true;
}
void ShutGame()
{
   Game.World.del(); // manually delete the world to unload all the memory before switching back to the world editing
   Ms.show().clip(null, 0); // restore cursor visibility and disable any clipping
   ShutGameButtons();
   DataPath(S);
   Proj.resume();
}
/******************************************************************************/
void UpdateGameCam()
{
   Cam.at.y+=DPadY.dir*Time.d()*Cam.dist;
   if(DPad.touched)
   {
      Vec2 d; CosSin(d.x, d.y, DPad.angle+Cam.yaw);
      Cam.at.x+=d.x*Time.d()*Cam.dist;
      Cam.at.z+=d.y*Time.d()*Cam.dist;
   }
   Cam.yaw  -=Rot.delta.x;
   Cam.pitch+=Rot.delta.y;
   Cam.dist *=ScaleFactor(Zoom.zoom);
   Cam.transformByMouse(0.1, 1000, CAMH_ZOOM | (((Ms.b(0) || Ms.b(3)) && (!Gui.ms() || Gui.ms()==Gui.desktop())) ? CAMH_MOVE_XZ : (Ms.b(1) ? CAMH_MOVE : CAMH_ROT)));
}
bool UpdateGame()
{
   if(Kb.bp(KB_ESC) || Kb.bp(KB_NAV_BACK))ExitGame();
   Server.update(null, true);

   DrawPhysics  ^=(Kb.alt() && Kb.shift() && !Kb.ctrlCmd() && Kb.bp(KB_B));
   DrawWorldPath^=(Kb.alt() && Kb.shift() && !Kb.ctrlCmd() && Kb.bp(KB_P));

   if(Kb.bp(KB_LBR)){flt range=Mid(D.viewRange()*0.8, 32.0, Demo ? MaxGameViewRangeDemo : MaxGameViewRange); D.viewRange(range); Game.World.activeRange(range);}
   if(Kb.bp(KB_RBR)){flt range=Mid(D.viewRange()/0.8, 32.0, Demo ? MaxGameViewRangeDemo : MaxGameViewRange); D.viewRange(range); Game.World.activeRange(range);}

   UpdateGameCam();
   Game.World.update(ActiveCam.matrix.pos);
   Clouds.layered.update();
   Water.update(0.02);
   Ms.visible(Players.elms() && ViewMode==VIEW_ISO);
   Ms.clip   (null, Ms.hidden());
   if(Ms.hidden())Ms.freeze();
   Gui.update();
   return true;
}
/******************************************************************************/
void RenderGame()
{
   Game.World.draw();
}
void DrawGame()
{
   Renderer.wire=Kb.b(KB_TILDE);
   SHADOW_MODE shd_mode=D.shadowMode(); if(Demo)D.shadowMode(SHADOW_NONE);
   Renderer(RenderGame);
   D.shadowMode(shd_mode);
   Renderer.wire=false;

   if(DrawPhysics || DrawWorldPath)
   {
      Renderer.setDepthForDebugDrawing();
      if(DrawPhysics  )Physics.draw();
      if(DrawWorldPath)Game.World.path().draw(64, 0.5);
   }

   Gui.draw();
}
/******************************************************************************/
