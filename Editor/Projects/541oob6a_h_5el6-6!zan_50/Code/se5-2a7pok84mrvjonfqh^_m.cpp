/******************************************************************************/
void Distribute(File &f, Memc<ClientServer.Client*> &clients) // distribute file data to all listed clients
{
   REPA(clients)
   {
      f.pos(0); clients[i].connection.send(f, -1, false);
   }
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_WORK_IN_BACKGROUND|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE;
   D.mode(400, 300);
   D.scale(1.5);
}
bool Init()
{
   if(!Server.create())Exit("Can't create Server"); // create server
   return true;
}
/******************************************************************************/
void Shut()
{
   Server.del();
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   Server.update(); // update server, this accepts new clients and processes all existing
   Time.wait(1);

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   flt y=D.h()-0.1;
   D.text(0, y, Server.localAddress().asText()); y-=0.1;
   D.text(0, y, S+"Clients: "+Server.clients.elms()); y-=0.1;
   FREPA(Server.clients)
   {
      ClientServer.Client &client=Server.client(i);
      D.text(0, y, S+"id: "+i+", e-mail: "+client.email+", addr: "+client.connection.address().asText());
      y-=0.1;
   }
}
/******************************************************************************/
