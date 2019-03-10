/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   TODO: implement in the Editor Pak Generation / Publishing:
      Caching of compressed files for future re-use, perhaps in the Project's "Temp" folder, with user permission?
      Or better reuse existing PAK's, save to new and rename to old on finish.

/******************************************************************************

   'Pak._file_name' should always include normalized full path

   'PakSet._lock' potentially could be a 'SimpleReadWriteSync', however tests show that 1 thread (most likely scenario) is faster with 'SyncLock'

   Test Results for doing simplified version of 'Paks.find' on multiple threads (in operations per second)
                 SyncLock  SimpleReadWriteSync
      1 Thread :    50             33
      2 Threads:    34             74
      3 Threads:    34             90

/******************************************************************************/
#define CC4_PAK CC4('P', 'A', 'K', 0)
/******************************************************************************/
PakSet Paks;
/******************************************************************************/
File* DataSource::open(File &temp)C
{
   switch(type)
   {
      default      :                                                             return  null;
      case FILE    :                                                             return  file;
      case PAK_FILE: if(pak_file && pak && temp.readTry   (*pak_file, *pak     ))return &temp; return null; // have to return null on fail
      case NAME    : if(                   temp.readTry   ( name               ))return &temp; return null; // have to return null on fail
      case STD     : if(                   temp.readStdTry( name               ))return &temp; return null; // have to return null on fail
      case MEM     :                       temp.readMem   ( memory, memory_size);return &temp;
   }
}
File* DataSource::openRaw(File &temp)C
{
   switch(type)
   {
      default      :                                                             return  null;
      case FILE    :                                                             return  file;
      case PAK_FILE: if(pak_file && pak && temp.readTryRaw(*pak_file, *pak     ))return &temp; return null; // have to return null on fail
      case NAME    : if(                   temp.readTry   ( name               ))return &temp; return null; // have to return null on fail
      case STD     : if(                   temp.readStdTry( name               ))return &temp; return null; // have to return null on fail
      case MEM     :                       temp.readMem   ( memory, memory_size);return &temp;
   }
}
Long DataSource::size()C
{
   switch(type)
   {
      default      : return 0;
      case FILE    : return                  file ?     file->     size() :  0;
      case PAK_FILE: return              pak_file ? pak_file->data_size   :  0;
      case NAME    : {FileInfo       fi(name); return fi.type ? fi.size   : -1;} // have to return -1 on error
      case STD     : {FileInfoSystem fi(name); return fi.type ? fi.size   : -1;} // have to return -1 on error
      case MEM     : return                                 memory_size;
   }
}
Long DataSource::sizeCompressed()C
{
   switch(type)
   {
      default      : return 0;
      case FILE    : return                  file ?     file->     size()          :  0;
      case PAK_FILE: return              pak_file ? pak_file->data_size_compressed :  0;
      case NAME    : {FileInfo       fi(name); return fi.type ? fi.size            : -1;} // have to return -1 on error
      case STD     : {FileInfoSystem fi(name); return fi.type ? fi.size            : -1;} // have to return -1 on error
      case MEM     : return                                 memory_size;
   }
}
Str DataSource::srcName()C
{
   switch(type)
   {
      case PAK_FILE: if(pak && pak_file)return pak->fullName(*pak_file); break;

      case NAME:
      case STD : return name;
   }
   return S;
}
/******************************************************************************/
Bool PakProgress::wantStop(Str *error_message)C
{
   if(stop)
   {
      if(error_message)*error_message="Stopped on user request";
      return true;
   }
   return false;
}
/******************************************************************************/
PakFile& PakFile::reset()
{
   // !! if a new member is added to 'PakFile' then it must be set in 'PakCreator.create' !!
   name=null;
   flag=0;
   compression=COMPRESS_NONE;
   parent=-1;
   children_offset=children_num=0;
   data_offset=0;
   data_size=data_size_compressed=0;
   data_xxHash64_32=0;
   modify_time_utc.zero();
   return T;
}
PakFile& PakFile::type(FSTD_TYPE type)
{
   FlagSet(flag, PF_STD_DIR , type==FSTD_DIR );
   FlagSet(flag, PF_STD_LINK, type==FSTD_LINK);
   return T;
}
/******************************************************************************/
// PAK
/******************************************************************************/
void Pak::zero()
{
   // !! if a new member is added to Pak then it must be set in PakCreator.create !!
  _root_files        =0;
  _data_offset       =0;
  _cipher_per_file   =false;
  _file_type         =0;
  _file_cipher_offset=0;
  _file_cipher       =null;
  _data              =null;
}
Pak& Pak::del()
{
  _files            .del();
  _names            .del();
  _file_name        .del();
  _data_decompressed.del();
   zero(); return T;
}
Pak::Pak() {zero();}
/******************************************************************************/
#pragma pack(push, 1) // use 1 because there are usually many files and any useless bytes contribute to bigger PAK headers, also we need the PAK headers hashes to always be the same, while gaps between members could result in undefined values which could make the hashes different
struct PakFile4
{
   Byte          flag                ;
   COMPRESS_TYPE compression         ;
   Int           name_offset         ,
                 parent              ,
                 children_offset     ,
                 children_num        ;
   ULong         data_offset         ;
   UInt          data_size           ,
                 data_size_compressed,
                 data_xxHash64_32    ;
   Byte          second              ,
                 minute              ,
                 hour                ,
                 day                 ,
                 month               ;
   U16           year                ;
};
struct PakFile3
{
   Byte     flag                ;
   Int      name_offset         ,
            parent              ,
            children_offset     ,
            children_num        ;
   ULong    data_offset         ;
   UInt     data_size           ,
            data_size_compressed,
            data_xxHash32       ;
   DateTime modify_time_utc     ;
};
#pragma pack(pop)
#pragma pack(push, 4)
struct PakFile2
{
   Byte     flag                ;
   Int      name_offset         ,
            parent              ,
            children_offset     ,
            children_num        ;
   ULong    data_offset         ;
   UInt     data_size           ,
            data_size_compressed,
            data_crc32          ;
   DateTime modify_time_utc     ;
};
struct PakFile1
{
   Byte     flag           ;
   Int      name_offset    ,
            parent         ,
            children_offset,
            children_num   ;
   ULong    data_offset    ,
            data_size      ;
   DateTime modify_time_utc;
};
struct PakFile0
{
   Int      name_offset    ,
            parent         ,
            children_offset,
            children_num   ,
            data_offset    ,
            data_size      ;
   DateTime modify_time_utc;
};
#pragma pack(pop)
Byte GetOldFlag(Byte flag)
{
   return FlagTest(flag, 1<<1)*PF_REMOVED
        | FlagTest(flag, 1<<2)*PF_STD_DIR
      //| FlagTest(flag, 1<<3)*PF_NO_COMPRESS
        | FlagTest(flag, 1<<4)*PF_STD_LINK;
}
/******************************************************************************/
Bool Pak::saveHeader(File &f)C
{
   Memt<PakFile4> filei; filei.setNum(totalFiles());
   FREPA(filei)
   {
      PakFile4 &dest=filei[i];
    C PakFile  &src =file (i);

     _Unaligned(dest.name_offset         , src.name-_names.data()    );
      Unaligned(dest.flag                , src.flag                  );
      Unaligned(dest.compression         , src.compression           );
      Unaligned(dest.parent              , src.parent                );
      Unaligned(dest.children_offset     , src.children_offset       );
      Unaligned(dest.children_num        , src.children_num          );
      Unaligned(dest.data_offset         , src.data_offset           );
      Unaligned(dest.data_size           , src.data_size             );
      Unaligned(dest.data_size_compressed, src.data_size_compressed  );
      Unaligned(dest.data_xxHash64_32    , src.data_xxHash64_32      );
      Unaligned(dest.second              , src.modify_time_utc.second);
      Unaligned(dest.minute              , src.modify_time_utc.minute);
      Unaligned(dest.hour                , src.modify_time_utc.hour  );
      Unaligned(dest.day                 , src.modify_time_utc.day   );
      Unaligned(dest.month               , src.modify_time_utc.month );
     _Unaligned(dest.year                , src.modify_time_utc.year  );
   }

   f.putUInt (CC4_PAK); // CC4
   f.cmpUIntV(      4); // version
   f.cmpUIntV(_root_files);
   if(_names.saveRaw(f))
   if( filei.saveRaw(f))
      return f.ok();
   return false;
}
PAK_LOAD Pak::loadHeader(File &f, Long *expected_size, Long *actual_size)
{
   if(expected_size)*expected_size=0;
   if(  actual_size)*  actual_size=0;

   del();

   PAK_LOAD result=PAK_LOAD_INCOMPLETE_HEADER;
   ULong    data_size=0;
   Bool     fix_compressed=false;

   UInt cc4=f.getUInt();
   if(  cc4==CC4_PAK
     || cc4==CC4('P', 'A', 'K', 1) // don't remove this as there are still some *.EsenthelProject files using it
     )switch(f.decUIntV()) // version
   {
      default: result=PAK_LOAD_UNSUPPORTED_VERSION; break;

      case 4:
      {
         // main
         f.decUIntV(_root_files);

         // names
         if(_names.loadRaw(f))
         {
            // files
            Memt<PakFile4> filei; if(filei.loadRaw(f))
            {
              _files.setNum(filei.elms()); REPA(_files)
               {
                  PakFile  &dest=_files[i];
                C PakFile4 &src = filei[i];
                            dest.name                  =_names.data()+Unaligned(src.name_offset);
                  Unaligned(dest.flag                  , src.flag                );
                  Unaligned(dest.compression           , src.compression         );
                  Unaligned(dest.parent                , src.parent              );
                  Unaligned(dest.children_offset       , src.children_offset     );
                  Unaligned(dest.children_num          , src.children_num        );
                  Unaligned(dest.data_offset           , src.data_offset         );
                  Unaligned(dest.data_size             , src.data_size           );
                  Unaligned(dest.data_size_compressed  , src.data_size_compressed);
                  Unaligned(dest.data_xxHash64_32      , src.data_xxHash64_32    );
                  Unaligned(dest.modify_time_utc.second, src.second              );
                  Unaligned(dest.modify_time_utc.minute, src.minute              );
                  Unaligned(dest.modify_time_utc.hour  , src.hour                );
                  Unaligned(dest.modify_time_utc.day   , src.day                 );
                  Unaligned(dest.modify_time_utc.month , src.month               );
                 _Unaligned(dest.modify_time_utc.year  , src.year                );

                  MAX(data_size, dest.data_offset+dest.data_size_compressed);
               }

              _cipher_per_file=true;
               goto ok;
            }
         }
      }break;

      case 3:
      {
         // main
         f>>_root_files;

         // names
         if(_names._loadRaw(f))
         {
            // files
            Memt<PakFile3> filei; if(filei._loadRaw(f))
            {
              _files.setNum(filei.elms()); REPA(_files)
               {
                  PakFile  &dest=_files[i];
                C PakFile3 &src = filei[i];
                            dest.name                =_names.data()+Unaligned(src.name_offset);
                  Unaligned(dest.flag                , GetOldFlag(src.flag)    );
                  Unaligned(dest.compression         , COMPRESS_NONE           );
                  Unaligned(dest.parent              , src.parent              );
                  Unaligned(dest.children_offset     , src.children_offset     );
                  Unaligned(dest.children_num        , src.children_num        );
                  Unaligned(dest.data_offset         , src.data_offset         );
                  Unaligned(dest.data_size           , src.data_size           );
                  Unaligned(dest.data_size_compressed, src.data_size_compressed);
                 _Unaligned(dest.data_xxHash64_32    ,                        0); // src.data_xxHash32 - this version used xxHash32
                  Unaligned(dest.modify_time_utc     , src.modify_time_utc     );

                  MAX(data_size, dest.data_offset+dest.data_size_compressed);
               }

              _cipher_per_file=true;
               fix_compressed=true;
               goto ok;
            }
         }
      }break;

      case 2:
      {
         // main
         f>>_root_files;

         // names
         if(_names._loadRaw(f))
         {
            // files
            Memt<PakFile2> filei; if(filei._loadRaw(f))
            {
              _files.setNum(filei.elms()); REPA(_files)
               {
                  PakFile  &dest=_files[i];
                C PakFile2 &src = filei[i];
                  dest.name                =_names.data()+src.name_offset         ;
                  dest.flag                =   GetOldFlag(src.flag               );
                  dest.compression         =                         COMPRESS_NONE;
                  dest.parent              =              src.parent              ;
                  dest.children_offset     =              src.children_offset     ;
                  dest.children_num        =              src.children_num        ;
                  dest.data_offset         =              src.data_offset         ;
                  dest.data_size           =              src.data_size           ;
                  dest.data_size_compressed=              src.data_size_compressed;
                  dest.data_xxHash64_32    =                                     0; // src.data_crc32 - this version used CRC32
                  dest.modify_time_utc     =              src.modify_time_utc     ;

                  MAX(data_size, dest.data_offset+dest.data_size_compressed);
               }

              _cipher_per_file=false;
               fix_compressed=true;
               goto ok;
            }
         }
      }break;

      case 1:
      {
         // main
         Int files=0;
         f>>_root_files>>files;

         // names
         if(_names._loadRaw(f))
         {
            // files
                                 _files.setNum(files);
            Memt<PakFile1> filei; filei.setNum(files); if(f.getN(filei.data(), filei.elms()))
            {
               REPA(_files)
               {
                  PakFile  &dest=_files[i];
                  PakFile1 &src = filei[i];
                  dest.name                =_names.data()+src.name_offset    ;
                  dest.flag                =   GetOldFlag(src.flag          );
                  dest.compression         =                    COMPRESS_NONE;
                  dest.parent              =              src.parent         ;
                  dest.children_offset     =              src.children_offset;
                  dest.children_num        =              src.children_num   ;
                  dest.data_offset         =              src.data_offset    ;
                  dest.data_size           =              src.data_size      ;
                  dest.data_size_compressed=              src.data_size      ;
                  dest.data_xxHash64_32    =                                0;
                  dest.modify_time_utc     =              src.modify_time_utc;

                  MAX(data_size, dest.data_offset+dest.data_size_compressed);
               }

              _cipher_per_file=false;
               fix_compressed=true;
               goto ok;
            }
         }
      }break;

      case 0:
      {
         // main
         Int files=0;
         f>>_root_files>>files; UInt dat_ofs=f.getUInt(); f.skip(4); // 4=data_size

         // names
         if(_names._loadRaw(f))
         {
            // files
                                 _files.setNum(files);
            Memt<PakFile0> filei; filei.setNum(files); if(f.getN(filei.data(), filei.elms()))
            {
               REPA(_files)
               {
                  PakFile  &dest=_files[i];
                  PakFile0 &src = filei[i];
                  dest.name                =_names.data()+src.name_offset    ;
                  dest.flag                =                                0;
                  dest.compression         =                    COMPRESS_NONE;
                  dest.parent              =              src.parent         ;
                  dest.children_offset     =              src.children_offset;
                  dest.children_num        =              src.children_num   ;
                  dest.data_offset         =       Max(0, src.data_offset-dat_ofs); // old 'data_offset' already included Pak data_offset
                  dest.data_size           =              src.data_size      ;
                  dest.data_size_compressed=              src.data_size      ;
                  dest.data_xxHash64_32    =                                0;
                  dest.modify_time_utc     =              src.modify_time_utc;

                  MAX(data_size, dest.data_offset+dest.data_size_compressed);
               }

              _cipher_per_file=false;
               fix_compressed=true;
               goto ok;
            }
         }
      }break;
   }else result=PAK_LOAD_NOT_PAK;

   del(); return result;

ok:
  _file_type         =f._type;
  _file_cipher_offset=f._cipher_offset+f.pos(); // use existing cipher offset adjusted by current position where data starts ('pos' and not 'posAbs')
  _file_cipher       =f._cipher; if(!_file_cipher)_cipher_per_file=true; // if there's no cipher at all, then force '_cipher_per_file' because it speeds up '_cipher_offset' calculations in files, it's not going to be needed anyway
  _data_offset       =f.posAbs();
   data_size        +=f.pos   (); // this needs to be 'pos' and not 'posAbs'

   if(fix_compressed) // old versions stored compressed files with an extra header per file
   {
      Long pos=f.pos(); FREPA(_files)
      {
         PakFile &pf=_files[i]; if(pf.data_size!=pf.data_size_compressed)
         {
            Long p=_data_offset+pf.data_offset;
            COMPRESS_TYPE compress; ULong compressed_size, decompressed_size;
            if((f._cipher && _cipher_per_file) // this can't work
            || !f.pos(p)
            || !DecompressHeader(f, compress, compressed_size, decompressed_size)){del(); return PAK_LOAD_UNSUPPORTED_VERSION;}
            pf.compression         =  compress       ;
            pf.data_size_compressed=  compressed_size;
            pf.data_size           =decompressed_size;
            pf.data_offset        +=f.pos()-p;
         }
      }
      f.pos(pos);
   }

   if(expected_size)*expected_size=data_size;
   if(  actual_size)*  actual_size= f.size();
   return (data_size>f.size()) ? PAK_LOAD_INCOMPLETE_DATA : PAK_LOAD_OK;
}
PAK_LOAD Pak::loadMemEx(CPtr data, Int size, Cipher *cipher, Long *expected_size, Long *actual_size)
{
   File f; f.readMem(data, size, cipher);
   PAK_LOAD result=loadHeader(f, expected_size, actual_size);
   switch(  result)
   {
      case PAK_LOAD_INCOMPLETE_DATA:
      case PAK_LOAD_OK             : T._data=data; return result;
   }
 //del(); no need to call because 'loadHeader' does that already
   return result;
}
PAK_LOAD Pak::loadEx(C Str &name, Cipher *cipher, Long pak_offset, Long *expected_size, Long *actual_size)
{
   if(expected_size)*expected_size=0;
   if(  actual_size)*  actual_size=0;

   PAK_LOAD result;
   File f; Bool processed; if(f.readTryEx(name, cipher, &processed))
   {
      if(processed) // if the Pak container had to be processed, then we need to copy it to a memory buffer and keep that memory for further usage
      {
         f.cipher(null); // we have to disable file cipher, because Pak may have cipher_per_file, in which case we need to handle cipher for each file separately
         Mems<Byte> data; // must use temporary memory, because in 'loadMem->loadHeader' first is called 'del' which would delete '_data_decompressed'
         if(!f.copyToAndDiscard(data))result=PAK_LOAD_NOT_PAK;else
         switch(result=loadMemEx(data.data(), data.elms(), cipher, expected_size, actual_size)) // have to use 'cipher' here, because it was disabled earlier
         {
            case PAK_LOAD_INCOMPLETE_DATA:
            case PAK_LOAD_OK             : Swap(_data_decompressed, data); return result; // remember this data, because it will be used when reading files from this Pak (do not copy it, but just swap it, because 'loadMem' keeps pointer to existing data, and also swapping is faster than copying)
         }
      }else
      {
         if(pak_offset) // this is used only for Linux when embedding PAK's into the executable, in that case 'processed' is always false and encryption starts at current position
         {
            if(!f.pos(pak_offset))goto error;
            f.cipherOffsetClear();
         }
         switch(result=loadHeader(f, expected_size, actual_size))
         {
            case PAK_LOAD_INCOMPLETE_DATA:
            case PAK_LOAD_OK             :
            {
              _file_name=(f._pak ? f._pak->_file_name : NormalizePath(MakeFullPath(name, f._path))); // if file comes from another Pak then remember that Pak's name, otherwise remember full path in case the path is relative and 'CurDir' or 'DataPath' will be changed later making that relative path no longer valid
               if(_file_cipher && !cipher)_cipher_per_file=false; // if there is a cipher, but it comes from parent file/container and not this Pak, then we always have to disable '_cipher_per_file', because that cipher was used to encrypt entire Pak as a one file, so when decrypting, we have to treat it the same, that the cipher affects entire Pak and its content continuously and not per Pak's files, here the Pak is one big file
            }return result;
         }
      }
   }else
   {
   error:
      result=PAK_LOAD_NOT_FOUND;
   }
   del(); return result;
}
Bool Pak::loadMem  (CPtr   data, Int size, Cipher *cipher) {return loadMemEx(data, size, cipher)==PAK_LOAD_OK;}
Bool Pak::load     (C Str &name,           Cipher *cipher) {return loadEx   (name,       cipher)==PAK_LOAD_OK;}
void Pak::operator=(C Str &name                          ) {   if(!load     (name))Exit(S+"Can't load Pak \""+name+"\"");}
/******************************************************************************/
Pak& Pak::pakFileName(C Str &name) {T._file_name=NormalizePath(MakeFullPath(name)); return T;}

Str Pak::fullName(C PakFile &file)C
{
   Str    s=file.name; for(Int p=file.parent; InRange(p, T.files()); p=T.file(p).parent)s=S+T.file(p).name+'\\'+s;
   return s;
}
Long Pak::totalSize(C PakFile &file, Bool compressed)C
{
   Long   size=(compressed ? file.data_size_compressed : file.data_size); REP(file.children_num)size+=totalSize(file.children_offset+i, compressed);
   return size;
}
Long PakSet::totalSize(C PaksFile &file, Bool compressed)C
{
   Long   size=(file.file ? compressed ? file.file->data_size_compressed : file.file->data_size : 0); REP(file.children_num)size+=totalSize(file.children_offset+i, compressed);
   return size;
}
Str PaksFile::fullName(                )C {return (pak && file)       ?    pak->fullName(*file   ) : S;}
Str Pak     ::fullName(  Int       i   )C {return InRange(i, files()) ?         fullName( file(i)) : S;}
Str PakSet  ::fullName(  Int       i   )C {return InRange(i, files()) ? file(i).fullName(        ) : S;}
Str PakSet  ::fullName(C PaksFile &file)C {return file.fullName();}

Long Pak   ::totalSize(Int i, Bool compressed)C {return InRange(i, files()) ? totalSize(file(i), compressed) : 0;}
Long PakSet::totalSize(Int i, Bool compressed)C {return InRange(i, files()) ? totalSize(file(i), compressed) : 0;}
/******************************************************************************/
static C PakFile* PakFind(C PakFile *file, CChar8 *name, Int from, Int elms)
{
   for(Int l=from, r=from+elms; l<r; )
   {
      Int mid    =UInt(l+r)/2,
          compare=Compare(name, file[mid].name);
      if(!compare  )return     &file[mid];
      if( compare<0)r=mid;
      else          l=mid+1;
   }
   return null;
}
static C PakFile* PakFind(C PakFile *file, CChar *name, Int from, Int elms)
{
   for(Int l=from, r=from+elms; l<r; )
   {
      Int mid    =UInt(l+r)/2,
          compare=Compare(name, file[mid].name);
      if(!compare  )return     &file[mid];
      if( compare<0)r=mid;
      else          l=mid+1;
   }
   return null;
}
static C PaksFile* PakFind(C PaksFile *file, CChar8 *name, Int from, Int elms)
{
   for(Int l=from, r=from+elms; l<r; )
   {
      Int mid    =UInt(l+r)/2,
          compare=Compare(name, file[mid].file->name);
      if(!compare  )return     &file[mid];
      if( compare<0)r=mid;
      else          l=mid+1;
   }
   return null;
}
static C PaksFile* PakFind(C PaksFile *file, CChar *name, Int from, Int elms)
{
   for(Int l=from, r=from+elms; l<r; )
   {
      Int mid    =UInt(l+r)/2,
          compare=Compare(name, file[mid].file->name);
      if(!compare  )return     &file[mid];
      if( compare<0)r=mid;
      else          l=mid+1;
   }
   return null;
}
/******************************************************************************/
C PakFile* Pak::find(CChar8 *name, Bool include_removed)C
{
   if(Is(name) && rootFiles())
   {
      Bool  last  =false;
      Char8 pf_name[MAX_LONG_PATH];
      Int   pf_pos=0,
            pf_len=rootFiles();
      for(;;)
      {
         for(Int i=0; ; i++)
         {
            if(!InRange(i, pf_name))return null; // name too long
            Char8 c=*name++; if(!c)last=true;else if(IsSlash(c))c=0;
            if(!(pf_name[i]=c))break;
         }
       C PakFile *file=PakFind(files().data(), pf_name, pf_pos, pf_len);
         if(!file)return null;
         if( last)
         {
            if((file->flag&PF_REMOVED) && !include_removed)return null; // PF_REMOVED can be checked only for the last file, because if any parent is marked with PF_REMOVED during 'PakUpdate', then all its children are removed
            return file;
         }
         pf_pos=file->children_offset;
         pf_len=file->children_num;
      }
   }
   return null;
}
C PakFile* Pak::find(CChar *name, Bool include_removed)C
{
   if(Is(name) && rootFiles())
   {
      Bool last  =false;
      Char pf_name[MAX_LONG_PATH];
      Int  pf_pos=0,
           pf_len=rootFiles();
      for(;;)
      {
         for(Int i=0; ; i++)
         {
            if(!InRange(i, pf_name))return null; // name too long
            Char c=*name++; if(!c)last=true;else if(IsSlash(c))c=0;
            if(!(pf_name[i]=c))break;
         }
       C PakFile *file=PakFind(files().data(), pf_name, pf_pos, pf_len);
         if(!file)return null;
         if( last)
         {
            if((file->flag&PF_REMOVED) && !include_removed)return null; // PF_REMOVED can be checked only for the last file, because if any parent is marked with PF_REMOVED during 'PakUpdate', then all its children are removed
            return file;
         }
         pf_pos=file->children_offset;
         pf_len=file->children_num;
      }
   }
   return null;
}
C PakFile* Pak::find(C UID &id, Bool include_removed)C
{
   if(id.valid())
      if(C PakFile *file=PakFind(files().data(), _EncodeFileName(id), 0, rootFiles()))
   {
      if((file->flag&PF_REMOVED) && !include_removed)return null;
      return file;
   }
   return null;
}
C PaksFile* PakSet::find(CChar8 *name)C
{
   if(Is(name) && rootFiles())
   {
      Bool  last  =false;
      Char8 pf_name[MAX_LONG_PATH];
      Int   pf_pos=0,
            pf_len=rootFiles();
      for(;;)
      {
         for(Int i=0; ; i++)
         {
            if(!InRange(i, pf_name))return null; // name too long
            Char8 c=*name++; if(!c)last=true;else if(IsSlash(c))c=0;
            if(!(pf_name[i]=c))break;
         }
       C PaksFile *file=PakFind(files().data(), pf_name, pf_pos, pf_len);
         if(!file)return null;
         if( last)return file;
         pf_pos=file->children_offset;
         pf_len=file->children_num;
      }
   }
   return null;
}
C PaksFile* PakSet::find(CChar *name)C
{
   if(Is(name) && rootFiles())
   {
      Bool last  =false;
      Char pf_name[MAX_LONG_PATH];
      Int  pf_pos=0,
           pf_len=rootFiles();
      for(;;)
      {
         for(Int i=0; ; i++)
         {
            if(!InRange(i, pf_name))return null; // name too long
            Char c=*name++; if(!c)last=true;else if(IsSlash(c))c=0;
            if(!(pf_name[i]=c))break;
         }
       C PaksFile *file=PakFind(files().data(), pf_name, pf_pos, pf_len);
         if(!file)return null;
         if( last)return file;
         pf_pos=file->children_offset;
         pf_len=file->children_num;
      }
   }
   return null;
}
C PaksFile* PakSet::find(C UID &id)C
{
   return id.valid() ? PakFind(files().data(), _EncodeFileName(id), 0, rootFiles()) : null;
}
/******************************************************************************/
// PAK SET
/******************************************************************************/
static Int ComparePath(C PaksFile &a, C PaksFile &b) {return Compare(a.file->name, b.file->name);}

struct ChildrenInfo
{
   Int children_offset,
       children_num;

   void set(Int offset, Int num) {children_offset=offset; children_num=num;}
};
struct PakChildrenInfo : ChildrenInfo
{
   Int pak;

   void set(Int pak, Int offset, Int num) {T.pak=pak; children_offset=offset; children_num=num;}
};
struct PaksCreator
{
   Int            root_files; // number of root files
   Mems<Pak*>     paks;       // lookup table for the Pak's
   Memc<PaksFile> files,      // continuously increased file array
                  temp;       // temporary container stored globally to improve performance of memory usage

   void add(Mems<ChildrenInfo> &pak_children, Int parent) // 'pak_children' is an array of length equal to number of Pak's, in each 'add' it will be modified to have a list of what files from paks to add
   {
      // init
      Memc<PaksFile>                level; // files to be added in this level
      Memc< Memc<PakChildrenInfo> > level_children;

      // set list of files in this level
      FREPAD(p, paks) // for each Pak (order important, with every new Pak we're replacing old files)
      {
         Pak          &pak=*paks        [p]; // get p-th Pak
         ChildrenInfo &pc = pak_children[p]; // get requested children of that Pak to be added in this level
         FREPD(f, pc.children_num) // for each file (update or add)
         {
          C PakFile  &src=pak.file(pc.children_offset+f);
            PaksFile *get=ConstCast(PakFind(level.data(), src.name, 0, level.elms())); // find existing one
            if(src.flag&PF_REMOVED)
            {
               level.removeData(get, true); // remove any previous information about the file (keep order as 'level' needs to be sorted)
            }else
            {
               if(!get)get=&temp.New(); // add new file
               get->pak =&pak;
               get->file=&src;
               if(src.children_num) // if the file has children, we need to store information about them, to add them later
               {
                  // here 'PaksFile.children_offset' is used to store information about children in the 'level_children' container (storing index in 'level_children'+1)
                  if(            get->children_offset<=0)get->children_offset=level_children.addNum(1)+1; // if no children info was set yet, then create new and store index+1
                  level_children[get->children_offset-1].New().set(p, src.children_offset, src.children_num);
               }
            }
         }
         if(temp.elms()) // add files from 'temp' to 'level'
         {
            FREPA(temp)level.add(temp[i]); temp.clear();
            level.sort(ComparePath); // 'level' needs to be sorted
         }
      }

      Int cur_files=files.elms(); // store the amount of files before adding those from this level, this also marks the starting index of files in this level

      // update parent
      if(parent<0)root_files=level.elms();else
      {
         PaksFile       &pf=files[parent];
         pf.children_offset=cur_files;
         pf.children_num   =level.elms();
      }

      // add files from this 'level' to 'files' list
      FREPA(level)files.add(level[i]);

      // add children
      FREPAD(f, level) // for each file that was added in this level
      {
         Int c=level[f].children_offset;
         if( c>0) // check if it has information about children in the 'level_children' container
         {
            Memc<PakChildrenInfo> &children=level_children[c-1]; // access it, index was stored as +1 so we need to use -1 here
            REPAO(pak_children).set(0, 0); FREPA(children){PakChildrenInfo &c=children[i]; pak_children[c.pak]=c;} // setup the 'pak_children' array
            add  (pak_children, cur_files+f);
         }
      }
   }

   PaksCreator(Meml<PakSet::Src> &paks)
   {
      root_files=0;
      T.paks.setNum(paks.elms()); REPAO(T.paks)=&paks[i];
   }
};
/******************************************************************************/
PakSet& PakSet::del()
{
  _root_files=0;
  _files.del();
  _paks .del();
   return T;
}
PakSet::PakSet()
{
  _root_files=0;
}
void PakSet::rebuild()
{
   SyncLocker locker(_lock);

   // init
   PaksCreator pc(_paks);

   // add
   Mems<ChildrenInfo> pak_children; pak_children.setNum(pc.paks.elms());
   REPAO (pak_children).set(0, pc.paks[i]->rootFiles()); // set to add all root files from each Pak in the next 'add'
   pc.add(pak_children, -1);

   // create new
   _root_files=pc.root_files;
   Swap(_files, pc.files);
}
/******************************************************************************/
Bool    PakSet::addTry   (C Str &name,           Cipher *cipher, Bool auto_rebuild                 ) {return addTry(name, cipher, auto_rebuild, 0);}
Bool    PakSet::addTry   (C Str &name,           Cipher *cipher, Bool auto_rebuild, Long pak_offset) {Src temp; if(temp.loadEx    (name      , cipher              , pak_offset)==PAK_LOAD_OK){temp.name=name; temp.pak_offset=pak_offset  ; SyncLocker locker(_lock); Swap(temp, _paks.New()); if(auto_rebuild)T.rebuild(); return true;} return false;}
Bool    PakSet::addMemTry( CPtr  data, Int size, Cipher *cipher, Bool auto_rebuild                 ) {Src temp; if(temp.loadMem   (data, size, cipher                          )             ){                temp.pak_offset=IntPtr(data); SyncLocker locker(_lock); Swap(temp, _paks.New()); if(auto_rebuild)T.rebuild(); return true;} return false;}
PakSet& PakSet::add      (C Str &name,           Cipher *cipher, Bool auto_rebuild                 ) {          if(     !addTry   (name,       cipher, auto_rebuild            )             )Exit(S+"Can't load Pak \""+name+'"'); return T;}
PakSet& PakSet::addMem   ( CPtr  data, Int size, Cipher *cipher, Bool auto_rebuild                 ) {          if(     !addMemTry(data, size, cipher, auto_rebuild            )             )Exit(S+"Can't load Pak from memory"); return T;}

Bool PakSet::remove(C Str &name/*, Long pak_offset*/) // keep 'pak_offset' in case we make this functionality available in the future
{
   if(name.is())
   #if !SYNC_LOCK_SAFE
      if(_paks.elms())
   #endif
   {
      SyncLocker locker(_lock);
      MREP(_paks)
      {
       C Src &src=_paks[i]; if(EqualPath(src.name, name)/* && src.pak_offset==pak_offset*/)
         {
           _paks.remove(i, true);
            // !! here can't access 'i' MemlNode anymore after deletion !!
            rebuild();
            return true;
         }
      }
   }
   return false;
}
Bool PakSet::removeMem(CPtr data)
{
   if(data)
   #if !SYNC_LOCK_SAFE
      if(_paks.elms())
   #endif
   {
      SyncLocker locker(_lock);
      MREP(_paks)
      {
       C Src &src=_paks[i]; if(!src.name.is() && src.pak_offset==IntPtr(data))
         {
           _paks.remove(i, true);
            // !! here can't access 'i' MemlNode anymore after deletion !!
            rebuild();
            return true;
         }
      }
   }
   return false;
}
/******************************************************************************/
// PAK - Create
/******************************************************************************/
static UInt DecompressedFilexxHash64_32(File &file, File &temp, COMPRESS_TYPE compression, Long compressed_size, Long decompressed_size) // !! this assumes that "temp.writeMem()" was called !!
{
   file.pos(0); // reset file position
   if(!compression)return file.xxHash64_32();
   xxHash64Calc hasher; UInt hash=(DecompressRaw(file, temp, compression, compressed_size, decompressed_size, false, &hasher) ? hasher.hash.hash32() : 0); // don't use memory optimization here, because we care more about fewer memory allocations
   temp.reset(); // reset to free memory, this is needed as 'f_temp' is always expected to be empty
   return hash;
}
/******************************************************************************/
struct FileTemp
{
   enum TYPE : Byte
   {
      NODE,
      STD ,
   };
   Str  name; // for STD this will include full path (including drive and folders), for NODE it will be set to 'node.name'
   TYPE type;
   union
   {
    C PakNode *node; // used when "type==NODE"
      FileInfo fi  ; // used when "type==STD"
   };

   Bool isDir()C
   {
      FSTD_TYPE type; switch(T.type)
      {
         case NODE: type=node->type; break;
         case STD : type=fi   .type; break;
         default  : type=FSTD_NONE ; break;
      }
      return type==FSTD_DRIVE || type==FSTD_DIR;
   }
   void set(C PakNode &node)
   {
      T.name= node.name;
      T.type= NODE;
      T.node=&node;
   }
   void set(C Str &name, C FileInfo &fi)
   {
      T.name=name;
      T.type=STD;
      T.fi  =fi;
   }
   FileTemp() {} // needed because of union
};
static Int ComparePath(C FileTemp &f0, C FileTemp &f1) {return ComparePath(f0.name, f1.name);}
/******************************************************************************/
struct PakCreator
{
   struct FileTempContainer
   {
      Memc<FileTemp> files;

      void add(C PakNode &node                                                      ) {                                                                                         files.New().set(node    );}
      void add(Str name, C FileInfo &fi, Bool (*filter)(C Str &name)                ) {name.tailSlash(false); if(!filter || filter(name))if(!Equal(_GetBase(name), ".DS_Store"))files.New().set(name, fi);}
      Bool add(Str name,                 Bool (*filter)(C Str &name), PakCreator &pc) {name.tailSlash(false); if(!filter || filter(name)){FileInfo fi; if(!fi.get(name))return pc.setErrorAccess(name); add(name, fi, null);} return true;}

      void sort()
      {
         files.sort(ComparePath);
         REPA(files)if(i && EqualPath(files[i].name, files[i-1].name))files.remove(i, true); // remove files with same names (in case someone provides incorrect input)
      }
   };

   STRUCT(PakFileEx , PakFile)
   //{
      Bool          ready;
      COMPRESS_MODE compress_mode;
      Str           name,
                    data_name; // can come from STD
    C DataSource   *data     ; // can come from NODE
      File          processed; // this is processed file data (that was decompressed/compressed and ready to use)
      // !! clear values in 'set' instead of constructor !!

      Bool needHash      ()C {return data_xxHash64_32==0                      && data_size>                     0;} // we need hash only if it wasn't set yet, if hash is already set, then we don't need it
      Bool needCompress  ()C {return    compress_mode==COMPRESS_ENABLE        && data_size>=MIN_COMPRESSABLE_SIZE;} // if file wants to be compressed or doesn't want to be
      Bool needDecompress()C {return    compress_mode!=COMPRESS_KEEP_ORIGINAL && compression                     ;} // if file wants to be decompressed (we don't keep original and it's compressed)
      Str  srcFullName   ()C {return data ? data->srcName() : data_name;} // don't return 'name' because that's only 1 element without the path, but for this method we need the full version for debug purposes

      File* get(Cipher *src_cipher, File &temp) // this is called only if file has data (size!=0)
      {
         File *f=null;
         if(processed.is()                     )f=&processed          ;else // if we've de/compressed data for storage, then we have to use it (this will be valid only if decompressed or compressed)
         if(data                               )f= data->openRaw(temp);else // open without decompressing, because we expect files as they are, and if they're compressed, it should be speicified with 'compression'
         if(temp.readTry(data_name, src_cipher))f=&temp               ;
         if(f)f->pos(0);
         return f;
      }
      Bool set(C FileTemp &ft, Int parent_index, PakCreator &pc)
      {
         reset(); ready=false; compress_mode=COMPRESS_ENABLE; data=null;
         T.parent=parent_index;
         switch(ft.type)
         {
            default: return pc.setError("Invalid FileTemp.type");

            case FileTemp::STD:
            {
               name           =_GetBase(ft.name);
               data_name      =         ft.name ; // here we leave 'data' as empty, because that's for NODE
               modify_time_utc=ft.fi.modify_time_utc;
               type           (ft.fi.type);

               data_size           =
               data_size_compressed=ft.fi.size;
            }break;

            case FileTemp::NODE:
            {
               name           =ft.name;
               modify_time_utc=ft.node->modify_time_utc;
               type           (ft.node->type);
               if(ft.node->exists)
               {
                  compression     = ft.node->compressed;
                  compress_mode   = ft.node->compress_mode;
                  data            =&ft.node->data; // here we leave 'data_name' as empty, because that's for STD
                  data_xxHash64_32= ft.node->xxHash64_32;
                  switch(data->type)
                  {
                     case DataSource::NAME: if(data->name.is()) // ignore empty names to avoid errors in 'FileInfo.get' (treat them as empty data)
                     {
                        FileInfo fi; if(!fi.get(data->name))return pc.setErrorAccess(data->name);
                        type                (fi.type          ); // this is optional
                        data_size_compressed=fi.size           ; // this is required
                        modify_time_utc     =fi.modify_time_utc; // alternatively this could be performed if(!modify_time_utc.valid())
                     }break;

                     case DataSource::STD: if(data->name.is()) // ignore empty names to avoid errors in 'FileInfo.getSystem' (treat them as empty data)
                     {
                        FileInfo fi; if(!fi.getSystem(data->name))return pc.setErrorAccess(data->name);
                        type                (fi.type          ); // this is optional
                        data_size_compressed=fi.size           ; // this is required
                        modify_time_utc     =fi.modify_time_utc; // alternatively this could be performed if(!modify_time_utc.valid())
                     }break;

                     case DataSource::PAK_FILE: if(C PakFile *pf=data->pak_file)
                     {
                        // override values in case user didn't specify them
                                             FlagCopy(flag       ,pf->flag, PF_STD_DIR|PF_STD_LINK); // this is optional
                                             compression         =pf->compression                  ; // this is required
                                             data_size           =pf->data_size                    ; // this is required
                                             data_size_compressed=pf->data_size_compressed         ; // this is required
                                             modify_time_utc     =pf->modify_time_utc              ; // alternatively this could be performed if(!modify_time_utc.valid())
                        if(!data_xxHash64_32)data_xxHash64_32    =pf->data_xxHash64_32             ; // override only if user didn't calculate it (because it's possible that 'data_xxHash64_32' is calculated but 'pf->data_xxHash64_32' left at 0)
                        goto size_ok; // we have to skip checking size, because if the user didn't provide 'decompressed_size', but the file is compressed then error will be returned
                     }break;

                     default:
                     {
                        Long size_compressed=data->sizeCompressed();
                        if(  size_compressed<0)return pc.setErrorAccess(srcFullName());
                        data_size_compressed=size_compressed;
                     }break;
                  }

                  if(compression)
                  {
                               if(ft.node->decompressed_size<0)return pc.setError(S+"File \""+ft.name+"\" was marked as compressed, however its 'decompressed_size' is unspecified");
                        data_size=ft.node->decompressed_size;
                  }else data_size=      data_size_compressed;
                  if(!data_size != !data_size_compressed)return pc.setError(S+"File \""+ft.name+"\" has invalid data size"); // both have to be zeros or not zeros
               size_ok:;

                  if(ft.node->decompressed_size>=0 && ft.node->decompressed_size!=data_size)return pc.setError(S+"File \""+ft.name+"\" has specified 'decompressed_size' but it doesn't match source data");
               }else
               {
                  flag|=PF_REMOVED;
               }
            }break;
         }
         return true;
      }
   };
   struct Compressor
   {
      File f_std, f_temp;
      Str  error_message;
      SyncEvent  waiting;

      Compressor() {f_temp.writeMem();}
   };

   Bool                  error_occurred, header_changed, data_size_changed;
   Int                   file_being_processed;
   UInt                  pak_flag;
   Long                  mem_available;
   PakProgress          *progress;
   Str                  *error_message;
   Memc<PakFileEx>       files;
   Pak                  &pak;
   Cipher               *src_cipher;
   COMPRESS_TYPE         compress;
   Int                   compression_level;
   SyncEvent             ready;
   Threads               threads;
   MemtN<Compressor, 16> compressors;

   PakCreator(Pak &pak, UInt pak_flag, Cipher *src_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress) : pak(pak)
   {
      if(pak_flag&PAK_NO_FILE)pak_flag|=PAK_NO_DATA;  // if we're not creating any file, then we can't save data either
      if(pak_flag&PAK_NO_DATA)compress=COMPRESS_NONE; // if we're not saving data, then disable compression (because it's not needed, however we still may want to process files for calculating hash)

      T.error_occurred      =header_changed=data_size_changed=false;
      T.file_being_processed=0;
      T.pak_flag            =pak_flag;
      T.src_cipher          =src_cipher;
      T.compress            =compress;
      T.compression_level   =compression_level;
      T.error_message       =error_message; if(error_message)error_message->clear();
      T.progress            =progress     ; if(progress     )progress     ->progress=0;
   }

   Bool setErrorAccess(C Str &name ) {return setError(S+"Can't access \""+name+'"');}
   Bool setError      (C Str &error)
   {
      error_occurred=true;
      if(error_message)*error_message=error;
      return false;
   }

   Bool add(C FileTemp &ft, Int parent_index) {return files.New().set(ft, parent_index, T);}

   void enter(FileTemp &parent, Int parent_index, Bool (*filter)(C Str &name))
   {
      FileTempContainer ftc;

      // get files
      switch(parent.type)
      {
         case FileTemp::STD : if(parent.isDir()     )for(FileFind ff(parent.name); ff(); )ftc.add(ff.pathName(), ff, filter); break;
         case FileTemp::NODE: if(parent.node->exists)FREPA(parent.node->children         )ftc.add(parent.node->children[i] ); break; // don't add children for nodes that are marked as removed
      }
      ftc.sort();

      // add files
      Int file_elms=files.elms();
      if(parent_index>=0)
      {
         files[parent_index].children_offset=file_elms;
         files[parent_index].children_num   =ftc.files.elms();
      }
      FREPA(ftc.files)add  (ftc.files[i], parent_index       );
      FREPA(ftc.files)enter(ftc.files[i], file_elms+i, filter);
   }

   static Str SrcName  (C PakFileEx &src, C PakFile &dest, C Pak &pak) {Str s=src.srcFullName(); return s.is() ? s : pak.fullName(dest);}
   static Str CantFlush(C Str &file) {return S+"Can't write data to:\n\""+file+"\".\nPlease verify you have enough free disk space.";}
   static Str CantCopy (C PakFileEx &src, C PakFile &dest, C Pak &pak) {return S+"Can't store:\n\""+SrcName(src, dest, pak)+"\"\nin:\n\""+pak.pakFileName()+"\".\nPlease verify that source is readable, and you have enough free disk space.";}
   static Str CantOpen (C PakFileEx &src, C PakFile &dest, C Pak &pak) {return S+"Can't open file:\n\""+SrcName(src, dest, pak)+'"';}
   static Str CantDec  (C PakFileEx &src, C PakFile &dest, C Pak &pak) {return S+"Can't decompress file:\n\""+SrcName(src, dest, pak)+'"';}
   static Str CantWrite(C Str &file)
   {
      Str path=MakeFullPath(GetPath(file)), error=S+"Can't write to:\n\""+file+"\".\nPath:\n\""+path+"\"\n";
      if(FExistSystem(path))error+="does exist, perhaps there's no permission.";else error+="does not exist.";
      return error;
   }

   static void CompressorFunc(PakFile &dest, PakCreator &pc, Int thread_index) {pc.compressor(pc.pak._files.index(&dest), pc.compressors[thread_index]);}
          void compressor    (Int i, Compressor &compressor)
   {
      PakFile &dest=pak._files[i];
      if(      dest.data_size)
      {
         PakFileEx &src=files[i];
         Long       mem_reserved=0; // memory currently reserved for this file
         Bool         decompress=                                     src.needDecompress() , // if decompress
                    try_compress=(compress                         && src.needCompress  ()), // if   compress
                    set_hash    =(FlagTest(pak_flag, PAK_SET_HASH) && src.needHash      ()); // if set hash

         if(error_occurred
         || progress && progress->wantStop())goto finished;

         if(decompress || try_compress || set_hash)
         {
            // wait until there's memory available
            Long mem_needed= // estimate memory needed for this file
            (  
           Long(  decompress                                                   ? dest.data_size+DecompressionMemUsage(dest.compression, 255, dest.data_size) : 0)
              +(try_compress                                                   ? dest.data_size                                                              : 0)
              +((set_hash && dest.compression && !decompress && !try_compress) ? dest.data_size                                                              : 0) // if we need to set hash, but the file is compressed, then we will have to decompress it first to temp memory, we don't need to do this if we already decompress or compress because we can calculate hash over there too
            );
            for(;;)
            {
               if(error_occurred)goto finished; // if error occurred then always finish
               if(file_being_processed==i) // if we're processing this file then we always need to proceed
               {
                  AtomicSub(mem_available, mem_reserved=mem_needed); // reserve memory
                  break; // proceed
               }
            check_mem:
               Long temp=AtomicGet(mem_available); if(temp>=mem_needed) // check if we have enough available memory
               {
                  if(AtomicCAS(mem_available, temp, temp-mem_needed)) // if we were able to reserve the memory
                  {
                     mem_reserved=mem_needed; // mark it as reserved
                     break; // proceed
                  }
                  goto check_mem; // check memory again
               }
               compressor.waiting.wait(); // wait
            }

            // open file
            File *f_src=src.get(src_cipher, compressor.f_std);
            if(  !f_src){compressor.error_message=CantOpen(src, dest, pak); error_occurred=true; goto finished;}

            xxHash64Calc hasher;

            // decompress (before compressing)
            if(decompress)
            {
               if(DecompressRaw(*f_src, src.processed.writeMem(), dest.compression, dest.data_size_compressed, dest.data_size, false, set_hash ? &hasher : null))
               {
                  if(set_hash)
                  {
                     if(dest.data_xxHash64_32=hasher.hash.hash32())header_changed=true; // if it's different than zero, then it means we've changed the value and header was changed
                     set_hash=false; // we now have it, so no need to calculate any more
                  }
                  dest.data_size_compressed=dest.data_size; dest.compression=COMPRESS_NONE;
                  data_size_changed=header_changed=true; // data size and header were changed
                  f_src=&src.processed; f_src->pos(0);
               }else
               {
                  compressor.error_message=CantDec(src, dest, pak); error_occurred=true; goto finished;
               }
            }

            // compress (before setting hash, because we can do it here in one go)
            if(try_compress)
            {
               File &compressed=((f_src!=&src.processed) ? src.processed.writeMem() : compressor.f_temp);
               if(CompressRaw(*f_src, compressed, compress, compression_level, compressors.elms()<=1, set_hash ? &hasher : null)) // use multi-threaded compression only if we have up to 1 compressor (otherwise we have multiple compressors which call 'CompressRaw' on multiple threads)
               {
                  if(set_hash)
                  {
                     if(dest.data_xxHash64_32=hasher.hash.hash32())header_changed=true; // if it's different than zero, then it means we've changed the value and header was changed
                     set_hash=false; // we now have it, so no need to calculate any more
                  }
                  if(compressed.size()<f_src->size()) // if compressed OK and the result is smaller, then use this version
                  {
                     dest.data_size_compressed=compressed.size(); dest.compression=compress; // use that version and mark file as compressed
                     data_size_changed=header_changed=true; // data size and header were changed
                     if(&src.processed!=&compressed)
                     {
                        src.processed.reset(); // this will be moved to 'f_temp' so reset it to release memory, this is needed as 'f_temp' is always expected to be empty
                        Swap(src.processed, compressed);
                     }
                     f_src=&src.processed; //f_src->pos(0); not needed
                     goto compress_ok;
                  }
               }
               if(&compressed==&src.processed)compressed.del();else compressed.reset(); // if we were compressing to 'src.processed' but we didn't use the result (compress failed or result is not smaller), then delete it, otherwise it's 'f_temp' so reset it
            compress_ok:;
            }

            // set hash (after de/compressing)
            if(set_hash)
               if(dest.data_xxHash64_32=DecompressedFilexxHash64_32(*f_src, compressor.f_temp, dest.compression, dest.data_size_compressed, dest.data_size))
                  header_changed=true; // if it's different than zero, then it means we've changed the value and header was changed
         }

      finished:
            mem_reserved-=src.processed.size(); // when releasing the reserved memory below, we need to keep the 'processed' memory that's going to be used later
         if(mem_reserved)AtomicAdd(mem_available, mem_reserved); // release what was reserved
         src.ready=true; ready.on(); // we need to set 'ready' always for files with data, because the main thread waits for it
      }
   }

   void wakeUp() {REPAO(compressors).waiting.on();}
   void stopThreads() {error_occurred=true; wakeUp(); threads.del();} // set error first, then wake up threads and finally delete them

   Bool create(C Str &pak_name, Cipher *cipher)
   {
      // !! these are needed before any "goto error" !!
      File f_dest;
      pak.pakFileName(pak_name); // !! set this first because 'pak_name' can be a Pak member, don't use 'pak_name' after, but use 'pak.pakFileName' instead !!

      if(error_occurred
      || progress && progress->wantStop(error_message))goto error;

      {
         // create !! must set all members because we could be operating on Pak that's already created !!
         Int  names_elms=0; REPA(files)names_elms+=files[i].name.length()+1;
         pak._names.setNum(names_elms  );
         pak._files.setNum(files.elms());
         pak._cipher_per_file   =true; // if changing then please remember that currently '_cipher_per_file' is set based on the Pak file format version
         pak._file_type         =FILE_STD_READ;
         pak._file_cipher_offset=0;
         pak._file_cipher       =cipher;
         pak._root_files        =0;
         pak._data_offset       =0;
         pak._data_decompressed.del();
         pak._data              =null;

              names_elms              =0;
         Int  files_to_process        =0;
         UInt max_data_size_compress  =0;
         Long thread_mem_usage        =0,
              total_data_size_compressed=0, total_data_size_decompressed=0, decompressed_processed=0;
         FREPA(files)
         {
          C PakFileEx &src =     files[i];
            PakFile   &dest=pak._files[i];

            // !! must set all members because we could be operating on PakFile that's already created !!
            Int src_name_chars=src.name.length()+1;
            dest.name                =Set(pak._names.data()+names_elms, src.name, src_name_chars); names_elms+=src_name_chars;
            dest.flag                =  src.flag                ;
            dest.compression         =  src.compression         ;
            dest.parent              =  src.parent              ; if(dest.parent<0)pak._root_files++;
            dest.children_offset     =  src.children_offset     ;
            dest.children_num        =  src.children_num        ;
            dest.modify_time_utc     =  src.modify_time_utc     ;
            dest.data_xxHash64_32    =  src.data_xxHash64_32    ;
            dest.data_size           =  src.data_size           ;
            dest.data_size_compressed=  src.data_size_compressed;
            dest.data_offset         =total_data_size_compressed;
                                      total_data_size_compressed  +=src.data_size_compressed;
                                      total_data_size_decompressed+=src.data_size;
            Bool file_decompress=(                                    src.needDecompress()),
                 file_compress  =(compress                         && src.needCompress  ()),
                 file_hash      =(FlagTest(pak_flag, PAK_SET_HASH) && src.needHash      ());
            Long mem_usage= // estimate memory needed for this file
            (
           Long( file_decompress                                                      ? dest.data_size+DecompressionMemUsage(dest.compression, 255, dest.data_size) : 0)
              +( file_compress                                                        ? dest.data_size                                                              : 0)
              +((file_hash && dest.compression && !file_decompress && !file_compress) ? dest.data_size                                                              : 0) // if we need to set hash, but the file is compressed, then we will have to decompress it first to temp memory, we don't need to do this if we already decompress or compress because we can calculate hash over there too
            );
            MAX(thread_mem_usage, mem_usage);
            if(file_compress)MAX(max_data_size_compress, src.data_size);
            if(file_decompress || file_compress || file_hash)files_to_process++;
         }

         if(progress && progress->wantStop(error_message))goto error;

         // save header
         if(!FlagTest(pak_flag, PAK_NO_FILE)) // create file
         {
            if(!pak.pakFileName().is()                    ){if(error_message)*error_message="Pak name was not specified"; goto error;}
            if(!f_dest.writeTry(pak.pakFileName(), cipher)){if(error_message)*error_message=CantWrite(pak.pakFileName()); goto error;}
            if(!pak.saveHeader(f_dest)                    ){if(error_message)*error_message=CantFlush(pak.pakFileName()); goto error;}
            pak._file_cipher_offset=f_dest._cipher_offset+f_dest.pos();
            pak._data_offset       =f_dest.posAbs();
         }
         auto f_dest_cipher_offset=f_dest._cipher_offset;

         // process files
         if(!FlagTest(pak_flag, PAK_NO_DATA) || FlagTest(pak_flag, PAK_SET_HASH)) // write data or set hash
         {
         #if HAS_THREADS
            if(files_to_process)
            {
               // calculate memory usage
               UInt compression_usage=CompressionMemUsage(compress, compression_level, max_data_size_compress);
               thread_mem_usage+=compression_usage;
               MemStats mem_stats; mem_stats.get(); mem_available=mem_stats.avail_phys;
            #if !X64
               MIN(mem_available, INT_MAX); // limit to 32-bit on 32-bit platforms, use 2GB just in case
            #endif
               Int num=Min(Cpu.threads(), files_to_process);
               if(thread_mem_usage)MIN(num, mem_available/thread_mem_usage);
               compressors.setNum(Max(1, num)); if(compressors.elms())
               {
                  mem_available-=compressors.elms()*compression_usage; // reserve memory needed for compression up-front, don't include others because they will be handled per-file
                  threads.create(true, compressors.elms()); // process in order
                  FREPA(pak)threads.queue(pak._files[i], CompressorFunc, T); // queue in order
               }
            }
         #endif
            if(!compressors.elms())compressors.New(); // we need at least one

            // process all files
            File f_std; // keep 'f_std' outside of loop to decrease overhead
            FREPA(pak)
            {
               if(progress && progress->wantStop(error_message))goto error;

               PakFileEx &src =     files[i];
               PakFile   &dest=pak._files[i];

               if(data_size_changed) // if data size of at least one file was changed due to compression, then
               {
                  dest.data_offset=f_dest.posAbs()-pak._data_offset; // all files after it need to have their data offset adjusted (set this even for empty files with no data, so there are no files with 'data_offset' outside of the Pak file size)
                //header_changed=true; we don't need to set this because it's already set along with 'data_size_changed'
               }
               if(dest.data_size)
               {
                  // wait until file is processed
                  file_being_processed=i;
                  if(threads.threads())
                  {
                     if(!src.ready){wakeUp(); do ready.wait();while(!src.ready);}
                  }else compressor(i, compressors[0]);

                  // check for errors
                  if(error_occurred)
                  {
                     stopThreads(); // stop the threads first
                     if(error_message)REPA(compressors)if(compressors[i].error_message.is()) // grab any first error found
                     {
                        Swap(*error_message, compressors[i].error_message);
                        break;
                     }
                     goto error;
                  }

                  // write data
                  if(!FlagTest(pak_flag, PAK_NO_DATA))
                  {
                     if(progress && progress->wantStop(error_message))goto error;

                     // get source file
                     File *f_src=src.get(src_cipher, f_std);
                     if(  !f_src){if(error_message)*error_message=CantOpen(src, dest, pak); goto error;}

                     // check for invalid data
                     if(!dest.compression && dest.data_size!=dest.data_size_compressed)
                     {
                        if(error_message)*error_message=S+"File is not compressed but its 'data_size' != 'data_size_compressed':\n\""+SrcName(src, dest, pak)+'"';
                        goto error;
                     }
                     if(f_src->size()!=dest.data_size_compressed)
                     {
                        if(error_message)*error_message=S+"'data_size_compressed' doesn't match file size:\n\""+SrcName(src, dest, pak)+'"';
                        goto error;
                     }

                     // save data
                     if(pak._cipher_per_file)f_dest.cipherOffsetClear(); // make encryption result always the same regardless of position in Pak file
                     if(!f_src->copy(f_dest))
                        {if(error_message)*error_message=CantCopy(src, dest, pak); goto error;} // don't flush here, flush only one time at the end

                     // release memory
                     Long size=src.processed.size(); src.processed.del(); AtomicAdd(mem_available, size); // release memory first, then increase the counter
                     wakeUp(); // notify compressors that memory was released
                  }

                  // update progress
                  decompressed_processed+=dest.data_size;
                  if(progress)progress->progress=Sat(total_data_size_decompressed ? Dbl(decompressed_processed)/total_data_size_decompressed : 0); // use Dbl because we operate on ULong
               }
            }

            // flush and update header
            if(f_dest.is())
            {
               if(!f_dest.flush()){if(error_message)*error_message=CantFlush(pak.pakFileName()); goto error;} // check for correct flush before eventual adjusting the Pak header on start of the file
               if(header_changed) // if during file processing, the header was changed, then we need to resave it
               {
                  Long pos=f_dest.pos();
                  if(pak._cipher_per_file)f_dest.cipherOffset(f_dest_cipher_offset); // reset the cipher offset here so that saving file header will use it
                  if(!f_dest.pos(  0) || !pak.saveHeader(f_dest)
                  || !f_dest.pos(pos)){if(error_message)*error_message=CantFlush(pak.pakFileName()); goto error;}
               }
            }
         }

         return true;
      }

   error:
      stopThreads();
      f_dest.del(); FDelFile(pak.pakFileName()); pak.del(); // release the file handle first, then delete Pak file, then delete Pak object, this is important to avoid having partial incomplete Pak files (which headers could be OK, but the data is missing), we delete Pak object last because we need its file name to delete the file
      return false;
   }
};
/******************************************************************************/
Bool Pak::create(C Str &file, C Str &pak_name, UInt flag, Cipher *dest_cipher, Cipher *src_cipher, COMPRESS_TYPE compress, Int compression_level, Bool (*filter)(C Str &name), Str *error_message, PakProgress *progress)
{
   Str f=file;
   return create(MemPtr<Str>(f), pak_name, flag, dest_cipher, src_cipher, compress, compression_level, filter, error_message, progress);
}
Bool Pak::create(C MemPtr<Str> &files, C Str &pak_name, UInt flag, Cipher *dest_cipher, Cipher *src_cipher, COMPRESS_TYPE compress, Int compression_level, Bool (*filter)(C Str &name), Str *error_message, PakProgress *progress)
{
   if(progress && progress->wantStop(error_message))return false;
   // !! don't delete Pak anywhere here because we still need 'pak_name' which can be a Pak member !!
   PakCreator pc(T, flag, src_cipher, compress, compression_level, error_message, progress);

   // get files
   PakCreator::FileTempContainer ftc; FREPA(files)ftc.add(files[i], filter, pc); ftc.sort();

   // add files
   if(FlagTest(flag, PAK_SHORTEN) && ftc.files.elms()==1 && ftc.files[0].isDir())pc.enter(ftc.files[0], -1, filter);else
   {
      FREPA(ftc.files)pc.add  (ftc.files[i], -1        );
      FREPA(ftc.files)pc.enter(ftc.files[i],  i, filter);
   }

   // adjust Pak name
   Str pn=pak_name; // !! Warning: 'pak_name' can be a Pak member !!
   if(!pn.is())
   {
      if(ftc.files.elms()==1) // one file
      {
         if(ftc.files[0].fi.type!=FSTD_DRIVE)
         { // when packing "c:/esenthel" create "c:/esenthel.pak"
            pn=ftc.files[0].name;
         }
      }else
      if(ftc.files.elms()>1) // multiple files
      {
         if(ftc.files[0].fi.type!=FSTD_DRIVE)
         { // when packing ["c:/esenthel/1", "c:/esenthel/2", ..] create "c:/esenthel/esenthel.pak"
            pn=GetPath(ftc.files[0].name);
            if(pn.is())pn.tailSlash(true)+=GetBase(pn);
         }
      }
      if(pn.is())pn+=".pak";
   }

   // create
   return pc.create(pn, dest_cipher);
}
Bool Pak::create(C MemPtr<PakNode> &files, C Str &pak_name, UInt flag, Cipher *dest_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress)
{
   if(progress && progress->wantStop(error_message))return false;
   // !! don't delete Pak anywhere here because we still need 'pak_name' which can be a Pak member !!
   PakCreator pc(T, flag, null, compress, compression_level, error_message, progress);

   // get files
   PakCreator::FileTempContainer ftc; FREPA(files)ftc.add(files[i]); ftc.sort();

   // add files
   if(FlagTest(flag, PAK_SHORTEN) && ftc.files.elms()==1 && ftc.files[0].isDir())pc.enter(ftc.files[0], -1, null);else
   {
      FREPA(ftc.files)pc.add  (ftc.files[i], -1      );
      FREPA(ftc.files)pc.enter(ftc.files[i],  i, null);
   }

   // create
   return pc.create(pak_name, dest_cipher); // !! Warning: 'pak_name' can be a Pak member !!
}
/******************************************************************************/
// TODO: use binarySearch, make sure Editor passes files sorted in order and that name from UID is sorted too
static Int ComparePath(C PakFileData* &a, C PakFileData* &b)
{
   return ComparePath(a->name, b->name);
}
static      PakNode * FindElm         (Memb<PakNode> &nodes, Str name) {REPA(nodes)if(nodes[i].name==name)return &nodes[i]; return null;}
static Memb<PakNode>* FindNodeChildren(Memb<PakNode> &nodes, Str path) // find 'children' container of 'path' node, null if not found (don't create new elements)
{
   Str start=_GetStart(path); if(!start.is())return &nodes;
   if(PakNode *node=FindElm(nodes, start))return FindNodeChildren(node->children, GetStartNot(path));
   return null;
}
static Memb<PakNode>* FindNodeChildrenIfNotRemoved(Memb<PakNode> &nodes, C Str &path, DateTime &date_time_utc) // find 'children' container of 'path' node, New if path is not removed (given path can be null or 'exists'==true, but not 'exists'==false), null otherwise, we use this function to specify an element as MARK_REMOVED so we don't need to do this in already removed path (exists==false)
{
   Str start=_GetStart(path); if(!start.is())return &nodes;

   PakNode *node=FindElm(nodes, start);
   if(!node        )node=&nodes.New().newSetFolder(start, date_time_utc);
   if(!node->exists)return null; // we don't want to add new things into removed node, so return null

   return FindNodeChildrenIfNotRemoved(node->children, GetStartNot(path), date_time_utc);
}
static Memb<PakNode>& GetNodeChildren(Memb<PakNode> &nodes, C Str &path, DateTime &date_time_utc) // find 'children' container of 'path' node, New if not found, force 'exists'==true everywhere, we're setting this path to specify element with REPLACE mode
{
   Str start=_GetStart(path); if(!start.is())return nodes;

   PakNode *node=FindElm(nodes, start);
   if(node)node->exists=true; // force as existing
   else    node=&nodes.New().newSetFolder(start, date_time_utc);

   return GetNodeChildren(node->children, GetStartNot(path), date_time_utc);
}
Bool Pak::create(C Mems<C PakFileData*> &files, C Str &pak_name, UInt flag, Cipher *dest_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress)
{
   DateTime      date_time_utc; date_time_utc.getUTC();
   Memb<PakNode> nodes;

   FREPA(files) // process files in order in which they were given (in case there are multiple files of same full name, we will update them according to their order)
   {
      if(progress && progress->wantStop(error_message))return false;

    C PakFileData &pfd =*files[i];
      Str          base=GetBase(pfd.name),
                   path=GetPath(pfd.name);
      switch(pfd.mode)
      {
         case PakFileData::REMOVE:
         {
            if(Memb<PakNode> *parent_children=FindNodeChildren(nodes, path)) // find parent
               if(PakNode *node=FindElm(*parent_children, base)) // if that node exists in parent
                  parent_children->removeData(node); // remove it from parent, we don't need to keep order, because we're processing files in order in this loop, and if a file is found with the same name, then it replaces it
         }break;

         case PakFileData::MARK_REMOVED:
         {
            if(Memb<PakNode> *parent_children=FindNodeChildrenIfNotRemoved(nodes, path, date_time_utc)) // find parent
            {
               PakNode *node=FindElm(*parent_children, base); // find that node in parent
               if(     !node)node=&parent_children->New();    // if doesn't exist then create new one
               node->setRemoved(base, pfd.modify_time_utc, pfd.type);
            }
         }break;

         case PakFileData::REPLACE:
         {
            Memb<PakNode> &parent_children=GetNodeChildren(nodes, path, date_time_utc);
            PakNode *node=FindElm(parent_children, base); // find that node in parent
            if(     !node)node=&parent_children.New();    // if doesn't exist then create new one
            node->set(base, pfd);
         }break;
      }
   }

   // create according to created nodes
   return create(nodes, pak_name, flag, dest_cipher, compress, compression_level, error_message, progress);
}
Bool Pak::create(C MemPtr<PakFileData> &files, C Str &pak_name, UInt flag, Cipher *dest_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress)
{
   Mems<C PakFileData*> f; f.setNum(files.elms()); REPAO(f)=&files[i];
   return create(f, pak_name, flag, dest_cipher, compress, compression_level, error_message, progress);
}
/******************************************************************************/
// MAIN
/******************************************************************************/
static void ExcludeChildren(Pak &pak, C PakFile &pf, Memt<Bool> &is)
{
   REP(pf.children_num)
   {
      Int child_i =pf.children_offset+i;           // get index of i-th child in 'pf' file
       is[child_i]=false;                          // exclude that child
      ExcludeChildren(pak, pak.file(child_i), is); // exclude all  children of that child too
   }
}
Bool PakUpdate(Pak &src_pak, C MemPtr<PakFileData> &update_files, C Str &pak_name, Cipher *dest_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress)
{
   if(error_message)error_message->clear();

   // set 'src_pak' files as 'PakFileData'
   Memc<PakFileData> src_files; // this container will include all files from 'src_pak' that weren't excluded (weren't replaced by newer versions from 'update_files')
   {
      Memt<Bool> is; is.setNum(src_pak.totalFiles()); SetMem(is.data(), true, is.elms()); // set 'is' array specifying which files from 'src_pak' should be placed in target file
      REPA(update_files) // check all new elements (order is not important as we're comparing them to 'src_pak' files only)
      {
            C PakFileData &pfd=update_files[i]; // take new element
         if(C PakFile     *pf =src_pak.find(pfd.name, true)) // if there exists an original version (file in 'src_pak')
         {
            Int i=src_pak.files().index(pf); // take the index of file in 'src_pak'
            is[i]=false; // exclude that file from 'src_pak' (instead of it, we'll use 'pfd' - the newer version from 'update_files')
            if(pfd.mode!=PakFileData::REPLACE)ExcludeChildren(src_pak, *pf, is); // if the newer version removes old file then we need to exclude also all children of 'pf' in 'src_pak'
         }
      }

      FREPA(src_pak)if(is[i])
      {
       C PakFile     &pf =src_pak  .file(i);
         PakFileData &pfd=src_files.New ( );
         pfd.mode             =(FlagTest(pf.flag, PF_REMOVED) ? PakFileData::MARK_REMOVED : PakFileData::REPLACE);
         pfd.type             =pf.type();
         pfd.compress_mode    =COMPRESS_KEEP_ORIGINAL; // keep source files in original compression (for example if a Sound file was requested to have no compression before, to speed up streaming playback, then let's keep it)
         pfd.compressed       =pf.compression;
         pfd.decompressed_size=pf.data_size;
         pfd.name             =src_pak.fullName(i);
         pfd.xxHash64_32      =pf.data_xxHash64_32;
         pfd.modify_time_utc  =pf.modify_time_utc;
         pfd.data.set(pf, src_pak);
      }

      // here 'is' is deleted
   }

   // move everything to one container, order is important
   Mems<C PakFileData*> all_files; all_files.setNum(src_files.elms()+update_files.elms());
   FREPA(   src_files)all_files[i                 ]=&   src_files[i]; // first the src    files
   FREPA(update_files)all_files[i+src_files.elms()]=&update_files[i]; // now   the update files, in order in which they were given (in case there are multiple files of same full name)

   // create Pak basing on all files
   Pak  temp; temp.pakFileName(pak_name); // this will normalize and make full path, we need the full name to make the comparison
   Bool temp_file=(EqualPath(src_pak.pakFileName(), temp.pakFileName()) && temp.pakFileName().is()); // if dest name is the same as source name, then we have to write to a temporary file (but ignore if the name is empty, perhaps it wants to update Pak object only without operating on files)
   if(  temp.create(all_files, temp_file ? temp.pakFileName()+"@new" : temp.pakFileName(), 0, dest_cipher, compress, compression_level, error_message, progress)) // have to work on a temporary Pak, because during creation we may access files from the old Pak
   {
      if(temp_file) // if we've created a temp file, we need to rename it first
      {
         if(!FRename(temp.pakFileName(), src_pak.pakFileName())) // if failed to rename
         {
            FDelFile(temp.pakFileName()); // delete the new file
            return false; // return without updating 'src_pak'
         }
         Swap(temp._file_name, src_pak._file_name); // swap file names, because we need 'temp' to have the original file name, and 'src_pak' is going to be deleted so we don't care
      }
      Swap(src_pak, temp); return true;
   } // no need to do anything because 'Pak.create' will delete the file on failure
   return false;
}
/******************************************************************************/
Bool Equal(C PakFile *a, C PakFile *b)
{
   if(a && (a->flag&PF_REMOVED))a=null; // treat as if doesn't exist
   if(b && (b->flag&PF_REMOVED))b=null; // treat as if doesn't exist

   if(a && b) // both exist
   {
      return  a->type()   ==b->type()    // same type
         &&   a->data_size==b->data_size // same size
         && ((a->data_size                                              ) ? !Compare(a->modify_time_utc , b->modify_time_utc, 1) : true)  // if they have data                                      then their modification time must match (1 second tolerance due to Fat32)
         && ((a->data_size && a->data_xxHash64_32 && b->data_xxHash64_32) ?          a->data_xxHash64_32==b->data_xxHash64_32    : true); // if they have data and both have information about hash then it                      must match
   }
   return !a && !b; // true only if both don't exist (one exists and other doesn't -> they're different)
}
Bool Equal(C PakFileData *pfd, C PakFile *pf)
{
   if(pfd && (pfd->mode==PakFileData::REMOVE || pfd->mode==PakFileData::MARK_REMOVED))pfd=null; // treat as if doesn't exist
   if(pf  && (pf ->flag&PF_REMOVED                                                  ))pf =null; // treat as if doesn't exist

   if(pfd && pf) // both exist
   {
      if(pfd->type!=pf->type())return false; // different type

      Long     pfd_size=pfd->decompressed_size;
      DateTime pfd_time=pfd->modify_time_utc;
      UInt     pfd_hash=pfd->xxHash64_32;

      // always override values from PAK_FILE to keep consistency with 'Pak.create'
      if(pfd->data.type==DataSource::PAK_FILE)if(C PakFile *pf=pfd->data.pak_file) // from PAK_FILE we can always extract the size, even if it's compressed
      {
                      pfd_size=pf->data_size;
                      pfd_time=pf->modify_time_utc;
         if(!pfd_hash)pfd_hash=pf->data_xxHash64_32; // override only if user didn't calculate it (because it's possible that 'pfd_hash' is calculated but 'pf->data_xxHash64_32' left at 0)
      }

      if(pfd_size<0 && !pfd->compressed)switch(pfd->data.type) // if size is unknown and source is not compressed
      {
         case DataSource::NAME:
         {
            FileInfo fi; if(fi.get(pfd->data.name))
            {
               pfd_size=fi.size;
               pfd_time=fi.modify_time_utc;
            }
         }break;

         case DataSource::STD:
         {
            FileInfo fi; if(fi.getSystem(pfd->data.name))
            {
               pfd_size=fi.size;
               pfd_time=fi.modify_time_utc;
            }
         }break;

         default: pfd_size=pfd->data.size(); break;
      }
      if(pfd_size!=pf->data_size)return false; // different size

      if(pfd_size) // check time and hash only if have data (to skip empty files)
      {
         if(pfd_hash && pf->data_xxHash64_32 && pfd_hash!=pf->data_xxHash64_32)return false; // both hashes known and different

         if(!pfd_time.valid())switch(pfd->data.type) // if time is unknown
         {
            case DataSource::NAME: {FileInfo fi; if(fi.get      (pfd->data.name))pfd_time=fi.modify_time_utc;} break;
            case DataSource::STD : {FileInfo fi; if(fi.getSystem(pfd->data.name))pfd_time=fi.modify_time_utc;} break;
         }
         if(Compare(pfd_time, pf->modify_time_utc, 1))return false; // different time (1 second tolerance due to Fat32)
      }
   }
   return !pfd == !pf; // true only if both exist or both don't exist
}
/******************************************************************************/
static Int ComparePF(C PakFileData*C &a, C PakFileData*C &b) {return ComparePath(a->name, b->name);}
static Int ComparePF(C PakFileData*C &a, C Str           &b) {return ComparePath(a->name, b      );}

Bool PakEqual(C MemPtr<PakFileData> &files, C Pak &pak)
{
   Memt<C PakFileData*> files_sorted; files_sorted.setNum(files.elms()); REPAO(files_sorted)=&files[i];
   files_sorted.sort(ComparePF);
   REPA(files)
   {
    C PakFileData &pfd=files[i];
      if(!Equal(&pfd, pak.find(pfd.name)))return false;
   }
   REPA(pak)
   {
    C PakFile &pf=pak.file(i);
      if(!FlagTest(pf.flag, PF_STD_DIR)) // skip folders because they're not required to be in the 'PakFileData' list
      {
       C PakFileData **files_pfd=files_sorted.binaryFind(pak.fullName(pf), ComparePF);
         if(!Equal(files_pfd ? *files_pfd : null, &pf))return false;
      }
   }
   return true;
}
Bool PakEqual(C MemPtr<PakFileData> &files, C Str &name, Cipher *cipher)
{
   if(name.is())
   {
      Pak pak; if(pak.load(name, cipher))return PakEqual(files, pak);
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
