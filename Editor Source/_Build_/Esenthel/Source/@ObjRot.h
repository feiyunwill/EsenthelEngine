/******************************************************************************/
/******************************************************************************/
class ObjRot : Region
{
   Memx<Property> props;
   TextBlack      ts;
   Vec            angles;
   bool           multi_val;

   static Vec Angles(C Matrix3 &m);

   class Rot
   {
      Vec angles, scale;

      operator Matrix3();
       Rot(C Matrix3 &m);
   };

   static void ChangedX(ObjRot &os);
   static void ChangedY(ObjRot &os);
   static void ChangedZ(ObjRot &os);

   ObjRot& create(C Vec2 &up);
   void reset(int skip=-1);
   virtual void update(C GuiPC &gpc)override;

public:
   ObjRot();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
