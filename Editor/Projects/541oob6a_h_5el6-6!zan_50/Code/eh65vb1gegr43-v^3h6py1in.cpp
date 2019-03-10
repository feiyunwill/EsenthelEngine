/******************************************************************************

   This tutorial presents a sample ConnectionServer,
      which receives data from clients, and then sends the same data to all other clients.

   It can be used together with second tutorial "Connection/Client"

/******************************************************************************/
class ClientServer : ConnectionServer
{
   class Client : ConnectionServer.Client
   {
      bool sent_hello=false;

      virtual bool update() // override client update method
      {
         if(super.update())
         {
            if(!sent_hello) // if not yet sent hello
               if(connection.state()==CONNECT_GREETED) // if connection is fully completed (we can actually send data)
            {
               sent_hello=true;
               File f; f.writeMem().putStr("Hello from Server").pos(0); // write a greeting
               connection.send(f); // send it to the client
            }

            if(connection.receive(0)) // if received any data from the client
            {
               // send it to all other clients
               REPA(Server.clients) // iterate all clients
               {
                  Client &client=Server.client(i);
                  if(this!=&client) // don't send back where it came from
                  {
                     connection.data.pos(0); // set received data position to start
                     client.connection.send(connection.data); // send received data to other client
                  }
               }
            }
            return true;
         }
         return false;
      }
   }

   Client &client(int i) {return (Client&)clients[i];}

   ClientServer() {clients.replaceClass<Client>();} // use custom client class
}
ClientServer Server;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_WORK_IN_BACKGROUND|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE; // specify work in background flag to work also when not focused
   D.mode(400, 300);
   D.scale(2);
}
bool Init()
{
   if(!Server.create())Exit("Can't create ConnectionServer"); // create server
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
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text(0, 0.4, S+"Clients: "       +Server.clients.elms());
   D.text(0, 0.2, S+"Local Address: " +Server. localAddress().asText());
 //D.text(0, 0.0, S+"Global Address: "+Server.globalAddress().asText());
}
/******************************************************************************/
