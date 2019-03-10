/******************************************************************************/
/******************************************************************************/
class EditParam : Param
{
   UID       id;
   bool      removed;
   TimeStamp removed_time, name_time, type_time, value_time;

   // get
   bool old(C TimeStamp &now=TimeStamp().getUTC())C;
   bool equal(C EditParam &src)C;
   bool newer(C EditParam &src)C;
   uint memUsage()C;                         
   bool contains(C Str &text, Project&proj)C;

   // operations
   EditParam& create(C Param &src, C TimeStamp &time=TimeStamp().getUTC());
   void       setRemoved      (  bool   removed                     );  
   EditParam& setName         (C Str   &name                        );  
   void       clearValue      (                                     );  
   void       setBool         (  bool   v                           );  
   void       setColor        (C Color &v                           );  
   void       setValue        (  int    v                           );  
   void       setValue        (C Str   &v                           );  
   EditParam& forceBool       (  bool   v                           );  
   EditParam& forceInt        (  int    v                           );  
   EditParam& forceFlt        (  flt    v                           );  
   EditParam& forceColor      (C Color &v                           );  
   EditParam&     setAsIDArray(C MemPtr<UID> &ids                   );  
   EditParam& includeAsIDArray(C MemPtr<UID> &ids                   );  
   EditParam& setType         (PARAM_TYPE type, Enum *enum_type=null);  
   EditParam& setTypeValue    (C Param &src                         );  
   void       nameTypeValueUTC(                                     );  

   bool sync(C EditParam &src);
   bool undo(C EditParam &src);

   // io
   bool save(File &f, cchar *game_path=null)C;
   bool load(File &f, cchar *game_path=null);

public:
   EditParam();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
