/******************************************************************************/
/******************************************************************************/
class RenameElmClass : ClosableWindow
{
   UID      elm_id;
   TextLine textline;

   void create();
   void activate(C UID &elm_id, C Str &name);
   void activate(C UID &elm_id);             
   void activate(  Elm *elm   );             
   virtual void update(C GuiPC &gpc)override;

public:
   RenameElmClass();
};
/******************************************************************************/
/******************************************************************************/
extern RenameElmClass RenameElm;
/******************************************************************************/
