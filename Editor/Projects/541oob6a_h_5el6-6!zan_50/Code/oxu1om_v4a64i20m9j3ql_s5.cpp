/******************************************************************************

   This tutorial will present how to obtain a list of elements from current project.

/******************************************************************************/
Edit.EditorInterface EI;
Memc<Edit.Elm> elms; // list of project elements
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // connect to a running instance of the Editor
   Str message; if(!EI.connect(message))Exit(message);

   EI.getElms(elms); // get a list of project elements

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text(0, 0.90, "Project Elements:");
   flt y=0.8; FREPA(elms)
   {
      TextStyleParams ts; ts.size=0.065;
      ts.align.set( 1, 0); D.text(ts, -D.w(), y, elms[i].full_name);
      ts.align.set(-1, 0); D.text(ts,  D.w(), y, elms[i].id.asCString());
      y-=ts.lineHeight();
   }
}
/******************************************************************************/
