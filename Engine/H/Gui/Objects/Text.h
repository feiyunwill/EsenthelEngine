/******************************************************************************/
const_mem_addr STRUCT(Text , GuiObj) // Gui Text !! must be stored in constant memory address !!
//{
   AUTO_LINE_MODE auto_line; // automatically calculate new lines when drawing text, default=AUTO_LINE_NONE
   TextStylePtr  text_style; // text style   , default=null (if set to null then current value of 'skin.text.text_style' is used)
   GuiSkinPtr          skin; // skin override, default=null (if set to null then current value of 'Gui.skin'             is used)

   // manage
   Text& del   (                                                    );                                         // delete
   Text& create(              C Str &text=S, C TextStylePtr &ts=null);                                         // create, 'ts'=text style (the object is not copied, only the pointer to the object is remembered, therefore it must point to a constant memory address !!)
   Text& create(C Vec2 &rect, C Str &text=S, C TextStylePtr &ts=null) {create(text, ts).rect(rect); return T;} // create, 'ts'=text style (the object is not copied, only the pointer to the object is remembered, therefore it must point to a constant memory address !!)
   Text& create(C Rect &rect, C Str &text=S, C TextStylePtr &ts=null) {create(text, ts).rect(rect); return T;} // create, 'ts'=text style (the object is not copied, only the pointer to the object is remembered, therefore it must point to a constant memory address !!)
   Text& create(C Text &src                                         );                                         // create from 'src'

   // get / set
   Text& clear(           );                                      // clear   text value
   Text& set  (C Str &text); C Str& operator()()C {return _text;} // set/get text value
   Text& code (C Str &code);   Str  code      ()C;                // set/get text value in code format
      // 1. code format accepts following keywords:   in following formats:
      //    col, color                                RGB, RGBA, RRGGBB, RRGGBBAA (hexadecimal format)
      //    shadow                                    X, XX                       (hexadecimal format)
      // 2. codes should be surrounded by '[' ']' signs
      // 3. removing the effect of a code should be handled by '/' sign followed by code name
      // 4. sample codes:
      //       "Text without code. [color=F00]Text with code[/color]"        - will force red color  on "Text with code"
      //       "[shadow=0]No Shadow[/shadow] [shadow=F]Full Shadow[/shadow]" - will force no  shadow on "No Shadow"      and full shadow on "Full Shadow"

   GuiSkin  * getSkin     ()C {return skin ? skin() : Gui.skin();} // get actual skin
   TextStyle* getTextStyle()C;                                     // get actual text style

   Vec2 textSize()C; // get visible text size

   // main
   virtual void draw(C GuiPC &gpc); // draw object

#if EE_PRIVATE
   void zero();
#endif

  ~Text() {del();}
   Text();

protected:
   virtual Bool save(File &f, CChar *path=null)C;
   virtual Bool load(File &f, CChar *path=null) ;

   Str _text;
#if !EE_PRIVATE
   Ptr _code; Int _codes;
#else
   TextCodeData *_code; Int _codes;
#endif

   NO_COPY_CONSTRUCTOR(Text);
};
/******************************************************************************/
