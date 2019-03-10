/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Edit{
/******************************************************************************/
ColorTheme ThemeLight=
{
   Color(255, 255, 255), // NONE
   TRANSPARENT,          // REMOVE
   Color(  0,   0,   0), // OPERATOR
   Color(  0,   0, 255), // KEYWORD
   Color(  0,   0,   0), // CODE
   Color(  0, 128,   0), // COMMENT
   Color(163,  21,  21), // TEXT8
   Color(163,  21,  21), // TEXT16
   Color(163,  21,  21), // CHAR8
   Color(163,  21,  21), // CHAR16
   Color(  0,   0,   0), // NUMBER
   Color(  0,   0, 255), // PREPROC
   Color(  0,   0, 255), // MACRO
   Color(  0,   0, 255), // ENUM_TYPE
   Color(  0,   0,   0), // ENUM_ELM
   Color(  0,   0,   0), // FUNC
   Color( 51, 153, 255), // SELECT
   Color(  0,   0,   0, 96), // LINE_HIGHLIGHT
   Color( 64, 128, 255, 60), // SYMBOL_HIGHLIGHT
   Color( 64, 128, 255, 40), //  BRACE_HIGHLIGHT
   Color(128, 128, 128), // PREPROC_DISABLED
   Color( 86,  86, 244, 239), // TOKEN_ELM_BACKGROUND
   Color(255, 255, 255), // TOKEN_ELM_NAME
   Color(220, 220, 220), // TOKEN_LINE_NUM_BACKGROUND
   Color(143, 143, 143), // TOKEN_LINE_NUM
}, ThemeBlue=
{
   Color(  0,   0,  96), // NONE
   TRANSPARENT,          // REMOVE
   Color(255, 255, 255), // OPERATOR
   Color(255, 255, 255), // KEYWORD
   Color(255, 255,   0), // CODE
   Color(192, 192, 192), // COMMENT
   Color(  0, 255, 255), // TEXT8
   Color(  0, 255, 255), // TEXT16
   Color(  0, 255, 255), // CHAR8
   Color(  0, 255, 255), // CHAR16
   Color(  0, 255, 255), // NUMBER
   Color(  0, 255,   0), // PREPROC
   Color(255, 255, 255), // MACRO
   Color(255, 255, 255), // ENUM_TYPE
   Color(255, 255,   0), // ENUM_ELM
   Color(255, 255,   0), // FUNC
   Color( 51, 153, 255), // SELECT
   Color(255, 255,   0, 128), // LINE_HIGHLIGHT
   Color(255, 255, 255, 85), // SYMBOL_HIGHLIGHT
   Color(255, 255, 255, 40), //  BRACE_HIGHLIGHT
   Color(128, 128, 128), // PREPROC_DISABLED
   Color( 36, 106, 172, 245), // TOKEN_ELM_BACKGROUND
   Color(255, 255, 255), // TOKEN_ELM_NAME
   Color(  0,   0,  75), // TOKEN_LINE_NUM_BACKGROUND
   Color(128, 128, 128), // TOKEN_LINE_NUM
}, ThemeDark=
{
   Color( 30,  30,  30), // NONE
   TRANSPARENT,          // REMOVE
   Color(220, 220, 220), // OPERATOR
   Color( 86, 156, 214), // KEYWORD
   Color(200, 200, 200), // CODE
   Color( 87, 166,  74), // COMMENT
   Color(211, 155, 105), // TEXT8
   Color(211, 155, 105), // TEXT16
   Color(211, 155, 105), // CHAR8
   Color(211, 155, 105), // CHAR16
   Color(181, 206, 168), // NUMBER
   Color(155, 155, 155), // PREPROC
   Color( 86, 156, 214), // MACRO
   Color( 86, 156, 214), // ENUM_TYPE
   Color(161, 193, 225)/*Color(184, 215, 163)/*Color(200, 200, 200)*/, // ENUM_ELM
   Color(200, 200, 200), // FUNC
   Color( 38,  79, 120), // SELECT
   Color(255, 255, 255, 80), // LINE_HIGHLIGHT
   Color(255, 255, 255, 45), // SYMBOL_HIGHLIGHT
   Color(255, 255, 255, 30), //  BRACE_HIGHLIGHT
   Color( 90,  90,  90), // PREPROC_DISABLED
   Color( 36, 106, 172, 245), // TOKEN_ELM_BACKGROUND
   Color(255, 255, 255), // TOKEN_ELM_NAME
   Color( 42,  42,  42), // TOKEN_LINE_NUM_BACKGROUND
   Color(128, 128, 128), // TOKEN_LINE_NUM
}, ThemeCustom=ThemeDark,
   Theme      =ThemeDark;
/******************************************************************************/
void ColorTheme::save(TextNode &node)C
{
   node.getNode("Background"          ).value=S+colors[TOKEN_NONE               ].v4;
   node.getNode("Operator"            ).value=S+colors[TOKEN_OPERATOR           ].v4;
   node.getNode("Keyword"             ).value=S+colors[TOKEN_KEYWORD            ].v4;
   node.getNode("Code"                ).value=S+colors[TOKEN_CODE               ].v4;
   node.getNode("Comment"             ).value=S+colors[TOKEN_COMMENT            ].v4;
   node.getNode("Text"                ).value=S+colors[TOKEN_TEXT8              ].v4;
   node.getNode("Character"           ).value=S+colors[TOKEN_CHAR8              ].v4;
   node.getNode("Number"              ).value=S+colors[TOKEN_NUMBER             ].v4;
   node.getNode("Preprocessor"        ).value=S+colors[TOKEN_PREPROC            ].v4;
   node.getNode("PreprocessorDisabled").value=S+colors[TOKEN_PREPROC_DISABLED   ].v4;
   node.getNode("Macro"               ).value=S+colors[TOKEN_MACRO              ].v4;
   node.getNode("EnumType"            ).value=S+colors[TOKEN_ENUM_TYPE          ].v4;
   node.getNode("EnumElement"         ).value=S+colors[TOKEN_ENUM_ELM           ].v4;
   node.getNode("Function"            ).value=S+colors[TOKEN_FUNC               ].v4;
   node.getNode("Selection"           ).value=S+colors[TOKEN_SELECT             ].v4;
   node.getNode("LineHighlight"       ).value=S+colors[TOKEN_LINE_HIGHLIGHT     ].v4;
   node.getNode("SymbolHighlight"     ).value=S+colors[TOKEN_SYMBOL_HIGHLIGHT   ].v4;
   node.getNode("BraceHighlight"      ).value=S+colors[TOKEN_BRACE_HIGHLIGHT    ].v4;
   node.getNode("ElementBackground"   ).value=S+colors[TOKEN_ELM_BACKGROUND     ].v4;
   node.getNode("ElementName"         ).value=S+colors[TOKEN_ELM_NAME           ].v4;
   node.getNode("LineNumberBackground").value=S+colors[TOKEN_LINE_NUM_BACKGROUND].v4;
   node.getNode("LineNumber"          ).value=S+colors[TOKEN_LINE_NUM           ].v4;
}
void ColorTheme::load(C TextNode &node)
{
   if(C TextNode *p=node.findNode("Background"          ))colors[TOKEN_NONE               ]=p->asColor();
   if(C TextNode *p=node.findNode("Operator"            ))colors[TOKEN_OPERATOR           ]=p->asColor();
   if(C TextNode *p=node.findNode("Keyword"             ))colors[TOKEN_KEYWORD            ]=p->asColor();
   if(C TextNode *p=node.findNode("Code"                ))colors[TOKEN_CODE               ]=p->asColor();
   if(C TextNode *p=node.findNode("Comment"             ))colors[TOKEN_COMMENT            ]=p->asColor();
   if(C TextNode *p=node.findNode("Text"                ))colors[TOKEN_TEXT8              ]=colors[TOKEN_TEXT16]=p->asColor();
   if(C TextNode *p=node.findNode("Character"           ))colors[TOKEN_CHAR8              ]=colors[TOKEN_CHAR16]=p->asColor();
   if(C TextNode *p=node.findNode("Number"              ))colors[TOKEN_NUMBER             ]=p->asColor();
   if(C TextNode *p=node.findNode("Preprocessor"        ))colors[TOKEN_PREPROC            ]=p->asColor();
   if(C TextNode *p=node.findNode("PreprocessorDisabled"))colors[TOKEN_PREPROC_DISABLED   ]=p->asColor();
   if(C TextNode *p=node.findNode("Macro"               ))colors[TOKEN_MACRO              ]=p->asColor();
   if(C TextNode *p=node.findNode("EnumType"            ))colors[TOKEN_ENUM_TYPE          ]=p->asColor();
   if(C TextNode *p=node.findNode("EnumElement"         ))colors[TOKEN_ENUM_ELM           ]=p->asColor();
   if(C TextNode *p=node.findNode("Function"            ))colors[TOKEN_FUNC               ]=p->asColor();
   if(C TextNode *p=node.findNode("Selection"           ))colors[TOKEN_SELECT             ]=p->asColor();
   if(C TextNode *p=node.findNode("LineHighlight"       ))colors[TOKEN_LINE_HIGHLIGHT     ]=p->asColor();
   if(C TextNode *p=node.findNode("SymbolHighlight"     ))colors[TOKEN_SYMBOL_HIGHLIGHT   ]=p->asColor();
   if(C TextNode *p=node.findNode("BraceHighlight"      ))colors[TOKEN_BRACE_HIGHLIGHT    ]=p->asColor();
   if(C TextNode *p=node.findNode("ElementBackground"   ))colors[TOKEN_ELM_BACKGROUND     ]=p->asColor();
   if(C TextNode *p=node.findNode("ElementName"         ))colors[TOKEN_ELM_NAME           ]=p->asColor();
   if(C TextNode *p=node.findNode("LineNumberBackground"))colors[TOKEN_LINE_NUM_BACKGROUND]=p->asColor();
   if(C TextNode *p=node.findNode("LineNumber"          ))colors[TOKEN_LINE_NUM           ]=p->asColor();
}
/******************************************************************************/
void CodeEditor::HideAndFocusCE(GuiObj &go) {go.hide(); CE.cei().kbSet();}
/******************************************************************************/
CodeEditor::MenuBarEx& CodeEditor::MenuBarEx::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      super::rect(rect);
      Flt h=T.rect().h();

      CE.view_mode.rect(Rect_L (   T        .elmsRect().right()+Vec2(0.01f, 0), 0.21f, h*0.84f));
      CE.view_what.rect(Rect_LU(CE.view_mode.    rect().ru   ()               , CE.view_mode.rect().h()));

      CE.b_close       .rect(Rect_RU(   T      .rect().ru  (), h  , h));
      CE.build_progress.rect(Rect_R (CE.b_close.rect().left(), h*4, h*0.6f));
   }
   return T;
}
void CodeEditor::MenuBarEx::parentClientRectChanged(C Rect *old_client, C Rect *new_client)
{
   if(new_client)
   {
      Rect r=CE.cei().menuRect()&*new_client;
      if(CE.menu_on_top)rect(Rect_LU(r.min.x, r.max.y, r.w(), 0.06f));else rect(Rect_LD(r.min.x, r.min.y, r.w(), 0.06f));
   }
}
/******************************************************************************/
void CodeEditor::GotoLineWindow::create(CodeEditor &ce)
{
   if(ce.parent)
   {
      T.ce=&ce;
     *ce.parent+=super  ::create(Rect_C(0, 0, 1, 0.14f), "Go To Line").hide(); button[2].func(HideAndFocusCE, SCAST(GuiObj, T)).show();
      T        +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01f));
   }
}
GuiObj& CodeEditor::GotoLineWindow::activate()
{
   if(ce && ce->cur() && hidden())
   {
      setTitle(S+"Go To Line (1.."+Max(1, ce->cur()->lines.elms())+')');
      textline.set(S+(ce->cur()->cur.y+1)).selectAll();
   }
   return super::activate();
}
void CodeEditor::GotoLineWindow::update(C GuiPC &gpc)
{
   super::update(gpc);

   if(Gui.window()==this)
   {
      if(Kb.k(KB_ENTER)){if(ce && ce->cur()){ce->markCurPos(); ce->cur()->highlight(TextInt(textline())-1, false);} button[2].push(); Kb.eatKey();}
   }
}
/******************************************************************************/
Window& CodeEditor::Options::VSVersions::show()
{
   if(visible())return super::show();

   // #VisualStudio
   versions.del();
   Memc<VisualStudioInstallation> installs; GetVisualStudioInstallations(installs); REPA(installs) // list newest first
   {
    C VisualStudioInstallation &install=installs[i];
      T+=versions.New().create(Vec2(clientWidth()/2, -0.07f-versions.elms()*0.08f), install, ce);
   }
   if(versions.elms())
   {
      size(Vec2(size().x, barHeight() + -versions.last().rect().min.y+0.07f));
      return super::show();
   }else
   {
      hide();
      Str msg; CheckVisualStudio(0, &msg); Error(msg);
      return T;
   }
}
void CodeEditor::Options::VSVersions::create(CodeEditor &ce)
{
   T.ce=&ce;
   Gui+=super::create(Rect_C(0, 0, 1.0f, 0.44f), "Visual Studio Versions Detected").hide(); button[2].show();
}
/******************************************************************************/
static void ThemeColorChanged(C Property &prop)
{
   ThemeCustom.colors[TOKEN_TEXT16]=ThemeCustom.colors[TOKEN_TEXT8];
   ThemeCustom.colors[TOKEN_CHAR16]=ThemeCustom.colors[TOKEN_CHAR8];
   CE.themeChanged();
}
CodeEditor::Options::ColorThemeEditor& CodeEditor::Options::ColorThemeEditor::create()
{
   props.New().create("Background"            , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_NONE               ]))).setColor();
   props.New().create("Operator"              , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_OPERATOR           ]))).setColor();
   props.New().create("Keyword"               , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_KEYWORD            ]))).setColor();
   props.New().create("Code"                  , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_CODE               ]))).setColor();
   props.New().create("Comment"               , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_COMMENT            ]))).setColor();
   props.New().create("Text"                  , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_TEXT8              ]))).setColor();
   props.New().create("Character"             , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_CHAR8              ]))).setColor();
   props.New().create("Number"                , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_NUMBER             ]))).setColor();
   props.New().create("Preprocessor"          , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_PREPROC            ]))).setColor();
   props.New().create("Preprocessor Disabled" , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_PREPROC_DISABLED   ]))).setColor();
   props.New().create("Macro"                 , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_MACRO              ]))).setColor();
   props.New().create("Enum Type"             , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_ENUM_TYPE          ]))).setColor();
   props.New().create("Enum Element"          , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_ENUM_ELM           ]))).setColor();
   props.New().create("Function"              , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_FUNC               ]))).setColor();
   props.New().create("Line Highlight"        , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_LINE_HIGHLIGHT     ]))).setColor();
   props.New().create("Symbol Highlight"      , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_SYMBOL_HIGHLIGHT   ]))).setColor();
   props.New().create("Brace Highlight"       , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_BRACE_HIGHLIGHT    ]))).setColor();
   props.New().create("Selection"             , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_SELECT             ]))).setColor();
   props.New().create("Element Background"    , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_ELM_BACKGROUND     ]))).setColor();
   props.New().create("Element Name"          , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_ELM_NAME           ]))).setColor();
   props.New().create("Line Number Background", MemberDesc(MEMBER(ColorTheme, colors[TOKEN_LINE_NUM_BACKGROUND]))).setColor();
   props.New().create("Line Number"           , MemberDesc(MEMBER(ColorTheme, colors[TOKEN_LINE_NUM           ]))).setColor();

   ts.reset(true).size=0.036f; ts.align.set(1, 0);
   Gui+=super::create("Color Theme Editor").hide();
   Rect r=AddProperties(props, T, Vec2(0.02f, -0.02f), 0.043f, 0.3f, &ts); r.max+=defaultInnerPaddingSize()+0.04f;
   rect(Rect_RD(D.w(), -D.h(), r.w(), r.h()));
   REPAO(props).autoData(&ThemeCustom).changed(ThemeColorChanged); button[2].show();
   return T;
}
void CodeEditor::Options::ColorThemeEditor::skinChanged()
{
   ts.resetColors(true);
}
/******************************************************************************/
CodeEditor::Options::FontData::FontData()
{
   size   =18;
   spacing.set(0.59f, 1.05f);
   font   ="Lucida Console";
}
CodeEditor::Options::FontParams::FontParams()
{
   sample_text="bool Init()\\n{\\n   int variable=123;\\n}\\nclass Test\\n{\\n   Str member=\"value\";\\n}\\n";
   chinese=japanese=korean=false;
}
static CChar8 *Ansi="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()[]<>{}`~_-+=;:,.?/|\\'\" ";
Str CodeEditor::Options::FontData::chars()C
{
   Str c;
   if(1      )c+=Ansi;
   if(1      )c+=Ellipsis;
   if(1      )c+=LanguageSpecific(DE);
   if(1      )c+=LanguageSpecific(FR);
   if(1      )c+=LanguageSpecific(PL);
   if(1      )c+=LanguageSpecific(RU);
   REPA(langs)c+=LanguageSpecific(langs[i]);
   return c+custom_chars;
}
Bool CodeEditor::Options::FontData::make(Font &font, C Str *chars)C
{
   Font::Params fp;
   fp.system_font    =T.font;
   fp.characters     =(chars ? *chars : T.chars());
   fp.size           =size;
   fp.mode           =Font::SUB_PIXEL;
   fp.mip_maps       =1;
   fp.shadow_blur    =0;
   fp.shadow_diagonal=false;
   fp.shadow_opacity =0;
   fp.shadow_spread  =0;
   return font.create(fp);
}
void CodeEditor::Options::FontData::save(TextNode &node)C
{
          node.getNode("size"        ).value=S+size;
          node.getNode("spacing.x"   ).value=S+spacing.x;
          node.getNode("spacing.y"   ).value=S+spacing.y;
          node.getNode("font"        ).value=font;
          node.getNode("custom_chars").value=custom_chars;
   Str &l=node.getNode("languages"   ).value; l.clear(); FREPA(langs)l.space()+=langs[i];
}
void CodeEditor::Options::FontData::load(C TextNode &node)
{
   langs.clear();
   if(C TextNode *p=node.findNode("size"        ))size        =Mid(p->asInt(), 1, 256);
   if(C TextNode *p=node.findNode("spacing.x"   ))spacing.x   =Mid(p->asFlt(), 0.0f, 1.0f);
   if(C TextNode *p=node.findNode("spacing.y"   ))spacing.y   =Mid(p->asFlt(), 0.0f, 2.0f);
   if(C TextNode *p=node.findNode("font"        ))font        =p->value;
   if(C TextNode *p=node.findNode("custom_chars"))custom_chars=p->value;
   if(C TextNode *p=node.findNode("languages"   )){Memt<Str> l; Split(l, p->value, ' '); FREPA(l)if(l[i].is())langs.include((LANG_TYPE)TextInt(l[i]));}
}
void CodeEditor::Options::FontParams::load(C TextNode &node)
{
   super::load(node);
   chinese =langs.has(CN);
   japanese=langs.has(JP);
   korean  =langs.has(KO);
}
static void ParamsFont      (CodeEditor::Options::FontParams &p, C Str &t) {   p.font        =         t ;}
static void ParamsSize      (CodeEditor::Options::FontParams &p, C Str &t) {   p.size        =TextInt (t);}
static void ParamsSpacingX  (CodeEditor::Options::FontParams &p, C Str &t) {   p.spacing.x   =TextFlt (t);}
static void ParamsSpacingY  (CodeEditor::Options::FontParams &p, C Str &t) {   p.spacing.y   =TextFlt (t);}
static void ParamsChinese   (CodeEditor::Options::FontParams &p, C Str &t) {if(p.chinese     =TextBool(t))p.langs.include(CN);else p.langs.exclude(CN);}
static void ParamsJapanese  (CodeEditor::Options::FontParams &p, C Str &t) {if(p.japanese    =TextBool(t))p.langs.include(JP);else p.langs.exclude(JP);}
static void ParamsKorean    (CodeEditor::Options::FontParams &p, C Str &t) {if(p.korean      =TextBool(t))p.langs.include(KO);else p.langs.exclude(KO);}
static void ParamsCustomChar(CodeEditor::Options::FontParams &p, C Str &t) {   p.custom_chars=         t ;}

static void Preview(Viewport &viewport) {((CodeEditor::Options::FontEditor*)viewport.user)->preview();}
       void CodeEditor::Options::FontEditor::preview()
{
   SyncLocker      locker(lock);
   Rect            rect=D.viewRect();
   TextStyleParams ts; ts.font(&font); ts.color=BLACK; ts.shadow=0; ts.setPerPixelSize(); ts.align.set(1, -1); ts.spacing=SPACING_CONST; ts.space.set(params.spacing.x, AlignRound(params.spacing.y, 1.0f/font.height()));

   rect.draw(WHITE);
   D.text(ts, rect.extend(-0.01f), Replace(params.sample_text, "\\n", "\n"));
}
static Bool Make(Thread &thread) {return (*(CodeEditor::Options::FontEditor*)thread.user).make();}
       Bool CodeEditor::Options::FontEditor::make()
{
   if(visible())
   {
      SyncLockerEx locker(lock); CodeEditor::Options::FontParams params=T.params; locker.off();
      ThreadMayUseGPUData();
      Font temp; if(params.make(temp, &params.sample_text)){locker.on(); Swap(font, temp); locker.off();}
   }
   Time.wait(10);
   return true;
}

static void Apply(CodeEditor::Options::FontEditor &font_edit)
{
   Font temp; if(font_edit.params.make(temp))
   {
      File f; temp.save(f.writeMem()); f.pos(0); if(SafeOverwrite(f, "Bin/Code Editor.font"))
      {
         FontPtr font; if(font.find("Bin/Code Editor.font"))Swap(*font, temp);
         CE.fontChanged();
      }
   }
}

CodeEditor::Options::FontEditor& CodeEditor::Options::FontEditor::hide() {if(visible())thread.stop  (          ); super::hide(); return T;}
CodeEditor::Options::FontEditor& CodeEditor::Options::FontEditor::show() {if(hidden ())thread.create(Make, this); super::show(); return T;}

CodeEditor::Options::FontEditor& CodeEditor::Options::FontEditor::del()
{
   thread.del(); super::del(); return T;
}
CodeEditor::Options::FontEditor& CodeEditor::Options::FontEditor::create()
{
   props.New().create("System Font" , MemberDesc(MEMBER(FontParams, font        )).setTextToDataFunc(ParamsFont      ));
   props.New().create("Size"        , MemberDesc(MEMBER(FontParams, size        )).setTextToDataFunc(ParamsSize      )).range(8, 48);
   props.New().create("Char Spacing", MemberDesc(MEMBER(FontParams, spacing.x   )).setTextToDataFunc(ParamsSpacingX  )).range(0.45f, 0.75f).desc("Spacing between characters, default=0.59");
   props.New().create("Line Spacing", MemberDesc(MEMBER(FontParams, spacing.y   )).setTextToDataFunc(ParamsSpacingY  )).range(0.65f, 1.25f).desc("Spacing between lines, default=1.05");
   props.New().create("Chinese"     , MemberDesc(MEMBER(FontParams, chinese     )).setTextToDataFunc(ParamsChinese   )).desc("Will include Chinese characters in the font");
   props.New().create("Japanese"    , MemberDesc(MEMBER(FontParams, japanese    )).setTextToDataFunc(ParamsJapanese  )).desc("Will include Japanese characters in the font");
   props.New().create("Korean"      , MemberDesc(MEMBER(FontParams, korean      )).setTextToDataFunc(ParamsKorean    )).desc("Will include Korean characters in the font");
   props.New().create("Custom Chars", MemberDesc(MEMBER(FontParams, custom_chars)).setTextToDataFunc(ParamsCustomChar)).desc("Will include custom characters in the font");
   props.New();
   props.New().create("Display:");
   props.New().create("Text", MemberDesc(MEMBER(FontParams, sample_text)));

   ts.reset(true).size=0.036f; ts.align.set(1, 0);
   Gui+=super::create("Font Editor").hide();
   Rect r=AddProperties(props, T, Vec2(0.02f, -0.02f), 0.043f, 0.3f, &ts);
   REPAO(props).autoData(&params); button[2].show();
   T+=apply.create(Rect_U(r.down()-Vec2(0, 0.043f), 0.3f, 0.05f), "Apply").func(Apply, T);
   T+=viewport.create(Rect_LU(0.56f, -0.02f, 0.72f, -apply.rect().min.y-0.02f), Preview, this);
   Vec2 size(viewport.rect().max.x, -apply.rect().min.y); size+=defaultInnerPaddingSize()+0.02f;
   rect(Rect_RD(D.w(), -D.h(), size.x, size.y));
   return T;
}
void CodeEditor::Options::FontEditor::skinChanged()
{
   ts.resetColors(true);
}
/******************************************************************************/
void CodeEditor::Options::skinChanged()
{
   ts.resetColors(true);

   color_theme_editor.skinChanged();
          font_editor.skinChanged();
}
Window& CodeEditor::Options::show()
{
   super::show();
   if(ce)ce->cei().visibleChangedOptions();
   return T;
}
Window& CodeEditor::Options::hide()
{
   w_vs_path         .hide();
   w_netbeans_path   .hide();
   w_android_sdk     .hide();
   w_android_ndk     .hide();
   vs_versions       .hide();
   color_theme_editor.hide();
   font_editor       .hide();
               super::hide();
   if(ce)ce->cei().visibleChangedOptions();
   return T;
}
static void  VSDownload    (             CodeEditor::Options &op) {Explore("https://www.visualstudio.com/downloads/download-visual-studio-vs#d-community");}
static void  NBDownload    (             CodeEditor::Options &op) {Explore("https://netbeans.org/downloads/start.html?platform=linux&lang=en&option=cpp&bits=x64");}
static void EASDownload    (             CodeEditor::Options &op) {Explore("http://www.esenthel.com/?id=store");}
static void  ASDownload    (             CodeEditor::Options &op) {Explore("https://developer.android.com/studio/#command-tools");}
static void  ANDownload    (             CodeEditor::Options &op) {Explore("https://developer.android.com/ndk/downloads/");}
static void JDKDownload    (             CodeEditor::Options &op) {Explore("https://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html");}
static void   VSChanged    (             CodeEditor::Options &op) {if(op.ce)op.ce->setVSPath  (op.      vs_path());}
static void   NBChanged    (             CodeEditor::Options &op) {if(op.ce)op.ce->setNBPath  (op.netbeans_path());}
static void   ASChanged    (             CodeEditor::Options &op) {if(op.ce)op.ce->setASPath  (op.  android_sdk());}
static void   ANChanged    (             CodeEditor::Options &op) {if(op.ce)op.ce->setANPath  (op.  android_ndk());}
static void  JDKChanged    (             CodeEditor::Options &op) {if(op.ce)op.ce->setJDKPath (op.     jdk_path());}
static void CertChangedF   (             CodeEditor::Options &op) {if(op.ce)op.ce->setCertPath(op.    cert_file());}
static void CertChangedP   (             CodeEditor::Options &op) {if(op.ce)op.ce->setCertPass(op.    cert_pass());}
static void   VSSelect     (             CodeEditor::Options &op) {op.w_vs_path      .activate();}
static void   NBSelect     (             CodeEditor::Options &op) {op.w_netbeans_path.activate();}
static void   ASSelect     (             CodeEditor::Options &op) {op.w_android_sdk  .activate();}
static void   ANSelect     (             CodeEditor::Options &op) {op.w_android_ndk  .activate();}
static void  JDKSelect     (             CodeEditor::Options &op) {op.w_jdk_path     .activate();}
static void CertSelect     (             CodeEditor::Options &op) {op.w_cert_file    .activate();}
static void   VSLoad       (C Str &path, CodeEditor::Options &op) {if(op.ce)op.ce->setVSPath  (path);}
static void   NBLoad       (C Str &path, CodeEditor::Options &op) {if(op.ce)op.ce->setNBPath  (path);}
static void   ASLoad       (C Str &path, CodeEditor::Options &op) {if(op.ce)op.ce->setASPath  (path);}
static void   ANLoad       (C Str &path, CodeEditor::Options &op) {if(op.ce)op.ce->setANPath  (path);}
static void  JDKLoad       (C Str &path, CodeEditor::Options &op) {if(op.ce)op.ce->setJDKPath (path);}
static void CertLoad       (C Str &path, CodeEditor::Options &op) {if(op.ce)op.ce->setCertPath(path);}
static void  VSAutodetect  (             CodeEditor::Options &op) {op.vs_versions.activate();}
static void  ASAutodetect  (             CodeEditor::Options &op) {Str p=GetRegStr(RKG_LOCAL_MACHINE, "Software/Android SDK Tools/Path"); if(p.is())ASLoad(p, op);}
static void JDKAutodetect  (             CodeEditor::Options &op) {Str p=GetRegStr(RKG_LOCAL_MACHINE, "Software/JavaSoft/Java Development Kit/1.8/JavaHome"); if(p.is())JDKLoad(p, op);}
static void  ANAutodetect  (             CodeEditor::Options &op)
{
   Memc<Str> paths; // list of Android SDK paths
   Str p=GetPath(op.android_sdk()); if(p.is())paths.binaryInclude(p, ComparePathCI);
       p=GetPath(GetRegStr(RKG_LOCAL_MACHINE, "Software/Android SDK Tools/Path")); if(p.is())paths.binaryInclude(p, ComparePathCI);
   FREPA(paths)for(FileFind ff(paths[i]); ff(); )if(FExistSystem(ff.pathName()+("/" PLATFORM("ndk-build.cmd", "ndk-build")))){ANLoad(ff.pathName(), op); return;} // check if in same paths there is NDK found
}
static void CertCreate            (CodeEditor::Options &op) {if(op.ce)op.ce->android_certificate.activate();}
static void FontSizeChanged       (CodeEditor::Options &op) {if(op.ce)op.ce->  fontChanged();}
static void ThemeChanged          (CodeEditor::Options &op) {if(op.ce)op.ce-> themeChanged();}
static void EditCustomTheme       (CodeEditor::Options &op) {op.color_theme_editor.visibleToggleActivate();}
static void EditCustomFont        (CodeEditor::Options &op) {op.       font_editor.visibleToggleActivate();}
static void ScrollChanged         (CodeEditor::Options &op) {if(op.ce)op.ce->scrollChanged();}
static void LineNumbersChanged    (CodeEditor::Options &op) {if(op.ce && op.ce->cur())op.ce->cur()->setRegionSize();}
static void HideHorizontalSlidebar(CodeEditor::Options &op) {if(op.ce)op.ce->hideSlideBarChanged();}
static void AutoHideMenuBar       (CodeEditor::Options &op) {if(op.ce)op.ce->setMenuBarVisibility();}

static void FacebookAndroidKeyHash(CodeEditor::Options &op)
{
#if WINDOWS
   if(!CE.jdk_path.is() || !FExistSystem(CE.jdk_path.tailSlash(true)+"bin/keytool.exe")){CE.options.activatePaths(); Gui.msgBox(S, "Path to Java Development Kit was not specified or is invalid."); return;}
#endif
   if(!CE.cert_file.is() || !FExistSystem(CE.cert_file)){CE.options.activateCert(); Gui.msgBox(S, "Path to Android Certificate File was not specified or is invalid."); return;}
   if(!CE.cert_pass.is()                               ){CE.options.activateCert(); Gui.msgBox(S, "Android Certificate Password was not specified."); return;}
   ConsoleProcess cp;
   if(!cp.create(PLATFORM(CE.jdk_path.tailSlash(true)+"bin/keytool.exe", "keytool"), S+"-exportcert -keystore \""+CE.cert_file+"\" -storepass \""+CE.cert_pass+"\" -keypass \""+CE.cert_pass+"\" -alias \"key\"", true, true))
      {Gui.msgBox(S, "Failed to start the Java Development Kit Key Tool."); return;}
   if(!cp.wait(-1)){Gui.msgBox(S, "Java Development Kit Key Tool timed out."); return;}
   Str out=cp.get(); Memt<Byte> data; data.setNum(out.length()); REPAO(data)=out[i];
   SHA1::Hash hash=SHA1Mem(data.data(), data.elms());
   out=Base64(&hash, SIZE(hash));
   ClipSet(out);
   Gui.msgBox("Success", S+"Key Hash\n"+out+"\nhas been copied to clipboard.");
}

void CodeEditor::Options::activatePaths() {tabs.set(1); activate();}
void CodeEditor::Options::activateCert () {tabs.set(2); activate();}
static CChar8 *color_theme_t[]=
{
   "Light",
   "Blue",
   "Dark",
   "Custom",
};
static CChar8 *path_mode_t[]=
{
   "Absolute",
   "Relative",
};
void CodeEditor::Options::create(CodeEditor &ce)
{
   font_sizes.New().set(12, "12"); // 0
   font_sizes.New().set(13, "13"); // 1
   font_sizes.New().set(14, "14"); // 2
   font_sizes.New().set(15, "15"); // 3
   font_sizes.New().set(16, "16"); // 4
   font_sizes.New().set(17, "17"); // 5
   font_sizes.New().set(18, "18"); // 6
   font_sizes.New().set( 0, "Custom"); // 7
   ListColumn lc[]=
   {
      ListColumn(MEMBER(FontSize, text), LCW_MAX_DATA_PARENT, "size"),
   };

   T.ce=&ce;
   Gui+=super::create(Rect_C(0, 0, 1.2f, 0.885f), "Editor Options").hide(); button[2].show();
   CChar8 *tabs_t[]={"Code Editor", "Paths", "Certificates", "Importing"};
   Flt Y=-0.05f;
   T+=tabs.create(Rect_C(clientWidth()/2, Y, 0.8f, 0.05f), 0, tabs_t, Elms(tabs_t), true).valid(true).set(0); Y-=0.10f;
   ts.reset(true).align.set(1, 1);

   // code editor
   {
      Tab &tab=tabs.tab(0);
      Flt y=Y, s=0.069f, w=0.64f, h=0.055f;
      tab+=t_font_size               .create(Vec2(0.32f, y), "Font:");
      tab+=  font_size               .create(Rect_R((clientWidth()+w)/2, y, 0.22f, h)).setColumns(lc, Elms(lc)).setData(font_sizes).func(FontSizeChanged, T, true).set(4);
      tab+=  font_edit               .create(Rect_L(font_size.rect().right()+Vec2(0.02f,0), 0.10f, h), "Edit").func(EditCustomFont, T); y-=s;
      tab+=t_color_theme             .create(Vec2(0.40f, y), "Color Theme:");
      tab+=  color_theme             .create(Rect_R((clientWidth()+w)/2, y, 0.22f, h), color_theme_t, Elms(color_theme_t)).func(ThemeChanged, T).set(2);
      tab+=  color_theme_edit        .create(Rect_L(color_theme.rect().right()+Vec2(0.02f,0), 0.10f, h), "Edit").func(EditCustomTheme, T); y-=s;
      tab+=  ac_on_enter             .create(Rect_C(clientWidth()/2, y, w, h), "Autocomplete on Enter only"); ac_on_enter.mode=BUTTON_TOGGLE; y-=s;
      tab+=  simple                  .create(Rect_C(clientWidth()/2, y, w, h), "Simple Edit Mode"          ).desc("Editing text will not perform any additional helper operations."); simple.mode=BUTTON_TOGGLE; y-=s;
      tab+=  imm_scroll              .create(Rect_C(clientWidth()/2, y, w, h), "Immediate Scroll"          ).func(ScrollChanged, T).desc("Enable immediate mouse-wheel scrolling instead of default smooth."); imm_scroll.mode=BUTTON_TOGGLE; y-=s;
      tab+=  eol_clip                .create(Rect_C(clientWidth()/2, y, w, h), "End of Line Cursor Clip"   ).desc("Clip cursor position to end of line when mouse clicking."); eol_clip.mode=BUTTON_TOGGLE; eol_clip.set(true); y-=s;
      tab+=  line_numbers            .create(Rect_C(clientWidth()/2, y, w, h), "Show Line Numbers"         ).func(LineNumbersChanged, T); line_numbers.mode=BUTTON_TOGGLE; y-=s;
      tab+=  hide_horizontal_slidebar.create(Rect_C(clientWidth()/2, y, w, h), "Hide Horizontal SlideBar"  ).func(HideHorizontalSlidebar, T); hide_horizontal_slidebar.mode=BUTTON_TOGGLE; y-=s;
      tab+=  auto_hide_menu          .create(Rect_C(clientWidth()/2, y, w, h), "Auto-Hide MenuBar"         ).func(AutoHideMenuBar, T); auto_hide_menu.mode=BUTTON_TOGGLE; y-=s;
      tab+=t_export_path_mode        .create(Vec2(0.475f, y), "Project Export Paths:");
      tab+=  export_path_mode        .create(Rect_R((clientWidth()+w)/2, y, 0.24f, h), path_mode_t, Elms(path_mode_t)).set(0); y-=s;
   }

   // paths
   {
      Tab &tab=tabs.tab(1);
      Flt y=Y-0.05f, w=1.0f, h=0.05f, s=0.13f;
   #if WINDOWS
      tab+=  vs_path     .create(Rect_C (clientWidth()/2, y, w, h), ce.vs_path).func(VSChanged, T); y-=s;
      tab+=t_vs_path     .create(vs_path.rect().lu(), "Visual Studio Path", &ts);
      tab+=b_vs_path     .create(Rect_LU(vs_path.rect().ru(), h, h), "...").func(VSSelect, T);
      tab+=d_vs          .create(Rect_RD(vs_path.rect().ru(), 0.22f, h), "Download").func(VSDownload  , T);
      tab+=  vs_path_auto.create(Rect_RU(d_vs.rect().lu(), 0.26f, h), "Auto-Detect").func(VSAutodetect, T);
   #elif LINUX
      tab+=  netbeans_path     .create(Rect_C (clientWidth()/2, y, w, h), ce.netbeans_path).func(NBChanged, T); y-=s;
      tab+=t_netbeans_path     .create(netbeans_path.rect().lu(), "NetBeans Path", &ts);
      tab+=b_netbeans_path     .create(Rect_LU(netbeans_path.rect().ru(), h, h), "...").func(NBSelect, T);
      tab+=d_netbeans          .create(Rect_RD(netbeans_path.rect().ru(), 0.22f, h), "Download").func(NBDownload  , T);
    //tab+=  netbeans_path_auto.create(Rect_RU(d_netbeans.rect().lu(), 0.26f, h), "Auto-Detect").func(NBAutodetect, T);
   #endif

      tab+=  android_sdk.create(Rect_C(clientWidth()/2, y, w, h), ce.android_sdk).func(ASChanged, T); y-=s;
      tab+=t_android_sdk.create(android_sdk.rect().lu(), "Android SDK Path", &ts);
      tab+=b_android_sdk.create(Rect_LU(android_sdk.rect().ru(), h, h), "...").func(ASSelect, T);
      tab+=d_android_sdk.create(Rect_RD(android_sdk.rect().ru(), 0.22f, h), "Download").func(ASDownload, T);
      tab+=  android_sdk_auto.create(Rect_RU(d_android_sdk.rect().lu(), 0.26f, h), "Auto-Detect").func(ASAutodetect, T);

      tab+=  android_ndk.create(Rect_C(clientWidth()/2, y, w, h), ce.android_ndk).func(ANChanged, T); y-=s;
      tab+=t_android_ndk.create(android_ndk.rect().lu(), "Android NDK Path", &ts);
      tab+=b_android_ndk.create(Rect_LU(android_ndk.rect().ru(), h, h), "...").func(ANSelect, T);
      tab+=d_android_ndk.create(Rect_RD(android_ndk.rect().ru(), 0.22f, h), "Download").func(ANDownload, T);
      tab+=  android_ndk_auto.create(Rect_RU(d_android_ndk.rect().lu(), 0.26f, h), "Auto-Detect").func(ANAutodetect, T);

   #if WINDOWS
      tab+=  jdk_path     .create(Rect_C (clientWidth()/2, y, w, h), ce.jdk_path).func(JDKChanged, T); y-=s;
      tab+=t_jdk_path     .create(jdk_path.rect().lu(), "Java Development Kit (JDK) Path", &ts);
      tab+=d_jdk          .create(Rect_RD(jdk_path.rect().ru(), 0.22f, h), "Download").func(JDKDownload, T);
      tab+=b_jdk_path     .create(Rect_LU(jdk_path.rect().ru(), h, h), "...").func(JDKSelect, T);
      tab+=  jdk_path_auto.create(Rect_RU(d_jdk.rect().lu(), 0.26f, h), "Auto-Detect").func(JDKAutodetect, T);
   #elif MAC // not needed on Linux
      tab+=t_jdk_path     .create(Rect_C(clientWidth()/2, y, w, h).lu(), "Java Development Kit (JDK)", &ts);
      tab+=d_jdk          .create(Rect_RD(Rect_C(clientWidth()/2, y, w, h).ru(), 0.22f, h), "Download").func(JDKDownload, T);
   #endif
   }

   // certificates
   {
      Tab &tab=tabs.tab(2);
      Flt y=Y-0.05f, w=1.0f, h=0.05f, s=0.06f;
      tab+=  cert_file.create(Rect_C (clientWidth()*0.5f, y, w, h)).func(CertChangedF, T); y-=s;
      tab+=t_cert_file.create(cert_file.rect().lu(), "Android Certificate File", &ts); y-=s;
      tab+=b_cert_file.create(Rect_LU(cert_file.rect().ru(), h, h), "...").func(CertSelect, T);
      tab+=cert_create.create(Rect_RD(cert_file.rect().ru(), 0.22f, h), "Create").func(CertCreate, T);
      tab+=  cert_pass.create(Rect_C (clientWidth()*0.5f, y, w, h)).func(CertChangedP, T); y-=s;
      tab+=t_cert_pass.create(cert_pass.rect().lu(), "Android Certificate Password", &ts); y-=s;
      tab+=facebook_android_key_hash.create(Rect_C(clientWidth()*0.5f, y, 0.80f, 0.06f), "Get Android Key Hash for Facebook").func(FacebookAndroidKeyHash, T); y-=s;
      y-=s;
      tab+=authenticode.create(Rect_C(clientWidth()/2, y, 0.60f, 0.06f), "Use Microsoft Authenticode").desc("If automatically sign the application when publishing for Windows EXE platform.\nWindows signtool.exe must be installed together with your Microsoft Windows Authenticode Digital Signature in the Certificate Store.\nSign tool will be used with the /a option making it to choose the best certificate out of all available."); authenticode.mode=BUTTON_TOGGLE; authenticode.set(false); y-=s;
   }

   // importing
   {
      Tab &tab=tabs.tab(3);
      Flt y=Y, s=0.069f, w=0.64f, h=0.055f;
      tab+=t_import_path_mode.create(Vec2(0.475f, y), "Asset Paths:");
      tab+=  import_path_mode.create(Rect_R((clientWidth()+w)/2, y, 0.24f, h), path_mode_t, Elms(path_mode_t)).set(0); y-=s;
      tab+=  import_image_mip_maps.create(Rect_C(clientWidth()/2, y, w, h), "Create Image Mip Maps").desc("This option defines if newly imported images should have mip maps created."); import_image_mip_maps.mode=BUTTON_TOGGLE; import_image_mip_maps.set(true); y-=s;
   }

   w_vs_path         .create().modeDirSelect().io(  VSLoad,   VSLoad, T);
   w_netbeans_path   .create().modeDirSelect().io(  NBLoad,   NBLoad, T);
   w_android_sdk     .create().modeDirSelect().io(  ASLoad,   ASLoad, T);
   w_android_ndk     .create().modeDirSelect().io(  ANLoad,   ANLoad, T);
   w_jdk_path        .create().modeDirSelect().io( JDKLoad,  JDKLoad, T);
   w_cert_file       .create()                .io(CertLoad, CertLoad, T);
          vs_versions.create(ce);
   color_theme_editor.create(  );
          font_editor.create(  );
}
/******************************************************************************/
static void CreateCert(CodeEditor::AndroidCertificate &ac)
{
#if WINDOWS
   if(!CE.jdk_path.is() || !FExistSystem(CE.jdk_path.tailSlash(true)+"bin/keytool.exe")){CE.options.activatePaths(); Gui.msgBox(S, "Path to Java Development Kit was not specified or is invalid."); return;}
#endif
   if(Contains(ac.org_name(), '"')){Gui.msgBox(S, "Developer Name cannot contain quotation marks"); return;}
   if(Contains(ac.pass    (), '"')){Gui.msgBox(S, "Password cannot contain quotation marks"      ); return;}
   if(ac.pass().length()<6        ){Gui.msgBox(S, "Password must be at least 6 characters long"  ); return;}
   ac.win_io.save();
}
static void CreateCert(C Str &name, CodeEditor::AndroidCertificate &ac)
{
   FDelFile(name);
   ConsoleProcess cp;
   if(!cp.create(PLATFORM(CE.jdk_path.tailSlash(true)+"bin/keytool.exe", "keytool"), S+"-genkey -keystore \""+name+"\" -storepass \""+ac.pass()+"\" -keypass \""+ac.pass()+"\" -keyalg RSA -keysize 2048 -validity 10000 -alias \"key\" -dname \"o="+ac.org_name()+"\""))
      {Gui.msgBox(S, "Failed to start the Java Development Kit Certificate Generator."); return;}
   if(cp.wait(-1))
   {
      if(!FExistSystem(name)){Gui.msgBox(S, S+"Failed to create certificate:\n"+cp.get()); return;}
      Gui.msgBox("Success", "Certificate created successfully");
      CE.setCertPath(   name  );
      CE.setCertPass(ac.pass());
   }
   ac.hide();
}
Window& CodeEditor::AndroidCertificate::hide()
{
          win_io.hide();
   return super::hide();
}
void CodeEditor::AndroidCertificate::create(CodeEditor &ce)
{
   Gui+=super::create(Rect_C(0, 0, 0.7f, 0.42f), "Android Certificate Creator").hide(); button[2].show();
   Flt y=-0.05f, h=0.05f, s=0.06f;
   T+=torg_name.create(Rect_C(clientWidth()*0.5f, y, 0, 0), "Developer Name"); y-=s*0.8f;
   T+= org_name.create(Rect_C(clientWidth()*0.5f, y, clientWidth()*0.9f, h )); y-=s;
   T+=tpass    .create(Rect_C(clientWidth()*0.5f, y, 0, 0), "Password"      ); y-=s*0.8f;
   T+= pass    .create(Rect_C(clientWidth()*0.5f, y, clientWidth()*0.9f, h )); y-=s*1.4f;
   T+= save    .create(Rect_C(clientWidth()*0.5f, y,               0.3f, h ), "Create").func(CreateCert, T); y-=s;
   win_io.create(S, S, SystemPath(SP_DESKTOP), CreateCert, CreateCert, T);
}
/******************************************************************************/
void CodeEditor::hideAll()
{
   options            .hide();
   android_certificate.hide();
   find               .hide();
   replace            .hide();
   build_progress     .hide();
   build_io           .hide();
   devlog_io          .hide();
   visibleOutput       (false);
   visibleAndroidDevLog(false);
}
Bool CodeEditor::visibleOpenedFiles() {return false;}
void CodeEditor::visibleOpenedFiles(Bool on)
{
}
Bool CodeEditor::visibleOutput(       ) {return build_region.visible();}
void CodeEditor::visibleOutput(Bool on)
{
   if(build_region.visible()!=on)
   {
      build_region.visible(on);
      build_close .visible(on);
      build_export.visible(on);
      build_copy  .visible(on);
      resize();
   }
   cei().visibleChangedOutput();
}
Bool CodeEditor::visibleAndroidDevLog(       ) {return devlog_region.visible();}
void CodeEditor::visibleAndroidDevLog(Bool on)
{
   if(devlog_region.visible()!=on)
   {
      devlog_region.visible(on);
      devlog_close .visible(on);
      devlog_export.visible(on);
      devlog_filter.visible(on);
      devlog_clear .visible(on);

      if(on && !devlog_process.active())
      {
         adb_path=adbPath();
         if(!adb_path.is()){options.activatePaths(); Gui.msgBox(S, "The path to Android SDK has not been specified or is invalid.");}else
         {
            if(adb_server.create(adb_path, "start-server")) // if we're going to launch the app then make sure that the ADB server is running, or else custom launched ADB processess will never exit
            {
               if(adb_server.wait(1000/60)) // wait up to 1 frame of 60 fps for it to show up
               {
                  devlog_process.create(adb_path, "logcat -v time");
                  adb_server.del();
               }else
               {  // let it run and start devlog once it finished
                  devlog_data.clear();
                  devlog_data.New().message="Starting Android Debug Bridge..";
                  devlog_list.setData(devlog_data);
               }
            }
         }
      }
      resize();
   }
   cei().visibleChangedAndroidDevLog();
}
/******************************************************************************/
static void RegionRect(Region &region, C Rect &rect)
{
   if(region.rect()!=rect)
   {
      Bool at_end=region.slidebar[1].wantedAtEnd(); region.rect(rect); if(at_end)region.slidebar[1].scrollEnd(true);
   }
}
void CodeEditor::resize()
{
   ts      .setPerPixelSize();
   ts_small.setPerPixelSize();

   menu.parentClientRectChanged(null, &NoTemp(D.rect()));
   Flt menu_h=menu.rect().h();

   RegionRect(build_region, Rect(-D.w(), -D.h(), D.w(), -D.h()*0.5f)+Vec2(0, menu_on_top ? 0 : menu_h));
   build_list.elmHeight(ts.size.y).textSize(ts.size.y);//.columnHeight(ts.size.y*1.3f);
   build_close .rect(Rect_RU(build_region.rect().ru()-Vec2(build_region.slidebarSize(), 0), build_list.columnHeight(), build_list.columnHeight()));
   build_export.rect(Rect_RU(build_close .rect().lu(), 0.18f, build_list.columnHeight())).desc("Export to file");
   build_copy  .rect(Rect_RU(build_export.rect().lu(), 0.16f, build_list.columnHeight())).desc("Copy to System Clipboard");

   RegionRect(devlog_region, Rect_LD(build_region.visible() ? build_region.rect().lu() : Vec2(-D.w(), -D.h()+(menu_on_top ? 0 : menu_h)), build_region.rect().w(), D.h()*(build_region.visible() ? 1.2f : 1.5f)));
   devlog_list.elmHeight(ts.size.y).textSize(ts.size.y);
   devlog_list.columnWidth(0, ts.size.y*3.8f);
   devlog_list.columnWidth(1, ts.size.y*7.8f);
   devlog_close .rect(Rect_RU(devlog_region.rect().ru()-Vec2(devlog_region.slidebarSize(), 0), devlog_list.columnHeight(), devlog_list.columnHeight()));
   devlog_clear .rect(Rect_RU(devlog_close .rect().lu(), 0.17f, devlog_list.columnHeight()));
   devlog_export.rect(Rect_RU(devlog_clear .rect().lu(), 0.20f, devlog_list.columnHeight()));
   devlog_filter.rect(Rect_RU(devlog_export.rect().lu(), 0.23f, devlog_list.columnHeight()));

   find.resize();

   if(cur())cur()->resize();
}
void CodeEditor::skinChanged()
{
   FontPtr cjk_font=UID(3047797125, 1232117798, 1870176427, 3857765718) /* Extra\Data\Verdana (Small CJK) */; // get reference before resetting 'ts_cjk' in case that would unload it
   ts      .resetColors(true);
   ts_small.resetColors(true);
   ts_cjk  .reset      (true).font(cjk_font);

   if(Gui.skin)suggestions_skin=find_result_skin=source_skin=cjk_skin=*Gui.skin;
   source_skin.region.normal.clear();
   source_skin.region.normal_color.zero();
   find_result_skin.list.text_style=&ts_small;
   suggestions_skin.list.text_style=&ts;
           cjk_skin.list.text_style=&ts_cjk;

   REPAO( build_data).setColor();
   REPAO(devlog_data).setColor();
   options.skinChanged();
   ColorPicker::SetTextStyle();
}
/******************************************************************************/
void CodeEditor::zoom(Int zoom)
{
   if(InRange(options.font_size(), options.font_sizes)
   &&                              options.font_sizes[options.font_size()].size>0)
   {
      Int new_index=options.font_size()+zoom;
      if(InRange(new_index, options.font_sizes)
      &&                    options.font_sizes[new_index].size>0)
         options.font_size.set(new_index);
   }
}
/******************************************************************************/
void CodeEditor::fontChanged()
{
   if(InRange(options.font_size(), options.font_sizes))
   {
      Int size=options.font_sizes[options.font_size()].size;

      options.font_edit.visible(size<=0);

      ts.font(null);
      Vec2 spacing(0.59f, 1.05f);
      if(size<=0) // try to load custom font
      {
         FontPtr font;
         if(font.get("Bin/Code Editor.font"))
            if(font->is())
         {
            ts.font(font); size=font->height(); spacing=options.font_editor.params.spacing;
         }
      }

      if(size<=0)size=18;
      if(!ts.font())ts.font(S+"Font/Lucida Console "+Mid(size, 11, 18)+".font");

      ts.space.set(           spacing.x, // ts.space.x=AlignRound(0.58f, 1.0f/ts.font->height());
                   AlignRound(spacing.y, 1.0f/ts.font()->height()));

      ts_small.font(S+"Font/Lucida Console "+Mid(size-2, 11, 18)+".font");
      ts_small.space.set(AlignRound(0.55f, 1.0f/ts_small.font()->height()),
                         AlignRound(0.90f, 1.0f/ts_small.font()->height()));

      resize();
   }
}
void CodeEditor::        themeChanged() {Theme=(options.color_theme()==0 ? ThemeLight : options.color_theme()==1 ? ThemeBlue : options.color_theme()==2 ? ThemeDark : ThemeCustom); options.color_theme_edit.visible(options.color_theme()==3); REPAO(sources).themeChanged   ();}
void CodeEditor::       scrollChanged() {                                                                                                                                                                                                       REPAO(sources).setScroll      ();}
void CodeEditor:: hideSlideBarChanged() {                                                                                                                                                                                                       REPAO(sources).setHideSlideBar();}
void CodeEditor::setMenuBarVisibility()
{
   Bool visible=(options.auto_hide_menu() ? menu.contains(Gui.ms()) || view_mode.contains(Gui.ms()) || view_what.contains(Gui.ms()) || b_close.contains(Gui.ms()) || Ms.pos().y<=menu.rect().min.y+EPS : true);
   if(menu.visible()!=visible)
   {
      menu     .visible(visible);
      view_mode.visible(visible);
      view_what.visible(visible);
      b_close  .visible(visible);
      resize();
   }
}
/******************************************************************************/
static Bool JumpTo(C Str &file, Int line)
{
   Source *old=CE.cur();
   if(LineMap *lm=CE.build_line_maps.find(file))
   {
      SourceLoc src; Int original_index; UID original_id; lm->get(line, src, original_index, original_id);
      if(CE.load(src))
      {
         Int l=CE.cur()->findLine(original_id); if(l>=0)original_index=l;
         CE.cur()->highlight(original_index, old!=CE.cur());
         return true;
      }
   }
   if(CE.load(file, true, true)) // if failed to load original source, then display cpp/h file version instead
   {
      CE.cur()->highlight(line, old!=CE.cur());
      return true;
   }
   return false;
}
Bool CodeEditor::BuildResult::jumpTo()
{
   if(source_loc.is()) // if source is specified
   {
      Source *old=CE.cur();
      if(CE.load(source_loc))
      {
         CE.cur()->highlight(CE.cur()->findLine(line), old!=CE.cur());
         return true;
      }
   }else // try to parse the text and detect file paths and lines
   {
      Str text=T.text;
      // Visual Studio       - 1>file.cpp(line) : error C2065: '' : undeclared identifier
      // MSBuild             - file.cpp(23): error C2039: '' : is not a member of..
      // Apple/Linux/Android - "C:/path/file.cpp:line:message" or "jni/../path/file.cpp:line:message"

      // remove Visual Studio process number
      FREPA(text)if(!(CharFlag(text[i])&CHARF_DIG))
      {
         if(text[i]=='>')text.remove(0, i+1);
         break;
      }
      text=SkipWhiteChars(text);
      text=SkipStart     (text, "could be '");
      text=SkipStart     (text, "or       '");
      text=SkipWhiteChars(text);

      // try Visual Studio and MSBuild
      Int posa=TextPosI(text, ") : "), // "file.cpp(line) : error/warning"
          posb=TextPosI(text, "): " ); // "file.cpp(line): error/warning" (this case can be encountered in VS 2010)
      Int pos =posa; if(posb>=0 && (pos<0 || posb<pos))pos=posb; // take whichever is first and valid
      if( pos>=0)
      {
         text.clip(pos);
         REPA(text)
         {
            if(text[i]=='(')
            {
               Int line=TextInt(text()+i+1)-1;
               text.clip(i);
               if( Starts  (text, ".\\"))text.remove(0, 2); // remove ".\\" in ".\\Source\\file.cpp"
               if(!FullPath(text       ))text=CE.build_path+text; // "Source\\file.cpp"
               if(JumpTo(text, line))return true;
               break;
            }else
            if(!(CharFlag(text[i])&CHARF_DIG) && text[i]!=',')break;
         }
      }else
      {
         Int pos =TextPosI(text, ':'); if(pos==1)pos=TextPosIN(text, ':', 1); // if it's ':' from "C:" then get the next ':'
         if( pos>=0)
         {
            Int line =TextInt(text()+pos+1)-1;
            if( line>=0)
            {
               text.clip(pos);
               if(FullPath(text) || StartsPath(text, "Source") || StartsPath(text, "jni") || StartsPath(text, ".."))
               {
                  if(!FullPath(text))
                  {
                     if(CE.build_exe_type==EXE_APK)text=NormalizePath(CE.build_path+"Android/"+text);
                     else                          text=NormalizePath(CE.build_path+           text);
                  }
                  if(JumpTo(text, line))return true;
               }
            }
         }
      }
   }
   return false;
}
void CodeEditor::BuildList::update(C GuiPC &gpc)
{
   super::update(gpc);
   if(Gui.ms()==this && Ms.bp(0))if(BuildResult *br=T())br->jumpTo();
}
void CodeEditor::BuildList::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible && InRange(highlight_line, T))
   {
      Flt alpha=1-(Time.appTime()-highlight_time);
      if( alpha>0)
      {
         D.clip(gpc.clip);
         visToScreenRect(highlight_line).draw(ColorAlpha(CYAN, alpha*0.5f));
      }
   }
   super::draw(gpc);
}
void                     CodeEditor::buildClear(                   ) {       build_data.clear();}
CodeEditor::BuildResult& CodeEditor::buildNew  (                   ) {return build_data.New  ();}
void                     CodeEditor::buildNew  (Memc<Message> &msgs)
{
   Source *last_source=null;
   FREPA(msgs)
   {
      Message &msg=msgs[i];
      Source  *cur_source=msg.source;
      if(last_source!=cur_source){last_source=cur_source; buildNew().set(S+"\""+(cur_source ? cur_source->loc.asText() : S)+"\":", cur_source);}
      if(msg.type==Message::ERROR  ){if(msg.token)buildNew().set(S+"  Error: "  +msg.text, msg.token);else buildNew().set(S+"  Error: "  +msg.text, msg.source);}else
      if(msg.type==Message::WARNING){if(msg.token)buildNew().set(S+"  Warning: "+msg.text, msg.token);else buildNew().set(S+"  Warning: "+msg.text, msg.source);}else
                                    {if(msg.token)buildNew().set(                msg.text, msg.token);else buildNew().set(                msg.text, msg.source);}
      FREPA(msg.children)
      {
         Message &c=msg.children[i];
         if(msg.token)buildNew().set(S+"     "+c.text, c.token);else buildNew().set(S+"     "+c.text, c.source);
      }
   }
}
void CodeEditor::buildUpdate(Bool show)
{
   build_list.setData(build_data);
   if(show)visibleOutput(true);
}
Str CodeEditor::DeviceLog::asText() {return S+"Time("+time+"), App("+app+"): "+message;}
/******************************************************************************/
}}
/******************************************************************************/
