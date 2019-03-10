/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
static void SetTokenSymbol(Token &token, Str &temp)
{
   if(!token.symbol)
   {
      if(token.line->source->cpp && token=="char")token.symbol.find("char8");else // treat 'char' as 'char8' on C++ files
      if(token.line->source->cpp && token=="long")token.symbol.find("int"  );else // treat 'long' as 'int  ' on C++ files
                                                  token.symbol.find(temp=token);
   }
}
/******************************************************************************/
// NAMES
/******************************************************************************/
static void MakeUnique(Str &name)
{
   name=S+"Unique "+name+' '+RandomName();
}
static Str UniqueName(C SymbolPtr &parent, Token &token, UInt &modifiers, Bool allow_namespace=false) // modifiers may have enabled MODIF_NAMELESS, MODIF_TRANSPARENT and MODIF_SKIP_SUGGESTIONS
{
   Str name; if(parent)name=parent->full_name+SEP; if(token=='{'){modifiers|=Symbol::MODIF_NAMELESS|Symbol::MODIF_TRANSPARENT|Symbol::MODIF_SKIP_SUGGESTIONS; name+=NamelessName(parent());}else name+=token;
   SymbolPtr symbol; if(symbol.find(name))if(symbol->valid)if(allow_namespace ? symbol->type!=Symbol::NAMESPACE : true){modifiers|=Symbol::MODIF_SKIP_SUGGESTIONS; MakeUnique(name);} // is 'duplicate' name
   return name;
}
static Str UniqueVarFuncName(C SymbolPtr &parent, C Str &name)
{
   Str full; if(parent)full=parent->full_name+SEP; full+=name;
   SymbolPtr symbol; if(symbol.find(full))
   {
      if((symbol->valid && symbol->type==Symbol::TYPEDEF) || symbol->valid_decl)MakeUnique(full); // create a unique name only for "valid typedefs" or "valid class forward declarations" (typedefs in order to allow multiple same typedefs, like "ASSERT" macro uses "typedef xx" making one typedef declared multiple times, also for classes because if we have a class declared, we can't override it with some var/func)
   }
   return full;
}
/******************************************************************************/
// PARSING
/******************************************************************************/
void ParseFuncTemplate(Memc<Token*> &tokens, Int i)
{
   if(InRange(i  , tokens)
   && InRange(i+1, tokens)
   && *tokens[i]=='>' && *tokens[i+1]=='(') // "<..>(" mark function templates
   {
      for(Int level=0, j=i-1; j>=0; j--)
      {
         Token &token=*tokens[j];
         switch(token[0])
         {
            case '}': case '{': case ';': return;
            case ')': case ']': case '>': level--; break;
            case '(': case '[': case '<': level++; if(level>0)
            {
               if(token[0]=='<')
               {
                  for(; j<=i; j++)
                  {
                     Token &token=*tokens[j];
                     if(token=='<')token.BStr::setBorrowed(TMPL_B);else
                     if(token=='>')token.BStr::setBorrowed(TMPL_E);
                  }
               }
            }return;
         }
      }
   }
}
/******************************************************************************/
static void ParseTemplates(Memc<Token*> &tokens, Int &i, Str &temp, Symbol *set_parent, Bool inside_template) // 'inside_template'=if we're already inside templates <..|..>
{
   Memc<Symbol::Modif> templates;
   for(Int level=0, template_level=0, original=i; i<tokens.elms(); )
   {
      Int    start=i;
      Token &token=*tokens[i];
      if(inside_template && !level)
      {
         if(token=='<' || token==TMPL_B)    template_level++;
         if(token=='>' || token==TMPL_E)if(!template_level--)return;
      }
      switch(token[0])
      {
         case '>':                            ParseFuncTemplate(tokens, i); break;
         case '{': case '(': case '[':     level++;                         break;
         case '}': case ')': case ']': if(!level--                 )return; break;
         case ';': case ',':           if(!level && !template_level)return; break;
         default :
         {
            Token *prev=(InRange(i-1, tokens) ? tokens[i-1] : null);
            if(!(prev && (*prev=='.' || *prev=="->" || *prev=="::"))) // if not preceeded by separator
               GetFullSymbol(tokens, i, temp, set_parent, templates);
         }break;
      }
      token.parent=set_parent;
      MAX(i, start+1);
   }
}
/******************************************************************************/
static void ReadTemplates(Memc<SymbolDef> &symbols, Memc<Token*> &tokens, Int i, Str &temp, Symbol &symbol, Bool as_template) // "template<typename TYPE>", "<TYPE>", "<const int x>", "<const int* *x>"
{
   Symbol *set_parent=&symbol;
   if(InRange(i, tokens))for(; i<tokens.elms(); )
   {
      Token &token=*tokens[i], *next=(InRange(i+1, tokens) ? tokens[i+1] : null);
      if(token=="typename" || token=="class" || token=="struct"){token.parent=set_parent; i++; continue;} // skip 'typename'

      if(token=="const_mem_addr"){token.parent=set_parent; i++; continue;} // skip modifiers

      if(token.type==TOKEN_CODE && next && (*next=='=' || *next==',' || *next=='>' || *next==TMPL_E)) // if there's only one TOKEN_CODE then treat it as 'typename'
      {
         token.symbol=symbols.New().require(symbol.full_name+SEP+token).set(&symbol, Symbol::TYPENAME, i, token.line->source); if(as_template)symbol.templates.add(token.symbol);
         token.parent=set_parent; i++; // skip name

         UInt const_level=0; Memc<Int> new_templates; ReadModifiers(tokens, i-2, const_level, token.symbol->modifiers, new_templates, temp);
      }else
      {
         // TODO:
         // for this block use new function GetFuncParam (which extract from GetFuncParams), watch out for small differences between here and GetFuncParams such as ParseTemplates has true/false parameters
         // as currently array_dims are ignored
         Int    bracket_level=0,   ptr_level=0,   ptr_level_in_bracket=0, type_start=i;
         UInt       modifiers=0, const_level=0, const_level_in_bracket=0;
         Token *par=tokens[i++]; par->parent=set_parent;
         if(*par=="const"    ){par=(InRange(i, tokens) ? tokens[i++] : null); par->parent=set_parent; const_level=1;}else
         if(*par=="constexpr"){par=(InRange(i, tokens) ? tokens[i++] : null); par->parent=set_parent;}
         if( par)
         {
            if(*par==')' || *par=='>' || *par==TMPL_E)break;
            if(*par==',')continue;

            i--; Int    start=i  ; Memc<Symbol::Modif> templates; Symbol *type=GetFullSymbol(tokens, i, temp, set_parent, templates); MAX(i, start+1);
                 Int type_end=i-1;
            for(; i<tokens.elms(); )
            {
               Int    param_token_index=i;
               Token &name=*tokens[i++]; name.parent=set_parent;
               switch(name.type)
               {
                  case TOKEN_OPERATOR:
                  {
                     switch(name[0])
                     {
                        case '*': if(bracket_level)ptr_level_in_bracket++;else ptr_level++; break;
                        case '&': modifiers|=Symbol::MODIF_REF; break;
                        case '(': bracket_level++; break;

                        case ',':
                        case ')':
                        case '>':
                        default : // TMPL_E
                        {
                           // TODO: create empty param
                           if(name[0]==',')goto finished_param;
                           return; // error
                        }break;
                     }
                  }break;

                  case TOKEN_KEYWORD:
                  {
                     if(name=="const")
                     {
                        if(bracket_level)const_level_in_bracket|=(1<<ptr_level_in_bracket);
                        else             const_level           |=(1<<ptr_level           );
                     }else
                     if(name=="long"){}else // long long
                        return; // error
                  }break;

                  case TOKEN_CODE:
                  {
                     for(; bracket_level>0 && i<tokens.elms() && *tokens[i]==')'; ){bracket_level--; tokens[i++]->parent=set_parent;} // force closing brackets started earlier "int ((x))|"
                     if( !bracket_level)
                     {
                        name.def_decl=true;
                        name.symbol =symbols.New().require(symbol.full_name+SEP+name).set(&symbol, Symbol::VAR, param_token_index, name.line->source); if(as_template)symbol.templates.add(name.symbol);
                        // TODO: when name.symbol will be as FUNC, then calculate differently
                        name.symbol->       modifiers|=(modifiers & ~Symbol::MODIF_REF); // set all except MODIF_REF
                        name.symbol->value. modifiers|=(modifiers &  Symbol::MODIF_REF); // set only       MODIF_REF
                        name.symbol->value            =type; Swap(name.symbol->value.templates, templates);
                        name.symbol->value.  ptr_level=  ptr_level+   ptr_level_in_bracket;
                        name.symbol->value.const_level=const_level|(const_level_in_bracket<<ptr_level);
                        name.symbol->type_range.set(type_start, type_end);
                        name.symbol-> def_range.set(type_end+1);

                        for(; i<tokens.elms(); )
                        {
                           Token &op=*tokens[i++]; op.parent=set_parent;
                           switch(op[0])
                           {
                              case '(': // parameter list or default value declaration
                              {
                                 // TODO:
                                 GetFuncParams(symbols, tokens, i, temp, null, param_token_index, true, set_parent);
                              }break;

                              case '[': // array
                              {
                                 for(Str dim; i<tokens.elms(); )
                                 {
                                    Token &token=*tokens[i++]; token.parent=set_parent;
                                    if(token==']')
                                    {
                                       // TODO: watch out for error in calculation of 'CalculateI' or <=0 value
                                       // TODO: this is wrong, ignores ptr_level_in_bracket
                                       if(name.symbol)name.symbol->value.array_dims.NewAt(0)=CalculateI(dim);
                                       goto finished_dim;
                                    }
                                    dim+=token;
                                 }
                                 return; // error
                              finished_dim:;
                              }break;

                              case '=': // default value
                              {
                                 // for example: 5, x, 1*(x+2), Map<Image,Int>::static_var+Mems<X>::stat_var
                                 Int start=i; ParseTemplates(tokens, i, temp, set_parent, true);
                                 if(name.symbol){name.symbol->modifiers|=Symbol::MODIF_DEF_VALUE; name.symbol->def_range.y=i-1; name.symbol->def_val_range.set(start, i-1);}
                              }break;

                              case ',':
                              {
                                 if(name.symbol)name.symbol->def_range.y=i-2;
                              }goto finished_param; // proceed to next param

                              default:
                              {
                                 if(name.symbol)name.symbol->def_range.y=i-2;
                              }return; // end of param declaration
                           }
                        }
                     }
                  }break;

                  default: return; // error
               }
            }
         }
      }
   finished_param:;
   }
}
/******************************************************************************/
static void ReadModifier(Token &token, UInt &const_level, UInt &modifiers)
{
   if(token=="const"         )const_level=1;else
   if(token=="static"        )modifiers|=Symbol::MODIF_STATIC        ;else
   if(token=="inline"        )modifiers|=Symbol::MODIF_INLINE        ;else
   if(token=="virtual"       )modifiers|=Symbol::MODIF_VIRTUAL       ;else
   if(token=="friend"        )modifiers|=Symbol::MODIF_FRIEND        ;else
   if(token=="mutable"       )modifiers|=Symbol::MODIF_MUTABLE       ;else
   if(token=="explicit"      )modifiers|=Symbol::MODIF_EXPLICIT      ;else
   if(token=="extern"        )modifiers|=Symbol::MODIF_EXTERN        ;else
   if(token=="const_mem_addr")modifiers|=Symbol::MODIF_CONST_MEM_ADDR;else
   if(token=="public"        )Symbol::SetPublic   (modifiers)        ;else
   if(token=="protected"     )Symbol::SetProtected(modifiers)        ;else
   if(token=="private"       )Symbol::SetPrivate  (modifiers)        ;
}
Int ReadModifiers(Memc<Token*> &tokens, Int i, UInt &const_level, UInt &modifiers, Memc<Int> &new_templates, Str &temp)
{
   new_templates.clear();
   for(; i>=0; i--)
   {
      Token &mod=*tokens[i];
      if(mod.type==TOKEN_KEYWORD)SetTokenSymbol(mod, temp);
      if(mod.symbol && (mod.symbol->modifiers&Symbol::MODIF_CPP_MODIFIER))ReadModifier(mod, const_level, modifiers);else
      {
         // check if there are templates
         for(; InRange(i, tokens); )
         {
            Token &t=*tokens[i];
            if(t=='>' || t==TMPL_E)
            {
               for(Int level=0, j=i; j>=0; )
               {
                  Token &token=*tokens[j--];
                  if(token=='{' || token=='}' || token==';')goto finished;
                  if(token=='>' || token==TMPL_E)    --level;else
                  if(token=='<' || token==TMPL_B)if(!++level) // we've reached the start
                  {
                     token.BStr::setBorrowed(TMPL_B);
                     new_templates.NewAt(0)=j+2; // set 'new_templates' to first token after '<'
                     if(InRange(j, tokens) && *tokens[j]=="template")j--;
                     i=j; // this template section was successfully read, so we can adjust the 'i' index
                     break;
                  }
               }
            }else break;
         }
         break;
      }
   }
finished:
   return i+1;
}
/******************************************************************************/
static void SkipUnknownSymbol(Memc<Token*> &tokens, Int &i, Symbol *set_parent) // "X", "EE::X", "StaticAssertionHelper<sizeof(StaticAssertion<static_cast<bool>((test))>)>"
{
   if(InRange(i, tokens))
   {
      Token &token_start   =*tokens[i];
      Bool   precise_parent=false;
      if(token_start=='.' || token_start=="::" || token_start=="->"){precise_parent=true; token_start.parent=set_parent; i++;}
      for(; i<tokens.elms(); )
      {
         Token &token=*tokens[i];
         switch(token.type)
         {
            default: return;

            case TOKEN_KEYWORD:
            case TOKEN_CODE   : token.parent=set_parent; i++; break;
         }
         
         if(InRange(i, tokens)) // check operator following symbol, can be . :: -> < TMPL_B
         {
            Token *op=tokens[i];
            if(*op=='<' || *op==TMPL_B)
            {
               for(Int template_level=0, bracket_level=0; i<tokens.elms(); )
               {
                  Token &token=*tokens[i];
                  if(token==';' || token=='{' || token=='}')break;

                  token.parent=set_parent; i++;

                  if(token=='(')bracket_level++;else
                  if(token==')')bracket_level--;

                  if(!bracket_level)
                  {
                     if(token=='<' || token==TMPL_B)   ++template_level;else
                     if(token=='>' || token==TMPL_E)if(--template_level<=0)break;
                  }
               }

               op=(InRange(i, tokens) ? tokens[i] : null);
            }

            if(op)
               if(*op=='.' || *op=="::" || *op=="->"){i++; op->parent=set_parent; continue;} // wishes to use sub-symbol
         }
         break;
      }
   }
}
/******************************************************************************/
Symbol* GetFullSymbol(Memc<Token*> &tokens, Int &i, Str &temp, Symbol *set_parent, Memc<Symbol::Modif> &templates, Symbol* *symbol_parent, Int *symbol_index, Bool adjust_super) // 'i' will update only on tokens that were processed
{
   Symbol *symbol=null;
   templates.clear();
   if(symbol_parent)*symbol_parent=null;
   if(symbol_index )*symbol_index =0;

   if(InRange(i, tokens))
   {
      Symbol *parent;
      Bool    precise_parent=false, allow_self=true;
      Token  &token_start   =*tokens[i], *prev=(InRange(i-1, tokens) ? tokens[i-1] : null);
      if(token_start=='.' || token_start=="::" || token_start=="->") // start with global namespace, for example ::EE::Image
      {
         if(prev)
         {
            if(prev->type==TOKEN_CODE   )return null; // can't process symbols followed by code
            if(prev->type==TOKEN_KEYWORD)if(!(prev->symbol && (prev->symbol->modifiers&Symbol::MODIF_CPP_MODIFIER)))return null; // can't process symbols followed by keywords that aren't modifiers
         }
         parent=null; precise_parent=true; token_start.parent=set_parent; i++;
      }else
      {
         if(prev && (*prev=='.' || *prev=="::" || *prev=="->"))return null; // can't process symbols followed by separators
         parent=set_parent;
      }

      for(; i<tokens.elms(); )
      {
         if(symbol_parent)*symbol_parent=symbol;

         Token &token=*tokens[i]; // read symbol name
         Int    token_index=i;
         switch(token.type)
         {
            default: symbol=null; break;

            case TOKEN_OPERATOR:
            {
               symbol=null;
               if(token=='~' && InRange(i+1, tokens)) // possible destructor declaration "class X{~X()} X::~X(){}"
               {
                  Token &name=*tokens[i+1]; // get class name
                  Int    name_index=i+1;
                  if(name.type==TOKEN_CODE)
                  {
                     if(!name.symbol && parent && name==*parent)name.symbol=parent;
                     if( name.symbol)
                     {
                        token.parent=name.parent=set_parent; i+=2;
                        allow_self=true;
                        precise_parent=true;
                        parent=symbol=name.symbol(); if(symbol_index)*symbol_index=name_index;
                     }
                  }
               }
            }break;

            case TOKEN_KEYWORD:
            {
               token.parent=set_parent; SetTokenSymbol(token, temp);
               symbol=token.symbol(); if(symbol_index)*symbol_index=token_index;
               if(adjust_super && (token=="__super" || token=="super"))
               {
                  allow_self=false;
                  symbol    =(parent ? parent : set_parent ? set_parent->Class() : null); // if "parent.super" is available then use that parent, if not then use class of current space
               }else allow_self=true;

               if(symbol && (symbol->modifiers&Symbol::MODIF_DATA_TYPE)) // check for "signed/unsigned/short/long" combination
               {
                  Int j=i;
                  for(; InRange(j+1, tokens) && tokens[j+1]->type==TOKEN_KEYWORD; j++)SetTokenSymbol(*tokens[j+1], temp); // iterate all continuous keywords, i..j range
                  for(; j>i && !(tokens[j]->symbol && (tokens[j]->symbol->modifiers&Symbol::MODIF_DATA_TYPE)); j--); // ignore last keywords which are not data type
                  if(j>i)
                  {
                     Bool signed_=false, unsigned_=false, short_=false, long_=false, char_=false, int_=false, int8_=false, int16_=false, int32_=false, int64_=false;
                     for(Int k=i; k<=j; k++)
                     {
                        Token &t=*tokens[k]; t.parent=set_parent;
                        if(t==  "signed")  signed_=true;else
                        if(t=="unsigned")unsigned_=true;else
                        if(t==     "int")     int_=true;else
                        if(t==   "short")   short_=true;else
                        if(t==    "long" && t.line->source->cpp)long_=true;else
                        if(t==    "char" && t.line->source->cpp)char_=true;else
                        if(t==  "__int8")int8_ =true;else
                        if(t== "__int16")int16_=true;else
                        if(t== "__int32")int32_=true;else
                        if(t== "__int64")int64_=true;
                     }
                     i=j;
                     if(char_ )symbol=TypeSymbol(unsigned_ ? VAR_BYTE   : signed_ ? VAR_SBYTE : VAR_CHAR8);else
                     if(long_ )symbol=TypeSymbol(unsigned_ ? VAR_UINT   : VAR_INT  );else
                     if(int8_ )symbol=TypeSymbol(unsigned_ ? VAR_BYTE   : VAR_SBYTE);else
                     if(int16_)symbol=TypeSymbol(unsigned_ ? VAR_USHORT : VAR_SHORT);else
                     if(int32_)symbol=TypeSymbol(unsigned_ ? VAR_UINT   : VAR_INT  );else
                     if(int64_)symbol=TypeSymbol(unsigned_ ? VAR_ULONG  : VAR_LONG );else
                     {
                        // TODO: compile error
                     }
                  }
               }

               i++;
               precise_parent=true;
               parent=symbol;
            }break;

            case TOKEN_CODE:
            {
               i++; token.parent=set_parent;
               if( !token.symbol)
               {
                  if(parent && token==*parent)token.symbol=parent                                                ;else // constructor "class X{X();} X::X(){}"
                  if(precise_parent && parent)token.symbol=FindChild (temp=token, parent, null, true, allow_self);else // if there's no parent then allow usings too (go to next line)
                                              token.symbol=FindSymbol(temp=token, parent, ProjectUsings         );
               }
               allow_self=true;
               precise_parent=true;
               parent=symbol=token.symbol(); if(symbol_index)*symbol_index=token_index;
            }break;
         }

         if(!symbol){templates.clear(); break;}

         if(symbol->type==Symbol::KEYWORD)break; // 'operator', in this case 'symbol_parent' can point to parent class or null if it's inside class declaration

         if(symbol->type==Symbol::TYPEDEF)
            if(InRange(i+1, tokens)) // check operator following symbol, can be . :: -> < TMPL_B
         {
            Token &op=*tokens[i];
            if(op=='<' || op==TMPL_B || op=='.' || op=="::" || op=="->") // proceed with typedef to target
            {
               // TODO: this typedef can be not yet resolved (if we're inside 'LinkDataTypes' and proceeding to another typedef that was not yet processed)
               Symbol::Modif modif; modif=symbol; Swap(modif.templates, templates);
               modif.proceedToFinal(&modif.templates);
               parent=symbol=modif(); Swap(modif.templates, templates);
            }
         }

         if(symbol->type==Symbol::NAMESPACE || symbol->type==Symbol::CLASS)
            if(InRange(i+1, tokens)) // check operator following symbol, can be . :: -> < TMPL_B
         {
            Token *op=tokens[i];

            if(*op=='<' || *op==TMPL_B) // template declaration, for example "Map<Image, Int>", "Map< Mems<Image>::ImageSub, Int >"
            {
               // TODO: template param can be also a function
               op->BStr::setBorrowed(TMPL_B); i++; op->parent=set_parent; // replace '<' with TMPL_B to specify template brackets explicitly
               for(Int template_index=0; i<tokens.elms(); template_index++)
               {
                  Memc<Symbol::Modif> ts;
                  UInt                const_level=0, modifiers=0;
                  for(; i<tokens.elms(); i++) // read modifiers
                  {
                     Token &token=*tokens[i];
                     if(token=="constexpr"     ){token.parent=set_parent;}else
                     if(token=="const"         ){token.parent=set_parent; const_level=1;}else
                     if(token=="const_mem_addr"){token.parent=set_parent; modifiers|=Symbol::MODIF_CONST_MEM_ADDR;}else
                     if(token=="volatile"      ){token.parent=set_parent;}else
                        break;
                  }
                  Int start=i;
                  if(Symbol *templat=GetFullSymbol(tokens, i, temp, set_parent, ts))
                  {
                     Symbol::Modif &t=templates.New(); t=templat; // add new template
                     if(InRange(template_index, symbol->templates))t.src_template=symbol->templates[template_index]; // if specified template actually fits in templates available
                     t.const_level=const_level; t.modifiers|=modifiers; Swap(t.templates, ts);

                     for(; i<tokens.elms(); i++) // process remaining modifiers
                     {
                        Token &token=*tokens[i];
                        if(token=='*'    ){token.parent=set_parent; t.ptr_level++;                  }else
                        if(token=='&'    ){token.parent=set_parent; t.modifiers|=Symbol::MODIF_REF; }else
                        if(token=="const"){token.parent=set_parent; t.const_level|=(1<<t.ptr_level);}else
                        {
                           ParseTemplates(tokens, i, temp, set_parent, true);
                           break;
                        }
                     }
                  }else
                  {
                     i=start;
                     ParseTemplates(tokens, i, temp, set_parent, true); // skip unknown template
                  }
                  if(InRange(i, tokens))
                  {
                     op=tokens[i];
                     if(*op==','               ){                               i++; op->parent=set_parent; continue;}
                     if(*op=='>' || *op==TMPL_E){op->BStr::setBorrowed(TMPL_E); i++; op->parent=set_parent; break   ;} // replace '>' with TMPL_E to specify template brackets explicitly
                  }
                  return null;
               }
               op=(InRange(i, tokens) ? tokens[i] : null);
            }

            if(op)
               if(*op=='.' || *op=="::" || *op=="->"){i++; op->parent=set_parent; continue;} // wishes to use sub-symbol
         }
         break;
      }
   }
   return symbol;
}
/******************************************************************************/
enum CLASS_TYPE
{
   CLASS_CLASS ,
   CLASS_STRUCT,
   CLASS_UNION ,
};
static void CreateClass(Memc<SymbolDef> &symbols, Memc<SymbolDecl> &decls, Memc<Token*> &tokens, Symbol *set_parent, SymbolPtr &clazz, Int &i, CLASS_TYPE class_type, Symbol::ACCESS_LEVEL access_level, Str &temp, Memc<Symbol::Modif> &templates, Memc<Int> &new_templates)
{
   Int start=i;
   if(InRange(i-1, tokens))
   {
      Token &prev=*tokens[i-1];
      if(prev=="friend" || prev=="<" || prev==TMPL_B || prev==",")return; // skip "friend class X;", "template<class X, class Y>"
   }
   if(InRange(i+1, tokens))
   {
      Token &token=*tokens[i+1];
      if(token=="__declspec")
      {
         token.parent=set_parent; i+=2;
         for(Int level=0; i<tokens.elms(); i++)
         {
            Token &token=*tokens[i]; token.parent=set_parent;
            if(token=='{' || token=='}' || token==';'){i--; return;}
            if(token=='(')   ++level;
            if(token==')')if(--level<=0)break;
         }
      }
   }
   if(InRange(i+1, tokens))
   {
      Token &name=*tokens[i+1]; name.def_decl=true; // set 'def_decl' without checking for "InRange(i+2, tokens)" because when typing new class at the end of file there is no next token, while we still want to set the 'def_decl' so suggestions are disabled
      if(InRange(i+2, tokens))
      {
         Token &next=*tokens[i+2];
         UInt   const_level=0, modifiers=Symbol::AccessLevelToModif(access_level);
         FlagSet(modifiers, Symbol::MODIF_UNION  , class_type==CLASS_UNION);
         FlagSet(modifiers, Symbol::MODIF_TYPEDEF, InRange(start-1, tokens) && *tokens[start-1]=="typedef");

         if(name.line->source->cpp && name.type==TOKEN_CODE && (next=='<' || next==TMPL_B || next=='.' || next=="->" || next=="::")) // class A<B>::C {}, class A::C {} (this can happen if C was forward declared in A)
         {
            i++; // now points to 'name'
            Int     symbol_index, local_start=i;
            Symbol *symbol_parent, *symbol=GetFullSymbol(tokens, i, temp, set_parent, templates, &symbol_parent, &symbol_index, false);
            DEBUG_ASSERT(symbol || !name.line->source->cpp, S+"Invalid Class definition\nLine ("+name.lineIndex()+"): \""+*name.line+"\"\nSource: \""+name.line->source->loc.asText()+"\"");
            if(symbol)
            {
               i--;
               Token &name=*tokens[symbol_index];
               clazz=symbols.New().require(UniqueName(set_parent, name, modifiers)).set(set_parent, Symbol::CLASS, symbol_index, name.line->source);
               name.symbol=clazz;
               clazz->type_range.set(ReadModifiers(tokens, start-1, const_level, modifiers, new_templates, temp), start);
               clazz-> def_range.set(local_start, i);
               clazz->modifiers  |=modifiers;
               clazz->access_level=((class_type==CLASS_CLASS && name.line->source->cpp) ? Symbol::ACCESS_PRIVATE : Symbol::ACCESS_PUBLIC); // C++ has classes with default private access, .es has classes with default public access
               if(new_templates.elms())ReadTemplates(symbols, tokens, new_templates[0], temp, *clazz, true);
            }
         }else
         if(name.type==TOKEN_CODE && next==';') // forward declaration "class X;"
         {
            name.symbol=decls.New().require((set_parent ? set_parent->full_name+SEP : S)+name).set(set_parent, Symbol::CLASS, i+1, name.line->source);
         }else
         if(name.type==TOKEN_CODE || name=='{')
         {
            clazz=symbols.New().require(UniqueName(set_parent, name, modifiers)).set(set_parent, Symbol::CLASS, (name=='{') ? start : i+1, name.line->source);
            if(name.type==TOKEN_CODE)name.symbol=clazz; // not '{'
            clazz->type_range.set(ReadModifiers(tokens, start-1, const_level, modifiers, new_templates, temp), start);
            clazz-> def_range.set((name=='{') ? start : i+1);
            clazz->modifiers  |=modifiers;
            clazz->access_level=((class_type==CLASS_CLASS && name.line->source->cpp) ? Symbol::ACCESS_PRIVATE : Symbol::ACCESS_PUBLIC); // C++ has classes with default private access, .es has classes with default public access
            if(new_templates.elms())ReadTemplates(symbols, tokens, new_templates[0], temp, *clazz, true);
         }
      }
   }
}
/******************************************************************************/
void DetectDataTypes(Memc<SymbolDef> &symbols, Memc<SymbolDecl> &decls, Memc<Token*> &tokens, SymbolPtr parent)
{
   Str                        temp;
   Bool                       extern_c   =false;
   Int                        level      =(parent ? parent->level+1 : 0);
   SymbolPtr                  next_parent= parent;
   Memc<Symbol::Modif       > templates;
   Memc<Int                 > new_templates;
   Memc<Symbol::ACCESS_LEVEL> access_levels; access_levels.add(parent ? parent->accessLevel() : Symbol::ACCESS_PUBLIC);
   FREPA(tokens)
   {
      Token &token=*tokens[i];
      token.parent=parent();
      switch(token.type)
      {
         case TOKEN_OPERATOR:
         {
            Char c=token[0];
            if(c=='{')
            {
               if(!next_parent && !level && InRange(i-2, tokens) && (*tokens[i-2])=="extern")extern_c=true;else level++; // check for "extern "C" {"
               parent=next_parent; token.parent=parent(); access_levels.add(parent ? parent->accessLevel() : Symbol::ACCESS_PUBLIC);
            }else
            if(c=='}')
            {
               if(parent && parent->type==Symbol::CLASS && (parent->modifiers&Symbol::MODIF_TYPEDEF) && level==parent->level+1) // check for typedefs after "typedef struct {}|"
               {
                  if(InRange(i+1, tokens) && *tokens[i+1]!=';')FlagDisable(parent->modifiers, Symbol::MODIF_TRANSPARENT); // if the class has a typedef defined, then it can be accessed through it, and will not be a transparent class
                  i++; ReadVarFunc(parent(), symbols, tokens, i, parent->token_index, SPACE_NORMAL, temp, templates, new_templates);
                  i--;
               }
               if(extern_c && !level)extern_c=false;else level--;
               if(parent && level<=parent->level)next_parent=parent=parent->parent;
               if(access_levels.elms()>1)access_levels.removeLast(); // keep at least one because codes operate on 'access_levels.last()'
               DEBUG_ASSERT(level>=0 || !token.line->source->cpp, S+"level<0\nLine ("+token.lineIndex()+"): \""+*token.line+"\"\nSource: \""+token.line->source->loc.asText()+"\"");
            }
         }break;

         case TOKEN_KEYWORD:
         {
            if(parent ? (level==parent->level+1) : (level==0)) // skip everything inside functions
               if(InRange(i+1, tokens))
                  switch(token[0])
            {
               case 'p': if(*tokens[i+1]==':') // only for "private:" and not "private int"
               {
                  if(token=="private"  )access_levels.last()=Symbol::ACCESS_PRIVATE  ;else
                  if(token=="protected")access_levels.last()=Symbol::ACCESS_PROTECTED;else
                  if(token=="public"   )access_levels.last()=Symbol::ACCESS_PUBLIC   ;
               }break;

               case 'n': if(token=="namespace")if(!(InRange(i-1, tokens) && *tokens[i-1]=="using")) // skip "using namespace X;"
               {
                  Token &name=*tokens[i+1]; name.def_decl=true;
                  if(name.type==TOKEN_CODE)
                  {
                     UInt modifiers=0;
                     next_parent=name.symbol=symbols.New().require(UniqueName(parent, name, modifiers, true)).set(parent, Symbol::NAMESPACE, i+1, name.line->source);
                     next_parent->type_range.set(i); next_parent->def_range.set(i+1); next_parent->modifiers|=modifiers;
                  }
               }break;

               case 's': if(token=="struct")CreateClass(symbols, decls, tokens, parent(), next_parent, i, CLASS_STRUCT, access_levels.last(), temp, templates, new_templates); break;
               case 'c': if(token=="class" )CreateClass(symbols, decls, tokens, parent(), next_parent, i, CLASS_CLASS , access_levels.last(), temp, templates, new_templates); break;
               case 'u': if(token=="union" )CreateClass(symbols, decls, tokens, parent(), next_parent, i, CLASS_UNION , access_levels.last(), temp, templates, new_templates); break;

               case 'e': if(token=="enum")
               {
                  Token &name=*tokens[i+1]; name.def_decl=true; name.parent=parent();
                  if(InRange(i+2, tokens))
                  {
                     Token &next=*tokens[i+2];
                     if(next!=';') // skip forward declarations
                        if(name.type==TOKEN_CODE && (next=='{' || (next==':' && InRange(i+4, tokens) && *tokens[i+4]=='{')) // valid name for enum "enum A {" or "enum A : B {"
                        || name=='{')                                                                                       // nameless enum
                     {
                        UInt      modifiers=0;
                        Int       start=i;
                        SymbolPtr enm=symbols.New().require(UniqueName(parent, name, modifiers)).set(parent, Symbol::ENUM, (name=='{') ? i : i+1, name.line->source); if(name.type==TOKEN_CODE)name.symbol=enm();
                        enm->type_range.set(i); enm->def_range.set(enm->token_index); enm->modifiers|=modifiers;
                        for(; i<tokens.elms(); )
                        {
                           Token &token=*tokens[i++]; if(token=='{')
                           {
                              token.parent=enm();
                              break;
                           }
                        }
                        for(; i<tokens.elms(); )
                        {
                           Token &enum_elm=*tokens[i++]; enum_elm.def_decl=true; enum_elm.parent=enm();
                           if(enum_elm.type==TOKEN_CODE)
                           {
                              modifiers=0;
                              enum_elm.symbol=symbols.New().require(UniqueName(parent, enum_elm, modifiers)).set(parent, Symbol::ENUM_ELM, i-1, enum_elm.line->source); // ENUM_ELM's are assigned to the parent of ENUM
                              enum_elm.symbol->value=enm; enum_elm.symbol->type_range=enm->def_range; enum_elm.symbol->def_range.set(i-1); enum_elm.symbol->modifiers|=modifiers;
                              if(InRange(i, tokens) && *tokens[i]=='=') // custom value
                              {
                                 tokens[i++]->parent=enm();
                                 for(; i<tokens.elms(); i++)
                                 {
                                    Token &token=*tokens[i];
                                    if(token=='{' || token=='}' || token==',' || token==';')break;
                                    token.parent=enm();
                                 }
                              }
                           }else
                           if(enum_elm=='}') // enum declaration end
                           {
                              FlagSet(enm->modifiers, Symbol::MODIF_TYPEDEF, InRange(start-1, tokens) && *tokens[start-1]=="typedef");
                              if(enm->modifiers&Symbol::MODIF_TYPEDEF)ReadVarFunc(enm(), symbols, tokens, i, enm->token_index, SPACE_NORMAL, temp, templates, new_templates);
                              i--; break;
                           }else
                           if(enum_elm!=',') // ',' means next enum element so just skip it
                           {
                              // not TOKEN_CODE, not '}', not ',' means error
                              DEBUG_ASSERT(token.line->source->cpp==false, S+"invalid enum declaration\nLine ("+token.lineIndex()+"): \""+*token.line+"\"\nSource: \""+token.line->source->loc.asText()+"\""); // report error only on C++ files
                           }
                        }
                     }
                  }
               }break;

               case 't': if(token=="typedef")
               {
                  i++;
                  Int  type_start=i;
                  Bool data_type =false;
                  for(; i<tokens.elms(); i++) // skip modifiers ("typedef const int volatile x", "typedef int const x")
                  {
                     Token &token=*tokens[i];
                     if(token.type==TOKEN_KEYWORD)
                     {
                        token.parent=parent(); SetTokenSymbol(token, temp);
                        if(token.symbol && (token.symbol->modifiers&Symbol::MODIF_DATA_TYPE)){if(!data_type)type_start=i; data_type=true;}
                     }else break;
                  }
                  if(InRange(i+1, tokens)) // check if it's a possible struct/class/union/enum definition
                  {
                     Token &token=*tokens[i-1];
                     if(token.type==TOKEN_KEYWORD)
                        if(token=="struct" || token=="class" || token=="union" || token=="enum")
                     {
                        Token &name=*tokens[i], &next=*tokens[i+1];
                        if(name=='{'                                           // nameless "typedef struct   {} X;"
                        ||(name.type==TOKEN_CODE && (next=='{' || next==':'))) // named    "typedef struct A {} X;" or "typedef struct A:B {} X;"
                        {
                           i-=2; break; // if it's a definition then don't process it here (move cursor back to 'struct' and additional step back because it will be increased in main loop)
                        }
                     }
                  }
                  if(!data_type){type_start=i; SkipUnknownSymbol(tokens, i, parent());} // if there wasn't any keyword data type then we must skip unknown data type
                  ReadVarFunc(null, symbols, tokens, i, type_start, SPACE_NORMAL, temp, templates, new_templates);
                  i--;
               }break;
            }
         }break;
      }
   }
}
/******************************************************************************/
// TODO: LinkDataTypes will fail for "typedef CLASS<TEMPLATE_FROM_VAR_FUNC> TYPEDEF;" because var funcs were not yet processed during LinkDataTypes stage
void LinkDataTypes(Memc<SymbolDef> &symbols, Memc<Token*> &tokens)
{
   Str temp;
   REPA(symbols) // iterate through all symbol definitions in this source
   {
      Symbol &symbol=*symbols[i];
      if(     symbol.valid)
      {
         if(symbol.valid>1) // if symbol was defined more than one time then check if we're processing the one which source fits the source of the tokens (this is needed in case symbol is from different source, in which case we would be operating on tokens of different source)
         {
            Bool same_source=false;
            if(InRange(symbol.token_index, tokens))
               if(Line *line=tokens[symbol.token_index]->line)same_source=(line->source==symbol.source);
            if(!same_source)continue; // skip this symbol
         }
         switch(symbol.type)
         {
            case Symbol::TYPEDEF: // if it's a typedef
            {
               if(!(symbol.modifiers&Symbol::MODIF_TYPEDEF)) // not direct typedef's must be linked
               {
                  symbol.value=null; // clear only value symbol pointer (and keep modifiers, const_level, ptr_level, array_dims, etc.)
                  for(Int i=GetSymbolStart(tokens, symbol.token_index)+1; InRange(i, tokens); ) // +1 to skip the 'typedef' token
                  {
                     Int     start=i;
                     Symbol *ts   =GetFullSymbol(tokens, i, temp, tokens[i]->parent, symbol.value.templates);
                     if(ts && (ts->modifiers&Symbol::MODIF_CPP_MODIFIER))
                     {
                        // ignore modifiers
                     }else
                     {
                        symbol.value=ts;
                        symbol.type_range.y=i-1;
                        break;
                     }
                     MAX(i, start+1);
                  }
               }
            }break;

            case Symbol::CLASS: // if it's a class, then detect its base classes
            {
               symbol.base.clear();
               Int i=symbol.token_index+1; // skip class name, and check next operator
               if(InRange(i, tokens) && *tokens[i]==':')for(i++; ; )
               {
                  Symbol::ACCESS_LEVEL access_level=symbol.access_level; // get default access
                  Memc<Symbol::Modif>  templates;
                  for(; i<tokens.elms(); i++) // read modifiers
                  {
                     Token &token=*tokens[i];
                     if(token=="private"  )access_level=Symbol::ACCESS_PRIVATE  ;else
                     if(token=="protected")access_level=Symbol::ACCESS_PROTECTED;else
                     if(token=="public"   )access_level=Symbol::ACCESS_PUBLIC   ;else break;
                  }
                  if(Symbol *base=GetFullSymbol(tokens, i, temp, &symbol, templates))
                  {
                     Symbol::Modif &symbol_base=symbol.base.New(); symbol_base=base; Swap(symbol_base.templates, templates); symbol_base.modifiers=Symbol::AccessLevelToModif(access_level);
                     symbol.def_range.y=i-1;
                     if(InRange(i, tokens) && *tokens[i++]==',')continue;
                  }
                  break;
               }
            }break;
         }
      }
   }
}
/******************************************************************************/
static Symbol* ConvertToFunc(Memc<SymbolDef> &symbols, SymbolPtr *var_func, Int token_index, Source *source, Bool func_ptr_or_ref)
{
   if(var_func)if(SymbolPtr &symbol=*var_func)
   {
      if(symbol->type==Symbol::VAR && symbol->valid==1)symbol->type=(func_ptr_or_ref ? Symbol::FUNC : Symbol::FUNC_LIST);
      if(symbol->type==Symbol::FUNC_LIST              )
      {
         symbol->modifiers|=Symbol::MODIF_TRANSPARENT; // FUNC_LIST need to be transparent
         SymbolPtr &s=symbols.New().require(symbol->full_name+SEP+RandomName()).set(symbol, Symbol::FUNC, token_index, source); // function instance
         symbol->funcs.add(s); // add function instance to the function list
         symbol=s; // set token symbol to function instance instead of list
      }
   }
   return var_func ? (*var_func)() : null;
}
/******************************************************************************/
Bool IsDeclaration(Memc<Token*> &tokens, Symbol *symbol, Int i)
{
   // if(const A<X,B<Y,Z>>.C *x)
   for(Int level=0, start=i; i>0; i--)
   {
      Token &token=*tokens[i];
      if(token=='.' || token=="->" || token=="::" || token==',')continue; // skip operators
      if(token==TMPL_B){level++; if(level<=0)continue;} // don't continue if level>0
      if(token==TMPL_E){level--;             continue;}
      if(!token.symbol)break; // if no symbol then break
      if(!(token.symbol->modifiers&Symbol::MODIF_DATA_TYPE) && !(token.symbol->modifiers&Symbol::MODIF_CPP_MODIFIER) && token.symbol->type!=Symbol::NAMESPACE)break; // if symbol is not data type or modifier or namespace then break
      if(!level && start!=i && token.symbol==symbol)return false; // if we have constructor declaration (X::X) then return false
   }
   if(!i)return true;

   Token &token=*tokens[i];

   if(token==';'
   || token=='{'
   || token=='}'
   || token=="do"
   || token=="else")return true;

   if(token=='(')
   {
      // if(TYPE ..)TYPE ..
      // for(TYPE ..; TYPE ..; )TYPE ..
      // while(TYPE ..)TYPE ..
      // switch(TYPE ..)
      if(InRange(i-1, tokens))
      {
         Token &token=*tokens[i-1];
         if(token=="if"
         || token=="for"
         || token=="while"
         || token=="switch")return true;
      }
   }else
   if(token==')')
   {
      // if(TYPE ..)TYPE ..
      // for(TYPE ..; TYPE ..; )TYPE ..
      // while(TYPE ..)TYPE ..
      // switch(TYPE ..)TYPE ..
      for(Int level=0; --i>=0; )
      {
         Token &token=*tokens[i];
         switch(token[0])
         {
            case '{':
            case '}':
            case ';': return false;
            case ')':    --level; break;
            case '(': if(++level>0)
            {
               if(InRange(i-1, tokens)){Token &token=*tokens[i-1]; return token=="if" || token=="for" || token=="while" || token=="switch";}
               return false;
            }break;
         }
      }
   }else
   if(token==':') // label, access mode, case
   {
      if(InRange(i-2, tokens))
      {
         if(tokens[i-1]->type==TOKEN_CODE) // label name
         {
            Token &token=*tokens[i-2]; // allowed tokens before label name
            if(token==';'
            || token=='{'
            || token=='}')return true;
         }
         if(tokens[i-1]->type==TOKEN_KEYWORD) // access
         {
            Token &token=*tokens[i-1];
            if(token=="public"
            || token=="protected"
            || token=="private")return true;
         }
         for(Int level=0; i>0; ) // try "case:"
         {
            Token &token=*tokens[--i];
            switch(token[0])
            {
               case '{':
               case '}':
               case ';': return false;
               case '(': case '[': level++; break;
               case ')': case ']': level--; break;
               default :       if(!level && token=="case")return true; break;
            }
         }
      }
   }else
   if(token=="typedef")return true; // typedef TYPE
   return false;
}
/******************************************************************************

   struct X
   {
      int   x (); // forward declaration of function, FUNC
      int ( y)(); // forward declaration of function, FUNC
      int (*z)(); // pointer to function            , FUNC
      int   w   ; // var                            , VAR
      int  (u)  ; // var                            , VAR
      int  *v   ; // pointer to var                 , VAR
      int*(*a)  ; // pointer to pointer             , VAR
   };
   int   x ()      ; x(); // forward declaration of function, FUNC
   int ( y)()      ; y(); // forward declaration of function, FUNC
   int (*z)()=null ; z(); // pointer to function            , FUNC
   int   w         ; w  ; // var                            , VAR
   int   u (0)     ; u  ; // var                            , VAR
   int   v= 0      ; v  ; // var                            , VAR
   int  *a= null   ; a  ; // pointer to var                 , VAR
   int  *b( null)  ; b  ; // pointer to var                 , VAR
   int   c[2][2]={}; c  ; // array of int's                 , VAR
   Vec2  v(1,2)    ;
   Vec2  v(Class::stat_var);
   Vec2  v(obj.var);

   struct X
   {
      X   x (     ); // forward declaration of function, FUNC
      X ( y)(     ); // forward declaration of function, FUNC
      X (*z)(     ); // pointer to function            , FUNC
      X   w        ; // var                            , VAR
      X  (u)       ; // var                            , VAR
      X  *v        ; // pointer to var                 , VAR
      X (*a)       ; // pointer to var                 , VAR
      X*(*b)       ; // pointer to pointer             , VAR
      X     (     ); // constructor                    , FUNC
      X     (int x); // constructor                    , FUNC
   };

   void * (*GetFunc(..params1..)) (..params2..); function which returns pointer to function

/******************************************************************************/
Bool IsVarFuncDefinition(Memc<Token*> &tokens, Int &i, Str &temp, Symbol* &symbol, Memc<Symbol::Modif> &templates, Symbol *set_parent, Symbol* *symbol_parent, Int *symbol_index)
{
/**
   int a();
   int b(5);
   int c(int);
   int d(int z);
   int e(int(5));
   int f(int(z));
   int g(int( ));
   int h(Vec);
   int i(Vec z);
   Vec j(Vec(5));
   int k(Vec(z));
   int l(Vec( ));

   a();
   b=0;
   c(0);
   d(0);
   e=0;
   f(0);
   g(0);
   h(0);
   i(0);
   j=0;
   k(0);
   l(0);

   void func(const  ..) OK  modifier at start
   void func(Vec2     ) OK  data type followed by *&,)         (unnamed parameter)
   void func(Vec2    v) OK  data type followed by *&TOKEN_CODE (  named parameter)
   void func(Vec2(   )) OK  data type followed by ()
   void func(Vec2(*& )) OK  data type followed by (*&)
   void func(Vec2( 0 )) BAD data type followed by (TOKEN_NUMBER
   void func(Vec2('a')) BAD data type followed by (CHR
   void func(Vec2(  v)) OK  token.type==TOKEN_CODE
   void func(Vec2(*&v)) OK  token.type==TOKEN_CODE
   void func(Vec2(v.a)) BAD name not followed by )
/**/
   if(symbol=GetFullSymbol(tokens, i, temp, set_parent, templates, symbol_parent, symbol_index))
   {
      if(symbol->modifiers&Symbol::MODIF_CPP_MODIFIER)return true;
      if(symbol->modifiers&Symbol::MODIF_DATA_TYPE   )
      {
         if(InRange(i, tokens) && *tokens[i]=='(') // if has brackets then we need to perform additional checks
         {
            for(Int j=i+1; j<tokens.elms(); j++)
            {
               Token &token=*tokens[j];
               if(token=='*' || token=='&')continue;
               if(token==')')return true;
               if(token.type==TOKEN_CODE && InRange(j+1, tokens) && *tokens[j+1]==')')return true; // allow only if name is TOKEN_CODE and is followed by ')'
               break;
            }
            return false;
         }
         return true;
      }
   }
   return false;
}
/******************************************************************************/
static void SkipParams(Memc<Token*> &tokens, Int &i, Symbol *set_parent)
{
   for(Int level=0; i<tokens.elms(); )
   {
      Token &token=*tokens[i];
      if(token=='{' || token=='}' || token==';')return;
      if(token==')')if(!level--)return;
      if(token=='(')    level++;
      i++; token.parent=set_parent;
   }
}
static void SkipThrow(Memc<Token*> &tokens, Int &i, Symbol *set_parent)
{
   if(InRange(i, tokens) && *tokens[i]=='(')
   {
      tokens[i++]->parent=set_parent;
      SkipParams(tokens, i, set_parent);
      if(InRange(i, tokens) && *tokens[i]==')')tokens[i++]->parent=set_parent;
   }
}
/******************************************************************************/
// TODO: add support for template based auto-casts like "T1(BASE) operator Memc<BASE>& (); // casting to container of 'BASE' elements, 'TYPE' must be extended from BASE
static void ReadAutoCast(Symbol *symbol, Memc<SymbolDef> &symbols, Memc<Token*> &tokens, Int &i, Int start, Str &temp, Memc<Symbol::Modif> &templates) // operator const char * () {}
{
   Token  &token_start =*tokens[start];
   Symbol *start_parent= token_start.parent;
   if(symbol ? (!start_parent || start_parent->type==Symbol::NAMESPACE)  // we can define auto-cast operator only in namespace (if Class was     specified)
             : ( start_parent && start_parent->type==Symbol::CLASS    )) // or inside class                                    (if Class was not specified)
   {
      if(!symbol)symbol=start_parent;
      if( symbol && symbol->type==Symbol::CLASS)
      {
         UInt const_level=0;
         Int    ptr_level=0;
         UInt   modifiers=0;
         if(InRange(i, tokens) && *tokens[i]=="const"){const_level=1; tokens[i++]->parent=start_parent;}

         Memc<Symbol::Modif> type_templates;
         if(Symbol *type=GetFullSymbol(tokens, i, temp, start_parent, type_templates))
         {
            for(; i<tokens.elms(); )
            {
               Token &mod=*tokens[i];
               if(mod=='*'    )ptr_level++;else
               if(mod=='&'    )modifiers  |=Symbol::MODIF_REF;else
               if(mod=="const")const_level|=(1<<ptr_level);else break;
               i++; mod.parent=start_parent;
            }
            if(InRange(i+1, tokens) && *tokens[i]=='(' && *tokens[i+1]==')')
            {
               tokens[i++]->parent=start_parent; // '('
               tokens[i++]->parent=start_parent; // ')'
               if(InRange(i, tokens) && *tokens[i]=="const"){tokens[i++]->parent=start_parent; modifiers|=Symbol::MODIF_FUNC_CONST;} // ()const
               if(InRange(i, tokens) && *tokens[i]=="throw"){tokens[i++]->parent=start_parent; SkipThrow(tokens, i, start_parent); } // throw(..)

               SymbolPtr op=symbols.New().require(symbol->full_name+SEP+"operator cast").set(symbol, Symbol::FUNC_LIST, start, token_start.line->source);
               op->modifiers|=Symbol::MODIF_SKIP_SUGGESTIONS; // operate on FUNC_LIST before converting to FUNC
               ConvertToFunc(symbols, &op, start, token_start.line->source, false); // 'func_ptr_or_ref'=false because auto cast operators are never a pointer to function
               op->def_range.set(start, i-1);
               op->      modifiers|=(modifiers & ~Symbol::MODIF_REF); // set all except MODIF_REF
               op->value.modifiers|=(modifiers &  Symbol::MODIF_REF); // set only       MODIF_REF
               op->value=type;
               op->value.  ptr_level=  ptr_level;
               op->value.const_level=const_level;
               Swap(op->value.templates, type_templates);

               if(InRange(i, tokens) && *tokens[i]=='{')
               {
                  op->modifiers|=Symbol::MODIF_FUNC_BODY;
                  for(Int level=0; i<tokens.elms(); )
                  {
                     Token &token=*tokens[i++]; token.parent=op();
                     if(token=='{')   ++level;else
                     if(token=='}')if(--level<=0)break;
                  }
               }
            }
         }
      }
   }
}
/******************************************************************************/
// TODO: remove this
void GetFuncParams(Memc<SymbolDef> &symbols, Memc<Token*> &tokens, Int &i, Str &temp, SymbolPtr *var_func, Int var_func_name_token_index, Bool func_ptr_or_ref, Symbol *set_parent)
{
   Int params_as_values=0;
   for(; i<tokens.elms(); )
   {
      UInt   const_level=0, const_level_in_bracket=0;
      Int      ptr_level=0,   ptr_level_in_bracket=0, bracket_level=0, type_start=i;
      UInt     modifiers=0,   modifiers_in_bracket=Symbol::MODIF_FUNC_PARAM;
      Token *par=tokens[i++]; par->parent=set_parent;
      if(*par=="const"){par=(InRange(i, tokens) ? tokens[i++] : null); par->parent=set_parent; const_level=1;}
      if( par)
      {
         if(*par==')')break;
         if(*par==',')continue;

         Bool definition=false; i--; Int start=i; Memc<Symbol::Modif> templates; Symbol *type; if(IsVarFuncDefinition(tokens, i, temp, type, templates, set_parent))if(type->modifiers&Symbol::MODIF_DATA_TYPE)definition=true;
         if(  definition)
         {
            ConvertToFunc(symbols, var_func, var_func_name_token_index, tokens[var_func_name_token_index]->line->source, func_ptr_or_ref);
            Int type_end=i-1;
            for(; i<tokens.elms(); )
            {
               Int    param_token_index=i;
               Token &name=*tokens[i++]; name.parent=set_parent;
               switch(name.type)
               {
                  case TOKEN_OPERATOR:
                  {
                     switch(name[0])
                     {
                        case '*': if(bracket_level)ptr_level_in_bracket++;else ptr_level++; break;
                        case '&': if(bracket_level)modifiers_in_bracket|=Symbol::MODIF_REF;else modifiers|=Symbol::MODIF_REF;break;
                        case '(': bracket_level++; break;

                        case ',':
                        case ')':
                        case '=':
                        {
                           // TODO: create empty param

                           if(name[0]=='=') // default value
                           {
                              // for example: 5, x, 1*(x+2), Map<Image,Int>::static_var+Mems<X>::stat_var
                              Int start=i; ParseTemplates(tokens, i, temp, set_parent, false);
                            //if(name.symbol){name.symbol->modifiers|=Symbol::MODIF_DEF_VALUE; name.symbol->def_range.y=i-1; name.symbol->def_val_range.set(start, i-1);}
                              break;
                           }

                           if(name[0]==',')goto finished_param;
                           else            goto finished_params;
                        }break;

                        default: goto finished_params; // error
                     }
                  }break;
                  
                  case TOKEN_KEYWORD:
                  {
                     if(name=="const")
                     {
                        if(bracket_level)const_level_in_bracket|=(1<<ptr_level_in_bracket);
                        else             const_level           |=(1<<ptr_level           );
                     }else
                     if(name=="long"){}else // long long
                        goto finished_params; // error
                  }break;

                  case TOKEN_CODE:
                  {
                     for(; bracket_level>0 && i<tokens.elms() && *tokens[i]==')'; ){bracket_level--; tokens[i++]->parent=set_parent;} // force closing brackets started earlier "int ((x))|"
                     if(  !bracket_level)
                     {
                        name.def_decl=true;
                        if(var_func)if(SymbolPtr &func=*var_func)
                        {
                           name.symbol=symbols.New().require(func->full_name+SEP+name).set(func, Symbol::VAR, param_token_index, name.line->source);
                           name.symbol->value     =type; Swap(name.symbol->value.templates, templates);
                           // TODO: when name.symbol will be as FUNC, then calculate differently
                           name.symbol->       modifiers|=((modifiers|   modifiers_in_bracket) & ~Symbol::MODIF_REF); // set all except MODIF_REF
                           name.symbol->value. modifiers|=((modifiers|   modifiers_in_bracket) &  Symbol::MODIF_REF); // set only       MODIF_REF
                           name.symbol->value.  ptr_level=  ptr_level+   ptr_level_in_bracket;
                           name.symbol->value.const_level=const_level|(const_level_in_bracket<<ptr_level);
                           name.symbol->type_range.set(type_start, type_end);
                           name.symbol-> def_range.set(type_end+1);
                           func->params.add(name.symbol);
                        }

                        for(; i<tokens.elms(); )
                        {
                           Token &op=*tokens[i++]; op.parent=set_parent;
                           switch(op[0])
                           {
                              case '(': // parameter list or default value declaration
                              {
                                 // TODO: 
                                 GetFuncParams(symbols, tokens, i, temp, null, param_token_index, true, set_parent); // 'func_ptr_or_ref'=true because we're processing function parameters and in such we can pass only pointer to function (never define new function)
                              }break;

                              case '[': // array
                              {
                                 for(Str dim; i<tokens.elms(); )
                                 {
                                    Token &token=*tokens[i++]; token.parent=set_parent;
                                    if(token==']')
                                    {
                                       // TODO: watch out for error in calculation of 'CalculateI' or <=0 value
                                       // TODO: this is wrong, ignores ptr_level_in_bracket
                                       if(name.symbol)name.symbol->value.array_dims.NewAt(0)=CalculateI(dim);
                                       goto finished_dim;
                                    }
                                    dim+=token;
                                 }
                                 goto finished_params; // error
                              finished_dim:;
                              }break;

                              case '=': // default value
                              {
                                 // for example: 5, x, 1*(x+2), Map<Image,Int>::static_var+Mems<X>::stat_var
                                 Int start=i; ParseTemplates(tokens, i, temp, set_parent, false);
                                 if(name.symbol){name.symbol->modifiers|=Symbol::MODIF_DEF_VALUE; name.symbol->def_range.y=i-1; name.symbol->def_val_range.set(start, i-1);}
                              }break;

                              case ',':
                              {
                                 if(name.symbol)name.symbol->def_range.y=i-2;
                              }goto finished_param; // proceed to next param

                              default:
                              {
                                 if(name.symbol)name.symbol->def_range.y=i-2;
                              }goto finished_params; // end of param declaration
                           }
                        }
                     }
                  }break;

                  default: goto finished_params; // error
               }
            }
         }else
         {
            // for example: 5, x, 1*(x+2), Map<Image,Int>::static_var+Mems<X>::stat_var
               i= start; ParseTemplates(tokens, i, temp, set_parent, false);
            if(i==start)goto finished_params;
            params_as_values++;
         }
      }
   finished_param:;
   }
finished_params:;
   if(!params_as_values)ConvertToFunc(symbols, var_func, var_func_name_token_index, tokens[var_func_name_token_index]->line->source, func_ptr_or_ref);
}
void ReadVarFuncs(Memc<SymbolDef> &symbols, Memc<Token*> &tokens, Int &i, SPACE_MODE space_mode, Str &temp, Symbol *set_parent) // don't re-use 'templates' and 'new_templates' from other functions because they might be still used
{
   Memc<Symbol::Modif> templates;
   Memc<Int>       new_templates;

   // process var funcs
   for(; i<tokens.elms(); )
   {
      Int    start=i;
      Token &token=*tokens[i];
      if(token.type==TOKEN_OPERATOR)
      {
         if(token==','){i++; token.parent=set_parent; continue;} // proceed to next element
         if(token==')' || token=='{' || token=='}')return;
      }
      Symbol *symbol_parent;
      if(Symbol *type=GetFullSymbol(tokens, i, temp, set_parent, templates, &symbol_parent))
      {
         if(type->modifiers&Symbol::MODIF_DATA_TYPE) // if current element is of data type, then probably it's member/var/method/function definition/declaration
         {
            ReadVarFunc(type, symbols, tokens, i, start, space_mode, temp, templates, new_templates, symbol_parent);
            continue;
         }else
         if(type->modifiers&Symbol::MODIF_CPP_MODIFIER)
         {
            continue;
         }
      }
      ParseTemplates(tokens, i, temp, set_parent, space_mode==SPACE_TEMPLATE);
      MAX(i, start+1);
   }
}
void ReadVarFunc(Symbol *type, Memc<SymbolDef> &symbols, Memc<Token*> &tokens, Int &i, Int type_start, SPACE_MODE space_mode, Str &temp, Memc<Symbol::Modif> &templates, Memc<Int> &new_templates, Symbol *symbol_parent)
{
   Int type_start_backup=type_start;
   if(InRange(type_start-1, tokens)) // 'type_start' may be different between named/nameless struct/class/union/enum, so adjust 'type_start' if needed (case of "struct {}obj;" ?)
   {
      Token &token=*tokens[type_start-1];
      if(token.type==TOKEN_KEYWORD && (token=="struct" || token=="class" || token=="union" || token=="enum"))type_start--;
   }
   Token  &token_start =*tokens[type_start];
   Symbol *start_parent= token_start.parent;
   Bool    skip_suggestions=false,
           inside_class=(start_parent && start_parent->type==Symbol::CLASS),
           ctor_dtor=false, dtor=(InRange(i-2, tokens) && *tokens[i-2]=='~'); // ~X   X::~X
   UInt      modifiers=0,   modifiers_in_bracket=0, group_modifiers=0, // "group_modifiers TYPE modifiers (modifiers_in_bracket VAR)", "const int& (*var)()"
           const_level=0, const_level_in_bracket=0, group_const_level=0;
   Int       ptr_level=0,   ptr_level_in_bracket=0,
         bracket_level=0;
   Str       name;
   Mems<Int> array_dims;

   Int type_modif_start=ReadModifiers(tokens, type_start-1, group_const_level, group_modifiers, new_templates, temp); // start of type modifiers (including templates) "|<TYPE> const Int"

   // check if it's typedef (check tokens before 'type_modif_start')
   Bool is_typedef=false;
   if(InRange(type_modif_start-1, tokens))
   {
      Token &token=*tokens[type_modif_start-1];
      if(token.type==TOKEN_KEYWORD)
         if(token=="typedef"                                                                                     // "typedef"
         ||(token=="typename" && InRange(type_modif_start-2, tokens) && *tokens[type_modif_start-2]=="typedef")) // "typedef typename"
      {
         is_typedef=true;
         type_modif_start-=((token=="typedef") ? 1 : 2); // 1 for "typedef", 2 for "typedef typename"
         if(type && (type->type==Symbol::CLASS || type->type==Symbol::ENUM) && (type->modifiers&Symbol::MODIF_TYPEDEF)) // if type is class/enum with direct typedef
            if(type->source==token.line->source && type->token_index==type_start_backup)group_modifiers|=Symbol::MODIF_TYPEDEF; // if typedef being declared in the same source as type, and the type definition token index is equal to start it means we're declaring direct B typedef "typedef class A {} B"
      }
   }

   // process modifiers at name position (at 'i')
   for(; i<tokens.elms(); )
   {
      Token &token=*tokens[i];
      if(token.type==TOKEN_KEYWORD)
      {
         SetTokenSymbol(token, temp);
         if(token.symbol && (token.symbol->modifiers&Symbol::MODIF_CPP_MODIFIER))
         {
            i++; token.parent=start_parent;
            ReadModifier(token, group_const_level, group_modifiers);
            continue;
         }
      }
      break; // break on any non-modifier token
   }

   // detect 'type_end' (check tokens between 'type_start' and 'i')
   Int type_end=type_start;
   for(; ; type_end++)
   {
      if(type_end>=i-1)break; // max limit
      Token &token=*tokens[type_end+1];
      if(token==':' || token=='{')break; // struct X : Y { } x;
   }

   // process modifiers in type range (between 'type_start' and 'type_end', sample: "int const unsigned")
   for(Int i=type_start; i<=type_end; i++)
   {
      Token &token=*tokens[i];
      if(token.type==TOKEN_KEYWORD)
      {
         if(token.symbol && (token.symbol->modifiers&Symbol::MODIF_CPP_MODIFIER))ReadModifier(token, group_const_level, group_modifiers);
         continue;
      }
      break; // break on any non-keyword token
   }

   // process var/func
   Int  var_func_def_start=i; // var func definition start, this includes pointers "int |**x"
   Bool is_friend=FlagTest(group_modifiers, Symbol::MODIF_FRIEND);

   if(space_mode==SPACE_NORMAL    ){if(inside_class && !is_friend)group_modifiers|=Symbol::AccessLevelToModif(start_parent->access_level);}else
   if(space_mode==SPACE_FUNC_PARAM){                              group_modifiers|=Symbol::MODIF_FUNC_PARAM;}

   for(; i<tokens.elms(); )
   {
      Int    var_func_name_token_index=i; // this must be precisely set to the token which contains the name of the var_func
      Token &token=*tokens[i];
      switch(token.type)
      {
         case TOKEN_OPERATOR:
         {
            switch(token[0])
            {
               case ';': i++; token.parent=start_parent; return;

               case '=':
               case '[':
               case ',':
               case ')':
               {
                  if(space_mode==SPACE_NORMAL)return; // "{int,}", "{int=}" is not supported
                  if(!name.is()){name="unused"; MakeUnique(name); skip_suggestions=true;} goto create; // "void func(int=", "void func(int,", "void func(int)", "void func(int[])" is supported
               }break;

               case '{':
               case '}': return; // don't process {} symbols

               default : i++; token.parent=start_parent; break;
               case '*': i++; token.parent=start_parent; if(bracket_level)ptr_level_in_bracket++;else ptr_level++; break;
               case '&': i++; token.parent=start_parent; if(bracket_level)modifiers_in_bracket|=Symbol::MODIF_REF;else modifiers|=Symbol::MODIF_REF; break;
               case '(':
               {
                  i++; token.parent=start_parent;
                     bracket_level++;
                  if(bracket_level==1) // check if we're declaring constructor/destructor X(|..)
                  {
                     if(type && type->type==Symbol::CLASS)
                        if( start_parent==type                                                              // class X{X();}   class A{class B{A::B();}}
                        || symbol_parent==type && (!start_parent || start_parent->type==Symbol::NAMESPACE)) // X::X();
                     {
                        /*Int j=i; Symbol *symbol; Memc<Symbol::Modif> param_templates;
                        if(InRange(i, tokens) && *tokens[i]==')'                                        // if nothing X(|)
                        || IsVarFuncDefinition(tokens, j, temp, symbol, param_templates, start_parent)) // or parameter definition*/ // can't use 'IsVarFuncDefinition' because it may use unknown yet templates
                        for(Int j=i, level=0; j<tokens.elms(); j++)
                        {
                           Token &token=*tokens[j];
                           if(token=='{' || token=='}' || token==';')break;
                           if(token=='(')    level++;
                           if(token==')')if(!level--)
                           {
                              if(InRange(j+1, tokens) && *tokens[j+1]!='(')
                              {
                                 name=(dtor ? S+'~'+*type : *type); ctor_dtor=true; var_func_name_token_index=type_start; bracket_level=0; i--;
                                 goto create;
                              }
                              break;
                           }
                        }
                     }
                  }
               }break;
            }
         }break;

         case TOKEN_CODE   :
         case TOKEN_KEYWORD:
         {
         create:;
            Memc<Symbol::Modif> name_templates;
            Symbol             *parent, *symbol=GetFullSymbol(tokens, i, temp, start_parent, name_templates, &parent, null, false); // 'parent'=to which symbol 'symbol' should belong to as a child
            SymbolPtr           symbol_ptr_temp, *symbol_ptr=&symbol_ptr_temp; // 'symbol_ptr'=pointer to Token::symbol which will be set to the created element
            if(!parent             )parent=start_parent        ; // if parent hasn't been specified then use current space
            if( is_friend && parent)parent=parent->Namespace() ; // friends are in nearest namespace
            if( ctor_dtor          )parent=type                ; // constructors/destructors are always in their same class space
            if(      dtor          )type  =TypeSymbol(VAR_VOID); // destructors are of void type
            if(!ctor_dtor          )
            {
               if(symbol && (symbol->modifiers&Symbol::MODIF_CPP_MODIFIER))
               {
                  if(*symbol=="const")
                  {
                     if(bracket_level)const_level_in_bracket|=(1<<ptr_level_in_bracket);else
                                      const_level           |=(1<<ptr_level           );
                  }
         // TODO: else compile error
                  break;
               }

               if(!InRange(i-1, tokens))return;

               // check for pointer to member (function/variable) "func: void (Class::*func)();", "var: int Class::*var;"
               if(!symbol && InRange(i+1, tokens)) // check for "i+1" because we're adding +2 and later accessing -1
               {
                  Token &sep=*tokens[i-1];
                  if((sep=='.' || sep=="->" || sep=="::") && *tokens[i]=='*')i+=2;
               }

               Token &token=*tokens[i-1]; symbol_ptr=&token.symbol; token.def_decl=true;
               if(!symbol)
               {
                  if(!name.is())if(token.type==TOKEN_CODE)name=token;else return; // can only accept names as TOKEN_CODE (not keywords)
               }else
               if(symbol->type!=Symbol::KEYWORD)name=*symbol;else
               {
                  if(*symbol!="operator")return; // can't use any other keyword except 'operator'
                  skip_suggestions=true;
                  if(is_friend || !parent || parent->type!=Symbol::CLASS)parent=null; // in C++ global operators work from all namespaces, which means we must store them in global namespace
                  if(InRange(i+1, tokens))
                  {
                     Token &p=*tokens[i], &n=*tokens[i+1];
                     if(p=='(' && n==')'){name=  "operator()"      ; p.parent=start_parent; n.parent=start_parent; i+=2;}else
                     if(p=='[' && n==']'){name=  "operator[]"      ; p.parent=start_parent; n.parent=start_parent; i+=2;}else
                     if(p=='<' && n=='<'){name=  "operator<<"      ; p.parent=start_parent; n.parent=start_parent; i+=2;}else
                     if(p=='>' && n=='>'){name=  "operator>>"      ; p.parent=start_parent; n.parent=start_parent; i+=2;}else
                     if(p=='+'          ){name=  "operator+"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='-'          ){name=  "operator-"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='*'          ){name=  "operator*"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='/'          ){name=S+"operator"+DIV    ; p.parent=start_parent;                        i+=1;}else
                     if(p=='%'          ){name=  "operator%"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='!'          ){name=  "operator!"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='~'          ){name=  "operator~"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='='          ){name=  "operator="       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='<'          ){name=  "operator<"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='>'          ){name=  "operator>"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='&'          ){name=  "operator&"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='^'          ){name=  "operator^"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=='|'          ){name=  "operator|"       ; p.parent=start_parent;                        i+=1;}else
                     if(p=="->"         ){name=  "operator->"      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="++"         ){name=  "operator++"      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="--"         ){name=  "operator--"      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="+="         ){name=  "operator+="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="-="         ){name=  "operator-="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="*="         ){name=  "operator*="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="/="         ){name=S+"operator"+DIV+'='; p.parent=start_parent;                        i+=1;}else
                     if(p=="%="         ){name=  "operator%="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="&="         ){name=  "operator&="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="^="         ){name=  "operator^="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="|="         ){name=  "operator|="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="=="         ){name=  "operator=="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="!="         ){name=  "operator!="      ; p.parent=start_parent;                        i+=1;}else
                     if(p==">="         ){name=  "operator>="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="<="         ){name=  "operator<="      ; p.parent=start_parent;                        i+=1;}else
                     if(p=="==="        ){name=  "operator==="     ; p.parent=start_parent;                        i+=1;}else
                     if(p=="!!="        ){name=  "operator!!="     ; p.parent=start_parent;                        i+=1;}else
                     if(p=="<<="        ){name=  "operator<<="     ; p.parent=start_parent;                        i+=1;}else
                     if(p==">>="        ){name=  "operator>>="     ; p.parent=start_parent;                        i+=1;}else
                     if(p=="new"        ){name=  "operator new"    ; p.parent=start_parent;                        i+=1;}else
                     if(p=="delete"     ){name=  "operator delete" ; p.parent=start_parent;                        i+=1;}else return;
                  }else return;
               }
            }

            // set FUNC_LIST modifiers (those specified here will be the only ones set for FUNC_LIST, all others will be set to FUNC)
            UInt func_list_modifiers=0; // modifiers set to the FUNC_LIST before converting it to FUNC
            if(ctor_dtor)func_list_modifiers|=(dtor ? Symbol::MODIF_DTOR : Symbol::MODIF_CTOR);
            FlagSet(func_list_modifiers, Symbol::MODIF_SKIP_SUGGESTIONS, skip_suggestions);
            modifiers_in_bracket|=func_list_modifiers;

            // create the var func
            SymbolPtr &var_func=*symbol_ptr; var_func=symbols.New().require(UniqueVarFuncName(parent ? parent->firstNonTransparent() : null, name)).set(parent, is_typedef ? Symbol::TYPEDEF : Symbol::VAR, var_func_name_token_index, tokens[var_func_name_token_index]->line->source); // use first non-transparent parent for the symbol name (read more at MODIF_TRANSPARENT)
            if(var_func->valid==1)var_func->modifiers|=func_list_modifiers;

            // check if the var func has defined templates "<TYPE> void func()"
            Symbol *set_parent_var_func=start_parent; // set_parent which may be possibly set to function itself
            if(new_templates.elms()) // if templates are specified then it must be a function
            {
               set_parent_var_func=ConvertToFunc(symbols, &var_func, var_func_name_token_index, tokens[var_func_name_token_index]->line->source, ptr_level_in_bracket!=0 || (modifiers_in_bracket&Symbol::MODIF_REF)); // set the adjusted set_parent to the func, to allow template types detection (which belong to the func) inside the func params list
               Int template_classes=0; if(!var_func->insideClass())template_classes=var_func->templateClasses(); // how many parent classes are with templates
               FREPA(new_templates)ReadTemplates(symbols, tokens, new_templates[i], temp, *var_func, i>=template_classes);
               if(!type) // try detecting function type again after reading templates if it was unknown
               {
                  Int j=type_start; if(type=GetFullSymbol(tokens, j, temp, var_func(), templates))if(!(type->modifiers&Symbol::MODIF_DATA_TYPE))type=null;
               }
            }

            Bool func_params_setup=false;
            for(; i<tokens.elms(); )
            {
               Token &op=*tokens[i++]; op.parent=start_parent; // set parent of 'func' '(' ')' in "func(..)" to the class, so when evaluating the symbol of the func and checking from ')' backwards, we start from the same parent as the func parent
               switch(op[0])
               {
                  case '(': // parameter list or default value declaration
                  {
                     if(!func_params_setup)
                     {
                        func_params_setup=true;
                        SymbolPtr temp_parent=var_func->parent; // if we're processing functions which have parent set different than which defined then temporarily swap parents to detect the symbols: "class Parent { class Child { friend void func(Child &c); } }" - normally 'Child &c' would not be detected because 'func' parent is global namespace
                        if(!var_func->contains(start_parent))var_func->parent=start_parent; // adjusting parent needs to be done before calling 'IsVarFuncDefinition' because there the symbol may already get detected using the parent, perform this only if the new parent isn't actually a child of this symbol (which could cause never ending Symbol.parent->.. loops)

                        // check if these are default values
                        Int start=i; Symbol *type; Memc<Symbol::Modif> templates; // must use temporary 'templates' container
                        if(!(InRange(i, tokens) && *tokens[i]==')')                                     // if not "Vec v(|)"
                        && !IsVarFuncDefinition(tokens, i, temp, type, templates, set_parent_var_func)) // if not "Vec v(|int v)"
                        {  // default value
                           i=start;
                           SkipParams(tokens, i, set_parent_var_func);
                           if(var_func->valid==1){var_func->modifiers|=Symbol::MODIF_DEF_VALUE; var_func->def_val_range.set(start, i-1);}
                        }else
                        {  // function with params
                           i=start;
                           var_func->parent=temp_parent; // restore parent before converting to func
                           set_parent_var_func=ConvertToFunc(symbols, &var_func, var_func_name_token_index, tokens[var_func_name_token_index]->line->source, ptr_level_in_bracket!=0 || (modifiers_in_bracket&Symbol::MODIF_REF));
                           temp_parent=var_func->parent; // keep backup
                           if(!var_func->contains(start_parent))var_func->parent=start_parent; // adjust parent after converting to func, perform this only if the new parent isn't actually a child of this symbol (which could cause never ending Symbol.parent->.. loops)
                           ReadVarFuncs(symbols, tokens, i, SPACE_FUNC_PARAM, temp, set_parent_var_func);
                        }

                        var_func->parent=temp_parent; // restore parent
                        if(InRange(i, tokens) && *tokens[i]==')'    ) tokens[i++]->parent=start_parent; // adjust ')' parent
                        for(; InRange(i, tokens); i++)
                        {
                           Token &token=*tokens[i];
                           if(token=="const"   ){token.symbol="const"   ; token.parent=start_parent; modifiers_in_bracket|=Symbol::MODIF_FUNC_CONST   ;}else // ()const
                           if(token=="final"   ){token.symbol="final"   ; token.parent=start_parent; /*modifiers_in_bracket|=Symbol::MODIF_FUNC_FINAL   ;*/}else // ()final
                           if(token=="override"){token.symbol="override"; token.parent=start_parent; /*modifiers_in_bracket|=Symbol::MODIF_FUNC_OVERRIDE;*/}else // ()override
                           if(token=="throw"   ){token.symbol="throw"   ; token.parent=start_parent; SkipThrow(tokens, i, set_parent_var_func);        }else // throw(..)
                              break;
                        }
                     }else // Flt (*Func(Byte type)) (Flt s) {} - function which returns function, "Func(Byte type)" -> "Flt f(Flt s)"
                     {
                        SkipThrow(tokens, --i, set_parent_var_func);
                     }
                  }break;
                  
                  case '[': // array
                  {
                     for(Str dim; i<tokens.elms(); )
                     {
                        Token &token=*tokens[i++]; token.parent=set_parent_var_func;
                        if(token==']')
                        {
                           // TODO: watch out for error in calculation of "CalculateI(dim)"
                           Int d=CalculateI(dim); array_dims.NewAt(0)=((d>=1) ? d : Symbol::DIM_UNKNOWN);
                           goto finished_dim;
                        }
                        dim+=token;
                     }
                     DEBUG_ASSERT(op.line->source->cpp==false, "Invalid array definition");
                     return; // error
                  finished_dim:;
                  }break;

                  case '=': // default value
                  {
                     // for example: 5, x, 1*(x+2), Map<Image,Int>::static_var+Mems<X>::stat_var
                     Int start=i; ParseTemplates(tokens, i, temp, set_parent_var_func, space_mode==SPACE_TEMPLATE);
                     if(var_func->valid==1){modifiers_in_bracket|=Symbol::MODIF_DEF_VALUE; var_func->def_val_range.set(start, i-1);}
                  }break;

                  case ':': // constructor initializers "struct B : A {B() : A() {}}" or bit count "int x:1;"
                  {
                     if(InRange(i, tokens) && tokens[i]->type==TOKEN_CODE) // ctor initializer
                     {
                        op.ctor_initializer=true;
                        ConvertToFunc(symbols, &var_func, var_func_name_token_index, tokens[var_func_name_token_index]->line->source, false); // 'func_ptr_or_ref'=false because constructors are never a pointer to function
                        for(; i<tokens.elms(); )
                        {
                           Token &token=*tokens[i];
                           if(token.type==TOKEN_CODE)
                           {
                              Int   start=i; SkipUnknownSymbol(tokens, i, set_parent_var_func);
                              for(; start<i; )tokens[start++]->ctor_initializer=true;

                              if(InRange(i, tokens) && *tokens[i]=='(')
                              {
                                 Token &token=*tokens[i++];
                                 token.ctor_initializer=true;
                                 token.parent          =set_parent_var_func;
                                 for(Int level=0; i<tokens.elms(); )
                                 {
                                    Token &token=*tokens[i++]; token.parent=set_parent_var_func;
                                    if(token=='(')    level++;else
                                    if(token==')')if(!level--)break;
                                 }
                                 if(InRange(i, tokens) && *tokens[i]==',')
                                 {
                                    Token &token=*tokens[i++];
                                    token.ctor_initializer=true;
                                    token.parent          =set_parent_var_func;
                                    continue; // continue to next constructor call
                                 }
                              }
                           }
                           if(InRange(i, tokens))
                           {
                              Token &token=*tokens[i];
                              if(token==',' || token==';' || token=='{' || token=='}')break;
                              DEBUG_ASSERT(token.line->source->cpp==false, S+"Invalid constructor initializer.\nLine ("+token.lineIndex()+"): \""+*token.line+"\"\nSource: \""+token.line->source->loc.asText()+"\"");
                              token.parent=set_parent_var_func; i++;
                           }
                        }
                     }else // possibly bit count "int x:1;"
                     {
                        for(; i<tokens.elms(); )
                        {
                           Token &token=*tokens[i];
                           if(token==',' || token==';' || token=='{' || token=='}')break;
                           token.parent=set_parent_var_func; i++;
                        }
                     }
                  }break;

                  case ')':
                  case ',': // end of var/func declaration
                  case ';':
                  case '{':
                  default : // if we haven't finished declaring yet, and there's garbage after cursor "struct X{void name|};"
                  {
                     if(op[0]==')') // int (x), if(int x=..)
                     {
                        bracket_level--;
                        if(bracket_level>=0)break; // we still have possible tokens to process
                     }

                     DEBUG_ASSERT((op[0]==',' || op[0]==';' || op[0]==')' || op[0]=='{') || !op.line->source->cpp, S+"Unexpected Symbol.\nLine ("+op.lineIndex()+"): \""+*op.line+"\"\nSource: \""+op.line->source->loc.asText()+"\"");

                     if(op[0]=='{')
                     {
                        ConvertToFunc(symbols, &var_func, var_func_name_token_index, tokens[var_func_name_token_index]->line->source, false); // if we haven't finished declaring yet, but there's '{' "void x {" then force func, since we've encountered function body we know what this is not a pointer to function so 'func_ptr_or_ref'=false
                        modifiers_in_bracket|=Symbol::MODIF_FUNC_BODY;
                     }

                     if(!inside_class && var_func->Parent() && var_func->Parent()->type==Symbol::CLASS)modifiers_in_bracket|=Symbol::MODIF_OUTSIDE_METHOD;

                     if(var_func->valid==1)
                     {
                        var_func->type_range     .set(type_modif_start  , type_end);
                        var_func-> def_range     .set(var_func_def_start,      i-2);
                        var_func->value          =type;
                        var_func->value.templates=templates; // don't use 'Swap' because 'templates' may be used by multiple variables "Memc<Int> a, b;" (by both 'a' and 'b')
                        if(var_func->type==Symbol::FUNC) // "int& func()", "int& (&func)()" or "int& (*func)()"
                        {
                           // TODO:
                           DEBUG_ASSERT(array_dims.elms()==0, "Array of pointer to functions is not currently supported"); // change to compile error
                           var_func->      modifiers |=  modifiers_in_bracket|group_modifiers;
                           var_func->value.modifiers |=  modifiers;
                           var_func->   func_ptr_level=  ptr_level_in_bracket;
                           var_func->value.  ptr_level=  ptr_level;
                           var_func->value.const_level=const_level|group_const_level;
                        }else // "int x", "int* x", "int *x[2]" - 2 pointers to 1 int, "int (*)x[2]" - 1 pointer to 2 int's, "int* (*)x[2]" - 1 pointer to 2 int*
                        {
                           var_func->      modifiers|=((modifiers|modifiers_in_bracket|group_modifiers) & ~Symbol::MODIF_REF); // set all except MODIF_REF
                           var_func->value.modifiers|=((modifiers|modifiers_in_bracket|group_modifiers) &  Symbol::MODIF_REF); // set only       MODIF_REF
                           if(array_dims.elms())
                           {
                              // order: 'ptr_level', 'array_dims', 'ptr_level_in_bracket' (stored in 'array_dims' as DIM_PTR)
                              var_func->value.  ptr_level=  ptr_level; // set only base pointers
                              var_func->value.const_level=const_level|(const_level_in_bracket<<(ptr_level+array_dims.elms()))|group_const_level;
                              FREPA(array_dims          )var_func->value.const_level|=(1<<(ptr_level+1+i)); // +1 is because for "int x[2];" 'ptr_level' is 0 so '0+i' would make "const int x[2];"
                              FREP (ptr_level_in_bracket)array_dims.add(Symbol::DIM_PTR); // append 'array_dims' with DIM_PTR from 'ptr_level_in_bracket'
                              Swap(var_func->value.array_dims, array_dims);
                           }else
                           {
                              var_func->value.  ptr_level=  ptr_level+   ptr_level_in_bracket;
                              var_func->value.const_level=const_level|(const_level_in_bracket<<ptr_level)|group_const_level;
                           }
                        }
                     }

                     // assign newly created var func to 'children' container of the parent FUNC "<TYPE> void func(int p) {int x=0;}" (this does not process func params like 'p' or templates like 'TYPE'), this is needed for compilation
                     if(var_func && var_func->parent)
                        if(Symbol *var_func_parent_func=var_func->parent->func())
                     {
                        if(space_mode==SPACE_NORMAL    )var_func_parent_func->children.add(var_func());else
                        if(space_mode==SPACE_FUNC_PARAM)var_func_parent_func->params  .add(var_func());
                     }

                     if(op[0]=='{') // fill all until '}' with parent as newly created symbol
                     {
                        op.parent=var_func();
                        for(Int level=0; i<tokens.elms(); )
                        {
                           Token &token=*tokens[i++]; token.parent=var_func();
                           if(token=='{')level++;else
                           if(token=='}')level--;
                           if(level<0)break;
                        }
                     }else
                     if(op[0]==',') // reset individual modifiers, and proceed to next var/member def/decl
                     {
                        if(space_mode==SPACE_NORMAL)
                        {
                           name.clear();
                           const_level=0; const_level_in_bracket=0;
                             ptr_level=0;   ptr_level_in_bracket=0;
                             modifiers=0;   modifiers_in_bracket=0;
                           var_func_def_start=i;
                           goto next_var_func;
                        }else
                        {
                           return;
                        }
                     }else
                     if(op[0]==')')
                     {
                        i--;
                     }
                  }return;
               }
            }
         }return;

         default: return;
      }
   next_var_func:;
   }
}
/******************************************************************************/
void DetectVarFuncs(Memc<SymbolDef> &symbols, Memc<Token*> &tokens) // get list of 2nd main symbols (variables, functions)
{
   Symbol             *symbol_parent;
   Str                 temp;
   Memc<Symbol::Modif> templates;
   Memc<Int          > new_templates;
   for(Int i=0; i<tokens.elms(); )
   {
      Int    start=i;
      Token &token=*tokens[i];

      if(!token.symbol) // if not yet processed by DataTypes/Typedefs
         if(!token.parent || token.parent->type==Symbol::NAMESPACE || token.parent->type==Symbol::CLASS) // detect only in namespaces and class bodies
      {
         if(Symbol *symbol=GetFullSymbol(tokens, i, temp, token.parent, templates, &symbol_parent))
         {
            if(symbol->modifiers&Symbol::MODIF_DATA_TYPE) // if current element is of data type, then probably it's member/var/method/function definition/declaration
               ReadVarFunc(symbol, symbols, tokens, i, start, SPACE_NORMAL, temp, templates, new_templates, symbol_parent);
            else
            if(symbol->type==Symbol::KEYWORD)
            {
               if(*symbol=="operator") // or auto-cast "operator CChar*()"
                  ReadAutoCast(symbol_parent, symbols, tokens, i, start, temp, templates);
               else
               if(token.parent && token.parent->type==Symbol::CLASS) // or access-level specification
                  if(InRange(i, tokens) && *tokens[i]==':') // only for "private:" and not "private int"
               {
                  if(*symbol=="private"  )token.parent->access_level=Symbol::ACCESS_PRIVATE  ;else
                  if(*symbol=="protected")token.parent->access_level=Symbol::ACCESS_PROTECTED;else
                  if(*symbol=="public"   )token.parent->access_level=Symbol::ACCESS_PUBLIC   ;
               }
            }
         }else
         if(token=='<' || token==TMPL_B) // parse "template<..>"
         {
            for(Int level=0; i<tokens.elms(); ) // skip all templates inside
            {
               Token &token=*tokens[i];
               if(token=='{' || token=='}' || token==';')break;
               i++; token.def_decl=true;
               if(token=='<' || token==TMPL_B)   ++level;else
               if(token=='>' || token==TMPL_E)if(--level<=0) // when last '>' encountered
               {
                  if(InRange(i, tokens)) // check what follows "template<..>"
                  {
                     Token &token=*tokens[i];
                     if(token!="struct" && token!="class" && token!="union" && token!="namespace" // check if it's not "template<..>        struct"
                     && token!="friend"                                                           // check if it's not "template<..> friend struct"
                     && token!="template" && token!="<" && token!=TMPL_B)                         // check if it's not "template<..> template"
                     {
                        for(; InRange(i, tokens); ) // skip modifiers
                        {
                           Token &token=*tokens[i];
                           if(token=="inline"
                           || token=="const"
                           || token=="constexpr"
                           || token=="typename"
                           || token=="static"
                           || token=="mutable")i++;else break;
                        }
                        if(InRange(i, tokens)) // now we should encounter data type (this can be template typename)
                        {
                           Int     type_start=i;
                           Symbol *type =GetFullSymbol(tokens, i, temp, token.parent, templates, &symbol_parent); if(type && type->type==Symbol::TYPENAME){type=null; templates.clear();} // skip typenames, in case they're detected from parent "T1(TYPE) class X {T1(TYPE) void x();}"
                           if(!type){i=type_start; SkipUnknownSymbol(tokens, i, token.parent);} // if type was not detected, then skip it fully

                           if(type && *type=="operator") // auto-cast "operator TYPE()"
                              ReadAutoCast(symbol_parent, symbols, tokens, i, type_start, temp, templates);
                           else
                           if(InRange(i, tokens))
                           {
                              Token &token=*tokens[i];
                              if(token!='{' && token!='}' && token!=';' && token!=',') // skip "class Ext : Base<TYPE>, Base<TYPE2> {}"
                                 ReadVarFunc(type, symbols, tokens, i, type_start, SPACE_NORMAL, temp, templates, new_templates, symbol_parent);
                           }
                        }
                     }
                  }
                  break;
               }
            }
         }else
         if(token.line->source->cpp) // allow variable definition after struct/class/union/enum definition "class X{}x;" (remember it can also be "typedef class X{}x;" in which case 'x' is typedef for 'X' class)
         {
            if(token=='}' && token.parent)if(token.parent->type==Symbol::CLASS || token.parent->type==Symbol::ENUM)
            {
               i=start+1;
               if(InRange(i, tokens))
               {
                  if(tokens[i]->parent==token.parent)
                  {
                     DEBUG_ASSERT(false, S+"Trying to read Var/Func after class/enum definition while still in class.\nLine ("+token.lineIndex()+"): \""+*token.line+"\"\nSource: \""+token.line->source->loc.asText()+"\"");
                  }else
                  if(!tokens[i]->symbol) // if not yet processed by DataTypes/Typedefs
                  {
                     if(*tokens[i]!=';')FlagDisable(token.parent->modifiers, Symbol::MODIF_TRANSPARENT); // if the class has a variable defined, then it can be accessed through it, and will not be a transparent class
                     ReadVarFunc(token.parent, symbols, tokens, i, token.parent->token_index, SPACE_NORMAL, temp, templates, new_templates);
                  }
               }
            }
         }
      }

      MAX(i, start+1);
   }
}
/******************************************************************************/
Int GetSymbolStart(Memc<Token*> &tokens, Int i)
{
   if(!InRange(i, tokens))return -1;
   for(Int level=0; i>=0; i--)
   {
      Token &c=*tokens[i];
      if(level)
      {
         if(c=='}')level++;else
         if(c=='{')level--;
      }else
      {
         if(c=='{')break;
         if(c=='}')if(InRange(i+1, tokens) && *tokens[i+1]==',')level++;else break; // allow going through "{}" if followed by ',', like this: "int a, b[]={1,2}, c;" when starting from the end
      }
      if(c==';')break;
      if(c==':') // this can be: label "found:", "case 5:", "public/private/protected:", "x ? y : z"
      {          // we need to stop on all except "?:"
                 // watch out for mixed "case (1 ? 2 : 3): (4 ? 5: 6);"
         for(Int level=0, j=i; --j>=0; )
         {
            Token &c=*tokens[j];
            if(c.symbol)
               if(c.symbol->type==Symbol::LABEL
               || c.symbol->type==Symbol::KEYWORD && (c=="public" || c=="private" || c=="protected" || c=="case" || c=="default"))goto end;
            if(c==':')level++;else
            if(c=='?')if(!level--){i=j; break;}
         }
      }
   }
end:
   return i+1;
}
/******************************************************************************/
}}
/******************************************************************************/
