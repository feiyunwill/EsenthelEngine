/******************************************************************************/
class CalculatorClass : ClosableWindow
{
   TextLine expression;

   CalculatorClass& create(C Rect &rect)
   {
      super.create("Calculator").clientRect(rect);
      T+=expression.create(Rect(0.01, -clientHeight()+0.01, clientWidth()-0.01, -clientHeight()+0.01+0.05)); button[2].func(HideEditAct, SCAST(GuiObj, T)).show();
      return T;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(Gui.kb()==&expression)
      {
         CalcValue value; if(Kb.kf(KB_ENTER) && Calculate(value, expression())){expression.set(value.asText(-9)); expression.cursor(expression().length());}
      }
   }
   virtual void draw(C GuiPC &gpc)override
   {
      if(visible() && gpc.visible)
      {
         super.draw(gpc);
         D.clip(gpc.clip);

         CalcValue value; bool ok=Calculate(value, expression());
         TextStyleParams ts(true); ts.align.y=1;
         TextStyleParams tsb=ts;
         ts .size=0.040;
         tsb.size=0.055;
         Rect rect=clientRect(); rect.min.y+=expression.rect().max.y+clientHeight(); rect.extendX(-0.01); rect+=gpc.offset;

         if(!ok){tsb.align.y=0; D.text(tsb, rect.center(), S+"Error : "+CalcError);}else
         {
            D.text(tsb, rect.down(), value.asText(-9));

            if(value.type==CVAL_INT || value.type==CVAL_REAL)
            {
               int i;
               flt f;
               dbl d;
               if(value.type==CVAL_INT ){d=value.i; f=value.i; i=      value.i ;}else
               if(value.type==CVAL_REAL){d=value.r; f=value.r; i=Round(value.r);}

               ts.align.x=-1;
               D.text(ts, rect.max.x, rect.min.y+0.13, TextInt (i    ));
               D.text(ts, rect.max.x, rect.min.y+0.09, TextReal(f, -9));
               D.text(ts, rect.max.x, rect.min.y+0.05, TextReal(d, -9));

               ts.align.x=1;
               D.text(ts, rect.min.x, rect.min.y+0.13, "Int");
               D.text(ts, rect.min.x, rect.min.y+0.09, "Flt");
               D.text(ts, rect.min.x, rect.min.y+0.05, "Dbl");

                                  D.text(ts, rect.min.x+0.07, rect.min.y+0.13, TextHex((uint&)i, 8, 0, true));
                                  D.text(ts, rect.min.x+0.07, rect.min.y+0.09, TextHex((uint&)f, 8, 0, true));
               uint *x=(uint*)&d; D.text(ts, rect.min.x+0.07, rect.min.y+0.05, TextHex(    x[1], 8, 0, true)+TextHex(x[0], 8));
            }
         }
      }
   }
}
CalculatorClass Calculator;
/******************************************************************************/
