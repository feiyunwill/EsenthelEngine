/******************************************************************************/
/******************************************************************************/
class EditTextStyle : TextStyle
{
   UID       font;
   TimeStamp shadow_time, shade_time, color_time, selection_time, align_time, size_time, space_time, font_time;

   // get
   bool newer(C EditTextStyle &src)C;
   bool equal(C EditTextStyle &src)C;

   // operations
   void newData();
   bool sync(C EditTextStyle &src);
   bool undo(C EditTextStyle &src);
   void reset();                    
   void create(C TextStyle &src, C UID &font, C TimeStamp &time=TimeStamp().getUTC());
   void copyTo(TextStyle &dest, C Project &proj)C;

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   EditTextStyle();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
