/******************************************************************************/
const int RES=1024;
/******************************************************************************/
Vec2     center;
dbl      size=1;
Vec2     val[RES];
Str      func[3];
TextEdit ed;
Int      edit;
CalcVar  var[2];
/******************************************************************************/
void InitPre()
{
   EE_INIT(false, false);
   App.flag=APP_RESIZABLE|APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_FULL_TOGGLE;
   flt scale=D.screenH()/1080.0;
   D.mode(500*scale, 500*scale).shadowMapSize(0);
   Ms.clip(0, 1).hide();
#if DEBUG
   Paks.add(EE_ENGINE_PATH);
#else
   Paks.add("Engine.pak");
#endif
}
bool Init()
{
   var[0].set("x");
   var[1].set("t");
   ed.cur=func[edit].length();
   return true;
}
void Shut()
{
}
/******************************************************************************/
void DrawFunc(Color col, Str func)
{
   FREP(RES)
   {
      dbl  s=dbl(i)/(RES-1); var[0].value.r=center.x+size*(2*s-1);
      dbl  y=(CalculateD(func, var)-center.y)/size;
      Vec2 point(-1+2*s, y);
      val[i]=point;
   }
   D.lines(col, val, Elms(val));
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   if(Kb.k(KB_UP  ))if(edit  ){ed.cur=func[--edit].length(); ed.sel=-1;}
   if(Kb.k(KB_DOWN))if(edit<2){ed.cur=func[++edit].length(); ed.sel=-1;}
   EditText(func[edit], ed);

   center+=Ms.d()*flt(size);
   if(Ms.wheel()<0)size*=1.4;
   if(Ms.wheel()>0)size/=1.4;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);
   {
      Color color=ColorBA(0, 0.2);
      D.line(color, -1,  0, 1, 0);
      D.line(color,  0, -1, 0, 1);
      dbl x=-center.x/size,
          y=-center.y/size;
      D.line(color,  x, 1, x, -1);
      D.line(color, -1, y, 1,  y);
   }
   var[1].value.r=Time.curTime();
   if(func[0].is())DrawFunc(RED  , func[0]);
   if(func[1].is())DrawFunc(GREEN, func[1]);
   if(func[2].is())DrawFunc(BLUE , func[2]);

   {
      TextStyleParams ts(true); ts.align.set(1, -1); ts.size=0.1;
      flt y=D.h();
      D.text(ts, -D.w(), y    , S+"Zoom: "+flt(1.0/size));
      D.text(ts, -D.w(), y-0.1, S+"Center: "+center);
      ts.edit=((edit==0) ? &ed : NULL); D.text(ts, -D.w(), y-0.2, func[0]);
      ts.edit=((edit==1) ? &ed : NULL); D.text(ts, -D.w(), y-0.3, func[1]);
      ts.edit=((edit==2) ? &ed : NULL); D.text(ts, -D.w(), y-0.4, func[2]);
   }
}
/******************************************************************************/
