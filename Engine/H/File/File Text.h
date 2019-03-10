/******************************************************************************

   Use 'FileText' to handle text files management.

/******************************************************************************/
enum ENCODING : Byte // text encoding mode
{
   ANSI       , //        8-bit per character
   UTF_16     , //       16-bit per character
   UTF_8      , // variable-bit per character
   UTF_8_NAKED, // variable-bit per character with no Byte Order Mark (BOM)
};
enum INDENT : Byte // indentation mode
{
   INDENT_NONE  , // disabled
   INDENT_TABS  , // use tabs
   INDENT_SPACES, // use spaces
};
/******************************************************************************/
struct FileText
{
   Int    depth       ; // depth level which affects indentation when using 'startLine' and 'putLine' methods
   Bool   fix_new_line; // if replace "\n" characters with "\r\n" which are required in Windows operating system, default=true
   INDENT indent      ; // indentation mode, default=INDENT_TABS

   // manage
   FileText&   del   (                                                                              ); // delete manually
   Bool       read   (C Str &name                               , const_mem_addr Cipher *cipher=null); // read  from     file  , 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool       read   (C UID &id                                 , const_mem_addr Cipher *cipher=null); // read  from     file  , 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool       read   (C Str &name, Pak &pak                                                         ); // read  from Pak file
   Bool       read   (C UID &id  , Pak &pak                                                         ); // read  from Pak file
   FileText&  readMem(  CPtr data, Int  size, Int encoding=   -1, const_mem_addr Cipher *cipher=null); // read  from memory    , 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'encoding'=encoding of the memory (-1=autodetect)
   FileText& writeMem(                   ENCODING encoding=UTF_8, const_mem_addr Cipher *cipher=null); // write to   memory    , 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool      write   (C Str &name,       ENCODING encoding=UTF_8, const_mem_addr Cipher *cipher=null); // write to   stdio file, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool      append  (C Str &name,       ENCODING encoding=UTF_8, const_mem_addr Cipher *cipher=null); // append     stdio file, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)

   // get
   Bool     end     ()C {return _f.end ();} // if  current position is at the end of the file
   Long     size    ()C {return _f.size();} // get file size
   Long     pos     ()C {return _f.pos ();} // get file position
   ENCODING encoding()C {return _code    ;} // get file encoding

   // read
   FileText& skipLine(       );                                                  // skip text until end of line found
   FileText& fullLine(Str  &s);   Str fullLine() {Str s; fullLine(s); return s;} // read text until end of line found, includes starting white chars (spaces and tabs)
   FileText&  getLine(Str  &s);   Str  getLine() {Str s;  getLine(s); return s;} // read text until end of line found, skips    starting white chars (spaces and tabs)
   FileText&  getLine(Str8 &s);                                                  // read text until end of line found, skips    starting white chars (spaces and tabs)
   FileText&  getAll (Str  &s);   Str  getAll () {Str s;  getAll (s); return s;} // read text until end of file
   Char       getChar(       );                                                  // read a single character

   // write
   FileText& startLine(            ); // write indentation according to 'indent'
   FileText&   endLine(            ); // write end of line marker
   FileText&   putChar(  Char8 c   ); // write a single character
   FileText&   putChar(  Char  c   ); // write a single character
   FileText&   putText(C Str  &text); // write              text
   FileText&   putLine(C Str  &text); // write indentation, text and end of line marker

   // operations
   FileText& rewind ();                        // reset to starting position, this can be useful when wanting to read previously written data
   Bool      ok     ()C {return _f.ok     ();} // check if no errors occurred during reading/writing. When a new file is opened this will be set to true by default, if any read/write call will fail then this will be set to false
   Bool      flush  ()  {return _f.flush  ();} // flush all buffered data to the disk, false on fail
   Bool      flushOK()  {return _f.flushOK();} // flush all buffered data to the disk and check if no other errors occurred before - "flush() && ok()", false on fail
   Bool      copy   (File &dest);              // copy the entire contents (from start to end) of this FileText into 'dest' file, including byte order mark (BOM) if present, false on fail
   Char      posInfo(Long pos, VecI2 &col_line); // get character, its column and line indexes at specified 'pos' file offset (in raw bytes), this method will start reading each character in the file from the start, until 'pos' file offset

#if EE_PRIVATE
   void zero();
#endif
   FileText();

#if !EE_PRIVATE
private:
#endif
   ENCODING _code;
   File     _f;
   NO_COPY_CONSTRUCTOR(FileText);
};
/******************************************************************************/
#if EE_PRIVATE
struct FileTextEx : FileText
{
   Str text;

   void get(Int   &i); // get Int
   void get(Flt   &f); // get Flt
   void get(Dbl   &d); // get Dbl
   void get(Vec2  &v); // get Vec2
   void get(Vec   &v); // get Vec
   void get(Vec4  &v); // get Vec4
   void get(VecI2 &v); // get VecI2
   void get(VecI  &v); // get VecI
   void get(VecI4 &v); // get VecI4
   void get(VecB4 &v); // get VecB4

   Bool  getBool (); // get Bool
   Int   getInt  (); // get Int
   UInt  getUInt (); // get UInt
   Flt   getFlt  (); // get Flt
   Dbl   getDbl  (); // get Dbl
   Vec2  getVec2 (); // get Vec2
   Vec   getVec  (); // get Vec
   Vec4  getVec4 (); // get Vec4
   VecI2 getVecI2(); // get VecI2
   VecI  getVecI (); // get VecI
   VecI4 getVecI4(); // get VecI4
   VecB4 getVecB4(); // get VecB4
 C Str&  getWord (); // get word, skips starting spaces and empty lines, stops on one of the following '\n' ' ', returns 'text'
 C Str&  getName (); // get name, skips everything until name surrounded with "" is encountered, returns 'text'

   Bool cur   (C Str &name)C {return text==name;} // if last read data is equal to 'name'
   Bool getIn (           );                      // try to get inside next '{' level
   void getOut(           );                      // get out from current   '}' level
   Bool level (           );                      // if still in current level
};
#endif
/******************************************************************************/
