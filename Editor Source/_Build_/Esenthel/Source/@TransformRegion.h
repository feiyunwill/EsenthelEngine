/******************************************************************************/
/******************************************************************************/
class TransformRegion : Region
{
   enum ANCHOR
   {
      SEL_CENTER,
      SEL_LEFT,
      SEL_RIGHT,
      SEL_DOWN,
      SEL_UP,
      SEL_BACK,
      SEL_FRONT,
      WORLD_CENTER,
      CAM_TARGET,
   };
   static cchar8 *AnchorName[]
;

   bool           full;
   Property      *move_p[3];
   Memx<Property> props;
   TextWhite      ts;
   Button         rescale_height, rescale_depth, move_bottom, move_back, move_center, move_center_xz, rest_bottom, rot_y_min_box, ok, reset, cancel, close, original,
                  rot[3][2]; // [xyz][dec/inc]
   TextLine       rescale_height_value, rescale_depth_value;
   Pose           trans;
   flt            trans_normal;
   Vec            trans_scale, anchor_pos;
   Vec2           move_uv, scale_uv;
   Matrix         matrix;
   ANCHOR         anchor;

   static void RescaleHeight(TransformRegion &tr);
   static void RescaleDepth(TransformRegion &tr);
   static void MoveBottom  (TransformRegion &tr);
   static void MoveBack    (TransformRegion &tr);
   static void MoveCenter  (TransformRegion &tr);
   static void MoveCenterXZ(TransformRegion &tr);
   static void RestBottom  (TransformRegion &tr);
   static void RotYMinBox  (TransformRegion &tr);
   static void OK          (TransformRegion &tr);
   static void Reset       (TransformRegion &tr);
   static void Cancel      (TransformRegion &tr);
   static void Original    (TransformRegion &tr);

   static void RotX(  TransformRegion &tr, C Str &text);
   static void RotY(  TransformRegion &tr, C Str &text);
   static void RotZ(  TransformRegion &tr, C Str &text);
   static Str  RotX(C TransformRegion &tr             );
   static Str  RotY(C TransformRegion &tr             );
   static Str  RotZ(C TransformRegion &tr             );

   static void Dec90(Property &prop);
   static void Inc90(Property &prop);

   void moveBottom  ();
   void moveBack    ();
   void moveCenter  ();
   void moveCenterXZ();
   void restBottom  ();
   void rotYMinBox();

   void toGui();

   static void Changed  (C Property &prop);
          void setMatrix();

   Matrix drawMatrix()C;
   void setAnchorPos(); 
   Vec anchorPos()C;
   void resetDo();
   void hideDo();
   void cancelDo();
   void apply();
   virtual void update(C GuiPC &gpc)override;

   TransformRegion& create(bool full);

public:
   TransformRegion();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
