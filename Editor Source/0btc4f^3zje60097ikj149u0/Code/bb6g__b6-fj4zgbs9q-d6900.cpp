/******************************************************************************/
const Str  ConnectionName   ="Esenthel File Transfer";
const uint ConnectionVersion=0;
/******************************************************************************/
enum COMMAND
{
   CMD_VERSION_CHECK, // must be first
   CMD_FILE_LIST,
   CMD_FILE_LIST_RECURSIVE,
   CMD_RENAME,
   CMD_REPLACE,
   CMD_CREATE_DIR,
   CMD_DELETE,
   CMD_GET,
   CMD_REPLY,
   CMD_NUM,
}
cchar8* CommandName(COMMAND cmd) // !! these names are saved !!
{
   switch(cmd)
   {
      case CMD_VERSION_CHECK      : return "VersionCheck";
      case CMD_FILE_LIST          : return "FileList";
      case CMD_FILE_LIST_RECURSIVE: return "FileListRecursive";
      case CMD_RENAME             : return "Rename";
      case CMD_REPLACE            : return "Replace";
      case CMD_CREATE_DIR         : return "CreateDir";
      case CMD_DELETE             : return "Delete";
      case CMD_GET                : return "Get";
      case CMD_REPLY              : return "Reply";
      default                     : return null;
   }
}
bool UsesDestPath(COMMAND cmd)
{
   switch(cmd)
   {
      case CMD_REPLACE: return true;
      default         : return false;
   }
}
bool UsesDestNames(COMMAND cmd)
{
   switch(cmd)
   {
      case CMD_RENAME :
      case CMD_REPLACE: return true;
      default         : return false;
   }
}
/******************************************************************************/
class ServerClass : ConnectionServer
{
   class Client : ConnectionServer.Client
   {
      bool   verified=false, all_ok=true;
      Thread thread;

     ~Client() {thread.del();} // delete the thread first

      virtual void create(ConnectionServer &server) {super.create(server); thread.create(Update, this);}
      virtual bool update(                        ) {return thread.active();}

      static bool Update(Thread &thread) {return ((Client*)thread.user).threadUpdate();}
             bool threadUpdate()
      {
         if(!super.update())return false;
         if(connection.receive(1)) // wait because this is our own thread
         {
            File &f=connection.data;
            byte cmd=f.getByte(); if(!InRange(cmd, CMD_NUM))return false;
            if(!verified)
            {
               if(cmd==CMD_VERSION_CHECK && Equal(f.getStr(), ConnectionName, true))
               {
                  bool  ver_ok=(f.decUIntV()==ConnectionVersion);
                  bool pass_ok=(f.getStr()==Host.pass());
                  verified=(ver_ok && pass_ok);
                  f.reset().putByte(CMD_VERSION_CHECK).putStr(ConnectionName).putBool(verified);
                  if(! ver_ok)f.putStr("App Version conflict. Please upgrade your software.");else
                  if(!pass_ok)f.putStr("Incorrect password");
                  f.pos(0);
                  connection.tcpNoDelay(true);
                  connection.send(f);
               }
               if(!verified)return false; // if received something that wasn't correct version match then remove
            }else switch(cmd)
            {
               case CMD_FILE_LIST:
               {
                  Str rel=f.getStr(), full;
                  Memc<Pane.ElmBase> elms; if(Server.fullPath(rel, full))GetFileList(full, elms); // don't disconnect in case user requests a path outside of shared folder, instead return empty list, because this can happen by accident (like typing wrong path, or having an old path stored in settings)
                  File temp; temp.writeMem(); elms.save(temp); temp.pos(0);
                  f.reset().putByte(CMD_FILE_LIST).putStr(rel); Compress(temp, f, COMPRESS_LZ4); f.pos(0);
                  connection.send(f);
               }break;

               case CMD_FILE_LIST_RECURSIVE:
               {
                  FileList file_list;
                  f>>file_list.name; Str full; if(!Server.fullPath(file_list.name, full))return false;
                  file_list.fi.getSystem(full); if(file_list.fi.type==FSTD_DIR){FileLister lister(full, thread); Swap(file_list.files, lister.files);}
                  File temp; temp.writeMem(); file_list.save(temp); temp.pos(0);
                  f.reset().putByte(CMD_FILE_LIST_RECURSIVE); Compress(temp, f, COMPRESS_LZ4); f.pos(0);
                  connection.send(f);
               }break;

               case CMD_RENAME:
               {
                  Memc<Str2> names; if(names.load(f))FREPA(names)
                  {
                     Str2 &name=names[i];
                     if(!Server.fullPath(name.src , name.src )
                     || !Server.fullPath(name.dest, name.dest))return false;
                     if(!FExistSystem(name.dest) || EqualPath(name.src, name.dest) && !Equal(GetBase(name.src), GetBase(name.dest), true)) // dest doesn't exist, or the same path, but name with different case
                     {
                        FCreateDirs(GetPath  (name.dest));
                        FRename    (name.src, name.dest );
                     }
                  }
               }break;

               case CMD_CREATE_DIR:
               {
                  Memc<Str> names; if(names.load(f))FREPA(names)
                  {
                     Str &name=names[i]; if(!Server.fullPath(name, name))return false;
                     FCreateDirs(name);
                  }
               }break;

               case CMD_DELETE:
               {
                  Memc<Str> names; if(names.load(f))FREPA(names)
                  {
                     Str &name=names[i]; if(!Server.fullPath(name, name))return false;
                     FRecycle(name);
                  }
               }break;

               case CMD_GET:
               {
                  Memc<Str2> names; names.setNum(f.decUIntV()); FREPA(names)
                  {
                     Str2 &name=names[i];
                     f>>name.dest; if(!Server.fullPath(name.dest, name.src))return false;
                  }
                  long progress=0; SendFiles(names, connection, progress, thread, false);
               }break;

               case CMD_REPLACE:
               {
                  bool ok=false;
                  IOFileData file; if(file.load(f))
                  {
                     if(!Server.fullPath(file.name, file.name))return false;
                     ok=file.append(file.name, f);
                  }
                  all_ok&=ok;
               }break;

               case CMD_REPLY:
               {
                  f.reset().putByte(CMD_REPLY).putBool(all_ok).pos(0); all_ok=true; // reset status after sending so future operations will not report errors
                  connection.send(f);
               }break;
            }
         }
         return true;
      }
   }

   Client& client(int i) {return (Client&)clients[i];}

   Str   getPath()C {return Str(Host.path()).tailSlash(true);}
   bool fullPath(C Str &rel, Str &full)C // warning: 'full' may be reference to same 'Str' as 'rel'
   {
      Str path=getPath();
      full=ExpandPath(path, rel); // 'full' may be 'rel'
      if(StartsPath(full, path)) // proceed only if full path is inside shared path
         if(!full.is() || FullPath(full)) // if full is specified then require that it's a full path (allow empty if the client wants the drive list, otherwise this may happen if the shared path is empty but client requests something like "x" instead of "c:/")
            return true;
      full.clear(); return false;
   }

   ServerClass() {clients.replaceClass<Client>();}
}
ServerClass Server;
/******************************************************************************/
