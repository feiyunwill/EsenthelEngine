/******************************************************************************/
class LoginClass
{
   Text   t_email; TextLine email;
   Text   t_pass ; TextLine pass ;
   Button login;

   static void Login(LoginClass &login)
   {
      ClientSendLogin(Server, login.email(), login.pass());
   }

   void create()
   {
      Gui+=t_email.create(Vec2(0.3, 0.4), "E-mail"  ); Gui+=email.create(Rect_C(0.3,  0.3, 0.5, 0.08), "test@test.com");
      Gui+=t_pass .create(Vec2(0.3, 0.1), "Password"); Gui+=pass .create(Rect_C(0.3,  0.0, 0.5, 0.08), "pass");
                                                       Gui+=login.create(Rect_C(0.3, -0.3, 0.5, 0.08), "Login").func(Login, T);
   }
}
LoginClass Login;
/******************************************************************************/
class MoveClass
{
   Button up, down, left, right;

   static void Up   (ptr) {Plr.move(Vec2( 0,  1));}
   static void Down (ptr) {Plr.move(Vec2( 0, -1));}
   static void Left (ptr) {Plr.move(Vec2(-1,  0));}
   static void Right(ptr) {Plr.move(Vec2( 1,  0));}

   void create()
   {
      Vec2 pos(-0.3, -0.3); flt size=0.04;
      Gui+=up   .create(Rect_C(pos+Vec2(0,  size*2), size), "U").func(Up);
      Gui+=down .create(Rect_C(pos+Vec2(0, -size*2), size), "D").func(Down);
      Gui+=left .create(Rect_C(pos+Vec2(-size*2, 0), size), "L").func(Left);
      Gui+=right.create(Rect_C(pos+Vec2( size*2, 0), size), "R").func(Right);
      up.mode=down.mode=left.mode=right.mode=BUTTON_CONTINUOUS;
   }
}
MoveClass Move;
/******************************************************************************/
void InitGui()
{
   Login.create();
   Move .create();
}
/******************************************************************************/
void DrawMap()
{
   Rect_C map_rect(-0.3, 0.1, 0.5, 0.5);

   map_rect.draw(WHITE, true );
   map_rect.draw(BLACK, false);

   D.clip(map_rect);
   REPAO(Neighbors).draw(map_rect);
   Plr.draw(map_rect);
   
   D.clip(null);
   D.text(map_rect.up()+Vec2(0, 0.04), S+"Neighbors: "+Neighbors.elms());
}
/******************************************************************************/
