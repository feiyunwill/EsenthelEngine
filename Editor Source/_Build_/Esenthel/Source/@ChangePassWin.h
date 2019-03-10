/******************************************************************************/
/******************************************************************************/
class ChangePassWin : ClosableWindow
{
   bool     use_key;
   Text   t_cur, t_new_pass;
   TextLine cur,   new_pass;
   Button   ok;

   static void ChangePass(ChangePassWin &cpw);
   void changePass();
   void activate(bool use_key);
   void create();
   virtual void update(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern ChangePassWin ChangePass;
/******************************************************************************/
