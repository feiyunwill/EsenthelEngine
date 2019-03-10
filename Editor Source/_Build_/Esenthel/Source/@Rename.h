/******************************************************************************/
/******************************************************************************/
class Rename
{
   Str src, dest;

   Rename& set(C Str &src, C Str &dest);
   
   bool operator==(C Rename &rename)C;
   bool operator!=(C Rename &rename)C;
   
   /*enum CHECK
   {
      SAME,
      REVERSE,
      REQUIRED,
      UNKNOWN,
   }
   CHECK check(C Rename &rename)C
   {
      bool src_equal=Equal( src, rename. src, true),
          dest_equal=Equal(dest, rename.dest, true);
      if(src_equal && dest_equal)return SAME; // this is the same change

      bool src_equal_dest=Equal( src, rename.dest, true),
          dest_equal_src =Equal(dest, rename.src , true);
      if(src_equal_dest && dest_equal_src)return REVERSE; // this is a reverse change

      return (src_equal || dest_equal || src_equal_dest || dest_equal_src) ? REQUIRED : UNKNOWN; // if any of the names is used, then it is required
   }*/
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
