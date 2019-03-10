/******************************************************************************

   Use 'Fog' to set custom global fog.

/******************************************************************************/
struct FogClass
{
   Bool draw      , // if draw the fog   ,    true/false   , default=false
        affect_sky; // if fog affects sky,    true/false   , default=false
   Flt  density   ; // fog density       ,       0..1      , default=0.02
   Vec  color     ; // fog color         , (0,0,0)..(1,1,1), default=(0.5, 0.5, 0.5)

#if EE_PRIVATE
   void Draw(Bool after_sky);

   FogClass();
#endif
}extern
   Fog; // Global Fog Control
/******************************************************************************/
void       FogDraw(C OBox &obox, Flt density, C Vec &color); // draw local 'obox' based        fog, with uniform  'density', this can be called only in RM_CLOUD or RM_BLEND rendering modes
void       FogDraw(C Ball &ball, Flt density, C Vec &color); // draw local 'ball' based        fog, with uniform  'density', this can be called only in RM_CLOUD or RM_BLEND rendering modes
void HeightFogDraw(C OBox &obox, Flt density, C Vec &color); // draw local 'obox' based height fog, with variable 'density', this can be called only in RM_CLOUD or RM_BLEND rendering modes
/******************************************************************************/
