/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
CalculatorClass Calculator;
/******************************************************************************/

/******************************************************************************/
   CalculatorClass& CalculatorClass::create(C Rect &rect)
   {
      ::EE::Window::create("Calculator").clientRect(rect);
      T+=expression.create(Rect(0.01f, -clientHeight()+0.01f, clientWidth()-0.01f, -clientHeight()+0.01f+0.05f)); button[2].func(HideEditAct, SCAST(GuiObj, T)).show();
      return T;
   }
   void CalculatorClass::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(Gui.kb()==&expression)
      {
         CalcValue value; if(Kb.kf(KB_ENTER) && Calculate(value, expression())){expression.set(value.asText(-9)); expression.cursor(expression().length());}
      }
   }
   void CalculatorClass::draw(C GuiPC &gpc)
{
      if(visible() && gpc.visible)
      {
         ::EE::Window::draw(gpc);
         D.clip(gpc.clip);

         CalcValue value; bool ok=Calculate(value, expression());
         TextStyleParams ts(true); ts.align.y=1;
         TextStyleParams tsb=ts;
         ts .size=0.040f;
         tsb.size=0.055f;
         Rect rect=clientRect(); rect.min.y+=expression.rect().max.y+clientHeight(); rect.extendX(-0.01f); rect+=gpc.offset;

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
               D.text(ts, rect.max.x, rect.min.y+0.13f, TextInt (i    ));
               D.text(ts, rect.max.x, rect.min.y+0.09f, TextReal(f, -9));
               D.text(ts, rect.max.x, rect.min.y+0.05f, TextReal(d, -9));

               ts.align.x=1;
               D.text(ts, rect.min.x, rect.min.y+0.13f, "Int");
               D.text(ts, rect.min.x, rect.min.y+0.09f, "Flt");
               D.text(ts, rect.min.x, rect.min.y+0.05f, "Dbl");

                                  D.text(ts, rect.min.x+0.07f, rect.min.y+0.13f, TextHex((uint&)i, 8, 0, true));
                                  D.text(ts, rect.min.x+0.07f, rect.min.y+0.09f, TextHex((uint&)f, 8, 0, true));
               uint *x=(uint*)&d; D.text(ts, rect.min.x+0.07f, rect.min.y+0.05f, TextHex(    x[1], 8, 0, true)+TextHex(x[0], 8));
            }
         }
      }
   }
/******************************************************************************/
