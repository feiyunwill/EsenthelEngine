/******************************************************************************/
BUFFER(FurVel) // this CB is dynamically resized, do not add other members
   VecH FurVel[MAX_MATRIX];
BUFFER_END

BUFFER(FurStep)
   VecH2 FurStep; // x=step, y=step+1
BUFFER_END

Image FurCol, FurLight;
/******************************************************************************/
