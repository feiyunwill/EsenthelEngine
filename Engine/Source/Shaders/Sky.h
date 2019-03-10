/******************************************************************************/
BUFFER(Sky)
   Flt      SkyDnsExp      ,
            SkyHorExp      ;
   Half     SkyBoxBlend    ;
   VecH4    SkyHorCol      ,
            SkySkyCol      ;
   Vec2     SkyFracMulAdd  ,
            SkyDnsMulAdd   ;
   VecH2    SkySunHighlight;
   Vec      SkySunPos      ;
   MatrixH3 SkyStarOrn     ;
BUFFER_END
/******************************************************************************/
