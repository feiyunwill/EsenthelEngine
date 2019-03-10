/******************************************************************************/
/******************************************************************************/
bool LoadEditHeightmap(C Str &name, Heightmap &hm, C Str &game_path);
void SaveEditObject(Chunks &chunks, Memc<ObjData> &objs, C Str &edit_path);
void SaveGameObject(Chunks &chunks, Memc<ObjData> &objs, C Project &proj, WorldVer &world_ver);
bool LoadEditObject(int ver, File &f, Memc<ObjData> &objs, C Str &edit_path);
bool LoadEditObject(C Str &name, Memc<ObjData> &objs, C Str &edit_path);
bool LoadEdit(C Str &name, Heightmap *hm, Memc<ObjData> *objs, C Str &game_path, C Str &edit_path);
bool LoadGame(C Str &name, Mesh *hm_mesh, PhysPart *hm_phys, Memc<Game::Area::Data::AreaObj> *objs, MeshGroup *obj_mesh, PhysBody *obj_phys, Memc<WaterMesh> *waters, C Str &game_path);
/******************************************************************************/
