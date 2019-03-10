/******************************************************************************/
class Obj
{
   Vec                     pos;
   Memc<ShaderParamChange> changes;

   void create()
   {
      pos=Random(Box(8, 4, 8)); // set random position

      Vec4 color(Random.vec(), 1); // create a random color

      changes.New().set(Renderer.material_color) // create a new change and initialize it for changing the 'Material.color' shader parameter
                   .set(color); // set its value
   }
   void draw()C
   {
        LinkShaderParamChanges(changes); ball.draw(Matrix(pos));
      UnlinkShaderParamChanges(changes);
   }
}

Material material;
Mesh     ball;
Obj      obj[32];
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
   // set sky
   Sky.atmospheric();

   // set material
   material.reset().validate();

   // set mesh
   MeshPart &part=ball.parts.New();
   part.base.create(Ball(0.5), VTX_TEX0|VTX_NRM|VTX_TAN);
   ball.material(&material).setRender().setBox();

   // setup objects
   REPAO(obj).create();

   Cam.dist=10;

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
   Cam.transformByMouse(0.01, 100, CAMH_ZOOM | (Ms.b(1) ? CAMH_MOVE : CAMH_ROT));
   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         REPAO(obj).draw();

         LightDir(!Vec(1, -1, 1), 1-D.ambientColor()).add();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
