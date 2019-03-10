/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   TODO: in the future use SByte exp/shift as Number member to have more control of the value range.

   sin(x) = x - x**3/ 3!
   cos(x) = 1 - x**2/ 2! + x**4/ 4! - x**6/ 6! ..
   tan(x) = x + x**3/ 3!

/******************************************************************************/
#if NUMBER_DIGS<4
   #error NUMBER_DIGS must be >= 4
#elif !IS_POW_2(NUMBER_DIGS)
   #error NUMBER_DIGS must be a power of 2
#endif
#pragma runtime_checks("", off)
/******************************************************************************/
Int Number::digits()C
{
   REP(NUMBER_DIGS)if(d[i])return i+1;
   return 0;
}
Number& Number::zero()
{
   Zero(T);
   return T;
}

Int   Number::asInt  ()C {UInt  u=(U32&)d[_real ? NUMBER_DIGS/2 : 0]; return sign ? -Int (u) : u;}
UInt  Number::asUInt ()C {return  (U32&)d[_real ? NUMBER_DIGS/2 : 0];}
Long  Number::asLong ()C {ULong u=(U64&)d[_real ? NUMBER_DIGS/2 : 0]; return sign ? -Long(u) : u;}
ULong Number::asULong()C {return  (U64&)d[_real ? NUMBER_DIGS/2 : 0];}

// !! keep as operator=, even though these could be converted to constructors, Android toolchain will throw compilation errors !!
Number& Number::operator=(Int   i) {zero(); sign=(i<0); *(U32*)d=Abs(i); return T;}
Number& Number::operator=(UInt  u) {zero();             *(U32*)d=    u ; return T;}
Number& Number::operator=(Long  i) {zero(); sign=(i<0); *(U64*)d=Abs(i); return T;}
Number& Number::operator=(ULong u) {zero();             *(U64*)d=    u ; return T;}

Flt Number::asFlt()C
{
   if(!_real)return asInt();
   if(Int digits=T.digits())
   {
      digits--;
      UInt pos=digits*16+BitHi(Unsigned(d[digits]));
      Int  exp=Mid(pos+(_real ? -NUMBER_DIGS*16/2 : 0), -127, 127);
      UInt significand=0;
      pos--  ; if(pos<NUMBER_DIGS*16)significand|=Shr(d[pos>>4], (pos&15)-22);
      pos-=16; if(pos<NUMBER_DIGS*16)significand|=Shr(d[pos>>4], (pos&15)- 6);
      pos-=16; if(pos<NUMBER_DIGS*16)significand|=Shr(d[pos>>4], (pos&15)+10);
      UInt f=(sign<<31)|(((exp+127)&0xFF)<<23)|(significand&0x7FFFFF);
      return (Flt&)f;
   }
   return 0;
}
Dbl Number::asDbl()C
{
   if(!_real)return asLong();
   if(Int digits=T.digits())
   {
      digits--;
      UInt pos=digits*16+BitHi(Unsigned(d[digits]));
      Int  exp=Mid(pos+(_real ? -NUMBER_DIGS*16/2 : 0), -1023, 1023);
      U32  m0=0, m1=0;
      pos--  ; if(pos<NUMBER_DIGS*16) m0|=Shr(d[pos>>4], (pos&15)-19);
      pos-=16; if(pos<NUMBER_DIGS*16){m0|=Shr(d[pos>>4], (pos&15)- 3); m1|=Shr(d[pos>>4], (pos&15)-35);}
      pos-=16; if(pos<NUMBER_DIGS*16){m0|=Shr(d[pos>>4], (pos&15)+13); m1|=Shr(d[pos>>4], (pos&15)-19);}
      pos-=16; if(pos<NUMBER_DIGS*16)                                  m1|=Shr(d[pos>>4], (pos&15)- 3);
      pos-=16; if(pos<NUMBER_DIGS*16)                                  m1|=Shr(d[pos>>4], (pos&15)+13);
      U32 f[2]=
      {
         m1,
         (m0&0xFFFFF)|(((exp+1023)&0x7FF)<<20)|(sign<<31),
      };
      return (Dbl&)f[0];
   }
   return 0;
}

Number& Number::operator=(Flt f)
{
   zero();
   U32  d          =(U32&)f,
        significand=(d&0x7FFFFF);
   Int  exp        =((d>>23)&0xFF)-127;
        sign       =((d>>31)!=0);
   UInt pos        =exp+NUMBER_DIGS*16/2;
       _real       =true;
            if(pos<NUMBER_DIGS*16)T.d[pos>>4] =             (1<<(pos&15)   );
   pos--  ; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=Shl(significand, (pos&15)-22);
   pos-=16; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=Shl(significand, (pos&15)- 6);
   pos-=16; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=Shl(significand, (pos&15)+10);

   return T;
}
Number& Number::operator=(Dbl f)
{
   zero();
   U32 *d   =(U32*)&f,
        m0  =(d[1]&0xFFFFF),
        m1  =(d[0]);
   Int  exp =((d[1]>>20)&0x7FF)-1023;
        sign=((d[1]>>31)!=0);
   UInt pos =exp+NUMBER_DIGS*16/2;
       _real=true;
            if(pos<NUMBER_DIGS*16)T.d[pos>>4] =    (1<<(pos&15)   );
   pos--  ; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=Shl(m0, (pos&15)-19);
   pos-=16; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=Shl(m0, (pos&15)- 3)|Shl(m1, (pos&15)-35);
   pos-=16; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=Shl(m0, (pos&15)+13)|Shl(m1, (pos&15)-19);
   pos-=16; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=                     Shl(m1, (pos&15)- 3);
   pos-=16; if(pos<NUMBER_DIGS*16)T.d[pos>>4]|=                     Shl(m1, (pos&15)+13);

   return T;
}

Number& Number::operator=(C Str &s)
{
   Int    mode=10;
   Number frac_m=1.0f;
   zero();

   if(CChar *t=s)
   {
      if(t[0]=='-'){sign=true; t++;}
      if(t[0]=='0')
      {
         if(t[1]=='b' || t[1]=='B'){t+=2; mode= 2;}else
         if(t[1]=='x' || t[1]=='X'){t+=2; mode=16;}
      }
      for(;;)
      {
         Char c=*t++;
         if(c=='.')
         {
            if(!_real)toReal();
         }else
         {
            Int i=CharInt(c);
            if(!InRange(i, mode))break;
            if(!_real){T*=mode; T+=i;}
            else       T+=(frac_m/=mode)*i;
         }
      }
   }
   return T;
}
/******************************************************************************/
Number& Number::toReal() {if(!_real){shlDig(NUMBER_DIGS/2); _real=true ;} return T;}
Number& Number::toInt () {if( _real){shrDig(NUMBER_DIGS/2); _real=false;} return T;}
/******************************************************************************/
Int Number::rawCompare(C Number &N)C
{
   REP(NUMBER_DIGS)
   {
      Int d=T.d[i]-N.d[i];
      if(d>0)return +1;
      if(d<0)return -1;
   }
   return 0;
}
Int Number::rawCompare(UInt N)C
{
   for(Int i=NUMBER_DIGS-1; i>=2; i--)if(d[i])return +1;
   Int d=T.d[1]-(N>>16   ); if(d>0)return +1; if(d<0)return -1;
       d=T.d[0]-(N&0xFFFF); if(d>0)return +1; if(d<0)return -1;
   return 0;
}
Int Number::absCompare(C Number &N)C
{
   if(_real==N._real)
   {
      return rawCompare(N);
   }else
   {
      if(_real){REP(NUMBER_DIGS/2)if(N.d[i+NUMBER_DIGS/2])return -1; REP(NUMBER_DIGS/2){Int d=T.d[i+NUMBER_DIGS/2]-N.d[i]; if(d>0)return +1; if(d<0)return -1;} REP(NUMBER_DIGS/2)if(T.d[i])return +1;}
      else     {REP(NUMBER_DIGS/2)if(T.d[i+NUMBER_DIGS/2])return +1; REP(NUMBER_DIGS/2){Int d=T.d[i]-N.d[i+NUMBER_DIGS/2]; if(d>0)return +1; if(d<0)return -1;} REP(NUMBER_DIGS/2)if(N.d[i])return -1;}
   }
   return 0;
}
Int Number::absCompare(UInt N)C
{
   if(!_real)
   {
      return rawCompare(N);
   }else
   {
      for(Int i=NUMBER_DIGS-1; i>=NUMBER_DIGS/2+2; i--)if(d[i])return +1;
      Int d=T.d[NUMBER_DIGS/2+1]-(N>>16   ); if(d>0)return +1; if(d<0)return -1;
          d=T.d[NUMBER_DIGS/2  ]-(N&0xFFFF); if(d>0)return +1; if(d<0)return -1;
      REP(NUMBER_DIGS/2)if(T.d[i])return +1;
   }
   return 0;
}
Int Compare(C Number &n0, C Number &n1) {if(n0.sign== n1.sign)return n0.sign ? -n0.absCompare(    n1 ) : n0.absCompare(    n1 ); return n0.sign ? -1 : +1;}
Int Compare(C Number &n0, C Int    &n1) {if(n0.sign==(n1<0  ))return n0.sign ? -n0.absCompare(Abs(n1)) : n0.absCompare(Abs(n1)); return n0.sign ? -1 : +1;}
/******************************************************************************/
// SHIFT ROLL
/******************************************************************************/
Number& Number::operator<<=(Int bits)
{
   if(bits>0)
   {
      UInt D=bits>>4; bits&=15;
      UInt r=16-bits;
      REP(NUMBER_DIGS)
      {
         UInt j=i-D;
         T.d[i]=(dig(j  )<<bits)
               |(dig(j-1)>>r   );
      }
   }else if(bits)T>>=(-bits);
   return T;
}
Number& Number::operator>>=(Int bits)
{
   if(bits>0)
   {
      UInt D=bits>>4; bits&=15;
      UInt r=16-bits;
      FREP(NUMBER_DIGS)
      {
         UInt j=i+D;
         T.d[i]=(dig(j  )>>bits)
               |(dig(j+1)<<r   );
      }
   }else if(bits)T<<=(-bits);
   return T;
}
Number& Number::rol(Int bits)
{
   if(bits>0)
   {
      UInt D=bits>>4; bits&=15;
      UInt r=16-bits;
      REP(NUMBER_DIGS)
      {
         UInt j=i-D;
         T.d[i]=(digMod(j  )<<bits)
               |(digMod(j-1)>>r   );
      }
   }else if(bits)ror(-bits);
   return T;
}
Number& Number::ror(Int bits)
{
   if(bits>0)
   {
      UInt D=bits>>4; bits&=15;
      UInt r=16-bits;
      FREP(NUMBER_DIGS)
      {
         UInt j=i+D;
         T.d[i]=(digMod(j  )>>bits)
               |(digMod(j+1)<<r   );
      }
   }else if(bits)rol(-bits);
   return T;
}
Number& Number::setShlDig(C Number &N, Int digs)
{
   if(digs>0){sign=N.sign; _real=N._real; REP(NUMBER_DIGS)T.d[i]=N.dig(i-digs);}else
   if(digs<0)setShrDig(N, -digs);else T=N;
   return T;
}
Number& Number::setShrDig(C Number &N, Int digs)
{
   if(digs>0){sign=N.sign; _real=N._real; FREP(NUMBER_DIGS)T.d[i]=N.dig(i+digs);}else
   if(digs<0)setShlDig(N, -digs);else T=N;
   return T;
}
Number& Number::shlDig(Int digs)
{
   if(digs>0)REP(NUMBER_DIGS)T.d[i]=dig(i-digs);else
   if(digs<0)shrDig(-digs);
   return T;
}
Number& Number::shrDig(Int digs)
{
   if(digs>0)FREP(NUMBER_DIGS)T.d[i]=dig(i+digs);else
   if(digs<0)shlDig(-digs);
   return T;
}
Number operator<<(C Number &a, Int bits) {return Number(a)<<=bits;}
Number operator>>(C Number &a, Int bits) {return Number(a)>>=bits;}
/******************************************************************************/
// ADD SUB MUL DIV MOD
/******************************************************************************/
Number& Number::rawAdd(C Number &N)
{
   UInt x=0;
   FREP(NUMBER_DIGS)
   {
      x  +=d[i]+N.d[i];
      d[i]=x;
      x >>=16;
   }
   return T;
}
Number& Number::rawSub(C Number &N)
{
   Byte carry=0;
   if(rawCompare(N)>=0)
   {
      FREP(NUMBER_DIGS)
      {
         Int x=d[i]-N.d[i]-carry;
         d[i] =x;
         carry=(x<0);
      }
   }else
   {
      FREP(NUMBER_DIGS)
      {
         Int x=N.d[i]-d[i]-carry;
         d[i] =x;
         carry=(x<0);
      }
      sign^=1;
   }
   return T;
}
Number& Number::absAdd(C Number &N)
{
   if(!_real && N._real)toReal();
   if( _real == N._real)
   {
      rawAdd(N);
   }else
   {
      UInt x=0;
      for(Int i=NUMBER_DIGS/2; i<NUMBER_DIGS; i++)
      {
         x  +=d[i]+N.d[i-NUMBER_DIGS/2];
         d[i]=x;
         x >>=16;
      }
   }
   return T;
}
Number& Number::absAdd(UInt N)
{
   UInt x=0;
   if(!_real)
   {
      FREP(NUMBER_DIGS)
      {
           x+=d[i]+(N&0xFFFF);
         d[i]=x;
         x >>=16;
         N >>=16;
      }
   }else
   {
      for(Int i=NUMBER_DIGS/2; i<NUMBER_DIGS; i++)
      {
           x+=d[i]+(N&0xFFFF);
         d[i]=x;
         x >>=16;
         N >>=16;
      }
   }
   return T;
}
Number& Number::absSub(C Number &N)
{
   if(!_real && N._real)toReal();
   if( _real == N._real)
   {
      rawSub(N);
   }else
   {
      Byte carry=0;
      if(absCompare(N)>=0)
      {
         for(Int i=NUMBER_DIGS/2; i<NUMBER_DIGS; i++)
         {
            Int x=d[i]-N.d[i-NUMBER_DIGS/2]-carry;
            d[i] =x;
            carry=(x<0);
         }
      }else
      {
         FREP(NUMBER_DIGS/2)
         {
            Int x=-d[i]-carry;
            d[i] =x;
            carry=(x<0);
         }
         for(Int i=NUMBER_DIGS/2; i<NUMBER_DIGS; i++)
         {
            Int x=N.d[i-NUMBER_DIGS/2]-d[i]-carry;
            d[i] =x;
            carry=(x<0);
         }
         sign^=1;
      }
   }
   return T;
}
Number& Number::absSub(UInt N)
{
   Byte carry=0;
   if(!_real)
   {
      if(rawCompare(N)>=0)
      {
         FREP(NUMBER_DIGS)
         {
            Int x=d[i]-(N&0xFFFF)-carry;
            d[i] =x;
            carry=(x<0);
            N  >>=16;
         }
      }else
      {
         FREP(NUMBER_DIGS)
         {
            Int x=(N&0xFFFF)-d[i]-carry;
            d[i] =x;
            carry=(x<0);
            N  >>=16;
         }
         sign^=1;
      }
   }else
   {
      if(absCompare(N)>=0)
      {
         for(Int i=NUMBER_DIGS/2; i<NUMBER_DIGS; i++)
         {
            Int x=d[i]-(N&0xFFFF)-carry;
            d[i] =x;
            carry=(x<0);
            N  >>=16;
         }
      }else
      {
         FREP(NUMBER_DIGS/2)
         {
            Int x=-d[i]-carry;
            d[i] =x;
            carry=(x<0);
         }
         for(Int i=NUMBER_DIGS/2; i<NUMBER_DIGS; i++)
         {
            Int x=(N&0xFFFF)-d[i]-carry;
            d[i] =x;
            carry=(x<0);
            N  >>=16;
         }
         sign^=1;
      }
   }
   return T;
}
/******************************************************************************/
Number& Number::operator+=(C Number &N) {if(sign==N.sign)absAdd(    N );else absSub(    N ); return T;}
Number& Number::operator+=(  Int     N) {if(sign==(N<0 ))absAdd(Abs(N));else absSub(Abs(N)); return T;}
Number& Number::operator-=(C Number &N) {if(sign==N.sign)absSub(    N );else absAdd(    N ); return T;}
Number& Number::operator-=(  Int     N) {if(sign==(N<0 ))absSub(Abs(N));else absAdd(Abs(N)); return T;}
/******************************************************************************/
Number& Number::operator*=(C Number &N)
{
	Number t0, t1;
	t0.zero().sign=sign^N.sign;
	FREPD(n0, NUMBER_DIGS)
	{
	   t1.zero();
		UInt x=0;
		if(!_real || !N._real)
		{
		   FREPD(n1, NUMBER_DIGS)
		   {
		      if(n0+n1>=NUMBER_DIGS)break;
		      x+=d[n0]*N.d[n1];
		      t1.d[n0+n1]=x;
		      x>>=16;
		   }
		}else
		{
		   FREPD(n1, NUMBER_DIGS)
		   {
		      x+=d[n0]*N.d[n1];
		      UInt p=n0+n1-NUMBER_DIGS/2; if(p<NUMBER_DIGS)t1.d[p]=x;
		      x>>=16;
		   }
		}
		t0.rawAdd(t1);
	}
	t0._real=(_real|N._real); T=t0;
	return T;
}
/******************************************************************************/
static Bool DivModFast(Number &x, C Number &y, Number &div)
{
   Int yd=y.digits();
   if( yd<=4)
   {
      if(!yd){div.zero(); return true;}
      Int xd=x.digits();
      if( xd<=4)
      {
         Bool dsign=x. sign^y.sign,
              msign=x. sign,
             dmreal=x._real;
         if(xd<=2 && yd<=2)
         {
            U32 _x=(U32&)x.d[0],
                _y=(U32&)y.d[0];
            div=_x/_y;
            x  =_x%_y;
         }else
         {
            U64 _x=(U64&)x.d[0],
                _y=(U64&)y.d[0];
            div=_x/_y;
            x  =_x%_y;
         }
         div.sign=dsign; div._real=dmreal;
           x.sign=msign;   x._real=dmreal;
         return true;
      }
   }
   return false;
}
static void DivModSub(Number &x, C Number &y, Number &div)
{
   if(!DivModFast(x, y, div))
   {
      UInt div_sub=0; for(; x.rawCompare(y)>=0; x.rawSub(y), div_sub++);
       div=div_sub;
   }
}
static void DivMod(Number &x, C Number &y, Number &div)
{
   Number  y_temp;
 C Number *Y;
   if(!y._real)
   {
      if(DivModFast(x, y, div))return;
      Y=&y;
   }else
   {
      // auto shifting for more precision
      Int xd=x.toReal().digits(),
          yd=y         .digits(),
          m =Max(xd, yd-NUMBER_DIGS/2),
          l =NUMBER_DIGS-m;
      if(yd+l-NUMBER_DIGS/2<=0){div.zero(); return;}
              x.   shlDig(l);
      Y=&y_temp.setShlDig(y, l-NUMBER_DIGS/2);
   }

   div.zero();
   div. sign=x. sign^y.sign;
   div._real=x._real;
   REP(NUMBER_DIGS)
   {
      Number temp; temp.setShrDig(x, i);
      if(temp.rawCompare(*Y)>=0)
      {
         Number bkp=temp, div_sub; DivModSub(temp, *Y, div_sub);
           x.rawSub(bkp.rawSub(temp).shlDig(i));
         div.rawAdd(         div_sub.shlDig(i));
      }
   }
}
Number& Number::operator/=(C Number &N) {Number div; DivMod(T, N, div); T=div; return T;}
Number& Number::operator%=(C Number &N) {Number div; DivMod(T, N, div);        return T;}
/******************************************************************************/
                                                           Number& Number::operator+=(Dbl N) {return T+=Number(N);}
                                                           Number& Number::operator-=(Dbl N) {return T-=Number(N);}
Number& Number::operator*=(Int N) {return T*=Number(N);}   Number& Number::operator*=(Dbl N) {return T*=Number(N);}
Number& Number::operator/=(Int N) {return T/=Number(N);}   Number& Number::operator/=(Dbl N) {return T/=Number(N);}
Number& Number::operator%=(Int N) {return T%=Number(N);}
/******************************************************************************/
Number operator+(C Number &a, C Number &b){return Number(a)+=b;}   Number operator+(C Number &a, Int b) {return Number(a)+=b;}   Number operator+(Int a, C Number &b) {return Number(a)+=b;}   Number operator+(C Number &a, Dbl b) {return Number(a)+=b;}   Number operator+(Dbl a, C Number &b) {return Number(a)+=b;}
Number operator-(C Number &a, C Number &b){return Number(a)-=b;}   Number operator-(C Number &a, Int b) {return Number(a)-=b;}   Number operator-(Int a, C Number &b) {return Number(a)-=b;}   Number operator-(C Number &a, Dbl b) {return Number(a)-=b;}   Number operator-(Dbl a, C Number &b) {return Number(a)-=b;}
Number operator*(C Number &a, C Number &b){return Number(a)*=b;}   Number operator*(C Number &a, Int b) {return Number(a)*=b;}   Number operator*(Int a, C Number &b) {return Number(a)*=b;}   Number operator*(C Number &a, Dbl b) {return Number(a)*=b;}   Number operator*(Dbl a, C Number &b) {return Number(a)*=b;}
Number operator/(C Number &a, C Number &b){return Number(a)/=b;}   Number operator/(C Number &a, Int b) {return Number(a)/=b;}   Number operator/(Int a ,C Number &b) {return Number(a)/=b;}   Number operator/(C Number &a, Dbl b) {return Number(a)/=b;}   Number operator/(Dbl a, C Number &b) {return Number(a)/=b;}
Number operator%(C Number &a, C Number &b){return Number(a)%=b;}
/******************************************************************************/
Number& Number::sqr()
{
	Number t0,t1;
	t0.zero();
	FREPD(n0, NUMBER_DIGS)
	{
	   t1.zero();
		UInt x=0;
		if(_real)
		{
		   FREPD(n1, NUMBER_DIGS)
		   {
		      x+=d[n0]*d[n1];
		      UInt p=n0+n1-NUMBER_DIGS/2; if(p<NUMBER_DIGS)t1.d[p]=x;
		      x>>=16;
		   }
		}else
		{
		   FREPD(n1, NUMBER_DIGS)
		   {
		      if(n0+n1>=NUMBER_DIGS)break;
		      x+=d[n0]*d[n1];
		      t1.d[n0+n1]=x;
		      x>>=16;
		   }
		}
		t0.rawAdd(t1);
	}
	t0._real=_real; T=t0;
	return T;
}
Number& Number::sqrt()
{
   if(Int digits=T.digits())
   {
      Int shl=0;
      if(_real) // auto shifting for more precision
      {
         shl=((NUMBER_DIGS-digits)&~1);
         shlDig(shl); digits+=shl;
         shl=NUMBER_DIGS/4 - (shl>>1);
      }

      Number res=0, one=0; digits--;
      UInt   bit=1<<14; for(UInt dig=d[digits]; bit>dig; )bit>>=2; one.d[digits]=bit;
      for(; one.digits(); )
      {
         Number res_one=res; res_one.rawAdd(one);
         if(rawCompare(res_one)>=0)
         {
            rawSub(res_one);
            res=res_one.rawAdd(one);
         }
         res>>=1;
         one>>=2;
      }
      res.shlDig(shl)._real=_real;
      T=res;
   }
   return T;
}
/******************************************************************************/
Number Number2::length2  ()C {return  x*x + y*y              ;}
Number Number3::length2  ()C {return  x*x + y*y + z*z        ;}
Number Number2::length   ()C {return (x*x + y*y      ).sqrt();}
Number Number3::length   ()C {return (x*x + y*y + z*z).sqrt();}
Number Number2::normalize()  {Number l=length(); T/=l; return l;}
Number Number3::normalize()  {Number l=length(); T/=l; return l;}
/******************************************************************************/
#pragma runtime_checks("", restore)
}
/******************************************************************************/
