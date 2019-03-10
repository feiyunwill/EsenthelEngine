/******************************************************************************/
class SelectionClass
{
   enum SEL_MODE // Selection Mode
   {
      SM_TOGGLE , // toggle  element in   selection
      SM_INCLUDE, // include element in   selection
      SM_SET    , // set     element as   selection
      SM_EXCLUDE, // exclude element from selection
      SM_KEEP   , // keep only highlighted elements in selection
      SM_NUM    ,
   }
   enum SEL_SCOPE // Selection Scope
   {
      SS_NONE     ,
      SS_HIGHLIGHT, // highlighted
      SS_SCREEN   , // visible on the screen
      SS_LOADED   , // all loaded
   }

   class TouchSelTabs : GuiCustom
   {
      int            sel =-1, // currently selected mode
                     keep=-1, // mode activated through tapping
                     keep_on_touch_start=-1; // value of 'keep' at the moment of new touch start
      Mems<ImagePtr> images;
      Mems<Rect    > rects;

      // get / set
      SEL_MODE selMode   (           ) {return InRange(sel, SM_NUM) ?  SEL_MODE(sel) : SM_SET;}
      int      operator()(           ) {return sel;}
    C Rect&    tabRect   (int i      ) {return rects[i];}
      int      tabIndex  (C Vec2 &pos)
      {
         if(rect().w()>=rect().h()){FREPA(rects)if(pos.x<=rects[i].max.x)return i;}
         else                      {FREPA(rects)if(pos.y>=rects[i].min.y)return i;}
         return rects.elms()-1;
      }
      void image(int i, C ImagePtr &image) {if(InRange(i, images))images[i]=image;}
      void clear() {keep=-1; keep_on_touch_start=sel;} // clear 'keep' and store 'keep_on_touch_start' as 'sel' for the case of (button press->do quick selection->release button, doing everything so quickly that tap is triggered on release) making the tap not enable 'keep'

      // manage
      TouchSelTabs& create(int elms)
      {
         images.setNum(elms);
         rects .setNum(elms);
         super.create();
         return T;
      }
            C Rect&         rect()C {return super.rect();}
      virtual TouchSelTabs& rect(C Rect &rect)override
      {
         super.rect(rect);
         flt a=1.4, b=1.0, c=0.8, sum=a+b*2+c*2, x=rect.min.x, y=rect.max.y, w=rect.w(), h=rect.h();
         if(w>=h)
         {
            a*=w/sum; b*=w/sum; c*=w/sum;
            rects[SM_TOGGLE ].setLU(x, y, c, h); x+=c;
            rects[SM_INCLUDE].setLU(x, y, b, h); x+=b;
            rects[SM_SET    ].setLU(x, y, a, h); x+=a;
            rects[SM_EXCLUDE].setLU(x, y, b, h); x+=b;
            rects[SM_KEEP   ].setLU(x, y, c, h); x+=c;
         }else
         {
            a*=h/sum; b*=h/sum; c*=h/sum;
            rects[SM_TOGGLE ].setLU(x, y, w, c); y-=c;
            rects[SM_INCLUDE].setLU(x, y, w, b); y-=b;
            rects[SM_SET    ].setLU(x, y, w, a); y-=a;
            rects[SM_EXCLUDE].setLU(x, y, w, b); y-=b;
            rects[SM_KEEP   ].setLU(x, y, w, c); y-=c;
         }
         return T;
      }
      virtual TouchSelTabs& move(C Vec2 &delta)override
      {
         super.move(delta);
         REPAO(rects)+=delta;
         return T;
      }

      // update / draw
      virtual void update(C GuiPC &gpc)override
      {
         sel=keep; REPA(MT)if(MT.guiObj(i)==this)
         {
            int t=tabIndex(MT.pos(i));
            if(MT.bp(i))keep_on_touch_start=keep;
            if(MT.b (i)){sel=t; keep=-1;}
            if(Touch *touch=MT.touch(i))if(touch.tapped())sel=keep=((t==keep_on_touch_start) ? -1 : t); // if tapped (disable if tapping previous element)
         }
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            D.clip(gpc.clip);
            /*if(Gui.image_button)REPA(T)
            {
               Color color(255, 255, 255, 70);
               Rect  &rect=rects[i], r=rect;
               if(sel==i)
               {
                  Swap(r.min.x, r.max.x);
                  Swap(r.min.y, r.max.y);
                  color=ColorBrightness(color, 0.75);
               }
               Gui.image_button->draw3x3(color, TRANSPARENT, r, 0.008);
               if(images[i])images[i]->drawFit(Color(255, 255, 255, 160), TRANSPARENT, rect);
            }*/
         }
      }
   }

   bool                 selecting=false, disable_selection=false, tsm_side=false, sel_changed=false;
   Vec                  sel_pos;
   Edit.Viewport4.View *sel_view=null;
   Memc<Obj*>           objs, highlighted;
   TouchSelTabs         tsm; // touch selection mode

   // get
   int  elms      (     )C {return  objs.elms();}
   Obj& operator[](int i)  {return *objs[i];}
   bool has       (C UID &world_obj_instance_id)C {if(world_obj_instance_id.valid())REPA(objs)if(objs[i].id==world_obj_instance_id)return true; return false;}

   bool selRect(Rect &rect)
   {
      if(selecting)
      {
         C Vec2 *pos=null; if(Edit.Viewport4.View *view=WorldEdit.v4.getView(Gui.ms()))if(view==sel_view)pos=&Ms.pos(); FREPA(Touches)if(Edit.Viewport4.View *view=WorldEdit.v4.getView(Touches[i].guiObj()))if(view==sel_view){pos=&Touches[i].pos(); break;} // accept all states, even releases, first set from mouse but override from any touch
         if(pos)
         {
            sel_view.setViewportCamera();
            if(PosToScreen(sel_pos, rect.min))
            {
               rect.max=*pos & D.viewRect();
               if(!rect.validX())Swap(rect.min.x, rect.max.x);
               if(!rect.validY())Swap(rect.min.y, rect.max.y);
               return true;
            }
         }
      }
      return false;
   }

   // manage
   void create()
   {
      Gui+=tsm.create(SM_NUM).visible(SupportedTouches());
      tsm.image(SM_SET    , "Gui/Misc/select.img");
      tsm.image(SM_TOGGLE , "Gui/Misc/select_xor.img");
      tsm.image(SM_INCLUDE, "Gui/Misc/select_add.img");
      tsm.image(SM_EXCLUDE, "Gui/Misc/select_sub.img");
      tsm.image(SM_KEEP   , "Gui/Misc/select_and.img");
   }
   void resize()
   {
      flt w=0.12, h=0.12;
      if(!tsm_side) // on the left side
      {
         flt l=h*SM_NUM, min_y=-D.h();
         tsm.rect(Rect_L(-D.w(), Max(0, min_y+l/2), w, l));
      }else
      {
         flt l=w*SM_NUM;
         if(D.w()>=D.h())
         {
            flt minx=-l/2;
            flt maxx=Min(minx+l, D.w());
            tsm.rect(Rect(minx, -D.h(), maxx, -D.h()+h)); // directly on bottom
         }else
         {
            tsm.rect(Rect_C(0, -D.h()*0.6, l, h)); // slightly above bottom
         }
      }
   }

   // operations
   void   select      (Obj &obj) {if(!obj.selected){obj.selected=true ; objs.add    (&obj); sel_changed=true;}}
   void deselect      (Obj &obj) {if( obj.selected){obj.selected=false; objs.exclude(&obj); sel_changed=true;}}
   void   selectToggle(Obj &obj) {if( obj.selected)deselect(obj);else select(obj);}

   void   highlight(Obj &obj) {if(!obj.highlighted){obj.highlighted=true ; highlighted.add    (&obj);}}
   void unhighlight(Obj &obj) {if( obj.highlighted){obj.highlighted=false; highlighted.exclude(&obj);}}

   SelectionClass& clearSelect   () {if(objs.elms())sel_changed=true; REPAO(objs       ).selected   =false; objs       .clear(); return T;}
   SelectionClass& clearHighlight() {                                 REPAO(highlighted).highlighted=false; highlighted.clear(); return T;}

   void highlightScreenObjs(Obj &base)
   {
      REPA(WorldEdit.obj_visible)
      {
         Obj &obj=*WorldEdit.obj_visible[i];
         if(ObjCenterVisible(obj) && ObjSimilar(&obj, &base))highlight(obj);
      }
   }
   void removed(Obj &obj) {deselect(obj); unhighlight(obj);}

   void setUndo() {REPAO(T).setUndo();}

   // update
   void update()
   {
      if(WorldEdit.mode()!=WorldView.OBJECT){clearHighlight(); disable_selection=true; return;} // 'disable_selection' so releasing button after will not force setting selection

      // get highlighted objects
      clearHighlight();
      if(selecting) // highlight by selection
      {
         bool center=WorldEdit.obj_center_points,
              matrix=WorldEdit.obj_matrix_points; if(!center && !matrix)matrix=true;
         Rect rect; if(selRect(rect))if(center || matrix)REPA(WorldEdit.obj_visible)
         {
            Vec2 p; Obj &obj=*WorldEdit.obj_visible[i];
            if(center && PosToScreen(obj.center(), p))if(Cuts(p, rect))highlight(obj);
            if(matrix && PosToScreen(obj.pos   (), p))if(Cuts(p, rect))highlight(obj);
         }
      }else
      if(WorldEdit.cur._obj && !ObjPaint.available())highlight(*WorldEdit.cur._obj); // highlight by mouse on the world
      if(ObjListClass.Elm *obj=ObjList.list.visToData(ObjList.list.lit))if(obj.obj)highlight(*obj.obj); // highlight by ObjList highlight

      // enable rectangle selection
      bool      clear_highlight=false;
      SEL_SCOPE sel_scope=SS_NONE;
      if(!selecting && !ObjPaint.available())
      {
         if(!disable_selection)REPA(MT)if(Edit.Viewport4.View *view=WorldEdit.v4.getView(MT.guiObj(i)))if(MT.touch(i) ? tsm()>=0 : true) // touches require tab selection to be enabled
         {
            if(MT.bp(i))
            {
               view.setViewportCamera();
               sel_pos =ScreenToPos(MT.pos(i), Max(10, ActiveCam.dist));
               sel_view=view;
            }
            if(MT.selecting(i) && MT.b(i))selecting=true;
         }
      }

      // end rectangle selection
      if(selecting)
         if(Ms.bp(1)){tsm.clear(); selecting=false; disable_selection=true;}else // cancel selection, 'disable_selection' so releasing later any button currently pressed will not force selection
      {
         FREPA(MT)if(Edit.Viewport4.View *view=WorldEdit.v4.getView(MT.guiObj(i)))if(view==sel_view)if(MT.br(i)) // find first on world, if released then end selection
         {
            tsm.clear(); selecting=false; disable_selection=true; sel_scope=SS_HIGHLIGHT; clear_highlight=true; // 'disable_selection' so any other touches currently pressed will not restart selection next frame
         }else if(MT.b(i))break; // if it's pressed then stop looking for other touches
      }

      // select objs on tap
      if(!disable_selection && !selecting && !sel_scope && !ObjPaint.available())
         REPA(MT)
            if(!MT.selecting(i) && (MT.br(i) || MT.bd(i)))
      {  // touching on world requires tab selection to be enabled
         // disable_selection on double clicks so that the release afterwards will not force selection
         // touch selection requires calculating highlighted objects because it's not yet available
         if(OnWorld(MT.guiObj(i)) && (MT.touch(i) ? tsm()>=0 : true))if(Cursor *cur=WorldEdit.findCur(MT.touch(i)))
         {
            tsm.clear(); disable_selection|=MT.bd(i); sel_scope=(MT.bd(i) ? SS_SCREEN : SS_HIGHLIGHT);
            if(MT.touch(i)){clearHighlight(); if(cur._obj)highlight(*cur._obj); clear_highlight=true;}
         }
      }
      bool on_world=false; REPA(MT)if((MT.b(i) || MT.br(i)) && OnWorld(MT.guiObj(i))){on_world=true; break;}
      if( !on_world)disable_selection=false; // if there are no presses and releases then re-enable selection

      // process selection
      if(sel_scope)
      {
         WorldEdit.obj_pos.apply();
         SEL_MODE sel=((Kb.ctrlCmd() && Kb.shift()) ? SM_KEEP : Kb.ctrlCmd() ? SM_TOGGLE : Kb.shift() ? SM_INCLUDE : Kb.alt() ? SM_EXCLUDE : tsm.selMode());
         switch(sel_scope)
         {
            case SS_HIGHLIGHT:
            {
               if(sel==SM_KEEP)
               {
                  REPA(objs)if(!objs[i].highlighted)deselect(*objs[i]);
               }else
               {
                  if(sel==SM_SET)clearSelect();
                  REPA(highlighted)switch(sel)
                  {
                     case SM_INCLUDE:
                     case SM_SET    :   select      (*highlighted[i]); break;
                     case SM_TOGGLE :   selectToggle(*highlighted[i]); break;
                     case SM_EXCLUDE: deselect      (*highlighted[i]); break;
                  }
               }
            }break;

            /*case SS_LOADED:
            {
               if(sel==SM_SET)clearSelect();
               REPA(WorldEdit.objs)
               {
                  Obj &obj=WorldEdit.objs[i];
                  switch(sel)
                  {
                     case SM_INCLUDE:
                     case SM_SET    : if(obj.type==unit_type                    )  select      (obj); break;
                     case SM_TOGGLE : if(obj.type==unit_type && !obj.highlighted)  selectToggle(obj); break; // skip the 'obj.highlighted' obj which was processed in the 1st tap
                     case SM_EXCLUDE: if(obj.type==unit_type                    )deselect      (obj); break;
                     case SM_KEEP   : if(obj.type!=unit_type                    )deselect      (obj); break;
                  }
               }
            }break; */

            case SS_SCREEN:
            {
               if(highlighted.elms())
               {
                  Obj &h=*highlighted[0];
                  highlightScreenObjs(h);
                  if(sel==SM_KEEP)
                  {
                     REPA(objs)if(!objs[i].highlighted)deselect(*objs[i]);
                  }else
                  {
                     if(sel==SM_SET)clearSelect();
                     REPA(highlighted)switch(sel)
                     {
                        case SM_INCLUDE:
                        case SM_SET    :                         select      (*highlighted[i]); break;
                        case SM_TOGGLE : if(&h!=highlighted[i])  selectToggle(*highlighted[i]); break; // skip the 'h' obj which was processed in the 1st tap
                        case SM_EXCLUDE:                       deselect      (*highlighted[i]); break;
                        case SM_KEEP   :                       deselect      (*highlighted[i]); break;
                     }
                  }
                  clearHighlight().highlight(h);
               }
            }break;
         }
      }
      if(clear_highlight)clearHighlight();
      if(sel_changed)
      {
         sel_changed=false;
         WorldEdit.objToGui();
      }
   }

   void draw(Edit.Viewport4.View &view)
   {
      // draw selection rectangle
      Rect rect; if(sel_view==&view && selRect(rect))
      {
         rect.draw(ColorAlpha(ColorBrightness(LitColor, 0.25), 0.1), true );
         rect.draw(ColorAlpha(ColorBrightness(LitColor, 0.80), 0.6), false);
      }
   }
   void drawSelLit()
   {
      REPAO(objs).drawSelected();
      REPA (highlighted)if(Obj *obj=highlighted[i])if(!obj.selected)obj.drawSelected();
   }

  ~SelectionClass() {REPA(objs)deselect(*objs[i]);}
}
SelectionClass Selection;
/******************************************************************************/
int Elms(SelectionClass              &sel) {return sel.       elms();}
int Elms(SelectionClass.TouchSelTabs &tst) {return tst.images.elms();}

bool ObjSimilar(Obj *a, Obj *b)
{
   if(a && b)
   {
      if(a.mesh_proper || b.mesh_proper)return a.mesh_proper==b.mesh_proper;
      if(a.params.base || b.params.base)return a.params.base==b.params.base;
   }
   return false;
}
bool ObjCenterVisible(Obj &obj)
{
   Vec2 p; if(PosToScreen(obj.center(), p))if(Cuts(p, D.viewRect()))return true;
   return false;
}
bool OnWorld(GuiObj *go)
{
   return WorldEdit.v4.getView(go)!=null;
}
/*Obj* NearestObj(GuiObj *go, C Vec2 &screen_pos, bool by_touch)
{
   Obj *nearest=null; flt dist2;
   if(Edit.Viewport4.View *view=WorldEdit.v4.getView(go))
   {
      view.setViewportCamera();
      const flt dist_tolerance=(by_touch ? 1.5 : 1.0), // touches are less precise, so we need bigger distance tolerance for the position
                 obj_dist2    =Sqr(dist_tolerance*0.04);
      REPA(WorldEdit.objs)
      {
         Obj &obj=WorldEdit.objs[i];
         Vec2 p; if(obj.visible && PosToScreen(obj.matrix.pos, p))
         {
            flt d2=Dist2(p, screen_pos);
            if(!nearest || d2<dist2)if(d2<=obj_dist2){nearest=&obj; dist2=d2;}
         }
      }
   }
   return nearest;
}
/******************************************************************************/
