/******************************************************************************/
struct _Cell // Cell - Do not use this class, use 'Cell' instead
{
#if !EE_PRIVATE
private:
#endif
   Ptr   _data;
   Int   _x[4],
         _y[4];
  _Cell *_parent;
   union
   {
      struct{_Cell *_g[9];};
      struct{_Cell *_lb, *_b, *_rb, *_l, *_c, *_r, *_lf, *_f, *_rf;};
   };

#if EE_PRIVATE
   Int   x ()C {return _x[0];}
   Int    y()C {return _y[0];}
   VecI2 xy()C {return VecI2(x(), y());}

   Bool final()C {return _x[0]==_x[3] && _y[0]==_y[3];} // need to check both, because for special case outside normal range, one index can be the same, but other not

   void  del           (                                               _Grid &grid);
  _Cell* create        (Int x , Int y                 , _Cell *parent, _Grid &grid);
  _Cell* create        (Int x0, Int y0, Int x1, Int y1, _Cell *parent, _Grid &grid);
  _Cell* createAsParent(                                _Cell *child , _Grid &grid);

  _Cell*  get(C VecI2 &xy, _Grid &grid);
  _Cell* find(C VecI2 &xy             );

   Bool getLeft  (Int &value)C;
   Bool getRight (Int &value)C;
   Bool getBottom(Int &value)C;
   Bool getTop   (Int &value)C;

                 void func      (               void func(_Cell &cell, Ptr        user), Ptr        user             );
   T1(USER_DATA) void func      (               void func(_Cell &cell, USER_DATA &user), USER_DATA &user             ) {T.func(      (void(*)(_Cell &cell, Ptr user))func, &user);}
                 void func      (C RectI &rect, void func(_Cell &cell, Ptr        user), Ptr        user             );
   T1(USER_DATA) void func      (C RectI &rect, void func(_Cell &cell, USER_DATA &user), USER_DATA &user             ) {T.func(rect, (void(*)(_Cell &cell, Ptr user))func, &user);}
                 void funcCreate(C RectI &rect, void func(_Cell &cell, Ptr        user), Ptr        user, _Grid &grid);
#endif

  _Cell() {Zero(T);}

   NO_COPY_CONSTRUCTOR(_Cell);
};
/******************************************************************************/
struct _Grid // Grid - Do not use this class, use 'Grid' instead
{
   Ptr user;

   void   del(             ) ;
   void   del( _Cell  *cell) ;
  _Cell&  get(C VecI2 &xy  ) ;
  _Cell* find(C VecI2 &xy  )C;
   Bool  size(  RectI &rect)C;
   void  fastAccess(C RectI *rect);

   void func      (               void func(_Cell &cell, Ptr user), Ptr user);
   void func      (C RectI &rect, void func(_Cell &cell, Ptr user), Ptr user);
   void funcCreate(C RectI &rect, void func(_Cell &cell, Ptr user), Ptr user);

   void mtFunc(Threads &threads,                void func(_Cell &cell, Ptr user, Int thread_index), Ptr user);
   void mtFunc(Threads &threads, C RectI &rect, void func(_Cell &cell, Ptr user, Int thread_index), Ptr user);

   T2(TYPE, EXTENDED)   void replaceClass()
   {
      struct Helper
      {
         static void New(EXTENDED* &elm, C VecI2 &xy, Ptr grid_user) {Alloc(elm); new(elm)EXTENDED(xy, grid_user);} // first allocate memory to setup pointer, then call the constructor in case the constructor checks the grid for self
         static void Del(EXTENDED* &elm                            ) {if(elm){elm->~EXTENDED(); Free(elm);}}
      };
      ASSERT_BASE_EXTENDED<TYPE, EXTENDED>();
      del();
     _new=(void (*)(Ptr &elm, C VecI2 &xy, Ptr grid_user))Helper::New;
     _del=(void (*)(Ptr &elm                            ))Helper::Del;
   }

 ~_Grid() {del();}

#if !EE_PRIVATE
private:
#endif
  _Cell        *_root;
   Int          _fast_access_mul;
   RectI        _fast_access_rect;
   Mems<_Cell*> _fast_access_cell;
   void       (*_new)(Ptr &elm, C VecI2 &xy, Ptr grid_user);
   void       (*_del)(Ptr &elm                            );

#if EE_PRIVATE
  _Cell* &fastAccessCell(C VecI2 &xy)  {return _fast_access_cell[(xy.x-_fast_access_rect.min.x) + (xy.y-_fast_access_rect.min.y)*_fast_access_mul];} // warning: this does not do any out of range checks
  _Cell*  fastAccessCell(C VecI2 &xy)C {return ConstCast(T).fastAccessCell(xy);}
#endif
   explicit _Grid() {user=null; _root=null; _fast_access_mul=0; _fast_access_rect.set(0, 0, -1, -1); _new=null; _del=null;}

   NO_COPY_CONSTRUCTOR(_Grid);

   T1(TYPE) friend struct Grid;
};
/******************************************************************************/
