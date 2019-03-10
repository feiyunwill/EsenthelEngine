/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_PHMT CC4('P','H','M','T') // Physics Material
/******************************************************************************/
Cache<PhysMtrl> PhysMtrls("Physics Material");
/******************************************************************************/
PhysMtrl::PhysMtrl()
{
  _m=null;
  _bounciness=_friction_static=_friction_dynamic=_density=_damping=_adamping=-1; // set -1 so when setting initial values in 'reset' they will always be changed
  _bounciness_mode=_friction_mode=MODE_AVG;
}
/******************************************************************************/
void PhysMtrl::del()
{
   if(this!=&Physics.mtrl_default) // don't allow deleting the default material because it is required to exist on PhysX
      if(_m)
   {
      SafeWriteLock lock(Physics._rws);
      if(_m)
      {
      #if PHYSX
         if(Physx.physics){_m->userData=null; _m->release();} // set user data to null in case material is ref counted and released later
      #endif
        _m=null;
      }
   }
}
PhysMtrl& PhysMtrl::create() // don't delete so pointers don't need to be reallocated, instead just reset its values
{
#if PHYSX
   if(!_m)
   {
      WriteLock lock(Physics._rws);
      if(!_m)
      {
         if(Physx.physics)if(_m=Physx.physics->createMaterial(0, 0, 0))_m->userData=this;
      }
   }
#endif
   reset();
   return T;
}
void PhysMtrl::reset()
{
   bounciness(0).frictionStatic(0).frictionDynamic(0).bouncinessMode(MODE_AVG).frictionMode(MODE_AVG).anisotropic(false).density(1).damping(0.05f).adamping(0.05f);
}
/******************************************************************************/
Flt PhysMtrl::bounciness     ()C {return _bounciness      ;}
Flt PhysMtrl::frictionStatic ()C {return _friction_static ;}
Flt PhysMtrl::frictionDynamic()C {return _friction_dynamic;}
Flt PhysMtrl:: density       ()C {return _density         ;}   PhysMtrl& PhysMtrl:: density(Flt x) {MAX(x, 0); _density =x; return T;}
Flt PhysMtrl:: damping       ()C {return _damping         ;}   PhysMtrl& PhysMtrl:: damping(Flt x) {MAX(x, 0); _damping =x; return T;}
Flt PhysMtrl::adamping       ()C {return _adamping        ;}   PhysMtrl& PhysMtrl::adamping(Flt x) {MAX(x, 0); _adamping=x; return T;}
/******************************************************************************/
#if PHYSX
/*Flt PhysMtrl::bounciness      ()C {return _m ? _m->getRestitution    () : 0;}*/   PhysMtrl& PhysMtrl::bounciness      (Flt x) {SAT(x   ); if(_bounciness      !=x){_bounciness      =x; if(_m)_m->setRestitution    (x);} return T;}
/*Flt PhysMtrl::frictionStatic  ()C {return _m ? _m->getStaticFriction () : 0;}*/   PhysMtrl& PhysMtrl::frictionStatic  (Flt x) {MAX(x, 0); if(_friction_static !=x){_friction_static =x; if(_m)_m->setStaticFriction (x);} return T;}
/*Flt PhysMtrl::frictionDynamic ()C {return _m ? _m->getDynamicFriction() : 0;}*/   PhysMtrl& PhysMtrl::frictionDynamic (Flt x) {MAX(x, 0); if(_friction_dynamic!=x){_friction_dynamic=x; if(_m)_m->setDynamicFriction(x);} return T;}
  Flt PhysMtrl::frictionStaticA ()C {return                                 0;}     PhysMtrl& PhysMtrl::frictionStaticA (Flt x) {return T;}
  Flt PhysMtrl::frictionDynamicA()C {return                                 0;}     PhysMtrl& PhysMtrl::frictionDynamicA(Flt x) {return T;}

Bool      PhysMtrl::anisotropic(       )C {return false;}
PhysMtrl& PhysMtrl::anisotropic(Bool on)  {return T;}

Vec       PhysMtrl::anisotropicDir(          )C {return Vec(0, 1, 0);}
PhysMtrl& PhysMtrl::anisotropicDir(C Vec &dir)  {return T;}

PhysMtrl& PhysMtrl::bouncinessMode(MODE mode)
{
   if(_m)switch(mode)
   {
      case MODE_AVG: _m->setRestitutionCombineMode(PxCombineMode::eAVERAGE ); break;
      case MODE_MUL: _m->setRestitutionCombineMode(PxCombineMode::eMULTIPLY); break;
      case MODE_MIN: _m->setRestitutionCombineMode(PxCombineMode::eMIN     ); break;
      case MODE_MAX: _m->setRestitutionCombineMode(PxCombineMode::eMAX     ); break;
   }
   return T;
}
PhysMtrl& PhysMtrl::frictionMode(MODE mode)
{
   if(_m)switch(mode)
   {
      case MODE_AVG: _m->setFrictionCombineMode(PxCombineMode::eAVERAGE ); break;
      case MODE_MUL: _m->setFrictionCombineMode(PxCombineMode::eMULTIPLY); break;
      case MODE_MIN: _m->setFrictionCombineMode(PxCombineMode::eMIN     ); break;
      case MODE_MAX: _m->setFrictionCombineMode(PxCombineMode::eMAX     ); break;
   }
   return T;
}
PhysMtrl::MODE PhysMtrl::bouncinessMode()C
{
   if(_m)switch(_m->getRestitutionCombineMode())
   {
      case PxCombineMode::eAVERAGE : return MODE_AVG;
      case PxCombineMode::eMULTIPLY: return MODE_MUL;
      case PxCombineMode::eMIN     : return MODE_MIN;
      case PxCombineMode::eMAX     : return MODE_MAX;
   }
   return MODE_AVG;
}
PhysMtrl::MODE PhysMtrl::frictionMode()C
{
   if(_m)switch(_m->getFrictionCombineMode())
   {
      case PxCombineMode::eAVERAGE : return MODE_AVG;
      case PxCombineMode::eMULTIPLY: return MODE_MUL;
      case PxCombineMode::eMIN     : return MODE_MIN;
      case PxCombineMode::eMAX     : return MODE_MAX;
   }
   return MODE_AVG;
}
#else
static void Changed(PhysMtrl *material)
{
   /*if(material)
   {
      SyncLocker locker(Physics._lock);

      if(Bullet.world)
         REP(Bullet.world->getNumCollisionObjects())
	         if(RigidBody *rb=CAST(RigidBody, Bullet.world->getCollisionObjectArray()[i]))
	            if(rb->material==material)rb->materialApply();
	}*/
}

PhysMtrl& PhysMtrl::bounciness     (Flt x) {SAT(x   ); if(_bounciness      !=x){_bounciness      =x; Changed(this);} return T;}
PhysMtrl& PhysMtrl::frictionStatic (Flt x) {MAX(x, 0); if(_friction_static !=x){_friction_static =x; Changed(this);} return T;}
PhysMtrl& PhysMtrl::frictionDynamic(Flt x) {MAX(x, 0); if(_friction_dynamic!=x){_friction_dynamic=x; Changed(this);} return T;}

// TODO: Bullet
Bool      PhysMtrl::anisotropic   (          )C {return 0;}
PhysMtrl& PhysMtrl::anisotropic   (  Bool on )  {return T;}
Vec       PhysMtrl::anisotropicDir(          )C {return 0;}
PhysMtrl& PhysMtrl::anisotropicDir(C Vec &dir)  {return T;}

Flt PhysMtrl::frictionStaticA ()C {return 0;}   PhysMtrl& PhysMtrl::frictionStaticA (Flt x) {return T;}
Flt PhysMtrl::frictionDynamicA()C {return 0;}   PhysMtrl& PhysMtrl::frictionDynamicA(Flt x) {return T;}

PhysMtrl::MODE PhysMtrl::bouncinessMode()C {return _bounciness_mode;}   PhysMtrl& PhysMtrl::bouncinessMode(MODE mode) {_bounciness_mode=mode; return T;}
PhysMtrl::MODE PhysMtrl::frictionMode  ()C {return _friction_mode  ;}   PhysMtrl& PhysMtrl::frictionMode  (MODE mode) {_friction_mode  =mode; return T;}
#endif
/******************************************************************************/
Bool PhysMtrl::save(File &f)C
{
   f.putMulti(UInt(CC4_PHMT), Byte(0)); // version
   f.putFlt(bounciness()).putFlt(frictionStatic()).putFlt(frictionDynamic()).putFlt(density()).putFlt(damping()).putFlt(adamping()).putByte(bouncinessMode()).putByte(frictionMode());
   if(!anisotropic())f.putBool(false);else{f.putBool(true).putFlt(frictionStaticA()).putFlt(frictionDynamicA())<<anisotropicDir();}
   return f.ok();
}
Bool PhysMtrl::load(File &f)
{
   create();
   if(f.getUInt()==CC4_PHMT)switch(f.decUIntV())
   {
      case 0:
      {
         bounciness     (f.getFlt());
         frictionStatic (f.getFlt());
         frictionDynamic(f.getFlt());
         density        (f.getFlt());
          damping       (f.getFlt());
         adamping       (f.getFlt());
         bouncinessMode ((MODE)f.getByte());
           frictionMode ((MODE)f.getByte());
         if(!f.getBool())anisotropic(false);else
         {
                             anisotropic     (true);
                             frictionStaticA (f.getFlt());
                             frictionDynamicA(f.getFlt());
            Vec dir; f>>dir; anisotropicDir  (dir);
         }
         if(f.ok())return true;
      }break;
   }
   create(); return false;
}
Bool PhysMtrl::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool PhysMtrl::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   create(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
