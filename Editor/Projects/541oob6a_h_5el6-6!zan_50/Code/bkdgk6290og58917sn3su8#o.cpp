/******************************************************************************/
class SaveGame
{
   class WorldState
   {
      File world_state;
   }

   Map<UID, WorldState> world_states(Compare); // map of multiple world states sorted by world ID

   // operations
   void saveCurrentWorld()
   {
      UID world_id=Game.World.id();
      if( world_id.valid()) // if world id is valid
      {
         WorldState &ws=*world_states(world_id); // get world state for that world
         Game.World.save(ws.world_state.writeMem()); // save current world state into file object
      }
   }
   void changeWorld(C UID &world_id)
   {
      // save player data to a temporary file
      File player_data; player_data.writeMem();
      if(Players.elms())
      {
         Players[0].save(player_data);
         // after saving the player, remove it from the object container so that it will not be included in the old world state
         // without this there would be 2 copies of the same player, one in the old world and one in the new world
         Players.remove(0);
      }

      // save old world state
      saveCurrentWorld();
      
      // load new world
      if(world_id.valid()) // if it's valid
      {
         if(WorldState *ws=world_states.find(world_id)) // if world state of 'world_id' is already stored in the save game
         {
            ws.world_state.pos(0); // reset file position
            Game.World.load(ws.world_state); // load world from last saved state
         }
         else // if the 'world_id' was not yet stored in save game
         {
            Game.World.New(world_id);
         }

         // restore the player in the new world
         if(player_data.size()) // if the file contains any data
         {
            player_data.pos(0); // reset file position to the start
            Game.World.objInject(OBJ_CHR, player_data, null); // specify null to don't change player position after loading
         }
      }else // we want to set empty world
      {
         Game.World.del(); // just delete current one
      }
   }
}
/******************************************************************************/
