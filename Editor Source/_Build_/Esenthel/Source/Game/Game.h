/******************************************************************************/
extern State     StateGame;
extern Str       GameWorld;
extern bool      DrawPhysics, DrawWorldPath;
extern VIEW_MODE ViewMode;
extern Memx<Game::ObjMap<Game::Static       > >      Statics;
extern Memx<Game::ObjMap<Game::ObjLightPoint> >  PointLights;
extern Memx<Game::ObjMap<Game::ObjLightCone > >   ConeLights;
extern Memx<Game::ObjMap<Game::ObjParticles > > ObjParticles;
extern Memx<Game::ObjMap<Player            > >      Players;
/******************************************************************************/
template<typename TYPE> Game::ObjMap<TYPE>& GetObjContainer(Memx<Game::ObjMap<TYPE> > &containers, int &counter);
void StartGame(Elm &elm);
void ExitGame(ptr=null);
void InitGameObjContainers();
bool InitGame();
void ShutGame();
void UpdateGameCam();
bool UpdateGame();
void RenderGame();
void DrawGame();
/******************************************************************************/
