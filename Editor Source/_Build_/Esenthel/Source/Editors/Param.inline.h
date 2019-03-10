/******************************************************************************/
   template<typename TYPE> void ParamEditor::multiFunc(bool Func(ParamEditor &pe, EditObject &obj, C TYPE &user), C TYPE &user     ) {multiFunc((bool(*)(ParamEditor &pe, EditObject &obj, cptr user))Func, cptr(&user));}
/******************************************************************************/
