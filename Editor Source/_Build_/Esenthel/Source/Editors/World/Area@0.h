/******************************************************************************/
/******************************************************************************/
flt HmHeight(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec  &pos, bool smooth=true);
flt HmHeight(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec2 &xz , bool smooth=true);
Vec HmNormal(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec  &pos);
Vec HmNormal(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec2 &xz );
Vec HmNormalAvg(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec  &pos, flt r);
Vec HmNormalAvg(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Vec2 &xz , flt r);
Vec HmNormalAvg(C Heightmap &hm, flt area_size, C VecI2 &area_xy, C Matrix &matrix, C Box &box);
/******************************************************************************/
