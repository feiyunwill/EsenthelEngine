/******************************************************************************/
class ClientServer : ConnectionServer // server class that manages all client connections
{
   class Client : ConnectionServer.Client
   {
      bool sent_initial_data=false;
      Str  email, password;
      Vec2 position(0, 0);

      virtual void create(ConnectionServer &server)
      {
         // notify other clients that new client entered the game (add neighbor)
         {
            // setup data
            ClientInfo ci;
            ci.addr=connection.address();

            // send to all clients
            REPA(Server.clients)
            {
               Client &client=Server.client(i);
               if(&client!=this)ServerSendAddNeighbor(client.connection, ci);
            }
         }
      }

      virtual bool update()
      {
         if(!super.update())return false;

         if(!sent_initial_data) // if not yet sent initial data
            if(connection.state()==CONNECT_GREETED) // if connection is fully completed (we can actually send data)
         {
            sent_initial_data=true;

            // send greeting
            File f; f.writeMem().putByte(CS_WELCOME).putStr(CSWelcomeMessage).pos(0); // write a greeting
            connection.send(f); // send it to the client

            // notify this client about other clients in the game
            REPA(Server.clients)
            {
               Client &client=Server.client(i);
               if(&client!=this)
               {
                  // setup data
                  ClientInfo ci;
                  ci.addr=client.connection.address();

                  // send to this client
                  ServerSendAddNeighbor(connection, ci);
               }
            }
         }

         REP(8)if(!connection.receive(0))return true;else // process 8 commands at once
         {
            byte cmd=connection.data.getByte();

            switch(cmd)
            {
               case CS_LOGIN:
               {
                  ServerReceiveLogin(connection.data, email, password);
                  // perform login here
               }break;

               case CS_POSITION:
               {
                  ServerReceivePosition(connection.data, position); // receive position
                  File f; ServerWritePosition(f, connection.address(), position); // write data to a file
                  Memc<Client*> neighbors; REPA(Server.clients){Client &client=Server.client(i); if(&client!=this)neighbors.add(&client);} // get list of neighbors
                  Distribute(f, neighbors); // send position data to all neighbors
               }break;
            }
         }

         return true;
      }
      
     ~Client()
      {
         // notify other clients that this client no longer exists (delete neighbor)
         REPA(Server.clients)
            ServerSendDelNeighbor(Server.client(i).connection, connection.address());
      }
   }

   void update()
   {
      super.update();

      // flush all queued commands
      REPA(clients)client(i).connection.flush();
   }

   Client &client(int i) {return (Client&)clients[i];}

   ClientServer() {clients.replaceClass<Client>();} // use extended client class
}
ClientServer Server;
/******************************************************************************/
