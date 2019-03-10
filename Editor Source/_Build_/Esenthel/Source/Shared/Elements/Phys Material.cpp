/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   bool EditPhysMtrl::newer(C EditPhysMtrl &src)C
   {
      return friction_static_time>src.friction_static_time || friction_dynamic_time>src.friction_dynamic_time || bounciness_time>src.bounciness_time || density_time>src.density_time
          || damping_time>src.density_time || adamping_time>src.adamping_time || friction_mode_time>src.friction_mode_time || bounciness_mode_time>src.bounciness_mode_time;
   }
   bool EditPhysMtrl::equal(C EditPhysMtrl &src)C
   {
      return friction_static_time==src.friction_static_time && friction_dynamic_time==src.friction_dynamic_time && bounciness_time==src.bounciness_time && density_time==src.density_time
          && damping_time==src.density_time && adamping_time==src.adamping_time && friction_mode_time==src.friction_mode_time && bounciness_mode_time==src.bounciness_mode_time;
   }
   void EditPhysMtrl::reset() {T=EditPhysMtrl();}
   void EditPhysMtrl::newData()
   {
      friction_static_time++; friction_dynamic_time++; bounciness_time++; density_time++; damping_time++; adamping_time++; friction_mode_time++; bounciness_mode_time++;
   }
   bool EditPhysMtrl::sync(C EditPhysMtrl &src)
   {
      bool changed=false;
      changed|=Sync( friction_static_time, src. friction_static_time, friction_static , src.friction_static );
      changed|=Sync(friction_dynamic_time, src.friction_dynamic_time, friction_dynamic, src.friction_dynamic);
      changed|=Sync(      bounciness_time, src.      bounciness_time,       bounciness, src.      bounciness);
      changed|=Sync(         density_time, src.         density_time,          density, src.         density);
      changed|=Sync(         damping_time, src.         damping_time,          damping, src.         damping);
      changed|=Sync(        adamping_time, src.        adamping_time,         adamping, src.        adamping);
      changed|=Sync(   friction_mode_time, src.   friction_mode_time,    friction_mode, src.   friction_mode);
      changed|=Sync( bounciness_mode_time, src. bounciness_mode_time,  bounciness_mode, src. bounciness_mode);
      return changed;
   }
   bool EditPhysMtrl::undo(C EditPhysMtrl &src)
   {
      bool changed=false;
      changed|=Undo( friction_static_time, src. friction_static_time, friction_static , src.friction_static );
      changed|=Undo(friction_dynamic_time, src.friction_dynamic_time, friction_dynamic, src.friction_dynamic);
      changed|=Undo(      bounciness_time, src.      bounciness_time,       bounciness, src.      bounciness);
      changed|=Undo(         density_time, src.         density_time,          density, src.         density);
      changed|=Undo(         damping_time, src.         damping_time,          damping, src.         damping);
      changed|=Undo(        adamping_time, src.        adamping_time,         adamping, src.        adamping);
      changed|=Undo(   friction_mode_time, src.   friction_mode_time,    friction_mode, src.   friction_mode);
      changed|=Undo( bounciness_mode_time, src. bounciness_mode_time,  bounciness_mode, src. bounciness_mode);
      return changed;
   }
   void EditPhysMtrl::copyTo(PhysMtrl &dest)C
   {
      dest.create()
          .frictionStatic(friction_static).frictionDynamic(friction_dynamic).bounciness(bounciness).density(density)
          .damping(damping).adamping(adamping).frictionMode(friction_mode).bouncinessMode(bounciness_mode);
   }
   bool EditPhysMtrl::save(File &f)C
   {
      f.cmpUIntV(0);
      f<<friction_static<<friction_dynamic<<bounciness<<density<<damping<<adamping
       <<friction_mode<<bounciness_mode
       <<friction_static_time<<friction_dynamic_time<<bounciness_time<<density_time<<damping_time<<adamping_time<<friction_mode_time<<bounciness_mode_time;
      return f.ok();
   }
   bool EditPhysMtrl::load(File &f)
   {
      switch(f.decUIntV())
      {
         case 0:
         {
            f>>friction_static>>friction_dynamic>>bounciness>>density>>damping>>adamping
             >>friction_mode>>bounciness_mode
             >>friction_static_time>>friction_dynamic_time>>bounciness_time>>density_time>>damping_time>>adamping_time>>friction_mode_time>>bounciness_mode_time;
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   bool EditPhysMtrl::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
EditPhysMtrl::EditPhysMtrl() : friction_static(1), friction_dynamic(1), bounciness(0.2f), density(1), damping(0.05f), adamping(0.05f), friction_mode(PhysMtrl::MODE_AVG), bounciness_mode(PhysMtrl::MODE_AVG) {}

/******************************************************************************/
