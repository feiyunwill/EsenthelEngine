/******************************************************************************/
namespace Edit{
/******************************************************************************/
struct Item
{
   enum TYPE : Byte
   {
      UNKNOWN,
      LIB    ,
      APP    ,
      FOLDER ,
      FILE   ,
   };
   TYPE       type;
   Str        base_name, full_name;
   Memx<Item> children;

#if EE_PRIVATE
   Item& set(TYPE type, C Str &base_name, C Str &full_name=S) {T.type=type; T.base_name=base_name; T.full_name=full_name; return T;}

   void replacePath(C Str &src, C Str &dest);

   Bool save(File &f, StrLibrary &sl)C;
   Bool load(File &f, StrLibrary &sl);
#endif

   Item() {type=UNKNOWN;}
};
#if EE_PRIVATE
Item& GetFolder(Memx<Item> &items, C Str &name);
Item& GetFile  (Memx<Item> &items, C Str &name, C Str &full_name);
#endif
/******************************************************************************/
} // namespace
/******************************************************************************/
