/******************************************************************************/
class ColorBrushClass : Window
{
   class Col : GuiCustom
   {
      Vec col;

      virtual void update(C GuiPC &gpc)override
      {
         int sel=uintptr(user);
         REPA(MT)if(MT.guiObj(i)==this)
         {
            if(MT.bd(i)){ColorBrush.set(sel); if(ColorBrush.sel>=0)ColorBrush.cp.activate();}else
            if(MT.bp(i)){ColorBrush.set((ColorBrush.sel==sel) ? -1 : sel);}
         }
      }
      virtual void draw(C GuiPC &gpc)override
      {
         if(gpc.visible && visible())
         {
            D.clip(gpc.clip);
            (rect()+gpc.offset).draw(col);
            (rect()+gpc.offset).draw((ColorBrush.sel==uintptr(user)) ? RED : Gui.borderColor(), false);
         }
      }
   }

   Col         col[3*4];
   int         sel=-1;
   ColorPicker cp;
   SyncLock    lock;

   void set(int i, SET_MODE mode=SET_DEFAULT)
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
   static void Changed(ColorBrushClass &cb)
   {
      if(InRange(cb.sel, cb.col))cb.col[cb.sel].col=cb.cp().xyz;
   }
   ColorBrushClass& create()
   {
      int X=4,
          Y=(Elms(col)+X-1)/X;
      flt b=0.01  ,
          e=0.005 ,
          s=0.0425;
      super.create("Color"/*MLT("Color Brush", PL,"Kolor", DE,"Farb Pinsel", RU,"Цвет кисти", PO,"Cor do Pincel")*/);
      clientRect(Rect_R(WorldEdit.rect().w(), -WorldEdit.rect().h()/2, b*2+(s+e)*X-e, b*2+(s+e)*Y-e));
   #if MOBILE
      pos(HB.rect().lu()-Vec2(0.05+rect().w(), 0)); // on mobile platforms with small screens move to top, left of heightbrush (so it won't be close to orientation cube)
   #endif
      FREPA(col)
      {
         int x=i%X,
             y=i/X;
         T+=col[i].create(Rect_LU(b+(s+e)*x, -b-(s+e)*y, s, s), ptr(i)).desc(
            MLT("Press LMB to select\nPress LMBx2 to edit",
             PL,"Naciśnij LMB aby wybrać\nNaciśnij LMBx2 aby edytować",
             DE,"LMB zum Auswählen drücken\nLMBx2 zum edit",
             RU,"Нажмите LMB чтобы выбрать\nНажмите LMBx2 чтобы править"));
      }

      // set default colors
      {
         int i=0;
         col[i++].col.set(1.0, 1.0, 1.0); col[i++].col.set(1.0, 0.8, 0.8); col[i++].col.set(0.8, 1.0, 0.8); col[i++].col.set(0.8, 0.8, 1.0);
         col[i++].col.set(0.8, 0.8, 0.8); col[i++].col.set(0.8, 0.6, 0.6); col[i++].col.set(0.6, 0.8, 0.6); col[i++].col.set(0.6, 0.6, 0.8);
         col[i++].col.set(0.6, 0.6, 0.6); col[i++].col.set(0.6, 0.4, 0.4); col[i++].col.set(0.4, 0.6, 0.4); col[i++].col.set(0.4, 0.4, 0.6);
      }

      cp.create(S).func(Changed, T).hide();
      return T;
   }

   virtual Window& hide()override {cp.hide(); return super.hide();}

   static void EditColor(Cell<Area> &cell, Vec &color, int thread_index)
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
                  if(Brush.slope.side)d*=Sat((slope-s)/0.15);
                  else                d*=Sat((s-slope)/0.15);
               }
               Vec src=hm.colorF(x, y);
                       hm.colorF(x, y, Lerp(src, color, Sat(d)));
            }
         }

         // build
         area.hmBuild();
      }
   }

   void update()
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
}
ColorBrushClass ColorBrush;
/******************************************************************************/
