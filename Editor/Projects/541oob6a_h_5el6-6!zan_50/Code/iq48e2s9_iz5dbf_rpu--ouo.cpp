/******************************************************************************/
Mesh mesh;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
}
/******************************************************************************/
bool Init()
{
   Cam.dist=2;

   mesh.parts.New().base.create(Ball(1), VTX_TEX0|VTX_NRM|VTX_TAN);                        // create Mesh with 1 MeshPart, and create this MeshPart's base from Ball with automatic texture coordinates, normals and tangents
   mesh.material(UID(2123216029, 1141820639, 615850919, 3316401700)).setRender().setBox(); // set mesh material, rendering version and bounding box

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
   Cam.transformByMouse(1.5, 10, CAMH_ZOOM|CAMH_ROT);
   return true;
}
/******************************************************************************/
void Render() // Rendering Function
{
   switch(Renderer()) // Rendering Function will be called multiple times for different Rendering Modes
   {
      // the most important is the "prepare" mode in which you should draw all the meshes and add lights
      case RM_PREPARE:
      {
         // check if the mesh is inside the viewing frustum and add it to the drawing list
         if(Frustum(mesh))mesh.draw(MatrixIdentity);

         // add directional light
         LightDir(Vec(0, 0, 1)).add();
      }break;
   }
}
void Draw()
{
   // instead of typical "D.clear(WHITE);" we'll now use advanced rendering:
   Renderer(Render); // render using 'Render' function

   D.text(0, 0.9, S+"Fps: "+Time.fps()); // show number of fps
}
/******************************************************************************/
