/******************************************************************************

   Use 'Connection' to handle safe client/server TCP based connections.

   Use 'ConnectionServer' to handle server managing multiple client connections.

/******************************************************************************/
enum CONNECT_STATE : Byte
{
   CONNECT_INVALID         , // not connected or connection lost
   CONNECT_CONNECTING      , // connection is in progress
   CONNECT_AWAIT_GREET     , // awaiting to receive greeting
   CONNECT_VERSION_CONFLICT, // server/client use different software version
   CONNECT_GREETED         , // greeted
};
/******************************************************************************/
struct Connection // reliable TCP based client/server connection with automatic data encryption and boundaries management, data always reaches the target, multiple data packets are always received in the same order as they were sent, received data size will always be the same as when it was sent
{
   File data; // received data from the external connection, use it for reading incoming data

   // manage
   Connection& del                  (                       );
   Connection& clientConnectToServer(C SockAddr &server_addr);
   Bool        serverAcceptClient   (  Socket   &server     );

   // get
   CONNECT_STATE state   ()C {return           _state;} // get connection state
 C SockAddr&     address ()C {return         _address;} // get destination address
   Long          received()C {return       _in_offset;} // get total  amount of received data
   Long          sent    ()C {return      _out_offset;} // get total  amount of sent     data
   Int           queued  ()C {return (Int)_out.left();} // get queued amount of bytes currently awaiting to be sent
   UInt          life    ()C;                           // how long the connection is alive (in milliseconds), 0 if not yet created

   Bool receive    (Int timeout); // wait up to 'timeout' milliseconds to receive data, false if no data is available, this method automatically calls 'updateState'
   Bool updateState(Int timeout); // this method verifies current connection state, and performs necessary steps needed for connection to reach the CONNECT_GREETED state, 'timeout'=how long (in milliseconds) to wait for connection to reach CONNECT_GREETED state in this step, true is returned if connection has reached CONNECT_GREETED state, false is returned on fail (if connection failed or is still in progress)

   // io
   Bool send(CPtr  buf, Int size   , Bool flush=true); //                                                 'flush'=if automatically call 'flush' method to send this message immediately, the method will always fail if connection state is not CONNECT_GREETED
   Bool send(File &f  , Int size=-1, Bool flush=true); // 'size'=size to send (-1=all remaining in file), 'flush'=if automatically call 'flush' method to send this message immediately, the method will always fail if connection state is not CONNECT_GREETED

   Bool flush(Int timeout=-1); // flush all queued commands for sending within 'timeout' milliseconds (use <0 for unlimited amount of time), false on timeout (when timeout occurs then all remaining data will be sent next time)

   // operations
   Bool tcpNoDelay(Bool on) {return _socket.tcpNoDelay(on);} // set TCP_NODELAY option, false on fail

   Connection() {_state=CONNECT_INVALID; _msg_size_progress=0; _msg_size=_birth=0; _in_offset=_out_offset=0;}
  ~Connection() {del();}

#if !EE_PRIVATE
private:
#endif
   CONNECT_STATE _state;
   Byte          _msg_size_progress;
   UInt          _msg_size, _birth;
   Long          _in_offset, _out_offset;
   File          _out;
   Socket        _socket;
   SockAddr      _address;
   Cipher1       _cipher;

#if EE_PRIVATE
   Bool greet   ();
   Bool flushEx (Int timeout); // wait 'timeout' until all data has been sent
   Bool updateEx(Int timeout, Bool read);
#endif
};
/******************************************************************************/
struct FastConnection // fast but unreliable UDP based connection, data is not guaranteed to reach the target, multiple data packets may not always be received in the same order as they were sent, received data size will always be the same as when it was sent
{
   // manage
   void del   (                   ); // delete
   Bool create(  Int       port=-1); // create using automatic address at specified 'port' (-1=autodetect), false on fail
   Bool create(C SockAddr &addr   ); // create using custom    address                                    , false on fail

   // get
   Int  port    ()C {return _socket.port();} // get port at which connection was created
   Long sent    ()C {return _sent         ;} // get total amount of sent     data
   Long received()C {return _received     ;} // get total amount of received data

   // io
   Bool send(C SockAddr &addr, CPtr data, Int size, Cipher *cipher=null); // send 'size' amount of 'data' to 'addr' address          , if 'cipher' is given then it will be used to encrypt the data, this method supports sending up to 64KB data, false on fail
   Bool send(C SockAddr &addr, File &f            , Cipher *cipher=null); // send 'f' file data, from its current position to its end, if 'cipher' is given then it will be used to encrypt the data, this method supports sending up to 64KB data, false on fail

   Int receive(SockAddr &addr, Byte (&data)[65536]); // receive data, 'data'=custom buffer which will receive data, if data was received successfully, the 'addr' will contain the address of the sender, and method will return size of received data, -1 if there was no data

   FastConnection() {_sent=_received=0;}

private:
   Long   _sent, _received;
   Socket _socket;
};
/******************************************************************************/
struct ConnectionServer // connection server managing multiple client connections using 'Connection' class
{
   struct Client
   {
      Connection connection;

      virtual void create(ConnectionServer &server) {} // called upon creation of the client with 'server' ConnectionServer responsible for the connection
      virtual Bool update(                        );   // override this method and manually process the client update: check if super::update() is true, check and process incoming connection data, return false if client wants to be removed and true if wants to live
   };

   Map<SockAddr, Client> clients; // map of clients

   // manage
   void del   (                   ); // delete manually
   Bool create(  Int       port=-1); // create server using automatic address, 'port'=port for creating the socket server (-1=autodetect), false on fail
   Bool create(C SockAddr &addr   ); // create server using custom    address, 'addr'=address on which the server should be created      , false on fail

   // get
   Bool      is              ()C {return _server.is  ();} // if  server is created
   Int       port            ()C {return _server.port();} // get server port
   Str       localAddressName()C;                         // get server local  address using Computer Name format
   SockAddr  localAddress    ()C;                         // get server local  address
   SockAddr globalAddress    ()C;                         // get server global address

   // operations
   void update(); // update server, call this continuously to accept new clients and process existing ones

  ~ConnectionServer() {del();}
   ConnectionServer();

private:
   Socket _server;
};
/******************************************************************************/
