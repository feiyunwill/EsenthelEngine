/******************************************************************************/
Mesh box,
     ball;
int  fog_type=0; // 0..3
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.1);
}
/******************************************************************************/
bool Init()
{
   Cam.at.set(0, -1, 0);
   Cam.pitch=-0.5;
   Cam.dist =7;

   // create standard meshes and materials
   MaterialPtr material=UID(2123216029, 1141820639, 615850919, 3316401700);

   box .parts.New().base.create( Box(5), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); box .material(material).setRender().setBox();
   ball.parts.New().base.create(Ball(1), VTX_TEX0|VTX_NRM|VTX_TAN)          ; ball.material(material).setRender().setBox();

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
   Cam.transformByMouse(0.1, 1000, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   if(Kb.bp(KB_0))fog_type=0;
   if(Kb.bp(KB_1))fog_type=1;
   if(Kb.bp(KB_2))fog_type=2;
   if(Kb.bp(KB_3))fog_type=3;

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         box .draw(MatrixIdentity);
         ball.draw(Matrix(Vec(0, -3, 0)));

         LightPoint(20, Vec(0, 3, 3)).add();
      }break;

      case RM_BLEND: // local fog needs to be drawn in RM_BLEND mode
      {
         switch(fog_type)
         {
            case 1:       FogDraw(OBox(Box(10, 4, 10, Vec(0, -3, 0))), 0.6, Vec(0.5, 0.5, 0.5)); break;
            case 2: HeightFogDraw(OBox(Box(10, 4, 10, Vec(0, -3, 0))), 0.6, Vec(0.5, 0.5, 0.5)); break;
            case 3:       FogDraw(Ball(5            , Vec(0, -5, 0) ), 0.6, Vec(0.5, 0.5, 0.5)); break;
         }
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, S+"Fps: "+Time.fps());
   D.text(0, 0.8, S+"Press 0,1,2,3 keys for different local fogs");
}
/******************************************************************************/
