/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
ASSERT(WHEEL_LEFT_FRONT==0 && WHEEL_RIGHT_FRONT==1 && WHEEL_LEFT_REAR==2 && WHEEL_RIGHT_REAR==3); // used by 'IsFront', 'IsRear', 'IsLeft', 'IsRight'
/******************************************************************************/
// VEHICLE
/******************************************************************************/
static Int CompareVehicle(Vehicle*C &a, Vehicle*C &b) {return Compare(Ptr(a), Ptr(b));}

     Vehicle::Vehicle() {zero();}
void Vehicle::zero()
{
  _accel=_brake=_angle=_max_accel=_max_back_accel=_max_brake=_brake_ratio=_max_angle=_fric_linear=_fric_scalar=_susp_spring=_susp_damping=_susp_compress=_scale=0;
  _wd=AWD;
   REPA(_wheel)
   {
     _Wheel &w=_wheel[i];
      w.pos.zero();
      w.contact.pos.zero();
      w.contact.normal.zero();
      w.radius=w.compress=w.angle=w.angle_vel=0;
   }
}
Vehicle& Vehicle::del()
{
   if(is())
   {
      SafeWriteLock lock(Physics._rws);
      Physics._vehicles.binaryExclude(this, CompareVehicle);
     _actor.del();
      zero();
   }
   return T;
}
Vehicle& Vehicle::create(C PhysBody &body, C Params &params, Flt density, Flt scale)
{
   if(!createTry(body, params, density, scale))Exit("Can't create Vehicle");
   return T;
}
Bool Vehicle::createTry(C PhysBody &body, C Params &params, Flt density, Flt scale)
{
   WriteLock lock(Physics._rws);
   del();
   if(_actor.createTry(body, density, scale))
   {
     _max_accel=10; _max_back_accel=4.5f; _max_brake=12; _brake_ratio=0.43f; _max_angle=PI_3; _fric_linear=13; _fric_scalar=0.3f; _susp_spring=20; _susp_damping=0.7f; _susp_compress=0.75f; _scale=scale;
     _wd=AWD;
      Flt wheels_density    =body.finalDensity()*density*0.5f, // assume that wheel density is half of the chassis density
          wheels_mass       =0;
      Vec wheels_mass_center=0,
          wheels_center     =0;
      REPA(_wheel)
      {
        _Wheel &dest=      _wheel[i];
       C Wheel &src =params.wheel[i];
         dest.pos   =src.pos   *scale;
         dest.radius=src.radius*scale;
         dest.contact.normal.set(0, 1, 0);
         Flt wheel_volume=Tube(dest.radius, src.width*scale).volume(),
             wheel_mass  =wheels_density*wheel_volume;
         wheels_center     +=           dest.pos;
         wheels_mass_center+=wheel_mass*dest.pos;
         wheels_mass       +=wheel_mass;
      }
      // adjust mass center by wheels
      if(Flt total_mass=mass()+wheels_mass)
      {
         massCenterL((massCenterL()*mass() + wheels_mass_center)/total_mass); // 'wheels_mass_center' is already multiplied by 'wheels_mass'
         mass(total_mass);
      }

      wheels_center/=WHEEL_NUM; _avg_wheel_dist=0; REPA(_wheel)_avg_wheel_dist+=Dist(_wheel[i].pos, wheels_center); _avg_wheel_dist/=WHEEL_NUM; _avg_wheel_dist*=SQRT2_2; // mul by SQRT2_2 because it's applied to both X and Z

      // once the vehicle is ready, add it to the global container
      Physics._vehicles.binaryInclude(this, CompareVehicle);

      return true;
   }
   return false;
}
Bool Vehicle::is()C {return _actor.is();}

Int  Vehicle::onGroundNum(                )C {Int n=0; REPA(_wheel)if(_wheel[i].compress> 0)n++         ; return n    ;}
Bool Vehicle::onGroundAny(                )C {         REPA(_wheel)if(_wheel[i].compress> 0)return true ; return false;}
Bool Vehicle::onGroundAll(                )C {         REPA(_wheel)if(_wheel[i].compress<=0)return false; return true ;}
Bool Vehicle::onGround   (WHEEL_TYPE wheel)C {return InRange(wheel, _wheel) ? _wheel[wheel].compress>0 : false;}

Flt Vehicle::    speed()C {return Dot(_actor.vel(), _actor.orn().z);}
Flt Vehicle::sideSpeed()C {return Dot(_actor.vel(), _actor.orn().x);}

Vec Vehicle::wheelAxis(WHEEL_TYPE wheel)C
{
   Vec axis;
   if(IsFront(wheel))
   {
      Flt c, s; CosSin(c, s, _angle*_max_angle); axis.set(c, 0, -s)*=_actor.orn(); // setRotateY(angle).x
   }else axis=_actor.orn().x;
   // this should not change the direction for left wheels, 'wheelAxis' should always point to right direction
   return axis;
}
Vec Vehicle::wheelDir(WHEEL_TYPE wheel)C
{
   Vec dir;
   if(IsFront(wheel))
   {
      Flt c, s; CosSin(c, s, _angle*_max_angle); dir.set(s, 0, c)*=_actor.orn(); // setRotateY(angle).z
   }else dir=_actor.orn().z;
   return dir;
}
Matrix Vehicle::wheelMatrix(WHEEL_TYPE wheel, Int flip_side)C
{
   if(InRange(wheel, _wheel))
   {
   C _Wheel &w=_wheel[wheel];
      Matrix m;
      if(IsFront(wheel))m.orn().setRotateXY(w.angle, _angle*_max_angle);else m.orn().setRotateX(w.angle);
      m.pos=w.pos;
      m.pos.y+=w.compress*w.radius*_susp_compress;
      m*=_actor.matrix();
      if(flip_side && (flip_side>0)==IsRight(wheel))
      {
         m.x.chs();
         m.z.chs();
      }
      return m;
   }
   return MatrixIdentity;
}
Vec Vehicle::wheelVel   (WHEEL_TYPE wheel)C {return InRange(wheel, _wheel) ? _actor.pointVelL(_wheel[wheel].pos) : 0;}
Vec Vehicle::wheelAngVel(WHEEL_TYPE wheel)C {return InRange(wheel, _wheel) ? wheelAxis(wheel)*_wheel[wheel].angle_vel : 0;}

Vec Vehicle::wheelContact (WHEEL_TYPE wheel)C {return InRange(wheel, _wheel) ? _wheel[wheel].contact.pos    : 0;}
Vec Vehicle::wheelContactN(WHEEL_TYPE wheel)C {return InRange(wheel, _wheel) ? _wheel[wheel].contact.normal : 0;}
Flt Vehicle::wheelRadius  (WHEEL_TYPE wheel)C {return InRange(wheel, _wheel) ? _wheel[wheel].radius         : 0;}
Flt Vehicle::wheelCompress(WHEEL_TYPE wheel)C {return InRange(wheel, _wheel) ? _wheel[wheel].compress       : 0;}

Flt Vehicle::wheelLongSlip(WHEEL_TYPE wheel)C
{
   if(onGround(wheel) && InRange(wheel, _wheel))
   {
   C _Wheel &w=_wheel[wheel];
      Vec point_vel=_actor.pointVelW(w.contact.pos); // vehicle velocity at ground contact point
      // TODO: should wheel angular velocity be applied to 'point_vel' ?
      Vec wheel_dir  =wheelDir(wheel);
      Flt  long_speed=Dot(point_vel, wheel_dir);

      // TODO: should w.angle_vel be negative for IsLeft ?
		if(long_speed==0 && w.angle_vel==0)return 0;
		if(Abs(long_speed)>Abs(w.angle_vel*w.radius))return (w.angle_vel*w.radius - long_speed)/(Abs(long_speed)+0.1f);
		else                                         return (w.angle_vel*w.radius - long_speed)/(Abs(w.angle_vel*w.radius)+1.0f);
   }
   return 0;
}
Flt Vehicle::wheelLatSlip(WHEEL_TYPE wheel)C
{
   if(onGround(wheel) && InRange(wheel, _wheel))
   {
      // TODO: should wheel angular velocity be applied to 'point_vel' ?
         Vec  point_vel =_actor.pointVelW(_wheel[wheel].contact.pos); // vehicle velocity at ground contact point
         Vec  wheel_dir =wheelDir(wheel);
      if(Flt  long_speed=Dot(point_vel, wheel_dir))
      {
         Vec wheel_axis =wheelAxis(wheel);
         Flt   lat_speed=Dot(point_vel, wheel_axis);
	      return Atan(lat_speed/Abs(long_speed));
	   }
   }
   return 0;
}

         Flt         Vehicle::         accel()C {return _accel         ;}   Vehicle& Vehicle::         accel(Flt   accel   ) {T._accel         =Mid(accel   ,-1.0f, 1.0f); return T;}
         Flt         Vehicle::         brake()C {return _brake         ;}   Vehicle& Vehicle::         brake(Flt   brake   ) {T._brake         =Sat(brake               ); return T;}
         Flt         Vehicle::         angle()C {return _angle         ;}   Vehicle& Vehicle::         angle(Flt   angle   ) {T._angle         =Mid(angle   ,-1.0f, 1.0f); return T;}
         Flt         Vehicle::      maxAccel()C {return _max_accel     ;}   Vehicle& Vehicle::      maxAccel(Flt   accel   ) {T._max_accel     =Max(accel   , 0.0f      ); return T;}
         Flt         Vehicle::  maxBackAccel()C {return _max_back_accel;}   Vehicle& Vehicle::  maxBackAccel(Flt   accel   ) {T._max_back_accel=Max(accel   , 0.0f      ); return T;}
         Flt         Vehicle::      maxBrake()C {return _max_brake     ;}   Vehicle& Vehicle::      maxBrake(Flt   brake   ) {T._max_brake     =Max(brake   , 0.0f      ); return T;}
         Flt         Vehicle::    brakeRatio()C {return _brake_ratio   ;}   Vehicle& Vehicle::    brakeRatio(Flt   ratio   ) {T._brake_ratio   =Sat(ratio               ); return T;}
         Flt         Vehicle::      maxAngle()C {return _max_angle     ;}   Vehicle& Vehicle::      maxAngle(Flt   angle   ) {T._max_angle     =Mid(angle   , 0.0f, PI_2); return T;}
         Flt         Vehicle::frictionLinear()C {return _fric_linear   ;}   Vehicle& Vehicle::frictionLinear(Flt   friction) {T._fric_linear   =Max(friction, 0.0f      ); return T;}
         Flt         Vehicle::frictionScalar()C {return _fric_scalar   ;}   Vehicle& Vehicle::frictionScalar(Flt   friction) {T._fric_scalar   =Sat(friction            ); return T;}
         Flt         Vehicle::  suspSpring  ()C {return _susp_spring   ;}   Vehicle& Vehicle::  suspSpring  (Flt   spring  ) {T._susp_spring   =Max(spring  , 0.0f      ); return T;}
         Flt         Vehicle::  suspDamping ()C {return _susp_damping  ;}   Vehicle& Vehicle::  suspDamping (Flt   damping ) {T._susp_damping  =Max(damping , 0.0f      ); return T;}
         Flt         Vehicle::  suspCompress()C {return _susp_compress ;}   Vehicle& Vehicle::  suspCompress(Flt   compress) {T._susp_compress =Sat(compress            ); return T;}
Vehicle::WHEEL_DRIVE Vehicle::  wheelDrive  ()C {return _wd            ;}   Vehicle& Vehicle::  wheelDrive  (WHEEL_DRIVE wd) {T._wd            =wd                       ; return T;}

Flt     Vehicle::energy     (          )C {return _actor.energy     (   );}
Flt     Vehicle::damping    (          )C {return _actor.damping    (   );}   Vehicle& Vehicle:: damping   (  Flt      damping) {_actor. damping   (damping); return T;}
Flt     Vehicle::adamping   (          )C {return _actor.adamping   (   );}   Vehicle& Vehicle::adamping   (  Flt      damping) {_actor.adamping   (damping); return T;}
Flt     Vehicle::mass       (          )C {return _actor.mass       (   );}   Vehicle& Vehicle::mass       (  Flt      mass   ) {_actor.mass       (mass   ); return T;}
Vec     Vehicle::massCenterL(          )C {return _actor.massCenterL(   );}   Vehicle& Vehicle::massCenterL(C Vec     &center ) {_actor.massCenterL(center ); return T;}
Vec     Vehicle::massCenterW(          )C {return _actor.massCenterW(   );}   Vehicle& Vehicle::massCenterW(C Vec     &center ) {_actor.massCenterW(center ); return T;}
Vec     Vehicle::inertia    (          )C {return _actor.inertia    (   );}   Vehicle& Vehicle::inertia    (C Vec     &inertia) {_actor.inertia    (inertia); return T;}
Vec     Vehicle::pos        (          )C {return _actor.pos        (   );}   Vehicle& Vehicle::pos        (C Vec     &pos    ) {_actor.pos        (pos    ); return T;}
Matrix3 Vehicle::orn        (          )C {return _actor.orn        (   );}   Vehicle& Vehicle::orn        (C Matrix3 &orn    ) {_actor.orn        (orn    ); return T;}
Matrix  Vehicle::matrix     (          )C {return _actor.matrix     (   );}   Vehicle& Vehicle::matrix     (C Matrix  &matrix ) {_actor.matrix     (matrix ); return T;}
Vec     Vehicle::     vel   (          )C {return _actor.     vel   (   );}   Vehicle& Vehicle::   vel     (C Vec     &vel    ) {_actor.   vel     (vel    ); return T;}
Vec     Vehicle::  angVel   (          )C {return _actor.  angVel   (   );}   Vehicle& Vehicle::angVel     (C Vec     &vel    ) {_actor.angVel     (vel    ); return T;}
Vec     Vehicle::pointVelL  (C Vec &pos)C {return _actor.pointVelL  (pos);}
Vec     Vehicle::pointVelW  (C Vec &pos)C {return _actor.pointVelW  (pos);}
Box     Vehicle::box        (          )C {return _actor.box        (   );}

Vehicle& Vehicle::addTorque (C Vec &torque             ) {_actor.addTorque (torque      ); return T;}
Vehicle& Vehicle::addAngVel (C Vec &ang_vel            ) {_actor.addAngVel (ang_vel     ); return T;}
Vehicle& Vehicle::addForce  (C Vec &force              ) {_actor.addForce  (force       ); return T;}
Vehicle& Vehicle::addForce  (C Vec &force  , C Vec &pos) {_actor.addForce  (force  , pos); return T;}
Vehicle& Vehicle::addImpulse(C Vec &impulse            ) {_actor.addImpulse(impulse     ); return T;}
Vehicle& Vehicle::addImpulse(C Vec &impulse, C Vec &pos) {_actor.addImpulse(impulse, pos); return T;}
Vehicle& Vehicle::addVel    (C Vec &vel                ) {_actor.addVel    (vel         ); return T;}
Vehicle& Vehicle::addAccel  (C Vec &accel              ) {_actor.addAccel  (accel       ); return T;}

Bool      Vehicle::collision  ()C {return _actor.collision  ();}   Vehicle& Vehicle::collision  (Bool      on       ) {_actor.collision  (on       ); return T;}
Bool      Vehicle::gravity    ()C {return _actor.gravity    ();}   Vehicle& Vehicle::gravity    (Bool      on       ) {_actor.gravity    (on       ); return T;}
Bool      Vehicle::ray        ()C {return _actor.ray        ();}   Vehicle& Vehicle::ray        (Bool      on       ) {_actor.ray        (on       ); return T;}
Bool      Vehicle::sleep      ()C {return _actor.sleep      ();}   Vehicle& Vehicle::sleep      (Bool      sleep    ) {_actor.sleep      (sleep    ); return T;}
Flt       Vehicle::sleepEnergy()C {return _actor.sleepEnergy();}   Vehicle& Vehicle::sleepEnergy(Flt       energy   ) {_actor.sleepEnergy(energy   ); return T;}
Bool      Vehicle::ccd        ()C {return _actor.ccd        ();}   Vehicle& Vehicle::ccd        (Bool      on       ) {_actor.ccd        (on       ); return T;}
Ptr       Vehicle::user       ()C {return _actor.user       ();}   Vehicle& Vehicle::user       (Ptr       user     ) {_actor.user       (user     ); return T;}
Ptr       Vehicle::obj        ()C {return _actor.obj        ();}   Vehicle& Vehicle::obj        (Ptr       obj      ) {_actor.obj        (obj      ); return T;}
Byte      Vehicle::group      ()C {return _actor.group      ();}   Vehicle& Vehicle::group      (Byte      group    ) {_actor.group      (group    ); return T;}
Byte      Vehicle::dominance  ()C {return _actor.dominance  ();}   Vehicle& Vehicle::dominance  (Byte      dominance) {_actor.dominance  (dominance); return T;}
PhysMtrl* Vehicle::material   ()C {return _actor.material   ();}   Vehicle& Vehicle::material   (PhysMtrl *material ) {_actor.material   (material ); return T;}

static inline Bool HasAccel(WHEEL_TYPE wheel, Vehicle::WHEEL_DRIVE wd)
{
   switch(wd)
   {
      default          : return true; // AWD
      case Vehicle::FWD: return IsFront(wheel);
      case Vehicle::RWD: return IsRear (wheel);
   }
}

struct VehicleCallback : PhysHitCallback
{
   Actor &actor;
   Bool   has;
   Flt    dist;
   Plane  plane;
   Vec    up;

   VehicleCallback(Vehicle &vehicle, C Vec &up) : actor(vehicle._actor), up(up) {}

   virtual Bool hit(PhysHit &phys_hit)
   {
      if(phys_hit.collision)
         if(!has || phys_hit.dist<dist)
            if(Dot(phys_hit.plane.normal, up)>=0.1f) // ignore accidental obstacles that got under ray (for example if where the wheel is, there is no physical body on the vehicle, then due to lack of collisions the wheel position and the ray can get inside other actors)
               if(!actor.ignored(phys_hit)){has=true; dist=phys_hit.dist; plane=phys_hit.plane;}
      return true;
   }
};

inline Vec PointVel(C Vec &vel, C Vec &ang_vel, C Vec &com_offset               ) {return vel+Cross(ang_vel, com_offset);} // 'com_offset' must be in world space
inline Vec PointVel(C Vec &vel, C Vec &ang_vel, C Vec &mass_center, C Vec &point) {return PointVel(vel, ang_vel, point-mass_center);}

void Vehicle::update()
{
   Vec     wheel_pos[WHEEL_NUM];
   Matrix3 wheel_orn[WHEEL_NUM];
   Matrix  actor_matrix =_actor.matrix();
   Vec     actor_com    =_actor.massCenterW();
   Flt     actor_mass   =_actor.mass();
   Flt     accel        =_accel; if(accel)accel*=((_accel>0) ? T._max_accel : T._max_back_accel)*((T._wd==AWD) ? 0.25f : 0.5f)*actor_mass; // do not remove actor.mass and change 'addForce' to 'addAccel' because add accel at pos does not include mass for torque but inertia, it was tested that car doing fast turns on flat surfaced sticked to ground better when using 'addForce' instead of 'addAccel' (the simulation was more enjoyable to play), with 'addAccel' the car was losing ground contact and controls/friction were lost and it appeared jittery
   Flt     brake        =_brake; if(brake)brake*=_max_brake*Physics.time()*0.5f; // by half because of 2 wheels per front/rear

   VehicleCallback hit_callback(T, actor_matrix.y);

   REPA(_wheel)
   {
     _Wheel &wheel       =_wheel[i];
             wheel_pos[i]= wheel.pos*actor_matrix;

      Flt start=wheel.radius*0.5f, compress;
         hit_callback.has=false; Physics.ray(wheel_pos[i] + actor_matrix.y*start, actor_matrix.y*-(start+wheel.radius), hit_callback, Physics.collisionGroups(_actor.group()));
      if(hit_callback.has) // hit something
      {
         Flt dist=hit_callback.dist-start-wheel.radius;
         if( dist<0) // we're touching it
         {
            // wheel contact
            wheel.contact=hit_callback.plane;

            Flt wheel_susp=wheel.radius*_susp_compress;
            compress=-dist/wheel_susp;
            MIN(compress, 1);

            // wheel dir
            wheel_orn[i]=actor_matrix; if(IsFront(WHEEL_TYPE(i)))wheel_orn[i].rotateYL(_angle*_max_angle);

            // accelerate
            if(accel && HasAccel(WHEEL_TYPE(i), _wd))
            {
               Vec dir=PointOnPlane(wheel_orn[i].z, wheel.contact.normal); dir.setLength(accel);

            #if DEBUG
               //if(Kb.win())addForce(dir, wheel_pos[i]);else // don't use local pos to avoid generating torque here, rotation is handled below in the friction section
            #endif
                 addForce(dir);
            }
         }else compress=0;
      }else compress=0;

      // suspension force
      Flt spring_force= compress                *_susp_spring                , // how far     to bounce
          damper_force=(compress-wheel.compress)*_susp_damping/Physics.time(), // how quickly to stabilize
                 force=(spring_force + damper_force)*actor_mass; // do not remove 'actor.mass' and replace 'addForce' with 'addAccel' for the same reasons as above
      addForce(force*wheel.contact.normal, wheel_pos[i]);
      wheel.compress=compress;
   }

   Vec actor_vel     =_actor.vel   (),
       actor_ang_vel =_actor.angVel();
   Flt speed         =Dot(actor_vel, actor_matrix.z);
   Flt fric_lin_rot  =Mid(actor_vel.length()*-0.125f+1.0f, -1.0f, 0.0f), // should be "Sat((actor_vel.length()-8)/8)" but we negate it to optimize "addVel(-left_vel*fric_lin_rot)" into "addVel(left_vel*fric_lin_rot)" (now is in 0 .. -1 range)
       fric_lin_rot1 =(1+fric_lin_rot); // for velocity 0 this should be 1, for high velocity this should be 0 (should be "1+fric_lin_rot" but we've negated 'fric_lin_rot')
       fric_lin_rot *=0.25f; // mul by 0.25 by each 4 wheel
       fric_lin_rot1*=0.25f; // mul by 0.25 by each 4 wheel
   Flt scalar_mul    =1-_fric_scalar; if(scalar_mul>0)scalar_mul=Pow(scalar_mul, Physics.time()); // watch out for Pow(<=0, ..) which causes NaN
   Flt linear_add    =  _fric_linear*Physics.time();

   // friction
   REP(WHEEL_NUM)
   {
     _Wheel &wheel=_wheel[i];
      if(onGround(WHEEL_TYPE(i)))
      {
         wheel.angle_vel=speed/wheel.radius;

         // scalar friction multiplies current velocity
         // linear friction subtracts  current velocity (this friction can cause never ending actor rotation if it's moving fast, therefore rotation is applied only for low velocities)
         Vec   wheel_uni(IsRight(WHEEL_TYPE(i)) ? _avg_wheel_dist : -_avg_wheel_dist, 0, IsFront(WHEEL_TYPE(i)) ? _avg_wheel_dist : -_avg_wheel_dist); (wheel_uni*=actor_matrix.orn())+=actor_com; // set as uniform point along center of mass (reduces jittering)
         Vec   point_vel=PointVel(actor_vel, actor_ang_vel, actor_com, wheel_uni);
         Vec lateral_vel=wheel_orn[i].x*DistPointPlane(point_vel, wheel_orn[i].x);

         Vec  add_vel=lateral_vel*(scalar_mul-1); // use -1 because we need to first decrease the existing velocity ('addVel' is used below)
         Vec left_vel=lateral_vel* scalar_mul   ; // this velocity is still left after scalar friction (it is in direction of the velocity)
         left_vel.clipLength(linear_add); // limit to allowed linear friction

         // apply 'left_vel' divided in two parts, one at low velocities to 'add_vel' so it generates angular velocity, second to 'addVel' without angular velocites
         addVel(  left_vel*fric_lin_rot); // apply without angular velocities (the higher vehicle velocity then this has 1.0 blend), normally should be "addVel(-left_vel)" but we've negated 'fric_lin_rot'
         add_vel-=left_vel*fric_lin_rot1; // apply with    angular velocities (the lower  vehicle velocity then this has 1.0 blend)

         addImpulse(add_vel*actor_mass, wheel_uni); // apply, do not remove 'actor.mass' and replace 'addImpulse' with 'addVel' because of torque (mass<->inertia), this would work incorrect if vehicle is very big, and angular velocities would be applied too big because of big distance between point of applied force and center of mass
      }else
      {
         wheel.angle_vel*=Pow(0.9f, Physics.time());
      }
      wheel.angle+=wheel.angle_vel*Physics.time();
   }

   // braking
   if(brake)REP(WHEEL_NUM)if(onGround(WHEEL_TYPE(i)))
   {
     _Wheel &wheel    =_wheel[i];
      Vec    point_vel=PointVel(actor_vel, actor_ang_vel, actor_com, wheel_pos[i]);
      Vec   ground_vel=PointOnPlane(point_vel, wheel.contact.normal);
      ground_vel.clipLength(brake*(IsFront(WHEEL_TYPE(i)) ? _brake_ratio : 1-_brake_ratio));
      addImpulse(-ground_vel*actor_mass, wheel_pos[i]); // do not remove 'actor.mass' and replace 'addImpulse' with 'addVel' because of torque (mass<->inertia), this would work incorrect if vehicle is very big, and angular velocities would be applied too big because of big distance between point of applied force and center of mass
   }
}
/******************************************************************************/
Bool Vehicle::saveState(File &f)C
{
   f.cmpUIntV(0); // version
   if(_actor.saveState(f))
   {
      f<<_wd<<_accel<<_brake<<_angle<<_max_accel<<_max_back_accel<<_max_brake<<_brake_ratio<<_max_angle<<_fric_linear<<_fric_scalar<<_susp_spring<<_susp_damping<<_susp_compress; // scale is not saved
      FREPA(_wheel)
      {
      C _Wheel &w=_wheel[i];
         f<<w.compress<<w.angle<<w.angle_vel<<w.contact; // pos and radius are not saved
      }
      return f.ok();
   }
   return false;
}
Bool Vehicle::loadState(File &f) // don't delete on fail, as here we're loading only state
{
   switch(f.decUIntV()) // version
   {
      case 0: if(_actor.loadState(f))
      {
         f>>_wd>>_accel>>_brake>>_angle>>_max_accel>>_max_back_accel>>_max_brake>>_brake_ratio>>_max_angle>>_fric_linear>>_fric_scalar>>_susp_spring>>_susp_damping>>_susp_compress;
         FREPA(_wheel)
         {
           _Wheel &w=_wheel[i];
            f>>w.compress>>w.angle>>w.angle_vel>>w.contact;
         }
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
// PHYSX VEHICLE
/******************************************************************************/
#if SUPPORT_PHYSX_VEHICLE
/******************************************************************************/
struct PhysXVehicle // Physical actor of vehicle type (this class is available only in PhysX)
{
   enum DIFF_TYPE
   {
      DIFF_LS_4WD  , // limited slip differential for car with four  wheel drive
      DIFF_LS_FWD  , // limited slip differential for car with front wheel drive
      DIFF_LS_RWD  , // limited slip differential for car with rear  wheel drive
      DIFF_OPEN_4WD, // open         differential for car with four  wheel drive
      DIFF_OPEN_FWD, // open         differential for car with front wheel drive
      DIFF_OPEN_RWD, // open         differential for car with rear  wheel drive
   };
   struct Wheel
   {
      Flt radius, // wheel radius, default=0.35 m
          width ; // wheel width , default=0.25 m
      Vec pos   ; // wheel position in vehicle

      void set(Flt radius, Flt width, C Vec &pos) {T.radius=radius; T.width=width; T.pos=pos;}

      Wheel() {set(0.35f, 0.25f, VecZero);}
   };
   struct Params // all of these parameters (except masses) will be scaled according to 'scale' during vehicle creation
   {
      Flt   mass              , // vehicle mass, default=1500 kg
            wheel_mass        ; // wheel   mass, default=  20 kg
      Vec   mass_center_offset; // offset to center of mass in vehicle actor, default=(0, -0.1, 0) (slightly below)
      Wheel wheel[WHEEL_NUM]  ; // parameters for each wheel

      Params() {mass=1500; wheel_mass=20; mass_center_offset.set(0, -0.1f, 0);}
   };

   // manage
   PhysXVehicle& del      ();                                                // manually delete
   Bool          createTry(C PhysBody &body, C Params &params, Flt scale=1); // create from 'body' physical body, 'params' parameters, and 'scale' scaling, false on fail
   PhysXVehicle& create   (C PhysBody &body, C Params &params, Flt scale=1); // create from 'body' physical body, 'params' parameters, and 'scale' scaling, Exit  on fail

   // get / set
   Bool is()C {return _vehicle!=null;} // if created

   Bool        onGround()C;                                                   // if      vehicle is on ground (at least one wheel on ground)
   Bool        onGround(WHEEL_TYPE wheel)C;                                   // if      wheel   is on ground
#if EE_PRIVATE
   Vec    wheelPosL    (WHEEL_TYPE wheel)C;                                   // get     wheel   position         in actor local space
#endif
   Matrix wheelMatrix  (WHEEL_TYPE wheel)C;                                   // get     wheel   matrix           in world space
   Vec    wheelVel     (WHEEL_TYPE wheel)C;                                   // get     wheel           velocity in world space
   Vec    wheelAngVel  (WHEEL_TYPE wheel)C;                                   // get     wheel   angular velocity in world space
   Vec    wheelContact (WHEEL_TYPE wheel)C;                                   // get     wheel   contact point with the ground, Vec(0,0,0) if not on ground
   Flt    wheelRadius  (WHEEL_TYPE wheel)C;                                   // get     wheel   radius
   Flt    wheelCompress(WHEEL_TYPE wheel)C;                                   // get     wheel   compression, value <0 means wheel is loose in air, value 0 means wheel is at rest, value 1 means wheel is fully compressed
   Flt    wheelLongSlip(WHEEL_TYPE wheel)C;                                   // get     wheel   longitudinal slip (how much does the wheel slip on ground in "forward" direction)
   Flt    wheelLatSlip (WHEEL_TYPE wheel)C;                                   // get     wheel   lateral      slip (how much does the wheel slip on ground in "side   " direction)
   Flt            speed()C;                                                   // get     vehicle forward  speed (this value is positive when moving forward and negative when moving backwards)
   Flt        sideSpeed()C;                                                   // get     vehicle side     speed (this value is positive when moving right   and negative when moving left     )
   Flt      engineSpeed()C;                                                   // get     engine  rotation speed (in radians per second)
   Flt      engineFrac ()C {return engineSpeed()/engineMaxSpeed();}           // get     engine  rotation speed fraction (0..1) this can be useful information when changing gears manually (for example: for fraction < 0.5 you can gear down, for fraction > 0.65 you can gear up)
   Int            gears()C;                                                   // get     number of gears
   Int             gear()C;   PhysXVehicle&             gear(Int   gear    ); // get/set current   gear (-1=reverse, 0=neutral, 1..=normal gear)
#if EE_PRIVATE
   Int       targetGear()C;   PhysXVehicle&       targetGear(Int   gear    ); // set     target    gear
#endif
   Bool        autoGear()C;   PhysXVehicle&         autoGear(Bool  on      ); // get/set automatic gear changing     , true/false, default=true
   Flt            accel()C;   PhysXVehicle&            accel(Flt   accel   ); // get/set           acceleration      ,  0..1
   Flt            brake()C;   PhysXVehicle&            brake(Flt   brake   ); // get/set current         brake       ,  0..1
   Flt        handBrake()C;   PhysXVehicle&        handBrake(Flt   brake   ); // get/set current   hand  brake       ,  0..1
   Flt            angle()C;   PhysXVehicle&            angle(Flt   angle   ); // get/set current   steer angle       , -1..1
   Flt         maxBrake()C;   PhysXVehicle&         maxBrake(Flt   brake   ); // get/set max             brake torque,  0..Inf   , default=1500
   Flt     maxHandBrake()C;   PhysXVehicle&     maxHandBrake(Flt   brake   ); // get/set max       hand  brake torque,  0..Inf   , default=4000
   Flt         maxAngle()C;   PhysXVehicle&         maxAngle(Flt   angle   ); // get/set max       steer angle       ,  0..PI_2  , default=PI_3
   Flt     wheelDamping()C;   PhysXVehicle&     wheelDamping(Flt   damping ); // get/set           wheel damping     ,  0..Inf   , default=0.05
   Flt     suspSpring  ()C;   PhysXVehicle&     suspSpring  (Flt   spring  ); // get/set   suspension spring strength,  0..Inf   , default=35000
   Flt     suspDamping ()C;   PhysXVehicle&     suspDamping (Flt   damping ); // get/set   suspension spring damping ,  0..Inf   , default= 4500
   Flt     suspCompress()C;   PhysXVehicle&     suspCompress(Flt   compress); // get/set   suspension max compression,  0..1     , default=0.75 (1.0 means full wheel radius)
   Flt     suspElongate()C;   PhysXVehicle&     suspElongate(Flt   elongate); // get/set   suspension max elongation ,  0..1     , default=0.25 (1.0 means full wheel radius)
   Flt enginePeakTorque()C;   PhysXVehicle& enginePeakTorque(Flt   peak    ); // get/set   engine peak torque        ,  0..Inf   , default= 500, maximum acceleration applied to engine rotation
   Flt engineMaxSpeed  ()C;   PhysXVehicle& engineMaxSpeed  (Flt   omega   ); // get/set   engine max rotation spead ,  0..Inf   , default= 600 (in radians per second)
   DIFF_TYPE   diffType()C;   PhysXVehicle&         diffType(DIFF_TYPE diff); // get/set   differential type         ,  DIFF_TYPE, default=DIFF_LS_4WD
#if EE_PRIVATE
   Int         tireType()C;   PhysXVehicle&         tireType(Int   type    ); // get/set   tire type                 ,  0..Inf   , default=0 (this is a custom value not used internally by the engine, you can set it to anything you want and use it for custom friction calculations when using 'Physics.wheelFriction')
#endif
   Flt  tireLongStiff  ()C;   PhysXVehicle&  tireLongStiff  (Flt   f       ); // get/set longitudinal stiffness per unit longitudinal slip per unit gravity, specified in N per radian per unit gravitational acceleration. Longitudinal stiffness of the tire per unit longitudinal slip is calculated as "gravity*tireLongStiff", 0..Inf, default=1000
   Flt  tireLatStiffX  ()C;   PhysXVehicle&  tireLatStiffX  (Flt   f       ); // get/set the minimum normalised load (load/restLoad) that gives a flat lateral stiffness response        (Tire lateral stiffness is typically a graph of tire load that has linear behaviour near zero load and flattens at large loads)          , 0..Inf, default=2
   Flt  tireLatStiffY  ()C;   PhysXVehicle&  tireLatStiffY  (Flt   f       ); // get/set the maximum possible lateral stiffness divided by the rest tire load, specified in "per radian" (Tire lateral stiffness is typically a graph of tire load that has linear behaviour near zero load and flattens at large loads)          , 0..Inf, default=17.9
   Flt  tireCamberStiff()C;   PhysXVehicle&  tireCamberStiff(Flt   f       ); // get/set Camber stiffness, specified in kilograms per radian                                                                                                                                                                                      , 0..Inf, default=5.73

      // function of friction vs longitudinal slip with 3 points, these values define a function "wheelFriction=Func(wheelLongSlip)", like "y=f(x)" and you define control points of the "x" (slip) and "y" (friction)
    //Flt tireFuncPoint0Slip()C;   PhysXVehicle& tireFuncPoint0Slip(Flt f); // get/set slip     value at point 0, this value is always zero
      Flt tireFuncPoint0Fric()C;   PhysXVehicle& tireFuncPoint0Fric(Flt f); // get/set friction value at point 0, 0..Inf, default=1
      Flt tireFuncPoint1Slip()C;   PhysXVehicle& tireFuncPoint1Slip(Flt f); // get/set slip     value at point 1, 0..Inf, default=0.1
      Flt tireFuncPoint1Fric()C;   PhysXVehicle& tireFuncPoint1Fric(Flt f); // get/set friction value at point 1, 0..Inf, default=1
      Flt tireFuncPoint2Slip()C;   PhysXVehicle& tireFuncPoint2Slip(Flt f); // get/set slip     value at point 2, 0..Inf, default=1
      Flt tireFuncPoint2Fric()C;   PhysXVehicle& tireFuncPoint2Fric(Flt f); // get/set friction value at point 2, 0..Inf, default=1

   PhysXVehicle& precision(Flt thresholdLongitudinalSpeed, UInt lowForwardSpeedSubStepCount, UInt highForwardSpeedSubStepCount); // Set the number of vehicle sub-steps that will be performed when the vehicle's longitudinal speed is below and above a threshold longitudinal speed. More sub-steps provides better stability but with greater computational cost. Typically, vehicles require more sub-steps at very low forward speeds. The threshold longitudinal speed has a default value of 5 metres per second. The sub-step count below the threshold longitudinal speed has a default of 3. The sub-step count above the threshold longitudinal speed has a default of 1. Each sub-step has time advancement equal to the time-step passed to 'Physics.startSimulation' divided by the number of required sub-steps. The contact planes of the most recent suspension line raycast are reused across all sub-steps. Each sub-step computes tire and suspension forces and then advances a velocity, angular velocity and transform. At the end of all sub-steps the vehicle actor is given the velocity and angular velocity that would move the actor from its start transform prior to the first sub-step to the transform computed at the end of the last substep, assuming it doesn't collide with anything along the way in the next PhysX SDK update. The global pose of the actor is left unchanged throughout the sub-steps. 'thresholdLongitudinalSpeed'=threshold speed that is used to categorize vehicle speed as low speed or high speed. 'lowForwardSpeedSubStepCount'=number of sub-steps performed in PxVehicleUpates for vehicles that have longitudinal speed lower than thresholdLongitudinalSpeed. 'highForwardSpeedSubStepCount'=number of sub-steps performed in PxVehicleUpdates for vehicles that have longitudinal speed graeter than thresholdLongitudinalSpeed.

   Flt      energy    (          )C;                                               // get     kinetic energy , 0..Inf
   Flt      damping   (          )C;   PhysXVehicle&  damping(  Flt      damping); // get/set linear  damping, 0..Inf, default=0.05
   Flt     adamping   (          )C;   PhysXVehicle& adamping(  Flt      damping); // get/set angular damping, 0..Inf, default=0.05
   Flt     mass       (          )C;                                               // get     mass           , 0..Inf
   Vec     massCenterL(          )C;                                               // get     mass center in actor local space
   Vec     massCenterW(          )C;                                               // get     mass center in world       space
   Vec     inertia    (          )C;   PhysXVehicle& inertia (C Vec     &inertia); // get/set inertia tensor
   Vec     pos        (          )C;   PhysXVehicle& pos     (C Vec     &pos    ); // get/set position
   Matrix3 orn        (          )C;   PhysXVehicle& orn     (C Matrix3 &orn    ); // get/set orientation, 'orn'    must be normalized
   Matrix  matrix     (          )C;   PhysXVehicle& matrix  (C Matrix  &matrix ); // get/set matrix     , 'matrix' must be normalized
   Vec          vel   (          )C;   PhysXVehicle&    vel  (C Vec     &vel    ); // get/set         velocity
   Vec       angVel   (          )C;   PhysXVehicle& angVel  (C Vec     &vel    ); // get/set angular velocity
   Vec     pointVelL  (C Vec &pos)C;                                               // get     point   velocity ('pos' is in actor local space)
   Vec     pointVelW  (C Vec &pos)C;                                               // get     point   velocity ('pos' is in world       space)
   Box     box        (          )C;                                               // get     bounding box in world space
   Flt     scale      (          )C {return _scale;}                               // get     scale that was used during vehicle creation

   PhysXVehicle& addTorque (C Vec &torque             ); // add torque                         , unit = mass * rotation / time**2
   PhysXVehicle& addAngVel (C Vec &ang_vel            ); // add angular velocity               , unit =        rotation / time
   PhysXVehicle& addForce  (C Vec &force              ); // add force                          , unit = mass * distance / time**2
   PhysXVehicle& addForce  (C Vec &force  , C Vec &pos); // add force   at world 'pos' position, unit = mass * distance / time**2
   PhysXVehicle& addImpulse(C Vec &impulse            ); // add impulse                        , unit = mass * distance / time
   PhysXVehicle& addImpulse(C Vec &impulse, C Vec &pos); // add impulse at world 'pos' position, unit = mass * distance / time
   PhysXVehicle& addVel    (C Vec &vel                ); // add velocity                       , unit =        distance / time
   PhysXVehicle& addAccel  (C Vec &accel              ); // add acceleration                   , unit =        distance / time**2

   Bool ray        ()C;   PhysXVehicle& ray        (Bool on       ); // get/set if this actor should be included when performing ray tests
   Bool collision  ()C;   PhysXVehicle& collision  (Bool on       ); // get/set if this actor should collide with other actors in the world
   Bool sleep      ()C;   PhysXVehicle& sleep      (Bool sleep    ); // get/set sleeping
   Flt  sleepEnergy()C;   PhysXVehicle& sleepEnergy(Flt  energy   ); // get/set the amount of energy below the actor is put to sleep, default=0.005
   Bool ccd        ()C;   PhysXVehicle& ccd        (Bool on       ); // get/set continuous collision detection
   Ptr  user       ()C;   PhysXVehicle& user       (Ptr  user     ); // get/set custom user data
   Ptr  obj        ()C;   PhysXVehicle& obj        (Ptr  obj      ); // get/set pointer to object containing the actor
   Byte group      ()C;   PhysXVehicle& group      (Byte group    ); // get/set collision group (0..31, default value is taken according to ACTOR_GROUP)
   Byte dominance  ()C;   PhysXVehicle& dominance  (Byte dominance); // get/set dominance index (0..31, default=0), for more information about dominance please check comments on 'Physics.dominance' method

   PhysMtrl*  bodyMaterial()C;   PhysXVehicle&  bodyMaterial(PhysMtrl *material); // get/set physics material for vehicle body   (use 'null' for default material)
   PhysMtrl* wheelMaterial()C;   PhysXVehicle& wheelMaterial(PhysMtrl *material); // get/set physics material for vehicle wheels (use 'null' for default material)

   // operations
   PhysXVehicle& reset(); // this method will reset the engine, wheel and control state (acceleration, torque, forces, gear, etc.)

   PhysXVehicle& gearUp  () {Int g=gear(); return gear((g<=0) ?  1 : g+1);} // set gear up   (avoiding neutral gear)
   PhysXVehicle& gearDown() {Int g=gear(); return gear((g<=1) ? -1 : g-1);} // set gear down (avoiding neutral gear)

  ~PhysXVehicle() {del();}
   PhysXVehicle();

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   PHYS_API(PxVehicleDrive4W, void) *_vehicle;
#else
   Ptr   _vehicle;
#endif
   UInt  _vehicle_id;
   Flt   _scale;
   Actor _actor;

   NO_COPY_CONSTRUCTOR(PhysXVehicle);
};
/******************************************************************************

   PhysXVehicle:
      Wheel shapes have disabled collision (eSIMULATION_SHAPE)

/******************************************************************************/
using 'PhysXVehicle' requires calling 'PxInitVehicleSDK' in 'PhysxClass::create'

PhysXVehicle::PhysXVehicle() {_vehicle=null; _vehicle_id=0; _scale=0;}
PhysXVehicle& PhysXVehicle::create(C PhysBody &body, C Params &params, Flt scale)
{
   if(!createTry(body, params, scale))Exit("Can't create PhysXVehicle"
   #if !PHYSX
      "\nThis class is only supported on PhysX"
   #endif
   );
   return T;
}

Flt     PhysXVehicle::energy     (          )C {return _actor.energy     (   );}
Flt     PhysXVehicle::damping    (          )C {return _actor.damping    (   );}   PhysXVehicle& PhysXVehicle:: damping(  Flt      damping) {_actor. damping(damping); return T;}
Flt     PhysXVehicle::adamping   (          )C {return _actor.adamping   (   );}   PhysXVehicle& PhysXVehicle::adamping(  Flt      damping) {_actor.adamping(damping); return T;}
Flt     PhysXVehicle::mass       (          )C {return _actor.mass       (   );}
Vec     PhysXVehicle::massCenterL(          )C {return _actor.massCenterL(   );}
Vec     PhysXVehicle::massCenterW(          )C {return _actor.massCenterW(   );}
Vec     PhysXVehicle::inertia    (          )C {return _actor.inertia    (   );}   PhysXVehicle& PhysXVehicle::inertia (C Vec     &inertia) {_actor.inertia (inertia); return T;}
Vec     PhysXVehicle::pos        (          )C {return _actor.pos        (   );}   PhysXVehicle& PhysXVehicle::pos     (C Vec     &pos    ) {_actor.pos     (pos    ); return T;}
Matrix3 PhysXVehicle::orn        (          )C {return _actor.orn        (   );}   PhysXVehicle& PhysXVehicle::orn     (C Matrix3 &orn    ) {_actor.orn     (orn    ); return T;}
Matrix  PhysXVehicle::matrix     (          )C {return _actor.matrix     (   );}   PhysXVehicle& PhysXVehicle::matrix  (C Matrix  &matrix ) {_actor.matrix  (matrix ); return T;}
Vec     PhysXVehicle::     vel   (          )C {return _actor.     vel   (   );}   PhysXVehicle& PhysXVehicle::   vel  (C Vec     &vel    ) {_actor.   vel  (vel    ); return T;}
Vec     PhysXVehicle::  angVel   (          )C {return _actor.  angVel   (   );}   PhysXVehicle& PhysXVehicle::angVel  (C Vec     &vel    ) {_actor.angVel  (vel    ); return T;}
Vec     PhysXVehicle::pointVelL  (C Vec &pos)C {return _actor.pointVelL  (pos);}
Vec     PhysXVehicle::pointVelW  (C Vec &pos)C {return _actor.pointVelW  (pos);}
Box     PhysXVehicle::box        (          )C {return _actor.box        (   );}

PhysXVehicle& PhysXVehicle::addTorque (C Vec &torque             ) {_actor.addTorque (torque      ); return T;}
PhysXVehicle& PhysXVehicle::addAngVel (C Vec &ang_vel            ) {_actor.addAngVel (ang_vel     ); return T;}
PhysXVehicle& PhysXVehicle::addForce  (C Vec &force              ) {_actor.addForce  (force       ); return T;}
PhysXVehicle& PhysXVehicle::addForce  (C Vec &force  , C Vec &pos) {_actor.addForce  (force  , pos); return T;}
PhysXVehicle& PhysXVehicle::addImpulse(C Vec &impulse            ) {_actor.addImpulse(impulse     ); return T;}
PhysXVehicle& PhysXVehicle::addImpulse(C Vec &impulse, C Vec &pos) {_actor.addImpulse(impulse, pos); return T;}
PhysXVehicle& PhysXVehicle::addVel    (C Vec &vel                ) {_actor.addVel    (vel         ); return T;}
PhysXVehicle& PhysXVehicle::addAccel  (C Vec &accel              ) {_actor.addAccel  (accel       ); return T;}

Bool PhysXVehicle::collision  ()C {return _actor.collision  ();}
Bool PhysXVehicle::ray        ()C {return _actor.ray        ();}   PhysXVehicle& PhysXVehicle::ray        (Bool on       ) {_actor.ray        (on       ); return T;}
Bool PhysXVehicle::sleep      ()C {return _actor.sleep      ();}   PhysXVehicle& PhysXVehicle::sleep      (Bool sleep    ) {_actor.sleep      (sleep    ); return T;}
Flt  PhysXVehicle::sleepEnergy()C {return _actor.sleepEnergy();}   PhysXVehicle& PhysXVehicle::sleepEnergy(Flt  energy   ) {_actor.sleepEnergy(energy   ); return T;}
Bool PhysXVehicle::ccd        ()C {return _actor.ccd        ();}   PhysXVehicle& PhysXVehicle::ccd        (Bool on       ) {_actor.ccd        (on       ); return group(group());} // need to re-apply group because of 'eCCD_LINEAR' in 'SimulationFilterData' (also 'Actor.ccd' calls 'Actor.group' and that clears 'vehicle_id')
Ptr  PhysXVehicle::user       ()C {return _actor.user       ();}   PhysXVehicle& PhysXVehicle::user       (Ptr  user     ) {_actor.user       (user     ); return T;}
Ptr  PhysXVehicle::obj        ()C {return _actor.obj        ();}   PhysXVehicle& PhysXVehicle::obj        (Ptr  obj      ) {_actor.obj        (obj      ); return T;}
Byte PhysXVehicle::dominance  ()C {return _actor.dominance  ();}   PhysXVehicle& PhysXVehicle::dominance  (Byte dominance) {_actor.dominance  (dominance); return T;}
Byte PhysXVehicle::group      ()C {return _actor.group      ();}
/******************************************************************************/
#if PHYSX
ASSERT(WHEEL_LEFT_FRONT ==(Int)PxVehicleDrive4WWheelOrder::eFRONT_LEFT );
ASSERT(WHEEL_RIGHT_FRONT==(Int)PxVehicleDrive4WWheelOrder::eFRONT_RIGHT);
ASSERT(WHEEL_LEFT_REAR  ==(Int)PxVehicleDrive4WWheelOrder::eREAR_LEFT  );
ASSERT(WHEEL_RIGHT_REAR ==(Int)PxVehicleDrive4WWheelOrder::eREAR_RIGHT );
/******************************************************************************/
static Int CompareVehicle(PxVehicleDrive4W*C &a, PxVehicleDrive4W*C &b) {return Compare(Ptr(a), Ptr(b));}

PhysXVehicle& PhysXVehicle::del()
{
   if(_vehicle)
   {
      SafeWriteLock lock(Physics._rws);
      if(_vehicle)
      {
         if(Physx.physics)_vehicle->free();
         Physx.vehicles.binaryExclude(_vehicle, CompareVehicle);
        _vehicle=null;
         if(_vehicle_id){Physx.vehicle_id_gen.Return(_vehicle_id); _vehicle_id=0;}
      }
   }
  _actor.del();
   return T;
}
Bool PhysXVehicle::createTry(C PhysBody &body, C Params &params, Flt scale)
{
   WriteLock lock(Physics._rws); del();
   if(Physx.physics)
      if(_actor.createTry(body, 1, scale))
         if(_actor.isDynamic())
            if(_vehicle=PxVehicleDrive4W::allocate(WHEEL_NUM))
               if(PxVehicleWheelsSimData *wheels_sim_data=PxVehicleWheelsSimData::allocate(WHEEL_NUM)) // this must be manually released
   {
      // create ID (before calling other methods which rely on id)
     _vehicle_id=Physx.vehicle_id_gen.New();

      // get dimensions
      T._scale=scale;
      if(!body.box.size().any())ConstCast(body).setBox(); // if someone forgot to set phys box then recalculate it (can result in invalid vehicle behavior with zero box)
      Box body_box=body.box*scale; Vec body_dims=body_box.size();

      // create wheel shapes
      PxVehicleDriveSimData4W drive_sim_data;
      PxFilterData            susp_fd(0, _vehicle_id, 0, 0);
      PxVehicleWheelData      wheels   [WHEEL_NUM];
      PxVec3                  wheel_pos[WHEEL_NUM];
      FREP(WHEEL_NUM)
      {
       C Wheel &wheel=params.wheel[i];
         PxConvexMeshGeometry geom(Physx.wheel_mesh._pm->_convex, PxMeshScale(Physx.vec(Vec(wheel.width, wheel.radius, wheel.radius)*scale), PxQuat(PxIdentity)));
         Int  actor_shapes=_actor._actor->getNbShapes();
         if(PxShape *shape=_actor._actor->createShape(geom, *Physics.mtrl_default._m))
         {
            shape->setLocalPose(Physx.matrix(wheel.pos*scale));
            wheels_sim_data->setWheelShapeMapping   (i, actor_shapes);
            wheels_sim_data->setSceneQueryFilterData(i, susp_fd     );
         }
         wheels[i].mRadius=wheel.radius*scale;
         wheels[i].mWidth =wheel.width *scale;
         wheels[i].mMass  =params.wheel_mass; // *scale.x*scale.y*scale.z; don't scale
         wheels[i].mMOI   =0.5f*wheels[i].mMass*Sqr(wheels[i].mRadius); // moment of inertia around the axle's axis (http://en.wikipedia.org/wiki/List_of_moments_of_inertia)
         wheel_pos[i]=Physx.vec(wheel.pos*scale);
      }

      // mass
      Flt mass=params.mass; // *scale.x*scale.y*scale.z; don't scale
     _actor.mass(mass);

      // moment of inertia (use the moment of inertia of a cuboid as an approximate value)
      Vec chassis_moi((body_dims.y*body_dims.y + body_dims.z*body_dims.z)*mass/12.0f,
                      (body_dims.x*body_dims.x + body_dims.z*body_dims.z)*mass/12.0f,
                      (body_dims.x*body_dims.x + body_dims.y*body_dims.y)*mass/12.0f);
      inertia(chassis_moi);

      // center of mass
      Vec mass_center_offset=params.mass_center_offset*scale,
          mass_center       =_actor.massCenterL()+mass_center_offset;
      if(_actor._dynamic)_actor._dynamic->setCMassLocalPose(PxTransform(Physx.vec(mass_center))); // _actor.massCenterL(mass_center); (reset mass orientation as well because otherwise there was a case when rear wheels were placed below ground)

      // front/rear mass ratio
      // Work out the front/rear mass split from the cm offset.
      // This is a very approximate calculation with lots of assumptions. 
      // mass_rear*zRear + mass_front*zFront = mass*cm      (1)
      // mass_rear       + mass_front        = mass         (2)
      // Rearrange (2)
      // mass_front = mass - mass_rear                  (3)
      // Substitute (3) into (1)
      // mass_rear(zRear - zFront) + mass*zFront = mass*cm   (4)
      // Solve (4) for mass_rear
      // mass_rear = mass(cm - zFront)/(zRear-zFront)      (5)
      // Now we also have
      // zFront = (z-cm)/2                           (6a)
      // zRear = (-z-cm)/2                           (6b)
      // Substituting (6a-b) into (5) gives
      // mass_rear = mass*0.5*(z-3cm)/z                  (7)
      Flt mass_rear =mass*Mid(0.5f*(body_dims.z-3*mass_center_offset.z)/body_dims.z, 0.1f, 0.9f),
          mass_front=mass-mass_rear;

      wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT ].mMaxSteer=PI_3;
      wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer=PI_3;
      wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT  ].mMaxSteer=0;
      wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT ].mMaxSteer=0;
      wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT ].mMaxBrakeTorque=1500;
      wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxBrakeTorque=1500;
      wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT  ].mMaxBrakeTorque=1500;
      wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT ].mMaxBrakeTorque=1500;
      wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT ].mMaxHandBrakeTorque=0;
      wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxHandBrakeTorque=0;
      wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT  ].mMaxHandBrakeTorque=4000;
      wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT ].mMaxHandBrakeTorque=4000;
      wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT ].mDampingRate=Physics.mtrl_default.adamping();
      wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mDampingRate=Physics.mtrl_default.adamping();
      wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT  ].mDampingRate=Physics.mtrl_default.adamping();
      wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT ].mDampingRate=Physics.mtrl_default.adamping();

      // Let's set up the tire data structures now.
      PxVehicleTireData tires[WHEEL_NUM];

      // Let's set up the suspension data structures now.
      PxVehicleSuspensionData susps[WHEEL_NUM];
      REPA(susps)
      {
         susps[i].mSpringStrength  =35000;
         susps[i].mSpringDamperRate=4500;
         susps[i].mMaxCompression  =wheels[i].mRadius*0.75f;
         susps[i].mMaxDroop        =wheels[i].mRadius*0.25f;
      }
      susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT ].mSprungMass=mass_front*0.5f;
      susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mSprungMass=mass_front*0.5f;
      susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT  ].mSprungMass=mass_rear *0.5f;
      susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT ].mSprungMass=mass_rear *0.5f;

      // We need to set up geometry data for the suspension, wheels, and tires.
      // We already know the wheel centers described as offsets from the rigid body centre of mass.
      // From here we can approximate application points for the tire and suspension forces.
      // Lets assume that the suspension travel directions are absolutely vertical.
      // Also assume that we apply the tire and suspension forces 30cm below the centre of mass.
      PxVec3  wheelCentreCMOffsets[WHEEL_NUM];
      PxVec3 suspForceAppCMOffsets[WHEEL_NUM];
      PxVec3 tireForceAppCMOffsets[WHEEL_NUM];
      FREP(WHEEL_NUM)
      {
          wheelCentreCMOffsets[i]=wheel_pos[i]-Physx.vec(mass_center);
         suspForceAppCMOffsets[i]=PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
         tireForceAppCMOffsets[i]=PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
      }

      // Now add the wheel, tire and suspension data.
      FREP(WHEEL_NUM)
      {
         wheels_sim_data->setWheelData     (i, wheels[i]);
         wheels_sim_data->setTireData      (i, tires [i]);
         wheels_sim_data->setSuspensionData(i, susps [i]);
         wheels_sim_data->setSuspTravelDirection(i, PxVec3(0, -1, 0));
         wheels_sim_data->setWheelCentreOffset  (i, wheelCentreCMOffsets[i]);
         wheels_sim_data->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
         wheels_sim_data->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
      }

      // differential
      PxVehicleDifferential4WData diff;
      diff.mType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
      drive_sim_data.setDiffData(diff);

      // engine
      PxVehicleEngineData engine;
      engine.mPeakTorque=500;
      engine.mMaxOmega  =600;
      drive_sim_data.setEngineData(engine);

      // gears
      PxVehicleGearsData gears;
      gears.mSwitchTime=0.5f;
      drive_sim_data.setGearsData(gears);

      // clutch
      PxVehicleClutchData clutch;
      clutch.mStrength=10.0f;
      drive_sim_data.setClutchData(clutch);

      // ackermann steering
      PxVehicleAckermannGeometryData ackermann;
      ackermann.mAccuracy      =1.0f;
      ackermann.mAxleSeparation=wheel_pos[PxVehicleDrive4WWheelOrder::eFRONT_LEFT ].z-wheel_pos[PxVehicleDrive4WWheelOrder::eREAR_LEFT ].z;
      ackermann.mFrontWidth    =wheel_pos[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].x-wheel_pos[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].x;
      ackermann.mRearWidth     =wheel_pos[PxVehicleDrive4WWheelOrder::eREAR_RIGHT ].x-wheel_pos[PxVehicleDrive4WWheelOrder::eREAR_LEFT ].x;
      drive_sim_data.setAckermannGeometryData(ackermann);

     _vehicle->setup(Physx.physics, _actor._dynamic, *wheels_sim_data, drive_sim_data, 0);
      wheels_sim_data->free();

      group(0).collision(true)._actor.materialForce(&Physics.mtrl_default); // set params after having all shapes and vehicle being initialized
      reset().autoGear(true);

      // once the vehicle is ready, add it to the global container
      Physx.vehicles.binaryInclude(_vehicle, CompareVehicle);
      return true;
   }
   del(); return false;
}
/******************************************************************************/
PhysXVehicle& PhysXVehicle::reset()
{
   if(_vehicle)
   {
     _vehicle->setToRestState();
      gear(1);
   }
   return T;
}
/******************************************************************************/
Bool PhysXVehicle::onGround(WHEEL_TYPE wheel)C {return (_vehicle && InRange(wheel, _vehicle->wheel_query_result)) ? !_vehicle->wheel_query_result[wheel].isInAir : false;}
Bool PhysXVehicle::onGround(                )C {if(_vehicle)REPA(_vehicle->wheel_query_result)if(!_vehicle->wheel_query_result[i].isInAir)return true; return false;} // if any is not in air then return true (on ground)

Vec PhysXVehicle::wheelPosL(WHEEL_TYPE wheel)C
{
   if(_vehicle && _actor._actor && InRange(wheel, WHEEL_NUM))
   {
      Int shape_index=_vehicle->mWheelsSimData.getWheelShapeMapping(wheel);
      if(InRange(shape_index, _actor._actor->getNbShapes()))
      {
         PxShape *shape; if(_actor._actor->getShapes(&shape, 1, shape_index))
         {
            return Physx.vec(shape->getLocalPose().p);
         }
      }
   }
   return _actor.pos();
}
Matrix PhysXVehicle::wheelMatrix(WHEEL_TYPE wheel)C
{
   if(_vehicle && _actor._actor && InRange(wheel, WHEEL_NUM))
   {
      Int shape_index=_vehicle->mWheelsSimData.getWheelShapeMapping(wheel);
      if(InRange(shape_index, _actor._actor->getNbShapes()))
      {
         PxShape *shape; if(_actor._actor->getShapes(&shape, 1, shape_index))
         {
            Matrix m=Physx.matrix(_actor._actor->getGlobalPose()*shape->getLocalPose());
            if(IsLeft(wheel))
            {
               m.x.chs();
               m.z.chs();
            }
            return m;
         }
      }
   }
   return _actor.matrix();
}
Vec PhysXVehicle::wheelVel   (WHEEL_TYPE wheel)C {return _actor.pointVelL(wheelPosL(wheel));}
Vec PhysXVehicle::wheelAngVel(WHEEL_TYPE wheel)C
{
   if(_vehicle && InRange(wheel, WHEEL_NUM))
   {
      Vec dir=wheelMatrix(wheel).x; if(IsLeft(wheel))dir.chs();
      return dir*_vehicle->mWheelsDynData.getWheelRotationSpeed(wheel);
   } 
   return 0;
}
Vec  PhysXVehicle::   wheelContact (WHEEL_TYPE wheel)C {return (_vehicle && InRange(wheel, _vehicle->wheel_query_result)) ? Physx.vec(_vehicle->wheel_query_result         [wheel].tireContactPoint) : 0;}
Flt  PhysXVehicle::   wheelRadius  (WHEEL_TYPE wheel)C {return (_vehicle && InRange(wheel, _vehicle->wheel_query_result)) ?           _vehicle->mWheelsSimData.getWheelData(wheel).mRadius           : 0;}
Flt  PhysXVehicle::   wheelCompress(WHEEL_TYPE wheel)C {return (_vehicle && InRange(wheel, _vehicle->wheel_query_result)) ?           _vehicle->wheel_query_result         [wheel].suspJounce*_vehicle->mWheelsSimData.getSuspensionData(wheel).getRecipMaxCompression() : 0;}
Flt  PhysXVehicle::   wheelLongSlip(WHEEL_TYPE wheel)C {return (_vehicle && InRange(wheel, _vehicle->wheel_query_result)) ?           _vehicle->wheel_query_result         [wheel].longitudinalSlip  : 0;}
Flt  PhysXVehicle::   wheelLatSlip (WHEEL_TYPE wheel)C {return (_vehicle && InRange(wheel, _vehicle->wheel_query_result)) ?           _vehicle->wheel_query_result         [wheel].lateralSlip       : 0;}
Flt  PhysXVehicle::           speed()C {return _vehicle ? _vehicle->computeForwardSpeed ()                                                        : 0;}
Flt  PhysXVehicle::       sideSpeed()C {return _vehicle ? _vehicle->computeSidewaysSpeed()                                                        : 0;}
Flt  PhysXVehicle::     engineSpeed()C {return _vehicle ? _vehicle-> mDriveDynData.mEnginespeed                                                   : 0;}
Int  PhysXVehicle::            gear()C {return _vehicle ? _vehicle-> mDriveDynData.getCurrentGear   ()          -PxVehicleGearsData::eNEUTRAL     : 0;}
Int  PhysXVehicle::      targetGear()C {return _vehicle ? _vehicle-> mDriveDynData.getTargetGear    ()          -PxVehicleGearsData::eNEUTRAL     : 0;}
Int  PhysXVehicle::           gears()C {return _vehicle ? _vehicle-> mDriveSimData.getGearsData     ().mNbRatios-PxVehicleGearsData::eFIRST       : 0;}
Bool PhysXVehicle::        autoGear()C {return _vehicle ? _vehicle-> mDriveDynData.getUseAutoGears  ()                                            : false;}
Flt  PhysXVehicle::           angle()C {return _vehicle ? _vehicle-> mDriveDynData.getAnalogInput   (PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT) : 0;}
Flt  PhysXVehicle::        maxAngle()C {return _vehicle ? _vehicle->mWheelsSimData.getWheelData     (WHEEL_LEFT_FRONT).mMaxSteer                         : 0;}
Flt  PhysXVehicle::        maxBrake()C {return _vehicle ? _vehicle->mWheelsSimData.getWheelData     (WHEEL_LEFT_REAR ).mMaxBrakeTorque                   : 0;}
Flt  PhysXVehicle::    maxHandBrake()C {return _vehicle ? _vehicle->mWheelsSimData.getWheelData     (WHEEL_LEFT_REAR ).mMaxHandBrakeTorque               : 0;}
Flt  PhysXVehicle::    wheelDamping()C {return _vehicle ? _vehicle->mWheelsSimData.getWheelData     (WHEEL_LEFT_FRONT).mDampingRate                      : 0;}
Flt  PhysXVehicle::    suspSpring  ()C {return _vehicle ? _vehicle->mWheelsSimData.getSuspensionData(WHEEL_LEFT_FRONT).mSpringStrength                   : 0;}
Flt  PhysXVehicle::    suspDamping ()C {return _vehicle ? _vehicle->mWheelsSimData.getSuspensionData(WHEEL_LEFT_FRONT).mSpringDamperRate                 : 0;}
Flt  PhysXVehicle::    suspCompress()C {return _vehicle ? _vehicle->mWheelsSimData.getSuspensionData(WHEEL_LEFT_FRONT).mMaxCompression/_vehicle->mWheelsSimData.getWheelData(WHEEL_LEFT_FRONT).mRadius : 0;}
Flt  PhysXVehicle::    suspElongate()C {return _vehicle ? _vehicle->mWheelsSimData.getSuspensionData(WHEEL_LEFT_FRONT).mMaxDroop      /_vehicle->mWheelsSimData.getWheelData(WHEEL_LEFT_FRONT).mRadius : 0;}
Flt  PhysXVehicle::enginePeakTorque()C {return _vehicle ? _vehicle-> mDriveSimData.getEngineData    ().mPeakTorque                                       : 0;}
Flt  PhysXVehicle::engineMaxSpeed  ()C {return _vehicle ? _vehicle-> mDriveSimData.getEngineData    ().mMaxOmega                                         : 0;}
Int  PhysXVehicle::        tireType()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData      (WHEEL_LEFT_FRONT).mType                             : 0;}
Flt  PhysXVehicle::           accel()C {return _vehicle ? _vehicle-> mDriveDynData.getAnalogInput   (PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL      ) : 0;}
Flt  PhysXVehicle::           brake()C {return _vehicle ? _vehicle-> mDriveDynData.getAnalogInput   (PxVehicleDrive4WControl::eANALOG_INPUT_BRAKE      ) : 0;}
Flt  PhysXVehicle::       handBrake()C {return _vehicle ? _vehicle-> mDriveDynData.getAnalogInput   (PxVehicleDrive4WControl::eANALOG_INPUT_HANDBRAKE  ) : 0;}

Flt PhysXVehicle::tireLongStiff  ()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mLongitudinalStiffnessPerUnitGravity : 0;}
Flt PhysXVehicle::tireLatStiffX  ()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mLatStiffX                           : 0;}
Flt PhysXVehicle::tireLatStiffY  ()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mLatStiffY                           : 0;}
Flt PhysXVehicle::tireCamberStiff()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mCamberStiffnessPerUnitGravity       : 0;}

Flt PhysXVehicle::tireFuncPoint0Fric()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mFrictionVsSlipGraph[0][1] : 0;}
Flt PhysXVehicle::tireFuncPoint1Slip()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mFrictionVsSlipGraph[1][0] : 0;}
Flt PhysXVehicle::tireFuncPoint1Fric()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mFrictionVsSlipGraph[1][1] : 0;}
Flt PhysXVehicle::tireFuncPoint2Slip()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mFrictionVsSlipGraph[2][0] : 0;}
Flt PhysXVehicle::tireFuncPoint2Fric()C {return _vehicle ? _vehicle->mWheelsSimData.getTireData(WHEEL_LEFT_FRONT).mFrictionVsSlipGraph[2][1] : 0;}

PhysXVehicle& PhysXVehicle::      gear(Int  gear ) {if(_vehicle)_vehicle->mDriveDynData.forceGearChange(Mid(gear+PxVehicleGearsData::eNEUTRAL, 0, gears()+PxVehicleGearsData::eNEUTRAL)); return T;}
PhysXVehicle& PhysXVehicle::targetGear(Int  gear ) {if(_vehicle)_vehicle->mDriveDynData.startGearChange(Mid(gear+PxVehicleGearsData::eNEUTRAL, 0, gears()+PxVehicleGearsData::eNEUTRAL)); return T;}
PhysXVehicle& PhysXVehicle::  autoGear(Bool on   ) {if(_vehicle)_vehicle->mDriveDynData.setUseAutoGears(on); return T;}
PhysXVehicle& PhysXVehicle::     angle(Flt  angle) {if(_vehicle)_vehicle->mDriveDynData.setAnalogInput (PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT, Mid(angle, -1.0f, 1.0f)); return T;}
PhysXVehicle& PhysXVehicle::     accel(Flt  accel) {if(_vehicle)_vehicle->mDriveDynData.setAnalogInput (PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL      , Sat(accel             )); return T;}
PhysXVehicle& PhysXVehicle::     brake(Flt  brake) {if(_vehicle)_vehicle->mDriveDynData.setAnalogInput (PxVehicleDrive4WControl::eANALOG_INPUT_BRAKE      , Sat(brake             )); return T;}
PhysXVehicle& PhysXVehicle:: handBrake(Flt  brake) {if(_vehicle)_vehicle->mDriveDynData.setAnalogInput (PxVehicleDrive4WControl::eANALOG_INPUT_HANDBRAKE  , Sat(brake             )); return T;}

PhysXVehicle& PhysXVehicle::maxAngle(Flt angle)
{
   if(_vehicle)
   {
      Clamp(angle, 0, PI_2);
      PxVehicleWheelData data=_vehicle->mWheelsSimData.getWheelData(WHEEL_LEFT_FRONT ); data.mMaxSteer=angle; _vehicle->mWheelsSimData.setWheelData(WHEEL_LEFT_FRONT , data);
                         data=_vehicle->mWheelsSimData.getWheelData(WHEEL_RIGHT_FRONT); data.mMaxSteer=angle; _vehicle->mWheelsSimData.setWheelData(WHEEL_RIGHT_FRONT, data);
   }
   return T;
}
PhysXVehicle& PhysXVehicle::maxBrake(Flt brake)
{
   if(_vehicle)
   {
      MAX(brake, 0);
      PxVehicleWheelData data=_vehicle->mWheelsSimData.getWheelData(WHEEL_LEFT_FRONT ); data.mMaxBrakeTorque=brake; _vehicle->mWheelsSimData.setWheelData(WHEEL_LEFT_FRONT , data);
                         data=_vehicle->mWheelsSimData.getWheelData(WHEEL_RIGHT_FRONT); data.mMaxBrakeTorque=brake; _vehicle->mWheelsSimData.setWheelData(WHEEL_RIGHT_FRONT, data);
                         data=_vehicle->mWheelsSimData.getWheelData(WHEEL_LEFT_REAR  ); data.mMaxBrakeTorque=brake; _vehicle->mWheelsSimData.setWheelData(WHEEL_LEFT_REAR  , data);
                         data=_vehicle->mWheelsSimData.getWheelData(WHEEL_RIGHT_REAR ); data.mMaxBrakeTorque=brake; _vehicle->mWheelsSimData.setWheelData(WHEEL_RIGHT_REAR , data);
   }
   return T;
}
PhysXVehicle& PhysXVehicle::maxHandBrake(Flt brake)
{
   if(_vehicle)
   {
      MAX(brake, 0);
      PxVehicleWheelData data=_vehicle->mWheelsSimData.getWheelData(WHEEL_LEFT_REAR ); data.mMaxHandBrakeTorque=brake; _vehicle->mWheelsSimData.setWheelData(WHEEL_LEFT_REAR , data);
                         data=_vehicle->mWheelsSimData.getWheelData(WHEEL_RIGHT_REAR); data.mMaxHandBrakeTorque=brake; _vehicle->mWheelsSimData.setWheelData(WHEEL_RIGHT_REAR, data);
   }
   return T;
}
PhysXVehicle& PhysXVehicle::wheelDamping(Flt damping ) {if(_vehicle){MAX(damping, 0); REP(WHEEL_NUM){PxVehicleWheelData      data=_vehicle->mWheelsSimData.getWheelData     (i); data.mDampingRate     =damping                                                  ; _vehicle->mWheelsSimData.setWheelData     (i, data);}} return T;}
PhysXVehicle& PhysXVehicle::suspSpring  (Flt spring  ) {if(_vehicle){MAX(spring , 0); REP(WHEEL_NUM){PxVehicleSuspensionData data=_vehicle->mWheelsSimData.getSuspensionData(i); data.mSpringStrength  =spring                                                   ; _vehicle->mWheelsSimData.setSuspensionData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::suspDamping (Flt damping ) {if(_vehicle){MAX(damping, 0); REP(WHEEL_NUM){PxVehicleSuspensionData data=_vehicle->mWheelsSimData.getSuspensionData(i); data.mSpringDamperRate=damping                                                  ; _vehicle->mWheelsSimData.setSuspensionData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::suspCompress(Flt compress) {if(_vehicle){SAT(compress  ); REP(WHEEL_NUM){PxVehicleSuspensionData data=_vehicle->mWheelsSimData.getSuspensionData(i); data.mMaxCompression  =compress*_vehicle->mWheelsSimData.getWheelData(i).mRadius; _vehicle->mWheelsSimData.setSuspensionData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::suspElongate(Flt elongate) {if(_vehicle){SAT(elongate  ); REP(WHEEL_NUM){PxVehicleSuspensionData data=_vehicle->mWheelsSimData.getSuspensionData(i); data.mMaxDroop        =elongate*_vehicle->mWheelsSimData.getWheelData(i).mRadius; _vehicle->mWheelsSimData.setSuspensionData(i, data);}} return T;}

PhysXVehicle& PhysXVehicle::enginePeakTorque(Flt peak ) {if(_vehicle){MAX(peak , 0); PxVehicleEngineData data=_vehicle->mDriveSimData.getEngineData(); data.mPeakTorque=peak ; _vehicle->mDriveSimData.setEngineData(data);} return T;}
PhysXVehicle& PhysXVehicle::engineMaxSpeed  (Flt omega) {if(_vehicle){MAX(omega, 0); PxVehicleEngineData data=_vehicle->mDriveSimData.getEngineData(); data.mMaxOmega  =omega; _vehicle->mDriveSimData.setEngineData(data);} return T;}

PhysXVehicle& PhysXVehicle::tireType          (Int type) {if(_vehicle)              REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mType                               =type; _vehicle->mWheelsSimData.setTireData(i, data);}  return T;}
PhysXVehicle& PhysXVehicle::tireLongStiff     (Flt f   ) {if(_vehicle){MAX(f, 0  ); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mLongitudinalStiffnessPerUnitGravity=f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::tireLatStiffX     (Flt f   ) {if(_vehicle){MAX(f, 0  ); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mLatStiffX                          =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::tireLatStiffY     (Flt f   ) {if(_vehicle){MAX(f, 0  ); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mLatStiffY                          =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::tireCamberStiff   (Flt f   ) {if(_vehicle){MAX(f, 0  ); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mCamberStiffnessPerUnitGravity      =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint0Fric(Flt f   ) {if(_vehicle){MAX(f, 0  ); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mFrictionVsSlipGraph[0][1]          =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint1Slip(Flt f   ) {if(_vehicle){MAX(f, 0  ); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mFrictionVsSlipGraph[1][0]          =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint1Fric(Flt f   ) {if(_vehicle){MAX(f, EPS); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mFrictionVsSlipGraph[1][1]          =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;} // without EPS crash may occur
PhysXVehicle& PhysXVehicle::tireFuncPoint2Slip(Flt f   ) {if(_vehicle){MAX(f, 0  ); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mFrictionVsSlipGraph[2][0]          =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint2Fric(Flt f   ) {if(_vehicle){MAX(f, EPS); REP(WHEEL_NUM){PxVehicleTireData data=_vehicle->mWheelsSimData.getTireData(i); data.mFrictionVsSlipGraph[2][1]          =f   ; _vehicle->mWheelsSimData.setTireData(i, data);}} return T;} // without EPS crash may occur

PhysXVehicle& PhysXVehicle::precision(Flt thresholdLongitudinalSpeed, UInt lowForwardSpeedSubStepCount, UInt highForwardSpeedSubStepCount)
{
   if(_vehicle)_vehicle->mWheelsSimData.setSubStepCount(thresholdLongitudinalSpeed, lowForwardSpeedSubStepCount, highForwardSpeedSubStepCount);
   return T;
}
/******************************************************************************/
PhysXVehicle& PhysXVehicle::group(Byte group)
{
   if(_actor._actor && InRange(group, AG_NUM)) // must always apply group even if applying the same one, because '_ignore_id' or '_vehicle_id' could be different (and 'qfd' needs to be set)
   {
      PxFilterData sfd(group, _actor._ignore_id, ccd() ? PxPairFlag::eDETECT_CCD_CONTACT : 0, 0), qfd(IndexToFlag(group), _vehicle_id, 0, 0);
      for(Int offset=0, shapes=_actor._actor->getNbShapes(); shapes>0; )
      {
         PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
         REP(_actor._actor->getShapes(shape, s, offset))
         {
            shape[i]->setSimulationFilterData(sfd);
            shape[i]->setQueryFilterData     (qfd);
         }
         shapes-=s;
         offset+=s;
      }
   }
   return T;
}
PhysXVehicle& PhysXVehicle::collision(Bool on)
{
   if(_actor._actor)
   {
      VecI4 wheel; if(_vehicle)wheel.set(_vehicle->mWheelsSimData.getWheelShapeMapping(0), _vehicle->mWheelsSimData.getWheelShapeMapping(1), _vehicle->mWheelsSimData.getWheelShapeMapping(2), _vehicle->mWheelsSimData.getWheelShapeMapping(3));else wheel=-1;
         for(Int offset=0, shapes=_actor._actor->getNbShapes(); shapes>0; )
      {
         PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
         REP(_actor._actor->getShapes(shape, s, offset))
         {
            Bool shape_on=on;
            Int  io=i+offset; if(io==wheel.x || io==wheel.y || io==wheel.z || io==wheel.w)shape_on=false; // always disable for wheels
            if(shape_on)shape[i]->setFlag(PxShapeFlag::eTRIGGER_SHAPE   , false   ); // if enabling collision, we need to first disable trigger, because PhysX will ignore setting collision flag
                        shape[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, shape_on);
         }
         shapes-=s;
         offset+=s;
      }
   }
   return T;
}
PhysMtrl* PhysXVehicle::bodyMaterial()C
{
   if(_actor._actor)
   {
      Int  shapes=_actor._actor->getNbShapes();
      FREP(shapes)
      {
         if(_vehicle)
            if(_vehicle->mWheelsSimData.getWheelShapeMapping(0)==i
            || _vehicle->mWheelsSimData.getWheelShapeMapping(1)==i
            || _vehicle->mWheelsSimData.getWheelShapeMapping(2)==i
            || _vehicle->mWheelsSimData.getWheelShapeMapping(3)==i)continue; // skip wheels
         PxShape *shape; if(_actor._actor->getShapes(&shape, 1, i))
         {
            PxMaterial *mtrl; if(shape->getMaterials(&mtrl, 1))if(mtrl)if(PhysMtrl *phys_mtrl=(PhysMtrl*)mtrl->userData)return (phys_mtrl!=&Physics.mtrl_default) ? phys_mtrl : null; // return default material always as null (we can use 'userData' because PhysMtrl's are stored in non-ref-counted cache and kept forever)
         }
      }
   }
   return null;
}
PhysMtrl* PhysXVehicle::wheelMaterial()C
{
   if(_vehicle && _actor._actor)
   {
      Int shape_index=_vehicle->mWheelsSimData.getWheelShapeMapping(0);
      if(InRange(shape_index, _actor._actor->getNbShapes()))
      {
         PxShape *shape; if(_actor._actor->getShapes(&shape, 1, shape_index))
         {
            PxMaterial *mtrl; if(shape->getMaterials(&mtrl, 1))if(mtrl)if(PhysMtrl *phys_mtrl=(PhysMtrl*)mtrl->userData)if(phys_mtrl!=&Physics.mtrl_default)return phys_mtrl; // return default material always as null (we can use 'userData' because PhysMtrl's are stored in non-ref-counted cache and kept forever)
         }
      }
   }
   return null;
}
PhysXVehicle& PhysXVehicle::bodyMaterial(PhysMtrl *material)
{
   if(_actor._actor)
   {
      if(!material)material=&Physics.mtrl_default; // always set valid material
      if(PxMaterial *m=material->_m)
      {
         VecI4 wheel; if(_vehicle)wheel.set(_vehicle->mWheelsSimData.getWheelShapeMapping(0), _vehicle->mWheelsSimData.getWheelShapeMapping(1), _vehicle->mWheelsSimData.getWheelShapeMapping(2), _vehicle->mWheelsSimData.getWheelShapeMapping(3));else wheel=-1;
         for(Int offset=0, shapes=_actor._actor->getNbShapes(); shapes>0; )
         {
            PxShape *shape[32]; Int s=Min(shapes, Elms(shape));
            REP(_actor._actor->getShapes(shape, s, offset))
            {
               Int io=i+offset; if(io!=wheel.x && io!=wheel.y && io!=wheel.z && io!=wheel.w)shape[i]->setMaterials(&m, 1); // skip wheels
            }
            shapes-=s;
            offset+=s;
         }
      }
      damping(material->damping()).adamping(material->adamping()); // don't adjust mass
   }
   return T;
}
PhysXVehicle& PhysXVehicle::wheelMaterial(PhysMtrl *material)
{
   if(_vehicle && _actor._actor)
   {
      if(!material)material=&Physics.mtrl_default; // always set valid material
      if(PxMaterial *m=material->_m)
      {
         Int  shapes  =_actor._actor->getNbShapes();
         Int  wheel[4]={_vehicle->mWheelsSimData.getWheelShapeMapping(0), _vehicle->mWheelsSimData.getWheelShapeMapping(1), _vehicle->mWheelsSimData.getWheelShapeMapping(2), _vehicle->mWheelsSimData.getWheelShapeMapping(3)};
         REPA(wheel)if(InRange(wheel[i], shapes))
         {
            PxShape *shape; if(_actor._actor->getShapes(&shape, 1, wheel[i]))shape->setMaterials(&m, 1);
         }
      }
      wheelDamping(material->adamping());
   }
   return T;
}
PhysXVehicle::DIFF_TYPE PhysXVehicle::diffType()C
{
   if(_vehicle)switch(_vehicle->mDriveSimData.getDiffData().mType)
   {
      case PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD      : return DIFF_LS_4WD;
      case PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD  : return DIFF_LS_FWD;
      case PxVehicleDifferential4WData::eDIFF_TYPE_LS_REARWD   : return DIFF_LS_RWD;
      case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_4WD    : return DIFF_OPEN_4WD;
      case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD: return DIFF_OPEN_FWD;
      case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_REARWD : return DIFF_OPEN_RWD;
   }
   return DIFF_LS_4WD;
}
PhysXVehicle& PhysXVehicle::diffType(DIFF_TYPE diff)
{
   if(_vehicle)
   {
      PxVehicleDifferential4WData data=_vehicle->mDriveSimData.getDiffData(); switch(diff)
      {
         case DIFF_LS_4WD  : data.mType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD      ; break;
         case DIFF_LS_FWD  : data.mType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD  ; break;
         case DIFF_LS_RWD  : data.mType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_REARWD   ; break;
         case DIFF_OPEN_4WD: data.mType=PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_4WD    ; break;
         case DIFF_OPEN_FWD: data.mType=PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD; break;
         case DIFF_OPEN_RWD: data.mType=PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_REARWD ; break;
      }
     _vehicle->mDriveSimData.setDiffData(data);
   }
   return T;
}
/******************************************************************************/
static PxVehicleDrivableSurfaceToTireFrictionPairs vdsttfp;
void PhysxClass::updateVehicles()
{
   REPA(vehicles)
   {
      PxVehicleDrive4W         *vehicle=vehicles[i];
      PxVehicleWheels          *vehicle_wheels[1]={vehicles[i]};
      PxVehicleWheelQueryResult wqr; wqr.wheelQueryResults=vehicle->wheel_query_result; wqr.nbWheelQueryResults=Elms(vehicle->wheel_query_result);
      PxVehicleSuspensionRaycasts(batch_query_4, Elms(vehicle_wheels), vehicle_wheels, Elms(raycast_query_result), raycast_query_result);
      PxVehicleUpdates           (Physics.time(), vec(Physics.gravity()), vdsttfp, Elms(vehicle_wheels), vehicle_wheels, &wqr);
   }
}
/******************************************************************************/
#else
/******************************************************************************/
PhysXVehicle& PhysXVehicle::del() {return T;}
Bool          PhysXVehicle::createTry(C PhysBody &body, C Params &params, Flt scale) {return false;}
PhysXVehicle& PhysXVehicle::reset() {return T;}
Bool          PhysXVehicle::onGround(                )C {return false;}
Bool          PhysXVehicle::onGround(WHEEL_TYPE wheel)C {return false;}

Vec    PhysXVehicle::   wheelPosL    (WHEEL_TYPE wheel)C {return 0;}
Matrix PhysXVehicle::   wheelMatrix  (WHEEL_TYPE wheel)C {return MatrixIdentity;}
Vec    PhysXVehicle::   wheelVel     (WHEEL_TYPE wheel)C {return 0;}
Vec    PhysXVehicle::   wheelAngVel  (WHEEL_TYPE wheel)C {return 0;}
Vec    PhysXVehicle::   wheelContact (WHEEL_TYPE wheel)C {return 0;}
Flt    PhysXVehicle::   wheelRadius  (WHEEL_TYPE wheel)C {return 0;}
Flt    PhysXVehicle::   wheelCompress(WHEEL_TYPE wheel)C {return 0;}
Flt    PhysXVehicle::   wheelLatSlip (WHEEL_TYPE wheel)C {return 0;}
Flt    PhysXVehicle::   wheelLongSlip(WHEEL_TYPE wheel)C {return 0;}
Flt    PhysXVehicle::           speed()C {return 0;}
Flt    PhysXVehicle::       sideSpeed()C {return 0;}
Flt    PhysXVehicle::     engineSpeed()C {return 0;}
Int    PhysXVehicle::            gear()C {return 0;}
Int    PhysXVehicle::      targetGear()C {return 0;}
Int    PhysXVehicle::           gears()C {return 0;}
Bool   PhysXVehicle::        autoGear()C {return false;}
Flt    PhysXVehicle::           angle()C {return 0;}
Flt    PhysXVehicle::        maxAngle()C {return 0;}
Flt    PhysXVehicle::        maxBrake()C {return 0;}
Flt    PhysXVehicle::    maxHandBrake()C {return 0;}
Flt    PhysXVehicle::    wheelDamping()C {return 0;}
Flt    PhysXVehicle::    suspSpring  ()C {return 0;}
Flt    PhysXVehicle::    suspDamping ()C {return 0;}
Flt    PhysXVehicle::    suspCompress()C {return 0;}
Flt    PhysXVehicle::    suspElongate()C {return 0;}
Flt    PhysXVehicle::enginePeakTorque()C {return 0;}
Flt    PhysXVehicle::engineMaxSpeed  ()C {return 0;}
Int    PhysXVehicle::        tireType()C {return 0;}
Flt    PhysXVehicle::           accel()C {return 0;}
Flt    PhysXVehicle::           brake()C {return 0;}
Flt    PhysXVehicle::       handBrake()C {return 0;}
PhysMtrl* PhysXVehicle:: bodyMaterial()C {return null;}
PhysMtrl* PhysXVehicle::wheelMaterial()C {return null;}
PhysXVehicle::DIFF_TYPE PhysXVehicle::diffType()C {return DIFF_LS_4WD;}

Flt PhysXVehicle::tireLatStiffX  ()C {return 0;}
Flt PhysXVehicle::tireLatStiffY  ()C {return 0;}
Flt PhysXVehicle::tireLongStiff  ()C {return 0;}
Flt PhysXVehicle::tireCamberStiff()C {return 0;}

Flt PhysXVehicle::tireFuncPoint0Fric()C {return 0;}
Flt PhysXVehicle::tireFuncPoint1Slip()C {return 0;}
Flt PhysXVehicle::tireFuncPoint1Fric()C {return 0;}
Flt PhysXVehicle::tireFuncPoint2Slip()C {return 0;}
Flt PhysXVehicle::tireFuncPoint2Fric()C {return 0;}

PhysXVehicle& PhysXVehicle::            gear(Int   gear    ) {return T;}
PhysXVehicle& PhysXVehicle::      targetGear(Int   gear    ) {return T;}
PhysXVehicle& PhysXVehicle::        autoGear(Bool  on      ) {return T;}
PhysXVehicle& PhysXVehicle::           angle(Flt   angle   ) {return T;}
PhysXVehicle& PhysXVehicle::           accel(Flt   accel   ) {return T;}
PhysXVehicle& PhysXVehicle::           brake(Flt   brake   ) {return T;}
PhysXVehicle& PhysXVehicle::       handBrake(Flt   brake   ) {return T;}
PhysXVehicle& PhysXVehicle::        maxAngle(Flt   angle   ) {return T;}
PhysXVehicle& PhysXVehicle::        maxBrake(Flt   brake   ) {return T;}
PhysXVehicle& PhysXVehicle::    maxHandBrake(Flt   brake   ) {return T;}
PhysXVehicle& PhysXVehicle::    wheelDamping(Flt   damping ) {return T;}
PhysXVehicle& PhysXVehicle::    suspSpring  (Flt   spring  ) {return T;}
PhysXVehicle& PhysXVehicle::    suspDamping (Flt   damping ) {return T;}
PhysXVehicle& PhysXVehicle::    suspCompress(Flt   compress) {return T;}
PhysXVehicle& PhysXVehicle::    suspElongate(Flt   elongate) {return T;}
PhysXVehicle& PhysXVehicle::enginePeakTorque(Flt   peak    ) {return T;}
PhysXVehicle& PhysXVehicle::engineMaxSpeed  (Flt   omega   ) {return T;}
PhysXVehicle& PhysXVehicle::        tireType(Int   type    ) {return T;}
PhysXVehicle& PhysXVehicle::        diffType(DIFF_TYPE diff) {return T;}

PhysXVehicle& PhysXVehicle::group    (Byte group) {return T;}
PhysXVehicle& PhysXVehicle::collision(Bool on   ) {return T;}

PhysXVehicle& PhysXVehicle:: bodyMaterial(PhysMtrl *material) {return T;}
PhysXVehicle& PhysXVehicle::wheelMaterial(PhysMtrl *material) {return T;}

PhysXVehicle& PhysXVehicle::tireLatStiffX  (Flt f) {return T;}
PhysXVehicle& PhysXVehicle::tireLatStiffY  (Flt f) {return T;}
PhysXVehicle& PhysXVehicle::tireLongStiff  (Flt f) {return T;}
PhysXVehicle& PhysXVehicle::tireCamberStiff(Flt f) {return T;}

PhysXVehicle& PhysXVehicle::tireFuncPoint0Fric(Flt f) {return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint1Slip(Flt f) {return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint1Fric(Flt f) {return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint2Slip(Flt f) {return T;}
PhysXVehicle& PhysXVehicle::tireFuncPoint2Fric(Flt f) {return T;}

PhysXVehicle& PhysXVehicle::precision(Flt thresholdLongitudinalSpeed, UInt lowForwardSpeedSubStepCount, UInt highForwardSpeedSubStepCount) {return T;}
/******************************************************************************/
#endif
/******************************************************************************/
// PHYSX VEHICLE TUTORIAL
/******************************************************************************/
const bool EasyReverse=true; // if use simplified reversing (will automatically switch between reverse<->forward gears)

Memc<Actor> actors;
PhysXVehicle vehicle;
/******************************************************************************/
flt WheelFriction(C PhysMtrl &ground_material, C PhysMtrl &wheel_material, C ActorInfo &vehicle, WHEEL_TYPE wheel)
{
   return 1;
}
void InitPre()
{
   EE_INIT();
   App.flag=APP_RESIZABLE|APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_WORK_IN_BACKGROUND|APP_FULL_TOGGLE;
#ifdef DEBUG
   App.flag|=APP_MEM_LEAKS|APP_BREAKPOINT_ON_ERROR;
#endif
   Cam.dist=10;
   Cam.pitch=-0.4;
   D.viewFov(DegToRad(60)).mode(App.desktopW()*0.8, App.desktopH()*0.8);
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH).wheelFriction(WheelFriction); // create physics and set custom friction calculation callback
   actors.New().create(Plane(Vec(0, -1, 0), Vec(0, 1, 0))); // create ground
   PhysXVehicle.Params params; // setup vehicle params
   flt x=0.9, y=-0.5, z=1.8;
   params.wheel[WHEEL_LEFT_FRONT ].pos.set(-x, y,  z);
   params.wheel[WHEEL_RIGHT_FRONT].pos.set( x, y,  z);
   params.wheel[WHEEL_LEFT_REAR  ].pos.set(-x, y, -z);
   params.wheel[WHEEL_RIGHT_REAR ].pos.set( x, y, -z);
   PhysBody body; body.parts.New().create(Box(2, 1, 4)); body.setBox(); // create vehicle body
   vehicle.create(body, params, 1); // create vehicle
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Physics.startSimulation().stopSimulation(); // update physics

   // set vehicle input
   flt forward=Kb.b(KB_W),
       back   =Kb.b(KB_S),
       angle  =Kb.b(KB_D)-Kb.b(KB_A),
       hand   =Kb.b(KB_SPACE);

   // adjust gears when reversing
   if(EasyReverse)
   {
      if(back   >forward+EPS && vehicle.speed()<=0.5)vehicle.gear(-1); // set reverse gear when 'back   ' exceeds 'forward' and moving very slow
      if(forward>back   +EPS && vehicle.gear ()<=0  )vehicle.gear( 1); // set     1st gear when 'forward' exceeds 'back   ' and gear is reverse or neutral
      if(vehicle.gear()<0)Swap(forward, back);
   }
   vehicle.accel(forward).brake(back).angle(angle).handBrake(hand);

   // manual gear change
   if(Kb.bp(KB_R))vehicle.gearUp  ();
   if(Kb.bp(KB_F))vehicle.gearDown();
   if(Kb.bp(KB_Z))vehicle.matrix(MatrixIdentity).vel(0).angVel(0);

   /* sample code for changing gears depending on engine speed (for this code you should disable 'autoGear')
   if(vehicle.gear()>0)
   {
      flt frac=vehicle.engineFrac();
      if( frac>=0.70f                    )vehicle.gearUp  ();else
      if( frac< 0.50f && vehicle.gear()>1)vehicle.gearDown();
   }*/

   // setup camera
   Cam.at =vehicle.pos();
   Cam.yaw=Angle(vehicle.matrix().z.xz())-PI_2;
   Cam.setSpherical().updateVelocities(CAM_ATTACH_ACTOR).set();

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   Physics.draw();
}
/******************************************************************************/
#endif
/******************************************************************************/
}
/******************************************************************************/
