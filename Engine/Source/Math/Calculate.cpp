/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
enum CALC_ELM_TYPE : Byte
{
   CET_NONE,
   CET_OP  ,
   CET_VAL ,
   CET_VAR ,
   CET_FUNC,
};
/******************************************************************************/
Char CalcError[128];
/******************************************************************************/
static Bool E(C Str &name                                                                     ) {Set(CalcError,   name                                                                                ); return false;}
static Bool E(C Str &name, C CalcValue &arg                                                   ) {Set(CalcError, S+name+'('+arg.typeName()                                                         +')'); return false;}
static Bool E(C Str &name, C CalcValue &a0 , C CalcValue &a1                                  ) {Set(CalcError, S+name+'('+a0 .typeName()+", "+a1.typeName()                                      +')'); return false;}
static Bool E(C Str &name, C CalcValue &a0 , C CalcValue &a1, C CalcValue &a2                 ) {Set(CalcError, S+name+'('+a0 .typeName()+", "+a1.typeName()+", "+a2.typeName()                   +')'); return false;}
static Bool E(C Str &name, C CalcValue &a0 , C CalcValue &a1, C CalcValue &a2, C CalcValue &a3) {Set(CalcError, S+name+'('+a0 .typeName()+", "+a1.typeName()+", "+a2.typeName()+", "+a3.typeName()+')'); return false;}
/******************************************************************************/
CChar8* CalcValue::typeName()C
{
   switch(type)
   {
      case CVAL_INT : return "Int";
      case CVAL_REAL: return "Real";
      case CVAL_VEC2: return "Vec2";
      case CVAL_VEC : return "Vec";
      case CVAL_VEC4: return "Vec4";
      default       : return  null;
   }
}
Str CalcValue::asText(Int precision)C
{
   switch(type)
   {
      case CVAL_INT : return TextInt (i); break;
      case CVAL_REAL: return TextReal(r, (precision==INT_MAX) ? PRECISION_DBL : precision);
      case CVAL_VEC2: return v2.asText(precision);
      case CVAL_VEC : return v .asText(precision);
      case CVAL_VEC4: return v4.asText(precision);
      default       : return S;
   }
}
Int CalcValue::asInt()C
{
   switch(type)
   {
      case CVAL_INT : return       i;
      case CVAL_REAL: return Round(r   );
      case CVAL_VEC2: return Round(v2.x);
      case CVAL_VEC : return Round(v .x);
      case CVAL_VEC4: return Round(v4.x);
      default       : return       0;
   }
}
UInt CalcValue::asUInt()C
{
   switch(type)
   {
      case CVAL_INT : return        i;
      case CVAL_REAL: return RoundU(r   );
      case CVAL_VEC2: return RoundU(v2.x);
      case CVAL_VEC : return RoundU(v .x);
      case CVAL_VEC4: return RoundU(v4.x);
      default       : return        0;
   }
}
Long CalcValue::asLong()C
{
   switch(type)
   {
      case CVAL_INT : return        i;
      case CVAL_REAL: return RoundL(r   );
      case CVAL_VEC2: return RoundL(v2.x);
      case CVAL_VEC : return RoundL(v .x);
      case CVAL_VEC4: return RoundL(v4.x);
      default       : return        0;
   }
}
ULong CalcValue::asULong()C
{
   switch(type)
   {
      case CVAL_INT : return         i;
      case CVAL_REAL: return RoundUL(r   );
      case CVAL_VEC2: return RoundUL(v2.x);
      case CVAL_VEC : return RoundUL(v .x);
      case CVAL_VEC4: return RoundUL(v4.x);
      default       : return         0;
   }
}
Flt CalcValue::asFlt()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return v2.x;
      case CVAL_VEC : return v .x;
      case CVAL_VEC4: return v4.x;
      default       : return 0;
   }
}
Dbl CalcValue::asDbl()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return v2.x;
      case CVAL_VEC : return v .x;
      case CVAL_VEC4: return v4.x;
      default       : return 0;
   }
}
Vec2 CalcValue::asVec2()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return v2;
      case CVAL_VEC : return v .xy;
      case CVAL_VEC4: return v4.xy;
      default       : return 0;
   }
}
VecD2 CalcValue::asVecD2()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return v2;
      case CVAL_VEC : return v .xy;
      case CVAL_VEC4: return v4.xy;
      default       : return 0;
   }
}
VecI2 CalcValue::asVecI2()C
{
   switch(type)
   {
      case CVAL_INT : return       i;
      case CVAL_REAL: return Round(r    );
      case CVAL_VEC2: return Round(v2   );
      case CVAL_VEC : return Round(v .xy);
      case CVAL_VEC4: return Round(v4.xy);
      default       : return       0;
   }
}
Vec CalcValue::asVec()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return Vec(v2, 0);
      case CVAL_VEC : return v;
      case CVAL_VEC4: return v4.xyz;
      default       : return 0;
   }
}
VecD CalcValue::asVecD()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return VecD(v2, 0);
      case CVAL_VEC : return v;
      case CVAL_VEC4: return v4.xyz;
      default       : return 0;
   }
}
VecI CalcValue::asVecI()C
{
   switch(type)
   {
      case CVAL_INT : return            i;
      case CVAL_REAL: return      Round(r     );
      case CVAL_VEC2: return VecI(Round(v2    ), 0);
      case CVAL_VEC : return      Round(v     );
      case CVAL_VEC4: return      Round(v4.xyz);
      default       : return            0;
   }
}
Vec4 CalcValue::asVec4()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return Vec4(v2, 0, 0);
      case CVAL_VEC : return Vec4(v    , 0);
      case CVAL_VEC4: return v4;
      default       : return 0;
   }
}
VecD4 CalcValue::asVecD4()C
{
   switch(type)
   {
      case CVAL_INT : return i;
      case CVAL_REAL: return r;
      case CVAL_VEC2: return VecD4(v2, 0, 0);
      case CVAL_VEC : return VecD4(v    , 0);
      case CVAL_VEC4: return v4;
      default       : return 0;
   }
}
VecI4 CalcValue::asVecI4()C
{
   switch(type)
   {
      case CVAL_INT : return             i;
      case CVAL_REAL: return       Round(r );
      case CVAL_VEC2: return VecI4(Round(v2), 0, 0);
      case CVAL_VEC : return VecI4(Round(v )   , 0);
      case CVAL_VEC4: return       Round(v4);
      default       : return             0;
   }
}
/******************************************************************************/
Bool CalcValue::notLogical()
{
   switch(type)
   {
      case CVAL_INT : i =!i ; return true;
      case CVAL_REAL: r =!r ; return true;
      case CVAL_VEC2: v2=!v2; return true;
      case CVAL_VEC : v =!v ; return true;
      case CVAL_VEC4: v4=!v4; return true;
      default       :         return false;
   }
}
Bool CalcValue::notBitwise()
{
   switch(type)
   {
      case CVAL_INT: i=~i; return true;
      default      :       return false;
   }
}
Bool CalcValue::chs()
{
   switch(type)
   {
      case CVAL_INT : CHS(i)  ; return true;
      case CVAL_REAL: CHS(r)  ; return true;
      case CVAL_VEC2: v2.chs(); return true;
      case CVAL_VEC : v .chs(); return true;
      case CVAL_VEC4: v4.chs(); return true;
      default       :           return false;
   }
}
Bool CalcValue::abs()
{
   switch(type)
   {
      case CVAL_INT : ABS(i)  ; return true;
      case CVAL_REAL: ABS(r)  ; return true;
      case CVAL_VEC2: v2.abs(); return true;
      case CVAL_VEC : v .abs(); return true;
      case CVAL_VEC4: v4.abs(); return true;
      default       :           return false;
   }
}
Bool CalcValue::sign()
{
   switch(type)
   {
      case CVAL_INT : i=Sign(i); return true;
      case CVAL_REAL: i=Sign(r); type=CVAL_INT; return true;
      case CVAL_VEC2: v2.set(Sign(v2.x), Sign(v2.y)); return true;
      case CVAL_VEC : v .set(Sign(v .x), Sign(v .y), Sign(v .z)); return true;
      case CVAL_VEC4: v4.set(Sign(v4.x), Sign(v4.y), Sign(v4.z), Sign(v4.w)); return true;
      default       : return false;
   }
}
/******************************************************************************/
Bool CalcValue::sat()
{
   switch(type)
   {
      case CVAL_INT : SAT(i)  ; return true;
      case CVAL_REAL: SAT(r)  ; return true;
      case CVAL_VEC2: v2.sat(); return true;
      case CVAL_VEC : v .sat(); return true;
      case CVAL_VEC4: v4.sat(); return true;
      default       :           return false;
   }
}
Bool CalcValue::min(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=Min(    i , x.i);                 return true;
         case CVAL_REAL: r=Min(Dbl(i), x.r); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=Min(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=Min(r,     x.r ); return true;
      }break;

      case CVAL_VEC2: if(x.type==CVAL_VEC2){v2=Min(v2, x.v2); return true;} break;
      case CVAL_VEC : if(x.type==CVAL_VEC ){v =Min(v , x.v ); return true;} break;
      case CVAL_VEC4: if(x.type==CVAL_VEC4){v4=Min(v4, x.v4); return true;} break;
   }
   return false;
}
Bool CalcValue::max(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=Max(    i , x.i);                 return true;
         case CVAL_REAL: r=Max(Dbl(i), x.r); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=Max(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=Max(r,     x.r ); return true;
      }break;

      case CVAL_VEC2: if(x.type==CVAL_VEC2){v2=Max(v2, x.v2); return true;} break;
      case CVAL_VEC : if(x.type==CVAL_VEC ){v =Max(v , x.v ); return true;} break;
      case CVAL_VEC4: if(x.type==CVAL_VEC4){v4=Max(v4, x.v4); return true;} break;
   }
   return false;
}
Bool CalcValue::avg(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r=Avg(Dbl(i), Dbl(x.i)); type=CVAL_REAL; return true;
         case CVAL_REAL: r=Avg(Dbl(i),     x.r ); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=Avg(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=Avg(r,     x.r ); return true;
      }break;

      case CVAL_VEC2: if(x.type==CVAL_VEC2){v2=Avg(v2, x.v2); return true;} break;
      case CVAL_VEC : if(x.type==CVAL_VEC ){v =Avg(v , x.v ); return true;} break;
      case CVAL_VEC4: if(x.type==CVAL_VEC4){v4=Avg(v4, x.v4); return true;} break;
   }
   return false;
}
/******************************************************************************/
Bool CalcValue::add(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i =i+x.i ;                 return true;
         case CVAL_REAL: r =i+x.r ; type=CVAL_REAL; return true;
         case CVAL_VEC2: v2=i+x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =i+x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=i+x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r =r+x.i ;                 return true;
         case CVAL_REAL: r =r+x.r ;                 return true;
         case CVAL_VEC2: v2=r+x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =r+x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=r+x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_VEC2: switch(x.type)
      {
         case CVAL_INT : v2+=x.i ; return true;
         case CVAL_REAL: v2+=x.r ; return true;
         case CVAL_VEC2: v2+=x.v2; return true;
      }break;

      case CVAL_VEC: switch(x.type)
      {
         case CVAL_INT : v+=x.i; return true;
         case CVAL_REAL: v+=x.r; return true;
         case CVAL_VEC : v+=x.v; return true;
      }break;

      case CVAL_VEC4: switch(x.type)
      {
         case CVAL_INT : v4+=x.i ; return true;
         case CVAL_REAL: v4+=x.r ; return true;
         case CVAL_VEC4: v4+=x.v4; return true;
      }break;
   }
   return false;
}
Bool CalcValue::sub(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i =i-x.i ;                 return true;
         case CVAL_REAL: r =i-x.r ; type=CVAL_REAL; return true;
         case CVAL_VEC2: v2=i-x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =i-x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=i-x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r =r-x.i ;                 return true;
         case CVAL_REAL: r =r-x.r ;                 return true;
         case CVAL_VEC2: v2=r-x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =r-x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=r-x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_VEC2: switch(x.type)
      {
         case CVAL_INT : v2-=x.i ; return true;
         case CVAL_REAL: v2-=x.r ; return true;
         case CVAL_VEC2: v2-=x.v2; return true;
      }break;

      case CVAL_VEC: switch(x.type)
      {
         case CVAL_INT : v-=x.i; return true;
         case CVAL_REAL: v-=x.r; return true;
         case CVAL_VEC : v-=x.v; return true;
      }break;

      case CVAL_VEC4: switch(x.type)
      {
         case CVAL_INT : v4-=x.i ; return true;
         case CVAL_REAL: v4-=x.r ; return true;
         case CVAL_VEC4: v4-=x.v4; return true;
      }break;
   }
   return false;
}
Bool CalcValue::mul(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i =i*x.i ;                 return true;
         case CVAL_REAL: r =i*x.r ; type=CVAL_REAL; return true;
         case CVAL_VEC2: v2=i*x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =i*x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=i*x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r =r*x.i ;                 return true;
         case CVAL_REAL: r =r*x.r ;                 return true;
         case CVAL_VEC2: v2=r*x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =r*x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=r*x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_VEC2: switch(x.type)
      {
         case CVAL_INT : v2*=x.i ; return true;
         case CVAL_REAL: v2*=x.r ; return true;
         case CVAL_VEC2: v2*=x.v2; return true;
      }break;

      case CVAL_VEC: switch(x.type)
      {
         case CVAL_INT : v*=x.i; return true;
         case CVAL_REAL: v*=x.r; return true;
         case CVAL_VEC : v*=x.v; return true;
      }break;

      case CVAL_VEC4: switch(x.type)
      {
         case CVAL_INT : v4*=x.i ; return true;
         case CVAL_REAL: v4*=x.r ; return true;
         case CVAL_VEC4: v4*=x.v4; return true;
      }break;
   }
   return false;
}
Bool CalcValue::div(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r =Dbl(i)/x.i ; type=CVAL_REAL; return true;
         case CVAL_REAL: r =    i /x.r ; type=CVAL_REAL; return true;
         case CVAL_VEC2: v2=    i /x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =    i /x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=    i /x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r =r/x.i ;                 return true;
         case CVAL_REAL: r =r/x.r ;                 return true;
         case CVAL_VEC2: v2=r/x.v2; type=CVAL_VEC2; return true;
         case CVAL_VEC : v =r/x.v ; type=CVAL_VEC ; return true;
         case CVAL_VEC4: v4=r/x.v4; type=CVAL_VEC4; return true;
      }break;

      case CVAL_VEC2: switch(x.type)
      {
         case CVAL_INT : v2/=x.i ; return true;
         case CVAL_REAL: v2/=x.r ; return true;
         case CVAL_VEC2: v2/=x.v2; return true;
      }break;

      case CVAL_VEC: switch(x.type)
      {
         case CVAL_INT : v/=x.i; return true;
         case CVAL_REAL: v/=x.r; return true;
         case CVAL_VEC : v/=x.v; return true;
      }break;

      case CVAL_VEC4: switch(x.type)
      {
         case CVAL_INT : v4/=x.i ; return true;
         case CVAL_REAL: v4/=x.r ; return true;
         case CVAL_VEC4: v4/=x.v4; return true;
      }break;
   }
   return false;
}
Bool CalcValue::mod(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT: if(x.i)i%=x.i;else i=0; return true;
      }break;
   }
   return false;
}
/******************************************************************************/
Bool CalcValue::sqr()
{
   switch(type)
   {
      case CVAL_INT : i *=i ; return true;
      case CVAL_REAL: r *=r ; return true;
      case CVAL_VEC2: v2*=v2; return true;
      case CVAL_VEC : v *=v ; return true;
      case CVAL_VEC4: v4*=v4; return true;
      default       :         return false;
   }
}
Bool CalcValue::cube()
{
   switch(type)
   {
      case CVAL_INT : i *=i *i ; return true;
      case CVAL_REAL: r *=r *r ; return true;
      case CVAL_VEC2: v2*=v2*v2; return true;
      case CVAL_VEC : v *=v *v ; return true;
      case CVAL_VEC4: v4*=v4*v4; return true;
      default       :            return false;
   }
}
Bool CalcValue::sqrt()
{
   switch(type)
   {
      case CVAL_INT : r=Sqrt(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Sqrt(    r );                 return true;
      case CVAL_VEC2: v2.set(Sqrt(v2.x), Sqrt(v2.y)); return true;
      case CVAL_VEC : v .set(Sqrt(v .x), Sqrt(v .y), Sqrt(v .z)); return true;
      case CVAL_VEC4: v4.set(Sqrt(v4.x), Sqrt(v4.y), Sqrt(v4.z), Sqrt(v4.w)); return true;
      default       : return false;
   }
}
Bool CalcValue::cbrt()
{
   switch(type)
   {
      case CVAL_INT : r=Cbrt(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Cbrt(    r );                 return true;
      case CVAL_VEC2: v2.set(Cbrt(v2.x), Cbrt(v2.y)); return true;
      case CVAL_VEC : v .set(Cbrt(v .x), Cbrt(v .y), Cbrt(v .z)); return true;
      case CVAL_VEC4: v4.set(Cbrt(v4.x), Cbrt(v4.y), Cbrt(v4.z), Cbrt(v4.w)); return true;
      default       : return false;
   }
}
Bool CalcValue::pow(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r=Pow(Dbl(i), Dbl(x.i)); type=CVAL_REAL; return true;
         case CVAL_REAL: r=Pow(Dbl(i),     x.r ); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=Pow(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=Pow(r,     x.r ); return true;
      }break;
   }
   return false;
}
Bool CalcValue::ln()
{
   switch(type)
   {
      case CVAL_INT : r=Ln(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Ln(    r );                 return true;
   }
   return false;
}
Bool CalcValue::log2()
{
   switch(type)
   {
      case CVAL_INT : r=Log2(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Log2(    r );                 return true;
   }
   return false;
}
Bool CalcValue::log(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r=Log(Dbl(i), Dbl(x.i)); type=CVAL_REAL; return true;
         case CVAL_REAL: r=Log(Dbl(i),     x.r ); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=Log(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=Log(r,     x.r ); return true;
      }break;
   }
   return false;
}
/******************************************************************************/
Bool CalcValue::And(C CalcValue &x) {if(type==CVAL_INT && x.type==CVAL_INT){i &=x.i; return true;} return false;}
Bool CalcValue::Or (C CalcValue &x) {if(type==CVAL_INT && x.type==CVAL_INT){i |=x.i; return true;} return false;}
Bool CalcValue::Xor(C CalcValue &x) {if(type==CVAL_INT && x.type==CVAL_INT){i ^=x.i; return true;} return false;}
Bool CalcValue::shl(C CalcValue &x) {if(type==CVAL_INT && x.type==CVAL_INT){i<<=x.i; return true;} return false;}
Bool CalcValue::shr(C CalcValue &x) {if(type==CVAL_INT && x.type==CVAL_INT){i>>=x.i; return true;} return false;}
Bool CalcValue::rol(C CalcValue &x) {if(type==CVAL_INT && x.type==CVAL_INT){i=Rol(i, x.i); return true;} return false;}
Bool CalcValue::ror(C CalcValue &x) {if(type==CVAL_INT && x.type==CVAL_INT){i=Ror(i, x.i); return true;} return false;}
/******************************************************************************/
Bool CalcValue::equal(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i==x.i); return true;
         case CVAL_REAL: i=(i==x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r==x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r==x.r); type=CVAL_INT; return true;
      }break;

      case CVAL_VEC2: if(x.type==CVAL_VEC2){i=(v2==x.v2); type=CVAL_INT; return true;} break;
      case CVAL_VEC : if(x.type==CVAL_VEC ){i=(v ==x.v ); type=CVAL_INT; return true;} break;
      case CVAL_VEC4: if(x.type==CVAL_VEC4){i=(v4==x.v4); type=CVAL_INT; return true;} break;
   }
   return false;
}
Bool CalcValue::notEqual(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i!=x.i); return true;
         case CVAL_REAL: i=(i!=x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r!=x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r!=x.r); type=CVAL_INT; return true;
      }break;

      case CVAL_VEC2: if(x.type==CVAL_VEC2){i=(v2!=x.v2); type=CVAL_INT; return true;} break;
      case CVAL_VEC : if(x.type==CVAL_VEC ){i=(v !=x.v ); type=CVAL_INT; return true;} break;
      case CVAL_VEC4: if(x.type==CVAL_VEC4){i=(v4!=x.v4); type=CVAL_INT; return true;} break;
   }
   return false;
}
Bool CalcValue::equalGreater(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i>=x.i); return true;
         case CVAL_REAL: i=(i>=x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r>=x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r>=x.r); type=CVAL_INT; return true;
      }break;
   }
   return false;
}
Bool CalcValue::equalSmaller(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i<=x.i); return true;
         case CVAL_REAL: i=(i<=x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r<=x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r<=x.r); type=CVAL_INT; return true;
      }break;
   }
   return false;
}
Bool CalcValue::greater(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i>x.i); return true;
         case CVAL_REAL: i=(i>x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r>x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r>x.r); type=CVAL_INT; return true;
      }break;
   }
   return false;
}
Bool CalcValue::smaller(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i<x.i); return true;
         case CVAL_REAL: i=(i<x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r<x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r<x.r); type=CVAL_INT; return true;
      }break;
   }
   return false;
}
/******************************************************************************/
Bool CalcValue::andLogical(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i && x.i); return true;
         case CVAL_REAL: i=(i && x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r && x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r && x.r); type=CVAL_INT; return true;
      }break;
   }
   return false;
}
Bool CalcValue::orLogical(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=(i || x.i); return true;
         case CVAL_REAL: i=(i || x.r); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=(r || x.i); type=CVAL_INT; return true;
         case CVAL_REAL: i=(r || x.r); type=CVAL_INT; return true;
      }break;
   }
   return false;
}
Bool CalcValue::xorLogical(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : i=((i!=0) ^ (x.i!=0)); return true;
         case CVAL_REAL: i=((i!=0) ^ (x.r!=0)); return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : i=((r!=0) ^ (x.i!=0)); type=CVAL_INT; return true;
         case CVAL_REAL: i=((r!=0) ^ (x.r!=0)); type=CVAL_INT; return true;
      }break;
   }
   return false;
}
/******************************************************************************/
Bool CalcValue::trunc()
{
   switch(type)
   {
      case CVAL_INT : return true;
      case CVAL_REAL: i =TruncL(r ); type=CVAL_INT; return true;
      case CVAL_VEC2: v2=Trunc (v2); return true;
      case CVAL_VEC : v =Trunc (v ); return true;
      case CVAL_VEC4: v4=Trunc (v4); return true;
      default       : return false;
   }
}
Bool CalcValue::round()
{
   switch(type)
   {
      case CVAL_INT : return true;
      case CVAL_REAL: i =RoundL(r ); type=CVAL_INT; return true;
      case CVAL_VEC2: v2=Round (v2); return true;
      case CVAL_VEC : v =Round (v ); return true;
      case CVAL_VEC4: v4=Round (v4); return true;
      default       : return false;
   }
}
Bool CalcValue::floor()
{
   switch(type)
   {
      case CVAL_INT : return true;
      case CVAL_REAL: i =FloorL(r ); type=CVAL_INT; return true;
      case CVAL_VEC2: v2=Floor (v2); return true;
      case CVAL_VEC : v =Floor (v ); return true;
      case CVAL_VEC4: v4=Floor (v4); return true;
      default       : return false;
   }
}
Bool CalcValue::ceil()
{
   switch(type)
   {
      case CVAL_INT : return true;
      case CVAL_REAL: i =CeilL(r ); type=CVAL_INT; return true;
      case CVAL_VEC2: v2=Ceil (v2); return true;
      case CVAL_VEC : v =Ceil (v ); return true;
      case CVAL_VEC4: v4=Ceil (v4); return true;
      default       : return false;
   }
}
Bool CalcValue::frac()
{
   switch(type)
   {
      case CVAL_INT : i=0; return true;
      case CVAL_REAL: r =Frac(r ); return true;
      case CVAL_VEC2: v2=Frac(v2); return true;
      case CVAL_VEC : v =Frac(v ); return true;
      case CVAL_VEC4: v4=Frac(v4); return true;
      default       : return false;
   }
}
Bool CalcValue::alignTrunc(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r=AlignTrunc(Dbl(i), Dbl(x.i)); type=CVAL_REAL; return true;
         case CVAL_REAL: r=AlignTrunc(Dbl(i),     x.r ); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=AlignTrunc(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=AlignTrunc(r,     x.r ); return true;
      }break;
   }
   return false;
}
Bool CalcValue::alignRound(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r=AlignRound(Dbl(i), Dbl(x.i)); type=CVAL_REAL; return true;
         case CVAL_REAL: r=AlignRound(Dbl(i),     x.r ); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=AlignRound(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=AlignRound(r,     x.r ); return true;
      }break;
   }
   return false;
}
Bool CalcValue::alignFloor(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r=AlignFloor(Dbl(i), Dbl(x.i)); type=CVAL_REAL; return true;
         case CVAL_REAL: r=AlignFloor(Dbl(i),     x.r ); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=AlignFloor(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=AlignFloor(r,     x.r ); return true;
      }break;
   }
   return false;
}
Bool CalcValue::alignCeil(C CalcValue &x)
{
   switch(type)
   {
      case CVAL_INT: switch(x.type)
      {
         case CVAL_INT : r=AlignCeil(Dbl(i), Dbl(x.i)); type=CVAL_REAL; return true;
         case CVAL_REAL: r=AlignCeil(Dbl(i),     x.r ); type=CVAL_REAL; return true;
      }break;

      case CVAL_REAL: switch(x.type)
      {
         case CVAL_INT : r=AlignCeil(r, Dbl(x.i)); return true;
         case CVAL_REAL: r=AlignCeil(r,     x.r ); return true;
      }break;
   }
   return false;
}
/******************************************************************************/
Bool CalcValue::sin()
{
   switch(type)
   {
      case CVAL_INT : r=Sin(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Sin(    r );                 return true;
   }
   return false;
}
Bool CalcValue::cos()
{
   switch(type)
   {
      case CVAL_INT : r=Cos(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Cos(    r );                 return true;
   }
   return false;
}
Bool CalcValue::tan()
{
   switch(type)
   {
      case CVAL_INT : r=Tan(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Tan(    r );                 return true;
   }
   return false;
}
Bool CalcValue::ctg()
{
   switch(type)
   {
      case CVAL_INT : r=Ctg(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Ctg(    r );                 return true;
   }
   return false;
}
Bool CalcValue::acos()
{
   switch(type)
   {
      case CVAL_INT : r=Acos(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Acos(    r );                 return true;
   }
   return false;
}
Bool CalcValue::asin()
{
   switch(type)
   {
      case CVAL_INT : r=Asin(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Asin(    r );                 return true;
   }
   return false;
}
Bool CalcValue::atan()
{
   switch(type)
   {
      case CVAL_INT : r=Atan(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=Atan(    r );                 return true;
   }
   return false;
}
Bool CalcValue::degToRad()
{
   switch(type)
   {
      case CVAL_INT : r=DegToRad(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=DegToRad(    r );                 return true;
   }
   return false;
}
Bool CalcValue::radToDeg()
{
   switch(type)
   {
      case CVAL_INT : r=RadToDeg(Dbl(i)); type=CVAL_REAL; return true;
      case CVAL_REAL: r=RadToDeg(    r );                 return true;
   }
   return false;
}
/******************************************************************************/
Bool CalcValue::length()
{
   switch(type)
   {
      case CVAL_VEC2: r=v2.length(); type=CVAL_REAL; return true;
      case CVAL_VEC : r=v .length(); type=CVAL_REAL; return true;
      case CVAL_VEC4: r=v4.length(); type=CVAL_REAL; return true;
   }
   return false;
}
Bool CalcValue::length2()
{
   switch(type)
   {
      case CVAL_VEC2: r=v2.length2(); type=CVAL_REAL; return true;
      case CVAL_VEC : r=v .length2(); type=CVAL_REAL; return true;
      case CVAL_VEC4: r=v4.length2(); type=CVAL_REAL; return true;
   }
   return false;
}
Bool CalcValue::dot(C CalcValue &x)
{
   if(type==x.type)switch(type)
   {
      case CVAL_VEC2: r=Dot(v2, x.v2); type=CVAL_REAL; return true;
      case CVAL_VEC : r=Dot(v , x.v ); type=CVAL_REAL; return true;
      case CVAL_VEC4: r=Dot(v4, x.v4); type=CVAL_REAL; return true;
   }
   return false;
}
Bool CalcValue::cross(C CalcValue &x)
{
   if(type==CVAL_VEC && x.type==CVAL_VEC){v=Cross(v, x.v); return true;}
   return false;
}
Bool CalcValue::perp()
{
   switch(type)
   {
      case CVAL_VEC2: v2=Perp(v2); return true;
      case CVAL_VEC : v =Perp(v ); return true;
   }
   return false;
}
/******************************************************************************/
static Bool CLogicalNot(CalcValue &x) {return x.notLogical() ? true : E("!"   , x);}
static Bool CBitwiseNot(CalcValue &x) {return x.notBitwise() ? true : E("~"   , x);}
static Bool CChs       (CalcValue &x) {return x.chs       () ? true : E("-"   , x);}
static Bool CAbs       (CalcValue &x) {return x.abs       () ? true : E("Abs" , x);}
static Bool CSign      (CalcValue &x) {return x.sign      () ? true : E("Sign", x);}

static Bool CSat(CalcValue &x              ) {return x.sat( ) ? true : E("Sat", x   );}
static Bool CMin(CalcValue &x, CalcValue &y) {return x.min(y) ? true : E("Min", x, y);}
static Bool CMax(CalcValue &x, CalcValue &y) {return x.max(y) ? true : E("Max", x, y);}
static Bool CAvg(CalcValue &x, CalcValue &y) {return x.avg(y) ? true : E("Avg", x, y);}

static Bool CAdd(CalcValue &x, CalcValue &y) {return x.add(y) ? true : E("+", x, y);}
static Bool CSub(CalcValue &x, CalcValue &y) {return x.sub(y) ? true : E("-", x, y);}
static Bool CMul(CalcValue &x, CalcValue &y) {return x.mul(y) ? true : E("*", x, y);}
static Bool CDiv(CalcValue &x, CalcValue &y) {return x.div(y) ? true : E("/", x, y);}
static Bool CMod(CalcValue &x, CalcValue &y) {return x.mod(y) ? true : E("%", x, y);}

static Bool CSqr (CalcValue &x              ) {return x.sqr ( ) ? true : E("Sqr" , x   );}
static Bool CCube(CalcValue &x              ) {return x.cube( ) ? true : E("Cube", x   );}
static Bool CSqrt(CalcValue &x              ) {return x.sqrt( ) ? true : E("Sqrt", x   );}
static Bool CCbrt(CalcValue &x              ) {return x.cbrt( ) ? true : E("Cbrt", x   );}
static Bool CPow (CalcValue &x, CalcValue &y) {return x.pow (y) ? true : E("Pow" , x, y);}
static Bool CLn  (CalcValue &x              ) {return x.ln  ( ) ? true : E("Ln"  , x   );}
static Bool CLog2(CalcValue &x              ) {return x.log2( ) ? true : E("Log2", x   );}
static Bool CLog (CalcValue &x, CalcValue &y) {return x.log (y) ? true : E("Log" , x, y);}

static Bool CAnd(CalcValue &x, CalcValue &y) {return x.And(y) ? true : E("&" , x, y);}
static Bool COr (CalcValue &x, CalcValue &y) {return x.Or (y) ? true : E("|" , x, y);}
static Bool CXor(CalcValue &x, CalcValue &y) {return x.Xor(y) ? true : E("^" , x, y);}
static Bool CShl(CalcValue &x, CalcValue &y) {return x.shl(y) ? true : E("<<", x, y);}
static Bool CShr(CalcValue &x, CalcValue &y) {return x.shr(y) ? true : E(">>", x, y);}
static Bool CRol(CalcValue &x, CalcValue &y) {return x.rol(y) ? true : E("Rol", x, y);}
static Bool CRor(CalcValue &x, CalcValue &y) {return x.ror(y) ? true : E("Ror", x, y);}

static Bool CEqual(CalcValue &x, CalcValue &y) {return x.     equal  (y) ? true : E("==", x, y);}
static Bool CNequ (CalcValue &x, CalcValue &y) {return x.  notEqual  (y) ? true : E("!=", x, y);}
static Bool CGr   (CalcValue &x, CalcValue &y) {return x.     greater(y) ? true : E(">" , x, y);}
static Bool CSm   (CalcValue &x, CalcValue &y) {return x.     smaller(y) ? true : E("<" , x, y);}
static Bool CEgr  (CalcValue &x, CalcValue &y) {return x.equalGreater(y) ? true : E(">=", x, y);}
static Bool CEsm  (CalcValue &x, CalcValue &y) {return x.equalSmaller(y) ? true : E("<=", x, y);}

static Bool CLogicalAnd(CalcValue &x, CalcValue &y) {return x.andLogical(y) ? true : E("&&", x, y);}
static Bool CLogicalOr (CalcValue &x, CalcValue &y) {return x. orLogical(y) ? true : E("||", x, y);}
static Bool CLogicalXor(CalcValue &x, CalcValue &y) {return x.xorLogical(y) ? true : E("^^", x, y);}

static Bool CTrunc (CalcValue &x              ) {return x.trunc     ( ) ? true : E("Trunc"     , x);}
static Bool CRound (CalcValue &x              ) {return x.round     ( ) ? true : E("Round"     , x);}
static Bool CFloor (CalcValue &x              ) {return x.floor     ( ) ? true : E("Floor"     , x);}
static Bool CCeil  (CalcValue &x              ) {return x.ceil      ( ) ? true : E("Ceil"      , x);}
static Bool CFrac  (CalcValue &x              ) {return x.frac      ( ) ? true : E("Frac"      , x);}
static Bool CAlignT(CalcValue &x, CalcValue &y) {return x.alignTrunc(y) ? true : E("AlignTrunc", x, y);}
static Bool CAlignR(CalcValue &x, CalcValue &y) {return x.alignRound(y) ? true : E("AlignRound", x, y);}
static Bool CAlignF(CalcValue &x, CalcValue &y) {return x.alignFloor(y) ? true : E("AlignFloor", x, y);}
static Bool CAlignC(CalcValue &x, CalcValue &y) {return x.alignCeil (y) ? true : E("AlignCeil" , x, y);}

static Bool CSin     (CalcValue &x) {return x.sin     () ? true : E("Sin"     , x);}
static Bool CCos     (CalcValue &x) {return x.cos     () ? true : E("Cos"     , x);}
static Bool CTan     (CalcValue &x) {return x.tan     () ? true : E("Tan"     , x);}
static Bool CCtg     (CalcValue &x) {return x.ctg     () ? true : E("Ctg"     , x);}
static Bool CAcos    (CalcValue &x) {return x.acos    () ? true : E("Acos"    , x);}
static Bool CAsin    (CalcValue &x) {return x.asin    () ? true : E("Asin"    , x);}
static Bool CAtan    (CalcValue &x) {return x.atan    () ? true : E("Atan"    , x);}
static Bool CDegToRad(CalcValue &x) {return x.degToRad() ? true : E("DegToRad", x);}
static Bool CRadToDeg(CalcValue &x) {return x.radToDeg() ? true : E("RadToDeg", x);}

static Bool CLength (CalcValue &x              ) {return x.length ( ) ? true : E("Length" , x   );}
static Bool CLength2(CalcValue &x              ) {return x.length2( ) ? true : E("Length2", x   );}
static Bool CDot    (CalcValue &x, CalcValue &y) {return x.dot    (y) ? true : E("Dot"    , x, y);}
static Bool CCross  (CalcValue &x, CalcValue &y) {return x.cross  (y) ? true : E("Cross"  , x, y);}
static Bool CPerp   (CalcValue &x              ) {return x.perp   ( ) ? true : E("Perp"   , x   );}

static Bool CAngle(CalcValue &x, CalcValue &y)
{
   Dbl xr, yr;
   if(x.type==CVAL_INT )xr=x.i;else
   if(x.type==CVAL_REAL)xr=x.r;else return E("Angle", x, y);
   if(y.type==CVAL_INT )yr=y.i;else
   if(y.type==CVAL_REAL)yr=y.r;else return E("Angle", x, y);
   x.r   =Angle(VecD2(xr, yr));
   x.type=CVAL_REAL; return true;
}
static Bool CPinch(CalcValue &x, CalcValue &y)
{
   Dbl step, pinch;
   if(x.type==CVAL_INT )step =x.i;else
   if(x.type==CVAL_REAL)step =x.r;else return E("Pinch", x, y);
   if(y.type==CVAL_INT )pinch=y.i;else
   if(y.type==CVAL_REAL)pinch=y.r;else return E("Pinch", x, y);
   x.r   =Pinch(step, pinch);
   x.type=CVAL_REAL; return true;
}

static Bool CSmoothSqr(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=SmoothSqr(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=SmoothSqr(    x.r );else return E("SmoothSqr", x);
   x.type=CVAL_REAL; return true;
}
static Bool CSmoothCube(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=SmoothCube(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=SmoothCube(    x.r );else return E("SmoothCube", x);
   x.type=CVAL_REAL; return true;
}
static Bool CSmoothCubeInv(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=SmoothCubeInv(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=SmoothCubeInv(    x.r );else return E("CSmoothCubeInv", x);
   x.type=CVAL_REAL; return true;
}
static Bool CSmoothCube2(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=SmoothCube2(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=SmoothCube2(    x.r );else return E("SmoothCube2", x);
   x.type=CVAL_REAL; return true;
}
static Bool CSmoothSextic(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=SmoothSextic(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=SmoothSextic(    x.r );else return E("SmoothSextic", x);
   x.type=CVAL_REAL; return true;
}
static Bool CSmoothSin(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=SmoothSin(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=SmoothSin(    x.r );else return E("SmoothSin", x);
   x.type=CVAL_REAL; return true;
}
static Bool CSmoothPow(CalcValue &x, CalcValue &y)
{
   Dbl step, pow;
   if(x.type==CVAL_INT )step=x.i;else
   if(x.type==CVAL_REAL)step=x.r;else return E("SmoothPow", x, y);
   if(y.type==CVAL_INT )pow =y.i;else
   if(y.type==CVAL_REAL)pow =y.r;else return E("SmoothPow", x, y);
   x.r   =SmoothPow(step, pow);
   x.type=CVAL_REAL; return true;
}
static Bool CSmoothPinch(CalcValue &x, CalcValue &y)
{
   Dbl step, pinch;
   if(x.type==CVAL_INT )step =x.i;else
   if(x.type==CVAL_REAL)step =x.r;else return E("SmoothPinch", x, y);
   if(y.type==CVAL_INT )pinch=y.i;else
   if(y.type==CVAL_REAL)pinch=y.r;else return E("SmoothPinch", x, y);
   x.r   =SmoothPinch(step, pinch);
   x.type=CVAL_REAL; return true;
}

static Bool CVec2(CalcValue &x, CalcValue &y)
{
   if(x.type==CVAL_INT )x.v2.x=x.i;else
   if(x.type==CVAL_REAL)x.v2.x=x.r;else return E("Vec2", x, y);
   if(y.type==CVAL_INT )x.v2.y=y.i;else
   if(y.type==CVAL_REAL)x.v2.y=y.r;else return E("Vec2", x, y);
   x.type=CVAL_VEC2; return true;
}
static Bool CVec(CalcValue &x, CalcValue &y, CalcValue &z)
{
   if(x.type==CVAL_INT )x.v.x=x.i;else
   if(x.type==CVAL_REAL)x.v.x=x.r;else return E("Vec", x, y, z);
   if(y.type==CVAL_INT )x.v.y=y.i;else
   if(y.type==CVAL_REAL)x.v.y=y.r;else return E("Vec", x, y, z);
   if(z.type==CVAL_INT )x.v.z=z.i;else
   if(z.type==CVAL_REAL)x.v.z=z.r;else return E("Vec", x, y, z);
   x.type=CVAL_VEC; return true;
}
static Bool CVec4(CalcValue &x, CalcValue &y, CalcValue &z, CalcValue &w)
{
   if(x.type==CVAL_INT )x.v4.x=x.i;else
   if(x.type==CVAL_REAL)x.v4.x=x.r;else return E("Vec4", x, y, z, w);
   if(y.type==CVAL_INT )x.v4.y=y.i;else
   if(y.type==CVAL_REAL)x.v4.y=y.r;else return E("Vec4", x, y, z, w);
   if(z.type==CVAL_INT )x.v4.z=z.i;else
   if(z.type==CVAL_REAL)x.v4.z=z.r;else return E("Vec4", x, y, z, w);
   if(w.type==CVAL_INT )x.v4.w=w.i;else
   if(w.type==CVAL_REAL)x.v4.w=w.r;else return E("Vec4", x, y, z, w);
   x.type=CVAL_VEC4; return true;
}

static Bool CLerp(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(z.type==CVAL_INT )s=z.i;else
   if(z.type==CVAL_REAL)s=z.r;else return E("Lerp", x, y, z);

   if(x.type==CVAL_VEC2 && y.type==CVAL_VEC2){x.v2=Lerp(x.v2, y.v2, s); return true;}
   if(x.type==CVAL_VEC  && y.type==CVAL_VEC ){x.v =Lerp(x.v , y.v , s); return true;}
   if(x.type==CVAL_VEC4 && y.type==CVAL_VEC4){x.v4=Lerp(x.v4, y.v4, s); return true;}

   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("Lerp", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("Lerp", x, y, z);

   x.r   =Lerp(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpR(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpR", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpR", x, y, z);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpR", x, y, z);

   x.r   =LerpR(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpRS(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpRS", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpRS", x, y, z);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpRS", x, y, z);

   x.r   =LerpRS(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpSmoothSqr(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpSmoothSqr", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpSmoothSqr", x, y, z);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpSmoothSqr", x, y, z);

   x.r   =LerpSmoothSqr(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpSmoothCube(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpSmoothCube", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpSmoothCube", x, y, z);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpSmoothCube", x, y, z);

   x.r   =LerpSmoothCube(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpSmoothCube2(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpSmoothCube2", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpSmoothCube2", x, y, z);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpSmoothCube2", x, y, z);

   x.r   =LerpSmoothCube2(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpSmoothSextic(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpSmoothSextic", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpSmoothSextic", x, y, z);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpSmoothSextic", x, y, z);

   x.r   =LerpSmoothSextic(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpSmoothSin(CalcValue &x, CalcValue &y, CalcValue &z)
{
   Dbl from, to, s;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpSmoothSin", x, y, z);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpSmoothSin", x, y, z);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpSmoothSin", x, y, z);

   x.r   =LerpSmoothSin(from, to, s);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpSmoothPow(CalcValue &x, CalcValue &y, CalcValue &z, CalcValue &w)
{
   Dbl from, to, s, p;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpSmoothPow", x, y, z, w);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpSmoothPow", x, y, z, w);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpSmoothPow", x, y, z, w);
   if(w.type==CVAL_INT )p   =w.i;else
   if(w.type==CVAL_REAL)p   =w.r;else return E("LerpSmoothPow", x, y, z, w);

   x.r   =LerpSmoothPow(from, to, s, p);
   x.type=CVAL_REAL; return true;
}
static Bool CLerpSmoothPinch(CalcValue &x, CalcValue &y, CalcValue &z, CalcValue &w)
{
   Dbl from, to, s, p;
   if(x.type==CVAL_INT )from=x.i;else
   if(x.type==CVAL_REAL)from=x.r;else return E("LerpSmoothPinch", x, y, z, w);
   if(y.type==CVAL_INT )to  =y.i;else
   if(y.type==CVAL_REAL)to  =y.r;else return E("LerpSmoothPinch", x, y, z, w);
   if(z.type==CVAL_INT )s   =z.i;else
   if(z.type==CVAL_REAL)s   =z.r;else return E("LerpSmoothPinch", x, y, z, w);
   if(w.type==CVAL_INT )p   =w.i;else
   if(w.type==CVAL_REAL)p   =w.r;else return E("LerpSmoothPinch", x, y, z, w);

   x.r   =LerpSmoothPinch(from, to, s, p);
   x.type=CVAL_REAL; return true;
}

static Bool CScaleFactor(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=ScaleFactor(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=ScaleFactor(    x.r );else return E("ScaleFactor", x);
   x.type=CVAL_REAL; return true;
}
static Bool CScaleFactorR(CalcValue &x)
{
   if(x.type==CVAL_INT )x.r=ScaleFactorR(Dbl(x.i));else
   if(x.type==CVAL_REAL)x.r=ScaleFactorR(    x.r );else return E("ScaleFactorR", x);
   x.type=CVAL_REAL; return true;
}
/******************************************************************************/
static struct CalcOpInfo
{
   Byte    priority, args;
   CChar8 *t;
   Ptr     func;
}C CalcOpInfos[]=
{
   #define COT_LOGICAL_NOT 0
   {0, 1, "!", (Ptr)CLogicalNot},
   #define COT_BITWISE_NOT 1
   {0, 1, "~", (Ptr)CBitwiseNot},
   #define COT_ADD   2
   {3, 2, "+", (Ptr)CAdd},
   #define COT_SUB   3
   {3, 2, "-", (Ptr)CSub},
   #define COT_MUL   4
   {2, 2, "*", (Ptr)CMul},
   #define COT_POW   5
   {1, 2, "**", (Ptr)CPow},
   #define COT_DIV   6
   {2, 2, "/", (Ptr)CDiv},
   #define COT_MOD   7
   {2, 2, "%", (Ptr)CMod},
   #define COT_AND   8
   {4, 2, "&", (Ptr)CAnd},
   #define COT_OR    9
   {4, 2, "|", (Ptr)COr},
   #define COT_XOR  10
   {4, 2, "^", (Ptr)CXor},
   #define COT_SHL  11
   {5, 2, "<<", (Ptr)CShl},
   #define COT_SHR  12
   {5, 2, ">>", (Ptr)CShr},
   #define COT_EQU  13
   {6, 2, "==", (Ptr)CEqual},
   #define COT_NEQ  14
   {6, 2, "!=", (Ptr)CNequ},
   #define COT_GR   15
   {6, 2, ">", (Ptr)CGr},
   #define COT_SM   16
   {6, 2, "<", (Ptr)CSm},
   #define COT_EGR  17
   {6, 2, ">=", (Ptr)CEgr},
   #define COT_ESM  18
   {6, 2, "<=", (Ptr)CEsm},
   #define COT_LOGICAL_AND 19
   {7, 2, "&&", (Ptr)CLogicalAnd},
   #define COT_LOGICAL_OR  20
   {7, 2, "||", (Ptr)CLogicalOr},
   #define COT_LOGICAL_XOR 21
   {7, 2, "^^", (Ptr)CLogicalXor},
   #define COT_COMMA 22
   {8, 0, ",", null},
   #define COT_LBR  23
   {9, 0, "(", null},
   #define COT_RBR  24
   {9, 0, ")", null},
};
struct CalcOp
{
   UInt type;
};
/******************************************************************************/
static struct CalcFuncInfo
{
   Byte    args;
   CChar8 *t;
   Ptr     func;
}C CalcFuncInfos[]=
{
   {1, "Abs"             , (Ptr)CAbs             },
   {1, "Sign"            , (Ptr)CSign            },
   {1, "Sat"             , (Ptr)CSat             },
   {2, "Min"             , (Ptr)CMin             },
   {2, "Max"             , (Ptr)CMax             },
   {2, "Avg"             , (Ptr)CAvg             },
   {1, "Trunc"           , (Ptr)CTrunc           },
   {1, "Round"           , (Ptr)CRound           },
   {1, "Floor"           , (Ptr)CFloor           },
   {1, "Ceil"            , (Ptr)CCeil            },
   {1, "Frac"            , (Ptr)CFrac            },
   {2, "AlignTrunc"      , (Ptr)CAlignT          },
   {2, "AlignRound"      , (Ptr)CAlignR          },
   {2, "AlignFloor"      , (Ptr)CAlignF          },
   {2, "AlignCeil"       , (Ptr)CAlignC          },
   {1, "Ln"              , (Ptr)CLn              },
   {1, "Log2"            , (Ptr)CLog2            },
   {2, "Log"             , (Ptr)CLog             },
   {1, "Sin"             , (Ptr)CSin             },
   {1, "Cos"             , (Ptr)CCos             },
   {1, "Tan"             , (Ptr)CTan             },
   {1, "Ctg"             , (Ptr)CCtg             },
   {1, "Acos"            , (Ptr)CAcos            },
   {1, "Asin"            , (Ptr)CAsin            },
   {1, "Atan"            , (Ptr)CAtan            },
   {1, "DegToRad"        , (Ptr)CDegToRad        },
   {1, "RadToDeg"        , (Ptr)CRadToDeg        },
   {2, "Angle"           , (Ptr)CAngle           },
   {1, "Sqr"             , (Ptr)CSqr             },
   {1, "Cube"            , (Ptr)CCube            },
   {1, "Sqrt"            , (Ptr)CSqrt            },
   {1, "Cbrt"            , (Ptr)CCbrt            },
   {2, "Pow"             , (Ptr)CPow             },
   {2, "Pinch"           , (Ptr)CPinch           },
   {1, "SmoothSqr"       , (Ptr)CSmoothSqr       },
   {1, "SmoothCube"      , (Ptr)CSmoothCube      },
   {1, "SmoothCubeInv"   , (Ptr)CSmoothCubeInv   },
   {1, "SmoothCube2"     , (Ptr)CSmoothCube2     },
   {1, "SmoothSextic"    , (Ptr)CSmoothSextic    },
   {1, "SmoothSin"       , (Ptr)CSmoothSin       },
   {2, "SmoothPow"       , (Ptr)CSmoothPow       },
   {2, "SmoothPinch"     , (Ptr)CSmoothPinch     },
   {2, "Vec2"            , (Ptr)CVec2            },
   {3, "Vec"             , (Ptr)CVec             },
   {4, "Vec4"            , (Ptr)CVec4            },
   {1, "Len"             , (Ptr)CLength          },
   {1, "Length"          , (Ptr)CLength          },
   {1, "Len2"            , (Ptr)CLength2         },
   {1, "Length2"         , (Ptr)CLength2         },
   {2, "Dot"             , (Ptr)CDot             },
   {2, "Cross"           , (Ptr)CCross           },
   {1, "Perp"            , (Ptr)CPerp            },
   {3, "Lerp"            , (Ptr)CLerp            },
   {3, "LerpR"           , (Ptr)CLerpR           },
   {3, "LerpRS"          , (Ptr)CLerpRS          },
   {3, "LerpSmoothSqr"   , (Ptr)CLerpSmoothSqr   },
   {3, "LerpSmoothCube"  , (Ptr)CLerpSmoothCube  },
   {3, "LerpSmoothCube2" , (Ptr)CLerpSmoothCube2 },
   {3, "LerpSmoothSextic", (Ptr)CLerpSmoothSextic},
   {3, "LerpSmoothSin"   , (Ptr)CLerpSmoothSin   },
   {4, "LerpSmoothPow"   , (Ptr)CLerpSmoothPow   },
   {4, "LerpSmoothPinch" , (Ptr)CLerpSmoothPinch },
   {1, "ScaleFactor"     , (Ptr)CScaleFactor     },
   {1, "ScaleFactorR"    , (Ptr)CScaleFactorR    },
   {2, "Rol"             , (Ptr)CRol             },
   {2, "Ror"             , (Ptr)CRor             },
};
struct CalcFunc
{
 C CalcFuncInfo *funi;
};
/******************************************************************************/
struct CalcElm
{
   CALC_ELM_TYPE type;
   union
   {
      CalcOp    o;
      CalcValue v;
      CalcFunc  f;
   };

   CalcElm() {} // needed because of union
};
/******************************************************************************/
static Bool ReadChar(Meml<CalcElm> &elms, CChar* &text, C MemPtr<CalcVar> &vars)
{
   Char name[256];
   for(Int i=0; ; )
   {
      Char c=*text; if(!(CharFlag(c)&(CHARF_ALPHA|CHARF_DIG|CHARF_UNDER)) && c!='.'){name[i]='\0'; break;}
      name[i++]=c; text++;
      if(!InRange(i, name))return false;
   }

   if(Equal(name, "pi"))
   {
      CalcElm &elm=elms.New();
      elm.type  =CET_VAL;
      elm.v.type=CVAL_REAL;
      elm.v.r   =PID;
      return true;
   }else
   if(Equal(name, "e"))
   {
      CalcElm &elm=elms.New();
      elm.type  =CET_VAL;
      elm.v.type=CVAL_REAL;
      elm.v.r   =2.7182818284590452353602874713527;
      return true;
   }
   REPA(CalcFuncInfos)if(Equal(name, CalcFuncInfos[i].t))
   {
      CalcElm &elm=elms.New();
      elm.type  =CET_FUNC;
      elm.f.funi=&CalcFuncInfos[i];
      return true;
   }
   REPA(vars)if(Equal(name, vars[i].name))
   {
      CalcElm &elm=elms.New();
      elm.type=CET_VAL;
      elm.v   =vars[i].value;
      return true;
   }
   return E(name);
}
/******************************************************************************/
static void AddOp(Meml<CalcElm> &elms, UInt op)
{
   CalcElm &elm=elms.New();
   elm.type  =CET_OP;
   elm.o.type=op;
}
static Bool ReadOp(Meml<CalcElm> &elms, CChar* &text)
{
   Char c=text[1];
   switch(text[0])
   {
      case '+': AddOp(elms, COT_ADD        ); break;
      case '-': AddOp(elms, COT_SUB        ); break;
      case '/': AddOp(elms, COT_DIV        ); break;
      case '%': AddOp(elms, COT_MOD        ); break;
      case '~': AddOp(elms, COT_BITWISE_NOT); break;
      case ',': AddOp(elms, COT_COMMA      ); break;
      case '(': AddOp(elms, COT_LBR        ); break;
      case ')': AddOp(elms, COT_RBR        ); break;
      case '=': if(c=='='){AddOp(elms, COT_EQU        ); text++;} break;
      case '!': if(c=='='){AddOp(elms, COT_NEQ        ); text++;}else AddOp(elms, COT_LOGICAL_NOT); break;
      case '*': if(c=='*'){AddOp(elms, COT_POW        ); text++;}else AddOp(elms, COT_MUL); break;
      case '&': if(c=='&'){AddOp(elms, COT_LOGICAL_AND); text++;}else AddOp(elms, COT_AND); break;
      case '|': if(c=='|'){AddOp(elms, COT_LOGICAL_OR ); text++;}else AddOp(elms, COT_OR ); break;
      case '^': if(c=='^'){AddOp(elms, COT_LOGICAL_XOR); text++;}else AddOp(elms, COT_XOR); break;
      case '<': if(c=='<'){AddOp(elms, COT_SHL        ); text++;}else if(c=='='){AddOp(elms, COT_ESM); text++;}else AddOp(elms, COT_SM); break;
      case '>': if(c=='>'){AddOp(elms, COT_SHR        ); text++;}else if(c=='='){AddOp(elms, COT_EGR); text++;}else AddOp(elms, COT_GR); break;
      default : return E(text);
   }
   text++; return true;
}
/******************************************************************************/
static CalcElm* Celm(MemlNode *node) {return node ? (CalcElm*)node->data() : null;}
/******************************************************************************/
static Bool CFuncReady(Meml<CalcElm> &elms, MemlNode *node, Int args)
{
   CalcElm *c;
   if(args<1 || args>4 || !node)return false;

      if(!(c=Celm(node=node->next())))return false; if(c->type!=CET_OP || c->o.type!=COT_LBR  )return false; // (
      if(!(c=Celm(node=node->next())))return false; if(c->type!=CET_VAL                       )return false; // x
   for(; --args; )
   {
      if(!(c=Celm(node=node->next())))return false; if(c->type!=CET_OP || c->o.type!=COT_COMMA)return false; // ,
      if(!(c=Celm(node=node->next())))return false; if(c->type!=CET_VAL                       )return false; // y
   }
      if(!(c=Celm(node=node->next())))return false; if(c->type!=CET_OP || c->o.type!=COT_RBR  )return false; // )

   return true;
}
/******************************************************************************/
Bool Calculate(CalcValue &value, C Str &expression, C MemPtr<CalcVar> &vars)
{
   CalcError[0]=0;
   value.type=CVAL_NONE;

   Meml<CalcElm> elms;

   if(CChar *text=expression)for(;;)
   {
      Char c =*text; if(!c)break; if(c==' '){text++; continue;}
      UInt cf=CharFlag(c);
      if(  cf& CHARF_DIG || c=='.'      ){CalcElm &elm=elms.New(); elm.type=CET_VAL; text=TextValue(text, elm.v); if(!elm.v.type)goto error;}else
      if(!(cf&(CHARF_ALPHA|CHARF_UNDER))){if(!ReadOp  (elms, text      ))goto error;}else
                                         {if(!ReadChar(elms, text, vars))goto error;}
   }

   for(Int loop=true; loop--; )MFREP(elms)
   {
      #define NEXT(next) {i=next; loop=true; continue;}
      MemlNode * next=i->next(),
               * prev=i->prev(),
               *nnext=(next ? next->next() : null),
               *pprev=(prev ? prev->prev() : null);
      CalcElm  * c=Celm(    i),
               * n=Celm( next),
               * p=Celm( prev),
               *nn=Celm(nnext),
               *pp=Celm(pprev);
      UInt cet=( c ?  c->type : CET_NONE),
           net=( n ?  n->type : CET_NONE),
           pet=( p ?  p->type : CET_NONE),
          nnet=(nn ? nn->type : CET_NONE),
          ppet=(pp ? pp->type : CET_NONE);

      if(cet==CET_OP && net==CET_VAL)
      {
         UInt        cot=c->o.type;
       C CalcOpInfo &opi=CalcOpInfos[cot];
         if(opi.args==1)
         {
            if(!((Bool(*)(CalcValue&))opi.func)(n->v))goto error;
            elms.remove(i);
            NEXT(next);
         }

         if(opi.args==2 && pet==CET_VAL)
            if(!ppet || (ppet==CET_OP && opi.priority< CalcOpInfos[pp->o.type].priority))
            if(!nnet || (nnet==CET_OP && opi.priority<=CalcOpInfos[nn->o.type].priority))
         {
            if(!((Bool(*)(CalcValue&, CalcValue&))opi.func)(p->v, n->v))goto error;
            elms.remove(i   );
            elms.remove(next);
            NEXT(prev);
         }

         if(cot==COT_ADD)if(!pet || (pet==CET_OP && p->o.type!=COT_RBR)){            elms.remove(i);                     NEXT(next);}
         if(cot==COT_SUB)if(!pet || (pet==CET_OP && p->o.type!=COT_RBR)){CChs(n->v); elms.remove(i);                     NEXT(next);}
         if(cot==COT_LBR)if(nnet==CET_OP && nn->o.type==COT_RBR && pet!=CET_FUNC){   elms.remove(i); elms.remove(nnext); NEXT(next);}
      }else
      if(cet==CET_FUNC)
      {
       C CalcFuncInfo &funi=*c->f.funi;
         if(CFuncReady(elms, i, funi.args))
         {
            if(funi.args==1){if(!((Bool(*)(CalcValue&                                    ))funi.func)(nn->v))goto error;}else
            if(funi.args==2){if(!((Bool(*)(CalcValue&, CalcValue&                        ))funi.func)(nn->v, Celm(nnext->next()->next())->v))goto error;}else
            if(funi.args==3){if(!((Bool(*)(CalcValue&, CalcValue&, CalcValue&            ))funi.func)(nn->v, Celm(nnext->next()->next())->v, Celm(nnext->next()->next()->next()->next())->v))goto error;}else
            if(funi.args==4){if(!((Bool(*)(CalcValue&, CalcValue&, CalcValue&, CalcValue&))funi.func)(nn->v, Celm(nnext->next()->next())->v, Celm(nnext->next()->next()->next()->next())->v, Celm(nnext->next()->next()->next()->next()->next()->next())->v))goto error;}
            elms.remove(i   ); // func
            elms.remove(next); // (
            i=nnext->next(); REPD(j, 2*(funi.args-1)){next=i->next(); elms.remove(i); i=next;} // ,y
            elms.remove(i   ); // )
            NEXT(nnext);
         }
      }
   }

   if(elms.elms()==1)
   {
      CalcElm &elm=elms[elms.first()];
      if(elm.type==CET_VAL)value=elm.v;
   }
   return true;
error:
   return false;
}
/******************************************************************************/
Int   CalculateI (C Str &expression, C MemPtr<CalcVar> &vars) {CalcValue value; Calculate(value, expression, vars); return value.asInt  ();}
Long  CalculateL (C Str &expression, C MemPtr<CalcVar> &vars) {CalcValue value; Calculate(value, expression, vars); return value.asLong ();}
Flt   CalculateF (C Str &expression, C MemPtr<CalcVar> &vars) {CalcValue value; Calculate(value, expression, vars); return value.asFlt  ();}
Dbl   CalculateD (C Str &expression, C MemPtr<CalcVar> &vars) {CalcValue value; Calculate(value, expression, vars); return value.asDbl  ();}
VecD2 CalculateV2(C Str &expression, C MemPtr<CalcVar> &vars) {CalcValue value; Calculate(value, expression, vars); return value.asVecD2();}
VecD  CalculateV (C Str &expression, C MemPtr<CalcVar> &vars) {CalcValue value; Calculate(value, expression, vars); return value.asVecD ();}
VecD4 CalculateV4(C Str &expression, C MemPtr<CalcVar> &vars) {CalcValue value; Calculate(value, expression, vars); return value.asVecD4();}
/******************************************************************************/
}
/******************************************************************************/
