/******************************************************************************

   This tutorial presents the basics of patching game files.

   First, you need to upload all your files to the server using "Uploader" tool.

   Once files are available, you need to use 'Patcher' class,
      which will help you perform the patching:
         -remove   obsolete files
         -download updated  files

   For test purposes we'll use files that have been uploaded on Esenthel server,
      using "Uploader" tool with following options:
         Http Directory: "http://www.esenthel.com/download/Patcher"
         Upload Name   : "Test"
         Secure        :  NULL

/******************************************************************************/
const_mem_addr class PakFileDataEx : PakFileData
{
   File file_data;
}
Patcher                 patcher; // define patcher object
Str                     paks_dir="LocalData/Patch"; // define where we'll store pak files
Memc<Patcher.LocalFile> local_files; // list of files currently present in pak files
bool                    compared=false; // helper variable used to specify if we have compared local with server files
Memx<PakFileDataEx>     update_files; // list of files to be updated in our paks
bool                    multiple_paks=true; // if we're using multiple paks for our data files (for example: "data.pak", "patch.pak"), you can set this to false if in your game you're using only one pak (for example: "data.pak")
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
bool Init()
{
   FCreateDirs(paks_dir); // make sure paks folder exists
   PakSet paks; // use PakSet to obtain information about all files in our Paks
   paks.addTry(paks_dir.tailSlash(true) + "data.pak" , null, false); // add files from the main data  pak
   paks.addTry(paks_dir.tailSlash(true) + "patch.pak", null, false); // add files from the      patch pak
   paks.rebuild(); // rebuild pak database
   Patcher.SetList(local_files, paks); // automatically generate list of files present in our paks

   patcher.create("http://www.esenthel.com/download/Patcher", "Test", null); // initialize the patcher, those parameters must exactly match those specified in the "Uploader" tool (!! strings are case-sensitive in this case !!)
   patcher.downloadIndex(); // request files index to be downloaded

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC  ))return false;
   if(Kb.bp(KB_SPACE))Explore(paks_dir); // preview data folder when space pressed

   // check if the index was downloaded
   if(C Pak *pak=patcher.index()) // if index is available
   {
      if(!compared) // if we haven't compared the files yet
      {
         compared=true; // set compared, so we won't process this again next frame

         // compare files
         Memc<int> local_remove  , // list of indexes in 'local_files' container that should be removed
                  server_download; // list of indexes in 'pak->file'   container that should be downloaded
         patcher.compare(local_files, local_remove, server_download); // compare information about local files with server files, and get list of which files should be removed, and which should be downloaded

         // remove files
         REPA(local_remove)
         {
            Patcher.LocalFile  &lf =local_files[local_remove[i]];
            PakFileData        &pfd=update_files.New();
            pfd.name           =lf.full_name;       // set file full name
            pfd.modify_time_utc=lf.modify_time_utc; // set file modification time
            pfd.mode           =(multiple_paks ? PakFileData.MARK_REMOVED : PakFileData.REMOVE); // remove the file
         }

         // download files
         REPA(server_download)patcher.downloadFile(server_download[i]); // request the files to be downloaded
      }
   }

   // process all files that have been downloaded
   Patcher.Downloaded downloaded;
   if(patcher.getNextDownload(downloaded)) // if there is a file downloaded
   {
      if(downloaded.success)
      {
         // file was downloaded successfully
         PakFileDataEx &pfd=update_files.New();
         pfd.name           =downloaded.full_name; // set file full name
         pfd.mode           =PakFileData.REPLACE; // mark the file as to be replaced
         pfd.xxHash64_32    =downloaded.xxHash64_32;
         pfd.modify_time_utc=downloaded.modify_time_utc;
         Swap(pfd.file_data, downloaded.data); // swap the file data from 'downloaded' object to 'PakFileDataEx'
         pfd.data.set(pfd.file_data);
      }else
      {
         // file wasn't properly downloaded, probable causes:
         //    -we got disconnected from the server
         //    -file was removed or was of different version than expected (this can happen if during our download, there was a newer version uploaded)
         // what we can do:
         //    -try downloading the file again - call 'downloadFile'  again (can help if we got disconnected, but won't help if the server has a newer version of the file)
         //    -redownload the index           - call 'downloadIndex' again, which should have latest information about all updates files
      }
   }else
   {
      // there is no file downloaded, check if we have finished downloading all of them:
      if(!patcher.filesLeft()  // there are no files left to download
      &&  compared           ) // the index was downloaded and we have performed the comparison
      {
         // being here means that we have downloaded all files and we're ready to update the paks
         if(update_files.elms()) // if there are any files that need to be updated
         {
            // perform updating the "patch.pak"
            Str pak_name=paks_dir.tailSlash(true) + "patch.pak";
            Pak pak; pak.load(pak_name, null); // load current version
            PakUpdate(pak, SCAST(Memx<PakFileData>, update_files), pak_name, null); // perform the update
            update_files.del(); // clear the container so we won't process the files again
         }
         // paks are now updated
      }
   }

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   
   Str index_state;
   switch(patcher.indexState())
   {
      case DWNL_NONE      : index_state="None"                  ; break;
      case DWNL_CONNECTING: index_state="Waiting for connection"; break;
      case DWNL_SENDING   : index_state="Sending"               ; break;
      case DWNL_DOWNLOAD  : index_state="Downloading"           ; break;
      case DWNL_DONE      : index_state="Finished"              ; break;
      case DWNL_ERROR     : index_state="Error encountered"     ; break;
   }

   D.text(0, 0.9, S+"Server Index Download State: "+index_state);
   D.text(0, 0.8, S+"Local Files Before Patching: "+local_files.elms());
   if(C Pak *pak=patcher.index())
   {
      D.text(0, 0.7, S+"Server Files: "+pak->totalFiles());
   }
   D.text(0, 0.6, S+"Files Left To Download: "+patcher.filesLeft());
   D.text(0, 0.5, S+"Files Download Progress: "+patcher.progress()+" / "+patcher.filesSize());
   D.text(0, 0  , S+"Press Space to preview local data folder");
}
/******************************************************************************/
