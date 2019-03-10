/******************************************************************************/
#if EE_PRIVATE
namespace Edit{
/******************************************************************************/
enum SPACE_MODE
{
   SPACE_NORMAL    , // |int x|;
   SPACE_FUNC_PARAM, // void func(|int x|);
   SPACE_TEMPLATE  , // template<|int x|>
};

Symbol* GetFullSymbol(Memc<Token*> &tokens, Int &i, Str &temp, Symbol *set_parent, Memc<Symbol::Modif> &templates, Symbol* *symbol_parent=null, Int *symbol_index=null, Bool adjust_super=true); // 'i' will update only on tokens that were processed

void DetectDataTypes(Memc<SymbolDef> &symbols, Memc<SymbolDecl> &decls, Memc<Token*> &tokens, SymbolPtr parent=null); // get list of main symbols (namespace, struct/class/union, enum, typedef), remember it can also be "typedef class X{}x;" in which case 'x' is typedef for 'X' class
void   LinkDataTypes(Memc<SymbolDef> &symbols,                          Memc<Token*> &tokens);
void  DetectVarFuncs(Memc<SymbolDef> &symbols,                          Memc<Token*> &tokens); // get list of 2nd main symbols (variables, functions)

Bool IsDeclaration(Memc<Token*> &tokens, Symbol *symbol, Int i);

Bool IsVarFuncDefinition(Memc<Token*> &tokens, Int &i, Str &temp, Symbol* &symbol, Memc<Symbol::Modif> &templates, Symbol *set_parent, Symbol* *symbol_parent=null, Int *symbol_index=null);

void ParseFuncTemplate(Memc<Token*> &tokens, Int i);

Int ReadModifiers(Memc<Token*> &tokens, Int i, UInt &const_level, UInt &modifiers, Memc<Int> &new_templates, Str &temp);

void ReadVarFunc(Symbol *type, Memc<SymbolDef> &symbols, Memc<Token*> &tokens, Int &i, Int type_start, SPACE_MODE space_mode, Str &temp, Memc<Symbol::Modif> &templates, Memc<Int> &new_templates, Symbol *symbol_parent=null);

void GetFuncParams(Memc<SymbolDef> &symbols, Memc<Token*> &tokens, Int &i, Str &temp, SymbolPtr *var_func, Int var_func_token_index, Bool func_ptr, Symbol *set_parent);

Int GetSymbolStart(Memc<Token*> &tokens, Int i); // get first ';' or '{' or '}' or ':' or beginning of the file (':' because of "private/protected/public:")
/******************************************************************************/
} // namespace
#endif
/******************************************************************************/
