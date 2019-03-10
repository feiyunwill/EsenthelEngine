/******************************************************************************

   Use 'Chr' as a base class for creating your own character class.
   'Chr' already implements the following functionality:
      -moving according to input
      -moving according to target position
      -moving according to direction
      -automatic skeleton animating according to the default animations (set in 'Chr::sac')
      -ragdoll support
      -picking up and dropping down items
      -grabbing actors

/******************************************************************************/
namespace Game{
/******************************************************************************/
enum ACTION_TYPE // Character Action Type
{
   ACTION_NONE    , // none (update movement according to input)
   ACTION_MOVE_TO , // move to position
   ACTION_MOVE_DIR, // move to direction
};
/******************************************************************************/
STRUCT(Chr , Obj) // Game Character Object
//{
   struct Input // Movement Input
   {
      Bool  crouch, // if want to crouch
            walk  ; // if want to walk
      Flt   jump  ; // jump     velocity  ( 0..Inf)
      Int   dodge ; // dodge    direction (-1..1)
      VecI  move  ; // movement direction (-1..1)
      VecI2 turn  ; // turn     angle     (-1..1)

      Bool  adjust_move, // if adjust movement from 'Input.move' when in ACTION_NONE, default=true
            adjust_turn; // if adjust turning  from 'Input.turn' when in ACTION_NONE, default=true

      Input() {crouch=walk=false; jump=0; dodge=0; move.zero(); turn.zero(); adjust_move=adjust_turn=true;}
   };

   ACTION_TYPE action      ; // action type, default=ACTION_NONE
   Bool        move_walking; // if move through path by walking instead of running, default=false
   Flt         speed       , // move   speed, default= 4.1, for changing animations speed please check Chr::Animation::speed
               turn_speed  , // turn   speed, default= 3.5
               flying_speed; // flying speed, default=25.0
   Vec2        angle       ; // look   angles
   Input       input       ; // input (not saved inside 'save' method)

   Controller       ctrl          ; // controller
   AnimatedSkeleton skel          ; // animated skeleton
   ObjectPtr        base          ; // base object
   MeshPtr          mesh          ; // mesh
   Int              mesh_variation; // mesh variation index
   Ragdoll          ragdoll       ; // ragdoll, before manual operations on 'ragdoll' you must call at least once 'ragdollValidate' to make sure the 'ragdoll' is properly initialized

   // manage
   virtual void create(Object &obj); // create from object, Chr class accepts only 'PhysBody' made from 1 Capsule (ignoring its "up" vector), if no PhysBody is given then the character controller will be created basing on the object scale

   virtual void setUnsavedParams(); // set parameters which are not included in the save file

   // get / set
   virtual Vec    pos   (                );                   // get position
   virtual void   pos   (C Vec    &pos   );                   // set position
   virtual Matrix matrix(                );                   // get matrix
   virtual void   matrix(C Matrix &matrix) {pos(matrix.pos);} // set matrix, only 'matrix.pos' is used in this method

   // actions
   void actionBreak  (          ); // break active action
   Bool actionMoveTo (C Vec &pos); // set action to move to position, false on fail (if path not found)
   void actionMoveDir(C Vec &dir); // set action to move to direction

   // animation
   virtual Flt  animateFaceBlend   ();   // facial animation blending value, dependent on character visibility and distance from camera, returns value    0..1    (0     disables facial animation), you can override this method and return your custom values
   virtual Bool animateFingers     ();   // if animate hand/foot fingers   , dependent on character visibility and distance from camera, returns value true/false (false disables hands  animation), you can override this method and return your custom values
   virtual Flt  animateAlignFeet   ();   // how much to align feet on to the ground                                                    , returns value    0..1    (0     disables feet   aligning ), you can override this method and return your custom values
   virtual void animateStepOccurred() {} // this method is called when "step" event has occurred in the most significant movement animation, you can override this method and for example play "step" sound

   virtual void animate(); // this method animates the character skeleton using default animations, it consists only of following : 'skel.clear().animate(stand).animate(walk)...', you can override this method and use it in 2 ways:
                           // 1) use default animations and modify them - "super::animate(); skel.animate(custom animation)..."
                           // 2) use only custom animations             - "skel  .clear  (); skel.animate(custom animation)..."

   virtual void animateUpdateMatrix(C Matrix &body_matrix) {skel.updateMatrix(body_matrix);} // this method is called when the skeleton is fully animated and is ready to have its matrixes updated, you can override this method and optionally modify the skeleton before or after calling the default 'animateUpdateMatrix' method

   // ragdoll
   virtual void ragdollValidate(); // call this at least once before manual operations on 'ragdoll' to make sure it has been properly initialized
   virtual void ragdollEnable  (); // switch from skeleton animations to ragdoll  animations (RAGDOLL_FULL)
   virtual void ragdollDisable (); // switch from ragdoll  animations to skeleton animations (RAGDOLL_NONE)
   virtual Bool ragdollBlend   (); // enable ragdoll blending for shot/hit simulation        (RAGDOLL_PART), if current 'ragdoll_mode' is set to RAGDOLL_FULL then the call will fail and false will be returned

   // callbacks
   virtual void memoryAddressChanged(); // called when object memory address has been changed, you should override it and adjust Actor::obj pointer for all actors

   // grabbing
   void   grabStart   (Item &item, C Vec &local_pos=VecZero, Flt power=5); // start grabbing 'item' at its 'local_pos' position with 'power'
   void   grabStop    (          );                                 // stop grabbing
   Bool   grabIs      (          ) {return grab.is          (   );} // if   grabbing something
   Actor* grabbedActor(          ) {return grab.grabbedActor(   );} // get  grabbed actor
   Vec    grabPos     (          ) {return grab.pos         (   );} // get  world position of grabber
   void   grabPos     (C Vec &pos) {       grab.pos         (pos);} // move to 'pos' world position

   // update
   virtual Bool update(); // update, return false when object wants to be deleted, and true if wants to exist

   // draw
   virtual UInt drawPrepare(); // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh::draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)
   virtual void drawShadow (); // in this method you should add the meshes to the shadow draw list (Mesh::drawShadow)

   // enable / disable
   virtual void disable(); // disable object when becomes too far away, here all dynamic actors should  enable kinematic flag - actor.kinematic(true ), this is called when an object changes its state to AREA_INACTIVE
   virtual void  enable(); //  enable object when closes in           , here all dynamic actors should disable kinematic flag - actor.kinematic(false), this is called when an object changes its state to AREA_ACTIVE

   // io
   virtual Bool save(File &f); // save, false on fail
   virtual Bool load(File &f); // load, false on fail

  ~Chr();
   Chr();

public: // following members/methods are public, however don't modify/call them manually unless you know what you're doing
   Vec  input_move;
   Vec2 input_turn;

   Int dodging;
   Flt dodge_step,
       dodge_availability;

   Vec2      move_dir;
   Vec       path_target;
   Memc<Vec> path;

   enum RAGDOLL_MODE
   {
      RAGDOLL_NONE, // animations fully controlled by skeleton animations                        (alive character                         )
      RAGDOLL_PART, // animations       controlled by skeleton animations + small ragdoll blends (alive character with simulated shot hits)
      RAGDOLL_FULL, // animations fully controlled by                             ragdoll        (dead  character                         )
   };
   RAGDOLL_MODE ragdoll_mode;
   Flt          ragdoll_time;

   struct Animation
   {
      Flt b_turn_l, t_turn_l, // blend and time value of turning left  animation
          b_turn_r, t_turn_r; // blend and time value of turning right animation

      Flt stand_crouch, // stand    0..1 crouch blend value
           stop_move  , // stop     0..1 move   blend value
           walk_run   , // walk     0..1 run    blend value
        forward_back  , // forward  0..1 back   blend value
           left_right , // left     0..1 right  blend value
       straight_strafe; // straight 0..1 strafe blend value

      Flt strafe_yaw, // -1..1 value
          lean      ; // -1..1 lean value

      Flt time  , // 0..Inf, default=0         , character movement animation time
          speed , // 0..Inf, default=0.68      , character movement animation scale factor, this affects only animation speed, you can change it when setting different 'Chr::speed' values so the animations will be played proportionally to movement speed, suggested formula : Chr::anim.speed=Chr::speed/6
          unique; // 0..1  , default=Random.f(), character unique value used for animation offsets

      // flying values
      Flt fly,
          fly_x, fly_z,
          fly_strafe, fly_turn,
          fly_full_body;
   }anim;

   struct SkelAnimCache // Skeleton Animation Cache, these are the pointers to character default animations, you can replace them with custom ones, for example 'walk=skel.getSkelAnim("custom walk.anim")'
   {
      SkelAnim
         *fist_l   , // left  fist
         *fist_r   , // right fist
         *stand    , // stand  still
         *crouch   , // crouch still
         *turn_l   , // turn left
         *turn_r   , // turn right
         *walk     , // walk forward
         *run      , // run  forward
         *crouch_f , // crouch and move forward
         *crouch_l , // crouch and move left
         *crouch_r , // crouch and move right
         *strafe_wl, // strafe left  walking
         *strafe_wr, // strafe right walking
         *strafe_l , // strafe left  running
         *strafe_r , // strafe right running
         *dodge_l  , // dodge  left
         *dodge_r  , // dodge  right
         *floating ; // float  still
      Int head, neck, body, body_u, arm_ru, arm_lu, arm_rd, arm_ld, leg_lu, leg_ru, leg_ld, leg_rd, hand_l, hand_r, toe_l, toe_r; // indexes of skeleton bones found in 'skel', -1=none

      void set(AnimatedSkeleton &anim_skel, C ObjectPtr &obj); // set default values using 'skel' skeleton
   }sac; // not saved in SaveGame

   Grab grab;

   Flt desiredSpeed();

   void updateAction       ();
   void updateAnimationAnim(); // here all members from 'Chr::anim' are updated according to input
   void updateAnimation    ();
   void updateController   ();
};
/******************************************************************************/
} // namespace
/******************************************************************************/
