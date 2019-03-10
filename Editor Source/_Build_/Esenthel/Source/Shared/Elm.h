/******************************************************************************/
extern cchar8 *ElmTypeName[]
;
extern cchar8 *ElmTypeClass[]
;
extern cchar8 *ElmTypeDesc[]
;
/******************************************************************************/
bool ElmCompressable(ELM_TYPE type);
bool ElmHasFile     (ELM_TYPE type);
bool ElmFileInShort (ELM_TYPE type);
bool ElmEdit        (ELM_TYPE type);
bool ElmGame        (ELM_TYPE type);
bool ElmSendBoth    (ELM_TYPE type);
bool ElmInFolder    (ELM_TYPE type);
bool ElmManualSync       (ELM_TYPE type);
bool ElmPublish          (ELM_TYPE type);
bool ElmPublishNoCompress(ELM_TYPE type);
bool ElmCanAccessID      (ELM_TYPE type);
bool ElmImageLike (ELM_TYPE type              );
bool ElmCompatible(ELM_TYPE src, ELM_TYPE dest);
bool ElmMovable        (ELM_TYPE type);
bool ElmVisible        (ELM_TYPE type);
bool ElmCanHaveChildren(ELM_TYPE type);
bool EqualID(C UID &id, C UID &src_id);
bool NewerID(C UID &id, C UID &src_id);
bool  SyncID(  UID &id, C UID &src_id);
bool  UndoID(  UID &id, C UID &src_id);
/******************************************************************************/
