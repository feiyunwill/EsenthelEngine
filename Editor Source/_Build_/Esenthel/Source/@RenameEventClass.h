/******************************************************************************/
/******************************************************************************/
class RenameEventClass : ClosableWindow
{
   int      event_index; // number of events before this one with similar name
   Str      event_name;
   TextLine textline;

   static void Hide(RenameEventClass &re);

   void create();
   void activate(int event);
   virtual void update(C GuiPC &gpc)override;

public:
   RenameEventClass();
};
/******************************************************************************/
/******************************************************************************/
extern RenameEventClass RenameEvent;
/******************************************************************************/
