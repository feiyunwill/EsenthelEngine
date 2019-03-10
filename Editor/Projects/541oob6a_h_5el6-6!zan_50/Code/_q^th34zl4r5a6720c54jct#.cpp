/******************************************************************************

   This tutorial will present how to place objects in a world.

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

   // find an object for placing
   UID obj_id=UIDZero;
   Memc<Edit.Elm> elms; EI.getElms(elms); REPA(elms)if(elms[i].type==Edit.ELM_OBJ && elms[i].name=="barrel"){obj_id=elms[i].id; break;}
   if(!obj_id.valid())Exit("Not found any object to use");

   // place objects
   Memc<Edit.WorldObjParams> objs;
   REP(32) // 32 objects
   {
      Matrix matrix; matrix.setScalePos(2, Vec(RandomF(0, 64), 0, RandomF(0, 64)));
      objs.New().set(obj_id, matrix, false, 0);
   }
   ok=EI.worldObjCreate(world_id, objs);

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
   D.text(Rect(Vec2(0)), ok ? "A new world has been created in the root of the project\nPlease open it to see it" : "Placing objects failed");
}
/******************************************************************************/
