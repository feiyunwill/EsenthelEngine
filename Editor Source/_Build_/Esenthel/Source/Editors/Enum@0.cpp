/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
EnumEditor EnumEdit;
/******************************************************************************/

/******************************************************************************/
   cchar8 *EnumEditor::Type_t[]=
   {
      "Default", // 0
      "Byte"   , // 1
      "UShort" , // 2
      "UInt"   , // 3
   };
/******************************************************************************/
      void EnumEditor::Enum::setColor() {color=GuiListTextColor(); if(removed)color.a/=2;}
      void EnumEditor::Enum::operator=(C EditEnum &src) {enum_id=src.id; name=src.name; removed=src.removed; setColor();}
      void EnumEditor::RenameWindow::create()
      {
         Gui+=::EE::Window::create(Rect_C(0, 0, 1, 0.14f), "Rename Enum").hide(); button[2].show();
         T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01f));
      }
      void EnumEditor::RenameWindow::activate(C UID &enum_id, C Str &name)
      {
         T.enum_id=enum_id;
         textline.set(name).selectAll().activate();
         ::EE::GuiObj::activate();
      }
      void EnumEditor::RenameWindow::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);

         if(Gui.window()==this)
         {
            if(Kb.k(KB_ENTER)){if(EnumEdit.rename(enum_id, textline()))hide();else Gui.msgBox(S, "Invalid enum name.\nEnum may contain only letters, digits and underline characters.\nIt may not contain spaces, symbols or start with a digit."); Kb.eatKey();}
         }
      }
      int EnumEditor::ListEnum::getPos(C Vec2 &pos) {int p=screenToVisY(pos.y-elmHeight()/2); return (p>=0) ? p : visibleElms();}
      void EnumEditor::ListEnum::draw(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            GuiSkin skin; 
            int elm=-1; if(Gui.dragging() && Gui.objAtPos(Gui.dragPos())==this)elm=getPos(Gui.dragPos());
            if( elm>=0)
            {
               if(Gui.skin)skin=*Gui.skin; skin.list.highlight_color.zero(); T.skin(&skin);
            }
            ::EE::_List::draw(gpc);
            if(elm>=0)
            {
               T.skin(null);
               D.clip(gpc.clip);
               flt y=visToScreenPos(elm, &gpc).y, b=0.005f;
               Rect(-D.w(), y-b, D.w(), y+b).draw(ColorAlpha(TURQ, 0.5f));
            }
         }
      }
      void EnumEditor::Change::create(ptr user)
{
         enums=EnumEdit.enums;
         EnumEdit.undoVis();
      }
      void EnumEditor::Change::apply(ptr user)
{
         EnumEdit.enums.undo(enums);
         EnumEdit.setChanged();
         EnumEdit.undoVis();
      }
   void EnumEditor::undoVis() {SetUndo(undos, undo, redo);}
   void EnumEditor::NewEnum(EnumEditor &ee) {ee.newEnum();}
          void EnumEditor::newEnum()
   {
      if(elm)FREP(1000)
      {
         Str name=S+"ENUM_"+i;
         REPA(enums)if(Equal(enums[i].name, name, true))goto exists;

         {
            undos.set("new", true);
            EditEnum &e=enums.New().setName(name); e.order_time.getUTC();
            setChanged();
            rename_window.activate(e.id, e.name);
            break;
         }
      exists:;
      }
   }
   bool EnumEditor::rename(C UID &enum_id, C Str &name)
   {
      if(ValidEnum(name))
      {
         REPA(enums)if(enums[i].id==enum_id)
         {
            undos.set("rename", true); enums[i].setName(name); setChanged(); break;
         }
         return true;
      }
      return false;
   }
   void EnumEditor::Undo(EnumEditor &editor) {editor.undos.undo();}
   void EnumEditor::Redo(EnumEditor &editor) {editor.undos.redo();}
   void EnumEditor::Locate(EnumEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void EnumEditor::ShowRemoved(EnumEditor &editor) {editor.toGui();}
   void EnumEditor::TypeChanged(EnumEditor &editor)
   {
      editor.undos.set("type");
      editor.enums.type=EditEnums::TYPE(editor.type());
      editor.enums.type_time.getUTC();
      editor.setChanged();
   }
   void EnumEditor::create()
   {
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Enum, name), LCW_MAX_DATA_PARENT, "name"),
      };
      Gui+=::EE::Window::create("Enum Editor").hide(); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=undo  .create(Rect_LU(0.01f, -0.01f     , 0.055f, 0.055f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.055f, 0.055f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.15f, 0.055f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      ts.reset().align.set(1, 0);
      T+=type_t.create(Rect_LU(locate.rect().ru()+Vec2(0.03f, 0), 0.09f, 0.055f), "Type:", &ts);
      T+=type  .create(Rect_LU(type_t.rect().ru()+Vec2(0.01f, 0), 0.19f, 0.055f), Type_t, Elms(Type_t)).func(TypeChanged, T);
      T+=show_removed.create().func(ShowRemoved, T).focusable(false).desc("Show removed elements"); show_removed.image="Gui/Misc/trash.img"; show_removed.mode=BUTTON_TOGGLE;
      T+=new_enum.create("New Enum").func(NewEnum, T);
      T+=region.create();
      region+=list.create(lc, Elms(lc), true).desc("Double click to rename element\nDrag and drop to change order"); list.flag|=LIST_MULTI_SEL; list.setElmOffset(MEMBER(Enum, offset)).setElmTextColor(MEMBER(Enum, color));
      rect(Rect_C(0, 0, 0.6f, 1));
      rename_window.create();
   }
   Rect   EnumEditor::sizeLimit(            )C {Rect r=::EE::Window::sizeLimit(); r.min.set(0.61f, 0.35f); return r;}
   EnumEditor& EnumEditor::rect(C Rect &rect)  
{
      ::EE::Window::rect(rect);
      Rect_RU r(clientWidth()-0.01f, -0.01f, 0.23f, 0.055f); if(r.min.x-0.01f-type.rect().h()<type.rect().max.x+0.01f)r-=Vec2(0, type.rect().h()+0.01f);
       new_enum   .rect(r);
      show_removed.rect(Rect_RU(new_enum.rect().lu()-Vec2(0.01f, 0), new_enum.rect().h()));
      region      .rect(Rect(0, -clientHeight(), clientWidth(), new_enum.rect().min.y).extend(-0.01f));
      return T;
   }
   void EnumEditor::skinChanged()
   {
      REPAO(data).setColor();
   }
   void EnumEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmEnum *data=elm->enumData()){data->newVer(); data->from(enums);} // modify just before saving/sending in case we've received data from server after edit
         Save(enums, Proj.editPath(elm->id));
        ::Enum e; enums.copyTo(e, elm->name); Save(e, Proj.gamePath(elm->id)); Proj.savedGame(*elm);
         Server.setElmLong(elm->id);
         Proj.enumChanged(elm->id);
      }
      changed=false;
   }
   void EnumEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmEnum *data=elm->enumData()){data->newVer(); data->from(enums);}
         toGui();
      }
   }
   void EnumEditor::Remove(Enum &e) {EnumEdit.remove(e);}
          void EnumEditor::remove(Enum &e)
   {
      if(EditEnum *ee=enums.find(e.enum_id))
      {
         undos.set("remove", true);
         ee->setRemoved(!ee->removed);
         setChanged();
      }
   }
   void EnumEditor::toGui()
   {
      int v=0; flt h=list.elmHeight();
      data.clear(); FREPA(enums)if(!enums[i].removed || show_removed())
      {
         Enum &e=data.New(); e=enums[i];
         region+=e.remove.create(Rect_LU(0, -(v++)*h, h, h), "R").func(Remove, e).desc(enums[i].removed ? "Restore this element" : "Remove this element");
         e.offset=h;
      }
      list.setData(data);
      type.set(enums.type, QUIET);
   }
   void EnumEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_ENUM)elm=null;
      if(T.elm!=elm)
      {
         rename_window.hide();
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm)enums.load(Proj.editPath(elm->id));else enums.del();
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
           void        EnumEditor::activate(Elm *elm)         {set(elm); if(T.elm)::EE::GuiObj::activate();}
           void        EnumEditor::toggle(Elm *elm)         {if(elm==T.elm)elm=null; set(elm);}
   EnumEditor& EnumEditor::hide(        ){if(visible()){::EE::Window::hide(); set(null);} return T;}
   void EnumEditor::DragEnums(EnumEditor &ee, GuiObj *obj, C Vec2 &screen_pos) {ee.dragEnums(obj, screen_pos);}
          void EnumEditor::dragEnums(                GuiObj *obj, C Vec2 &screen_pos)
   {
      if(elm && obj==&list)
      {
         list.sel.sort(Compare); // sort to make sure we're processing from first to last
         int target=list.visToAbs(list.getPos(screen_pos));
         if(list.sel.elms() && target>list.sel.first())target--;

         // if before 'target' there are other elements selected, then adjust 'target' to the first selected element (which is connected to target)
         int ti=list.sel.find(target); if(ti>=0)for(; InRange(ti-1, list.sel); )
         {
            int prev_target=list.sel[ti-1];
            if(list.absToVis(prev_target)==list.absToVis(target)-1){ti--; target=prev_target;}else break;
         }

         UID       list_cur; if(Enum *e=list())list_cur=e->enum_id;else list_cur.zero();
         Memt<UID> enum_id; FREPA(list.sel)if(Enum *e=list.absToData(list.sel[i]))enum_id.add(e->enum_id);
         if(enum_id.elms())
         {
            undos.set("move", true);
            int tar=-1; if(InRange(target, list.totalElms()))if(Enum *e=list.absToData(target))tar=enums.findI(e->enum_id);
            if(!InRange(tar, enums))tar=enums.elms(); // move to end
            FREPA(enum_id)tar=enums.move(enum_id[i], tar);
            setChanged();
                          REPD(v, list.visibleElms())if(Enum *e=list.visToData(v))if(e->enum_id==list_cur  ){list.cur        =v ; break;}
            FREPA(enum_id)REPD(a, list.  totalElms())if(Enum *e=list.absToData(a))if(e->enum_id==enum_id[i]){list.sel.include(a); break;}
         }
      }
   }
   void EnumEditor::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.window()==this)REPA(MT)if(MT.bp(i) && MT.guiObj(i)==&list)if(Enum *e=list())
      {
         if(list.selMode()==LSM_SET)Gui.drag(DragEnums, T, MT.touch(i));
         if(MT.bd(i))rename_window.activate(e->enum_id, e->name);
      }
   }
   void EnumEditor::elmChanged(C UID &enum_id)
   {
      if(enum_id==elm_id && enum_id.valid())
      {
         undos.set(null, true);
         EditEnums temp; if(temp.load(Proj.editPath(elm_id)))if(enums.sync(temp))toGui();
      }
   }
   void EnumEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
EnumEditor::EnumEditor() : elm_id(UIDZero), elm(null), changed(false), undos(true) {}

EnumEditor::Enum::Enum() : removed(false), color(BLACK), offset(0), enum_id(UIDZero) {}

EnumEditor::RenameWindow::RenameWindow() : enum_id(UIDZero) {}

/******************************************************************************/
