/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
MtrlBrushClass MtrlBrush;
/******************************************************************************/

/******************************************************************************/
   MtrlBrushClass::MtrlBrushClass() : sel(-1), scale_material_id(UIDZero) {REPAO(mtrl_id).zero();}
   void MtrlBrushClass::PushedScale(MtrlBrushClass &mb) {SetHmOp(mb.scale() ? OP_HM_MTRL_SCALE : OP_HM_NONE);}
   void MtrlBrushClass::PushedHole(MtrlBrushClass &mb) {SetHmOp(mb.hole () ? OP_HM_MTRL_HOLE  : OP_HM_NONE);}
   void MtrlBrushClass::PushedMax1(MtrlBrushClass &mb) {SetHmOp(mb.max1 () ? OP_HM_MTRL_MAX_1 : OP_HM_NONE);}
   void MtrlBrushClass::PushedMax2(MtrlBrushClass &mb) {SetHmOp(mb.max2 () ? OP_HM_MTRL_MAX_2 : OP_HM_NONE);}
   MtrlBrushClass& MtrlBrushClass::create()
   {
      flt s=0.090f,
          b=0.010f,
          e=0.005f,
          x=    b;
   #if MOBILE
      int     skip=5; // skip 5 materials for small screens
      Rect_LD rect(0, -WorldEdit.rect().h(), 1.585f-skip*(s+e), s+b*2);
   #else
      int    skip=0;
      Rect_D rect(WorldEdit.rect().w()/2-0.16f, -WorldEdit.rect().h(), 1.585f, s+b*2);
   #endif
      ::EE::Window::create("Material"/*MLTC(u"Material Brush", PL,u"Materiał", DE,u"Material Pinsel", RU,u"Материал кисти", PO,u"Pincel do Material")*/);
      clientRect(rect);
      FREP(Elms(image)-skip)
      {
         T+=image[i].create(Rect_LU(x, -b, s, s)).desc(MLTC(
               u"Drag and drop a material here\nPress LMB to select\nPress LMBx2 to edit", //\nPress RMB to change",
            PL,u"Przeciągnij i upuść tutaj materiał\nNaciśnij LMB aby wybrać\nNaciśnij LMBx2 aby edytować", //\nNaciśnij RMB aby zmienić",
            DE,u"LMB zum Auswählen drücken\nLMBx2 zum edit", //\nRMB zum Ändern drücken",
            RU,u"Нажмите LMB чтобы выбрать\nНажмите LMBx2 чтобы править", //\nНажмите RMB чтобы изменить",
          //PO,u"Pressione o Botăo Esquerdo do Rato para seleccionar\nPressione o Botăo Direito do Rato para mudar",
            CN,u"按鼠标左键为选择\n2x按鼠标左键为编辑\n按鼠标右键修改"));
         image[i].alpha_mode=ALPHA_NONE;
         x+=s+e;
      }
      x+=b-e;

      T+=scale.create(Rect_L(x, -clientHeight()*1/4, 0.11f, 0.04f), MLTC(u"Scale", PL, u"Skaluj", DE, u"Skaliere", RU, u"Масштаб", PO, u"Escala", CN, u"缩放")).func(PushedScale, T).focusable(false).desc(MLTC(
             u"Stretch Material's texture.\nTo change material's scale please push the \"Scale\" button and then press RMB on the terrain.\n\nWARNING:\nThis setting is per material.\nChanging the scale results in auto re-saving the Material file!",
         PL, u"Rozciągnij tekstury Materiału.\nAby rozciągnąć teksturę proszę nacisnąć przycisk \"Skaluj\" a następnie RMB na terenie.\n\nUWAGA:\nTe ustawienia są dokonywane dla każdego materiały oddzielnie.\nZmiana skali owocuje automatycznym zapisaniu pliku Materiału!",
         DE, u"Streckt die Material Textur.\nUm die Skalierung des Materials zu ändern drücken Sie bitte den \"Skaliere\" Button und anschließend RMB auf das Terrain.\n\nACHTUNG:\nDiese Einstellung ist pro Material.\nBeim ändern der Skalierung wird die Material Datei automatisch erneut gespeichert!",
         RU, u"Растягивание текстуры материала.\nЧтобы изменить масштаб материала, нажмите кнопку \"Масштаб\", а затем RMB на ландшафте.\n\nПРЕДУПРЕЖДЕНИЕ:\nЭта настройка на весь материал.\nПри изменение масштаба автоматически перезаписывается файл материала!",
         PO, u"Esticar a textura do Material.\nPara mudar a escala do material por favor carregue no botăo \"Escala\" e depois pressione o Botăo Direito do Rato no terreno.\n\nCUIDADO:\nEsta configuraçăo é por material.\nMudar os resultados de escala ao auto regravar o ficheiro do Material!",
         CN, u"材质缩放.\n修改材质的缩放率请按 \"缩放\" 按钮,然后用鼠标右键换到地形上.\n\n警告:\n这项设置是针对每个材质的.\n修改缩放率会自动保存到材质文件中!")); scale.mode=BUTTON_TOGGLE;
      T+=hole.create(Rect_L(x, -clientHeight()*3/4, 0.11f, 0.04f), MLTC(u"Hole", PL, u"Dziura")).func(PushedHole, T).focusable(false).desc("Make hole in the terrain"); hole.mode=BUTTON_TOGGLE; x+=0.12f;
      T+=max1.create(Rect_L(x, -clientHeight()*1/4, 0.11f, 0.04f), "Max 1").func(PushedMax1, T).focusable(false).desc(MLTC(u"Limit the number of materials to a maximum of 1.\nDecreasing the number of materials increases rendering performance.", PL, u"Zmniejsz liczbę materiałów w terenie do maksymalnie 1.\nZmniejszenie liczby materiałów zwiększa wydajność rysowania.", DE, u"Limitiert die Anzahl der Materials auf ein Maximum von 1.\nDas Verringern der Anzahl der Materials erhöht die Render Performance.", RU, u"Ограничить максимальное число материалов одним.\nУменьшение числа материалов увеличивает производительность рендеринга.", PO, u"Limita o número de materiais para o máximo de 1.\nDiminuindo o número de materiais aumenta a velocidade de renderizaçăo.", CN, u"最多材质控制1\n减少材质数量会降低渲染效果")); max1.mode=BUTTON_TOGGLE;
      T+=max2.create(Rect_L(x, -clientHeight()*3/4, 0.11f, 0.04f), "Max 2").func(PushedMax2, T).focusable(false).desc(MLTC(u"Limit the number of materials to a maximum of 2.\nDecreasing the number of materials increases rendering performance.", PL, u"Zmniejsz liczbę materiałów w terenie do maksymalnie 2.\nZmniejszenie liczby materiałów zwiększa wydajność rysowania.", DE, u"Limitiert die Anzahl der Materials auf ein Maximum von 2.\nDas Verringern der Anzahl der Materials erhöht die Render Performance.", RU, u"Ограничить максимальное число материалов двумя.\nУменьшение числа материалов увеличивает производительность рендеринга.", PO, u"Limita o número de materiais para o máximo de 2.\nDiminuindo o número de materiais aumenta a velocidade de renderizaçăo.", CN, u"最多材质控制2\n减少材质数量会降低渲染效果")); max2.mode=BUTTON_TOGGLE;

      setColors();
      return T;
   }
   void MtrlBrushClass::validateIcons() {REPA(mtrl_id)setMaterial(i, mtrl_id[i]);}
   MaterialPtr MtrlBrushClass::getMaterial() {return InRange(sel, mtrl) ? mtrl[sel] : MaterialPtr();}
   void        MtrlBrushClass::setMaterial(int i, C UID &mtrl_id)
   {
      if(InRange(i, Elms(T.mtrl)))
      {
         Elm *elm=Proj.findElm(mtrl_id); if(elm && elm->type!=ELM_MTRL)elm=null;

         if(InRange(i, Proj.mtrl_brush_id))Proj.mtrl_brush_id[i]=(elm ? elm->id : UIDZero);
         T.mtrl_id[i]      =(elm ? elm->id : UIDZero);
         T.mtrl   [i]      =Proj.gamePath(T.mtrl_id[i]);
         T.image  [i].color=MaterialColor(T.mtrl[i]);
         T.image  [i].set  (MaterialImage(T.mtrl[i]));
      }
   }
   void MtrlBrushClass::setColors()
   {
      REPAO(image).rect_color=((i==sel) ? RED : Gui.borderColor());
   }
   void MtrlBrushClass::set(int i, SET_MODE mode)
   {
      if(sel!=i)
      {
         sel=i;
         setColors();
         if(mode!=QUIET)SetHmOp((sel>=0) ? OP_HM_MTRL : OP_HM_NONE);
      }
   }
   void MtrlBrushClass::erasing(C UID &elm_id)
   {
      REPA(mtrl_id)if(mtrl_id[i]==elm_id)setMaterial(i, UIDZero);
   }
   void MtrlBrushClass::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))REPAD(slot, image)if(focus_obj==&image[slot])
      {
         FREPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_MTRL))
         {
            setMaterial(slot, elm->id);
            break;
         }
         break;
      }
   }
   void MtrlBrushClass::UpdateMaterial(Cell<Area> &cell, MaterialPtr &material, int thread_index)
   {
      Area &area=cell();
      if(   area.hm && Brush.affected(cell.xy()))
         if(!Kb.alt()) // rotate image
      {
         ThreadMayUseGPUData();

         Heightmap2 &hm      =*area.hm;
         int         res     =WorldEdit.hmRes();
         flt         size    =Brush.size,
                     slope   =Brush.slope.cos();
         Vec2        area_pos=area.pos2D();

         area.setChangedMtrl();

         // edit pixels
         REPD(y, res)
         REPD(x, res)
         {
            Vec2 p(x, y); p*=WorldEdit.areaSize()/flt(res-1); p+=area_pos; // (p/res+cell)*UNIT == p*UNIT/res + cell*UNIT
            if(flt d=Brush.powerTotal(p))
            {
               d*=Brush.speed;
               if(Brush.slope_b())
               {
                  flt s=1-area.hmNormalNeighbor(x, y).y;
                  if(Brush.slope.side)d*=Sat((slope-s)/0.15f);
                  else                d*=Sat((s-slope)/0.15f);
               }
               hm.addMaterial(x, y, material, d);
            }
         }

         // build
         area.hmBuild();
      }
   }
      int MtrlBrushClass::MtrlBlend::Compare(C MtrlBlend &a, C MtrlBlend &b) {return ::Compare(b.blend, a.blend);}
   void MtrlBrushClass::ReduceMaterial(Cell<Area> &cell, ptr user, int thread_index)
   {
      Area &area=cell();
      if(   area.hm && Brush.affected(cell.xy()))
         if(!Kb.alt()) // rotate image
      {
         ThreadMayUseGPUData();

         Heightmap2 &hm      =*area.hm;
         int         res     =WorldEdit.hmRes();
         flt         size    =Brush.size;
         Vec2        area_pos=area.pos2D();

         area.setChangedMtrl();

         // edit pixels
         REPD(y, res)
         REPD(x, res)
         {
            Vec2 p(x, y); p*=WorldEdit.areaSize()/flt(res-1); p+=area_pos; // (p/res+cell)*UNIT == p*UNIT/res + cell*UNIT
            if(flt d=Brush.powerTotal(p))
            {
               d*=Brush.speed;

               MtrlBlend mb[4]; Vec4 blend; hm.getMaterial(x, y, mb[0].mtrl, mb[1].mtrl, mb[2].mtrl, mb[3].mtrl, blend); REPAO(mb).blend=blend.c[i];
               Sort(mb, Elms(mb), MtrlBlend::Compare);

               REPA(mb)if(i>=(MtrlBrush.max1() ? 1 : 2)) // go from the end, to process least significant first
               {
                  flt &blend=mb[i].blend, decrease=Min(blend, d); // the value to decrease
                  blend-=decrease;
                  d    -=decrease;
               }

               REPAO(blend.c)=mb[i].blend; hm.setMaterial(x, y, mb[0].mtrl, mb[1].mtrl, mb[2].mtrl, mb[3].mtrl, blend);
            }
         }

         // build
         area.hmBuild();
      }
   }
   void MtrlBrushClass::DragMtrl(ptr slot_index, GuiObj *go, C Vec2 &screen_pos)
   {
      int i=intptr(slot_index); if(InRange(i, MtrlBrush.mtrl_id))if(MtrlBrush.mtrl_id[i].valid())
      {
         Memc<UID> elms; elms.add(MtrlBrush.mtrl_id[i]); Drag(elms, go, screen_pos);
      }
   }
   void MtrlBrushClass::update()
   {
      if(visibleFull())
      {
         // edit heightmap material
         {
            MaterialPtr mtrl_null;
            if(InRange(sel, mtrl) && mtrl[sel])WorldEdit.grid.mtFunc(WorkerThreads, Brush.affectedAreas(1), UpdateMaterial, mtrl[sel]);else
            if(hole()                         )WorldEdit.grid.mtFunc(WorkerThreads, Brush.affectedAreas(1), UpdateMaterial, mtrl_null);else
            if(max1() || max2()               )WorldEdit.grid.mtFunc(WorkerThreads, Brush.affectedAreas(1), ReduceMaterial           );
         }

         // get heightmap material
         if(WorldEdit.cur.valid() && sel>=0 && Ms.bp(0))
         {
            UID mtrl_id=WorldEdit.hmMtrl(WorldEdit.cur.pos(), MtrlBrush.getMaterial()).id();
            if( mtrl_id.valid())setMaterial(sel, mtrl_id);
         }

         // edit material scale
         if(scale() && !scale_material)
         {
            Cursor *cur=null; if(WorldEdit.cur.valid() && Ms.bp(1))cur=&WorldEdit.cur; REPA(WorldEdit.cur_touch)if(WorldEdit.cur_touch[i].valid() && WorldEdit.cur_touch[i].pd())cur=&WorldEdit.cur_touch[i];
            if(     cur){scale_material=WorldEdit.hmMtrl(cur->pos()); scale_material_id=scale_material.id();}
         }
         if(scale_material)
         {
            C Vec2 *d=((WorldEdit.cur.onViewport() && Ms.b(1)) ? &Ms.d() : null); if(!d)REPA(Touches)if(Touches[i].on() && WorldEdit.v4.getView(Touches[i].guiObj())){d=&Touches[i].ad(); break;}
            if(d)
            {
               Ms.freeze();
               flt tex_scale=scale_material->tex_scale;
               if(Ms.b(0) || Touches.elms()>=2)tex_scale=1;else Clamp(tex_scale*=ScaleFactor(d->sum()), 0.01f, 16.0f);
               if(scale_material->tex_scale!=tex_scale)
               {
                  scale_material->tex_scale=tex_scale;
                  scale_material->validate();
                  MtrlEdit.elmChanged(scale_material_id);
               }
            }else
            {
               if(scale_material_id.valid())
               {
                  if(Elm *mtrl=Proj.findElm(scale_material_id))if(ElmMaterial *mtrl_data=mtrl->mtrlData())
                  {
                     mtrl_data->newVer();
                     EditMaterial edit_mtrl; edit_mtrl.load(Proj.editPath(*mtrl));
                     edit_mtrl.tex_scale=scale_material->tex_scale;
                     edit_mtrl.tex_scale_time.getUTC();
                     Save(  edit_mtrl    , Proj.editPath(*mtrl));
                     Save(*scale_material, Proj.gamePath(*mtrl)); Proj.savedGame(*mtrl);
                     Server.setElmLong(scale_material_id);
                  }
                  scale_material_id.zero();
               }
               scale_material=null;
            }
         }

         // click on material list
         if(Gui.window()==this)
         {
            REPA(image)if(Gui.ms()==&image[i])
            {
               if(Ms.bd(0)                                                            ){set(                i); MtrlEdit.set(mtrl[i]);}else
               if(Ms.bp(0)                                                            ){set((i==sel) ? -1 : i); Gui.drag(DragMtrl, ptr(i));}/*else
               if(Ms.bp(1) || Ms.b(0) && Ms.life()>=LongPressTime && MtrlList.hidden()){set(                i); MtrlList.select(this);}*/
               break;
            }
            REPA(Touches)
            {
               Touch &touch=Touches[i]; REPA(image)if(touch.guiObj()==&image[i])
               {
                  if(touch.db()                                                    ){set(                i); MtrlEdit.set(mtrl[i]);}else
                  if(touch.pd()                                                    ){set((i==sel) ? -1 : i); Gui.drag(DragMtrl, ptr(i), &touch);}/*else
                  if(touch.on() && touch.life()>=LongPressTime && MtrlList.hidden()){set(                i); MtrlList.select(this);}*/
                  break;
               }
            }
         }
      }
   }
/******************************************************************************/
