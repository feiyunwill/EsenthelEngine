/******************************************************************************/
/******************************************************************************/
class EditParams : Memc<EditParam>
{
   // get
   EditParam* findParam(C UID &id                   ); 
   EditParam* findParam(C Str &name                 ); 
   EditParam* findParam(C Str &name, PARAM_TYPE type); 
 C EditParam* findParam(C UID &id                   )C;
 C EditParam* findParam(C Str &name                 )C;
 C EditParam* findParam(C Str &name, PARAM_TYPE type)C;

   EditParam* findParamInclRemoved(C Str &name                 ); // prioritize non-removed first
   EditParam* findParamInclRemoved(C Str &name, PARAM_TYPE type); // prioritize non-removed first

   uint memUsage()C;

   bool old(C TimeStamp &now=TimeStamp().getUTC())C;
   bool equal(C EditParams &src)C;
   bool newer(C EditParams &src)C;

   // operations
   void create(C EditParams &src);
   void create(C Object &src, C TimeStamp &time=TimeStamp().getUTC());
   void copyTo(Object &obj)C;
   bool sync(C EditParams &src);
   bool undo(C EditParams &src);

   // io
   bool save(File &f, cchar *game_path=null)C;
   bool load(File &f, cchar *game_path=null);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
