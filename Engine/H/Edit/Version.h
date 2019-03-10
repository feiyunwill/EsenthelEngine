/******************************************************************************/
namespace Edit{
/******************************************************************************/
struct FileVersion
{
   enum FLAG
   {
      REMOVED   =1<<0, // file is removed
      COMPRESSED=1<<1, // file is compressed
   };

   DateTime time    ; // time at which version was submitted
   UInt     user    , // ID of the user who submitted the version
            revision, // global revision number
            flag    ; // optional flags

   Bool removed   ()C {return FlagTest(flag, REMOVED   );}
   Bool compressed()C {return FlagTest(flag, COMPRESSED);}

   FileVersion& zero(                                                       ) {T.time.zero(); T.user=0   ; T.revision=       0; T.flag=   0; return T;}
   FileVersion& set (C DateTime &time, UInt user, UInt revision, UInt flag=0) {T.time=time  ; T.user=user; T.revision=revision; T.flag=flag; return T;}

            FileVersion(                                                       ) {zero();}
   explicit FileVersion(C DateTime &time, UInt user, UInt revision, UInt flag=0) {set (time, user, revision, flag);}
};
/******************************************************************************/
Bool GetVersions(         C Str &name, MemPtr<FileVersion> versions);
Bool GetVersion (File &f, C Str &name, C DateTime       &time    , Bool treat_base_as_latest); // 'treat_base_as_latest'=if treat the base file as the latest version, this way the latest version will always be saved only as the base file, and not in the version folder
Bool AddVersion (File &f, C Str &name, C FileVersion    &version , Bool treat_base_as_latest); // 'treat_base_as_latest'=if treat the base file as the latest version, this way the latest version will always be saved only as the base file, and not in the version folder
/******************************************************************************/
} // namespace
/******************************************************************************/
