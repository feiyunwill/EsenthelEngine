/******************************************************************************/
/******************************************************************************/
class UpdateWindowClass : ClosableWindow
{
   Text   text;
   Button apply, show_changes, not_now;

   static void ApplyDo(bool all_saved=true, ptr=null);

   static void ShowChanges(UpdateWindowClass &uw);
   static void Apply      (UpdateWindowClass &uw);

   void create();
   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
   virtual void update(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern UpdateWindowClass UpdateWindow;
/******************************************************************************/
