/******************************************************************************/
/******************************************************************************/
class WorldBrushClass : BrushClass
{
   flt size, speed, soft;

   WorldBrushClass& create(GuiObj &parent);

   // get
   virtual bool hasMsWheelFocus()C override;

   bool affects(C Edge2 &edge, C VecI2 &xz)C;
   RectI affectedAreas(C Edge2 &edge)C;
   RectI affectedAreas(int mouse_button=-1)C; // always calculate on demand, and for example not in 'update' because that method may get called later in frame than when other codes need it (resulting in 1 frame delay which could result in wrong behavior)

   bool affected(C VecI2 &xz)C; // if area of 'xz' coordinates is affected by brush, this is more precise than the 'affectedAreas'

   flt power(C Vec2 &pos, bool texture=true, Cursor &cur=WorldEdit.cur)C; // get brush power (0..1) at world XZ position, 'texture'=if allow applying image coverage, 'cur'=get power from specified cursor
   flt powerTotal(C Vec2 &pos, bool texture=true)C; // get brush power (0..1) at world XZ position, 'texture'=if allow applying image coverage, get power from all cursors

   Vec randomPos(bool align_heightmap)C;

   void setVisibility();

   virtual void update(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern WorldBrushClass Brush;
/******************************************************************************/
