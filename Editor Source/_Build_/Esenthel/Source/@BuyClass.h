/******************************************************************************/
/******************************************************************************/
class BuyClass : Window
{
   TextNoTest text;
   Button     yes, no;
   flt        time;

   static void Yes(ptr);

   void create();
   virtual Window& show()override;
   virtual Window& hide()override;
   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
   virtual void update(C GuiPC &gpc)override;
   virtual void draw(C GuiPC &gpc)override;

public:
   BuyClass();
};
/******************************************************************************/
/******************************************************************************/
extern BuyClass Buy;
/******************************************************************************/
