/******************************************************************************

   This tutorial will present how to create a new world, and set some terrain in it.

/******************************************************************************/
Edit.EditorInterface EI;
bool ok;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // connect to a running instance of the Editor
   Str message; if(!EI.connect(message))Exit(message);

   // create new world element in the project
   UID world_id=EI.newWorld("Programatic World");
   if(!world_id.valid())Exit("Creating world failed");

   int terrain_res=EI.worldTerrainRes(world_id); if(terrain_res<=0)Exit("Invalid terrain resolution");

   // set heightmap
   Image height;
   height.createSoft(terrain_res, terrain_res, 1, IMAGE_F32);
   REPD(y, height.h())
   REPD(x, height.w())height.pixelF(x, y, Random(0, 10)); // random value from 0 to 10 meters
   height.blur(4, true); // blur the image

   // setting terrain requires specifying a material, so let's look for one in the project
   UID material_id=UIDZero;
   Memc<Edit.Elm> elms; EI.getElms(elms); REPA(elms)if(elms[i].type==Edit.ELM_MTRL && elms[i].name=="grass"){material_id=elms[i].id; break;}
   if(!material_id.valid())Exit("Not found any material to use");

   // set terrain
   ok=EI.worldTerrainSetHeight(world_id, VecI2(0, 0), height, material_id);

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
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(BLACK);
   D.text(Rect(Vec2(0)), ok ? "A new world has been created in the root of the project\nPlease open it to see it" : "Setting terrain failed");
}
/******************************************************************************/
