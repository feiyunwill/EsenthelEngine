/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.sync(false); // disable screen synchronization to compare performance
}
/******************************************************************************/
bool Init()
{
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
   D.clear(TURQ);

   // to draw big number of shapes more efficiently please use VertexIndexBuffer (VI)

   CChar *mode;

   if(Kb.b(KB_SPACE)) // when space pressed, draw separately
   {
      REP(10000)D.line (GREEN, 0, 0, Cos(i*PI2/10000), Sin(i*PI2/10000)); // draw 10000 lines      separately
      REP(10000)Rect_C(0, 0, i/20000.0).draw(ColorHue(i/10000.0), false); // draw 10000 rectangles separately

      mode="Separate";
   }
   else // draw in buffered mode
   { 
      // draw lines
      {
                   VI.color(GREEN); // set green color
         REP(10000)VI.line (Vec2(0, 0), Vec2(Cos(i*PI2/10000), Sin(i*PI2/10000))); // draw 10000 lines in buffered mode
                   VI.end  ();      // finish drawing
      }
      //draw rectangles
      {
         REP(10000)VI.rectL(ColorHue(i/10000.0), Rect_C(0, 0, i/20000.0)); // draw 10000 lines in buffered mode
                   VI.end  (); // finish drawing
      }
      mode="Buffered";
   }

   D.text(0, 0.9, "Hold space to use different drawing mode");
   D.text(0, 0.8, S+mode+" drawing"   );
   D.text(0, 0.7, S+"Fps: "+Time.fps());
}
/******************************************************************************

   NOTE:
   Each type of drawing must be ended with its own "end" method

      // forbidden
      {
         REP(10)VI.line(..);
         REP(10)VI.rect(..);
                VI.end ();
      }

      // good
      {
         REP(10)VI.line(..);
                VI.end ();

         REP(10)VI.rect(..)
                VI.end ();
      }

   if one drawing method has color parameter and the other doesn't, then it means that they're different, and separate "end" is needed here as well

      // forbidden
      {
         REP(10)VI.line(RED, x0, y0, x1, y1);
         REP(10)VI.line(     x0, y0, x1, y1);
                VI.end ();
      }

      // good
      {
         REP(10)VI.line(RED, x0, y0, x1, y1);
                VI.end ();

                VI.color(RED);
         REP(10)VI.line (x0, y0, x1, y1);
                VI.end  ();
      }

/******************************************************************************/
