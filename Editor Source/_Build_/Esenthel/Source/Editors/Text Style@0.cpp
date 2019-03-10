/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
TextStyleEditor TextStyleEdit;
/******************************************************************************/

/******************************************************************************/
      void TextStyleEditor::Change::create(ptr user)
{
         data=TextStyleEdit.edit;
         TextStyleEdit.undoVis();
      }
      void TextStyleEditor::Change::apply(ptr user)
{
         TextStyleEdit.edit.undo(data);
         TextStyleEdit.setChanged();
         TextStyleEdit.toGui();
         TextStyleEdit.undoVis();
      }
      void TextStyleEditor::TextClip::draw(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            D.clip(gpc.clip);
            Rect r=rect()+gpc.offset;
            TextStyleEdit.panel.draw(r);
            text_style=TextStyleEdit.game;
            GuiPC gpc2=gpc; gpc2.clip&=r; ::EE::Text::draw(gpc2);
         }
      }
   void TextStyleEditor::undoVis() {SetUndo(undos, undo, redo);}
   void TextStyleEditor::PreChanged(C Property &prop) {TextStyleEdit.undos.set(&prop);}
   void    TextStyleEditor::Changed(C Property &prop) {TextStyleEdit.setChanged();}
   void TextStyleEditor::ParamsShadow(EditTextStyle &e, C Str &t) {e.shadow   =TextInt (t); e.   shadow_time.getUTC();}
   void TextStyleEditor::ParamsShade(EditTextStyle &e, C Str &t) {e.shade    =TextInt (t); e.    shade_time.getUTC();}
   void TextStyleEditor::ParamsColor(EditTextStyle &e, C Str &t) {e.color    =TextVec4(t); e.    color_time.getUTC();}
   void TextStyleEditor::ParamsSelect(EditTextStyle &e, C Str &t) {e.selection=TextVec4(t); e.selection_time.getUTC();}
   void TextStyleEditor::ParamsAlign(EditTextStyle &e, C Str &t) {e.align    =TextVec2(t); e.    align_time.getUTC();}
   void TextStyleEditor::ParamsSize(EditTextStyle &e, C Str &t) {e.size     =TextVec2(t); e.     size_time.getUTC();}
   void TextStyleEditor::ParamsSpace(EditTextStyle &e, C Str &t) {e.space    =TextVec2(t); e.    space_time.getUTC();}
   void TextStyleEditor::ParamsFont(EditTextStyle &e, C Str &t)
   {
      e.font.zero(); int i=TextInt(t); if(InRange(i, Proj.font_node.children))
      {
       C Str &name=Proj.font_node.children[i].name;
         REPA(Proj.publish_fonts)if(Elm *elm=Proj.findElm(Proj.publish_fonts[i]))if(Equal(elm->name, name, true))
         {
            e.font=elm->id;
            break;
         }
      }
      e.font_time.getUTC();
   }
   Str TextStyleEditor::ParamsFont(C EditTextStyle &e)
   {
      if(Elm *elm=Proj.findElm(e.font))REPA(Proj.font_node.children)if(Equal(elm->name, Proj.font_node.children[i].name, true))return i;
      return S;
   }
   void TextStyleEditor::Undo(TextStyleEditor &editor) {editor.undos.undo();}
   void TextStyleEditor::Redo(TextStyleEditor &editor) {editor.undos.redo();}
   void TextStyleEditor::Locate(TextStyleEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void TextStyleEditor::create()
   {
                 add("Shadow"   , MemberDesc(MEMBER(EditTextStyle, shadow   )).setTextToDataFunc(ParamsShadow)).desc("Shadow Intensity");
                 add("Shade"    , MemberDesc(MEMBER(EditTextStyle, shade    )).setTextToDataFunc(ParamsShade )).desc("Shading");
                 add("Color"    , MemberDesc(MEMBER(EditTextStyle, color    )).setTextToDataFunc(ParamsColor )).setColor();
                 add("Selection", MemberDesc(MEMBER(EditTextStyle, selection)).setTextToDataFunc(ParamsSelect)).setColor();
                 add("Align"    , MemberDesc(MEMBER(EditTextStyle, align    )).setTextToDataFunc(ParamsAlign )).range(-1, 1);
                 add("Size"     , MemberDesc(MEMBER(EditTextStyle, size     )).setTextToDataFunc(ParamsSize  )).min(0).mouseEditSpeed(0.3f);
                 add("Space"    , MemberDesc(MEMBER(EditTextStyle, space    )).setTextToDataFunc(ParamsSpace )).min(0);
      font_prop=&add("Font"     , MemberDesc().setFunc(ParamsFont, ParamsFont)).setEnum(); font_prop->combobox.setData(Proj.font_node);
      autoData(&edit);

      ::PropWin::create("Text Style Editor", Vec2(0.02f, -0.07f), 0.036f, 0.043f, 0.3f); ::PropWin::changed(Changed, PreChanged); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=undo  .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      T+=text  .create("Sample Text\nNew Line");
      panel.border_color.a=0;
      rect(Rect_C(0, 0, Min(1.7f, D.w()*2), Min(1.07f, D.h()*2)));
   }
   void TextStyleEditor::toGame() {if(game)edit.copyTo(*game, Proj);}
   void TextStyleEditor::toGui()
   {
      ::PropWin::toGui(); toGame();
      if(font_prop && font_prop->combobox()<=0 && edit.font.valid()) // if there is a font specified but not in combobox selection (for example if combobox selection lists only certain fonts)
      {
         Elm *font=Proj.findElm(edit.font); font_prop->combobox.setText(font ? font->name : UnknownName, true, QUIET);
      }
   }
   TextStyleEditor& TextStyleEditor::hide(            )  {set(null); ::PropWin::hide(); return T;}
   Rect             TextStyleEditor::sizeLimit(            )C {Rect r=::EE::Window::sizeLimit(); r.min.set(1.0f, 0.49f); return r;}
   TextStyleEditor& TextStyleEditor::rect(C Rect &rect)  
{
      ::EE::Window::rect(rect);
      flt  x=0; if(props.elms())x=props[0].button.rect().max.x;
      Rect r(x, -clientHeight(), clientWidth(), 0); r.extend(-0.05f); //r.setC(r.center(), Min(r.w(), 0.8), Min(r.h(), 0.7));
      text.rect(r);
      return T;
   }
   void TextStyleEditor::flush()
   {
      if(elm && changed && game)
      {
         if(ElmTextStyle *data=elm->textStyleData()){data->newVer(); data->from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(*elm)); // edit
         Save(*game, Proj.gamePath(*elm)); Proj.savedGame(*elm); // game
         Server.setElmLong(elm->id);
      }
      changed=false;
   }
   void TextStyleEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmTextStyle *data=elm->textStyleData()){data->newVer(); data->from(edit);}
         toGame();
      }
   }
   void TextStyleEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_TEXT_STYLE)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm){edit.load(Proj.editPath(*elm)); game=Proj.gamePath(elm_id);}else{edit.reset(); game.clear();}
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void TextStyleEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void TextStyleEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void TextStyleEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         undos.set(null, true);
         EditTextStyle temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp))toGui();
      }
   }
   void TextStyleEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
   void TextStyleEditor::fontChanged()
   {
      if(font_prop)font_prop->combobox.setData(Proj.font_node);
   }
   void TextStyleEditor::drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj))REPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_FONT))
      {
         undos.set("font");
         edit.font=elm->id; edit.font_time.getUTC(); setChanged(); toGui();
         elms.remove(i, true); // remove at the end
         break;
      }
   }
TextStyleEditor::TextStyleEditor() : elm_id(UIDZero), elm(null), changed(false), font_prop(null), undos(true) {}

/******************************************************************************/
