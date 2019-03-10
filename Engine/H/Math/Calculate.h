/******************************************************************************

   Use 'Calculate' functions to calculate string expressions and convert them to values.

   Sample Usage:

      Flt result=CalculateF("2*3");

/******************************************************************************/
enum CVAL_TYPE : Byte // Calculator Value Type
{
   CVAL_NONE, // none
   CVAL_INT , // integer
   CVAL_REAL, // real
   CVAL_VEC2, // vector 2D
   CVAL_VEC , // vector 3D
   CVAL_VEC4, // vector 4D
};
/******************************************************************************/
struct CalcValue // Calculator Value
{
   CVAL_TYPE type; // value type
   union
   {
      Long  i ; // CVAL_INT
      Dbl   r ; // CVAL_REAL
      VecD2 v2; // CVAL_VEC2
      VecD  v ; // CVAL_VEC
      VecD4 v4; // CVAL_VEC4
   };

   // get
   CChar8* typeName(                     )C; // get CVAL_TYPE type name in text format
   Str     asText  (Int precision=INT_MAX)C; // get value as text
   Int     asInt   (                     )C; // get value as Int  , 0 on fail
   UInt    asUInt  (                     )C; // get value as UInt , 0 on fail
   Long    asLong  (                     )C; // get value as Long , 0 on fail
   ULong   asULong (                     )C; // get value as ULong, 0 on fail
   Flt     asFlt   (                     )C; // get value as Flt  , 0 on fail
   Dbl     asDbl   (                     )C; // get value as Dbl  , 0 on fail
   Vec2    asVec2  (                     )C; // get value as Vec2 , (0,0) on fail
   VecD2   asVecD2 (                     )C; // get value as VecD2, (0,0) on fail
   VecI2   asVecI2 (                     )C; // get value as VecI2, (0,0) on fail
   Vec     asVec   (                     )C; // get value as Vec  , (0,0,0) on fail
   VecD    asVecD  (                     )C; // get value as VecD , (0,0,0) on fail
   VecI    asVecI  (                     )C; // get value as VecI , (0,0,0) on fail
   Vec4    asVec4  (                     )C; // get value as Vec4 , (0,0,0,0) on fail
   VecD4   asVecD4 (                     )C; // get value as VecD4, (0,0,0,0) on fail
   VecI4   asVecI4 (                     )C; // get value as VecI4, (0,0,0,0) on fail

#if EE_PRIVATE
   // operations, false on fail
   Bool notLogical(); // !
   Bool notBitwise(); // ~
   Bool chs       (); // change sign
   Bool abs       (); // absolute value
   Bool sign      (); // sign

   Bool sat(              ); // saturate
   Bool min(C CalcValue &x); // minimum
   Bool max(C CalcValue &x); // maximum
   Bool avg(C CalcValue &x); // average

   Bool add(C CalcValue &x); // add
   Bool sub(C CalcValue &x); // subtract
   Bool mul(C CalcValue &x); // multiply
   Bool div(C CalcValue &x); // divide
   Bool mod(C CalcValue &x); // modulo

   Bool sqr (              ); // square
   Bool cube(              ); // cube
   Bool sqrt(              ); // square root
   Bool cbrt(              ); // cube   root
   Bool pow (C CalcValue &x); // power
   Bool ln  (              ); // e-base logarithm
   Bool log2(              ); // 2-base logarithm
   Bool log (C CalcValue &x); // x-base logarithm

   Bool And(C CalcValue &x); // &
   Bool Or (C CalcValue &x); // |
   Bool Xor(C CalcValue &x); // ^
   Bool shl(C CalcValue &x); // <<
   Bool shr(C CalcValue &x); // >>
   Bool rol(C CalcValue &x); // rotate left
   Bool ror(C CalcValue &x); // rotate right

   Bool    equal    (C CalcValue &x); // ==
   Bool notEqual    (C CalcValue &x); // !=
   Bool equalGreater(C CalcValue &x); // >=
   Bool equalSmaller(C CalcValue &x); // <=
   Bool      greater(C CalcValue &x); // >
   Bool      smaller(C CalcValue &x); // <

   Bool andLogical(C CalcValue &x); // &&
   Bool  orLogical(C CalcValue &x); // ||
   Bool xorLogical(C CalcValue &x); // ^^

   Bool trunc(); // truncate
   Bool round(); // round
   Bool floor(); // floor
   Bool ceil (); // ceil
   Bool frac (); // fraction

   Bool alignTrunc(C CalcValue &x); // align to 'x' with truncation
   Bool alignRound(C CalcValue &x); // align to 'x' with rounding
   Bool alignFloor(C CalcValue &x); // align to 'x' with floor
   Bool alignCeil (C CalcValue &x); // align to 'x' with ceil

   Bool      sin(); //   sine
   Bool      cos(); // cosine
   Bool      tan(); //   tangent
   Bool      ctg(); // cotangent
   Bool     acos(); // arc cosine
   Bool     asin(); // arc   sine
   Bool     atan(); // arc tangent
   Bool degToRad(); // convert angle degrees to radians
   Bool radToDeg(); // convert angle radians to degrees
   
   Bool length (              ); //         length
   Bool length2(              ); // squared length
   Bool dot    (C CalcValue &x); //   dot product
   Bool cross  (C CalcValue &x); // cross product
   Bool perp   (              ); // perpendicular
#endif

   CalcValue() {} // needed because of union
};
/******************************************************************************/
struct CalcVar // Calculator Variable
{
   Str       name ;
   CalcValue value;

   CalcVar& set(C Str &name, CVAL_TYPE type=CVAL_REAL) {T.name=name; T.value.type=type; return T;}

            CalcVar(                                     ) {value.type=CVAL_REAL;}
   explicit CalcVar(C Str &name, CVAL_TYPE type=CVAL_REAL) {set(name, type);}
};
/******************************************************************************/
extern Char CalcError[]; // contains error on failed Calc()
/******************************************************************************/
// Calculate 'expression', with 'var' array of 'vars' variables
Bool  Calculate  (CalcValue &value, C Str &expression, C MemPtr<CalcVar> &vars=null); // put    result value to 'value' and return if ok
Int   CalculateI (                  C Str &expression, C MemPtr<CalcVar> &vars=null); // return result value as Int
Long  CalculateL (                  C Str &expression, C MemPtr<CalcVar> &vars=null); // return result value as Long
Flt   CalculateF (                  C Str &expression, C MemPtr<CalcVar> &vars=null); // return result value as Flt
Dbl   CalculateD (                  C Str &expression, C MemPtr<CalcVar> &vars=null); // return result value as Dbl
VecD2 CalculateV2(                  C Str &expression, C MemPtr<CalcVar> &vars=null); // return result value as Vec2
VecD  CalculateV (                  C Str &expression, C MemPtr<CalcVar> &vars=null); // return result value as Vec
VecD4 CalculateV4(                  C Str &expression, C MemPtr<CalcVar> &vars=null); // return result value as Vec4
/******************************************************************************/
