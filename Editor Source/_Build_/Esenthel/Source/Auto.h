/******************************************************************************/
extern const bool EE_PUBLISH          ;
extern const bool EE_ENGINE_EMBED     ;
extern const bool EE_ANDROID_EXPANSION;
extern cchar *C   EE_PHYSX_DLL_PATH   ;
extern cchar *C   EE_ENGINE_PATH      ;
extern cchar *C   EE_PROJECT_PATH     ;
extern cchar *C   EE_PROJECT_NAME     ;
extern cchar *C   EE_APP_NAME         ;
extern const UID  EE_GUI_SKIN         ;
extern Cipher *C  EE_PROJECT_CIPHER   ;
/******************************************************************************/
void EE_INIT(bool load_engine_data=true, bool load_project_data=true);
void EE_INIT_OBJ_TYPE();
/******************************************************************************/
