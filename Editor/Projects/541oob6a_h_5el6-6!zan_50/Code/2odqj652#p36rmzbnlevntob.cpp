/******************************************************************************/
PathFind    pf        ; // path finder
Memc<VecI2> path      ; // path
VecI2       start, end; // start and end position of wanted path
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
bool Init()
{
   ImagePtr map=UID(1766339818, 1107945275, 1903918499, 1075877620);
   if(map->lockRead())
   {
      pf.create(map->w(), map->h());
      REPD(y, pf.h())
      REPD(x, pf.w())
      {
         Color color=map->color(x, y); // get pixel color of loaded map
         pf.pixelFlag(x, y, (color!=BLACK) ? PFP_WALKABLE : 0); // set non black color for walkable pixels
      }
      map->unlock();
   }
   return true;
}
void Shut()
{
}
/******************************************************************************/
VecI2 ScreenToPixel(C Vec2 &screen)
{
   int size=Max(pf.w(), pf.h())*4/3;
   return VecI2(Round((screen.x+D.h()*3/4)*size/2),
                Round((screen.y+D.h()*3/4)*size/2));
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   // set starting position on LMB
   if(Ms.b(0))
   {
      start=ScreenToPixel(Ms.pos());
      pf.find(&start, &end, path);
   }

   // set ending position on RMB
   if(Ms.b(1))
   {
      end=ScreenToPixel(Ms.pos());
      pf.find(&start, &end, path);
   }

   return true;
}
/******************************************************************************/
void DrawPixel(int x, int y, Color color)
{
   int size=Max(pf.w(), pf.h())*4/3;
   VI.dot(color, Vec2(x, y)*2/size-D.h()*3/4, 1.0/size);
}
void Draw()
{
   D.clear(GREY);

   REPD(y, pf.h())
   REPD(x, pf.w())DrawPixel(x, y, FlagTest(pf.pixelFlag(x, y), PFP_WALKABLE) ? WHITE : BLACK); // draw map
   REPA(path     )DrawPixel(path[i].x, path[i].y, YELLOW);                                     // draw path              with yellow color
                  DrawPixel(start  .x, start  .y, GREEN );                                     // draw starting position with green  color
                  DrawPixel(end    .x, end    .y, RED   );                                     // draw ending   position with red    color
   VI.end();

   D.text(0, 0.9, "Press LMB and RMB to set Start and End position");
}
/******************************************************************************/
