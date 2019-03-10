/******************************************************************************/
/******************************************************************************/
class RenameSlotClass : ClosableWindow
{
   int      slot_index; // number of slots before this one with similar name
   Str      slot_name;
   TextLine textline;

   void create();
   void activate(int slot);
   virtual void update(C GuiPC &gpc)override;

public:
   RenameSlotClass();
};
/******************************************************************************/
/******************************************************************************/
extern RenameSlotClass RenameSlot;
/******************************************************************************/
