/******************************************************************************/
class Obj
{
   Vec pos;
   int mesh_variation;

   void create()
   {
      pos=Random(Box(8, 4, 8));
      mesh_variation=Random(ball.variations());
   }
   void draw()C
   {
      SetVariation(mesh_variation); ball.draw(Matrix(pos));
      SetVariation();
   }
}

Material materials[8];
Mesh     ball;
Obj      obj[32];
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0);
}
/******************************************************************************/
bool Init()
{
   // set cel shading
   D.bumpMode  (BUMP_FLAT      ); // cel-shading works best with flat bump mapping
   D.edgeDetect(EDGE_DETECT_FAT); // set edge detection
   Renderer.cel_shade_palette=UID(870572307, 1257265829, 2574792609, 949141860); // set light palette which modifies global lighting

   // set sky
   Sky.atmospheric();

   // set mesh
   MeshPart &part=ball.parts.New();
   part.base.create(Ball(0.5), VTX_TEX0|VTX_NRM|VTX_TAN);
   part.variations(Elms(materials));
   ball.variations(Elms(materials)).setRender().setBox();

   // set materials
   FREPA(materials)
   {
      Material &mtrl=materials[i];
      mtrl.reset().color=ColorHue(flt(i)/Elms(materials)).asVec4();
      mtrl.validate();
      ball.parts[0].variation(i, &mtrl);
   }

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
   Cam.transformByMouse(0.01, 100, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

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

         LightDir(!Vec(1, -1, 1)).add();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
