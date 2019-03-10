/******************************************************************************/
/******************************************************************************/
class WorldChange : Edit::_Undo::Change
{
   class Terrain : Heightmap
   {
      AreaVer ver;

      void set(C Area &area);
   };

   // need to be thread-safe
   ThreadSafeMap<VecI2, Terrain     >   terrain;
   ThreadSafeMap<UID  , ObjData     >      objs;
   ThreadSafeMap<UID  , EditWaypoint> waypoints;
   ThreadSafeMap<UID  ,  LakeBase   >     lakes;
   ThreadSafeMap<UID  , RiverBase   >    rivers;

   virtual uint memUsage()C override;
   virtual void swap(ptr user)override;

public:
   WorldChange();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
