/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define WINDOW_PADD 0.030f
/******************************************************************************/
GuiPC::GuiPC(C GuiPC &old, Window &window)
{
   T=old;
   visible   &=window.visible();
   enabled   &=window.enabled();
   client_rect=window._crect+offset;
   offset     =client_rect.lu();
   GuiSkin *skin=window.getSkin(); if(skin && skin->region.normal && skin->region.normal->pixelBorder())clip&=Rect(client_rect).extend(-D._pixel_size);else clip&=client_rect; // if the panel draws 1-pixel-border then leave it, this is so that Region children will not draw on top of it, because border should look like it's on top

   if(1)D.alignScreenToPixel(offset);
}
/******************************************************************************/
static void Hide    (Window &window) {window.hide();}
static void FadeOut (Window &window) {window.fadeOut();}
static void Maximize(Window &window) {window.maximize();}
/******************************************************************************/
void Window::zero()
{
   flag  =WIN_MOVABLE;
  _base_level=GBL_WINDOW;
  _level =0;
  _resize=0;
  _crect.zero();
   ripple=null;
   resize_mask=DIRF_RIGHT|DIRF_LEFT|DIRF_DOWN|DIRF_UP;

  _bar_visible=true;
  _fade_type  =FADE_NONE;
  _alpha      =_fade_alpha=_final_alpha=0; // keep as zero when deleted
  _lit_hover  =0;
  _lit_focus  =0;
}
Window::Window() {zero();}
Window& Window::del()
{
  _children.del  ();
   title   .clear();
  _skin    .clear();
   REPA(button)button[i].del();
   super::del(); zero(); return T;
}
void Window::setParent()
{
   REPAO(button)._parent=this;
}
void Window::setParams()
{
  _type=GO_WINDOW;
   setParent();
   button[0]._sub_type=BUTTON_TYPE_WINDOW_MINIMIZE;
   button[1]._sub_type=BUTTON_TYPE_WINDOW_MAXIMIZE;
   button[2]._sub_type=BUTTON_TYPE_WINDOW_CLOSE;
   button[1].func(Maximize, T);
   button[2].func(Gui.window_fade ? FadeOut : Hide, T);
}
Window& Window::create(C Str &title)
{
   del();

  _visible=true;
  _alpha  =_fade_alpha=_final_alpha=1;

   T.title=title;

   // create buttons and setup their initial sizes
   Flt h=defaultBarHeight(), w;
   button[0].create().hide(); w=h; if(GuiSkin *s=button[0].getSkin())if(PanelImage *pi=s->window.minimize.normal())if(pi->image.is())w*=pi->image.aspect(); button[0].rect(Rect_LU(0, 0, w, h));
   button[1].create().hide(); w=h; if(GuiSkin *s=button[1].getSkin())if(PanelImage *pi=s->window.maximize.normal())if(pi->image.is())w*=pi->image.aspect(); button[1].rect(Rect_LU(0, 0, w, h));
   button[2].create().hide(); w=h; if(GuiSkin *s=button[2].getSkin())if(PanelImage *pi=s->window.close   .normal())if(pi->image.is())w*=pi->image.aspect(); button[2].rect(Rect_LU(0, 0, w, h));
   button[0]._focusable=button[1]._focusable=button[2]._focusable=false;
   setParams();
   return T;
}
Window& Window::create(C Window &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
        _children.del();
         copyParams(src);
        _type       =GO_WINDOW;
         flag       =src. flag;
         resize_mask=src. resize_mask;
         title      =src. title;
         ripple     =src. ripple;
        _bar_visible=src._bar_visible;
        _fade_type  =src._fade_type;
        _resize     =src._resize;
        _level      =src._level;
        _alpha      =src._alpha;
        _fade_alpha =src._fade_alpha;
        _final_alpha=src._final_alpha;
        _lit_hover  =src._lit_hover;
        _lit_focus  =src._lit_focus;
        _crect      =src._crect;
        _skin       =src._skin;
         REPAO(T.button).create(src.button[i]);
         setParent();
      }   
   }
   return T;
}
/******************************************************************************/
void Window::   addChild(GuiObj &child) {_children.add   (child, T);}
void Window::removeChild(GuiObj &child) {_children.remove(child   );}
/******************************************************************************/
Flt Window::       barHeight   ()C {return rect().max.y-clientRect().max.y;}
Flt Window::defaultBarHeight   ()C {if(Panel *panel=getNormalPanel()){Rect padd; panel->innerPadding(T.rect(), padd); return padd.max.y;} return 0;}
Flt Window::defaultBarTextWidth()C
{
   if(GuiSkin *skin=getSkin())
      if(TextStyle *text_style=skin->window.normal_text_style())
   {
      TextStyleParams ts=*text_style; ts.size=defaultBarHeight()*skin->window.text_size; return ts.textWidth(title);
   }
   return 0;
}
Flt Window::defaultBarFullWidth()C
{
   Flt width=0, bar_height=defaultBarHeight();
   if(GuiSkin *skin=getSkin())
   {
      width+=skin->window.text_padd*2*bar_height;
      if(TextStyle *text_style=skin->window.normal_text_style())
      {
         TextStyleParams ts=*text_style; ts.size=skin->window.text_size*bar_height; width+=ts.textWidth(title);
      }
   }
   REPA(button)if(button[i].visible())width+=button[i].rect().aspect()*bar_height;
   return width;
}
Panel* Window::getNormalPanel()C
{
   if(GuiSkin *skin=getSkin())return barVisible() ? skin->window.normal() : skin->window.normal_no_bar();
   return null;
}
void Window::extendedRect           (Rect &rect     )C {if(Panel *panel=getNormalPanel())panel->extendedRect           (T.rect(), rect);else    rect=T.rect();}
void Window::defaultInnerPadding    (Rect &padding  )C {if(Panel *panel=getNormalPanel())panel->defaultInnerPadding    (padding       );else padding  .zero();}
void Window::defaultInnerPaddingSize(Vec2 &padd_size)C {if(Panel *panel=getNormalPanel())panel->defaultInnerPaddingSize(padd_size     );else padd_size.zero();}
Vec2 Window::defaultInnerPaddingSize(               )C {Vec2 size; defaultInnerPaddingSize(size); return size;}
/******************************************************************************/
static Rect ResizedRect(C Rect &src, C Rect &dest, UInt mask)
{
 C Rect *r[2]={&src, &dest};
   Rect  resized;
   resized.min.x=r[FlagTest(mask, DIRF_LEFT )]->min.x;
   resized.min.y=r[FlagTest(mask, DIRF_DOWN )]->min.y;
   resized.max.x=r[FlagTest(mask, DIRF_RIGHT)]->max.x;
   resized.max.y=r[FlagTest(mask, DIRF_UP   )]->max.y;
   return resized;
}
static Rect MaximizedRect(C Window &window)
{
   if(GuiObj *parent=window.parent())return parent->localClientRect();
   return D.rect();
}
Bool    Window::maximized()C {return InsideEps(ResizedRect(rect(), MaximizedRect(T), resize_mask), rect());}
Window& Window::maximize ()  {return rect(ResizedRect(rect(), maximized() ? MaximizedRect(T)*0.5f : MaximizedRect(T), resize_mask));}
/******************************************************************************/
Bool    Window::showing()C {return visible() && _fade_type!=FADE_OUT;}
Bool    Window::hiding ()C {return hidden () || _fade_type==FADE_OUT;}
Window& Window::fadeIn ()
{
   if(hiding())
   {
      Flt fade_alpha=fadeAlpha(); // remember current alpha because 'show' may change it
      show();
      if(visible() && Gui.allow_window_fade) // don't modify fade unless we're actually visible ('show' succeeded)
      {
        _fade_type =FADE_IN;
        _fade_alpha=fade_alpha; setFinalAlpha();
      }
   }
   return T;
}
Window& Window::fadeOut()
{
   if(visible())
   {
      if(!Gui.allow_window_fade)hide();else
      {
        _fade_type=FADE_OUT;   
         if(contains(Gui.menu()))Gui.menu()->hide();
      }
   }
   return T;
}
Window& Window::fadeToggle()
{
   if(hiding())fadeIn ();
   else        fadeOut();
   return T;
}
Window& Window::fade(Bool in)
{
   return in ? fadeIn() : fadeOut();
}
void Window::focusToggle()
{
   if(hidden() || partiallyOccludedInSameLevel())activate();else hide();
}
  void    Window::setFinalAlpha(            ) {_final_alpha=_alpha*_fade_alpha;}
  Window& Window::alpha        (Flt  alpha  ) {_alpha=Sat(alpha); setFinalAlpha(); return T;}
  Window& Window::level        (Int  level  ) {if(_level      !=level  ){_level       =level ; validateLevel();} return T;}
  Window& Window::barVisible   (Bool visible) {if(_bar_visible!=visible){_bar_visible^=1     ; setRect      ();} return T;}
//Window& Window::barHeight    (Flt  height ) {if(_bar_height !=height ){_bar_height  =height; setRect      ();} return T;}
/******************************************************************************/
Window& Window::skin(C GuiSkinPtr &skin)
{
   if(_skin!=skin)
   {
     _skin=skin;
      REPAO(button).skin=skin;
      setRect();
   }
   return T;
}
Window& Window::setTitle(C Str  &title) {T.title=title; return T;}
void Window::setButtons()
{
   Flt  size=barHeight();
   Vec2 pos(_crect.max.x, _rect.max.y);
   if(GuiSkin *skin=getSkin())
   {
      Flt scale;
      if(Panel *panel=(barVisible() ? skin->window.normal() : skin->window.normal_no_bar()))
         if(panel->getSideScale(T.rect(), scale))pos+=skin->window.button_offset*scale;else pos+=skin->window.button_offset;

      if(!barVisible())if(Panel *panel=skin->window.normal()) // if the window has no bar, then we need to detect the size from regular window panel with bar
      {
         Rect padd; panel->innerPadding(T.rect(), padd); MAX(size, padd.max.y);
      }
      if(size<=0)size=skin->menubar.bar_height; // if the size is unavailable then default to size taken from menu bar
   }
   if(!Gui.windowButtonsRight())pos.x=_crect.min.x+(_crect.max.x-pos.x); // mirror to the left side

   // setup buttons
   if(button[2].visible()){Flt w=size; if(GuiSkin *s=button[2].getSkin())if(PanelImage *pi=s->window.close   .normal())if(pi->image.is())w*=pi->image.aspect(); Rect rect; if(Gui.windowButtonsRight()){rect.setRU(pos, w, size); pos.x-=w;}else{rect.setLU(pos, w, size); pos.x+=w;} button[2].rect(rect);}
   if(button[1].visible()){Flt w=size; if(GuiSkin *s=button[1].getSkin())if(PanelImage *pi=s->window.maximize.normal())if(pi->image.is())w*=pi->image.aspect(); Rect rect; if(Gui.windowButtonsRight()){rect.setRU(pos, w, size); pos.x-=w;}else{rect.setLU(pos, w, size); pos.x+=w;} button[1].rect(rect);}
   if(button[0].visible()){Flt w=size; if(GuiSkin *s=button[0].getSkin())if(PanelImage *pi=s->window.minimize.normal())if(pi->image.is())w*=pi->image.aspect(); Rect rect; if(Gui.windowButtonsRight()){rect.setRU(pos, w, size); pos.x-=w;}else{rect.setLU(pos, w, size); pos.x+=w;} button[0].rect(rect);}
}
void Window::setRect()
{
   Rect old_client=localClientRect();

   // first calculate client rectangle
   T._crect=rect();
   if(GuiSkin *skin=getSkin())
      if(Panel *panel=panel=(barVisible() ? skin->window.normal() : skin->window.normal_no_bar()))
   {
      Rect padd; panel->innerPadding(T.rect(), padd);
     _crect.min+=padd.min;
     _crect.max-=padd.max;
   }
 
   // now that client rectangle is known, we can calculate bar height and button rectangles
   setButtons();

   Rect new_client=localClientRect();
   notifyChildrenOfClientRectChange(&old_client, &new_client);
}
Window& Window::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      Rect old_rect=T.rect(), r=rect, size_limit=sizeLimit();
      r.max.x=r.min.x+Mid(r.w(), size_limit.min.x, size_limit.max.x);
      r.min.y=r.max.y-Mid(r.h(), size_limit.min.y, size_limit.max.y);
      T._rect=r; // set manually instead of calling 'super::rect' because that will call 'notifyChildrenOfClientRectChange' before we finish setting client rectangle

      setRect();

      notifyParentOfRectChange(old_rect, visible());
   }
   return T;
}
Window& Window::clientRect(C Rect &rect)
{
   Rect r=rect, padd;
   defaultInnerPadding(padd);
   r.min-=padd.min;
   r.max+=padd.max;
   return T.rect(r);
}
Window& Window::move(C Vec2 &delta)
{
   if(delta.any())
   {
      super::move(delta);
     _crect+=delta;
      button[0].move(delta);
      button[1].move(delta);
      button[2].move(delta);
   }
   return T;
}
/******************************************************************************/
Window& Window::show()
{
   super::show();
  _fade_type =FADE_NONE;
  _fade_alpha=1; setFinalAlpha();
   return T;
}
Window& Window::hide()
{
   if(visible())
   {
      Bool activate_next=(Gui.window()==this);

      super::hide();

      // activate next window from mutual parent
      if(activate_next && parent())if(GuiObjChildren *children=parent()->children())
         REPA(*children)
            if(GuiObj *go=(*children)[i])if(go->type()==GO_WINDOW && go!=this && go->asWindow().showing()/* && !FlagTest(go->asWindow().flag, WIN_IMMEDIATE_DEACT)*/)
               {go->activate(); break;}
   }
  _fade_type =FADE_NONE;
  _fade_alpha=0; setFinalAlpha();
   return T;
}
/******************************************************************************/
void Window::parentClientRectChanged(C Rect *old_client, C Rect *new_client)
{
   if(old_client && new_client)
   {
      Flt eps    =0.03f,
          new_top=new_client->max.y,
          old_top=old_client->max.y;
      if(parent())REP(parent()->childNum())if(GuiObj *menu=parent()->child(i))if(menu->type()==GO_MENU_BAR && menu->visible())
      {
         Flt h=menu->rect().h();
         new_top-=h;
         old_top-=h;
         break;
      }

      if(new_client->w()>old_client->w())
      {
         Bool stick_r=(rect().max.x>=old_client->max.x-eps),
              stick_l=(rect().min.x<=old_client->min.x+eps);
         if(stick_r && !stick_l)move(Vec2(new_client->max.x-old_client->max.x, 0));else
         if(stick_l && !stick_r)move(Vec2(new_client->min.x-old_client->min.x, 0));
      }else
      if(new_client->w()<old_client->w())
      {
         Bool stick_r=(new_client->max.x<rect().max.x), // new smaller size doesn't contain the full window
              stick_l=(new_client->min.x>rect().min.x); // new smaller size doesn't contain the full window
         if(stick_r && !stick_l)move(Vec2(Max(new_client->max.x-rect().max.x, new_client->max.x-old_client->max.x), 0));else
         if(stick_l && !stick_r)move(Vec2(Min(new_client->min.x-rect().min.x, new_client->min.x-old_client->min.x), 0));
      }

      if(new_client->h()>old_client->h())
      {
         Bool stick_u=(rect().max.y>=old_top          -eps),
              stick_d=(rect().min.y<=old_client->min.y+eps);
         if(stick_u && !stick_d)move(Vec2(0, new_client->max.y-old_client->max.y));else
         if(stick_d && !stick_u)move(Vec2(0, new_client->min.y-old_client->min.y));
      }else
      if(new_client->h()<old_client->h())
      {
         Bool stick_u=(new_top          <rect().max.y), // new smaller size doesn't contain the full window
              stick_d=(new_client->min.y>rect().min.y); // new smaller size doesn't contain the full window
         if(stick_u && !stick_d)move(Vec2(0, Max(new_top          -rect().max.y, new_client->max.y-old_client->max.y)));else
         if(stick_d && !stick_u)move(Vec2(0, Min(new_client->min.y-rect().min.y, new_client->min.y-old_client->min.y)));
      }

      // if shrinking screen and window is resizable, then try to adjust its size for new screen size
      if(flag&WIN_RESIZABLE)
      {
         Flt w=rect().w(),
             h=rect().h();
         if(w>new_client->w() || h>new_client->h())
         {
            size(Vec2(Min(w, new_client->w()), Min(h, new_client->h())));
            if(rect().min.x<new_client->min.x
            || rect().max.y>new_client->max.y)
               move(Vec2(Mid(new_client->min.x-rect().min.x, 0.0f, w-rect().w()),
                         Mid(new_client->max.y-rect().max.y, rect().h()-h, 0.0f)));
         }
      }
   }
}
/******************************************************************************/
GuiObj* Window::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(visible() && gpc.visible && _fade_type!=FADE_OUT)
   {
      Rect r=rect()+gpc.offset;
      if(flag&WIN_RESIZABLE)
      {
         if(resize_mask&DIRF_LEFT )r.min.x-=Gui.resize_radius;
         if(resize_mask&DIRF_RIGHT)r.max.x+=Gui.resize_radius;
         if(resize_mask&DIRF_DOWN )r.min.y-=Gui.resize_radius;
         if(resize_mask&DIRF_UP   )r.max.y+=Gui.resize_radius;
      }
      if(Cuts(pos, r&gpc.clip))
      {
         mouse_wheel=this;

         GuiPC gpc2(gpc, true, enabled());
         if(GuiObj *go=button[2].test(gpc2, pos, mouse_wheel))return go;
         if(GuiObj *go=button[1].test(gpc2, pos, mouse_wheel))return go;
         if(GuiObj *go=button[0].test(gpc2, pos, mouse_wheel))return go;

         GuiPC gpc_children(gpc, T); if(GuiObj *go=_children.test(gpc_children, pos, mouse_wheel))return go;

         return this;
      }
   }
   return null;
}
/******************************************************************************/
static inline void UpdateStretch(Flt side, Flt &l, Flt &r, Int &li, Int &ri, Flt d)
{
   if(side<0)
   {
      li++;
      l+=d;
   }else
   if(side>0)
   {
      ri++;
      r+=d;
   }
}
static void UpdateResize(Flt &min, Flt &max, Flt l, Flt r, Flt size_min, Flt size_max, UInt move, UInt resize_l, UInt resize_r)
{
   // move, and reduce 'l', 'r' by what was moved
   if(move)
   {
      if(l>0 && r>0){Flt d=Min(l, r); l-=d; r-=d; min+=d; max+=d;}else
      if(l<0 && r<0){Flt d=Max(l, r); l-=d; r-=d; min+=d; max+=d;}
   }

   // resize both by the same amount, and reduce 'l', 'r' by what was resized
   if(resize_l && resize_r)
   {
      if(l>0 && r<0) // shrink
      {
         Flt d=Min(l, -r); Flt w=max-min; MIN(d, (w-size_min)*0.5f); l-=d; r+=d; min+=d; max-=d; // new_width=w-d*2, size_min=new_width -> size_min=w-d*2 -> d*2=w-size_min -> d=(w-size_min)/2
      }else
      if(l<0 && r>0) // extend
      {
         Flt d=Min(-l, r); Flt w=max-min; MIN(d, (size_max-w)*0.5f); l+=d; r-=d; min-=d; max+=d; // new_width=w+d*2, size_max=new_width -> size_max=w+d*2 -> d*2=size_max-w -> d=(size_max-w)/2
      }
   }

   // resize separately by what's left
   if(resize_l)min+=l;
   if(resize_r)max+=r;

   // clamp
   Clamp(min, max-size_max, max-size_min);
   Clamp(max, min+size_min, min+size_max);
}
/******************************************************************************/
void Window::update(C GuiPC &gpc)
{
   GuiPC gpc_this(gpc, visible(), enabled()),
         gpc_children(gpc, T);

   if(gpc_this.visible && Gui.ms()==this && (flag&WIN_RESIZABLE) && _fade_type!=FADE_OUT && !Ms.b(1))
   {
      if(!Ms.b(0)) // if not pressed then detect new resize, if pressed then keep previous
      {
        _resize=0;
         Vec2 aligned_offset=gpc_children.offset-_crect.lu(), pos=Ms.pos()-aligned_offset;
         if(pos.x<=rect().min.x+D.pixelToScreenSize().x)_resize|=DIRF_LEFT ;else
         if(pos.x>=rect().max.x-D.pixelToScreenSize().x)_resize|=DIRF_RIGHT;
         if(pos.y<=rect().min.y+D.pixelToScreenSize().y)_resize|=DIRF_DOWN ;else
         if(pos.y>=rect().max.y-D.pixelToScreenSize().y)_resize|=DIRF_UP   ;
        _resize&=resize_mask;
      }
   }else _resize=0;

   if(gpc_this.visible)
   {
      if(gpc.enabled)
      {
         Bool have_client_rect=false; Rect client_rect; if(parent()){have_client_rect=true; client_rect=parent()->localClientRect();}
         if(Gui.ms()==this)
         {
            if(Ms.b(0) && _resize)
            {
               Vec2 pos=Ms.pos()-gpc.offset;
               Rect r  =rect(), size=sizeLimit();
               MIN(size.max.x, Max(size.min.x, D.w2(), rect().w()));
               MIN(size.max.y, Max(size.min.y, D.h2(), rect().h()));
               if(_resize&DIRF_LEFT )r.min.x=Mid(have_client_rect ? Min(pos.x, client_rect.max.x-WINDOW_PADD) : pos.x, r.max.x-size.max.x, r.max.x-size.min.x);
               if(_resize&DIRF_UP   )r.max.y=Mid(have_client_rect ? Max(pos.y, client_rect.min.y+WINDOW_PADD) : pos.y, r.min.y+size.min.y, r.min.y+size.max.y);
               if(_resize&DIRF_RIGHT)r.max.x=Mid(have_client_rect ? Max(pos.x, client_rect.min.x+WINDOW_PADD) : pos.x, r.min.x+size.min.x, r.min.x+size.max.x);
               if(_resize&DIRF_DOWN )r.min.y=Mid(have_client_rect ? Min(pos.y, client_rect.max.y-WINDOW_PADD) : pos.y, r.max.y-size.max.y, r.max.y-size.min.y);
               rect(r);
            }
            if(flag&WIN_RESIZABLE)
            {
               if(Ms.bd(0) && Ms.pos().y>=_crect.max.y+gpc.offset.y){Maximize(T); _resize=0;} // maximize when double clicking on the title bar
               if(Ms.b (1))
               {
                  Ms.freeze();
                  if(Ms.d().any() && resize_mask)
                  {
                     Rect r=rect(), size=sizeLimit();
                     MIN(size.max.x, Max(size.min.x, D.w2(), rect().w()));
                     MIN(size.max.y, Max(size.min.y, D.h2(), rect().h()));
                     if(resize_mask&DIRF_RIGHT)Clamp(r.max.x+=Ms.d().x, r.min.x+size.min.x, r.min.x+size.max.x);else if(resize_mask&DIRF_LEFT)Clamp(r.min.x+=Ms.d().x, r.max.x-size.max.x, r.max.x-size.min.x);
                     if(resize_mask&DIRF_DOWN )Clamp(r.min.y+=Ms.d().y, r.max.y-size.max.y, r.max.y-size.min.y);else if(resize_mask&DIRF_UP  )Clamp(r.max.y+=Ms.d().y, r.min.y+size.min.y, r.min.y+size.max.y);
                     rect(r);
                  }
               }
            }
         }
         if(flag&(WIN_MOVABLE|WIN_RESIZABLE))
         {
            Vec2 pos=0, delta=0; Int n=0;
                                               if(  Gui.ms()==this && Ms.b(0) && !_resize){pos+=Ms.pos()-Ms.dc(); delta+=Ms.dc(); n++;}
            REPA(Touches){Touch &t=Touches[i]; if(t.guiObj()==this && t.on( )            ){pos+= t.pos()- t.d (); delta+= t.d (); n++;}}
            if(n)
            {
               if(n>1 && (flag&WIN_RESIZABLE))
               {
                  pos/=n; // touch center before movement
                  Flt l =0, r =0, u =0, d =0; // left right up down movement
                  Int li=0, ri=0, ui=0, di=0; // number of touches on each side
                                                     if(  Gui.ms()==this && Ms.b(0) && !_resize){delta=(Ms.pos()-Ms.dc())-pos; UpdateStretch(delta.x, l, r, li, ri, Ms.dc().x); UpdateStretch(delta.y, d, u, di, ui, Ms.dc().y);}
                  REPA(Touches){Touch &t=Touches[i]; if(t.guiObj()==this && t.on( )            ){delta=( t.pos()- t.d ())-pos; UpdateStretch(delta.x, l, r, li, ri,  t.d ().x); UpdateStretch(delta.y, d, u, di, ui,  t.d ().y);}}
                  Rect rect=T.rect(), size=sizeLimit();
                  MIN(size.max.x, Max(size.min.x, D.w2(), rect.w()));
                  MIN(size.max.y, Max(size.min.y, D.h2(), rect.h()));
                  UpdateResize(rect.min.x, rect.max.x, li ? l/li : 0, ri ? r/ri : 0, size.min.x, size.max.x, flag&WIN_MOVABLE, resize_mask&DIRF_LEFT, resize_mask&DIRF_RIGHT);
                  UpdateResize(rect.min.y, rect.max.y, di ? d/di : 0, ui ? u/ui : 0, size.min.y, size.max.y, flag&WIN_MOVABLE, resize_mask&DIRF_DOWN, resize_mask&DIRF_UP   );
                  T.rect(rect);
               }else
               if((flag&WIN_MOVABLE) && delta.any())
               {
                  delta/=n;
                  if(have_client_rect)
                  {
                     Flt paddx=Min(WINDOW_PADD, rect().w()),
                         paddy=Min(WINDOW_PADD, rect().h());
                     Clamp(delta.x, client_rect.min.x+paddx-rect().max.x, client_rect.max.x-paddx-rect().min.x);
                     Clamp(delta.y, client_rect.min.y+paddy-rect().max.y, client_rect.max.y-paddy-rect().min.y);
                  }
                  move(delta);
               }
            }
         }
      }
    /*if((flag&WIN_IMMEDIATE) && !Gui.menu() && !Touches.elms() && !Ms.b(0) && !Ms.br(0))
      {
         if((flag&WIN_IMMEDIATE_ACT  ) && Gui.window()!=this &&  contains(Gui.ms()) && !_was_lit)  activate();else
         if((flag&WIN_IMMEDIATE_DEACT) && Gui.window()==this && !contains(Gui.ms()) &&  _was_lit)deactivate();
      }*/
    //AdjustValBool(_lit_hover,    lit(), Gui._time_d_fade_in, Gui._time_d_fade_out);
    //AdjustValBool(_lit_focus, active(), Gui._time_d_fade_in, Gui._time_d_fade_out);
      if(   lit())_lit_hover=1;else MAX(_lit_hover-=Gui._time_d_fade_out, 0);
      if(active())_lit_focus=1;else MAX(_lit_focus-=Gui._time_d_fade_out, 0);
      switch(_fade_type)
      {
         case FADE_IN : _fade_alpha+=Time.ad()*Gui.window_fade_in_speed ; if(_fade_alpha>1){_fade_alpha=1; _fade_type=FADE_NONE;        } setFinalAlpha(); break;
         case FADE_OUT: _fade_alpha-=Time.ad()*Gui.window_fade_out_speed; if(_fade_alpha<0){_fade_alpha=0; _fade_type=FADE_NONE; hide();} setFinalAlpha(); break;
      }
   }

   if(gpc_this.enabled)
   {
      _children.update(gpc_children);
      button[0].update(gpc_this);
      button[1].update(gpc_this);
      button[2].update(gpc_this);
   }
}
/******************************************************************************/
void Window::draw(C GuiPC &gpc)
{
   if(visible() && gpc.visible)
   {
      Bool  active=T.active(), transparent=(finalAlpha()<1-EPS_COL);
      GuiPC gpc_children(gpc, T), // first calculate the GuiPC for children which sets the offset per pixel aligned exactly at client rect top left corner
            gpc_aligned(gpc, true, active); // use 'active' for 'gpc_aligned' to draw buttons as half transparent when inactive
            gpc_aligned.offset=gpc_children.offset-_crect.lu(); // now based on that offset calculate the GuiPC for the Window, which is just as 'gpc' however with offset compatible with 'gpc_children'
      Rect  r=rect()+gpc_aligned.offset, ext_rect;

      if(ripple || transparent)
      {
         D.clip   (null);
         D.fxBegin();
         if(ripple)D.clearCol();else // clear entire screen for ripple as it may use various tex coordinates basing on the ripple intensity
         { // clear only the screen part that we're going to use
            ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
            extendedRect(ext_rect); ext_rect+=gpc_aligned.offset;
            Sh.clear(Vec4Zero, &Rect(ext_rect).extend(D.pixelToScreenSize())); // extend by 1 pixel to avoid tex filtering issues
            D .alpha(alpha);
         }
      }else
      {
         D.clip(gpc.clip);
      }

      GuiSkin *skin=getSkin();
      if(skin)
      {
         if(Panel *panel=(barVisible() ? (active ? skin->window.active() : skin->window.normal()) : (active ? skin->window.active_no_bar() : skin->window.normal_no_bar())))
            panel->draw(active ? skin->window.active_color : skin->window.normal_color, r);

         if(barVisible() && title.is())
            if(TextStyle *text_style=(active ? skin->window.active_text_style() : skin->window.normal_text_style()))
         {
            Flt  bar_height=barHeight(), text_padd=bar_height*skin->window.text_padd;
            Rect text_rect(_crect.min.x+gpc_aligned.offset.x+text_padd, r.max.y-bar_height, _crect.max.x+gpc_aligned.offset.x-text_padd, r.max.y);
            TextStyleParams ts=*text_style; ts.size=bar_height*skin->window.text_size; ts.color=ColorAdd(ts.color, ColorBA(highlightHover()*0.11f, 0));
            // check if title overlaps the buttons
            if(Gui.windowButtonsRight())
            {
               if(ts.align.x<1-EPS) // if alignment is not to the right
               {
                  Flt text_width=ts.textWidth(title),
                     x_align_mul=ts.align.x*0.5f-0.5f,
                           right=text_rect.lerpX(-x_align_mul) + text_width*x_align_mul + text_width,
                           max_x;
                  if(button[0].visible())max_x=button[0].rect().min.x+gpc_aligned.offset.x-text_padd;else
                  if(button[1].visible())max_x=button[1].rect().min.x+gpc_aligned.offset.x-text_padd;else
                  if(button[2].visible())max_x=button[2].rect().min.x+gpc_aligned.offset.x-text_padd;else
                                         max_x=text_rect.max.x;
                  if(right>max_x)
                  {
                     text_rect.max.x=max_x;
                     if(text_width>text_rect.w())ts.align.x=1; // if text width is bigger than available space, then align to the right
                  }
               }
            }else
            {
               Flt min_x;
               if(button[0].visible())min_x=button[0].rect().max.x+gpc_aligned.offset.x+text_padd;else
               if(button[1].visible())min_x=button[1].rect().max.x+gpc_aligned.offset.x+text_padd;else
               if(button[2].visible())min_x=button[2].rect().max.x+gpc_aligned.offset.x+text_padd;else
                                      min_x=text_rect.min.x;
               if(ts.align.x<1-EPS) // if alignment is not to the right
               {
                  Flt text_width=ts.textWidth(title),
                     x_align_mul=ts.align.x*0.5f-0.5f,
                            left=text_rect.lerpX(-x_align_mul) + text_width*x_align_mul;
                  if(left<min_x)
                  {
                     text_rect.min.x=min_x;
                     if(text_width>text_rect.w())ts.align.x=1; // if text width is bigger than available space, then align to the right
                  }
               }else // if alignment is to the right
               {
                  text_rect.min.x=min_x;
               }
            }
            D.clip(text_rect&gpc.clip);
            D.text(ts, text_rect, title);
         }
      }
      _children.draw(gpc_children);
      button[0].draw(gpc_aligned );
      button[1].draw(gpc_aligned );
      button[2].draw(gpc_aligned );

      if(ripple || transparent)
         if(C ImageRTPtr &rt=D.fxEnd())
      {
         D.clip(gpc.clip);
         if(ripple     )ripple->draw(*rt, D.rect());else
         if(transparent)
         {
            Color      color=ColorMul(finalAlpha());
            ALPHA_MODE alpha=D.alpha(ALPHA_MERGE);
            if(Equal(Gui.window_fade_scale, 1))Sh .draw    (*rt, color, Vec4Zero, &ext_rect);
            else                               rt->drawPart(color, TRANSPARENT, Rect_C(ext_rect.center(), ext_rect.size()*Lerp(Mid(Gui.window_fade_scale, 0.0f, 2.0f), 1.0f, fadeAlpha())), D.screenToUV(ext_rect));
            D.alpha(alpha);
         }
      }

      if(_resize && Gui.ms()==this) // check for Gui.ms as well in case we've lost focus
      {
         Image *image=null;
         switch(_resize)
         {
            case DIRF_LEFT           : image=Gui.image_resize_x (); break;
            case DIRF_RIGHT          : image=Gui.image_resize_x (); break;
            case DIRF_UP             : image=Gui.image_resize_y (); break;
            case DIRF_DOWN           : image=Gui.image_resize_y (); break;
            case DIRF_LEFT |DIRF_UP  : image=Gui.image_resize_lu(); break;
            case DIRF_RIGHT|DIRF_UP  : image=Gui.image_resize_ru(); break;
            case DIRF_LEFT |DIRF_DOWN: image=Gui.image_resize_ld(); break;
            case DIRF_RIGHT|DIRF_DOWN: image=Gui.image_resize_rd(); break;
         }
         if(image)
         {
            D.clip(gpc.clip);
            Vec2 pos=Ms.pos(), size(MOUSE_IMAGE_SIZE*image->aspect(), MOUSE_IMAGE_SIZE); pos+=Vec2(-size.x, size.y)*0.5f;
            image->draw(Rect_LU(D.screenAlignedToPixel(pos), size));
         }
      }
   }
}
/******************************************************************************/
Bool Window::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.putMulti(Byte(5), flag, resize_mask, _level, _crect, _bar_visible, _fade_type, _fade_alpha, _alpha)<<title; // version
      f.putAsset(_skin.id());
      if(button[0].save(f, path))
      if(button[1].save(f, path))
      if(button[2].save(f, path))
         return f.ok();
   }
   return false;
}
Bool Window::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 5:
      {
         f.getMulti(flag, resize_mask, _level, _crect, _bar_visible, _fade_type, _fade_alpha, _alpha)>>title;
        _skin.require(f.getAssetID(), path);
         if(button[0].load(f, path))
         if(button[1].load(f, path))
         if(button[2].load(f, path))
            if(f.ok()){setFinalAlpha(); setParams(); return true;}
      }break;

      case 4:
      {
         f.getMulti(flag, resize_mask, _level, _crect, _bar_visible, _fade_type, _fade_alpha, _alpha)._getStr2(title);
        _skin.require(f.getAssetID(), path);
         if(button[0].load(f, path))
         if(button[1].load(f, path))
         if(button[2].load(f, path))
            if(f.ok()){setFinalAlpha(); setParams(); return true;}
      }break;

      case 3:
      {
         f>>flag>>resize_mask>>_level>>_crect>>_bar_visible>>_fade_type>>_fade_alpha; f._getStr2(title); _alpha=1;
        _skin.require(f._getAsset(), path);
         if(button[0].load(f, path))
         if(button[1].load(f, path))
         if(button[2].load(f, path))
            if(f.ok()){setFinalAlpha(); setParams(); return true;}
      }break;

      case 2:
      {
         f>>flag>>_level>>_crect; _bar_visible=!f.getBool(); f>>_fade_type; f.skip(4); f>>_fade_alpha; f.skip(4); _alpha=1;
         f._getStr(title); f._getStr();
         if(button[0].load(f, path))
         if(button[1].load(f, path))
         if(button[2].load(f, path))
            if(f.ok()){setFinalAlpha(); setParams(); return true;}
      }break;

      case 1:
      {
         f>>flag>>_level>>_crect; _bar_visible=!f.getBool(); f>>_fade_type; f.skip(4); f>>_fade_alpha; f.skip(4); _alpha=1;
         title=f._getStr16(); f._getStr16();
         if(button[0].load(f, path))
         if(button[1].load(f, path))
         if(button[2].load(f, path))
            if(f.ok()){setFinalAlpha(); setParams(); return true;}
      }break;

      case 0:
      {
         f>>flag>>_level>>_crect; _bar_visible=!f.getBool(); f>>_fade_type; f.skip(4); f>>_fade_alpha; f.skip(4); _alpha=1;
         title=f._getStr8(); f._getStr8();
         if(button[0].load(f, path))
         if(button[1].load(f, path))
         if(button[2].load(f, path))
            if(f.ok()){setFinalAlpha(); setParams(); return true;}
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// CLOSABLE WINDOW
/******************************************************************************/
void ClosableWindow::update(C GuiPC &gpc)
{
   super::update(gpc);
   if(Gui.window()==this && button[2].func()) // this is the active window and close button has a function assigned
   {
      if((Kb.kf(KB_ESC) && !Kb.k.ctrlCmd() && !Kb.k.shift()) || Kb.kf(KB_NAV_BACK) || (Ms.bp(2) && contains(Gui.ms()) && !Gui.menu()->contains(Gui.ms())))
      {
         Kb.eatKey(); Ms.eat(2); button[2].push(); // use "button.push" to trigger calling custom function assigned to that button
      }
   }
}
/******************************************************************************/
// MODAL WINDOW
/******************************************************************************/
GuiObj* ModalWindow::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(visible())
   {
      if(GuiObj *go=super::test(gpc, pos, mouse_wheel))return go;
      return &_background;
   }
   return null;
}
void ModalWindow::update(C GuiPC &gpc)
{
   super::update(gpc);
   if(visible())REPA(MT)if(MT.bp(i) && MT.guiObj(i)==&_background)fadeOut();
}
void ModalWindow::draw(C GuiPC &gpc)
{
   if(visible())
   {
      D.clip();
      D.rect().draw(ColorBA(0, finalAlpha()*0.5f));
      super::draw(gpc);
   }
}
/******************************************************************************/
// DIALOG
/******************************************************************************/
GuiObj* Dialog::Text2::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel) {return null;}

Dialog& Dialog::autoSize()
{
   Flt button_w=Gui.dialog_button_padd*buttons.elms(); if(buttons.elms())button_w+=Gui.dialog_button_margin*(buttons.elms()-1);
   FREPA(buttons)button_w+=buttons[i].textWidth(&Gui.dialog_button_height);

   Flt text_w=0, text_h=0;
   if(text().is())
   if(C TextStyle *ts=text.getTextStyle())
   if(Flt line_h=ts->lineHeight())
   {
      const Flt desired_aspect=2.5f, min_w=line_h*17, max_w=(D.w()-Gui.dialog_padd)*2; // min_w gives some tolerable minimum width based on a single line height
            Int lines=ts->textLines(text(), max_w, text.auto_line, &text_w);
      text_h=line_h*lines;
      if(text_w>min_w)
      {
         AUTO_LINE_MODE auto_line=text.auto_line; if(auto_line==AUTO_LINE_SPACE_SPLIT || auto_line==AUTO_LINE_SPLIT)auto_line=AUTO_LINE_SPACE; // don't split words in next attempts, attempt above can split because we've set max_w as entire screen, however next attempts will use less
         Flt aspect=text_w/text_h;
         REP(3) // 3 attempts
         {
            Flt multiplier=Sqrt(desired_aspect/aspect), // need to apply 'Sqrt' because this affects both width and height
                test_w=Mid(text_w*multiplier, min_w, max_w);
            Int test_lines=ts->textLines(text(), test_w, auto_line, &test_w);
            Flt test_h=line_h*test_lines,
                test_aspect=test_w/test_h;
            if(AbsScale(test_aspect, desired_aspect)<AbsScale(aspect, desired_aspect)) // if 'test_aspect' is closer to 'desired_aspect'
            {
               text_w=test_w;
               text_h=test_h;
               aspect=test_aspect;
            }else break;
         }
      }
   }

   Flt  max_w=Max(button_w, text_w);
   Vec2 size(Gui.dialog_padd+max_w+Gui.dialog_padd, Gui.dialog_padd+text_h+Gui.dialog_padd+Gui.dialog_button_height+Gui.dialog_padd);
   rect(Rect_C(Vec2(0), size+defaultInnerPaddingSize()));
   text.rect(Rect_U(size.x/2, -Gui.dialog_padd, text_w, text_h));

   Flt x=(size.x-button_w)/2, y=text.rect().min.y-Gui.dialog_padd;
   FREPA(buttons)
   {
      Button &button=buttons[i];
      Flt w=button.textWidth(&Gui.dialog_button_height)+Gui.dialog_button_padd;
      button.rect(Rect_LU(x, y, w, Gui.dialog_button_height));
      x+=w+Gui.dialog_button_margin;
   }
   return T;
}
Dialog& Dialog::set(C Str &title, C Str &text, C MemPtr<Str> &buttons, C TextStylePtr &text_style)
{
   super::setTitle(title).barVisible(title.is());
   T+=T.text.create(text, text_style); T.text.auto_line=AUTO_LINE_SPACE_SPLIT;

   T.buttons.setNum(buttons.elms());
   FREPA(T.buttons)T+=T.buttons[i].create(buttons[i]);

   return autoSize();
}
Dialog& Dialog::create(C Str &title, C Str &text, C MemPtr<Str> &buttons, C TextStylePtr &text_style)
{
   super::create();
   return set(title, text, buttons, text_style);
}
/******************************************************************************/
}
/******************************************************************************/
