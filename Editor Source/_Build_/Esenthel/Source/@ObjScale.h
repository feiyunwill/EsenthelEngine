/******************************************************************************/
/******************************************************************************/
class ObjScale : Region
{
   Memx<Property> props;
   TextBlack      ts;
   Vec            scale;
   flt            scale_all;
   bool           multi_val[4];

   static void ChangedX(ObjScale &os);
   static void ChangedY(ObjScale &os);
   static void ChangedZ(ObjScale &os);
   static void ChangedXYZ(ObjScale &os);

   ObjScale& create(C Vec2 &up);
   void reset(int skip=-1);
   virtual void update(C GuiPC &gpc)override;

public:
   ObjScale();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
