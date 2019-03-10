/******************************************************************************

   Use 'Ftp' to handle FTP connections.

/******************************************************************************/
struct FtpFile : FileInfo
{
   Str name;
};
struct Ftp // File Transfer Protocol, Ftp supports only one command/transfer at a time
{
   // manage
   Bool login (C Str8 &host, C Str8 &user, C Str8 &password, Bool ignore_auth_result=false); //    connect to   FTP, false on fail, connection will use secure mode if 'host' starts with "ftps://", 'ignore_auth_result'=if ignore authorization results and continue even when they failed
   void logout(                                                                           ); // disconnect from FTP

   // get / set
   Bool is        (                             )C {return _socket.is();} // if  connected to the FTP
   Long progress  (                             )C {return _progress   ;} // get transfer progress (in bytes)
   Long total     (                             )C {return _total      ;} // get transfer total    (in bytes, -1=unknown)
   Bool noop      (                             ); // send empty message to keep up the connection, false on fail
   Long fileSize  (C Str &file                  ); // get    file size                            , -1    on fail
   Bool fileTime  (C Str &file,   DateTime &dt  ); // get    file modification time               , false on fail
   Bool rename    (C Str &src , C Str      &dest); // rename file/directory                       , false on fail
   Bool removeFile(C Str &file                  ); // remove file                                 , false on fail
   Bool removeDir (C Str &dir                   ); // remove empty   directory                    , false on fail
   Bool createDir (C Str &dir                   ); // create empty   directory                    , false on fail
   Bool changeDir (C Str &dir                   ); // change current directory                    , false on fail

   // io, if following functions will be called in a secondary thread, then you may not perform any operations on the given 'File', until the methods return
   Bool  download(C Str  &src ,   File &dest, Long offset=0, Bool passive=true, Cipher * src_cipher=null); // download 'src' file from FTP to 'dest' file       , 'passive'=transfer mode, false on fail, 'dest' file should be already opened for writing mode (either to   disk or memory)
   Bool    upload(  File &src , C Str  &dest               , Bool passive=true, Cipher *dest_cipher=null); //   upload 'src' file to   FTP    'dest' location   , 'passive'=transfer mode, false on fail, 'src'  file should be already opened for reading mode (either from disk or memory), data is uploaded from current position of 'src' file (not from start)
   Bool    append(  File &src , C Str  &dest               , Bool passive=true                          ); //   append 'src' file to   FTP    'dest' location   , 'passive'=transfer mode, false on fail, 'src'  file should be already opened for reading mode (either from disk or memory), data is uploaded from current position of 'src' file (not from start)
   Bool listFiles(C Str  &path, MemPtr<FtpFile> files      , Bool passive=true                          ); // retrieve a list of all files in the specified path, 'passive'=transfer mode, false on fail, warning: not all fields in the 'FtpFile' may be valid, this is because hosts don't necessarily provide all the info
   Bool listNames(C Str  &path, MemPtr<Str    > names      , Bool passive=true                          ); // retrieve a list of all names in the specified path, 'passive'=transfer mode, false on fail
   void     abort(                                                                                      ); // notify that the current transfer operation should be aborted, this can be called in a secondary thread to cancel any active transfer

  ~Ftp() {logout();}
   Ftp();

private:
   Bool         _binary  , _abort, _timeout, _timeouts;
   Int          _port;
   Long         _progress, _total;
   Str8         _response, _host, _user, _password;
   SecureSocket _socket;

#if EE_PRIVATE
   Bool send     (SecureSocket &socket, CPtr data, Int size);
   Str8 response (Int           time);
   Str8 command  (Str8          cmd);
   Bool mode     (Bool          binary);
   Bool connect  (SecureSocket &transfer, C Str &ftp_file, CChar8 *cmd, Long offset, Bool passive);
   Bool transfer (File         &file    , C Str &ftp_file, CChar8 *cmd, Long offset, Bool passive, Bool send, Bool binary, Cipher *cipher=null);
   Bool reconnect();
#endif
};
/******************************************************************************/
