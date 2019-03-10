/******************************************************************************

   Use 'DLL' class to load a dll/so file.

   'DLL' operates on DLL files on Windows platforms, and SO files on Unix platforms.

/******************************************************************************/
struct DLL // DLL/SO loader
{
   // manage
   Bool createMem ( CPtr  data, Int size); // create DLL from memory, false on fail
   Bool createFile(C Str &file          ); // create DLL from file  , false on fail
   Bool createFile(CChar *file          ); // create DLL from file  , false on fail

   // get
   Bool is     (            )C; // if  DLL is created
   Ptr  getFunc(CChar8 *name)C; // get DLL function from its 'name'

#if EE_PRIVATE
   void delForce(); // delete continuously until it's completely released
#endif
   DLL&   del(); // delete manually
  ~DLL() {del();}
   DLL();

private:
   Ptr _dll_file, _dll_mem;
   NO_COPY_CONSTRUCTOR(DLL);
};
/******************************************************************************/
