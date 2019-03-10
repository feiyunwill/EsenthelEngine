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
   D.clear(TURQ);
   if(Joypads.elms())
   {
      D.text(0, 0.9, Joypads[0].name());
      Str buttons; FREP(32)buttons+=Joypads[0].b(i); D.text(Rect_C(0, 0.7, 0, 0), S+"Buttons:\n"+buttons);
      Rect_C( 0  , 0, 0.5, 0.5).draw(BLACK, false); (Joypads[0].dir     *0.25              ).draw(RED);
      Rect_C( 0.6, 0, 0.5, 0.5).draw(BLACK, false); (Joypads[0].dir_a[0]*0.25+Vec2(-0.6, 0)).draw(RED);
      Rect_C(-0.6, 0, 0.5, 0.5).draw(BLACK, false); (Joypads[0].dir_a[1]*0.25+Vec2( 0.6, 0)).draw(RED);
   }else
   {
      D.text(0, 0, S+"No Joypads detected");
   }
}
/******************************************************************************/
