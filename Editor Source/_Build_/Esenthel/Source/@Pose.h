/******************************************************************************/
/******************************************************************************/
class Pose
{
   flt scale;
   Vec pos, rot;

   Matrix operator()()C;

   Str asText()C;

   Pose& reset();

   Pose& operator+=(C Vec    &offset   );
   Pose& operator*=(C Pose   &transform);
   Pose& operator*=(C Matrix &matrix   );

   Pose(C Matrix &matrix);

   bool operator==(C Pose &t)C;
   bool operator!=(C Pose &t)C;

public:
   Pose();
};
/******************************************************************************/
/******************************************************************************/
extern const Pose PoseIdentity;
/******************************************************************************/
