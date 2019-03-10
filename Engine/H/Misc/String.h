/******************************************************************************

   Use 'Str' to handle text string management.

      Sample Usage:
         Str s="abc";   // set    's' string to "abc" text
         s+="def";      // append 's' string by "def" text                (now 's' has "abcdef" text)
         s=S+"text "+5; // set    's' string from text appended by number (now 's' has "text 5" text)

   Use 'StrLibrary' to store multiple strings in a file using least possible amount of bytes.

      'StrLibrary' works by creating first a database of common strings which may occur frequently when storing them in files.
      Later when strings are written to files, they are first checked if they're present in the database,
      if they are, then only index is stored (saving index requires less space than saving full string, which saves space).
      If the string is not present in the database then it is stored not as index but as a string.

      For example if string library was created with "abcdef" "zzzz" strings in the database:
         storing "abcdef" string in the file will require storing only index
         storing "qwerty" string in the file will require storing full string.

      Please remember that this implies the requirement that when reading previously written strings,
      you must use a 'StrLibrary' which was initialized with the exact same common strings array database
      as the 'StrLibrary' which was used for writing the strings.

/******************************************************************************/
// STRING
/******************************************************************************/
struct Str // Text String (16-bit per character)
{
   // get
         operator CChar*()C {return _d.data();} // cast to CChar*
  CChar* operator()(     )C {return _d.data();} // get  text data
   Char  operator[](Int i)C;                    // get  i-th character, returns '\0' if 'i' is out of range

   Bool is      ()C {return _length>0                     ;} // if  contains any data
   Int  length  ()C {return _length                       ;} // get current length
   Char first   ()C {return _length ? _d[        0] : '\0';} // get first character present in the string, '\0' if empty
   Char last    ()C {return _length ? _d[_length-1] : '\0';} // get last  character present in the string, '\0' if empty
   UInt memUsage()C {return           _d.memUsage()       ;} // get memory usage

   // operations
   Str& del       (                   ); // clear stored data and free helper memory
   Str& clear     (                   ); // clear stored data
   Str& insert    (Int i,   Char c    ); // insert 'c'    at 'i' string position
   Str& insert    (Int i, C Str &text ); // insert 'text' at 'i' string position
   Str& remove    (Int i,   Int  num=1); // remove      'num' characters starting from 'i-th'
   Str& removeLast(         Int  num=1); // remove last 'num' characters in the string
   Str& trim      (Int pos, Int length); // trim string by removing start and end, to keep only the part starting at 'pos' of 'length' length
   Str& clip      (Int length         ); // clip current length to 'length'
   Str& reserve   (Int length         ); // allocate enough space for the string to handle 'length' characters, if the parameter is smaller than current string length then no operation is performed
   Str& reverse   (                   ); // reverse the order of characters
   Str& replace   (Char src, Char dest); // replace all 'src' characters to 'dest'
   Str& setChar   (Int  i  , Char c   ); // replace i-th character with 'c' (if 'i' is at the end of the string, then 'c' will be appended, if 'i' is after the end, then nothing will happen)
   Str& tailSlash (Bool on            ); // exclude or include slash after string (if including and string is empty then slash will not be added, if excluding and string consists only of slash then it will not be removed)

   Str& removeOuterWhiteChars(); // remove white characters at the start and end of the string

   Str& space(Int num=1); // add a space if string isn't empty and does not end with a new line or space, 'num'=number of spaces to insert
   Str& line (Int num=1); // add a line  if string isn't empty and does not end with a new line         , 'num'=number of lines  to insert

#if EE_PRIVATE
#if WINDOWS
   ASSERT(SIZE(wchar_t)==SIZE(Char));
   operator C wchar_t*()C {return (wchar_t*)T();} // cast to C wchar_t*
#elif APPLE
   Str(NSString *s);   Str& operator=(NSString *s);   Str& operator+=(NSString *s);   Str operator+(NSString *s)C;
#endif
   void alwaysAppend(Char c);
   explicit Str(C Str8 &s, Int additional_length);
   explicit Str(C Str  &s, Int additional_length);
#endif
   Str(           );
   Str(  Str   &&s);   Str& operator=(  Str   &&s);
   Str(  Char    c);   Str& operator=(  Char    c);   Str& operator+=(  Char    c);   Str operator+(  Char    c)C;
   Str(  Char8   c);   Str& operator=(  Char8   c);   Str& operator+=(  Char8   c);   Str operator+(  Char8   c)C;
   Str( CChar   *t);   Str& operator=( CChar   *t);   Str& operator+=( CChar   *t);   Str operator+( CChar   *t)C;
   Str( CChar8  *t);   Str& operator=( CChar8  *t);   Str& operator+=( CChar8  *t);   Str operator+( CChar8  *t)C;
   Str(C wchar_t*t);   Str& operator=(C wchar_t*t);   Str& operator+=(C wchar_t*t);   Str operator+(C wchar_t*t)C;
   Str(C Str    &s);   Str& operator=(C Str    &s);   Str& operator+=(C Str    &s);   Str operator+(C Str    &s)C;
   Str(C Str8   &s);   Str& operator=(C Str8   &s);   Str& operator+=(C Str8   &s);   Str operator+(C Str8   &s)C;
   Str(  Bool    b);   Str& operator=(  Bool    b);   Str& operator+=(  Bool    b);   Str operator+(  Bool    b)C;
   Str(  SByte   i);   Str& operator=(  SByte   i);   Str& operator+=(  SByte   i);   Str operator+(  SByte   i)C;
   Str(  Int     i);   Str& operator=(  Int     i);   Str& operator+=(  Int     i);   Str operator+(  Int     i)C;
   Str(  Long    i);   Str& operator=(  Long    i);   Str& operator+=(  Long    i);   Str operator+(  Long    i)C;
   Str(  Byte    u);   Str& operator=(  Byte    u);   Str& operator+=(  Byte    u);   Str operator+(  Byte    u)C;
   Str(  UInt    u);   Str& operator=(  UInt    u);   Str& operator+=(  UInt    u);   Str operator+(  UInt    u)C;
   Str(  ULong   u);   Str& operator=(  ULong   u);   Str& operator+=(  ULong   u);   Str operator+(  ULong   u)C;
   Str(  Flt     f);   Str& operator=(  Flt     f);   Str& operator+=(  Flt     f);   Str operator+(  Flt     f)C;
   Str(  Dbl     d);   Str& operator=(  Dbl     d);   Str& operator+=(  Dbl     d);   Str operator+(  Dbl     d)C;
   Str(  CPtr    p);   Str& operator=(  CPtr    p);   Str& operator+=(  CPtr    p);   Str operator+(  CPtr    p)C;
   Str(C Vec2   &v);   Str& operator=(C Vec2   &v);   Str& operator+=(C Vec2   &v);   Str operator+(C Vec2   &v)C;
   Str(C VecD2  &v);   Str& operator=(C VecD2  &v);   Str& operator+=(C VecD2  &v);   Str operator+(C VecD2  &v)C;
   Str(C VecI2  &v);   Str& operator=(C VecI2  &v);   Str& operator+=(C VecI2  &v);   Str operator+(C VecI2  &v)C;
   Str(C VecB2  &v);   Str& operator=(C VecB2  &v);   Str& operator+=(C VecB2  &v);   Str operator+(C VecB2  &v)C;
   Str(C VecSB2 &v);   Str& operator=(C VecSB2 &v);   Str& operator+=(C VecSB2 &v);   Str operator+(C VecSB2 &v)C;
   Str(C VecUS2 &v);   Str& operator=(C VecUS2 &v);   Str& operator+=(C VecUS2 &v);   Str operator+(C VecUS2 &v)C;
   Str(C Vec    &v);   Str& operator=(C Vec    &v);   Str& operator+=(C Vec    &v);   Str operator+(C Vec    &v)C;
   Str(C VecD   &v);   Str& operator=(C VecD   &v);   Str& operator+=(C VecD   &v);   Str operator+(C VecD   &v)C;
   Str(C VecI   &v);   Str& operator=(C VecI   &v);   Str& operator+=(C VecI   &v);   Str operator+(C VecI   &v)C;
   Str(C VecB   &v);   Str& operator=(C VecB   &v);   Str& operator+=(C VecB   &v);   Str operator+(C VecB   &v)C;
   Str(C VecSB  &v);   Str& operator=(C VecSB  &v);   Str& operator+=(C VecSB  &v);   Str operator+(C VecSB  &v)C;
   Str(C VecUS  &v);   Str& operator=(C VecUS  &v);   Str& operator+=(C VecUS  &v);   Str operator+(C VecUS  &v)C;
   Str(C Vec4   &v);   Str& operator=(C Vec4   &v);   Str& operator+=(C Vec4   &v);   Str operator+(C Vec4   &v)C;
   Str(C VecD4  &v);   Str& operator=(C VecD4  &v);   Str& operator+=(C VecD4  &v);   Str operator+(C VecD4  &v)C;
   Str(C VecI4  &v);   Str& operator=(C VecI4  &v);   Str& operator+=(C VecI4  &v);   Str operator+(C VecI4  &v)C;
   Str(C VecB4  &v);   Str& operator=(C VecB4  &v);   Str& operator+=(C VecB4  &v);   Str operator+(C VecB4  &v)C;
   Str(C VecSB4 &v);   Str& operator=(C VecSB4 &v);   Str& operator+=(C VecSB4 &v);   Str operator+(C VecSB4 &v)C;
   Str(C BStr   &s);   Str& operator=(C BStr   &s);   Str& operator+=(C BStr   &s);   Str operator+(C BStr   &s)C;

   T1(TYPE) Str(TYPE i, ENABLE_IF_ENUM(TYPE, Ptr ) dummy=null)       : Str(Int(i)) {}
   T1(TYPE)             ENABLE_IF_ENUM(TYPE, Str&) operator =(TYPE i)  {T =Int(i); return T;}
   T1(TYPE)             ENABLE_IF_ENUM(TYPE, Str&) operator+=(TYPE i)  {T+=Int(i); return T;}
   T1(TYPE)             ENABLE_IF_ENUM(TYPE, Str ) operator+ (TYPE i)C {return T+Int(i);}

   // io
   Bool save(File &f)C; // save string using f.putStr(T), false on fail
   Bool load(File &f) ; // load string using f.getStr(T), false on fail

#if !EE_PRIVATE
private:
#endif
   Mems<Char> _d;
   Int        _length;

   friend struct _List;
};extern
   const Str S; // Constant Empty String
/******************************************************************************/
// STRING LIBRARY
/******************************************************************************/
struct StrLibrary // String Library, efficient solution for storing multiple strings in a file using least possible amount of bytes
{
   // manage
   void del   (                                                       ); // delete manually
   void create(C MemPtr<Str> &strings, Bool case_sensitive, Bool paths); // create library database from 'strings' array of strings, 'paths'=if treat strings as paths (they can have '/' replaced with '\')

   // get / set
   Int elms(     )C {return _elms;} // get number of strings stored in this library
   Str elm (Int i)C;                // get i-th      string  stored in this library, null on fail

   void putStr(File &f, C Str &str)C;                                                         // put 'str' string into 'f' file
   void getStr(File &f,   Str &str)C;   Str getStr(File &f)C {Str s; getStr(f, s); return s;} // get 'str' string from 'f' file

   // io
   Bool save(File &f)C; // save, false on fail
   Bool load(File &f) ; // load, false on fail

           ~StrLibrary() {del();}
            StrLibrary();
   explicit StrLibrary(C MemPtr<Str> &strings, Bool case_sensitive, Bool paths);

private:
   Bool  _case_sensitive, _paths;
   Int   _elms, _size, *_index;
   Byte *_data;

   NO_COPY_CONSTRUCTOR(StrLibrary);
};
/******************************************************************************/
// MAIN
/******************************************************************************/
inline Int Elms(C Str &str) {return str.length();}
/******************************************************************************/
// OPERATORS
/******************************************************************************/
inline Str&& operator+(Str &&a,   CChar  *b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   CChar8 *b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C wchar_t*b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C Str    &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C Str8   &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Char    b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Char8   b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Bool    b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   SByte   b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Int     b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Long    b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Byte    b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   UInt    b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   ULong   b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Flt     b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   Dbl     b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a,   CPtr    b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C Vec2   &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecD2  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecI2  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecB2  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecSB2 &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecUS2 &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C Vec    &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecD   &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecI   &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecB   &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecSB  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecUS  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C Vec4   &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecD4  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecI4  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecB4  &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C VecSB4 &b) {return RValue(a+=b);}
inline Str&& operator+(Str &&a, C BStr   &b) {return RValue(a+=b);}
T1(TYPE) ENABLE_IF_ENUM(TYPE, Str&&) operator+(Str &&a, TYPE b) {return RValue(a+=Int(b));}
/******************************************************************************/
