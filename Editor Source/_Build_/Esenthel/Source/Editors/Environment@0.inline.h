/******************************************************************************/
   template<int i>   void EnvEditor::CloudsColor(  EditEnv &env, C Str &text) {env.clouds.layers[i].color=TextVec4(text); env.clouds_color_time[i].getUTC();}
   template<int i>   Str  EnvEditor::CloudsColor(C EditEnv &env             ) {return env.clouds.layers[i].color;}
   template<int i>   void EnvEditor::CloudsScale(  EditEnv &env, C Str &text) {env.clouds.layers[i].scale=TextFlt(text); env.clouds_scale_time[i].getUTC();}
   template<int i>   Str  EnvEditor::CloudsScale(C EditEnv &env             ) {return env.clouds.layers[i].scale;}
   template<int i>   void EnvEditor::CloudsVel(  EditEnv &env, C Str &text) {env.clouds.layers[i].velocity=TextVec2(text); env.clouds_velocity_time[i].getUTC();}
   template<int i>   Str  EnvEditor::CloudsVel(C EditEnv &env             ) {return env.clouds.layers[i].velocity;}
   template<int i>   void EnvEditor::CloudsImage(  EditEnv &env, C Str &text) {env.cloud_id[i]=Proj.findElmImageID(text); env.clouds_image_time[i].getUTC();}
   template<int i>   Str  EnvEditor::CloudsImage(C EditEnv &env             ) {return Proj.elmFullName(env.cloud_id[i]);}
/******************************************************************************/
