/******************************************************************************/
/******************************************************************************/
class ImportCodeClass : Window
{
   class Update
   {
      UID id;
      Str data;

      Update& set(C UID &id);

      static Str AsText(C Update &code);

public:
   Update();
   };

   TextNoTest   text;
   Memc<Update> updates;
   List<Update> list;
   Region       region;
   Button       yes, no;

   static void No (ImportCodeClass &is);
   static void Yes(ImportCodeClass &is);

   virtual Rect sizeLimit()C override;                
                    C Rect& rect()C;                  
   virtual ImportCodeClass& rect(C Rect&rect)override;
   bool apply();
   void import();
};
/******************************************************************************/
/******************************************************************************/
extern ImportCodeClass ImportCode;
/******************************************************************************/
