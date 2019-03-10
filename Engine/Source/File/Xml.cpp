/******************************************************************************/
#include "stdafx.h"
namespace EE{
#define XML_NODE_DATA_SUB_NODE 1 // if keep 'XmlNode.data' as a sub-node when converting it to 'TextNode'
/******************************************************************************/
// TEXT DATA
/******************************************************************************

   TextData stores data in C++ like syntax:

   group=
   {
      name=value
      array=[0 1 2] // the same as below
      array=
      {
         ``=0
         ``=1
         ``=2
      }
   }

   simple strings are stored without quotes:
      simple

   complex strings are stored with quotes:
      `complex string`

   special characters are encoded by preceeding them with a tilde:
      `First Line~nSecond Line`

   special characters are: '\0', '\n', '`', '~'

   TextNode can have either 1 param 'value' or multiple 'nodes' (sub values)
      it cannot have 'value' and 'nodes' set at the same time (if it does then only 'nodes' will be saved)

/******************************************************************************

   JSON:

      Example #1
         {
            "name":"value",
            "name2":"value2"
         }

      Example #2
         object= // here '=' can occur
         {
            "name":"value",
            "group":{"a":"a_val", "b":"b_val"},
            "array":["a", "b"]
         }

/******************************************************************************/
// DO NOT CHANGE !!
static const     Char8 CodeTextArray[]={'!', '@', '#', '$', '%', '^', '*', '(', ')', '[', ']', '{', '}', '|', '/', '_', '-', '+', '=', ';', ':', ',', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
static const     Byte  CodeTextIndex[]={255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 2, 3, 4, 255, 255, 7, 8, 6, 17, 21, 16, 22, 14, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 20, 19, 255, 18, 255, 255, 1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 9, 255, 10, 5, 15, 255, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 11, 13, 12, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}; ASSERT(Elms(CodeTextIndex)==256);
static constexpr Int   CodeTextElms   =Elms(CodeTextArray);
/******************************************************************************

   Binary Text Encoding is similar to Ascii85
      It Encodes:
         1 byte  -> 2 chars (or 1 char depending on byte value)
         2 bytes -> 3 chars
         3 bytes -> 4 chars
         4 bytes -> 5 chars

/******************************************************************************
void MakeCodeTextArray()
{
   Str s;
 //s+="!@#$%^&*()[]{}<>\\|/_-+=;:,. ~`'\"?"; all
   s+="!@#$%^*()[]{}|/_-+=;:,."; // removed: space, &'"<> because they're used in Xml, \ because it's used in JSON, 3 more characters had to be removed, so: ~` because of TextData and ?
   for(Int i='0'; i<='9'; i++)s+=Char8(i);
   for(Int i='a'; i<='z'; i++)s+=Char8(i);
   for(Int i='A'; i<='Z'; i++)s+=Char8(i);
   Memc<Char8> cs; FREPA(s)cs.include(s[i]);

   DEBUG_ASSERT(cs.elms()==85, S+"85 was the smallest value to represent UInt using 5 characters ("+cs.elms()+')');
   FREPA(cs)DEBUG_ASSERT(XmlString(cs[i])==cs[i], "test in case this is stored in xml");

   s.clear(); FREPA(cs){if(s.is())s+=", "; s+='\''; if(cs[i]=='\'' || cs[i]=='\\')s+='\\'; s+=cs[i]; s+='\'';}
   s.line();
   for(Int i=0; i<256; i++){if(s.is() && s.last()!='\n')s+=", "; s+=Byte(cs.find(Char8(i)));} // use Byte cast so -1 -> 255
   s.line();
   ClipSet(s);
}
void Test()
{
   REPA(CodeTextArray)DEBUG_ASSERT(CodeTextIndex[(Byte)CodeTextArray[i]]==i, "err");
}
/******************************************************************************/
#if 0
/******************************************************************************/
Str8 EncodeText(           CPtr src , Int size); // encode 'src' binary data of 'size' size, into human readable text
Bool DecodeText(C Str &src, Ptr dest, Int size); // decode       binary data                 from human readable text, false on fail

Str8 EncodeText(CPtr src, Int size)
{
   Str8 dest;
   if(Byte *s=(Byte*)src)if(size>0)
   {
   #if 0 // 64-bit version, didn't make a reduction for string length, so use 32-bit version (warning: enabling this would break compatibility with existing 32-bit encoded strings, and would require making decoder 64-bit as well, which is currently not done)
      ULong u, max;
      REP(UInt(size)/8)
      {
         for(u=*(ULong*)s, max=ULONG_MAX; ; ){dest+=CodeTextArray[u%CodeTextElms]; max/=CodeTextElms; if(!max)break; u/=CodeTextElms;}
         s+=8;
      }
      switch(size&7)
      {
         case  1: u=*      s                                     ; max=0x000000000000FF; break;
         case  2: u=*(U16*)s                                     ; max=0x0000000000FFFF; break;
         case  3: u=*(U16*)s|(s[2]<<16)                          ; max=0x00000000FFFFFF; break;
         case  4: u=*(U32*)s                                     ; max=0x000000FFFFFFFF; break;
         case  5: u=*(U32*)s|(ULong(        s[4]           )<<32); max=0x0000FFFFFFFFFF; break;
         case  6: u=*(U32*)s|(ULong(*(U16*)(s+4)           )<<32); max=0x00FFFFFFFFFFFF; break;
         case  7: u=*(U32*)s|(ULong(*(U16*)(s+4)|(s[6]<<16))<<32); max=0xFFFFFFFFFFFFFF; break;
         default: goto end;
      }
   #else
      UInt u, max;
      REP(UInt(size)/4)
      {
         for(u=*(UInt*)s, max=UINT_MAX; ; ){dest+=CodeTextArray[u%CodeTextElms]; max/=CodeTextElms; if(!max)break; u/=CodeTextElms;}
         s+=4;
      }
      switch(size&3)
      {
         case  1: u=*      s           ; max=0x0000FF; break;
         case  2: u=*(U16*)s           ; max=0x00FFFF; break;
         case  3: u=*(U16*)s|(s[2]<<16); max=0xFFFFFF; break;
         default: goto end;
      }
   #endif
      for(;;){dest+=CodeTextArray[u%CodeTextElms]; max/=CodeTextElms; if(!max)break; u/=CodeTextElms;}
   end:;
   }
   return dest;
}
Bool DecodeText(C Str &src, Ptr dest, Int size)
{
   if(Byte *d=(Byte*)dest)if(size>0)
   {
      UInt src_pos=0, u, max, mul;
      for(; size>=4; )
      {
         u=0; max=UINT_MAX; mul=1; for(;;)
         {
            U16  c=  src[src_pos++]; if(!InRange(c, CodeTextIndex)){invalid_char: *(UInt*)d=u; d+=4; size-=4; REP(size)*d++=0; return false;}
            Byte i=CodeTextIndex[c]; if(!InRange(i, CodeTextElms ))goto invalid_char;
            u+=i*mul; max/=CodeTextElms; if(!max)break; mul*=CodeTextElms;
         }
         *(UInt*)d=u; d+=4; size-=4;
      }
      Bool error=false; switch(size)
      {
         case  1: max=0x0000FF; break;
         case  2: max=0x00FFFF; break;
         case  3: max=0xFFFFFF; break;
         default: goto end;
      }
      u=0; mul=1; for(;;)
      {
         U16  c=  src[src_pos++]; if(!InRange(c, CodeTextIndex)){error=true; break;}
         Byte i=CodeTextIndex[c]; if(!InRange(i, CodeTextElms )){error=true; break;}
         u+=i*mul; max/=CodeTextElms; if(!max)break; mul*=CodeTextElms;
      }
      switch(size)
      {
         case 1: *      d=u;               break;
         case 2: *(U16*)d=u;               break;
         case 3: *(U16*)d=u; d[2]=(u>>16); break;
      }
   end:;
      if(error || src.length()!=src_pos)return false; // if there are still characters left then it means that the string is bigger than expected, leave contents on fail
   }
   return true;
}
/******************************************************************************/
void EncodeText    (CPtr src, Int size, Char *dest, Int dest_elms); // encode 'src' binary data of 'size' size, into 'dest' of 'dest_elms' as human readable text
Bool DecodeTextReal(CChar* &src, Dbl &real);

void EncodeText(CPtr src, Int size, Char *dest, Int dest_elms)
{
   if(dest && dest_elms>0)
   {
      dest_elms--; // make room for '\0'
      if(dest_elms)if(Byte *s=(Byte*)src)if(size>0)
      {
         UInt u, max;
         REP(UInt(size)/4)
         {
            for(u=*(UInt*)s, max=UINT_MAX; ; ){*dest++=CodeTextArray[u%CodeTextElms]; if(!--dest_elms)goto end; max/=CodeTextElms; if(!max)break; u/=CodeTextElms;}
            s+=4;
         }
         switch(size&3)
         {
            case  1: u=*      s           ; max=0x0000FF; break;
            case  2: u=*(U16*)s           ; max=0x00FFFF; break;
            case  3: u=*(U16*)s|(s[2]<<16); max=0xFFFFFF; break;
            default: goto end;
         }
         for(;;){*dest++=CodeTextArray[u%CodeTextElms]; if(!--dest_elms)goto end; max/=CodeTextElms; if(!max)break; u/=CodeTextElms;}
      end:;
      }
      *dest='\0';
   }
}
Bool DecodeTextReal(CChar* &src, Dbl &real)
{
   UInt u=0, max=UINT_MAX, mul=1; for(;;)
   {
      U16  c=*src            ; if(!InRange(c, CodeTextIndex))return false;
      Byte i=CodeTextIndex[c]; if(!InRange(i, CodeTextElms ))return false;
      src++; u+=i*mul; max/=CodeTextElms; if(!max)break; mul*=CodeTextElms;
   }

   U16  c=*src            ; if(!InRange(c, CodeTextIndex)){as_flt: real=(Flt&)u; return true;}
   Byte i=CodeTextIndex[c]; if(!InRange(i, CodeTextElms ))goto as_flt;

   (UInt&)real=u;

   u=i; max=UINT_MAX/CodeTextElms; mul=CodeTextElms; src++; for(; max>0; ) // already processed one char
   {
      U16  c=*src            ; if(!InRange(c, CodeTextIndex))return false;
      Byte i=CodeTextIndex[c]; if(!InRange(i, CodeTextElms ))return false;
      src++; u+=i*mul; max/=CodeTextElms; mul*=CodeTextElms;
   }
   ((UInt*)&real)[1]=u;

   return true;
}
#endif
/******************************************************************************/
#define  ERROR       '\1' // avoid '\0' because that one means end of file and success
#define  QUOTE_BEGIN '`'
#define  QUOTE_END   '`'
#define BINARY_BEGIN '<'
#define BINARY_END   '>'
#define BINARY_ZERO  '?' // optimization to store UInt(0) as only one character
#define BINARY_TRIM   0  // because binary data comes from Str and will be loaded into Str, then it will always be aligned to 2 bytes (size of wide Char), this will enable optimizations to reduce the binary size, however it works on assumption that this data will be loaded into Str, if in the future that would be changed, then binary data length may not be preserved

enum TEXT_TYPE
{
   TEXT_SIMPLE,
   TEXT_QUOTE ,
   TEXT_BINARY,
};

static Bool SimpleChar(Char c) {return CharType(c)==CHART_CHAR || c=='-' || c=='.';} // allow - and . for storing negative floats without the quotes

static TEXT_TYPE TextType(C Str &t)
{
   Bool simple=t.is(); // empty strings aren't simple and require quote
   REPA(t) // entire string needs to be checked, if encountered binary then always return it
   {
      U16 c=t()[i]; // () avoids range check
      if(!SimpleChar(c))
      {
         if(c>=32 && c<127 // ASCII characters (32 is ' ', 127 is DEL)
       //|| c=='\0' even though '\0' can be encoded in TEXT_QUOTE, it will always use 2 characters, so prefer TEXT_BINARY, because this character can occur frequently when encoding raw memory of 0.0f value
         || c=='\n' || c=='\t')simple=false;else return TEXT_BINARY;
      }
   }
   return simple ? TEXT_SIMPLE : TEXT_QUOTE;
}
static void SaveText(FileText &f, C Str &t)
{
   switch(TextType(t))
   {
      case TEXT_SIMPLE: f.putText(t); break;

      case TEXT_QUOTE:
      {
         f.putChar(QUOTE_BEGIN);
         FREPA(t)switch(Char c=t()[i]) // () avoids range check
         {
            case '\0': f.putChar('~').putChar('0'); break;
            case '\n': f.putChar('~').putChar('n'); break; // we can encode new line below normally too, but prefer this version
          //case '\t': f.putChar('~').putChar('t'); break; // we can encode tab below normally instead
            case '`' : f.putChar('~').putChar('`'); break;
            case '~' : f.putChar('~').putChar('~'); break;
            default  : if(U16(c)>=32 || c=='\t' || c=='\n')f.putChar(c); break; // '\n' here is supported as well, but prefer as "~n"
         }
         f.putChar(QUOTE_END);
      }break;

      case TEXT_BINARY:
      {
         f.putChar(BINARY_BEGIN);
         Byte *src =(Byte*)t();
         UInt  size=t.length()*SIZE(Char), u, max;
      #if BINARY_TRIM
         if(size>0 && !(t.last()&0xFF00))size--; // since data is binary packed in Str as Char, the last byte in the Str may not actually be needed, so if it's zero, then remove it
      #endif
         REP(size/4)
         {
            u=*(UInt*)src;
         #ifdef BINARY_ZERO
            if(!u)f.putChar(BINARY_ZERO);else
         #endif
               for(max=UINT_MAX; ; ){f.putChar(CodeTextArray[u%CodeTextElms]); max/=CodeTextElms; if(!max)break; u/=CodeTextElms;}
            src+=4;
         }
         switch(size&3)
         {
            case  1: u=*      src             ; max=               u            ; break; // for the last byte we can actually write only as much as we need, because this will generate 1 or 2 chars, and in both cases they will generate only 1 byte
            case  2: u=*(U16*)src             ; max=(BINARY_TRIM ? u : 0x00FFFF); break; // because data will be stored in Str with Char, it will be aligned to 2 bytes, so we can actually write only as much as we need in this case, because if we write 1..3 chars then during loading they will always be loaded as 1 wide Char (2 bytes)
            case  3: u=*(U16*)src|(src[2]<<16); max=                   0xFFFFFF ; break;
            default: goto end;
         }
         for(;;){f.putChar(CodeTextArray[u%CodeTextElms]); max/=CodeTextElms; if(!max)break; u/=CodeTextElms;}
      end:
         f.putChar(BINARY_END);
      }break;
   }
}
static Bool DecodeText(Char *src, Int src_elms, UInt &out)
{
   UInt u=0, mul=1;
   REP(src_elms)
   {
      U16  c=*src++          ; if(!InRange(c, CodeTextIndex))return false;
      Byte v=CodeTextIndex[c]; if(!InRange(v, CodeTextElms ))return false;
      u+=v*mul; mul*=CodeTextElms;
   }
   out=u; return true;
}
static Char LoadText(FileText &f, Str &t, Char c)
{
   switch(c)
   {
      default: // TEXT_SIMPLE
      {
         t=c; // we've already read the first character
         for(;;)
         {
            c=f.getChar();
            if(SimpleChar(c))t.alwaysAppend(c); // valid name char
               else break;
         }
      }break;

      case QUOTE_BEGIN: // TEXT_QUOTE
      {
         for(;;)
         {
            c=f.getChar();
            if(c==QUOTE_END)break; // end of name
            if(c=='~'      )       // special char
            {
               c=f.getChar();
               if(c=='0')t.alwaysAppend('\0');else
               if(c=='n')t.alwaysAppend('\n');else
               if(c=='t')t.alwaysAppend('\t');else // just in case it was written as special char
               if(c=='`')t.alwaysAppend('`' );else
               if(c=='~')t.alwaysAppend('~' );else
                  return ERROR; // invalid char
            }else
            if(U16(c)>=32 || c=='\t' || c=='\n')t.alwaysAppend(c);else // valid char, '\n' here is supported as well, but prefer as "~n"
            if(c!='\r')return ERROR; // skip '\r'
         }
         c=f.getChar(); // read next char after the name, so we're at the same situation as with the "simple name" case
      }break;

      case BINARY_BEGIN: // TEXT_BINARY
      {
         Char src[5]; // max chars per chunk is 5
         Int  src_elms=0;
         UInt out;
         for(;;)
         {
            c=f.getChar();
         #ifdef BINARY_ZERO
            if(c==BINARY_ZERO)
            {
               if(src_elms!=0)return ERROR; // BINARY_ZERO can occur only at the start of a chunk
               t.alwaysAppend(Char(0));
               t.alwaysAppend(Char(0));
               continue;
            }
         #endif
            if(c==BINARY_END)break;  // end of binary data
            src[src_elms++]=c;       // add to buffer
            if( src_elms==Elms(src)) // if buffer full
            {
               if(DecodeText(src, src_elms, out))
               {
                  t.alwaysAppend(Char(out&0xFFFF));
                  t.alwaysAppend(Char(out>>   16));
               }else return ERROR; // invalid input (this also handles '\0' chars)
               src_elms=0; // clear buffer
            }
         }
         if(src_elms)
         {
            if(DecodeText(src, src_elms, out)) // process leftovers
            {
                              t.alwaysAppend(Char(out&0xFFFF)); // 1..3 chars correspond to 2 bytes
               if(src_elms>=4)t.alwaysAppend(Char(out>>16   )); // 4    chars correspond to 3 bytes
            }else return ERROR; // invalid input (this also handles '\0' chars)
         }
         c=f.getChar(); // read next char after the name, so we're at the same situation as with the "simple name" case
      }break;
   }
   return c;
}
/******************************************************************************/
static Bool SimpleCharJSON(Char c) {return CharType(c)==CHART_CHAR || c=='-' || c=='.';} // JSON treats both - and . as simple chars to allow storing numbers - http://json.org/
static Char LoadTextJSON(FileText &f, Str &t, Char c)
{
   if(c=='"') // string
   {
      for(;;)
      {
         c=f.getChar();
         if(c=='\0')break; // end of file
         if(c=='"' )break; // end of string
         if(c=='\\') // special char
         {
            c=f.getChar();
            if(c=='0' )t.alwaysAppend('\0');else
            if(c=='n' )t.alwaysAppend('\n');else
            if(c=='t' )t.alwaysAppend('\t');else // just in case
            if(c=='"' )t.alwaysAppend('"' );else
            if(c=='\\')t.alwaysAppend('\\');else
            if(c=='u' || c=='U')
            {
               Byte a=CharInt(f.getChar());
               Byte b=CharInt(f.getChar());
               Byte c=CharInt(f.getChar());
               Byte d=CharInt(f.getChar());
               t.alwaysAppend(Char((a<<12)|(b<<8)|(c<<4)|d));
            }else continue; // invalid char, just skip it
         }else
         if(U16(c)>=32 || c=='\t')t.alwaysAppend(c);else // valid char
            return c; // skip '\r', invalid char (return this one)
      }
      c=f.getChar(); // read next char after the string, so we're at the same situation as with the "simple name" case
   }else // simple name
   {
      t=c; // we've already read the first character
      for(;;)
      {
         c=f.getChar();
         if(SimpleCharJSON(c))t.alwaysAppend(c);else // valid name char
         if(c!='\r')break; // skip '\r'
      }
   }
   return c;
}
/******************************************************************************/
static Bool     YAMLNameStart(Char c) {return c>' ' && c!='-';}
static Bool     YAMLName     (Char c) {return c>' ' && c!=':';}
static Char LoadYAMLName     (FileText &f, Str &t, Char c)
{
   t=c; // we've already read the first character
   for(;;)
   {
      c=f.getChar();
      if(YAMLName(c))t.alwaysAppend(c);else // valid name char
      if(c!='\r')break;
   }
   return c;
}
static Bool     YAMLValueStart(Char c) {return c> ' ';}
static Bool     YAMLValue     (Char c) {return c>=' ';}
static Char LoadYAMLValue     (FileText &f, Str &t, Char c)
{
   if(c=='"') // string
   {
      for(;;)
      {
         c=f.getChar();
      process:
         if(c=='\0')break; // end of file
         if(c=='"' )break; // end of string
         if(c=='\\') // special char
         {
            c=f.getChar();
            if(c=='0' )t.alwaysAppend('\0');else
            if(c=='n' )t.alwaysAppend('\n');else
            if(c=='t' )t.alwaysAppend('\t');else // just in case
            if(c=='"' )t.alwaysAppend('"' );else
            if(c=='\\')t.alwaysAppend('\\');else
            if(c=='u' || c=='U')
            {
               Byte a=CharInt(f.getChar());
               Byte b=CharInt(f.getChar());
               Byte c=CharInt(f.getChar());
               Byte d=CharInt(f.getChar());
               t.alwaysAppend(Char((a<<12)|(b<<8)|(c<<4)|d));
            }else
               continue; // invalid char, just skip it
         }else
         if(U16(c)>=32 || c=='\t')t.alwaysAppend(c);else // valid char
         if(c=='\n')
         {
            t.space(); for(;;){c=f.getChar(); if(c!=' ' && c!='\r')goto process;}
         }else
         if(c!='\r')return c; // skip '\r', invalid char (return this one)
      }
      c=f.getChar(); // read next char after the string, so we're at the same situation as with the "simple name" case
   }else
   if(c=='\'') // string
   {
      for(;;)
      {
         c=f.getChar();
      process2:
         if(c=='\0')break; // end of file
         if(c=='\'')
         {
            c=f.getChar();
            if(c=='\'')t.alwaysAppend('\'');else
               return c;
         }else
       /*if(c=='\\') // special char
         {
            c=f.getChar();
            if(c=='0' )t.alwaysAppend('\0');else
            if(c=='n' )t.alwaysAppend('\n');else
            if(c=='t' )t.alwaysAppend('\t');else // just in case
            if(c=='"' )t.alwaysAppend('"' );else
            if(c=='\\')t.alwaysAppend('\\');else
            if(c=='u' || c=='U')
            {
               Byte a=CharInt(f.getChar());
               Byte b=CharInt(f.getChar());
               Byte c=CharInt(f.getChar());
               Byte d=CharInt(f.getChar());
               t.alwaysAppend(Char((a<<12)|(b<<8)|(c<<4)|d));
            }else continue; // invalid char, just skip it
         }else*/
         if(U16(c)>=32 || c=='\t')t.alwaysAppend(c);else // valid char
         if(c=='\n')
         {
            t.space(); for(;;){c=f.getChar(); if(c!=' ' && c!='\r')goto process2;}
         }else
         if(c!='\r')return c; // skip '\r', invalid char (return this one)
      }
      c=f.getChar(); // read next char after the string, so we're at the same situation as with the "simple name" case
   }else // simple name
   {
      t=c; // we've already read the first character
      for(;;)
      {
         c=f.getChar();
         if(YAMLValue(c))t.alwaysAppend(c);else // valid name char
         if(c!='\r')break;
      }
   }
   return c;
}
static Char LoadYAMLInlineValue(FileText &f, Str &t, Char c, Char end)
{
   t=c; // we've already read the first character
   for(;;)
   {
      c=f.getChar();
      if(YAMLValue(c) && c!=',' && c!=end)t.alwaysAppend(c);else // valid name char
         break;
   }
   return c;
}
/******************************************************************************/
TextParam& TextParam::setValueHex(Flt value) {T.value=_TextHex(value); return T;}
TextParam& TextParam::setValueHex(Dbl value) {T.value=_TextHex(value); return T;}
#if 0
TextParam& setValuePacked(  Flt   value); // set value to 'value' in packed text format
TextParam& setValuePacked(  Dbl   value); // set value to 'value' in packed text format
TextParam& setValuePacked(C Vec2 &value); // set value to 'value' in packed text format
TextParam& setValuePacked(C Vec  &value); // set value to 'value' in packed text format
TextParam& setValuePacked(C Vec4 &value); // set value to 'value' in packed text format

TextParam& setPacked(C Str &name,   Flt   value) {return setName(name).setValuePacked(value);}
TextParam& setPacked(C Str &name,   Dbl   value) {return setName(name).setValuePacked(value);}
TextParam& setPacked(C Str &name, C Vec2 &value) {return setName(name).setValuePacked(value);}
TextParam& setPacked(C Str &name, C Vec  &value) {return setName(name).setValuePacked(value);}
TextParam& setPacked(C Str &name, C Vec4 &value) {return setName(name).setValuePacked(value);}

TextParam& TextParam::setValuePacked(  Flt   value) {T.value=_TextPacked(value); return T;}
TextParam& TextParam::setValuePacked(  Dbl   value) {T.value=_TextPacked(value); return T;}
TextParam& TextParam::setValuePacked(C Vec2 &value) {T.value=_TextPacked(value); return T;}
TextParam& TextParam::setValuePacked(C Vec  &value) {T.value=_TextPacked(value); return T;}
TextParam& TextParam::setValuePacked(C Vec4 &value) {T.value=_TextPacked(value); return T;}
#endif
/******************************************************************************/
TextNode* FindNode(MemPtr<TextNode> nodes, C Str &name, Int i)
{
   if(InRange(i, nodes))FREPAD(n, nodes) // process in order
   {
      TextNode &node=nodes[n]; if(node.name==name)
      {
         if(i==0)return &node; i--;
      }
   }
   return null;
}
TextNode& GetNode(MemPtr<TextNode> nodes, C Str &name)
{
   if(TextNode *node=FindNode(nodes, name))return *node; TextNode &node=nodes.New(); node.setName(name); return node;
}

TextNode* TextNode::findNode(C Str &name, Int i) {return FindNode(nodes, name, i);}
TextNode* TextData::findNode(C Str &name, Int i) {return FindNode(nodes, name, i);}

TextNode& TextNode::getNode(C Str &name) {return GetNode(nodes, name);}
TextNode& TextData::getNode(C Str &name) {return GetNode(nodes, name);}

TextNode::TextNode(C XmlNode &xml)
{
   set(xml.name, S);
   nodes.clear().setNum(xml.params.elms() + xml.nodes.elms() + (XML_NODE_DATA_SUB_NODE ? (xml.data.elms()!=0) : xml.data.elms()));
   Int n=0;
   FREPA(xml.params)SCAST(TextParam, nodes[n++])=xml.params[i];
   FREPA(xml.nodes )                 nodes[n++] =xml.nodes [i];
#if XML_NODE_DATA_SUB_NODE
   if(xml.data.elms())
   {
      TextNode &data=nodes[n++];
      data.nodes.setNum(xml.data.elms());
      FREPAO(data.nodes).value=xml.data[i];
   }
#else
   FREPA(xml.data)nodes[n++].value=xml.data[i];
#endif
}
XmlNode::XmlNode(C TextNode &text)
{
   clear().setName(text.name);
   Int sub_nodes=0, data_nodes=0, datas=0;
   FREPA(text.nodes)
   {
    C TextNode &src=text.nodes[i];
      if(!src.name .is  ()){data_nodes++; datas+=src.nodes.elms();}else
      if( src.nodes.elms())  sub_nodes++;
   }
   params.setNum(text.nodes.elms()-sub_nodes-data_nodes);
   nodes .setNum(                  sub_nodes           );
#if XML_NODE_DATA_SUB_NODE
   data  .setNum(datas                                 );
#else
   data  .setNum(                            data_nodes);
#endif
   Int p=0, n=0, d=0;
   FREPA(text.nodes)
   {
    C TextNode &src=text.nodes[i];
   #if XML_NODE_DATA_SUB_NODE
      if(!src.name.is())
      {
         FREPA(src.nodes)data[d++]=src.nodes[i].value;
      }else
   #else
      if(!src.name.is())
      {
         data[d++]=src.value;
      }else
   #endif
      if(src.nodes.elms())
      {
         nodes[n++]=src;
      }else
      {
         SCAST(TextParam, params[p++])=src;
      }
   }
}
TextData::TextData(C  XmlData & xml) {nodes.setNum( xml.nodes.elms()); FREPAO(nodes)= xml.nodes[i];}
 XmlData:: XmlData(C TextData &text) {nodes.setNum(text.nodes.elms()); FREPAO(nodes)=text.nodes[i];}
/******************************************************************************/
static Bool EmptyNames(C Memc<TextNode> &nodes) // this can ignore checking for children because it's not necessary
{
   REPA(nodes)if(nodes[i].name.is())return false;
   return true;
}
static Bool HasChildren(C Memc<TextNode> &nodes)
{
   REPA(nodes)if(nodes[i].nodes.elms())return true;
   return false;
}
/******************************************************************************/
Bool TextNode::save(FileText &f, Bool just_values)C
{
   if(!just_values)
   {
      f.startLine(); SaveText(f, name);
   }
   if(value.is() || nodes.elms())
   {
      if(!just_values)f.putChar('=');
      if(!nodes.elms())SaveText(f, value);else // just 'value' is present (save this only when there are no nodes, because they have the priority)
      if(EmptyNames(nodes)) // store just the values
      {
         Bool has_children=HasChildren(nodes);
         if(  has_children)f.endLine().startLine().depth++;else
         if(  just_values )f.endLine().startLine();
         f.putChar('[');
         Bool after_elm=false;
         FREPA(nodes)
         {
          C TextNode &node=nodes[i];
            if(node.nodes.elms()) // this node has children
            {
               after_elm=false;
            }else
            {
               if(after_elm)f.putChar(' ');
               else        {if(has_children)f.endLine().startLine(); after_elm=true;}
            }
            if(!node.save(f, true))return false;
         }
         if(has_children){f.endLine(); f.depth--; f.startLine();}
         f.putChar(']');
      }else
      {
         f.endLine().startLine().putChar('{').endLine(); f.depth++;
         FREPA(nodes)if(!nodes[i].save(f, false))return false;
         f.depth--; f.startLine().putChar('}');
      }
   }else
   if(just_values)SaveText(f, S); // we're storing values, so we need to store something

   if(!just_values)f.endLine();
   return f.ok();
}
/******************************************************************************/
Char TextNode::load(FileText &f, Bool just_values, Char c)
{
   if(just_values)goto get_value;
   c=LoadText(f, name, c);
   for(; WhiteChar(c); c=f.getChar()); // skip white chars after name
   if(c=='=') // has value specified
   {
      for(c=f.getChar(); WhiteChar(c); c=f.getChar()); // skip white chars after '='
   get_value:
      if(c=='{') // children
      {
         for(c=f.getChar(); ; )
         {
            if(SimpleChar(c) || c==QUOTE_BEGIN || c==BINARY_BEGIN)c=nodes.New().load(f, false, c);else
            if( WhiteChar(c)){c=f.getChar();       }else
            if(c=='}'       ){c=f.getChar(); break;}else
                             {c=      ERROR; break;}
         }
      }else
      if(c=='[') // values
      {
         for(c=f.getChar(); ; )
         {
            if(SimpleChar(c) || c==QUOTE_BEGIN || c==BINARY_BEGIN || c=='{' || c=='[')c=nodes.New().load(f, true, c);else
            if( WhiteChar(c)){c=f.getChar();       }else
            if(c==']'       ){c=f.getChar(); break;}else
                             {c=      ERROR; break;}
         }
      }else
      if(SimpleChar(c) || c==QUOTE_BEGIN || c==BINARY_BEGIN) // value
         c=LoadText(f, value, c);
   }
   return c;
}
Char TextNode::loadJSON(FileText &f, Bool just_values, Char c)
{
   if(just_values)goto get_value;
   c=LoadTextJSON(f, name, c);
   for(; WhiteChar(c); c=f.getChar()); // skip white chars after name
   if(c==':' || c=='=') // has value specified
   {
      for(c=f.getChar(); WhiteChar(c); c=f.getChar()); // skip white chars after ':'
   get_value:;
      if(c=='{') // children
      {
         for(c=f.getChar(); c; )
         {
            if(c=='}'){c=' '; break;} // eat this char (replace it with white char, but not '\0' because that's end of file)
            if(SimpleCharJSON(c) || c=='"')c=nodes.New().loadJSON(f, false, c);else c=f.getChar();
         }
      }else
      if(c=='[') // values
      {
         for(c=f.getChar(); c; )
         {
            if(c==']'){c=' '; break;} // eat this char (replace it with white char, but not '\0' because that's end of file)
            if(SimpleCharJSON(c) || c=='"' || c=='{' || c=='[')c=nodes.New().loadJSON(f, true, c);else c=f.getChar();
         }
      }else
      if(SimpleCharJSON(c) || c=='"') // value
         c=LoadTextJSON(f, value, c);
   }
   return c;
}
Char TextNode::loadYAML(FileText &f, Bool just_values, Char c, const Int node_spaces, Int &cur_spaces)
{
   if(just_values)goto just_values;
   c=LoadYAMLName(f, name, c);
   for(; c==' ' || c=='\r'; c=f.getChar()); // skip spaces after name
   if(c==':') // has value specified
   {
      for(c=f.getChar(); c==' ' || c=='\r'; c=f.getChar()); // skip spaces after ':'
      if(c=='{') // inline children
         for(;;)
      {
         c=f.getChar();
      process_a:
         if(c==' ' || c=='\n' || c=='\r'){}else
         if(c==','                      )nodes.New();else
         if(c=='}'                      ){c=f.getChar(); break;}else
         if(YAMLNameStart(c))
         {
            TextNode &child=nodes.New();
            c=LoadYAMLName(f, child.name, c);
            for(; c==' ' || c=='\r'; c=f.getChar()); // skip spaces after name
            if(c==':') // has value specified
            {
               for(c=f.getChar(); c==' ' || c=='\r'; c=f.getChar()); // skip spaces after ':'
               if(YAMLValueStart(c))c=LoadYAMLInlineValue(f, child.value, c, '}');
            }
            if(c==',')c=' '; // eat this
            goto process_a;
         }else
            return '\0'; // fail
      }else
      if(c=='[') // inline values
         for(;;)
      {
         c=f.getChar();
      process_b:
         if(c==' ' || c=='\n' || c=='\r'){}else
         if(c==',')nodes.New();else
         if(c==']'){c=f.getChar(); break;}else
         if(YAMLValueStart(c))
         {
            c=LoadYAMLInlineValue(f, nodes.New().value, c, ']'); if(c==',')c=f.getChar();
            goto process_b;
         }else
            return '\0'; // fail
      }else
      if(YAMLValueStart(c)) // value
         c=LoadYAMLValue(f, value, c);
      else
      if(c=='\n') // potential children nodes, or end of this node
      {
         cur_spaces=0;
      just_values:
         for(c=f.getChar(); ; )
         {
            if(c==' ' ){cur_spaces++;               c=f.getChar();}else
            if(c=='\n'){cur_spaces=0;               c=f.getChar();}else
            if(c=='#' ){cur_spaces=0; f.skipLine(); c=f.getChar();}else // comment
            if(c=='\r'){                            c=f.getChar();}else
            if(YAMLNameStart(c))
            {
               if(cur_spaces>node_spaces)
               {
                  c=nodes.New().loadYAML(f, false, c, cur_spaces, cur_spaces);
               }else return c;
            }else
            if(c=='-')
            {
               if(cur_spaces+2>node_spaces && !just_values) // "!just_values" means that we're not going to add another list element to existing nameless element, instead we need to return to parent, and create another nameless element
               {
                  cur_spaces+=2;
                  c=f.getChar(); if(c==' ')c=nodes.New().loadYAML(f, true, c, cur_spaces-2, cur_spaces);else return '\0';
               }else return c;
            }else
               break;
         }
      }
   }
   return c;
}
/******************************************************************************/
Bool TextData::save(FileText &f)C
{
   FREPA(nodes)if(!nodes[i].save(f, false))return false;
   return f.ok();
}
Bool TextData::save(C Str &name, ENCODING encoding, INDENT indent, Cipher *cipher)C
{
   FileText f; if(f.write(name, encoding, cipher)){f.indent=indent; if(save(f) && f.flush())return true; /*f.del(); FDelFile(name);*/} // no need to delete incomplete text files, because they can be still readable, just partially
   return false;
}
Bool TextData::load(FileText &f)
{
   clear();
   for(Char c=f.getChar(); ; )
   {
      if(SimpleChar(c) || c==QUOTE_BEGIN || c==BINARY_BEGIN)c=nodes.New().load(f, false, c);else
      if( WhiteChar(c))c=f.getChar();else
      if(!c)return true ;else // don't check for 'f.ok' because methods stop on null char and not on 'f.end'
            return false;
   }
}
Bool TextData::loadJSON(FileText &f)
{
   clear();
   for(Char c=f.getChar(); ; )
   {
      if(SimpleCharJSON(c) || c=='"' || c=='{' || c=='[')c=nodes.New().loadJSON(f, c=='{' || c=='[', c);else
      if( WhiteChar(c))c=f.getChar();else
      if(!c)return true ;else // don't check for 'f.ok' because methods stop on null char and not on 'f.end'
            return false;
   }
}
Bool TextData::loadYAML(FileText &f)
{
   clear();
   Int spaces=0;
   for(Char c=f.getChar(); ; )
   {
      if(c==' ' ){spaces++;               c=f.getChar();}else
      if(c=='\n'){spaces=0;               c=f.getChar();}else
      if(c=='#' ){spaces=0; f.skipLine(); c=f.getChar();}else // comment
      if(c=='-' ){spaces=0; f.skipLine(); c=f.getChar();}else
      if(c=='\r'){                        c=f.getChar();}else
      if(YAMLNameStart(c))c=nodes.New().loadYAML(f, false, c, spaces, spaces);else
      if(!c)return true ;else // don't check for 'f.ok' because methods stop on null char and not on 'f.end'
            return false;
   }
}
Bool TextData::load(C Str &name, Cipher *cipher)
{
   FileText f; if(f.read(name, cipher))return load(f);
   clear(); return false;
}
Bool TextData::load(C UID &id, Cipher *cipher)
{
   FileText f; if(f.read(id, cipher))return load(f);
   clear(); return false;
}
Bool TextData::loadJSON(C Str &name, Cipher *cipher)
{
   FileText f; if(f.read(name, cipher))return loadJSON(f);
   clear(); return false;
}
Bool TextData::loadYAML(C Str &name, Cipher *cipher)
{
   FileText f; if(f.read(name, cipher))return loadYAML(f);
   clear(); return false;
}
/******************************************************************************/
// XML DATA
/******************************************************************************/
XmlNode* FindNode(MemPtr<XmlNode> nodes, C Str &name, Int i)
{
   if(InRange(i, nodes))FREPAD(n, nodes) // process in order
   {
      XmlNode &node=nodes[n]; if(node.name==name)
      {
         if(i==0)return &node; i--;
      }
   }
   return null;
}
XmlNode& GetNode(MemPtr<XmlNode> nodes, C Str &name)
{
   if(XmlNode *node=FindNode(nodes, name))return *node; return nodes.New().setName(name);
}

XmlParam* XmlNode::findParam(C Str &name, Int i)
{
   if(InRange(i, params))FREPAD(n, params) // process in order
   {
      XmlParam &param=params[n]; if(param.name==name)
      {
         if(i==0)return &param; i--;
      }
   }
   return null;
}
XmlNode* XmlNode::findNode(C Str &name, Int i) {return FindNode(nodes, name, i);}
XmlNode* XmlData::findNode(C Str &name, Int i) {return FindNode(nodes, name, i);}

XmlParam& XmlNode::getParam(C Str &name) {if(XmlParam *par=findParam(name))return *par; return params.New().setName(name);}
XmlNode & XmlNode::getNode (C Str &name) {return GetNode(nodes, name);}
XmlNode & XmlData::getNode (C Str &name) {return GetNode(nodes, name);}
/******************************************************************************/
static Str XmlParamString(C Str &str)
{
   Str temp; temp.reserve(str.length());
   FREPA(str)
   {
      Char c=str()[i]; // () avoids range check
      if(c=='&' )temp+="&amp;" ;else
      if(c=='<' )temp+="&lt;"  ;else
      if(c=='>' )temp+="&gt;"  ;else
    //if(c=='\'')temp+="&apos;";else we always use " to save params, so we can skip encoding ' to reduce size
      if(c=='"' )temp+="&quot;";else
                 temp+=c;
   }
   return temp;
}
static Str XmlDataString(C Str &str)
{
   Str temp; temp.reserve(str.length());
   FREPA(str)
   {
      Char c=str()[i]; // () avoids range check
      if(c=='&' )temp+="&amp;" ;else
      if(c=='<' )temp+="&lt;"  ;else
      if(c=='>' )temp+="&gt;"  ;else
    //if(c=='\'')temp+="&apos;";else this is not required for data so skip to reduce size
    //if(c=='"' )temp+="&quot;";else this is not required for data so skip to reduce size
                 temp+=c;
   }
   return temp;
}
Bool XmlNode::save(FileText &f, Bool params_in_separate_lines)C
{
   Str s=S+'<'+name;
   if(params_in_separate_lines)
   {
      if(params.elms())
      {
         f.putLine(s); s.clear();
         f.depth++;
         FREPA(params)f.putLine(params[i].name+"=\""+XmlParamString(params[i].value)+'"');
         f.depth--;
      }
   }else
   {
      FREPA(params)s+=S+' '+params[i].name+"=\""+XmlParamString(params[i].value)+'"';
   }

#if 0 // 'data' in different line
   if(data.elms() || nodes.elms())s+='>';else s+="/>"; f.putLine(s);
   if(data.elms() || nodes.elms())
   {
      f.depth++;
      if(data.elms()){s.clear(); FREPA(data)s.space()+=XmlDataString(data[i]); f.putLine(s);}
      FREPA(nodes)if(!nodes[i].save(f, params_in_separate_lines))return false;
      f.depth--;
      f.putLine(S+"</"+name+'>');
   }
#else // MS Visual Studio 2010 xml parser breaks if 'data' is in different line, so put data in the same line:
   if(!data.elms() && !nodes.elms())f.putLine(s+="/>");else
   {
      s+='>';
      FREPA(data)
      {
         if(i)s+=' ';
         s+=XmlDataString(data[i]);
      }
      if(!nodes.elms())f.putLine(s+=S+"</"+name+'>');else
      {
         f.putLine(s);
         f.depth++; FREPA(nodes)if(!nodes[i].save(f, params_in_separate_lines))return false;
         f.depth--;
         f.putLine(S+"</"+name+'>');
      }
   }
#endif
   return f.ok();
}
Bool XmlData::save(FileText &f, Bool params_in_separate_lines)C
{
   FREPA(nodes)if(!nodes[i].save(f, params_in_separate_lines))return false;
   return f.ok();
}
Bool XmlData::save(C Str &name, Bool params_in_separate_lines, ENCODING encoding)C
{
   FileText f; if(f.write(name, encoding)){if(save(f, params_in_separate_lines) && f.flush())return true; /*f.del(); FDelFile(name);*/} // no need to delete incomplete text files, because they can be still readable, just partially
   return false;
}
/******************************************************************************/
static Bool XmlChar(Char c) {return CharType(c)==CHART_CHAR || c=='-' || c==':' || c=='.';} // '-' is a valid char for xml node names, ':' is a valid char for xml param names ("AndroidManifest.xml" uses those), '.' is a valid char for xml param names, http://www.w3.org/TR/2000/REC-xml-20001006#NT-Name
static Char LoadXmlName(FileText &f, Str &name, Char c)
{
   for(; XmlChar(c); c=f.getChar())name+=c;
   return c;
}
static Bool LoadXmlValue(FileText &f, Str &value)
{
   Char q=f.getChar();
   for(; WhiteChar(q); q=f.getChar()); // skip white chars
   if(q!='"' && q!='\'')return false; // value (both " and ' are valid in XML Attributes - https://www.w3schools.com/xml/xml_attributes.asp)
   for(;;)
   {
      Char c=f.getChar(); if(!c)return false; // eof
      if(c==q) // end of value
      {
         value=DecodeXmlString(value);
         return true;
      }
      if(U16(c)>=32 || c=='\t')value+=c;
   }
}
static Char LoadXmlData(FileText &f, Str &data, Char c)
{
   data=c;
   for(;;)
   {
      c=f.getChar(); if(!c || WhiteChar(c) || c=='<')break;
      if(U16(c)>=32 || c=='\t' || c=='\n')data+=c;
   }
   data=DecodeXmlString(data);
   return c;
}
/******************************************************************************/
Bool XmlNode::load(FileText &f, Char first_char)
{
   // node name
   Char c=LoadXmlName(f, name, first_char);

   // node params
   for(;;)
   {
      if(WhiteChar(c))c=f.getChar();else // skip white char
      if(XmlChar(c)) // param
      {
         XmlParam &param=params.New();
         c=LoadXmlName(f, param.name, c); // load param name
         for(; WhiteChar(c); c=f.getChar()); // skip white chars
         if(c=='='){if(!LoadXmlValue(f, param.value))return false; c=' ';} // param value
      }else break;
   }
   
   // node end
   if(c=='/')return f.getChar()=='>';
   if(c!='>')return false;

   // node body
   for(;;)
   {
      Char c=f.getChar();
   next:;
      if( !c)return false; // unexpected end
      if(  c=='<') // <node ..>, </node>, <?xml version="1.0" encoding="UTF-8" ?>, <!-- Comment --> or <!Data .. >
      {
         c=f.getChar();
         if(c=='/' || c=='?')
         {
            for(;;)
            {
               Char t=f.getChar();
               if( !t     )return false; // unexpected end
               if(  t=='>')break; // tag end
            }
            if(c=='/')return true; // if this was a node end
         }else
         if(c=='!')
         {
            Bool comment=false;
            c=f.getChar();
            if(c=='-'){c=f.getChar(); if(c=='-'){c=f.getChar(); comment=true;}}
            for(Int dashes=0; ; )
            {
               if(!c                                )return false; // unexpected end
               if( c=='>' && (!comment || dashes>=2))break; // tag end
               if( c=='-'                           )dashes++;else dashes=0;
               c=f.getChar();
            }
         }else
         if(!nodes.New().load(f, c))return false;
      }else
      if(!WhiteChar(c)){c=LoadXmlData(f, data.New(), c); goto next;}
   }
   return true; // don't check for 'f.ok' because methods stop on null char and not on 'f.end'
}
/******************************************************************************/
Bool XmlData::load(FileText &f) // if file is incomplete then false is returned and partial data is available
{
   clear();
   for(; !f.end(); )
   {
      Char c=f.getChar();
      if(c=='<') // <node ..>, <?xml version="1.0" encoding="UTF-8" ?>, <!-- Comment --> or <!Data .. >
      {
         c=f.getChar();
         if(c=='?')
         {
            for(;;)
            {
               c=f.getChar();
               if(!c     )return false; // unexpected end
               if( c=='>')break; // tag end
            }
         }else
         if(c=='!')
         {
            Bool comment=false;
            c=f.getChar();
            if(c=='-'){c=f.getChar(); if(c=='-'){c=f.getChar(); comment=true;}}
            for(Int dashes=0; ; )
            {
               if(!c                                )return false; // unexpected end
               if( c=='>' && (!comment || dashes>=2))break; // tag end
               if( c=='-'                           )dashes++;else dashes=0;
               c=f.getChar();
            }
         }else
         if(!nodes.New().load(f, c))return false;
      }else if(!WhiteChar(c))return false;
   }
   return true; // don't check for 'f.ok' because methods stop on null char and not on 'f.end'
}
Bool XmlData::load(C Str &name)
{
   FileText f; if(f.read(name))return load(f);
   clear(); return false;
}
Bool XmlData::load(C UID &id)
{
   FileText f; if(f.read(id))return load(f);
   clear(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
