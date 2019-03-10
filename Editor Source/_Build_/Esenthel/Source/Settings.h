/******************************************************************************/
extern TextData Settings;
/******************************************************************************/
bool SaveSettings(C Str &name="Settings.txt");
bool LoadSettings(C Str &name="Settings.txt");
void ApplyVideoSettings(C TextData &data=Settings);
void ApplySettings(C TextData &data=Settings);
/******************************************************************************/
