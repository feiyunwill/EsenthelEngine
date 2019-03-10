/******************************************************************************/
class SmallMatrix : SmallMatrix3 // Matrix stored using 3xVec (3*3xFlt, 3*3*4xByte, 36 bytes)
{
   Vec pos;

   SmallMatrix3& orn(); 
 C SmallMatrix3& orn()C;

   bool operator==(C SmallMatrix &m)C;
   bool operator!=(C SmallMatrix &m)C;

   void set(Matrix &matrix)C;

   Matrix operator()()C;

   SmallMatrix();  
   SmallMatrix(C Matrix &matrix);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
