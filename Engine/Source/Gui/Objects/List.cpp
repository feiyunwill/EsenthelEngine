/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   LCM_MOUSE is disabled if last action was by keyboard,
      to allow changing cursor with the keyboard.

/******************************************************************************/
#define MIN_COLUMN_EDGE_WIDTH 0.030f
#define COLUMN_RESIZE_WIDTH   0.015f
/******************************************************************************/
static _Memx& NodeChildren( Ptr   data, Int children_offset) {return *(_Memx*)(((Byte*)data)+children_offset);}
static  Int   NodeElms    (_Memx &node, Int children_offset)
{
   Int    elms=0; FREPA(node)elms+=1+NodeElms(NodeChildren(node[i], children_offset), children_offset); // increase self + children of self
   return elms;
}
static void NodeVisible(_Memx &node, Int children_offset, Ptr* &abs_to_data, Ptr* &vis_to_data, C MemPtr<Bool> &visible, Int &index)
{
   FREPA(node)
   {
      Ptr data=node[i];
                                           *abs_to_data++=data;
      if(!visible || ElmIs(visible, index))*vis_to_data++=data;
      NodeVisible(NodeChildren(data, children_offset), children_offset, abs_to_data, vis_to_data, visible, ++index);
   }
}
static Ptr NodeAbsToData(_Memx &node, Int children_offset, Int &abs)
{
   FREPA(node)
   {
      Ptr e=node[i]; if(!abs--)return e;
      if( e=NodeAbsToData(NodeChildren(e, children_offset), children_offset, abs))return e;
   }
   return null;
}
static Int NodeDataToAbs(_Memx &node, Int children_offset, Ptr data, Int &abs)
{
   FREPA(node)
   {
      Ptr e=node[i]; if(e==data)return abs; abs++;
      Int j=NodeDataToAbs(NodeChildren(e, children_offset), children_offset, data, abs); if(j>=0)return j;
   }
   return -1;
}
/******************************************************************************/
GuiPC::GuiPC(C GuiPC &old, _List &list) // this is to be used for columns only
{
   T=old;
   visible&=(list.visible() && list.columnsVisible());
   enabled&= list.enabled();
#if 0
   if(list.parent() && list.parent()->type()==GO_REGION)offset.y-=list.parent()->asRegion().slidebar[1].offset(); // this can't be used due to pixel align (when scrolling list, the column buttons shake)
#else
   offset.y=client_rect.max.y;
#endif
}
static Bool SetGPC(_List &list, GuiPC &gpc, C Vec2 &offset, C VecI2 &abs_col, C VecI2 &visible_range) // this is to be used for children only
{
   // column can be greater than the number of columns, in that case it will be drawn after the last visible column
   if(abs_col.y>=0)
      if(!(list.drawMode()==LDM_RECTS && abs_col.y!=list.draw_column)) // for 'LDM_RECTS' we draw only the 'draw_column'
      if(!(list.drawMode()==LDM_LIST  && InRange(abs_col.y, list._columns) && list.column(abs_col.y).hidden())) // for 'LDM_LIST' don't draw hidden columns (do this check only for existing columns, but not when we want after all columns)
   {
      Int vis=list.absToVis(abs_col.x);
      if( vis>=0 && vis>=visible_range.x && vis<=visible_range.y)
      {
         gpc.offset=list.visToLocalPos(vis)+offset;
         if(list.drawMode()==LDM_LIST)gpc.offset.x+=list.columnOffset(abs_col.y);
         return true;
      }
   }
   return false;
}
/******************************************************************************/
// COLUMN
/******************************************************************************/
ListColumn::ListColumn(                                                            )                          {create(null,     0,    S);}
ListColumn::ListColumn(DATA_TYPE type, Int offset, Int size, Flt width, C Str &name) : md(type, offset, size) {create(null, width, name);}
ListColumn::ListColumn(ListColumn &&lc                                             ) : ListColumn()           {Swap(T, lc);}
/******************************************************************************/
void ListColumn::create(Str (*data_to_text)(CPtr data), Flt width, C Str &name)
{
   T.md._data_to_text=data_to_text;
   if(data_to_text && !T.md.type)T.md.type=DATA_CHAR_PTR;
   T._resize_edge=0;
   T.text_align  =DataAlign(md.type);
   T.precision   =3    ;
   T.width       =width;
   T.sort        =null ;

   super::create(name).focusable(false)._sub_type=BUTTON_TYPE_LIST_COLUMN;
   super::text_align=1;
}
void ListColumn::pushed()
{
   if(_List *list=CAST(_List, parent()))
   {
      Int index =list->_columns.index(this);
      if( index>=0)
      {
         if(index==list->draw_column) // if clicked a draw column, then toggle list draw mode
         {
            list->drawMode((list->drawMode()==LDM_LIST) ? LDM_RECTS : LDM_LIST);
         }else // otherwise
         {
            if(list->flag&LIST_SORTABLE)list->sort(index); // check for sorting
         }
      }
   }
}
static void Pushed(ListColumn &lc) {lc.pushed();}

void ListColumn::create(C ListColumn &src, _List &list)
{
   super::create(src).func(Pushed, T)._parent=&list;
   skin=list.skin();

   precision =src.precision;
   width     =src.width;
   text_align=src.text_align;
   md        =src.md;
   sort      =src.sort;

   if(md.type==DATA_IMAGE
   || md.type==DATA_IMAGE_PTR
   || md.type==DATA_IMAGEPTR)
   {
      Int index =  list._columns.index(this);
      if( index>=0)list.draw_column=index;
   }
}
/******************************************************************************/
static Int ResizeColumn(_List *list, ListColumn &column, Int &resize_edge)
{
   if(resize_edge)
   {
      if(!list)resize_edge=0;else
      {
         Int i=list->_columns.index(&column); if(i>=0)
         {
            if(resize_edge==-1)for(i--; i>=0; i--)if(list->_columns[i].visible())break; // find first previous visible column
            if(!InRange(i, list->_columns))resize_edge=0;
            return i;
         }
      }
   }
   return -1;
}
void ListColumn::update(C GuiPC &gpc)
{
   if(Gui.ms()==this)
   {
     _List *list=((parent() && parent()->type()==GO_LIST) ? &parent()->asList() : null);

      if(Ms.b(0) || Ms.br(0)) // resize
      {
         Int col=ResizeColumn(list, T, _resize_edge);
         if(list && InRange(col, list->_columns))
         {
            if(Ms.bd(0)) // auto-size
            {
               list->columnWidth(col, Max(MIN_COLUMN_EDGE_WIDTH, list->columnDataWidthEx(col)));
            }else
            if(Flt d=Ms.dc().x)
               list->columnWidth(col, Max(MIN_COLUMN_EDGE_WIDTH, list->columnWidth(col)+d));
         }
      }else
      {
        _resize_edge=0;
         if(list && (list->flag&LIST_RESIZABLE_COLUMNS))
         {
            if(Ms.pos().x<=rect().min.x+gpc.offset.x+COLUMN_RESIZE_WIDTH)_resize_edge=-1;else
            if(Ms.pos().x>=rect().max.x+gpc.offset.x-COLUMN_RESIZE_WIDTH)_resize_edge=+1;
         }
         ResizeColumn(list, T, _resize_edge);
      }
   }else _resize_edge=0;

   if(_resize_edge)super::update(GuiPC(gpc, true, false));else super::update(gpc);
}
/******************************************************************************/
Flt _List::parentWidth()C
{
   if(GuiObj *parent=T.parent())
   {
      if(parent->type()==GO_MENU)
      {
         if(GuiObj *owner=parent->asMenu().Owner())if(owner->type()==GO_COMBOBOX)return owner->clientSize().x;
      }

      if(parent->type()==GO_REGION)return parent->size().x-parent->asRegion().slidebarSize();
      else                         return parent->clientSize().x;
   }
   return 0;
}
Flt _List::columnDataWidth(Int i, Bool visible)C
{
   Flt max_width=0;
   if(InRange(i, _columns))
   {
    C ListColumn &lc=_columns[i];
      if(GuiSkin *skin=getSkin())
         if(TextStyle *text_style=skin->list.text_style())
      {
         TextStyleParams ts=*text_style; ts.size=textSizeActual();
      #if DEFAULT_FONT_FROM_CUSTOM_SKIN
         if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
      #endif
         REP(visible ? visibleElms() : totalElms())
         {
            Ptr      data=(visible ? visToData(i) : absToData(i));
            Flt elm_width=ts.textWidth(lc.md.asText(data, lc.precision));
            if(_offset_offset>=0)elm_width+=*(Flt*)((Byte*)data+_offset_offset);
            MAX(max_width, elm_width);
         }
      }
   }
   return max_width;
}
Flt _List::columnDataWidthEx(Int i)C
{
   Flt w=columnDataWidth(i);
   if(InRange(i, _columns))
   {
    C ListColumn &lc=_columns[i];
      if(lc.visible())MAX(w, lc.textWidth(&_column_height)); // specify '_column_height' because 'ListColumn' height may not be set yet
      w+=0.02f;
   }
   return w;
}
Flt _List::columnWidthActual(Int i)C
{
   if(InRange(i, _columns))
   {
    C ListColumn &lc=_columns[i];
      if(lc.width<0) // special modes
      {
         if(Equal(lc.width, LCW_DATA           , 0.5f))return     columnDataWidthEx(i)                ;
         if(Equal(lc.width, LCW_PARENT         , 0.5f))return                           parentWidth() ;
         if(Equal(lc.width, LCW_MAX_DATA_PARENT, 0.5f))return Max(columnDataWidthEx(i), parentWidth());
         return 0;
      }
      return lc.width;
   }
   return 0;
}
_List& _List::columnWidth(Int i, Flt width)
{
   if(InRange(i, _columns))
   {
      ListColumn &lc=_columns[i];
      if(lc.width!=width){lc.width=width; setRects();}
   }
   return T;
}
_List& _List::columnVisible(Int i, Bool visible)
{
   if(InRange(i, _columns))
   {
      ListColumn &lc=_columns[i]; 
      if(lc.visible()!=visible){lc.visible(visible); setRects();}
   }
   return T;
}
Flt  _List::columnWidth  (Int i)C {return InRange(i, _columns) ? _columns[i].rect().w() :     0;}
Bool _List::columnVisible(Int i)C {return InRange(i, _columns) ? _columns[i].visible () : false;}
Flt  _List::columnOffset (Int i)C
{
   if(i>=1)
   {
      if(InRange(i, _columns      ))return _columns    [i].rect().min.x;
      if(           _columns.elms())return _columns.last().rect().max.x;
   }
   return 0;
}
Int _List::localToColumnX(Flt local_x)C
{
   REPA(_columns)
   {
    C ListColumn &lc=_columns[i]; if(lc.visible() && lc.rect().includesX(local_x))return i;
   }
   return -1;
}
/******************************************************************************/
Int _List::firstColumn(DATA_TYPE type)
{
   FREP(columns())if(column(i).md.type==type)return i;
   return -1;
}
Int _List::firstColumnText()
{
   FREP(columns())if(DataIsText(column(i).md.type))return i;
   return -1;
}
_List& _List::offsetAllColumns(Bool on) {_offset_first_column=!on; return T;}
/******************************************************************************/
// CHILDREN
/******************************************************************************/
  _List::Children::Children() {children.replaceClass<Child>();}
  _List::Children::Child* _List::Children::add       (GuiObj &child, GuiObj &parent) {return static_cast<Child*>(GuiObjChildren::add(child, parent));}
  _List::Children::Child& _List::Children::operator[](Int i)  {return SCAST(  Child, children[i]);}
C _List::Children::Child& _List::Children::operator[](Int i)C {return SCAST(C Child, children[i]);}

Vec2 _List::childOffset(C GuiObj &child)C
{
   Vec2 offset=0; Int index; if(_children.find(child, index))
   {
    C Children::Child &child=_children[index];
      Int vis=absToVis(child.abs_col.x);
      if( vis>=0)
      {
         offset=visToLocalPos(vis);
         if(drawMode()==LDM_LIST)offset.x+=columnOffset(child.abs_col.y);
      }
   }
   return offset;
}
/******************************************************************************/
// LIST
/******************************************************************************/
void _List::zero()
{
   sort_swap  [0]=sort_swap  [1]=sort_swap  [2]=false;
   sort_column[0]=sort_column[1]=sort_column[2]=-1;
   draw_column   =-1;

   cur=lit=-1;

    cur_mode=LCM_DEFAULT;
  _draw_mode=LDM_LIST;
    sel_mode=LSM_SET;
   flag=LIST_SORTABLE|LIST_SEARCHABLE;
   image_alpha=ALPHA_BLEND;
   zoom_min=Pow(1.2f, -3);
   zoom_max=Pow(1.2f,  2);
   padding.zero();

  _total_elms  =0;
  _visible_elms=0;
  _elm_size=0;
  _abs_to_vis =null;
  _abs_to_data=null;
  _vis_to_abs =null;
  _vis_to_data=null;
  _rects=null;
  _horizontal=false;
  _columns_hidden=false;
  _offset_first_column=true;
  _kb_action=false;

  _data=null;
  _memb=null;
  _memx=null;
  _meml=null;
  _map =null;
  _node=null;

  _search[0]=0;
  _search_i=0;
  _search_t=0;

  _tap_vis=-1;
  _tap_touch_id=0;

         _type_offset=-1;
         _desc_offset=-1;
  _image_color_offset=-1;
   _text_color_offset=-1;
  _text_shadow_offset=-1;
   _alpha_mode_offset=-1;
       _offset_offset=-1;
        _group_offset=-1;
     _children_offset= 0;

  _column_height=0.055f;
     _elm_height=0.050f;
    _text_base  =0.050f;
    _text_rel   =0;
   _image_base  =0;
   _image_rel   =0.1f/64;
   _image_padd  .zero();
   _zoom        =1;
   _height_ez=_elm_height*_zoom;

  _cur_changed_user=_sel_changed_user=_sel_changing_user=null;
  _cur_changed     =_sel_changed     =_sel_changing     =null;
}
_List::_List() {zero();}
_List& _List::del()
{
  _children.del  ();
  _columns .del  ();
   sel     .del  ();
  _skin    .clear();
   Free(_abs_to_vis );
   Free(_abs_to_data);
   Free(_vis_to_abs );
   Free(_vis_to_data);
   Free(_rects      );
   super::del(); zero(); return T;
}
_List& _List::clear(SET_MODE mode)
{
   Bool cur_changed=(cur       >=0 && mode!=QUIET),
        sel_changed=(sel.elms()> 0 && mode!=QUIET); // if had selection but now it's empty
   if(  sel_changed)callSelChanging();

  _children.del();
   Free(_abs_to_vis );
   Free(_abs_to_data);
   Free(_vis_to_abs );
   Free(_vis_to_data);
   Free(_rects      );

  _data=null;
  _memb=null;
  _memx=null;
  _meml=null;
  _map =null;
  _node=null;
  _total_elms=_visible_elms=_elm_size=0;
   cur=lit=-1; sel.clear();
   setRects();

   if(cur_changed)callCurChanged();
   if(sel_changed)callSelChanged();
   return T;
}
/******************************************************************************/
_List& _List::create()
{
   del();

  _type   =GO_LIST;
  _visible=true;
   return T;
}
_List& _List::create(C _List &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
        _children.del();
         copyParams(src);
        _type=GO_LIST;

         CopyFast(sort_swap  , src.sort_swap  );
         CopyFast(sort_column, src.sort_column);
         
         cur=src.cur;
         lit=src.lit;
         sel=src.sel;

           cur_mode =src.  cur_mode;
         _draw_mode =src._draw_mode;
           sel_mode =src.  sel_mode;
         flag       =src. flag;
         image_alpha=src. image_alpha;
         zoom_min   =src. zoom_min;
         zoom_max   =src. zoom_max;
         padding    =src. padding;
        _skin       =src._skin;

        _total_elms  =src._total_elms;
        _visible_elms=src._visible_elms;
        _elm_size    =src._elm_size;

         if(src._abs_to_vis )CopyFastN(Alloc(_abs_to_vis ,   totalElms()), src._abs_to_vis ,   totalElms());
         if(src._abs_to_data)CopyFastN(Alloc(_abs_to_data,   totalElms()), src._abs_to_data,   totalElms());
         if(src._vis_to_abs )CopyFastN(Alloc(_vis_to_abs , visibleElms()), src._vis_to_abs , visibleElms());
         if(src._vis_to_data)CopyFastN(Alloc(_vis_to_data, visibleElms()), src._vis_to_data, visibleElms());
         if(src._rects      )CopyFastN(Alloc(_rects      , visibleElms()), src._rects      , visibleElms());

        _offset_first_column=src._offset_first_column;
        _horizontal         =src._horizontal;
        _kb_action          =false;

        _data=src._data;
        _memb=src._memb;
        _memx=src._memx;
        _meml=src._meml;
        _map =src._map ;
        _node=src._node;

CopyFast(_search  ,src._search);
         _search_i=src._search_i;
         _search_t=src._search_t;

                _type_offset=src.       _type_offset;
                _desc_offset=src.       _desc_offset;
         _image_color_offset=src._image_color_offset;
          _text_color_offset=src. _text_color_offset;
         _text_shadow_offset=src._text_shadow_offset;
          _alpha_mode_offset=src. _alpha_mode_offset;
              _offset_offset=src.     _offset_offset;
               _group_offset=src.      _group_offset;
            _children_offset=src.   _children_offset;

         _column_height=src._column_height;
            _elm_height=src.   _elm_height;
           _text_base  =src.  _text_base;
           _text_rel   =src.  _text_rel;
          _image_base  =src. _image_base;
          _image_rel   =src. _image_rel;
          _image_padd  =src. _image_padd;
         _zoom         =src._zoom;
         _height_ez    =src._height_ez;

         _cur_changed_user =src._cur_changed_user;
         _cur_changed      =src._cur_changed;
         _sel_changed_user =src._sel_changed_user;
         _sel_changed      =src._sel_changed;
         _sel_changing_user=src._sel_changing_user;
         _sel_changing     =src._sel_changing;

         setColumns(src._columns.data(), src.columns(), src.columnsHidden()); // do this as last because it relies on other parameters
      }   
   }
   return T;
}
/******************************************************************************/
_List& _List::setColumns(C ListColumn *column, Int columns, Bool columns_hidden)
{
   T. draw_column   =-1;
   T._columns_hidden=columns_hidden;
   T._columns.clear().setNum(columns); FREPAO(T._columns).create(column[i], T);
   parentClientRectChanged(null, null);
   return T;
}
/******************************************************************************/
_List& _List::_setData(Ptr data, Int elms, Int elm_size, C MemPtr<Bool> &visible, Bool keep_cur)
{
   T._data    =data;
   T._memb    =null;
   T._memx    =null;
   T._meml    =null;
   T._map     =null;
   T._node    =null;
   T._elm_size=elm_size;

   init(elms, visible, keep_cur);
   return T;
}
_List& _List::setData(_Memc &memc, C MemPtr<Bool> &visible, Bool keep_cur) {return _setData(memc.data(), memc.elms(), memc.elmSize(), visible, keep_cur);}
_List& _List::setData(_Memb &memb, C MemPtr<Bool> &visible, Bool keep_cur)
{
   T._data    = null;
   T._memb    =&memb;
   T._memx    = null;
   T._meml    = null;
   T._map     = null;
   T._node    = null;
   T._elm_size= memb.elmSize();

   init(memb.elms(), visible, keep_cur);
   return T;
}
_List& _List::setData(_Memx &memx, C MemPtr<Bool> &visible, Bool keep_cur)
{
   T._data    = null;
   T._memb    = null;
   T._memx    =&memx;
   T._meml    = null;
   T._map     = null;
   T._node    = null;
   T._elm_size= memx.elmSize();

   init(memx.elms(), visible, keep_cur);
   return T;
}
_List& _List::setData(_Meml &meml, C MemPtr<Bool> &visible, Bool keep_cur)
{
   T._data    = null;
   T._memb    = null;
   T._memx    = null;
   T._meml    =&meml;
   T._map     = null;
   T._node    = null;
   T._elm_size= meml.elmSize();

   init(meml.elms(), visible, keep_cur);
   return T;
}
_List& _List::setData(_Map &map, C MemPtr<Bool> &visible, Bool keep_cur)
{
   T._data    = null;
   T._memb    = null;
   T._memx    = null;
   T._meml    = null;
   T._map     =&map ;
   T._node    = null;
   T._elm_size= map.dataSize();

   init(map.elms(), visible, keep_cur);
   return T;
}
_List& _List::_setData(_Memx &node, Int children_offset, C MemPtr<Bool> &visible, Bool keep_cur)
{
   T._data    = null;
   T._memb    = null;
   T._memx    = null;
   T._meml    = null;
   T._map     = null;
   T._node    =&node;
   T._elm_size= node.elmSize();
   T._children_offset=children_offset;

   init(NodeElms(node, children_offset), visible, keep_cur);
   return T;
}
/******************************************************************************/
void _List::init(Int elms, C MemPtr<Bool> &visible, Bool keep_cur)
{
 //Int vis_cur=cur; // remember visible cursor, in case we want to keep cursor as visible (not absolute)

   T._total_elms  =elms;
   T._visible_elms=(visible ? CountIs(visible) : elms);

   cur=visToAbs(cur); // convert to abs index because we're about to reset vis<->abs remap

   Free(_abs_to_data);
   Free(_vis_to_data);
   Free(_abs_to_vis ); Alloc(_abs_to_vis,   totalElms()); REP(totalElms())_abs_to_vis[i]=-1;
   Free(_vis_to_abs ); Alloc(_vis_to_abs, visibleElms()); Int v=0; FREPD(e, elms)if(!visible || ElmIs(visible, e)){_abs_to_vis[e]=v; _vis_to_abs[v]=e; v++;}
   Free(_rects      );
   if(_meml)
   {
      Alloc(_abs_to_data,   totalElms());
      Alloc(_vis_to_data, visibleElms());
      Int e=0, v=0; MFREP(*_meml){_abs_to_data[e]=i->data(); if(!visible || ElmIs(visible, e))_vis_to_data[v++]=i->data(); e++;}
   }
   if(_node)
   {
      Alloc(_abs_to_data,   totalElms());
      Alloc(_vis_to_data, visibleElms());
      Ptr *etd=_abs_to_data, *otd=_vis_to_data; // use copied pointers, because below variables are passed as references and increased inside
      Int index=0; NodeVisible(*_node, _children_offset, etd, otd, visible, index);
   }

   cur=absToVis(cur); // convert back to vis index

   sort                   ();
   parentClientRectChanged(null, null);

   if(cur_mode==LCM_MOUSE && !_kb_action && Gui.ms()==this)lit=cur=screenToVis(Ms.pos());else
   if(!keep_cur                                           )    cur=-1;                          
   if(!keep_cur)sel.clear();else REPA(sel)if(sel[i]>=totalElms())sel.remove(i);
}
/******************************************************************************/
void _List::setRects()
{
   Vec2 size=0;

   // process columns
   FREPA(_columns) // set all columns so we can access their horizontal offsets properly
   {
      ListColumn &lc=_columns[i];
      Flt next=size.x; if(_columns[i].visible())next+=columnWidthActual(i);
      lc.rect(Rect(size.x, -columnHeight(), next, 0));
      size.x=next;
   }

   // process children objects
   Flt children_width=0; REPA(_children)
   {
      Children::Child &child=_children[i];
      if(child.abs_col.y>=_columns.elms() && absToVis(child.abs_col.x)>=0)
         if(GuiObj *go=child.go)if(go->visible())MAX(children_width, GuiMaxX(go->rect()));
   }
   size.x+=children_width;

   // process elements
   if(drawMode()==LDM_LIST)
   {
      size.y=visibleElms()*_height_ez;
   }else
   if(drawMode()==LDM_RECTS)
   {
      if(!_rects)Alloc(_rects, visibleElms());

      Bool type_new_line=(FlagTest(flag, LIST_TYPE_SORT) && FlagTest(flag, LIST_TYPE_LINE) && _type_offset>=0);
      UInt type_prev    =0,
           type_cur     =0;
      Flt  x=0, y=0, W=0, H=0;
      Vec2 max_size=size;
      if(parent() && parent()->type()==GO_REGION)
      {
         Region &region=parent()->asRegion();
         MAX(max_size.x, region.rect().w()-region.slidebarSize());
         MAX(max_size.y, region.rect().h()-region.slidebarSize());
      }
      max_size+=EPS;
      FREPA(T)
      {
         Flt w=0, h=0;
         if(Ptr data=visToData(i))
         {
            w=imageSizeBase().x+_image_padd.w();
            h=imageSizeBase().y+_image_padd.h();
            if(InRange(draw_column, _columns))if(Image *image=_columns[draw_column].md.asImage(data))
            {
               w+=image->w()*imageSizeRel();
               h+=image->h()*imageSizeRel();
            }
            w*=zoom();
            h*=zoom();

            if(type_new_line)type_cur=*(UInt*)((Byte*)data+_type_offset);
            if(_horizontal)
            {
               if(y && ((type_new_line && type_cur!=type_prev) || -y+h>max_size.y)){y=0; x+=W; W=0;}
               MAX(W, w);
            }else
            {
               if(x && ((type_new_line && type_cur!=type_prev) || x+w>max_size.x)){x=0; y-=H; H=0;}
               MAX(H, h);
            }
            type_prev=type_cur;
         }
         Flt max_x=x+w,
             min_y=y-h;
        _rects[i].set(x, min_y, max_x, y);
         MAX(size.x,  max_x);
         MAX(size.y, -min_y);
         if(_horizontal)y-=h;else x+=w;
      }
   }
   if(columnsVisible())size.y+=columnHeight();
   size+=padding;

   T.size(size);

   if(_parent)switch(_parent->type())
   {
    /*case GO_REGION:
      {
         Region &region=_parent->asRegion();
         Flt add=-_height_ez*0.5f;
         region.slidebar[0]._scroll_add=add;
         region.slidebar[1]._scroll_add=add;
      }break;*/

      case GO_MENU:
      {
        _parent->asMenu().clientSize(size);
      }break;
   }
}
void _List::parentClientRectChanged(C Rect *old_client, C Rect *new_client) {setRects();}
/******************************************************************************/
_List& _List::skin(C GuiSkinPtr &skin)
{
  _skin=skin;
   REPAO(_columns).skin=skin;
   return T;
}
TextStyle* _List::getTextStyle()C
{
   if(GuiSkin *skin=getSkin())return skin->list.text_style();
   return null;
}
Flt _List::textSizeActual()C {return _height_ez*textSizeRel()+textSizeBase();}

_List& _List::columnsHidden(  Bool  hidden                             ) {                                             if(T._columns_hidden!=hidden                                              ){T._columns_hidden=hidden    ;                                               setRects();} return T;}
_List& _List:: columnHeight(  Flt   height                             ) {MAX(height, 0);                              if(T. _column_height!=height                                              ){T. _column_height=height    ;                                               setRects();} return T;}
_List& _List::    elmHeight(  Flt   height                             ) {MAX(height, 0);                              if(T.    _elm_height!=height                                              ){T.    _elm_height=height    ;   _height_ez=_elm_height*_zoom;               setRects();} return T;}
_List& _List::     textSize(  Flt   base, Flt relative                 ) {MAX(base, 0);              MAX(relative, 0); if(T. _text_base!=base || T. _text_rel!=relative                          ){T.     _text_base=base      ; T. _text_rel=relative;                        setRects();} return T;}
_List& _List::    imageSize(C Vec2 &base, Flt relative, C Rect &padding) {Vec2 b=Max(base, Vec2(0)); MAX(relative, 0); if(T._image_base!=base || T._image_rel!=relative || T._image_padd!=padding){T.    _image_base=base      ; T._image_rel=relative; T._image_padd=padding; setRects();} return T;}
_List& _List::         zoom(  Flt   zoom                               ) {Clamp(zoom, zoom_min, zoom_max);             if(T._zoom!=zoom                                                          ){T.          _zoom=zoom      ;   _height_ez=_elm_height*_zoom;               setRects();} return T;}
_List& _List::     drawMode(  LIST_DRAW_MODE mode                      ) {                                             if(T._draw_mode!=mode                                                     ){T.     _draw_mode=mode      ;                                               setRects();} return T;}
_List& _List::   horizontal(  Bool           horizontal                ) {                                             if(T._horizontal!=horizontal                                              ){T.    _horizontal=horizontal;                                               setRects();} return T;}
_List& _List::     vertical(  Bool           vertical                  ) {return horizontal(!vertical);}
/******************************************************************************/
Ptr _List::visToData(Int visible)C
{
   if(InRange(visible, visibleElms()))
   {
      if(_vis_to_data)return _vis_to_data[visible];
      return absToData(visToAbs(visible));
   }
   return null;
}
Int _List::visToAbs(Int visible)C
{
   if(InRange(visible, visibleElms()) && _vis_to_abs)return _vis_to_abs[visible];
   return -1;
}
Int _List::absToVis(Int absolute)C
{
   if(InRange(absolute, totalElms()) && _abs_to_vis)return _abs_to_vis[absolute];
   return -1;
}
Ptr _List::absToData(Int absolute)C
{
   if(InRange(absolute, totalElms()))
   {
      if(_abs_to_data)return _abs_to_data[absolute];
      if(_data)return (Byte*)_data + absolute*_elm_size;
      if(_memb)return (*_memb)[absolute];
      if(_memx)return (*_memx)[absolute];
      if(_meml)return (*_meml)[absolute];
      if(_map )return (*_map )[absolute];
      if(_node)return NodeAbsToData(*_node, _children_offset, absolute);
   }
   return null;
}
Int _List::dataToVis(CPtr data)C
{
   if(data)REP(visibleElms())if(visToData(i)==data)return i;
   return -1;
}
Int _List::dataToAbs(CPtr data)C
{
   if(data)REP(totalElms())if(absToData(i)==data)return i;
   return -1;
}
/******************************************************************************/
static Int CompareRectX(C Rect &rect, C Flt &x) {return Compare(rect.min.x, x);}
static Int CompareRectY(C Rect &rect, C Flt &y) {return Compare(y, rect.max.y);}

Int _List::localToVirtualX(Flt local_x)C
{
   switch(drawMode())
   {
      case LDM_RECTS: if(_rects && local_x>=0) // if "local_x<0" then we want -1, so just skip to the end
      {
         if(local_x>rect().max.x)return elms(); // out of range
         Int i; if(!BinarySearch(_rects, elms(), local_x, i, CompareRectX))i--; // if didn't found an exact match, then we need to get the previous one, and not the next one, this will make 'i' always in range (-1..elms-1) inclusive
       //if(InRange(i, T)) no need to check this, because we're checking below "InRange(i-1, T)" and in this case if i-1 is in range, then 'i' is in range too
            for(; InRange(i-1, T) && Equal(_rects[i-1].min.x, _rects[i].min.x); i--); // if multiple elements have the same rect.min.x then get the first one
         return i;
      }break;
   }
   return -1;
}
Int _List::localToVirtualY(Flt local_y)C
{
   if(columnsVisible())local_y+=columnHeight();
   switch(drawMode())
   {
      case LDM_LIST : return Floor(-local_y/_height_ez);
      case LDM_RECTS: if(_rects && local_y<=0) // if "local_y>0" then we want -1, so just skip to the end
      {
         if(local_y<rect().min.y)return elms(); // out of range
         Int i; if(!BinarySearch(_rects, elms(), local_y, i, CompareRectY))i--; // if didn't found an exact match, then we need to get the previous one, and not the next one, this will make 'i' always in range (-1..elms-1) inclusive
       //if(InRange(i, T)) no need to check this, because we're checking below "InRange(i-1, T)" and in this case if i-1 is in range, then 'i' is in range too
            for(; InRange(i-1, T) && Equal(_rects[i-1].max.y, _rects[i].max.y); i--); // if multiple elements have the same rect.max.y then get the first one
         return i;
      }break;
   }
   return -1;
}
Int _List::localToVis(C Vec2 &local_pos)C
{
   if(drawMode()==LDM_RECTS)
   {
      if(_rects)
      {
         Vec2 pos=local_pos; if(columnsVisible())pos.y+=columnHeight();
         if(pos.y<=0 && pos.y>=rect().min.y
         && pos.x>=0 && pos.x<=rect().max.x)
         {
            if(_horizontal)
            {
               for(Int i=Max(0, localToVirtualX(local_pos.x)); i<elms(); i++){C Rect &r=_rects[i]; if(r.min.x>pos.x)break; if(Cuts(pos, r))return i;}
            }else
            {
               for(Int i=Max(0, localToVirtualY(local_pos.y)); i<elms(); i++){C Rect &r=_rects[i]; if(r.max.y<pos.y)break; if(Cuts(pos, r))return i;}
            }
         }
      }
      return -1;
   }
   return /*_horizontal ? localToVisX(local_pos.x) : */localToVisY(local_pos.y); // '_horizontal' affects only LDM_RECTS which we've already checked above, other modes always use Y
}

Int _List::localToVisX(Flt local_x)C {Int v=localToVirtualX(local_x); return InRange(v, visibleElms()) ? v : -1;}
Int _List::localToVisY(Flt local_y)C {Int v=localToVirtualY(local_y); return InRange(v, visibleElms()) ? v : -1;}

Int _List::screenToVisX    (  Flt   x  , C GuiPC *gpc)C {return localToVisX    (x  -(gpc ? gpc->offset.x : screenPos().x));}
Int _List::screenToVisY    (  Flt   y  , C GuiPC *gpc)C {return localToVisY    (y  -(gpc ? gpc->offset.y : screenPos().y));}
Int _List::screenToVis     (C Vec2 &pos, C GuiPC *gpc)C {return localToVis     (pos-(gpc ? gpc->offset   : screenPos()  ));}
Int _List::screenToVirtualY(  Flt   y  , C GuiPC *gpc)C {return localToVirtualY(y  -(gpc ? gpc->offset.y : screenPos().y));}
Int _List::screenToColumnX (  Flt   x  , C GuiPC *gpc)C {return localToColumnX (x  -(gpc ? gpc->offset.x : screenPos().x));}

Ptr _List::screenToData(  Flt   y  , C GuiPC *gpc)C {return visToData(screenToVisY(y  , gpc));}
Ptr _List::screenToData(C Vec2 &pos, C GuiPC *gpc)C {return visToData(screenToVis (pos, gpc));}

Flt _List::visToLocalY(Int visible)C
{
   Flt y=(columnsVisible() ? -columnHeight() : 0);
   switch(drawMode())
   {
      case LDM_LIST : y-=visible*_height_ez; break;
      case LDM_RECTS: if(_rects && InRange(visible, visibleElms()))y+=_rects[visible].max.y; break;
   }
   return y;
}
Vec2 _List::visToLocalPos(Int visible)C
{
   Vec2 pos(0, columnsVisible() ? -columnHeight() : 0);
   switch(drawMode())
   {
      case LDM_LIST : pos.y-=visible*_height_ez; break;
      case LDM_RECTS: if(_rects && InRange(visible, visibleElms()))pos+=_rects[visible].lu(); break;
   }
   return pos;
}
Rect _List::visToLocalRect(Int visible)C
{
   Vec2 pos(0, columnsVisible() ? -columnHeight() : 0);
   switch(drawMode())
   {
      case LDM_LIST : pos.y-=visible*_height_ez; return Rect_LU(pos, rect().w(), _height_ez);
      case LDM_RECTS: if(_rects && InRange(visible, visibleElms()))return _rects[visible]+pos; break;
   }
   return pos;
}

Vec2 _List::visToScreenPos(Int visible, C GuiPC *gpc)C
{
   Vec2 pos=(gpc ? gpc->offset : screenPos()); if(columnsVisible())pos.y-=columnHeight();
   switch(drawMode())
   {
      case LDM_LIST : pos.y-=visible*_height_ez; break;
      case LDM_RECTS: if(_rects && InRange(visible, visibleElms()))pos+=_rects[visible].lu(); break;
   }
   return pos;
}
Rect _List::visToScreenRect(Int visible, C GuiPC *gpc)C
{
   Vec2 pos=(gpc ? gpc->offset : screenPos()); if(columnsVisible())pos.y-=columnHeight();
   switch(drawMode())
   {
      case LDM_LIST : pos.y-=visible*_height_ez; return Rect_LU(pos, rect().w(), _height_ez);
      case LDM_RECTS: if(_rects && InRange(visible, visibleElms()))return _rects[visible]+pos; break;
   }
   return pos;
}
/******************************************************************************/
Rect _List::elmsScreenRect(C GuiPC *gpc)C
{
   Rect clip     =(gpc ? gpc->clip        : D.rect());
   Rect elms_rect=(gpc ? gpc->client_rect : parent() ? parent()->screenClientRect() : clip); if(columnsVisible())elms_rect.max.y-=columnHeight(); elms_rect&=clip;
   return elms_rect;
}
VecI2 _List::visibleElmsOnScreen(C GuiPC *gpc)C
{
   if(visibleElms())
      if(gpc ? visible() && gpc->visible : visibleFull())
   {
      Vec2  offset   =(gpc ? gpc->offset : screenPos());
      Rect  elms_rect=elmsScreenRect(gpc);
      VecI2 range;
      if(_horizontal && drawMode()==LDM_RECTS)
      {
         range.set(Max(0              , localToVirtualX(elms_rect.min.x-offset.x)),  // clip start only using Max to allow start>end (invalid range)
                   Min(visibleElms()-1, localToVirtualX(elms_rect.max.x-offset.x))); // clip end   only using Min to allow start>end (invalid range)
         if(drawMode()==LDM_RECTS)for(; InRange(range.y+1, visibleElms()) && _rects[range.y+1].min.x+offset.x<elms_rect.max.x; range.y++);
      }else
      {
         range.set(Max(0              , localToVirtualY(elms_rect.max.y-offset.y)),  // clip start only using Max to allow start>end (invalid range)
                   Min(visibleElms()-1, localToVirtualY(elms_rect.min.y-offset.y))); // clip end   only using Min to allow start>end (invalid range)
         if(drawMode()==LDM_RECTS)for(; InRange(range.y+1, visibleElms()) && _rects[range.y+1].max.y+offset.y>elms_rect.min.y; range.y++);
      }
      return range;
   }
   return VecI2(0, -1);
}
Int _List::pageElms(C GuiPC *gpc)C
{
   Int page_elms=1;
   if(drawMode()==LDM_LIST)
   {
      Rect elms_rect=elmsScreenRect(gpc);
      MAX(page_elms, Trunc(elms_rect.h()/_height_ez-0.5f));
   }
   return page_elms;
}
/******************************************************************************/
_List& _List::scrollTo(Int i, Bool immediate, Flt center)
{
   Clamp(i, 0, elms());
   if(InRange(i, T) && _parent && _parent->type()==GO_REGION)
   {
      Region &region=_parent->asRegion();
      Flt     add   =(columnsVisible() ? columnHeight() : 0), h=region.clientHeight()-add, e;
      center=Sat(center)*0.5f;
      switch(drawMode())
      {
         case LDM_LIST:
         {
            Flt pos= i*_height_ez;
                e  =(h-_height_ez)*center;
            region.scrollFitY(pos-e, pos+_height_ez+add+e, immediate);
         }break;

         case LDM_RECTS: if(_rects)
         {
          C Rect &rect=_rects[i];
            if(_horizontal){e=(region.clientWidth()-rect.w())*center; region.scrollFitX( rect.min.x-e,  rect.max.x    +e, immediate);}
            else           {e=(             h      -rect.h())*center; region.scrollFitY(-rect.max.y-e, -rect.min.y+add+e, immediate);}
         }break;
      }
   }
   return T;
}
/******************************************************************************/
static struct ListComparer
{
   Int         elm_size, type_offset;
   Ptr        *abs_to_data;
   Byte       *data;
  _Memb       *memb;
  _Memx       *memx;
  _Map        *map;
   Bool        swap[3], type_sort;
   MemberDesc *md[3];
   SyncLock    lock;
}LCmp;

static inline Int ListCompareData(CPtr d0, CPtr d1, Int i0, Int i1)
{
   if(LCmp.type_sort) // sort by type first
   {
      UInt t0=*(UInt*)((Byte*)d0+LCmp.type_offset),
           t1=*(UInt*)((Byte*)d1+LCmp.type_offset);
      if(t0<t1)return -1;
      if(t0>t1)return +1;
   }
   if(LCmp.md[0])if(Int c=LCmp.md[0]->compare(d0, d1))return LCmp.swap[0] ? -c : c;
   if(LCmp.md[1])if(Int c=LCmp.md[1]->compare(d0, d1))return LCmp.swap[1] ? -c : c;
   if(LCmp.md[2])if(Int c=LCmp.md[2]->compare(d0, d1))return LCmp.swap[2] ? -c : c;
 //return Compare(d0, d1); // compare by memory address to avoid randomizing element visible order (this can happen because sorting algorithm may not preserve order for equal elements)
   return Compare(i0, i1); // compare by index          to avoid randomizing element visible order (this can happen because sorting algorithm may not preserve order for equal elements)
}
static Int ListComparePtr(C Int &i0, C Int &i1)
{
   Ptr d0=LCmp.abs_to_data[i0],
       d1=LCmp.abs_to_data[i1];
   return ListCompareData(d0, d1, i0, i1);
}
static Int ListCompareContinuous(C Int &i0, C Int &i1)
{
   Ptr d0=LCmp.data+i0*LCmp.elm_size,
       d1=LCmp.data+i1*LCmp.elm_size;
   return ListCompareData(d0, d1, i0, i1);
}
static Int ListCompareMemb(C Int &i0, C Int &i1)
{
   Ptr d0=(*LCmp.memb)[i0],
       d1=(*LCmp.memb)[i1];
   return ListCompareData(d0, d1, i0, i1);
}
static Int ListCompareMemx(C Int &i0, C Int &i1)
{
   Ptr d0=(*LCmp.memx)[i0],
       d1=(*LCmp.memx)[i1];
   return ListCompareData(d0, d1, i0, i1);
}
static Int ListCompareMap(C Int &i0, C Int &i1)
{
   Ptr d0=(*LCmp.map)[i0],
       d1=(*LCmp.map)[i1];
   return ListCompareData(d0, d1, i0, i1);
}
void _List::sort()
{
   Bool type_sort=(FlagTest(flag, LIST_TYPE_SORT) && _type_offset>=0);
   if(  type_sort || sort_column[0]>=0 || sort_column[1]>=0 || sort_column[2]>=0)
   {
      cur=visToAbs(cur);

      { // use synchronization lock because we're operating on one global 'LCmp' object, because 'Sort' does not accept user pointer parameters
         SyncLocker locker(LCmp.lock);
         REPA(sort_column)
         {
            Int          c=sort_column[i];
            ListColumn *lc=(InRange(c, _columns) ? &_columns[c] : null);
            LCmp.md  [i]=(lc ? (lc->sort ? lc->sort : &lc->md) : null);
            LCmp.swap[i]=sort_swap[i];
         }
         LCmp.elm_size   =_elm_size;
         LCmp.type_sort  = type_sort;
         LCmp.type_offset=_type_offset;
         LCmp.abs_to_data=_abs_to_data;
         LCmp.data       =(Byte*)_data;
         LCmp.memb       =_memb;
         LCmp.memx       =_memx;
         LCmp.map        =_map ;

         Int (&compare)(C Int &i0, C Int &i1)=(_abs_to_data ? ListComparePtr : _data ? ListCompareContinuous : _memb ? ListCompareMemb : _memx ? ListCompareMemx : ListCompareMap);
         Sort(_vis_to_abs, visibleElms(), compare);
      }

      REPD(v, visibleElms())
      {
         Int    absolute=_vis_to_abs [v       ];
                         _abs_to_vis [absolute]=v;
         if(_vis_to_data)_vis_to_data[v       ]=absToData(absolute);
      }

      cur=absToVis(cur);
   }
}
_List& _List::sort(Int column, Int swap)
{
   if(InRange(column, _columns))
   {
      // set sorting parameters
      if(column==sort_column[0])
      {
         Bool new_swap=((swap<0) ? sort_swap[0]^1 : (swap!=0)); // if swap is not specified (<0) then toggle current mode, otherwise set as specified
         if( sort_swap[0]==new_swap)return T; // if swap is the same, then do nothing
             sort_swap[0] =new_swap;
      }else
      if(column==sort_column[1])
      {
         Swap(sort_column[0], sort_column[1]);
         Swap(sort_swap  [0], sort_swap  [1]);
         if(swap>=0)sort_swap[0]=(swap!=0);
      }else
      if(column==sort_column[2])
      {
         if(swap<0)swap=sort_swap[2];
         sort_column[2]=sort_column[1]; sort_swap[2]=sort_swap[1];
         sort_column[1]=sort_column[0]; sort_swap[1]=sort_swap[0];
         sort_column[0]=     column   ; sort_swap[0]=    (swap!=0);
      }else
      {
         sort_column[2]=sort_column[1]; sort_swap[2]=sort_swap[1];
         sort_column[1]=sort_column[0]; sort_swap[1]=sort_swap[0];
         sort_column[0]=     column   ; sort_swap[0]=((swap<0) ? false : (swap!=0));
      }

      // sort
      if(sorting()) // call after 'sort_column' and 'sort_swap' have been changed, so the callback can verify latest sorting parameters
      {
         sort();
         if(drawMode()==LDM_RECTS)setRects();
      }
   }
   return T;
}
LIST_SEL_MODE _List::selMode()C
{
   if(Kb.alt    ())return Kb.shift() ? LSM_EXCLUDE_MULTI : LSM_EXCLUDE;
   if(Kb.shift  ())return LSM_INCLUDE;
   if(Kb.ctrlCmd())return LSM_TOGGLE ;
   return sel_mode;
}
void _List::childRectChanged(C Rect *old_rect, C Rect *new_rect, GuiObj &child)
{
   // null rectangle means object is hidden
   if(old_rect || new_rect)
   {
      Flt     x=(new_rect ? GuiMaxX(*new_rect) : 0),
          old_x=(old_rect ? GuiMaxX(*old_rect) : 0),
          ofs_x=0; if(_columns.elms())ofs_x+=_columns.last().rect().max.x; ofs_x+=padding.x;
          x+=ofs_x;
      old_x+=ofs_x;
      Int index; if(_children.find(child, index)) // if child belongs to List
      {
       C Children::Child &c=_children[index]; if(c.abs_col.y>=_columns.elms() && absToVis(c.abs_col.x)>=0) // process only if it's attached to the last column
         {
            if(old_x>=rect().w()-EPS && x<rect().w()-EPS)setRects();else // if decreasing then we need to recalculate fully
            if(                         x>rect().w()+EPS)size(Vec2(x, rect().h()));
         }
      }else // assume that it was removed and belonged to last column
      {
         if(old_x>=rect().w()-EPS && x<rect().w()-EPS)setRects(); // if decreasing then we need to recalculate fully
      }
   }//else having null rectangles means object is hidden and we don't need to do anything
}
_List& _List::addChild(GuiObj &child, Int abs, Int column)
{
   Bool same_parent=(child.parent()==this);
   if(Children::Child *c=_children.add(child, T)) // add to the end
   {
      if(same_parent && c->abs_col.x==abs && c->abs_col.y==column)return T; // if child had the same parent and we're not changing indexes, then we're done

      c->abs_col.set(abs, column); // setup abs column
     _children.validateLevel(child); // validate after setting abs and column

      if(column>=_columns.elms() && absToVis(abs)>=0 && child.visible())
      {
         Flt width=GuiMaxX(child.rect()); MAX(width, 0); if(_columns.elms())width+=_columns.last().rect().max.x; width+=padding.x;
         if(width>rect().w())size(Vec2(width, rect().h()));
      }
   }
   return T;
}
void _List::removeChild(GuiObj &child)
{
   if(_children.remove(child))childRectChanged(child.visible() ? &child.rect() : null, null, child);
}
/******************************************************************************/
_List& _List::curChanged(void (*func)(Ptr), Ptr user)
{
   T._cur_changed     =func;
   T._cur_changed_user=user;
   return T;
}
_List& _List::selChanged(void (*func)(Ptr), Ptr user)
{
   T._sel_changed     =func;
   T._sel_changed_user=user;
   return T;
}
_List& _List::selChanging(void (*func)(Ptr), Ptr user)
{
   T._sel_changing     =func;
   T._sel_changing_user=user;
   return T;
}
void _List::callCurChanged()
{
   Gui.addFuncCall(_cur_changed, _cur_changed_user);
}
void _List::callSelChanged()
{
   Gui.addFuncCall(_sel_changed, _sel_changed_user);
}
void _List::callSelChanging()
{
   if(_sel_changing)_sel_changing(_sel_changing_user);
}
/******************************************************************************/
_List& _List::clearElmType      (                  ) {       _type_offset=-1                                    ; return T;}
_List& _List::clearElmDesc      (                  ) {       _desc_offset=-1                                    ; return T;}
_List& _List::clearElmImageColor(                  ) {_image_color_offset=-1                                    ; return T;}
_List& _List::clearElmTextColor (                  ) { _text_color_offset=-1                                    ; return T;}
_List& _List::clearElmTextShadow(                  ) {_text_shadow_offset=-1                                    ; return T;}
_List& _List::clearElmAlphaMode (                  ) { _alpha_mode_offset=-1                                    ; return T;}
_List& _List::clearElmOffset    (                  ) {     _offset_offset=-1                                    ; return T;}
_List& _List::clearElmGroup     (                  ) {      _group_offset=-1                                    ; return T;}
_List& _List::  setElmType      (UInt       &member) {       _type_offset=UInt(UIntPtr(&member))                ; return T;}
_List& _List::  setElmDesc      (CChar*     &member) {       _desc_offset=UInt(UIntPtr(&member))                ; return T;}
_List& _List::  setElmDesc      (Str        &member) {       _desc_offset=UInt(UIntPtr(&member))+OFFSET(Str, _d); return T;}
_List& _List::  setElmImageColor(Color      &member) {_image_color_offset=UInt(UIntPtr(&member))                ; return T;}
_List& _List::  setElmTextColor (Color      &member) { _text_color_offset=UInt(UIntPtr(&member))                ; return T;}
_List& _List::  setElmTextShadow(Byte       &member) {_text_shadow_offset=UInt(UIntPtr(&member))                ; return T;}
_List& _List::  setElmAlphaMode (ALPHA_MODE &member) { _alpha_mode_offset=UInt(UIntPtr(&member))                ; return T;}
_List& _List::  setElmOffset    (Flt        &member) {     _offset_offset=UInt(UIntPtr(&member))                ; return T;}
_List& _List::  setElmGroup     (Str        &member) {      _group_offset=UInt(UIntPtr(&member))                ; return T;}
/******************************************************************************/
GuiObj* _List::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(visible() && gpc.visible && Cuts(pos, gpc.clip))
   {
      if(Kb.ctrlCmd() && (flag&LIST_SCALABLE))mouse_wheel=this;
      GuiPC gpc_col(gpc, T); REP(columns())if(GuiObj *go=column(i).test(gpc_col, pos, mouse_wheel))return go;
      if(_children.children.elms())
      {
         VecI2 visible_range=visibleElmsOnScreen(&gpc); if(visible_range.y>=visible_range.x)
         {
            GuiPC gpc2(gpc, visible(), enabled()); Vec2 offset=gpc2.offset;
            if(columnsVisible())
            {
               Flt max_y=gpc.client_rect.max.y-columnHeight();
               MIN(gpc2.clip.max.y, max_y);
            }
            REPA(_children) // 'test' must be done from the end
            {
               Children::Child &child=_children[i];
               if(SetGPC(T, gpc2, offset, child.abs_col, visible_range))if(GuiObj *go=child.go->test(gpc2, pos, mouse_wheel))return go;
            }
         }
      }
      return this;
   }
   return null;
}
/******************************************************************************/
Bool _List::setSel(Int visible) // returns if selection has changed, this may call ONLY 'selChanging', but NOT 'selChanged', 'curChanged'
{
   if((flag&LIST_MULTI_SEL) && InRange(visible, visibleElms())) // set only 'visible'
   {
      Int abs=visToAbs(visible);
      if(sel.elms()!=1 || sel[0]!=abs)
      {
         callSelChanging();
         sel.setNum(1)[0]=abs;
         return true;
      }
   }else
   if(sel.elms()) // clear
   {
      callSelChanging();
      sel.clear();
      return true;
   }
   return false;
}
_List& _List::setCur(Int i)
{
   // first set what's changing
   Bool cur_changed=(T.cur!=i),
        sel_changed=setSel(i);

   // make the change
   T.cur=i;

   // call after making changes
   if(cur_changed)callCurChanged();
   if(sel_changed)callSelChanged();
   return T;
}
_List& _List::processSel(Int absolute, Int sel_mode) // this may call ONLY 'selChanging', 'selChanged', but NOT 'curChanged'
{
   if(flag&LIST_MULTI_SEL)
   {
      Bool sel_changed=false;
      if(sel_mode<0)sel_mode=selMode();
      if(!InRange(absolute, totalElms()))absolute=-1;

      if(sel_mode==LSM_SET)
      {
         if(absolute>=0) // set only 'absolute'
         {
            if(sel.elms()!=1 || sel[0]!=absolute){callSelChanging(); sel.setNum(1)[0]=absolute; sel_changed=true;}
         }else
         {
            if(sel.elms()){callSelChanging(); sel.clear(); sel_changed=true;}
         }
      }else
      if(absolute>=0)switch(sel_mode)
      {
         case LSM_TOGGLE       :                                        callSelChanging();    sel. toggle(absolute, true); sel_changed=true; break;
         case LSM_INCLUDE      : if(_sel_changing && !sel.has(absolute))callSelChanging(); if(sel.include(absolute      ) )sel_changed=true; break;

         case LSM_EXCLUDE      :
         case LSM_EXCLUDE_MULTI: if(_sel_changing &&  sel.has(absolute))callSelChanging(); if(sel.exclude(absolute, true) )sel_changed=true; break;
      }

      if(sel_changed)callSelChanged();
   }
   return T;
}
Bool _List::setCurEx(Int cur, Int dir, Bool pushed, UInt touch_id) // returns if selection has changed, this may call ONLY 'selChanging', but NOT 'selChanged', 'curChanged'
{
   Bool sel_changed=false;
   if(flag&LIST_MULTI_SEL)
   {
      if(_group_offset>=0)
         if(Ptr data=visToData(cur))
            if(((Str*)((Byte*)data+_group_offset))->is())goto skip_sel_change; // do not modify 'sel' because we want to click on a group, to open it, to show its elements

      switch(selMode())
      {
         case LSM_SET:
         {
            if(touch_id && !pushed){lit=cur; return false;} // if it's a touch and it's not pushed then set 'lit' only and don't do anything on the cursor and selection
            if(pushed && sel.has(visToAbs(cur))){_tap_vis=cur; _tap_touch_id=touch_id;} // if we're activating using button push, and the element is already selected, then there's possibility we want to drag and drop the elements, so instead of activating that element, we will store its index to temporary member, and activate it only quick release of the button (tap)
               else sel_changed|=setSel(cur);
         }break;

         case LSM_TOGGLE:
         {
            if(!pushed)return false; // don't do anything if it's TOGGLE mode and button was not pushed
            if(InRange(cur, visibleElms())){Int abs=visToAbs(cur); callSelChanging(); sel_changed=true; sel.toggle(abs, true);}
         }break;

         case LSM_INCLUDE:
         {
            if(InRange(cur, visibleElms()))
            {
               if(InRange(T.cur, visibleElms()))
               {
                  if(!dir)dir=((cur>T.cur) ? +1 : -1);else Clamp(dir, -1, +1);
                  for(Bool called=false; ; T.cur=Mod(T.cur+dir, visibleElms()))
                  {
                     Int abs=visToAbs(T.cur);
                     if(_sel_changing && !called && !sel.has(abs)){called=true; callSelChanging();}
                     sel_changed|=sel.include(abs);
                     if(T.cur==cur)break;
                  }
               }else
               {
                  Int abs=visToAbs(cur);
                  if(_sel_changing && !sel.has(abs))callSelChanging();
                  sel_changed|=sel.include(abs);
               }
            }
         }break;

         case LSM_EXCLUDE:
         {
            if(InRange(cur, visibleElms())){exclude_single: Int abs=visToAbs(cur); if(_sel_changing && sel.has(abs))callSelChanging(); sel_changed|=sel.exclude(abs, true);} 
         }break;

         case LSM_EXCLUDE_MULTI:
         {
            if(InRange(cur, visibleElms()))
            {
               if(InRange(T.cur, visibleElms()))
               {
                  if(!dir)dir=((cur>T.cur) ? +1 : -1);else Clamp(dir, -1, +1);
                  for(Bool called=false; ; T.cur=Mod(T.cur+dir, visibleElms()))
                  {
                     Int abs=visToAbs(T.cur);
                     if(_sel_changing && !called && sel.has(abs)){called=true; callSelChanging();}
                     sel_changed|=sel.exclude(abs, true);
                     if(T.cur==cur)break;
                  }
               }else goto exclude_single;
            }
         }break;
      }
   }else sel_changed|=setSel(-1);

skip_sel_change:
   T.cur=cur;
   return sel_changed;
}
void _List::update(C GuiPC &gpc)
{
   lit=-1;

   if(disabled() || gpc.disabled())
   {
      if(cur_mode==LCM_MOUSE)setCur(-1);
   }else
   if(visible() && gpc.visible && visibleElms())
   {
      if(Gui.wheel()==this)
      {
         if(Kb.ctrlCmd() && Ms.wheel() && (flag&LIST_SCALABLE))
         {
            zoom(zoom()*ScaleFactor(0.2f*Ms.wheel()));
            Ms.eatWheel();
         }
      }

      Bool sel_changed=false;
      Int  cur_prev=cur;

     _kb_action&=!Ms._action; // disable keyboard action if there's a mouse action

      // mouse highlight
      if(Gui.msLit()==this &&
           (Gui.ms()==this || Gui.dragging()) // allow highlight when mouse original focus is at the list or when dragging
      )lit=screenToVis(Ms.pos(), &gpc);

      // touch
      REPA(Touches)
      {
         Touch &touch=Touches[i]; if(touch.guiObj()==this && (touch.state()&(BS_PUSHED|BS_ON|BS_RELEASED))) // process cursor for (pushed to set) and (on+released to skip from mouse), release needed so 'Menu' detection can work
         {
            GuiObj *container=this; if(parent()->is(GO_MENU))container=parent(); if(container->contains(Gui.objAtPos(touch.pos())))
            {
               if( touch.pd() // set cursor only when pushed
               || !touch.scrolling() // or not scrolling, to avoid situations when scrolling could possibly change the cursor, but allow for 'Menu'
               )sel_changed|=setCurEx(screenToVis(touch.pos(), &gpc), 0, touch.pd(), touch.id());
               goto skip_mt; // skip processing cursor by mouse touches
            }
         }
      }

      // mouse cursor
      if(Gui.ms()==this)
      {
         if(Ms.bp(0) || (cur_mode==LCM_MOUSE && !_kb_action))sel_changed|=setCurEx(lit, 0, Ms.bp(0));
      }else
      {
         if(cur_mode==LCM_MOUSE && !_kb_action)sel_changed|=setSel(cur=-1);
      }
   skip_mt:

      Int cur_prev2=cur;

      // select on tap
      if(_tap_vis>=0)
      {
         Touch *touch=FindTouch(_tap_touch_id);
         if(_tap_touch_id ? (!touch || touch->rs()) : !Ms.b(0))
         {
            if(InRange(_tap_vis, T))if(_tap_touch_id ? (touch && touch->tapped()) : Ms.tapped(0))sel_changed|=setCurEx(_tap_vis);
           _tap_vis=-1;
         }
      }

      // keyboard
      if(Gui.kb()==this)
      {
         if(Kb.k.ctrlCmd())
         {
            // select all (Ctrl+A)
            if(Kb.k(KB_A) && !Kb.k.shift() && !Kb.k.alt() && (flag&LIST_MULTI_SEL))
            {
               if(Kb.k.first())
               {
                                     REPA(sel          )if(               absToVis(sel[i])<0        )goto different; // if element is selected but not visible
                  sel.sort(Compare); REP (visibleElms())if(!sel.binaryHas(visToAbs(    i ), Compare))goto different; // if element is visible  but not selected (sort first)
                  if(0)
                  {
                  different:
                     callSelChanging();
                     sel.clear(); FREP(visibleElms())sel.add(visToAbs(i));
                     sel_changed=true;
                  }
               }
               Kb.eatKey();
            }
         }else
         {
            if(Kb.k(KB_HOME)){_kb_action=true; sel_changed|=setCurEx(              0); Kb.eatKey();}
            if(Kb.k(KB_END )){_kb_action=true; sel_changed|=setCurEx(visibleElms()-1); Kb.eatKey();}
            if(Kb.k(KB_PGUP) || Kb.k(KB_PGDN))
            {
              _kb_action=true;
               if(drawMode()==LDM_LIST)
               {
                  if(cur<0)cur=0;
                  Int page_elms=pageElms(&gpc), dir, new_cur;
                  if(Kb.k(KB_PGUP)){new_cur=cur-page_elms; dir=-1;}
                  else             {new_cur=cur+page_elms; dir=+1;}
                  Kb.eatKey();
                  sel_changed|=setCurEx((flag&LIST_ROLLABLE) ? Mod(new_cur, visibleElms()) : Mid(new_cur, 0, visibleElms()-1), dir);
               }
            }
            if(drawMode()==LDM_LIST)
            {
               if(Kb.k(KB_UP  )){_kb_action=true; sel_changed|=setCurEx((flag&LIST_ROLLABLE) ? ((cur<0) ? visibleElms()-1 : Mod(cur-1, visibleElms())) : Max(0              , cur-1), -1);}
               if(Kb.k(KB_DOWN)){_kb_action=true; sel_changed|=setCurEx((flag&LIST_ROLLABLE) ?                              Mod(cur+1, visibleElms())  : Min(visibleElms()-1, cur+1), +1);}
            }else
            if(drawMode()==LDM_RECTS)
            {
               if(Kb.k(KB_LEFT )){_kb_action=true; sel_changed|=setCurEx(Max(0              , cur-1));}
               if(Kb.k(KB_RIGHT)){_kb_action=true; sel_changed|=setCurEx(Min(visibleElms()-1, cur+1));}
            }
         }

         // smooth scroll
         if(Kb.ctrlCmd() && _parent && _parent->type()==GO_REGION)
         {
            Region &region=_parent->asRegion();
            if(Kb.b(KB_UP   )){Kb.eat(KB_UP   ); region.slidebar[1].button[1].push();}
            if(Kb.b(KB_DOWN )){Kb.eat(KB_DOWN ); region.slidebar[1].button[2].push();}
            if(Kb.b(KB_LEFT )){Kb.eat(KB_LEFT ); region.slidebar[0].button[1].push();}
            if(Kb.b(KB_RIGHT)){Kb.eat(KB_RIGHT); region.slidebar[0].button[2].push();}
         }

         // quick search
         if(flag&LIST_SEARCHABLE)
         {
            if(Kb.k.c && !Kb.k.ctrl() && !Kb.k.lalt() && !Kb.k.win())
            {
              _kb_action=true;
               for(; Kb.k.c; Kb.nextKey())if(_search_i<Elms(_search)-1)
               {
                 _search[  _search_i]=Kb.k.c;
                 _search[++_search_i]=0;
               }
               FREPA(_columns)
               {
                  ListColumn &lc=_columns[i];
                  if(DataIsText(lc.md.type))
                  {
                     FREPA(T)if(Ptr data=visToData(i))if(Starts(lc.md.asText(data, lc.precision), _search)){sel_changed|=setSel(cur=i); break;}
                     break;
                  }
               }
              _search_t=0.9f;
            }else
            if((_search_t-=Time.ad())<=0
            || (Kb.k.k && Kb.k.k!=KB_SHIFT && Kb.k.k!=KB_LSHIFT && Kb.k.k!=KB_RSHIFT))_search[_search_i=0]=0; // if time has passed, or function key was pressed (like arrow up/down or enter, but not shift)
         }
      }
      if(cur!=cur_prev)
      {
         if((cur_prev2!=cur || cur_mode!=LCM_MOUSE) && cur>=0)scrollTo(cur, _search_i!=0, _search_i!=0); // scroll immediately if cursor was changed by "search"
         callCurChanged();
      }
      if(sel_changed)callSelChanged();

      // update children
      GuiPC gpc_col(gpc, T); REPAO(_columns).update(gpc_col);
      GuiPC gpc2(gpc, visible(), enabled()); _children.update(gpc2);
   }
}
/******************************************************************************/
void _List::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      GuiSkin       *     skin=getSkin();
      GuiSkin::List *list_skin=(skin ? &skin->list : null);

      // background
      Vec2 _offset=gpc.offset     , elms_offset=_offset;
      Rect _rect  =gpc.client_rect, elms_rect  =_rect  ;
      if(columnsVisible())
      {
         elms_offset.    y-=columnHeight();
         elms_rect  .max.y-=columnHeight();
      }

      // elements
      if(visibleElms())
      {
         Bool show_sel=(list_skin && list_skin->selection_color.a),
              show_cur=(((cur_mode==LCM_DEFAULT) ? Gui.kb()==this : true) && InRange(cur, T) && list_skin && list_skin->cursor_color.a),
              show_lit=(InRange(lit, T) && list_skin && list_skin->highlight_color.a);

         if(show_lit && show_cur && lit==cur) // if 'lit' is the same as 'cur'
            show_lit=(Gui.dragging() || !Ms.b(0)); // disable highlight when clicking but always allow when dragging

         elms_rect&=gpc.clip;
         D.clip(elms_rect);
         if(drawMode()==LDM_RECTS)
         {
            if(_rects)
            {
               Int pos=Max(0, _horizontal ? screenToVisX(elms_rect.min.x, &gpc) : screenToVisY(elms_rect.max.y, &gpc));
               
               // selection
               if(show_sel && sel.elms())
               {
                  REPA(sel)
                  {
                     Int vis=absToVis(sel[i]); if(/*vis>=0 && */vis>=pos/* && vis<end*/) // no need to test for >=0 because 'pos' is always >=0, we don't test for 'end' because we don't know it yet
                     {
                        Rect r=_rects[vis]+elms_offset; if(_horizontal ? r.min.x<=elms_rect.max.x : r.max.y>=elms_rect.min.y)
                        {
                           if(list_skin->selection)list_skin->selection->draw(list_skin->selection_color, TRANSPARENT, r);
                           else                                        r.draw(list_skin->selection_color);
                        }
                     }
                  }
               }

               // draw elements
               ALPHA_MODE alpha=D.alpha();
               Color      color=WHITE, color_add=TRANSPARENT;
               for(; pos<elms(); pos++)
                  if(Ptr data=visToData(pos))
               {
                  Rect r=_rects[pos]+elms_offset; if(_horizontal ? r.min.x>elms_rect.max.x : r.max.y<elms_rect.min.y)break;
                  if(show_lit && lit==pos)
                  {
                     D.alpha(alpha);
                     if(list_skin->highlight)list_skin->highlight->draw(list_skin->highlight_color, TRANSPARENT, r);
                     else                                        r.draw(list_skin->highlight_color);
                  }
                  if(show_cur && cur==pos)
                  {
                     D.alpha(alpha);
                     if(flag&LIST_MULTI_SEL)
                     {
                        Color c=list_skin->cursor_color; c.a=Min(255, c.a*2); // since we're drawing only borders then increase the alpha
                        if(list_skin->cursor)list_skin->cursor->drawBorders(c, TRANSPARENT, r);
                        else                                  r.draw       (c, false);
                     }else
                     {
                        if(list_skin->cursor)list_skin->cursor->draw(list_skin->cursor_color, TRANSPARENT, r);
                        else                                  r.draw(list_skin->cursor_color);
                     }
                  }
                  if(InRange(draw_column, _columns))if(Image *image=_columns[draw_column].md.asImage(data))
                  {
                     ALPHA_MODE alpha=image_alpha;
                     if( _alpha_mode_offset>=0)alpha=*(ALPHA_MODE*)((Byte*)data+ _alpha_mode_offset); D.alpha(alpha);
                     if(_image_color_offset>=0)color=*(Color     *)((Byte*)data+_image_color_offset);
                     r.min-=_image_padd.min*zoom();
                     r.max-=_image_padd.max*zoom();
                     image->drawFit(color, color_add, r);
                  }
               }
               D.alpha(alpha); // restore alpha
            }
         }else
         {
            Int pos=Max(       0, screenToVirtualY(elms_rect.max.y, &gpc)), // clip start only using Max to allow start>end (invalid range)
                end=Min(elms()-1, screenToVirtualY(elms_rect.min.y, &gpc)); // clip end   only using Min to allow start>end (invalid range)

            // highlight
            if(columnsVisible() && list_skin && list_skin->highlight_color.a)REPA(_columns)
            {
               ListColumn &lc=column(i); if(lc.lit())
               {
                  Rect  r(lc.rect().min.x+_offset.x, _rect.min.y, lc.rect().max.x+_offset.x, _rect.max.y);
                  Color color=ColorAlpha(list_skin->highlight_color, lc.lit());
                  if(list_skin->highlight)list_skin->highlight->draw(color, TRANSPARENT, r);else r.draw(color);
               }
            }
            if(show_sel && sel.elms())
            {
               Rect r=_rect;
               REPA(sel)
               {
                  Int vis=absToVis(sel[i]);
                  if(/*vis>=0 && */vis>=pos && vis<=end) // no need to test for >=0 because 'pos' is always >=0
                  {
                     r.max.y=elms_offset.y-vis*_height_ez;
                     r.min.y=r.max.y      -    _height_ez;
                     if(list_skin->selection)list_skin->selection->draw(list_skin->selection_color, TRANSPARENT, r);
                     else                                        r.draw(list_skin->selection_color);
                  }
               }
            }
            if(show_lit)
            {
               Rect r(_rect.min.x, elms_offset.y-(lit+1)*_height_ez, _rect.max.x, elms_offset.y-lit*_height_ez);
               if(list_skin->highlight)list_skin->highlight->draw(list_skin->highlight_color, TRANSPARENT, r);
               else                                        r.draw(list_skin->highlight_color);
            }
            if(show_cur)
            {
               Rect r(_rect.min.x, elms_offset.y-(cur+1)*_height_ez, _rect.max.x, elms_offset.y-cur*_height_ez);
               if(flag&LIST_MULTI_SEL)
               {
                  Color c=list_skin->cursor_color; c.a=Min(255, c.a*2); // since we're drawing only borders then increase the alpha
                  if(list_skin->cursor)list_skin->cursor->drawBorders(c, TRANSPARENT, r);
                  else                                  r.draw       (c, false);
               }else
               {
                  if(list_skin->cursor)list_skin->cursor->draw(list_skin->cursor_color, TRANSPARENT, r);
                  else                                  r.draw(list_skin->cursor_color);
               }
            }

            // draw elements
            if(skin)
               if(TextStyle *text_style=skin->list.text_style())
            {
               Flt wae  =0.01f,
                   wae_2=wae*0.5f;
               TextStyleParams ts=*text_style; ts.align.set(1, 0); ts.size=textSizeActual();
            #if DEFAULT_FONT_FROM_CUSTOM_SKIN
               if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
            #endif

               // if we're drawing text list, and spacing between elements is a fixed number of pixels then flickering can occur if positions will be at 0.5 pixels (0.5, 1.5, 2.5, ..), to prevent that from happening align the vertical position
               elms_offset+=D.alignScreenToPixelOffset(Vec2(elms_offset.x, ts.posY(elms_offset.y-0.5f*_height_ez)));

               for(; pos<=end; pos++)if(Ptr data=visToData(pos))
               {
                  ALPHA_MODE image_alpha=T.image_alpha;
                  Color      color=WHITE, color_add=TRANSPARENT;
                  Flt        ofs  =0,
                             x    =elms_offset.x,
                             y    =elms_offset.y-pos *_height_ez, // element      position
                             yp   =              pos *_height_ez*D._pixel_size_inv.y; if(Equal(Frac(yp), 0.5f, 0.001f))y+=D._pixel_size_2.y; // vertical offset in pixel coordinates (this can happen very often, text being aligned exactly between 2 pixels)
                  Flt        yt   =            y-0.5f*_height_ez; // element text position

                  if( _text_color_offset>=0)ts.color   =*(Color     *)((Byte*)data+ _text_color_offset);
                  if(_text_shadow_offset>=0)ts.shadow  =*(Byte      *)((Byte*)data+_text_shadow_offset);
                  if( _alpha_mode_offset>=0)image_alpha=*(ALPHA_MODE*)((Byte*)data+ _alpha_mode_offset);
                  if(_image_color_offset>=0)color      =*(Color     *)((Byte*)data+_image_color_offset);
                  if(     _offset_offset>=0)ofs        =*(Flt       *)((Byte*)data+     _offset_offset);
                        Str *group=((_group_offset>=0) ? (Str       *)((Byte*)data+      _group_offset) : null);

                  if(group && group->is())
                  {
                     ts.align.x=1;
                     D.clip(elms_rect);
                     D.text(ts, wae+x+ofs, yt, *group);
                  }else
                  FREPAD(c, _columns)
                  {
                     ListColumn &lc=_columns[c];
                     if(lc.visible() && lc.md.type)
                     {
                        if(Image *image=lc.md.asImage(data))
                        {
                           ALPHA_MODE alpha=D.alpha(image_alpha);
                           D.clip (elms_rect);
                           image->drawFit(color, color_add, Rect_LU(lc.rect().min.x+x+ofs, y, lc.rect().w(), _height_ez));
                           D.alpha(alpha);
                        }else
                        {
                         C Str &text=lc.md.asText(data, lc.precision); if(text.is())
                           {
                              ts.align.x=lc.text_align;
                              D.clip(gpc.clip & Rect(lc.rect().min.x+wae_2+x, elms_rect.min.y, lc.rect().max.x-wae_2+x, elms_rect.max.y));
                              D.text(ts, Lerp(lc.rect().max.x-wae, lc.rect().min.x+wae, lc.text_align*0.5f+0.5f)+x+ofs, yt, text);
                           }
                        }
                        Color color;
                        if(C ImagePtr &image=DataGuiImage(data, lc, color))
                        {
                           D.clip(gpc.clip & Rect(lc.rect().min.x+wae_2+x, elms_rect.min.y, lc.rect().max.x-wae_2+x, elms_rect.max.y));
                           image->drawFit(color, TRANSPARENT, Rect_C(Vec2(lc.rect().centerX()+x+ofs, yt), Vec2(ts.size.avg())));
                        }
                        if(!DataIsImage(lc.md.type) && _offset_first_column)ofs=0; // reset offset for >=1 columns
                     }
                  }
               }
            }
         }

         // draw children
         if(_children.children.elms())
         {
            VecI2 visible_range=visibleElmsOnScreen(&gpc); if(visible_range.y>=visible_range.x)
            {
               GuiPC gpc2(gpc, visible(), enabled()); gpc2.clip=elms_rect; Vec2 offset=gpc2.offset;
               FREPA(_children) // 'draw' must be done from the start
               {
                  Children::Child &child=_children[i];
                  if(SetGPC(T, gpc2, offset, child.abs_col, visible_range))child.go->draw(gpc2);
               }
            }
         }
      }

      // draw columns
      if(columnsVisible())
      {
         ListColumn *resize_col=null;
         GuiPC gpc_col(gpc, T); FREPA(_columns){ListColumn &lc=_columns[i]; lc.draw(gpc_col); if(lc.resizeEdge())resize_col=&lc;}
         if(resize_col && list_skin)if(Image *image=list_skin->resize_column())
         {
            D.clip(gpc.clip);
            Vec2 pos=Ms.pos(), size(MOUSE_IMAGE_SIZE*image->aspect(), MOUSE_IMAGE_SIZE); pos+=Vec2(-size.x, size.y)*0.5f;
            image->draw(Rect_LU(D.screenAlignedToPixel(pos), size));
         }
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
