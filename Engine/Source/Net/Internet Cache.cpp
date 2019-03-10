/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
#define COMPARE ComparePathCI
namespace EE{
/******************************************************************************/
static Str EatWWW(Str url) // convert "http://www.esenthel.com" -> "http://esenthel.com"
{
   Bool http =          StartsPath(url, "http://" ) ; if(http )url.remove(0, 7);
   Bool https=(!http && StartsPath(url, "https://")); if(https)url.remove(0, 8);
   if(Starts(url, "www."))url.remove(0, 4);
   if(https)url.insert(0, "https://");
   if(http )url.insert(0, "http://" );
   return url;
}
static Str ShortName(Str url) // convert "http://www.esenthel.com" -> "esenthel.com"
{
   if(StartsPath(url, "http://" ))url.remove(0, 7);else
   if(StartsPath(url, "https://"))url.remove(0, 8);
   if(Starts    (url, "www."    ))url.remove(0, 4);
   return url;
}
static void ImportImageFunc(InternetCache::ImportImage &ii, InternetCache &ic, Int thread_index=0)
{
   File *src=null, temp;
   if(ii.pf){if(temp.readTry(*ii.pf, ic._pak))src=&temp;}else 
   if(ii.ds)src=ii.ds->open(temp);
   if(src)
   {
      src->pos(0);
      ThreadMayUseGPUData();
      ii.image_temp.ImportTry(*src, -1, IMAGE_2D, ic._image_mip_maps);
      ThreadFinishedUsingGPUData();
   }
   if(src)src->pos(0); // reset position after loading
   ii.done=true; // !! don't do anything after this step because the object can get removed !!
}
static void ICUpdate(InternetCache &ic) {ic.update();}
/******************************************************************************/
void InternetCache::flush()
{
   if(_downloaded.elms() && _pak.pakFileName().is()) // we want to save data
   {
      if(_threads)REPA(_import_images)_threads->wait(_import_images[i], ImportImageFunc, T); // wait until the worker threads finish processing the importing, as they operate on Pak which we're about to update

      Str name=_pak.pakFileName(); Cipher *cipher=_pak._file_cipher; // copy in case load/update will delete this
     _pak.load(name, cipher); // reload the pak in case it was modified by another app/instance, in which case the 'PakUpdate' would make the file corrupt with out-dated info
      PakUpdate(_pak, SCAST(Memb<PakFileData>, _downloaded), name, cipher, _compress);
     _downloaded.del();
   }
}
void InternetCache::del()
{
   App._callbacks.exclude(ICUpdate, T);
   REPAO(_downloading).del();
   if(_threads)REPA(_import_images)_threads->cancel(_import_images[i], ImportImageFunc, T); // cancel importing
   flush();
   if(_threads)
   {
      REPA(_import_images)_threads->wait(_import_images[i], ImportImageFunc, T);
     _threads=null;
   }
  _downloaded   .del();
  _import_images.del();
  _to_download  .del();
  _verified     .del();
  _to_verify    .del();
  _pak          .del();
}
void InternetCache::create(C Str &db_name, Threads *threads, Cipher *cipher, COMPRESS_TYPE compress, Int image_mip_maps)
{
   del();

   if(D.canUseGPUDataOnSecondaryThread())_threads=threads; // setup worker threads only if we can operate on GPU on secondary threads
  _compress      =compress;
  _image_mip_maps=image_mip_maps;
   if(db_name.is())if(!_pak.load(db_name, cipher))
   {
      Mems<PakFileData> pfd; _pak.create(pfd, db_name, 0, cipher); // create an empty pak
   }
}
/******************************************************************************/
void InternetCache::changed(C Str &url)
{
   if(url.is())
   {
      Str name=ShortName(url); if(name.is())
      {
         Str slim=EatWWW(url);
        _verified.binaryExclude(slim, COMPARE);
         REPA(_downloading)if(EqualPath(_downloading[i].url(), slim))
         {
           _downloading[i].del();
           _to_download.binaryInclude(slim, COMPARE); enable(); // restart the download
            return;
         }
         if(_to_verify  .binaryHas(slim, COMPARE))return; // it will be checked
         if(_to_download.binaryHas(slim, COMPARE))return; // it will be downloaded
         if(ImagePtr().find(slim)) // download if currently referenced
         {
           _to_download.binaryInclude(slim, COMPARE); enable();
         }
      }
   }
}
Bool InternetCache::getFile(C Str &url, SrcFile &file)
{
   file.clear();
   if(!url.is())return true;
   Str name=ShortName(url); if(!name.is())return false;
   Str slim=   EatWWW(url);

   if(file.pf=_pak.find(name, false))
   {
      // if file was cached, then check if it was verified
                        if(_verified   .binaryHas    (slim, COMPARE))return true;
                        if(_to_download.binaryHas    (slim, COMPARE))return true;
      REPA(_downloading)if(EqualPath   (_downloading[i].url(), slim))return true;
                        if(_to_verify  .binaryInclude(slim, COMPARE))enable(); // verify
      return true;
   }
   REPA(_downloaded )if(EqualPath   (_downloaded [i].name , name)){file.ds=&_downloaded[i].data; return true;}
   REPA(_downloading)if(EqualPath   (_downloading[i].url(), slim))return false;
                     if(_to_download.binaryInclude(slim, COMPARE))enable();
   return false;
}
ImagePtr InternetCache::getImage(C Str &url)
{
   ImagePtr img; if(url.is())
   {
      Str slim=EatWWW(url); if(slim.is())if(!img.find(slim))
      {
         CACHE_MODE mode=Images.mode(CACHE_DUMMY); img=slim;
                         Images.mode(mode       );
         SrcFile file; if(getFile(url, file))
         {
            ImportImage &ii=_import_images.New();
            Swap<SrcFile>(ii, file);
            ii.image_ptr=img;
            import(ii);
            enable();
         }
      }
   }
   return img;
}
/******************************************************************************/
void InternetCache::import(ImportImage &ii)
{
   if(_threads)_threads->queue(ii, ImportImageFunc, T);else ImportImageFunc(ii, T);
}
Bool InternetCache::busy()C
{
   if(_to_download.elms() || _to_verify.elms() || _import_images.elms())return true;
   REPA(_downloading)if(_downloading[i].state()!=DWNL_NONE)return true;
   return false;
}
void InternetCache::enable()
{
   App._callbacks.include(ICUpdate, T);
}
void InternetCache::update()
{
   // update imported images
   REPA(_import_images)
   {
      ImportImage &ii=_import_images[i];
      if(ii.done)
      {
         Swap(*ii.image_ptr, ii.image_temp);
        _import_images.removeValid(i);
      }
   }

   // process downloaded data
   REPA(_downloading)
   {
      Download &down=_downloading[i];
      switch(down.state())
      {
         case DWNL_NONE:
         {
            if(_to_download.elms()){down.create(_to_download.last()                       ); _to_download.removeLast();}else
            if(_to_verify  .elms()){down.create(_to_verify  .last(), null, null, -1, -1, 0); _to_verify  .removeLast();} // use offset as -1 to encode special mode of verification
         }break;

         case DWNL_DONE: // finished downloading
         {
            Str name=ShortName(down.url());
            if(down.offset()<0) // if this was verification
            {
               if(C PakFile *pf=_pak.find(name))if(pf->data_size==down.totalSize() && pf->modify_time_utc==down.modifyTimeUTC()){_verified.binaryInclude(down.url(), COMPARE); down.del(); break;} // file is the same
               down.create(Str(down.url())); // it's different so download it fully, copy the 'url' because it might get deleted in the 'create'
            }else // move to 'downloaded'
            {
               Downloaded *downloaded=null; REPA(_downloaded)if(EqualPath(_downloaded[i].name, name)){downloaded=&_downloaded[i]; break;} if(!downloaded)downloaded=&_downloaded.New();
               downloaded->name=name;
               downloaded->file_data.setNum(down.size()).copyFrom((Byte*)down.data()); downloaded->data.set(downloaded->file_data.data(), downloaded->file_data.elms());
               downloaded->modify_time_utc=down.modifyTimeUTC();
               downloaded->compress_mode=(Compressable(GetExt(name)) ? COMPRESS_ENABLE : COMPRESS_DISABLE);
               ImagePtr img; if(img.find(down.url())) // reload image
               {
                  ImportImage &ii=_import_images.New();
                  ii.image_ptr=img;
                  ii.ds       =&downloaded->data;
                  import(ii);
               }
              _verified.binaryInclude(down.url(), COMPARE);
               down.del(); // delete as last
            }
         }break;

         case DWNL_ERROR: down.del(); break; // failed, so just ignore it
      }
   }

   if(busy())enable();
}
/******************************************************************************/
}
/******************************************************************************/
