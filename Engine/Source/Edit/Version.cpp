/******************************************************************************/
#include "stdafx.h"
namespace EE{
static Int Compare(C Edit::FileVersion &a, C Edit::FileVersion &b) {return Compare(b.time, a.time);} // compare in reversed order so dates are stored from latest to oldest
namespace Edit{
/******************************************************************************/
static Bool LoadVersions(File &f, MemPtr<FileVersion> versions, Bool latest_only=false)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         if(!latest_only)return versions.loadRaw(f);
         if(f.decUIntV()>=1) // if at least one element then read it
         {
            if(versions.resizable()   )versions.setNum(1);
            if(versions.elms     ()!=1)goto error;
            f>>versions[0];
         }else // otherwise clear result
         {
            if(versions.resizable()   )versions.clear();
            if(versions.elms     ()!=0)goto error;
         }
      }return true;
   }
error:;
   if(versions.resizable())versions.clear();else REPAO(versions).zero();
   return false;
}
static void SaveVersions(File &f, C MemPtr<FileVersion> &versions)
{
   f.cmpUIntV(0);
   versions.saveRaw(f);
}
static Bool          LoadVersions(C Str &name,   MemPtr<FileVersion>  versions, Bool latest_only=false) {File f; if(f.readStdTry(name))return LoadVersions(f, versions, latest_only); if(versions.resizable())versions.clear();else REPAO(versions).zero(); return false;}
static Bool          SaveVersions(C Str &name, C MemPtr<FileVersion> &versions                        ) {File f; if(f.writeTry  (name)){      SaveVersions(f, versions             ); return true;} return false;}
static Bool SafeOverwriteVersions(C Str &name, C MemPtr<FileVersion> &versions                        ) {File f;    f.writeMem  (    );       SaveVersions(f, versions             ); f.pos(0); return SafeOverwrite(f, name);}
/******************************************************************************/
static Str VersionFolder(C Str &name) // get the name of the folder which will contain all versions of the 'name' file, "C:/Folder/file.ext" -> "C:/Folder/Version/file.ext"
{
   return GetPath(name).tailSlash(true)+"Version/"+GetBase(name);
}
static Str VersionIndex(C Str &name) // get the name of the version index file of the 'name' file
{
   return VersionFolder(name)+"/Index";
}
static Str VersionName(C Str &name, C DateTime &date_time) // get the name of the 'date_time' version of the 'name' file
{
   return VersionFolder(name)+'/'+date_time.asFileName();
}
/******************************************************************************/
static Bool ReadVersion(File &f, C Str &name, C DateTime &date_time) // open the 'f' file for reading of the 'date_time' version of the 'name' file
{
   return f.readStdTry(VersionName(name, date_time));
}
static Bool WriteVersion(File &f, C Str &name, C DateTime &date_time) // open the 'f' file for writing of the 'date_time' version of the 'name' file
{
   return f.writeTry(VersionName(name, date_time));
}
static Bool SaveVersion(File &f, C Str &name, C DateTime &date_time) // save 'f' file contents as the 'date_time' version of the 'name' file
{
   File dest; if(WriteVersion(dest, name, date_time))return f.copy(dest) && dest.flush();
   return false;
}
static Bool SafeOverwriteVersion(File &f, C Str &name, C DateTime &date_time)
{
   return SafeOverwrite(f, VersionName(name, date_time));
}
/******************************************************************************/
Bool GetVersions(C Str &name, MemPtr<FileVersion> versions)
{
   return LoadVersions(VersionIndex(name), versions);
}
Bool GetVersion(File &f, C Str &name, C DateTime &time, Bool treat_base_as_latest)
{
   if(treat_base_as_latest) // check if we're loading latest version
   {
      FileVersion version;
      if(LoadVersions(VersionIndex(name), version, true)) // load info only about latest version
         if(version.time==time) // if that's it
            return f.readStdTry(name); // read base file
   }
   return ReadVersion(f, name, time);
}
Bool AddVersion(File &f, C Str &name, C FileVersion &version, Bool treat_base_as_latest)
{
   Memt<FileVersion> versions;
   Str               index=VersionIndex(name);

   if(!LoadVersions(index, versions)) // if fails to load then continue still, because the file may just not exist yet
      FCreateDirs(VersionFolder(name)); // however create the version folder as it may not exist yet as well

   if(versions.binaryInclude(version, Compare)) // if version doesn't exist yet
      if(version.removed() || SaveVersion(f, name, version.time)) // if written file data (skip saving if file is removed)
   {
      if(treat_base_as_latest) // check if we're saving latest version
         if(versions[0].time==version.time) // we've just added latest version
      {
         if(versions.elms()>=2 && !versions[1].removed())FRename(name, VersionName(name, versions[1].time)      ); // move previous latest from base into the version folder
         if(                      !versions[0].removed())FRename(      VersionName(name, versions[0].time), name); // move new      latest from version folder to base
      }
      return SafeOverwriteVersions(index, versions); // save new index
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
