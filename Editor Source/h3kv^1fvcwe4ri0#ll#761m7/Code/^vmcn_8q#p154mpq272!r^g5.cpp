/******************************************************************************/
class ClientServer : ConnectionServer // server class that manages all client connections
{
   int  clients_peak=0;
   long bytes_sent=0, bytes_received=0; // bytes sent/received by inactive clients

   ClientServer() {clients.replaceClass<.Client>();} // use extended client class

   // get
  .Client& client(int i) {return (.Client&)clients[i];}

   long bytesSentTotal    () {long x=bytes_sent    ; REPA(clients)x+=client(i).connection.sent    (); return x;}
   long bytesReceivedTotal() {long x=bytes_received; REPA(clients)x+=client(i).connection.received(); return x;}

   // send
   void distribute(File &f, Project *project, .Client *except) // distribute data to all clients (except 'except') operating on 'project'
   {
      REPA(clients)
      {
        .Client &client=T.client(i);
         if(client.project==project && &client!=except)
         {
            f.pos(0); client.connection.send(f, -1, false);
         }
      }
   }
   void distributeProjList()
   {
      REPA(clients){.Client &client=T.client(i); if(client.user && client.user.canRead() && !client.project)ServerSendProjectsList(client.connection, Projects);}
   }
   void distributeTex(C UID &tex_id, Project *project, .Client *except) // distribute texture to all clients (except 'except') operating on 'project'
   {
      REPA(clients){.Client &client=T.client(i); if(client.project==project && &client!=except)client.send_tex.include(tex_id);}
   }
   void distributeElmShort(C UID &elm_id, Project *project, .Client *except) // distribute element to all clients (except 'except') operating on 'project'
   {
      REPA(clients){.Client &client=T.client(i); if(client.project==project && &client!=except)client.send_elm_short.include(elm_id);}
   }
   void distributeElmLong(C UID &elm_id, Project *project, .Client *except) // distribute element to all clients (except 'except') operating on 'project'
   {
      REPA(clients){.Client &client=T.client(i); if(client.project==project && &client!=except)client.send_elm_long.include(elm_id);}
   }
   void distributeElmFull(C UID &elm_id, Project *project, .Client *except) // distribute element to all clients (except 'except') operating on 'project'
   {
      REPA(clients){.Client &client=T.client(i); if(client.project==project && &client!=except)client.send_elm_full.include(elm_id);}
   }
   void distributeWorldArea(C UID &world_id, C VecI2 &area_xy, uint area_sync_flag, Project *project, .Client *except) // distribute world area to all clients (except 'except') operating on 'project'
   {
      REPA(clients){.Client &client=T.client(i); if(client.project==project && &client!=except)if(.Client.WorldSync *ws=client.world_sync.get(world_id))if(byte *flag=ws.areas.get(area_xy))*flag|=area_sync_flag;}
   }
   void distributeMiniMapImage(C UID &mini_map_id, C VecI2 &image_xy, Project *project, .Client *except) // distribute mini map image to all clients (except 'except') operating on 'project'
   {
      REPA(clients){.Client &client=T.client(i); if(client.project==project && &client!=except)if(.Client.MiniMapSync *mms=client.mini_map_sync.get(mini_map_id))mms.images.binaryInclude(image_xy, Compare);}
   }

   // operations
   void disconnect(User &user)
   {
      REPA(clients)if(client(i).user==&user)clients.remove(i); // process all clients as user can be accessed by many
   }
   LOGIN_RESULT connected(.Client &client) // DO NOT delete 'client' here, because this is called inside 'client.update' func
   {
      // disconnect same users with same OS (this allows 1 user to be connected through different OS)
      REPA(clients)
      {
        .Client &c=T.client(i);
         if(&c!=&client && c.user==client.user && SameOS(c.os_ver, client.os_ver))
            clients.remove(i);
      }

      // disallow connection if there is other user using the same license key
      /*if(client.license_key.is())
         REPA(clients)
      {
        .Client &c=T.client(i);
         if(&c!=&client && c.user!=client.user && c.license_key==client.license_key)return LOGIN_LICENSE_USED;
      }*/

      /*// disconnect demo users if number of total users >2
      if(clients.elms()>2) // if there are more than 2 users
      {
         if(!client.license_key.is())return LOGIN_DEMO_USER_NOT_ALLOWED; // if we're trying to connect using demo account
         // disconnect last demo user
         REPA(clients)
         {
           .Client &c=T.client(i);
            if(&c!=&client && c.user && !c.license_key.is()){clients.remove(i); break;}
         }
      }*/
      return LOGIN_SUCCESS;
   }

   // update
   void update()
   {
      super.update();
      MAX(clients_peak, clients.elms());

      // send all queued commands
      REPA(clients)client(i).connection.flush(1);
   }
}
ClientServer Server;
/******************************************************************************/
