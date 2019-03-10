/******************************************************************************

   Xor Shift
      http://prng.di.unimi.it/
      http://xoroshiro.di.unimi.it/xoroshiro128plus.c
      http://xorshift.di.unimi.it/xorshift128plus.c
      https://en.wikipedia.org/wiki/Xorshift#Xorshift.2B

   Interesting links:
      https://gist.github.com/KdotJPG
      https://github.com/smcameron/open-simplex-noise-in-c
      https://github.com/Auburns/FastNoise
      http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/
      http://accidentalnoise.sourceforge.net/

   Performance comparison (64/32 means 64/32-bit platform targets, NOT size of integer), time in seconds needed for 1000000x4 Random calls
                UInt-64   ULong-64  UInt-32   ULong-32
      xoroshiro 0.012922  0.012616  0.019902  0.020393
      xorshift  0.012998  0.012683  0.016544  0.016328

      Measured using code below:
         Dbl tc;
         tc=Time.curTime(); REP(1000000){Random  (); Random  (); Random  (); Random  ();} tc=Time.curTime()-tc; MIN(t[0], tc);
         tc=Time.curTime(); REP(1000000){Random.l(); Random.l(); Random.l(); Random.l();} tc=Time.curTime()-tc; MIN(t[1], tc);
      Performance in 64-bit is the same, in 32-bit xorshift is better, however xoroshiro is chosen, because supposedly it has better statistical quality - http://xoroshiro.di.unimi.it/

/******************************************************************************/
#include "stdafx.h"

#define USE_UID       0 // this is high quality however it's very slow and ignores seed
#define USE_XOROSHIRO 1
#define USE_XORSHIFT  0

#define E   2.71828182845904523536028747135266249775724709369995f
#define E_D 2.71828182845904523536028747135266249775724709369995

// average max values
#define PERLIN_2D_AVG_MAX 0.805
#define PERLIN_3D_AVG_MAX 0.805

#define SIMPLEX_2D_AVG_MAX 0.716
#define SIMPLEX_3D_AVG_MAX 0.755
#define SIMPLEX_4D_AVG_MAX 0.827

// !! if changing this then recalc SIMPLEX_2D_AVG_MAX !! constants were set so average value of Abs(noise) = average value of Abs(Random.normal()/PI)
#define SIMPLEX_NORM_CONSTANT_2D 57.8897114 // Kurt Spencer=47
#define SIMPLEX_NORM_CONSTANT_3D 106.285923 // Kurt Spencer=103
#define SIMPLEX_NORM_CONSTANT_4D 23.9058128 // Kurt Spencer=30

//static Flt OrganicAlt(Flt x) {return 1-Sqr(1-Abs(x));} this is brighter

#define SIMPLEX_WRAP_CIRCLE 0 // don't use circle, because 2D tiled circle version doesn't look much like 2D non-tiled version, and 3D/4D introduce artifacts (the shape is not uniform but bent)

namespace EE{
/******************************************************************************/
Randomizer Random;
/******************************************************************************/
Flt SkewNormalAvg(Flt shape) // calculate average value for Skew Normal distribution
{
   return shape/SqrtFast(1+Sqr(shape))*SqrtFast(2/PI);
}
Flt SkewNormalShape(Flt avg) // calculate shape value for Skew Normal distribution which has an average value of 'avg'
{
   // avg=shape/SqrtFast(1+Sqr(shape))*SqrtFast(2/PI);
   // avg/SqrtFast(2/PI)=shape/SqrtFast(1+Sqr(shape));
   // Sqr(avg/SqrtFast(2/PI))=Sqr(shape)/(1+Sqr(shape));
   Flt y=avg/SqrtFast(2/PI); // x=shape
   // Sqr(y)=Sqr(x)/(1+Sqr(x))
   // x*x/(1+x*x)=y*y
   // x*x=y*y*(1+x*x)
   // x*x=y*y+y*y*x*x
   // -y*y=(y*y-1)*x*x
   // -y*y/(y*y-1)=x*x
   // y*y/(1-y*y)=x*x
   // x*x=y*y/(1-y*y)
   // x=Sqrt(y*y/(1-y*y))
   // x=y*Sqrt(1/(1-y*y))
   // x=y/Sqrt(1-y*y)
   if(y>= 1)return  2048; // 2048 was calculated by using "y=1-FLT_EPSILON"
   if(y<=-1)return -2048;
   return y/Sqrt(1-y*y);
}
/******************************************************************************/
void Randomizer::fix()
{
   if(!seed.valid())seed.b[0]=1; // can't be zero
}
void Randomizer::randomize()
{
   seed.randomize(); fix();
}
Randomizer::Randomizer(UInt  s0, UInt  s1, UInt s2, UInt s3) : seed(s0, s1, s2, s3) {fix();}
Randomizer::Randomizer(ULong s0, ULong s1                  ) : seed(s0, s1        ) {fix();}
Randomizer::Randomizer(C UID &seed                         ) : seed(seed          ) {fix();}
/******************************************************************************/
INLINE static ULong rotl(ULong x, UInt k) {return (x<<k) | (x>>(64-k));}
INLINE static ULong rotr(ULong x, UInt k) {return (x>>k) | (x<<(64-k));}

static ULong SolveXorShr(ULong y, UInt s) // solve the formula "y=x^(x>>s)" and return x, assumes s!=0
{
   ULong x=0; // start with zero
   for(UInt t=s; ; t+=s)
   {
      x=y^(x>>s); // guess what we should have based on what we know
      if(t>=64)break; // if we've reached all bits then stop
      x&=(~0ULL)<<(64-t); // AND with a growing mask (at start we know only 's' bits, but with every step we know 's' bits more)
   }
   return x;
}
static ULong SolveXorShl(ULong y, UInt s) // solve the formula "y=x^(x<<s)" and return x, assumes s!=0
{
   ULong x=0; // start with zero
   for(UInt t=s; ; t+=s)
   {
      x=y^(x<<s); // guess what we should have based on what we know
      if(t>=64)break; // if we've reached all bits then stop
      x&=(~0ULL)>>(64-t); // AND with a growing mask (at start we know only 's' bits, but with every step we know 's' bits more)
   }
   return x;
}

#pragma runtime_checks("", off)
UInt Randomizer::operator()()
{
#if USE_UID
   UID id; id.randomize(); return id.i[0]^id.i[1]^id.i[2]^id.i[3];
#elif USE_XOROSHIRO
   const ULong s0=seed.l[0];
         ULong s1=seed.l[1];
   s1^=s0;
   seed.l[0]=rotl(s0, 55)^s1^(s1<<14); // a, b
   seed.l[1]=rotl(s1, 36);             // c
   return seed.i[0]+seed.i[2];
#elif USE_XORSHIFT
         ULong s1=seed.l[0];
   const ULong s0=seed.l[1];
   seed.l[0]=s0;
   s1^=s1<<23;
   seed.l[1]=s1^s0^(s1>>17)^(s0>>26);
   return seed.l[1]+s0;
#else
   return 0;
#endif
}
#pragma runtime_checks("", restore)
ULong Randomizer::l()
{
#if USE_UID
   UID id; id.randomize(); return id.l[0]^id.l[1];
#elif USE_XOROSHIRO
   const ULong s0=seed.l[0];
         ULong s1=seed.l[1];
   s1^=s0;
   seed.l[0]=rotl(s0, 55)^s1^(s1<<14); // a, b
   seed.l[1]=rotl(s1, 36);             // c
   return seed.l[0]+seed.l[1];
#elif USE_XORSHIFT
         ULong s1=seed.l[0];
   const ULong s0=seed.l[1];
   seed.l[0]=s0;
   s1^=s1<<23;
   seed.l[1]=s1^s0^(s1>>17)^(s0>>26);
   return seed.l[1]+s0;
#else
   return 0;
#endif
}

Randomizer& Randomizer::back()
{
#if USE_XOROSHIRO
   ULong s1=rotr(seed.l[1], 36), // at this time it's actually "s1^s0" and not "s1"
         s0=rotr(seed.l[0]^s1^(s1<<14), 55); // rotl(s0, 55)=seed.l[0]^s1^(s1<<14);

   s1^=s0; // get actual s1

   seed.l[0]=s0;
   seed.l[1]=s1;
#elif USE_XORSHIFT
   ULong s0=seed.l[0],
         s1=SolveXorShr(seed.l[1]^s0^(s0>>26), 17); // s1^(s1>>17)=seed.l[1]^s0^(s0>>26);
         s1=SolveXorShl(s1, 23); // s1=s1^(s1<<23);

   seed.l[0]=s1;
   seed.l[1]=s0;
#else
   #error not supported
#endif
   return T;
}

UInt Randomizer::operator()(UInt n          ) {return n ? T()%n : 0;}
Int  Randomizer::operator()(Int min, Int max) {return min+T(max-min+1);}
Bool Randomizer::b         (                ) {return     T()&1;}
Flt  Randomizer::f         (                ) {return     T()*(1.0/UINT_MAX);}
Flt  Randomizer::f         (Flt x           ) {return     f()*x;}
Flt  Randomizer::f         (Flt min, Flt max) {return min+f()*(max-min);}
Vec2 Randomizer::vec2      (                ) {return Vec2(f(        ), f(        )             );}
Vec2 Randomizer::vec2      (Flt x           ) {return Vec2(f(x       ), f(x       )             );}
Vec2 Randomizer::vec2      (Flt min, Flt max) {return Vec2(f(min, max), f(min, max)             );}
Vec  Randomizer::vec       (                ) {return Vec (f(        ), f(        ), f(        ));}
Vec  Randomizer::vec       (Flt x           ) {return Vec (f(x       ), f(x       ), f(x       ));}
Vec  Randomizer::vec       (Flt min, Flt max) {return Vec (f(min, max), f(min, max), f(min, max));}
/******************************************************************************/
UInt Randomizer::align (UInt n ,          Flt (&func)(Flt x)) {if(!n)return 0; UInt u=TruncU(alignF(func)*n); if(u==n)u--; return u;}
Int  Randomizer::align (Int min, Int max, Flt (&func)(Flt x)) {return min+align(max-min+1, func);}
Flt  Randomizer::alignF(                  Flt (&func)(Flt x)) {return func(f());}
Flt  Randomizer::alignF(Flt x           , Flt (&func)(Flt x)) {return alignF(func)* x     ;}
Flt  Randomizer::alignF(Flt min, Flt max, Flt (&func)(Flt x)) {return alignF(func)*(max-min)+min;}

UInt Randomizer::align (UInt n          , Flt (&func)(Flt x, Flt y), Flt y) {if(!n)return 0; UInt u=TruncU(alignF(func, y)*n); if(u==n)u--; return u;}
Int  Randomizer::align (Int min, Int max, Flt (&func)(Flt x, Flt y), Flt y) {return min+align(max-min+1, func, y);}
Flt  Randomizer::alignF(                  Flt (&func)(Flt x, Flt y), Flt y) {return func(f(), y);}
Flt  Randomizer::alignF(Flt x           , Flt (&func)(Flt x, Flt y), Flt y) {return alignF(func, y)* x     ;}
Flt  Randomizer::alignF(Flt min, Flt max, Flt (&func)(Flt x, Flt y), Flt y) {return alignF(func, y)*(max-min)+min;}

static Flt ProbabilityPow(Flt x, Flt y) {return (y>=1) ? Pow(x, y) : 1-Pow(1-x, 1.0f/y);}

UInt Randomizer::alignPow (UInt n          , Flt pow) {return align (n,        ProbabilityPow, pow);}
Int  Randomizer::alignPow (Int min, Int max, Flt pow) {return align (min, max, ProbabilityPow, pow);}
Flt  Randomizer::alignPowF(                  Flt pow) {return alignF(          ProbabilityPow, pow);}
Flt  Randomizer::alignPowF(Flt x           , Flt pow) {return alignF(x,        ProbabilityPow, pow);}
Flt  Randomizer::alignPowF(Flt min, Flt max, Flt pow) {return alignF(min, max, ProbabilityPow, pow);}
/******************************************************************************/
Flt Randomizer::alignTargetNormalF(Flt min, Flt max, Flt target, Flt sharpness)
{
   Flt bias=LerpR(min, max, target)*2-1; // -1..1

   // tweak bias
   if(bias> 0.5f)bias+=Cube(bias-0.5f)*9;else
   if(bias<-0.5f)bias+=Cube(bias+0.5f)*9;

   // for sharpness=2 -> E, sharpness=1 -> 2
 //bias=powf(E, bias*2); for sharpness=2 which is bias=expf(bias*2);
 //bias=powf(2, bias*2); for sharpness=1
   const Flt scale=E-2, add=2-scale, base=Min(E, sharpness*scale+add);
   bias=Pow(base, bias*2);

   return min+(max-min)*bias/(bias+expf(normal()/sharpness));
}
/******************************************************************************/
Flt Randomizer::normal()
{
   return SqrtFast(-2*Ln(f()+FLT_MIN))*Cos(PI2*f());
}
Flt Randomizer::normalSkew(Flt shape)
{
   Flt sigma=shape/SqrtFast(1+Sqr(shape)),
       u0   =normal(),
       u1   =sigma*u0 + SqrtFast(1-Sqr(sigma))*normal();
   return (u0>=0) ? u1 : -u1;
}
/******************************************************************************/
Vec Randomizer::dir(C Vec &dir, Flt a)
{
   Flt c0, s0; CosSin(c0, s0, f(a  ));
   Flt c1, s1; CosSin(c1, s1, f(PI2));
   Vec z=PerpN(dir),
       x=Cross(dir, z);
   return (x*c1 + z*s1)*s0 + dir*c0;
}
Vec Randomizer::dir(C Vec &dir, Flt min, Flt max)
{
   Flt c0, s0; CosSin(c0, s0, f(min, max));
   Flt c1, s1; CosSin(c1, s1, f(PI2));
   Vec z=PerpN(dir),
       x=Cross(dir, z);
   return (x*c1 + z*s1)*s0 + dir*c0;
}
/******************************************************************************/
Vec Randomizer::sphere(C Ball &ball, Flt min, Flt max)
{
   Flt ac, as; CosSin(ac, as, f(PI2));
   Flt bs=f(min, max),
       bc=CosSin(bs);

   return Vec(bc*ac*ball.r, bs*ball.r,
              bc*as*ball.r)   +ball.pos;
}
/******************************************************************************/
Vec Randomizer::operator()(C Edge &edge)
{
   return Lerp(edge.p[0], edge.p[1], f());
}
/******************************************************************************/
Vec2 Randomizer::operator()(C Tri2 &tri, Bool inside)
{
   if(inside)
   {
      Flt u=f(),
          v=f();
      if(u+v>1)
      {
         u=1-u;
         v=1-v;
      }
      return           tri.p[0]
            +(tri.p[1]-tri.p[0])*u
            +(tri.p[2]-tri.p[0])*v;
   }else
   {
      Flt l01=Dist(tri.p[0], tri.p[1]),
          l12=Dist(tri.p[1], tri.p[2]),
          l20=Dist(tri.p[2], tri.p[0]),
          l  =f   (l01+l12+l20);

      // when changing codes watch for division by zero
      if(l<l01)return Lerp(tri.p[0], tri.p[1], l/l01); l-=l01;
      if(l<l12)return Lerp(tri.p[1], tri.p[2], l/l12); l-=l12;
      if(l<l20)return Lerp(tri.p[2], tri.p[0], l/l20);
               return      tri.p[0];
   }
}
Vec Randomizer::operator()(C Tri &tri, Bool inside)
{
   if(inside)
   {
      Flt u=f(),
          v=f();
      if(u+v>1)
      {
         u=1-u;
         v=1-v;
      }
      return           tri.p[0]
            +(tri.p[1]-tri.p[0])*u
            +(tri.p[2]-tri.p[0])*v;
   }else
   {
      Flt l01=Dist(tri.p[0], tri.p[1]),
          l12=Dist(tri.p[1], tri.p[2]),
          l20=Dist(tri.p[2], tri.p[0]),
          l  =f   (l01+l12+l20);

      // when changing codes watch for division by zero
      if(l<l01)return Lerp(tri.p[0], tri.p[1], l/l01); l-=l01;
      if(l<l12)return Lerp(tri.p[1], tri.p[2], l/l12); l-=l12;
      if(l<l20)return Lerp(tri.p[2], tri.p[0], l/l20);
               return      tri.p[0];
   }
}
/******************************************************************************/
Vec2 Randomizer::operator()(C Quad2 &quad, Bool inside)
{
   if(inside)
   {
      Tri2 t0=quad.tri013(),
           t1=quad.tri123();
      Flt  a0=t0  .area  (),
           a1=t1  .area  ();
      Flt  a =f(a0+a1);
      if(  a<=a0)return T(t0);
                 return T(t1);
   }else
   {
      Flt l01=Dist(quad.p[0], quad.p[1]),
          l12=Dist(quad.p[1], quad.p[2]),
          l23=Dist(quad.p[2], quad.p[3]),
          l30=Dist(quad.p[3], quad.p[0]),
          l  =f   (l01+l12+l23+l30);

      // when changing codes watch for division by zero
      if(l<l01)return Lerp(quad.p[0], quad.p[1], l/l01); l-=l01;
      if(l<l12)return Lerp(quad.p[1], quad.p[2], l/l12); l-=l12;
      if(l<l23)return Lerp(quad.p[2], quad.p[3], l/l23); l-=l23;
      if(l<l30)return Lerp(quad.p[3], quad.p[0], l/l30);
               return      quad.p[0];
   }
}
Vec Randomizer::operator()(C Quad &quad, Bool inside)
{
   if(inside)
   {
      Tri t0=quad.tri013(),
          t1=quad.tri123();
      Flt a0=t0  .area  (),
          a1=t1  .area  ();
      Flt a =f(a0+a1);
      if( a<=a0)return T(t0);
                return T(t1);
   }else
   {
      Flt l01=Dist(quad.p[0], quad.p[1]),
          l12=Dist(quad.p[1], quad.p[2]),
          l23=Dist(quad.p[2], quad.p[3]),
          l30=Dist(quad.p[3], quad.p[0]),
          l  =f   (l01+l12+l23+l30);

      // when changing codes watch for division by zero
      if(l<l01)return Lerp(quad.p[0], quad.p[1], l/l01); l-=l01;
      if(l<l12)return Lerp(quad.p[1], quad.p[2], l/l12); l-=l12;
      if(l<l23)return Lerp(quad.p[2], quad.p[3], l/l23); l-=l23;
      if(l<l30)return Lerp(quad.p[3], quad.p[0], l/l30);
               return      quad.p[0];
   }
}
/******************************************************************************/
Vec2 Randomizer::operator()(C Rect &rect, Bool inside)
{
   if(inside)return rect.min+vec2()*rect.size();

   Flt w=rect.w(),
       h=rect.h(),
       r=f(w*2 + h*2);
   if(r<=w)return Vec2(rect.min.x+r, rect.min.y  ); r-=w;
   if(r<=w)return Vec2(rect.min.x+r, rect.max.y  ); r-=w;
   if(r<=h)return Vec2(rect.min.x  , rect.min.y+r); r-=h;
           return Vec2(rect.max.x  , rect.min.y+r);
}
/******************************************************************************/
Vec Randomizer::operator()(C Box &box, Bool inside)
{
   if(inside)return box.min+vec()*box.size();

   Flt w=box.w(),
       h=box.h(),
       d=box.d(),
       x=d*h,
       y=w*d,
       z=w*h,
       r=f(x*2 + y*2 + z*2);
   if(r<=x)return Vec(box.min.x     , box.min.y+f(h), box.min.z+f(d)); r-=x;
   if(r<=x)return Vec(box.max.x     , box.min.y+f(h), box.min.z+f(d)); r-=x;
   if(r<=y)return Vec(box.min.x+f(w), box.min.y     , box.min.z+f(d)); r-=y;
   if(r<=y)return Vec(box.min.x+f(w), box.max.y     , box.min.z+f(d)); r-=y;
   if(r<=z)return Vec(box.min.x+f(w), box.min.y+f(h), box.min.z     ); r-=z;
           return Vec(box.min.x+f(w), box.min.y+f(h), box.max.z     );
}
Vec Randomizer::operator()(C OBox   &obox, Bool inside) {return T(obox.box, inside)*obox.matrix;}
Vec Randomizer::operator()(C Extent &ext , Bool inside) {return T(Box(ext), inside);}
/******************************************************************************/
Vec2 Randomizer::circle1(Bool inside)
{
   Vec2 O; CosSin(O.x, O.y, f(PI2));
   if(inside)O*=Sqrt(f());
   return O;
}
Vec2 Randomizer::circle(Flt radius, Bool inside)
{
   Vec2 O; CosSin(O.x, O.y, f(PI2));
   return O*(inside ? Sqrt(f())*radius : radius);
}
Vec2 Randomizer::operator()(C Circle &circle, Bool inside)
{
   Vec2 O; CosSin(O.x, O.y, f(PI2));
   return O*(inside ? Sqrt(f())*circle.r : circle.r)+circle.pos;
}
/******************************************************************************/
Vec Randomizer::operator()(C Ball &ball, Bool inside)
{
   Flt ac, as; CosSin(ac, as, f(PI2));
   Flt bs=f(-1, 1),
       bc=CosSin(bs);
   Vec O(bc*ac, bs,
         bc*as);
   return O*(inside ? Cbrt(f())*ball.r : ball.r)+ball.pos;
}
/******************************************************************************/
Vec Randomizer::operator()(C Capsule &capsule, Bool inside)
{
   Flt c, s;
   if(inside){c=capsule.volume(); s=Ball(capsule.r).volume();}
   else      {c=capsule.area  (); s=Ball(capsule.r).area  ();}
   if(f(c)>=s)
   {
      return T(Tube(capsule.r, capsule.h-capsule.r*2, capsule.pos, capsule.up), inside);
   }else
   {
      Vec O=T(Ball(capsule.r), inside);
      return O+capsule.pos
              +capsule.up*((capsule.h*0.5f-capsule.r)*Sign(Dot(O, capsule.up)));
   }
}
/******************************************************************************/
Vec Randomizer::operator()(C Tube &tube, Bool inside)
{
   Vec  z=PerpN (tube.up),
        x=Cross (tube.up, z);
   Vec2 c=circle(tube.r, inside);

   return tube.pos
         +tube.up*(f(-0.5f, 0.5f)*tube.h)
         +x*c.x
         +z*c.y;
}
/******************************************************************************/
Vec Randomizer::operator()(C Torus &torus, Bool inside)
{
   Vec2   circle=T.circle(torus.r, inside);
   Matrix matrix; matrix.setPosUp(torus.pos, torus.up).rotateYL(f(PI2));
   return Vec(circle.x+torus.R, circle.y, 0)*matrix;
}
/******************************************************************************/
Vec Randomizer::operator()(C Cone &cone, Bool inside)
{
   if(inside)
   {
      Flt y_step;
      if(cone.h<=0)y_step=0;else
      if(Equal(cone.r_low, cone.r_high))y_step=f(1);else
      {
         // normally 'step' should be calculated from formula : "step=Cbrt(f(1))"
         // however since top and bottom of the cone can have different radius values, that's why we need to add the "missing piece" (in it calculate the limit values of radiuses, and randomize a point in this step range)
         Flt r0=0,
             r1=Min(cone.r_low, cone.r_high),
             r2=Max(cone.r_low, cone.r_high);
         if( r1<=EPS)
         {
            y_step=Cbrt(f(1));
         }else
         {
            Flt dr=(r2-r1)/cone.h, // radius increase on unit of height

                h0=0,
                h1=r1/dr,
                h2=h1+cone.h;

            y_step=Cbrt(f(Cube(h1/h2), 1))*h2; // gives uniform distribution in range h1..h2
            y_step=LerpRS(h1, h2, y_step);
         }
         if(cone.r_low>cone.r_high)y_step=1-y_step;
      }
      Vec2 p=T.circle(Lerp(cone.r_low, cone.r_high, y_step));

      Matrix3 m; m.setUp(cone.up);
      return cone.pos
            +m.y*(y_step*cone.h)
            +m.x*p.x
            +m.z*p.y;
   }else
   {
      Flt     area=cone.area(),
              f   =T.f(area);
      Matrix3 m; m.setUp(cone.up);

      // check lower surface
      Circle circle_lo(cone.r_low);
      Flt    circle_lo_area=circle_lo.area();
      if( f<=circle_lo_area)
      {
         Vec2 p=T.circle(circle_lo.r);
         return cone.pos
               +m.x*p.x
               +m.z*p.y;
      }
      f-=circle_lo_area;

      // check upper surface
      Circle circle_hi(cone.r_high);
      Flt    circle_hi_area=circle_hi.area();
      if( f<=circle_hi_area)
      {
         Vec2 p=T.circle(circle_hi.r);
         return cone.pos
               +m.y*cone.h
               +m.x*p.x
               +m.z*p.y;
      }

      // create on side surface
      Flt y_step;
      if(cone.h<=0)y_step=0;else
      if(Equal(cone.r_low, cone.r_high))y_step=T.f(1);else
      {
         // normally 'step' should be calculated from formula : "step=Sqrt(f(1))"
         // however since top and bottom of the cone can have different radius values, that's why we need to add the "missing piece" (in it calculate the limit values of radiuses, and randomize a point in this step range)
         Flt r0=0,
             r1=Min(cone.r_low, cone.r_high),
             r2=Max(cone.r_low, cone.r_high);
         if( r1<=EPS)
         {
            y_step=Sqrt(T.f(1));
         }else
         {
            Flt dr=(r2-r1)/cone.h, // radius increase on unit of height

                h0=0,
                h1=r1/dr,
                h2=h1+cone.h;

            y_step=Sqrt(T.f(Sqr(h1/h2), 1))*h2; // gives uniform distribution in range h1..h2
            y_step=LerpRS(h1, h2, y_step);
         }
         if(cone.r_low>cone.r_high)y_step=1-y_step;
      }
      Flt  angle=T.f(PI2);
      Vec2 p; CosSin(p.x, p.y, angle); p*=Lerp(cone.r_low, cone.r_high, y_step);
      return cone.pos
            +m.y*(cone.h*y_step)
            +m.x*p.x
            +m.z*p.y;
   }
}
/******************************************************************************/
Vec Randomizer::operator()(C Pyramid &pyramid, Bool inside)
{
   if(inside)
   {
      Flt z=Cbrt(f(  1  ))*pyramid.h,
          y=     f(-1, 1) *pyramid.scale*z,
          x=     f(-1, 1) *pyramid.scale*z;

      return pyramid.pos
            +pyramid.dir    *z
            +pyramid.perp   *y
            +pyramid.cross()*x;
   }else
   {
      Flt  area  =pyramid.area(),
           f     =T.f(area), // randomize 'f' on surface
           square=Sqr(pyramid.side()); // base surface area
      if(f<square) // if placed on base surface (use '<' instead of '<=' to take division by zero into account)
      {
         Flt x=(f/square)*2-1,
             y=T.f(-1, 1);
         return pyramid.pos
               +pyramid.dir    *   pyramid.h
               +pyramid.perp   *(y*pyramid.scale*pyramid.h)
               +pyramid.cross()*(x*pyramid.scale*pyramid.h);
      }
      Vec x     =pyramid.cross()*(pyramid.scale*pyramid.h),
          y     =pyramid.perp   *(pyramid.scale*pyramid.h),
          z     =pyramid.dir    * pyramid.h,
          center=pyramid.pos    + z,
          lu    =center-x+y,
          ru    =center+x+y,
          rd    =center+x-y,
          ld    =center-x-y;
      switch(T(4))
      {
         case  0: return T(Tri(pyramid.pos, lu, ru));
         case  1: return T(Tri(pyramid.pos, ru, rd));
         case  2: return T(Tri(pyramid.pos, rd, ld));
         default: return T(Tri(pyramid.pos, ld, lu));
      }
   }
}
/******************************************************************************/
Vec Randomizer::operator()(C Shape &shape, Bool inside)
{
   switch(shape.type)
   {
      case SHAPE_POINT  : return       shape.point           ;
      case SHAPE_EDGE   : return     T(shape.edge           );
      case SHAPE_RECT   : return Vec(T(shape.rect   , inside), 0);
      case SHAPE_BOX    : return     T(shape.box    , inside);
      case SHAPE_OBOX   : return     T(shape.obox   , inside);
      case SHAPE_CIRCLE : return Vec(T(shape.circle , inside), 0);
      case SHAPE_BALL   : return     T(shape.ball   , inside);
      case SHAPE_CAPSULE: return     T(shape.capsule, inside);
      case SHAPE_TUBE   : return     T(shape.tube   , inside);
      case SHAPE_TORUS  : return     T(shape.torus  , inside);
      case SHAPE_CONE   : return     T(shape.cone   , inside);
      case SHAPE_PYRAMID: return     T(shape.pyramid, inside);
      default           : return 0;
   }
}
/******************************************************************************/
Vec Randomizer::operator()(C MeshBase &mshb, C AnimatedSkeleton *anim_skel)
{
   if(  Int  faces=mshb.faces  ())
   if(C Vec *pos  =mshb.vtx.pos())
   {
    C VecB4 *bone;
    C VecB4 *weight;
      Int face=T(faces);
      if( face<mshb.tris()) // triangle
      {
       C VecI &ind=mshb.tri.ind(face);
       C Vec  &a  =pos[ind.x],
              &b  =pos[ind.y],
              &c  =pos[ind.z];
         if(anim_skel && (bone=mshb.vtx.matrix()) && (weight=mshb.vtx.blend())) // animated
         {
            Byte msb_a=bone[ind.x].c[weight[ind.x].maxI()], // for performance reasons only one bone with biggest weight is used
                 msb_b=bone[ind.y].c[weight[ind.y].maxI()],
                 msb_c=bone[ind.z].c[weight[ind.z].maxI()];
            return T(Tri(a*anim_skel->boneRoot(msb_a-1).matrix(),
                         b*anim_skel->boneRoot(msb_b-1).matrix(),
                         c*anim_skel->boneRoot(msb_c-1).matrix()));
         }else // static
         {
            return T(Tri(a, b, c));
         }
      }else // quad
      {
         face-=mshb.tris();
       C VecI4 &ind=mshb.quad.ind(face);
       C Vec   &a  =pos[ind.x],
               &b  =pos[ind.y],
               &c  =pos[ind.z],
               &d  =pos[ind.w];
         if(anim_skel && (bone=mshb.vtx.matrix()) && (weight=mshb.vtx.blend())) // animated
         {
            Byte msb_a=bone[ind.x].c[weight[ind.x].maxI()], // for performance reasons only one bone with biggest weight is used
                 msb_b=bone[ind.y].c[weight[ind.y].maxI()],
                 msb_c=bone[ind.z].c[weight[ind.z].maxI()],
                 msb_d=bone[ind.w].c[weight[ind.w].maxI()];
            return T(Quad(a*anim_skel->boneRoot(msb_a-1).matrix(),
                          b*anim_skel->boneRoot(msb_b-1).matrix(),
                          c*anim_skel->boneRoot(msb_c-1).matrix(),
                          d*anim_skel->boneRoot(msb_d-1).matrix()));
         }else // static
         {
            return T(Quad(a, b, c, d));
         }
      }
   }
   return 0;
}
Vec Randomizer::operator()(C MeshRender &mshr, C AnimatedSkeleton *anim_skel)
{
   Vec out(0);
   if(mshr.tris())
   {
      Int pos_ofs =mshr.vtxOfs(VTX_POS);
      if( pos_ofs>=0)
      if(C Byte *vtx_data=mshr.vtxLockRead())
      {
         if(CPtr ind=mshr.indLockRead())
         {
            // calculate random face and its vertexes
            VecI vtx_ofs;
            Int  face=T(mshr.tris())*3; // 3 vtx indexes in each triangle
            if(mshr._ib.bit16()){U16 *d=(U16*)ind; vtx_ofs.set(d[face], d[face+1], d[face+2]);}
            else                {U32 *d=(U32*)ind; vtx_ofs.set(d[face], d[face+1], d[face+2]);}
            vtx_ofs*=mshr.vtxSize();

            // get vertex positions
          C Vec &a=*(Vec*)(vtx_data+vtx_ofs.x+pos_ofs),
                &b=*(Vec*)(vtx_data+vtx_ofs.y+pos_ofs),
                &c=*(Vec*)(vtx_data+vtx_ofs.z+pos_ofs);

            Int bone_ofs,
              weight_ofs;
            if(anim_skel && ((bone_ofs=mshr.vtxOfs(VTX_MATRIX))>=0) && ((weight_ofs=mshr.vtxOfs(VTX_BLEND))>=0)) // animated
            {
             C VecB4 &  bone_a=*(VecB4*)(vtx_data+vtx_ofs.x+  bone_ofs),
                     &  bone_b=*(VecB4*)(vtx_data+vtx_ofs.y+  bone_ofs),
                     &  bone_c=*(VecB4*)(vtx_data+vtx_ofs.z+  bone_ofs),
                     &weight_a=*(VecB4*)(vtx_data+vtx_ofs.x+weight_ofs),
                     &weight_b=*(VecB4*)(vtx_data+vtx_ofs.y+weight_ofs),
                     &weight_c=*(VecB4*)(vtx_data+vtx_ofs.z+weight_ofs);
               // get most significant bone
               Byte msb_a=bone_a.c[weight_a.maxI()],
                    msb_b=bone_b.c[weight_b.maxI()],
                    msb_c=bone_c.c[weight_c.maxI()];
               if(mshr._bone_split)
               {
                  Int tris=0; FREP(mshr._bone_splits)
                  {
                     MeshRender::BoneSplit &bs=mshr._bone_split[i];
                     if(face>=tris && face<tris+bs.tris)
                     {
                        msb_a=bs.split_to_real[msb_a];
                        msb_b=bs.split_to_real[msb_b];
                        msb_c=bs.split_to_real[msb_c];
                        break;
                     }
                     tris+=bs.tris;
                  }
               }
               out=T(Tri(a*anim_skel->boneRoot(msb_a-1).matrix(),
                         b*anim_skel->boneRoot(msb_b-1).matrix(),
                         c*anim_skel->boneRoot(msb_c-1).matrix()));
            }else // static
            {
               out=T(Tri(a, b, c));
            }
            mshr.indUnlock();
         }
         mshr.vtxUnlock();
      }
   }
   return out;
}
Vec Randomizer::operator()(C MeshPart &part, C AnimatedSkeleton *anim_skel)
{
   return part.base.faces() ? T(part.base, anim_skel) : T(part.render, anim_skel);
}
Vec Randomizer::operator()(C Mesh &mesh, C AnimatedSkeleton *anim_skel)
{
   return mesh.parts.elms() ? T(mesh.parts[T(mesh.parts.elms())], anim_skel) : 0;
}
/******************************************************************************/
StrO Randomizer::password(Int length, Bool chars, Bool digs, Bool symbols)
{
   if(length>0)
   {
      Str8 elms; if(chars)elms+="abcdefghijklmnopqrstuvwxyz"; if(digs)elms+="0123456789"; if(symbols)elms+="~!@#$%^&*()_-+=[]{};:,.<>?'\"/|\\";
      if(  elms.length())
      {
         StrO   pass; REP(length)pass+=elms[T(elms.length())];
         return pass;
      }
   }
   return S;
}
static Char8 LicenseKeyChars[]={'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
StrO Randomizer::licenseKey()
{
   StrO key; key.reserve(5*5+4);
   FREP(5)
   {
      if(i)key+='-';
      REP(5)key+=LicenseKeyChars[T(Elms(LicenseKeyChars))];
   }
   return key;
}
/******************************************************************************/
// PERLIN NOISE
/******************************************************************************/
#if 0 // Esenthel version (not finished) produces less blocky results, however requires a lot more memory
static Flt R[256][256][2];
struct TEMP
{
   TEMP()
   {
      Random.seed.set(1,1,1,1);
      REPD(x, 256)
      REPD(y, 256)
      {
         //R[y][x][0]=Random.f()*2-1;
         //R[y][x][1]=Random.f()*2-1;
         Vec2 p=Random.circle(1, false);
         R[y][x][0]=p.x;
         R[y][x][1]=p.y;
         //R[y][x][0]=SignBool(Random.b());
         //R[y][x][1]=SignBool(Random.b());
         //R[y][x][0]=Round(p.x);
         //R[y][x][1]=Round(p.y);
      }
   }
}TT;
static Flt Grad(Int x, Int y, Flt fx, Flt fy, UInt seed)
{
   //Randomizer rnd(x, y, 0, seed);
   //return (rnd.f()*2-1)*fx + (rnd.f()*2-1)*fy;
   return R[x&255][y&255][0]*fx + R[x&255][y&255][1]*fy;
}
PerlinNoise::PerlinNoise(UInt seed)
{
   T.seed=seed;
}
Flt PerlinNoise::noise(Dbl x, Dbl y)C
{
   Int X=Floor(x); Flt fx=x-X, fx1=fx-1; Flt u=_SmoothQuintic(fx);
   Int Y=Floor(y); Flt fy=y-Y, fy1=fy-1; Flt v=_SmoothQuintic(fy);

   return Lerp(Lerp(Grad(X  , Y  , fx , fy , seed),
                    Grad(X+1, Y  , fx1, fy , seed), u),
               Lerp(Grad(X  , Y+1, fx , fy1, seed),
                    Grad(X+1, Y+1, fx1, fy1, seed), u), v);
}
#else // Ken Perlin "Optimized" version, with "Byte p[512];" member
static Flt Grad(Int hash, Flt x)
{
   Int h=(hash&15);
   Flt u=(h<8) ? x : 0,
       v=(h<4) ? 0 : (h==12 || h==14) ? x : 0;
   return ((h&1) ? -u : u) + ((h&2) ? -v : v);
}
static Flt Grad(Int hash, Flt x, Flt y)
{
   Int h=(hash&15);
   Flt u=(h<8) ? x : y,
       v=(h<4) ? y : (h==12 || h==14) ? x : 0;
   return ((h&1) ? -u : u) + ((h&2) ? -v : v);
}
static Flt Grad(Int hash, Flt x, Flt y, Flt z)
{
   Int h=(hash&15);
   Flt u=(h<8) ? x : y,
       v=(h<4) ? y : (h==12 || h==14) ? x : z;
   return ((h&1) ? -u : u) + ((h&2) ? -v : v);
}
static Flt Grad(Int hash, Flt x, Flt y, Flt z, Flt t)
{
   Int h=(hash&31);
   Flt u=(h<24) ? x : y,
       v=(h<16) ? y : z,
       w=(h< 8) ? z : t;
   return ((h&1) ? -u : u) + ((h&2) ? -v : v) + ((h&4) ? -w : w);
}
PerlinNoise::PerlinNoise(UInt seed)
{
   Randomizer rnd(seed, 0, 0, 0);
    REP(256)p[i]=i;
   FREP(255)Swap(p[i], p[rnd(i, 255)]); // 'RandomizeOrder'
   CopyN(p+256, p, 256);
}
Flt PerlinNoise::noise(Dbl x)C
{
   Int X=Floor(x); Flt fx=x-X, fx1=fx-1; X&=0xFF; Flt u=_SmoothQuintic(fx);

   Int A=p[X  ], AA=p[A],
       B=p[X+1], BA=p[B];

   return Lerp(Grad(p[AA], fx ),
               Grad(p[BA], fx1), u)/0.5f;
}
Flt PerlinNoise::noise(Dbl x, Dbl y)C
{
   Int X=Floor(x); Flt fx=x-X, fx1=fx-1; X&=0xFF; Flt u=_SmoothQuintic(fx);
   Int Y=Floor(y); Flt fy=y-Y, fy1=fy-1; Y&=0xFF; Flt v=_SmoothQuintic(fy);

   Int A=p[X  ]+Y, AA=p[A], AB=p[A+1],
       B=p[X+1]+Y, BA=p[B], BB=p[B+1];

   return Lerp(Lerp(Grad(p[AA], fx , fy ),
                    Grad(p[BA], fx1, fy ), u),
               Lerp(Grad(p[AB], fx , fy1),
                    Grad(p[BB], fx1, fy1), u), v);
}
Flt PerlinNoise::noise(Dbl x, Dbl y, Dbl z)C
{
   Int X=Floor(x); Flt fx=x-X, fx1=fx-1; X&=0xFF; Flt u=_SmoothQuintic(fx);
   Int Y=Floor(y); Flt fy=y-Y, fy1=fy-1; Y&=0xFF; Flt v=_SmoothQuintic(fy);
   Int Z=Floor(z); Flt fz=z-Z, fz1=fz-1; Z&=0xFF; Flt w=_SmoothQuintic(fz);

   Int A=p[X  ]+Y, AA=p[A]+Z, AB=p[A+1]+Z,
       B=p[X+1]+Y, BA=p[B]+Z, BB=p[B+1]+Z;

   return Lerp(Lerp(Lerp(Grad(p[AA  ], fx , fy , fz ),
                         Grad(p[BA  ], fx1, fy , fz ), u),
                    Lerp(Grad(p[AB  ], fx , fy1, fz ),
                         Grad(p[BB  ], fx1, fy1, fz ), u), v),
               Lerp(Lerp(Grad(p[AA+1], fx , fy , fz1),
                         Grad(p[BA+1], fx1, fy , fz1), u),
                    Lerp(Grad(p[AB+1], fx , fy1, fz1),
                         Grad(p[BB+1], fx1, fy1, fz1), u), v), w);
}
/******************************************************************************/
Flt PerlinNoise::tiledNoise(Dbl x, Int tile)C
{
   MAX(tile, 1); // can't be smaller than 1, because of division by zero resulting in exception, and <0 giving negative values

   Int X=Floor(x); Flt fx=x-X, fx1=fx-1; X=Mod(X, tile); Int X1=((X+1)%tile)&255; X&=255; Flt u=_SmoothQuintic(fx);

   Int A=p[X ], AA=p[A],
       B=p[X1], BA=p[B];

   return Lerp(Grad(p[AA], fx ),
               Grad(p[BA], fx1), u);
}
Flt PerlinNoise::tiledNoise(Dbl x, Dbl y, C VecI2 &tile)C
{
   VecI2 t(Max(tile.x, 1), Max(tile.y, 1)); // can't be smaller than 1, because of division by zero resulting in exception, and <0 giving negative values

   Int X=Floor(x); Flt fx=x-X, fx1=fx-1; X=Mod(X, t.x); Int X1=((X+1)%t.x)&255; X&=255; Flt u=_SmoothQuintic(fx);
   Int Y=Floor(y); Flt fy=y-Y, fy1=fy-1; Y=Mod(Y, t.y); Int Y1=((Y+1)%t.y)&255; Y&=255; Flt v=_SmoothQuintic(fy);

   Int A=p[X ], AA=p[A+Y], AB=p[A+Y1],
       B=p[X1], BA=p[B+Y], BB=p[B+Y1];

   return Lerp(Lerp(Grad(p[AA], fx , fy ),
                    Grad(p[BA], fx1, fy ), u),
               Lerp(Grad(p[AB], fx , fy1),
                    Grad(p[BB], fx1, fy1), u), v);
}
Flt PerlinNoise::tiledNoise(Dbl x, Dbl y, Dbl z, C VecI &tile)C
{
   VecI t(Max(tile.x, 1), Max(tile.y, 1), Max(tile.z, 1)); // can't be smaller than 1, because of division by zero resulting in exception, and <0 giving negative values

   Int X=Floor(x); Flt fx=x-X, fx1=fx-1; X=Mod(X, t.x); Int X1=((X+1)%t.x)&255; X&=255; Flt u=_SmoothQuintic(fx);
   Int Y=Floor(y); Flt fy=y-Y, fy1=fy-1; Y=Mod(Y, t.y); Int Y1=((Y+1)%t.y)&255; Y&=255; Flt v=_SmoothQuintic(fy);
   Int Z=Floor(z); Flt fz=z-Z, fz1=fz-1; Z=Mod(Z, t.z); Int Z1=((Z+1)%t.z)&255; Z&=255; Flt w=_SmoothQuintic(fz);

   Int A=p[X ], AA=p[A+Y], AB=p[A+Y1],
       B=p[X1], BA=p[B+Y], BB=p[B+Y1];

   return Lerp(Lerp(Lerp(Grad(p[AA+Z ], fx , fy , fz ),
                         Grad(p[BA+Z ], fx1, fy , fz ), u),
                    Lerp(Grad(p[AB+Z ], fx , fy1, fz ),
                         Grad(p[BB+Z ], fx1, fy1, fz ), u), v),
               Lerp(Lerp(Grad(p[AA+Z1], fx , fy , fz1),
                         Grad(p[BA+Z1], fx1, fy , fz1), u),
                    Lerp(Grad(p[AB+Z1], fx , fy1, fz1),
                         Grad(p[BB+Z1], fx1, fy1, fz1), u), v), w);
}
#endif
/******************************************************************************/
// MULTI
/******************************************************************************/
Flt PerlinNoise::noise1(Dbl x, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=noise(x); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      amp   *=gain;
   }
   return result;
}
Flt PerlinNoise::noise2(Dbl x, Dbl y, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=noise(x, y); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      amp   *=gain;
   }
   return result;
}
Flt PerlinNoise::noise3(Dbl x, Dbl y, Dbl z, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=noise(x, y, z); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      z     *=2;
      amp   *=gain;
   }
   return result;
}
/******************************************************************************/
Flt PerlinNoise::tiledNoise1(Dbl x, Int tile, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=tiledNoise(x, tile); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      tile  *=2;
      amp   *=gain;
   }
   return result;
}
Flt PerlinNoise::tiledNoise2(Dbl x, Dbl y, VecI2 tile, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=tiledNoise(x, y, tile); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      tile  *=2;
      amp   *=gain;
   }
   return result;
}
Flt PerlinNoise::tiledNoise3(Dbl x, Dbl y, Dbl z, VecI tile, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=tiledNoise(x, y, z, tile); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      z     *=2;
      tile  *=2;
      amp   *=gain;
   }
   return result;
}
/******************************************************************************/
// BLOOM
/******************************************************************************/
static const Flt BloomGain=0.85f;
Flt PerlinNoise::noise1Bloom(Dbl x, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=noise(x), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=noise(x*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt PerlinNoise::noise2Bloom(Dbl x, Dbl y, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=noise(x, y), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=noise(x*=2, y*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt PerlinNoise::noise3Bloom(Dbl x, Dbl y, Dbl z, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=noise(x, y, z), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=noise(x*=2, y*=2, z*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
/******************************************************************************/
Flt PerlinNoise::tiledNoise1Bloom(Dbl x, Int tile, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=tiledNoise(x, tile), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=tiledNoise(x*=2, tile*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt PerlinNoise::tiledNoise2Bloom(Dbl x, Dbl y, VecI2 tile, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=tiledNoise(x, y, tile), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=tiledNoise(x*=2, y*=2, tile*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt PerlinNoise::tiledNoise3Bloom(Dbl x, Dbl y, Dbl z, VecI tile, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=tiledNoise(x, y, z, tile), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=tiledNoise(x*=2, y*=2, z*=2, tile*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
/******************************************************************************/
#if 0 // so so, Open Simplex is much better
/******************************************************************************/
// SIMPLEX NOISE
/******************************************************************************
SimplexNoise1234
Copyright 2003-2011, Stefan Gustavson
Contact: stegu@itn.liu.se
This library is public domain software, released by the author into the public domain in February 2011. You may do anything you like with it. You may even remove all attributions, but of course I'd appreciate it if you kept my name somewhere.
This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
/******************************************************************************/
static Byte Simplex[64][4]=
{
   {0,1,2,3}, {0,1,3,2}, {0,0,0,0}, {0,2,3,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,2,3,0},
   {0,2,1,3}, {0,0,0,0}, {0,3,1,2}, {0,3,2,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,3,2,0},
   {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0},
   {1,2,0,3}, {0,0,0,0}, {1,3,0,2}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,3,0,1}, {2,3,1,0},
   {1,0,2,3}, {1,0,3,2}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,0,3,1}, {0,0,0,0}, {2,1,3,0},
   {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0},
   {2,0,1,3}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {3,0,1,2}, {3,0,2,1}, {0,0,0,0}, {3,1,2,0},
   {2,1,0,3}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {3,1,0,2}, {0,0,0,0}, {3,2,0,1}, {3,2,1,0},
};
/******************************************************************************/
struct PerlinSimplexNoise
{
   Flt noise(Dbl x       )C;
   Flt noise(Dbl x, Dbl y)C;

   PerlinSimplexNoise(UInt seed=0);

private:
   Byte p[512];
};
PerlinSimplexNoise::PerlinSimplexNoise(UInt seed)
{
   Randomizer rnd(seed, 0, 0, 0);
    REP(256)p[i]=i;
   FREP(255)Swap(p[i], p[rnd(i, 255)]); // 'RandomizeOrder'
   CopyN(p+256, p, 256);
}
/******************************************************************************/
Flt PerlinSimplexNoise::noise(Dbl x)C
{
   Int i0=Floor(x);
   Int i1=i0+1;
   Flt x0=x-i0;
   Flt x1=x0-1.0f;

   Flt n0, n1;

   Flt t0=1.0f-x0*x0;
 //if(t0<0.0f)t0=0.0f;
   t0*=t0;
   n0=t0*t0*Grad(p[i0&0xff], x0);

   Flt t1=1.0f-x1*x1;
 //if(t1<0.0f)t1=0.0f;
   t1*=t1;
   n1=t1*t1*Grad(p[i1&0xff], x1);

   return (n0+n1)/0.316406310f;
}
Flt PerlinSimplexNoise::noise(Dbl x, Dbl y)C
{
   #define F2 0.366025403f // F2=0.5*(Sqrt(3.0)-1.0)
   #define G2 0.211324865f // G2=(3.0-Sqrt(3.0))/6.0

   Flt n0, n1, n2; // Noise contributions from the three corners

   // Skew the input space to determine which simplex cell we're in
   Flt s=(x+y)*F2; // Hairy factor for 2D
   Flt xs=x+s;
   Flt ys=y+s;
   Int i=Floor(xs);
   Int j=Floor(ys);

   Flt t=(Flt)(i+j)*G2;
   Flt X0=i-t; // Unskew the cell origin back to (x,y)space
   Flt Y0=j-t;
   Flt x0=x-X0; // The x,y distances from the cell origin
   Flt y0=y-Y0;

   // For the 2D case, the simplex shape is an equilateral triangle.
   // Determine which simplex we are in.
   Int i1, j1; // Offsets for second (middle)corner of simplex in (i,j)coords
   if(x0>y0){i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
   else     {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)

   // A step of (1,0)in (i,j)means a step of (1-c,-c)in (x,y), and
   // a step of (0,1)in (i,j)means a step of (-c,1-c)in (x,y), where
   // c=(3-Sqrt(3))/6

   Flt x1=x0-i1+G2; // Offsets for middle corner in (x,y)unskewed coords
   Flt y1=y0-j1+G2;
   Flt x2=x0-1.0f+2.0f*G2; // Offsets for last corner in (x,y)unskewed coords
   Flt y2=y0-1.0f+2.0f*G2;

   // Wrap the integer indices at 256, to avoid indexing p[] out of bounds
   Int ii=i&0xff;
   Int jj=j&0xff;

   // Calculate the contribution from the three corners
   Flt t0=0.5f-x0*x0-y0*y0;
   if(t0<0.0f)n0=0.0f;else
   {
      t0*=t0;
      n0=t0*t0*Grad(p[ii+p[jj]], x0, y0); 
   }

   Flt t1=0.5f-x1*x1-y1*y1;
   if(t1<0.0f)n1=0.0f;else
   {
      t1*=t1;
      n1=t1*t1*Grad(p[ii+i1+p[jj+j1]], x1, y1);
   }

   Flt t2=0.5f-x2*x2-y2*y2;
   if(t2<0.0f)n2=0.0f;else
   {
      t2*=t2;
      n2=t2*t2*Grad(p[ii+1+p[jj+1]], x2, y2);
   }

   return (n0+n1+n2)/0.014255565f;
}
Flt PerlinSimplexNoise::noise(Dbl x, Dbl y, Dbl z)C
{
   // Simple skewing factors for the 3D case
   #define F3 0.333333333f
   #define G3 0.166666667f

   Flt n0, n1, n2, n3; // Noise contributions from the four corners

   // Skew the input space to determine which simplex cell we're in
   Flt s=(x+y+z)*F3; // Very nice and simple skew factor for 3D
   Flt xs=x+s;
   Flt ys=y+s;
   Flt zs=z+s;
   Int i=Floor(xs);
   Int j=Floor(ys);
   Int k=Floor(zs);

   Flt t=(Flt)(i+j+k)*G3; 
   Flt X0=i-t; // Unskew the cell origin back to (x,y,z)space
   Flt Y0=j-t;
   Flt Z0=k-t;
   Flt x0=x-X0; // The x,y,z distances from the cell origin
   Flt y0=y-Y0;
   Flt z0=z-Z0;

   // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
   // Determine which simplex we are in.
   Int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k)coords
   Int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k)coords

   /* This code would benefit from a backport from the GLSL version! */
   if(x0>=y0)
   {
      if(y0>=z0){i1=1; j1=0; k1=0; i2=1; j2=1; k2=0;}else // X Y Z order
      if(x0>=z0){i1=1; j1=0; k1=0; i2=1; j2=0; k2=1;}else // X Z Y order
                {i1=0; j1=0; k1=1; i2=1; j2=0; k2=1;}     // Z X Y order
   }else // x0<y0
   {
      if(y0<z0){i1=0; j1=0; k1=1; i2=0; j2=1; k2=1;}else // Z Y X order
      if(x0<z0){i1=0; j1=1; k1=0; i2=0; j2=1; k2=1;}else // Y Z X order
               {i1=0; j1=1; k1=0; i2=1; j2=1; k2=0;}     // Y X Z order
   }

   // A step of (1,0,0)in (i,j,k)means a step of (1-c,-c,-c)in (x,y,z),
   // a step of (0,1,0)in (i,j,k)means a step of (-c,1-c,-c)in (x,y,z), and
   // a step of (0,0,1)in (i,j,k)means a step of (-c,-c,1-c)in (x,y,z), where
   // c=1/6.

   Flt x1=x0-i1+G3; // Offsets for second corner in (x,y,z)coords
   Flt y1=y0-j1+G3;
   Flt z1=z0-k1+G3;
   Flt x2=x0-i2+2.0f*G3; // Offsets for third corner in (x,y,z)coords
   Flt y2=y0-j2+2.0f*G3;
   Flt z2=z0-k2+2.0f*G3;
   Flt x3=x0-1.0f+3.0f*G3; // Offsets for last corner in (x,y,z)coords
   Flt y3=y0-1.0f+3.0f*G3;
   Flt z3=z0-1.0f+3.0f*G3;

   // Wrap the integer indices at 256, to avoid indexing p[] out of bounds
   Int ii=i&0xff;
   Int jj=j&0xff;
   Int kk=k&0xff;

   // Calculate the contribution from the four corners
   Flt t0=0.6f-x0*x0-y0*y0-z0*z0;
   if(t0<0.0f)n0=0.0f;else
   {
      t0*=t0;
      n0=t0*t0*Grad(p[ii+p[jj+p[kk]]], x0, y0, z0);
   }

   Flt t1=0.6f-x1*x1-y1*y1-z1*z1;
   if(t1<0.0f)n1=0.0f;else
   {
      t1*=t1;
      n1=t1*t1*Grad(p[ii+i1+p[jj+j1+p[kk+k1]]], x1, y1, z1);
   }

   Flt t2=0.6f-x2*x2-y2*y2-z2*z2;
   if(t2<0.0f)n2=0.0f;else
   {
      t2*=t2;
      n2=t2*t2*Grad(p[ii+i2+p[jj+j2+p[kk+k2]]], x2, y2, z2);
   }

   Flt t3=0.6f-x3*x3-y3*y3-z3*z3;
   if(t3<0.0f)n3=0.0f;else
   {
      t3*=t3;
      n3=t3*t3*Grad(p[ii+1+p[jj+1+p[kk+1]]], x3, y3, z3);
   }

   return 32.0f*(n0+n1+n2+n3);
}
Flt PerlinSimplexNoise::noise(Dbl x, Dbl y, Dbl z, Dbl w)C
{
   // The skewing and unskewing factors are hairy again for the 4D case
   #define F4 0.309016994f // F4=(Sqrt(5.0)-1.0)/4.0
   #define G4 0.138196601f // G4=(5.0-Sqrt(5.0))/20.0

   Flt n0, n1, n2, n3, n4; // Noise contributions from the five corners

   // Skew the (x,y,z,w)space to determine which cell of 24 simplices we're in
   Flt s=(x+y+z+w)*F4; // Factor for 4D skewing
   Flt xs=x+s;
   Flt ys=y+s;
   Flt zs=z+s;
   Flt ws=w+s;
   Int i=Floor(xs);
   Int j=Floor(ys);
   Int k=Floor(zs);
   Int l=Floor(ws);

   Flt t=(i+j+k+l)*G4; // Factor for 4D unskewing
   Flt X0=i-t; // Unskew the cell origin back to (x,y,z,w)space
   Flt Y0=j-t;
   Flt Z0=k-t;
   Flt W0=l-t;

   Flt x0=x-X0;  // The x,y,z,w distances from the cell origin
   Flt y0=y-Y0;
   Flt z0=z-Z0;
   Flt w0=w-W0;

   // For the 4D case, the simplex is a 4D shape I won't even try to describe.
   // To find out which of the 24 possible simplices we're in, we need to
   // determine the magnitude ordering of x0, y0, z0 and w0.
   // The method below is a good way of finding the ordering of x,y,z,w and
   // then find the correct traversal order for the simplex we're in.
   // First, six pair-wise comparisons are performed between each possible pair
   // of the four coordinates, and the results are used to add up binary bits
   // for an integer index.
   Int c1=(x0>y0)? 32 : 0;
   Int c2=(x0>z0)? 16 : 0;
   Int c3=(y0>z0)? 8 : 0;
   Int c4=(x0>w0)? 4 : 0;
   Int c5=(y0>w0)? 2 : 0;
   Int c6=(z0>w0)? 1 : 0;
   Int c=c1+c2+c3+c4+c5+c6;

   Int i1, j1, k1, l1; // The integer offsets for the second simplex corner
   Int i2, j2, k2, l2; // The integer offsets for the third simplex corner
   Int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner

   // simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
   // Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
   // impossible. Only the 24 indices which have non-zero entries make any sense.
   // We use a thresholding to set the coordinates in turn from the largest magnitude.
   // The number 3 in the "Simplex" array is at the position of the largest coordinate.
   i1=Simplex[c][0]>=3 ? 1 : 0;
   j1=Simplex[c][1]>=3 ? 1 : 0;
   k1=Simplex[c][2]>=3 ? 1 : 0;
   l1=Simplex[c][3]>=3 ? 1 : 0;
   // The number 2 in the "Simplex" array is at the second largest coordinate.
   i2=Simplex[c][0]>=2 ? 1 : 0;
   j2=Simplex[c][1]>=2 ? 1 : 0;
   k2=Simplex[c][2]>=2 ? 1 : 0;
   l2=Simplex[c][3]>=2 ? 1 : 0;
   // The number 1 in the "Simplex" array is at the second smallest coordinate.
   i3=Simplex[c][0]>=1 ? 1 : 0;
   j3=Simplex[c][1]>=1 ? 1 : 0;
   k3=Simplex[c][2]>=1 ? 1 : 0;
   l3=Simplex[c][3]>=1 ? 1 : 0;
   // The fifth corner has all coordinate offsets=1, so no need to look that up.

   Flt x1=x0-i1+G4; // Offsets for second corner in (x,y,z,w)coords
   Flt y1=y0-j1+G4;
   Flt z1=z0-k1+G4;
   Flt w1=w0-l1+G4;
   Flt x2=x0-i2+2.0f*G4; // Offsets for third corner in (x,y,z,w)coords
   Flt y2=y0-j2+2.0f*G4;
   Flt z2=z0-k2+2.0f*G4;
   Flt w2=w0-l2+2.0f*G4;
   Flt x3=x0-i3+3.0f*G4; // Offsets for fourth corner in (x,y,z,w)coords
   Flt y3=y0-j3+3.0f*G4;
   Flt z3=z0-k3+3.0f*G4;
   Flt w3=w0-l3+3.0f*G4;
   Flt x4=x0-1.0f+4.0f*G4; // Offsets for last corner in (x,y,z,w)coords
   Flt y4=y0-1.0f+4.0f*G4;
   Flt z4=z0-1.0f+4.0f*G4;
   Flt w4=w0-1.0f+4.0f*G4;

   // Wrap the integer indices at 256, to avoid indexing p[] out of bounds
   Int ii=i&0xff;
   Int jj=j&0xff;
   Int kk=k&0xff;
   Int ll=l&0xff;

   // Calculate the contribution from the five corners
   Flt t0=0.6f-x0*x0-y0*y0-z0*z0-w0*w0;
   if(t0<0.0f)n0=0.0f;else
   {
      t0*=t0;
      n0=t0*t0*Grad(p[ii+p[jj+p[kk+p[ll]]]], x0, y0, z0, w0);
   }

   Flt t1=0.6f-x1*x1-y1*y1-z1*z1-w1*w1;
   if(t1<0.0f)n1=0.0f;else
   {
      t1*=t1;
      n1=t1*t1*Grad(p[ii+i1+p[jj+j1+p[kk+k1+p[ll+l1]]]], x1, y1, z1, w1);
   }

   Flt t2=0.6f-x2*x2-y2*y2-z2*z2-w2*w2;
   if(t2<0.0f)n2=0.0f;else
   {
      t2*=t2;
      n2=t2*t2*Grad(p[ii+i2+p[jj+j2+p[kk+k2+p[ll+l2]]]], x2, y2, z2, w2);
   }

   Flt t3=0.6f-x3*x3-y3*y3-z3*z3-w3*w3;
   if(t3<0.0f)n3=0.0f;else
   {
      t3*=t3;
      n3=t3*t3*Grad(p[ii+i3+p[jj+j3+p[kk+k3+p[ll+l3]]]], x3, y3, z3, w3);
   }

   Flt t4=0.6f-x4*x4-y4*y4-z4*z4-w4*w4;
   if(t4<0.0f)n4=0.0f;else
   {
      t4*=t4;
      n4=t4*t4*Grad(p[ii+1+p[jj+1+p[kk+1+p[ll+1]]]], x4, y4, z4, w4);
   }

   // Sum up and scale the result to cover the range [-1,1]
   return 27.0f*(n0+n1+n2+n3+n4); // TODO: The scale factor is preliminary!
}
/******************************************************************************/
#endif
/******************************************************************************/
// OPEN SIMPLEX NOISE
// Ported by Stephen M. Cameron from Kurt Spencer's Java implementation
/******************************************************************************/
#define STRETCH_CONSTANT_2D (-0.211324865405187) // (1 / Sqrt(2 + 1) - 1 ) / 2
#define SQUISH_CONSTANT_2D  (0.366025403784439)  // (Sqrt(2 + 1) -1) / 2
#define STRETCH_CONSTANT_3D (-1.0 / 6.0)         // (1 / Sqrt(3 + 1) - 1) / 3
#define SQUISH_CONSTANT_3D  (1.0 / 3.0)          // (Sqrt(3+1)-1)/3
#define STRETCH_CONSTANT_4D (-0.138196601125011) // (1 / Sqrt(4 + 1) - 1) / 4
#define SQUISH_CONSTANT_4D  (0.309016994374947)  // (Sqrt(4 + 1) - 1) / 4

// Gradients for 2D. They approximate the directions to the vertices of an octagon from the center
static const I8 gradients2D[]=
{
    5,  2,    2,  5,
   -5,  2,   -2,  5,
    5, -2,    2, -5,
   -5, -2,   -2, -5,
};

// Gradients for 3D. They approximate the directions to the vertices of a rhombicuboctahedron from the center, skewed so that the triangular and square facets can be inscribed inside circles of the same radius
static const I8 gradients3D[]=
{
   -11,  4,  4,     -4,  11,  4,    -4,  4,  11,
    11,  4,  4,      4,  11,  4,     4,  4,  11,
   -11, -4,  4,     -4, -11,  4,    -4, -4,  11,
    11, -4,  4,      4, -11,  4,     4, -4,  11,
   -11,  4, -4,     -4,  11, -4,    -4,  4, -11,
    11,  4, -4,      4,  11, -4,     4,  4, -11,
   -11, -4, -4,     -4, -11, -4,    -4, -4, -11,
    11, -4, -4,      4, -11, -4,     4, -4, -11,
};

// Gradients for 4D. They approximate the directions to the vertices of a disprismatotesseractihexadecachoron from the center, skewed so that the tetrahedral and cubic facets can be inscribed inside spheres of the same radius.
static const I8 gradients4D[]=
{
    3,  1,  1,  1,      1,  3,  1,  1,      1,  1,  3,  1,      1,  1,  1,  3,
   -3,  1,  1,  1,     -1,  3,  1,  1,     -1,  1,  3,  1,     -1,  1,  1,  3,
    3, -1,  1,  1,      1, -3,  1,  1,      1, -1,  3,  1,      1, -1,  1,  3,
   -3, -1,  1,  1,     -1, -3,  1,  1,     -1, -1,  3,  1,     -1, -1,  1,  3,
    3,  1, -1,  1,      1,  3, -1,  1,      1,  1, -3,  1,      1,  1, -1,  3,
   -3,  1, -1,  1,     -1,  3, -1,  1,     -1,  1, -3,  1,     -1,  1, -1,  3,
    3, -1, -1,  1,      1, -3, -1,  1,      1, -1, -3,  1,      1, -1, -1,  3,
   -3, -1, -1,  1,     -1, -3, -1,  1,     -1, -1, -3,  1,     -1, -1, -1,  3,
    3,  1,  1, -1,      1,  3,  1, -1,      1,  1,  3, -1,      1,  1,  1, -3,
   -3,  1,  1, -1,     -1,  3,  1, -1,     -1,  1,  3, -1,     -1,  1,  1, -3,
    3, -1,  1, -1,      1, -3,  1, -1,      1, -1,  3, -1,      1, -1,  1, -3,
   -3, -1,  1, -1,     -1, -3,  1, -1,     -1, -1,  3, -1,     -1, -1,  1, -3,
    3,  1, -1, -1,      1,  3, -1, -1,      1,  1, -3, -1,      1,  1, -1, -3,
   -3,  1, -1, -1,     -1,  3, -1, -1,     -1,  1, -3, -1,     -1,  1, -1, -3,
    3, -1, -1, -1,      1, -3, -1, -1,      1, -1, -3, -1,      1, -1, -1, -3,
   -3, -1, -1, -1,     -1, -3, -1, -1,     -1, -1, -3, -1,     -1, -1, -1, -3,
};
/******************************************************************************/
SimplexNoise::SimplexNoise(UInt seed)
{
   Randomizer rnd(seed, 0, 0, 0);
    REP(256)p[i]=i;
   FREP(255)Swap(p[i], p[rnd(i, 255)]); // 'RandomizeOrder'
   REPAO(permGradIndex3D)=(p[i]%(Elms(gradients3D)/3))*3;
}
/******************************************************************************/
Dbl SimplexNoise::extrapolate2(Int xsb, Int ysb, Dbl dx, Dbl dy)C
{
   Int index=p[(p[xsb&0xFF]+ysb)&0xFF]&0x0E; // 0x0E mask limits to 0..15 (0xF) elements and disables bit 1 so index+1 is in range
   return gradients2D[index  ]*dx
         +gradients2D[index+1]*dy;
}
Dbl SimplexNoise::extrapolate3(Int xsb, Int ysb, Int zsb, Dbl dx, Dbl dy, Dbl dz)C
{
   Int index=permGradIndex3D[(p[(p[xsb&0xFF]+ysb)&0xFF]+zsb)&0xFF];
   return gradients3D[index  ]*dx
         +gradients3D[index+1]*dy
         +gradients3D[index+2]*dz;
}
Dbl SimplexNoise::extrapolate4(Int xsb, Int ysb, Int zsb, Int wsb, Dbl dx, Dbl dy, Dbl dz, Dbl dw)C
{
   Int index=p[(p[(p[(p[xsb&0xFF]+ysb)&0xFF]+zsb)&0xFF]+wsb)&0xFF]&0xFC; // 0xFC mask limits to 0..255 (0xFF) elements and disables bit 1+2 so index+1..3 are in range
   return gradients4D[index  ]*dx
         +gradients4D[index+1]*dy
         +gradients4D[index+2]*dz
         +gradients4D[index+3]*dw;
}
/******************************************************************************/
Flt SimplexNoise::noise(Dbl x)C {return noise(x, 0);}
/******************************************************************************/
Flt SimplexNoise::noise(Dbl x, Dbl y)C
{
   // Place input coordinates onto grid.
   Dbl stretchOffset=(x+y)*STRETCH_CONSTANT_2D;
   Dbl xs=x+stretchOffset;
   Dbl ys=y+stretchOffset;
      
   // Floor to get grid coordinates of rhombus (stretched square) super-cell origin.
   Int xsb=Floor(xs);
   Int ysb=Floor(ys);
      
   // Skew out to get actual coordinates of rhombus origin. We'll need these later.
   Dbl squishOffset=(xsb+ysb)*SQUISH_CONSTANT_2D;
   Dbl xb=xsb+squishOffset;
   Dbl yb=ysb+squishOffset;
      
   // Compute grid coordinates relative to rhombus origin.
   Dbl xins=xs-xsb;
   Dbl yins=ys-ysb;
      
   // Sum those together to get a value that determines which region we're in.
   Dbl inSum=xins+yins;

   // Positions relative to origin point.
   Dbl dx0=x-xb;
   Dbl dy0=y-yb;
      
   // We'll be defining these inside the next block and using them afterwards.
   Dbl dx_ext, dy_ext;
   Int xsv_ext, ysv_ext;

   Dbl dx1;
   Dbl dy1;
   Dbl attn1;
   Dbl dx2;
   Dbl dy2;
   Dbl attn2;
   Dbl zins;
   Dbl attn0;
   Dbl attn_ext;

   Dbl value=0;

   // Contribution (1,0)
   dx1=dx0-1-SQUISH_CONSTANT_2D;
   dy1=dy0-0-SQUISH_CONSTANT_2D;
   attn1=2-dx1*dx1-dy1*dy1;
   if(attn1>0)
   {
      attn1*=attn1;
      value+=attn1*attn1*extrapolate2(xsb+1, ysb+0, dx1, dy1);
   }

   // Contribution (0,1)
   dx2=dx0-0-SQUISH_CONSTANT_2D;
   dy2=dy0-1-SQUISH_CONSTANT_2D;
   attn2=2-dx2*dx2-dy2*dy2;
   if(attn2>0)
   {
      attn2*=attn2;
      value+=attn2*attn2*extrapolate2(xsb+0, ysb+1, dx2, dy2);
   }
      
   if(inSum<=1) // We're inside the triangle (2-Simplex) at (0,0)
   {
      zins=1-inSum;
      if(zins>xins || zins>yins) // (0,0) is one of the closest two triangular vertices
      {
         if(xins>yins)
         {
            xsv_ext=xsb+1;
            ysv_ext=ysb-1;
            dx_ext=dx0-1;
            dy_ext=dy0+1;
         }else
         {
            xsv_ext=xsb-1;
            ysv_ext=ysb+1;
            dx_ext=dx0+1;
            dy_ext=dy0-1;
         }
      }else // (1,0) and (0,1) are the closest two vertices.
      {
         xsv_ext=xsb+1;
         ysv_ext=ysb+1;
         dx_ext=dx0-1-2*SQUISH_CONSTANT_2D;
         dy_ext=dy0-1-2*SQUISH_CONSTANT_2D;
      }
   }else // We're inside the triangle (2-Simplex) at (1,1)
   {
      zins=2-inSum;
      if(zins<xins || zins<yins) // (0,0) is one of the closest two triangular vertices
      {
         if(xins>yins)
         {
            xsv_ext=xsb+2;
            ysv_ext=ysb+0;
            dx_ext=dx0-2-2*SQUISH_CONSTANT_2D;
            dy_ext=dy0+0-2*SQUISH_CONSTANT_2D;
         }else
         {
            xsv_ext=xsb+0;
            ysv_ext=ysb+2;
            dx_ext=dx0+0-2*SQUISH_CONSTANT_2D;
            dy_ext=dy0-2-2*SQUISH_CONSTANT_2D;
         }
      }else // (1,0) and (0,1) are the closest two vertices.
      {
         dx_ext=dx0;
         dy_ext=dy0;
         xsv_ext=xsb;
         ysv_ext=ysb;
      }
      xsb+=1;
      ysb+=1;
      dx0=dx0-1-2*SQUISH_CONSTANT_2D;
      dy0=dy0-1-2*SQUISH_CONSTANT_2D;
   }
      
   // Contribution (0,0) or (1,1)
   attn0=2-dx0*dx0-dy0*dy0;
   if(attn0>0)
   {
      attn0*=attn0;
      value+=attn0*attn0*extrapolate2(xsb, ysb, dx0, dy0);
   }
   
   // Extra Vertex
   attn_ext=2-dx_ext*dx_ext-dy_ext*dy_ext;
   if(attn_ext>0)
   {
      attn_ext*=attn_ext;
      value+=attn_ext*attn_ext*extrapolate2(xsv_ext, ysv_ext, dx_ext, dy_ext);
   }
   
   return value/(SIMPLEX_NORM_CONSTANT_2D);
}
/******************************************************************************/
Flt SimplexNoise::noise(Dbl x, Dbl y, Dbl z)C
{
   // Place input coordinates on simplectic honeycomb.
   Dbl stretchOffset=(x+y+z)*STRETCH_CONSTANT_3D;
   Dbl xs=x+stretchOffset;
   Dbl ys=y+stretchOffset;
   Dbl zs=z+stretchOffset;
   
   // Floor to get simplectic honeycomb coordinates of rhombohedron (stretched cube) super-cell origin.
   Int xsb=Floor(xs);
   Int ysb=Floor(ys);
   Int zsb=Floor(zs);
   
   // Skew out to get actual coordinates of rhombohedron origin. We'll need these later.
   Dbl squishOffset=(xsb+ysb+zsb)*SQUISH_CONSTANT_3D;
   Dbl xb=xsb+squishOffset;
   Dbl yb=ysb+squishOffset;
   Dbl zb=zsb+squishOffset;
   
   // Compute simplectic honeycomb coordinates relative to rhombohedral origin.
   Dbl xins=xs-xsb;
   Dbl yins=ys-ysb;
   Dbl zins=zs-zsb;
   
   // Sum those together to get a value that determines which region we're in.
   Dbl inSum=xins+yins+zins;

   // Positions relative to origin point.
   Dbl dx0=x-xb;
   Dbl dy0=y-yb;
   Dbl dz0=z-zb;
   
   // We'll be defining these inside the next block and using them afterwards.
   Dbl dx_ext0, dy_ext0, dz_ext0;
   Dbl dx_ext1, dy_ext1, dz_ext1;
   Int xsv_ext0, ysv_ext0, zsv_ext0;
   Int xsv_ext1, ysv_ext1, zsv_ext1;

   Dbl wins;
   I8 c, c1, c2;
   I8 aPoint, bPoint;
   Dbl aScore, bScore;
   Int aIsFurtherSide;
   Int bIsFurtherSide;
   Dbl p1, p2, p3;
   Dbl score;
   Dbl attn0, attn1, attn2, attn3, attn4, attn5, attn6;
   Dbl dx1, dy1, dz1;
   Dbl dx2, dy2, dz2;
   Dbl dx3, dy3, dz3;
   Dbl dx4, dy4, dz4;
   Dbl dx5, dy5, dz5;
   Dbl dx6, dy6, dz6;
   Dbl attn_ext0, attn_ext1;
   
   Dbl value=0;
   if(inSum<=1) // We're inside the tetrahedron (3-Simplex) at (0,0,0)
   {
      // Determine which two of (0,0,1), (0,1,0), (1,0,0) are closest.
      aPoint=0x01;
      aScore=xins;
      bPoint=0x02;
      bScore=yins;
      if(aScore>=bScore && zins>bScore)
      {
         bScore=zins;
         bPoint=0x04;
      }else
      if(aScore<bScore && zins>aScore)
      {
         aScore=zins;
         aPoint=0x04;
      }
      
      // Now we determine the two lattice points not part of the tetrahedron that may contribute. This depends on the closest two tetrahedral vertices, including (0,0,0)
      wins=1-inSum;
      if(wins>aScore || wins>bScore) // (0,0,0) is one of the closest two tetrahedral vertices.
      {
         c=(bScore>aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
         
         if((c&0x01) == 0)
         {
            xsv_ext0=xsb-1;
            xsv_ext1=xsb;
            dx_ext0=dx0+1;
            dx_ext1=dx0;
         }else
         {
            xsv_ext0=xsv_ext1=xsb+1;
            dx_ext0=dx_ext1=dx0-1;
         }

         if((c&0x02) == 0)
         {
            ysv_ext0=ysv_ext1=ysb;
            dy_ext0=dy_ext1=dy0;
            if((c&0x01) == 0)
            {
               ysv_ext1-=1;
               dy_ext1+=1;
            }else
            {
               ysv_ext0-=1;
               dy_ext0+=1;
            }
         }else
         {
            ysv_ext0=ysv_ext1=ysb+1;
            dy_ext0=dy_ext1=dy0-1;
         }

         if((c&0x04) == 0)
         {
            zsv_ext0=zsb;
            zsv_ext1=zsb-1;
            dz_ext0=dz0;
            dz_ext1=dz0+1;
         }else
         {
            zsv_ext0=zsv_ext1=zsb+1;
            dz_ext0=dz_ext1=dz0-1;
         }
      }else // (0,0,0) is not one of the closest two tetrahedral vertices.
      {
         c=(I8)(aPoint | bPoint); // Our two extra vertices are determined by the closest two.
         
         if((c&0x01) == 0)
         {
            xsv_ext0=xsb;
            xsv_ext1=xsb-1;
            dx_ext0=dx0-2*SQUISH_CONSTANT_3D;
            dx_ext1=dx0+1-SQUISH_CONSTANT_3D;
         }else
         {
            xsv_ext0=xsv_ext1=xsb+1;
            dx_ext0=dx0-1-2*SQUISH_CONSTANT_3D;
            dx_ext1=dx0-1-SQUISH_CONSTANT_3D;
         }

         if((c&0x02) == 0)
         {
            ysv_ext0=ysb;
            ysv_ext1=ysb-1;
            dy_ext0=dy0-2*SQUISH_CONSTANT_3D;
            dy_ext1=dy0+1-SQUISH_CONSTANT_3D;
         }else
         {
            ysv_ext0=ysv_ext1=ysb+1;
            dy_ext0=dy0-1-2*SQUISH_CONSTANT_3D;
            dy_ext1=dy0-1-SQUISH_CONSTANT_3D;
         }

         if((c&0x04) == 0)
         {
            zsv_ext0=zsb;
            zsv_ext1=zsb-1;
            dz_ext0=dz0-2*SQUISH_CONSTANT_3D;
            dz_ext1=dz0+1-SQUISH_CONSTANT_3D;
         }else
         {
            zsv_ext0=zsv_ext1=zsb+1;
            dz_ext0=dz0-1-2*SQUISH_CONSTANT_3D;
            dz_ext1=dz0-1-SQUISH_CONSTANT_3D;
         }
      }

      // Contribution (0,0,0)
      attn0=2-dx0*dx0-dy0*dy0-dz0*dz0;
      if(attn0>0)
      {
         attn0*=attn0;
         value+=attn0*attn0*extrapolate3(xsb+0, ysb+0, zsb+0, dx0, dy0, dz0);
      }

      // Contribution (1,0,0)
      dx1=dx0-1-SQUISH_CONSTANT_3D;
      dy1=dy0-0-SQUISH_CONSTANT_3D;
      dz1=dz0-0-SQUISH_CONSTANT_3D;
      attn1=2-dx1*dx1-dy1*dy1-dz1*dz1;
      if(attn1>0)
      {
         attn1*=attn1;
         value+=attn1*attn1*extrapolate3(xsb+1, ysb+0, zsb+0, dx1, dy1, dz1);
      }

      // Contribution (0,1,0)
      dx2=dx0-0-SQUISH_CONSTANT_3D;
      dy2=dy0-1-SQUISH_CONSTANT_3D;
      dz2=dz1;
      attn2=2-dx2*dx2-dy2*dy2-dz2*dz2;
      if(attn2>0)
      {
         attn2*=attn2;
         value+=attn2*attn2*extrapolate3(xsb+0, ysb+1, zsb+0, dx2, dy2, dz2);
      }

      // Contribution (0,0,1)
      dx3=dx2;
      dy3=dy1;
      dz3=dz0-1-SQUISH_CONSTANT_3D;
      attn3=2-dx3*dx3-dy3*dy3-dz3*dz3;
      if(attn3>0)
      {
         attn3*=attn3;
         value+=attn3*attn3*extrapolate3(xsb+0, ysb+0, zsb+1, dx3, dy3, dz3);
      }
   }else
   if(inSum>=2) // We're inside the tetrahedron (3-Simplex) at (1,1,1)
   {
      // Determine which two tetrahedral vertices are the closest, out of (1,1,0), (1,0,1), (0,1,1) but not (1,1,1).
      aPoint=0x06;
      aScore=xins;
      bPoint=0x05;
      bScore=yins;
      if(aScore<=bScore && zins<bScore)
      {
         bScore=zins;
         bPoint=0x03;
      }else
      if(aScore>bScore && zins<aScore)
      {
         aScore=zins;
         aPoint=0x03;
      }
      
      // Now we determine the two lattice points not part of the tetrahedron that may contribute. This depends on the closest two tetrahedral vertices, including (1,1,1)
      wins=3-inSum;
      if(wins<aScore || wins<bScore) // (1,1,1) is one of the closest two tetrahedral vertices.
      {
         c=(bScore<aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
         
         if((c&0x01) != 0)
         {
            xsv_ext0=xsb+2;
            xsv_ext1=xsb+1;
            dx_ext0=dx0-2-3*SQUISH_CONSTANT_3D;
            dx_ext1=dx0-1-3*SQUISH_CONSTANT_3D;
         }else
         {
            xsv_ext0=xsv_ext1=xsb;
            dx_ext0=dx_ext1=dx0-3*SQUISH_CONSTANT_3D;
         }

         if((c&0x02) != 0)
         {
            ysv_ext0=ysv_ext1=ysb+1;
            dy_ext0=dy_ext1=dy0-1-3*SQUISH_CONSTANT_3D;
            if((c&0x01) != 0)
            {
               ysv_ext1+=1;
               dy_ext1-=1;
            }else
            {
               ysv_ext0+=1;
               dy_ext0-=1;
            }
         }else
         {
            ysv_ext0=ysv_ext1=ysb;
            dy_ext0=dy_ext1=dy0-3*SQUISH_CONSTANT_3D;
         }

         if((c&0x04) != 0)
         {
            zsv_ext0=zsb+1;
            zsv_ext1=zsb+2;
            dz_ext0=dz0-1-3*SQUISH_CONSTANT_3D;
            dz_ext1=dz0-2-3*SQUISH_CONSTANT_3D;
         }else
         {
            zsv_ext0=zsv_ext1=zsb;
            dz_ext0=dz_ext1=dz0-3*SQUISH_CONSTANT_3D;
         }
      }else // (1,1,1) is not one of the closest two tetrahedral vertices.
      {
         c=(I8)(aPoint&bPoint); // Our two extra vertices are determined by the closest two.
         
         if((c&0x01) != 0)
         {
            xsv_ext0=xsb+1;
            xsv_ext1=xsb+2;
            dx_ext0=dx0-1-SQUISH_CONSTANT_3D;
            dx_ext1=dx0-2-2*SQUISH_CONSTANT_3D;
         }else
         {
            xsv_ext0=xsv_ext1=xsb;
            dx_ext0=dx0-SQUISH_CONSTANT_3D;
            dx_ext1=dx0-2*SQUISH_CONSTANT_3D;
         }

         if((c&0x02) != 0)
         {
            ysv_ext0=ysb+1;
            ysv_ext1=ysb+2;
            dy_ext0=dy0-1-SQUISH_CONSTANT_3D;
            dy_ext1=dy0-2-2*SQUISH_CONSTANT_3D;
         }else
         {
            ysv_ext0=ysv_ext1=ysb;
            dy_ext0=dy0-SQUISH_CONSTANT_3D;
            dy_ext1=dy0-2*SQUISH_CONSTANT_3D;
         }

         if((c&0x04) != 0)
         {
            zsv_ext0=zsb+1;
            zsv_ext1=zsb+2;
            dz_ext0=dz0-1-SQUISH_CONSTANT_3D;
            dz_ext1=dz0-2-2*SQUISH_CONSTANT_3D;
         }else
         {
            zsv_ext0=zsv_ext1=zsb;
            dz_ext0=dz0-SQUISH_CONSTANT_3D;
            dz_ext1=dz0-2*SQUISH_CONSTANT_3D;
         }
      }
      
      // Contribution (1,1,0)
      dx3=dx0-1-2*SQUISH_CONSTANT_3D;
      dy3=dy0-1-2*SQUISH_CONSTANT_3D;
      dz3=dz0-0-2*SQUISH_CONSTANT_3D;
      attn3=2-dx3*dx3-dy3*dy3-dz3*dz3;
      if(attn3>0)
      {
         attn3*=attn3;
         value+=attn3*attn3*extrapolate3(xsb+1, ysb+1, zsb+0, dx3, dy3, dz3);
      }

      // Contribution (1,0,1)
      dx2=dx3;
      dy2=dy0-0-2*SQUISH_CONSTANT_3D;
      dz2=dz0-1-2*SQUISH_CONSTANT_3D;
      attn2=2-dx2*dx2-dy2*dy2-dz2*dz2;
      if(attn2>0)
      {
         attn2*=attn2;
         value+=attn2*attn2*extrapolate3(xsb+1, ysb+0, zsb+1, dx2, dy2, dz2);
      }

      // Contribution (0,1,1)
      dx1=dx0-0-2*SQUISH_CONSTANT_3D;
      dy1=dy3;
      dz1=dz2;
      attn1=2-dx1*dx1-dy1*dy1-dz1*dz1;
      if(attn1>0)
      {
         attn1*=attn1;
         value+=attn1*attn1*extrapolate3(xsb+0, ysb+1, zsb+1, dx1, dy1, dz1);
      }

      // Contribution (1,1,1)
      dx0=dx0-1-3*SQUISH_CONSTANT_3D;
      dy0=dy0-1-3*SQUISH_CONSTANT_3D;
      dz0=dz0-1-3*SQUISH_CONSTANT_3D;
      attn0=2-dx0*dx0-dy0*dy0-dz0*dz0;
      if(attn0>0)
      {
         attn0*=attn0;
         value+=attn0*attn0*extrapolate3(xsb+1, ysb+1, zsb+1, dx0, dy0, dz0);
      }
   }else // We're inside the octahedron (Rectified 3-Simplex) in between. Decide between point (0,0,1) and (1,1,0) as closest
   {
      p1=xins+yins;
      if(p1>1)
      {
         aScore=p1-1;
         aPoint=0x03;
         aIsFurtherSide=1;
      }else
      {
         aScore=1-p1;
         aPoint=0x04;
         aIsFurtherSide=0;
      }

      // Decide between point (0,1,0) and (1,0,1) as closest
      p2=xins+zins;
      if(p2>1)
      {
         bScore=p2-1;
         bPoint=0x05;
         bIsFurtherSide=1;
      }else
      {
         bScore=1-p2;
         bPoint=0x02;
         bIsFurtherSide=0;
      }
      
      // The closest out of the two (1,0,0) and (0,1,1) will replace the furthest out of the two decided above, if closer.
      p3=yins+zins;
      if(p3>1)
      {
         score=p3-1;
         if(aScore<=bScore && aScore<score)
         {
            aScore=score;
            aPoint=0x06;
            aIsFurtherSide=1;
         }else
         if(aScore>bScore && bScore<score)
         {
            bScore=score;
            bPoint=0x06;
            bIsFurtherSide=1;
         }
      }else
      {
         score=1-p3;
         if(aScore<=bScore && aScore<score)
         {
            aScore=score;
            aPoint=0x01;
            aIsFurtherSide=0;
         }else
         if(aScore>bScore && bScore<score)
         {
            bScore=score;
            bPoint=0x01;
            bIsFurtherSide=0;
         }
      }
      
      // Where each of the two closest points are determines how the extra two vertices are calculated.
      if(aIsFurtherSide == bIsFurtherSide)
      {
         if(aIsFurtherSide) // Both closest points on (1,1,1) side
         {
            // One of the two extra points is (1,1,1)
            dx_ext0=dx0-1-3*SQUISH_CONSTANT_3D;
            dy_ext0=dy0-1-3*SQUISH_CONSTANT_3D;
            dz_ext0=dz0-1-3*SQUISH_CONSTANT_3D;
            xsv_ext0=xsb+1;
            ysv_ext0=ysb+1;
            zsv_ext0=zsb+1;

            // Other extra point is based on the shared axis.
            c=(I8)(aPoint&bPoint);
            if((c&0x01) != 0)
            {
               dx_ext1=dx0-2-2*SQUISH_CONSTANT_3D;
               dy_ext1=dy0-2*SQUISH_CONSTANT_3D;
               dz_ext1=dz0-2*SQUISH_CONSTANT_3D;
               xsv_ext1=xsb+2;
               ysv_ext1=ysb;
               zsv_ext1=zsb;
            }else
            if((c&0x02) != 0)
            {
               dx_ext1=dx0-2*SQUISH_CONSTANT_3D;
               dy_ext1=dy0-2-2*SQUISH_CONSTANT_3D;
               dz_ext1=dz0-2*SQUISH_CONSTANT_3D;
               xsv_ext1=xsb;
               ysv_ext1=ysb+2;
               zsv_ext1=zsb;
            }else
            {
               dx_ext1=dx0-2*SQUISH_CONSTANT_3D;
               dy_ext1=dy0-2*SQUISH_CONSTANT_3D;
               dz_ext1=dz0-2-2*SQUISH_CONSTANT_3D;
               xsv_ext1=xsb;
               ysv_ext1=ysb;
               zsv_ext1=zsb+2;
            }
         }else // Both closest points on (0,0,0) side
         {
            // One of the two extra points is (0,0,0)
            dx_ext0=dx0;
            dy_ext0=dy0;
            dz_ext0=dz0;
            xsv_ext0=xsb;
            ysv_ext0=ysb;
            zsv_ext0=zsb;

            // Other extra point is based on the omitted axis.
            c=(I8)(aPoint | bPoint);
            if((c&0x01) == 0)
            {
               dx_ext1=dx0+1-SQUISH_CONSTANT_3D;
               dy_ext1=dy0-1-SQUISH_CONSTANT_3D;
               dz_ext1=dz0-1-SQUISH_CONSTANT_3D;
               xsv_ext1=xsb-1;
               ysv_ext1=ysb+1;
               zsv_ext1=zsb+1;
            }else
            if((c&0x02) == 0)
            {
               dx_ext1=dx0-1-SQUISH_CONSTANT_3D;
               dy_ext1=dy0+1-SQUISH_CONSTANT_3D;
               dz_ext1=dz0-1-SQUISH_CONSTANT_3D;
               xsv_ext1=xsb+1;
               ysv_ext1=ysb-1;
               zsv_ext1=zsb+1;
            }else
            {
               dx_ext1=dx0-1-SQUISH_CONSTANT_3D;
               dy_ext1=dy0-1-SQUISH_CONSTANT_3D;
               dz_ext1=dz0+1-SQUISH_CONSTANT_3D;
               xsv_ext1=xsb+1;
               ysv_ext1=ysb+1;
               zsv_ext1=zsb-1;
            }
         }
      }else // One point on (0,0,0) side, one point on (1,1,1) side
      {
         if(aIsFurtherSide)
         {
            c1=aPoint;
            c2=bPoint;
         }else
         {
            c1=bPoint;
            c2=aPoint;
         }

         // One contribution is a permutation of (1,1,-1)
         if((c1&0x01) == 0)
         {
            dx_ext0=dx0+1-SQUISH_CONSTANT_3D;
            dy_ext0=dy0-1-SQUISH_CONSTANT_3D;
            dz_ext0=dz0-1-SQUISH_CONSTANT_3D;
            xsv_ext0=xsb-1;
            ysv_ext0=ysb+1;
            zsv_ext0=zsb+1;
         }else
         if((c1&0x02) == 0)
         {
            dx_ext0=dx0-1-SQUISH_CONSTANT_3D;
            dy_ext0=dy0+1-SQUISH_CONSTANT_3D;
            dz_ext0=dz0-1-SQUISH_CONSTANT_3D;
            xsv_ext0=xsb+1;
            ysv_ext0=ysb-1;
            zsv_ext0=zsb+1;
         }else
         {
            dx_ext0=dx0-1-SQUISH_CONSTANT_3D;
            dy_ext0=dy0-1-SQUISH_CONSTANT_3D;
            dz_ext0=dz0+1-SQUISH_CONSTANT_3D;
            xsv_ext0=xsb+1;
            ysv_ext0=ysb+1;
            zsv_ext0=zsb-1;
         }

         // One contribution is a permutation of (0,0,2)
         dx_ext1=dx0-2*SQUISH_CONSTANT_3D;
         dy_ext1=dy0-2*SQUISH_CONSTANT_3D;
         dz_ext1=dz0-2*SQUISH_CONSTANT_3D;
         xsv_ext1=xsb;
         ysv_ext1=ysb;
         zsv_ext1=zsb;
         if((c2&0x01) != 0)
         {
            dx_ext1-=2;
            xsv_ext1+=2;
         }else
         if((c2&0x02) != 0)
         {
            dy_ext1-=2;
            ysv_ext1+=2;
         }else
         {
            dz_ext1-=2;
            zsv_ext1+=2;
         }
      }

      // Contribution (1,0,0)
      dx1=dx0-1-SQUISH_CONSTANT_3D;
      dy1=dy0-0-SQUISH_CONSTANT_3D;
      dz1=dz0-0-SQUISH_CONSTANT_3D;
      attn1=2-dx1*dx1-dy1*dy1-dz1*dz1;
      if(attn1>0)
      {
         attn1*=attn1;
         value+=attn1*attn1*extrapolate3(xsb+1, ysb+0, zsb+0, dx1, dy1, dz1);
      }

      // Contribution (0,1,0)
      dx2=dx0-0-SQUISH_CONSTANT_3D;
      dy2=dy0-1-SQUISH_CONSTANT_3D;
      dz2=dz1;
      attn2=2-dx2*dx2-dy2*dy2-dz2*dz2;
      if(attn2>0)
      {
         attn2*=attn2;
         value+=attn2*attn2*extrapolate3(xsb+0, ysb+1, zsb+0, dx2, dy2, dz2);
      }

      // Contribution (0,0,1)
      dx3=dx2;
      dy3=dy1;
      dz3=dz0-1-SQUISH_CONSTANT_3D;
      attn3=2-dx3*dx3-dy3*dy3-dz3*dz3;
      if(attn3>0)
      {
         attn3*=attn3;
         value+=attn3*attn3*extrapolate3(xsb+0, ysb+0, zsb+1, dx3, dy3, dz3);
      }

      // Contribution (1,1,0)
      dx4=dx0-1-2*SQUISH_CONSTANT_3D;
      dy4=dy0-1-2*SQUISH_CONSTANT_3D;
      dz4=dz0-0-2*SQUISH_CONSTANT_3D;
      attn4=2-dx4*dx4-dy4*dy4-dz4*dz4;
      if(attn4>0)
      {
         attn4*=attn4;
         value+=attn4*attn4*extrapolate3(xsb+1, ysb+1, zsb+0, dx4, dy4, dz4);
      }

      // Contribution (1,0,1)
      dx5=dx4;
      dy5=dy0-0-2*SQUISH_CONSTANT_3D;
      dz5=dz0-1-2*SQUISH_CONSTANT_3D;
      attn5=2-dx5*dx5-dy5*dy5-dz5*dz5;
      if(attn5>0)
      {
         attn5*=attn5;
         value+=attn5*attn5*extrapolate3(xsb+1, ysb+0, zsb+1, dx5, dy5, dz5);
      }

      // Contribution (0,1,1)
      dx6=dx0-0-2*SQUISH_CONSTANT_3D;
      dy6=dy4;
      dz6=dz5;
      attn6=2-dx6*dx6-dy6*dy6-dz6*dz6;
      if(attn6>0)
      {
         attn6*=attn6;
         value+=attn6*attn6*extrapolate3(xsb+0, ysb+1, zsb+1, dx6, dy6, dz6);
      }
   }

   // First extra vertex
   attn_ext0=2-dx_ext0*dx_ext0-dy_ext0*dy_ext0-dz_ext0*dz_ext0;
   if(attn_ext0>0)
   {
      attn_ext0*=attn_ext0;
      value+=attn_ext0*attn_ext0*extrapolate3(xsv_ext0, ysv_ext0, zsv_ext0, dx_ext0, dy_ext0, dz_ext0);
   }

   // Second extra vertex
   attn_ext1=2-dx_ext1*dx_ext1-dy_ext1*dy_ext1-dz_ext1*dz_ext1;
   if(attn_ext1>0)
   {
      attn_ext1*=attn_ext1;
      value+=attn_ext1*attn_ext1*extrapolate3(xsv_ext1, ysv_ext1, zsv_ext1, dx_ext1, dy_ext1, dz_ext1);
   }
   
   return value/(SIMPLEX_NORM_CONSTANT_3D);
}
/******************************************************************************/
Flt SimplexNoise::noise(Dbl x, Dbl y, Dbl z, Dbl w)C
{
   Dbl uins;
   Dbl dx1, dy1, dz1, dw1;
   Dbl dx2, dy2, dz2, dw2;
   Dbl dx3, dy3, dz3, dw3;
   Dbl dx4, dy4, dz4, dw4;
   Dbl dx5, dy5, dz5, dw5;
   Dbl dx6, dy6, dz6, dw6;
   Dbl dx7, dy7, dz7, dw7;
   Dbl dx8, dy8, dz8, dw8;
   Dbl dx9, dy9, dz9, dw9;
   Dbl dx10, dy10, dz10, dw10;
   Dbl attn0, attn1, attn2, attn3, attn4;
   Dbl attn5, attn6, attn7, attn8, attn9, attn10;
   Dbl attn_ext0, attn_ext1, attn_ext2;
   I8 c, c1, c2;
   I8 aPoint, bPoint;
   Dbl aScore, bScore;
   Int aIsBiggerSide;
   Int bIsBiggerSide;
   Dbl p1, p2, p3, p4;
   Dbl score;

   // Place input coordinates on simplectic honeycomb.
   Dbl stretchOffset=(x+y+z+w)*STRETCH_CONSTANT_4D;
   Dbl xs=x+stretchOffset;
   Dbl ys=y+stretchOffset;
   Dbl zs=z+stretchOffset;
   Dbl ws=w+stretchOffset;
   
   // Floor to get simplectic honeycomb coordinates of rhombo-hypercube super-cell origin.
   Int xsb=Floor(xs);
   Int ysb=Floor(ys);
   Int zsb=Floor(zs);
   Int wsb=Floor(ws);
   
   // Skew out to get actual coordinates of stretched rhombo-hypercube origin. We'll need these later.
   Dbl squishOffset=(xsb+ysb+zsb+wsb)*SQUISH_CONSTANT_4D;
   Dbl xb=xsb+squishOffset;
   Dbl yb=ysb+squishOffset;
   Dbl zb=zsb+squishOffset;
   Dbl wb=wsb+squishOffset;
   
   // Compute simplectic honeycomb coordinates relative to rhombo-hypercube origin.
   Dbl xins=xs-xsb;
   Dbl yins=ys-ysb;
   Dbl zins=zs-zsb;
   Dbl wins=ws-wsb;
   
   // Sum those together to get a value that determines which region we're in.
   Dbl inSum=xins+yins+zins+wins;

   // Positions relative to origin point.
   Dbl dx0=x-xb;
   Dbl dy0=y-yb;
   Dbl dz0=z-zb;
   Dbl dw0=w-wb;
   
   // We'll be defining these inside the next block and using them afterwards.
   Dbl dx_ext0, dy_ext0, dz_ext0, dw_ext0;
   Dbl dx_ext1, dy_ext1, dz_ext1, dw_ext1;
   Dbl dx_ext2, dy_ext2, dz_ext2, dw_ext2;
   Int xsv_ext0, ysv_ext0, zsv_ext0, wsv_ext0;
   Int xsv_ext1, ysv_ext1, zsv_ext1, wsv_ext1;
   Int xsv_ext2, ysv_ext2, zsv_ext2, wsv_ext2;
   
   Dbl value=0;
   if(inSum<=1) { // We're inside the pentachoron (4-Simplex) at (0,0,0,0)

      // Determine which two of (0,0,0,1), (0,0,1,0), (0,1,0,0), (1,0,0,0) are closest.
      aPoint=0x01;
      aScore=xins;
      bPoint=0x02;
      bScore=yins;
      if(aScore>=bScore && zins>bScore) {
         bScore=zins;
         bPoint=0x04;
      } else if(aScore<bScore && zins>aScore) {
         aScore=zins;
         aPoint=0x04;
      }
      if(aScore>=bScore && wins>bScore) {
         bScore=wins;
         bPoint=0x08;
      } else if(aScore<bScore && wins>aScore) {
         aScore=wins;
         aPoint=0x08;
      }
      
      // Now we determine the three lattice points not part of the pentachoron that may contribute. This depends on the closest two pentachoron vertices, including (0,0,0,0)
      uins=1-inSum;
      if(uins>aScore || uins>bScore) { // (0,0,0,0) is one of the closest two pentachoron vertices.
         c=(bScore>aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
         if((c&0x01) == 0) {
            xsv_ext0=xsb-1;
            xsv_ext1=xsv_ext2=xsb;
            dx_ext0=dx0+1;
            dx_ext1=dx_ext2=dx0;
         } else {
            xsv_ext0=xsv_ext1=xsv_ext2=xsb+1;
            dx_ext0=dx_ext1=dx_ext2=dx0-1;
         }

         if((c&0x02) == 0) {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb;
            dy_ext0=dy_ext1=dy_ext2=dy0;
            if((c&0x01) == 0x01) {
               ysv_ext0-=1;
               dy_ext0+=1;
            } else {
               ysv_ext1-=1;
               dy_ext1+=1;
            }
         } else {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb+1;
            dy_ext0=dy_ext1=dy_ext2=dy0-1;
         }
         
         if((c&0x04) == 0) {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb;
            dz_ext0=dz_ext1=dz_ext2=dz0;
            if((c&0x03) != 0) {
               if((c&0x03) == 0x03) {
                  zsv_ext0-=1;
                  dz_ext0+=1;
               } else {
                  zsv_ext1-=1;
                  dz_ext1+=1;
               }
            } else {
               zsv_ext2-=1;
               dz_ext2+=1;
            }
         } else {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb+1;
            dz_ext0=dz_ext1=dz_ext2=dz0-1;
         }
         
         if((c&0x08) == 0) {
            wsv_ext0=wsv_ext1=wsb;
            wsv_ext2=wsb-1;
            dw_ext0=dw_ext1=dw0;
            dw_ext2=dw0+1;
         } else {
            wsv_ext0=wsv_ext1=wsv_ext2=wsb+1;
            dw_ext0=dw_ext1=dw_ext2=dw0-1;
         }
      } else { // (0,0,0,0) is not one of the closest two pentachoron vertices.
         c=(I8)(aPoint | bPoint); // Our three extra vertices are determined by the closest two.
         
         if((c&0x01) == 0) {
            xsv_ext0=xsv_ext2=xsb;
            xsv_ext1=xsb-1;
            dx_ext0=dx0-2*SQUISH_CONSTANT_4D;
            dx_ext1=dx0+1-SQUISH_CONSTANT_4D;
            dx_ext2=dx0-SQUISH_CONSTANT_4D;
         } else {
            xsv_ext0=xsv_ext1=xsv_ext2=xsb+1;
            dx_ext0=dx0-1-2*SQUISH_CONSTANT_4D;
            dx_ext1=dx_ext2=dx0-1-SQUISH_CONSTANT_4D;
         }
         
         if((c&0x02) == 0) {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb;
            dy_ext0=dy0-2*SQUISH_CONSTANT_4D;
            dy_ext1=dy_ext2=dy0-SQUISH_CONSTANT_4D;
            if((c&0x01) == 0x01) {
               ysv_ext1-=1;
               dy_ext1+=1;
            } else {
               ysv_ext2-=1;
               dy_ext2+=1;
            }
         } else {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb+1;
            dy_ext0=dy0-1-2*SQUISH_CONSTANT_4D;
            dy_ext1=dy_ext2=dy0-1-SQUISH_CONSTANT_4D;
         }
         
         if((c&0x04) == 0) {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb;
            dz_ext0=dz0-2*SQUISH_CONSTANT_4D;
            dz_ext1=dz_ext2=dz0-SQUISH_CONSTANT_4D;
            if((c&0x03) == 0x03) {
               zsv_ext1-=1;
               dz_ext1+=1;
            } else {
               zsv_ext2-=1;
               dz_ext2+=1;
            }
         } else {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb+1;
            dz_ext0=dz0-1-2*SQUISH_CONSTANT_4D;
            dz_ext1=dz_ext2=dz0-1-SQUISH_CONSTANT_4D;
         }
         
         if((c&0x08) == 0) {
            wsv_ext0=wsv_ext1=wsb;
            wsv_ext2=wsb-1;
            dw_ext0=dw0-2*SQUISH_CONSTANT_4D;
            dw_ext1=dw0-SQUISH_CONSTANT_4D;
            dw_ext2=dw0+1-SQUISH_CONSTANT_4D;
         } else {
            wsv_ext0=wsv_ext1=wsv_ext2=wsb+1;
            dw_ext0=dw0-1-2*SQUISH_CONSTANT_4D;
            dw_ext1=dw_ext2=dw0-1-SQUISH_CONSTANT_4D;
         }
      }

      // Contribution (0,0,0,0)
      attn0=2-dx0*dx0-dy0*dy0-dz0*dz0-dw0*dw0;
      if(attn0>0) {
         attn0*=attn0;
         value+=attn0*attn0*extrapolate4(xsb+0, ysb+0, zsb+0, wsb+0, dx0, dy0, dz0, dw0);
      }

      // Contribution (1,0,0,0)
      dx1=dx0-1-SQUISH_CONSTANT_4D;
      dy1=dy0-0-SQUISH_CONSTANT_4D;
      dz1=dz0-0-SQUISH_CONSTANT_4D;
      dw1=dw0-0-SQUISH_CONSTANT_4D;
      attn1=2-dx1*dx1-dy1*dy1-dz1*dz1-dw1*dw1;
      if(attn1>0) {
         attn1*=attn1;
         value+=attn1*attn1*extrapolate4(xsb+1, ysb+0, zsb+0, wsb+0, dx1, dy1, dz1, dw1);
      }

      // Contribution (0,1,0,0)
      dx2=dx0-0-SQUISH_CONSTANT_4D;
      dy2=dy0-1-SQUISH_CONSTANT_4D;
      dz2=dz1;
      dw2=dw1;
      attn2=2-dx2*dx2-dy2*dy2-dz2*dz2-dw2*dw2;
      if(attn2>0) {
         attn2*=attn2;
         value+=attn2*attn2*extrapolate4(xsb+0, ysb+1, zsb+0, wsb+0, dx2, dy2, dz2, dw2);
      }

      // Contribution (0,0,1,0)
      dx3=dx2;
      dy3=dy1;
      dz3=dz0-1-SQUISH_CONSTANT_4D;
      dw3=dw1;
      attn3=2-dx3*dx3-dy3*dy3-dz3*dz3-dw3*dw3;
      if(attn3>0) {
         attn3*=attn3;
         value+=attn3*attn3*extrapolate4(xsb+0, ysb+0, zsb+1, wsb+0, dx3, dy3, dz3, dw3);
      }

      // Contribution (0,0,0,1)
      dx4=dx2;
      dy4=dy1;
      dz4=dz1;
      dw4=dw0-1-SQUISH_CONSTANT_4D;
      attn4=2-dx4*dx4-dy4*dy4-dz4*dz4-dw4*dw4;
      if(attn4>0) {
         attn4*=attn4;
         value+=attn4*attn4*extrapolate4(xsb+0, ysb+0, zsb+0, wsb+1, dx4, dy4, dz4, dw4);
      }
   } else if(inSum>=3) { // We're inside the pentachoron (4-Simplex) at (1,1,1,1) Determine which two of (1,1,1,0), (1,1,0,1), (1,0,1,1), (0,1,1,1) are closest.
      aPoint=0x0E;
      aScore=xins;
      bPoint=0x0D;
      bScore=yins;
      if(aScore<=bScore && zins<bScore) {
         bScore=zins;
         bPoint=0x0B;
      } else if(aScore>bScore && zins<aScore) {
         aScore=zins;
         aPoint=0x0B;
      }
      if(aScore<=bScore && wins<bScore) {
         bScore=wins;
         bPoint=0x07;
      } else if(aScore>bScore && wins<aScore) {
         aScore=wins;
         aPoint=0x07;
      }
      
      // Now we determine the three lattice points not part of the pentachoron that may contribute. This depends on the closest two pentachoron vertices, including (0,0,0,0)
      uins=4-inSum;
      if(uins<aScore || uins<bScore) { // (1,1,1,1) is one of the closest two pentachoron vertices.
         c=(bScore<aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
         
         if((c&0x01) != 0) {
            xsv_ext0=xsb+2;
            xsv_ext1=xsv_ext2=xsb+1;
            dx_ext0=dx0-2-4*SQUISH_CONSTANT_4D;
            dx_ext1=dx_ext2=dx0-1-4*SQUISH_CONSTANT_4D;
         } else {
            xsv_ext0=xsv_ext1=xsv_ext2=xsb;
            dx_ext0=dx_ext1=dx_ext2=dx0-4*SQUISH_CONSTANT_4D;
         }

         if((c&0x02) != 0) {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb+1;
            dy_ext0=dy_ext1=dy_ext2=dy0-1-4*SQUISH_CONSTANT_4D;
            if((c&0x01) != 0) {
               ysv_ext1+=1;
               dy_ext1-=1;
            } else {
               ysv_ext0+=1;
               dy_ext0-=1;
            }
         } else {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb;
            dy_ext0=dy_ext1=dy_ext2=dy0-4*SQUISH_CONSTANT_4D;
         }
         
         if((c&0x04) != 0) {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb+1;
            dz_ext0=dz_ext1=dz_ext2=dz0-1-4*SQUISH_CONSTANT_4D;
            if((c&0x03) != 0x03) {
               if((c&0x03) == 0) {
                  zsv_ext0+=1;
                  dz_ext0-=1;
               } else {
                  zsv_ext1+=1;
                  dz_ext1-=1;
               }
            } else {
               zsv_ext2+=1;
               dz_ext2-=1;
            }
         } else {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb;
            dz_ext0=dz_ext1=dz_ext2=dz0-4*SQUISH_CONSTANT_4D;
         }
         
         if((c&0x08) != 0) {
            wsv_ext0=wsv_ext1=wsb+1;
            wsv_ext2=wsb+2;
            dw_ext0=dw_ext1=dw0-1-4*SQUISH_CONSTANT_4D;
            dw_ext2=dw0-2-4*SQUISH_CONSTANT_4D;
         } else {
            wsv_ext0=wsv_ext1=wsv_ext2=wsb;
            dw_ext0=dw_ext1=dw_ext2=dw0-4*SQUISH_CONSTANT_4D;
         }
      } else { // (1,1,1,1) is not one of the closest two pentachoron vertices.
         c=(I8)(aPoint&bPoint); // Our three extra vertices are determined by the closest two.
         
         if((c&0x01) != 0) {
            xsv_ext0=xsv_ext2=xsb+1;
            xsv_ext1=xsb+2;
            dx_ext0=dx0-1-2*SQUISH_CONSTANT_4D;
            dx_ext1=dx0-2-3*SQUISH_CONSTANT_4D;
            dx_ext2=dx0-1-3*SQUISH_CONSTANT_4D;
         } else {
            xsv_ext0=xsv_ext1=xsv_ext2=xsb;
            dx_ext0=dx0-2*SQUISH_CONSTANT_4D;
            dx_ext1=dx_ext2=dx0-3*SQUISH_CONSTANT_4D;
         }
         
         if((c&0x02) != 0) {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb+1;
            dy_ext0=dy0-1-2*SQUISH_CONSTANT_4D;
            dy_ext1=dy_ext2=dy0-1-3*SQUISH_CONSTANT_4D;
            if((c&0x01) != 0) {
               ysv_ext2+=1;
               dy_ext2-=1;
            } else {
               ysv_ext1+=1;
               dy_ext1-=1;
            }
         } else {
            ysv_ext0=ysv_ext1=ysv_ext2=ysb;
            dy_ext0=dy0-2*SQUISH_CONSTANT_4D;
            dy_ext1=dy_ext2=dy0-3*SQUISH_CONSTANT_4D;
         }
         
         if((c&0x04) != 0) {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb+1;
            dz_ext0=dz0-1-2*SQUISH_CONSTANT_4D;
            dz_ext1=dz_ext2=dz0-1-3*SQUISH_CONSTANT_4D;
            if((c&0x03) != 0) {
               zsv_ext2+=1;
               dz_ext2-=1;
            } else {
               zsv_ext1+=1;
               dz_ext1-=1;
            }
         } else {
            zsv_ext0=zsv_ext1=zsv_ext2=zsb;
            dz_ext0=dz0-2*SQUISH_CONSTANT_4D;
            dz_ext1=dz_ext2=dz0-3*SQUISH_CONSTANT_4D;
         }
         
         if((c&0x08) != 0) {
            wsv_ext0=wsv_ext1=wsb+1;
            wsv_ext2=wsb+2;
            dw_ext0=dw0-1-2*SQUISH_CONSTANT_4D;
            dw_ext1=dw0-1-3*SQUISH_CONSTANT_4D;
            dw_ext2=dw0-2-3*SQUISH_CONSTANT_4D;
         } else {
            wsv_ext0=wsv_ext1=wsv_ext2=wsb;
            dw_ext0=dw0-2*SQUISH_CONSTANT_4D;
            dw_ext1=dw_ext2=dw0-3*SQUISH_CONSTANT_4D;
         }
      }

      // Contribution (1,1,1,0)
      dx4=dx0-1-3*SQUISH_CONSTANT_4D;
      dy4=dy0-1-3*SQUISH_CONSTANT_4D;
      dz4=dz0-1-3*SQUISH_CONSTANT_4D;
      dw4=dw0-3*SQUISH_CONSTANT_4D;
      attn4=2-dx4*dx4-dy4*dy4-dz4*dz4-dw4*dw4;
      if(attn4>0) {
         attn4*=attn4;
         value+=attn4*attn4*extrapolate4(xsb+1, ysb+1, zsb+1, wsb+0, dx4, dy4, dz4, dw4);
      }

      // Contribution (1,1,0,1)
      dx3=dx4;
      dy3=dy4;
      dz3=dz0-3*SQUISH_CONSTANT_4D;
      dw3=dw0-1-3*SQUISH_CONSTANT_4D;
      attn3=2-dx3*dx3-dy3*dy3-dz3*dz3-dw3*dw3;
      if(attn3>0) {
         attn3*=attn3;
         value+=attn3*attn3*extrapolate4(xsb+1, ysb+1, zsb+0, wsb+1, dx3, dy3, dz3, dw3);
      }

      // Contribution (1,0,1,1)
      dx2=dx4;
      dy2=dy0-3*SQUISH_CONSTANT_4D;
      dz2=dz4;
      dw2=dw3;
      attn2=2-dx2*dx2-dy2*dy2-dz2*dz2-dw2*dw2;
      if(attn2>0) {
         attn2*=attn2;
         value+=attn2*attn2*extrapolate4(xsb+1, ysb+0, zsb+1, wsb+1, dx2, dy2, dz2, dw2);
      }

      // Contribution (0,1,1,1)
      dx1=dx0-3*SQUISH_CONSTANT_4D;
      dz1=dz4;
      dy1=dy4;
      dw1=dw3;
      attn1=2-dx1*dx1-dy1*dy1-dz1*dz1-dw1*dw1;
      if(attn1>0) {
         attn1*=attn1;
         value+=attn1*attn1*extrapolate4(xsb+0, ysb+1, zsb+1, wsb+1, dx1, dy1, dz1, dw1);
      }

      // Contribution (1,1,1,1)
      dx0=dx0-1-4*SQUISH_CONSTANT_4D;
      dy0=dy0-1-4*SQUISH_CONSTANT_4D;
      dz0=dz0-1-4*SQUISH_CONSTANT_4D;
      dw0=dw0-1-4*SQUISH_CONSTANT_4D;
      attn0=2-dx0*dx0-dy0*dy0-dz0*dz0-dw0*dw0;
      if(attn0>0) {
         attn0*=attn0;
         value+=attn0*attn0*extrapolate4(xsb+1, ysb+1, zsb+1, wsb+1, dx0, dy0, dz0, dw0);
      }
   } else if(inSum<=2) { // We're inside the first dispentachoron (Rectified 4-Simplex)
      aIsBiggerSide=1;
      bIsBiggerSide=1;
      
      // Decide between (1,1,0,0) and (0,0,1,1)
      if(xins+yins>zins+wins) {
         aScore=xins+yins;
         aPoint=0x03;
      } else {
         aScore=zins+wins;
         aPoint=0x0C;
      }
      
      // Decide between (1,0,1,0) and (0,1,0,1)
      if(xins+zins>yins+wins) {
         bScore=xins+zins;
         bPoint=0x05;
      } else {
         bScore=yins+wins;
         bPoint=0x0A;
      }
      
      // Closer between (1,0,0,1) and (0,1,1,0) will replace the further of a and b, if closer.
      if(xins+wins>yins+zins) {
         score=xins+wins;
         if(aScore>=bScore && score>bScore) {
            bScore=score;
            bPoint=0x09;
         } else if(aScore<bScore && score>aScore) {
            aScore=score;
            aPoint=0x09;
         }
      } else {
         score=yins+zins;
         if(aScore>=bScore && score>bScore) {
            bScore=score;
            bPoint=0x06;
         } else if(aScore<bScore && score>aScore) {
            aScore=score;
            aPoint=0x06;
         }
      }
      
      // Decide if(1,0,0,0) is closer.
      p1=2-inSum+xins;
      if(aScore>=bScore && p1>bScore) {
         bScore=p1;
         bPoint=0x01;
         bIsBiggerSide=0;
      } else if(aScore<bScore && p1>aScore) {
         aScore=p1;
         aPoint=0x01;
         aIsBiggerSide=0;
      }
      
      // Decide if(0,1,0,0) is closer.
      p2=2-inSum+yins;
      if(aScore>=bScore && p2>bScore) {
         bScore=p2;
         bPoint=0x02;
         bIsBiggerSide=0;
      } else if(aScore<bScore && p2>aScore) {
         aScore=p2;
         aPoint=0x02;
         aIsBiggerSide=0;
      }
      
      // Decide if(0,0,1,0) is closer.
      p3=2-inSum+zins;
      if(aScore>=bScore && p3>bScore) {
         bScore=p3;
         bPoint=0x04;
         bIsBiggerSide=0;
      } else if(aScore<bScore && p3>aScore) {
         aScore=p3;
         aPoint=0x04;
         aIsBiggerSide=0;
      }
      
      // Decide if(0,0,0,1) is closer.
      p4=2-inSum+wins;
      if(aScore>=bScore && p4>bScore) {
         bScore=p4;
         bPoint=0x08;
         bIsBiggerSide=0;
      } else if(aScore<bScore && p4>aScore) {
         aScore=p4;
         aPoint=0x08;
         aIsBiggerSide=0;
      }
      
      // Where each of the two closest points are determines how the extra three vertices are calculated.
      if(aIsBiggerSide == bIsBiggerSide) {
         if(aIsBiggerSide) { // Both closest points on the bigger side
            c1=(I8)(aPoint | bPoint);
            c2=(I8)(aPoint&bPoint);
            if((c1&0x01) == 0) {
               xsv_ext0=xsb;
               xsv_ext1=xsb-1;
               dx_ext0=dx0-3*SQUISH_CONSTANT_4D;
               dx_ext1=dx0+1-2*SQUISH_CONSTANT_4D;
            } else {
               xsv_ext0=xsv_ext1=xsb+1;
               dx_ext0=dx0-1-3*SQUISH_CONSTANT_4D;
               dx_ext1=dx0-1-2*SQUISH_CONSTANT_4D;
            }
            
            if((c1&0x02) == 0) {
               ysv_ext0=ysb;
               ysv_ext1=ysb-1;
               dy_ext0=dy0-3*SQUISH_CONSTANT_4D;
               dy_ext1=dy0+1-2*SQUISH_CONSTANT_4D;
            } else {
               ysv_ext0=ysv_ext1=ysb+1;
               dy_ext0=dy0-1-3*SQUISH_CONSTANT_4D;
               dy_ext1=dy0-1-2*SQUISH_CONSTANT_4D;
            }
            
            if((c1&0x04) == 0) {
               zsv_ext0=zsb;
               zsv_ext1=zsb-1;
               dz_ext0=dz0-3*SQUISH_CONSTANT_4D;
               dz_ext1=dz0+1-2*SQUISH_CONSTANT_4D;
            } else {
               zsv_ext0=zsv_ext1=zsb+1;
               dz_ext0=dz0-1-3*SQUISH_CONSTANT_4D;
               dz_ext1=dz0-1-2*SQUISH_CONSTANT_4D;
            }
            
            if((c1&0x08) == 0) {
               wsv_ext0=wsb;
               wsv_ext1=wsb-1;
               dw_ext0=dw0-3*SQUISH_CONSTANT_4D;
               dw_ext1=dw0+1-2*SQUISH_CONSTANT_4D;
            } else {
               wsv_ext0=wsv_ext1=wsb+1;
               dw_ext0=dw0-1-3*SQUISH_CONSTANT_4D;
               dw_ext1=dw0-1-2*SQUISH_CONSTANT_4D;
            }
            
            // One combination is a permutation of (0,0,0,2) based on c2
            xsv_ext2=xsb;
            ysv_ext2=ysb;
            zsv_ext2=zsb;
            wsv_ext2=wsb;
            dx_ext2=dx0-2*SQUISH_CONSTANT_4D;
            dy_ext2=dy0-2*SQUISH_CONSTANT_4D;
            dz_ext2=dz0-2*SQUISH_CONSTANT_4D;
            dw_ext2=dw0-2*SQUISH_CONSTANT_4D;
            if((c2&0x01) != 0) {
               xsv_ext2+=2;
               dx_ext2-=2;
            } else if((c2&0x02) != 0) {
               ysv_ext2+=2;
               dy_ext2-=2;
            } else if((c2&0x04) != 0) {
               zsv_ext2+=2;
               dz_ext2-=2;
            } else {
               wsv_ext2+=2;
               dw_ext2-=2;
            }
            
         } else { // Both closest points on the smaller side
            // One of the two extra points is (0,0,0,0)
            xsv_ext2=xsb;
            ysv_ext2=ysb;
            zsv_ext2=zsb;
            wsv_ext2=wsb;
            dx_ext2=dx0;
            dy_ext2=dy0;
            dz_ext2=dz0;
            dw_ext2=dw0;
            
            // Other two points are based on the omitted axes.
            c=(I8)(aPoint | bPoint);
            
            if((c&0x01) == 0) {
               xsv_ext0=xsb-1;
               xsv_ext1=xsb;
               dx_ext0=dx0+1-SQUISH_CONSTANT_4D;
               dx_ext1=dx0-SQUISH_CONSTANT_4D;
            } else {
               xsv_ext0=xsv_ext1=xsb+1;
               dx_ext0=dx_ext1=dx0-1-SQUISH_CONSTANT_4D;
            }
            
            if((c&0x02) == 0) {
               ysv_ext0=ysv_ext1=ysb;
               dy_ext0=dy_ext1=dy0-SQUISH_CONSTANT_4D;
               if((c&0x01) == 0x01)
               {
                  ysv_ext0-=1;
                  dy_ext0+=1;
               } else {
                  ysv_ext1-=1;
                  dy_ext1+=1;
               }
            } else {
               ysv_ext0=ysv_ext1=ysb+1;
               dy_ext0=dy_ext1=dy0-1-SQUISH_CONSTANT_4D;
            }
            
            if((c&0x04) == 0) {
               zsv_ext0=zsv_ext1=zsb;
               dz_ext0=dz_ext1=dz0-SQUISH_CONSTANT_4D;
               if((c&0x03) == 0x03)
               {
                  zsv_ext0-=1;
                  dz_ext0+=1;
               } else {
                  zsv_ext1-=1;
                  dz_ext1+=1;
               }
            } else {
               zsv_ext0=zsv_ext1=zsb+1;
               dz_ext0=dz_ext1=dz0-1-SQUISH_CONSTANT_4D;
            }
            
            if((c&0x08) == 0)
            {
               wsv_ext0=wsb;
               wsv_ext1=wsb-1;
               dw_ext0=dw0-SQUISH_CONSTANT_4D;
               dw_ext1=dw0+1-SQUISH_CONSTANT_4D;
            } else {
               wsv_ext0=wsv_ext1=wsb+1;
               dw_ext0=dw_ext1=dw0-1-SQUISH_CONSTANT_4D;
            }
            
         }
      } else { // One point on each "side"
         if(aIsBiggerSide) {
            c1=aPoint;
            c2=bPoint;
         } else {
            c1=bPoint;
            c2=aPoint;
         }
         
         // Two contributions are the bigger-sided point with each 0 replaced with -1.
         if((c1&0x01) == 0) {
            xsv_ext0=xsb-1;
            xsv_ext1=xsb;
            dx_ext0=dx0+1-SQUISH_CONSTANT_4D;
            dx_ext1=dx0-SQUISH_CONSTANT_4D;
         } else {
            xsv_ext0=xsv_ext1=xsb+1;
            dx_ext0=dx_ext1=dx0-1-SQUISH_CONSTANT_4D;
         }
         
         if((c1&0x02) == 0) {
            ysv_ext0=ysv_ext1=ysb;
            dy_ext0=dy_ext1=dy0-SQUISH_CONSTANT_4D;
            if((c1&0x01) == 0x01) {
               ysv_ext0-=1;
               dy_ext0+=1;
            } else {
               ysv_ext1-=1;
               dy_ext1+=1;
            }
         } else {
            ysv_ext0=ysv_ext1=ysb+1;
            dy_ext0=dy_ext1=dy0-1-SQUISH_CONSTANT_4D;
         }
         
         if((c1&0x04) == 0) {
            zsv_ext0=zsv_ext1=zsb;
            dz_ext0=dz_ext1=dz0-SQUISH_CONSTANT_4D;
            if((c1&0x03) == 0x03) {
               zsv_ext0-=1;
               dz_ext0+=1;
            } else {
               zsv_ext1-=1;
               dz_ext1+=1;
            }
         } else {
            zsv_ext0=zsv_ext1=zsb+1;
            dz_ext0=dz_ext1=dz0-1-SQUISH_CONSTANT_4D;
         }
         
         if((c1&0x08) == 0) {
            wsv_ext0=wsb;
            wsv_ext1=wsb-1;
            dw_ext0=dw0-SQUISH_CONSTANT_4D;
            dw_ext1=dw0+1-SQUISH_CONSTANT_4D;
         } else {
            wsv_ext0=wsv_ext1=wsb+1;
            dw_ext0=dw_ext1=dw0-1-SQUISH_CONSTANT_4D;
         }

         // One contribution is a permutation of (0,0,0,2) based on the smaller-sided point
         xsv_ext2=xsb;
         ysv_ext2=ysb;
         zsv_ext2=zsb;
         wsv_ext2=wsb;
         dx_ext2=dx0-2*SQUISH_CONSTANT_4D;
         dy_ext2=dy0-2*SQUISH_CONSTANT_4D;
         dz_ext2=dz0-2*SQUISH_CONSTANT_4D;
         dw_ext2=dw0-2*SQUISH_CONSTANT_4D;
         if((c2&0x01) != 0) {
            xsv_ext2+=2;
            dx_ext2-=2;
         } else if((c2&0x02) != 0) {
            ysv_ext2+=2;
            dy_ext2-=2;
         } else if((c2&0x04) != 0) {
            zsv_ext2+=2;
            dz_ext2-=2;
         } else {
            wsv_ext2+=2;
            dw_ext2-=2;
         }
      }
      
      // Contribution (1,0,0,0)
      dx1=dx0-1-SQUISH_CONSTANT_4D;
      dy1=dy0-0-SQUISH_CONSTANT_4D;
      dz1=dz0-0-SQUISH_CONSTANT_4D;
      dw1=dw0-0-SQUISH_CONSTANT_4D;
      attn1=2-dx1*dx1-dy1*dy1-dz1*dz1-dw1*dw1;
      if(attn1>0) {
         attn1*=attn1;
         value+=attn1*attn1*extrapolate4(xsb+1, ysb+0, zsb+0, wsb+0, dx1, dy1, dz1, dw1);
      }

      // Contribution (0,1,0,0)
      dx2=dx0-0-SQUISH_CONSTANT_4D;
      dy2=dy0-1-SQUISH_CONSTANT_4D;
      dz2=dz1;
      dw2=dw1;
      attn2=2-dx2*dx2-dy2*dy2-dz2*dz2-dw2*dw2;
      if(attn2>0) {
         attn2*=attn2;
         value+=attn2*attn2*extrapolate4(xsb+0, ysb+1, zsb+0, wsb+0, dx2, dy2, dz2, dw2);
      }

      // Contribution (0,0,1,0)
      dx3=dx2;
      dy3=dy1;
      dz3=dz0-1-SQUISH_CONSTANT_4D;
      dw3=dw1;
      attn3=2-dx3*dx3-dy3*dy3-dz3*dz3-dw3*dw3;
      if(attn3>0) {
         attn3*=attn3;
         value+=attn3*attn3*extrapolate4(xsb+0, ysb+0, zsb+1, wsb+0, dx3, dy3, dz3, dw3);
      }

      // Contribution (0,0,0,1)
      dx4=dx2;
      dy4=dy1;
      dz4=dz1;
      dw4=dw0-1-SQUISH_CONSTANT_4D;
      attn4=2-dx4*dx4-dy4*dy4-dz4*dz4-dw4*dw4;
      if(attn4>0) {
         attn4*=attn4;
         value+=attn4*attn4*extrapolate4(xsb+0, ysb+0, zsb+0, wsb+1, dx4, dy4, dz4, dw4);
      }
      
      // Contribution (1,1,0,0)
      dx5=dx0-1-2*SQUISH_CONSTANT_4D;
      dy5=dy0-1-2*SQUISH_CONSTANT_4D;
      dz5=dz0-0-2*SQUISH_CONSTANT_4D;
      dw5=dw0-0-2*SQUISH_CONSTANT_4D;
      attn5=2-dx5*dx5-dy5*dy5-dz5*dz5-dw5*dw5;
      if(attn5>0) {
         attn5*=attn5;
         value+=attn5*attn5*extrapolate4(xsb+1, ysb+1, zsb+0, wsb+0, dx5, dy5, dz5, dw5);
      }
      
      // Contribution (1,0,1,0)
      dx6=dx0-1-2*SQUISH_CONSTANT_4D;
      dy6=dy0-0-2*SQUISH_CONSTANT_4D;
      dz6=dz0-1-2*SQUISH_CONSTANT_4D;
      dw6=dw0-0-2*SQUISH_CONSTANT_4D;
      attn6=2-dx6*dx6-dy6*dy6-dz6*dz6-dw6*dw6;
      if(attn6>0) {
         attn6*=attn6;
         value+=attn6*attn6*extrapolate4(xsb+1, ysb+0, zsb+1, wsb+0, dx6, dy6, dz6, dw6);
      }

      // Contribution (1,0,0,1)
      dx7=dx0-1-2*SQUISH_CONSTANT_4D;
      dy7=dy0-0-2*SQUISH_CONSTANT_4D;
      dz7=dz0-0-2*SQUISH_CONSTANT_4D;
      dw7=dw0-1-2*SQUISH_CONSTANT_4D;
      attn7=2-dx7*dx7-dy7*dy7-dz7*dz7-dw7*dw7;
      if(attn7>0) {
         attn7*=attn7;
         value+=attn7*attn7*extrapolate4(xsb+1, ysb+0, zsb+0, wsb+1, dx7, dy7, dz7, dw7);
      }
      
      // Contribution (0,1,1,0)
      dx8=dx0-0-2*SQUISH_CONSTANT_4D;
      dy8=dy0-1-2*SQUISH_CONSTANT_4D;
      dz8=dz0-1-2*SQUISH_CONSTANT_4D;
      dw8=dw0-0-2*SQUISH_CONSTANT_4D;
      attn8=2-dx8*dx8-dy8*dy8-dz8*dz8-dw8*dw8;
      if(attn8>0) {
         attn8*=attn8;
         value+=attn8*attn8*extrapolate4(xsb+0, ysb+1, zsb+1, wsb+0, dx8, dy8, dz8, dw8);
      }
      
      // Contribution (0,1,0,1)
      dx9=dx0-0-2*SQUISH_CONSTANT_4D;
      dy9=dy0-1-2*SQUISH_CONSTANT_4D;
      dz9=dz0-0-2*SQUISH_CONSTANT_4D;
      dw9=dw0-1-2*SQUISH_CONSTANT_4D;
      attn9=2-dx9*dx9-dy9*dy9-dz9*dz9-dw9*dw9;
      if(attn9>0) {
         attn9*=attn9;
         value+=attn9*attn9*extrapolate4(xsb+0, ysb+1, zsb+0, wsb+1, dx9, dy9, dz9, dw9);
      }
      
      // Contribution (0,0,1,1)
      dx10=dx0-0-2*SQUISH_CONSTANT_4D;
      dy10=dy0-0-2*SQUISH_CONSTANT_4D;
      dz10=dz0-1-2*SQUISH_CONSTANT_4D;
      dw10=dw0-1-2*SQUISH_CONSTANT_4D;
      attn10=2-dx10*dx10-dy10*dy10-dz10*dz10-dw10*dw10;
      if(attn10>0) {
         attn10*=attn10;
         value+=attn10*attn10*extrapolate4(xsb+0, ysb+0, zsb+1, wsb+1, dx10, dy10, dz10, dw10);
      }
   } else { // We're inside the second dispentachoron (Rectified 4-Simplex)
      aIsBiggerSide=1;
      bIsBiggerSide=1;
      
      // Decide between (0,0,1,1) and (1,1,0,0)
      if(xins+yins<zins+wins) {
         aScore=xins+yins;
         aPoint=0x0C;
      } else {
         aScore=zins+wins;
         aPoint=0x03;
      }
      
      // Decide between (0,1,0,1) and (1,0,1,0)
      if(xins+zins<yins+wins) {
         bScore=xins+zins;
         bPoint=0x0A;
      } else {
         bScore=yins+wins;
         bPoint=0x05;
      }
      
      // Closer between (0,1,1,0) and (1,0,0,1) will replace the further of a and b, if closer.
      if(xins+wins<yins+zins) {
         score=xins+wins;
         if(aScore<=bScore && score<bScore) {
            bScore=score;
            bPoint=0x06;
         } else if(aScore>bScore && score<aScore) {
            aScore=score;
            aPoint=0x06;
         }
      } else {
         score=yins+zins;
         if(aScore<=bScore && score<bScore) {
            bScore=score;
            bPoint=0x09;
         } else if(aScore>bScore && score<aScore) {
            aScore=score;
            aPoint=0x09;
         }
      }
      
      // Decide if(0,1,1,1) is closer.
      p1=3-inSum+xins;
      if(aScore<=bScore && p1<bScore) {
         bScore=p1;
         bPoint=0x0E;
         bIsBiggerSide=0;
      } else if(aScore>bScore && p1<aScore) {
         aScore=p1;
         aPoint=0x0E;
         aIsBiggerSide=0;
      }
      
      // Decide if(1,0,1,1) is closer.
      p2=3-inSum+yins;
      if(aScore<=bScore && p2<bScore) {
         bScore=p2;
         bPoint=0x0D;
         bIsBiggerSide=0;
      } else if(aScore>bScore && p2<aScore) {
         aScore=p2;
         aPoint=0x0D;
         aIsBiggerSide=0;
      }
      
      // Decide if(1,1,0,1) is closer.
      p3=3-inSum+zins;
      if(aScore<=bScore && p3<bScore) {
         bScore=p3;
         bPoint=0x0B;
         bIsBiggerSide=0;
      } else if(aScore>bScore && p3<aScore) {
         aScore=p3;
         aPoint=0x0B;
         aIsBiggerSide=0;
      }
      
      // Decide if(1,1,1,0) is closer.
      p4=3-inSum+wins;
      if(aScore<=bScore && p4<bScore) {
         bScore=p4;
         bPoint=0x07;
         bIsBiggerSide=0;
      } else if(aScore>bScore && p4<aScore) {
         aScore=p4;
         aPoint=0x07;
         aIsBiggerSide=0;
      }
      
      // Where each of the two closest points are determines how the extra three vertices are calculated.
      if(aIsBiggerSide == bIsBiggerSide) {
         if(aIsBiggerSide) { // Both closest points on the bigger side
            c1=(I8)(aPoint&bPoint);
            c2=(I8)(aPoint | bPoint);
            
            // Two contributions are permutations of (0,0,0,1) and (0,0,0,2) based on c1
            xsv_ext0=xsv_ext1=xsb;
            ysv_ext0=ysv_ext1=ysb;
            zsv_ext0=zsv_ext1=zsb;
            wsv_ext0=wsv_ext1=wsb;
            dx_ext0=dx0-SQUISH_CONSTANT_4D;
            dy_ext0=dy0-SQUISH_CONSTANT_4D;
            dz_ext0=dz0-SQUISH_CONSTANT_4D;
            dw_ext0=dw0-SQUISH_CONSTANT_4D;
            dx_ext1=dx0-2*SQUISH_CONSTANT_4D;
            dy_ext1=dy0-2*SQUISH_CONSTANT_4D;
            dz_ext1=dz0-2*SQUISH_CONSTANT_4D;
            dw_ext1=dw0-2*SQUISH_CONSTANT_4D;
            if((c1&0x01) != 0) {
               xsv_ext0+=1;
               dx_ext0-=1;
               xsv_ext1+=2;
               dx_ext1-=2;
            } else if((c1&0x02) != 0) {
               ysv_ext0+=1;
               dy_ext0-=1;
               ysv_ext1+=2;
               dy_ext1-=2;
            } else if((c1&0x04) != 0) {
               zsv_ext0+=1;
               dz_ext0-=1;
               zsv_ext1+=2;
               dz_ext1-=2;
            } else {
               wsv_ext0+=1;
               dw_ext0-=1;
               wsv_ext1+=2;
               dw_ext1-=2;
            }
            
            // One contribution is a permutation of (1,1,1,-1) based on c2
            xsv_ext2=xsb+1;
            ysv_ext2=ysb+1;
            zsv_ext2=zsb+1;
            wsv_ext2=wsb+1;
            dx_ext2=dx0-1-2*SQUISH_CONSTANT_4D;
            dy_ext2=dy0-1-2*SQUISH_CONSTANT_4D;
            dz_ext2=dz0-1-2*SQUISH_CONSTANT_4D;
            dw_ext2=dw0-1-2*SQUISH_CONSTANT_4D;
            if((c2&0x01) == 0) {
               xsv_ext2-=2;
               dx_ext2+=2;
            } else if((c2&0x02) == 0) {
               ysv_ext2-=2;
               dy_ext2+=2;
            } else if((c2&0x04) == 0) {
               zsv_ext2-=2;
               dz_ext2+=2;
            } else {
               wsv_ext2-=2;
               dw_ext2+=2;
            }
         } else { // Both closest points on the smaller side
            // One of the two extra points is (1,1,1,1)
            xsv_ext2=xsb+1;
            ysv_ext2=ysb+1;
            zsv_ext2=zsb+1;
            wsv_ext2=wsb+1;
            dx_ext2=dx0-1-4*SQUISH_CONSTANT_4D;
            dy_ext2=dy0-1-4*SQUISH_CONSTANT_4D;
            dz_ext2=dz0-1-4*SQUISH_CONSTANT_4D;
            dw_ext2=dw0-1-4*SQUISH_CONSTANT_4D;
            
            // Other two points are based on the shared axes.
            c=(I8)(aPoint&bPoint);
            
            if((c&0x01) != 0) {
               xsv_ext0=xsb+2;
               xsv_ext1=xsb+1;
               dx_ext0=dx0-2-3*SQUISH_CONSTANT_4D;
               dx_ext1=dx0-1-3*SQUISH_CONSTANT_4D;
            } else {
               xsv_ext0=xsv_ext1=xsb;
               dx_ext0=dx_ext1=dx0-3*SQUISH_CONSTANT_4D;
            }
            
            if((c&0x02) != 0) {
               ysv_ext0=ysv_ext1=ysb+1;
               dy_ext0=dy_ext1=dy0-1-3*SQUISH_CONSTANT_4D;
               if((c&0x01) == 0)
               {
                  ysv_ext0+=1;
                  dy_ext0-=1;
               } else {
                  ysv_ext1+=1;
                  dy_ext1-=1;
               }
            } else {
               ysv_ext0=ysv_ext1=ysb;
               dy_ext0=dy_ext1=dy0-3*SQUISH_CONSTANT_4D;
            }
            
            if((c&0x04) != 0) {
               zsv_ext0=zsv_ext1=zsb+1;
               dz_ext0=dz_ext1=dz0-1-3*SQUISH_CONSTANT_4D;
               if((c&0x03) == 0)
               {
                  zsv_ext0+=1;
                  dz_ext0-=1;
               } else {
                  zsv_ext1+=1;
                  dz_ext1-=1;
               }
            } else {
               zsv_ext0=zsv_ext1=zsb;
               dz_ext0=dz_ext1=dz0-3*SQUISH_CONSTANT_4D;
            }
            
            if((c&0x08) != 0)
            {
               wsv_ext0=wsb+1;
               wsv_ext1=wsb+2;
               dw_ext0=dw0-1-3*SQUISH_CONSTANT_4D;
               dw_ext1=dw0-2-3*SQUISH_CONSTANT_4D;
            } else {
               wsv_ext0=wsv_ext1=wsb;
               dw_ext0=dw_ext1=dw0-3*SQUISH_CONSTANT_4D;
            }
         }
      } else { // One point on each "side"
         if(aIsBiggerSide) {
            c1=aPoint;
            c2=bPoint;
         } else {
            c1=bPoint;
            c2=aPoint;
         }
         
         // Two contributions are the bigger-sided point with each 1 replaced with 2.
         if((c1&0x01) != 0) {
            xsv_ext0=xsb+2;
            xsv_ext1=xsb+1;
            dx_ext0=dx0-2-3*SQUISH_CONSTANT_4D;
            dx_ext1=dx0-1-3*SQUISH_CONSTANT_4D;
         } else {
            xsv_ext0=xsv_ext1=xsb;
            dx_ext0=dx_ext1=dx0-3*SQUISH_CONSTANT_4D;
         }
         
         if((c1&0x02) != 0) {
            ysv_ext0=ysv_ext1=ysb+1;
            dy_ext0=dy_ext1=dy0-1-3*SQUISH_CONSTANT_4D;
            if((c1&0x01) == 0) {
               ysv_ext0+=1;
               dy_ext0-=1;
            } else {
               ysv_ext1+=1;
               dy_ext1-=1;
            }
         } else {
            ysv_ext0=ysv_ext1=ysb;
            dy_ext0=dy_ext1=dy0-3*SQUISH_CONSTANT_4D;
         }
         
         if((c1&0x04) != 0) {
            zsv_ext0=zsv_ext1=zsb+1;
            dz_ext0=dz_ext1=dz0-1-3*SQUISH_CONSTANT_4D;
            if((c1&0x03) == 0) {
               zsv_ext0+=1;
               dz_ext0-=1;
            } else {
               zsv_ext1+=1;
               dz_ext1-=1;
            }
         } else {
            zsv_ext0=zsv_ext1=zsb;
            dz_ext0=dz_ext1=dz0-3*SQUISH_CONSTANT_4D;
         }
         
         if((c1&0x08) != 0) {
            wsv_ext0=wsb+1;
            wsv_ext1=wsb+2;
            dw_ext0=dw0-1-3*SQUISH_CONSTANT_4D;
            dw_ext1=dw0-2-3*SQUISH_CONSTANT_4D;
         } else {
            wsv_ext0=wsv_ext1=wsb;
            dw_ext0=dw_ext1=dw0-3*SQUISH_CONSTANT_4D;
         }

         // One contribution is a permutation of (1,1,1,-1) based on the smaller-sided point
         xsv_ext2=xsb+1;
         ysv_ext2=ysb+1;
         zsv_ext2=zsb+1;
         wsv_ext2=wsb+1;
         dx_ext2=dx0-1-2*SQUISH_CONSTANT_4D;
         dy_ext2=dy0-1-2*SQUISH_CONSTANT_4D;
         dz_ext2=dz0-1-2*SQUISH_CONSTANT_4D;
         dw_ext2=dw0-1-2*SQUISH_CONSTANT_4D;
         if((c2&0x01) == 0) {
            xsv_ext2-=2;
            dx_ext2+=2;
         } else if((c2&0x02) == 0) {
            ysv_ext2-=2;
            dy_ext2+=2;
         } else if((c2&0x04) == 0) {
            zsv_ext2-=2;
            dz_ext2+=2;
         } else {
            wsv_ext2-=2;
            dw_ext2+=2;
         }
      }
      
      // Contribution (1,1,1,0)
      dx4=dx0-1-3*SQUISH_CONSTANT_4D;
      dy4=dy0-1-3*SQUISH_CONSTANT_4D;
      dz4=dz0-1-3*SQUISH_CONSTANT_4D;
      dw4=dw0-3*SQUISH_CONSTANT_4D;
      attn4=2-dx4*dx4-dy4*dy4-dz4*dz4-dw4*dw4;
      if(attn4>0) {
         attn4*=attn4;
         value+=attn4*attn4*extrapolate4(xsb+1, ysb+1, zsb+1, wsb+0, dx4, dy4, dz4, dw4);
      }

      // Contribution (1,1,0,1)
      dx3=dx4;
      dy3=dy4;
      dz3=dz0-3*SQUISH_CONSTANT_4D;
      dw3=dw0-1-3*SQUISH_CONSTANT_4D;
      attn3=2-dx3*dx3-dy3*dy3-dz3*dz3-dw3*dw3;
      if(attn3>0) {
         attn3*=attn3;
         value+=attn3*attn3*extrapolate4(xsb+1, ysb+1, zsb+0, wsb+1, dx3, dy3, dz3, dw3);
      }

      // Contribution (1,0,1,1)
      dx2=dx4;
      dy2=dy0-3*SQUISH_CONSTANT_4D;
      dz2=dz4;
      dw2=dw3;
      attn2=2-dx2*dx2-dy2*dy2-dz2*dz2-dw2*dw2;
      if(attn2>0) {
         attn2*=attn2;
         value+=attn2*attn2*extrapolate4(xsb+1, ysb+0, zsb+1, wsb+1, dx2, dy2, dz2, dw2);
      }

      // Contribution (0,1,1,1)
      dx1=dx0-3*SQUISH_CONSTANT_4D;
      dz1=dz4;
      dy1=dy4;
      dw1=dw3;
      attn1=2-dx1*dx1-dy1*dy1-dz1*dz1-dw1*dw1;
      if(attn1>0) {
         attn1*=attn1;
         value+=attn1*attn1*extrapolate4(xsb+0, ysb+1, zsb+1, wsb+1, dx1, dy1, dz1, dw1);
      }
      
      // Contribution (1,1,0,0)
      dx5=dx0-1-2*SQUISH_CONSTANT_4D;
      dy5=dy0-1-2*SQUISH_CONSTANT_4D;
      dz5=dz0-0-2*SQUISH_CONSTANT_4D;
      dw5=dw0-0-2*SQUISH_CONSTANT_4D;
      attn5=2-dx5*dx5-dy5*dy5-dz5*dz5-dw5*dw5;
      if(attn5>0) {
         attn5*=attn5;
         value+=attn5*attn5*extrapolate4(xsb+1, ysb+1, zsb+0, wsb+0, dx5, dy5, dz5, dw5);
      }
      
      // Contribution (1,0,1,0)
      dx6=dx0-1-2*SQUISH_CONSTANT_4D;
      dy6=dy0-0-2*SQUISH_CONSTANT_4D;
      dz6=dz0-1-2*SQUISH_CONSTANT_4D;
      dw6=dw0-0-2*SQUISH_CONSTANT_4D;
      attn6=2-dx6*dx6-dy6*dy6-dz6*dz6-dw6*dw6;
      if(attn6>0) {
         attn6*=attn6;
         value+=attn6*attn6*extrapolate4(xsb+1, ysb+0, zsb+1, wsb+0, dx6, dy6, dz6, dw6);
      }

      // Contribution (1,0,0,1)
      dx7=dx0-1-2*SQUISH_CONSTANT_4D;
      dy7=dy0-0-2*SQUISH_CONSTANT_4D;
      dz7=dz0-0-2*SQUISH_CONSTANT_4D;
      dw7=dw0-1-2*SQUISH_CONSTANT_4D;
      attn7=2-dx7*dx7-dy7*dy7-dz7*dz7-dw7*dw7;
      if(attn7>0) {
         attn7*=attn7;
         value+=attn7*attn7*extrapolate4(xsb+1, ysb+0, zsb+0, wsb+1, dx7, dy7, dz7, dw7);
      }
      
      // Contribution (0,1,1,0)
      dx8=dx0-0-2*SQUISH_CONSTANT_4D;
      dy8=dy0-1-2*SQUISH_CONSTANT_4D;
      dz8=dz0-1-2*SQUISH_CONSTANT_4D;
      dw8=dw0-0-2*SQUISH_CONSTANT_4D;
      attn8=2-dx8*dx8-dy8*dy8-dz8*dz8-dw8*dw8;
      if(attn8>0) {
         attn8*=attn8;
         value+=attn8*attn8*extrapolate4(xsb+0, ysb+1, zsb+1, wsb+0, dx8, dy8, dz8, dw8);
      }
      
      // Contribution (0,1,0,1)
      dx9=dx0-0-2*SQUISH_CONSTANT_4D;
      dy9=dy0-1-2*SQUISH_CONSTANT_4D;
      dz9=dz0-0-2*SQUISH_CONSTANT_4D;
      dw9=dw0-1-2*SQUISH_CONSTANT_4D;
      attn9=2-dx9*dx9-dy9*dy9-dz9*dz9-dw9*dw9;
      if(attn9>0) {
         attn9*=attn9;
         value+=attn9*attn9*extrapolate4(xsb+0, ysb+1, zsb+0, wsb+1, dx9, dy9, dz9, dw9);
      }
      
      // Contribution (0,0,1,1)
      dx10=dx0-0-2*SQUISH_CONSTANT_4D;
      dy10=dy0-0-2*SQUISH_CONSTANT_4D;
      dz10=dz0-1-2*SQUISH_CONSTANT_4D;
      dw10=dw0-1-2*SQUISH_CONSTANT_4D;
      attn10=2-dx10*dx10-dy10*dy10-dz10*dz10-dw10*dw10;
      if(attn10>0) {
         attn10*=attn10;
         value+=attn10*attn10*extrapolate4(xsb+0, ysb+0, zsb+1, wsb+1, dx10, dy10, dz10, dw10);
      }
   }

   // First extra vertex
   attn_ext0=2-dx_ext0*dx_ext0-dy_ext0*dy_ext0-dz_ext0*dz_ext0-dw_ext0*dw_ext0;
   if(attn_ext0>0)
   {
      attn_ext0*=attn_ext0;
      value+=attn_ext0*attn_ext0*extrapolate4(xsv_ext0, ysv_ext0, zsv_ext0, wsv_ext0, dx_ext0, dy_ext0, dz_ext0, dw_ext0);
   }

   // Second extra vertex
   attn_ext1=2-dx_ext1*dx_ext1-dy_ext1*dy_ext1-dz_ext1*dz_ext1-dw_ext1*dw_ext1;
   if(attn_ext1>0)
   {
      attn_ext1*=attn_ext1;
      value+=attn_ext1*attn_ext1*extrapolate4(xsv_ext1, ysv_ext1, zsv_ext1, wsv_ext1, dx_ext1, dy_ext1, dz_ext1, dw_ext1);
   }

   // Third extra vertex
   attn_ext2=2-dx_ext2*dx_ext2-dy_ext2*dy_ext2-dz_ext2*dz_ext2-dw_ext2*dw_ext2;
   if(attn_ext2>0)
   {
      attn_ext2*=attn_ext2;
      value+=attn_ext2*attn_ext2*extrapolate4(xsv_ext2, ysv_ext2, zsv_ext2, wsv_ext2, dx_ext2, dy_ext2, dz_ext2, dw_ext2);
   }

   return value/(SIMPLEX_NORM_CONSTANT_4D);
}
/******************************************************************************/
Flt SimplexNoise::tiledNoise(Dbl x, Int tile)C
{
#if SIMPLEX_WRAP_CIRCLE // CosSin
   VecD2 X; Dbl mul=PID2/tile;
   CosSin(X.x, X.y, x*mul);
   mul=1/mul; // 'CosSin' generates a circle with radius=1, that Circle will have a perimeter "PI2*r" = "PI2", so the coordinates will travel 'PI2' distance, however we want them to travel 'tile' distance, so we need to multiply by "tile/PI2", since we already have 'mul' calculated as "PI2/tile", we can just inverse it
   return noise(X.x*mul, X.y*mul);
#else // Lerp
   x=Frac(x, tile);
   Flt n=noise(x);
   if(x>tile-1) // if X is at the border
   {
      Flt x1=x-tile;
      n=Lerp(noise(x1), n, _SmoothCube(-x1)); // then lerp smoothly with the start
   }
   return n;
#endif
}
Flt SimplexNoise::tiledNoise(Dbl x, Dbl y, C VecI2 &tile)C
{
#if SIMPLEX_WRAP_CIRCLE // CosSin
   VecD2 X, Y; VecD2 mul=PID2/tile;
   CosSin(X.x, X.y, x*mul.x);
   CosSin(Y.x, Y.y, y*mul.y);
   mul=1/mul; // 'CosSin' generates a circle with radius=1, that Circle will have a perimeter "PI2*r" = "PI2", so the coordinates will travel 'PI2' distance, however we want them to travel 'tile' distance, so we need to multiply by "tile/PI2", since we already have 'mul' calculated as "PI2/tile", we can just inverse it
   return noise(X.x*mul.x, X.y*mul.x, Y.x*mul.y, Y.y*mul.y);
#else // Lerp
   x=Frac(x, tile.x);
   y=Frac(y, tile.y);
   Flt n=noise(x, y);
   if(y>tile.y-1) // if Y is at the border
   {
      Flt y1=y-tile.y, sy=_SmoothCube(-y1), sy1=1-sy; // use smooth blending
      n=n*sy + noise(x, y1)*sy1; // lerp with the start
      if(x>tile.x-1) // if X is at the border
      {
         Flt x1=x-tile.x;
         n=Lerp(noise(x1, y)*sy + noise(x1, y1)*sy1, n, _SmoothCube(-x1));
      }
   }else
   if(x>tile.x-1) // if X is at the border
   {
      Flt x1=x-tile.x;
      n=Lerp(noise(x1, y), n, _SmoothCube(-x1)); // then lerp smoothly with the start
   }
   return n;
#endif
}
Flt SimplexNoise::tiledNoise(Dbl x, Dbl y, Dbl z, C VecI &tile)C
{
#if SIMPLEX_WRAP_CIRCLE // CosSin
   VecD2 X, Y, Z; VecD mul=PID2/tile;
   CosSin(X.x, X.y, x*mul.x);
   CosSin(Y.x, Y.y, y*mul.y);
   CosSin(Z.x, Z.y, z*mul.z);
   mul=1/mul; // 'CosSin' generates a circle with radius=1, that Circle will have a perimeter "PI2*r" = "PI2", so the coordinates will travel 'PI2' distance, however we want them to travel 'tile' distance, so we need to multiply by "tile/PI2", since we already have 'mul' calculated as "PI2/tile", we can just inverse it
   return noise(X.x*mul.x + Z.x*mul.z, X.y*mul.x,  // Warning: here we just offset 2 coordinates by the Z coordinate, this is not perfect
                Y.x*mul.y + Z.y*mul.z, Y.y*mul.y);
#else // Lerp
   x=Frac(x, tile.x);
   y=Frac(y, tile.y);
   z=Frac(z, tile.z);
   Flt n=noise(x, y, z);
   if(z>tile.z-1) // if Z is at the border
   {
      Flt z1=z-tile.z, sz=_SmoothCube(-z1), sz1=1-sz; // use smooth blending
      n=n*sz + noise(x, y, z1)*sz1; // lerp with the start
      if(y>tile.y-1) // if Y is at the border
      {
         Flt y1=y-tile.y, sy=_SmoothCube(-y1), sy1=1-sy; // use smooth blending
         n=n*sy + (noise(x, y1, z)*sz + noise(x, y1, z1)*sz1)*sy1;
         if(x>tile.x-1) // if X is at the border
         {
            Flt x1=x-tile.x;
            n=Lerp((noise(x1, y , z)*sz + noise(x1, y , z1)*sz1)*sy
                  +(noise(x1, y1, z)*sz + noise(x1, y1, z1)*sz1)*sy1, n, _SmoothCube(-x1));
         }
      }else
      if(x>tile.x-1) // if X is at the border
      {
         Flt x1=x-tile.x;
         n=Lerp(noise(x1, y, z)*sz + noise(x1, y, z1)*sz1, n, _SmoothCube(-x1));
      }
   }else
   if(y>tile.y-1) // if Y is at the border
   {
      Flt y1=y-tile.y, sy=_SmoothCube(-y1), sy1=1-sy; // use smooth blending
      n=n*sy + noise(x, y1, z)*sy1; // lerp with the start
      if(x>tile.x-1) // if X is at the border
      {
         Flt x1=x-tile.x;
         n=Lerp(noise(x1, y, z)*sy + noise(x1, y1, z)*sy1, n, _SmoothCube(-x1));
      }
   }else
   if(x>tile.x-1) // if X is at the border
   {
      Flt x1=x-tile.x;
      n=Lerp(noise(x1, y, z), n, _SmoothCube(-x1)); // then lerp smoothly with the start
   }
   return n;
#endif
}
Flt SimplexNoise::tiledNoise(Dbl x, Dbl y, Dbl z, Dbl w, C VecI4 &tile)C
{
#if SIMPLEX_WRAP_CIRCLE // CosSin
   VecD2 X, Y, Z, W; VecD4 mul=PID2/tile;
   CosSin(X.x, X.y, x*mul.x);
   CosSin(Y.x, Y.y, y*mul.y);
   CosSin(Z.x, Z.y, z*mul.z);
   CosSin(W.x, W.y, w*mul.w);
   mul=1/mul; // 'CosSin' generates a circle with radius=1, that Circle will have a perimeter "PI2*r" = "PI2", so the coordinates will travel 'PI2' distance, however we want them to travel 'tile' distance, so we need to multiply by "tile/PI2", since we already have 'mul' calculated as "PI2/tile", we can just inverse it
   return noise(X.x*mul.x + Z.x*mul.z, X.y*mul.x + W.x*mul.w,  // Warning: here we just offset coordinates by the ZW coordinates, this is not perfect
                Y.x*mul.y + Z.y*mul.z, Y.y*mul.y + W.y*mul.w);
#else // Lerp
   x=Frac(x, tile.x);
   y=Frac(y, tile.y);
   z=Frac(z, tile.z);
   w=Frac(w, tile.w);
   Flt n=noise(x, y, z, w);

   if(w>tile.w-1) // if W is at the border
   {
      Flt w1=w-tile.w, sw=_SmoothCube(-w1), sw1=1-sw; // use smooth blending
      n=n*sw + noise(x, y, z, w1)*sw1; // lerp with the start
      if(z>tile.z-1) // if Z is at the border
      {
         Flt z1=z-tile.z, sz=_SmoothCube(-z1), sz1=1-sz; // use smooth blending
         n=n*sz + (noise(x, y, z1, w)*sw + noise(x, y, z1, w1)*sw1)*sz1; // lerp with the start
         if(y>tile.y-1) // if Y is at the border
         {
            Flt y1=y-tile.y, sy=_SmoothCube(-y1), sy1=1-sy; // use smooth blending
            n=n*sy + ((noise(x, y1, z, w )*sz + noise(x, y1, z1, w )*sz1)*sw
                     +(noise(x, y1, z, w1)*sz + noise(x, y1, z1, w1)*sz1)*sw1)*sy1;
            if(x>tile.x-1) // if X is at the border
            {
               Flt x1=x-tile.x;
               n=Lerp(((noise(x1, y , z, w )*sz + noise(x1, y , z1, w )*sz1)*sy
                      +(noise(x1, y1, z, w )*sz + noise(x1, y1, z1, w )*sz1)*sy1)*sw
                     +((noise(x1, y , z, w1)*sz + noise(x1, y , z1, w1)*sz1)*sy
                      +(noise(x1, y1, z, w1)*sz + noise(x1, y1, z1, w1)*sz1)*sy1)*sw1, n, _SmoothCube(-x1));
            }
         }else
         if(x>tile.x-1) // if X is at the border
         {
            Flt x1=x-tile.x;
            n=Lerp((noise(x1, y, z, w )*sz + noise(x1, y, z1, w )*sz1)*sw
                  +(noise(x1, y, z, w1)*sz + noise(x1, y, z1, w1)*sz1)*sw1, n, _SmoothCube(-x1));
         }
      }else
      if(y>tile.y-1) // if Y is at the border
      {
         Flt y1=y-tile.y, sy=_SmoothCube(-y1), sy1=1-sy; // use smooth blending
         n=n*sy + (noise(x, y1, z, w)*sw + noise(x, y1, z, w1)*sw1)*sy1; // lerp with the start
         if(x>tile.x-1) // if X is at the border
         {
            Flt x1=x-tile.x;
            n=Lerp((noise(x1, y, z, w )*sy + noise(x1, y1, z, w )*sy1)*sw
                  +(noise(x1, y, z, w1)*sy + noise(x1, y1, z, w1)*sy1)*sw1, n, _SmoothCube(-x1));
         }
      }else
      if(x>tile.x-1) // if X is at the border
      {
         Flt x1=x-tile.x;
         n=Lerp(noise(x1, y, z, w)*sw + noise(x1, y, z, w1)*sw1, n, _SmoothCube(-x1)); // then lerp smoothly with the start
      }
   }else
   if(z>tile.z-1) // if Z is at the border
   {
      Flt z1=z-tile.z, sz=_SmoothCube(-z1), sz1=1-sz; // use smooth blending
      n=n*sz + noise(x, y, z1, w)*sz1; // lerp with the start
      if(y>tile.y-1) // if Y is at the border
      {
         Flt y1=y-tile.y, sy=_SmoothCube(-y1), sy1=1-sy; // use smooth blending
         n=n*sy + (noise(x, y1, z, w)*sz + noise(x, y1, z1, w)*sz1)*sy1;
         if(x>tile.x-1) // if X is at the border
         {
            Flt x1=x-tile.x;
            n=Lerp((noise(x1, y , z, w)*sz + noise(x1, y , z1, w)*sz1)*sy
                  +(noise(x1, y1, z, w)*sz + noise(x1, y1, z1, w)*sz1)*sy1, n, _SmoothCube(-x1));
         }
      }else
      if(x>tile.x-1) // if X is at the border
      {
         Flt x1=x-tile.x;
         n=Lerp(noise(x1, y, z, w)*sz + noise(x1, y, z1, w)*sz1, n, _SmoothCube(-x1));
      }
   }else
   if(y>tile.y-1) // if Y is at the border
   {
      Flt y1=y-tile.y, sy=_SmoothCube(-y1), sy1=1-sy; // use smooth blending
      n=n*sy + noise(x, y1, z, w)*sy1; // lerp with the start
      if(x>tile.x-1) // if X is at the border
      {
         Flt x1=x-tile.x;
         n=Lerp(noise(x1, y, z, w)*sy + noise(x1, y1, z, w)*sy1, n, _SmoothCube(-x1));
      }
   }else
   if(x>tile.x-1) // if X is at the border
   {
      Flt x1=x-tile.x;
      n=Lerp(noise(x1, y, z, w), n, _SmoothCube(-x1)); // then lerp smoothly with the start
   }
   return n;
#endif
}
/******************************************************************************/
// MULTI
/******************************************************************************/
Flt SimplexNoise::noise1(Dbl x, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=noise(x); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      amp   *=gain;
   }
   return result;
}
Flt SimplexNoise::noise2(Dbl x, Dbl y, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=noise(x, y); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      amp   *=gain;
   }
   return result;
}
Flt SimplexNoise::noise3(Dbl x, Dbl y, Dbl z, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=noise(x, y, z); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      z     *=2;
      amp   *=gain;
   }
   return result;
}
Flt SimplexNoise::noise4(Dbl x, Dbl y, Dbl z, Dbl w, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=noise(x, y, z, w); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      z     *=2;
      w     *=2;
      amp   *=gain;
   }
   return result;
}
/******************************************************************************/
Flt SimplexNoise::tiledNoise1(Dbl x, Int tile, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=tiledNoise(x, tile); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      tile  *=2;
      amp   *=gain;
   }
   return result;
}
Flt SimplexNoise::tiledNoise2(Dbl x, Dbl y, VecI2 tile, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=tiledNoise(x, y, tile); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      tile  *=2;
      amp   *=gain;
   }
   return result;
}
Flt SimplexNoise::tiledNoise3(Dbl x, Dbl y, Dbl z, VecI tile, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=tiledNoise(x, y, z, tile); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      z     *=2;
      tile  *=2;
      amp   *=gain;
   }
   return result;
}
Flt SimplexNoise::tiledNoise4(Dbl x, Dbl y, Dbl z, Dbl w, VecI4 tile, Int octaves, Flt gain, Flt Transform(Flt noise))C
{
   Flt result=0, amp=1; REP(octaves)
   {
      Flt n=tiledNoise(x, y, z, w, tile); if(Transform)n=Transform(n);
      result+=n*amp;
      x     *=2;
      y     *=2;
      z     *=2;
      w     *=2;
      tile  *=2;
      amp   *=gain;
   }
   return result;
}
/******************************************************************************/
// BLOOM
/******************************************************************************/
Flt SimplexNoise::noise1Bloom(Dbl x, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=noise(x), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=noise(x*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt SimplexNoise::noise2Bloom(Dbl x, Dbl y, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=noise(x, y), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=noise(x*=2, y*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt SimplexNoise::noise3Bloom(Dbl x, Dbl y, Dbl z, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=noise(x, y, z), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=noise(x*=2, y*=2, z*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt SimplexNoise::noise4Bloom(Dbl x, Dbl y, Dbl z, Dbl w, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=noise(x, y, z, w), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=noise(x*=2, y*=2, z*=2, w*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
/******************************************************************************/
Flt SimplexNoise::tiledNoise1Bloom(Dbl x, Int tile, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=tiledNoise(x, tile), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=tiledNoise(x*=2, tile*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt SimplexNoise::tiledNoise2Bloom(Dbl x, Dbl y, VecI2 tile, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=tiledNoise(x, y, tile), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=tiledNoise(x*=2, y*=2, tile*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt SimplexNoise::tiledNoise3Bloom(Dbl x, Dbl y, Dbl z, VecI tile, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=tiledNoise(x, y, z, tile), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=tiledNoise(x*=2, y*=2, z*=2, tile*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
Flt SimplexNoise::tiledNoise4Bloom(Dbl x, Dbl y, Dbl z, Dbl w, VecI4 tile, Int octaves, Flt bloom, Flt sharpness)C
{
   Flt result=tiledNoise(x, y, z, w, tile), weight=result*bloom+sharpness, amp=1;
   for(Int i=1; i<octaves; i++)
   {
      Flt n=tiledNoise(x*=2, y*=2, z*=2, w*=2, tile*=2);
      amp   *=BloomGain;
      weight*=amp;
      MAX(weight, 0);
      result+=weight*n;
      weight*=n*bloom+sharpness;
   }
   return result;
}
/******************************************************************************/
// MASK
/******************************************************************************/
Flt PerlinNoise::mask2(Dbl x, Dbl y, Int octaves, Flt sharpness)C
{
   Flt mask=1; REP(octaves)
   {
      Flt m=noise(x, y);
      mask*=SmoothCube(m*sharpness+0.5f);
      x*=2;
      y*=2;
   }
   return mask;
}
/******************************************************************************/
Flt SimplexNoise::mask2(Dbl x, Dbl y, Int octaves, Flt sharpness)C
{
   Flt mask=1; REP(octaves)
   {
      Flt m=noise(x, y);
      mask*=SmoothCube(m*sharpness+0.5f);
      x*=2;
      y*=2;
   }
   return mask;
}
/******************************************************************************/
}
/******************************************************************************/
