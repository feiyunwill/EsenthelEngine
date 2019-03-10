/******************************************************************************/
/******************************************************************************/
class PublishClass
{
   ProjectCipher cipher;
   PakProgress   progress;

   WindowIO      export_data;
   Edit::EXE_TYPE export_data_exe;

   static void ExportData(C Str &name, PublishClass &publish);

   void create();
   void exportData(Edit::EXE_TYPE exe=Edit::EXE_EXE);

public:
   PublishClass();
};
/******************************************************************************/
/******************************************************************************/
extern PublishClass Publish;
/******************************************************************************/
