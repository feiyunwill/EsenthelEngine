/******************************************************************************/
/******************************************************************************

   Publishing is best done in separate state so:
      -we don't allow editing   project elements during paking
      -we don't allow receiving project elements from the server during paking

   Windows:
      Engine.pak
      Project.pak
      EngineEmbed.pak (for engine data embedded into EXE, used when 'appEmbedEngineData')
      App.pak         (for app specific data embedded into EXE, used always             )

   Android:
      Engine.pak
      Project.pak (will contain only app specific data when "!appPublishProjData")

/******************************************************************************/
class ImageGenerate
{
   Str      src_mtrl, dest_base_0;
   DateTime time;

   void set(C Str &src_mtrl, C Str &dest_base_0, C DateTime &time);

   void process();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
