/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void ProcPriority(Int priority)
{
#if WINDOWS_OLD
   if(priority<=-2)priority=        IDLE_PRIORITY_CLASS;else
   if(priority==-1)priority=BELOW_NORMAL_PRIORITY_CLASS;else
   if(priority== 0)priority=      NORMAL_PRIORITY_CLASS;else
   if(priority== 1)priority=ABOVE_NORMAL_PRIORITY_CLASS;else
                   priority=        HIGH_PRIORITY_CLASS;
   SetPriorityClass(GetCurrentProcess(), priority);
#elif APPLE || LINUX
   if(priority<=-2)priority= 20;else
   if(priority==-1)priority= 10;else
   if(priority== 0)priority=  0;else
   if(priority== 1)priority=-10;else
                   priority=-20;
   setpriority(PRIO_PROCESS, App.processID(), priority);
#endif
}
/******************************************************************************/
#if WINDOWS_OLD
static BOOL CALLBACK EnumWindowClose(HWND hwnd, LPARAM process_id)
{
   if(WindowProc((Ptr)hwnd)==process_id)WindowClose((Ptr)hwnd);
   return true;
}
#endif
void ProcClose(UInt id)
{
   if(id)
   {
   #if WINDOWS_OLD
      EnumWindows(EnumWindowClose, id);
   #else
      // TODO: Unix 'ProcClose'
   #endif
   }
}
Bool ProcKill(UInt id)
{
   Bool ok=false;
   if(id)
   {
   #if WINDOWS
      if(HANDLE hproc=OpenProcess(PROCESS_TERMINATE, false, id))
      {
         if(TerminateProcess(hproc, 0))ok=true;
         CloseHandle(hproc);
      }
   #else
      kill(id, SIGTERM);
      int status; pid_t ret=waitpid(id, &status, WNOHANG);
      ok=true;
   #endif
   }
   return ok;
}
Bool ProcWait(UInt id, Int milliseconds)
{
   Bool ok=true;
   if(id)
   {
   #if WINDOWS
      if(HANDLE hproc=OpenProcess(SYNCHRONIZE, false, id))
      {
         if(WaitForSingleObject(hproc, (milliseconds<0) ? INFINITE : milliseconds)==WAIT_TIMEOUT)ok=false;
         CloseHandle(hproc);
      }
   #else
      if(milliseconds<0)waitpid(id, null, 0);else
      {
         UInt start=Time.curTimeMs();
      wait:;
         int  status; pid_t pid=waitpid(id, &status, WNOHANG);
         if(pid==0)
         {
            UInt duration=Time.curTimeMs()-start; // this code was tested OK for UInt overflow
            if(  duration<milliseconds){Time.wait(1); goto wait;} // wait again
            ok=false; // timeout
         }
      }
   #endif
   }
   return ok;
}
void ProcClose(C Str &name) {       ProcClose(ProcFind(name));}
Bool ProcKill (C Str &name) {return ProcKill (ProcFind(name));}
/******************************************************************************/
#if WINDOWS_OLD
struct ProcWindowFind
{
   UInt process_id;
   HWND hwnd;
};
static BOOL CALLBACK EnumWindowFind(HWND hwnd, LPARAM proc_window_find)
{
   ProcWindowFind &pwf=*(ProcWindowFind*)proc_window_find;
   DWORD id; if(GetWindowThreadProcessId(hwnd, &id) && id==pwf.process_id)
   if(IsWindowVisible(hwnd) || IsIconic(hwnd))
   {
      pwf.hwnd=hwnd;
      return false;
   }else
   if(!pwf.hwnd) // if found a window but it is hidden, then set it only if previous was not set
   {
      pwf.hwnd=hwnd;
   }
   return true;
}
#endif
Ptr ProcWindow(UInt id)
{
   if(id)
   {
   #if WINDOWS_OLD
      ProcWindowFind pwf;
      pwf.process_id=id;
      pwf.hwnd      =null;
      EnumWindows(EnumWindowFind, LPARAM(&pwf));
      return (Ptr)pwf.hwnd;
   #else
      if(id==App.processID())return App.hwnd();
   #endif
   }
   return null;
}
/******************************************************************************/
#if WINDOWS_OLD && SUPPORT_WINDOWS_XP
static BOOL (WINAPI *QueryFullProcessImageNameW)(HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);
static Bool          QueryFullProcessImageNameTried;
#endif
Str ProcName(UInt id)
{
   if(id)
   {
   #if WINDOWS_OLD
      HANDLE    hproc=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION        , false, id); // first try accessing using PROCESS_QUERY_LIMITED_INFORMATION, it's not supported on WindowsXP, but if we're using OS in which it's supported, then it can open more processes than method below
   #if SUPPORT_WINDOWS_XP
      if(!hproc)hproc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, false, id); // if failed, then try using WindowsXP way, 'PROCESS_VM_READ' is needed here
   #endif
      if( hproc)
      {
         wchar_t name[MAX_LONG_PATH]; name[0]=0;
      #if SUPPORT_WINDOWS_XP
         DWORD dword; HMODULE hmod; if(EnumProcessModules(hproc, &hmod, SIZE(hmod), &dword))GetModuleFileNameEx(hproc, hmod, name, Elms(name)); // this requires PROCESS_QUERY_INFORMATION|PROCESS_VM_READ
         if(!name[0]) // requesting name failed, it's possible that encountered process is 64-bit and we're 32-bit, or PROCESS_QUERY_LIMITED_INFORMATION was used, and 'QueryFullProcessImageNameW' needs to be used
         {
            if(!QueryFullProcessImageNameTried)
            {
               if(HMODULE kernel=GetModuleHandle(L"Kernel32.dll"))QueryFullProcessImageNameW=(decltype(QueryFullProcessImageNameW))GetProcAddress(kernel, "QueryFullProcessImageNameW"); // available on Vista+
               QueryFullProcessImageNameTried=true;
            }
            if(QueryFullProcessImageNameW)
            {
      #endif
               DWORD size=Elms(name); QueryFullProcessImageNameW(hproc, 0, name, &size);
      #if SUPPORT_WINDOWS_XP
            }
         }
      #endif
         CloseHandle(hproc);
         return name;
      }
   #elif APPLE
      #if 1 // returns process full path file name
         int    mib[3], max_data=0;
         size_t size;

         mib[0]=CTL_KERN;
         mib[1]=KERN_ARGMAX;
         size=SIZE(max_data);
         if(!sysctl(mib, 2, &max_data, &size, null, 0))
         {
            Memt<Char8> temp; temp.setNum(max_data);

            mib[0]=CTL_KERN;
            mib[1]=KERN_PROCARGS2;
            mib[2]=id;
            size=(size_t)max_data;
            if(!sysctl(mib, 3, temp.data(), &size, null, 0))return FromUTF8(temp.data()+4);
         }
      #else // returns process window name
         ProcessSerialNumber psn; 
         if(GetProcessForPID(id, &psn)==noErr)
         {
            CFStringRef proc_name=null; CopyProcessName(&psn, &proc_name);
            if(proc_name)
            {
               Char8 name[MAX_UTF_PATH]; CFStringGetCString(proc_name, name, Elms(name), kCFStringEncodingUTF8);
               CFRelease(proc_name);
               return FromUTF8(name);
            }
         }
      #endif
   #else
      // TODO: ANDROID, LINUX 'ProcName'
   #endif
   }
   return S;
}
void ProcModules(UInt id, MemPtr<Str> modules)
{
#if WINDOWS_OLD
   if(id)
      if(HANDLE hproc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, false, id)) // these params needed for 'EnumProcessModules' and 'GetModuleFileNameEx'
   {
      Memt<HMODULE> temp; temp.setNum(temp.maxElms());
   again:;
      DWORD num=0, size=temp.elms()*temp.elmSize();
      if(EnumProcessModules(hproc, temp.data(), size, &num))
      {
         Int elms=num/temp.elmSize();
         if(num>size){temp.setNum(elms); goto again;}
         modules.setNum(elms);
         REPA(modules)
         {
            wchar_t name[MAX_LONG_PATH]; name[0]=0; GetModuleFileNameEx(hproc, temp[i], name, Elms(name));
            modules[i]=name;
         }
         return;
      }
   }
#endif
   modules.clear();
}
/******************************************************************************/
UInt ProcFind(C Str &name)
{
   UInt ret=0;
   if(name.is())
   {
      Bool drive=HasDrive(name);
   #if 0 && WINDOWS_OLD // old method, avoid since 'ProcName' should now support all cases
      HANDLE snap =CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
      if(    snap!=INVALID_HANDLE_VALUE)
      {
         PROCESSENTRY32 proc; proc.dwSize=SIZE(PROCESSENTRY32);
         if(Process32First(snap, &proc))do
         {
            if(drive){if(!EqualPath(name, ProcName(proc.th32ProcessID)))continue;}
            else     {if(!Equal    (name,          proc.szExeFile     ))continue;}
            ret=proc.th32ProcessID; break;
         }while(Process32Next(snap, &proc));
         CloseHandle(snap);
      }
   #else
      Memt<UInt> id; ProcList(id);
      REPA(id)
      {
         Str pn=ProcName(id[i]);
         if(drive){if(!EqualPath(name,          pn ))continue;}
         else     {if(!Equal    (name, _GetBase(pn)))continue;}
         ret=id[i]; break;
      }
   #endif
   }
   return ret;
}
/******************************************************************************/
void ProcList(MemPtr<UInt> id)
{
#if WINDOWS_OLD
   Memt<DWORD> temp; temp.setNum(temp.maxElms());
again:;
   DWORD num=0, size=temp.elms()*temp.elmSize();
   if(EnumProcesses(temp.data(), size, &num))
   {
      if(num>=size){temp.addNum(Max(1, temp.elms()/4)); goto again;} // according to docs and tests, 'num' will be equal to 'size' if there are more processes that can fit into the buffer
      ASSERT(SIZE(DWORD)==SIZE(UInt)); id.setNum(num/temp.elmSize()).copyFrom((UInt*)temp.data());
      return;
   }
#elif APPLE
   size_t size=0;
   int    names[]={CTL_KERN, KERN_PROC, KERN_PROC_ALL},
          code=sysctl(names, Elms(names), null, &size, null, 0); // first call must be with null because it will fail with no info
   if(!code && size>0)
   {
      Memt<kinfo_proc> temp; temp.setNum(Max(temp.maxElms(), Int(size/temp.elmSize()*5/4))); // allocate a bit more
   again:;
      size_t size2=temp.elms()*temp.elmSize();
             code =sysctl(names, Elms(names), temp.data(), &size2, null, 0);
      if(!code && !size2 && size){size=0; temp.addNum(Max(1, temp.elms()/2)); goto again;} // if not enough memory then try again
      if(!code &&  size2>0)
      {
         id.setNum(size2/temp.elmSize()); REPAO(id)=temp[i].kp_proc.p_pid;
         return;
      }
   }
#else
   // TODO: ANDROID, LINUX 'ProcList'
#endif
   id.clear();
}
/******************************************************************************/
// CONSOLE PROCESS
/******************************************************************************/
#if WINDOWS_OLD
Bool ConsoleProcess::created(                )C {return _proc!=null;}
Bool ConsoleProcess::active (                )C {return _proc!=null && _thread.active();}
Bool ConsoleProcess::wait   (Int milliseconds)  {return _proc==null || _thread.wait  (milliseconds);}
void ConsoleProcess::stop   (                )  {ProcClose(_proc_id);}
void ConsoleProcess::kill   (                )
{
   if(_proc)
   {
      TerminateProcess(_proc, 0);
      del();
   }
}
/******************************************************************************/
static Bool ConsoleProcessFunc(Thread &thread)
{
   ConsoleProcess &cp=*(ConsoleProcess*)thread.user;
   SyncLockerEx locker(cp._lock);
   if(cp._proc && cp._out_read)
   {
      HANDLE temp     =null;
      UInt   proc_id  =cp._proc_id;
      DWORD  available=0;
      Bool   active   =(WaitForSingleObject(cp._proc, 0)==WAIT_TIMEOUT);
      if(   !active)
      {
         DWORD exit_code=0; GetExitCodeProcess(cp._proc, &exit_code); // warning: process may have used 'STILL_ACTIVE' for the 'ExitProcess'
         cp._exit_code=((exit_code==STILL_ACTIVE) ? -1 : exit_code);
      }
      if(PeekNamedPipe(cp._out_read, null, 0, null, &available, null)) // check if there's any data available, this is to prevent freezes in 'ReadFile' when there's nothing available
         if(available)
            if(DuplicateHandle(GetCurrentProcess(), cp._out_read, GetCurrentProcess(), &temp, 0, true, DUPLICATE_SAME_ACCESS))
      {
         Char8 buf[65536+1]; MIN(available, SIZE(buf)-1);
      read_again:;
         locker.off();
         DWORD read=0; ReadFile(temp, buf, available, &read, null); // read without 'SyncLock' on handle duplicate
         if(   read)
         {
            buf[read]='\0'; if(!cp._binary)ReplaceSelf(buf, '\r', '\0');
            locker.on();
            if(proc_id==cp._proc_id) // if it's still the same process then append the data
            {
               if(!cp._binary)cp._data+=buf;else // append string in text mode
               {
                  Int length=cp._data.length()+read, total=length+1; // get length, and total size (+ null character)
                  if( total >cp._data._d.elms())cp._data._d.setNum(total);
                  CopyFast(cp._data._d.data()+cp._data.length(), buf, read+1); // append read data (including null character)
                  cp._data._length=length; // adjust length
               }
               if(!active && read==available){available=SIZE(buf)-1; goto read_again;} // if the process ended and full buffer was read then read again (there will be no delay this time) because we must access all output before closing the thread
            }else active=true; // if process changed then set active to true because we want the thread to keep on running and check for new process data in next step
            locker.off();
         }
         CloseHandle(temp);
      }
      if(active)
      {
      #if HAS_THREADS
         locker.off();
         Time.wait(1);
      #endif
         return true;
      }
   }
   return false;
}
void ConsoleProcess::del()
{
   stop();

   SyncLocker locker(_lock);
      _data.clear();
      _exit_code=-1; _binary=false;
      _proc_id=0;
   if(_proc){CloseHandle(_proc); _proc=null;}

   if(_out_read){CloseHandle(_out_read); _out_read=null;}
   if(_in_write){CloseHandle(_in_write); _in_write=null;}
}
Bool ConsoleProcess::create(C Str &name, C Str &params, Bool hidden, Bool binary)
{
   del();
   if(name.is())
   {
      HANDLE out_write=null,
              in_read =null;

      // Set up the security attributes struct
      SECURITY_ATTRIBUTES sa; Zero(sa);
      sa.nLength=SIZE(sa);
      sa.bInheritHandle=TRUE;

      SyncLockerEx locker(_lock);
      if(CreatePipe(&_out_read, &out_write, &sa, 0)) // Create the child output pipe
      {
         if(SetHandleInformation(_out_read, HANDLE_FLAG_INHERIT, 0)) // Ensure the read handle to the pipe for STDOUT is not inherited
         if(CreatePipe(&in_read, &_in_write, &sa, 0)) // Create the child input pipe
         {
            if(SetHandleInformation(_in_write, HANDLE_FLAG_INHERIT, 0)) // Ensure the write handle to the pipe for STDIN is not inherited
            {
               locker.off();

               PROCESS_INFORMATION pi;
               STARTUPINFOW        si; Zero(si); si.cb=SIZE(si);
               si.dwFlags   =STARTF_USESTDHANDLES;
               si.hStdInput =in_read;
               si.hStdOutput=out_write;
               si.hStdError =out_write;
               if(hidden)
               {
                  si.dwFlags   |=STARTF_USESHOWWINDOW;
                  si.wShowWindow=SW_HIDE;
               }

               Str app=name, base=GetBase(app), cmd, cur_dir;
               if(GetExt(app)=="bat" || GetExt(app)=="cmd")
               {
                  cur_dir=GetPath(app);
                  cmd    =(Contains(base, ' ') ? S+"/c \""+base+'"' : S+"/c "+base); // must use quotes only when needed, otherwise this will fail
                  wchar_t comspec[MAX_LONG_PATH]; comspec[0]='\0'; if(GetEnvironmentVariable(L"COMSPEC", comspec, Elms(comspec)))app=comspec;
                  //cmd =S+"/c "+name+"";
                  //cmd =S+"\"/c "+name+"\"";//+' '+params+"\"";
                  //cmd =S+"/c \""+name+' '+params+"\"";
                  //cmd =S+"/c \""+name+"\"";
                  //cmd =S+"/c \""+name+"\" "+params+"";
                  //cmd =S+"\"cmd.exe\" /c \""+name+"\"";
                  //name="cmd.exe";
                  //name="cmd";
                  //name=S;
                  //cmd =S+'"'+name+'"';
               }else cmd=base;
               if(params.is())cmd.space()+=params;
               Memt<Char> cmd_temp; cmd_temp.setNum(cmd.length()+1); Set(cmd_temp.data(), cmd, cmd_temp.elms()); // can be very long, copy to 'cmd_temp' because 'CreateProcessW' can modify it
               if(CreateProcessW(app, WChar(cmd_temp.data()), null, null, TRUE, CREATE_NEW_CONSOLE, null, cur_dir, &si, &pi))
               {
                  locker.on();
                 _proc   =pi.hProcess;
                 _proc_id=pi.dwProcessId;
                 _binary =binary;
                  locker.off();
                  if(!_thread.active() || _thread.wantStop())_thread.create(ConsoleProcessFunc, this);
               }
            }
            CloseHandle(in_read); in_read=null;
         }
         CloseHandle(out_write); out_write=null;
      }
   }
   return _proc!=null;
}
Bool ConsoleProcess::createMem(C Str &script, C Str &cur_dir, Bool hidden, Bool binary)
{
   del();
   if(script.is())
   {
      wchar_t comspec[MAX_LONG_PATH]; comspec[0]='\0'; if(GetEnvironmentVariable(L"COMSPEC", comspec, Elms(comspec)))
      {
         HANDLE out_write=null,
                 in_read =null;

         // Set up the security attributes struct
         SECURITY_ATTRIBUTES sa; Zero(sa);
         sa.nLength=SIZE(sa);
         sa.bInheritHandle=TRUE;

         SyncLockerEx locker(_lock);
         if(CreatePipe(&_out_read, &out_write, &sa, 0)) // Create the child output pipe
         {
            if(SetHandleInformation(_out_read, HANDLE_FLAG_INHERIT, 0)) // Ensure the read handle to the pipe for STDOUT is not inherited
            if(CreatePipe(&in_read, &_in_write, &sa, 0)) // Create the child input pipe
            {
               if(SetHandleInformation(_in_write, HANDLE_FLAG_INHERIT, 0)) // Ensure the write handle to the pipe for STDIN is not inherited
               {
                  locker.off();

                  PROCESS_INFORMATION pi;
                  STARTUPINFOW        si; Zero(si); si.cb=SIZE(si);
                  si.dwFlags   =STARTF_USESTDHANDLES;
                  si.hStdInput =in_read;
                  si.hStdOutput=out_write;
                  si.hStdError =out_write;
                  if(hidden)
                  {
                     si.dwFlags   |=STARTF_USESHOWWINDOW;
                     si.wShowWindow=SW_HIDE;
                  }

                  Str cmd=S+"/c "+script; cmd.replace('\r', '\0').replace('\n', '&'); // commands need to be separated with '&' character
                  Memt<Char> cmd_temp; cmd_temp.setNum(cmd.length()+1); Set(cmd_temp.data(), cmd, cmd_temp.elms()); // can be very long, copy to 'cmd_temp' because 'CreateProcessW' can modify it
                  if(CreateProcessW(comspec, WChar(cmd_temp.data()), null, null, TRUE, CREATE_NEW_CONSOLE, null, cur_dir, &si, &pi))
                  {
                     locker.on();
                    _proc   =pi.hProcess;
                    _proc_id=pi.dwProcessId;
                    _binary =binary;
                     locker.off();
                     if(!_thread.active() || _thread.wantStop())_thread.create(ConsoleProcessFunc, this);
                  }
               }
               CloseHandle(in_read); in_read=null;
            }
            CloseHandle(out_write); out_write=null;
         }
      }
   }
   return _proc!=null;
}
/******************************************************************************/
#elif WINDOWS_NEW
Bool ConsoleProcess::created  (                )C {return false;}
Bool ConsoleProcess::active   (                )C {return false;}
Bool ConsoleProcess::wait     (Int milliseconds)  {return false;}
void ConsoleProcess::stop     (                )  {}
void ConsoleProcess::kill     (                )  {}
void ConsoleProcess::del      (                )  {}
Bool ConsoleProcess::create   (C Str &name  , C Str &params , Bool hidden, Bool binary) {return false;}
Bool ConsoleProcess::createMem(C Str &script, C Str &cur_dir, Bool hidden, Bool binary) {return false;}
#else
#define FD_READ  0
#define FD_WRITE 1
static pid_t popen2(CChar8 *cur_dir, CChar8 *command, int *in_fd, int *out_fd)
{
   if( in_fd)* in_fd=0;
   if(out_fd)*out_fd=0;

   int p_stdin[2], p_stdout[2];
   if(pipe(p_stdin ))return -1;
   if(pipe(p_stdout)){close(p_stdin[0]); close(p_stdin[1]); return -1;}
   pid_t pid=fork();
   if(pid==0) // this code gets executed on the child process
   {
      close(p_stdin [FD_WRITE]); dup2(p_stdin [FD_READ ],  STDIN_FILENO);
      close(p_stdout[FD_READ ]); dup2(p_stdout[FD_WRITE], STDOUT_FILENO);
                                 dup2(p_stdout[FD_WRITE], STDERR_FILENO);
      if(Is(cur_dir))chdir(cur_dir);
      execl("/bin/sh", "sh", "-c", command, __null);
      perror(null);
     _exit(0);
   }else
   if(pid<0) // error
   {
      close(p_stdin[0]); close(p_stdout[0]);
      close(p_stdin[1]); close(p_stdout[1]);
      pid=0; // return 0
   }else // success
   {
      if( in_fd)* in_fd=p_stdin [FD_WRITE];else close(p_stdin [FD_WRITE]);
      if(out_fd)*out_fd=p_stdout[FD_READ ];else close(p_stdout[FD_READ ]);
   }
   return pid;
}
Bool ConsoleProcess::created(                )C {return _proc_id!=0;}
Bool ConsoleProcess::active (                )C {return _proc_id!=0 && _thread.active();}
Bool ConsoleProcess::wait   (Int milliseconds)  {return _proc_id==0 || _thread.wait  (milliseconds);}
void ConsoleProcess::stop   () {} // do nothing
void ConsoleProcess::kill   () {if(_proc_id){ProcKill(_proc_id); del();}}
void ConsoleProcess::del    ()
{
   stop();

   SyncLocker locker(_lock);
  _data.clear();
  _exit_code=-1; _binary=false;
  _proc_id=0;

   if(_out_read){close(_out_read); _out_read=0;}
   if(_in_write){close(_in_write); _in_write=0;}
}
static Bool ConsoleProcessFunc(Thread &thread)
{
   ConsoleProcess &cp=*(ConsoleProcess*)thread.user;
   SyncLockerEx locker(cp._lock);
   if(cp._proc_id && cp._out_read)
   {
      UInt proc_id=cp._proc_id;
      int  status; pid_t pid=waitpid(proc_id, &status, WNOHANG);
      Bool active=(pid==0);
      if( !active)cp._exit_code=(WIFEXITED(status) ? WEXITSTATUS(status) : -1);
      // read
      {
         int temp =dup(cp._out_read);
         if( temp>=0)
         {
         read_again:;
            locker.off();

            fd_set fd; int time=0;
         #if WINDOWS
            fd.fd_count=1; fd.fd_array[0]=temp;
         #else
            FD_ZERO(&fd); FD_SET(temp, &fd);
         #endif
            timeval tv; tv.tv_sec=time/1000; tv.tv_usec=(time%1000)*1000;
            if(select(temp+1, &fd, null, null, &tv)>0) // if there's data available
            {
               Char8 buf[65536+1]; Int to_read=SIZE(buf)-1; // make room for '\0'
            #if APPLE // do this only on Apple, as on Linux 'fstat' will give zero even if there's data available
               struct stat s; fstat(temp, &s); MIN(to_read, s.st_size); // limit reading to remaining file size, to avoid freezes when trying to read from empty file
            #endif
               Int r=read(temp, buf, to_read);
               if( r>0)
               {
                  buf[r]='\0'; if(!cp._binary)ReplaceSelf(buf, '\r', '\0');
                  locker.on();
                  if(proc_id==cp._proc_id) // if it's still the same process then append the data
                  {
                     if(!cp._binary)cp._data+=buf;else // append string in text mode
                     {
                        Int length=cp._data.length()+r, total=length+1; // get length, and total size (+ null character)
                        if( total >cp._data._d.elms())cp._data._d.setNum(total);
                        CopyFast(cp._data._d.data()+cp._data.length(), buf, r+1); // append read data (including null character)
                        cp._data._length=length; // adjust length
                     }
                     if(!active && r==to_read)goto read_again;// if the process ended and full buffer was read then read again (there will be no delay this time) because we must access all output before closing the thread
                  }else active=true; // if process changed then set active to true because we want the thread to keep on running and check for new process data in next step
                  locker.off();
               }
            }
            close(temp);
         }
      }
      if(active)
      {
      #if HAS_THREADS
         locker.off();
         Time.wait(1);
      #endif
         return true;
      }
   }
   return false;
}
Bool ConsoleProcess::create(C Str &name, C Str &params, Bool hidden, Bool binary)
{
   del();
   if(name.is())
   {
      Str8 cur_dir=UnixPathUTF8(S), command=S+'"'+UnixPathUTF8(name)+'"';
      if(params.is())command.space()+=UnixPathUTF8(params);
      SyncLockerEx locker(_lock);
      if(_proc_id=popen2(cur_dir, command, null, &_out_read))
      {
        _binary=binary;
         locker.off();
         if(!_thread.active() || _thread.wantStop())_thread.create(ConsoleProcessFunc, this);
      }
   }
   return _proc_id!=0;
}
Bool ConsoleProcess::createMem(C Str &script, C Str &cur_dir, Bool hidden, Bool binary) {del(); return false;}
#endif
/******************************************************************************/
Str ConsoleProcess::get()
{
   if(_data.is())
   {
      SyncLocker locker(_lock);
      if(_binary)
      {
         Str temp;
         temp._length  =_data.length();
         temp._d.setNum(_data.length()+1); temp._d.last()='\0'; REPA(_data)temp._d[i]=_data._d[i]; // do raw copy of 'Char8' to 'Char' to avoid code-page changes, can't do 'CopyFastN' because we're copying from 'Str8' to 'Str'
         return temp;
      }else
   #if WINDOWS
      if(1) // Windows uses Code Pages
      {
         Str s; Char8 last=_data.last();
         if((last&0x80) && active())_data.removeLast();else last='\0'; // possibly beginning of a multi-byte character, and the process is still active (which may generate more characters) then don't process this char now (remove it from '_data' and keep it in 'last' to restore later), otherwise clear it
         if(Int length=_data.length())
         {
            Int size=MultiByteToWideChar(CP_ACP, 0, _data(), length, null, 0); if(size>0)
            {
               s.reserve(size);
               if(size==MultiByteToWideChar(CP_ACP, 0, _data(), length, WChar(s._d.data()), s._d.elms()))s._d[s._length=size]='\0';else s.clear();
            }
         }
        _data.clear()+=last; // restore what was removed
         return s;
      }else
   #endif
      { // UTF8
         /* We need to skip UTF8 chars that aren't complete:
         UTF8 is encoded in following way:
         Bytes Byte0    Byte1    Byte2
         1     0xxxxxxx
         2     110xxxxx 10xxxxxx          - this needs fixing if we're missing last 1 byte
         3     1110xxxx 10xxxxxx 10xxxxxx - this needs fixing if we're missing last 2 bytes
         */
         Int   skip=0;
         Char8 last=_data.last(), temp[2];
         if(   last&0x80)
         {
            if((last&0xE0)==0xC0)skip=1;else // 110xxxxx, missing
            if((last&0xF0)==0xE0)skip=1;else // 1110xxxx, missing, missing
            if((_data[_data.length()-2]&0xF0)==0xE0)skip=2; // 1110xxxx, available, missing
            REP(skip){temp[i]=_data.last(); _data.removeLast();}
         }
         Str str=FromUTF8(_data); _data.clear(); FREP(skip)_data+=temp[i]; return str;
      }
   }
   return S;
}
/******************************************************************************/
#if WINDOWS_OLD
#if 0
}
#include "../../../ThirdPartyLibs/begin.h"
#include <SubAuth.h>
#include "../../../ThirdPartyLibs/end.h"
namespace EE{
#endif

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

#define DUPLICATE_SAME_ATTRIBUTES 0x00000004

#define SystemHandleInformation 16

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

enum OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectTypesInformation,
	ObjectHandleFlagInformation,
	ObjectSessionInformation,
	MaxObjectInfoClass
};
typedef struct _SYSTEM_HANDLE
{
    ULONG ProcessId;
    BYTE ObjectTypeNumber;
    BYTE Flags;
    USHORT Handle;
    PVOID Object;
    ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
    ULONG HandleCount;
    SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef enum _POOL_TYPE
{
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS
} POOL_TYPE, *PPOOL_TYPE;

typedef struct _OBJECT_BASIC_INFORMATION
{
	ULONG Attributes;
	ACCESS_MASK GrantedAccess;
	ULONG HandleCount;
	ULONG PointerCount;
	ULONG PagedPoolCharge;
	ULONG NonPagedPoolCharge;
	ULONG Reserved[3];
	ULONG NameInfoSize;
	ULONG TypeInfoSize;
	ULONG SecurityDescriptorSize;
	LARGE_INTEGER CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING Name;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG TotalPagedPoolUsage;
    ULONG TotalNonPagedPoolUsage;
    ULONG TotalNamePoolUsage;
    ULONG TotalHandleTableUsage;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG HighWaterPagedPoolUsage;
    ULONG HighWaterNonPagedPoolUsage;
    ULONG HighWaterNamePoolUsage;
    ULONG HighWaterHandleTableUsage;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccess;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    USHORT MaintainTypeList;
    POOL_TYPE PoolType;
    ULONG PagedPoolUsage;
    ULONG NonPagedPoolUsage;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

static Byte             HANDLE_TYPE_PROCESS;
static NTSTATUS (NTAPI *NtQuerySystemInformation)(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
static NTSTATUS (NTAPI *NtDuplicateObject       )(HANDLE SourceProcessHandle, HANDLE SourceHandle, HANDLE TargetProcessHandle, PHANDLE TargetHandle, ACCESS_MASK DesiredAccess, ULONG Attributes, ULONG Options);
static NTSTATUS (NTAPI *NtQueryObject           )(HANDLE ObjectHandle, ULONG ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);

static Int Compare(C SYSTEM_HANDLE &a, C SYSTEM_HANDLE &b) {return Compare((UInt)a.ProcessId, (UInt)b.ProcessId);}
#endif

Bool GetProcessesAccessingThisProcess(MemPtr<ProcessAccess> proc, Bool write_only, Mems<Byte> *temp)
{
   proc.clear();
#if WINDOWS_OLD
   if(!HANDLE_TYPE_PROCESS)
   {
      if(HMODULE ntdll=GetModuleHandle(L"ntdll.dll"))
      if(NtQueryObject           =(decltype(NtQueryObject           ))GetProcAddress(ntdll, "NtQueryObject"))
      if(NtDuplicateObject       =(decltype(NtDuplicateObject       ))GetProcAddress(ntdll, "NtDuplicateObject"))
      if(NtQuerySystemInformation=(decltype(NtQuerySystemInformation))GetProcAddress(ntdll, "NtQuerySystemInformation")) // set this as last so by checking 'NtQuerySystemInformation' below, we know that all are loaded
         {}
      // set type after handles were obtained
      VecI4 ver=OSVerNumber(); // WinXP is 5.1, Vista is 6.0, Win7 is 6.1, Win8 is 6.2, Win8.1 is 6.3, Win10 is 10
      if(Compare(ver, VecI4(6, 1, 0, 0))>=0)HANDLE_TYPE_PROCESS=7;else // Win7/8/10 - https://cybercoding.wordpress.com/2011/08/20/delphi-process-detection-handle-table-enumeration/ "In Windows 7 Sp1 ObjectTypeNumber = 7 is Process!"
      if(Compare(ver, VecI4(6, 0, 0, 0))>=0)HANDLE_TYPE_PROCESS=6;else // Vista
                                            HANDLE_TYPE_PROCESS=5;     // WinXP

   // HANDLE_TYPE_FILE - Windows XP 28, Windows Vista 28, Windows 7 28, Windows 8 31, Windows 8.1 30 - https://www.codeproject.com/Tips/992827/Section-Handles-Enumeration-Extending-File-Unlocki
   }
   if(NtQuerySystemInformation)
   {
      Mems<Byte> temp_local; if(!temp)temp=&temp_local;
   again:
      ULONG size=0; NTSTATUS status=NtQuerySystemInformation(SystemHandleInformation, temp->data(), temp->elms(), &size);
      if(status==STATUS_INFO_LENGTH_MISMATCH)
      {
         MAX(size, temp->elms()); // maximize with 'temp' in case 'size' is left at zero
         temp->setNum(Max(65536, UInt(size+size/4))); // max with 65536 in case 'size' is 0, add extra size/4 in case there will be needed more memory in the next call than what was now
         goto again;
      }
      if(NT_SUCCESS(status))
      {
         UInt pid=0;
         HANDLE proc_handle=null, cur_proc=GetCurrentProcess(); // 'cur_proc' doesn't need to be closed
         SYSTEM_HANDLE_INFORMATION &shi=*(SYSTEM_HANDLE_INFORMATION*)temp->data();
         SYSTEM_HANDLE *handles=shi.Handles; Int handle_count=Min((UInt)shi.HandleCount, INT_MAX);
       //Sort(handles, handle_count, Compare); //compare by PID so we can obtain process handle one by one, no need to do this because handle array is groupped by PID's (handles with for the same process are located together)
         const UInt read_access=PROCESS_VM_READ , // |GENERIC_READ |READ_CONTROL,
                   write_access=PROCESS_VM_WRITE     |GENERIC_WRITE|WRITE_DAC|WRITE_OWNER,
                         access=(write_only ? write_access : read_access|write_access);
         FREP(handle_count)
         {
          C SYSTEM_HANDLE &handle=handles[i];
            if(handle.ObjectTypeNumber==HANDLE_TYPE_PROCESS) // check only "Process" handles
            if(handle.GrantedAccess&access) // only that have desired access
            if(handle.ProcessId!=App.processID()) // skip self
            {
               if(pid!=handle.ProcessId)
               {
                  pid=handle.ProcessId;
                  CloseHandle(proc_handle);
                  proc_handle=OpenProcess(PROCESS_DUP_HANDLE, false, pid);
               }
               if(proc_handle)
               {
                  HANDLE dup_handle=null;
                                 NtDuplicateObject(proc_handle, HANDLE(handle.Handle), cur_proc, &dup_handle, PROCESS_QUERY_LIMITED_INFORMATION, 0, 0); // don't use DUPLICATE_SAME_ATTRIBUTES because exception can occur when trying to release handles that were "protected from close via NtSetInformationObject", don't use DUPLICATE_SAME_ACCESS because if handle would have write permission then probably we would have to, but we don't want to get write permission, but only check the PID, first try accessing using PROCESS_QUERY_LIMITED_INFORMATION, it's not supported on WindowsXP, but if we're using OS in which it's supported, then it can open more processes than method below
               #if SUPPORT_WINDOWS_XP
                  if(!dup_handle)NtDuplicateObject(proc_handle, HANDLE(handle.Handle), cur_proc, &dup_handle, PROCESS_QUERY_INFORMATION        , 0, 0); // don't use DUPLICATE_SAME_ATTRIBUTES because exception can occur when trying to release handles that were "protected from close via NtSetInformationObject", don't use DUPLICATE_SAME_ACCESS because if handle would have write permission then probably we would have to, but we don't want to get write permission, but only check the PID, if failed, then try using WindowsXP way, 'PROCESS_VM_READ' is not needed here
               #endif
                  if( dup_handle)
                  {
                     if(GetProcessId(dup_handle)==App.processID()) // if accessing our process
                     {
                        ProcessAccess &p=proc.New();
                        p.proc_id=pid;
                        p.write=FlagTest(handle.GrantedAccess, write_access);
                     }

                  #if 0
                     // !! don't check type/name for HANDLE_TYPE_FILE with handle.GrantedAccess==0X0012019F or it may freeze - https://forum.sysinternals.com/topic18892.html !!
                     {
                        union
                        {
                           OBJECT_TYPE_INFORMATION  type;
                           OBJECT_BASIC_INFORMATION basic;
                           UNICODE_STRING           name;
                           Byte                     temp[SIZE(OBJECT_TYPE_INFORMATION)+MAX_LONG_PATH*SIZE(Char)];
                        }u;
                        if(NT_SUCCESS(NtQueryObject(dup_handle, ObjectTypeInformation, &u.type, SIZE(u), null)))
                        {
                        // some known types: "Key", "Event", "Semaphore", "Timer", "WaitCompletionPacket", "IoCompletion", "ALPC Port", "IRTimer", "TpWorkerFactory", "DxgkSharedSyncObject", "WindowStation", "Process"
                           if(Equal(u.type.Name.Buffer, "Thread", true))
                           {
                           #undef GetThreadId
                              Handles.add(S+ProcName(pid)+", Thread:"+(UInt)GetThreadId(dup_handle));
                           #define GetThreadId _GetThreadId
                           }
                        }
                        if(NT_SUCCESS(NtQueryObject(dup_handle, ObjectNameInformation, &u.name, SIZE(u), null)))
                        {
                           int z=0;
                        }
                        if(NT_SUCCESS(NtQueryObject(dup_handle, ObjectBasicInformation, &u.basic, SIZE(u.basic), null)))
                        {
                           int z=0;
                        }
                     }
                  #endif
                     CloseHandle(dup_handle);
                  }
               }
            }
         }
         CloseHandle(proc_handle);
         return true;
      }
   }
#endif
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
