/******************************************************************************/
extern State                         StateImport;
extern Str                           ImportSrc;
extern BackgroundFileFind            ImportBFF;
extern IMPORT_PHASE                  ImportPhase;
extern Memc<BackgroundFileFind::File> ImportFiles, ImportManual;
extern Map<Str, UID>                 ImportElms;
extern Memc<Str>                     ImportMtrlImages;
extern Memc<Str>                     ImportObjMeshes;
extern Map<UID, Matrix>              ImportObjMatrix;
/******************************************************************************/
int  ImportComparePath(C Str &a, C Str &b);
Elm* ImportFind(C Str &name, ELM_TYPE type);
UID ImportFindID(C Str &name, ELM_TYPE type);
Str ImportFindGamePath(C Str &name, ELM_TYPE type);
void Clean(GuiObj &obj);
void Adjust(GuiObj &obj);
void Adjust(MaterialPtr &mtrl);
void Adjust(Mesh &mesh);
void Adjust(Heightmap &heightmap);
void Adjust(EditObject &obj);
bool ImportFunc(Thread &thread);
bool ImportFilter(C Str &name);
bool InitImport();
void ShutImport();
bool UpdateImport();
void DrawImport();
/******************************************************************************/
