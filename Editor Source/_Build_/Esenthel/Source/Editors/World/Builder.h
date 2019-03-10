/******************************************************************************/
extern const bool FastTerrainPhys;
/******************************************************************************/
void Build(Heightmap &hm, Mesh &mesh, flt area_size, C VecI2 &area_xy, Heightmap *l, Heightmap *r, Heightmap *b, Heightmap *f, Heightmap *lb, Heightmap *lf, Heightmap *rb, Heightmap *rf);
void Simplify(Mesh &mesh);
/******************************************************************************/
