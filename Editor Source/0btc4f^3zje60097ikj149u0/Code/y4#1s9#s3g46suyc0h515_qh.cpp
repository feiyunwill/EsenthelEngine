/******************************************************************************/
Str       str;
TextEdit  ed;
bool      ok;
CalcValue val;
/******************************************************************************/
void InitPre()
{
   EE_INIT(false, false);
   App.flag=APP_MINIMIZABLE;
   flt scale=D.screenH()/1080.0;
   D.mode(500*scale, 94*scale).aspectMode(ASPECT_X).shadowMapSize(0);
#if DEBUG
   Paks.add(EE_ENGINE_PATH);
#else
   Paks.add("Engine.pak");
#endif
}
bool Init()
{
   return true;
}
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC  ))return false;
   if(Kb.k (KB_ENTER)){str=val.asText(-9); ed.cur=str.length();}
   EditText(str, ed);

   CalcVar var[]={CalcVar("t"), CalcVar("dt"), CalcVar("Ms.x"), CalcVar("Ms.y")};
   var[0].value.r=Time.time();
   var[1].value.r=Time.d();
   var[2].value.r=Ms.pos().x;
   var[3].value.r=Ms.pos().y;

   ok=Calculate(val, str, var);
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);
   TextStyleParams ts(true), tsb=ts;
   ts .size=0.070;
   tsb.size=0.085;

   if(!ok)D.text(tsb, 0, 0, S+"Error : "+CalcError);else
   {
      D.text(tsb, 0, -D.h()+0.12, val.asText(-9));

      if(val.type==CVAL_INT || val.type==CVAL_REAL)
      {
         int i;
         flt f;
         dbl d;
         if(val.type==CVAL_INT ){d=val.i; f=val.i; i=      val.i ;}else
         if(val.type==CVAL_REAL){d=val.r; f=val.r; i=Round(val.r);}

         ts.align.x=-1;
         D.text(ts, 0.98, -D.h()+0.33, TextInt (i    ));
         D.text(ts, 0.98, -D.h()+0.26, TextReal(f, -9));
         D.text(ts, 0.98, -D.h()+0.19, TextReal(d, -9));

         ts.align.x=1;
         D.text(ts, -0.98, -D.h()+0.33, "Int");
         D.text(ts, -0.98, -D.h()+0.26, "Flt");
         D.text(ts, -0.98, -D.h()+0.19, "Dbl");

                          D.text(ts, -0.85, -D.h()+0.33, TextHex((U32&)i, 8, 0, true));
                          D.text(ts, -0.85, -D.h()+0.26, TextHex((U32&)f, 8, 0, true));
         U32 *x=(U32*)&d; D.text(ts, -0.85, -D.h()+0.19, TextHex(   x[1], 8, 0, true)+TextHex(x[0], 8));
      }
   }

   ts.align=1;
   ts.edit =&ed;
   D .text(ts, -D.w()+0.02, -D.h()+0.02, str);
}
/******************************************************************************/
