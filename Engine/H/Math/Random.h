/******************************************************************************

   Use 'Random' to generate random values.

/******************************************************************************/
struct Randomizer // Randomizer is multi-threaded safe
{
   UID seed; // current seed value, this may be freely modified, it affects generated random values

   void        randomize(); // randomize seed
   Randomizer& back     (); // perform one step back, which will result in the next random value being the same as the last one

   UInt  operator()(                ); // return random UInt  in range             0..0xFFFFFFFF
   UInt  operator()(UInt n          ); // return random UInt  in range             0..n-1
   Int   operator()(Int min, Int max); // return random Int   in range           min..max
   Bool  b         (                ); // return random Bool  (false/true)
   ULong l         (                ); // return random ULong in range             0..0xFFFFFFFFFFFFFFFF
   Flt   f         (                ); // return random Flt   in range             0..1
   Flt   f         (Flt x           ); // return random Flt   in range             0..x
   Flt   f         (Flt min, Flt max); // return random Flt   in range           min..max
   Vec2  vec2      (                ); // return random Vec2  in range         (0,0)..(1,1)
   Vec2  vec2      (Flt x           ); // return random Vec2  in range         (0,0)..(x,x)
   Vec2  vec2      (Flt min, Flt max); // return random Vec2  in range     (min,min)..(max,max)
   Vec   vec       (                ); // return random Vec   in range       (0,0,0)..(1,1,1)
   Vec   vec       (Flt x           ); // return random Vec   in range       (0,0,0)..(x,x,x)
   Vec   vec       (Flt min, Flt max); // return random Vec   in range (min,min,min)..(max,max,max)

   Vec  dir(C Vec &dir, Flt a           ); // return random direction based on 'dir', and   0..a   angle difference, 'dir' must be normalized
   Vec  dir(C Vec &dir, Flt min, Flt max); // return random direction based on 'dir', and min..max angle difference, 'dir' must be normalized

   Vec2 circle1(              Bool inside=true); // return random point on/inside circle with  radius=1 and Vec2(0,0) position
   Vec2 circle (  Flt radius, Bool inside=true); // return random point on/inside circle with 'radius'  and Vec2(0,0) position
   Vec  sphere (C Ball &ball, Flt min, Flt max); // return random point on sphere surface, with min..max vertical angle limit, (min=-1..max, max=min..1)

   Vec  operator()(C Edge    &edge                     ); // return random point on        edge
   Vec2 operator()(C Tri2    &tri    , Bool inside=true); // return random point on/inside triangle
   Vec  operator()(C Tri     &tri    , Bool inside=true); // return random point on/inside triangle
   Vec2 operator()(C Quad2   &quad   , Bool inside=true); // return random point on/inside quad
   Vec  operator()(C Quad    &quad   , Bool inside=true); // return random point on/inside quad
   Vec2 operator()(C Rect    &rect   , Bool inside=true); // return random point on/inside rectangle
   Vec  operator()(C Box     &box    , Bool inside=true); // return random point on/inside box
   Vec  operator()(C OBox    &obox   , Bool inside=true); // return random point on/inside oriented box
   Vec  operator()(C Extent  &ext    , Bool inside=true); // return random point on/inside extent
   Vec2 operator()(C Circle  &circle , Bool inside=true); // return random point on/inside circle
   Vec  operator()(C Ball    &ball   , Bool inside=true); // return random point on/inside ball
   Vec  operator()(C Capsule &capsule, Bool inside=true); // return random point on/inside capsule
   Vec  operator()(C Tube    &tube   , Bool inside=true); // return random point on/inside tube
   Vec  operator()(C Torus   &torus  , Bool inside=true); // return random point on/inside torus
   Vec  operator()(C Cone    &cone   , Bool inside=true); // return random point on/inside cone
   Vec  operator()(C Pyramid &pyramid, Bool inside=true); // return random point on/inside pyramid
   Vec  operator()(C Shape   &shape  , Bool inside=true); // return random point on/inside shape

   Vec  operator()(C MeshBase   &mshb, C AnimatedSkeleton *anim_skel=null); // return random point on MeshBase   surface (if 'anim_skel' is not null then mesh is assumed to be animated by the Skeleton)
   Vec  operator()(C MeshRender &mshr, C AnimatedSkeleton *anim_skel=null); // return random point on MeshRender surface (if 'anim_skel' is not null then mesh is assumed to be animated by the Skeleton)
   Vec  operator()(C MeshPart   &part, C AnimatedSkeleton *anim_skel=null); // return random point on MeshPart   surface (if 'anim_skel' is not null then mesh is assumed to be animated by the Skeleton)
   Vec  operator()(C Mesh       &mesh, C AnimatedSkeleton *anim_skel=null); // return random point on Mesh       surface (if 'anim_skel' is not null then mesh is assumed to be animated by the Skeleton)

   Str password  (Int length, Bool chars, Bool digits, Bool symbols=false); // generate a random password, 'length'=length of the password, 'chars'=if use characters in the password, 'digits'=if use digits in the password, 'symbols'=if use symbols in the password
   Str licenseKey(); // generate a random license key in "XXXXX-XXXXX-XXXXX-XXXXX-XXXXX" format

   // custom probability alignment
   UInt align (UInt n          , Flt (&func)(Flt x)); // return random UInt in range   0..n-1 with probability aligned based on 'func' function
   Int  align (Int min, Int max, Flt (&func)(Flt x)); // return random Int  in range min..max with probability aligned based on 'func' function
   Flt  alignF(                  Flt (&func)(Flt x)); // return random Flt  in range   0..1   with probability aligned based on 'func' function
   Flt  alignF(Flt x           , Flt (&func)(Flt x)); // return random Flt  in range   0..x   with probability aligned based on 'func' function
   Flt  alignF(Flt min, Flt max, Flt (&func)(Flt x)); // return random Flt  in range min..max with probability aligned based on 'func' function

   UInt align (UInt n          , Flt (&func)(Flt x, Flt y), Flt y); // return random UInt in range   0..n-1 with probability aligned based on 'func' function
   Int  align (Int min, Int max, Flt (&func)(Flt x, Flt y), Flt y); // return random Int  in range min..max with probability aligned based on 'func' function
   Flt  alignF(                  Flt (&func)(Flt x, Flt y), Flt y); // return random Flt  in range   0..1   with probability aligned based on 'func' function
   Flt  alignF(Flt x           , Flt (&func)(Flt x, Flt y), Flt y); // return random Flt  in range   0..x   with probability aligned based on 'func' function
   Flt  alignF(Flt min, Flt max, Flt (&func)(Flt x, Flt y), Flt y); // return random Flt  in range min..max with probability aligned based on 'func' function

   UInt alignPow (UInt n          , Flt pow); // return random UInt in range   0..n-1 with probability aligned to "0"   (for pow>1 towards Inf) or to "n-1" (for pow<1 towards 0)
   Int  alignPow (Int min, Int max, Flt pow); // return random Int  in range min..max with probability aligned to "min" (for pow>1 towards Inf) or to "max" (for pow<1 towards 0)
   Flt  alignPowF(                  Flt pow); // return random Flt  in range   0..1   with probability aligned to "0"   (for pow>1 towards Inf) or to "1"   (for pow<1 towards 0)
   Flt  alignPowF(Flt x           , Flt pow); // return random Flt  in range   0..x   with probability aligned to "0"   (for pow>1 towards Inf) or to "x"   (for pow<1 towards 0)
   Flt  alignPowF(Flt min, Flt max, Flt pow); // return random Flt  in range min..max with probability aligned to "min" (for pow>1 towards Inf) or to "max" (for pow<1 towards 0)

   Flt alignTargetNormalF(Flt min, Flt max, Flt target, Flt sharpness); // return random Flt in range min..max with probability aligned to 'target' using normal distribution, 'sharpness'=determines sharpness of alignment (recommended value at least 0.7, for example 1.0, 1.5, or 2.0 are good choices)

   Flt normal    (         ); // return random Flt with      normal distribution in range -13.2163944 .. 13.2163944 (theoretical range is -Inf..Inf however due to floating point precision, the actual range is smaller)
   Flt normalSkew(Flt shape); // return random Flt with skew normal distribution

   // array element
   template<typename TYPE, Int elms>    TYPE& elm(         TYPE       (&array)[elms]) {return array[T(     elms  )];}
   template<typename TYPE, Int elms>  C TYPE& elm(       C TYPE       (&array)[elms]) {return array[T(     elms  )];}
   template<typename TYPE          >    TYPE& elm(  Mems  <TYPE      > &mems        ) {return mems [T(mems.elms())];}
   template<typename TYPE          >  C TYPE& elm(C Mems  <TYPE      > &mems        ) {return mems [T(mems.elms())];}
   template<typename TYPE          >    TYPE& elm(  Memc  <TYPE      > &memc        ) {return memc [T(memc.elms())];}
   template<typename TYPE          >  C TYPE& elm(C Memc  <TYPE      > &memc        ) {return memc [T(memc.elms())];}
   template<typename TYPE, Int size>    TYPE& elm(  Memt  <TYPE, size> &memt        ) {return memt [T(memt.elms())];}
   template<typename TYPE, Int size>  C TYPE& elm(C Memt  <TYPE, size> &memt        ) {return memt [T(memt.elms())];}
   template<typename TYPE          >    TYPE& elm(  Memb  <TYPE      > &memb        ) {return memb [T(memb.elms())];}
   template<typename TYPE          >  C TYPE& elm(C Memb  <TYPE      > &memb        ) {return memb [T(memb.elms())];}
   template<typename TYPE          >    TYPE& elm(  Memx  <TYPE      > &memx        ) {return memx [T(memx.elms())];}
   template<typename TYPE          >  C TYPE& elm(C Memx  <TYPE      > &memx        ) {return memx [T(memx.elms())];}
   template<typename TYPE, Int size>    TYPE& elm(  MemPtr<TYPE, size> &memp        ) {return memp [T(memp.elms())];}
   template<typename TYPE, Int size>  C TYPE& elm(C MemPtr<TYPE, size> &memp        ) {return memp [T(memp.elms())];}

   Randomizer() {randomize();}
   Randomizer(UInt  s0, UInt  s1, UInt s2, UInt s3);
   Randomizer(ULong s0, ULong s1                  );
   Randomizer(C UID &seed                         );
#if EE_PRIVATE
   void fix();
#endif   
}extern
   Random; // Main Randomizer
/******************************************************************************/
const Flt NoiseOrganicGain=0.4f; // can be used as 'gain' parameter along with 'NoiseOrganic' transform to get organic noise

// Noise Transform functions which can be passed to noise methods as 'Transform' parameter
inline Flt NoiseMountain(Flt noise) {return Sqr(1-Abs (noise          ))* 2+(-1 - 0.171f);}
inline Flt NoiseOrganic (Flt noise) {return       Abs (noise          ) * 2+(-1 + 0.492f);}
inline Flt NoiseElectric(Flt noise) {return       Abs (noise          ) *-2-(-1 + 0.492f);} // same as "-NoiseOrganic"
inline Flt NoiseAbsSqr  (Flt noise) {return       Sqr (noise          ) * 2+(-1 + 0.814f);}
inline Flt NoiseCbrt    (Flt noise) {return       Cbrt(noise*0.5f+0.5f) * 2+(-1 - 0.570f);}
inline Flt NoiseSqrt    (Flt noise) {return       Sqrt(noise*0.5f+0.5f) * 2+(-1 - 0.398f);}
inline Flt NoiseSqr     (Flt noise) {return       Sqr (noise*0.5f+0.5f) * 2+(-1 + 0.451f);}
inline Flt NoiseCube    (Flt noise) {return       Cube(noise*0.5f+0.5f) * 2+(-1 + 0.677f);}
/******************************************************************************/
struct PerlinNoise
{
   Flt      noise(Dbl x              )C;
   Flt      noise(Dbl x, Dbl y       )C;
   Flt      noise(Dbl x, Dbl y, Dbl z)C;
   Flt tiledNoise(Dbl x              ,   Int    tile)C;
   Flt tiledNoise(Dbl x, Dbl y       , C VecI2 &tile)C;
   Flt tiledNoise(Dbl x, Dbl y, Dbl z, C VecI  &tile)C;

   Flt      noise1(Dbl x,                           Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt      noise2(Dbl x, Dbl y,                    Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt      noise3(Dbl x, Dbl y, Dbl z,             Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt tiledNoise1(Dbl x,               Int   tile, Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt tiledNoise2(Dbl x, Dbl y,        VecI2 tile, Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt tiledNoise3(Dbl x, Dbl y, Dbl z, VecI  tile, Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;

   Flt      noise1Bloom(Dbl x,                           Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt      noise2Bloom(Dbl x, Dbl y,                    Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt      noise3Bloom(Dbl x, Dbl y, Dbl z,             Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt tiledNoise1Bloom(Dbl x,               Int   tile, Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt tiledNoise2Bloom(Dbl x, Dbl y,        VecI2 tile, Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt tiledNoise3Bloom(Dbl x, Dbl y, Dbl z, VecI  tile, Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;

   Flt mask2(Dbl x, Dbl y, Int octaves, Flt sharpness=0.5f)C;

   PerlinNoise(UInt seed=0);

private:
   Byte p[512];
};
/******************************************************************************/
struct SimplexNoise // Open Simplex Noise
{
   Flt      noise(Dbl x                     )C;
   Flt      noise(Dbl x, Dbl y              )C;
   Flt      noise(Dbl x, Dbl y, Dbl z       )C;
   Flt      noise(Dbl x, Dbl y, Dbl z, Dbl w)C;
   Flt tiledNoise(Dbl x                     ,   Int    tile)C;
   Flt tiledNoise(Dbl x, Dbl y              , C VecI2 &tile)C;
   Flt tiledNoise(Dbl x, Dbl y, Dbl z       , C VecI  &tile)C;
   Flt tiledNoise(Dbl x, Dbl y, Dbl z, Dbl w, C VecI4 &tile)C;

   Flt      noise1(Dbl x,                                  Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt      noise2(Dbl x, Dbl y,                           Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt      noise3(Dbl x, Dbl y, Dbl z,                    Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt      noise4(Dbl x, Dbl y, Dbl z, Dbl w,             Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt tiledNoise1(Dbl x,                      Int   tile, Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt tiledNoise2(Dbl x, Dbl y,               VecI2 tile, Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt tiledNoise3(Dbl x, Dbl y, Dbl z,        VecI  tile, Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;
   Flt tiledNoise4(Dbl x, Dbl y, Dbl z, Dbl w, VecI4 tile, Int octaves, Flt gain=0.5f, Flt Transform(Flt noise)=null)C;

   Flt      noise1Bloom(Dbl x,                                  Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt      noise2Bloom(Dbl x, Dbl y,                           Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt      noise3Bloom(Dbl x, Dbl y, Dbl z,                    Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt      noise4Bloom(Dbl x, Dbl y, Dbl z, Dbl w,             Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt tiledNoise1Bloom(Dbl x,                      Int   tile, Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt tiledNoise2Bloom(Dbl x, Dbl y,               VecI2 tile, Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt tiledNoise3Bloom(Dbl x, Dbl y, Dbl z,        VecI  tile, Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;
   Flt tiledNoise4Bloom(Dbl x, Dbl y, Dbl z, Dbl w, VecI4 tile, Int octaves, Flt bloom=1.0f, Flt sharpness=0.7f)C;

   Flt mask2(Dbl x, Dbl y, Int octaves, Flt sharpness=0.5f)C;

   SimplexNoise(UInt seed=0);

private:
	U16 p[256], permGradIndex3D[256];

#if EE_PRIVATE
   Dbl extrapolate2(int xsb, int ysb, Dbl dx, Dbl dy)C;
   Dbl extrapolate3(int xsb, int ysb, int zsb, Dbl dx, Dbl dy, Dbl dz)C;
   Dbl extrapolate4(int xsb, int ysb, int zsb, int wsb, Dbl dx, Dbl dy, Dbl dz, Dbl dw)C;
#endif
};
/******************************************************************************/
Flt SkewNormalAvg  (Flt shape); // calculate average value for Skew Normal distribution
Flt SkewNormalShape(Flt avg  ); // calculate shape value for Skew Normal distribution which has an average value of 'avg'
/******************************************************************************/
