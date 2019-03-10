/******************************************************************************/
MeshBase A, B, csg; // meshes

SHAPE_TYPE a_type   =SHAPE_BOX    , // A mesh type
           b_type   =SHAPE_CAPSULE; // B mesh type
int        operation=SEL_SUB      ; // csg operation

Material material_red  ,
         material_green;
/******************************************************************************/
Str OperationText()
{
   switch(operation)
   {
      case SEL_SUB : return S+ShapeTypeName(a_type)+'-'+ShapeTypeName(b_type);
      case SEL_SUBR: return S+ShapeTypeName(b_type)+'-'+ShapeTypeName(a_type);
      case SEL_AND : return S+ShapeTypeName(a_type)+'&'+ShapeTypeName(b_type);
      case SEL_ADD : return S+ShapeTypeName(a_type)+'+'+ShapeTypeName(b_type);
      default      : return S+ShapeTypeName(a_type)+'?'+ShapeTypeName(b_type);
   }
}
/******************************************************************************/
void Create(MeshBase &mshb, SHAPE_TYPE type)
{
   switch(type)
   {
      case SHAPE_BOX    : mshb.create(Box    (     0.8), VTX_NRM); break;
      case SHAPE_BALL   : mshb.create(Ball   (     1.0), VTX_NRM); break;
      case SHAPE_CAPSULE: mshb.create(Capsule(0.6, 3.0), VTX_NRM); break;
      case SHAPE_TUBE   : mshb.create(Tube   (0.6, 3.0), VTX_NRM); break;
   }
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0);
   Cam.dist =4;
   Cam.pitch=-0.6;
}
/******************************************************************************/
bool Init()
{
   Create(A, a_type);
   Create(B, b_type);
   material_red  .reset(); material_red  .color.v3().set(1.0, 0.5, 0.5); material_red  .validate();
   material_green.reset(); material_green.color.v3().set(0.5, 1.0, 0.5); material_green.validate();
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
   Cam.transformByMouse(0.01, 10, CAMH_ZOOM|(Ms.b(1) ? CAMH_MOVE : CAMH_ROT));

   // change operation type on 1, 2, 3, 4 keys
   if(Kb.b(KB_1))operation=SEL_SUB ; // subtract         (A-B)
   if(Kb.b(KB_2))operation=SEL_SUBR; // subtract reverse (B-A)
   if(Kb.b(KB_3))operation=SEL_AND ; // and              (A&B)
   if(Kb.b(KB_4))operation=SEL_ADD ; // add              (A+B)

   // change A mesh type on q,w,e,r keys
   if(Kb.b(KB_Q))a_type=SHAPE_BOX    ;
   if(Kb.b(KB_W))a_type=SHAPE_BALL   ;
   if(Kb.b(KB_E))a_type=SHAPE_CAPSULE;
   if(Kb.b(KB_R))a_type=SHAPE_TUBE   ;

   // change B mesh type on a,s,d,f keys
   if(Kb.b(KB_A))b_type=SHAPE_BOX    ;
   if(Kb.b(KB_S))b_type=SHAPE_BALL   ;
   if(Kb.b(KB_D))b_type=SHAPE_CAPSULE;
   if(Kb.b(KB_F))b_type=SHAPE_TUBE   ;

   // create A b meshes
   Create(A, a_type);
   Create(B, b_type);
   
   // move B mesh
   Flt t=Time.time();
   B.move(Vec(Cos(t), 0, Sin(t)));

   // perform Csg 'operation' on 'A B' meshes and store result into 'csg' mesh
   Csg(A, B, operation, &csg);

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   LightDir(Cam.matrix.z).set();
   
   SetMatrix();

   if(Ms.b(0)) // when mouse button pressed draw original meshes
   {
      A.drawAuto(&material_red  );
      B.drawAuto(&material_green);
   }
   else // if not then draw Csg output
   {
      csg.drawAuto(null);
   }

   Flt y=0.94, h=0.068;
   D.text(0, y, "Press 1, 2, 3, 4 to change operation"  ); y-=h;
   D.text(0, y, "Press q, w, e, r to change A mesh type"); y-=h;
   D.text(0, y, "Press a, s, d, f to change B mesh type"); y-=h;
   D.text(0, y, "Press LMB to view source meshes"); y-=h;
   D.text(0, y, S+"Operation: \""+OperationText()+'"');
}
/******************************************************************************/
