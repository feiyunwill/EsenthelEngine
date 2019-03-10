/******************************************************************************/
/******************************************************************************/
class EditEnums : Memc<EditEnum>
{
   enum TYPE : byte
   {
      DEFAULT, // no type
      TYPE_1 , // byte
      TYPE_2 , // ushort
      TYPE_4 , // uint
   };
   TYPE      type;
   TimeStamp type_time;

   static   int       FindI(C Memc<EditEnum> &enums, C UID &enum_id);
   static   EditEnum* Find (  Memc<EditEnum> &enums, C UID &enum_id);
   static C EditEnum* Find (C Memc<EditEnum> &enums, C UID &enum_id);

   void del();

   int       findI(C UID &enum_id)C;
   EditEnum* find (C UID &enum_id); 
 C EditEnum* find (C UID &enum_id)C;

   bool newer(C EditEnums &src)C;
   bool equal(C EditEnums &src)C;
   bool sync(C EditEnums &src);
   void undo(C EditEnums &src);
   int move(C UID &enum_id, int index);

   void copyTo(Enum &e, C Str &name)C;
   void create(C Enum &src);

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   EditEnums();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
