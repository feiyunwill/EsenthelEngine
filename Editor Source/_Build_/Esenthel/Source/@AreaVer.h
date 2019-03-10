/******************************************************************************/
/******************************************************************************/
class AreaVer
{
   TimeStamp hm_removed_time, hm_height_time, hm_mtrl_time, hm_color_time,
             rebuild_time; // 'rebuild_time' doesn't need to be saved, it is only a helper member specifying when was the last rebuild ordered
   Version   obj_ver;

   static bool HasHm(C TimeStamp&removed_time, C TimeStamp&height_time);
          bool hasHm()C;                                               // if has heightmap (not removed)

   // get
   bool newerHm(C AreaVer &ver)C;
   bool oldHm(C TimeStamp &now=TimeStamp().getUTC())C;
   bool old(C TimeStamp &now=TimeStamp().getUTC())C;
   uint compare(C AreaVer *dest)C; // return which elements from 'this' should be sent to 'dest'

   // operations
   uint setHm(bool on); // make sure that 'hasHm' will return 'on'
   bool sync(C AreaVer &src);
   uint sync(C AreaVer &src, Heightmap &hm, C Heightmap &src_hm, uint mask);
   uint undo(C AreaVer &src, Heightmap &hm, C Heightmap &src_hm);

   // io
   bool save(File &f)C;
   bool load(File &f);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
