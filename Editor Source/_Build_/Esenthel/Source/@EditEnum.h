/******************************************************************************/
/******************************************************************************/
class EditEnum
{
   bool      removed;
   UID       id;
   Str       name;
   TimeStamp removed_time, name_time, order_time;

   EditEnum& setName   (C Str &name );
   EditEnum& setRemoved(bool removed);

   bool newer(C EditEnum &src)C;
   bool equal(C EditEnum &src)C;
   bool sync(C EditEnum &src);
   bool undo(C EditEnum &src);

   // io
   bool save(File &f)C;
   bool load(File &f);

public:
   EditEnum();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
