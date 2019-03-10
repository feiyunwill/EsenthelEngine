/******************************************************************************/
struct CloudLayer
{
   VecH4 color;
   VecH2 scale, position;
};

BUFFER(CloudLayer)
   Half  LCScaleY;
   VecH2 LCMaskContrast;
   Vec2  LCRange;

   CloudLayer CL[4];
BUFFER_END
/******************************************************************************/
inline Flt CloudAlpha(Flt y) {return y*8-0.15f;}
/******************************************************************************/
