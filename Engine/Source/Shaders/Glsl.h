#define PAR uniform
#define VAR varying

#define EPS     0.0001
#define EPS_COL (1.0/256.0)
#define PI      3.14159265
#define PI2     6.28318531

#define SBUMP_ZERO   0
#define SBUMP_FLAT   1
#define SBUMP_NORMAL 2

#define FX_NONE  0
#define FX_GRASS 1
#define FX_LEAF  2
#define FX_LEAFS 3
#define FX_BONE  4

#define REVERSE_DEPTH false

#define Sat(x) Mid(x, 0.0, 1.0)

MP Flt Sum(MP Vec2 v) {return v.x+v.y        ;}
MP Flt Sum(MP Vec  v) {return v.x+v.y+v.z    ;}
MP Flt Sum(MP Vec4 v) {return v.x+v.y+v.z+v.w;}

MP Flt  Sqr (MP Flt  x) {return x*x  ;}
MP Vec2 Sqr (MP Vec2 x) {return x*x  ;}
MP Flt  Cube(MP Flt  x) {return x*x*x;}

HP Flt Length2(HP Vec2 v) {return Dot(v, v);}
HP Flt Length2(HP Vec  v) {return Dot(v, v);}
HP Flt Length2(HP Vec4 v) {return Dot(v, v);}

MP Vec2 Rotate(MP Vec2 vec, MP Vec2 cos_sin)
{
   return Vec2(vec.x*cos_sin.x - vec.y*cos_sin.y,
               vec.x*cos_sin.y + vec.y*cos_sin.x);
}

MP Flt LerpR (MP Flt from, MP Flt to, MP Flt v) {return     (v-from)/(to-from) ;}
MP Flt LerpRS(MP Flt from, MP Flt to, MP Flt v) {return Sat((v-from)/(to-from));}
