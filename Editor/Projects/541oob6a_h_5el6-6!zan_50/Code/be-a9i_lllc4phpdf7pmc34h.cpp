/******************************************************************************

   This tutorial presents a sample Client Connection.

   It can be used together with second tutorial "Connection/Server"

/******************************************************************************/
Connection connection; // connection for client/server connection
Str        data      ; // received data
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_WORK_IN_BACKGROUND|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE; // specify work in background flag to work also when not focused
   App.x=1;
   D.mode(400, 300);
   D.scale(2);
}
bool Init()
{
   Connection temp[16]; // try 16 connection attempts at the same time
   FREPA(temp)temp[i].clientConnectToServer(SockAddr().setLocal(0xFFFF-i)); // initiate connecting on all of them
   for(flt start=Time.curTime(); ; ) // wait until one connects
   {
      bool connecting=false;
      FREPA(temp)
      {
         Connection &conn=temp[i];
         if(conn.receive(0)) // if received any data
            if(conn.data.getStr()=="Hello from Server") // if this is hello from our test server
         {
            Swap(conn, connection); // keep this as the main connection
            goto found; // stop searching
         }
         if(conn.state()==CONNECT_CONNECTING || conn.state()==CONNECT_AWAIT_GREET || conn.state()==CONNECT_GREETED)connecting=true;
      }
      if(!connecting)Exit("Couldn't find server"); // if no connection is in progress then fail
      if(Time.curTime()-start>=2)Exit("Connection timeout"); // wait up to 2 seconds
      Time.wait(1); // wait a bit
   }
found:
   return true;
}
/******************************************************************************/
void Shut()
{
   connection.del();
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   // move window on mouse button
   if(Ms.b(0))WindowMove(Ms.pixelDelta().x, Ms.pixelDelta().y);

   // check if received any data
   if(connection.receive(0))
   {
      connection.data.getStr(data); // read data as string
   }

   // send random text when space pressed
   if(Kb.bp(KB_SPACE))
   {
      File f; f.writeMem();
      f.putStr(S+"Random Text "+Random(1024));
      f.pos(0);
      connection.send(f);
   }

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   if(connection.state()!=CONNECT_GREETED)
   {
      D.text(0, 0, S+"Invalid connection");
   }else
   {
      D.text(0, 0.4, S+"Server Address: "+connection.address().asText());
      D.text(0, 0.3, S+"Press Space to send random text");
      D.text(0, 0.0, S+"Received Data: "+data);
   }
}
/******************************************************************************/
