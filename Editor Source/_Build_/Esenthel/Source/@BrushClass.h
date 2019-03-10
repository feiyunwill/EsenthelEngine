/******************************************************************************/
/******************************************************************************/
class BrushClass : Window // Brush (Size + Speed + Soft + Image)
{
   class Slope : GuiCustom
   {
      static flt  SlopeF    (flt step           );
      static flt  SlopeX    (flt x, C Rect &rect);
      static flt  SlopeY    (flt y, C Rect &rect);
      static void PushedSide(Slope &slope       );

      flt    val ;
      bool   side;
      Button side_b;

      flt cos()C;

      virtual void update(C GuiPC &gpc)override;
      virtual void draw(C GuiPC &gpc)override;

public:
   Slope();
   };

   bool      alt_rotate;
   TextBlack ts;
   Window    slope_window, image_window;
   Button    bsize;
   Text      tsize, tspeed, tsoft, tangle;
   Slider    ssize, sspeed, ssoft, sangle;
   ComboBox  cpattern;
   Tabs      shape;
   Button    slope_b, image, pattern;
   ImageSkin image_image;
   WindowIO  image_io;
   flt       rotate_cos, rotate_sin, rotate_scale, pattern_scale;
   Image     image_sw, image_hw;
   Str       image_name;
   Slope     slope;

   static void SetImage   (C Str &file, BrushClass &brush);
   static void ImageWindow(             BrushClass &brush);
   static void SlopeWindow(             BrushClass &brush);

   static cchar8 *PatternText[]
;

   // manage
   BrushClass& create(GuiObj &parent, C Vec2 &rd);

   // operations
   void setImage(C Str &name);
   void setChildWindows();

   virtual Window& hide()override;
   virtual Window& show()override;

   virtual bool hasMsWheelFocus()C;

   virtual void update(C GuiPC &gpc)override;
   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos);

public:
   BrushClass();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
