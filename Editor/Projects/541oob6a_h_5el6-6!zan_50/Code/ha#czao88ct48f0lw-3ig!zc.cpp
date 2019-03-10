/******************************************************************************/
int      lights=3   ; // number of lights (1..3)
Material material[3]; // light materials
Mesh     box        , // mesh box
         ball       , // mesh ball
         light      ; // mesh light
Vec      ball_pos[8], // ball  positions
        light_pos[3]; // light positions
/******************************************************************************/
void SetShader()
{
   box .setShader();
   ball.setShader();
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.set_shader=SetShader;
   D.ambientPower(0).bumpMode(BUMP_RELIEF).shadowSoft(1).shadowJitter(true);
}
/******************************************************************************/
bool Init()
{
   // create materials
   MaterialPtr brick=UID(2123216029, 1141820639, 615850919, 3316401700);
   REPA(material)
   {
      Material &m=material[i];
      m.reset();
      m.glow=0.5;
      m.ambient=1;
      switch(i)
      {
         case 0: m.color.xyz.set(1  , 1, 0.5); break;
         case 1: m.color.xyz.set(0.5, 1, 1  ); break;
         case 2: m.color.xyz.set(0.5, 1, 0.5); break;
      }
      m.validate();
   }

   // create meshes
   box  .parts.New().base.create( Box(1   ), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); // create mesh box, reverse it because it's meant to be viewed from inside
   ball .parts.New().base.create(Ball(0.15), VTX_TEX0|VTX_NRM|VTX_TAN); // create mesh ball

   MeshPart &light_part=light.parts.New();
   light_part.base.create(Ball(0.04));
   light_part.variations(Elms(material));
   REP(light_part.variations())light_part.variation(i, &material[i]);

   // set mesh materials, rendering versions and bounding boxes
   box .material(brick).setRender().setBox();
   ball.material(brick).setRender().setBox();
   light               .setRender().setBox();

   // set random positions
   REPAO(ball_pos)=Random(Box(0.9));

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
   Cam.transformByMouse(0.01, 10, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   // update light positions when space is not pressed
   if(!Kb.b(KB_SPACE))
   {
      static Flt t; t+=Time.d()/2;
      light_pos[0].set(0, 0, 0.3); light_pos[0]*=Matrix3().setRotateX(t).rotateY(t/2  ).rotateZ(t/3  );
      light_pos[1].set(0, 0, 0.6); light_pos[1]*=Matrix3().setRotateX(t).rotateY(t/1.5).rotateZ(t/2.5);
      light_pos[2].set(0, 0, 0.5); light_pos[2]*=Matrix3().setRotateX(t).rotateY(t/1.3).rotateZ(t/3.2);
   }

   // change settings
   if(Kb.c('1'))lights=1;
   if(Kb.c('2'))lights=2;
   if(Kb.c('3'))lights=3;

   if(Kb.c('q'))D.bumpMode(BUMP_FLAT    );
   if(Kb.c('w'))D.bumpMode(BUMP_NORMAL  );
   if(Kb.c('e'))D.bumpMode(BUMP_PARALLAX);
   if(Kb.c('r'))D.bumpMode(BUMP_RELIEF  );

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         // solid objects
                       box .draw(MatrixIdentity);
         REPA(ball_pos)ball.draw(Matrix(ball_pos[i]));

         // lights
         switch(lights)
         {
            case 1:
               LightPoint(0.50, light_pos[0], Vec(1)).add();
            break;

            case 2:
               LightPoint(0.45, light_pos[0], Vec(1, 0.5, 0)).add();
               LightPoint(0.45, light_pos[1], Vec(0, 0.5, 1)).add();
            break;

            case 3:
               LightPoint(0.40, light_pos[0], Vec(1, 0.3, 0)).add();
               LightPoint(0.40, light_pos[1], Vec(0, 0.3, 1)).add();
               LightPoint(0.40, light_pos[2], Vec(0, 0.4, 0)).add();
            break;
         }

         // light meshes
                                        light.draw(Matrix(light_pos[0]));
         if(lights>=2){SetVariation(1); light.draw(Matrix(light_pos[1])); SetVariation();}
         if(lights>=3){SetVariation(2); light.draw(Matrix(light_pos[2])); SetVariation();}
      }break;

      case RM_SHADOW:
      {
         REPA(ball_pos)ball.drawShadow(Matrix(ball_pos[i]));
      }break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
