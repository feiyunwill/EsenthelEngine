/******************************************************************************/
struct _Cache // Cache (base) - Do not use this class, use 'Cache' instead
{
   struct Desc
   {
      Str  file; // file name
      UInt flag, ptr_num;
   };
   struct Elm
   {
   };

   Int elms()C {return _elms;}

   void   lock()C;
   void unlock()C;

 ~_Cache() {del();}

private:
   struct DelayRemove
   {
      Flt  time;
      Elm *elm;
   };
mutable Byte         _d_lock;
   Bool              _case_sensitive;
   Byte              _mode;
   Int               _elms, /*_data_offset, */_desc_offset, _delay_remove_counter;
   Flt               _delay_remove_time;
   Dbl               _delay_remove_check;
   CChar8           *_debug_name;
   Elm             **_order;
   Memx<Elm>         _memx;
   Memc<DelayRemove> _delay_remove;
   SyncLock          _lock;
   Ptr               _user;
   Bool            (*_load          )( Ptr data, C Str &file);
   Bool            (*_load_user     )( Ptr data, C Str &file, Ptr user);
   Bool            (*_can_be_removed)(CPtr data);

   void clear         ();
   void del           ();
   Byte mode          (Byte mode);
   void caseSensitive (Bool sensitive);
   void delayRemove   (Flt  time);
   void delayRemoveNow();
   void delayRemoveInc();
   void delayRemoveDec();
   void update        ();

   void setLoadUser(Bool (*load_user)(Ptr data, C Str &file, Ptr user), Ptr user);

#if EE_PRIVATE
   void processDelayRemove(Bool always);

   Desc& elmDesc(  Elm &elm )  {return *(Desc*)((Byte*)&elm+_desc_offset);}
 C Desc& elmDesc(C Elm &elm )C {return *(Desc*)((Byte*)&elm+_desc_offset);}
   Ptr   elmData(  Elm &elm )  {return          (Byte*)&elm/*+_data_offset*/ ;} // assumes that '_data_offset' is zero
  CPtr   elmData(C Elm &elm )C {return          (Byte*)&elm/*+_data_offset*/ ;} // assumes that '_data_offset' is zero
   Elm*  dataElm( CPtr  data)C {return  (Elm *)((Byte*)data/*-_data_offset*/);} // assumes that '_data_offset' is zero

   Elm*       findExact(CChar *file, Int    &stop);
   Elm*       findElm  (CChar *file, CChar  *path);
   Int  findDelayRemove(  Elm &elm);
   Ptr     validElmData(  Elm &elm , Bool counted);
   void      addToOrder(  Elm &elm);
   void removeFromOrder(  Elm &elm);
   Bool        contains(C Elm *elm )C {return _memx.  contains(        elm  );} // this is NOT thread-safe
   Int         absIndex(C Elm *elm )C {return _memx.  absIndex(        elm  );} // this is NOT thread-safe
   Int       validIndex(C Elm *elm )C {return _memx.validIndex(        elm  );} // this is NOT thread-safe
   Int         absIndex( CPtr  data)C {return         absIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero
   Int       validIndex( CPtr  data)C {return       validIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero
#endif
   Ptr    _find      (CChar *file, CChar *path, Bool counted);
   Ptr    _find      (C UID &id  , CChar *path, Bool counted);
   Ptr    _get       (CChar *file, CChar *path, Bool counted);
   Ptr    _get       (C UID &id  , CChar *path, Bool counted);
   Ptr    _require   (CChar *file, CChar *path, Bool counted);
   Ptr    _require   (C UID &id  , CChar *path, Bool counted);
   Bool   _contains  (CPtr   data                           )C;
   Int    _ptrCount  (CPtr   data                           )C;
   Bool   _dummy     (CPtr   data                           )C;
   void   _dummy     (CPtr   data, Bool   dummy             );
   CChar* _name      (CPtr   data, CChar *path              )C;
   UID    _id        (CPtr   data                           )C;
   void   _removeData(CPtr   data                           );

   void _incRef(CPtr data);
   void _decRef(CPtr data);

   void _lockedFrom(C _Cache &src);

 C Desc& lockedDesc(Int i)C;
   CPtr  lockedData(Int i)C;

   explicit _Cache(CChar8 *name, Int block_elms, Bool (*load)(Ptr data, C Str &file));

   NO_COPY_CONSTRUCTOR(_Cache);

   T1(TYPE)                                    friend struct Cache;
   template<typename TYPE, Cache<TYPE> &CACHE> friend struct CacheElmPtr;
};
/******************************************************************************/
