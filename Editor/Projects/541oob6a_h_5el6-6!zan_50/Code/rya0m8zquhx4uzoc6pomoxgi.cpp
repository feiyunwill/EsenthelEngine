/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.viewRange(50);

   Cam.at.set(16, 0, 16);
   Cam.dist = 20;
   Cam.pitch=-PI_3;
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   Game.World.activeRange(D.viewRange())
             .New(UID(3692432343, 1256681024, 2863582642, 4182715660));
   if(Game.World.settings().environment)Game.World.settings().environment->set();

   return true;
}
/******************************************************************************/
void Shut()
{
   Game.World.del();
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.1, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));
   Game.World.update(Cam.at);
   return true;
}
/******************************************************************************/
void Render()
{
   Game.World.draw();
}
void DrawWaypoint(C UID &id)
{
   if(Game.Waypoint *waypoint=Game.World.findWaypoint(id)) // if waypoint exists
   {
              waypoint.draw(             ); // draw waypoint
      Vec pos=waypoint.pos (Time.time()*3); // access waypoints position at 'Time.time()*3' position
          pos.draw(RED);                    // draw the position as red dot
   }
}
void Draw()
{
   Renderer(Render);

   // draw waypoints
   SetMatrix(); // first reset matrix
   DrawWaypoint(UID(2111469548, 1145422279, 2517019780, 1385428029)); // draw each waypoint
   DrawWaypoint(UID(3077532548, 1076832701, 1084491962, 386371395)); // draw each waypoint
   DrawWaypoint(UID(4136102066, 1311505035, 3425239484, 2087693256)); // draw each waypoint

   D.text(0, 0.9, "Notice that different Waypoints have different looping modes");
   D.text(0, 0.8, "Waypoints with their looping modes are set in World Editor");
}
/******************************************************************************/
