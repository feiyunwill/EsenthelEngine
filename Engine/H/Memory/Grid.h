/******************************************************************************

   Use 'Grid' to handle space partitioning (like a QuadTree).

/******************************************************************************/
T1(TYPE) struct Cell // Cell (template)
{
             TYPE * data      () {return             _data ;} // get cell data
             TYPE & operator()() {return            *_data ;} // get cell data
   T1(TYPE2) TYPE2* cast      () {return CAST(TYPE2, _data);} // get cell data casted to custom class

   Int    x()C {return       _x[0]        ;} // get x  coordinate  of the cell
   Int    y()C {return              _y[0] ;} // get  y coordinate  of the cell
   VecI2 xy()C {return VecI2(_x[0], _y[0]);} // get xy coordinates of the cell

private:
   TYPE *_data;
   Int   _x[4],
         _y[4];
   Cell *_parent;
   union
   {
      struct{Cell *_g[9];};
      struct{Cell *_lb, *_b, *_rb, *_l, *_c, *_r, *_lf, *_f, *_rf;};
   };

   Cell() {Zero(T);}
   NO_COPY_CONSTRUCTOR(Cell);
};
/******************************************************************************/
T1(TYPE) struct Grid : private _Grid // Grid
{
   // manage
   void del(                ); // delete all      cells
   void del(Cell<TYPE> *cell); // delete selected cell

   // get / set
   Cell<TYPE>&  get(C VecI2 &xy  ) ; // get 'xy' cell, create it   if not found
   Cell<TYPE>* find(C VecI2 &xy  )C; // get 'xy' cell, return null if not found
   Bool        size(  RectI &rect)C; // get rectangle covering all created grid cells, false on fail (if no grid cells are present)
   Ptr         user(             )C {return _Grid::user;}   void user(Ptr user) {_Grid::user=user;} // get/set custom user data

                 Grid& fastAccess  (C RectI *rect); // optimize accessing cells (via 'find/get' methods) within 'rect' rectangle, normally cells are accessed recursively, however after calling this method all cells within the rectangle will be available instantly, if null is provided then the optimization is disabled
   T1(EXTENDED)  Grid& replaceClass(             ); // replace the type of class stored in the grid, all grid cells are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

   // call custom function on grid cells
                  void func      (               void func(Cell<TYPE> &cell, Ptr        user)                      ); // call 'func' on all existing grid cells
   T1(USER_DATA)  void func      (               void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user=null); // call 'func' on all existing grid cells
   T1(USER_DATA)  void func      (               void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user     ); // call 'func' on all existing grid cells
                  void func      (C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user)                      ); // call 'func' on all existing grid cells in specified rectangle
   T1(USER_DATA)  void func      (C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user=null); // call 'func' on all existing grid cells in specified rectangle
   T1(USER_DATA)  void func      (C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user     ); // call 'func' on all existing grid cells in specified rectangle
                  void funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user)                      ); // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)
   T1(USER_DATA)  void funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user=null); // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)
   T1(USER_DATA)  void funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user     ); // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)

   // call custom function on grid cells (multi-threaded version)
                  void mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, Ptr        user, Int thread_index)                      ); // call 'func' on all existing grid cells
   T1(USER_DATA)  void mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, USER_DATA *user, Int thread_index), USER_DATA *user=null); // call 'func' on all existing grid cells
   T1(USER_DATA)  void mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, USER_DATA &user, Int thread_index), USER_DATA &user     ); // call 'func' on all existing grid cells
                  void mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user, Int thread_index)                      ); // call 'func' on all existing grid cells in specified rectangle
   T1(USER_DATA)  void mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user, Int thread_index), USER_DATA *user=null); // call 'func' on all existing grid cells in specified rectangle
   T1(USER_DATA)  void mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user, Int thread_index), USER_DATA &user     ); // call 'func' on all existing grid cells in specified rectangle

   Grid() {replaceClass<TYPE>();}
};
/******************************************************************************/
