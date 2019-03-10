/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
GuiPC::GuiPC(C GuiPC &old, Region &region)
{
   T=old;
   visible   &=region.visible();
   enabled   &=region.enabled();
   client_rect=region.clientRect()+offset;
   offset     =client_rect.lu();
   offset.x  -=region.slidebar[0].offset();
   offset.y  +=region.slidebar[1].offset();
   GuiSkin *skin=region.getSkin(); if(skin && skin->region.normal && skin->region.normal->pixelBorder())clip&=Rect(client_rect).extend(-D._pixel_size);else clip&=client_rect; // if the panel draws 1-pixel-border then leave it, this is so that Region children will not draw on top of it, because border should look like it's on top

   if(1)D.alignScreenToPixel(offset);
}
/******************************************************************************/
void Region::zero()
{
   kb_lit=true; _flag=0;
  _slidebar_size=0.05f;
  _crect.zero();
}
Region::Region() {zero();}
Region& Region::del()
{
  _children   .del  ();
   slidebar[0].del  ();
   slidebar[1].del  ();
   view       .del  ();
  _skin       .clear();
   super::del(); zero(); return T;
}
void Region::setParent(Bool on)
{
   slidebar[0]._parent=slidebar[1]._parent=view._parent=(on ? this : null);
}
void Region::setParams()
{
  _type=GO_REGION;
   view._sub_type=BUTTON_TYPE_REGION_VIEW;
   setParent();
}
Region& Region::create()
{
   del();

  _visible=true;

   view       .create().mode=BUTTON_CONTINUOUS;
   slidebar[0].create().setLengths(0, 0).hide();
   slidebar[1].create().setLengths(0, 0).hide();
   view._focusable=slidebar[0]._focusable=slidebar[1]._focusable=false;
   setParams();

   return T;
}
Region& Region::create(C Region &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
        _children.del();
         copyParams(src);
        _type         =GO_REGION;
         kb_lit       =src. kb_lit;
        _slidebar_size=src._slidebar_size;
        _flag         =src._flag;
        _skin         =src._skin;
        _crect        =src._crect;
         view       .create(src.view       );
         slidebar[0].create(src.slidebar[0]);
         slidebar[1].create(src.slidebar[1]);
         setParent();
      }   
   }
   return T;
}
/******************************************************************************/
void Region::childRectChanged(C Rect *old_rect, C Rect *new_rect, GuiObj &child)
{
   // null rectangle means object is hidden
   if(old_rect || new_rect)
   {
      // decreasing size if old size was greater than virtual size and new size isn't
      Flt     x=(new_rect ? GuiMaxX(*new_rect) : 0),
              y=(new_rect ? GuiMaxY(*new_rect) : 0),
          old_x=(old_rect ? GuiMaxX(*old_rect) : 0),
          old_y=(old_rect ? GuiMaxY(*old_rect) : 0);
      if((old_x>=virtualWidth ()-EPS && x<virtualWidth ()-EPS) // if decreasing in at least one dimension then we need to recalculate fully
      || (old_y>=virtualHeight()-EPS && y<virtualHeight()-EPS))virtualSize(null);else
      {
         // increasing size if new size is greater than virtual size
         Bool inc_w=(x>virtualWidth ()+EPS),
              inc_h=(y>virtualHeight()+EPS);
         if(inc_w || inc_h)
         {
            if(inc_w)slidebar[0]._length_total=x;
            if(inc_h)slidebar[1]._length_total=y;
            setButtons();
         }
      }
   }//else having null rectangles means object is hidden and we don't need to do anything
}
void Region::addChild(GuiObj &child)
{
   if(_children.add(child, T))childRectChanged(null, child.visible() ? &child.rect() : null, child);
}
void Region::removeChild(GuiObj &child)
{
   if(_children.remove(child))childRectChanged(child.visible() ? &child.rect() : null, null, child);
}
/******************************************************************************/
Region& Region::removeSlideBars()
{
   setParent(false); // detach before setting rectangle so virtual size won't be changed
   slidebar[0].del();
   slidebar[1].del();
   view       .del();
   setButtons(); return T;
}
Bool    Region::alwaysHideHorizontalSlideBar(         )C {return FlagTest(_flag, ALWAYS_HIDE_HORIZONTAL_SLIDEBAR);}
Region& Region::alwaysHideHorizontalSlideBar(Bool hide)
{
   if(hide!=alwaysHideHorizontalSlideBar())
   {
     _flag^=ALWAYS_HIDE_HORIZONTAL_SLIDEBAR;
      setButtons();
   }
   return T;
}

void Region::setButtons()
{
   Bool vertical, horizontal;
   Flt  width =virtualWidth (),
        height=virtualHeight();

   Rect srect=_crect=rect();
   if( vertical  =(slidebar[1].is() && height>clientHeight()+EPS)){srect.max.x-=slidebarSize();                                    _crect.max.x-=slidebarSize();}
   if( horizontal=(slidebar[0].is() && width >clientWidth ()+EPS)){srect.min.y+=slidebarSize(); if(!alwaysHideHorizontalSlideBar())_crect.min.y+=slidebarSize();
   if(!vertical && slidebar[1].is() && height>clientHeight()+EPS ){srect.max.x-=slidebarSize();                                    _crect.max.x-=slidebarSize(); vertical=true;}}

   setParent(false); // detach before setting rectangle so virtual size won't be changed
   slidebar[0].setLengths(clientWidth (), width ).rect(Rect(rect().min.x               ,  rect().min.y, srect  .max.x, rect().min.y+slidebarSize())); slidebar[0].visible(slidebar[0]._usable && !alwaysHideHorizontalSlideBar());
   slidebar[1].setLengths(clientHeight(), height).rect(Rect(rect().max.x-slidebarSize(), srect  .min.y,  rect().max.x, rect().max.y               )); slidebar[1].visible(slidebar[1]._usable                                   );
   view                                          .rect(Rect(rect().max.x-slidebarSize(),  rect().min.y,  rect().max.x, rect().min.y+slidebarSize())).visible(slidebar[0]._usable && slidebar[1]._usable);
   setParent();
}
Vec2 Region::childrenSize()C
{
   Vec2 size=0;
   REPA(_children)if(C GuiObj *c=_children[i])if(c->visible())
   {
      MAX(size.x, GuiMaxX(c->rect()));
      MAX(size.y, GuiMaxY(c->rect()));
   }
   return size;
}
Region& Region::slidebarSize(Flt size)
{
   MAX(size, 0);
   if(_slidebar_size!=size)
   {
     _slidebar_size=size;
      setButtons();
   }
   return T;
}
Region& Region::virtualSize(C Vec2 *size)
{
   Vec2 temp; if(!size)size=&(temp=childrenSize());
   slidebar[0]._length_total=size->x;
   slidebar[1]._length_total=size->y;
   setButtons(); return T;
}
Region& Region::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      Rect old_client=localClientRect(), old_rect=T.rect(); T._rect=rect; setButtons(); // set manually instead of calling 'super::rect' because that will call 'notifyChildrenOfClientRectChange' before we finish setting client rectangle
      Rect new_client=localClientRect();
      notifyChildrenOfClientRectChange(&old_client, &new_client);
      notifyParentOfRectChange        ( old_rect  ,  visible() );
   }
   return T;
}
Region& Region::move(C Vec2 &delta)
{
   if(delta.any())
   {
      super::move(delta);
     _crect  +=   delta ;
      setParent(false); // detach before setting rectangle so virtual size won't be changed
      view       .move(delta);
      slidebar[0].move(delta);
      slidebar[1].move(delta);
      setParent();
   }
   return T;
}
/******************************************************************************/
Region& Region::skin(C GuiSkinPtr &skin, Bool sub_objects)
{
  _skin=skin;
   if(sub_objects)
   {
            view     .skin=skin ;
      REPAO(slidebar).skin(skin);
   }
   return T;
}
/******************************************************************************/
GuiObj* Region::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(GuiObj *go=super::test(gpc, pos, mouse_wheel))
   {
      if(!Kb.ctrlCmd()) // when Ctrl is hold then don't set mouse wheel focus to slidebars when we focus on Region, to allow child 'ComboBox' and 'Slider' to catch it, and if nothing is catched, then still leave empty so we don't accidentally scroll when using mouse wheel near ComboBox or Slider but accidentally not on it
      {
         Bool priority=!Kb.shift(); // when 'Kb.shift' disabled (default) then priority=1 (vertical), when enabled then priority=0 (horizontal)
         if(slidebar[ priority]._usable)mouse_wheel=&slidebar[ priority];else // check  priority slidebar first
         if(slidebar[!priority]._usable)mouse_wheel=&slidebar[!priority];     // check !priority slidebar next
      }

      GuiPC gc  (gpc, T                   ); if(GuiObj *go=_children.test(gc, pos, mouse_wheel))return go;
      GuiPC gpc2(gpc, visible(), enabled());
      if(GuiObj *go=slidebar[0].test(gpc2, pos, mouse_wheel))return go;
      if(GuiObj *go=slidebar[1].test(gpc2, pos, mouse_wheel))return go;
      if(GuiObj *go=view       .test(gpc2, pos, mouse_wheel))return go;

      return go;
   }
   return null;
}
/******************************************************************************/
void Region::update(C GuiPC &gpc)
{
   GuiPC gpc2(gpc, visible(), enabled());
   if(   gpc2.enabled)
   {
      view.update(gpc2);
      if(view())
      {
         if(Gui.ms()==&view)
         {
            Ms.freeze();
            slidebar[0].setOffset(slidebar[0]._offset + Ms.d().x*2);
            slidebar[1].setOffset(slidebar[1]._offset - Ms.d().y*2);
         }
         REPA(Touches)
         {
            Touch &t=Touches[i]; if(t.guiObj()==&view && t.on())
            {
               slidebar[0].setOffset(slidebar[0]._offset + t.d().x*2);
               slidebar[1].setOffset(slidebar[1]._offset - t.d().y*2);
            }
         }
      }

      // scroll horizontally
      if(Ms.wheelX()
      && (Gui.wheel()==&slidebar[0] || Gui.wheel()==&slidebar[1]) // if has focus on any of slidebars
      && slidebar[0]._usable) // we will scroll only horizontally, so check if that's possible
         slidebar[0].scroll(Ms.wheelX()*(slidebar[0]._scroll_mul*slidebar[0].length()+slidebar[0]._scroll_add), slidebar[0]._scroll_immediate);

      slidebar[0].update(gpc2);
      slidebar[1].update(gpc2);

      GuiPC gc(gpc, T); _children.update(gc);
   }
}
void Region::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      GuiSkin *skin=getSkin();
      Rect     rect=T.rect()+gpc.offset, ext_rect;
      if(skin && skin->region.normal)skin->region.normal->extendedRect(rect, ext_rect);else ext_rect=rect;
      if(Cuts(ext_rect, gpc.clip))
      {
         if(skin)
         {
            D.clip(gpc.clip);
            if(skin->region.normal        )skin->region.normal->draw(skin->region.normal_color, rect);else
            if(skin->region.normal_color.a)                rect.draw(skin->region.normal_color);
         }
         view       .draw(gpc);
         slidebar[0].draw(gpc);
         slidebar[1].draw(gpc);

         GuiPC gc(gpc, T); _children.draw(gc);

         if(kb_lit && contains(Gui.kb())){D.clip(gpc.clip); Gui.kbLit(this, rect, skin);}
      }
   }
}
/******************************************************************************/
Bool Region::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      // !! in the future save '_flag' !!
      f.putMulti(Byte(3), kb_lit, _slidebar_size, _crect); // version
      f.putAsset(_skin.id());
      if(view       .save(f, path))
      if(slidebar[0].save(f, path))
      if(slidebar[1].save(f, path))
         return f.ok();
   }
   return false;
}
Bool Region::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 3:
      {
         f.getMulti(kb_lit, _slidebar_size, _crect);
        _skin.require(f.getAssetID(), path);
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){setParams(); return true;}
      }break;

      case 2:
      {
         f>>kb_lit>>_slidebar_size>>_crect;
        _skin.require(f._getAsset(), path);
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){setParams(); return true;}
      }break;

      case 1:
      {
         f>>kb_lit>>_slidebar_size>>_crect;
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){f._getStr(); setParams(); return true;}
      }break;

      case 0:
      {
         f>>kb_lit>>_slidebar_size>>_crect;
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){f._getStr8(); setParams(); return true;}
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
