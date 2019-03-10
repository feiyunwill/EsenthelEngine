/******************************************************************************/
/******************************************************************************/
class ElmWorld : ElmData
{
   int       area_size, hm_res;
   flt       ctrl_r, ctrl_h, max_climb, max_slope, cell_size, cell_h; // path settings
   UID       env_id; // environment settings
   TimeStamp hm_res_time, ctrl_r_time, ctrl_h_time, max_climb_time, max_slope_time, cell_size_time, cell_h_time, env_time;

   // get
   bool valid()C;               // if have known information about this world
   bool equal(C ElmWorld&src)C;
   bool newer(C ElmWorld&src)C;
   int hmRes()C;            

   // set
   ElmWorld& ctrlRadius(flt r    );
   ElmWorld& ctrlHeight(flt h    );
   ElmWorld& maxClimb  (flt climb);
   ElmWorld& maxSlope  (flt slope);
   ElmWorld& cellSize  (flt size );
   ElmWorld& cellHeight(flt h    );
   ElmWorld& env       (C UID &id);

   // operations
   virtual void newData()override;
   void copyTo(Game::WorldSettings &settings, C Project &proj)C;
   void copyTo(PathSettings &settings)C;
   uint undo(C ElmWorld &src);
   uint sync(C ElmWorld &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmWorld();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
