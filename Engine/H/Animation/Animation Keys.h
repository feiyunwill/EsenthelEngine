/******************************************************************************

   Use 'AnimationKeys' to store animation keys:
      -target   orientation
      -relative rotation
      -offset   position
      -scale

/******************************************************************************/
#if EE_PRIVATE

#define HAS_ANIM_TANGENT 0
#define HAS_ANIM_ROT     0
#define HAS_ANIM_COLOR   0

struct AnimParams
{
   Bool loop  ,
        linear;
   Flt  length,
        time  ;

   void set(Bool loop, Bool linear, Flt length, Flt time)
   {
      T.loop  =loop;
      T.linear=linear;
      T.length=length;
      T.time  =(loop ? Frac(time, length) : time);
   }
   void set(C Animation &animation, Flt time);

   AnimParams() {}
   AnimParams(Bool loop, Bool linear, Flt length, Flt time) {set(loop, linear, length, time);}
   AnimParams(C Animation &animation, Flt time            ) {set(animation           , time);}
};
#endif
typedef AnimationKeys AnimKeys;
struct  AnimationKeys // Animation Keys - set of animation keyframes for a single bone
{
   struct Key
   {
      Flt time; // time position (seconds)
   };
   struct Orn : Key // Target Orientation
   {
      Orient orn; // orientation

   #if EE_PRIVATE
      void save(MemPtr<TextNode> nodes)C; // save text
   #endif
   };
   struct Pos : Key // Offset Position
   {
      Vec pos; // position offset

   #if EE_PRIVATE
      void save(MemPtr<TextNode> nodes)C; // save text
   #endif
   };
   struct Scale : Key // Scale
   {
      Vec scale; // scale factor

   #if EE_PRIVATE
      void save(MemPtr<TextNode> nodes)C; // save text
   #endif
   };

   Mems<Orn  > orns;
   Mems<Pos  > poss;
   Mems<Scale> scales;

   // get
   Bool is()C {return orns.elms() || poss.elms() || scales.elms();} // if has any keys

   // transform
   AnimKeys& mirrorX(); // mirror keyframes in x axis

   // operations
   AnimKeys& scale      (Flt  scale                                                                                                                                                                              ); // scale position offset key values by 'scale'
   AnimKeys& scaleTime  (Flt  scale,                       Flt anim_length                                                                                                                                       ); // scale time positions of keyframes "each frame time *= scale", 'anim_length'=new animation length (after scale), it's used for clipping time values
   AnimKeys& slideTime  (Flt  dt,                          Flt anim_length                                                                                                                                       ); // slide time positions of keyframes, 'anim_length'=animation length
   AnimKeys& reverse    (                                  Flt anim_length                                                                                                                                       ); // reverse animation, 'anim_length'=animation length
   AnimKeys& sortFrames (                                                                                                                                                                                        ); // sort frames in time order, this should be called after manually modifying the keyframes and changing their time positions
   AnimKeys& setTangents(Bool anim_loop,                   Flt anim_length                                                                                                                                       ); // recalculate tangents, 'anim_loop'=if animation is looped, 'anim_length'=animation length
   AnimKeys& optimize   (Bool anim_loop, Bool anim_linear, Flt anim_length, Flt angle_eps=EPS_ANIM_ANGLE, Flt pos_eps=EPS_ANIM_POS, Flt scale_eps=EPS_ANIM_SCALE, C Orient *bone=null, C Orient *bone_parent=null); // optimize animation by removing similar keyframes, 'angle_eps'=angular epsilon 0..PI, 'pos_eps'=position epsilon 0..Inf, 'scale_eps'=scale epsilon 0..Inf, 'color_eps'=color epsilon 0..1, 'bone'=un-transformed orientation of skeleton bone (if specified then it will be used to check if orientation keyframe can be removed if there's only one), 'bone_parent'=un-transformed orientation of skeleton bone parent, 'anim_loop'=if animation is looped, 'anim_linear'=if animation is linear, 'anim_length'=animation length
   AnimKeys& clip       (Bool anim_loop, Bool anim_linear, Flt anim_length, Flt start_time, Flt end_time                                                                                                         ); // clip animation to 'start_time' .. 'end_time', this will remove all keyframes which aren't located in selected range, 'anim_loop'=if animation is looped, 'anim_linear'=if animation is linear, 'anim_length'=animation length

   void includeTimes(MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times)C;
#if EE_PRIVATE
   // get
   Bool timeRange(Flt &min, Flt &max)C; // get min/max time value out of all keyframes, false on fail (if there are no keyframes)

   Bool orn  (Orient   &orn  , C AnimParams &params)C; // get  orientation at specified time, false on fail (if there are no keyframes)
   Bool rot  (AxisRoll &rot  , C AnimParams &params)C; // get     rotation at specified time, false on fail (if there are no keyframes)
   Bool pos  (Vec      &pos  , C AnimParams &params)C; // get     position at specified time, false on fail (if there are no keyframes)
   Bool scale(Vec      &scale, C AnimParams &params)C; // get scale factor at specified time, false on fail (if there are no keyframes)
   Bool color(Vec4     &color, C AnimParams &params)C; // get        color at specified time, false on fail (if there are no keyframes)

   // transform
   AnimKeys& transform(C Matrix3 &matrix); // transform keyframes by 'matrix', 'matrix' must be normalized

   // convert
   AnimKeys& convertRotToOrn(C Skeleton &skeleton, Int skel_bone_index, Bool looped, Flt anim_length); // convert relative  rotations to target orientations according to given 'skeleton', 'skel_bone_index'=index of skeleton bone to which this keys belong to (-1=root bone), 'looped'=if animation is looped
   AnimKeys& convertOrnToRot(C Skeleton &skeleton, Int skel_bone_index, Bool looped, Flt anim_length); // convert target orientations to relative  rotations according to given 'skeleton', 'skel_bone_index'=index of skeleton bone to which this keys belong to (-1=root bone), 'looped'=if animation is looped

   // io
   Bool saveData (File &f)C;
   Bool loadData (File &f) ;
   void loadData3(File &f) ;
   void loadData2(File &f) ;
   void loadData1(File &f) ;
   void loadData0(File &f) ;

   void save(MemPtr<TextNode> nodes)C; // save text
#endif

   AnimKeys& del(); // delete manually
};
/******************************************************************************/
