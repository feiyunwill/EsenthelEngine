/******************************************************************************/
enum BRUSH_SHAPE
{
   BS_CIRCLE,
   BS_SQUARE,
}
/******************************************************************************/
class BrushClass : Window // Brush (Size + Speed + Soft + Image)
{
   class Slope : GuiCustom
   {
      static flt  SlopeF    (flt step           ) {return Sqr(step);}
      static flt  SlopeX    (flt x, C Rect &rect) {return rect.lerpX(x);}
      static flt  SlopeY    (flt y, C Rect &rect) {return rect.lerpY(Lerp(0.2, 1.0, y));}
      static void PushedSide(Slope &slope       ) {slope.side^=1; slope.side_b.text=(slope.side ? "<" : ">");}

      flt    val =0.5;
      bool   side=false;
      Button side_b;

      flt cos()C {return 1-Cos(val*PI_2);}

      virtual void update(C GuiPC &gpc)override
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
      virtual void draw(C GuiPC &gpc)override
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
               flt s0= i   /8.0,
                   s1=(i+1)/8.0,
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
   }

   bool      alt_rotate=true;
   TextBlack ts;
   Window    slope_window, image_window;
   Button    bsize;
   Text      tsize, tspeed, tsoft, tangle;
   Slider    ssize, sspeed, ssoft, sangle;
   ComboBox  cpattern;
   Tabs      shape;
   Button    slope_b, image, pattern;
   ImageSkin image_image;
   WindowIO  image_io;
   flt       rotate_cos, rotate_sin, rotate_scale, pattern_scale;
   Image     image_sw, image_hw;
   Str       image_name;
   Slope     slope;

   static void SetImage   (C Str &file, BrushClass &brush) {brush.setImage(file);}
   static void ImageWindow(             BrushClass &brush) {brush.image_window.visible(brush.image  ());}
   static void SlopeWindow(             BrushClass &brush) {brush.slope_window.visible(brush.slope_b());}

   static cchar8 *PatternText[]=
   {
      "1x",
      "2x",
      "4x",
      "8x",
   };

   // manage
   BrushClass& create(GuiObj &parent, C Vec2 &rd)
   {
      ts.reset().size*=0.8;
 parent+=super .create(Rect_RD(rd, 0.40, 0.29), MLTC(u"Brush Parameters", PL, u"Parametry Pędzla", DE, u"Pinsel Eigenschaften", RU, u"Параметры кисти", PO, u"Parâmetros do Pincel", CN, u"刷子参数"));
      T+=shape .create(Rect_L(0.02, -0.04, 0.125, 0.045), 0, (cchar8**)null, 2).layout(TABS_HORIZONTAL).valid(true).set(0).desc(MLTC(u"Brush Shape", PL, u"Kształt pędzla", RU, u"Фигура кисти", CN, u"刷子形状")); shape.tab(0).image="Gui/Misc/circle.img"; shape.tab(1).image="Gui/Misc/square.img";
      T+=tsize .create(Vec2  (0.08, -0.10), MLTC(u"Size" , PL, u"Rozmiar" , DE, u"Größe", RU, u"Размер"  , PO, u"Tamanho"   ), &ts); T+=ssize .create(Rect_R(clientWidth()-0.02, -0.10, 0.20, 0.05), 0.5).desc(MLT(S+"Change size with: "+Kb.ctrlCmdName()+" + MouseWheel"        , PL,S+"Zmień Rozmiar przy pomocy: "+Kb.ctrlCmdName()+" + MouseWheel"         , DE,"Größe ändern mit: Strg + Mausrad"           , RU,S+"Чтобы изменить размер нажмите: "+Kb.ctrlCmdName()+" + Колесо прокрутки мыши"          , PO,S+"Mudar o tamanho com: "+Kb.ctrlCmdName()+" + Scroll do Rato"       , CN,"改变大小:Ctrl键+鼠标滚轮"));
      T+=tspeed.create(Vec2  (0.08, -0.15), MLTC(u"Speed", PL, u"Prędkość", DE, u"Speed", RU, u"Скорость", PO, u"Velocidade"), &ts); T+=sspeed.create(Rect_R(clientWidth()-0.02, -0.15, 0.20, 0.05), 0.5).desc(MLT(S+"Change speed with: Shift + MouseWheel"      , PL,"Zmień Prędkość przy pomocy: Shift + MouseWheel"       , DE,"Geschwindigkeit ändern mit: Shift + Mausrad", RU,"Чтобы изменить скорость нажмите: Shift + Колесо прокрутки мыши"       , PO,"Mudar a velocidade com: Shift + Scroll do Rato"   , CN,"改变速度:Shift键+鼠标滚轮"));
      T+=tsoft .create(Vec2  (0.08, -0.20), MLTC(u"Soft" , PL, u"Miękkość", DE, u"Soft" , RU, u"Мягкость", PO, u"Suavidade" ), &ts); T+=ssoft .create(Rect_R(clientWidth()-0.02, -0.20, 0.20, 0.05), 1.0).desc(MLT(S+"Change soft with: "+Kb.ctrlCmdName()+" + Shift + MouseWheel", PL,S+"Zmień Miękkość przy pomocy: "+Kb.ctrlCmdName()+" + Shift + MouseWheel", DE,"Soft ändern mit: Strg + Shift + Mausrad"    , RU,S+"Чтобы изменить мягкость нажмите: "+Kb.ctrlCmdName()+" + Shift + Колесо прокрутки мыши", PO, S+"Mudar a dureza com: "+Kb.ctrlCmdName()+" + Shift + Scroll do Rato", CN,"改变柔软度:Ctrl键+Shift键+鼠标滚轮"));
      T+=bsize .create(Rect_C(0.08, -0.10, (App.lang()==EN ? 0.13 : 0.14), 0.045), tsize()).focusable(false); bsize.desc(MLTC(u"Scale size proportionally to camera distance", PL, u"Skaluj rozmiar proporcjonalnie do odległości kamery"    , DE, u"Größe propertional zum Kamera Abstand skalieren", RU, u"Размер масштаба кисти пропорционально растоянию до камеры", PO, u"Escalar proporcionalmente ŕ distancia da camera", CN, u"按比例缩放大小到摄像机的距离")); bsize.mode=BUTTON_TOGGLE; bsize.set(true);

      T+=slope_b.create(Rect_L (shape  .rect().right()+Vec2(0.01, 0), 0.11, 0.05), "Slope").func(SlopeWindow, T).focusable(false).desc("Make brush to be affected by terrain slope"); slope_b.mode=BUTTON_TOGGLE;
      T+=image  .create(Rect_LU(slope_b.rect().ru(),                  0.12, 0.05), MLTC(u"Image", CN, u"图片")).func(ImageWindow, T).focusable(false).desc(MLTC(u"Use source Image as brush modifier", PL, u"Użyj źródłowego obrazka (Image) jako modyfikator pędzla", DE, u"Bild als Pinsel Modifier verwenden", RU, u"Использовать изображение в качестве модификатора кисти", PO, u"Usar uma imagem como modificador do Pincel", CN, u"将源图片用于刷子")); image.mode=BUTTON_TOGGLE;

      skin(&NoShadowSkin);

parent+=slope_window.create(Rect_RD(rect().ld()-Vec2(0.03, 0), 0.22, 0.13), "Slope").barVisible(false).hide(); slope_window.skin(skin()); FlagDisable(slope_window.flag, WIN_MOVABLE);
        slope_window+=slope.create(Rect_LU(0.02, -0.02, 0.18, 0.09)).desc(MLTC(u"Slope limiter\nEdit with LMB and RMB", PL, u"Granica krzywizny terenu\nEdytuj przy pomocy LMB oraz RMB", DE, u"Hang Limiter\nMit LMB und RMB ändern", RU, u"Наклонный ограничитель\nРедактирование с помощью LMB и RMB кнопками мыши", PO, u"Limitar o declive\nEditar com Botăo Direito do Rato e Botăo Esquerdo do Rato", CN, u"斜率控制\n用鼠标左右键编辑"));
      if(SupportedTouches())slope_window+=slope.side_b.create(Rect_LU(slope.rect().lu(), 0.04, 0.04), ">").func(Slope.PushedSide, slope).focusable(false).desc("Change side of the slop limit");

            parent+=image_window.create(Rect_RD(rect().ru()+Vec2(0, 0.03), 0.3, 0.415), MLTC(u"Image", PL, u"Obraz", DE, u"Bild", RU, u"Изобращение", PO, u"Imagem", CN, u"图片")).barVisible(false).hide(); image_window.skin(skin()); FlagDisable(image_window.flag, WIN_MOVABLE);
      image_window+=image_image .create(Rect_LU(0, 0, 0.3, 0.3).extend(-0.02), &image_hw).desc(MLTC(u"Drag and Drop image here to use it", PL, u"Przeciągnij i upuść tutaj obrazek aby go użyć", DE, u"Legen Sie ein Bild mit Drag und Drop hier ab um es zu verwenden", RU, u"Перетащите изображение сюда, чтобыиспользовать его", PO, u"Agarrar e Largar a imagem aqui para a usar", CN, u"拖动图片到这里使用"));
      image_window+=tangle      .create(Vec2   (0.07, -0.32             ), MLTC(u"Angle"  , PL, u"Kąt" , DE, u"Winkel", RU, u"Угол"  , PO, u"Ângulo", CN, u"角度")); image_window+=sangle  .create(Rect_L(0.14, tangle .rect().centerY(), 0.14, 0.05), 0).desc(MLTC(u"Image rotation angle\nChange with: Alt+RMB", PL, u"Kąt obrotu obrazka\nZmień przy pomocy: Alt+RMB", DE, u"Bild Rotationswinkel\nÄndern mit: Alt + RMB", RU, u"Угол вращения изображения\nЧтобы изменить угол вращения нажмите: Alt+RMB", PO, u"Ângulo de rotaçăo da Imagem\nMuda com: Alt+Botăo Direito do Rato", CN, u"图片旋转:Alt键+鼠标右键"));
      image_window+=pattern     .create(Rect_C (0.07, -0.38, 0.11, 0.038), MLTC(u"Pattern", PL, u"Wzór", DE, u"Muster", RU, u"Шаблон", PO, u"Padrăo", CN, u"样式")); image_window+=cpattern.create(Rect_L(0.14, pattern.rect().centerY(), 0.14, 0.05), PatternText, Elms(PatternText)).set(0).desc(MLTC(u"Pattern Scale", PL, u"Skala wzoru", DE, u"Muster Skalierung", RU, u"Масштаб шаблона", PO, u"Escala Padrăo", CN, u"模板样式缩放")); pattern.mode=BUTTON_TOGGLE; pattern.desc(MLTC(u"Apply image using a repeating pattern", PL, u"Nałóż obrazek jako powtarzający się wzorzec"));

      image_io.create(S, S, S, SetImage, SetImage, T).ext(SUPPORTED_IMAGE_EXT, "image");

      setImage("Assets/Brushes/default.png");
      return T;
   }

   // operations
   void setImage(C Str &name)
   {
      image_name=name;
      image_hw.ImportTry(name,       -1, IMAGE_2D);
      image_sw.ImportTry(name, IMAGE_L8, IMAGE_SOFT, 1);
      SetPath(image_io, name);
   }
   void setChildWindows()
   {
      image_window.move(rect().ru()+Vec2(0, 0.02)-image_window.rect().rd());
      slope_window.move((image() ? image_window.rect().ld()-Vec2(0.01, 0) : rect().ru()+Vec2(0, 0.02))-slope_window.rect().rd());
   }

   virtual Window& hide()override {super.hide();            image_window.hide();              slope_window.hide();                    return T;}
   virtual Window& show()override {super.show(); if(image())image_window.show(); if(slope_b())slope_window.show(); setChildWindows(); return T;}

   virtual bool hasMsWheelFocus()C {return false;}

   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         setChildWindows();
         if(Ms.wheel() && (Gui.wheel()==Gui.desktop() || !Gui.wheel() || hasMsWheelFocus()))
         {
            if(Kb.ctrlCmd() && Kb.shift())ssoft .set(ssoft ()+Ms.wheel()/ 8.0);else
            if(Kb.ctrlCmd()              )ssize .set(ssize ()+Ms.wheel()/12.0);else
            if(Kb.shift  ()              )sspeed.set(sspeed()+Ms.wheel()/ 8.0);
         }
         if(alt_rotate && Kb.alt() && Ms.b(1))
         {
            sangle.set(Frac(sangle()+Ms.d().sum()*0.5));
            Ms.freeze();
         }
         if(Gui.window()==&image_window)REPA(MT)if(MT.bp(i) && MT.guiObj(i)==&image_image)image_io.activate();

         CosSin(rotate_cos, rotate_sin, sangle()*PI2);
         rotate_scale=((shape()==BS_SQUARE) ? Vec2(1, 1).rotateCosSin(rotate_cos, rotate_sin).abs().max() : 1);

         pattern_scale=1.0/Pow(2.0, cpattern())/64;
      }
   }
   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(focus_obj==&image_image)
      {
         Str n; REPA(names)if(ExtType(GetExt(names[i]))==EXT_IMAGE){n=names[i]; break;}
         if(n.is())setImage(n);
      }
   }
}
/******************************************************************************/
