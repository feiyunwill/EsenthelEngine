/******************************************************************************

   Use 'Actor' to create Physical Objects.

/******************************************************************************/
enum ACTOR_GROUP : Byte // Actor Group (Actor::group)
{
   AG_DEFAULT=0, // default group for all actors

   // user defined groups should be in the range of 1..15

   AG_CONTROLLER=30, // Controller
   AG_TERRAIN   =31, // World Terrain
   AG_NUM       =32, // number of actor groups
};
/******************************************************************************/
#if EE_PRIVATE
#if PHYSX
struct _ActorShapes
{
   struct Shape
   {
      Flt       density;
      EE::Shape shape  ;

      void set(Flt density, C EE::Shape &shape) {T.density=density; T.shape=shape;}
   };
   struct Convex
   {
      Flt           density;
      Vec           scale;
      PxConvexMesh *convex;

      void set(Flt density, C Vec &scale, PxConvexMesh *convex) {T.density=density; T.scale=scale; T.convex=convex;}
   };
   struct Mesh
   {
      Flt             density;
      Vec             scale;
      PxTriangleMesh *mesh;

      void set(Flt density, C Vec &scale, PxTriangleMesh *mesh) {T.density=density; T.scale=scale; T.mesh=mesh;}
   };

   Memc<Shape    > shape ;
   Memc<Convex   > convex;
   Memc<Mesh     > mesh  ;
   Memc<PhysMesh*> pm    ;

 ~_ActorShapes();
};
#else
struct _ActorShapes
{
   struct Shape
   {
      Flt       density;
      EE::Shape shape  ;

      void set(Flt density, C EE::Shape &shape) {T.density=density; T.shape=shape;}
   };
   struct Convex
   {
      Flt                density, volume;
      Vec                scale;
      btConvexHullShape *convex;

      void set(Flt density, Flt volume, C Vec &scale, btConvexHullShape *convex) {T.density=density; T.volume=volume; T.scale=scale; T.convex=convex;}
   };
   struct Mesh
   {
      Flt                     density;
      Vec                     scale;
      btBvhTriangleMeshShape *mesh;

      void set(Flt density, C Vec &scale, btBvhTriangleMeshShape *mesh) {T.density=density; T.scale=scale; T.mesh=mesh;}
   };

   Memc<Shape    > shape ;
   Memc<Convex   > convex;
   Memc<Mesh     > mesh  ;
   Memc<PhysMesh*> pm    ;

 ~_ActorShapes();
};
STRUCT(RigidBody , btRigidBody)
//{
   PhysMtrl                *material  ; // material index
   Ptr                      user, obj ; // user and obj pointers
   Vec                      offset_com; // offset applied only to Center of Mass
   Matrix                   offset    ; // offset Bullet<->EE space
   Memc<btCollisionObject*> ignore    ; // list of pair-coliision ignore

   void materialApply();

  ~RigidBody();
   RigidBody(btRigidBody::btRigidBodyConstructionInfo &info);
};
#endif
#endif
/******************************************************************************/
struct ActorShapes // Actor Shapes used for creating Actors made out of multiple shapes
{
   ActorShapes& add(C Plane    &plane                    ); // add plane
   ActorShapes& add(C Box      &box    ,   Flt  density=1); // add box
   ActorShapes& add(C OBox     &obox   ,   Flt  density=1); // add obox
   ActorShapes& add(C Extent   &ext    ,   Flt  density=1); // add extent
   ActorShapes& add(C Ball     &ball   ,   Flt  density=1); // add ball
   ActorShapes& add(C Capsule  &capsule,   Flt  density=1); // add capsule
   ActorShapes& add(C Tube     &tube   ,   Flt  density=1); // add tube
   ActorShapes& add(C Shape    &shape  ,   Flt  density=1); // add shape
   ActorShapes& add(C PhysPart &part   , C Vec &scale  =1); // add PhysPart
   ActorShapes& add(C PhysBody &phys   , C Vec &scale  =1); // add PhysBody

  ~ActorShapes();
   ActorShapes();

private:
#if EE_PRIVATE
   ActorShapes& add(C PhysPart &part, Flt density, C Vec &scale);

  _ActorShapes *_as;
   friend struct Actor;
#else
   Ptr       _as;
#endif
   PhysMtrl *_mtrl;
   NO_COPY_CONSTRUCTOR(ActorShapes);
};
/******************************************************************************/
struct Actor // Physical Object
{
   // manage
   Actor& del(); // delete manually

   Actor& create (C ActorShapes &shapes , Flt density=1,                     Bool kinematic=false); // create from actor shapes, Exit on fail, 'density'=density multiplier,                                 'kinematic'=if create actor as kinematic
   Actor& create (C Plane       &plane                                                           ); // create from plane       , Exit on fail
   Actor& create (C Box         &box    , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from box         , Exit on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Actor& create (C OBox        &obox   , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from oriented box, Exit on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Actor& create (C Extent      &ext    , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from extent      , Exit on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Actor& create (C Ball        &ball   , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from ball        , Exit on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Actor& create (C Capsule     &capsule, Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from capsule     , Exit on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Actor& create (C Tube        &tube   , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from tube        , Exit on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Actor& create (C Shape       &shape  , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from shape       , Exit on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Actor& create (C PhysPart    &part   , Flt density=1, C Vec &scale =1   , Bool kinematic=false); // create from PhysPart    , Exit on fail, 'density'=density multiplier, 'scale '=custom body  scale   , 'kinematic'=if create actor as kinematic
   Actor& create (C PhysBody    &phys   , Flt density=1, C Vec &scale =1   , Bool kinematic=false); // create from PhysBody    , Exit on fail, 'density'=density multiplier, 'scale '=custom body  scale   , 'kinematic'=if create actor as kinematic

   Bool createTry(C ActorShapes &shapes , Flt density=1,                     Bool kinematic=false); // create from actor shapes, false on fail, 'density'=density multiplier,                                 'kinematic'=if create actor as kinematic
   Bool createTry(C Plane       &plane                                                           ); // create from plane       , false on fail
   Bool createTry(C Box         &box    , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from box         , false on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Bool createTry(C OBox        &obox   , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from oriented box, false on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Bool createTry(C Extent      &ext    , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from extent      , false on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Bool createTry(C Ball        &ball   , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from ball        , false on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Bool createTry(C Capsule     &capsule, Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from capsule     , false on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Bool createTry(C Tube        &tube   , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from tube        , false on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Bool createTry(C Shape       &shape  , Flt density=1, C Vec *anchor=null, Bool kinematic=false); // create from shape       , false on fail, 'density'=density           , 'anchor'=custom actor position, 'kinematic'=if create actor as kinematic
   Bool createTry(C PhysPart    &part   , Flt density=1, C Vec &scale =1   , Bool kinematic=false); // create from PhysPart    , false on fail, 'density'=density multiplier, 'scale '=custom body  scale   , 'kinematic'=if create actor as kinematic
   Bool createTry(C PhysBody    &phys   , Flt density=1, C Vec &scale =1   , Bool kinematic=false); // create from PhysBody    , false on fail, 'density'=density multiplier, 'scale '=custom body  scale   , 'kinematic'=if create actor as kinematic

   // get / set
   Bool     is        (          )C {return _actor!=null;}                     // if      created
   Flt      energy    (          )C;                                           // get     kinetic energy      , 0..Inf
   Flt      damping   (          )C;   Actor&  damping   (  Flt      damping); // get/set linear  damping     , 0..Inf, default=0.05
   Flt     adamping   (          )C;   Actor& adamping   (  Flt      damping); // get/set angular damping     , 0..Inf, default=0.05
   Flt     maxAngVel  (          )C;   Actor& maxAngVel  (  Flt      vel    ); // get/set max angular velocity, 0..Inf
   Flt     mass       (          )C;   Actor& mass       (  Flt      mass   ); // get/set mass                , 0..Inf
   Vec     massCenterL(          )C;   Actor& massCenterL(C Vec     &center ); // get/set mass center in actor local space
   Vec     massCenterW(          )C;   Actor& massCenterW(C Vec     &center ); // get/set mass center in world       space
   Vec     inertia    (          )C;   Actor& inertia    (C Vec     &inertia); // get/set inertia tensor
   Vec     pos        (          )C;   Actor& pos        (C Vec     &pos    ); // get/set position
   Matrix3 orn        (          )C;   Actor& orn        (C Matrix3 &orn    ); // get/set orientation, 'orn'    must be normalized
   Matrix  matrix     (          )C;   Actor& matrix     (C Matrix  &matrix ); // get/set matrix     , 'matrix' must be normalized
   Vec          vel   (          )C;   Actor&    vel     (C Vec     &vel    ); // get/set         velocity
   Vec       angVel   (          )C;   Actor& angVel     (C Vec     &vel    ); // get/set angular velocity
   Vec     pointVelL  (C Vec &pos)C;                                           // get     point   velocity ('pos' is in actor local space)
   Vec     pointVelW  (C Vec &pos)C;                                           // get     point   velocity ('pos' is in world       space)
   Box     box        (          )C;                                           // get     bounding box in world space
   Shape   shape      (Bool local)C;                                           // get     shape, 'local'=if in actor local space or world space !! only first actor's shape is used !!

   Actor& kinematicMoveTo(C Vec     &pos   ); // move kinematic actor to position   , this is slightly different than 'pos'    method, 'pos'    method works more like "teleport to position"    while 'kinematicMoveTo' works by linearly moving from source to target position   , the position    isn't changed immediately - change occurs during the next physics simulation call, this method can be used only on kinematic actors
   Actor& kinematicMoveTo(C Matrix3 &orn   ); // move kinematic actor to orientation, this is slightly different than 'orn'    method, 'orn'    method works more like "teleport to orientation" while 'kinematicMoveTo' works by linearly moving from source to target orientation, the orientation isn't changed immediately - change occurs during the next physics simulation call, this method can be used only on kinematic actors, 'orn'    must be normalized
   Actor& kinematicMoveTo(C Matrix  &matrix); // move kinematic actor to matrix     , this is slightly different than 'matrix' method, 'matrix' method works more like "teleport to matrix"      while 'kinematicMoveTo' works by linearly moving from source to target matrix     , the matrix      isn't changed immediately - change occurs during the next physics simulation call, this method can be used only on kinematic actors, 'matrix' must be normalized

   Actor& addTorque (C Vec &torque             ); // add torque                         , unit = mass * rotation / time**2
   Actor& addAngVel (C Vec &ang_vel            ); // add angular velocity               , unit =        rotation / time
   Actor& addForce  (C Vec &force              ); // add force                          , unit = mass * distance / time**2
   Actor& addForce  (C Vec &force  , C Vec &pos); // add force   at world 'pos' position, unit = mass * distance / time**2
   Actor& addImpulse(C Vec &impulse            ); // add impulse                        , unit = mass * distance / time
   Actor& addImpulse(C Vec &impulse, C Vec &pos); // add impulse at world 'pos' position, unit = mass * distance / time
   Actor& addVel    (C Vec &vel                ); // add velocity                       , unit =        distance / time
   Actor& addAccel  (C Vec &accel              ); // add acceleration                   , unit =        distance / time**2

   Bool      freezePosX ()C;   Actor& freezePosX (Bool      freeze   ); // get/set freeze position (x   component )
   Bool      freezePosY ()C;   Actor& freezePosY (Bool      freeze   ); // get/set freeze position (y   component )
   Bool      freezePosZ ()C;   Actor& freezePosZ (Bool      freeze   ); // get/set freeze position (z   component )
   Bool      freezePos  ()C;   Actor& freezePos  (Bool      freeze   ); // get/set freeze position (all components)
   Bool      freezeRotX ()C;   Actor& freezeRotX (Bool      freeze   ); // get/set freeze rotation (x   axis      )
   Bool      freezeRotY ()C;   Actor& freezeRotY (Bool      freeze   ); // get/set freeze rotation (y   axis      )
   Bool      freezeRotZ ()C;   Actor& freezeRotZ (Bool      freeze   ); // get/set freeze rotation (z   axis      )
   Bool      freezeRot  ()C;   Actor& freezeRot  (Bool      freeze   ); // get/set freeze rotation (all components)
   Bool      kinematic  ()C;   Actor& kinematic  (Bool      on       ); // get/set if kinematic, only dynamic actors (with mass!=0) can be changed into kinematic actors
   Bool      gravity    ()C;   Actor& gravity    (Bool      on       ); // get/set if gravity is enabled for this actor
   Bool      ray        ()C;   Actor& ray        (Bool      on       ); // get/set if this actor should be included when performing ray tests
   Bool      collision  ()C;   Actor& collision  (Bool      on       ); // get/set if this actor should collide with other actors in the world
   Bool      trigger    ()C;   Actor& trigger    (Bool      on       ); // get/set if this actor should be treated as trigger, if actor is marked as trigger then it will make physics simulation call trigger callback functions specified using 'Physics.reportTrigger' method when the actor will collide with other actors, enabling trigger also disables all collision responses (just like disabling 'collision' method)
   Bool      sleep      ()C;   Actor& sleep      (Bool      sleep    ); // get/set sleeping
   Flt       sleepEnergy()C;   Actor& sleepEnergy(Flt       energy   ); // get/set the amount of energy below the actor is put to sleep, default=0.005
   Bool      ccd        ()C;   Actor& ccd        (Bool      on       ); // get/set continuous collision detection
   Ptr       user       ()C;   Actor& user       (Ptr       user     ); // get/set custom user data
   Ptr       obj        ()C;   Actor& obj        (Ptr       obj      ); // get/set pointer to object containing the actor
   Byte      group      ()C;   Actor& group      (Byte      group    ); // get/set collision group (0..31, default value is taken according to ACTOR_GROUP)
   Byte      dominance  ()C;   Actor& dominance  (Byte      dominance); // get/set dominance index (0..31, default=0), for more information about dominance please check comments on 'Physics.dominance' method
   PhysMtrl* material   ()C;   Actor& material   (PhysMtrl *material ); // get/set physics material (use 'null' for default material)

   Actor& ignore (  Actor     &actor, Bool ignore=true) ; // ignore collisions with 'actor' actor
   Bool   ignored(C Actor     &actor                  )C; // if this actor should ignore collisions with 'actor' actor, this does not test for ACTOR_GROUP collisions (specified using 'Physics.ignore') but only 'Actor::ignore' method
   Bool   ignored(C ActorInfo &actor                  )C; // if this actor should ignore collisions with 'actor' actor, this does not test for ACTOR_GROUP collisions (specified using 'Physics.ignore') but only 'Actor::ignore' method

   Actor& active(Bool on) {collision(on).ray(on).kinematic(!on).sleep(!on); return T;} // set if active, adjusts "collision, ray, kinematic, sleep" to either include or exclude actor in physics simulation

   // test
   Bool cuts (                                             UInt groups=~0)C; // if cuts some other actor on the scene                                                                                  , 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing - use 'IndexToFlag' function
   void cuts (             PhysCutsCallback &callback    , UInt groups=~0)C; // if cuts some other actor on the scene , 'callback' will be used for every contact check                                , 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing - use 'IndexToFlag' function
   Bool sweep(C Vec &move, PhysHit         *phys_hit=null, UInt groups=~0)C; // if hits some other actor on 'move' way, 'phys_hit'=optionally pass pointer to PhysHit object to receive additional data, 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing - use 'IndexToFlag' function
   void sweep(C Vec &move, PhysHitCallback &callback     , UInt groups=~0)C; // if hits some other actor on 'move' way, 'callback' will be used for every contact check                                , 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing - use 'IndexToFlag' function

   // draw
   void draw(C Color &color=WHITE, Bool fill=false)C; // this can be optionally called outside of Render function

   // io
   Bool saveState(File &f)C; // save actor state ('user' is  saved only as 'UInt' and not 'UIntPtr', following data is not  saved: physical body, mass, density, scale, damping, max ang vel, mass center, inertia, material, object, sleepEnergy), false on fail
   Bool loadState(File &f) ; // load actor state ('user' is loaded only as 'UInt' and not 'UIntPtr', following data is not loaded: physical body, mass, density, scale, damping, max ang vel, mass center, inertia, material, object, sleepEnergy), false on fail, typically you should first create an actor from a physical body and then call this method to set its state according to data from the file

  ~Actor() {del();}
   Actor() {_actor=null; _dynamic=null; _ignore_id=0;}

#if EE_PRIVATE
      Bool materialForce(PhysMtrl *material);
   #if PHYSX
      Bool isDynamic()C {return            _dynamic!=null;}
      Bool isStatic ()C {return _actor && !_dynamic      ;}
   #else
      Bool init(btRigidBody::btRigidBodyConstructionInfo &info, C Vec *anchor, Bool kinematic, PhysMesh *pm, PhysMtrl *material);

      Bool createTry(btConvexHullShape      *convex, C Vec &scale, Flt density, Bool kinematic, PhysMesh *pm, PhysMtrl *material);
      Bool createTry(btBvhTriangleMeshShape *mesh  , C Vec &scale                             , PhysMesh *pm, PhysMtrl *material);

      Matrix massCenterMatrix()C;
   #endif
#endif

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   PHYS_API(PxRigidActor  , RigidBody) *_actor;
   PHYS_API(PxRigidDynamic, void     ) *_dynamic;
   Mems<PhysMesh*>                      _pm;
#else
   Ptr       _actor, _dynamic;
   Mems<Ptr> _pm;
#endif
   UInt      _ignore_id;

   NO_COPY_CONSTRUCTOR(Actor);
};
/******************************************************************************/
#if EE_PRIVATE
enum ACTOR_FLAG
{
   ACTOR_FREEZE_POS_X=1<< 0,
   ACTOR_FREEZE_POS_Y=1<< 1,
   ACTOR_FREEZE_POS_Z=1<< 2,
   ACTOR_FREEZE_ROT_X=1<< 3,
   ACTOR_FREEZE_ROT_Y=1<< 4,
   ACTOR_FREEZE_ROT_Z=1<< 5,
   ACTOR_KINEMATIC   =1<< 6,
   ACTOR_GRAVITY     =1<< 7,
   ACTOR_RAY         =1<< 8,
   ACTOR_COLLISION   =1<< 9,
   ACTOR_TRIGGER     =1<<10,
   ACTOR_SLEEP       =1<<11,
   ACTOR_CCD         =1<<12,
};
#endif
/******************************************************************************/
