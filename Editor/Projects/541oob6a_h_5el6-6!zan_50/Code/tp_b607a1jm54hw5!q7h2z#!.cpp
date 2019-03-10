/******************************************************************************

   This tutorial presents a sample Client Connection.

   It can be used together with second tutorial "Connection 2/Server"

/******************************************************************************/
Connection Server;
flt        TimeToSendPosition;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.x=1;
   App.flag=APP_WORK_IN_BACKGROUND|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE;
   D.mode(400, 300).scale(2);
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
            if(conn.data.getByte()==CS_WELCOME)
            if(conn.data.getStr()==CSWelcomeMessage) // if this is hello from our test server
         {
            Swap(conn, Server); // keep this as the main connection
            goto found; // stop searching
         }
         if(conn.state()==CONNECT_CONNECTING || conn.state()==CONNECT_AWAIT_GREET || conn.state()==CONNECT_GREETED)connecting=true;
      }
      if(!connecting)Exit("Couldn't find server"); // if no connection is in progress then fail
      if(Time.curTime()-start>=2)Exit("Connection timeout"); // wait up to 2 seconds
      Time.wait(1); // wait a bit
   }
found:
   
   InitGui();

   return true;
}
void Shut()
{
   Server.del();
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Gui.update();

   // receive data from server
   REP(16) // process 16 commands at once
      if(!Server.receive(0))break; // no command receive then break
      else switch(Server.data.getByte()) // otherwise process the command
   {
      case CS_DEL_NEIGHBOR:
      {
         SockAddr addr; ClientReceiveDelNeighbor(Server.data, addr);
         Neighbors.removeKey(addr); // remove neighbor
      }break;

      case CS_ADD_NEIGHBOR:
      {
         ClientInfo ci; ClientReceiveAddNeighbor(Server.data, ci);
         Neighbor *nieghbor=Neighbors(ci.addr); // create neighbor
      }break;

      case CS_POSITION:
      {
         SockAddr addr; Vec2 pos; ClientReceivePosition(Server.data, addr, pos);
         if(Neighbor *neighbor=Neighbors.find(addr))neighbor.pos=pos; // set neighbor position
      }break;
   }

   // send player position
      TimeToSendPosition-=Time.rd();
   if(TimeToSendPosition<=0)
   {
      TimeToSendPosition=0.1;
      ClientSendPosition(Server, Plr.pos);
   }

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   Gui.draw();
   DrawMap();
}
/******************************************************************************/
