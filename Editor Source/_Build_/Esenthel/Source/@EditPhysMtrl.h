/******************************************************************************/
/******************************************************************************/
class EditPhysMtrl
{
   flt           friction_static, friction_dynamic, bounciness, density, damping, adamping;
   PhysMtrl::MODE friction_mode, bounciness_mode;
   TimeStamp     friction_static_time, friction_dynamic_time, bounciness_time, density_time, damping_time, adamping_time, friction_mode_time, bounciness_mode_time;

   bool newer(C EditPhysMtrl &src)C;
   bool equal(C EditPhysMtrl &src)C;
   
   // operations
   void reset();  
   void newData();
   bool sync(C EditPhysMtrl &src);
   bool undo(C EditPhysMtrl &src);
   void copyTo(PhysMtrl &dest)C;

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   EditPhysMtrl();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
