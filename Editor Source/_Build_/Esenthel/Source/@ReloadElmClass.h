/******************************************************************************/
/******************************************************************************/
class ReloadElmClass : ClosableWindow
{
   TextNoTest text, text_all, t_name;
   TextLine   path, name, start_frame, end_frame, speed, optimize;
   Button     path_sel, yes, yes_all, set_start_frame, set_end_frame, set_speed, set_optimize, mirror;
   WindowIO   win_io;
   Memc<UID>  elms;

   static void Select(C Str &name, ReloadElmClass&re);
   static void Select(ReloadElmClass &re);          
   static void Reload(ReloadElmClass &re);          
          void reload();
   void activate(Memc<UID> &elms); // multiple elements
   void activate(C UID &elm_id); // single element
   virtual ReloadElmClass& hide()override;

   static void ToggleStart   (ReloadElmClass &re);
   static void ToggleEnd     (ReloadElmClass &re);
   static void ToggleSpeed   (ReloadElmClass &re);
   static void ToggleOptimize(ReloadElmClass &re);

   void create();
   virtual void update(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern ReloadElmClass ReloadElm;
/******************************************************************************/
