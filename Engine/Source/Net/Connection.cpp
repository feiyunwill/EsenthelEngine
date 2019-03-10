/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CONNECTION_CC4     CC4('E','E','N','C') // Esenthel Engine Network Connection
#define CONNECTION_VERSION 0

#define MSG_SIZE_KNOWN 0xFF

#define SEND_WAIT_TIME (15*1000)

#define LOG_ERROR 0
#if     LOG_ERROR
   #pragma message("!! Warning: Use this only for debugging !!")
#endif
/******************************************************************************/
// CONNECTION
/******************************************************************************/
Connection& Connection::del()
{
   if(_state==CONNECT_GREETED)flush(0); // if we were connected then flush any data, don't attempt to flush in other states (like sending a greeting for example)
  _state            =CONNECT_INVALID;
  _birth            =0;
  _msg_size_progress=0;
  _msg_size         =0;
  _socket.del();
  _out   .del();
   data  .del();
   // don't clear '_in_offset' '_out_offset' '_address' as they may be called after connection got deleted (deletion may occur upon error/connection lost and we want to keep the stats even after that)
   return T;
}
Bool Connection::serverAcceptClient(Socket &server)
{
   del();
  _in_offset=_out_offset=0; // clear at this stage because it's needed before 'greet', also make sure to always clear at connection time for stats usage

   if(server.accept(_socket, _address))
   {
     _socket.block(false);
     _birth=Time.curTimeMs();
      return greet();
   }
   return false;
}
Connection& Connection::clientConnectToServer(C SockAddr &server_addr)
{
   del();
  _in_offset=_out_offset=0; // clear at this stage because it's needed before 'greet', also make sure to always clear at connection time for stats usage

   if(_socket.createTcp(server_addr))
   {
     _socket.block(false);
     _address=server_addr;
     _birth  =Time.curTimeMs();
      switch(_socket.connect(server_addr, 0))
      {
         case Socket::FAILED     : del(); break;
         case Socket::CONNECTED  : greet(); break; // 'greet' sets the '_state'
         case Socket::IN_PROGRESS: _state=CONNECT_CONNECTING; break;
      }
   }
   return T;
}
/******************************************************************************/
UInt Connection::life()C
{
   return (_state!=CONNECT_INVALID) ? Time.curTimeMs()-_birth : 0; // this code was tested OK for UInt overflow
}
/******************************************************************************/
Bool Connection::greet()
{
  _out .writeMem(4*1024);
   data.writeMem(4*1024);

  _cipher.randomizeKey();

   // since we need to know the size of the cipher key up-front, first let's save it to 'data'
   if(!_cipher.saveKey(data))goto error;

  _out.cmpUIntV(4+1+data.size()   ); // message length
  _out.putUInt (CONNECTION_CC4    ); // 4
  _out.putByte (CONNECTION_VERSION); // 1
  data.pos(0); data.copy(_out); data.reset(); // cipher key
  _out.pos(0);

  _state=CONNECT_AWAIT_GREET;
   if(flush())return true;
error:
   del(); return false;
}
/******************************************************************************/
Bool Connection::flush(Int timeout)
{
   for(;;)
   {
      if(_out.end()){_out.reset(); return true;} // all data was sent
      if(!_socket.flush((timeout<0) ? SEND_WAIT_TIME : timeout))return false; // socket is busy
      if(timeout>0)timeout=0; // don't wait for next steps
      Byte buf[(64+1)*1024]; // 1 extra for the headers
      Int  left=Min(SIZEI(buf), _out.left()); if(!_out.get(buf, left))break; if(_state==CONNECT_GREETED)_cipher.encrypt(buf, buf, left, _out_offset); // read data
      Int  sent=_socket.send(buf, left); // send data
      if(sent<0 || sent>left)sent=0; // if an error occurred then clear sent. "sent==0" is also an error, but in this statement we don't need to change sent=0 -> 0
      if(_state==CONNECT_GREETED)_out_offset+=sent;
      if(left!=sent) // if haven't sent everything
      {
        _out.skip(sent-left); // go back the amount of bytes that weren't sent
         if(!sent) // if nothing was sent
         {
            if(Socket::WouldBlock()) // this can happen if data doesn't fit in the buffer
            {
               if(timeout<0)continue; // if we have unlimited timeout, then try again
               return false; // flush failed, but don't delete
            }
            break; // break so we can delete
         }
      }
   }
  _state=CONNECT_INVALID; // this will prevent calling 'flush' in the 'del' method
   del(); return false;
}
Bool Connection::flushEx(Int timeout)
{
   return flush() && _socket.flush((timeout<0) ? SEND_WAIT_TIME : timeout);
}
/******************************************************************************/
Bool Connection::updateEx(Int timeout, Bool read)
{
   UInt    end_time; // this function was tested OK for UInt overflow
   Bool adjust_time; // if adjust 'timeout' based on 'Time.curTimeMs' and 'end_time' before making some func call

   switch(_state)
   {
      case CONNECT_CONNECTING:
      {
         if(timeout>0)end_time=Time.curTimeMs()+timeout;
         if(_socket.any(timeout) && greet()){adjust_time=true; goto read_data;} // connected
         if(_socket.connectFailed())goto error;
      }break;

      case CONNECT_GREETED:
      {
         if(!read) // if we're not going to try and read a message (which can detect disconnects), then we have to at least check if we've got disconnected
         {
            if(_socket.wait(0) && _socket.available()<=0) // connection lost
            {
            #if LOG_ERROR
               LogN(S+"Connection.updateEx 0:"+PLATFORM(WSAGetLastError(), errno)+", available:"+_socket.available());
            #endif
              _state=CONNECT_INVALID; goto error; // set '_state' to prevent calling 'flush' in the 'del' method
            }
            return true;
         } // otherwise continue below
      case CONNECT_AWAIT_GREET:
      // in this section only 'end_time' and 'adjust_time' should be set
         if(timeout>0){end_time=Time.curTimeMs()+timeout; adjust_time=false;}
      // in this section only 'end_time' and 'adjust_time' should be set

      read_data:
         for(; _msg_size_progress!=MSG_SIZE_KNOWN; ) // unknown packet size
         {
            if(timeout>0)if(adjust_time)MAX(timeout=end_time-Time.curTimeMs(), 0);else adjust_time=true;
            if(!_socket.wait(timeout))return false; // if there's no data yet then abort

            Byte b; Int l=_socket.receive(&b, 1); // '_msg_size' is compressed using 'File.cmpUIntV', so we need to decompress it byte by byte until last read byte doesn't have the last bit enabled (b&128)
            if(l!=1) // connection lost
            {
            #if LOG_ERROR
               LogN(S+"Connection.updateEx 1:"+PLATFORM(WSAGetLastError(), errno)+", receive:"+l);
            #endif
              _state=CONNECT_INVALID; goto error; // set '_state' to prevent calling 'flush' in the 'del' method
            }
            if(_state==CONNECT_GREETED){_cipher.decrypt(&b, &b, 1, _in_offset); _in_offset++;} // decrypt
           _msg_size|=((b&127)<<(_msg_size_progress*7)); // each step grants knowledge of 7 bits
            if(b&128)_msg_size_progress++;else // need more info
            {
               data.reset(); // reset for upcoming data
              _msg_size_progress=((_msg_size<=0) ? 0 : MSG_SIZE_KNOWN); // if the message size was zero, then ignore it, and read the next one
            }
         }

         if(_msg_size_progress==MSG_SIZE_KNOWN)for(; _msg_size>0; ) // we await data
         {
            if(timeout>0)if(adjust_time)MAX(timeout=end_time-Time.curTimeMs(), 0);else adjust_time=true;
            if(!_socket.wait(timeout))return false; // if there's no data yet then abort

            Byte buf[65536];
            Int  want=Min(SIZEU(buf), _msg_size),
                 got =_socket.receive(buf, want);
            if(  got<=0 || got>want) // connection lost
            {
            #if LOG_ERROR
               LogN(S+"Connection.updateEx 2:"+PLATFORM(WSAGetLastError(), errno)+", receive:"+got);
            #endif
              _state=CONNECT_INVALID; goto error; // set '_state' to prevent calling 'flush' in the 'del' method
            }
            if(_state==CONNECT_GREETED){_cipher.decrypt(buf, buf, got, _in_offset); _in_offset+=got;} // decrypt
            data.put(buf, got);
               _msg_size-=got;
            if(_msg_size<=0) // we've received entire message
            {
              _msg_size_progress=0;
               data.pos(0);
               if(_state==CONNECT_AWAIT_GREET)
               {
                  if(data.getUInt()!=CONNECTION_CC4    )goto error;
                  if(data.getByte()!=CONNECTION_VERSION){del()._state=CONNECT_VERSION_CONFLICT; return false;}
                  if(!_cipher.mixKey(data)             )goto error;
                  if(!data.end()                       )goto error; // there should be no data left after the cipher key
                 _state=CONNECT_GREETED;
                  data.reset(); // don't leave the greeting for the user
                  if(read)goto read_data; // here we've only greeted, however if we want an actual message, then try again
                  // proceed below to return 'true' as success
               }
               return true; // return this message to the user
            }
         }
      }break;
   }
   return false;
error:
   del(); return false;
}
Bool Connection::updateState(Int timeout) {return updateEx(timeout, false);}
Bool Connection::receive    (Int timeout) {return updateEx(timeout, true );}
/******************************************************************************/
Bool Connection::send(CPtr buf, Int size, Bool flush)
{
   if(size>0 && _state==CONNECT_GREETED)
   {
      Int pos=_out.pos(); _out.pos(_out.size());
     _out.cmpUIntV(size);
     _out.put     (buf, size); // size
     _out.pos     (pos);
      return flush ? T.flush() : true;
   }
   return size==0;
}
Bool Connection::send(File &f, Int size, Bool flush)
{
   if(size<0)size=f.left();
   if(size>0 && _state==CONNECT_GREETED)
   {
      Int pos=_out.pos(); _out.pos(_out.size());
     _out.cmpUIntV(size);
      for(; size>0; )
      {
         Byte buf[65536];
         Int  l=Min(SIZEU(buf), size);
         f  .get(buf, l);
        _out.put(buf, l); // size
         size-=l;
      }
     _out.pos(pos);
      return flush ? T.flush() : true;
   }
   return size==0;
}
/******************************************************************************/
// FAST CONNECTION
/******************************************************************************/
void FastConnection::del()
{
  _socket.del();
   // don't clear '_sent' '_received' so they can be read after connection is deleted
}
Bool FastConnection::create(Int port)
{
   del(); _sent=_received=0; // instead of clearing params in 'del', clear them here
   SockAddr addr; addr.setServer(port);
   if(_socket.createUdp(addr))
   {
     _socket.block(false);
      if(port>=0) // try specified port
      {
         if(_socket.bind(addr))return true;
      }else
      REP(0x10000)if(_socket.bind(addr.port(i)))return true; // find first available
   }
   del(); return false;
}
Bool FastConnection::create(C SockAddr &addr)
{
   del(); _sent=_received=0; // instead of clearing params in 'del', clear them here
   if(_socket.createUdp(addr) && _socket.bind(addr))
   {
     _socket.block(false);
      return true;
   }
   del(); return false;
}
/******************************************************************************/
Bool FastConnection::send(C SockAddr &addr, CPtr data, Int size, Cipher *cipher)
{
   if(size>=0 && size<=65536)
   {
      Byte temp[65536]; if(cipher){cipher->encrypt(temp, data, size, 0); data=temp;}
      Int  sent=_socket.send(addr, data, size);
      if(  sent>0)T._sent+=sent;
      return sent==size;
   }
   return false;
}
Bool FastConnection::send(C SockAddr &addr, File &f, Cipher *cipher)
{
   Int left =f.left();
   if( left<=65536)
   {
      Byte temp[65536]; f.get(temp, left);
      return send(addr, temp, left, cipher);
   }
   return false;
}
Int FastConnection::receive(SockAddr &addr, Byte (&data)[65536])
{
   Int    received=_socket.receive(addr, data, SIZE(data));
   if(    received>0)T._received+=received;
   return received;
}
/******************************************************************************/
// CONNECTION SERVER
/******************************************************************************/
Bool ConnectionServer::Client::update()
{
   if(connection.updateState(0))return true;
   switch(connection.state())
   {
      case CONNECT_CONNECTING :
      case CONNECT_AWAIT_GREET: return connection.life()<=5000; // allow some time to connect
   }
   return false;
}
/******************************************************************************/
ConnectionServer::ConnectionServer() : clients(Compare)
{
}
/******************************************************************************/
void ConnectionServer::del()
{
  _server .del();
   clients.del();
}
Bool ConnectionServer::create(Int port)
{
   del();
   SockAddr addr; addr.setServer(port);
   if(_server.createTcp(addr))
   {
     _server.block(false); // disable blocking before calling 'listen' in case that would block
      if(port>=0) // try specified port
      {
         if(_server.bind(addr))if(_server.listen())return true;
      }else
      REP(0x10000)if(_server.bind(addr.port(i))) // find first available
      {
         if(_server.listen())return true;
         break;
      }
   }
   del(); return false;
}
Bool ConnectionServer::create(C SockAddr &addr)
{
   del();
   if(_server.createTcp(addr))
   {
     _server.block(false); // disable blocking before calling 'listen' in case that would block
      if(_server.bind(addr))if(_server.listen())return true;
   }
   del(); return false;
}
Str ConnectionServer::localAddressName()C
{
   if(is())return GetComputerName()+':'+port();
   return S;
}
SockAddr ConnectionServer::localAddress()C
{
   SockAddr addr; if(is())addr.setLocal(port()); // can't use "_server.addr()" because if it was created with 'SockAddr.setServer' (INADDR_ANY) then that address will be returned
   return   addr;
}
SockAddr ConnectionServer::globalAddress()C
{
   SockAddr addr; if(is())addr.setGlobal(port());
   return   addr;
}
/******************************************************************************/
void ConnectionServer::update()
{
   // accept new clients
   for(; _server.wait(0); )
   {
      Connection connection; if(connection.serverAcceptClient(_server))
      {
         Client &client=*clients(connection.address());
         Swap(client.connection, connection);
         client.create(T);
      }
   }

   // update clients
   REPA(clients)if(!clients[i].update())clients.remove(i);
}
/******************************************************************************/
}
/******************************************************************************/
