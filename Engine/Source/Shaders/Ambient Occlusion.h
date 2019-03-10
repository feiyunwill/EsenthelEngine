/******************************************************************************/
BUFFER(AO)
   VecH AmbColor;
   Half AmbMaterial=1; // if apply Material Ambient
   Half AmbContrast=1.0f,
        AmbScale   =2.5f,
        AmbBias    =0.3f;
   Vec2 AmbRange   =Vec2(0.3f, 0.3f);
BUFFER_END
/******************************************************************************/
