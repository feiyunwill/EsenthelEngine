/******************************************************************************/
/******************************************************************************

   Usage of 'rotateToDir' instead of setting 'dir' and calling 'fix' gives better precision for cases when we're rotating the bone by 90 deg.

/******************************************************************************/
class AdjustBoneOrns : PropWin
{
   enum HAND_MODE
   {
      HAND_SKIP,
      HAND_MIDDLE,
      HAND_AVG,
      HAND_PARENT,
   };
   static cchar8 *HandMode[]
;

   enum FOOT_MODE
   {
      FOOT_SKIP,
      FOOT_DEFAULT,
      FOOT_DOWN,
      FOOT_PARENT,
   };
   static cchar8 *FootMode[]
;

   enum ROT_SHOULDER_MODE
   {
      ROT_SHOULDER_NO,
      ROT_SHOULDER_SRC,
      ROT_SHOULDER_DEST,
   };
   static cchar8 *RotShoulder[]
;

   bool       zero_child,
               one_child,
             multi_child,
             force_eye_forward,
             force_nose_forward,
             force_spine_up,
             force_neck_up,
             force_head_up,
             force_toe_forward,
             add_shoulder,
             reset_perp,
             refresh_needed;
   HAND_MODE hand_mode;
   FOOT_MODE foot_mode;
   ROT_SHOULDER_MODE rotate_shoulder;
   Button    adjust;
   Skeleton  skel;

   static void Changed(C Property &prop);  
   static void Adjust(AdjustBoneOrns &abo);

   static bool SetTarget(SkelBone &bone, C Vec &pos);
   static flt BoneMeshLength(C SkelBone &bone);
   static Str8 UniqueName(C Skeleton &skel, C Str8 &name);
   void setBoneLength(Skeleton &skel, SkelBone &bone, flt min_length); // set length as Max of all children
   bool adjustDo(Skeleton &skel, Mesh *mesh=null); // return if mesh was changed

   Skeleton& getSkel();
   void refresh();          
   AdjustBoneOrns& create();

public:
   AdjustBoneOrns();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
