/******************************************************************************/
BUFFER(WaterSurface)
   Flt  WaterScaleDif,
        WaterScaleNrm,
        WaterScaleBump,
        WaterRgh_2,
        WaterRflFake,
        WaterSpc,
        WaterFresnelPow,
        WaterFresnelRough;
   Vec  WaterFresnelColor;
   Vec  WaterCol;
   Vec4 WaterBumpDot;
BUFFER_END

BUFFER(Water)
   Flt  WaterRfl,
        WaterRfr,
        WaterRfrRfl,
        WaterUnder,
        WaterUnderRfr,
        WaterFlow;
   Vec2 WaterOfs;
   Vec2 WaterDns;
   Vec  WaterUnderCol0,
        WaterUnderCol1;
   Vec4 WaterRflMulAdd,
        WaterClamp;

   Flt  WaterWave;
   Vec2 WaterYMulAdd;
   Vec  WaterPlnPos,
        WaterPlnNrm;
BUFFER_END
/******************************************************************************/
