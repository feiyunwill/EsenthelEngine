/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ObjListClass ObjList;
/******************************************************************************/

/******************************************************************************/
   void  ObjListClass::GroupChanged(ObjListClass &ol) {REP(2)ol.list.columnVisible(i, i!=ol.group()); ol.set();}
   void ObjListClass::FilterChanged(ObjListClass &ol) {ol.setVisible();}
   void       ObjListClass::Refresh(ObjListClass &ol) {ol.set       ();}
   void          ObjListClass::Hide(ObjListClass &ol) {WorldEdit.obj_list.set(false);}
   void    ObjListClass::SelChanged(ObjListClass &ol) {ol.selChanged();}
   void ObjListClass::setChanged() {_changed=true;}
   ObjListClass& ObjListClass::create()
   {
      ::EE::Window::create(Rect_RD(D.w(), -D.h(), 1.09f, 1.2f), "Object List").hide(); button[1].show(); button[2].show().func(Hide, T); flag|=WIN_RESIZABLE;
      ts.reset().size=0.038f; ts.align.set(1, 0); ts2.reset().size=0.033f;
      cchar8 *group_t[]=
      {
         "Class",
         "Object",
      };
      flt y=-0.05f;
      T+=refresh.create(Rect_L(0.015f, y, 0.12f, 0.04f), "Refresh").func(Refresh, T).focusable(false).desc("Refresh object list");
      T+=tgroup .create(refresh.rect().right()+Vec2(0.03f, 0), "Group\nby:", &ts);
      T+= group .create(Rect_L(tgroup.pos()+Vec2(0.10f, 0), 0.25f, 0.05f), 0, group_t, Elms(group_t)).func(GroupChanged, T).set(0);
      T+= filter.create(Rect_L(group.rect().right()+Vec2(0.03f, 0), 0.24f, 0.05f)).func(FilterChanged, T); filter.kbSet(); filter.reset.show(); filter.show_find=true; filter.hint="Find Object";
      T+= params.create(Rect_L(filter.rect().right()+Vec2(0.01f, 0), Vec2(0.04f))).func(FilterChanged, T).desc("If include parameter names and values in custom filtering");
      T+=tparams.create(params.rect().right()+Vec2(0.052f, 0), "include\nparams", &ts2);

      T+= grass.create(Rect_L(tparams.rect().right()+Vec2(0.07f, 0), Vec2(0.04f))).set(true).func(FilterChanged, T).desc("If hide grass objects");
      T+=tgrass.create(grass.rect().right()+Vec2(0.042f, 0), "hide\ngrass", &ts2);

      T+=region.create().slidebarSize(0.04f);
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Elm, obj_type), 0.40f, "Class" ), // 0
         ListColumn(MEMBER(Elm, base    ), 0.40f, "Object"), // 1
         ListColumn(MEMBER(Elm, pos.x   ), 0.12f, "Pos.x" ), // 2
         ListColumn(MEMBER(Elm, pos.y   ), 0.12f, "Pos.y" ), // 3
         ListColumn(MEMBER(Elm, pos.z   ), 0.12f, "Pos.z" ), // 4
      };
      lc[2].precision=lc[3].precision=lc[4].precision=1;
      region+=list.create(lc, Elms(lc)).selChanged(SelChanged, T); list.flag|=LIST_TYPE_SORT|LIST_RESIZABLE_COLUMNS|LIST_MULTI_SEL;
      list.elmHeight(0.035f).textSize(0, 1).columnHeight(0.045f).setElmGroup(MEMBER(Elm, group)).setElmOffset(MEMBER(Elm, offset)).setElmType(MEMBER(Elm, type));

      rect(rect());
      return T;
   }
   int ObjListClass::CompareOL(C Elm &elm, C Str &group) {return Compare(elm.group, group);}
   ::ObjListClass::Elm& ObjListClass::dataNew(C Str &group)
   {
      if(T.group()>=0)
      {
         int i; if(data.binarySearch(group, i, CompareOL)){Elm &child=data[i].children.New(); child.offset=0.03f; return child;}
         Elm &elm=data.NewAt(i); elm.group=group;          Elm &child=elm    .children.New(); child.offset=0.03f; return child;
      }
      return data.New();
   }
   Str ObjListClass::ReplaceSlash(C Str &str) {return Replace(str, '/', '\\');}
   bool ObjListClass::SetVisible(Memt<bool> &visible, C Str &filter, bool filter_params, bool hide_grass, Memx<Elm> &data, bool parent_visible)
   {
      bool any_has_filter_or_visible=false;
      UID  filter_id;
      bool filter_is_id=filter_id.fromText(filter);
      FREPA(data)
      {
         Elm &elm=data[i];

         bool elm_visible=parent_visible,
              has_filter =true;

         if(elm.obj)
         {
            if(filter_is_id)
            {
               has_filter=(elm.obj->id==filter_id // instance
                        || elm.obj->params.base.id()==filter_id); // base
               if(!has_filter)elm_visible=false;
            }else 
            if(hide_grass && elm.obj->params.access==OBJ_ACCESS_GRASS)
            {
               elm_visible=false;
               has_filter =false;
            }else
            if(filter.is())
            {
               has_filter=(ContainsAll(elm.obj_type, filter)
                        || ContainsAll(elm.base    , filter)
                        || (filter_params && elm.obj->params.paramValuesContain(filter, Proj)));
               if(!has_filter)elm_visible=false;
            }
         }

         int  vi=visible.elms(); visible.add(elm_visible);
         bool any_child_has_filter_or_visible=SetVisible(visible, filter, filter_params, hide_grass, elm.children, elm_visible && elm.opened);
         if(!elm.obj && !any_child_has_filter_or_visible)visible[vi]=false; // if this is a group && no children (contain filter or are visible), then hide this group

         any_has_filter_or_visible|=(has_filter|visible[vi]);
      }
      return any_has_filter_or_visible;
   }
   void ObjListClass::setVisible()
   {
      Memt<bool> visible; SetVisible(visible, ReplaceSlash(filter()), params(), grass(), data);
      list.setDataNode(data, visible, true);
      list.sel.clear(); FREP(list.totalElms())if(Elm *elm=list.absToData(i))if(elm->obj && elm->obj->selected)list.sel.add(i);
   }
   void ObjListClass::SetType(Memx<Elm> &data, uint type)
   {
      FREPA(data)
      {
         Elm &elm=data[i]; elm.type=type; SetType(elm.children, type);
      }
   }
   void ObjListClass::set()
   {
      data.clear();
      FREPA(WorldEdit.obj_visible)
      {
         Obj &obj=*WorldEdit.obj_visible[i];
         Str  base=Proj.elmFullName(obj.params.base.id());
         Str  type;
         if(obj.params.access!=OBJ_ACCESS_CUSTOM && InRange(obj.params.access, ObjAccessNamesElms))type=ObjAccessNames[obj.params.access];else
         if(::Elm *obj_class=Proj.findElm(obj.params.type))type=obj_class->name;
         Elm &elm =dataNew((group()==0) ? type : (group()==1) ? base : S);
              elm.obj     =&obj;
              elm.pos     =obj.pos();
         Swap(elm.obj_type, type);
         Swap(elm.base    , base);
      }
      if(group()>=0)REPA(data  )data[i].group=S+'<'+data[i].group+'>';
      if(group()>=0)REPA(opened){int di; if(data.binarySearch(opened[i], di, CompareOL))data[di].opened=true;}
      if(group()>=0){uint type=0; FREPA(data){data[i].type=type++; SetType(data[i].children, type++);}}
      setVisible();
     _changed=false;
   }
   void ObjListClass::selChanged()
   {
      Selection.clearSelect();
      FREPA(list.sel)if(Elm *elm=list.absToData(list.sel[i]))if(elm->obj)Selection.select(*elm->obj);
   }
   Rect ObjListClass::sizeLimit()C 
{
      Rect   r=::EE::Window::sizeLimit(); r.min.set(0.4f, 0.3f);
      return r;
   }
           C Rect& ObjListClass::rect()C {return ::EE::Window::rect();}
   Window& ObjListClass::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      region.rect(Rect(0, -clientHeight(), clientWidth(), group.rect().min.y).extend(-0.03f));
      return T;
   }
   void ObjListClass::update(C GuiPC &gpc)
{
      visible(StateActive==&StateProject && Mode()==MODE_WORLD && WorldEdit.mode()==WorldView::OBJECT && WorldEdit.obj_list());
      if(visible() && gpc.visible && _changed)set(); // first set data
      ::EE::ClosableWindow::update(gpc); // now update list
      if(visible() && gpc.visible)
      {
         REPA(MT)if(MT.guiObj(i)==&list && MT.bp(i))if(Elm *elm=list())
         {
            if(elm->obj && MT.bd(i) && list.selMode()==LSM_SET)WorldEdit.v4.moveTo(elm->obj->center()); // camera center
            if(elm->children.elms()){opened.toggle(elm->group); elm->opened^=1; setVisible();} // open
         }
      }
   }
   void ObjListClass::removed(Obj &obj)
   {
      REPA(data)if(data[i].obj==&obj){data[i].obj=null; setChanged(); break;}
   }
ObjListClass::ObjListClass() : _changed(false) {}

ObjListClass::Elm::Elm() : opened(false), type(0), offset(0), obj(null) {}

/******************************************************************************/
