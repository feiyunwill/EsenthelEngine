/******************************************************************************

   Use 'Str8' to handle 8-bit per character text string management.

/******************************************************************************/
struct Str8 // Text String (8-bit per character)
{
   // get
          operator CChar8*()C {return _d.data();} // cast to CChar8*
  CChar8* operator()(      )C {return _d.data();} // get  text data
   Char8  operator[](Int  i)C;                    // get  i-th character, returns '\0' if 'i' is out of range

   Bool  is      ()C {return _length>0                     ;} // if  contains any data
   Int   length  ()C {return _length                       ;} // get current length
   Char8 first   ()C {return _length ? _d[        0] : '\0';} // get first character present in the string, '\0' if empty
   Char8 last    ()C {return _length ? _d[_length-1] : '\0';} // get last  character present in the string, '\0' if empty
   UInt  memUsage()C {return           _d.memUsage()       ;} // get memory usage

   // operations
   Str8& del       (                     ); // clear stored data and free helper memory
   Str8& clear     (                     ); // clear stored data
   Str8& insert    (Int i,   Char8 c     ); // insert 'c'    at 'i' string position
   Str8& insert    (Int i, C Str8 &text  ); // insert 'text' at 'i' string position
   Str8& remove    (Int i,   Int   num=1 ); // remove      'num' characters starting from 'i-th'
   Str8& removeLast(         Int   num=1 ); // remove last 'num' characters in the string
   Str8& trim      (Int pos, Int   length); // trim string by removing start and end, to keep only the part starting at 'pos' of 'length' length
   Str8& clip      (Int length           ); // clip current length to 'length'
   Str8& reserve   (Int length           ); // allocate enough space for the string to handle 'length' characters, if the parameter is smaller than current string length then no operation is performed
   Str8& reverse   (                     ); // reverse the order of characters
   Str8& replace   (Char8 src, Char8 dest); // replace all 'src' characters to 'dest'
   Str8& setChar   (Int   i  , Char8 c   ); // replace i-th character with 'c' (if 'i' is at the end of the string, then 'c' will be appended, if 'i' is after the end, then nothing will happen)
   Str8& tailSlash (Bool on              ); // exclude or include slash after string (if including and string is empty then slash will not be added, if excluding and string consists only of slash then it will not be removed)

   Str8& removeOuterWhiteChars(); // remove white characters at the start and end of the string

   Str8& space(Int num=1); // add a space if string isn't empty and does not end with a new line or space, 'num'=number of spaces to insert
   Str8& line (Int num=1); // add a line  if string isn't empty and does not end with a new line         , 'num'=number of lines  to insert

#if EE_PRIVATE
   void alwaysAppend(Char8 c);
   explicit Str8(C Str8 &s, Int additional_length);
#endif
   Str8(           );
   Str8(  Str8  &&s);   Str8& operator=(  Str8  &&s);
   Str8(  Char    c);   Str8& operator=(  Char    c);   Str8& operator+=(  Char    c);   Str  operator+(  Char    c)C;
   Str8(  Char8   c);   Str8& operator=(  Char8   c);   Str8& operator+=(  Char8   c);   Str8 operator+(  Char8   c)C;
   Str8( CChar   *t);   Str8& operator=( CChar   *t);   Str8& operator+=( CChar   *t);   Str  operator+( CChar   *t)C;
   Str8( CChar8  *t);   Str8& operator=( CChar8  *t);   Str8& operator+=( CChar8  *t);   Str8 operator+( CChar8  *t)C;
   Str8(C wchar_t*t);   Str8& operator=(C wchar_t*t);   Str8& operator+=(C wchar_t*t);   Str  operator+(C wchar_t*t)C;
   Str8(C Str    &s);   Str8& operator=(C Str    &s);   Str8& operator+=(C Str    &s);   Str  operator+(C Str    &s)C;
   Str8(C Str8   &s);   Str8& operator=(C Str8   &s);   Str8& operator+=(C Str8   &s);   Str8 operator+(C Str8   &s)C;
   Str8(  Bool    b);   Str8& operator=(  Bool    b);   Str8& operator+=(  Bool    b);   Str8 operator+(  Bool    b)C;
   Str8(  SByte   i);   Str8& operator=(  SByte   i);   Str8& operator+=(  SByte   i);   Str8 operator+(  SByte   i)C;
   Str8(  Int     i);   Str8& operator=(  Int     i);   Str8& operator+=(  Int     i);   Str8 operator+(  Int     i)C;
   Str8(  Long    i);   Str8& operator=(  Long    i);   Str8& operator+=(  Long    i);   Str8 operator+(  Long    i)C;
   Str8(  Byte    u);   Str8& operator=(  Byte    u);   Str8& operator+=(  Byte    u);   Str8 operator+(  Byte    u)C;
   Str8(  UInt    u);   Str8& operator=(  UInt    u);   Str8& operator+=(  UInt    u);   Str8 operator+(  UInt    u)C;
   Str8(  ULong   u);   Str8& operator=(  ULong   u);   Str8& operator+=(  ULong   u);   Str8 operator+(  ULong   u)C;
   Str8(  Flt     f);   Str8& operator=(  Flt     f);   Str8& operator+=(  Flt     f);   Str8 operator+(  Flt     f)C;
   Str8(  Dbl     d);   Str8& operator=(  Dbl     d);   Str8& operator+=(  Dbl     d);   Str8 operator+(  Dbl     d)C;
   Str8(  CPtr    p);   Str8& operator=(  CPtr    p);   Str8& operator+=(  CPtr    p);   Str8 operator+(  CPtr    p)C;
   Str8(C Vec2   &v);   Str8& operator=(C Vec2   &v);   Str8& operator+=(C Vec2   &v);   Str8 operator+(C Vec2   &v)C;
   Str8(C VecD2  &v);   Str8& operator=(C VecD2  &v);   Str8& operator+=(C VecD2  &v);   Str8 operator+(C VecD2  &v)C;
   Str8(C VecI2  &v);   Str8& operator=(C VecI2  &v);   Str8& operator+=(C VecI2  &v);   Str8 operator+(C VecI2  &v)C;
   Str8(C VecB2  &v);   Str8& operator=(C VecB2  &v);   Str8& operator+=(C VecB2  &v);   Str8 operator+(C VecB2  &v)C;
   Str8(C VecSB2 &v);   Str8& operator=(C VecSB2 &v);   Str8& operator+=(C VecSB2 &v);   Str8 operator+(C VecSB2 &v)C;
   Str8(C VecUS2 &v);   Str8& operator=(C VecUS2 &v);   Str8& operator+=(C VecUS2 &v);   Str8 operator+(C VecUS2 &v)C;
   Str8(C Vec    &v);   Str8& operator=(C Vec    &v);   Str8& operator+=(C Vec    &v);   Str8 operator+(C Vec    &v)C;
   Str8(C VecD   &v);   Str8& operator=(C VecD   &v);   Str8& operator+=(C VecD   &v);   Str8 operator+(C VecD   &v)C;
   Str8(C VecI   &v);   Str8& operator=(C VecI   &v);   Str8& operator+=(C VecI   &v);   Str8 operator+(C VecI   &v)C;
   Str8(C VecB   &v);   Str8& operator=(C VecB   &v);   Str8& operator+=(C VecB   &v);   Str8 operator+(C VecB   &v)C;
   Str8(C VecSB  &v);   Str8& operator=(C VecSB  &v);   Str8& operator+=(C VecSB  &v);   Str8 operator+(C VecSB  &v)C;
   Str8(C VecUS  &v);   Str8& operator=(C VecUS  &v);   Str8& operator+=(C VecUS  &v);   Str8 operator+(C VecUS  &v)C;
   Str8(C Vec4   &v);   Str8& operator=(C Vec4   &v);   Str8& operator+=(C Vec4   &v);   Str8 operator+(C Vec4   &v)C;
   Str8(C VecD4  &v);   Str8& operator=(C VecD4  &v);   Str8& operator+=(C VecD4  &v);   Str8 operator+(C VecD4  &v)C;
   Str8(C VecI4  &v);   Str8& operator=(C VecI4  &v);   Str8& operator+=(C VecI4  &v);   Str8 operator+(C VecI4  &v)C;
   Str8(C VecB4  &v);   Str8& operator=(C VecB4  &v);   Str8& operator+=(C VecB4  &v);   Str8 operator+(C VecB4  &v)C;
   Str8(C VecSB4 &v);   Str8& operator=(C VecSB4 &v);   Str8& operator+=(C VecSB4 &v);   Str8 operator+(C VecSB4 &v)C;
   Str8(C BStr   &s);   Str8& operator=(C BStr   &s);   Str8& operator+=(C BStr   &s);   Str  operator+(C BStr   &s)C;

   T1(TYPE) Str8(TYPE i, ENABLE_IF_ENUM(TYPE, Ptr  ) dummy=null)      : Str8(Int(i)) {}
   T1(TYPE)              ENABLE_IF_ENUM(TYPE, Str8&) operator =(TYPE i)  {T =Int(i); return T;}
   T1(TYPE)              ENABLE_IF_ENUM(TYPE, Str8&) operator+=(TYPE i)  {T+=Int(i); return T;}
   T1(TYPE)              ENABLE_IF_ENUM(TYPE, Str8 ) operator+ (TYPE i)C {return T+Int(i);}

   // io
   Bool save(File &f)C; // save string using f.putStr(T), false on fail
   Bool load(File &f) ; // load string using f.getStr(T), false on fail

#if !EE_PRIVATE
private:
#endif
   Mems<Char8> _d;
   Int         _length;

   friend struct _List;
};extern
   const Str8 S8; // Constant Empty String
/******************************************************************************/
inline Int Elms(C Str8 &str) {return str.length();}
/******************************************************************************/
// OPERATORS
/******************************************************************************/
inline Str8&& operator+(Str8 &&a,   CChar  *b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   CChar8 *b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C wchar_t*b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C Str    &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C Str8   &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Char    b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Char8   b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Bool    b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   SByte   b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Int     b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Long    b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Byte    b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   UInt    b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   ULong   b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Flt     b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   Dbl     b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a,   CPtr    b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C Vec2   &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecD2  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecI2  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecB2  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecSB2 &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecUS2 &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C Vec    &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecD   &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecI   &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecB   &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecSB  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecUS  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C Vec4   &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecD4  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecI4  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecB4  &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C VecSB4 &b) {return RValue(a+=b);}
inline Str8&& operator+(Str8 &&a, C BStr   &b) {return RValue(a+=b);}
T1(TYPE) ENABLE_IF_ENUM(TYPE, Str8&&) operator+(Str8 &&a, TYPE b) {return RValue(a+=Int(b));}
/******************************************************************************/
