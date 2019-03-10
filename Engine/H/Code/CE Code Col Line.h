/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
struct CodeCol // Code Column
{
   Char       c    ; // character
   TOKEN_TYPE type ;
   VecI2      pos  ; // original position
   Int        token; // original token index

   CodeCol& set   (Char c, Int col, Int line, Int token, TOKEN_TYPE type) {T.c=c; T.pos.set(col, line); T.token=token; T.type=type; return T;}
   CodeCol& remove(                                                     ) {return set(' ', -1, -1, -1, TOKEN_REMOVE);}
};
/******************************************************************************/
struct CodeLine // Code Line
{
   VecI2         lines; // original line index range (x=from, y=to, can be invalid "y<x")
   Memc<CodeCol> cols ; // columns

   Bool          hasLine    (  Int    l)C {return l  >=lines.x && l  <=lines.y;}
   Bool          hasAnyLines(C VecI2 &l)C {return l.y>=lines.x && l.x<=lines.y;} // if has any of these lines
   CodeLine& includeLine    (  Int    l)  {if(lines.y<lines.x)lines=l;else if(l  <lines.x)lines.x=l  ;else if(l  >lines.y)lines.y=l  ;  return T;}
   CodeLine& includeLines   (C VecI2 &l)  {if(lines.y<lines.x)lines=l;else{if(l.x<lines.x)lines.x=l.x;     if(l.y>lines.y)lines.y=l.y;} return T;}

   Char       operator[](Int i)C {return InRange(i, cols) ? cols[i].c    :       '\0';}
   TOKEN_TYPE type      (Int i)C {return InRange(i, cols) ? cols[i].type : TOKEN_NONE;}

   Str asStr()C {Str s; s.reserve(cols.elms()); FREPA(cols)s+=cols[i].c; return s;}

   Int  length  ()C {return cols.elms();}
   Bool empty   ()C {REPA(cols)if(ValidType(cols[i].type))return false; return true;}
   Bool toRemove()C
   {
      Bool   has_remove=false; REPA(cols){TOKEN_TYPE t=cols[i].type; if(t==TOKEN_REMOVE)has_remove=true; if(ValidType(t))return false;}
      return has_remove;
   }

   Int findPos  (C VecI2 &pos                   )C {REPA(cols)if(cols[i].pos==pos)return i; return -1;}
   Int findToken(  Int    token, Bool start=true)C
   {
      if(token>=0)
      {
         if(start){FREPA(cols)if(cols[i].token==token)return i;}
         else     { REPA(cols)if(cols[i].token==token)return i;}
      }
      return -1;
   }

   CodeLine& remove(Int i                                            ) {cols.remove(i, true); return T;}
   CodeLine& insert(Int i,   Char c   , TOKEN_TYPE type, Int token=-1) {cols.NewAt(i).set(c, -1, -1, token, type); return T;}
   CodeLine& insert(Int i, C Str &text, TOKEN_TYPE type, Int token=-1) {FREPAD(t, text)insert(i+t        , text[t], type, token); return T;}
   CodeLine& append(         Char c   , TOKEN_TYPE type, Int token=-1) {return         insert(cols.elms(), c      , type, token);}
   CodeLine& append(       C Str &text, TOKEN_TYPE type, Int token=-1) {return         insert(cols.elms(), text   , type, token);}

   Bool operator==(C Str &text)C;
   Bool isKeyword (Int i, CChar8 *keyword)C;
   Bool starts    (Int i, CChar8 *text   )C;

   virtual ~CodeLine() {} // force virtual class to enable memory container auto-casting when extending this class with another virtual class, this is needed for 'ViewLine'
            CodeLine() {lines.set(0, -1);} // set invalid at start
};
/******************************************************************************/
struct SourceLoc // Source Location
{
   Bool file; // if true then this is a file stored on disk (using 'file_name'), if false then this is a file stored in project (using 'id')
   Str  file_name, base_name; // 'file_name' full path+name of the file, 'base_name' helper used for find functionality
   UID  id;

   static Int Compare(C SourceLoc &a, C SourceLoc &b);

   // get
   Str  asText()C;
   Bool is    ()C {return file ? file_name.is() : id.valid();}
   Bool operator==(C SourceLoc &loc)C {return (file!=loc.file) ? false : file ? EqualPath(file_name, loc.file_name) : (id==loc.id);}
   Bool operator!=(C SourceLoc &loc)C {return !(T==loc);}

   // operations
   void clear  (           ) {file=false; file_name.clear(); base_name.clear(); id.zero();}
   void setFile(C Str &name) {file=true ; file_name=Replace(name, '/', '\\'); T.id.zero(); base_name=GetBaseNoExt(file_name);}
   void setID  (C UID &id  ) {file=false; file_name.clear();                  T.id=id    ; base_name=GetBaseNoExt(asText() );}

   void replacePath(C Str &src, C Str &dest) {if(file)ReplacePath(file_name, src, dest);}

   SourceLoc(           ) {clear();}
   SourceLoc(C Str &file) {clear(); setFile(file);}
   SourceLoc(C UID &id  ) {clear(); setID  (id  );}
};
/******************************************************************************/
struct LineMap
{
   struct Section
   {
      struct Map
      {
         Int line_index;
         UID line_id;
      };

      SourceLoc src;
      Int       offset;
      Memc<Map> target_to_original; // mapping from target line (offsetted by 'offset') to original line in 'src'
   };

   Memc<Section> sections;

   void get(Int line, SourceLoc &src, Int &original_index, UID &original_id)C; // get original location
   void add(C SourceLoc &src, C Memc<CodeLine> &lines); // add new section
   void operator++(int);

   Bool load(C Str &file) {return true;} // don't delete existing data in case we're accessing the map to add a new section
};
/******************************************************************************/
Int       FindLineI(Memc<CodeLine> &code_lines, Int line);
CodeLine* FindLine (Memc<CodeLine> &code_lines, Int line);

CodeLine* FindLineCol(Memc<CodeLine> &code_lines, C VecI2 &pos                                     , Int   &cl_col);
Bool      FindLineCol(Memc<CodeLine> &code_lines, C VecI2 &pos                                     , VecI2 &cl_pos);
Bool      FindLineCol(Memc<CodeLine> &code_lines, Int token_index, Int token_line, Bool token_start, VecI2 &cl_pos);

void Remove(Memc<CodeLine> &lines, C VecI2 &start, C VecI2 &end, Bool definite);
void Clean (Memc<CodeLine> &lines);

void Write(FileText &f, C Memc<CodeLine> &lines);

void AdjustNameSymbol(Memc<CodeLine> &lines, Symbol* &src, Symbol *dest);

void Parse(Memc<CodeLine> &lines);

Bool OverwriteOnChange(File           &src, C Str &dest, Bool *changed=null);
Bool OverwriteOnChange(FileText       &src, C Str &dest, Bool *changed=null);
Bool OverwriteOnChange(XmlData        &src, C Str &dest, Bool params_in_separate_lines=false, ENCODING encoding=UTF_8);
Bool OverwriteOnChange(Memc<CodeLine> &src, C Str &dest);

Bool OverwriteOnChangeLoud(File           &src, C Str &dest);
Bool OverwriteOnChangeLoud(FileText       &src, C Str &dest);
Bool OverwriteOnChangeLoud(XmlData        &src, C Str &dest, Bool params_in_separate_lines=false, ENCODING encoding=UTF_8);
Bool OverwriteOnChangeLoud(Memc<CodeLine> &src, C Str &dest);
/******************************************************************************/
} // namespace
/******************************************************************************/
inline Int Elms(C Edit::CodeLine &cl) {return cl.cols.elms();} // specify in EE namespace
/******************************************************************************/
#endif
/******************************************************************************/
