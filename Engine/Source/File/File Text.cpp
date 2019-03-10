/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define BOM_UTF_8   0xBBEF
#define BOM_UTF_8_3 0xBF

#define BOM_UTF_16  0xFEFF
/******************************************************************************/
static ENCODING LoadEncoding(File &f)
{
   if(f.size()>=2) // encoding can be present only if there are at least 2 bytes
   {
      switch(f.getUShort())
      {
         case BOM_UTF_16: return UTF_16;
         case BOM_UTF_8 : if(f.getByte()==BOM_UTF_8_3)return UTF_8; break;
      }
      f.pos(0); // if there was no byte order mark found then reset position to the start
   }
   return ANSI; // default encoding
}
static void SaveEncoding(File &f, ENCODING encoding)
{
   if(!f.size())switch(encoding) // we can save encoding only at the start of the file
   {
      case UTF_16: f.putUShort(BOM_UTF_16);                        break;
      case UTF_8 : f.putMulti (U16(BOM_UTF_8), Byte(BOM_UTF_8_3)); break;
   }
}
/******************************************************************************/
void      FileText::zero    () {fix_new_line=true; indent=INDENT_TABS; depth=0; _code=ANSI;}
FileText& FileText::del     () {_f.del(); zero(); return T;}
          FileText::FileText() {zero();}

FileText& FileText::writeMem(ENCODING encoding, Cipher *cipher)
{
   del();
  _f.writeMem(65536, cipher);
   SaveEncoding(_f, T._code=encoding);
   return T;
}
Bool FileText::write(C Str &name, ENCODING encoding, Cipher *cipher)
{
   del();
   if(_f.writeTry(name, cipher))
   {
      SaveEncoding(_f, T._code=encoding);
      return true;
   }
   return false;
}
Bool FileText::append(C Str &name, ENCODING encoding, Cipher *cipher)
{
   del();
   if(_f.readStdTry(name, cipher))if(_f.size())encoding=LoadEncoding(_f); // take encoding from the file if it has some data
   if(_f. appendTry(name, cipher))
   {
      SaveEncoding(_f, T._code=encoding);
      return true;
   }
   return false;
}
Bool FileText::read(C Str &name, Cipher *cipher)
{
   del();
   if(_f.readTry(name, cipher))
   {
     _code=LoadEncoding(_f);
      return true;
   }
   return false;
}
Bool FileText::read(C UID &id, Cipher *cipher)
{
   del();
   if(_f.readTry(id, cipher))
   {
     _code=LoadEncoding(_f);
      return true;
   }
   return false;
}
Bool FileText::read(C Str &name, Pak &pak)
{
   del();
   if(_f.readTry(name, pak))
   {
     _code=LoadEncoding(_f);
      return true;
   }
   return false;
}
Bool FileText::read(C UID &id, Pak &pak)
{
   del();
   if(_f.readTry(id, pak))
   {
     _code=LoadEncoding(_f);
      return true;
   }
   return false;
}
FileText& FileText::readMem(CPtr data, Int size, Int encoding, Cipher *cipher)
{
   del();
  _f.readMem(data, size, cipher);
  _code=LoadEncoding(_f); // load encoding always to skip potential BOM
   if(encoding>=0)_code=(ENCODING)encoding; // override encoding if it was specified
   return T;
}
/******************************************************************************/
FileText& FileText::startLine()
{
   if(indent)REP(depth)if(indent==INDENT_TABS)putChar('\t');else putText("   ");
   return T;
}
FileText& FileText::endLine()
{
   switch(_code)
   {
      case ANSI       :
      case UTF_8      :
      case UTF_8_NAKED: if(fix_new_line)_f.putUShort(0x0A0D);else _f.putByte(0x0A); break;

      case UTF_16: if(fix_new_line)_f.putUInt(0x000A000D);else _f.putUShort(0x000A); break;
   }
   return T;
}
FileText& FileText::putChar(Char8 c)
{
   switch(c)
   {
      case '\n': endLine(); break;

      case '\r': if(fix_new_line)break; // if we're fixing new line, then never write '\r' manually, otherwise continue, that's why there's no break on purpose
      default  : switch(_code)
      {
         case ANSI  : _f.putByte  (              c ); break;
         case UTF_16: _f.putUShort(Char8To16Fast(c)); break; // we can assume that Str was already initialized

         case UTF_8      :
         case UTF_8_NAKED:
         {
            U8 u=c;
            if(u<=0x7F)_f.putByte (u);
            else       _f.putMulti(Byte(0xC0 | (u>>6)), Byte(0x80 | (u&0x3F)));
         }break;
      }break;
   }
   return T;
}
FileText& FileText::putChar(Char c)
{
   switch(c)
   {
      case '\n': endLine(); break;

      case '\r': if(fix_new_line)break; // if we're fixing new line, then never write '\r' manually, otherwise continue, that's why there's no break on purpose
      default  : switch(_code)
      {
         case ANSI  : _f.putByte  (Char16To8Fast(c)); break; // we can assume that Str was already initialized
         case UTF_16: _f.putUShort(              c ); break;

         case UTF_8      :
         case UTF_8_NAKED:
         {
            U16 u=c;
            if(u<=0x07F)_f.putByte (u);else
            if(u<=0x7FF)_f.putMulti(Byte(0xC0 | (u>> 6)), Byte(0x80 | ( u    &0x3F)));else
                        _f.putMulti(Byte(0xE0 | (u>>12)), Byte(0x80 | ((u>>6)&0x3F)), Byte(0x80 | (u&0x3F)));
         }break;
      }break;
   }
   return T;
}
FileText& FileText::putText(C Str &text)
{
   Char temp[65536/SIZE(Char)]; // use Char to force alignment
   switch(_code)
   {
      case ANSI: if(!fix_new_line){Str8 t=text; _f.putN(t(), t.length());}else
      {
         Str8 t=FixNewLine(text); _f.putN(t(), t.length());
      }break;

      case UTF_16: if(!fix_new_line){_f.putN(text(), text.length());}else
      {
      #if 0 // slower
         Str t=FixNewLine(text); _f.putN(t(), t.length());
      #else // faster
         const Int buf_elms=Elms(temp)-1; // use size -1 because we may be writing 2 characters in one step for "\r\n"
         for(Int i=0, buf_pos=0; ; )
         {
            Bool end=(i>=text.length());
            if(  end || !InRange(buf_pos, buf_elms)) // if finished, or there's no more room in the buffer
            {
               if(buf_pos){_f.putN(temp, buf_pos); buf_pos=0;} // flush
               if(end)break;
            }
            U16 c=text()[i++]; // () avoids range checks
            if( c!='\r')
            if( c!='\n')temp[buf_pos++]=c;else
            {
               temp[buf_pos++]='\r';
               temp[buf_pos++]='\n';
            }
         }
      #endif
      }break;

      case UTF_8      :
      case UTF_8_NAKED:
      {
      #if 0 // slower
         FREPA(text)putChar(text[i]);
      #else // faster
         Char8 *C  buf     =(Char8*)temp;
         const Int buf_elms=SIZE(temp)/SIZE(Char8)-2; // use size -2 because we may be writing 3 characters in one step
         for(Int i=0, buf_pos=0; ; )
         {
            Bool end=(i>=text.length());
            if(  end || !InRange(buf_pos, buf_elms)) // if finished, or there's no more room in the buffer
            {
               if(buf_pos){_f.putN(buf, buf_pos); buf_pos=0;} // flush
               if(end)break;
            }
            U16 c=text()[i++]; // () avoids range checks
            if(fix_new_line)switch(c)
            {
               case '\r': continue; // use 'continue' to don't store this character
               case '\n':
               {
                  buf[buf_pos++]='\r';
                  buf[buf_pos++]='\n';
               }continue; // use 'continue' because we've already stored this character
            }
            if(c<=0x07F) buf[buf_pos++]=c;else
            if(c<=0x7FF){buf[buf_pos++]=(0xC0 | (c>> 6)); buf[buf_pos++]=(0x80 | ( c    &0x3F));}else
                        {buf[buf_pos++]=(0xE0 | (c>>12)); buf[buf_pos++]=(0x80 | ((c>>6)&0x3F)); buf[buf_pos++]=(0x80 | (c&0x3F));}
         }
      #endif
      }break;
   }
   return T;
}
FileText& FileText::putLine(C Str &text)
{
   return startLine().putText(text).endLine();
}
/******************************************************************************/
Char FileText::getChar()
{
   switch(_code)
   {
      default    : return 0;
      case UTF_16: return _f.getUShort();

      case ANSI:
      {
         Char8 c[2]; c[0]=_f.getByte();
      #if WINDOWS // Code Pages are used on Windows
         if((c[0]&0x80) && !_f.end()) // this may be a 2-character multi-byte wide char
         {
            c[1]=_f.getByte();
            wchar_t w[2]; Int size=MultiByteToWideChar(CP_ACP, 0, c, Elms(c), w, Elms(w));
            if(size==1)return w[0]; // if 2 bytes generated 1 wide char, then return it
           _f.skip(-1); // otherwise, go back the extra byte that we've read, because we want to get only 1 char at this time
         }
      #endif
         return Char8To16Fast(c[0]); // we can assume that Str was already initialized
      }break;

      case UTF_8      :
      case UTF_8_NAKED:
      {
         Byte b0=_f.getByte();
         if(b0&(1<<7))
         {
            Byte b1=(_f.getByte()&0x3F);
            if((b0&(1<<6)) && (b0&(1<<5)))
            {
               Byte b2=(_f.getByte()&0x3F);
               if(b0&(1<<4))
               {
                  Byte b3=(_f.getByte()&0x3F);
                  b0&=0x07;
                  UInt u=(b3|(b2<<6)|(b1<<12)|(b0<<18));
                  return (u<=0xFFFF) ? u : '?';
               }else
               {
                  b0&=0x0F;
                  return b2|(b1<<6)|(b0<<12);
               }
            }else
            {
               b0&=0x1F;
               return b1|(b0<<6);
            }
         }else
         {
            return b0;
         }
      }break;
   }
}
FileText& FileText::skipLine()
{
   for(; !end(); )
   {
      if(getChar()=='\n')break;
   }
   return T;
}
FileText& FileText::fullLine(Str &s)
{
   s.clear();
   for(; !end(); )
   {
      Char c=getChar();
      if(  c=='\n')break;
      if(U16(c)>=32 || c=='\t')s+=c;
   }
   return T;
}
FileText& FileText::getLine(Str &s)
{
   s.clear();
   for(Bool start=true; !end(); )
   {
      Char c=getChar();
      if(  c=='\n')break;
      if(U16(c)>=32 || c=='\t')
      {
         if(c!=' ' && c!='\t')start=false;
         if(!start)s+=c;
      }
   }
   return T;
}
FileText& FileText::getLine(Str8 &s)
{
   s.clear();
   for(Bool start=true; !end(); )
   {
      Char c=getChar();
      if(  c=='\n')break;
      if(U16(c)>=32 || c=='\t')
      {
         if(c!=' ' && c!='\t')start=false;
         if(!start)s+=c;
      }
   }
   return T;
}
FileText& FileText::getAll(Str &s)
{
   s.clear();
   Int chars=((_code==UTF_16) ? _f.left()/SIZEI(Char) : _f.left()); if(chars>0)
   {
      s.reserve(chars); switch(_code)
      {
         case UTF_16:
         {
            s._length=_f.getReturnSize(s._d.data(), chars*SIZEI(Char))/SIZEI(Char); if(s.length()!=chars)_f._ok=false;
      #if WINDOWS
         utf_16:
      #endif
            Int length=s.length(); CChar *t=s();
            FREP(length)
            {
               Char c=t[i]; if(!(U16(c)>=32 || c=='\t' || c=='\n')) // found invalid char
               {
                  Str temp; temp.reserve(length);
                  CopyFastN(temp._d.data(), t, i); temp._length+=i; // copy everything up to this point
               #if 0 // simple iteration (slower)
                  for(; ++i<length; ){Char c=t[i]; if(U16(c)>=32 || c=='\t' || c=='\n')temp._d[temp._length++]=c;}
               #else // batched copying
                  Int last_ok=i+1; for(; ++i<length; )
                  {
                     Char c=t[i]; if(!(U16(c)>=32 || c=='\t' || c=='\n'))
                     {
                        Int copy=i-last_ok; CopyFastN(temp._d.data()+temp._length, t+last_ok, copy); temp._length+=copy;
                        last_ok=i+1;
                     }
                  }
                  Int copy=length-last_ok; CopyFastN(temp._d.data()+temp._length, t+last_ok, copy); temp._length+=copy;
               #endif
                  Swap(temp, s);
                  break; 
               }
            }
         }break;

         case ANSI:
         {
            Str8 s8; s8.reserve(chars);
            Int length=s8._length=_f.getReturnSize(s8._d.data(), chars); if(length!=chars)_f._ok=false;
            Char *t=s._d.data(); CChar8 *t8=s8();
            FREP(length)
            {
               Char8 c=t8[i];
               if(U8(c)>=32 || c=='\t' || c=='\n')
               {
               #if WINDOWS // Code Pages are used on Windows
                  if(c&0x80) // this may be a 2-character multi-byte wide char
                  {
                     s._length=MultiByteToWideChar(CP_ACP, 0, t8, length, WChar(s._d.data()), chars); // let OS handle multi-byte conversion
                     goto utf_16; // we've converted to 16-bit so process as UTF-16
                  }
               #endif
                 *t++=Char8To16Fast(c); // we can assume that Str was already initialized
               }
            }
            s._length=t-s._d.data();
         }break;

         default:
         {
            Char *t=s._d.data();
            for(; !end(); )
            {
               Char c=getChar();
               if(U16(c)>=32 || c=='\t' || c=='\n')*t++=c;
            }
            s._length=t-s._d.data();
         }break;
      }
      s._d[s._length]='\0';
   }
   return T;
}
FileText& FileText::rewind()
{
   depth=0;
  _f.pos(0);
   LoadEncoding(_f); // we already know the encoding, but we need to skip the byte order mark at start, don't set it to '_code' because 'UTF_8_NAKED' may had been used
   return T;
}
Bool FileText::copy(File &dest)
{
   depth=0;
   if(!_f.pos(0))return false;
   return _f.copy(dest);
}
Char FileText::posInfo(Long pos, VecI2 &col_line)
{
   rewind();
   Char last='\0'; VecI2 cl=0;
   if(pos>=T.pos())
      for(MIN(pos, size()); ; ) // this will allow reading right after the last character
   {
      last=getChar();
      if(T.pos()>pos || !ok())break;
      if(last=='\n'){cl.x=0; cl.y++;}else
      if(last!='\r') cl.x++;
   }
   col_line=cl; return last;
}
/******************************************************************************/
}
/******************************************************************************/
void FileTextEx::get(Int   &i) {  i=getInt();}
void FileTextEx::get(Flt   &f) {  f=getFlt();}
void FileTextEx::get(Dbl   &d) {  d=getDbl();}
void FileTextEx::get(Vec2  &v) {v.x=getFlt(); v.y=getFlt();}
void FileTextEx::get(Vec   &v) {v.x=getFlt(); v.y=getFlt(); v.z=getFlt();}
void FileTextEx::get(Vec4  &v) {v.x=getFlt(); v.y=getFlt(); v.z=getFlt(); v.w=getFlt();}
void FileTextEx::get(VecI2 &v) {v.x=getInt(); v.y=getInt();}
void FileTextEx::get(VecI  &v) {v.x=getInt(); v.y=getInt(); v.z=getInt();}
void FileTextEx::get(VecI4 &v) {v.x=getInt(); v.y=getInt(); v.z=getInt(); v.w=getInt();}
void FileTextEx::get(VecB4 &v) {v.x=getInt(); v.y=getInt(); v.z=getInt(); v.w=getInt();}

Bool  FileTextEx::getBool () {return TextBool(getWord());}
Int   FileTextEx::getInt  () {return TextInt (getWord());}
UInt  FileTextEx::getUInt () {return TextUInt(getWord());}
Flt   FileTextEx::getFlt  () {return TextFlt (getWord());}
Dbl   FileTextEx::getDbl  () {return TextDbl (getWord());}
Vec2  FileTextEx::getVec2 () {Vec2  v; get(v); return v;}
Vec   FileTextEx::getVec  () {Vec   v; get(v); return v;}
Vec4  FileTextEx::getVec4 () {Vec4  v; get(v); return v;}
VecI2 FileTextEx::getVecI2() {VecI2 v; get(v); return v;}
VecI  FileTextEx::getVecI () {VecI  v; get(v); return v;}
VecI4 FileTextEx::getVecI4() {VecI4 v; get(v); return v;}
VecB4 FileTextEx::getVecB4() {VecB4 v; get(v); return v;}

C Str& FileTextEx::getWord()
{
   text.clear();
   for(Bool start=true; !end(); )
   {
      Char c=getChar();
      if( !c)break;
      if(  c=='\t' || c=='\n' || c==' ')if(start)continue;else break;
      if(U16(c)>32)
      {
         start=false;
         text+=c;
      }
   }
   return text;
}
C Str& FileTextEx::getName()
{
   text.clear();
   for(; !end(); )
   {
      Char c=getChar();
      if( !c || c=='\n')break;
      if(  c=='"')
      {
         for(; !end(); )
         {
            Char c=getChar();
            if( !c || c=='\n' || c=='"')break;
            if(U16(c)>=32 || c=='\t')text+=c;
         }
         break;
      }
   }
   return text;
}
/******************************************************************************/
Bool FileTextEx::getIn()
{
   for(; !end(); )
   {
      getWord();
      if(text.first()=='{')return true;
      if(text.first()=='}')return false;
   }
   return false;
}
void FileTextEx::getOut()
{
   for(Int depth=0; !end(); )
   {
      getWord();
      if(text.first()=='{')depth++;else
      if(text.first()=='}')if(--depth<0)break;
   }
}
Bool FileTextEx::level()
{
   for(; !end(); )
   {
      getWord();
      if(text.first()=='}')break;
      if(text.first()=='{'){getOut(); continue;}
      return true;
   }
   return false;
}
/******************************************************************************/
