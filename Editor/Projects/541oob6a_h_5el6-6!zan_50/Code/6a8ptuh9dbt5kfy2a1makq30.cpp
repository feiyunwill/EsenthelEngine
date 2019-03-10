/******************************************************************************

   Matrix is a set of values which can be used to determine object's:
      -position
      -orientation (rotation)
      -scale

   Matrix is built from 4 vectors:
      -position
      -x direction (right  )
      -y direction (up     )
      -z direction (forward)

   The default Matrix is called MatrixIdentity, it has its vectors set as following:
      position(0, 0, 0) - located at world center, zero position
      x       (1, 0, 0) - points right
      y       (0, 1, 0) - points up
      z       (0, 0, 1) - points forward

/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
}
/******************************************************************************/
Bool Init()
{
   Cam.dist=5;
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.1, 10, CAMH_ROT|CAMH_ZOOM);
   return true;
}
/******************************************************************************/
void Draw(C Matrix &matrix) // this function handles optional 'matrix' drawing
{
   if(Kb.b(KB_SPACE)) // draw matrix when space is pressed
   {
      SetMatrix(); // reset drawing matrix

      matrix.pos.draw(WHITE, 0.02);                   // draw matrix position with white color
      D.line(RED  , matrix.pos, matrix.pos+matrix.x); // draw matrix 'x' (right  ) vector with red   color
      D.line(GREEN, matrix.pos, matrix.pos+matrix.y); // draw matrix 'y' (up     ) vector with green color
      D.line(BLUE , matrix.pos, matrix.pos+matrix.z); // draw matrix 'z' (forward) vector with blue  color
   }
}
void Draw()
{
   D.clear(TURQ);

   // box
   {
      SetMatrix(MatrixIdentity); // set default matrix identity
      Box(1).draw(BLACK);        // draw box at current matrix

      Draw(MatrixIdentity); // draw matrix
   }

   // ball
   {
      Matrix m;                             // matrix (matrix is a position + orientation)
      m.setPos(Vec(-3, 0, 0));              // init matrix with initial position
      SetMatrix(m);                         // set as active matrix
      Ball(1).draw(BLACK, false, VecI2(8)); // draw ball at current matrix

      Draw(m); // draw matrix
   }

   // tube
   {
      Matrix m;
      m.setRotateX(Time.time());          // init matrix with initial x-rotation according to angle from time
      m.move(Vec(3, 0, 0));               // move matrix to the right
      SetMatrix(m);                       // set as active matrix
      Tube(0.2, 2).draw(BLACK, false, 6); // draw tube

      Draw(m); // draw matrix
   }
   
   D.text(0, 0.9, "Press Space to draw Matrixes");
}
/******************************************************************************

   NOTE : if matrix method         starts with 'set' it means it resets matrix and then applies transformation
          if matrix method doesn't start  with 'set' it means it                   just applies transformation

   instead of writing:
      Matrix m;
      m.setRotateX(Time.time());
      m.move(Vec(3,0,0));
      SetMatrix(m);

   you can write:
      SetMatrix(Matrix().setRotateX(Time.time()).move(Vec(3,0,0)));

/******************************************************************************/
