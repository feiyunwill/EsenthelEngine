/******************************************************************************/
/******************************************************************************/
class SetMtrlColorClass : PropWin
{
   bool      mul;
   Vec       rgb;
   Button    ok;
   Memc<UID> elms;

   static void OK(SetMtrlColorClass &smc);

   void create();
   void display(C MemPtr<UID> &elms, bool mul=false);

   virtual void update(C GuiPC &gpc)override;

public:
   SetMtrlColorClass();
};
/******************************************************************************/
/******************************************************************************/
extern SetMtrlColorClass SetMtrlColor;
/******************************************************************************/
