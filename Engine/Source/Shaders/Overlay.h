/******************************************************************************/
struct OverlayClass
{
   Vec4   param;
   Matrix mtrx ;
};

BUFFER(Overlay)
   OverlayClass OverlayParams;
BUFFER_END

inline Half OverlayOpaqueFrac() {return OverlayParams.param.x;}
inline Half OverlayAlpha     () {return OverlayParams.param.y;}
/******************************************************************************/
