/******************************************************************************/
class Obj
{
   Vec pos=0;

   void create     (         ) {pos=Random(box.box);}
   void draw       (         ) {ball.draw(Matrix(pos));}
   void drawOutline(flt alpha) {Color color=ColorAlpha(alpha); ball.drawOutline(color, pos);}
}

MaterialPtr brick;
Mesh        box  ,
            ball ;
Obj         obj[32];
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.ambientPower(0);
}
/******************************************************************************/
bool Init()
{
   brick=UID(2123216029, 1141820639, 615850919, 3316401700);

   box .parts.New().base.create( Box(4  ), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); // create mesh box, reverse it because it's meant to be viewed from inside
   ball.parts.New().base.create(Ball(0.5), VTX_TEX0|VTX_NRM|VTX_TAN)          ; // create mesh ball

   // set mesh materials, rendering versions and bounding boxes
   box .material(brick).setRender().setBox();
   ball.material(brick).setRender().setBox();

   // set objects
   REPAO(obj).create();

   // set camera
   Cam.dist=5;

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
         box.draw(MatrixIdentity);

         REPAO(obj).draw();

         LightPoint(25, Vec(0, 3, 0)).add();
      }break;

      // when we wan't to use Mesh Outlining we need to process additional rendering mode 'RM_OUTLINE'
      case RM_OUTLINE:
      {
         if(Kb.b(KB_SPACE)) // outline only when space pressed
         {
            Flt alpha=Frac(Time.time()*2, 2);
            if( alpha>1)alpha=2-alpha;
            REPAO(obj).drawOutline(alpha);
         }
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, "Press Space to outline the balls");
}
/******************************************************************************/
