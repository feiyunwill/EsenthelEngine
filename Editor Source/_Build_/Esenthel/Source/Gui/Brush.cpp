/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   cchar8 *BrushClass::PatternText[]=
   {
      "1x",
      "2x",
      "4x",
      "8x",
   };
/******************************************************************************/
      flt  BrushClass::Slope::SlopeF(flt step           ) {return Sqr(step);}
      flt  BrushClass::Slope::SlopeX(flt x, C Rect &rect) {return rect.lerpX(x);}
      flt  BrushClass::Slope::SlopeY(flt y, C Rect &rect) {return rect.lerpY(Lerp(0.2f, 1.0f, y));}
      void BrushClass::Slope::PushedSide(Slope &slope       ) {slope.side^=1; slope.side_b.text=(slope.side ? "<" : ">");}
      flt BrushClass::Slope::cos()C {return 1-Cos(val*PI_2);}
      void BrushClass::Slope::update(C GuiPC &gpc)
{
         if(Gui.ms()==this && (Ms.b(0) || Ms.b(1)))
         {
            val =LerpRS(rect().min.x, rect().max.x, Ms.pos().x-gpc.offset.x);
            side=Ms.b(0); side_b.text=(side ? "<" : ">");
         }
         REPA(Touches)if(Touches[i].guiObj()==this && Touches[i].on())
         {
            val=LerpRS(rect().min.x, rect().max.x, Touches[i].pos().x-gpc.offset.x);
         }
      }
      void BrushClass::Slope::draw(C GuiPC &gpc)
{
         if(gpc.visible && visible())
         {
            Rect r=rect()+gpc.offset;
            D.clip(gpc.clip);
          //if(Brush.slope_checkbox() && OpHm==OP_HM_MTRL)r.draw(ColorAlpha(RED, 0.15)); if active (hm mtrl, hm color, obj paint)
            r.draw(Gui.borderColor(), false);
            VI.color(GREY);
            FREP(8)
            {
               flt s0= i   /8.0f,
                   s1=(i+1)/8.0f,
                   x0=Lerp(side ? 0 : val, side ? val : 1, s0),
                   x1=Lerp(side ? 0 : val, side ? val : 1, s1),
                   yb=r.min.y              ,
                   y0=SlopeY(SlopeF(x0), r),
                   y1=SlopeY(SlopeF(x1), r);
                   x0=SlopeX(       x0 , r);
                   x1=SlopeX(       x1 , r);
               VI.quad(Vec2(x0, yb), Vec2(x0, y0), Vec2(x1, y1), Vec2(x1, yb));
            }
            VI.end();
         }
      }
   void BrushClass::SetImage(C Str &file, BrushClass &brush) {brush.setImage(file);}
   void BrushClass::ImageWindow(             BrushClass &brush) {brush.image_window.visible(brush.image  ());}
   void BrushClass::SlopeWindow(             BrushClass &brush) {brush.slope_window.visible(brush.slope_b());}
   BrushClass& BrushClass::create(GuiObj &parent, C Vec2 &rd)
   {
      ts.reset().size*=0.8f;
 parent+=::EE::Window::create(Rect_RD(rd, 0.40f, 0.29f), MLTC(u"Brush Parameters", PL, u"Parametry Pędzla", DE, u"Pinsel Eigenschaften", RU, u"Параметры кисти", PO, u"Parâmetros do Pincel", CN, u"刷子参数"));
      T+=shape .create(Rect_L(0.02f, -0.04f, 0.125f, 0.045f), 0, (cchar8**)null, 2).layout(TABS_HORIZONTAL).valid(true).set(0).desc(MLTC(u"Brush Shape", PL, u"Kształt pędzla", RU, u"Фигура кисти", CN, u"刷子形状")); shape.tab(0).image="Gui/Misc/circle.img"; shape.tab(1).image="Gui/Misc/square.img";
      T+=tsize .create(Vec2  (0.08f, -0.10f), MLTC(u"Size" , PL, u"Rozmiar" , DE, u"Größe", RU, u"Размер"  , PO, u"Tamanho"   ), &ts); T+=ssize .create(Rect_R(clientWidth()-0.02f, -0.10f, 0.20f, 0.05f), 0.5f).desc(MLT(S+"Change size with: "+Kb.ctrlCmdName()+" + MouseWheel"        , PL,S+u"Zmień Rozmiar przy pomocy: "+Kb.ctrlCmdName()+" + MouseWheel"         , DE,u"Größe ändern mit: Strg + Mausrad"           , RU,S+u"Чтобы изменить размер нажмите: "+Kb.ctrlCmdName()+u" + Колесо прокрутки мыши"          , PO,S+"Mudar o tamanho com: "+Kb.ctrlCmdName()+" + Scroll do Rato"       , CN,u"改变大小:Ctrl键+鼠标滚轮"));
      T+=tspeed.create(Vec2  (0.08f, -0.15f), MLTC(u"Speed", PL, u"Prędkość", DE, u"Speed", RU, u"Скорость", PO, u"Velocidade"), &ts); T+=sspeed.create(Rect_R(clientWidth()-0.02f, -0.15f, 0.20f, 0.05f), 0.5f).desc(MLT(S+"Change speed with: Shift + MouseWheel"      , PL,u"Zmień Prędkość przy pomocy: Shift + MouseWheel"       , DE,u"Geschwindigkeit ändern mit: Shift + Mausrad", RU,u"Чтобы изменить скорость нажмите: Shift + Колесо прокрутки мыши"       , PO,"Mudar a velocidade com: Shift + Scroll do Rato"   , CN,u"改变速度:Shift键+鼠标滚轮"));
      T+=tsoft .create(Vec2  (0.08f, -0.20f), MLTC(u"Soft" , PL, u"Miękkość", DE, u"Soft" , RU, u"Мягкость", PO, u"Suavidade" ), &ts); T+=ssoft .create(Rect_R(clientWidth()-0.02f, -0.20f, 0.20f, 0.05f), 1.0f).desc(MLT(S+"Change soft with: "+Kb.ctrlCmdName()+" + Shift + MouseWheel", PL,S+u"Zmień Miękkość przy pomocy: "+Kb.ctrlCmdName()+" + Shift + MouseWheel", DE,u"Soft ändern mit: Strg + Shift + Mausrad"    , RU,S+u"Чтобы изменить мягкость нажмите: "+Kb.ctrlCmdName()+u" + Shift + Колесо прокрутки мыши", PO, S+"Mudar a dureza com: "+Kb.ctrlCmdName()+" + Shift + Scroll do Rato", CN,u"改变柔软度:Ctrl键+Shift键+鼠标滚轮"));
      T+=bsize .create(Rect_C(0.08f, -0.10f, (App.lang()==EN ? 0.13f : 0.14f), 0.045f), tsize()).focusable(false); bsize.desc(MLTC(u"Scale size proportionally to camera distance", PL, u"Skaluj rozmiar proporcjonalnie do odległości kamery"    , DE, u"Größe propertional zum Kamera Abstand skalieren", RU, u"Размер масштаба кисти пропорционально растоянию до камеры", PO, u"Escalar proporcionalmente ŕ distancia da camera", CN, u"按比例缩放大小到摄像机的距离")); bsize.mode=BUTTON_TOGGLE; bsize.set(true);

      T+=slope_b.create(Rect_L (shape  .rect().right()+Vec2(0.01f, 0), 0.11f, 0.05f), "Slope").func(SlopeWindow, T).focusable(false).desc("Make brush to be affected by terrain slope"); slope_b.mode=BUTTON_TOGGLE;
      T+=image  .create(Rect_LU(slope_b.rect().ru(),                  0.12f, 0.05f), MLTC(u"Image", CN, u"图片")).func(ImageWindow, T).focusable(false).desc(MLTC(u"Use source Image as brush modifier", PL, u"Użyj źródłowego obrazka (Image) jako modyfikator pędzla", DE, u"Bild als Pinsel Modifier verwenden", RU, u"Использовать изображение в качестве модификатора кисти", PO, u"Usar uma imagem como modificador do Pincel", CN, u"将源图片用于刷子")); image.mode=BUTTON_TOGGLE;

      skin(&NoShadowSkin);

parent+=slope_window.create(Rect_RD(rect().ld()-Vec2(0.03f, 0), 0.22f, 0.13f), "Slope").barVisible(false).hide(); slope_window.skin(skin()); FlagDisable(slope_window.flag, WIN_MOVABLE);
        slope_window+=slope.create(Rect_LU(0.02f, -0.02f, 0.18f, 0.09f)).desc(MLTC(u"Slope limiter\nEdit with LMB and RMB", PL, u"Granica krzywizny terenu\nEdytuj przy pomocy LMB oraz RMB", DE, u"Hang Limiter\nMit LMB und RMB ändern", RU, u"Наклонный ограничитель\nРедактирование с помощью LMB и RMB кнопками мыши", PO, u"Limitar o declive\nEditar com Botăo Direito do Rato e Botăo Esquerdo do Rato", CN, u"斜率控制\n用鼠标左右键编辑"));
      if(SupportedTouches())slope_window+=slope.side_b.create(Rect_LU(slope.rect().lu(), 0.04f, 0.04f), ">").func(Slope::PushedSide, slope).focusable(false).desc("Change side of the slop limit");

            parent+=image_window.create(Rect_RD(rect().ru()+Vec2(0, 0.03f), 0.3f, 0.415f), MLTC(u"Image", PL, u"Obraz", DE, u"Bild", RU, u"Изобращение", PO, u"Imagem", CN, u"图片")).barVisible(false).hide(); image_window.skin(skin()); FlagDisable(image_window.flag, WIN_MOVABLE);
      image_window+=image_image .create(Rect_LU(0, 0, 0.3f, 0.3f).extend(-0.02f), &image_hw).desc(MLTC(u"Drag and Drop image here to use it", PL, u"Przeciągnij i upuść tutaj obrazek aby go użyć", DE, u"Legen Sie ein Bild mit Drag und Drop hier ab um es zu verwenden", RU, u"Перетащите изображение сюда, чтобыиспользовать его", PO, u"Agarrar e Largar a imagem aqui para a usar", CN, u"拖动图片到这里使用"));
      image_window+=tangle      .create(Vec2   (0.07f, -0.32f             ), MLTC(u"Angle"  , PL, u"Kąt" , DE, u"Winkel", RU, u"Угол"  , PO, u"Ângulo", CN, u"角度")); image_window+=sangle  .create(Rect_L(0.14f, tangle .rect().centerY(), 0.14f, 0.05f), 0).desc(MLTC(u"Image rotation angle\nChange with: Alt+RMB", PL, u"Kąt obrotu obrazka\nZmień przy pomocy: Alt+RMB", DE, u"Bild Rotationswinkel\nÄndern mit: Alt + RMB", RU, u"Угол вращения изображения\nЧтобы изменить угол вращения нажмите: Alt+RMB", PO, u"Ângulo de rotaçăo da Imagem\nMuda com: Alt+Botăo Direito do Rato", CN, u"图片旋转:Alt键+鼠标右键"));
      image_window+=pattern     .create(Rect_C (0.07f, -0.38f, 0.11f, 0.038f), MLTC(u"Pattern", PL, u"Wzór", DE, u"Muster", RU, u"Шаблон", PO, u"Padrăo", CN, u"样式")); image_window+=cpattern.create(Rect_L(0.14f, pattern.rect().centerY(), 0.14f, 0.05f), PatternText, Elms(PatternText)).set(0).desc(MLTC(u"Pattern Scale", PL, u"Skala wzoru", DE, u"Muster Skalierung", RU, u"Масштаб шаблона", PO, u"Escala Padrăo", CN, u"模板样式缩放")); pattern.mode=BUTTON_TOGGLE; pattern.desc(MLTC(u"Apply image using a repeating pattern", PL, u"Nałóż obrazek jako powtarzający się wzorzec"));

      image_io.create(S, S, S, SetImage, SetImage, T).ext(SUPPORTED_IMAGE_EXT, "image");

      setImage("Assets/Brushes/default.png");
      return T;
   }
   void BrushClass::setImage(C Str &name)
   {
      image_name=name;
      image_hw.ImportTry(name,       -1, IMAGE_2D);
      image_sw.ImportTry(name, IMAGE_L8, IMAGE_SOFT, 1);
      SetPath(image_io, name);
   }
   void BrushClass::setChildWindows()
   {
      image_window.move(rect().ru()+Vec2(0, 0.02f)-image_window.rect().rd());
      slope_window.move((image() ? image_window.rect().ld()-Vec2(0.01f, 0) : rect().ru()+Vec2(0, 0.02f))-slope_window.rect().rd());
   }
   Window& BrushClass::hide(){::EE::Window::hide();            image_window.hide();              slope_window.hide();                    return T;}
   Window& BrushClass::show(){::EE::Window::show(); if(image())image_window.show(); if(slope_b())slope_window.show(); setChildWindows(); return T;}
   bool BrushClass::hasMsWheelFocus()C {return false;}
   void BrushClass::update(C GuiPC &gpc)
{
      ::EE::Window::update(gpc);
      if(visible() && gpc.visible)
      {
         setChildWindows();
         if(Ms.wheel() && (Gui.wheel()==Gui.desktop() || !Gui.wheel() || hasMsWheelFocus()))
         {
            if(Kb.ctrlCmd() && Kb.shift())ssoft .set(ssoft ()+Ms.wheel()/ 8.0f);else
            if(Kb.ctrlCmd()              )ssize .set(ssize ()+Ms.wheel()/12.0f);else
            if(Kb.shift  ()              )sspeed.set(sspeed()+Ms.wheel()/ 8.0f);
         }
         if(alt_rotate && Kb.alt() && Ms.b(1))
         {
            sangle.set(Frac(sangle()+Ms.d().sum()*0.5f));
            Ms.freeze();
         }
         if(Gui.window()==&image_window)REPA(MT)if(MT.bp(i) && MT.guiObj(i)==&image_image)image_io.activate();

         CosSin(rotate_cos, rotate_sin, sangle()*PI2);
         rotate_scale=((shape()==BS_SQUARE) ? Vec2(1, 1).rotateCosSin(rotate_cos, rotate_sin).abs().max() : 1);

         pattern_scale=1.0f/Pow(2.0f, cpattern())/64;
      }
   }
   void BrushClass::drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(focus_obj==&image_image)
      {
         Str n; REPA(names)if(ExtType(GetExt(names[i]))==EXT_IMAGE){n=names[i]; break;}
         if(n.is())setImage(n);
      }
   }
BrushClass::BrushClass() : alt_rotate(true) {}

BrushClass::Slope::Slope() : val(0.5f), side(false) {}

/******************************************************************************/
