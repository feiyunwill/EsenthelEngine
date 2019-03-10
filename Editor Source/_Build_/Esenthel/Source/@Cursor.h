/******************************************************************************/
/******************************************************************************/
class Cursor
{
   bool   onViewport ()C; // if the cursor is on one of the editing world viewports
   bool   valid      ()C; // if the cursor points to a valid location , this should be tested before using the cursor positions
   bool   onHeightmap()C; // if cursor is on a heightmap
 C Vec2 & screenPos  ()C; // get cursor screen position
 C Vec  & pos        ()C; // get cursor world  position               , this can be used only after testing the 'valid' method
 C VecI2& area       ()C; // get area world coordinates               , this can be used only after testing the 'valid' method
 C VecI2& xz         ()C; // get coordinates according to current grid, this can be used only after testing the 'valid' method
 C Edge2& edge       ()C; // get 2D edge on the ground surface, where one point has the position of the cursor in previous frame, while other point has current cursor position, this can be used only after testing the 'valid' method

   bool on()C; // if cursor is on
   bool pd()C; // if cursor is pushed
   bool rs()C; // if cursor is released
   bool db()C; // if cursor is double clicked

   bool notFirstEdit()C;

   Touch* touch()C; // get touch linked with this cursor

   Edit::Viewport4::View* view()C;

   void update(C Vec2 &screen_pos, GuiObj*go, bool on, bool db, WorldView &we);
   void removed(Obj &obj);                             

//private:
   bool                 _valid, _on_heightmap;
   byte                 _state;
   uint                 _touch_id;
   flt                  _brush_height, _brush_height_help;
   Vec2                 _screen_pos;
   Vec                  _pos, _pos_prev;
   VecI2                _area, _xz;
   Edge2                _edge;
   Obj                 *_obj;
   Edit::Viewport4::View *_view;
   RayTest              _ray_test;

public:
   Cursor();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
