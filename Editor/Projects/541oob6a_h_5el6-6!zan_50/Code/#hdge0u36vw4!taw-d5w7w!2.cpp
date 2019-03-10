/******************************************************************************/
Actor ground, // ground
      obj   ; // object
Grab  grab  ; // grabber
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
bool Init()
{
   Cam.setSpherical(Vec(0, 0, -2), 0, -0.7, 0, 6).set(); // set camera position and activate it

   Physics.create(EE_PHYSX_DLL_PATH);

   ground.create(Box(15, 1, 15, Vec(0, -2, 0)), 0); // create ground
   obj   .create(Box(0.3                     )   ); // create object

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   Physics.startSimulation().stopSimulation();

   if(Kb.ctrl()) // if want to grab
   {
      if(!grab.is()) // if not yet grabbing
      {
         grab.create(obj, Vec(0, 0.3, 0), 10); // start grabbing 'obj' actor at (0,0.3,0) local position with a power of 10
      }
      if(grab.is()) // if grabbing something
      {
         // drag the object to left/right/forward/backwards
         Vec dir(0);
         if(Kb.b(KB_LEFT ))dir.x--;
         if(Kb.b(KB_RIGHT))dir.x++;
         if(Kb.b(KB_DOWN ))dir.y--;
         if(Kb.b(KB_UP   ))dir.y++;
         grab.pos(grab.pos()+dir*Time.d()*2);
      }
   }else
   {
      if(grab.is())grab.del();
   }

   return true;
}
/******************************************************************************/
void Draw()
{
   D      .clear();
   Physics.draw ();

   if(!grab.is())D.text(0, 0.9, "Hold Control to grab the object");
   else          D.text(0, 0.9, "Use the arrow keys to drag around the object");
}
/******************************************************************************/
