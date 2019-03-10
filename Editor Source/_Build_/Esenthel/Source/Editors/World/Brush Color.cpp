/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ColorBrushClass ColorBrush;
/******************************************************************************/

/******************************************************************************/
      void ColorBrushClass::Col::update(C GuiPC &gpc)
{
         int sel=uintptr(user);
         REPA(MT)if(MT.guiObj(i)==this)
         {
            if(MT.bd(i)){ColorBrush.set(sel); if(ColorBrush.sel>=0)ColorBrush.cp.activate();}else
            if(MT.bp(i)){ColorBrush.set((ColorBrush.sel==sel) ? -1 : sel);}
         }
      }
      void ColorBrushClass::Col::draw(C GuiPC &gpc)
{
         if(gpc.visible && visible())
         {
            D.clip(gpc.clip);
            (rect()+gpc.offset).draw(col);
            (rect()+gpc.offset).draw((ColorBrush.sel==uintptr(user)) ? RED : Gui.borderColor(), false);
         }
      }
   void ColorBrushClass::set(int i, SET_MODE mode)
   {
      if(sel!=i)
      {
         sel=i;
         if(InRange(sel, col))
         {
            cp.set(Vec4(col[sel].col, 1), QUIET);
            if(cp.visible())cp.hide().show(); // force set old color
         }
         if(mode!=QUIET)SetHmOp((sel>=0) ? OP_HM_COLOR : OP_HM_NONE);
      }
   }
   void ColorBrushClass::Changed(ColorBrushClass &cb)
   {
      if(InRange(cb.sel, cb.col))cb.col[cb.sel].col=cb.cp().xyz;
   }
   ColorBrushClass& ColorBrushClass::create()
   {
      int X=4,
          Y=(Elms(col)+X-1)/X;
      flt b=0.01f  ,
          e=0.005f ,
          s=0.0425f;
      ::EE::Window::create("Color"/*MLT("Color Brush", PL,"Kolor", DE,"Farb Pinsel", RU,"Цвет кисти", PO,"Cor do Pincel")*/);
      clientRect(Rect_R(WorldEdit.rect().w(), -WorldEdit.rect().h()/2, b*2+(s+e)*X-e, b*2+(s+e)*Y-e));
   #if MOBILE
      pos(HB.rect().lu()-Vec2(0.05f+rect().w(), 0)); // on mobile platforms with small screens move to top, left of heightbrush (so it won't be close to orientation cube)
   #endif
      FREPA(col)
      {
         int x=i%X,
             y=i/X;
         T+=col[i].create(Rect_LU(b+(s+e)*x, -b-(s+e)*y, s, s), ptr(i)).desc(
            MLT("Press LMB to select\nPress LMBx2 to edit",
             PL,u"Naciśnij LMB aby wybrać\nNaciśnij LMBx2 aby edytować",
             DE,u"LMB zum Auswählen drücken\nLMBx2 zum edit",
             RU,u"Нажмите LMB чтобы выбрать\nНажмите LMBx2 чтобы править"));
      }

      // set default colors
      {
         int i=0;
         col[i++].col.set(1.0f, 1.0f, 1.0f); col[i++].col.set(1.0f, 0.8f, 0.8f); col[i++].col.set(0.8f, 1.0f, 0.8f); col[i++].col.set(0.8f, 0.8f, 1.0f);
         col[i++].col.set(0.8f, 0.8f, 0.8f); col[i++].col.set(0.8f, 0.6f, 0.6f); col[i++].col.set(0.6f, 0.8f, 0.6f); col[i++].col.set(0.6f, 0.6f, 0.8f);
         col[i++].col.set(0.6f, 0.6f, 0.6f); col[i++].col.set(0.6f, 0.4f, 0.4f); col[i++].col.set(0.4f, 0.6f, 0.4f); col[i++].col.set(0.4f, 0.4f, 0.6f);
      }

      cp.create(S).func(Changed, T).hide();
      return T;
   }
   Window& ColorBrushClass::hide(){cp.hide(); return ::EE::Window::hide();}
   void ColorBrushClass::EditColor(Cell<Area> &cell, Vec &color, int thread_index)
   {
      Area &area=cell();
      if(   area.hm && Brush.affected(cell.xy()))
         if(!Kb.alt()) // rotate image
      {
         ThreadMayUseGPUData();

         Heightmap2 &hm      =*area.hm;
         int         res     =WorldEdit.hmRes();
         flt         slope   =Brush.slope.cos();
         Vec2        area_pos=area.pos2D();

         area.setChangedColor();

         // edit pixels
         REPD(y, res)
         REPD(x, res)
         {
            Vec2 pos(x, y); pos*=WorldEdit.areaSize()/flt(res-1); pos+=area_pos; // (pos/res+cell)*UNIT == pos*UNIT/res + cell*UNIT
            if(flt d=Brush.powerTotal(pos))
            {
               d*=Brush.speed;
               if(Brush.slope_b())
               {
                  flt s=1-area.hmNormalNeighbor(x, y).y;
                  if(Brush.slope.side)d*=Sat((slope-s)/0.15f);
                  else                d*=Sat((s-slope)/0.15f);
               }
               Vec src=hm.colorF(x, y);
                       hm.colorF(x, y, Lerp(src, color, Sat(d)));
            }
         }

         // build
         area.hmBuild();
      }
   }
   void ColorBrushClass::update()
   {
      if(visibleFull())
      {
         // edit
         if(sel>=0)WorldEdit.grid.mtFunc(WorkerThreads, Brush.affectedAreas(1), EditColor, col[sel].col);

         // get the color
         if(WorldEdit.cur.valid() && InRange(sel, T.col) && Ms.bp(0))
         {
            Vec color; if(WorldEdit.hmColor(WorldEdit.cur.pos(), color))
            {
               T.col[sel].col=color;
               cp.setRGB(color);
            }
         }
      }
   }
ColorBrushClass::ColorBrushClass() : sel(-1) {}

/******************************************************************************/
