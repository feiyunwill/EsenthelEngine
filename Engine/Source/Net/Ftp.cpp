/******************************************************************************/
#include "stdafx.h"
namespace EE{
#define FTP_WAIT_TIME (8*1000) // 8 seconds
#define FTP_INTERRUPT 1 // if separate long transfers into few smaller ones
/******************************************************************************/
enum
{
   LO=-2,      // logged in
   ER=-1,      // error
   NUMLOGIN=9, // 9 login sequences
};
static SByte LogInSequence[NUMLOGIN][18]=
{
   // this array stores all of the login sequences for the various firewalls in blocks of 3 numbers
   // 1st number is command to send
   // 2nd number is next point in login sequence array if 200 series response is received from server as the result of the command
   // 3rd number is next point in login sequence array if 300 series response is received from server as the result of the command

   { 0,LO,3,    1,LO, 6,   2,LO,ER                                 }, // no firewall
   { 3, 6,3,    4, 6,ER,   5,ER, 9,   0,LO,12,   1,LO,15,   2,LO,ER}, // SITE hostname
   { 3, 6,3,    4, 6,ER,   6,LO, 9,   1,LO,12,   2,LO,ER           }, // USER after login
   { 7, 3,3,    0,LO, 6,   1,LO, 9,   2,LO,ER                      }, // proxy OPEN
   { 3, 6,3,    4, 6,ER,   0,LO, 9,   1,LO,12,   2,LO,ER           }, // Transparent
   { 6,LO,3,    1,LO, 6,   2,LO,ER                                 }, // USER with no login
   { 8, 6,3,    4, 6,ER,   0,LO, 9,   1,LO,12,   2,LO,ER           }, // USER fireID@remotehost
   { 9,ER,3,    1,LO, 6,   2,LO,ER                                 }, // USER remoteID@remotehost fireID
   {10,LO,3,   11,LO, 6,   2,LO,ER                                 }, // USER remoteID@fireID@remotehost
};
/******************************************************************************/
Ftp::Ftp()
{
  _binary  =false;
  _abort   =false;
  _timeout =false;
  _timeouts=false;
  _progress= 0;
  _total   =-1;
  _port    = 0;
}
/******************************************************************************/
Bool Ftp::login(C Str8 &host, C Str8 &user, C Str8 &password, Bool ignore_auth_result)
{
   logout();

   T._host    =host;
   T._user    =user;
   T._password=password;

   CChar8 *url=host;
   Bool    secure=false;
   if(StartsPath(url, "ftps://"))
   {
      url+=7; T._port=21; // 990 = is the implicit FTPS port however not working on one tested server, so 21 with explicit is used
      secure=true;
   }else
   if(StartsPath(url, "ftp://"))
   {
      url+=6; T._port=21; // 21 = default FTP port
   }else // assume FTP
   {
      T._port=21; // 21 = default FTP port
   }

   {
      Char8 *account=null,
            *fw_user=null,
            *fw_pass=null;
      Str8 host_port=url; if(_port!=21){host_port+=':'; host_port+=_port;} // add port to the host

      // connect
      SockAddr addr; if(!addr.setHost(url, _port))goto error;

         _socket.createTcp(addr);
      if(_socket.connect  (addr)!=Socket::CONNECTED)goto error;

      if(response(FTP_WAIT_TIME).first()!='2')goto error;

      if(secure)
      {
         // send command
         Char8 reply=command("AUTH TLS").first(); if(reply!='2' && reply!='3')goto error;

         if(!_socket.secure(url))goto error;
             _socket.setDefaultFunc();
         for(; !_abort; )switch(_socket.handshake())
         {
            case SecureSocket::OK: goto ok;

            case SecureSocket::NEED_FLUSH: _socket.flush(DOWNLOAD_WAIT_TIME); break; // flush quickly and break to check '_abort'
            case SecureSocket::NEED_WAIT : _socket.wait (DOWNLOAD_WAIT_TIME); break; // wait  quickly and break to check '_abort'

            case SecureSocket::BAD_CERT: if(ignore_auth_result)goto ok; goto error;
            default: goto error;
         }
         ok:;
      }

      // go through appropriate log in procedure
      for(Int i=0; ; )
      {
         Str8 cmd;
         switch(LogInSequence[0][i])
         {
            case  0: cmd=S+"USER "+user    ; break;
            case  1: cmd=S+"PASS "+password; break;
            case  2: cmd=S+"ACCT "+account ; break;
            case  3: cmd=S+"USER "+fw_user ; break;
            case  4: cmd=S+"PASS "+fw_pass ; break;
            case  5: cmd=S+"SITE "+host_port; break;
            case  6: cmd=S+"USER "+user+'@'+host_port; break;
            case  7: cmd=S+"OPEN "+host_port         ; break;
            case  8: cmd=S+"USER "+fw_user +'@'+host_port            ; break;
            case  9: cmd=S+"USER "+user    +'@'+host_port+' '+fw_user; break;
            case 10: cmd=S+"USER "+user    +'@'+fw_user+'@'+host_port; break;
            case 11: cmd=S+"PASS "+password+'@'+fw_pass              ; break;
         }

         // send command
         Char8 reply=command(cmd).first(); if(reply!='2' && reply!='3')goto error;

         // get next command
         switch(i=LogInSequence[0][i + (reply-'0')-1])
         {
            case ER: goto error;
            case LO: if(!changeDir("/"))goto error; return true;
         }
      }
   }
error:
   logout(); return false;
}
void Ftp::logout()
{
   if(is())command("QUIT");
  _socket.del();
  _binary  =false;
  _abort   =false;
  _timeout =false;
  _timeouts=false;
  _progress= 0;
  _total   =-1;
  _port    = 0;
  _response.clear();
  _host    .clear();
  _user    .clear();
  _password.clear();
}
Bool Ftp::reconnect() // warning: this does not preserve the binary mode !!
{
   if(_host.is())
   {
      Str8 host, user, password; // move to temps because 'login' calls 'logout' which clears these values
      Swap(T._host    , host);
      Swap(T._user    , user);
      Swap(T._password, password);
      return login(host, user, password);
   }
   return true;
}
/******************************************************************************/
Bool Ftp::send(SecureSocket &socket, CPtr data, Int size)
{
   Int   sent=0;
   for(; sent<size && !_abort; )
   {
      if(  !socket.flush(FTP_WAIT_TIME))break;
      Int s=socket.send ((Byte*)data+sent, size-sent);
      if( s>0)sent+=s;else switch(s)
      {
         case SecureSocket::NEED_WAIT : if(socket.wait(FTP_WAIT_TIME))continue; goto error;
         case SecureSocket::NEED_FLUSH:                               continue; goto error; // just continue because there's 'flush' at the start of the loop
         default                      :                                         goto error;
      }
   }
error:
   return sent==size;
}
static void AdjustTime(Int &time, UInt &start_time)
{
   if(time>0)
   {
      UInt cur_time=Time.curTimeMs();
      Int  delta=cur_time-start_time; // this code was tested OK for UInt overflow, if 'start_time' is UINT_MAX and 'cur_time' is 0, then 'delta' will be 1
      start_time=cur_time;
      MAX(time-=delta, 0); // don't wait infinity
   }
}
Str8 Ftp::response(Int time)
{
   UInt start_time; if(time>0)start_time=Time.curTimeMs();
   Memt<Char8, 2*1024> out;

try_again:

   // get single response
   Int line_pos=0;
   {
   try_next_line:
      Int new_line =TextPosI(_response()+line_pos, '\n');
      if( new_line>=0)
      {
         new_line+=line_pos+1; // go to next character after '\n'
         if(_response[line_pos+3]==' ') // we've found a full response
         {
            out.setNum(new_line+1); Set(out.data(), _response, out.elms()); _response.remove(0, new_line); return out.data();
         }
         line_pos=new_line;
         goto try_next_line;
      }
   }

   // read from server
wait_again:
   if(_socket.wait(time))
   {
      Char8 buf[10*1024];
      Int   r=_socket.receive(buf, SIZE(buf)-1);
      AdjustTime(time, start_time);
      if(r>0)
      {
         buf[r]='\0';
        _response+=buf;
         goto try_again;
      }else switch(r)
      {
         case SecureSocket::NEED_WAIT :                                                       goto wait_again; // just continue because there's 'wait' at the start of the loop
         case SecureSocket::NEED_FLUSH: if(_socket.flush(time)){AdjustTime(time, start_time); goto wait_again;} break;
      }
   }

   // return what we've got
   out.setNum(_response.length()+1); Set(out.data(), _response, out.elms()); _response.clear(); return out.data();
}
Str8 Ftp::command(Str8 cmd)
{
  _abort=false;
   cmd+="\r\n";
   if(send(_socket, cmd(), cmd.length()))return response(_timeout ? 0 : FTP_WAIT_TIME);
   return S;
}
Bool Ftp::mode(Bool binary)
{
       if(binary!=T._binary)if(command(binary ? "TYPE I" : "TYPE A N").first()=='2')T._binary=binary;
   return binary==T._binary;
}
/******************************************************************************/
Bool Ftp::noop()
{
   return is() ? command("NOOP").first()=='2' : false;
}
Long Ftp::fileSize(C Str &file)
{
   if(is())
   {
      Str8 size=command(S+"SIZE "+UnixPathUTF8(file));
      if(  size.length()>=5)return TextLong(size()+4);
   }
   return -1;
}
Bool Ftp::fileTime(C Str &file, DateTime &dt)
{
   if(is())
   {
      Str8 time=command(S+"MDTM "+UnixPathUTF8(file));
      if(  time.length()>=18)
      {
         Char8   temp[5];
         CChar8 *src=time()+4;
         Set(temp, src, 4+1); src+=4; dt.year  =TextInt(temp);
         Set(temp, src, 2+1); src+=2; dt.month =TextInt(temp);
         Set(temp, src, 2+1); src+=2; dt.day   =TextInt(temp);
         Set(temp, src, 2+1); src+=2; dt.hour  =TextInt(temp);
         Set(temp, src, 2+1); src+=2; dt.minute=TextInt(temp);
         Set(temp, src, 2+1); src+=2; dt.second=TextInt(temp);
         return true;
      }
   }
   dt.zero(); return false;
}
Bool Ftp::rename(C Str &src, C Str &dest)
{
   if(is())
   {
      if(    command(S+"RNFR "+UnixPathUTF8(src ).tailSlash(false)).first()=='3')
      return command(S+"RNTO "+UnixPathUTF8(dest).tailSlash(false)).first()=='2';
   }
   return false;
}
Bool Ftp::removeFile(C Str &file)
{
   if(is())return command(S+"DELE "+UnixPathUTF8(file)).first()=='2';
           return false;
}
Bool Ftp::removeDir(C Str &dir)
{
   if(is())return command(S+"RMD "+UnixPathUTF8(dir).tailSlash(false)).first()=='2';
           return false;
}
Bool Ftp::createDir(C Str &dir)
{
   if(is())return command(S+"MKD "+UnixPathUTF8(dir).tailSlash(false)).first()=='2';
           return false;
}
Bool Ftp::changeDir(C Str &dir)
{
   if(is())return command(S+"CWD "+UnixPathUTF8(dir).tailSlash(false)).first()=='2';
           return false;
}
/******************************************************************************/
Bool Ftp::connect(SecureSocket &transfer, C Str &ftp_file, CChar8 *cmd, Long offset, Bool passive)
{
   Str8 ftp_file8=UnixPathUTF8(ftp_file).tailSlash(false);
   if(passive)
   {
         Str8    addr=command("PASV"); // sample reply: "227 Entering Passive Mode (174,142,230,146,135,63)\r\n"
      if(CChar8 *t   =TextPos(addr, '('))
      {
         t++;
         Byte ip[4], port[2];
         CalcValue val;
         t=_SkipChar(TextValue(t, val)); ip  [0]=val.asUInt();
         t=_SkipChar(TextValue(t, val)); ip  [1]=val.asUInt();
         t=_SkipChar(TextValue(t, val)); ip  [2]=val.asUInt();
         t=_SkipChar(TextValue(t, val)); ip  [3]=val.asUInt();
         t=_SkipChar(TextValue(t, val)); port[0]=val.asUInt();
         t=_SkipChar(TextValue(t, val)); port[1]=val.asUInt();
         SockAddr addr; addr.setIp4Port(VecB4(ip[0], ip[1], ip[2], ip[3]).u, (port[0]<<8)|port[1]);
         if(transfer.createTcp(addr))
         if(transfer.connect  (addr)==Socket::CONNECTED)
         {
            // set offset
            if(offset)if(!_abort && command(S+"REST "+offset).first()!='3')return false;

            // initialize transfer
            return !_abort && command(S+cmd+ftp_file8).first()=='1';
         }
      }
   }else
   {
      // create server
      Socket   server  ; server  .createTcp(GetDualStackSocket()); server.bind(SockAddr().setServer(0)); server.listen(); // creating an IPv6 socket when 'DualStackSocket' is not supported will result in FTP failing to connect (this was tested)
      SockAddr ftp_addr; ftp_addr.setFrom  (_socket);
      Int      port=server.port();

      // connect
      if(!_abort && command(Replace(S+"PORT "+ftp_addr.ip4Text()+','+(port>>8)+','+(port&0xFF), '.', ',')).first()=='2')
      {
         // set offset
         if(offset)if(!_abort && command(S+"REST "+offset).first()!='3')return false;

         // initialize transfer
         if(!_abort && command(S+cmd+ftp_file8).first()=='1')
            if(server.wait(FTP_WAIT_TIME)) // wait until connection arrives
         {
            SockAddr transfer_addr;
            return server.accept(transfer, transfer_addr);
         }
      }
   }
   return false;
}
Bool Ftp::transfer(File &file, C Str &ftp_file, CChar8 *cmd, Long offset, Bool passive, Bool send, Bool binary, Cipher *cipher)
{
  _abort   =false;
  _progress= 0;
  _total   =-1;
   if(is() && !_abort && mode(binary))
   {
      SecureSocket transfer; if(!_abort && connect(transfer, ftp_file, cmd, offset, passive))
      {
         Byte buf[1024*32];
         Bool ok=true;
         UInt start_time; if(_timeouts)start_time=Time.curTimeMs();
         if(send) // send data
         {
           _total=file.left();
            for(; !_abort && !file.end(); )
            {
               Int size=file.getReturnSize(buf, Min(SIZEI(buf), file.left()));
               if(cipher)cipher->encrypt(buf, buf, size, offset+_progress);
               if(!T.send(transfer, buf, size)){ok=false; break;}
              _progress+=size;

            #if FTP_INTERRUPT
               // interrupt in case server tends to timeout
               if(_timeouts && !_abort && !file.end() && Time.curTimeMs()-start_time>=50*1000) // if 50 seconds have passed, this code was tested OK for UInt overflow
                  if(Equal(cmd, "STOR ")
                  || Equal(cmd, "APPE "))
               {
                  if(!transfer.flush(FTP_WAIT_TIME)){ok=false; break;}
                  transfer.del();
                  if(response(FTP_WAIT_TIME).first()=='2' // check if current transfer was ok
                  && connect(transfer, ftp_file, "APPE ", 0, passive))start_time=Time.curTimeMs();else{ok=false; break;} // continue in appending mode
               }
            #endif
            }
            if(!transfer.flush(FTP_WAIT_TIME))ok=false;
         }else // receive data
         {
            for(; !_abort; )
            {
               if(!transfer.wait(FTP_WAIT_TIME)){ok=false; break;}
               Int size =transfer.receive(buf, SIZE(buf));
               if( size<=0)break;
               if(cipher)cipher->decrypt(buf, buf, size, offset+_progress);
               file.put(buf, size);
              _progress+=size;
            }
         }
         transfer.del();
         Char8 resp=response(FTP_WAIT_TIME).first();
         if(ok && resp!='2')
         {
            ok=false;
            if(!resp) // "no response" = timeout, which means that this may have succeeded but we've just lost connection
            {
               if(Equal(cmd, "STOR ")) // if we were uploading (can't do this for appending, because we would need to check initial size of the file on the server to verify how much we've uploaded)
               {
                  Long total=_total; // remember this because it will get cleared in reconnection
                 _timeout=true; // we're currently timed out
                  if(reconnect())
                  {
                    _timeouts=true; // mark this connection as tending to time out
                     if(total==fileSize(ftp_file)) // if uploaded data is all that we wanted to send
                        ok=true; // set as ok
                  }
               }else
               if(Equal(cmd, "RETR ")) // if we were downloading
               {
                  Long progress=_progress; // remember this because it will get cleared in reconnection
                 _timeout=true; // we're currently timed out
                  if(reconnect())
                  {
                    _timeouts=true; // mark this connection as tending to time out
                     if(progress==fileSize(ftp_file)-offset) // if downloaded data is all that we wanted to receive
                        ok=true; // set as ok
                  }
               }
            }
         }
        _abort   =false;
        _progress= 0;
        _total   =-1;
         return ok;
      }
   }
   return false;
}
Bool Ftp::download(C Str  &src,   File &dest, Long offset, Bool passive, Cipher * src_cipher) {return transfer(dest, src , "RETR ", offset, passive, false, true,  src_cipher);}
Bool Ftp::  upload(  File &src, C Str  &dest             , Bool passive, Cipher *dest_cipher) {return transfer(src , dest, "STOR ",      0, passive, true , true, dest_cipher);}
Bool Ftp::  append(  File &src, C Str  &dest             , Bool passive                     ) {return transfer(src , dest, "APPE ",      0, passive, true , true             );}
/******************************************************************************/
Bool Ftp::listFiles(C Str &path, MemPtr<FtpFile> files, Bool passive)
{
   files.clear();
   if(is())
   {
      File f; f.writeMem();
      if(transfer(f, path, "LIST ", 0, passive, false, false))
      {
         DateTime dt; dt.getUTC();
         Memt<Char8> data; data.setNum(f.size()+1); f.pos(0); f.get(data.data(), f.size()); data[f.size()]=0;
         for(Char8 *line=data.data(); line; )
         {
            CChar8 *t=line; if(line=(Char8*)TextPos(line, '\n'))line++; if(line){line[-1]=0; if(line[-2]=='\r')line[-2]=0;}
            if(*t=='-' || *t=='d' || *t=='l')
            {
               FtpFile f;
               switch(*t)
               {
                  case '-': f.type=FSTD_FILE; break;
                  case 'd': f.type=FSTD_DIR ; break;
                  case 'l': f.type=FSTD_LINK; break;
               }
               t=_SkipWhiteChars(TextPos(t, ' '));
               t=_SkipWhiteChars(TextPos(t, ' '));
               t=_SkipWhiteChars(TextPos(t, ' '));
               t=_SkipWhiteChars(TextPos(t, ' '));
               CalcValue val;
               f.size=TextLong(t);
               t=_SkipWhiteChars(TextPos(t, ' '));
               if(Starts(t, "Jan"))f.modify_time_utc.month= 1;else
               if(Starts(t, "Feb"))f.modify_time_utc.month= 2;else
               if(Starts(t, "Mar"))f.modify_time_utc.month= 3;else
               if(Starts(t, "Apr"))f.modify_time_utc.month= 4;else
               if(Starts(t, "May"))f.modify_time_utc.month= 5;else
               if(Starts(t, "Jun"))f.modify_time_utc.month= 6;else
               if(Starts(t, "Jul"))f.modify_time_utc.month= 7;else
               if(Starts(t, "Aug"))f.modify_time_utc.month= 8;else
               if(Starts(t, "Sep"))f.modify_time_utc.month= 9;else
               if(Starts(t, "Oct"))f.modify_time_utc.month=10;else
               if(Starts(t, "Nov"))f.modify_time_utc.month=11;else
               if(Starts(t, "Dec"))f.modify_time_utc.month=12;else continue;
               t=_SkipWhiteChars(TextPos  (t, ' '));
               t=_SkipWhiteChars(TextValue(t, val)); f.modify_time_utc.day=val.asUInt();
               if(Char8 *next=(Char8*)TextPos(t, ' '))
               {
                 *next=0; next=(Char8*)_SkipWhiteChars(next+1);
                  if(CChar8 *minute=TextPos(t, ':'))
                  {
                     TextValue(t       , val); f.modify_time_utc.hour  =val.asUInt();
                     TextValue(minute+1, val); f.modify_time_utc.minute=val.asUInt();
                                               f.modify_time_utc.year  =dt .year; // if the year is not specified then it means that date is from this year
                  }else
                  {
                     TextValue(t, val); f.modify_time_utc.year=val.asUInt();
                  }
                  if(!Equal(next, ".")
                  && !Equal(next, ".."))
                  {
                     f.name=FromUTF8(next);
                     files.add(f);
                  }
               }
            }
         }
         return true;
      }
   }
   return false;
}
Bool Ftp::listNames(C Str &path, MemPtr<Str> names, Bool passive)
{
   names.clear();
   if(is())
   {
      File f; f.writeMem();
      if(transfer(f, path, "NLST ", 0, passive, false, false))
      {
         Memt<Char8> data; data.setNum(f.size()+1); f.pos(0); f.get(data.data(), f.size()); data[f.size()]=0;
         for(Char8 *line=data.data(); line; )
         {
            Char8 *t=line; if(line=(Char8*)TextPos(line, '\n'))line++; if(line){line[-1]=0; if(line[-2]=='\r')line[-2]=0;}
            if( Is   (t      )
            && !Equal(t, "." )
            && !Equal(t, ".."))names.add(FromUTF8(t));
         }
         return true;
      }
   }
   return false;
}
/******************************************************************************/
void Ftp::abort() {_abort=true;}
/******************************************************************************/
}
/******************************************************************************/
