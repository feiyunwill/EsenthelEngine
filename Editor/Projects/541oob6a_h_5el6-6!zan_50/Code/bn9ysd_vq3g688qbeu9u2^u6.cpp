/******************************************************************************/
enum CLIENT_SERVER_COMMANDS
{
   CS_WELCOME     , // welcome message
   CS_LOGIN       , // login account
   CS_POSITION    , // exchange player position
   CS_DEL_NEIGHBOR, // remove information about another player
   CS_ADD_NEIGHBOR, // add    information about another player that we've come into contact
}
/******************************************************************************/
const Str CSWelcomeMessage="Test Server";
/******************************************************************************/
// CS_LOGIN
/******************************************************************************/
void ClientSendLogin(Connection &conn, C Str &email, C Str &pass)
{
   File f; f.writeMem().putByte(CS_LOGIN).putStr(email).putStr(pass).pos(0); conn.send(f);
}
void ServerReceiveLogin(File &f, Str &email, Str &pass)
{
   f.getStr(email);
   f.getStr(pass );
}
/******************************************************************************/
// CS_POSITION
/******************************************************************************/
void ClientSendPosition(Connection &conn, C Vec2 &pos)
{
   File f; f.writeMem().putByte(CS_POSITION)<<pos; f.pos(0); conn.send(f);
}
void ServerReceivePosition(File &f, Vec2 &pos)
{
   f>>pos;
}
void ServerWritePosition(File &f, C SockAddr &addr, C Vec2 &pos)
{
   f.writeMem().putByte(CS_POSITION); addr.save(f); f<<pos;
}
void ClientReceivePosition(File &f, SockAddr &addr, Vec2 &pos)
{
   addr.load(f); f>>pos;
}
/******************************************************************************/
// CS_DEL_NEIGHBOR
/******************************************************************************/
void ServerSendDelNeighbor(Connection &conn, C SockAddr &addr)
{
   File f; f.writeMem().putByte(CS_DEL_NEIGHBOR); addr.save(f); f.pos(0); conn.send(f, -1, false);
}
void ClientReceiveDelNeighbor(File &f, SockAddr &addr)
{
   addr.load(f);
}
/******************************************************************************/
// CS_ADD_NEIGHBOR
/******************************************************************************/
class ClientInfo
{
   SockAddr addr;
   Str      name;

   void save(File &f)
   {
      addr.save(f);
      f.putStr(name);
   }
   void load(File &f)
   {
      addr.load(f);
      f.getStr(name);
   }
}
void ServerSendAddNeighbor(Connection &conn, ClientInfo &ci)
{
   File f; f.writeMem().putByte(CS_ADD_NEIGHBOR); ci.save(f); f.pos(0); conn.send(f, -1, false);
}
void ClientReceiveAddNeighbor(File &f, ClientInfo &ci)
{
   ci.load(f);
}
/******************************************************************************/
