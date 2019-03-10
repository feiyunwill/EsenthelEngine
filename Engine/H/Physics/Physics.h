/******************************************************************************

   Use 'Physics' to create and manage physical scene parameters and general manipulation.

   These include:
      -collision detection
      -ray testing
      -sweeping
      -setting physical material parameters
      -gravity
      -skin width
      -continuous collision detection
      -physics precision
      -timestep modes
      -other scene parameters

/******************************************************************************/
enum CONTROLLER_SLOPE_SLIDING_MODE : Byte // handling the character controller slope sliding issue
{
   CSS_NONE     , // nothing is done in this case
   CSS_MATERIALS, // Character Controllers will switch  their actor material depending whether the character is moving or standing, between 'Physics.mtrl_ctrl' <-> 'Physics.mtrl_ctrl_stop', which have different frictions
   CSS_GRAVITY  , // Character Controllers will disable their gravity     when standing still on ground, not recommended when character weight should affect objects underneath
   CSS_FREEZE_XZ, // Character Controllers will disable their xz movement when standing still on ground, not recommended when character needs to be pushed by other objects
};
/******************************************************************************/
enum PHYS_TIMESTEP_MODE : Byte // Physics timestep updating modes
{
   PHYS_TIMESTEP_ROUND   , // physics timesteps will be rounded to a constant value, this results in more stable and reproducible simulation
   PHYS_TIMESTEP_VARIABLE, // physics timesteps are taken from frame time deltas   , this results in more smooth but less stable  simulation
   PHYS_TIMESTEP_NUM     , // number of physics timestep modes
};
/******************************************************************************/
enum PHYS_ENGINE // Physics Engine
{
   PHYS_ENGINE_PHYSX , // PhysX
   PHYS_ENGINE_BULLET, // Bullet
};
/******************************************************************************/
enum PHYS_CONTACT // Physics Contact
{
   CONTACT_NEW , // new contact was found
   CONTACT_KEEP, //     contact persists
   CONTACT_LOST, //     contact was lost
};
/******************************************************************************/
struct ActorInfo // Actor Information
{
   Bool collision, // if this actor should collide with other actors in the world
        dynamic  ; // actor is dynamic (not static and not kinematic)
   Int  group    ; // actor collision group (ACTOR_GROUP)
   Ptr  user, obj; // actor user data and object pointer

#if EE_PRIVATE
   void set(PHYS_API(PxShape, RigidBody) *actor);
#endif

   ActorInfo() {collision=dynamic=false; group=0; user=obj=null; _actor=null;}

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   PHYS_API(PxRigidActor, RigidBody) *_actor;
#else
   Ptr _actor;
#endif
};
/******************************************************************************/
struct PhysHitBasic // Physics Hit, contains information about physical collision
{
   Int   face    ; // face index, this index will point to the original face of the source mesh only if the physical body was created with 'keep_face_indexes' parameter set to true, otherwise this will point to an internal face index of the physical body
   Flt   frac    , // fraction of the movement on which collision occurred (0..1)
         dist    ; // distance travelled until          collision occurred (0..move.length())
   Vec   face_nrm; // face normal (if face index is known then it's set to face normal in world space, otherwise it's set to 'plane.normal')
   Plane plane   ; // contact plane (position and normal)

#if EE_PRIVATE
#if PHYSX
   void set       (C PxLocationHit          &hit ,   Flt      move_length);
   void updateFace(C PxTriangleMeshGeometry &mesh, C Matrix3 *orn        );
#endif
#endif
};
struct PhysHit : PhysHitBasic, ActorInfo // Physics Hit, contains information about physical collision
{
#if EE_PRIVATE
#if PHYSX
   void set(C PxLocationHit &hit, Flt move_length);
#endif
#endif
};
/******************************************************************************/
struct PhysContact // Physics contact, contains information about physical contact
{
   Vec pos   , // world space position
       normal, // world space normal
       force ; // world space force
};
/******************************************************************************/
struct PhysCutsCallback // Physical Cuts Test Callback interface
{
   virtual Bool hit(ActorInfo &actor_info)=NULL; // you should override this method, it will receive information about the contacts, you should return false if you no longer want to check any more contacts, and true to continue checking, creating or deleting physical objects inside callbacks is forbidden, instead you should record information about contacts, and perform operations on them outside of the callback, contacts are not listed in any specific order
};
struct PhysHitCallback // Physical Ray/Sweep Test Callback interface
{
   virtual Bool hit(PhysHit &phys_hit)=NULL; // you should override this method, it will receive information about the contacts, you should return false if you no longer want to check any more contacts, and true to continue checking, creating or deleting physical objects inside callbacks is forbidden, instead you should record information about contacts, and perform operations on them outside of the callback, contacts are not listed in any specific order (do not assume that they are sorted from nearest to furthest)
};
/******************************************************************************/
struct PhysicsClass
{
   UInt ctrl_ground_group_force    , // controller force      ground group (ACTOR_GROUP) bit combination, this is a bit combination of all actor groups that will always be walkable by character controllers even with high    slope angles, default="0"                           (none)
        ctrl_ground_group_allow    , // controller allowed    ground group (ACTOR_GROUP) bit combination, this is a bit combination of all actor groups that will        be walkable by character controllers      with correct slope angles, default="~IndexToFlag(AG_CONTROLLER)" (everything except Controllers)
        ctrl_slide_group_horizontal; // controller horizontal slide  group (ACTOR_GROUP) bit combination, this is a bit combination of all actor groups that will        be slided against only in horizontal mode                          , default=" IndexToFlag(AG_CONTROLLER)" (                  Controllers)
   Flt  ctrl_ground_slope          ; // controller            ground slope (0..1)                       , this factor determines what slope angles are still walkable by character controllers, default=0.65

   PhysMtrl mtrl_default  , // default material for all 'Actor'      objects
            mtrl_ctrl     , // default material for all 'Controller' objects
            mtrl_ctrl_stop; // default material for all 'Controller' objects when they are not moving and 'Physics' was initialized with 'CSS_MATERIALS' parameter

   void (*simulation_step_completed)(); // pointer to a custom callback (can be null) called each time a single simulation step has completed, this can be called 0, 1 or multiple times between a call to 'startSimulation' and 'stopSimulation', each call will be made on the main thread, amount of calls depends on 'dt' parameter used in 'startSimulation', 'precision' and 'timestep'.

   // manage
   PhysicsClass& create   (C Str &physx_dll_path=S, CONTROLLER_SLOPE_SLIDING_MODE css=CSS_MATERIALS, Bool hardware=false); // create, 'physx_dll_path'=path to look for physx DLL files (if empty then DLL's will be searched in 'CurDir', this parameter is needed only for Windows platform, on Linux platform Physx Shared Libraries are always expected to be in the "Bin" folder relative to app file, on other platforms it's ignored), 'hardware'=if use hardware physics simulations (on GPU), if it's not available or 'hardware' is set to false then CPU will be used, Exit  on fail
   Bool          createTry(C Str &physx_dll_path=S, CONTROLLER_SLOPE_SLIDING_MODE css=CSS_MATERIALS, Bool hardware=false); // create, 'physx_dll_path'=path to look for physx DLL files (if empty then DLL's will be searched in 'CurDir', this parameter is needed only for Windows platform, on Linux platform Physx Shared Libraries are always expected to be in the "Bin" folder relative to app file, on other platforms it's ignored), 'hardware'=if use hardware physics simulations (on GPU), if it's not available or 'hardware' is set to false then CPU will be used, false on fail

   // get / set
                                                           Bool               created        ()C;                        // if      physics has been created
                                                           Bool               updated        ()C {return _updated     ;} // if                       physics simulation            has been updated   during                          last simulation. This value is valid after calling  'stopSimulation'. This may be false if you use 'PHYS_TIMESTEP_ROUND' and requested simulation time didn't exceed minimum required value
                                                           Flt                updatedTime    ()C {return _updated_time;} // get     actual amount of physics simulation time which has been performed during             all steps of last simulation. This value is valid after calling  'stopSimulation'. When 'PHYS_TIMESTEP_ROUND' is used, the returned value will always be a multiple of a constant value based on the 'precision', for 'PHYS_TIMESTEP_VARIABLE' the returned value will be equal to 'dt' parameter used for 'startSimulation'. This value is equal to the sum of all 'stepTime' for each simulation step that occurred in the last simulation.
                                                           Flt                   stepTime    ()C {return    _step_time;} // get     actual amount of physics simulation time which has been performed during a single        step  of last simulation. This value is valid after calling 'startSimulation'. It's equal to 'updatedTime' divided by number of simulation steps. To get simulated time for all steps in a simulation, please use 'updatedTime'.
                                                           Flt                       time    ()C {return         _time;} // get     actual amount of physics simulation time which has been performed during a single or all steps of last simulation. This value is valid after calling  'stopSimulation' and during 'simulation_step_completed'. It's set to 'stepTime' during 'simulation_step_completed' and to 'updatedTime' when outside 'simulation_step_completed'.
                                                           Flt                    cpuTime    ()C {return     _cpu_time;} // get            amount of CPU time on the main thread which was needed to perform                          last simulation. This value is valid after calling  'stopSimulation'. You can use this method to know how much CPU time on the main thread was required to perform the last simulation. Simulation can also take time on other threads, however that time is not measured.
                                                           Bool               hw             ()C {return _hw          ;} // if      physics is simulated in the hardware
   PhysicsClass& skin     (  Flt              skin    );   Flt                skin           ()C {return _skin        ;} // set/get physics skin width (allowed object interpenetration), default=0.005 (changing this value will not affect actors that are already created, in order to have one global skin value, please call this function after physics creation and before any actor creation)
   PhysicsClass& gravity  (C Vec             &gravity ); C Vec&               gravity        ()C {return _gravity     ;} // set/get gravity acceleration                                , default=(0, -9.8, 0)
   PhysicsClass& precision(  Int              steps_ps);   Int                precision      ()C {return _precision   ;} // set/get number of desired simulation steps per second       , default=60 (setting precision to zero forces usage of screen refresh rate)
   PhysicsClass& timestep (PHYS_TIMESTEP_MODE mode    );   PHYS_TIMESTEP_MODE timestep       ()C {return _timestep    ;} // set/get physics time stepping mode                          , default=PHYS_TIMESTEP_ROUND
                                                           PHYS_ENGINE        engine         ()C;                        //     get physics engine which is available in current build
                                                           UInt               collisionGroups(Byte group)C;              //     get bit combination of ACTOR_GROUP that collide with 'group' ACTOR_GROUP
#if EE_PRIVATE
   Flt minShapeRadius()C {return _skin*4;}
   Flt minCapsuleEdge()C {return 0.01f  ;}
   Int actorShapes   ()C; // get number of actor shapes in world
#endif

   // if shape cuts with any actor on the scene, 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing, use 'IndexToFlag' function
   static Bool cuts(C Box     &box    , UInt groups=~0);
   static Bool cuts(C OBox    &obox   , UInt groups=~0);
   static Bool cuts(C Ball    &ball   , UInt groups=~0);
   static Bool cuts(C Capsule &capsule, UInt groups=~0);
   static Bool cuts(C Shape   &shape  , UInt groups=~0); // only SHAPE_BOX, SHAPE_OBOX, SHAPE_BALL, SHAPE_CAPSULE shape types are supported

   // if shape cuts with any actor on the scene, 'callback' will be used for every contact check, 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing, use 'IndexToFlag' function, contacts are not listed in any specific order
   static void cuts(C Box     &box    , PhysCutsCallback &callback, UInt groups=~0);
   static void cuts(C OBox    &obox   , PhysCutsCallback &callback, UInt groups=~0);
   static void cuts(C Ball    &ball   , PhysCutsCallback &callback, UInt groups=~0);
   static void cuts(C Capsule &capsule, PhysCutsCallback &callback, UInt groups=~0);
   static void cuts(C Shape   &shape  , PhysCutsCallback &callback, UInt groups=~0); // only SHAPE_BOX, SHAPE_OBOX, SHAPE_BALL, SHAPE_CAPSULE shape types are supported

   // if ray cuts with any actor on the scene, 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing, use 'IndexToFlag' function
   static Bool ray(C Vec &pos, C Vec &move, PhysHit         *phys_hit=null, UInt groups=~0); // 'phys_hit'=optionally pass pointer to 'PhysHit' class to receive additional data about the nearest contact
   static void ray(C Vec &pos, C Vec &move, PhysHitCallback &callback     , UInt groups=~0); // 'callback' will be used for every contact check, contacts are not listed in any specific order (do not assume that they are sorted from nearest to furthest)

   // if shape hits something on 'move' way, 'phys_hit'=optionally pass pointer to 'PhysHit' class to receive additional data about the nearest contact, 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing, use 'IndexToFlag' function
   static Bool sweep(C Box     &box    , C Vec &move, PhysHit *phys_hit=null, UInt groups=~0);
   static Bool sweep(C OBox    &obox   , C Vec &move, PhysHit *phys_hit=null, UInt groups=~0);
   static Bool sweep(C Ball    &ball   , C Vec &move, PhysHit *phys_hit=null, UInt groups=~0);
   static Bool sweep(C Capsule &capsule, C Vec &move, PhysHit *phys_hit=null, UInt groups=~0);
   static Bool sweep(C Shape   &shape  , C Vec &move, PhysHit *phys_hit=null, UInt groups=~0); // only SHAPE_BOX, SHAPE_OBOX, SHAPE_BALL, SHAPE_CAPSULE shape types are supported

   // if shape hits something on 'move' way, 'callback' will be used for every contact check, 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing, use 'IndexToFlag' function, contacts are not listed in any specific order (do not assume that they are sorted from nearest to furthest)
   static void sweep(C Box     &box    , C Vec &move, PhysHitCallback &callback, UInt groups=~0);
   static void sweep(C OBox    &obox   , C Vec &move, PhysHitCallback &callback, UInt groups=~0);
   static void sweep(C Ball    &ball   , C Vec &move, PhysHitCallback &callback, UInt groups=~0);
   static void sweep(C Capsule &capsule, C Vec &move, PhysHitCallback &callback, UInt groups=~0);
   static void sweep(C Shape   &shape  , C Vec &move, PhysHitCallback &callback, UInt groups=~0); // only SHAPE_BOX, SHAPE_OBOX, SHAPE_BALL, SHAPE_CAPSULE shape types are supported

   // move shape by 'move' way, if shape encounters an obstacle along the way it will be stopped at contact point, 'groups'=group flag (ACTOR_GROUP) bit combination specifying which groups should be included in testing, use 'IndexToFlag' function
   static void move(Box     &box    , C Vec &move, UInt groups=~0);
   static void move(OBox    &obox   , C Vec &move, UInt groups=~0);
   static void move(Ball    &ball   , C Vec &move, UInt groups=~0);
   static void move(Capsule &capsule, C Vec &move, UInt groups=~0);
   static void move(Shape   &shape  , C Vec &move, UInt groups=~0); // only SHAPE_BOX, SHAPE_OBOX, SHAPE_BALL, SHAPE_CAPSULE shape types are supported

   // operations
   PhysicsClass& ignore(Int group_a, Int group_b, Bool ignore=true); // ignore collisions between 'group_a' and 'group_b' actor group pairs (ACTOR_GROUP)

   PhysicsClass& reportContact(Int group_a, Int group_b, Bool report=true                                                         ); // allow contact reporting between 'group_a' and 'group_b' actor group pairs (ACTOR_GROUP), this will enable calling 'ReportContact' function which can be specified in 'reportContact' method
   PhysicsClass& reportContact(void (*ReportContact)(ActorInfo &actor_a, ActorInfo &actor_b, C PhysContact *contact, Int contacts)); // set custom function for reporting physical contacts, this function will be called for actor group pairs that have been enabled using the 'reportContact' method, 'ReportContact' function will be called during physical contacts between actors in physical simulation, this function will not be called if the actors aren't in contact or have their collisions disabled or are asleep, creating or deleting physical objects inside callbacks is forbidden, instead you should record information about contacts, and perform operations on them outside of the callback, 'contact'=array of contacts, 'contacts'=number of elements in 'contact' array

   PhysicsClass& reportTrigger(void (*ReportTrigger)(ActorInfo &trigger, ActorInfo &actor, PHYS_CONTACT contact)); // set custom function for reporting trigger events, this function will be called for actors (with enabled 'Actor::trigger') when they collide with other actors, creating or deleting physical objects inside callbacks is forbidden, instead you should record information about contacts, and perform operations on them outside of the callback

   PhysicsClass& wheelFriction(Flt (*WheelFriction)(C PhysMtrl &ground_material, C PhysMtrl &wheel_material, C ActorInfo &vehicle, WHEEL_TYPE wheel)); // set custom function for calculating vehicle wheel friction with ground depending on given parameters, this function will be called when vehicle wheels will be in contact with any physical surface, you should return friction value (in range 0..Inf), where 0=no friction, 1=good friction, set null for default function (always returning friction of 1)

   PhysicsClass& dominance(Int dominance_a, Int dominance_b, Bool a_factor, Bool b_factor); // set dominance constraints factors, 'dominance_a'=dominance index (0..31), 'dominance_b'=dominance index (0..31), 'a_factor'=how much actors of 'dominance_a' index are affected by actors of 'dominance_b' index (factor in range 0..1), 'b_factor'=how much actors of 'dominance_b' index are affected by actors of 'dominance_a' index (factor in range 0..1), this allows for specifying one-way interactions between different actor dominance groups

   // simulation
   PhysicsClass& startSimulation(Flt dt=Time.d()); // start physics  simulation !! on Bullet engine you may not perform any changes in physics between startSimulation() and stopSimulation(), this includes actor creation/removal, modifying parameters, etc. this does not apply to PhysX !! each simulation can be divided into 0, 1 or multiple simulation steps, amount of simulation steps depends on 'dt' parameter, 'timestep' and 'precision'. To get notified about when each simulation step completed, please use the 'simulation_step_completed' callback. Simulation may be performed on secondary threads.
   PhysicsClass&  stopSimulation(               ); // get results of simulation !! on Bullet engine you may not perform any changes in physics between startSimulation() and stopSimulation(), this includes actor creation/removal, modifying parameters, etc. this does not apply to PhysX !! this method waits until the last simulation has completed.

   // draw
   void draw(); // draw all actors, this can be optionally called outside of Render function

#if EE_PRIVATE
   static Bool ignored        (PHYS_API(PxRigidActor, RigidBody) &a, PHYS_API(PxRigidActor, RigidBody) &b); // if 'a' and 'b' actors are ignoring each other (based on Actor::ignore)
          Bool collides       (Byte a, Byte b)C {return FlagTest(_collision_array[a], 1<<b);} // if 'a' can     collide  with 'b' ACTOR_GROUP
          Bool collidesMask   (Byte a, UInt m)C {return FlagTest(                 m , 1<<a);} // if 'a' can     collide  with 'm' ACTOR_GROUP mask
          Bool reports        (Byte a, Byte b)C {return          _contact_report [a][b]    ;} // if 'a' reports contacts with 'b' ACTOR_GROUP
          Bool createMaterials();
          void step           ();
          void stepCompleted  ();
#endif

   void del(); // manually release physics and its DLL file usage, normally you don't need to call this as the engine will call this automatically

#if !EE_PRIVATE
private:
#endif
   Bool                          _hw, _simulated, _updated, _new_updated, _last_updated, _contact_report[32][32];
   Int                           _precision, _actual_precision, _step_left;
   UInt                          _update_count, _collision_array[32];
   Flt                           _skin, _cpu_time, _new_cpu_time, _updated_time, _new_updated_time, _last_updated_time, _prec_time, _accumulated_time, _step_time, _time;
   Vec                           _gravity;
   ReadWriteSync                 _rws;
   CONTROLLER_SLOPE_SLIDING_MODE _css;
   PHYS_TIMESTEP_MODE            _timestep;
   void                        (*_report_contact)(ActorInfo &actor_a, ActorInfo &actor_b, C PhysContact *contact, Int contacts);
   void                        (*_report_trigger)(ActorInfo &trigger, ActorInfo &actor, PHYS_CONTACT contact);
   Flt                         (*_wheel_friction)(C PhysMtrl &ground_material, C PhysMtrl &wheel_material, C ActorInfo &vehicle, WHEEL_TYPE wheel);
   Memc<Vehicle*>                _vehicles;

   PhysicsClass();
}extern
   Physics; // !! Physics needs to be manually created !!
/******************************************************************************/
#if EE_PRIVATE
#if PHYSX
struct PhysxClass
{
   #define MAX_ACTOR_IGNORE_SHIFT 10
   #define MAX_ACTOR_IGNORE       (1<<MAX_ACTOR_IGNORE_SHIFT)

   STRUCT(AllocatorCallback , PxDefaultAllocator)
   //{
	   virtual void*  allocate(size_t size, const char* typeName, const char* filename, int line)override;
	   virtual void deallocate(void* ptr)override;
   };

   static Vec         vec   (C PxVec3      &v     ) {return Vec      (v.x, v.y, v.z);}
   static PxVec3      vec   (C Vec         &v     ) {return PxVec3   (v.x, v.y, v.z);}
   static Box         box   (C PxBounds3   &box   ) {return Box      (vec(box.minimum), vec(box.maximum));}
   static PxBounds3   box   (C Box         &box   ) {return PxBounds3(vec(box.min    ), vec(box.max    ));}
   static Matrix      matrix(C PxTransform &matrix);
   static PxTransform matrix(C Matrix      &matrix);
   static Matrix3     orn   (C PxQuat      &quat  );
   static Matrix3     orn   (C PxMat33     &mat   );
   static PxQuat      orn   (C Matrix3     &matrix);

   void del   ();
   Bool create(Str dll_path, Bool hardware);

   void updateVehicles();

   INLINE Byte& ignoreMap(UInt a, UInt b) {return ignore_map[a + ((b>>3)<<MAX_ACTOR_IGNORE_SHIFT)];}

   Int                    _mem_leaks;
   PxFoundation           *foundation;
   PxPhysics              *physics;
   PxCooking              *cook[2];
   PxScene                *world;
   AllocatorCallback       allocator;
   Mems<Byte>              ignore_map;
   IDGenerator             ignore_id_gen;
#define SUPPORT_PHYSX_VEHICLE 0
#if     SUPPORT_PHYSX_VEHICLE
   IDGenerator             vehicle_id_gen;
   PhysPart                wheel_mesh;
   PxRaycastQueryResult    raycast_query_result[4];
   PxRaycastHit            raycast_hit         [4];
   PxBatchQuery           *batch_query_4;
   Memc<PxVehicleDrive4W*> vehicles;
#endif
#if PHYSX_DLL_ACTUAL
   DLL                     PhysX3, PhysXCommon, PhysXCooking, PhysXFoundation;
   Ptr                     raycast;
#endif

   PhysxClass()
   {
      foundation=null; physics=null; cook[0]=cook[1]=null; world=null; _mem_leaks=0;
   #if SUPPORT_PHYSX_VEHICLE
      batch_query_4=null;
   #endif
   #if PHYSX_DLL_ACTUAL
      raycast=null;
   #endif
   }
   NO_COPY_CONSTRUCTOR(PhysxClass);
}extern
   Physx;

void DrawConvex(PxConvexMesh   &convex, C Color &color=WHITE, Bool fill=false);
void DrawMesh  (PxTriangleMesh &mesh  , C Color &color=WHITE, Bool fill=false);
#else
struct BulletClass
{
   static Vec         vec   (C btVector3   &v) {return Vec      (v.x(), v.y(), v.z());}
   static btVector3   vec   (C Vec         &v) {return btVector3(v.x  , v.y  , v.z  );}
   static Matrix3     matrix(C btMatrix3x3 &t);
   static Matrix      matrix(C btTransform &t);
   static btMatrix3x3 matrix(C Matrix3     &m);
   static btTransform matrix(C Matrix      &m);

	void del   ();
	Bool create();

	btDefaultCollisionConfiguration *collision_config;
	btCollisionDispatcher           *dispatcher;
	btBroadphaseInterface           *broadphase;
	btConstraintSolver              *solver;
   btDynamicsWorld                 *world;
   Memc<btTypedConstraint*>         breakables;

   BulletClass() {collision_config=null; dispatcher=null; broadphase=null; solver=null; world=null;}
   NO_COPY_CONSTRUCTOR(BulletClass);
}extern
   Bullet;

void DrawConvex(btConvexHullShape      &convex, C Color &color=WHITE, Bool fill=false);
void DrawMesh  (btBvhTriangleMeshShape &mesh  , C Color &color=WHITE, Bool fill=false);
#endif
void DrawPhys(C MeshBase &base, C Color &color=WHITE, Bool fill=false);
#endif
/******************************************************************************/
