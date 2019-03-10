/******************************************************************************/
Vec2 point; // point position
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

   if(Kb.b(KB_LEFT ))point.x-=Time.d()/2; // move point left  when 'left  arrow' is enabled according to time delta
   if(Kb.b(KB_RIGHT))point.x+=Time.d()/2; // move point right when 'right arrow' is enabled according to time delta
   if(Kb.b(KB_DOWN ))point.y-=Time.d()/2; // move point down  when 'down  arrow' is enabled according to time delta
   if(Kb.b(KB_UP   ))point.y+=Time.d()/2; // move point up    when 'up    arrow' is enabled according to time delta

   if(Kb.bp(KB_Z))point.x-=0.1; // when 'z' is pushed        , move point left
   if(Kb.br(KB_X))point.x+=0.1; // when 'x' is released      , move point right
   if(Kb.bd(KB_C))point.y+=0.1; // when 'c' is double clicked, move point up

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   D.dot(RED  , Ms.pos()); // draw red   dot at mouse cursor position
   D.dot(GREEN, point   ); // draw green dot at 'point' position
   
   if(Ms.b(0))D.dot(BLACK, -0.1, 0.4, 0.1); // when 0th mouse button on, draw big black dot
   if(Ms.b(1))D.dot(BLACK,  0.1, 0.4, 0.1); // when 1st mouse button on, draw big black dot

   D.text(0, 0.9, S+"Press keyboard arrows to move the point");

   D.text(0, 0.7, S+"mouse : "+Ms.pos()); // draw mouse position
   D.text(0, 0.6, S+"point : "+point   ); // draw point position
}
/******************************************************************************/
