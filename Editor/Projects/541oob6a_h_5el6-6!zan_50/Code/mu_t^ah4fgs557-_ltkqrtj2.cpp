/******************************************************************************

   Here are presented the most basic drawing functions

/******************************************************************************/
void InitPre()
{
   EE_INIT();
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
   D.clear(WHITE); // clear screen to white

   // draw texts
   {   
      D.text(0, 0, "Default text settings"); // default text draw

      TextStyleParams ts; // text style, contains information about font size, aligning, color, ..

      ts.color=BLACK; // set black color
      ts.size*=1.4; // change scale
      D.text(ts, 0, -0.2, "Bigger");
      
      ts.size/=1.4;     // reset scale
      ts.align.set(1,0); // change aligning
      D.text(ts, 0, -0.4, "Right Aligned");

      ts.color=GREEN;     // change color to green
      ts.align.set(-1,0); // change aligning
      D.text(ts, 0, -0.6, "Colored Left Aligned");

      ts.align.set(0,0);                // reset aligning
      ts.color=ColorHue(Time.time()/3); // assign color to Color Hue according to current time
      D.text(ts, 0, -0.8, "Multi colored");
   }

   // draw shapes
   {
      Rect  ( 0.5, 0.5, 0.6, 0.6  ).draw(BLUE ); // draw blue rectangle at given coordinates
      Circle( 0.1, Vec2(-0.5, 0.5)).draw(RED  ); // draw red circle with 0.1 radius at (-0.5, 0.5) position
      Vec2  ( 0.0, 0.5            ).draw(BLACK); // draw black dot at (0, 0.5) position
      Edge2 (-0.4, 0.5, 0.4, 0.6  ).draw(GREEN); // draw green line
      Tri2  (Vec2(-0.5, -0.4), Vec2(-0.4, -0.2), Vec2(-0.4, -0.4)).draw(YELLOW); // draw yellow triangle from given points
   }
}
/******************************************************************************/
