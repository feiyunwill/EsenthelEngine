/******************************************************************************/
int    value   ; // simple integer value
Button button_a, // gui button
       button_b; // gui button
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
void Func(ptr) // custom function which will be called when button pushed
{
   value++;
}
bool Init()
{
   Gui+=button_a.create(Rect_C(-0.3, 0, 0.45, 0.08), "Button Func"  ).func(Func); // create button which will call 'Func' function when pushed
   Gui+=button_b.create(Rect_C( 0.3, 0, 0.45, 0.08), "Button Toggle")           ; button_b.mode=BUTTON_TOGGLE; // create button and set a special "toggle" mode

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

   if(button_b())value++; // manually check if button is enabled

   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(TURQ);
   Gui.draw ();

   D.text(0, 0.6, S+"value = "     +value     ); // draw 'value' on screen
   D.text(0, 0.5, S+"button_b() = "+button_b()); // draw state of 'button_b'
}
/******************************************************************************/
