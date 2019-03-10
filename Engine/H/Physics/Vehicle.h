/******************************************************************************

   Use 'Vehicle' to create vehicle type physical actor (like car).

/******************************************************************************/
enum WHEEL_TYPE
{
   WHEEL_LEFT_FRONT , //  left front wheel
   WHEEL_RIGHT_FRONT, // right front wheel
   WHEEL_LEFT_REAR  , //  left rear  wheel
   WHEEL_RIGHT_REAR , // right rear  wheel
   WHEEL_NUM        , // number of   wheels
};
inline Bool IsFront(WHEEL_TYPE wheel) {return   wheel<=WHEEL_RIGHT_FRONT;} // if wheel type is front wheel
inline Bool IsRear (WHEEL_TYPE wheel) {return   wheel>=WHEEL_LEFT_REAR  ;} // if wheel type is rear  wheel
inline Bool IsLeft (WHEEL_TYPE wheel) {return !(wheel&1)                ;} // if wheel type is left  wheel
inline Bool IsRight(WHEEL_TYPE wheel) {return  (wheel&1)                ;} // if wheel type is right wheel
/******************************************************************************/
const_mem_addr struct Vehicle // Physical actor of vehicle type !! must be stored in constant memory address !!
{
   enum WHEEL_DRIVE : Byte
   {
      AWD, // all   wheel drive (also known as 4WD four wheel drive)
      FWD, // front wheel drive
      RWD, // rear  wheel drive
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
      Wheel wheel[WHEEL_NUM]; // parameters for each wheel
   };

   // manage
   Vehicle& del      ();                                                               // manually delete
   Bool     createTry(C PhysBody &body, C Params &params, Flt density=1, Flt scale=1); // create from 'body' physical body, 'params' parameters, 'density' density and 'scale' scaling, false on fail
   Vehicle& create   (C PhysBody &body, C Params &params, Flt density=1, Flt scale=1); // create from 'body' physical body, 'params' parameters, 'density' density and 'scale' scaling, Exit  on fail

   // get / set
   Bool is()C; // if created

   Int        onGroundNum()C;                                            // get     number  of weels on ground
   Bool       onGroundAny()C;                                            // if      vehicle is       on ground (at least one wheel  on ground)
   Bool       onGroundAll()C;                                            // if      vehicle is       on ground (all          wheels on ground)
   Bool          onGround(WHEEL_TYPE wheel)C;                            // if      wheel   is       on ground
#if EE_PRIVATE
   Vec      wheelAxis    (WHEEL_TYPE wheel)C;                            // get     wheel   rotation  axis (X)
   Vec      wheelDir     (WHEEL_TYPE wheel)C;                            // get     wheel   direction axis (Z)
#endif
   Matrix   wheelMatrix  (WHEEL_TYPE wheel, Int flip_side=-1)C;          // get     wheel   matrix           in world space, 'flip_side'=if flip/mirror the wheel matrix (-1=flip left wheels only, 0=don't flip, 1=flip right wheels only)
   Vec      wheelVel     (WHEEL_TYPE wheel)C;                            // get     wheel           velocity in world space
   Vec      wheelAngVel  (WHEEL_TYPE wheel)C;                            // get     wheel   angular velocity in world space
   Vec      wheelContact (WHEEL_TYPE wheel)C;                            // get     wheel   last contact point        with the ground, Vec(0,0,0) if never was on ground
   Vec      wheelContactN(WHEEL_TYPE wheel)C;                            // get     wheel   last contact point normal with the ground, Vec(0,1,0) if never was on ground
   Flt      wheelRadius  (WHEEL_TYPE wheel)C;                            // get     wheel   radius
   Flt      wheelCompress(WHEEL_TYPE wheel)C;                            // get     wheel   compression, value <0 means wheel is loose in air, value 0 means wheel is at rest, value 1 means wheel is fully compressed
#if EE_PRIVATE
   Flt      wheelLongSlip(WHEEL_TYPE wheel)C;                            // get     wheel   longitudinal slip (how much does the wheel slip on ground in "forward" direction)
   Flt      wheelLatSlip (WHEEL_TYPE wheel)C;                            // get     wheel   lateral      slip (how much does the wheel slip on ground in "side   " direction)
#endif
   Flt              speed()C;                                            // get     vehicle forward  speed (this value is positive when moving forward and negative when moving backwards)
   Flt          sideSpeed()C;                                            // get     vehicle side     speed (this value is positive when moving right   and negative when moving left     )
   Flt              accel()C;   Vehicle&          accel(Flt   accel   ); // get/set           acceleration      , -1..1
   Flt              brake()C;   Vehicle&          brake(Flt   brake   ); // get/set current         brake       ,  0..1
   Flt              angle()C;   Vehicle&          angle(Flt   angle   ); // get/set current   steer angle       , -1..1
   Flt           maxAccel()C;   Vehicle&       maxAccel(Flt   accel   ); // get/set max       acceleration force,  0..Inf , default=10
   Flt       maxBackAccel()C;   Vehicle&   maxBackAccel(Flt   accel   ); // get/set max back  acceleration force,  0..Inf , default=4.5
   Flt           maxBrake()C;   Vehicle&       maxBrake(Flt   brake   ); // get/set max             brake  force,  0..Inf , default=12
   Flt         brakeRatio()C;   Vehicle&     brakeRatio(Flt   ratio   ); // get/set      front/rear brake  ratio,  0..1   , default=0.43, this parameter specifies how much of the braking goes into front wheels, rear wheels will have a value "1-ratio" (for example if front has 0.6, then rear will have 0.4)
   Flt           maxAngle()C;   Vehicle&       maxAngle(Flt   angle   ); // get/set max       steer angle       ,  0..PI_2, default=PI_3
   Flt     frictionLinear()C;   Vehicle& frictionLinear(Flt   friction); // get/set         wheel linear fricion,  0..Inf , default=13  , this parameter specifies the linear portion of the friction when moving sideways, side velocity will be decreased by this value by    subtraction
   Flt     frictionScalar()C;   Vehicle& frictionScalar(Flt   friction); // get/set         wheel scalar fricion,  0..1   , default=0.3 , this parameter specifies the scalar portion of the friction when moving sideways, side velocity will be decreased by this value by multiplication
   Flt       suspSpring  ()C;   Vehicle&   suspSpring  (Flt   spring  ); // get/set   suspension spring strength,  0..Inf , default=20  , this parameter specifies bounciness strength of the suspension
   Flt       suspDamping ()C;   Vehicle&   suspDamping (Flt   damping ); // get/set   suspension spring damping ,  0..Inf , default=0.7 , this parameter specifies how quickly does the suspension stabilize
   Flt       suspCompress()C;   Vehicle&   suspCompress(Flt   compress); // get/set   suspension max compression,  0..1   , default=0.75 (1.0 means full wheel radius)
   WHEEL_DRIVE wheelDrive()C;   Vehicle&     wheelDrive(WHEEL_DRIVE wd); // get/set wheel drive                           , default=WHEEL_AWD

   Flt      energy    (          )C;                                             // get     kinetic energy , 0..Inf
   Flt      damping   (          )C;   Vehicle&  damping   (  Flt      damping); // get/set linear  damping, 0..Inf, default=0.05
   Flt     adamping   (          )C;   Vehicle& adamping   (  Flt      damping); // get/set angular damping, 0..Inf, default=0.05
   Flt     mass       (          )C;   Vehicle& mass       (  Flt      mass   ); // get/set mass           , 0..Inf
   Vec     massCenterL(          )C;   Vehicle& massCenterL(C Vec     &center ); // get/set mass center in actor local space
   Vec     massCenterW(          )C;   Vehicle& massCenterW(C Vec     &center ); // get/set mass center in world       space
   Vec     inertia    (          )C;   Vehicle& inertia    (C Vec     &inertia); // get/set inertia tensor
   Vec     pos        (          )C;   Vehicle& pos        (C Vec     &pos    ); // get/set position
   Matrix3 orn        (          )C;   Vehicle& orn        (C Matrix3 &orn    ); // get/set orientation, 'orn'    must be normalized
   Matrix  matrix     (          )C;   Vehicle& matrix     (C Matrix  &matrix ); // get/set matrix     , 'matrix' must be normalized
   Vec          vel   (          )C;   Vehicle&    vel     (C Vec     &vel    ); // get/set         velocity
   Vec       angVel   (          )C;   Vehicle& angVel     (C Vec     &vel    ); // get/set angular velocity
   Vec     pointVelL  (C Vec &pos)C;                                             // get     point   velocity ('pos' is in actor local space)
   Vec     pointVelW  (C Vec &pos)C;                                             // get     point   velocity ('pos' is in world       space)
   Box     box        (          )C;                                             // get     bounding box in world space
   Flt     scale      (          )C {return _scale;}                             // get     scale that was used during vehicle creation

   Vehicle& addTorque (C Vec &torque             ); // add torque                         , unit = mass * rotation / time**2
   Vehicle& addAngVel (C Vec &ang_vel            ); // add angular velocity               , unit =        rotation / time
   Vehicle& addForce  (C Vec &force              ); // add force                          , unit = mass * distance / time**2
   Vehicle& addForce  (C Vec &force  , C Vec &pos); // add force   at world 'pos' position, unit = mass * distance / time**2
   Vehicle& addImpulse(C Vec &impulse            ); // add impulse                        , unit = mass * distance / time
   Vehicle& addImpulse(C Vec &impulse, C Vec &pos); // add impulse at world 'pos' position, unit = mass * distance / time
   Vehicle& addVel    (C Vec &vel                ); // add velocity                       , unit =        distance / time
   Vehicle& addAccel  (C Vec &accel              ); // add acceleration                   , unit =        distance / time**2

   Bool      gravity    ()C;   Vehicle& gravity    (Bool      on       ); // get/set if gravity is enabled for this vehicle
   Bool      ray        ()C;   Vehicle& ray        (Bool      on       ); // get/set if this actor should be included when performing ray tests
   Bool      collision  ()C;   Vehicle& collision  (Bool      on       ); // get/set if this actor should collide with other actors in the world
   Bool      sleep      ()C;   Vehicle& sleep      (Bool      sleep    ); // get/set sleeping
   Flt       sleepEnergy()C;   Vehicle& sleepEnergy(Flt       energy   ); // get/set the amount of energy below the actor is put to sleep, default=0.005
   Bool      ccd        ()C;   Vehicle& ccd        (Bool      on       ); // get/set continuous collision detection
   Ptr       user       ()C;   Vehicle& user       (Ptr       user     ); // get/set custom user data
   Ptr       obj        ()C;   Vehicle& obj        (Ptr       obj      ); // get/set pointer to object containing the actor
   Byte      group      ()C;   Vehicle& group      (Byte      group    ); // get/set collision group (0..31, default value is taken according to ACTOR_GROUP)
   Byte      dominance  ()C;   Vehicle& dominance  (Byte      dominance); // get/set dominance index (0..31, default=0), for more information about dominance please check comments on 'Physics.dominance' method
   PhysMtrl* material   ()C;   Vehicle& material   (PhysMtrl *material ); // get/set physics material for vehicle (use 'null' for default material)

   // io
   Bool saveState(File &f)C; // save vehicle state (following data is not  saved: physical body, mass, density, scale, damping, max ang vel, mass center, inertia, material), false on fail
   Bool loadState(File &f) ; // load vehicle state (following data is not loaded: physical body, mass, density, scale, damping, max ang vel, mass center, inertia, material), false on fail, typically you should first create a vehicle from a physical body and then call this method to set its state according to data from the file

#if EE_PRIVATE
   void zero  ();
   void update();
#endif
  ~Vehicle() {del();}
   Vehicle();

#if !EE_PRIVATE
private:
#endif
   struct _Wheel
   {
      Flt   radius, compress, angle, angle_vel;
      Vec   pos;
      Plane contact;
   };
   WHEEL_DRIVE _wd;
   Flt         _accel, _brake, _angle, _max_accel, _max_back_accel, _max_brake, _brake_ratio, _max_angle, _fric_linear, _fric_scalar, _susp_spring, _susp_damping, _susp_compress, _scale, _avg_wheel_dist;
  _Wheel       _wheel[WHEEL_NUM];
   Actor       _actor;
};
/******************************************************************************/
