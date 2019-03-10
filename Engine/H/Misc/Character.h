/******************************************************************************/
enum CHAR_FLAG // Character Flag
{
   CHARF_DIG2 =1<<0, // if binary  digit 01
   CHARF_DIG10=1<<1, // if decimal digit 0123456789
   CHARF_DIG16=1<<2, // if hex     digit 0123456789ABCDEFabcdef
   CHARF_SIGN =1<<3, // if sign          `~!@#$%^&-+*=()[]{}<>;:'",./|\?
   CHARF_ALPHA=1<<4, // if alphabetic
   CHARF_UNDER=1<<5, // if underline
   CHARF_UP   =1<<6, // if upper case
   CHARF_SPACE=1<<7, // if space character
#if EE_PRIVATE
   CHARF_COMBINING =1<< 8, // if combining character
   CHARF_STACK     =1<< 9, // if stack on top of each other
   CHARF_FONT_SPACE=1<<10, // if adjust font spacing
#endif

   CHARF_DIG=CHARF_DIG10,
};

UInt CharFlag(Char  c); // get CHAR_FLAG
UInt CharFlag(Char8 c); // get CHAR_FLAG
/******************************************************************************/
enum CHAR_TYPE : Byte // Character Type
{
   CHART_NONE , // none/unknown
   CHART_CHAR , // alphabetic, digit or underline
   CHART_SPACE, // space
   CHART_SIGN , // symbol
};

CHAR_TYPE CharType(Char  c); // get character type
CHAR_TYPE CharType(Char8 c); // get character type
/******************************************************************************/
constexpr UInt CC4(Byte a, Byte b, Byte c, Byte d) {return a | (b<<8) | (c<<16) | (d<<24);}

Bool WhiteChar(Char c); // if char is a white char - ' ', '\t', '\n', '\r'

Char8 Char16To8(Char  c); // convert 16-bit to  8-bit character
Char  Char8To16(Char8 c); // convert  8-bit to 16-bit character

Char  CaseDown(Char  c); // return lower case 'c'
Char8 CaseDown(Char8 c); // return lower case 'c'
Char  CaseUp  (Char  c); // return upper case 'c'
Char8 CaseUp  (Char8 c); // return upper case 'c'

Int Compare(Char  a, Char  b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1
Int Compare(Char  a, Char8 b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1
Int Compare(Char8 a, Char  b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1
Int Compare(Char8 a, Char8 b, Bool case_sensitive=false); // compare characters, returns -1, 0, +1

Bool Equal(Char  a, Char  b, Bool case_sensitive=false); // if characters are the same
Bool Equal(Char  a, Char8 b, Bool case_sensitive=false); // if characters are the same
Bool Equal(Char8 a, Char  b, Bool case_sensitive=false); // if characters are the same
Bool Equal(Char8 a, Char8 b, Bool case_sensitive=false); // if characters are the same

Bool EqualCS(Char  a, Char  b); // if characters are the same, case sensitive
Bool EqualCS(Char  a, Char8 b); // if characters are the same, case sensitive
Bool EqualCS(Char8 a, Char  b); // if characters are the same, case sensitive
Bool EqualCS(Char8 a, Char8 b); // if characters are the same, case sensitive

Char RemoveAccent(Char c); // convert accented character to one without an accent, for example RemoveAccent('ą') -> 'a', if character is not accented then it will be returned without any modifications, RemoveAccent('a') -> 'a'

inline Bool HasUnicode(Char  c) {return U16(c)>=128;} // if character is a unicode character
inline Bool HasUnicode(Char8 c) {return U8 (c)>=128;} // if character is a unicode character
/******************************************************************************/
const Char8 CharNull      ='\0',
            CharTab       ='\t',
            CharLine      ='\n';
const Char  CharBullet    =L'•',
            CharDegree    =L'°',
            CharSection   =L'§',
            CharStar      =L'★',
            CharLeft      =L'←',
            CharRight     =L'→',
            CharDown      =L'↓',
            CharUp        =L'↑',
            CharLeftRight =L'↔',
            CharDownUp    =L'↕',
            CharCopyright =L'©',
            CharRegTM     =L'®',
            CharTrademark =L'™',
            Nbsp          =L' ', // non-breaking space
            FullWidthSpace=L'　',
            Ellipsis      =L'…',
            CharAlpha     =L'α',
            CharBeta      =L'β',
            CharSuper2    =L'²',
            CharSuper3    =L'³',
            CharPermil    =L'‰'; // 1/1000
/******************************************************************************/
#if EE_PRIVATE
extern Char  _Char8To16[];
extern Char8 _Char16To8[];

extern U16 _CharFlag[];

extern const Char8 Digits16[]; // '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'

INLINE Char  Char8To16Fast(Char8 c) {return WINDOWS ? _Char8To16[U8 (c)] : Char(U8(c));} // only Windows uses code pages, other platforms always have direct mapping
INLINE Char8 Char16To8Fast(Char  c) {return           _Char16To8[U16(c)]              ;}

INLINE UInt CharFlagFast(Char  c) {return _CharFlag[U16(c)];}
INLINE UInt CharFlagFast(Char8 c) {return  CharFlagFast(Char8To16Fast(c));}

Int CharInt(Char c); // get character as integer, '0'->0, '1'->1, .., 'a/A'->10, 'b/B'->11, .., ?->-1

CChar8* CharName(Char c); // get character name, sample usage: CharName(' ') -> "Space"

UInt CharFlagFast(Char8 a, Char8 b);
UInt CharFlagFast(Char  a, Char  b);
#endif
/******************************************************************************/
