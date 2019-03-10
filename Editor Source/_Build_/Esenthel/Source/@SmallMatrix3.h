/******************************************************************************/
/******************************************************************************/
/*class Matrix3_8 // Matrix3 stored using 8 bytes
{
   ushort orn    , // orientation                  (2 bytes total)
          size[3]; // size encoded to 16-bit float (6 bytes total)

   Vec  scale(               )C {return Vec(F16To32(size[0]), F16To32(size[1]), F16To32(size[2]));}
   void set  (Matrix3 &matrix)C {File f; f.readMem(&orn, SIZE(orn)).decOrient2(matrix); matrix.scaleL(scale());}
   void zero (               )  {Zero(T);}

   bool operator==(C Matrix3_8 &matrix)C {return orn==matrix.orn && size[0]==matrix.size[0] && size[1]==matrix.size[1] && size[2]==matrix.size[2];}
   bool operator!=(C Matrix3_8 &matrix)C {return orn!=matrix.orn || size[0]!=matrix.size[0] || size[1]!=matrix.size[1] || size[2]!=matrix.size[2];}

   Matrix3_8() {}
   Matrix3_8(C Matrix3 &matrix)
   {
      size[0]=F32To16(Mid(matrix.x.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      size[1]=F32To16(Mid(matrix.y.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      size[2]=F32To16(Mid(matrix.z.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      File f; f.writeMem().cmpOrient2(matrix).pos(0); f>>orn;
   }
}*/
class SmallMatrix3 // Matrix3 stored using 2xVec (2*3xFlt, 2*3*4xByte, 24 bytes)
{
   Vec angle,
       scale;

   bool operator==(C SmallMatrix3 &m)C;
   bool operator!=(C SmallMatrix3 &m)C;

   void set(Matrix3 &matrix)C;

   SmallMatrix3();  
   SmallMatrix3(C Matrix3 &matrix);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
