/******************************************************************************

   Use 'Vec2' to handle 2D vectors, Flt type
   Use 'Vec'  to handle 3D vectors, Flt type
   Use 'Vec4' to handle 4D vectors, Flt type

   Use 'VecD2' to handle 2D vectors, Dbl type
   Use 'VecD'  to handle 3D vectors, Dbl type
   Use 'VecD4' to handle 4D vectors, Dbl type

   Use 'VecI2' to handle 2D vectors, Int type
   Use 'VecI'  to handle 3D vectors, Int type
   Use 'VecI4' to handle 4D vectors, Int type

   Use 'VecB2' to handle 2D vectors, Byte type
   Use 'VecB'  to handle 3D vectors, Byte type
   Use 'VecB4' to handle 4D vectors, Byte type

   Use 'VecSB2' to handle 2D vectors, SByte type
   Use 'VecSB'  to handle 3D vectors, SByte type
   Use 'VecSB4' to handle 4D vectors, SByte type

/******************************************************************************/
#define SIGN_BIT 0x80000000 // Float Sign Bit

#define PI_6 0.5235987755982988f // PI/6 ( 30 deg) Flt
#define PI_4 0.7853981633974483f // PI/4 ( 45 deg) Flt
#define PI_3 1.0471975511965977f // PI/3 ( 60 deg) Flt
#define PI_2 1.5707963267948966f // PI/2 ( 90 deg) Flt
#define PI   3.1415926535897932f // PI   (180 deg) Flt
#define PI2  6.2831853071795864f // PI*2 (360 deg) Flt

#define PID_6 0.5235987755982988 // PI/6 ( 30 deg) Dbl
#define PID_4 0.7853981633974483 // PI/4 ( 45 deg) Dbl
#define PID_3 1.0471975511965977 // PI/3 ( 60 deg) Dbl
#define PID_2 1.5707963267948966 // PI/2 ( 90 deg) Dbl
#define PID   3.1415926535897932 // PI   (180 deg) Dbl
#define PID2  6.2831853071795864 // PI*2 (360 deg) Dbl

#define SQRT2   1.4142135623730950 // Sqrt(2)
#define SQRT3   1.7320508075688773 // Sqrt(3)
#define SQRT2_2 0.7071067811865475 // Sqrt(2)/2
#define SQRT3_2 0.8660254037844386 // Sqrt(3)/2
#define SQRT3_3 0.5773502691896257 // Sqrt(3)/3

#define HEX_SIZE SQRT3_2 // distance between hexagonal cells along the smaller axis

#define EPSD 0.0000000037252902984619141 // Dbl           Epsilon (difference between 0x1000000 (16777216) and the nearest bigger 64-bit float value)
#define EPS  0.000030517578125f          // Flt           Epsilon (difference between 0x0000100 (     256) and the nearest bigger 32-bit float value)
#define EPSL 0.0078125f                  // Low Precision Epsilon (difference between 0x0010000 (   65536) and the nearest bigger 32-bit float value)

#define EPS_MATERIAL_BUMP (1.0f/8192)       // Material  Bump     Epsilon (default value used for ignoring Material.bump effect)
#define EPS_ANIM_POS      0.0002f           // Animation Position Epsilon (default value used for optimizing animation positions)
#define EPS_ANIM_SCALE    0.0002f           // Animation Scale    Epsilon (default value used for optimizing animation scales)
#define EPS_ANIM_ANGLE    0.001f            // Animation Angle    Epsilon (default value used for optimizing animation rotation angles)
#define EPS_COL8          (0.5f/ 256)       // Color              Epsilon (this gives  8-bit color precision)
#define EPS_COL           (0.5f/1024)       // Color              Epsilon (this gives 10-bit color precision)
#define EPS_COL8_COS      (1-EPS_COL8)      // Color Cos          Epsilon (Dot product of 2 directional vectors giving  8-bit color precision)
#define EPS_COL_COS       (1-EPS_COL)       // Color Cos          Epsilon (Dot product of 2 directional vectors giving 10-bit color precision)
#define EPS_COS           0.9999995f        // Cos Flt            Epsilon (Smallest Dot product of the same normalized directional Vec  vector)
#define EPSD_COS          0.999999999999999 // Cos Dbl            Epsilon (Smallest Dot product of the same normalized directional VecD vector)

#if EE_PRIVATE
#define EPS_GPU                (MOBILE ? HALF_EPS : FLT_EPS) // GPU             Epsilon (Mobile GPU's may have only half precision)
#define EPS_ANIM_BLEND         (1.0f/4096)                   // Animation Blend Epsilon (default value used for ignoring animations)
#define EPS_SKY_MIN_LERP_DIST  (1.0f/8)                      // 12.5 cm
#define EPS_SKY_MIN_VIEW_RANGE 0.999f                        // 0.999f was the biggest value that caused holes to disappear
#define EPS_TAN_COS            SQRT2_2                       // Tangent Cos Epsilon = Cos(PI_4) 45 deg, used for Dot product of 2 directional vectors
#endif
/******************************************************************************/
enum AXIS_TYPE
{
   AXIS_X,
   AXIS_Y,
   AXIS_Z,
};
enum DIR_ENUM : Byte // Direction Enums
{
   DIR_RIGHT  , // right
   DIR_LEFT   , // left
   DIR_UP     , // up
   DIR_DOWN   , // down
   DIR_FORWARD, // forward
   DIR_BACK   , // backwards
   DIR_NUM    , // number of directions
};
enum DIR_FLAG // Direction Flags
{
   DIRF_RIGHT  =1<<DIR_RIGHT  , // right
   DIRF_LEFT   =1<<DIR_LEFT   , // left
   DIRF_UP     =1<<DIR_UP     , // up
   DIRF_DOWN   =1<<DIR_DOWN   , // down
   DIRF_FORWARD=1<<DIR_FORWARD, // forward
   DIRF_BACK   =1<<DIR_BACK   , // backwards

   DIRF_X=DIRF_RIGHT  |DIRF_LEFT,
   DIRF_Y=DIRF_UP     |DIRF_DOWN,
   DIRF_Z=DIRF_FORWARD|DIRF_BACK,

   DIRF_XYZ=DIRF_X|DIRF_Y|DIRF_Z,
   DIRF_ALL=DIRF_XYZ,
};
/******************************************************************************/
// MINIMUM MAXIMUM AVERAGE
/******************************************************************************/
// get minimum/maximum value
constexpr Int   Min(Int   x, Int   y) {return (x<y) ? x : y;}
constexpr Int   Min(UInt  x, Int   y) {return (Long(x)<y) ? x : y;}
constexpr Int   Min(Int   x, UInt  y) {return (x<Long(y)) ? x : y;}
constexpr UInt  Min(UInt  x, UInt  y) {return (x<y) ? x : y;}
constexpr Long  Min(Int   x, Long  y) {return (x<y) ? x : y;}
constexpr Long  Min(UInt  x, Long  y) {return (x<y) ? x : y;}
constexpr Long  Min(Long  x, Int   y) {return (x<y) ? x : y;}
constexpr Long  Min(Long  x, UInt  y) {return (x<y) ? x : y;}
constexpr Long  Min(Long  x, Long  y) {return (x<y) ? x : y;}
constexpr ULong Min(ULong x, ULong y) {return (x<y) ? x : y;}
constexpr Int   Min(Int   x, ULong y) {return (x<=0) ? x : (Int)Min(ULong(x), y);}
constexpr Int   Min(ULong x, Int   y) {return (y<=0) ? y : (Int)Min(ULong(y), x);}
constexpr Flt   Min(Flt   x, Flt   y) {return (x<y) ? x : y;}
constexpr Flt   Min(Int   x, Flt   y) {return (x<y) ? x : y;}
constexpr Flt   Min(Flt   x, Int   y) {return (x<y) ? x : y;}
constexpr Dbl   Min(Dbl   x, Dbl   y) {return (x<y) ? x : y;}
constexpr Dbl   Min(Int   x, Dbl   y) {return (x<y) ? x : y;}
constexpr Dbl   Min(Dbl   x, Int   y) {return (x<y) ? x : y;}

constexpr Int   Max(Int   x, Int   y) {return (x>y) ? x : y;}
constexpr UInt  Max(UInt  x, Int   y) {return (Long(x)>y) ? x : y;}
constexpr UInt  Max(Int   x, UInt  y) {return (x>Long(y)) ? x : y;}
constexpr UInt  Max(UInt  x, UInt  y) {return (x>y) ? x : y;}
constexpr Long  Max(Int   x, Long  y) {return (x>y) ? x : y;}
constexpr Long  Max(UInt  x, Long  y) {return (x>y) ? x : y;}
constexpr Long  Max(Long  x, Int   y) {return (x>y) ? x : y;}
constexpr Long  Max(Long  x, UInt  y) {return (x>y) ? x : y;}
constexpr Long  Max(Long  x, Long  y) {return (x>y) ? x : y;}
constexpr ULong Max(ULong x, ULong y) {return (x>y) ? x : y;}
constexpr ULong Max(Int   x, ULong y) {return (x<=0) ? y : Max(ULong(x), y);}
constexpr ULong Max(ULong x, Int   y) {return (y<=0) ? x : Max(ULong(y), x);}
constexpr Flt   Max(Flt   x, Flt   y) {return (x>y) ? x : y;}
constexpr Flt   Max(Int   x, Flt   y) {return (x>y) ? x : y;}
constexpr Flt   Max(Flt   x, Int   y) {return (x>y) ? x : y;}
constexpr Dbl   Max(Dbl   x, Dbl   y) {return (x>y) ? x : y;}
constexpr Dbl   Max(Int   x, Dbl   y) {return (x>y) ? x : y;}
constexpr Dbl   Max(Dbl   x, Int   y) {return (x>y) ? x : y;}

inline Int  Min(Int  x, Int  y, Int  z        ) {return Min(x, Min(y, z));}
inline Long Min(Long x, Long y, Long z        ) {return Min(x, Min(y, z));}
inline Flt  Min(Flt  x, Flt  y, Flt  z        ) {return Min(x, Min(y, z));}
inline Dbl  Min(Dbl  x, Dbl  y, Dbl  z        ) {return Min(x, Min(y, z));}
inline Int  Max(Int  x, Int  y, Int  z        ) {return Max(x, Max(y, z));}
inline Long Max(Long x, Long y, Long z        ) {return Max(x, Max(y, z));}
inline Flt  Max(Flt  x, Flt  y, Flt  z        ) {return Max(x, Max(y, z));}
inline Dbl  Max(Dbl  x, Dbl  y, Dbl  z        ) {return Max(x, Max(y, z));}
inline Int  Min(Int  x, Int  y, Int  z, Int  w) {return Min(x, Min(y, z, w));}
inline Long Min(Long x, Long y, Long z, Long w) {return Min(x, Min(y, z, w));}
inline Flt  Min(Flt  x, Flt  y, Flt  z, Flt  w) {return Min(x, Min(y, z, w));}
inline Dbl  Min(Dbl  x, Dbl  y, Dbl  z, Dbl  w) {return Min(x, Min(y, z, w));}
inline Int  Max(Int  x, Int  y, Int  z, Int  w) {return Max(x, Max(y, z, w));}
inline Long Max(Long x, Long y, Long z, Long w) {return Max(x, Max(y, z, w));}
inline Flt  Max(Flt  x, Flt  y, Flt  z, Flt  w) {return Max(x, Max(y, z, w));}
inline Dbl  Max(Dbl  x, Dbl  y, Dbl  z, Dbl  w) {return Max(x, Max(y, z, w));}

// minimize/maximize value
T2(TA, TB) inline TA& MIN(TA &x, TB y) {if(y<x)x=y; return x;} // equal to: x=Min(x, y)
T2(TA, TB) inline TA& MAX(TA &x, TB y) {if(y>x)x=y; return x;} // equal to: x=Max(x, y)

// get index of minimum/maximum value (if the min/max value is shared between multiple values, then index of the first value is returned)
Int MinI(Int x, Int y);
Int MinI(Flt x, Flt y);
Int MinI(Dbl x, Dbl y);
Int MaxI(Int x, Int y);
Int MaxI(Flt x, Flt y);
Int MaxI(Dbl x, Dbl y);
Int MinI(Int x, Int y, Int z);
Int MinI(Flt x, Flt y, Flt z);
Int MinI(Dbl x, Dbl y, Dbl z);
Int MaxI(Int x, Int y, Int z);
Int MaxI(Flt x, Flt y, Flt z);
Int MaxI(Dbl x, Dbl y, Dbl z);
Int MinI(Int x, Int y, Int z, Int w);
Int MinI(Flt x, Flt y, Flt z, Flt w);
Int MinI(Dbl x, Dbl y, Dbl z, Dbl w);
Int MaxI(Int x, Int y, Int z, Int w);
Int MaxI(Flt x, Flt y, Flt z, Flt w);
Int MaxI(Dbl x, Dbl y, Dbl z, Dbl w);

// get average value
inline Int  AvgI(Int  x, Int  y                ) {return DivRound(x+y    , 2 )      ;}
inline UInt AvgI(UInt x, UInt y                ) {return DivRound(x+y    , 2u)      ;}
inline Flt  AvgF(Int  x, Int  y                ) {return         (x+y        )*0.5f ;}
inline Flt  Avg (Flt  x, Flt  y                ) {return         (x+y        )*0.5f ;}
inline Dbl  Avg (Dbl  x, Dbl  y                ) {return         (x+y        )*0.5  ;}
inline Int  AvgI(Int  x, Int  y, Int  z        ) {return DivRound(x+y+z  , 3 )      ;}
inline Byte AvgI(Byte x, Byte y, Byte z        ) {return DivRound(UInt(x+y+z), 3u)  ;}
inline UInt AvgI(UInt x, UInt y, UInt z        ) {return DivRound(x+y+z  , 3u)      ;}
inline Flt  AvgF(Int  x, Int  y, Int  z        ) {return         (x+y+z      )/3.0f ;}
inline Flt  Avg (Flt  x, Flt  y, Flt  z        ) {return         (x+y+z      )/3.0f ;}
inline Dbl  Avg (Dbl  x, Dbl  y, Dbl  z        ) {return         (x+y+z      )/3.0  ;}
inline Int  AvgI(Int  x, Int  y, Int  z, Int  w) {return DivRound(x+y+z+w, 4 )      ;}
inline UInt AvgI(UInt x, UInt y, UInt z, UInt w) {return DivRound(x+y+z+w, 4u)      ;}
inline Flt  AvgF(Int  x, Int  y, Int  z, Int  w) {return         (x+y+z+w    )*0.25f;}
inline Flt  Avg (Flt  x, Flt  y, Flt  z, Flt  w) {return         (x+y+z+w    )*0.25f;}
inline Dbl  Avg (Dbl  x, Dbl  y, Dbl  z, Dbl  w) {return         (x+y+z+w    )*0.25 ;}
/******************************************************************************/
// TESTING
/******************************************************************************/
// test if any of the parameters isn't zero
inline Bool Any(C Half &x                                 ); // faster version of "x!=0"
inline Bool Any(C Half &x, C Half &y                      ); // faster version of "x!=0 || y!=0"
inline Bool Any(C Half &x, C Half &y, C Half &z           ); // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Half &x, C Half &y, C Half &z, C Half &w); // faster version of "x!=0 || y!=0 || z!=0 || w!=0"

inline Bool Any(C Flt &x                              ); // faster version of "x!=0"
inline Bool Any(C Flt &x, C Flt &y                    ); // faster version of "x!=0 || y!=0"
inline Bool Any(C Flt &x, C Flt &y, C Flt &z          ); // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Flt &x, C Flt &y, C Flt &z, C Flt &w); // faster version of "x!=0 || y!=0 || z!=0 || w!=0"

inline Bool Any(C Dbl &x                              ); // faster version of "x!=0"
inline Bool Any(C Dbl &x, C Dbl &y                    ); // faster version of "x!=0 || y!=0"
inline Bool Any(C Dbl &x, C Dbl &y, C Dbl &z          ); // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Dbl &x, C Dbl &y, C Dbl &z, C Dbl &w); // faster version of "x!=0 || y!=0 || z!=0 || w!=0"
/******************************************************************************/
// 16-BIT FLOAT
/******************************************************************************/
typedef struct Half
{
   U16 data;

   Half() {}
   Half(Flt f); // create from float
   operator Flt()C; // convert to float

   Half& operator+=(Flt f) {T=Flt(T)+f; return T;}
   Half& operator-=(Flt f) {T=Flt(T)-f; return T;}
   Half& operator*=(Flt f) {T=Flt(T)*f; return T;}
   Half& operator/=(Flt f) {T=Flt(T)/f; return T;}

   Bool operator< (Flt  f)C {return Flt(T)< f;}
   Bool operator<=(Flt  f)C {return Flt(T)<=f;}
   Bool operator> (Flt  f)C {return Flt(T)> f;}
   Bool operator>=(Flt  f)C {return Flt(T)>=f;}
   Bool operator==(Flt  f)C {return Flt(T)==f;}
   Bool operator==(Half h)C {return data  ==h.data;}
}F16;
#if EE_PRIVATE
extern const Half HalfZero, HalfOne;
#endif
/******************************************************************************/
// VECTORS
/******************************************************************************/
struct Vec2 // Vector 2D
{
   union
   {
      struct{Flt x, y;};
      struct{Flt c[2];}; // component
   };

   Vec2& zero(            ) {x=y=0;        return T;}
   Vec2& set (Flt r       ) {x=y=r;        return T;}
   Vec2& set (Flt x, Flt y) {T.x=x; T.y=y; return T;}

   Vec2& operator+=(  Flt      r) {x+=  r; y+=  r; return T;}
   Vec2& operator-=(  Flt      r) {x-=  r; y-=  r; return T;}
   Vec2& operator*=(  Flt      r) {x*=  r; y*=  r; return T;}
   Vec2& operator/=(  Flt      r) {x/=  r; y/=  r; return T;}
   Vec2& operator+=(C Vec2    &v) {x+=v.x; y+=v.y; return T;}
   Vec2& operator-=(C Vec2    &v) {x-=v.x; y-=v.y; return T;}
   Vec2& operator*=(C Vec2    &v) {x*=v.x; y*=v.y; return T;}
   Vec2& operator/=(C Vec2    &v) {x/=v.x; y/=v.y; return T;}
   Vec2& operator*=(C Matrix3 &m) {return mul(m);}
   Vec2& operator*=(C Matrix  &m) {return mul(m);}
   Vec2& operator/=(C Matrix3 &m) {return div(m);}
   Vec2& operator/=(C Matrix  &m) {return div(m);}
   Vec2& operator&=(C Rect    &r) ; // intersect
   Bool  operator==(  Flt      r)C;
   Bool  operator!=(  Flt      r)C;
   Bool  operator==(C Vec2    &v)C;
   Bool  operator!=(C Vec2    &v)C;

   friend Vec2 operator+ (C Vec2 &v, Int i) {return Vec2(v.x+i, v.y+i);}
   friend Vec2 operator- (C Vec2 &v, Int i) {return Vec2(v.x-i, v.y-i);}
   friend Vec2 operator* (C Vec2 &v, Int i) {return Vec2(v.x*i, v.y*i);}
   friend Vec2 operator/ (C Vec2 &v, Int i) {return Vec2(v.x/i, v.y/i);}

   friend Vec2 operator+ (C Vec2 &v, Flt r) {return Vec2(v.x+r, v.y+r);}
   friend Vec2 operator- (C Vec2 &v, Flt r) {return Vec2(v.x-r, v.y-r);}
   friend Vec2 operator* (C Vec2 &v, Flt r) {return Vec2(v.x*r, v.y*r);}
   friend Vec2 operator/ (C Vec2 &v, Flt r) {return Vec2(v.x/r, v.y/r);}

   friend Vec2 operator+ (Int i, C Vec2 &v) {return Vec2(i+v.x, i+v.y);}
   friend Vec2 operator- (Int i, C Vec2 &v) {return Vec2(i-v.x, i-v.y);}
   friend Vec2 operator* (Int i, C Vec2 &v) {return Vec2(i*v.x, i*v.y);}
   friend Vec2 operator/ (Int i, C Vec2 &v) {return Vec2(i/v.x, i/v.y);}

   friend Vec2 operator+ (Flt r, C Vec2 &v) {return Vec2(r+v.x, r+v.y);}
   friend Vec2 operator- (Flt r, C Vec2 &v) {return Vec2(r-v.x, r-v.y);}
   friend Vec2 operator* (Flt r, C Vec2 &v) {return Vec2(r*v.x, r*v.y);}
   friend Vec2 operator/ (Flt r, C Vec2 &v) {return Vec2(r/v.x, r/v.y);}

   friend Vec2 operator+ (C Vec2 &a, C Vec2 &b) {return Vec2(a.x+b.x, a.y+b.y);}
   friend Vec2 operator- (C Vec2 &a, C Vec2 &b) {return Vec2(a.x-b.x, a.y-b.y);}
   friend Vec2 operator* (C Vec2 &a, C Vec2 &b) {return Vec2(a.x*b.x, a.y*b.y);}
   friend Vec2 operator/ (C Vec2 &a, C Vec2 &b) {return Vec2(a.x/b.x, a.y/b.y);}

   friend Vec2 operator* (C Vec2 &v, C Matrix3 &m) {return Vec2(v)*=m;}
   friend Vec2 operator* (C Vec2 &v, C Matrix  &m) {return Vec2(v)*=m;}
   friend Vec2 operator/ (C Vec2 &v, C Matrix3 &m) {return Vec2(v)/=m;}
   friend Vec2 operator/ (C Vec2 &v, C Matrix  &m) {return Vec2(v)/=m;}
   friend Vec2 operator& (C Vec2 &v, C Rect    &r) {return Vec2(v)&=r;} // intersection

   friend Vec2 operator- (C Vec2 &v) {return Vec2(-v.x, -v.y);}
   friend Vec2 operator! (C Vec2 &v) {       Vec2 ret=v; ret.normalize(); return ret;} // return normalized vector

   Vec2 yx ()C {return Vec2(y, x);} // return as Vec2(y, x)
   Vec  xy0()C;                     // return as Vec (x, y, 0)
   Vec  x0y()C;                     // return as Vec (x, 0, y)
   Vec _0xy()C;                     // return as Vec (0, x, y)
   Vec _0yx()C;                     // return as Vec (0, y, x)

   Bool      any          (                )C {return Any (x, y);}   // if any component  is  non-zero
   Bool      all          (                )C {return   x &&  y ;}   // if all components are non-zero
   Bool      allZero      (                )C {return  !x && !y ;}   // if all components are     zero
   Int       minI         (                )C {return MinI(x, y);}   // components minimum index
   Int       maxI         (                )C {return MaxI(x, y);}   // components maximum index
   Flt       min          (                )C {return Min (x, y);}   // components minimum
   Flt       max          (                )C {return Max (x, y);}   // components maximum
   Flt       avg          (                )C {return Avg (x, y);}   // components average
   Flt       sum          (                )C {return      x+ y ;}   // components sum
   Flt       mul          (                )C {return      x* y ;}   // components multiplication
   Flt       div          (                )C {return      x/ y ;}   // components division
   Flt       length       (                )C;                       // get               length
   Flt       length2      (                )C {return  x*x + y*y;}   // get       squared length
   Vec2&  satLength       (                );                        // saturate  length (clip it to 0..1)
   Flt    setLength       (Flt     length  );                        // set       length and return previous length
   Vec2& clipLength       (Flt max_length  );                        // clip      length to 0..max_length range
   Flt       normalize    (                );                        // normalize length and return previous length
   Vec2&     mul          (C Matrix3 &m    );                        // transform by matrix
   Vec2&     mul          (C Matrix  &m    );                        // transform by matrix
   Vec2&     div          (C Matrix3 &m    );                        // transform by matrix inverse
   Vec2&     div          (C Matrix  &m    );                        // transform by matrix inverse
   Vec2&     divNormalized(C Matrix3 &m    );                        // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   Vec2&     divNormalized(C Matrix  &m    );                        // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   Vec2&     rotate       (Flt angle       );                        // rotate by angle
   Vec2&     rotateCosSin (Flt cos, Flt sin);                        // rotate by cos and sin of angle
   Vec2&     chs          (                );                        // change sign of all components
   Vec2&     abs          (                );                        // absolute       all components
   Vec2&     sat          (                );                        // saturate       all components
   Vec2&     swap         (                ) {Swap(x, y); return T;} // swap               components

   Str asText(Int precision=INT_MAX)C; // return as text

#if EE_PRIVATE
   Vec2 asIso  ()C {return Vec2(x+y  , y-x  );}
   Vec2 asIso2 ()C {return Vec2(x*2+y, y*2-x);}
   Vec2 asIso_2()C {return Vec2(x+y*2, y-x*2);}
#endif

   // draw
   void draw(C Color &color, Flt r=0.007f)C; // this can be optionally called outside of Render function

   Vec2() {}
   Vec2(Flt r       ) {set(r   );}
   Vec2(Flt x, Flt y) {set(x, y);}
   Vec2(C VecH2  &v);
   Vec2(C VecD2  &v);
   Vec2(C VecI2  &v);
   Vec2(C VecB2  &v);
   Vec2(C VecSB2 &v);
   Vec2(C VecUS2 &v);
};extern const Vec2
   Vec2Zero; // Vec2(0, 0)
/******************************************************************************/
struct VecD2 // Vector 2D (double precision)
{
   union
   {
      struct{Dbl x, y;};
      struct{Dbl c[2];}; // component
   };

   VecD2& zero(            ) {x=y=0;        return T;}
   VecD2& set (Dbl r       ) {x=y=r;        return T;}
   VecD2& set (Dbl x, Dbl y) {T.x=x; T.y=y; return T;}

   VecD2& operator+=(  Dbl       r) {x+=  r; y+=  r; return T;}
   VecD2& operator-=(  Dbl       r) {x-=  r; y-=  r; return T;}
   VecD2& operator*=(  Dbl       r) {x*=  r; y*=  r; return T;}
   VecD2& operator/=(  Dbl       r) {x/=  r; y/=  r; return T;}
   VecD2& operator+=(C VecD2    &v) {x+=v.x; y+=v.y; return T;}
   VecD2& operator-=(C VecD2    &v) {x-=v.x; y-=v.y; return T;}
   VecD2& operator*=(C VecD2    &v) {x*=v.x; y*=v.y; return T;}
   VecD2& operator/=(C VecD2    &v) {x/=v.x; y/=v.y; return T;}
   VecD2& operator*=(C MatrixD3 &m) {return mul(m);}
   VecD2& operator*=(C MatrixD  &m) {return mul(m);}
   VecD2& operator/=(C MatrixD3 &m) {return div(m);}
   VecD2& operator/=(C MatrixD  &m) {return div(m);}
   Bool   operator==(C VecD2    &v)C;
   Bool   operator!=(C VecD2    &v)C;

   friend VecD2 operator+ (C VecD2 &v, Int i) {return VecD2(v.x+i, v.y+i);}
   friend VecD2 operator- (C VecD2 &v, Int i) {return VecD2(v.x-i, v.y-i);}
   friend VecD2 operator* (C VecD2 &v, Int i) {return VecD2(v.x*i, v.y*i);}
   friend VecD2 operator/ (C VecD2 &v, Int i) {return VecD2(v.x/i, v.y/i);}

   friend VecD2 operator+ (C VecD2 &v, Long i) {return VecD2(v.x+i, v.y+i);}
   friend VecD2 operator- (C VecD2 &v, Long i) {return VecD2(v.x-i, v.y-i);}
   friend VecD2 operator* (C VecD2 &v, Long i) {return VecD2(v.x*i, v.y*i);}
   friend VecD2 operator/ (C VecD2 &v, Long i) {return VecD2(v.x/i, v.y/i);}

   friend VecD2 operator+ (C VecD2 &v, Flt r) {return VecD2(v.x+r, v.y+r);}
   friend VecD2 operator- (C VecD2 &v, Flt r) {return VecD2(v.x-r, v.y-r);}
   friend VecD2 operator* (C VecD2 &v, Flt r) {return VecD2(v.x*r, v.y*r);}
   friend VecD2 operator/ (C VecD2 &v, Flt r) {return VecD2(v.x/r, v.y/r);}

   friend VecD2 operator+ (C VecD2 &v, Dbl r) {return VecD2(v.x+r, v.y+r);}
   friend VecD2 operator- (C VecD2 &v, Dbl r) {return VecD2(v.x-r, v.y-r);}
   friend VecD2 operator* (C VecD2 &v, Dbl r) {return VecD2(v.x*r, v.y*r);}
   friend VecD2 operator/ (C VecD2 &v, Dbl r) {return VecD2(v.x/r, v.y/r);}

   friend VecD2 operator+ (Int i, C VecD2 &v) {return VecD2(i+v.x, i+v.y);}
   friend VecD2 operator- (Int i, C VecD2 &v) {return VecD2(i-v.x, i-v.y);}
   friend VecD2 operator* (Int i, C VecD2 &v) {return VecD2(i*v.x, i*v.y);}
   friend VecD2 operator/ (Int i, C VecD2 &v) {return VecD2(i/v.x, i/v.y);}

   friend VecD2 operator+ (Long i, C VecD2 &v) {return VecD2(i+v.x, i+v.y);}
   friend VecD2 operator- (Long i, C VecD2 &v) {return VecD2(i-v.x, i-v.y);}
   friend VecD2 operator* (Long i, C VecD2 &v) {return VecD2(i*v.x, i*v.y);}
   friend VecD2 operator/ (Long i, C VecD2 &v) {return VecD2(i/v.x, i/v.y);}

   friend VecD2 operator+ (Flt r, C VecD2 &v) {return VecD2(r+v.x, r+v.y);}
   friend VecD2 operator- (Flt r, C VecD2 &v) {return VecD2(r-v.x, r-v.y);}
   friend VecD2 operator* (Flt r, C VecD2 &v) {return VecD2(r*v.x, r*v.y);}
   friend VecD2 operator/ (Flt r, C VecD2 &v) {return VecD2(r/v.x, r/v.y);}

   friend VecD2 operator+ (Dbl r, C VecD2 &v) {return VecD2(r+v.x, r+v.y);}
   friend VecD2 operator- (Dbl r, C VecD2 &v) {return VecD2(r-v.x, r-v.y);}
   friend VecD2 operator* (Dbl r, C VecD2 &v) {return VecD2(r*v.x, r*v.y);}
   friend VecD2 operator/ (Dbl r, C VecD2 &v) {return VecD2(r/v.x, r/v.y);}

   friend VecD2 operator+ (C VecD2 &a, C VecD2    &b) {return VecD2(a.x+b.x, a.y+b.y);}
   friend VecD2 operator- (C VecD2 &a, C VecD2    &b) {return VecD2(a.x-b.x, a.y-b.y);}
   friend VecD2 operator* (C VecD2 &a, C VecD2    &b) {return VecD2(a.x*b.x, a.y*b.y);}
   friend VecD2 operator/ (C VecD2 &a, C VecD2    &b) {return VecD2(a.x/b.x, a.y/b.y);}
   friend VecD2 operator* (C VecD2 &v, C MatrixD3 &m) {return VecD2(v)*=m;}
   friend VecD2 operator* (C VecD2 &v, C MatrixD  &m) {return VecD2(v)*=m;}
   friend VecD2 operator/ (C VecD2 &v, C MatrixD3 &m) {return VecD2(v)/=m;}
   friend VecD2 operator/ (C VecD2 &v, C MatrixD  &m) {return VecD2(v)/=m;}

   friend VecD2 operator- (C VecD2 &v) {return VecD2(-v.x, -v.y);}
   friend VecD2 operator! (C VecD2 &v) {       VecD2 ret=v; ret.normalize(); return ret;} // return normalized vector

   VecD2 yx ()C {return VecD2(y, x);} // return as VecD2(y, x)
   VecD  xy0()C;                      // return as VecD (x, y, 0)
   VecD  x0y()C;                      // return as VecD (x, 0, y)
   VecD _0xy()C;                      // return as VecD (0, x, y)
   VecD _0yx()C;                      // return as VecD (0, y, x)

   Bool       any          (              )C {return   x ||  y ;}   // if any component  is  non-zero
   Bool       all          (              )C {return   x &&  y ;}   // if all components are non-zero
   Bool       allZero      (              )C {return  !x && !y ;}   // if all components are     zero
   Int        minI         (              )C {return MinI(x, y);}   // components minimum index
   Int        maxI         (              )C {return MaxI(x, y);}   // components maximum index
   Dbl        min          (              )C {return Min (x, y);}   // components minimum
   Dbl        max          (              )C {return Max (x, y);}   // components maximum
   Dbl        avg          (              )C {return Avg (x, y);}   // components average
   Dbl        sum          (              )C {return      x+ y ;}   // components sum
   Dbl        mul          (              )C {return      x* y ;}   // components multiplication
   Dbl        div          (              )C {return      x/ y ;}   // components division
   Dbl        length       (              )C;                       // get               length
   Dbl        length2      (              )C {return  x*x + y*y;}   // get       squared length
   VecD2&  satLength       (              );                        // saturate  length (clip it to 0..1)
   Dbl     setLength       (Dbl     length);                        // set       length and return previous length
   VecD2& clipLength       (Dbl max_length);                        // clip      length to 0..max_length range
   Dbl        normalize    (              );                        // normalize length and return previous length
   VecD2&     mul          (C MatrixD3 &m );                        // transform by matrix
   VecD2&     mul          (C MatrixD  &m );                        // transform by matrix
   VecD2&     div          (C MatrixD3 &m );                        // transform by matrix inverse
   VecD2&     div          (C MatrixD  &m );                        // transform by matrix inverse
   VecD2&     divNormalized(C MatrixD3 &m );                        // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   VecD2&     divNormalized(C MatrixD  &m );                        // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   VecD2&     chs          (              );                        // change sign of all components
   VecD2&     abs          (              );                        // absolute       all components
   VecD2&     sat          (              );                        // saturate       all components
   VecD2&     swap         (              ) {Swap(x, y); return T;} // swap               components

   Str asText(Int precision=INT_MAX)C; // return as text

   // draw
   void draw(C Color &color, Flt r=0.007f)C; // this can be optionally called outside of Render function

   VecD2() {}
   VecD2(Dbl r       ) {set(r   );}
   VecD2(Dbl x, Dbl y) {set(x, y);}
   VecD2(C VecH2  &v);
   VecD2(C Vec2   &v);
   VecD2(C VecI2  &v);
   VecD2(C VecB2  &v);
   VecD2(C VecSB2 &v);
   VecD2(C VecUS2 &v);
};
/******************************************************************************/
struct Vec // Vector 3D
{
   union
   {
      struct{Flt  x, y, z;};
      struct{Flt  c[3]   ;}; // component
      struct{Vec2 xy     ;};
   };

   Vec& zero(                   ) {x=y=z=0;             return T;}
   Vec& set (Flt r              ) {x=y=z=r;             return T;}
   Vec& set (Flt x, Flt y, Flt z) {T.x=x; T.y=y; T.z=z; return T;}
   Vec& set (C Vec2 &xy  , Flt z) {T.xy=xy;      T.z=z; return T;}

   Vec& operator+=(  Flt       r) {x+=  r; y+=  r; z+=  r; return T;}
   Vec& operator-=(  Flt       r) {x-=  r; y-=  r; z-=  r; return T;}
   Vec& operator*=(  Flt       r) {x*=  r; y*=  r; z*=  r; return T;}
   Vec& operator/=(  Flt       r) {x/=  r; y/=  r; z/=  r; return T;}
   Vec& operator+=(C Vec      &v) {x+=v.x; y+=v.y; z+=v.z; return T;}
   Vec& operator-=(C Vec      &v) {x-=v.x; y-=v.y; z-=v.z; return T;}
   Vec& operator*=(C Vec      &v) {x*=v.x; y*=v.y; z*=v.z; return T;}
   Vec& operator/=(C Vec      &v) {x/=v.x; y/=v.y; z/=v.z; return T;}
   Vec& operator*=(C Orient   &o) {return mul(o);}
   Vec& operator*=(C OrientD  &o) {return mul(o);}
   Vec& operator*=(C OrientP  &o) {return mul(o);}
   Vec& operator*=(C OrientM  &o) {return mul(o);}
   Vec& operator*=(C Matrix3  &m) {return mul(m);}
   Vec& operator/=(C Matrix3  &m) {return div(m);}
   Vec& operator*=(C MatrixD3 &m) {return mul(m);}
   Vec& operator/=(C MatrixD3 &m) {return div(m);}
   Vec& operator*=(C Matrix   &m) {return mul(m);}
   Vec& operator/=(C Matrix   &m) {return div(m);}
   Vec& operator*=(C MatrixM  &m) {return mul(m);}
   Vec& operator/=(C MatrixM  &m) {return div(m);}
   Vec& operator*=(C MatrixD  &m) {return mul(m);}
   Vec& operator/=(C MatrixD  &m) {return div(m);}
   Vec& operator&=(C Box      &b) ; // intersect
   Vec& operator&=(C OBox     &b) ; // intersect
   Bool operator==(  Flt       r)C;
   Bool operator!=(  Flt       r)C;
   Bool operator==(C Vec      &v)C;
   Bool operator!=(C Vec      &v)C;

   friend Vec operator+ (C Vec &v, Int i) {return Vec(v.x+i, v.y+i, v.z+i);}
   friend Vec operator- (C Vec &v, Int i) {return Vec(v.x-i, v.y-i, v.z-i);}
   friend Vec operator* (C Vec &v, Int i) {return Vec(v.x*i, v.y*i, v.z*i);}
   friend Vec operator/ (C Vec &v, Int i) {return Vec(v.x/i, v.y/i, v.z/i);}

   friend Vec operator+ (C Vec &v, Flt r) {return Vec(v.x+r, v.y+r, v.z+r);}
   friend Vec operator- (C Vec &v, Flt r) {return Vec(v.x-r, v.y-r, v.z-r);}
   friend Vec operator* (C Vec &v, Flt r) {return Vec(v.x*r, v.y*r, v.z*r);}
   friend Vec operator/ (C Vec &v, Flt r) {return Vec(v.x/r, v.y/r, v.z/r);}

   friend Vec operator+ (Int i, C Vec &v) {return Vec(i+v.x, i+v.y, i+v.z);}
   friend Vec operator- (Int i, C Vec &v) {return Vec(i-v.x, i-v.y, i-v.z);}
   friend Vec operator* (Int i, C Vec &v) {return Vec(i*v.x, i*v.y, i*v.z);}
   friend Vec operator/ (Int i, C Vec &v) {return Vec(i/v.x, i/v.y, i/v.z);}

   friend Vec operator+ (Flt r, C Vec &v) {return Vec(r+v.x, r+v.y, r+v.z);}
   friend Vec operator- (Flt r, C Vec &v) {return Vec(r-v.x, r-v.y, r-v.z);}
   friend Vec operator* (Flt r, C Vec &v) {return Vec(r*v.x, r*v.y, r*v.z);}
   friend Vec operator/ (Flt r, C Vec &v) {return Vec(r/v.x, r/v.y, r/v.z);}

   friend Vec operator+ (C Vec &a, C Vec &b) {return Vec(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend Vec operator- (C Vec &a, C Vec &b) {return Vec(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend Vec operator* (C Vec &a, C Vec &b) {return Vec(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend Vec operator/ (C Vec &a, C Vec &b) {return Vec(a.x/b.x, a.y/b.y, a.z/b.z);}

   friend Vec operator* (C Vec &v, C Orient  &o) {return Vec(v)*=o;}
   friend Vec operator* (C Vec &v, C OrientP &o) {return Vec(v)*=o;}

   friend Vec operator* (C Vec &v, C Matrix3 &m) {return Vec(v)*=m;}
   friend Vec operator* (C Vec &v, C Matrix  &m) {return Vec(v)*=m;}
   friend Vec operator/ (C Vec &v, C Matrix3 &m) {return Vec(v)/=m;}
   friend Vec operator/ (C Vec &v, C Matrix  &m) {return Vec(v)/=m;}

   friend Vec operator& (C Vec &v, C Box  &b) {return Vec(v)&=b;} // intersection
   friend Vec operator& (C Vec &v, C OBox &b) {return Vec(v)&=b;} // intersection

   friend Vec operator- (C Vec &v) {return Vec(-v.x, -v.y, -v.z);}
   friend Vec operator! (C Vec &v) {       Vec ret=v; ret.normalize(); return ret;} // return normalized vector

   Vec2 xz ()C {return Vec2(x, z   );} // return as Vec2(x, z)
   Vec2 yx ()C {return Vec2(y, x   );} // return as Vec2(y, x)
   Vec2 yz ()C {return Vec2(y, z   );} // return as Vec2(y, z)
   Vec2 zx ()C {return Vec2(z, x   );} // return as Vec2(z, x)
   Vec2 zy ()C {return Vec2(z, y   );} // return as Vec2(z, y)
   Vec  xzy()C {return Vec (x, z, y);} // return as Vec (x, z, y)
   Vec  x0z()C {return Vec (x, 0, z);} // return as Vec (x, 0, z)
   Vec  xy0()C {return Vec (x, y, 0);} // return as Vec (x, y, 0)

   Bool     any          (              )C {return  Any (x, y, z);}  // if any component  is  non-zero
   Bool     all          (              )C {return  x &&  y &&  z;}  // if all components are non-zero
   Bool     allZero      (              )C {return !x && !y && !z;}  // if all components are     zero
   Int      minI         (              )C {return  MinI(x, y, z);}  // components minimum index
   Int      maxI         (              )C {return  MaxI(x, y, z);}  // components maximum index
   Flt      min          (              )C {return  Min (x, y, z);}  // components minimum
   Flt      max          (              )C {return  Max (x, y, z);}  // components maximum
   Flt      avg          (              )C {return  Avg (x, y, z);}  // components average
   Flt      sum          (              )C {return       x+ y+ z ;}  // components sum
   Flt      mul          (              )C {return       x* y* z ;}  // components multiplication
   Flt      length       (              )C;                          // get               length
   Flt      length2      (              )C {return x*x + y*y + z*z;} // get       squared length
   Vec&  satLength       (              );                           // saturate  length (clip it to 0..1)
   Flt   setLength       (Flt     length);                           // set       length and return previous length
   Vec& clipLength       (Flt max_length);                           // clip      length to 0..max_length range
   Flt      normalize    (              );                           // normalize length and return previous length
   Vec&     mul          (C Orient   &o );                           // transform by orientation
   Vec&     mul          (C OrientD  &o );                           // transform by orientation
   Vec&     mul          (C OrientP  &o );                           // transform by orientation
   Vec&     mul          (C OrientM  &o );                           // transform by orientation
   Vec&     mul          (C Matrix3  &m );                           // transform by matrix
   Vec&     mul          (C MatrixD3 &m );                           // transform by matrix
   Vec&     mul          (C Matrix   &m );                           // transform by matrix
   Vec&     mul          (C MatrixM  &m );                           // transform by matrix
   Vec&     mul          (C MatrixD  &m );                           // transform by matrix
   Vec&     div          (C Matrix3  &m );                           // transform by matrix inverse
   Vec&     div          (C MatrixD3 &m );                           // transform by matrix inverse
   Vec&     div          (C Matrix   &m );                           // transform by matrix inverse
   Vec&     div          (C MatrixM  &m );                           // transform by matrix inverse
   Vec&     div          (C MatrixD  &m );                           // transform by matrix inverse
   Vec&     divNormalized(C Matrix3  &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   Vec&     divNormalized(C MatrixD3 &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   Vec&     divNormalized(C Matrix   &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   Vec&     divNormalized(C MatrixM  &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   Vec&     divNormalized(C MatrixD  &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   Vec&     rotateX      (Flt angle     );                           // rotate along X axis by angle
   Vec&     rotateY      (Flt angle     );                           // rotate along Y axis by angle
   Vec&     rotateZ      (Flt angle     );                           // rotate along Z axis by angle
   Vec&     chs          (              );                           // change sign of all components
   Vec&     abs          (              );                           // absolute       all components
   Vec&     sat          (              );                           // saturate       all components
   Vec&     swapYZ       (              ) {Swap(y, z); return T;}    // swap       Y and Z components

   Vec& fromDiv          (C Vec  &v, C Matrix3  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C Vec  &v, C MatrixD3 &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C Vec  &v, C Matrix   &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C Vec  &v, C MatrixM  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C Vec  &v, C MatrixD  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C VecD &v, C Matrix3  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C VecD &v, C MatrixD3 &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C VecD &v, C Matrix   &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C VecD &v, C MatrixM  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDiv          (C VecD &v, C MatrixD  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix
   Vec& fromDivNormalized(C Vec  &v, C Matrix3  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C MatrixD3 &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C Matrix   &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C MatrixM  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C MatrixD  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C VecD &v, C Matrix3  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C VecD &v, C MatrixD3 &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C VecD &v, C Matrix   &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C VecD &v, C MatrixM  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C VecD &v, C MatrixD  &m); // set as "v/m", 'v' vector transformed by inverse of 'm' matrix     , this method is faster than 'div' however 'm' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C Orient   &o); // set as "v/o", 'v' vector transformed by inverse of 'o' orientation, this method is faster than 'div' however 'o' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C OrientD  &o); // set as "v/o", 'v' vector transformed by inverse of 'o' orientation, this method is faster than 'div' however 'o' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C OrientP  &o); // set as "v/o", 'v' vector transformed by inverse of 'o' orientation, this method is faster than 'div' however 'o' must be normalized
   Vec& fromDivNormalized(C Vec  &v, C OrientM  &o); // set as "v/o", 'v' vector transformed by inverse of 'o' orientation, this method is faster than 'div' however 'o' must be normalized

#if EE_PRIVATE
   void rightToLeft(); // convert from right hand to left hand coordinate system
#endif

   Str asText(Int precision=INT_MAX)C; // return as text

   // draw
   void draw (C Color &color, Flt r=0.007f)C; // draw                                                             , this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function
   void drawP(C Color &color, Flt r=0.007f)C; // draw with perspective making size dependent on distance to camera, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   Vec() {}
   Vec(Flt r              ) {set(r      );}
   Vec(Flt x, Flt y, Flt z) {set(x, y, z);}
   Vec(C Vec2 &xy  , Flt z) {set(xy  , z);}
   Vec(C VecH  &v);
   Vec(C VecD  &v);
   Vec(C VecI  &v);
   Vec(C VecB  &v);
   Vec(C VecSB &v);
};extern const Vec
   VecDir[DIR_NUM], // vector[DIR_ENUM] array
   VecZero        , // Vec(0, 0, 0)
   VecOne         ; // Vec(1, 1, 1)
/******************************************************************************/
struct VecD // Vector 3D (double precision)
{
   union
   {
      struct{Dbl   x, y, z;};
      struct{Dbl   c[3]   ;}; // component
      struct{VecD2 xy     ;};
   };

   VecD& zero(                   ) {x=y=z=0;             return T;}
   VecD& set (Dbl r              ) {x=y=z=r;             return T;}
   VecD& set (Dbl x, Dbl y, Dbl z) {T.x=x; T.y=y; T.z=z; return T;}
   VecD& set (C VecD2 &xy , Dbl z) {T.xy=xy;      T.z=z; return T;}

   VecD& operator+=(  Dbl       r) {x+=  r; y+=  r; z+=  r; return T;}
   VecD& operator-=(  Dbl       r) {x-=  r; y-=  r; z-=  r; return T;}
   VecD& operator*=(  Dbl       r) {x*=  r; y*=  r; z*=  r; return T;}
   VecD& operator/=(  Dbl       r) {x/=  r; y/=  r; z/=  r; return T;}
   VecD& operator+=(C Vec      &v) {x+=v.x; y+=v.y; z+=v.z; return T;}
   VecD& operator-=(C Vec      &v) {x-=v.x; y-=v.y; z-=v.z; return T;}
   VecD& operator*=(C Vec      &v) {x*=v.x; y*=v.y; z*=v.z; return T;}
   VecD& operator/=(C Vec      &v) {x/=v.x; y/=v.y; z/=v.z; return T;}
   VecD& operator+=(C VecD     &v) {x+=v.x; y+=v.y; z+=v.z; return T;}
   VecD& operator-=(C VecD     &v) {x-=v.x; y-=v.y; z-=v.z; return T;}
   VecD& operator*=(C VecD     &v) {x*=v.x; y*=v.y; z*=v.z; return T;}
   VecD& operator/=(C VecD     &v) {x/=v.x; y/=v.y; z/=v.z; return T;}
   VecD& operator*=(C Orient   &o) {return mul(o);}
   VecD& operator*=(C OrientD  &o) {return mul(o);}
   VecD& operator*=(C OrientP  &o) {return mul(o);}
   VecD& operator*=(C OrientM  &o) {return mul(o);}
   VecD& operator*=(C Matrix3  &m) {return mul(m);}
   VecD& operator/=(C Matrix3  &m) {return div(m);}
   VecD& operator*=(C MatrixD3 &m) {return mul(m);}
   VecD& operator/=(C MatrixD3 &m) {return div(m);}
   VecD& operator*=(C Matrix   &m) {return mul(m);}
   VecD& operator/=(C Matrix   &m) {return div(m);}
   VecD& operator*=(C MatrixM  &m) {return mul(m);}
   VecD& operator/=(C MatrixM  &m) {return div(m);}
   VecD& operator*=(C MatrixD  &m) {return mul(m);}
   VecD& operator/=(C MatrixD  &m) {return div(m);}
   Bool  operator==(C VecD     &v)C;
   Bool  operator!=(C VecD     &v)C;

   friend VecD operator+ (C VecD &v, Int i) {return VecD(v.x+i, v.y+i, v.z+i);}
   friend VecD operator- (C VecD &v, Int i) {return VecD(v.x-i, v.y-i, v.z-i);}
   friend VecD operator* (C VecD &v, Int i) {return VecD(v.x*i, v.y*i, v.z*i);}
   friend VecD operator/ (C VecD &v, Int i) {return VecD(v.x/i, v.y/i, v.z/i);}

   friend VecD operator+ (C VecD &v, Long i) {return VecD(v.x+i, v.y+i, v.z+i);}
   friend VecD operator- (C VecD &v, Long i) {return VecD(v.x-i, v.y-i, v.z-i);}
   friend VecD operator* (C VecD &v, Long i) {return VecD(v.x*i, v.y*i, v.z*i);}
   friend VecD operator/ (C VecD &v, Long i) {return VecD(v.x/i, v.y/i, v.z/i);}

   friend VecD operator+ (C VecD &v, Flt r) {return VecD(v.x+r, v.y+r, v.z+r);}
   friend VecD operator- (C VecD &v, Flt r) {return VecD(v.x-r, v.y-r, v.z-r);}
   friend VecD operator* (C VecD &v, Flt r) {return VecD(v.x*r, v.y*r, v.z*r);}
   friend VecD operator/ (C VecD &v, Flt r) {return VecD(v.x/r, v.y/r, v.z/r);}

   friend VecD operator+ (C VecD &v, Dbl r) {return VecD(v.x+r, v.y+r, v.z+r);}
   friend VecD operator- (C VecD &v, Dbl r) {return VecD(v.x-r, v.y-r, v.z-r);}
   friend VecD operator* (C VecD &v, Dbl r) {return VecD(v.x*r, v.y*r, v.z*r);}
   friend VecD operator/ (C VecD &v, Dbl r) {return VecD(v.x/r, v.y/r, v.z/r);}

   friend VecD operator+ (Int i, C VecD &v) {return VecD(i+v.x, i+v.y, i+v.z);}
   friend VecD operator- (Int i, C VecD &v) {return VecD(i-v.x, i-v.y, i-v.z);}
   friend VecD operator* (Int i, C VecD &v) {return VecD(i*v.x, i*v.y, i*v.z);}
   friend VecD operator/ (Int i, C VecD &v) {return VecD(i/v.x, i/v.y, i/v.z);}

   friend VecD operator+ (Long i, C VecD &v) {return VecD(i+v.x, i+v.y, i+v.z);}
   friend VecD operator- (Long i, C VecD &v) {return VecD(i-v.x, i-v.y, i-v.z);}
   friend VecD operator* (Long i, C VecD &v) {return VecD(i*v.x, i*v.y, i*v.z);}
   friend VecD operator/ (Long i, C VecD &v) {return VecD(i/v.x, i/v.y, i/v.z);}

   friend VecD operator+ (Flt r, C VecD &v) {return VecD(r+v.x, r+v.y, r+v.z);}
   friend VecD operator- (Flt r, C VecD &v) {return VecD(r-v.x, r-v.y, r-v.z);}
   friend VecD operator* (Flt r, C VecD &v) {return VecD(r*v.x, r*v.y, r*v.z);}
   friend VecD operator/ (Flt r, C VecD &v) {return VecD(r/v.x, r/v.y, r/v.z);}

   friend VecD operator+ (Dbl r, C VecD &v) {return VecD(r+v.x, r+v.y, r+v.z);}
   friend VecD operator- (Dbl r, C VecD &v) {return VecD(r-v.x, r-v.y, r-v.z);}
   friend VecD operator* (Dbl r, C VecD &v) {return VecD(r*v.x, r*v.y, r*v.z);}
   friend VecD operator/ (Dbl r, C VecD &v) {return VecD(r/v.x, r/v.y, r/v.z);}

   friend VecD operator+ (C Vec &a, C VecD &b) {return VecD(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecD operator- (C Vec &a, C VecD &b) {return VecD(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecD operator* (C Vec &a, C VecD &b) {return VecD(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecD operator/ (C Vec &a, C VecD &b) {return VecD(a.x/b.x, a.y/b.y, a.z/b.z);}

   friend VecD operator+ (C VecD &a, C Vec &b) {return VecD(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecD operator- (C VecD &a, C Vec &b) {return VecD(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecD operator* (C VecD &a, C Vec &b) {return VecD(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecD operator/ (C VecD &a, C Vec &b) {return VecD(a.x/b.x, a.y/b.y, a.z/b.z);}

   friend VecD operator+ (C VecD &a, C VecD &b) {return VecD(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecD operator- (C VecD &a, C VecD &b) {return VecD(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecD operator* (C VecD &a, C VecD &b) {return VecD(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecD operator/ (C VecD &a, C VecD &b) {return VecD(a.x/b.x, a.y/b.y, a.z/b.z);}

   friend VecD operator* (C VecD &v, C Orient  &o) {return VecD(v)*=o;}
   friend VecD operator* (C VecD &v, C OrientD &o) {return VecD(v)*=o;}
   friend VecD operator* (C VecD &v, C OrientP &o) {return VecD(v)*=o;}
   friend VecD operator* (C VecD &v, C OrientM &o) {return VecD(v)*=o;}

   friend VecD operator* (C VecD &v, C Matrix3  &m) {return VecD(v)*=m;}
   friend VecD operator* (C VecD &v, C MatrixD3 &m) {return VecD(v)*=m;}
   friend VecD operator* (C VecD &v, C Matrix   &m) {return VecD(v)*=m;}
   friend VecD operator* (C VecD &v, C MatrixM  &m) {return VecD(v)*=m;}
   friend VecD operator* (C VecD &v, C MatrixD  &m) {return VecD(v)*=m;}
   friend VecD operator/ (C VecD &v, C Matrix3  &m) {return VecD(v)/=m;}
   friend VecD operator/ (C VecD &v, C MatrixD3 &m) {return VecD(v)/=m;}
   friend VecD operator/ (C VecD &v, C Matrix   &m) {return VecD(v)/=m;}
   friend VecD operator/ (C VecD &v, C MatrixM  &m) {return VecD(v)/=m;}
   friend VecD operator/ (C VecD &v, C MatrixD  &m) {return VecD(v)/=m;}

   friend VecD operator- (C VecD &v) {return VecD(-v.x, -v.y, -v.z);}
   friend VecD operator! (C VecD &v) {       VecD ret=v; ret.normalize(); return ret;} // return normalized vector

   VecD2 xz()C {return VecD2(x, z);} // return as VecD2(x, z)
   VecD2 yx()C {return VecD2(y, x);} // return as VecD2(y, x)
   VecD2 yz()C {return VecD2(y, z);} // return as VecD2(y, z)
   VecD2 zx()C {return VecD2(z, x);} // return as VecD2(z, x)
   VecD2 zy()C {return VecD2(z, y);} // return as VecD2(z, y)

   Bool      any          (              )C {return  x ||  y ||  z;}  // if any component  is  non-zero
   Bool      all          (              )C {return  x &&  y &&  z;}  // if all components are non-zero
   Bool      allZero      (              )C {return !x && !y && !z;}  // if all components are     zero
   Int       minI         (              )C {return  MinI(x, y, z);}  // components minimum index
   Int       maxI         (              )C {return  MaxI(x, y, z);}  // components maximum index
   Dbl       min          (              )C {return  Min (x, y, z);}  // components minimum
   Dbl       max          (              )C {return  Max (x, y, z);}  // components maximum
   Dbl       avg          (              )C {return  Avg (x, y, z);}  // components average
   Dbl       sum          (              )C {return       x+ y+ z ;}  // components sum
   Dbl       mul          (              )C {return       x* y* z ;}  // components multiplication
   Dbl       length       (              )C;                          // get               length
   Dbl       length2      (              )C {return x*x + y*y + z*z;} // get       squared length
   VecD&  satLength       (              );                           // saturate  length (clip it to 0..1)
   Dbl    setLength       (Dbl     length);                           // set       length and return previous length
   VecD& clipLength       (Dbl max_length);                           // clip      length to 0..max_length range
   Dbl       normalize    (              );                           // normalize length and return previous length
   VecD&     mul          (C Orient   &o );                           // transform by orientation
   VecD&     mul          (C OrientD  &o );                           // transform by orientation
   VecD&     mul          (C OrientP  &o );                           // transform by orientation
   VecD&     mul          (C OrientM  &o );                           // transform by orientation
   VecD&     mul          (C Matrix3  &m );                           // transform by matrix
   VecD&     mul          (C MatrixD3 &m );                           // transform by matrix
   VecD&     mul          (C Matrix   &m );                           // transform by matrix
   VecD&     mul          (C MatrixM  &m );                           // transform by matrix
   VecD&     mul          (C MatrixD  &m );                           // transform by matrix
   VecD&     div          (C Matrix3  &m );                           // transform by matrix inverse
   VecD&     div          (C MatrixD3 &m );                           // transform by matrix inverse
   VecD&     div          (C Matrix   &m );                           // transform by matrix inverse
   VecD&     div          (C MatrixM  &m );                           // transform by matrix inverse
   VecD&     div          (C MatrixD  &m );                           // transform by matrix inverse
   VecD&     divNormalized(C Matrix3  &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   VecD&     divNormalized(C MatrixD3 &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   VecD&     divNormalized(C Matrix   &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   VecD&     divNormalized(C MatrixM  &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   VecD&     divNormalized(C MatrixD  &m );                           // transform by matrix inverse, this method is faster than 'div' however 'm' must be normalized
   VecD&     chs          (              );                           // change sign of all components
   VecD&     abs          (              );                           // absolute       all components
   VecD&     sat          (              );                           // saturate       all components
   VecD&     swapYZ       (              ) {Swap(y, z); return T;}    // swap       Y and Z components

   Str asText(Int precision=INT_MAX)C; // return as text

   // draw
   void draw (C Color &color, Flt r=0.007f)C; // draw                                                             , this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function
   void drawP(C Color &color, Flt r=0.007f)C; // draw with perspective making size dependent on distance to camera, this can be optionally called outside of Render function, this relies on active object matrix which can be set using 'SetMatrix' function

   VecD() {}
   VecD(Dbl r              ) {set(r      );}
   VecD(Dbl x, Dbl y, Dbl z) {set(x, y, z);}
   VecD(C VecD2 &xy , Dbl z) {set(xy  , z);}
   VecD(C VecH  &v);
   VecD(C Vec   &v);
   VecD(C VecI  &v);
   VecD(C VecB  &v);
   VecD(C VecSB &v);
};
/******************************************************************************/
struct Vec4 // Vector 4D
{
   union
   {
      struct{Flt  x, y, z, w;};
      struct{Flt  c[4]      ;}; // component
      struct{Vec2 xy, zw    ;};
      struct{Vec  xyz       ;};
   };

   Vec4& zero(                          ) {x=y=z=w=0;                  return T;}
   Vec4& set (Flt r                     ) {x=y=z=w=r;                  return T;}
   Vec4& set (Flt x, Flt y, Flt z, Flt w) {T.x=x; T.y=y; T.z=z; T.w=w; return T;}
   Vec4& set (C Vec2 &xy ,  Flt z, Flt w) {T.xy =xy ;    T.z=z; T.w=w; return T;}
   Vec4& set (C Vec  &xyz,         Flt w) {T.xyz=xyz;           T.w=w; return T;}
   Vec4& set (C Vec2 &xy ,  C Vec2 &zw  ) {T.xy =xy ;    T.zw=zw;      return T;}

   Vec4& operator+=(  Flt      r) {x+=  r; y+=  r; z+=  r; w+=  r; return T;}
   Vec4& operator-=(  Flt      r) {x-=  r; y-=  r; z-=  r; w-=  r; return T;}
   Vec4& operator*=(  Flt      r) {x*=  r; y*=  r; z*=  r; w*=  r; return T;}
   Vec4& operator/=(  Flt      r) {x/=  r; y/=  r; z/=  r; w/=  r; return T;}
   Vec4& operator+=(C Vec4    &v) {x+=v.x; y+=v.y; z+=v.z; w+=v.w; return T;}
   Vec4& operator-=(C Vec4    &v) {x-=v.x; y-=v.y; z-=v.z; w-=v.w; return T;}
   Vec4& operator*=(C Vec4    &v) {x*=v.x; y*=v.y; z*=v.z; w*=v.w; return T;}
   Vec4& operator/=(C Vec4    &v) {x/=v.x; y/=v.y; z/=v.z; w/=v.w; return T;}
   Vec4& operator*=(C Matrix4 &m) {return mul(m);}
   Bool  operator==(  Flt      r)C;
   Bool  operator!=(  Flt      r)C;
   Bool  operator==(C Vec4    &v)C;
   Bool  operator!=(C Vec4    &v)C;

   friend Vec4 operator+ (C Vec4 &v, Int i) {return Vec4(v.x+i, v.y+i, v.z+i, v.w+i);}
   friend Vec4 operator- (C Vec4 &v, Int i) {return Vec4(v.x-i, v.y-i, v.z-i, v.w-i);}
   friend Vec4 operator* (C Vec4 &v, Int i) {return Vec4(v.x*i, v.y*i, v.z*i, v.w*i);}
   friend Vec4 operator/ (C Vec4 &v, Int i) {return Vec4(v.x/i, v.y/i, v.z/i, v.w/i);}

   friend Vec4 operator+ (C Vec4 &v, Flt r) {return Vec4(v.x+r, v.y+r, v.z+r, v.w+r);}
   friend Vec4 operator- (C Vec4 &v, Flt r) {return Vec4(v.x-r, v.y-r, v.z-r, v.w-r);}
   friend Vec4 operator* (C Vec4 &v, Flt r) {return Vec4(v.x*r, v.y*r, v.z*r, v.w*r);}
   friend Vec4 operator/ (C Vec4 &v, Flt r) {return Vec4(v.x/r, v.y/r, v.z/r, v.w/r);}

   friend Vec4 operator+ (Int i, C Vec4 &v) {return Vec4(i+v.x, i+v.y, i+v.z, i+v.w);}
   friend Vec4 operator- (Int i, C Vec4 &v) {return Vec4(i-v.x, i-v.y, i-v.z, i-v.w);}
   friend Vec4 operator* (Int i, C Vec4 &v) {return Vec4(i*v.x, i*v.y, i*v.z, i*v.w);}
   friend Vec4 operator/ (Int i, C Vec4 &v) {return Vec4(i/v.x, i/v.y, i/v.z, i/v.w);}

   friend Vec4 operator+ (Flt r, C Vec4 &v) {return Vec4(r+v.x, r+v.y, r+v.z, r+v.w);}
   friend Vec4 operator- (Flt r, C Vec4 &v) {return Vec4(r-v.x, r-v.y, r-v.z, r-v.w);}
   friend Vec4 operator* (Flt r, C Vec4 &v) {return Vec4(r*v.x, r*v.y, r*v.z, r*v.w);}
   friend Vec4 operator/ (Flt r, C Vec4 &v) {return Vec4(r/v.x, r/v.y, r/v.z, r/v.w);}

   friend Vec4 operator+ (C Vec4 &a, C Vec4 &b) {return Vec4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend Vec4 operator- (C Vec4 &a, C Vec4 &b) {return Vec4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend Vec4 operator* (C Vec4 &a, C Vec4 &b) {return Vec4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend Vec4 operator/ (C Vec4 &a, C Vec4 &b) {return Vec4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}

   friend Vec4 operator* (C Vec4 &v, C Matrix4 &m) {return Vec4(v)*=m;}

   friend Vec4 operator- (C Vec4 &v) {return Vec4(-v.x, -v.y, -v.z, -v.w);}
   friend Vec4 operator! (C Vec4 &v) {       Vec4 ret=v; ret.normalize(); return ret;} // return normalized vector

   Bool  any      ()C {return     Any (x, y, z, w);}  // if any component  is  non-zero
   Bool  all      ()C {return  x &&  y &&  z &&  w;}  // if all components are non-zero
   Bool  allZero  ()C {return !x && !y && !z && !w;}  // if all components are     zero
   Int   minI     ()C {return     MinI(x, y, z, w);}  // components minimum index
   Int   maxI     ()C {return     MaxI(x, y, z, w);}  // components maximum index
   Flt   min      ()C {return     Min (x, y, z, w);}  // components minimum
   Flt   max      ()C {return     Max (x, y, z, w);}  // components maximum
   Flt   avg      ()C {return     Avg (x, y, z, w);}  // components average
   Flt   sum      ()C {return          x+ y+ z+ w ;}  // components sum
   Flt   mul      ()C {return          x* y* z* w ;}  // components multiplication
   Flt   length   ()C;                                // get         length
   Flt   length2  ()C {return x*x + y*y + z*z + w*w;} // get squared length
   Flt   normalize();                                 // normalize and return previous length
   Vec4& mul      (C Matrix4 &m);                     // transform by matrix
   Vec4& chs      ();                                 // change sign of all components
   Vec4& abs      ();                                 // absolute       all components
   Vec4& sat      ();                                 // saturate       all components

   Str asText(Int precision=INT_MAX)C; // return as text

   Vec4() {}
   Vec4(Flt r                     ) {set(r         );}
   Vec4(Flt x, Flt y, Flt z, Flt w) {set(x, y, z, w);}
   Vec4(C Vec2  &xy , Flt z, Flt w) {set(xy ,  z, w);}
   Vec4(C Vec   &xyz,        Flt w) {set(xyz,     w);}
   Vec4(C Vec2  &xy , C Vec2 &zw  ) {set(xy ,    zw);}
   Vec4(C VecH4  &v);
   Vec4(C VecD4  &v);
   Vec4(C VecI4  &v);
   Vec4(C VecB4  &v);
   Vec4(C VecSB4 &v);
};extern Vec4
   const Vec4Zero; // Vec4(0, 0, 0, 0)
/******************************************************************************/
struct VecD4 // Vector 4D (double precision)
{
   union
   {
      struct{Dbl   x, y, z, w;};
      struct{Dbl   c[4]      ;}; // component
      struct{VecD2 xy, zw    ;};
      struct{VecD  xyz       ;};
   };

   VecD4& zero(                          ) {x=y=z=w=0;                  return T;}
   VecD4& set (Dbl r                     ) {x=y=z=w=r;                  return T;}
   VecD4& set (Dbl x, Dbl y, Dbl z, Dbl w) {T.x=x; T.y=y; T.z=z; T.w=w; return T;}
   VecD4& set (C VecD2 &xy , Dbl z, Dbl w) {T.xy =xy ;    T.z=z; T.w=w; return T;}
   VecD4& set (C VecD  &xyz,        Dbl w) {T.xyz=xyz;           T.w=w; return T;}
   VecD4& set (C VecD2 &xy , C VecD2 &zw ) {T.xy =xy ;    T.zw=zw;      return T;}

   VecD4& operator+=(  Dbl    r) {x+=  r; y+=  r; z+=  r; w+=  r; return T;}
   VecD4& operator-=(  Dbl    r) {x-=  r; y-=  r; z-=  r; w-=  r; return T;}
   VecD4& operator*=(  Dbl    r) {x*=  r; y*=  r; z*=  r; w*=  r; return T;}
   VecD4& operator/=(  Dbl    r) {x/=  r; y/=  r; z/=  r; w/=  r; return T;}
   VecD4& operator+=(C VecD4 &v) {x+=v.x; y+=v.y; z+=v.z; w+=v.w; return T;}
   VecD4& operator-=(C VecD4 &v) {x-=v.x; y-=v.y; z-=v.z; w-=v.w; return T;}
   VecD4& operator*=(C VecD4 &v) {x*=v.x; y*=v.y; z*=v.z; w*=v.w; return T;}
   VecD4& operator/=(C VecD4 &v) {x/=v.x; y/=v.y; z/=v.z; w/=v.w; return T;}
   Bool   operator==(C VecD4 &v)C;
   Bool   operator!=(C VecD4 &v)C;

   friend VecD4 operator+ (C VecD4 &v, Int i) {return VecD4(v.x+i, v.y+i, v.z+i, v.w+i);}
   friend VecD4 operator- (C VecD4 &v, Int i) {return VecD4(v.x-i, v.y-i, v.z-i, v.w-i);}
   friend VecD4 operator* (C VecD4 &v, Int i) {return VecD4(v.x*i, v.y*i, v.z*i, v.w*i);}
   friend VecD4 operator/ (C VecD4 &v, Int i) {return VecD4(v.x/i, v.y/i, v.z/i, v.w/i);}

   friend VecD4 operator+ (C VecD4 &v, Long i) {return VecD4(v.x+i, v.y+i, v.z+i, v.w+i);}
   friend VecD4 operator- (C VecD4 &v, Long i) {return VecD4(v.x-i, v.y-i, v.z-i, v.w-i);}
   friend VecD4 operator* (C VecD4 &v, Long i) {return VecD4(v.x*i, v.y*i, v.z*i, v.w*i);}
   friend VecD4 operator/ (C VecD4 &v, Long i) {return VecD4(v.x/i, v.y/i, v.z/i, v.w/i);}

   friend VecD4 operator+ (C VecD4 &v, Flt r) {return VecD4(v.x+r, v.y+r, v.z+r, v.w+r);}
   friend VecD4 operator- (C VecD4 &v, Flt r) {return VecD4(v.x-r, v.y-r, v.z-r, v.w-r);}
   friend VecD4 operator* (C VecD4 &v, Flt r) {return VecD4(v.x*r, v.y*r, v.z*r, v.w*r);}
   friend VecD4 operator/ (C VecD4 &v, Flt r) {return VecD4(v.x/r, v.y/r, v.z/r, v.w/r);}

   friend VecD4 operator+ (C VecD4 &v, Dbl r) {return VecD4(v.x+r, v.y+r, v.z+r, v.w+r);}
   friend VecD4 operator- (C VecD4 &v, Dbl r) {return VecD4(v.x-r, v.y-r, v.z-r, v.w-r);}
   friend VecD4 operator* (C VecD4 &v, Dbl r) {return VecD4(v.x*r, v.y*r, v.z*r, v.w*r);}
   friend VecD4 operator/ (C VecD4 &v, Dbl r) {return VecD4(v.x/r, v.y/r, v.z/r, v.w/r);}

   friend VecD4 operator+ (Int i, C VecD4 &v) {return VecD4(i+v.x, i+v.y, i+v.z, i+v.w);}
   friend VecD4 operator- (Int i, C VecD4 &v) {return VecD4(i-v.x, i-v.y, i-v.z, i-v.w);}
   friend VecD4 operator* (Int i, C VecD4 &v) {return VecD4(i*v.x, i*v.y, i*v.z, i*v.w);}
   friend VecD4 operator/ (Int i, C VecD4 &v) {return VecD4(i/v.x, i/v.y, i/v.z, i/v.w);}

   friend VecD4 operator+ (Long i, C VecD4 &v) {return VecD4(i+v.x, i+v.y, i+v.z, i+v.w);}
   friend VecD4 operator- (Long i, C VecD4 &v) {return VecD4(i-v.x, i-v.y, i-v.z, i-v.w);}
   friend VecD4 operator* (Long i, C VecD4 &v) {return VecD4(i*v.x, i*v.y, i*v.z, i*v.w);}
   friend VecD4 operator/ (Long i, C VecD4 &v) {return VecD4(i/v.x, i/v.y, i/v.z, i/v.w);}

   friend VecD4 operator+ (Flt r, C VecD4 &v) {return VecD4(r+v.x, r+v.y, r+v.z, r+v.w);}
   friend VecD4 operator- (Flt r, C VecD4 &v) {return VecD4(r-v.x, r-v.y, r-v.z, r-v.w);}
   friend VecD4 operator* (Flt r, C VecD4 &v) {return VecD4(r*v.x, r*v.y, r*v.z, r*v.w);}
   friend VecD4 operator/ (Flt r, C VecD4 &v) {return VecD4(r/v.x, r/v.y, r/v.z, r/v.w);}

   friend VecD4 operator+ (Dbl r, C VecD4 &v) {return VecD4(r+v.x, r+v.y, r+v.z, r+v.w);}
   friend VecD4 operator- (Dbl r, C VecD4 &v) {return VecD4(r-v.x, r-v.y, r-v.z, r-v.w);}
   friend VecD4 operator* (Dbl r, C VecD4 &v) {return VecD4(r*v.x, r*v.y, r*v.z, r*v.w);}
   friend VecD4 operator/ (Dbl r, C VecD4 &v) {return VecD4(r/v.x, r/v.y, r/v.z, r/v.w);}

   friend VecD4 operator+ (C VecD4 &a, C VecD4 &b) {return VecD4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend VecD4 operator- (C VecD4 &a, C VecD4 &b) {return VecD4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend VecD4 operator* (C VecD4 &a, C VecD4 &b) {return VecD4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend VecD4 operator/ (C VecD4 &a, C VecD4 &b) {return VecD4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}

   friend VecD4 operator- (C VecD4 &v) {return VecD4(-v.x, -v.y, -v.z, -v.w);}
   friend VecD4 operator! (C VecD4 &v) {       VecD4 ret=v; ret.normalize(); return ret;} // return normalized vector

   Bool   any      ()C {return  x ||  y ||  z ||  w;}   // if any component  is  non-zero
   Bool   all      ()C {return  x &&  y &&  z &&  w;}   // if all components are non-zero
   Bool   allZero  ()C {return !x && !y && !z && !w;}   // if all components are     zero
   Int    minI     ()C {return     MinI(x, y, z, w);}   // components minimum index
   Int    maxI     ()C {return     MaxI(x, y, z, w);}   // components maximum index
   Dbl    min      ()C {return     Min (x, y, z, w);}   // components minimum
   Dbl    max      ()C {return     Max (x, y, z, w);}   // components maximum
   Dbl    avg      ()C {return     Avg (x, y, z, w);}   // components average
   Dbl    sum      ()C {return          x+ y+ z+ w ;}   // components sum
   Dbl    mul      ()C {return          x* y* z* w ;}   // components multiplication
   Dbl    length   ()C;                                 // get         length
   Dbl    length2  ()C {return  x*x + y*y + z*z + w*w;} // get squared length
   Dbl    normalize();                                  // normalize and return previous length
   VecD4& chs      ();                                  // change sign of all components
   VecD4& abs      ();                                  // absolute       all components
   VecD4& sat      ();                                  // saturate       all components

   Str asText(Int precision=INT_MAX)C; // return as text

   VecD4() {}
   VecD4(Dbl r                     ) {set(r         );}
   VecD4(Dbl x, Dbl y, Dbl z, Dbl w) {set(x, y, z, w);}
   VecD4(C VecD2 &xy , Dbl z, Dbl w) {set(xy ,  z, w);}
   VecD4(C VecD  &xyz,        Dbl w) {set(xyz,     w);}
   VecD4(C VecD2 &xy , C VecD2 &zw ) {set(xy ,    zw);}
   VecD4(C VecH4  &v);
   VecD4(C Vec4   &v);
   VecD4(C VecI4  &v);
   VecD4(C VecB4  &v);
   VecD4(C VecSB4 &v);
};
/******************************************************************************/
struct VecB2 // Vector 2D (Byte)
{
   union
   {
      struct{Byte x, y;};
      struct{Byte c[2];}; // component
   };

   VecB2& zero(              ) {x=y=0;        return T;}
   VecB2& set (Byte i        ) {x=y=i;        return T;}
   VecB2& set (Byte x, Byte y) {T.x=x; T.y=y; return T;}

   Bool operator==(C VecB2 &v)C {return x==v.x && y==v.y;}
   Bool operator!=(C VecB2 &v)C {return x!=v.x || y!=v.y;}

   Bool   any         ()C {return       x ||  y;} // if any component  is  non-zero
   Bool   all         ()C {return       x &&  y;} // if all components are non-zero
   Bool   allZero     ()C {return      !x && !y;} // if all components are     zero
   Bool   allDifferent()C {return         x!=y ;} // if all components are different
   Int    minI        ()C {return    MinI(x, y);} // components minimum index
   Int    maxI        ()C {return    MaxI(x, y);} // components maximum index
   Int    min         ()C {return    Min (x, y);} // components minimum
   Int    max         ()C {return    Max (x, y);} // components maximum
   Int    sum         ()C {return         x+ y ;} // components sum
   VecB2& reverse     ()  {Swap(x, y); return T;} // reverse components order

   VecB2() {}
   VecB2(Byte i        ) {set(i   );}
   VecB2(Byte x, Byte y) {set(x, y);}
};
/******************************************************************************/
struct VecSB2 // Vector 2D (SByte)
{
   union
   {
      struct{SByte x, y;};
      struct{SByte c[2];}; // component
   };

   VecSB2& zero(                ) {x=y=0;        return T;}
   VecSB2& set (SByte i         ) {x=y=i;        return T;}
   VecSB2& set (SByte x, SByte y) {T.x=x; T.y=y; return T;}

   Bool operator==(C VecSB2 &v)C {return x==v.x && y==v.y;}
   Bool operator!=(C VecSB2 &v)C {return x!=v.x || y!=v.y;}

   Bool    any         ()C {return       x ||  y;} // if any component  is  non-zero
   Bool    all         ()C {return       x &&  y;} // if all components are non-zero
   Bool    allZero     ()C {return      !x && !y;} // if all components are     zero
   Bool    allDifferent()C {return         x!=y ;} // if all components are different
   Int     minI        ()C {return    MinI(x, y);} // components minimum index
   Int     maxI        ()C {return    MaxI(x, y);} // components maximum index
   Int     min         ()C {return    Min (x, y);} // components minimum
   Int     max         ()C {return    Max (x, y);} // components maximum
   Int     sum         ()C {return         x+ y ;} // components sum
   VecSB2& reverse     ()  {Swap(x, y); return T;} // reverse components order

   VecSB2() {}
   VecSB2(SByte i         ) {set(i   );}
   VecSB2(SByte x, SByte y) {set(x, y);}
};
/******************************************************************************/
struct VecB // Vector 3D (Byte)
{
   union
   {
      struct{Byte  x, y, z;};
      struct{Byte  c[3]   ;}; // component
      struct{VecB2 xy     ;};
   };

   VecB& zero(                      ) {x=y=z=0;             return T;}
   VecB& set (Byte i                ) {x=y=z=i;             return T;}
   VecB& set (Byte x, Byte y, Byte z) {T.x=x; T.y=y; T.z=z; return T;}

   Bool operator==(C VecB &v)C {return x==v.x && y==v.y && z==v.z;}
   Bool operator!=(C VecB &v)C {return x!=v.x || y!=v.y || z!=v.z;}

   Bool  any         (         )C {return        x ||  y ||  z;} // if any component  is  non-zero
   Bool  all         (         )C {return        x &&  y &&  z;} // if all components are non-zero
   Bool  allZero     (         )C {return       !x && !y && !z;} // if all components are     zero
   Bool  allDifferent(         )C {return x!=y && x!=z && y!=z;} // if all components are different
   Int   minI        (         )C {return        MinI(x, y, z);} // components minimum index
   Int   maxI        (         )C {return        MaxI(x, y, z);} // components maximum index
   Int   min         (         )C {return        Min (x, y, z);} // components minimum
   Int   max         (         )C {return        Max (x, y, z);} // components maximum
   Int   sum         (         )C {return             x+ y+ z ;} // components sum
   Int   find        (Int value)C;                               // get index of first component that equals 'value' (-1 if none)
   VecB& reverse     (         )  {       Swap(x, z); return T;} // reverse components order

   VecB() {}
   VecB(Byte i                ) {set(i      );}
   VecB(Byte x, Byte y, Byte z) {set(x, y, z);}
};
/******************************************************************************/
struct VecSB // Vector 3D (SByte)
{
   union
   {
      struct{SByte  x, y, z;};
      struct{SByte  c[3]   ;}; // component
      struct{VecSB2 xy     ;};
   };

   VecSB& zero(                         ) {x=y=z=0;             return T;}
   VecSB& set (SByte i                  ) {x=y=z=i;             return T;}
   VecSB& set (SByte x, SByte y, SByte z) {T.x=x; T.y=y; T.z=z; return T;}

   Bool operator==(C VecSB &v)C {return x==v.x && y==v.y && z==v.z;}
   Bool operator!=(C VecSB &v)C {return x!=v.x || y!=v.y || z!=v.z;}

   Bool   any         (         )C {return        x ||  y ||  z;} // if any component  is  non-zero
   Bool   all         (         )C {return        x &&  y &&  z;} // if all components are non-zero
   Bool   allZero     (         )C {return       !x && !y && !z;} // if all components are     zero
   Bool   allDifferent(         )C {return x!=y && x!=z && y!=z;} // if all components are different
   Int    minI        (         )C {return        MinI(x, y, z);} // components minimum index
   Int    maxI        (         )C {return        MaxI(x, y, z);} // components maximum index
   Int    min         (         )C {return        Min (x, y, z);} // components minimum
   Int    max         (         )C {return        Max (x, y, z);} // components maximum
   Int    sum         (         )C {return             x+ y+ z ;} // components sum
   Int    find        (Int value)C;                               // get index of first component that equals 'value' (-1 if none)
   VecSB& reverse     (         )  {       Swap(x, z); return T;} // reverse components order

   VecSB() {}
   VecSB(SByte i                  ) {set(i      );}
   VecSB(SByte x, SByte y, SByte z) {set(x, y, z);}
};
/******************************************************************************/
struct VecB4 // Vector 4D (Byte)
{
   union
   {
      struct{Byte  x, y, z, w;};
      struct{Byte  c[4]      ;}; // component
      struct{UInt  u         ;};
      struct{VecB2 xy, zw    ;};
      struct{VecB  xyz       ;};
   };

   VecB4& zero(                              ) {u=0;                        return T;}
   VecB4& set (Byte i                        ) {x=y=z=w=i;                  return T;}
   VecB4& set (Byte x, Byte y, Byte z, Byte w) {T.x=x; T.y=y; T.z=z; T.w=w; return T;}

   void   operator++(  Int     )  {x++ ; y++ ; z++ ; w++ ;}
   void   operator--(  Int     )  {x-- ; y-- ; z-- ; w-- ;}
   VecB4& operator+=(  Int    i)  {x+=i; y+=i; z+=i; w+=i; return T;}
   VecB4& operator-=(  Int    i)  {x-=i; y-=i; z-=i; w-=i; return T;}
   Bool   operator==(C VecB4 &v)C {return u==v.u;}
   Bool   operator!=(C VecB4 &v)C {return u!=v.u;}

   friend VecB4 operator+ (C VecB4 &v, Int i) {return VecB4(v.x+i, v.y+i, v.z+i, v.w+i);}
   friend VecB4 operator- (C VecB4 &v, Int i) {return VecB4(v.x-i, v.y-i, v.z-i, v.w-i);}

   Bool   any         (         )C {return u!=0            ;}                             // if any component  is  non-zero, faster version of "x || y || z || w"
   Bool   all         (         )C {return x && y && z && w;}                             // if all components are non-zero
   Bool   allZero     (         )C {return u==0;}                                         // if all components are     zero
   Bool   allDifferent(         )C {return x!=y && x!=z && x!=w && y!=z && y!=w && z!=w;} // if all components are different
   Int    minI        (         )C {return MinI(x, y, z, w);}                             // components minimum index
   Int    maxI        (         )C {return MaxI(x, y, z, w);}                             // components maximum index
   Int    min         (         )C {return Min (x, y, z, w);}                             // components minimum
   Int    max         (         )C {return Max (x, y, z, w);}                             // components maximum
   Int    sum         (         )C {return      x+ y+ z+ w ;}                             // components sum
   Int    find        (Int value)C;                                                       // get index of first component that equals 'value' (-1 if none)
   VecB4& reverse     (         ) {Swap(c[0], c[3]); Swap(c[1], c[2]); return T;}         // reverse components order

   Str asTextDots()C; // return as text with components separated using dots "x.y.z.w"

   VecB4() {}
   VecB4(Byte i                        ) {set(i         );}
   VecB4(Byte x, Byte y, Byte z, Byte w) {set(x, y, z, w);}
};
/******************************************************************************/
struct VecSB4 // Vector 4D (SByte)
{
   union
   {
      struct{SByte  x, y, z, w;};
      struct{SByte  c[4]      ;}; // component
      struct{UInt   u         ;};
      struct{VecSB2 xy, zw    ;};
      struct{VecSB  xyz       ;};
   };

   VecSB4& zero(                                  ) {u=0;                        return T;}
   VecSB4& set (SByte i                           ) {x=y=z=w=i;                  return T;}
   VecSB4& set (SByte x, SByte y, SByte z, SByte w) {T.x=x; T.y=y; T.z=z; T.w=w; return T;}

   Bool operator==(C VecSB4 &v)C {return u==v.u;}
   Bool operator!=(C VecSB4 &v)C {return u!=v.u;}

   Bool    any         (         )C {return u!=0            ;}                             // if any component  is  non-zero, faster version of "x || y || z || w"
   Bool    all         (         )C {return x && y && z && w;}                             // if all components are non-zero
   Bool    allZero     (         )C {return u==0;}                                         // if all components are     zero
   Bool    allDifferent(         )C {return x!=y && x!=z && x!=w && y!=z && y!=w && z!=w;} // if all components are different
   Int     minI        (         )C {return MinI(x, y, z, w);}                             // components minimum index
   Int     maxI        (         )C {return MaxI(x, y, z, w);}                             // components maximum index
   Int     min         (         )C {return Min (x, y, z, w);}                             // components minimum
   Int     max         (         )C {return Max (x, y, z, w);}                             // components maximum
   Int     sum         (         )C {return      x+ y+ z+ w ;}                             // components sum
   Int     find        (Int value)C;                                                       // get index of first component that equals 'value' (-1 if none)
   VecSB4& reverse     (         ) {Swap(c[0], c[3]); Swap(c[1], c[2]); return T;}         // reverse components order

   VecSB4() {}
   VecSB4(SByte i                           ) {set(i         );}
   VecSB4(SByte x, SByte y, SByte z, SByte w) {set(x, y, z, w);}
};
/******************************************************************************/
struct VecUS2 // Vector 2D (Unsigned Short)
{
   union
   {
      struct{UShort x, y;};
      struct{UShort c[2];}; // component
   };

   VecUS2& zero(                  ) {x=y=0;        return T;}
   VecUS2& set (UShort u          ) {x=y=u;        return T;}
   VecUS2& set (UShort x, UShort y) {T.x=x; T.y=y; return T;}

#if EE_PRIVATE
   VecUS2& remap   (C        Int  *map) {if(map){x=map[x]; y=map[y];} return T;} // remap if map is provided
   VecUS2& remapAll(C MemPtr<Int> &map) {x=(InRange(x, map) ? map[x] : -1); y=(InRange(y, map) ? map[y] : -1); return T;} // remap all components
   VecUS2& remapFit(C MemPtr<Int> &map) {if(InRange(x, map))x=map[x];       if(InRange(y, map))y=map[y];       return T;} // remap     components which are in range of the remap array
#endif

   VecUS2& operator+=(Int i) {x+=i; y+=i; return T;}
   VecUS2& operator-=(Int i) {x-=i; y-=i; return T;}
   VecUS2& operator*=(Int i) {x*=i; y*=i; return T;}
   VecUS2& operator/=(Int i) {x/=i; y/=i; return T;}
   VecUS2& operator%=(Int i) {x%=i; y%=i; return T;}

   Bool operator==(C VecUS2 &v)C;
   Bool operator!=(C VecUS2 &v)C;
   Bool operator==(C VecB2  &v)C;
   Bool operator!=(C VecB2  &v)C;
   Bool operator==(C VecI2  &v)C;
   Bool operator!=(C VecI2  &v)C;

   friend VecUS2 operator+ (C VecUS2 &v, Int i) {return VecUS2(v.x+i, v.y+i);}
   friend VecUS2 operator- (C VecUS2 &v, Int i) {return VecUS2(v.x-i, v.y-i);}
   friend VecUS2 operator* (C VecUS2 &v, Int i) {return VecUS2(v.x*i, v.y*i);}
   friend VecUS2 operator/ (C VecUS2 &v, Int i) {return VecUS2(v.x/i, v.y/i);}
   friend VecUS2 operator% (C VecUS2 &v, Int i) {return VecUS2(v.x%i, v.y%i);}

   friend Vec2 operator+ (C VecUS2 &v, Flt f) {return Vec2(v.x+f, v.y+f);}
   friend Vec2 operator- (C VecUS2 &v, Flt f) {return Vec2(v.x-f, v.y-f);}
   friend Vec2 operator* (C VecUS2 &v, Flt f) {return Vec2(v.x*f, v.y*f);}
   friend Vec2 operator/ (C VecUS2 &v, Flt f) {return Vec2(v.x/f, v.y/f);}

   friend VecUS2 operator+ (Int i, C VecUS2 &v) {return VecUS2(i+v.x, i+v.y);}
   friend VecUS2 operator- (Int i, C VecUS2 &v) {return VecUS2(i-v.x, i-v.y);}
   friend VecUS2 operator* (Int i, C VecUS2 &v) {return VecUS2(i*v.x, i*v.y);}
   friend VecUS2 operator/ (Int i, C VecUS2 &v) {return VecUS2(i/v.x, i/v.y);}

   friend Vec2 operator+ (Flt f, C VecUS2 &v) {return Vec2(f+v.x, f+v.y);}
   friend Vec2 operator- (Flt f, C VecUS2 &v) {return Vec2(f-v.x, f-v.y);}
   friend Vec2 operator* (Flt f, C VecUS2 &v) {return Vec2(f*v.x, f*v.y);}
   friend Vec2 operator/ (Flt f, C VecUS2 &v) {return Vec2(f/v.x, f/v.y);}

   Bool any()C {return x || y;} // if any component is non-zero

   VecUS2() {}
   VecUS2(UShort u          ) {set(u   );}
   VecUS2(UShort x, UShort y) {set(x, y);}
   VecUS2(C VecB2 &v);
   VecUS2(C VecI2 &v);
};
struct VecUS // Vector 3D (Unsigned Short)
{
   union
   {
      struct{UShort x, y, z;};
      struct{UShort c[3]   ;}; // component
      struct{VecUS2 xy     ;};
   };

   VecUS& zero(                            ) {x=y=z=0;             return T;}
   VecUS& set (UShort u                    ) {x=y=z=u;             return T;}
   VecUS& set (UShort x, UShort y, UShort z) {T.x=x; T.y=y; T.z=z; return T;}
   VecUS& set (C VecUS2 &xy      , UShort z) {T.xy=xy;      T.z=z; return T;}

#if EE_PRIVATE
   VecUS& remap   (C        Int  *map) {if(map){x=map[x]; y=map[y]; z=map[z];} return T;} // remap if map is provided
   VecUS& remapAll(C MemPtr<Int> &map) {x=(InRange(x, map) ? map[x] : -1); y=(InRange(y, map) ? map[y] : -1); z=(InRange(z, map) ? map[z] : -1); return T;} // remap all components
   VecUS& remapFit(C MemPtr<Int> &map) {if(InRange(x, map))x=map[x];       if(InRange(y, map))y=map[y];       if(InRange(z, map))z=map[z];       return T;} // remap     components which are in range of the remap array
#endif

   VecUS& operator+=(Int i) {x+=i; y+=i; z+=i; return T;}
   VecUS& operator-=(Int i) {x-=i; y-=i; z-=i; return T;}
   VecUS& operator*=(Int i) {x*=i; y*=i; z*=i; return T;}
   VecUS& operator/=(Int i) {x/=i; y/=i; z/=i; return T;}
   VecUS& operator%=(Int i) {x%=i; y%=i; z%=i; return T;}

   Bool operator==(C VecUS &v)C;
   Bool operator!=(C VecUS &v)C;
   Bool operator==(C VecB  &v)C;
   Bool operator!=(C VecB  &v)C;
   Bool operator==(C VecI  &v)C;
   Bool operator!=(C VecI  &v)C;

   friend VecUS operator+ (C VecUS &v, Int i) {return VecUS(v.x+i, v.y+i, v.z+i);}
   friend VecUS operator- (C VecUS &v, Int i) {return VecUS(v.x-i, v.y-i, v.z-i);}
   friend VecUS operator* (C VecUS &v, Int i) {return VecUS(v.x*i, v.y*i, v.z*i);}
   friend VecUS operator/ (C VecUS &v, Int i) {return VecUS(v.x/i, v.y/i, v.z/i);}
   friend VecUS operator% (C VecUS &v, Int i) {return VecUS(v.x%i, v.y%i, v.z%i);}

   friend Vec operator+ (C VecUS &v, Flt f) {return Vec(v.x+f, v.y+f, v.z+f);}
   friend Vec operator- (C VecUS &v, Flt f) {return Vec(v.x-f, v.y-f, v.z-f);}
   friend Vec operator* (C VecUS &v, Flt f) {return Vec(v.x*f, v.y*f, v.z*f);}
   friend Vec operator/ (C VecUS &v, Flt f) {return Vec(v.x/f, v.y/f, v.z/f);}

   friend VecUS operator+ (Int i, C VecUS &v) {return VecUS(i+v.x, i+v.y, i+v.z);}
   friend VecUS operator- (Int i, C VecUS &v) {return VecUS(i-v.x, i-v.y, i-v.z);}
   friend VecUS operator* (Int i, C VecUS &v) {return VecUS(i*v.x, i*v.y, i*v.z);}
   friend VecUS operator/ (Int i, C VecUS &v) {return VecUS(i/v.x, i/v.y, i/v.z);}

   friend Vec operator+ (Flt f, C VecUS &v) {return Vec(f+v.x, f+v.y, f+v.z);}
   friend Vec operator- (Flt f, C VecUS &v) {return Vec(f-v.x, f-v.y, f-v.z);}
   friend Vec operator* (Flt f, C VecUS &v) {return Vec(f*v.x, f*v.y, f*v.z);}
   friend Vec operator/ (Flt f, C VecUS &v) {return Vec(f/v.x, f/v.y, f/v.z);}

   Bool any()C {return x || y || z;} // if any component is non-zero

   VecUS() {}
   VecUS(UShort u                    ) {set(u      );}
   VecUS(UShort x, UShort y, UShort z) {set(x, y, z);}
   VecUS(C VecUS2 &xy      , UShort z) {set( xy , z);}
   VecUS(C VecB &v);
   VecUS(C VecI &v);
};
/******************************************************************************/
struct VecH2 // Vector 2D (Half)
{
   union
   {
      struct{Half x, y;};
      struct{Half c[2];}; // component
   };

   VecH2& set(  Flt   f           ) {x=y=f;        return T;}
   VecH2& set(C Half &h           ) {x=y=h;        return T;}
   VecH2& set(C Half &x, C Half &y) {T.x=x; T.y=y; return T;}

   Bool any()C {return Any(x, y);} // if any component is non-zero

   Str asText(Int precision=INT_MAX)C; // return as text

   VecH2() {}
   VecH2(  Flt   f           ) {set(f       );}
   VecH2(C Half &h           ) {set(h       );}
   VecH2(C Half &x, C Half &y) {set(  x,   y);}
   VecH2(C Vec2 &v           ) {set(v.x, v.y);}
};
struct VecH // Vector 3D (Half)
{
   union
   {
      struct{Half  x, y, z;};
      struct{Half  c[3]   ;}; // component
      struct{VecH2 xy     ;};
   };

   VecH& set(  Flt    f                      ) {x=y=z=f;             return T;}
   VecH& set(C Half  &h                      ) {x=y=z=h;             return T;}
   VecH& set(C Half  &x, C Half &y, C Half &z) {T.x=x; T.y=y; T.z=z; return T;}
   VecH& set(C VecH2 &xy          , C Half &z) {T.xy=xy;      T.z=z; return T;}

   Bool any()C {return Any(x, y, z);} // if any component is non-zero

   Str asText(Int precision=INT_MAX)C; // return as text

   VecH() {}
   VecH(  Flt    f                      ) {set(f            );}
   VecH(C Half  &h                      ) {set(h            );}
   VecH(C Half  &x, C Half &y, C Half &z) {set(  x,   y,   z);}
   VecH(C VecH2 &xy          , C Half &z) {set(  xy    ,   z);}
   VecH(C Vec   &v                      ) {set(v.x, v.y, v.z);}
};
struct VecH4 // Vector 4D (Half)
{
   union
   {
      struct{Half  x, y, z, w;};
      struct{Half  c[4]      ;}; // component
      struct{VecH2 xy, zw    ;};
      struct{VecH  xyz       ;};
   };

   VecH4& set(  Flt    f                                 ) {x=y=z=w=f;                  return T;}
   VecH4& set(C Half  &h                                 ) {x=y=z=w=h;                  return T;}
   VecH4& set(C Half  &x, C Half &y, C Half &z, C Half &w) {T.x=x; T.y=y; T.z=z; T.w=w; return T;}
   VecH4& set(C VecH2 &xy          , C Half &z, C Half &w) {T.xy =xy ;    T.z=z; T.w=w; return T;}
   VecH4& set(C VecH  &xyz                    , C Half &w) {T.xyz=xyz;           T.w=w; return T;}
   VecH4& set(C VecH2 &xy          , C VecH2 &zw         ) {T.xy =xy ;    T.zw=zw;      return T;}

   Bool any()C {return Any(x, y, z, w);} // if any component is non-zero

   Str asText(Int precision=INT_MAX)C; // return as text

   VecH4() {}
   VecH4(  Flt   f                                 ) {set(f                 );}
   VecH4(C Half &h                                 ) {set(h                 );}
   VecH4(C Half &x, C Half &y, C Half &z, C Half &w) {set(  x,   y,   z,   w);}
   VecH4(C VecH2  &xy        , C Half &z, C Half &w) {set(xy      ,   z,   w);}
   VecH4(C VecH   &xyz                  , C Half &w) {set(xyz          ,   w);}
   VecH4(C VecH2  &xy        , C VecH2 &zw         ) {set(xy      ,  zw     );}
   VecH4(C Vec4 &v                                 ) {set(v.x, v.y, v.z, v.w);}
};
/******************************************************************************/
struct VecI2 // Vector 2D (integer)
{
   union
   {
      struct{Int x, y;};
      struct{Int c[2];}; // component
   };

   VecI2& zero(            ) {x=y=0;        return T;}
   VecI2& set (Int i       ) {x=y=i;        return T;}
   VecI2& set (Int x, Int y) {T.x=x; T.y=y; return T;}

   VecI2& operator ++(  Int      ) {x++   ; y++   ; return T;}
   VecI2& operator --(  Int      ) {x--   ; y--   ; return T;}
   VecI2& operator +=(  Int     i) {x+=  i; y+=  i; return T;}
   VecI2& operator -=(  Int     i) {x-=  i; y-=  i; return T;}
   VecI2& operator *=(  Int     i) {x*=  i; y*=  i; return T;}
   VecI2& operator /=(  Int     i) {x/=  i; y/=  i; return T;}
   VecI2& operator %=(  Int     i) {x%=  i; y%=  i; return T;}
   VecI2& operator +=(C VecI2  &v) {x+=v.x; y+=v.y; return T;}
   VecI2& operator +=(C VecSB2 &v) {x+=v.x; y+=v.y; return T;}
   VecI2& operator +=(C VecB2  &v) {x+=v.x; y+=v.y; return T;}
   VecI2& operator -=(C VecI2  &v) {x-=v.x; y-=v.y; return T;}
   VecI2& operator -=(C VecSB2 &v) {x-=v.x; y-=v.y; return T;}
   VecI2& operator -=(C VecB2  &v) {x-=v.x; y-=v.y; return T;}
   VecI2& operator *=(C VecI2  &v) {x*=v.x; y*=v.y; return T;}
   VecI2& operator /=(C VecI2  &v) {x/=v.x; y/=v.y; return T;}
   VecI2& operator %=(C VecI2  &v) {x%=v.x; y%=v.y; return T;}
   VecI2& operator<<=(  Int     i) {x<<= i; y<<= i; return T;}
   VecI2& operator>>=(  Int     i) {x>>= i; y>>= i; return T;}
   VecI2& operator &=(C RectI  &r); // intersect

   Bool operator==(C VecI2  &v)C {return x==v.x && y==v.y;}
   Bool operator!=(C VecI2  &v)C {return x!=v.x || y!=v.y;}
   Bool operator==(C VecB2  &v)C {return x==v.x && y==v.y;}
   Bool operator!=(C VecB2  &v)C {return x!=v.x || y!=v.y;}
   Bool operator==(C VecUS2 &v)C {return x==v.x && y==v.y;}
   Bool operator!=(C VecUS2 &v)C {return x!=v.x || y!=v.y;}

   friend VecI2 operator+ (C VecI2 &v, Int i) {return VecI2(v.x+i, v.y+i);}
   friend VecI2 operator- (C VecI2 &v, Int i) {return VecI2(v.x-i, v.y-i);}
   friend VecI2 operator* (C VecI2 &v, Int i) {return VecI2(v.x*i, v.y*i);}
   friend VecI2 operator/ (C VecI2 &v, Int i) {return VecI2(v.x/i, v.y/i);}
   friend VecI2 operator% (C VecI2 &v, Int i) {return VecI2(v.x%i, v.y%i);}

   friend Vec2 operator+ (C VecI2 &v, Flt f) {return Vec2(v.x+f, v.y+f);}
   friend Vec2 operator- (C VecI2 &v, Flt f) {return Vec2(v.x-f, v.y-f);}
   friend Vec2 operator* (C VecI2 &v, Flt f) {return Vec2(v.x*f, v.y*f);}
   friend Vec2 operator/ (C VecI2 &v, Flt f) {return Vec2(v.x/f, v.y/f);}

   friend VecD2 operator+ (C VecI2 &v, Dbl f) {return VecD2(v.x+f, v.y+f);}
   friend VecD2 operator- (C VecI2 &v, Dbl f) {return VecD2(v.x-f, v.y-f);}
   friend VecD2 operator* (C VecI2 &v, Dbl f) {return VecD2(v.x*f, v.y*f);}
   friend VecD2 operator/ (C VecI2 &v, Dbl f) {return VecD2(v.x/f, v.y/f);}

   friend VecI2 operator+ (Int i, C VecI2 &v) {return VecI2(i+v.x, i+v.y);}
   friend VecI2 operator- (Int i, C VecI2 &v) {return VecI2(i-v.x, i-v.y);}
   friend VecI2 operator* (Int i, C VecI2 &v) {return VecI2(i*v.x, i*v.y);}
   friend VecI2 operator/ (Int i, C VecI2 &v) {return VecI2(i/v.x, i/v.y);}

   friend Vec2 operator+ (Flt f, C VecI2 &v) {return Vec2(f+v.x, f+v.y);}
   friend Vec2 operator- (Flt f, C VecI2 &v) {return Vec2(f-v.x, f-v.y);}
   friend Vec2 operator* (Flt f, C VecI2 &v) {return Vec2(f*v.x, f*v.y);}
   friend Vec2 operator/ (Flt f, C VecI2 &v) {return Vec2(f/v.x, f/v.y);}

   friend VecD2 operator+ (Dbl f, C VecI2 &v) {return VecD2(f+v.x, f+v.y);}
   friend VecD2 operator- (Dbl f, C VecI2 &v) {return VecD2(f-v.x, f-v.y);}
   friend VecD2 operator* (Dbl f, C VecI2 &v) {return VecD2(f*v.x, f*v.y);}
   friend VecD2 operator/ (Dbl f, C VecI2 &v) {return VecD2(f/v.x, f/v.y);}

   friend VecI2 operator+ (C VecI2 &a, C VecI2 &b) {return VecI2(a.x+b.x, a.y+b.y);}
   friend VecI2 operator- (C VecI2 &a, C VecI2 &b) {return VecI2(a.x-b.x, a.y-b.y);}
   friend VecI2 operator* (C VecI2 &a, C VecI2 &b) {return VecI2(a.x*b.x, a.y*b.y);}
   friend VecI2 operator/ (C VecI2 &a, C VecI2 &b) {return VecI2(a.x/b.x, a.y/b.y);}
   friend VecI2 operator% (C VecI2 &a, C VecI2 &b) {return VecI2(a.x%b.x, a.y%b.y);}

   friend VecI2 operator+ (C VecI2 &a, C VecSB2 &b) {return VecI2(a.x+b.x, a.y+b.y);}
   friend VecI2 operator- (C VecI2 &a, C VecSB2 &b) {return VecI2(a.x-b.x, a.y-b.y);}
   friend VecI2 operator* (C VecI2 &a, C VecSB2 &b) {return VecI2(a.x*b.x, a.y*b.y);}
   friend VecI2 operator/ (C VecI2 &a, C VecSB2 &b) {return VecI2(a.x/b.x, a.y/b.y);}
   friend VecI2 operator% (C VecI2 &a, C VecSB2 &b) {return VecI2(a.x%b.x, a.y%b.y);}

   friend VecI2 operator+ (C VecSB2 &a, C VecI2 &b) {return VecI2(a.x+b.x, a.y+b.y);}
   friend VecI2 operator- (C VecSB2 &a, C VecI2 &b) {return VecI2(a.x-b.x, a.y-b.y);}
   friend VecI2 operator* (C VecSB2 &a, C VecI2 &b) {return VecI2(a.x*b.x, a.y*b.y);}
   friend VecI2 operator/ (C VecSB2 &a, C VecI2 &b) {return VecI2(a.x/b.x, a.y/b.y);}
   friend VecI2 operator% (C VecSB2 &a, C VecI2 &b) {return VecI2(a.x%b.x, a.y%b.y);}

   friend VecI2 operator+ (C VecI2 &a, C VecB2 &b) {return VecI2(a.x+b.x, a.y+b.y);}
   friend VecI2 operator- (C VecI2 &a, C VecB2 &b) {return VecI2(a.x-b.x, a.y-b.y);}
   friend VecI2 operator* (C VecI2 &a, C VecB2 &b) {return VecI2(a.x*b.x, a.y*b.y);}
   friend VecI2 operator/ (C VecI2 &a, C VecB2 &b) {return VecI2(a.x/b.x, a.y/b.y);}
   friend VecI2 operator% (C VecI2 &a, C VecB2 &b) {return VecI2(a.x%b.x, a.y%b.y);}

   friend VecI2 operator+ (C VecB2 &a, C VecI2 &b) {return VecI2(a.x+b.x, a.y+b.y);}
   friend VecI2 operator- (C VecB2 &a, C VecI2 &b) {return VecI2(a.x-b.x, a.y-b.y);}
   friend VecI2 operator* (C VecB2 &a, C VecI2 &b) {return VecI2(a.x*b.x, a.y*b.y);}
   friend VecI2 operator/ (C VecB2 &a, C VecI2 &b) {return VecI2(a.x/b.x, a.y/b.y);}
   friend VecI2 operator% (C VecB2 &a, C VecI2 &b) {return VecI2(a.x%b.x, a.y%b.y);}

   friend Vec2 operator+ (C VecI2 &a, C Vec2 &b) {return Vec2(a.x+b.x, a.y+b.y);}
   friend Vec2 operator- (C VecI2 &a, C Vec2 &b) {return Vec2(a.x-b.x, a.y-b.y);}
   friend Vec2 operator* (C VecI2 &a, C Vec2 &b) {return Vec2(a.x*b.x, a.y*b.y);}
   friend Vec2 operator/ (C VecI2 &a, C Vec2 &b) {return Vec2(a.x/b.x, a.y/b.y);}

   friend Vec2 operator+ (C Vec2 &a, C VecI2 &b) {return Vec2(a.x+b.x, a.y+b.y);}
   friend Vec2 operator- (C Vec2 &a, C VecI2 &b) {return Vec2(a.x-b.x, a.y-b.y);}
   friend Vec2 operator* (C Vec2 &a, C VecI2 &b) {return Vec2(a.x*b.x, a.y*b.y);}
   friend Vec2 operator/ (C Vec2 &a, C VecI2 &b) {return Vec2(a.x/b.x, a.y/b.y);}

   friend VecI2 operator<< (C VecI2 &v,   Int    i) {return VecI2(v.x<<i, v.y<<i);}
   friend VecI2 operator>> (C VecI2 &v,   Int    i) {return VecI2(v.x>>i, v.y>>i);}
   friend VecI2 operator&  (C VecI2 &v, C RectI &r) {return VecI2(v)&=r;} // intersection
   friend VecI2 operator-  (C VecI2 &v            ) {return VecI2(-v.x, -v.y);}

   VecI  xy0()C; // return as VecI(x, y, 0)
   VecI  x0y()C; // return as VecI(x, 0, y)
   VecI _0xy()C; // return as VecI(0, x, y)
   VecI _0yx()C; // return as VecI(0, y, x)

   Bool   any         (         )C {return    x ||  y;}         // if any  component  is  non-zero
   Bool   all         (         )C {return    x &&  y;}         // if all  components are non-zero
   Bool   allZero     (         )C {return   !x && !y;}         // if all  components are     zero
   Bool   allDifferent(         )C {return      x!=y ;}         // if all  components are different
   Int    minI        (         )C {return MinI(x, y);}         // components minimum index
   Int    maxI        (         )C {return MaxI(x, y);}         // components maximum index
   Int    min         (         )C {return Min (x, y);}         // components minimum
   Int    max         (         )C {return Max (x, y);}         // components maximum
   Int    avgI        (         )C {return AvgI(x, y);}         // components average
   Flt    avgF        (         )C {return AvgF(x, y);}         // components average
   Int    sum         (         )C {return      x+ y ;}         // components sum
   Int    mul         (         )C {return      x* y ;}         // components multiplication
   Flt    length      (         )C;                             // get         length
   Int    length2     (         )C {return  x*x + y*y;}         // get squared length
   Int    find        (Int value)C;                             // get index of first component that equals 'value' (-1 if none)
   VecI2& swap        (         ) {Swap(c[0], c[1]); return T;} // swap    components
   VecI2& reverse     (         ) {Swap(c[0], c[1]); return T;} // reverse components order
   VecI2& rotateOrder (         ) {Swap(c[0], c[1]); return T;} // rotate  components order
   VecI2& chs         (         );                              // change sign of all components
   VecI2& abs         (         );                              // absolute       all components
   VecI2& sat         (         );                              // saturate       all components

#if EE_PRIVATE
   VecI2& remap   (C        Int  *map) {if(map){x=map[x]; y=map[y];} return T;} // remap if map is provided
   VecI2& remapAll(C MemPtr<Int> &map) {x=(InRange(x, map) ? map[x] : -1); y=(InRange(y, map) ? map[y] : -1); return T;} // remap all components
   VecI2& remapFit(C MemPtr<Int> &map) {if(InRange(x, map))x=map[x];       if(InRange(y, map))y=map[y];       return T;} // remap     components which are in range of the remap array

   VecI2 asIso  ()C {return VecI2(x+y  , y-x  );}
   VecI2 asIso2 ()C {return VecI2(x*2+y, y*2-x);}
   VecI2 asIso_2()C {return VecI2(x+y*2, y-x*2);}
#endif

   VecI2() {}
   VecI2(Int i       ) {set(i       );}
   VecI2(Int x, Int y) {set(  x,   y);}
   VecI2(C VecB2  &v ) {set(v.x, v.y);}
   VecI2(C VecSB2 &v ) {set(v.x, v.y);}
   VecI2(C VecUS2 &v ) {set(v.x, v.y);}
};
/******************************************************************************/
struct VecI // Vector 3D (integer)
{
   union
   {
      struct{Int   x, y, z;};
      struct{Int   c[3]   ;}; // component
      struct{VecI2 xy     ;};
   };

   VecI& zero(                   ) {x=y=z=0;             return T;}
   VecI& set (Int i              ) {x=y=z=i;             return T;}
   VecI& set (Int x, Int y, Int z) {T.x=x; T.y=y; T.z=z; return T;}
   VecI& set (C VecI2 &xy , Int z) {T.xy=xy;      T.z=z; return T;}

   VecI& operator ++(  Int     ) {x++   ; y++   ; z++   ; return T;}
   VecI& operator --(  Int     ) {x--   ; y--   ; z--   ; return T;}
   VecI& operator +=(  Int    i) {x+=  i; y+=  i; z+=  i; return T;}
   VecI& operator -=(  Int    i) {x-=  i; y-=  i; z-=  i; return T;}
   VecI& operator *=(  Int    i) {x*=  i; y*=  i; z*=  i; return T;}
   VecI& operator /=(  Int    i) {x/=  i; y/=  i; z/=  i; return T;}
   VecI& operator %=(  Int    i) {x%=  i; y%=  i; z%=  i; return T;}
   VecI& operator +=(C VecI  &v) {x+=v.x; y+=v.y; z+=v.z; return T;}
   VecI& operator +=(C VecSB &v) {x+=v.x; y+=v.y; z+=v.z; return T;}
   VecI& operator +=(C VecB  &v) {x+=v.x; y+=v.y; z+=v.z; return T;}
   VecI& operator -=(C VecI  &v) {x-=v.x; y-=v.y; z-=v.z; return T;}
   VecI& operator -=(C VecSB &v) {x-=v.x; y-=v.y; z-=v.z; return T;}
   VecI& operator -=(C VecB  &v) {x-=v.x; y-=v.y; z-=v.z; return T;}
   VecI& operator *=(C VecI  &v) {x*=v.x; y*=v.y; z*=v.z; return T;}
   VecI& operator /=(C VecI  &v) {x/=v.x; y/=v.y; z/=v.z; return T;}
   VecI& operator %=(C VecI  &v) {x%=v.x; y%=v.y; z%=v.z; return T;}
   VecI& operator<<=(  Int    i) {x<<= i; y<<= i; z<<= i; return T;}
   VecI& operator>>=(  Int    i) {x>>= i; y>>= i; z>>= i; return T;}
   VecI& operator &=(C BoxI  &b); // intersect

   Bool operator==(C VecI  &v)C {return x==v.x && y==v.y && z==v.z;}
   Bool operator!=(C VecI  &v)C {return x!=v.x || y!=v.y || z!=v.z;}
   Bool operator==(C VecB  &v)C {return x==v.x && y==v.y && z==v.z;}
   Bool operator!=(C VecB  &v)C {return x!=v.x || y!=v.y || z!=v.z;}
   Bool operator==(C VecUS &v)C {return x==v.x && y==v.y && z==v.z;}
   Bool operator!=(C VecUS &v)C {return x!=v.x || y!=v.y || z!=v.z;}

   friend VecI operator+ (C VecI &v, Int i) {return VecI(v.x+i, v.y+i, v.z+i);}
   friend VecI operator- (C VecI &v, Int i) {return VecI(v.x-i, v.y-i, v.z-i);}
   friend VecI operator* (C VecI &v, Int i) {return VecI(v.x*i, v.y*i, v.z*i);}
   friend VecI operator/ (C VecI &v, Int i) {return VecI(v.x/i, v.y/i, v.z/i);}
   friend VecI operator% (C VecI &v, Int i) {return VecI(v.x%i, v.y%i, v.z%i);}

   friend Vec operator+ (C VecI &v, Flt f) {return Vec(v.x+f, v.y+f, v.z+f);}
   friend Vec operator- (C VecI &v, Flt f) {return Vec(v.x-f, v.y-f, v.z-f);}
   friend Vec operator* (C VecI &v, Flt f) {return Vec(v.x*f, v.y*f, v.z*f);}
   friend Vec operator/ (C VecI &v, Flt f) {return Vec(v.x/f, v.y/f, v.z/f);}

   friend VecD operator+ (C VecI &v, Dbl f) {return VecD(v.x+f, v.y+f, v.z+f);}
   friend VecD operator- (C VecI &v, Dbl f) {return VecD(v.x-f, v.y-f, v.z-f);}
   friend VecD operator* (C VecI &v, Dbl f) {return VecD(v.x*f, v.y*f, v.z*f);}
   friend VecD operator/ (C VecI &v, Dbl f) {return VecD(v.x/f, v.y/f, v.z/f);}

   friend VecI operator+ (Int i, C VecI &v) {return VecI(i+v.x, i+v.y, i+v.z);}
   friend VecI operator- (Int i, C VecI &v) {return VecI(i-v.x, i-v.y, i-v.z);}
   friend VecI operator* (Int i, C VecI &v) {return VecI(i*v.x, i*v.y, i*v.z);}
   friend VecI operator/ (Int i, C VecI &v) {return VecI(i/v.x, i/v.y, i/v.z);}

   friend Vec operator+ (Flt f, C VecI &v) {return Vec(f+v.x, f+v.y, f+v.z);}
   friend Vec operator- (Flt f, C VecI &v) {return Vec(f-v.x, f-v.y, f-v.z);}
   friend Vec operator* (Flt f, C VecI &v) {return Vec(f*v.x, f*v.y, f*v.z);}
   friend Vec operator/ (Flt f, C VecI &v) {return Vec(f/v.x, f/v.y, f/v.z);}

   friend VecD operator+ (Dbl f, C VecI &v) {return VecD(f+v.x, f+v.y, f+v.z);}
   friend VecD operator- (Dbl f, C VecI &v) {return VecD(f-v.x, f-v.y, f-v.z);}
   friend VecD operator* (Dbl f, C VecI &v) {return VecD(f*v.x, f*v.y, f*v.z);}
   friend VecD operator/ (Dbl f, C VecI &v) {return VecD(f/v.x, f/v.y, f/v.z);}

   friend VecI operator+ (C VecI &a, C VecI &b) {return VecI(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecI operator- (C VecI &a, C VecI &b) {return VecI(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecI operator* (C VecI &a, C VecI &b) {return VecI(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecI operator/ (C VecI &a, C VecI &b) {return VecI(a.x/b.x, a.y/b.y, a.z/b.z);}
   friend VecI operator% (C VecI &a, C VecI &b) {return VecI(a.x%b.x, a.y%b.y, a.z%b.z);}

   friend VecI operator+ (C VecI &a, C VecSB &b) {return VecI(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecI operator- (C VecI &a, C VecSB &b) {return VecI(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecI operator* (C VecI &a, C VecSB &b) {return VecI(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecI operator/ (C VecI &a, C VecSB &b) {return VecI(a.x/b.x, a.y/b.y, a.z/b.z);}
   friend VecI operator% (C VecI &a, C VecSB &b) {return VecI(a.x%b.x, a.y%b.y, a.z%b.z);}

   friend VecI operator+ (C VecSB &a, C VecI &b) {return VecI(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecI operator- (C VecSB &a, C VecI &b) {return VecI(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecI operator* (C VecSB &a, C VecI &b) {return VecI(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecI operator/ (C VecSB &a, C VecI &b) {return VecI(a.x/b.x, a.y/b.y, a.z/b.z);}
   friend VecI operator% (C VecSB &a, C VecI &b) {return VecI(a.x%b.x, a.y%b.y, a.z%b.z);}

   friend VecI operator+ (C VecI &a, C VecB &b) {return VecI(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecI operator- (C VecI &a, C VecB &b) {return VecI(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecI operator* (C VecI &a, C VecB &b) {return VecI(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecI operator/ (C VecI &a, C VecB &b) {return VecI(a.x/b.x, a.y/b.y, a.z/b.z);}
   friend VecI operator% (C VecI &a, C VecB &b) {return VecI(a.x%b.x, a.y%b.y, a.z%b.z);}

   friend VecI operator+ (C VecB &a, C VecI &b) {return VecI(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend VecI operator- (C VecB &a, C VecI &b) {return VecI(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend VecI operator* (C VecB &a, C VecI &b) {return VecI(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend VecI operator/ (C VecB &a, C VecI &b) {return VecI(a.x/b.x, a.y/b.y, a.z/b.z);}
   friend VecI operator% (C VecB &a, C VecI &b) {return VecI(a.x%b.x, a.y%b.y, a.z%b.z);}

   friend Vec operator+ (C VecI &a, C Vec &b) {return Vec(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend Vec operator- (C VecI &a, C Vec &b) {return Vec(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend Vec operator* (C VecI &a, C Vec &b) {return Vec(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend Vec operator/ (C VecI &a, C Vec &b) {return Vec(a.x/b.x, a.y/b.y, a.z/b.z);}

   friend Vec operator+ (C Vec &a, C VecI &b) {return Vec(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend Vec operator- (C Vec &a, C VecI &b) {return Vec(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend Vec operator* (C Vec &a, C VecI &b) {return Vec(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend Vec operator/ (C Vec &a, C VecI &b) {return Vec(a.x/b.x, a.y/b.y, a.z/b.z);}

   friend VecI operator<< (C VecI &v,   Int   i) {return VecI(v.x<<i, v.y<<i, v.z<<i);}
   friend VecI operator>> (C VecI &v,   Int   i) {return VecI(v.x>>i, v.y>>i, v.z>>i);}
   friend VecI operator&  (C VecI &v, C BoxI &b) {return VecI(v)&=b;} // intersection
   friend VecI operator-  (C VecI &v           ) {return VecI(-v.x, -v.y, -v.z);}

   VecI2 xz ()C {return VecI2(x, z   );} // return as VecI2(x, z)
   VecI2 yx ()C {return VecI2(y, x   );} // return as VecI2(y, x)
   VecI2 yz ()C {return VecI2(y, z   );} // return as VecI2(y, z)
   VecI2 zx ()C {return VecI2(z, x   );} // return as VecI2(z, x)
   VecI2 zy ()C {return VecI2(z, y   );} // return as VecI2(z, y)
   VecI  xzy()C {return VecI (x, z, y);} // return as VecI (x, z, y)
   VecI  x0z()C {return VecI (x, 0, z);} // return as VecI (x, 0, z)
   VecI  xy0()C {return VecI (x, y, 0);} // return as VecI (x, y, 0)

   Bool  any         (         )C {return  x ||  y ||  z;}       // if any  component  is  non-zero
   Bool  all         (         )C {return  x &&  y &&  z;}       // if all  components are non-zero
   Bool  allZero     (         )C {return !x && !y && !z;}       // if all  components are     zero
   Bool  allDifferent(         )C {return x!=y && x!=z && y!=z;} // if all  components are different
   Int   minI        (         )C {return MinI(x, y, z);}        // components minimum index
   Int   maxI        (         )C {return MaxI(x, y, z);}        // components maximum index
   Int   min         (         )C {return Min (x, y, z);}        // components minimum
   Int   max         (         )C {return Max (x, y, z);}        // components maximum
   Int   avgI        (         )C {return AvgI(x, y, z);}        // components average
   Flt   avgF        (         )C {return AvgF(x, y, z);}        // components average
   Int   sum         (         )C {return      x+ y+ z ;}        // components sum
   Int   mul         (         )C {return      x* y* z ;}        // components multiplication
   Flt   length      (         )C;                               // get         length
   Int   length2     (         )C {return x*x + y*y + z*z;}      // get squared length
   Int   find        (Int value)C;                               // get index of first component that equals 'value' (-1 if none)
   VecI& swapXZ      (         ) {Swap(c[0], c[2]); return T;}   // swap X Z components
   VecI& reverse     (         ) {Swap(c[0], c[2]); return T;}   // reverse  components order
   VecI& rotateOrder (         );                                // rotate   components order
   VecI& chs         (         );                                // change sign of all components
   VecI& abs         (         );                                // absolute       all components
   VecI& sat         (         );                                // saturate       all components

#if EE_PRIVATE
   VecI& remap   (C        Int  *map) {if(map){x=map[x]; y=map[y]; z=map[z];} return T;} // remap if map is provided
   VecI& remapAll(C MemPtr<Int> &map) {x=(InRange(x, map) ? map[x] : -1); y=(InRange(y, map) ? map[y] : -1); z=(InRange(z, map) ? map[z] : -1); return T;} // remap all components
   VecI& remapFit(C MemPtr<Int> &map) {if(InRange(x, map))x=map[x];       if(InRange(y, map))y=map[y];       if(InRange(z, map))z=map[z];       return T;} // remap     components which are in range of the remap array
#endif

   VecI() {}
   VecI(Int i              ) {set(i            );}
   VecI(Int x, Int y, Int z) {set(  x,   y,   z);}
   VecI(C VecI2 &xy , Int z) {set(xy      ,   z);}
   VecI(C VecB  &v         ) {set(v.x, v.y, v.z);}
   VecI(C VecSB &v         ) {set(v.x, v.y, v.z);}
   VecI(C VecUS &v         ) {set(v.x, v.y, v.z);}
};
/******************************************************************************/
struct VecI4 // Vector 4D (integer)
{
   union
   {
      struct{Int   x, y, z, w;};
      struct{Int   c[4]      ;}; // component
      struct{VecI2 xy, zw    ;};
      struct{VecI  xyz       ;};
   };

   VecI4& zero(                          ) {x=y=z=w=0;                  return T;}
   VecI4& set (Int i                     ) {x=y=z=w=i;                  return T;}
   VecI4& set (Int x, Int y, Int z, Int w) {T.x=x; T.y=y; T.z=z; T.w=w; return T;}
   VecI4& set (C VecI2 &xy , Int z, Int w) {T.xy =xy ;    T.z=z; T.w=w; return T;}
   VecI4& set (C VecI  &xyz,        Int w) {T.xyz=xyz;           T.w=w; return T;}
   VecI4& set (C VecI2 &xy , C VecI2 &zw ) {T.xy =xy ;    T.zw=zw;      return T;}

   VecI4& operator ++(  Int      ) {x++   ; y++   ; z++   ; w++   ; return T;}
   VecI4& operator --(  Int      ) {x--   ; y--   ; z--   ; w--   ; return T;}
   VecI4& operator +=(  Int     i) {x+=  i; y+=  i; z+=  i; w+=  i; return T;}
   VecI4& operator -=(  Int     i) {x-=  i; y-=  i; z-=  i; w-=  i; return T;}
   VecI4& operator *=(  Int     i) {x*=  i; y*=  i; z*=  i; w*=  i; return T;}
   VecI4& operator /=(  Int     i) {x/=  i; y/=  i; z/=  i; w/=  i; return T;}
   VecI4& operator %=(  Int     i) {x%=  i; y%=  i; z%=  i; w%=  i; return T;}
   VecI4& operator +=(C VecI4  &v) {x+=v.x; y+=v.y; z+=v.z; w+=v.w; return T;}
   VecI4& operator +=(C VecSB4 &v) {x+=v.x; y+=v.y; z+=v.z; w+=v.w; return T;}
   VecI4& operator +=(C VecB4  &v) {x+=v.x; y+=v.y; z+=v.z; w+=v.w; return T;}
   VecI4& operator -=(C VecI4  &v) {x-=v.x; y-=v.y; z-=v.z; w-=v.w; return T;}
   VecI4& operator -=(C VecSB4 &v) {x-=v.x; y-=v.y; z-=v.z; w-=v.w; return T;}
   VecI4& operator -=(C VecB4  &v) {x-=v.x; y-=v.y; z-=v.z; w-=v.w; return T;}
   VecI4& operator *=(C VecI4  &v) {x*=v.x; y*=v.y; z*=v.z; w*=v.w; return T;}
   VecI4& operator /=(C VecI4  &v) {x/=v.x; y/=v.y; z/=v.z; w/=v.w; return T;}
   VecI4& operator %=(C VecI4  &v) {x%=v.x; y%=v.y; z%=v.z; w%=v.w; return T;}
   VecI4& operator<<=(  Int     i) {x<<= i; y<<= i; z<<= i; w<<= i; return T;}
   VecI4& operator>>=(  Int     i) {x>>= i; y>>= i; z>>= i; w>>= i; return T;}
   Bool   operator ==(C VecI4  &v)C{return x==v.x && y==v.y && z==v.z && w==v.w;}
   Bool   operator !=(C VecI4  &v)C{return x!=v.x || y!=v.y || z!=v.z || w!=v.w;}

   friend VecI4 operator+ (C VecI4 &v, Int i) {return VecI4(v.x+i, v.y+i, v.z+i, v.w+i);}
   friend VecI4 operator- (C VecI4 &v, Int i) {return VecI4(v.x-i, v.y-i, v.z-i, v.w-i);}
   friend VecI4 operator* (C VecI4 &v, Int i) {return VecI4(v.x*i, v.y*i, v.z*i, v.w*i);}
   friend VecI4 operator/ (C VecI4 &v, Int i) {return VecI4(v.x/i, v.y/i, v.z/i, v.w/i);}
   friend VecI4 operator% (C VecI4 &v, Int i) {return VecI4(v.x%i, v.y%i, v.z%i, v.w%i);}

   friend Vec4 operator+ (C VecI4 &v, Flt f) {return Vec4(v.x+f, v.y+f, v.z+f, v.w+f);}
   friend Vec4 operator- (C VecI4 &v, Flt f) {return Vec4(v.x-f, v.y-f, v.z-f, v.w-f);}
   friend Vec4 operator* (C VecI4 &v, Flt f) {return Vec4(v.x*f, v.y*f, v.z*f, v.w*f);}
   friend Vec4 operator/ (C VecI4 &v, Flt f) {return Vec4(v.x/f, v.y/f, v.z/f, v.w/f);}

   friend VecD4 operator+ (C VecI4 &v, Dbl f) {return VecD4(v.x+f, v.y+f, v.z+f, v.w+f);}
   friend VecD4 operator- (C VecI4 &v, Dbl f) {return VecD4(v.x-f, v.y-f, v.z-f, v.w-f);}
   friend VecD4 operator* (C VecI4 &v, Dbl f) {return VecD4(v.x*f, v.y*f, v.z*f, v.w*f);}
   friend VecD4 operator/ (C VecI4 &v, Dbl f) {return VecD4(v.x/f, v.y/f, v.z/f, v.w/f);}

   friend VecI4 operator+ (Int i, C VecI4 &v) {return VecI4(i+v.x, i+v.y, i+v.z, i+v.w);}
   friend VecI4 operator- (Int i, C VecI4 &v) {return VecI4(i-v.x, i-v.y, i-v.z, i-v.w);}
   friend VecI4 operator* (Int i, C VecI4 &v) {return VecI4(i*v.x, i*v.y, i*v.z, i*v.w);}
   friend VecI4 operator/ (Int i, C VecI4 &v) {return VecI4(i/v.x, i/v.y, i/v.z, i/v.w);}

   friend Vec4 operator+ (Flt f, C VecI4 &v) {return Vec4(f+v.x, f+v.y, f+v.z, f+v.w);}
   friend Vec4 operator- (Flt f, C VecI4 &v) {return Vec4(f-v.x, f-v.y, f-v.z, f-v.w);}
   friend Vec4 operator* (Flt f, C VecI4 &v) {return Vec4(f*v.x, f*v.y, f*v.z, f*v.w);}
   friend Vec4 operator/ (Flt f, C VecI4 &v) {return Vec4(f/v.x, f/v.y, f/v.z, f/v.w);}

   friend VecD4 operator+ (Dbl f, C VecI4 &v) {return VecD4(f+v.x, f+v.y, f+v.z, f+v.w);}
   friend VecD4 operator- (Dbl f, C VecI4 &v) {return VecD4(f-v.x, f-v.y, f-v.z, f-v.w);}
   friend VecD4 operator* (Dbl f, C VecI4 &v) {return VecD4(f*v.x, f*v.y, f*v.z, f*v.w);}
   friend VecD4 operator/ (Dbl f, C VecI4 &v) {return VecD4(f/v.x, f/v.y, f/v.z, f/v.w);}

   friend VecI4 operator+ (C VecI4 &a, C VecI4 &b) {return VecI4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend VecI4 operator- (C VecI4 &a, C VecI4 &b) {return VecI4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend VecI4 operator* (C VecI4 &a, C VecI4 &b) {return VecI4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend VecI4 operator/ (C VecI4 &a, C VecI4 &b) {return VecI4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}
   friend VecI4 operator% (C VecI4 &a, C VecI4 &b) {return VecI4(a.x%b.x, a.y%b.y, a.z%b.z, a.w%b.w);}

   friend VecI4 operator+ (C VecI4 &a, C VecSB4 &b) {return VecI4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend VecI4 operator- (C VecI4 &a, C VecSB4 &b) {return VecI4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend VecI4 operator* (C VecI4 &a, C VecSB4 &b) {return VecI4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend VecI4 operator/ (C VecI4 &a, C VecSB4 &b) {return VecI4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}
   friend VecI4 operator% (C VecI4 &a, C VecSB4 &b) {return VecI4(a.x%b.x, a.y%b.y, a.z%b.z, a.w%b.w);}

   friend VecI4 operator+ (C VecSB4 &a, C VecI4 &b) {return VecI4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend VecI4 operator- (C VecSB4 &a, C VecI4 &b) {return VecI4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend VecI4 operator* (C VecSB4 &a, C VecI4 &b) {return VecI4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend VecI4 operator/ (C VecSB4 &a, C VecI4 &b) {return VecI4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}
   friend VecI4 operator% (C VecSB4 &a, C VecI4 &b) {return VecI4(a.x%b.x, a.y%b.y, a.z%b.z, a.w%b.w);}

   friend VecI4 operator+ (C VecI4 &a, C VecB4 &b) {return VecI4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend VecI4 operator- (C VecI4 &a, C VecB4 &b) {return VecI4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend VecI4 operator* (C VecI4 &a, C VecB4 &b) {return VecI4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend VecI4 operator/ (C VecI4 &a, C VecB4 &b) {return VecI4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}
   friend VecI4 operator% (C VecI4 &a, C VecB4 &b) {return VecI4(a.x%b.x, a.y%b.y, a.z%b.z, a.w%b.w);}

   friend VecI4 operator+ (C VecB4 &a, C VecI4 &b) {return VecI4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend VecI4 operator- (C VecB4 &a, C VecI4 &b) {return VecI4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend VecI4 operator* (C VecB4 &a, C VecI4 &b) {return VecI4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend VecI4 operator/ (C VecB4 &a, C VecI4 &b) {return VecI4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}
   friend VecI4 operator% (C VecB4 &a, C VecI4 &b) {return VecI4(a.x%b.x, a.y%b.y, a.z%b.z, a.w%b.w);}

   friend Vec4 operator+ (C VecI4 &a, C Vec4 &b) {return Vec4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend Vec4 operator- (C VecI4 &a, C Vec4 &b) {return Vec4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend Vec4 operator* (C VecI4 &a, C Vec4 &b) {return Vec4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend Vec4 operator/ (C VecI4 &a, C Vec4 &b) {return Vec4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}

   friend Vec4 operator + (C Vec4 &a, C VecI4 &b) {return Vec4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);}
   friend Vec4 operator - (C Vec4 &a, C VecI4 &b) {return Vec4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);}
   friend Vec4 operator * (C Vec4 &a, C VecI4 &b) {return Vec4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);}
   friend Vec4 operator / (C Vec4 &a, C VecI4 &b) {return Vec4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);}

   friend VecI4 operator<< (C VecI4 &v, Int i) {return VecI4(v.x<<i, v.y<<i, v.z<<i, v.w<<i);}
   friend VecI4 operator>> (C VecI4 &v, Int i) {return VecI4(v.x>>i, v.y>>i, v.z>>i, v.w>>i);}
   friend VecI4 operator - (C VecI4 &v       ) {return VecI4(-v.x, -v.y, -v.z, -v.w);}

   Bool   any         (         )C {return  x ||  y ||  z ||  w;}                         // if any  component  is  non-zero
   Bool   all         (         )C {return  x &&  y &&  z &&  w;}                         // if all  components are non-zero
   Bool   allZero     (         )C {return !x && !y && !z && !w;}                         // if all  components are     zero
   Bool   allDifferent(         )C {return x!=y && x!=z && x!=w && y!=z && y!=w && z!=w;} // if all  components are different
   Int    minI        (         )C {return  MinI(x, y, z, w);}                            // components minimum index
   Int    maxI        (         )C {return  MaxI(x, y, z, w);}                            // components maximum index
   Int    min         (         )C {return  Min (x, y, z, w);}                            // components minimum
   Int    max         (         )C {return  Max (x, y, z, w);}                            // components maximum
   Int    avgI        (         )C {return  AvgI(x, y, z, w);}                            // components average
   Flt    avgF        (         )C {return  AvgF(x, y, z, w);}                            // components average
   Int    sum         (         )C {return       x+ y+ z+ w ;}                            // components sum
   Int    mul         (         )C {return       x* y* z* w ;}                            // components multiplication
   Flt    length      (         )C;                                                       // get         length
   Int    length2     (         )C {return x*x + y*y + z*z + w*w;}                        // get squared length
   Int    find        (Int value)C;                                                       // get index of first component that equals 'value' (-1 if none)
   VecI4& swapXZ      (         ) {Swap(c[0], c[2]);                   return T;}         // swap X Z components
   VecI4& reverse     (         ) {Swap(c[0], c[3]); Swap(c[1], c[2]); return T;}         // reverse  components order
   VecI4& rotateOrder (         );                                                        // rotate   components order
   VecI4& chs         (         );                                                        // change sign of all components
   VecI4& abs         (         );                                                        // absolute       all components
   VecI4& sat         (         );                                                        // saturate       all components

#if EE_PRIVATE
   VecI4& remap   (C        Int  *map) {if(map){x=map[x]; y=map[y]; z=map[z]; w=map[w];} return T;} // remap if map is provided
   VecI4& remapAll(C MemPtr<Int> &map) {x=(InRange(x, map) ? map[x] : -1); y=(InRange(y, map) ? map[y] : -1); z=(InRange(z, map) ? map[z] : -1); w=(InRange(w, map) ? map[w] : -1); return T;} // remap all components
   VecI4& remapFit(C MemPtr<Int> &map) {if(InRange(x, map))x=map[x];       if(InRange(y, map))y=map[y];       if(InRange(z, map))z=map[z];       if(InRange(w, map))w=map[w];       return T;} // remap     components which are in range of the remap array
#endif

   VecI tri0()C {return VecI(x, y, w);}
   VecI tri1()C {return VecI(w, y, z);}

   Str asTextDots()C; // return as text with components separated using dots "x.y.z.w"

   VecI4() {}
   VecI4(Int i                     ) {set(i                 );}
   VecI4(Int x, Int y, Int z, Int w) {set(  x,   y,   z,   w);}
   VecI4(C VecI2 &xy , Int z, Int w) {set(xy      ,   z,   w);}
   VecI4(C VecI  &xyz,        Int w) {set(xyz          ,   w);}
   VecI4(C VecI2 &xy , C VecI2 &zw ) {set(xy      , zw      );}
   VecI4(C VecB4  &v               ) {set(v.x, v.y, v.z, v.w);}
   VecI4(C VecSB4 &v               ) {set(v.x, v.y, v.z, v.w);}
};
/******************************************************************************/
// ROUNDING
/******************************************************************************/
// truncate, remove fractional part, Sample Usage: Trunc(7.3) -> 7, Trunc(7.9) -> 7
#if EE_PRIVATE
       Dbl   TruncD (  Dbl    x);                   // truncate and return as Dbl
#endif
inline Int   Trunc  (  Int    x) {return       x ;} // truncate and return as Int
inline Int   Trunc  (  Flt    x) {return Int  (x);} // truncate and return as Int
inline Int   Trunc  (  Dbl    x) {return Int  (x);} // truncate and return as Int
inline UInt  TruncU (  UInt   x) {return       x ;} // truncate and return as UInt
inline UInt  TruncU (  Flt    x) {return UInt (x);} // truncate and return as UInt
inline UInt  TruncU (  Dbl    x) {return UInt (x);} // truncate and return as UInt
inline Long  TruncL (  Dbl    x) {return Long (x);} // truncate and return as Long
inline ULong TruncUL(  Dbl    x) {return ULong(x);} // truncate and return as ULong
inline VecI2 Trunc  (C Vec2  &v) {return VecI2(Trunc(v.x), Trunc(v.y)                        );}
inline VecI2 Trunc  (C VecD2 &v) {return VecI2(Trunc(v.x), Trunc(v.y)                        );}
inline VecI  Trunc  (C Vec   &v) {return VecI (Trunc(v.x), Trunc(v.y), Trunc(v.z)            );}
inline VecI  Trunc  (C VecD  &v) {return VecI (Trunc(v.x), Trunc(v.y), Trunc(v.z)            );}
inline VecI4 Trunc  (C Vec4  &v) {return VecI4(Trunc(v.x), Trunc(v.y), Trunc(v.z), Trunc(v.w));}
inline VecI4 Trunc  (C VecD4 &v) {return VecI4(Trunc(v.x), Trunc(v.y), Trunc(v.z), Trunc(v.w));}

// round, round to nearest integer, Sample Usage: Round(7.3) -> 7, Round(7.9) -> 8
inline Int   Round  (  Int    x) {return x;}
inline Int   Round  (  Flt    x) {return (x>=0) ? Trunc  (x+0.5f) : Trunc (x-0.5f);} // faster than 'lroundf'
inline Int   Round  (  Dbl    x) {return (x>=0) ? Trunc  (x+0.5 ) : Trunc (x-0.5 );} // faster than 'lround'
inline UInt  RoundU (  UInt   x) {return x;}
inline UInt  RoundU (  Flt    x) {return          TruncU (x+0.5f);}
inline UInt  RoundU (  Dbl    x) {return          TruncU (x+0.5 );}
inline Long  RoundL (  Dbl    x) {return (x>=0) ? TruncL (x+0.5 ) : TruncL(x-0.5 );}
inline ULong RoundUL(  Dbl    x) {return          TruncUL(x+0.5 );}
inline VecI2 Round  (C Vec2  &x) {return VecI2(Round(x.x), Round(x.y)                        );}
inline VecI2 Round  (C VecD2 &x) {return VecI2(Round(x.x), Round(x.y)                        );}
inline VecI  Round  (C Vec   &x) {return VecI (Round(x.x), Round(x.y), Round(x.z)            );}
inline VecI  Round  (C VecD  &x) {return VecI (Round(x.x), Round(x.y), Round(x.z)            );}
inline VecI4 Round  (C Vec4  &x) {return VecI4(Round(x.x), Round(x.y), Round(x.z), Round(x.w));}
inline VecI4 Round  (C VecD4 &x) {return VecI4(Round(x.x), Round(x.y), Round(x.z), Round(x.w));}
#if EE_PRIVATE
inline Int   RoundEps(  Flt   x, Flt eps) {return (x>=0) ? Trunc(x+eps) : Trunc(x-eps);}
inline Int   RoundPos(  Flt   x) {return Trunc(x+0.5f);} // doesn't care if round of negative value will not be precise, but unlike 'RoundU' the result will still be negative
inline Int   RoundPos(  Dbl   x) {return Trunc(x+0.5 );} // doesn't care if round of negative value will not be precise, but unlike 'RoundU' the result will still be negative
inline Int   RoundGPU(  Flt   x) {return Round(x-0.0001f);} // if the coordinate is located exactly between 2 pixels "Frac(x)==0.5" then due to numerical precision issues sometimes this can be rounded up and sometimes down, and flickering can occur for example when window is moved on the screen, to prevent that, apply a small offset, the value "0.0001f" has been tested having a Window and a button at 0.5 coordinates, then moving the window around the screen and noticing when does it stop flickering, keep as "-offset" instead of "+offset" because it works better with clipping (for example if Region draws a pixel border and it is located exactly between 2 pixels, then its children may overlap the border because the clipping has 1 extra pixel)
inline VecI2 RoundPos(C Vec2 &x) {return VecI2(RoundPos(x.x), RoundPos(x.y)               );}
inline VecI  RoundPos(C Vec  &x) {return VecI (RoundPos(x.x), RoundPos(x.y), RoundPos(x.z));}
inline VecI2 RoundGPU(C Vec2 &v) {return VecI2(RoundGPU(v.x), RoundGPU(v.y)               );}
#endif

// floor, round to nearest integer which is smaller or equal to value, Sample Usage: Floor(7.3) -> 7, Floor(7.9) -> 7
inline Int   Floor (  Int    x) {return x;}
inline Int   Floor (  Flt    x) {return floorf(x);}
inline Int   Floor (  Dbl    x) {return floor (x);}
inline Long  FloorL(  Dbl    x) {return floor (x);}
inline VecI2 Floor (C Vec2  &x) {return VecI2(Floor(x.x), Floor(x.y)                        );}
inline VecI2 Floor (C VecD2 &x) {return VecI2(Floor(x.x), Floor(x.y)                        );}
inline VecI  Floor (C Vec   &x) {return VecI (Floor(x.x), Floor(x.y), Floor(x.z)            );}
inline VecI  Floor (C VecD  &x) {return VecI (Floor(x.x), Floor(x.y), Floor(x.z)            );}
inline VecI4 Floor (C Vec4  &x) {return VecI4(Floor(x.x), Floor(x.y), Floor(x.z), Floor(x.w));}
inline VecI4 Floor (C VecD4 &x) {return VecI4(Floor(x.x), Floor(x.y), Floor(x.z), Floor(x.w));}

// ceil, round to nearest integer which is greater or equal to value, Sample Usage: Ceil(7.3) -> 8, Ceil(7.9) -> 8
inline Int   Ceil (  Int    x) {return x;}
inline Int   Ceil (  Flt    x) {return ceilf(x);}
inline Int   Ceil (  Dbl    x) {return ceil (x);}
inline Long  CeilL(  Dbl    x) {return ceil (x);}
inline VecI2 Ceil (C Vec2  &x) {return VecI2(Ceil(x.x), Ceil(x.y)                      );}
inline VecI2 Ceil (C VecD2 &x) {return VecI2(Ceil(x.x), Ceil(x.y)                      );}
inline VecI  Ceil (C Vec   &x) {return VecI (Ceil(x.x), Ceil(x.y), Ceil(x.z)           );}
inline VecI  Ceil (C VecD  &x) {return VecI (Ceil(x.x), Ceil(x.y), Ceil(x.z)           );}
inline VecI4 Ceil (C Vec4  &x) {return VecI4(Ceil(x.x), Ceil(x.y), Ceil(x.z), Ceil(x.w));}
inline VecI4 Ceil (C VecD4 &x) {return VecI4(Ceil(x.x), Ceil(x.y), Ceil(x.z), Ceil(x.w));}

#if EE_PRIVATE
inline Int FloorSpecial(Flt x) {return Ceil (x-1);} // this works in a similar way to "Floor(x)" however if a value falls exactly on integer (which means that fraction==0) then previous integer is chosen
inline Int  CeilSpecial(Flt x) {return Floor(x+1);} // this works in a similar way to "Ceil (x)" however if a value falls exactly on integer (which means that fraction==0) then next     integer is chosen
#endif

// get fraction, gets fractional part of a real value, Sample Usage: Frac(7.3) -> 0.3, Frac(7.9) -> 0.9
inline Flt  Frac(  Flt   x) {return x-floorf(x);} // [0..1), use 'floorf' instead of 'Floor' to avoid conversion to Int (faster this way)
inline Dbl  Frac(  Dbl   x) {return x-floor (x);} // [0..1), use 'floor'  instead of 'Floor' to avoid conversion to Int (faster this way)
inline Vec2 Frac(C Vec2 &v) {return Vec2(Frac(v.x), Frac(v.y)                      );}
inline Vec  Frac(C Vec  &v) {return Vec (Frac(v.x), Frac(v.y), Frac(v.z)           );}
inline Vec4 Frac(C Vec4 &v) {return Vec4(Frac(v.x), Frac(v.y), Frac(v.z), Frac(v.w));}

inline Flt FracS(Flt x) {return x-Trunc(x);} // (-1..1) (sign preserving)
inline Dbl FracS(Dbl x) {return x-Trunc(x);} // (-1..1) (sign preserving)

inline Flt Frac (Flt x, Flt range) {return Frac (x/range)*range;} // [     0..range)
inline Dbl Frac (Dbl x, Dbl range) {return Frac (x/range)*range;} // [     0..range)
inline Flt FracS(Flt x, Flt range) {return FracS(x/range)*range;} // (-range..range) (sign preserving)
inline Dbl FracS(Dbl x, Dbl range) {return FracS(x/range)*range;} // (-range..range) (sign preserving)

// align
inline Int AlignTrunc(Flt x, Int align) {return Trunc (x/align)*align;} // align 'x' to nearest multiple of 'align' using truncation
inline Flt AlignTrunc(Flt x, Flt align) {return Trunc (x/align)*align;} // align 'x' to nearest multiple of 'align' using truncation
inline Dbl AlignTrunc(Dbl x, Dbl align) {return Trunc (x/align)*align;} // align 'x' to nearest multiple of 'align' using truncation
inline Int AlignRound(Flt x, Int align) {return Round (x/align)*align;} // align 'x' to nearest multiple of 'align' using rounding
inline Flt AlignRound(Flt x, Flt align) {return Round (x/align)*align;} // align 'x' to nearest multiple of 'align' using rounding
inline Dbl AlignRound(Dbl x, Dbl align) {return Round (x/align)*align;} // align 'x' to nearest multiple of 'align' using rounding
inline Int AlignFloor(Flt x, Int align) {return Floor (x/align)*align;} // align 'x' to nearest multiple of 'align' using floor
inline Flt AlignFloor(Flt x, Flt align) {return floorf(x/align)*align;} // align 'x' to nearest multiple of 'align' using floor, use 'floorf' instead of 'Floor' to avoid conversion to Int (faster this way)
inline Dbl AlignFloor(Dbl x, Dbl align) {return floor (x/align)*align;} // align 'x' to nearest multiple of 'align' using floor, use 'floor'  instead of 'Floor' to avoid conversion to Int (faster this way)
inline Int AlignCeil (Flt x, Int align) {return Ceil  (x/align)*align;} // align 'x' to nearest multiple of 'align' using ceil
inline Flt AlignCeil (Flt x, Flt align) {return ceilf (x/align)*align;} // align 'x' to nearest multiple of 'align' using ceil , use 'ceilf'  instead of 'Ceil'  to avoid conversion to Int (faster this way)
inline Dbl AlignCeil (Dbl x, Dbl align) {return ceil  (x/align)*align;} // align 'x' to nearest multiple of 'align' using ceil , use 'ceil'   instead of 'Ceil'  to avoid conversion to Int (faster this way)
/******************************************************************************/
// FUNCTIONS
/******************************************************************************/
inline VecUS2::VecUS2(C VecB2  &v) {set(v.x, v.y          );}
inline VecUS2::VecUS2(C VecI2  &v) {set(v.x, v.y          );}
inline VecUS ::VecUS (C VecB   &v) {set(v.x, v.y, v.z     );}
inline VecUS ::VecUS (C VecI   &v) {set(v.x, v.y, v.z     );}
inline Vec2  ::Vec2  (C VecH2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecD2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecI2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecB2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecSB2 &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecUS2 &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecH2  &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C Vec2   &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecI2  &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecB2  &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecSB2 &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecUS2 &v) {set(v.x, v.y          );}
inline Vec   ::Vec   (C VecH   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecD   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecI   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecB   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecSB  &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecH   &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C Vec    &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecI   &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecB   &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecSB  &v) {set(v.x, v.y, v.z     );}
inline Vec4  ::Vec4  (C VecH4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecD4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecI4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecB4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecSB4 &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecH4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C Vec4   &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecI4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecB4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecSB4 &v) {set(v.x, v.y, v.z, v.w);}

inline Vec  Vec2 :: xy0()C {return Vec (x, y, 0);}
inline Vec  Vec2 :: x0y()C {return Vec (x, 0, y);}
inline Vec  Vec2 ::_0xy()C {return Vec (0, x, y);}
inline Vec  Vec2 ::_0yx()C {return Vec (0, y, x);}
inline VecD VecD2:: xy0()C {return VecD(x, y, 0);}
inline VecD VecD2:: x0y()C {return VecD(x, 0, y);}
inline VecD VecD2::_0xy()C {return VecD(0, x, y);}
inline VecD VecD2::_0yx()C {return VecD(0, y, x);}
inline VecI VecI2:: xy0()C {return VecI(x, y, 0);}
inline VecI VecI2:: x0y()C {return VecI(x, 0, y);}
inline VecI VecI2::_0xy()C {return VecI(0, x, y);}
inline VecI VecI2::_0yx()C {return VecI(0, y, x);}

inline Bool VecUS2::operator==(C VecUS2 &v)C {return x==v.x && y==v.y;}
inline Bool VecUS2::operator!=(C VecUS2 &v)C {return x!=v.x || y!=v.y;}
inline Bool VecUS2::operator==(C VecB2  &v)C {return x==v.x && y==v.y;}
inline Bool VecUS2::operator!=(C VecB2  &v)C {return x!=v.x || y!=v.y;}
inline Bool VecUS2::operator==(C VecI2  &v)C {return x==v.x && y==v.y;}
inline Bool VecUS2::operator!=(C VecI2  &v)C {return x!=v.x || y!=v.y;}

inline Bool VecUS::operator==(C VecUS &v)C {return x==v.x && y==v.y && z==v.z;}
inline Bool VecUS::operator!=(C VecUS &v)C {return x!=v.x || y!=v.y || z!=v.z;}
inline Bool VecUS::operator==(C VecB  &v)C {return x==v.x && y==v.y && z==v.z;}
inline Bool VecUS::operator!=(C VecB  &v)C {return x!=v.x || y!=v.y || z!=v.z;}
inline Bool VecUS::operator==(C VecI  &v)C {return x==v.x && y==v.y && z==v.z;}
inline Bool VecUS::operator!=(C VecI  &v)C {return x!=v.x || y!=v.y || z!=v.z;}

inline VecD operator+ (C Vec &v, Dbl r) {return VecD(v.x+r, v.y+r, v.z+r);}
inline VecD operator- (C Vec &v, Dbl r) {return VecD(v.x-r, v.y-r, v.z-r);}
inline VecD operator* (C Vec &v, Dbl r) {return VecD(v.x*r, v.y*r, v.z*r);}
inline VecD operator/ (C Vec &v, Dbl r) {return VecD(v.x/r, v.y/r, v.z/r);}

inline VecD operator+ (Dbl r, C Vec &v) {return VecD(r+v.x, r+v.y, r+v.z);}
inline VecD operator- (Dbl r, C Vec &v) {return VecD(r-v.x, r-v.y, r-v.z);}
inline VecD operator* (Dbl r, C Vec &v) {return VecD(r*v.x, r*v.y, r*v.z);}
inline VecD operator/ (Dbl r, C Vec &v) {return VecD(r/v.x, r/v.y, r/v.z);}

inline VecD operator* (C Vec &v, C OrientD &o) {return VecD(v)*=o;}
inline VecD operator* (C Vec &v, C OrientM &o) {return VecD(v)*=o;}

inline VecD operator* (C Vec &v, C MatrixD3 &m) {return VecD(v)*=m;}
inline VecD operator* (C Vec &v, C MatrixM  &m) {return VecD(v)*=m;}
inline VecD operator* (C Vec &v, C MatrixD  &m) {return VecD(v)*=m;}
inline VecD operator/ (C Vec &v, C MatrixD3 &m) {return VecD(v)/=m;}
inline VecD operator/ (C Vec &v, C MatrixM  &m) {return VecD(v)/=m;}
inline VecD operator/ (C Vec &v, C MatrixD  &m) {return VecD(v)/=m;}

// minimum & maximum
inline Vec2  Min(C Vec2  &a, C Vec2  &b) {return Vec2 (Min(a.x,b.x), Min(a.y,b.y)                            );}
inline VecI2 Min(C VecI2 &a, C VecI2 &b) {return VecI2(Min(a.x,b.x), Min(a.y,b.y)                            );}
inline Vec2  Max(C Vec2  &a, C Vec2  &b) {return Vec2 (Max(a.x,b.x), Max(a.y,b.y)                            );}
inline VecI2 Max(C VecI2 &a, C VecI2 &b) {return VecI2(Max(a.x,b.x), Max(a.y,b.y)                            );}
inline Vec   Min(C Vec   &a, C Vec   &b) {return Vec  (Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z)              );}
inline VecI  Min(C VecI  &a, C VecI  &b) {return VecI (Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z)              );}
inline Vec   Max(C Vec   &a, C Vec   &b) {return Vec  (Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z)              );}
inline VecI  Max(C VecI  &a, C VecI  &b) {return VecI (Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z)              );}
inline Vec4  Min(C Vec4  &a, C Vec4  &b) {return Vec4 (Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z), Min(a.w,b.w));}
inline VecI4 Min(C VecI4 &a, C VecI4 &b) {return VecI4(Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z), Min(a.w,b.w));}
inline Vec4  Max(C Vec4  &a, C Vec4  &b) {return Vec4 (Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z), Max(a.w,b.w));}
inline VecI4 Max(C VecI4 &a, C VecI4 &b) {return VecI4(Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z), Max(a.w,b.w));}
       VecB4 Min(C VecB4 &a, C VecB4 &b);
       VecB4 Max(C VecB4 &a, C VecB4 &b);
       VecB4 Min(C VecB4 &a, C VecB4 &b, C VecB4 &c);
       VecB4 Max(C VecB4 &a, C VecB4 &b, C VecB4 &c);
       VecB4 Min(C VecB4 &a, C VecB4 &b, C VecB4 &c, C VecB4 &d);
       VecB4 Max(C VecB4 &a, C VecB4 &b, C VecB4 &c, C VecB4 &d);

// average
inline Vec2  Avg(C Vec2  &a, C Vec2  &b                        ) {return (a+b    )*0.5f ;}
inline VecD2 Avg(C VecD2 &a, C VecD2 &b                        ) {return (a+b    )*0.5  ;}
inline Vec   Avg(C Vec   &a, C Vec   &b                        ) {return (a+b    )*0.5f ;}
inline VecD  Avg(C VecD  &a, C VecD  &b                        ) {return (a+b    )*0.5  ;}
inline Vec4  Avg(C Vec4  &a, C Vec4  &b                        ) {return (a+b    )*0.5f ;}
inline VecD4 Avg(C VecD4 &a, C VecD4 &b                        ) {return (a+b    )*0.5  ;}
inline Vec2  Avg(C Vec2  &a, C Vec2  &b, C Vec2  &c            ) {return (a+b+c  )/3.0f ;}
inline VecD2 Avg(C VecD2 &a, C VecD2 &b, C VecD2 &c            ) {return (a+b+c  )/3.0  ;}
inline Vec   Avg(C Vec   &a, C Vec   &b, C Vec   &c            ) {return (a+b+c  )/3.0f ;}
inline VecD  Avg(C VecD  &a, C VecD  &b, C VecD  &c            ) {return (a+b+c  )/3.0  ;}
inline Vec2  Avg(C Vec2  &a, C Vec2  &b, C Vec2  &c, C Vec2  &d) {return (a+b+c+d)*0.25f;}
inline VecD2 Avg(C VecD2 &a, C VecD2 &b, C VecD2 &c, C VecD2 &d) {return (a+b+c+d)*0.25 ;}
inline Vec   Avg(C Vec   &a, C Vec   &b, C Vec   &c, C Vec   &d) {return (a+b+c+d)*0.25f;}
inline VecD  Avg(C VecD  &a, C VecD  &b, C VecD  &c, C VecD  &d) {return (a+b+c+d)*0.25 ;}

VecB4 AvgI(C VecB4 &a, C VecB4 &b                        );
VecB4 AvgI(C VecB4 &a, C VecB4 &b, C VecB4 &c            );
VecB4 AvgI(C VecB4 &a, C VecB4 &b, C VecB4 &c, C VecB4 &d);

VecI2 AvgI(C VecI2 &a, C VecI2 &b);
VecI  AvgI(C VecI  &a, C VecI  &b);
VecI4 AvgI(C VecI4 &a, C VecI4 &b);
Vec2  AvgF(C VecI2 &a, C VecI2 &b);
Vec   AvgF(C VecI  &a, C VecI  &b);
Vec4  AvgF(C VecI4 &a, C VecI4 &b);

// distance between 2 points
Flt Dist(C Vec2  &a, C Vec2  &b);
Flt Dist(C VecI2 &a, C Vec2  &b);
Flt Dist(C Vec2  &a, C VecI2 &b);
Dbl Dist(C VecD2 &a, C VecD2 &b);
Flt Dist(C VecI2 &a, C VecI2 &b);
Flt Dist(C Vec   &a, C Vec   &b);
Dbl Dist(C VecD  &a, C Vec   &b);
Dbl Dist(C Vec   &a, C VecD  &b);
Dbl Dist(C VecD  &a, C VecD  &b);
Flt Dist(C VecI  &a, C VecI  &b);

// squared distance between 2 points
Flt Dist2(C Vec2  &a, C Vec2  &b);
Flt Dist2(C VecI2 &a, C Vec2  &b);
Flt Dist2(C Vec2  &a, C VecI2 &b);
Dbl Dist2(C VecD2 &a, C VecD2 &b);
Int Dist2(C VecI2 &a, C VecI2 &b);
Flt Dist2(C Vec   &a, C Vec   &b);
Dbl Dist2(C VecD  &a, C Vec   &b);
Dbl Dist2(C Vec   &a, C VecD  &b);
Dbl Dist2(C VecD  &a, C VecD  &b);
Int Dist2(C VecI  &a, C VecI  &b);
Flt Dist2(C Vec4  &a, C Vec4  &b);

// dot product
inline Flt Dot(C Vec2  &a, C Vec2  &b) {return a.x*b.x + a.y*b.y                    ;}
inline Flt Dot(C Vec2  &a, C VecI2 &b) {return a.x*b.x + a.y*b.y                    ;}
inline Flt Dot(C VecI2 &a, C Vec2  &b) {return a.x*b.x + a.y*b.y                    ;}
inline Dbl Dot(C VecD2 &a, C VecD2 &b) {return a.x*b.x + a.y*b.y                    ;}
inline Int Dot(C VecI2 &a, C VecI2 &b) {return a.x*b.x + a.y*b.y                    ;}
inline Flt Dot(C Vec   &a, C Vec   &b) {return a.x*b.x + a.y*b.y + a.z*b.z          ;}
inline Dbl Dot(C Vec   &a, C VecD  &b) {return a.x*b.x + a.y*b.y + a.z*b.z          ;}
inline Dbl Dot(C VecD  &a, C Vec   &b) {return a.x*b.x + a.y*b.y + a.z*b.z          ;}
inline Dbl Dot(C VecD  &a, C VecD  &b) {return a.x*b.x + a.y*b.y + a.z*b.z          ;}
inline Int Dot(C VecI  &a, C VecI  &b) {return a.x*b.x + a.y*b.y + a.z*b.z          ;}
inline Flt Dot(C Vec4  &a, C Vec4  &b) {return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;}
inline Dbl Dot(C VecD4 &a, C VecD4 &b) {return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;}
inline Int Dot(C VecI4 &a, C VecI4 &b) {return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;}

// cross product
Flt Cross(C Vec2  &a, C Vec2  &b);
Dbl Cross(C VecD2 &a, C VecD2 &b);

// cross product
Vec  Cross(C Vec  &a, C Vec  &b);
VecD Cross(C VecD &a, C VecD &b);

// cross product normalized
Vec  CrossN(C Vec  &a, C Vec  &b);
VecD CrossN(C VecD &a, C VecD &b);

// perpendicular vector
Vec2  Perp(C Vec2  &v); // returned vector is always rotated by 90 deg clockwise
VecD2 Perp(C VecD2 &v); // returned vector is always rotated by 90 deg clockwise
VecI2 Perp(C VecI2 &v); // returned vector is always rotated by 90 deg clockwise
Vec   Perp(C Vec   &v);
VecD  Perp(C VecD  &v);

// perpendicular vector normalized
Vec2  PerpN(C Vec2  &v); // returned vector is always rotated by 90 deg clockwise
VecD2 PerpN(C VecD2 &v); // returned vector is always rotated by 90 deg clockwise
Vec   PerpN(C Vec   &v);
VecD  PerpN(C VecD  &v);

// reflect vector (result is on the same side of the plane)
Vec2  Reflect(C Vec2  &vec,                       C Vec2  &plane_nrm); // here 'reflection_pos' is assumed to be (0,0)
VecD2 Reflect(C VecD2 &vec,                       C VecD2 &plane_nrm); // here 'reflection_pos' is assumed to be (0,0)               |  /result
Vec   Reflect(C Vec   &vec,                       C Vec   &plane_nrm); // here 'reflection_pos' is assumed to be (0,0,0)             | /
VecD  Reflect(C VecD  &vec,                       C VecD  &plane_nrm); // here 'reflection_pos' is assumed to be (0,0,0)             |/
Vec2  Reflect(C Vec2  &vec, C Vec2  &reflect_pos, C Vec2  &plane_nrm); //                                                  ReflectPos+----->normal
VecD2 Reflect(C VecD2 &vec, C VecD2 &reflect_pos, C VecD2 &plane_nrm); //                                                            |\　
Vec   Reflect(C Vec   &vec, C Vec   &reflect_pos, C Vec   &plane_nrm); //                                                            | \　
VecD  Reflect(C VecD  &vec, C VecD  &reflect_pos, C VecD  &plane_nrm); //                                                            |  \vec

// mirror vector (result is on the other side of the plane)
Vec2  Mirror(C Vec2  &vec,                     C Vec2  &plane_nrm); // here 'plane_pos' is assumed to be (0,0)
VecD2 Mirror(C VecD2 &vec,                     C VecD2 &plane_nrm); // here 'plane_pos' is assumed to be (0,0)             |
Vec   Mirror(C Vec   &vec,                     C Vec   &plane_nrm); // here 'plane_pos' is assumed to be (0,0,0)           |
VecD  Mirror(C VecD  &vec,                     C VecD  &plane_nrm); // here 'plane_pos' is assumed to be (0,0,0)           |
Vec2  Mirror(C Vec2  &vec, C Vec2  &plane_pos, C Vec2  &plane_nrm); //                                             PlanePos+----->normal
VecD2 Mirror(C VecD2 &vec, C VecD2 &plane_pos, C VecD2 &plane_nrm); //                                                    /|\　
Vec   Mirror(C Vec   &vec, C Vec   &plane_pos, C Vec   &plane_nrm); //                                                   / | \　
VecD  Mirror(C VecD  &vec, C VecD  &plane_pos, C Vec   &plane_nrm); //                                            result/  |  \vec
VecD  Mirror(C VecD  &vec, C VecD  &plane_pos, C VecD  &plane_nrm); //

#if EE_PRIVATE
// index of closer point to 'p' (returns 1 if 'p1' is closer to 'p', and 0 if 'p0' is closer to 'p')
Int Closer(C Vec2  &p, C Vec2  &p0, C Vec2  &p1);
Int Closer(C VecD2 &p, C VecD2 &p0, C VecD2 &p1);
Int Closer(C Vec   &p, C Vec   &p0, C Vec   &p1);
Int Closer(C VecD  &p, C VecD  &p0, C VecD  &p1);
#endif

// if equal (epsilon=EPS)
Bool Equal(  Flt   r0,   Flt   r1);
Bool Equal(C Vec2 &v0, C Vec2 &v1);
Bool Equal(C Vec  &v0, C Vec  &v1);
Bool Equal(C Vec4 &v0, C Vec4 &v1);

// if equal (epsilon=EPSD)
Bool Equal(  Dbl    r0,   Dbl    r1);
Bool Equal(C VecD2 &v0, C VecD2 &v1);
Bool Equal(C VecD  &v0, C VecD  &v1);
Bool Equal(C VecD4 &v0, C VecD4 &v1);

// if equal (with custom epsilon)
Bool Equal(  Flt    r0,   Flt    r1, Flt epsilon);
Bool Equal(  Dbl    r0,   Dbl    r1, Dbl epsilon);
Bool Equal(C Vec2  &v0, C Vec2  &v1, Flt epsilon);
Bool Equal(C VecD2 &v0, C VecD2 &v1, Dbl epsilon);
Bool Equal(C Vec   &v0, C Vec   &v1, Flt epsilon);
Bool Equal(C VecD  &v0, C VecD  &v1, Dbl epsilon);
Bool Equal(C Vec4  &v0, C Vec4  &v1, Flt epsilon);
Bool Equal(C VecD4 &v0, C VecD4 &v1, Dbl epsilon);

// if equal using 0..1 wrapping (epsilon=EPS)
Bool EqualWrap(  Flt   r0,   Flt   r1);
Bool EqualWrap(C Vec2 &v0, C Vec2 &v1);
Bool EqualWrap(C Vec  &v0, C Vec  &v1);

// compare
Int Compare(C Vec2   &v0, C Vec2   &v1);
Int Compare(C VecD2  &v0, C VecD2  &v1);
Int Compare(C Vec    &v0, C Vec    &v1);
Int Compare(C VecD   &v0, C VecD   &v1);
Int Compare(C Vec4   &v0, C Vec4   &v1);
Int Compare(C VecD4  &v0, C VecD4  &v1);
Int Compare(C VecI2  &v0, C VecI2  &v1);
Int Compare(C VecB2  &v0, C VecB2  &v1);
Int Compare(C VecSB2 &v0, C VecSB2 &v1);
Int Compare(C VecUS2 &v0, C VecUS2 &v1);
Int Compare(C VecI   &v0, C VecI   &v1);
Int Compare(C VecB   &v0, C VecB   &v1);
Int Compare(C VecSB  &v0, C VecSB  &v1);
Int Compare(C VecUS  &v0, C VecUS  &v1);
Int Compare(C VecI4  &v0, C VecI4  &v1);
Int Compare(C VecB4  &v0, C VecB4  &v1);
Int Compare(C VecSB4 &v0, C VecSB4 &v1);
Int Compare(C Color  &c0, C Color  &c1);
Int Compare(C VecH2  &v0, C VecH2  &v1);
Int Compare(C VecH   &v0, C VecH   &v1);
Int Compare(C VecH4  &v0, C VecH4  &v1);

// normalize array of vectors
void Normalize(Vec2  *v, Int num);
void Normalize(VecD2 *v, Int num);
void Normalize(Vec   *v, Int num);
void Normalize(VecD  *v, Int num);

// transform vectors by matrix
void Transform(Vec2 *v, C Matrix3 &m, Int num);
void Transform(Vec2 *v, C Matrix  &m, Int num);
void Transform(Vec  *v, C Matrix3 &m, Int num);
void Transform(Vec  *v, C Matrix  &m, Int num);
void Transform(VecD *v, C MatrixM &m, Int num);

// change sign of vectors
void Chs(Vec2  *v, Int num);
void Chs(VecD2 *v, Int num);
void Chs(Vec   *v, Int num);
void Chs(VecD  *v, Int num);

// change sign of X component in vectors
void ChsX(Vec2 *v, Int num);
void ChsX(Vec  *v, Int num);

// change sign of Y component in vectors
void ChsY(Vec2 *v, Int num);
void ChsY(Vec  *v, Int num);

// change sign of Z component in vectors
void ChsZ(Vec  *v, Int num);

// reverse components order in vectors
void Reverse(VecI2 *v, Int num);
void Reverse(VecI  *v, Int num);
void Reverse(VecI4 *v, Int num);

// swap X Z components in vectors
void SwapXZ(VecI  *v, Int num);
void SwapXZ(VecI4 *v, Int num);

// get minimum and maximum from array of vectors
Bool MinMax(C Vec2  *v, Int num, Vec2  &min, Vec2  &max);
Bool MinMax(C VecD2 *v, Int num, VecD2 &min, VecD2 &max);
Bool MinMax(C Vec   *v, Int num, Vec   &min, Vec   &max);
Bool MinMax(C VecD  *v, Int num, VecD  &min, VecD  &max);
Bool MinMax(C Vec4  *v, Int num, Vec4  &min, Vec4  &max);
Bool MinMax(C VecD4 *v, Int num, VecD4 &min, VecD4 &max);

#if EE_PRIVATE
// convert from right hand to left hand coordinate system
void RightToLeft(Vec *vec, Int num);
#endif

       CChar8*   DirToText(DIR_ENUM dir);                 // get direction in text format, "Right", "Left", "Up", "Down", "Forward", "Back"
inline UInt      DirToFlag(DIR_ENUM dir) {return 1<<dir;} // convert DIR_ENUM to DIR_FLAG
       AXIS_TYPE DirToAxis(DIR_ENUM dir);                 // convert DIR_ENUM to AXIS_TYPE
/******************************************************************************/
enum SMOOTH_VALUE_MODE : Byte // SmoothValue Mode
{
   SV_NONE, // no smoothing

   SV_WEIGHT2, // sets result equal to 2 previous values weighted average
   SV_WEIGHT3, // sets result equal to 3 previous values weighted average
   SV_WEIGHT4, // sets result equal to 4 previous values weighted average
   SV_WEIGHT8, // sets result equal to 8 previous values weighted average

   SV_AVERAGE2, // sets result equal to 2 previous values average
   SV_AVERAGE3, // sets result equal to 3 previous values average
   SV_AVERAGE4, // sets result equal to 4 previous values average
   SV_AVERAGE8, // sets result equal to 8 previous values average

   SV_NUM, // number of smoothing modes
};
struct SmoothValue
{
   void mode  (SMOOTH_VALUE_MODE mode);   SMOOTH_VALUE_MODE mode()C {return _mode;} // set/get SMOOTH_VALUE_MODE
   void init  (Flt        start_value);                                             // init with starting value
   Flt  update(Flt      current_value);                                             // update by giving current raw value, returns smooth result based on SMOOTH_VALUE_MODE and previous values

   explicit SmoothValue(SMOOTH_VALUE_MODE mode=SV_WEIGHT4);

private:
   SMOOTH_VALUE_MODE _mode;
   Byte              _pos;
   Flt               _history[8];
};
struct SmoothValue2
{
   void mode  (SMOOTH_VALUE_MODE mode);   SMOOTH_VALUE_MODE mode()C {return _mode;} // set/get SMOOTH_VALUE_MODE
   void init  (C Vec2 &   start_value);                                             // init with starting value
   Vec2 update(C Vec2 & current_value);                                             // update by giving current raw value, returns smooth result based on SMOOTH_VALUE_MODE and previous values

   explicit SmoothValue2(SMOOTH_VALUE_MODE mode=SV_WEIGHT4);

private:
   SMOOTH_VALUE_MODE _mode;
   Byte              _pos;
   Vec2              _history[8];
};
/******************************************************************************/
struct SmoothValueTime
{
   Flt update(Flt current_value, Flt dt); // update by giving current raw value, returns smooth result, 'dt'=time delta
#if EE_PRIVATE
   SmoothValueTime& historyTime(Flt time); // set how long keep the values in history
#endif

   explicit SmoothValueTime(Flt history_time=4.0f/60); // 'history_time'=how long keep the values in history

private:
   Byte _pos;
   Flt  _step_time, _time, _time_total, _history[8];
};
struct SmoothValueTime2
{
   Vec2 update(C Vec2 &current_value, Flt dt); // update by giving current raw value, returns smooth result, 'dt'=time delta
#if EE_PRIVATE
   SmoothValueTime2& historyTime(Flt time); // set how long keep the values in history
#endif

   explicit SmoothValueTime2(Flt history_time=4.0f/60); // 'history_time'=how long keep the values in history

private:
   Byte _pos;
   Flt  _step_time, _time, _time_total;
   Vec2 _history[8];
};
/******************************************************************************/
struct SmoothValueSettings
{
   Flt time_delta  , // time delta in this frame
       max_accel   , // max allowed acceleration
       max_velocity; // max allowed velocity

   void reset(Flt max_accel);
};
struct SmoothValueAccel
{
   Flt value, velocity;

   void update(Flt target, C SmoothValueSettings &settings);

   SmoothValueAccel(                         ) : value(    0), velocity(       0) {}
   SmoothValueAccel(Flt value, Flt velocity=0) : value(value), velocity(velocity) {}
};
struct SmoothValueAccel2
{
   Vec2 value, velocity;

   void update(C Vec2 &target, C SmoothValueSettings &settings);

   SmoothValueAccel2(                                        ) : value(    0), velocity(       0) {}
   SmoothValueAccel2(C Vec2 &value, C Vec2 &velocity=Vec2Zero) : value(value), velocity(velocity) {}
};
struct SmoothValueAccel3
{
   Vec value, velocity;

   void update(C Vec &target, C SmoothValueSettings &settings);

   SmoothValueAccel3(                                     ) : value(    0), velocity(       0) {}
   SmoothValueAccel3(C Vec &value, C Vec &velocity=VecZero) : value(value), velocity(velocity) {}
};
/******************************************************************************/
constexpr Int Elms(C VecH2  &v) {return 2;}
constexpr Int Elms(C Vec2   &v) {return 2;}
constexpr Int Elms(C VecD2  &v) {return 2;}
constexpr Int Elms(C VecI2  &v) {return 2;}
constexpr Int Elms(C VecB2  &v) {return 2;}
constexpr Int Elms(C VecSB2 &v) {return 2;}
constexpr Int Elms(C VecUS2 &v) {return 2;}

constexpr Int Elms(C VecH  &v) {return 3;}
constexpr Int Elms(C Vec   &v) {return 3;}
constexpr Int Elms(C VecD  &v) {return 3;}
constexpr Int Elms(C VecI  &v) {return 3;}
constexpr Int Elms(C VecB  &v) {return 3;}
constexpr Int Elms(C VecSB &v) {return 3;}
constexpr Int Elms(C VecUS &v) {return 3;}

constexpr Int Elms(C VecH4  &v) {return 4;}
constexpr Int Elms(C Vec4   &v) {return 4;}
constexpr Int Elms(C VecD4  &v) {return 4;}
constexpr Int Elms(C VecI4  &v) {return 4;}
constexpr Int Elms(C VecB4  &v) {return 4;}
constexpr Int Elms(C VecSB4 &v) {return 4;}
/******************************************************************************/
