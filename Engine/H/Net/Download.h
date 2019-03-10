/******************************************************************************

   Use 'Download' to download a single file from the internet.

   'Download' can be also used to upload files when 'File post' parameter is specified,
      in that case, POST command will be used instead of GET.
   You can use following PHP code on the server to process the file:

      <?php
      function LogN($text) {error_log($text."\n", 3, "log.txt");}

      $error    =$_FILES["file"]["error"   ]; // if error occurred during file receiving
      $file_size=$_FILES["file"]["size"    ]; // file size
      $temp_name=$_FILES["file"]["tmp_name"]; // temporary file name
      $file_name="Uploads/file";              // desired   file name

      if($error>0) // if any error occurred
      {
         LogN('Error:'.$error);
      }else
      {
         LogN("File Size: ".$file_size);
         LogN("Temp Name: ".$temp_name);

         if(file_exists($file_name)) // check if such file already exists
         {
	         LogN($file_name." already exists"); // don't store it
         }else
         {
	         move_uploaded_file($temp_name, $file_name); // store the file
         }
      }
      ?>

/******************************************************************************/
enum DWNL_STATE : Byte // Download States
{
   DWNL_NONE      , // nothing
   DWNL_CONNECTING, // waiting for connection
   DWNL_AUTH      , // authenticating (this occurs for secure https connections only)
   DWNL_SENDING   , // sending request and 'post' data if specified
   DWNL_DOWNLOAD  , // downloading
   DWNL_DONE      , // done
   DWNL_ERROR     , // error encountered
};
/******************************************************************************/
enum HTTP_TYPE : Byte // HTTP Parameter Type
{
   HTTP_GET   , // get    parameter
   HTTP_POST  , // post   parameter
   HTTP_HEADER, // header parameter
};
STRUCT(HTTPParam , TextParam) // optional parameter that can be passed to the 'Download'
//{
   HTTP_TYPE type; // parameter type

   HTTPParam& set(C Str &name, C Str &value, HTTP_TYPE type=HTTP_GET) {super::set(name, value); T.type=type; return T;}

   HTTPParam() {type=HTTP_GET;}

   static Str8 Encode(C MemPtr<HTTPParam> &params); // encode 'params' array into string
};
/******************************************************************************/
const_mem_addr struct Download // File Downloader !! must be stored in constant memory address !!
{
   // manage
   Download& del   (  Int  milliseconds=-1                                                                                                                                                            ); // wait 'milliseconds' time for thread to exit and delete (<0 = infinite wait)
   Download& create(C Str &url, C MemPtr<HTTPParam> &params=null, const_mem_addr File *post=null, Long max_post_size=-1, Long offset=0, Long size=-1, Bool paused=false, Bool ignore_auth_result=false); // download 'url' file, 'params'=optional parameters that you can pass if the 'url' is a php script, 'post'=data to be sent to the specified address (if this is null then HTTP GET is used, otherwise HTTP POST is used, 'post' File must point to a constant memory address as that pointer will be used until the data has been fully sent), 'max_post_size'=number of bytes to send (-1=all remaining), 'offset'=offset position of the file data to download, use this for example if you wish to resume previous download by starting from 'offset' position, 'size'=number of bytes to download (-1=all remaining), warning: some servers don't support manual specifying 'offset' and 'size', 'paused'=if create paused, 'ignore_auth_result'=if ignore authorization results and continue even when they failed

   // operations
   Download&  pause (                   ); // pause  downloading
   Download&  resume(                   ); // resume downloading
   Download&  stop  (                   ); // request  the download to be stopped and return immediately after that, without waiting for the thread to exit
   Download&  wait  (Int milliseconds=-1); // wait for the download thread to exit (<0 = infinite wait)

   // get
   DWNL_STATE state           ()C {return _state                 ;} // get download state
   UShort     code            ()C {return _code                  ;} // get HTTP status code, or 0 if unavailable
   Ptr        data            ()C {return _data                  ;} // get downloaded data            , this will be valid only in DWNL_DONE state
   Long       offset          ()C {return _offset                ;} // get offset of file
   Long       done            ()C {return _done                  ;} // get number of downloaded bytes
   Long       size            ()C {return _size                  ;} // get number of bytes to download, this will be valid only in DWNL_DONE state, in DWNL_DOWNLOAD state the value will be either valid or equal to -1 if the size is still unknown
   Long       totalSize       ()C {return _total_size            ;} // get total file size            , this will be valid only in DWNL_DONE state, in DWNL_DOWNLOAD state the value will be either valid or equal to -1 if the size is still unknown
   Long       sent            ()C {return _sent                  ;} // get number of sent bytes 
   Long       toSend          ()C {return _to_send               ;} // get number of bytes to send
   Long       totalSent       ()C {return _total_sent            ;} // get total number of bytes that were sent       , including ovearhead - headers, redirections and SSL/TLS/HTTPS handshake/data
   Long       totalReceived   ()C {return _total_rcvd            ;} // get total number of bytes that were received   , including ovearhead - headers, redirections and SSL/TLS/HTTPS handshake/data
   Long       totalTransferred()C {return _total_sent+_total_rcvd;} // get total number of bytes that were transferred, including ovearhead - headers, redirections and SSL/TLS/HTTPS handshake/data
 C Str&       url             ()C {return _url                   ;} // get url address
 C DateTime&  modifyTimeUTC   ()C {return _modif_time            ;} // get modification time in UTC time zone of the downloaded file, if value is provided then it will be available starting from DWNL_DOWNLOAD state
   Bool       paused          ()C {return _thread.wantPause()    ;} // get if download is currently requested to be paused
   Bool       authFailed      ()C;                                  // get if authorization failed, this will be valid after DWNL_AUTH finished

#if EE_PRIVATE
   void parse  (Byte *data, Int size);
   Int  send   (CPtr  data, Int size);
   Int  receive( Ptr  data, Int size);
   void finish ();
   void zero   ();
   Bool func   ();
   void delPartial();
   Bool error  (); // !! this is not thread-safe !! set DWNL_ERROR state, you can signal that an error has encountered for example when invalid data downloaded, always returns false
#endif

           ~Download() {del();}
            Download();
   explicit Download(C Str &url, C MemPtr<HTTPParam> &params=null, const_mem_addr File *post=null, Long max_post_size=-1, Long offset=0, Long size=-1, Bool paused=false);

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   enum PARSE_MODE : Byte
   {
      PARSE_DATA_SIZE,
      PARSE_DATA     ,
      PARSE_SKIP_LINE,
      PARSE_END      ,
   };
   enum FLAGS
   {
      CHUNKED         =1<<0, // if download is chunked
      HAS_ADDRS_HEADER=1<<1, // this flag is used to check if has "addresses" in connecting state, and "header" in downloading state
      AUTH_IGNORE     =1<<2, // ignore authorization results and continue with download
      AUTH_FAILED     =1<<3, // authorization failed
   };
   Bool chunked       ()C {return FlagTest(_flags, CHUNKED);}
   Bool hasAddrsHeader()C {return FlagTest(_flags, HAS_ADDRS_HEADER);}
#endif
   Byte           _flags, _parse;
   DWNL_STATE     _state;
   UShort         _code;
   Int            _expected_size, _pre_send, _pos_send;
   Long           _offset, _done, _size, _total_size, _sent, _to_send, _total_sent, _total_rcvd;
   Ptr            _data;
   File          *_post_file;
   DateTime       _modif_time;
   Str8           _url_full, _header;
   Str            _url;
   Thread         _thread;
   SecureSocket   _socket;
   Memb<Byte>     _memb;
   Mems<Byte>     _message;
   Mems<SockAddr> _addrs;
#if WEB
   Ptr            _js_download;
#endif

   NO_COPY_CONSTRUCTOR(Download);
};
/******************************************************************************/
#if EE_PRIVATE
   #if HAS_THREADS
      #define DOWNLOAD_WAIT_TIME 1
   #else
      #define DOWNLOAD_WAIT_TIME 0
   #endif
#endif
/******************************************************************************/
