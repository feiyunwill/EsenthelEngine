/******************************************************************************/
Window   window  ; // gui window
SlideBar slidebar; // gui slidebar
Progress progress; // gui progressbar
Slider   slider  ; // gui slider
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   Gui   +=window  .create(Rect(-0.5, -0.4, 0.5,  0.40), "Window with bars");
   window+=slidebar.create(Rect( 0.1, -0.2, 0.9, -0.12)); // create slidebar
   window+=progress.create(Rect( 0.1, -0.4, 0.9, -0.32)); // create progressbar
   window+=slider  .create(Rect( 0.1, -0.6, 0.9, -0.52)); // create slider

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
   Gui.update();

   progress.set(slidebar()); // set progress bar according to slidebar value

   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(TURQ);
   Gui.draw();
   D.text(0, -0.7, S+"SlideBar "   +slidebar()); // draw slidebar    value
   D.text(0, -0.8, S+"ProgressBar "+progress()); // draw progressbar value
   D.text(0, -0.9, S+"Slider "     +slider  ()); // draw slider      value
}
/******************************************************************************/
