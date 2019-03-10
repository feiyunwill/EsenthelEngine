/******************************************************************************/
class ElmTypeNameNoSpace
{
   Str names[ELM_NUM]; // this could be 'static' but then memory leaks are reported at app exit

   ElmTypeNameNoSpace();
};
/******************************************************************************/
/******************************************************************************/
extern ElmTypeNameNoSpace ElmTypeNameNoSpaceDummy;
/******************************************************************************/
