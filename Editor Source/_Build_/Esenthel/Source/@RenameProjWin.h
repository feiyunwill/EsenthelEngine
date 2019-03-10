/******************************************************************************/
/******************************************************************************/
class RenameProjWin : ClosableWindow
{
   Text   t_name;
   TextLine name;
   Button     ok;
   UID   proj_id;

   static void OK(RenameProjWin &rpw);

   void activate(Projects::Elm &proj);
   void create();
   virtual void update(C GuiPC &gpc)override;

public:
   RenameProjWin();
};
/******************************************************************************/
/******************************************************************************/
extern RenameProjWin RenameProj;
/******************************************************************************/
