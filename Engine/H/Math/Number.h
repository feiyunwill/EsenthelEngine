/******************************************************************************

   Use 'Number' for handling numbers with very big precision.

/******************************************************************************/
#define NUMBER_DIGS 32 // number of 'Number' digits
/******************************************************************************/
struct Number // Number with big precision (NUMBER_DIGS*16 bit precision)
{
   Bool   sign          ; // sign (false=positive, true=negative)
   UShort d[NUMBER_DIGS]; // digits

   // get / set
#if EE_PRIVATE
   Int digits()C; // get number of used digits (this is the highest non zero digit index +1)
#endif
   Number& zero(); // set to zero

   Int   asInt  ()C; // return as Int
   UInt  asUInt ()C; // return as UInt
   Long  asLong ()C; // return as Long
   ULong asULong()C; // return as ULong
   Flt   asFlt  ()C; // return as Flt
   Dbl   asDbl  ()C; // return as Dbl

   UShort dig   (UInt i)C {return InRange(i, d) ? d[i] : 0;} // safe get digit, 0 if 'i' is invalid
   UShort digMod(UInt i)C {return d[i&(NUMBER_DIGS-1)]    ;} // safe get digit, d[i%NUMBER_DIGS]

   // Int / Real
   Bool    isReal()C {return  _real;} // if the number is of Real type
   Bool    isInt ()C {return !_real;} // if the number is of Int  type
   Number& toReal();                  // convert to Real
   Number& toInt ();                  // convert to Int

   // compare
   friend Int  Compare   (C Number &n0, C Number &n1);                             // compare
   friend Int  Compare   (C Number &n0, C Int    &n1);                             // compare
   friend Bool operator==(C Number &n0, C Number &n1) {return Compare(n0, n1)==0;} // if equal
   friend Bool operator==(C Number &n0,   Int     n1) {return Compare(n0, n1)==0;} // if equal
   friend Bool operator!=(C Number &n0, C Number &n1) {return Compare(n0, n1)!=0;} // if not equal
   friend Bool operator!=(C Number &n0,   Int     n1) {return Compare(n0, n1)!=0;} // if not equal
   friend Bool operator>=(C Number &n0, C Number &n1) {return Compare(n0, n1)>=0;} // if greater or equal
   friend Bool operator>=(C Number &n0,   Int     n1) {return Compare(n0, n1)>=0;} // if greater or equal
   friend Bool operator<=(C Number &n0, C Number &n1) {return Compare(n0, n1)<=0;} // if smaller or equal
   friend Bool operator<=(C Number &n0,   Int     n1) {return Compare(n0, n1)<=0;} // if smaller or equal
   friend Bool operator> (C Number &n0, C Number &n1) {return Compare(n0, n1)> 0;} // if greater
   friend Bool operator> (C Number &n0,   Int     n1) {return Compare(n0, n1)> 0;} // if greater
   friend Bool operator< (C Number &n0, C Number &n1) {return Compare(n0, n1)< 0;} // if smaller
   friend Bool operator< (C Number &n0,   Int     n1) {return Compare(n0, n1)< 0;} // if smaller

   // add / subtract / multiply / divide / mod
          Number& operator+=(C Number &N);   Number& operator+=(Int);   Number& operator+=(Dbl);   Number& operator++() {T+=1; return T;}   void operator++(Int) {T+=1;} // add
          Number& operator-=(C Number &N);   Number& operator-=(Int);   Number& operator-=(Dbl);   Number& operator--() {T-=1; return T;}   void operator--(Int) {T-=1;} // subtract
          Number& operator*=(C Number &N);   Number& operator*=(Int);   Number& operator*=(Dbl);                                                                         // multiply
          Number& operator/=(C Number &N);   Number& operator/=(Int);   Number& operator/=(Dbl);                                                                         // divide
          Number& operator%=(C Number &N);   Number& operator%=(Int);                                                                                                    // mod
   friend Number  operator+ (C Number &a, C Number &b);   friend Number operator+(C Number&, Int);   friend Number operator+(Int, C Number&);   friend Number operator+(C Number&, Dbl);   friend Number operator+(Dbl, C Number&); // add
   friend Number  operator- (C Number &a, C Number &b);   friend Number operator-(C Number&, Int);   friend Number operator-(Int, C Number&);   friend Number operator-(C Number&, Dbl);   friend Number operator-(Dbl, C Number&); // subtract
   friend Number  operator* (C Number &a, C Number &b);   friend Number operator*(C Number&, Int);   friend Number operator*(Int, C Number&);   friend Number operator*(C Number&, Dbl);   friend Number operator*(Dbl, C Number&); // multiply
   friend Number  operator/ (C Number &a, C Number &b);   friend Number operator/(C Number&, Int);   friend Number operator/(Int, C Number&);   friend Number operator/(C Number&, Dbl);   friend Number operator/(Dbl, C Number&); // divide
   friend Number  operator% (C Number &a, C Number &b);                                                                                                                                                                             // mod

   // sqr / sqrt
   Number& sqr (); // T*=T       (square)
   Number& sqrt(); // T =sqrt(T) (square-root)

   // shift / rotate
          Number& operator<<=(             Int bits); // shift  bits left
          Number& operator>>=(             Int bits); // shift  bits right
   friend Number  operator<< (C Number &a, Int bits); // shift  bits left
   friend Number  operator>> (C Number &a, Int bits); // shift  bits right
          Number&         rol(             Int bits); // rotate bits left
          Number&         ror(             Int bits); // rotate bits right
#if EE_PRIVATE
          Number&   setShlDig(C Number &N, Int digs); // shift digits left  "T  =N<<(d*16)"
          Number&   setShrDig(C Number &N, Int digs); // shift digits right "T  =N>>(d*16)"
          Number&      shlDig(             Int digs); // shift digits left  "T<<=   (d*16)"
          Number&      shrDig(             Int digs); // shift digits right "T>>=   (d*16)"
#endif

   Number() {}
   Number& operator=(  Int   i);   Number(Int   i) {T=i;}
   Number& operator=(  UInt  u);   Number(UInt  u) {T=u;}
   Number& operator=(  Long  i);   Number(Long  i) {T=i;}
   Number& operator=(  ULong u);   Number(ULong u) {T=u;}
   Number& operator=(  Flt   f);   Number(Flt   f) {T=f;}
   Number& operator=(  Dbl   f);   Number(Dbl   f) {T=f;}
   Number& operator=(C Str  &s); // set from string

#if EE_PRIVATE
   Int rawCompare(C Number &N)C;
   Int rawCompare(  UInt    N)C;
   Int absCompare(C Number &N)C;
   Int absCompare(  UInt    N)C;
   Int absCompare(  Int     N)C {return absCompare((UInt)Abs(N));}

   Number& rawAdd(C Number &N);
   Number& rawSub(C Number &N);
   Number& absAdd(C Number &N);
   Number& absAdd(  UInt    N);
   Number& absSub(C Number &N);
   Number& absSub(  UInt    N);
#endif

#if !EE_PRIVATE
private:
#endif
   Bool _real;
};
/******************************************************************************/
struct Number2
{
   Number x, y;

   Number2& zero(                        ) {x.zero(); y.zero(); return T;}
   Number2& set (  Dbl     r             ) {x=y=r;              return T;}
   Number2& set (  Dbl     x,   Dbl     y) {T.x=x; T.y=y;       return T;}
   Number2& set (C Number &x, C Number &y) {T.x=x; T.y=y;       return T;}

          Number2& operator+=(Dbl      r) {x+=  r; y+=  r; return T;}
          Number2& operator-=(Dbl      r) {x-=  r; y-=  r; return T;}
          Number2& operator*=(Dbl      r) {x*=  r; y*=  r; return T;}
          Number2& operator/=(Dbl      r) {x/=  r; y/=  r; return T;}
          Number2& operator+=(Number  &n) {x+=n  ; y+=n  ; return T;}
          Number2& operator-=(Number  &n) {x-=n  ; y-=n  ; return T;}
          Number2& operator*=(Number  &n) {x*=n  ; y*=n  ; return T;}
          Number2& operator/=(Number  &n) {x/=n  ; y/=n  ; return T;}
          Number2& operator+=(Number2 &n) {x+=n.x; y+=n.y; return T;}
          Number2& operator-=(Number2 &n) {x-=n.x; y-=n.y; return T;}
          Number2& operator*=(Number2 &n) {x*=n.x; y*=n.y; return T;}
          Number2& operator/=(Number2 &n) {x/=n.x; y/=n.y; return T;}

   friend Number2  operator+ (C Number2 &n,   Dbl      r) {return Number2(n.x+r, n.y+r);}
   friend Number2  operator- (C Number2 &n,   Dbl      r) {return Number2(n.x-r, n.y-r);}
   friend Number2  operator* (C Number2 &n,   Dbl      r) {return Number2(n.x*r, n.y*r);}
   friend Number2  operator/ (C Number2 &n,   Dbl      r) {return Number2(n.x/r, n.y/r);}
   friend Number2  operator+ (  Dbl      r, C Number2 &n) {return Number2(r+n.x, r+n.y);}
   friend Number2  operator- (  Dbl      r, C Number2 &n) {return Number2(r-n.x, r-n.y);}
   friend Number2  operator* (  Dbl      r, C Number2 &n) {return Number2(r*n.x, r*n.y);}
   friend Number2  operator/ (  Dbl      r, C Number2 &n) {return Number2(r/n.x, r/n.y);}
   friend Number2  operator+ (C Number2 &n, C Number  &r) {return Number2(n.x+r, n.y+r);}
   friend Number2  operator- (C Number2 &n, C Number  &r) {return Number2(n.x-r, n.y-r);}
   friend Number2  operator* (C Number2 &n, C Number  &r) {return Number2(n.x*r, n.y*r);}
   friend Number2  operator/ (C Number2 &n, C Number  &r) {return Number2(n.x/r, n.y/r);}
   friend Number2  operator+ (C Number  &r, C Number2 &n) {return Number2(r+n.x, r+n.y);}
   friend Number2  operator- (C Number  &r, C Number2 &n) {return Number2(r-n.x, r-n.y);}
   friend Number2  operator* (C Number  &r, C Number2 &n) {return Number2(r*n.x, r*n.y);}
   friend Number2  operator/ (C Number  &r, C Number2 &n) {return Number2(r/n.x, r/n.y);}
   friend Number2  operator+ (C Number2 &a, C Number2 &b) {return Number2(a.x+b.x, a.y+b.y);}
   friend Number2  operator- (C Number2 &a, C Number2 &b) {return Number2(a.x-b.x, a.y-b.y);}
   friend Number2  operator* (C Number2 &a, C Number2 &b) {return Number2(a.x*b.x, a.y*b.y);}
   friend Number2  operator/ (C Number2 &a, C Number2 &b) {return Number2(a.x/b.x, a.y/b.y);}

   Number length   ()C; // get         length
   Number length2  ()C; // get squared length
   Number normalize() ; // normalize   length and return previous length

   Number2() {}
   Number2(  Dbl     r             ) {set(r   );}
   Number2(  Dbl     x,   Dbl     y) {set(x, y);}
   Number2(C Number &x, C Number &y) {set(x, y);}
};
/******************************************************************************/
struct Number3
{
   Number x, y, z;

   Number2& n2()  {return (Number2&)T;} // get this as       Number2 format (using x, y members only), this returns reference to self and not a new object!
 C Number2& n2()C {return (Number2&)T;} // get this as const Number2 format (using x, y members only), this returns reference to self and not a new object!

   Number3& zero(                                     ) {x.zero(); y.zero(); z.zero(); return T;}
   Number3& set (  Dbl     r                          ) {x=y=z=r;                      return T;}
   Number3& set (  Dbl     x,   Dbl     y,   Dbl     z) {T.x=x; T.y=y; T.z=z;          return T;}
   Number3& set (C Number &x, C Number &y, C Number &z) {T.x=x; T.y=y; T.z=z;          return T;}
   Number3& set (C Number2 &n2           , C Number &z) {T.n2()=n2;    T.z=z;          return T;}

          Number3& operator+=(  Dbl      r) {x+=  r; y+=  r; z+=  r; return T;}
          Number3& operator-=(  Dbl      r) {x-=  r; y-=  r; z-=  r; return T;}
          Number3& operator*=(  Dbl      r) {x*=  r; y*=  r; z*=  r; return T;}
          Number3& operator/=(  Dbl      r) {x/=  r; y/=  r; z/=  r; return T;}
          Number3& operator+=(C Number  &n) {x+=n  ; y+=n  ; z+=n  ; return T;}
          Number3& operator-=(C Number  &n) {x-=n  ; y-=n  ; z-=n  ; return T;}
          Number3& operator*=(C Number  &n) {x*=n  ; y*=n  ; z*=n  ; return T;}
          Number3& operator/=(C Number  &n) {x/=n  ; y/=n  ; z/=n  ; return T;}
          Number3& operator+=(C Number3 &n) {x+=n.x; y+=n.y; z+=n.z; return T;}
          Number3& operator-=(C Number3 &n) {x-=n.x; y-=n.y; z-=n.z; return T;}
          Number3& operator*=(C Number3 &n) {x*=n.x; y*=n.y; z*=n.z; return T;}
          Number3& operator/=(C Number3 &n) {x/=n.x; y/=n.y; z/=n.z; return T;}

   friend Number3  operator+ (C Number3 &n,   Dbl      r) {return Number3(n.x+r, n.y+r, n.z+r);}
   friend Number3  operator- (C Number3 &n,   Dbl      r) {return Number3(n.x-r, n.y-r, n.z-r);}
   friend Number3  operator* (C Number3 &n,   Dbl      r) {return Number3(n.x*r, n.y*r, n.z*r);}
   friend Number3  operator/ (C Number3 &n,   Dbl      r) {return Number3(n.x/r, n.y/r, n.z/r);}
   friend Number3  operator+ (  Dbl      r, C Number3 &n) {return Number3(r+n.x, r+n.y, r+n.z);}
   friend Number3  operator- (  Dbl      r, C Number3 &n) {return Number3(r-n.x, r-n.y, r-n.z);}
   friend Number3  operator* (  Dbl      r, C Number3 &n) {return Number3(r*n.x, r*n.y, r*n.z);}
   friend Number3  operator/ (  Dbl      r, C Number3 &n) {return Number3(r/n.x, r/n.y, r/n.z);}
   friend Number3  operator+ (C Number3 &n, C Number  &r) {return Number3(n.x+r, n.y+r, n.z+r);}
   friend Number3  operator- (C Number3 &n, C Number  &r) {return Number3(n.x-r, n.y-r, n.z-r);}
   friend Number3  operator* (C Number3 &n, C Number  &r) {return Number3(n.x*r, n.y*r, n.z*r);}
   friend Number3  operator/ (C Number3 &n, C Number  &r) {return Number3(n.x/r, n.y/r, n.z/r);}
   friend Number3  operator+ (C Number  &r, C Number3 &n) {return Number3(r+n.x, r+n.y, r+n.z);}
   friend Number3  operator- (C Number  &r, C Number3 &n) {return Number3(r-n.x, r-n.y, r-n.z);}
   friend Number3  operator* (C Number  &r, C Number3 &n) {return Number3(r*n.x, r*n.y, r*n.z);}
   friend Number3  operator/ (C Number  &r, C Number3 &n) {return Number3(r/n.x, r/n.y, r/n.z);}
   friend Number3  operator+ (C Number3 &a, C Number3 &b) {return Number3(a.x+b.x, a.y+b.y, a.z+b.z);}
   friend Number3  operator- (C Number3 &a, C Number3 &b) {return Number3(a.x-b.x, a.y-b.y, a.z-b.z);}
   friend Number3  operator* (C Number3 &a, C Number3 &b) {return Number3(a.x*b.x, a.y*b.y, a.z*b.z);}
   friend Number3  operator/ (C Number3 &a, C Number3 &b) {return Number3(a.x/b.x, a.y/b.y, a.z/b.z);}

   Number length   ()C; // get         length
   Number length2  ()C; // get squared length
   Number normalize() ; // normalize   length and return previous length

   Number3() {}
   Number3(  Dbl      r                           ) {set(r      );}
   Number3(  Dbl      x ,   Dbl     y,   Dbl     z) {set(x, y, z);}
   Number3(C Number  &x , C Number &y, C Number &z) {set(x, y, z);}
   Number3(C Number2 &n2,              C Number &z) {set(n2  , z);}
};
/******************************************************************************/
inline Number Dot(C Number2 &a, C Number2 &b) {return a.x*b.x + a.y*b.y          ;}
inline Number Dot(C Number3 &a, C Number3 &b) {return a.x*b.x + a.y*b.y + a.z*b.z;}
/******************************************************************************/
