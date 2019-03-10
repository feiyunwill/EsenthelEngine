/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_GOBJ CC4('G','O','B','J')
/******************************************************************************/
// MANAGE
/******************************************************************************/
GuiObjs& GuiObjs::del()
{
  _objs .del();
  _names.del();
   REP(GO_NUM)if(Memb<GuiObj> *objs=T.objs(GUI_OBJ_TYPE(i)))objs->del();
   return T;
}
/******************************************************************************/
struct GuiObjsObj : GuiObjs::Obj
{
 C GuiObj *go;

   void set(Int index, C GuiObj &go, CChar *name, Memc<C Memx<GuiObj>*> &src, Memc<Str> &names)
   {
      // set object
      T.type=go.type(); T.index=index; T.go=&go;

      // set parent
      parent_type=GO_NONE; parent_index=parent_sub=-1;
      if(GuiObj *parent=go.parent())
      {
         // if objects parent is a Tab, then instead of storing it as child of Tab, we store it as child of Tabs, with index specified to given Tab (this is because there are no single Tab objects in the Gui, but they're always a member of Tabs)
         Tab *tab=null; if(parent->type()==GO_TAB){tab=&parent->asTab(); parent=tab->parent();} // use parent of Tab (which is Tabs)
         if(parent)if(C Memx<GuiObj> *parent_container=src[parent->type()])
         {
               parent_index=parent_container->validIndex(parent);
            if(parent_index>=0)
            {
                  parent_type=parent->type();
               if(parent_type==GO_TABS)parent_sub=parent->asTabs()._tabs.validIndex(tab); // get index of Tab in Tabs
            }
         }
      }

      // set name
      name_offset=0;
      FREPA(names) // order is important
      {
         if(EqualPath(names[i], name))return; // this name is ok
         name_offset+=names[i].length()+1; // proceed to next name and null character
      }
      names.add(name); // name was not found, we need to add new one, 'name_offset' is already ok
   }
};
static Int Compare(C GuiObjsObj &a, C GuiObjsObj &b) // compare in order of creation
{
   if(C GuiObj *ga=a.go)
   if(C GuiObj *gb=b.go)
   {
      // do quick checks first which don't require memory allocation
      if(ga==gb          )return  0;
      if(ga->contains(gb))return -1; // if 'a' is parent of 'b', the 'a' must be stored first
      if(gb->contains(ga))return +1; // if 'b' is parent of 'a', the 'b' must be stored first

      // set history (add all parents starting from self to root)
      Memc<C GuiObj*> ah; for(C GuiObj *g=ga; g; g=g->parent())ah.add(g);
      Memc<C GuiObj*> bh; for(C GuiObj *g=gb; g; g=g->parent())bh.add(g);

      // go from root until different objects are found
      for(; ah.elms() && bh.elms(); )
      {
       C GuiObj *a=ah.pop();
       C GuiObj *b=bh.pop();

         if(a!=b)return a->compareLevel(*b); // return the order in which elements are stored
      }
   }
   return 0;
}
GuiObjs& GuiObjs::create( // create from objects
   C Memx< ObjName< Button   > > &button  ,
   C Memx< ObjName< CheckBox > > &checkbox,
   C Memx< ObjName< ComboBox > > &combobox,
   C Memx< ObjName< GuiCustom> > &custom  ,
   C Memx< ObjName< Desktop  > > &desktop ,
   C Memx< ObjName< GuiImage > > &image   ,
   C Memx< ObjName<_List     > > &list    ,
   C Memx< ObjName< Menu     > > &menu    ,
   C Memx< ObjName< MenuBar  > > &menubar ,
   C Memx< ObjName< Progress > > &progress,
   C Memx< ObjName< Region   > > &region  ,
   C Memx< ObjName< SlideBar > > &slidebar,
   C Memx< ObjName< Slider   > > &slider  ,
   C Memx< ObjName< Tabs     > > &tabs    ,
   C Memx< ObjName< Text     > > &text    ,
   C Memx< ObjName< TextBox  > > &textbox ,
   C Memx< ObjName< TextLine > > &textline,
   C Memx< ObjName< Viewport > > &viewport,
   C Memx< ObjName< Window   > > &window  )
{
   del();

   Memc<C Memx<GuiObj>*> src; src.setNumZero(GO_NUM);
   src[GO_BUTTON  ]=&SCAST(C Memx<GuiObj>, button  );
   src[GO_CHECKBOX]=&SCAST(C Memx<GuiObj>, checkbox);
   src[GO_COMBOBOX]=&SCAST(C Memx<GuiObj>, combobox);
   src[GO_CUSTOM  ]=&SCAST(C Memx<GuiObj>, custom  );
   src[GO_DESKTOP ]=&SCAST(C Memx<GuiObj>, desktop );
   src[GO_IMAGE   ]=&SCAST(C Memx<GuiObj>, image   );
   src[GO_LIST    ]=&SCAST(C Memx<GuiObj>, list    );
   src[GO_MENU    ]=&SCAST(C Memx<GuiObj>, menu    );
   src[GO_MENU_BAR]=&SCAST(C Memx<GuiObj>, menubar );
   src[GO_PROGRESS]=&SCAST(C Memx<GuiObj>, progress);
   src[GO_REGION  ]=&SCAST(C Memx<GuiObj>, region  );
   src[GO_SLIDEBAR]=&SCAST(C Memx<GuiObj>, slidebar);
   src[GO_SLIDER  ]=&SCAST(C Memx<GuiObj>, slider  );
   src[GO_TABS    ]=&SCAST(C Memx<GuiObj>, tabs    );
   src[GO_TEXT    ]=&SCAST(C Memx<GuiObj>, text    );
   src[GO_TEXTBOX ]=&SCAST(C Memx<GuiObj>, textbox );
   src[GO_TEXTLINE]=&SCAST(C Memx<GuiObj>, textline);
   src[GO_VIEWPORT]=&SCAST(C Memx<GuiObj>, viewport);
   src[GO_WINDOW  ]=&SCAST(C Memx<GuiObj>, window  );

   // create objects in dest containers (order is important, so it matches source indexes)
   FREPA(button  )T._button  .New().create(button  [i]);
   FREPA(checkbox)T._checkbox.New().create(checkbox[i]);
   FREPA(combobox)T._combobox.New().create(combobox[i]);
   FREPA(menu    )T._menu    .New().create(menu    [i]);
   FREPA(custom  )T._custom  .New().create(custom  [i]);
   FREPA(desktop )T._desktop .New().create(desktop [i]);
   FREPA(image   )T._image   .New().create(image   [i]);
   FREPA(list    )T._list    .New().create(list    [i]);
   FREPA(menubar )T._menubar .New().create(menubar [i]);
   FREPA(progress)T._progress.New().create(progress[i]);
   FREPA(region  )T._region  .New().create(region  [i]);
   FREPA(slidebar)T._slidebar.New().create(slidebar[i]);
   FREPA(slider  )T._slider  .New().create(slider  [i]);
   FREPA(tabs    )T._tabs    .New().create(tabs    [i]);
   FREPA(text    )T._text    .New().create(text    [i]);
   FREPA(textbox )T._textbox .New().create(textbox [i]);
   FREPA(textline)T._textline.New().create(textline[i]);
   FREPA(viewport)T._viewport.New().create(viewport[i]);
   FREPA(window  )T._window  .New().create(window  [i]);

   Memc<GuiObjsObj> goi  ; // create an array of all gui object types (no specific order)
   Memc<Str       > names;
   FREPA(button  )goi.New().set(i, button  [i], button  [i].name, src, names);
   FREPA(checkbox)goi.New().set(i, checkbox[i], checkbox[i].name, src, names);
   FREPA(combobox)goi.New().set(i, combobox[i], combobox[i].name, src, names);
   FREPA(menu    )goi.New().set(i, menu    [i], menu    [i].name, src, names);
   FREPA(custom  )goi.New().set(i, custom  [i], custom  [i].name, src, names);
   FREPA(desktop )goi.New().set(i, desktop [i], desktop [i].name, src, names);
   FREPA(image   )goi.New().set(i, image   [i], image   [i].name, src, names);
   FREPA(list    )goi.New().set(i, list    [i], list    [i].name, src, names);
   FREPA(menubar )goi.New().set(i, menubar [i], menubar [i].name, src, names);
   FREPA(progress)goi.New().set(i, progress[i], progress[i].name, src, names);
   FREPA(region  )goi.New().set(i, region  [i], region  [i].name, src, names);
   FREPA(slidebar)goi.New().set(i, slidebar[i], slidebar[i].name, src, names);
   FREPA(slider  )goi.New().set(i, slider  [i], slider  [i].name, src, names);
   FREPA(tabs    )goi.New().set(i, tabs    [i], tabs    [i].name, src, names);
   FREPA(text    )goi.New().set(i, text    [i], text    [i].name, src, names);
   FREPA(textbox )goi.New().set(i, textbox [i], textbox [i].name, src, names);
   FREPA(textline)goi.New().set(i, textline[i], textline[i].name, src, names);
   FREPA(viewport)goi.New().set(i, viewport[i], viewport[i].name, src, names);
   FREPA(window  )goi.New().set(i, window  [i], window  [i].name, src, names);
   Int name_length=0;  REPA(names)name_length+=names[i].length()+1; T._names.setNum(name_length);
       name_length=0; FREPA(names){C Str &name=names[i]; FREPA(name)T._names[name_length++]=name[i]; T._names[name_length++]='\0';}
   T._objs=goi.sort(Compare); // sort by order of creation
   return T;
}
/******************************************************************************/
static GuiObj* GetObj(Memc<Memx<GuiObj>*> &objs, Memc<Int> &offset, GUI_OBJ_TYPE type, Int index, Int sub=-1)
{
   if(InRange(type, objs) && InRange(type, offset))if(Memx<GuiObj> *obj=objs[type])
   {
      index+=offset[type];
      if(InRange(index, *obj))
      {
         GuiObj &go=(*obj)[index];
         if(go.type()==GO_TABS && InRange(sub, go.asTabs()))return &go.asTabs().tab(sub);
         return &go;
      }
   }
   return null;
}
void GuiObjs::copyTo( // copy self to object containers
   Memx< ObjName< Button   > > &button  ,
   Memx< ObjName< CheckBox > > &checkbox,
   Memx< ObjName< ComboBox > > &combobox,
   Memx< ObjName< GuiCustom> > &custom  ,
   Memx< ObjName< Desktop  > > &desktop ,
   Memx< ObjName< GuiImage > > &image   ,
   Memx< ObjName<_List     > > &list    ,
   Memx< ObjName< Menu     > > &menu    ,
   Memx< ObjName< MenuBar  > > &menubar ,
   Memx< ObjName< Progress > > &progress,
   Memx< ObjName< Region   > > &region  ,
   Memx< ObjName< SlideBar > > &slidebar,
   Memx< ObjName< Slider   > > &slider  ,
   Memx< ObjName< Tabs     > > &tabs    ,
   Memx< ObjName< Text     > > &text    ,
   Memx< ObjName< TextBox  > > &textbox ,
   Memx< ObjName< TextLine > > &textline,
   Memx< ObjName< Viewport > > &viewport,
   Memx< ObjName< Window   > > &window  ,
   GuiObj                      *parent  )C
{
   Memc<Memx<GuiObj>*> dest  ; dest  .setNumZero(GO_NUM);
   Memc<Int          > offset; offset.setNumZero(GO_NUM); // offset needed to be applied to object index in the target container, depending on elements present there already (before creating new elements)
   dest[GO_BUTTON  ]=&SCAST(Memx<GuiObj>, button  );
   dest[GO_CHECKBOX]=&SCAST(Memx<GuiObj>, checkbox);
   dest[GO_COMBOBOX]=&SCAST(Memx<GuiObj>, combobox);
   dest[GO_CUSTOM  ]=&SCAST(Memx<GuiObj>, custom  );
   dest[GO_DESKTOP ]=&SCAST(Memx<GuiObj>, desktop );
   dest[GO_IMAGE   ]=&SCAST(Memx<GuiObj>, image   );
   dest[GO_LIST    ]=&SCAST(Memx<GuiObj>, list    );
   dest[GO_MENU    ]=&SCAST(Memx<GuiObj>, menu    );
   dest[GO_MENU_BAR]=&SCAST(Memx<GuiObj>, menubar );
   dest[GO_PROGRESS]=&SCAST(Memx<GuiObj>, progress);
   dest[GO_REGION  ]=&SCAST(Memx<GuiObj>, region  );
   dest[GO_SLIDEBAR]=&SCAST(Memx<GuiObj>, slidebar);
   dest[GO_SLIDER  ]=&SCAST(Memx<GuiObj>, slider  );
   dest[GO_TABS    ]=&SCAST(Memx<GuiObj>, tabs    );
   dest[GO_TEXT    ]=&SCAST(Memx<GuiObj>, text    );
   dest[GO_TEXTBOX ]=&SCAST(Memx<GuiObj>, textbox );
   dest[GO_TEXTLINE]=&SCAST(Memx<GuiObj>, textline);
   dest[GO_VIEWPORT]=&SCAST(Memx<GuiObj>, viewport);
   dest[GO_WINDOW  ]=&SCAST(Memx<GuiObj>, window  );
   FREPAO(offset)=(dest[i] ? dest[i]->elms() : 0); // do this before new elm creation

   // create objects in dest containers (order is important, so it matches source indexes)
   FREPA(T._button  )button  .New().create(T._button  [i]);
   FREPA(T._checkbox)checkbox.New().create(T._checkbox[i]);
   FREPA(T._combobox)combobox.New().create(T._combobox[i]);
   FREPA(T._custom  )custom  .New().create(T._custom  [i]);
   FREPA(T._desktop )desktop .New().create(T._desktop [i]);
   FREPA(T._image   )image   .New().create(T._image   [i]);
   FREPA(T._list    )list    .New().create(T._list    [i]);
   FREPA(T._menu    )menu    .New().create(T._menu    [i]);
   FREPA(T._menubar )menubar .New().create(T._menubar [i]);
   FREPA(T._progress)progress.New().create(T._progress[i]);
   FREPA(T._region  )region  .New().create(T._region  [i]);
   FREPA(T._slidebar)slidebar.New().create(T._slidebar[i]);
   FREPA(T._slider  )slider  .New().create(T._slider  [i]);
   FREPA(T._tabs    )tabs    .New().create(T._tabs    [i]);
   FREPA(T._text    )text    .New().create(T._text    [i]);
   FREPA(T._textbox )textbox .New().create(T._textbox [i]);
   FREPA(T._textline)textline.New().create(T._textline[i]);
   FREPA(T._viewport)viewport.New().create(T._viewport[i]);
   FREPA(T._window  )window  .New().create(T._window  [i]);

   // set names and add children to parents
   FREPA(_objs) // from start to preserve order in containers
   {
    C Obj &goi=_objs[i];
      if(GuiObj *go=GetObj(dest, offset, goi.type, goi.index))
      {
         Int dest_index=goi.index+offset[goi.type];

         // set name
         CChar *name=_names.data()+goi.name_offset;
         if( Is(name))switch(goi.type)
         {
            case GO_BUTTON  : button  [dest_index].name=name; break;
            case GO_CHECKBOX: checkbox[dest_index].name=name; break;
            case GO_COMBOBOX: combobox[dest_index].name=name; break;
            case GO_CUSTOM  : custom  [dest_index].name=name; break;
            case GO_DESKTOP : desktop [dest_index].name=name; break;
            case GO_IMAGE   : image   [dest_index].name=name; break;
            case GO_LIST    : list    [dest_index].name=name; break;
            case GO_MENU    : menu    [dest_index].name=name; break;
            case GO_MENU_BAR: menubar [dest_index].name=name; break;
            case GO_PROGRESS: progress[dest_index].name=name; break;
            case GO_REGION  : region  [dest_index].name=name; break;
            case GO_SLIDEBAR: slidebar[dest_index].name=name; break;
            case GO_SLIDER  : slider  [dest_index].name=name; break;
            case GO_TABS    : tabs    [dest_index].name=name; break;
            case GO_TEXT    : text    [dest_index].name=name; break;
            case GO_TEXTBOX : textbox [dest_index].name=name; break;
            case GO_TEXTLINE: textline[dest_index].name=name; break;
            case GO_VIEWPORT: viewport[dest_index].name=name; break;
            case GO_WINDOW  : window  [dest_index].name=name; break;
         }

         // set parent
         if(GuiObj *p=GetObj(dest, offset, goi.parent_type, goi.parent_index, goi.parent_sub))*p+=*go;else
         if(parent)*parent+=*go;
      }
   }
}
/******************************************************************************/
// GET
/******************************************************************************/
C MembConst<GuiObj>* GuiObjs::objects(GUI_OBJ_TYPE type)C {return ConstCast(T).objs(type);}
  MembConst<GuiObj>* GuiObjs::objs   (GUI_OBJ_TYPE type)
{
   switch(type)
   {
      default         : return null;
      case GO_BUTTON  : return &SCAST(MembConst<GuiObj>, _button);
      case GO_CHECKBOX: return &SCAST(MembConst<GuiObj>, _checkbox);
      case GO_COMBOBOX: return &SCAST(MembConst<GuiObj>, _combobox);
      case GO_CUSTOM  : return &SCAST(MembConst<GuiObj>, _custom);
      case GO_DESKTOP : return &SCAST(MembConst<GuiObj>, _desktop);
      case GO_IMAGE   : return &SCAST(MembConst<GuiObj>, _image);
      case GO_LIST    : return &SCAST(MembConst<GuiObj>, _list);
      case GO_MENU    : return &SCAST(MembConst<GuiObj>, _menu);
      case GO_MENU_BAR: return &SCAST(MembConst<GuiObj>, _menubar);
      case GO_PROGRESS: return &SCAST(MembConst<GuiObj>, _progress);
      case GO_REGION  : return &SCAST(MembConst<GuiObj>, _region);
      case GO_SLIDEBAR: return &SCAST(MembConst<GuiObj>, _slidebar);
      case GO_SLIDER  : return &SCAST(MembConst<GuiObj>, _slider);
      case GO_TABS    : return &SCAST(MembConst<GuiObj>, _tabs);
      case GO_TEXT    : return &SCAST(MembConst<GuiObj>, _text);
      case GO_TEXTBOX : return &SCAST(MembConst<GuiObj>, _textbox);
      case GO_TEXTLINE: return &SCAST(MembConst<GuiObj>, _textline);
      case GO_VIEWPORT: return &SCAST(MembConst<GuiObj>, _viewport);
      case GO_WINDOW  : return &SCAST(MembConst<GuiObj>, _window);
   }
}
/******************************************************************************/
GuiObj* GuiObjs::go(GUI_OBJ_TYPE type, Int index, Int sub)
{
   if(Memb<GuiObj> *objs=T.objs(type))if(InRange(index, *objs))
   {
      GuiObj &go=(*objs)[index];
      if(go.type()==GO_TABS && InRange(sub, go.asTabs()))return &go.asTabs().tab(sub);
      return &go;
   }
   return null;
}
/******************************************************************************/
GuiObj* GuiObjs::find(C Str &name, GUI_OBJ_TYPE type)
{
#if 1 // elements are saved in back to front order, that's why linear searching is required (and not binary)
   REPA(_objs)
   {
    C Obj &goi=_objs[i];
      if(goi.type==type && EqualPath(T._names.data()+goi.name_offset, name))return go(goi.type, goi.index);
   }
#else // binary search
   Int l=0, r=_objs.elms(); for(; l<r; )
   {
      Int mid=UInt(l+r)/2; Obj &goi=_objs[mid];
      Int compare=ComparePath(name, T._names.data()+goi.name_offset);
      if(!compare)compare=type-goi.type;
      if(!compare)return go(goi.type, goi.index);
      if( compare<0)r=mid;
      else          l=mid+1;
   }
#endif
   return null;
}
GuiObj& GuiObjs::get(C Str &name, GUI_OBJ_TYPE type)
{
   GuiObj *go=find(name, type); if(!go)Exit(MLT(S+"Can't find Gui Object \""        +name+'"',
                                            PL,S+u"Nie można znaleźć Obiektu Gui \""+name+'"'));
   return *go;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
GuiObjs& GuiObjs::hide()
{
   REPA(_objs)
   {
    C Obj &goi=_objs[i];
      if(!goi.parent_type)if(GuiObj *go=T.go(goi.type, goi.index))go->hide();
   }
   return T;
}
GuiObjs& GuiObjs::show()
{
   REPA(_objs)
   {
    C Obj &goi=_objs[i];
      if(!goi.parent_type)if(GuiObj *go=T.go(goi.type, goi.index))go->show();
   }
   return T;
}
GuiObjs& GuiObjs::activate()
{
   REPA(_objs)
   {
    C Obj &goi=_objs[i];
      if(!goi.parent_type)if(GuiObj *go=T.go(goi.type, goi.index)){go->activate(); break;}
   }
   return T;
}
GuiObjs& GuiObjs::fadeIn()
{
   REPA(_objs)
   {
    C Obj &goi=_objs[i];
      if(!goi.parent_type)if(GuiObj *go=T.go(goi.type, goi.index))if(go->type()==GO_WINDOW)go->asWindow().fadeIn();else go->show();
   }
   return T;
}
GuiObjs& GuiObjs::fadeOut()
{
   REPA(_objs)
   {
    C Obj &goi=_objs[i];
      if(!goi.parent_type)if(GuiObj *go=T.go(goi.type, goi.index))if(go->type()==GO_WINDOW)go->asWindow().fadeOut();else go->hide();
   }
   return T;
}
/******************************************************************************/
// IO
/******************************************************************************/
struct GuiObjsObj2
{
   Char         name[48];
   GUI_OBJ_TYPE type , parent_type;
   Int          index, parent_index, parent_sub;
};
struct GuiObjsObj1
{
   Char         name[48];
   GUI_OBJ_TYPE type , parent_type ;
   Int          index, parent_index;
};
struct GuiObjsObj0
{
   Char8        name[24];
   GUI_OBJ_TYPE type , parent_type;
   Int          index, parent_index, desc_pos;
};
Bool GuiObjs::save(File &f, CChar *path)C
{
   f.putMulti(UInt(CC4_GOBJ), Byte(3)); // version
  _names._saveRaw(f);
  _objs ._saveRaw(f);
   FREPD(t, GO_NUM)if(C Memb<GuiObj> *objs=T.objects(GUI_OBJ_TYPE(t)))FREPA(*objs)
   {
      f.putByte(t); if(!(*objs)[i].save(f, path))return false;
   }
   return f.ok();
}
Bool GuiObjs::load(File &f, CChar *path)
{
   del(); if(f.getUInt()==CC4_GOBJ)switch(f.decUIntV()) // version
   {
      case 3:
      {
         // Gui Object Info
         if(_names._loadRaw(f))
         if(_objs ._loadRaw(f))
         {
            // Gui Object Data
            REPA(_objs)
            {
               if(Memb<GuiObj> *objs=T.objs(GUI_OBJ_TYPE(f.getByte())))
               {
                  if(!objs->New().load(f, path))goto error; // load gui object data
               }else goto error; // invalid Gui Object Type
            }

            // add children to parents
            FREPA(_objs) // from start to preserve order in containers
            {
                   C Obj &goi   =_objs[i];
               if(GuiObj *parent=T.go(goi.parent_type, goi.parent_index, goi.parent_sub))
               if(GuiObj *go    =T.go(goi.type       , goi.index                       ))*parent+=*go;
            }
            if(f.ok())return true;
         }
      }break;

      case 2:
      {
         // Gui Object Info
         Mems<GuiObjsObj2> old; if(old._loadRaw(f))
         {
            Memc<Char> old_names;
           _objs.setNum(old.elms());
            FREPA(_objs)
            {
                  Obj      &dest=_objs[i];
               GuiObjsObj2 &src = old [i];
               dest.name_offset =old_names.elms(); Int length=Length(src.name); FREP(length)old_names.add(src.name[i]); old_names.add('\0');
               dest.type        =src.type        ;
               dest.index       =src.index       ;
               dest.parent_type =src.parent_type ;
               dest.parent_index=src.parent_index;
               dest.parent_sub  =src.parent_sub  ;
            }
           _names=old_names;

            // Gui Object Data
            REPA(_objs)
            {
               if(Memb<GuiObj> *objs=T.objs(GUI_OBJ_TYPE(f.getByte())))
               {
                  if(!objs->New().load(f, path))goto error; // load gui object data
               }else goto error; // invalid Gui Object Type
            }

            // add children to parents
            FREPA(_objs) // from start to preserve order in containers
            {
                   C Obj &goi   =_objs[i];
               if(GuiObj *parent=T.go(goi.parent_type, goi.parent_index, goi.parent_sub))
               if(GuiObj *go    =T.go(goi.type       , goi.index                       ))*parent+=*go;
            }
            if(f.ok())return true;
         }
      }break;

      case 1:
      {
         // Gui Object Info
         Mems<GuiObjsObj1> old; if(old._loadRaw(f))
         {
            Memc<Char> old_names;
           _objs.setNum(old.elms());
            FREPA(_objs)
            {
                  Obj      &dest=_objs[i];
               GuiObjsObj1 &src = old [i];
               dest.name_offset =old_names.elms(); Int length=Length(src.name); FREP(length)old_names.add(src.name[i]); old_names.add('\0');
               dest.type        =src.type        ;
               dest.index       =src.index       ;
               dest.parent_type =src.parent_type ;
               dest.parent_index=src.parent_index;
               dest.parent_sub  =-1              ;
            }
           _names=old_names;

            // Gui Object Data
            REPA(_objs)
            {
               if(Memb<GuiObj> *objs=T.objs(GUI_OBJ_TYPE(f.getByte())))
               {
                  if(!objs->New().load(f, path))goto error; // load gui object data
               }else goto error; // invalid Gui Object Type
            }

            // add children to parents
            FREPA(_objs) // from start to preserve order in containers
            {
                   C Obj &goi   =_objs[i];
               if(GuiObj *parent=T.go(goi.parent_type, goi.parent_index))
               if(GuiObj *go    =T.go(goi.type       , goi.index       ))*parent+=*go;
            }
            if(f.ok())return true;
         }
      }break;

      case 0:
      {
         // helper texts
         Mems<Char8> helper; if(helper._loadRaw(f))
         {
            // Gui Object Info
            Mems<GuiObjsObj0> old; if(old._loadRaw(f))
            {
               Memc<Char> old_names;
              _objs.setNum(old.elms());
               FREPA(_objs)
               {
                     Obj      &dest=_objs[i];
                  GuiObjsObj0 &src = old [i];
                  dest.name_offset =old_names.elms(); Int length=Length(src.name); FREP(length)old_names.add(src.name[i]); old_names.add('\0');
                  dest.type        =src.type        ;
                  dest.index       =src.index       ;
                  dest.parent_type =src.parent_type ;
                  dest.parent_index=src.parent_index;
                  dest.parent_sub  =-1              ;
               }
              _names=old_names;

               // Gui Object Data
               REPA(_objs)
               {
                  if(Memb<GuiObj> *objs=T.objs(GUI_OBJ_TYPE(f.getByte())))
                  {
                     if(!objs->New().load(f, path))goto error; // load gui object data
                  }else goto error; // invalid Gui Object Type
               }

               // finalize
               FREPA(_objs) // from start to preserve order in containers
               {
                      C Obj &goi=_objs[i];
                  if(GuiObj *go =T.go(goi.type, goi.index))
                  {
                     if(old[i].desc_pos>=0)go->desc(helper.data()+old[i].desc_pos);
                     if(GuiObj *parent=T.go(goi.parent_type, goi.parent_index))*parent+=*go;
                  }
               }
               if(f.ok())return true;
            }
         }
      }break;
   }
error:
   del(); return false;
}
Bool GuiObjs::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool GuiObjs::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   del(); return false;
}
void GuiObjs::operator=(C Str &name)
{
   if(!load(name))Exit(MLT(S+"Can't load Gui Objects \""               +name+"\"",
                       PL,S+u"Nie można wczytać obiektów interfejsu \""+name+"\""));
}
Bool GuiObjs::load     (C UID &id) {return load(id.valid() ? _EncodeFileName(id) : null);}
void GuiObjs::operator=(C UID &id) {         T=(id.valid() ? _EncodeFileName(id) : null);}
/******************************************************************************/
}
/******************************************************************************/
