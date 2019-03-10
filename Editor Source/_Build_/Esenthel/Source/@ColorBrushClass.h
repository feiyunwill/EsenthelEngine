/******************************************************************************/
/******************************************************************************/
class ColorBrushClass : Window
{
   class Col : GuiCustom
   {
      Vec col;

      virtual void update(C GuiPC &gpc)override;
      virtual void draw(C GuiPC &gpc)override;
   };

   Col         col[3*4];
   int         sel;
   ColorPicker cp;
   SyncLock    lock;

   void set(int i, SET_MODE mode=SET_DEFAULT);
   static void Changed(ColorBrushClass &cb);
   ColorBrushClass& create();

   virtual Window& hide()override;

   static void EditColor(Cell<Area> &cell, Vec &color, int thread_index);

   void update();

public:
   ColorBrushClass();
};
/******************************************************************************/
/******************************************************************************/
extern ColorBrushClass ColorBrush;
/******************************************************************************/
