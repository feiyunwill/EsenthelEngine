/******************************************************************************/
/******************************************************************************/
class ObjPos : Region
{
   Memx<Property> props;
   TextBlack      ts;
   Button         cancel;
   bool           applying;
   Vec            delta;

   static void Cancel (ObjPos &op);
   static void Changed(ObjPos &op);

   ObjPos& create(C Vec2 &up);
   void apply();
   void move(C Vec &delta);
   void reset();
   virtual void update(C GuiPC &gpc)override;

public:
   ObjPos();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
