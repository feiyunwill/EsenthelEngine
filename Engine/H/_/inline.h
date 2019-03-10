/******************************************************************************/
inline Bool Any(C Half &x                                 ) {return FlagTest(x.data                           , 0x7FFF);} // faster version of "x!=0"
inline Bool Any(C Half &x, C Half &y                      ) {return FlagTest(x.data | y.data                  , 0x7FFF);} // faster version of "x!=0 || y!=0"
inline Bool Any(C Half &x, C Half &y, C Half &z           ) {return FlagTest(x.data | y.data | z.data         , 0x7FFF);} // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Half &x, C Half &y, C Half &z, C Half &w) {return FlagTest(x.data | y.data | z.data | w.data, 0x7FFF);} // faster version of "x!=0 || y!=0 || z!=0 || w!=0"

inline Bool Any(C Flt &x                              ) {return FlagTest((U32&)x                              , ~SIGN_BIT);} // faster version of "x!=0"
inline Bool Any(C Flt &x, C Flt &y                    ) {return FlagTest((U32&)x | (U32&)y                    , ~SIGN_BIT);} // faster version of "x!=0 || y!=0"
inline Bool Any(C Flt &x, C Flt &y, C Flt &z          ) {return FlagTest((U32&)x | (U32&)y | (U32&)z          , ~SIGN_BIT);} // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Flt &x, C Flt &y, C Flt &z, C Flt &w) {return FlagTest((U32&)x | (U32&)y | (U32&)z | (U32&)w, ~SIGN_BIT);} // faster version of "x!=0 || y!=0 || z!=0 || w!=0"

inline Bool Any(C Dbl &x                              ) {return FlagTest((U64&)x                              , (~0ull)>>1);} // faster version of "x!=0"
inline Bool Any(C Dbl &x, C Dbl &y                    ) {return FlagTest((U64&)x | (U64&)y                    , (~0ull)>>1);} // faster version of "x!=0 || y!=0"
inline Bool Any(C Dbl &x, C Dbl &y, C Dbl &z          ) {return FlagTest((U64&)x | (U64&)y | (U64&)z          , (~0ull)>>1);} // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Dbl &x, C Dbl &y, C Dbl &z, C Dbl &w) {return FlagTest((U64&)x | (U64&)y | (U64&)z | (U64&)w, (~0ull)>>1);} // faster version of "x!=0 || y!=0 || z!=0 || w!=0"
/******************************************************************************/
inline Int  AtomicGet(C Int  &x        ) {return x;}
inline UInt AtomicGet(C UInt &x        ) {return x;}
inline Flt  AtomicGet(C Flt  &x        ) {return x;}
inline void AtomicSet(  Int  &x, Int  y) {x=y     ;}
inline void AtomicSet(  UInt &x, UInt y) {x=y     ;}
inline void AtomicSet(  Flt  &x, Flt  y) {x=y     ;}
#if X64
inline Long AtomicGet(C Long &x        ) {return x;}
inline void AtomicSet(  Long &x, Long y) {x=y     ;}
#endif
T1(TYPE) void AtomicSet(TYPE* &x, TYPE *y) {x=y;}
/******************************************************************************/
inline       Str8::Str8     (Str8 &&s) {_length=0; Swap(T, s);}
inline       Str ::Str      (Str  &&s) {_length=0; Swap(T, s);}
inline Str8& Str8::operator=(Str8 &&s) {           Swap(T, s); return T;}
inline Str & Str ::operator=(Str  &&s) {           Swap(T, s); return T;}

inline Bool Str8::save(File &f)C {f.putStr(T); return f.ok();}
inline Bool Str ::save(File &f)C {f.putStr(T); return f.ok();}
inline Bool Str8::load(File &f)  {f.getStr(T); return f.ok();}
inline Bool Str ::load(File &f)  {f.getStr(T); return f.ok();}
/******************************************************************************/
inline C TextNode*           FindNodeC(C MemPtr<TextNode> &nodes, C Str &name, Int i)  {return FindNode(ConstCast(nodes),          name, i);}
inline C XmlParam*  XmlNode::findParam(C Str &name, Int i                           )C {return          ConstCast(T    ).findParam(name, i);}
inline C TextNode* TextNode::findNode (C Str &name, Int i                           )C {return          ConstCast(T    ).findNode (name, i);}
inline C TextNode* TextData::findNode (C Str &name, Int i                           )C {return          ConstCast(T    ).findNode (name, i);}
/******************************************************************************/
inline C TextParam* Edit::FileParams::findParam(C Str &name)C {return ConstCast(T).findParam(name);}
/******************************************************************************/
inline C SkelBone * Skeleton ::findBone (              BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T).findBone (      type, type_index, type_sub);}
inline C SkelBone & Skeleton :: getBone (              BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T). getBone (      type, type_index, type_sub);}
inline C SkelBone * Skeleton ::findBone (CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T).findBone (name, type, type_index, type_sub);}
inline C SkelBone * Skeleton ::findBone (CChar8 *name                                              )C {return ConstCast(T).findBone (name                            );}
inline C SkelSlot * Skeleton ::findSlot (CChar8 *name                                              )C {return ConstCast(T).findSlot (name                            );}
inline C SkelBone & Skeleton :: getBone (CChar8 *name                                              )C {return ConstCast(T). getBone (name                            );}
inline C SkelSlot & Skeleton :: getSlot (CChar8 *name                                              )C {return ConstCast(T). getSlot (name                            );}
inline C AnimBone * Animation::findBone (CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T).findBone (name, type, type_index, type_sub);}
inline C AnimEvent* Animation::findEvent(CChar8 *name                                              )C {return ConstCast(T).findEvent(name                            );}
/******************************************************************************/
inline C Param* Object::findParam(C Str &name)C {return ConstCast(T).findParam(name);}
inline C Param& Object:: getParam(C Str &name)C {return ConstCast(T). getParam(name);}
/******************************************************************************/
extern Bool  _CompressBC7 (C Image &src, Image &dest);
extern Bool (*CompressBC7)(C Image &src, Image &dest);
inline void   SupportCompressBC7() {CompressBC7=_CompressBC7;}

extern Bool  _CompressETC (C Image &src, Image &dest, Int quality=-1, Bool perceptual=true);
extern Bool (*CompressETC)(C Image &src, Image &dest, Int quality   , Bool perceptual     );
inline void   SupportCompressETC() {CompressETC=_CompressETC;}

extern Bool  _CompressPVRTC (C Image &src, Image &dest, Int quality=-1);
extern Bool (*CompressPVRTC)(C Image &src, Image &dest, Int quality   );
inline void   SupportCompressPVRTC() {if(WINDOWS_OLD || MAC || LINUX)CompressPVRTC=_CompressPVRTC;}

inline void SupportCompressAll() {SupportCompressBC7(); SupportCompressETC(); SupportCompressPVRTC();}
/******************************************************************************/
// STRING / TEXT
/******************************************************************************/
inline Char * TextPos(Char  *src, Char  c) {return ConstCast(TextPos((CChar *)src, c));}
inline Char8* TextPos(Char8 *src, Char8 c) {return ConstCast(TextPos((CChar8*)src, c));}
/******************************************************************************/
// MATRIX
/******************************************************************************/
inline void Matrix3::mul(C RevMatrix3 &matrix, Matrix3 &dest)C {matrix.mul(T, dest);}
inline void Matrix ::mul(C RevMatrix  &matrix, Matrix  &dest)C {matrix.mul(T, dest);}
/******************************************************************************/
// TEMPLATES
/******************************************************************************/
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Int   i, C TYPE &container) {return UInt (i)<UInt (Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(UInt  i, C TYPE &container) {return UInt (i)<UInt (Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Long  i, C TYPE &container) {return ULong(i)<ULong(Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(ULong i, C TYPE &container) {return ULong(i)<ULong(Elms(container));} // template specialization for not enum's
/******************************************************************************/
T1(TYPE) struct ClassFunc // various basic functions used by many classes
{
   static void New      (Ptr elm                        ) {    new(        elm )     TYPE            ;}
   static void Del      (Ptr elm                        ) {       ( (TYPE*)elm )->  ~TYPE(          );}
   static void Copy     (Ptr dest,  CPtr  src           ) {       (*(TYPE*)dest)=*(C TYPE*)src       ;}
   static Bool Load     (Ptr elm , C Str &file          ) {return ( (TYPE*)elm )->   load(file      );}
   static Bool LoadUser (Ptr elm , C Str &file, Ptr user) {return ( (TYPE*)elm )->   load(file, user);}
   static Bool LoadEmpty(Ptr elm                        ) {return ( (TYPE*)elm )->   load(          );}
   static void Unload   (Ptr elm                        ) {return ( (TYPE*)elm )-> unload(          );}

   static inline Bool HasNew() {return New!=ClassFunc<Int>::New;}   static inline void (*GetNew())(Ptr elm) {return HasNew() ? New : null;}
   static inline Bool HasDel() {return Del!=ClassFunc<Int>::Del;}   static inline void (*GetDel())(Ptr elm) {return HasDel() ? Del : null;}
};
// force empty new delete in case some compilers don't merge similar functions, this also helps in DEBUG mode which doesn't do merging
#define EMPTY_NEW_DEL(TYPE) \
   template<> inline Bool ClassFunc<TYPE>::HasNew() {return false;} \
   template<> inline Bool ClassFunc<TYPE>::HasDel() {return false;}
EMPTY_NEW_DEL(Bool )
EMPTY_NEW_DEL(Char )  EMPTY_NEW_DEL(Char8 )
EMPTY_NEW_DEL(SByte)  EMPTY_NEW_DEL(Byte  )
EMPTY_NEW_DEL(Short)  EMPTY_NEW_DEL(UShort)
EMPTY_NEW_DEL(Int  )  EMPTY_NEW_DEL(UInt  )
EMPTY_NEW_DEL(Long )  EMPTY_NEW_DEL(ULong )
EMPTY_NEW_DEL(Flt  )  EMPTY_NEW_DEL(Dbl   )
EMPTY_NEW_DEL(Ptr  )
#undef EMPTY_NEW_DEL
/******************************************************************************/
// SORT
/******************************************************************************/
void _Sort( Ptr   data, Int elms, Int elm_size, Int compare(CPtr a, CPtr b));
void _Sort(_Memb &memb,                         Int compare(CPtr a, CPtr b));
void _Sort(_Memx &memx,                         Int compare(CPtr a, CPtr b));
void _Sort(_Meml &meml,                         Int compare(CPtr a, CPtr b));

Bool _BinarySearch(   CPtr  data, Int elms, Int elm_size, CPtr value, Int &index, Int compare(CPtr a, CPtr b));
Bool _BinarySearch(C _Memb &data,                         CPtr value, Int &index, Int compare(CPtr a, CPtr b));
Bool _BinarySearch(C _Memx &data,                         CPtr value, Int &index, Int compare(CPtr a, CPtr b));
Bool _BinarySearch(C _Meml &data,                         CPtr value, Int &index, Int compare(CPtr a, CPtr b));

T1(TYPE) void Sort(TYPE *data, Int elms, Int compare(C TYPE &a, C TYPE &b)) {_Sort(Ptr(data), elms, SIZE(TYPE), (Int(*)(CPtr, CPtr))compare);}

T2(DATA, VALUE) Bool BinarySearch(C DATA *data, Int elms, C VALUE &value, Int &index, Int compare(C DATA &a, C VALUE &b)) {return _BinarySearch(data, elms, SIZE(DATA), &value, index, (Int(*)(CPtr, CPtr))compare);}

#if EE_PRIVATE
struct FloatIndex // Float + Index
{
   Flt f; // float
   Int i; // index

   static Int Compare(C FloatIndex &a, C FloatIndex &b) {return EE::Compare(a.f, b.f);}
};
inline void Sort(FloatIndex *data, Int elms) {Sort(data, elms, FloatIndex::Compare);}
#endif
/******************************************************************************/
// MEMORY
/******************************************************************************/
void _Realloc    (Ptr &data, IntPtr size_new, IntPtr size_old); // reallocate memory without losing data                      , Exit on fail !! this function can be used only for memory allocated using 'Alloc', but not 'New' !!
void _ReallocZero(Ptr &data, IntPtr size_new, IntPtr size_old); // reallocate memory without losing data and zero new elements, Exit on fail !! this function can be used only for memory allocated using 'Alloc', but not 'New' !!

T1(TYPE) void Realloc    (TYPE* &data, Int elms_new, Int elms_old) {_Realloc    (*(Ptr*)&data, elms_new*SIZE(TYPE), elms_old*SIZE(TYPE));}
T1(TYPE) void ReallocZero(TYPE* &data, Int elms_new, Int elms_old) {_ReallocZero(*(Ptr*)&data, elms_new*SIZE(TYPE), elms_old*SIZE(TYPE));}
#if EE_PRIVATE
T1(TYPE) void Realloc1    (TYPE* &data,              Int elms_old) {Realloc    (data, elms_old+1, elms_old);}
T1(TYPE) void ReallocZero1(TYPE* &data,              Int elms_old) {ReallocZero(data, elms_old+1, elms_old);}

void _MoveElmLeftUnsafe(Ptr data, UInt elm_size, Int elm, Int new_index, Ptr temp);
#endif

void   _ReverseOrder(Ptr data, Int elms, UInt elm_size                           ); // reverse   order of elements (first<->last)
void    _RotateOrder(Ptr data, Int elms, UInt elm_size, Int offset               ); // rotate    order of elements, changes the order of elements so "new_index=old_index+offset", 'offset'=offset of moving the original indexes into target indexes (-Inf..Inf)
void _RandomizeOrder(Ptr data, Int elms, UInt elm_size, Randomizer &random=Random); // randomize order of elements
void      _MoveElm  (Ptr data, Int elms, UInt elm_size, Int elm, Int new_index   ); // move 'elm' element to new position located at 'new_index'

T1(TYPE) void   ReverseOrder(TYPE *data, Int elms                        ) {  _ReverseOrder(data, elms, SIZE(TYPE)                );}
T1(TYPE) void    RotateOrder(TYPE *data, Int elms, Int offset            ) {   _RotateOrder(data, elms, SIZE(TYPE), offset        );}
T1(TYPE) void RandomizeOrder(TYPE *data, Int elms, Randomizer &random    ) {_RandomizeOrder(data, elms, SIZE(TYPE), random        );}
T1(TYPE) void      MoveElm  (TYPE *data, Int elms, Int elm, Int new_index) {     _MoveElm  (data, elms, SIZE(TYPE), elm, new_index);}
/******************************************************************************/
// REFERENCE
/******************************************************************************/
T1(TYPE)           Bool Reference<TYPE>::save(File  &f    )C {          return f.put(_object_id);}
T1(TYPE)           Bool Reference<TYPE>::load(File  &f    )  {_object=null; if(f.get(_object_id))return true; _object_id.zero(); return false;}
T1(TYPE) T1(WORLD) void Reference<TYPE>::link(WORLD &world)  {if(!valid() && _object_id.valid())_object=CAST(TYPE, world.findObjById(_object_id));}
/******************************************************************************/
// FIXED ARRAY
/******************************************************************************/
template<typename TYPE, Int NUM>                                       FixedArray<TYPE, NUM>::FixedArray(                 )                {_elm_size=SIZE(TYPE); _data=null;}
template<typename TYPE, Int NUM>                                       FixedArray<TYPE, NUM>::FixedArray(C FixedArray &src) : FixedArray() {T=src;}
template<typename TYPE, Int NUM>               FixedArray<TYPE, NUM>&  FixedArray<TYPE, NUM>::operator= (C FixedArray &src)                {FREPAO(T)=src[i]; return T;}
template<typename TYPE, Int NUM>               FixedArray<TYPE, NUM>&  FixedArray<TYPE, NUM>::del         () {DeleteN(_data); _elm_size=SIZE(TYPE); return T;}
template<typename TYPE, Int NUM> T1(EXTENDED)  FixedArray<TYPE, NUM>&  FixedArray<TYPE, NUM>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); del(); _elm_size=SIZE(EXTENDED); _data=new EXTENDED[NUM]; return T;}

template<typename TYPE>                                FixedElm<TYPE>::FixedElm (               )              {_data=null;}
template<typename TYPE>                                FixedElm<TYPE>::FixedElm (C FixedElm &src) : FixedElm() {T=src;}
template<typename TYPE>               FixedElm<TYPE>&  FixedElm<TYPE>::operator=(C FixedElm &src)              {T()=src(); return T;}
template<typename TYPE>               FixedElm<TYPE>&  FixedElm<TYPE>::del         () {Delete(_data); return T;}
template<typename TYPE> T1(EXTENDED)  FixedElm<TYPE>&  FixedElm<TYPE>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); del(); _data=new EXTENDED; return T;}
/******************************************************************************/
// MEMS
/******************************************************************************/
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::clear()
{
   if(ClassFunc<TYPE>::HasDel())REPA(T)T[i].~TYPE();
   Free(_data); _elms=0;
   return T;
}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::del() {return clear();}

T1(TYPE)  Int   Mems<TYPE>::elms    ()C {return _elms;}
T1(TYPE)  UInt  Mems<TYPE>::elmSize ()C {return SIZE(TYPE);}
T1(TYPE)  UInt  Mems<TYPE>::memUsage()C {return elms()*elmSize();}

T1(TYPE)  TYPE*  Mems<TYPE>::data      (     ) {                          return _data   ;}
T1(TYPE)  TYPE*  Mems<TYPE>::addr      (Int i) {return InRange(i, _elms) ?      &_data[i] : null;}
T1(TYPE)  TYPE&  Mems<TYPE>::operator[](Int i) {  RANGE_ASSERT(i, _elms); return _data[i];}
T1(TYPE)  TYPE&  Mems<TYPE>::first     (     ) {return T[       0];}
T1(TYPE)  TYPE&  Mems<TYPE>::last      (     ) {return T[elms()-1];}

T1(TYPE)  TYPE  Mems<TYPE>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
T1(TYPE)  TYPE  Mems<TYPE>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
T1(TYPE)  TYPE  Mems<TYPE>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

T1(TYPE)  C TYPE*  Mems<TYPE>::data      (     )C {return ConstCast(T).data ( );}
T1(TYPE)  C TYPE*  Mems<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  Mems<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  Mems<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  Mems<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  TYPE&  Mems<TYPE>::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside 'Mems.operator()(Int i)'");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
T1(TYPE)  TYPE&  Mems<TYPE>::New  (     ) {return T[addNum(1)];}
T1(TYPE)  TYPE&  Mems<TYPE>::NewAt(Int i)
{
   Clamp(i, 0, elms());
   TYPE *temp=Alloc<TYPE>(elms()+1);
   CopyN(temp    , data()  ,        i);
   CopyN(temp+i+1, data()+i, elms()-i);
   Free(_data); _data=temp; _elms++;
   TYPE &elm=T[i]; new(&elm)TYPE; return elm;
}

T1(TYPE)  Int  Mems<TYPE>::index(C TYPE *elm)C
{
   UIntPtr i=UIntPtr(elm)-UIntPtr(data());
   if(i<UIntPtr(elms()*elmSize()))return i/elmSize(); // unsigned compare will already guarantee "i>=0 && "
   return -1;
}
T1(TYPE)  Bool  Mems<TYPE>::contains(C TYPE *elm)C {return index(elm)>=0;}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::remove(Int i, Bool /*keep_order*/)
{
   if(InRange(i, T))
   {
      T[i].~TYPE();
      TYPE *temp=Alloc<TYPE>(elms()-1);
      CopyN(temp  , data()    ,        i  );
      CopyN(temp+i, data()+i+1, elms()-i-1);
      Free(_data); _data=temp; _elms--;
   }
   return T;
}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::removeLast(                            ) {return remove(elms()-1              );}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::removeData(C TYPE *elm, Bool keep_order) {return remove(index(elm), keep_order);}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::setNum(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add new elements
   {
      Int old_elms=elms();
      TYPE *temp=Alloc<TYPE>(num);
      CopyN(temp, data(), elms());
      Free (_data); _data=temp; _elms=num;
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE;
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=num; i<elms(); i++)T[i].~TYPE();
      TYPE *temp=Alloc<TYPE>(num);
      CopyN(temp, data(), num);
      Free (_data); _data=temp; _elms=num;
   }
   return T;
}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::setNumZero(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add new elements
   {
      Int old_elms=elms();
      TYPE *temp=Alloc<TYPE>(num);
      CopyN(temp       , data(),     elms());
      ZeroN(temp+elms(),         num-elms());
      Free (_data); _data=temp; _elms=num;
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE;
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=num; i<elms(); i++)T[i].~TYPE();
      TYPE *temp=Alloc<TYPE>(num);
      CopyN(temp, data(), num);
      Free (_data); _data=temp; _elms=num;
   }
   return T;
}
T1(TYPE)  Int  Mems<TYPE>::addNum(Int num) {Int index=elms(); setNum(elms()+num); return index;}

T1(TYPE) T1(VALUE)  Bool  Mems<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearch(data(), elms(), elmSize(), &value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::sort(Int compare(C TYPE &a, C TYPE &b)) {_Sort(data(), elms(), elmSize(), (Int(*)(CPtr, CPtr))compare); return T;}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::  reverseOrder(                      ) {  _ReverseOrder(data(), elms(), elmSize()                ); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::randomizeOrder(                      ) {_RandomizeOrder(data(), elms(), elmSize()                ); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::   rotateOrder(Int offset            ) {   _RotateOrder(data(), elms(), elmSize(), offset        ); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::     moveElm  (Int elm, Int new_index) {     _MoveElm  (data(), elms(), elmSize(), elm, new_index); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::     swapOrder(Int i  , Int j        ) {if(InRange(i, T) && InRange(j, T))Swap(_data[i], _data[j]); return T;}

T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C Mems  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C Memc  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Mems<TYPE>&  Mems<TYPE>::operator=(C Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C Memb  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C Memx  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Mems<TYPE>&  Mems<TYPE>::operator=(C MemPtr<TYPE, size>  &src) {if(this!=src._mems){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(  Mems  <TYPE      > &&src) {Swap(T, src); return T;}

#if EE_PRIVATE
T1(TYPE)  void         Mems<TYPE>::copyTo  (  TYPE *dest)C {Copy(dest  , data(), elms()*elmSize());          }
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::copyFrom(C TYPE *src )  {Copy(data(), src   , elms()*elmSize()); return T;}
T1(TYPE)  void         Mems<TYPE>:: setFrom(  TYPE* &data, Int elms) {if(data!=T._data){del(); T._data=data; T._elms=elms; data=null;}}
T1(TYPE)  void         Mems<TYPE>:: setTemp(  TYPE*  data, Int elms) {                         T._data=data; T._elms=elms;            }
#endif

T1(TYPE)  Bool  Mems<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T1(USER)  Bool  Mems<TYPE>::save(File &f, C USER &user)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user))return false; return f.ok();}
T1(TYPE) T1(USER)  Bool  Mems<TYPE>::load(File &f, C USER &user)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T2(USER, USER1)  Bool  Mems<TYPE>::save(File &f, C USER &user, C USER1 &user1)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user, user1))return false; return f.ok();}
T1(TYPE) T2(USER, USER1)  Bool  Mems<TYPE>::load(File &f, C USER &user, C USER1 &user1)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user, user1))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Bool  Mems<TYPE>::saveRawData(File &f)C {return f.putN(data(), elms());}
T1(TYPE)  Bool  Mems<TYPE>::loadRawData(File &f)  {return f.getN(data(), elms());}

T1(TYPE)  Bool  Mems<TYPE>::saveRaw(File &f)C {       f.cmpUIntV(elms()) ; saveRawData(f); return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::loadRaw(File &f)  {setNum(f.decUIntV(      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}

#if EE_PRIVATE
T1(TYPE)  Bool  Mems<TYPE>::_saveRaw(File &f)C {       f.putInt(elms()) ; saveRawData(f); return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::_loadRaw(File &f)  {setNum(f.getInt(      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}
T1(TYPE)  Bool  Mems<TYPE>::_save   (File &f)C {       f.putInt(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::_load   (File &f)  {setNum(f.getInt(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}
#endif

T1(TYPE)  Mems<TYPE>::~Mems(            )          {del();}
T1(TYPE)  Mems<TYPE>:: Mems(            )          {_data=null; _elms=0;}
T1(TYPE)  Mems<TYPE>:: Mems(C Mems  &src) : Mems() {T=src;}
T1(TYPE)  Mems<TYPE>:: Mems(  Mems &&src) : Mems() {Swap(T, src);}
/******************************************************************************/
// MEMC
/******************************************************************************/
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::clear() {_Memc::clear(); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::del  () {_Memc::del  (); return T;}

T1(TYPE)  Int   Memc<TYPE>::elms    ()C {return _Memc::elms    ();}
T1(TYPE)  UInt  Memc<TYPE>::elmSize ()C {return _Memc::elmSize ();}
T1(TYPE)  UInt  Memc<TYPE>::memUsage()C {return _Memc::memUsage();}

T1(TYPE)  TYPE*  Memc<TYPE>::data      (     ) {DEBUG_ASSERT(elmSize()==SIZE(TYPE) || elms()<=1, "'Memc.data' Can't cast to C++ pointer after using 'replaceClass'."); return (TYPE*)_Memc::data();}
T1(TYPE)  TYPE*  Memc<TYPE>::addr      (Int i) {return  (TYPE*)_Memc::addr      (i);}
T1(TYPE)  TYPE*  Memc<TYPE>::addrFirst (     ) {return  (TYPE*)_Memc::addrFirst ( );}
T1(TYPE)  TYPE*  Memc<TYPE>::addrLast  (     ) {return  (TYPE*)_Memc::addrLast  ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::operator[](Int i) {return *(TYPE*)_Memc::operator[](i);}
T1(TYPE)  TYPE&  Memc<TYPE>::operator()(Int i) {return *(TYPE*)_Memc::operator()(i);}
T1(TYPE)  TYPE&  Memc<TYPE>::first     (     ) {return *(TYPE*)_Memc::first     ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::last      (     ) {return *(TYPE*)_Memc::last      ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::New       (     ) {return *(TYPE*)_Memc::New       ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::NewAt     (Int i) {return *(TYPE*)_Memc::NewAt     (i);}

T1(TYPE)  TYPE  Memc<TYPE>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
T1(TYPE)  TYPE  Memc<TYPE>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
T1(TYPE)  TYPE  Memc<TYPE>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

T1(TYPE)  C TYPE*  Memc<TYPE>::data      (     )C {return ConstCast(T).data     ( );}
T1(TYPE)  C TYPE*  Memc<TYPE>::addr      (Int i)C {return ConstCast(T).addr     (i);}
T1(TYPE)  C TYPE*  Memc<TYPE>::addrFirst (     )C {return ConstCast(T).addrFirst( );}
T1(TYPE)  C TYPE*  Memc<TYPE>::addrLast  (     )C {return ConstCast(T).addrLast ( );}
T1(TYPE)  C TYPE&  Memc<TYPE>::operator[](Int i)C {return ConstCast(T)          [i];}
T1(TYPE)  C TYPE&  Memc<TYPE>::first     (     )C {return ConstCast(T).first    ( );}
T1(TYPE)  C TYPE&  Memc<TYPE>::last      (     )C {return ConstCast(T).last     ( );}

T1(TYPE)  Int   Memc<TYPE>::index   (C TYPE *elm)C {return _Memc::index   (elm);}
T1(TYPE)  Bool  Memc<TYPE>::contains(C TYPE *elm)C {return _Memc::contains(elm);}

T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::removeLast(                                   ) {_Memc::removeLast(                ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::remove    (  Int   i  ,        Bool keep_order) {_Memc::remove    (i,    keep_order); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::removeNum (  Int   i  , Int n, Bool keep_order) {_Memc::removeNum (i, n, keep_order); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::removeData(C TYPE *elm,        Bool keep_order) {_Memc::removeData(elm,  keep_order); return T;}

T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::setNum    (Int num) {       _Memc::setNum    (num); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::setNumZero(Int num) {       _Memc::setNumZero(num); return T;}
T1(TYPE)  Int          Memc<TYPE>::addNum    (Int num) {return _Memc::addNum    (num);          }

T1(TYPE) T1(VALUE)  Bool  Memc<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _Memc::binarySearch(&value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::          sort(Int compare(C TYPE &a, C TYPE &b)) {_Memc::          sort((Int(*)(CPtr, CPtr))compare); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::  reverseOrder(                                 ) {_Memc::  reverseOrder(                           ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::randomizeOrder(                                 ) {_Memc::randomizeOrder(                           ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::   rotateOrder(Int offset                       ) {_Memc::   rotateOrder(offset                     ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::     swapOrder(Int i  , Int j                   ) {_Memc::     swapOrder(i, j                       ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::     moveElm  (Int elm, Int new_index           ) {_Memc::     moveElm  (elm, new_index             ); return T;}

T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C Mems  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C Memc  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE) template<Int size>  Memc<TYPE>&  Memc<TYPE>::operator=(C Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C Memb  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C Memx  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memc<TYPE>&  Memc<TYPE>::operator=(C MemPtr<TYPE, size>  &src) {if(this!=src._memc){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(  Memc  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE) T1(EXTENDED)  Memc<TYPE>&  Memc<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>();           _Memc::_reset(SIZE(EXTENDED), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )               Memc<TYPE>::operator   Memc<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memc<BASE>*)this;}
T1(TYPE) T1(BASE    )               Memc<TYPE>::operator C Memc<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memc<BASE>*)this;}

T1(TYPE)  Bool  Memc<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memc<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memc<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T1(USER)  Bool  Memc<TYPE>::save(File &f, C USER &user)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user))return false; return f.ok();}
T1(TYPE) T1(USER)  Bool  Memc<TYPE>::load(File &f, C USER &user)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T2(USER, USER1)  Bool  Memc<TYPE>::save(File &f, C USER &user, C USER1 &user1)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user, user1))return false; return f.ok();}
T1(TYPE) T2(USER, USER1)  Bool  Memc<TYPE>::load(File &f, C USER &user, C USER1 &user1)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user, user1))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Bool  Memc<TYPE>::saveRaw(File &f)C {return _Memc::saveRaw(f);}
T1(TYPE)  Bool  Memc<TYPE>::loadRaw(File &f)  {return _Memc::loadRaw(f);}

#if EE_PRIVATE
T1(TYPE)  Bool  Memc<TYPE>::_saveRaw(File &f)C {return _Memc::_saveRaw(f);}
T1(TYPE)  Bool  Memc<TYPE>::_loadRaw(File &f)  {return _Memc::_loadRaw(f);}
T1(TYPE)  Bool  Memc<TYPE>::_save   (File &f)C {       f.putInt(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memc<TYPE>::_load   (File &f)  {setNum(f.getInt(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}
#endif

T1(TYPE)  Memc<TYPE>::Memc(            ) : _Memc(SIZE(TYPE)   , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Memc<TYPE>::Memc(C Memc  &src) : _Memc(src.elmSize(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Memc<TYPE>::Memc(  Memc &&src) : _Memc(            0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// MEMC ABSTRACT
/******************************************************************************/
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::clear() {_Memc::clear(); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::del  () {_Memc::del  (); return T;}

T1(TYPE)  Int   MemcAbstract<TYPE>::elms    ()C {return _Memc::elms    ();}
T1(TYPE)  UInt  MemcAbstract<TYPE>::elmSize ()C {return _Memc::elmSize ();}
T1(TYPE)  UInt  MemcAbstract<TYPE>::memUsage()C {return _Memc::memUsage();}

T1(TYPE)  TYPE*  MemcAbstract<TYPE>::data      (     ) {DEBUG_ASSERT(elmSize()==SIZE(TYPE) || elms()<=1, "'MemcAbstract.data' Can't cast to C++ pointer after using 'replaceClass'."); return (TYPE*)_Memc::data();}
T1(TYPE)  TYPE*  MemcAbstract<TYPE>::addr      (Int i) {return  (TYPE*)_Memc::addr      (i);}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::operator[](Int i) {return *(TYPE*)_Memc::operator[](i);}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::operator()(Int i) {return *(TYPE*)_Memc::operator()(i);}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::first     (     ) {return *(TYPE*)_Memc::first     ( );}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::last      (     ) {return *(TYPE*)_Memc::last      ( );}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::New       (     ) {return *(TYPE*)_Memc::New       ( );}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::NewAt     (Int i) {return *(TYPE*)_Memc::NewAt     (i);}

T1(TYPE)  C TYPE*  MemcAbstract<TYPE>::data      (     )C {return ConstCast(T).data ( );}
T1(TYPE)  C TYPE*  MemcAbstract<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  MemcAbstract<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  MemcAbstract<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  MemcAbstract<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  Int   MemcAbstract<TYPE>::index   (C TYPE *elm)C {return _Memc::index   (elm);}
T1(TYPE)  Bool  MemcAbstract<TYPE>::contains(C TYPE *elm)C {return _Memc::contains(elm);}

T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::removeLast(                            ) {_Memc::removeLast(               ); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::remove    (  Int   i  , Bool keep_order) {_Memc::remove    (i  , keep_order); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::removeData(C TYPE *elm, Bool keep_order) {_Memc::removeData(elm, keep_order); return T;}

T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::setNum    (Int num) {       _Memc::setNum    (num); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::setNumZero(Int num) {       _Memc::setNumZero(num); return T;}
T1(TYPE)  Int                  MemcAbstract<TYPE>::addNum    (Int num) {return _Memc::addNum    (num);          }

T1(TYPE) T1(EXTENDED)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>();           _Memc::_reset(SIZE(EXTENDED), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )                       MemcAbstract<TYPE>::operator   Memc<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memc<BASE>*)this;}
T1(TYPE) T1(BASE    )                       MemcAbstract<TYPE>::operator C Memc<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memc<BASE>*)this;}

T1(TYPE)  MemcAbstract<TYPE>::MemcAbstract() : _Memc(0, null, null) {}
/******************************************************************************/
// MEMC THREAD SAFE
/******************************************************************************/
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::clear() {_MemcThreadSafe::clear(); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::del  () {_MemcThreadSafe::del  (); return T;}

T1(TYPE)  Int   MemcThreadSafe<TYPE>::elms    ()C {return _MemcThreadSafe::elms    ();}
T1(TYPE)  UInt  MemcThreadSafe<TYPE>::elmSize ()C {return _MemcThreadSafe::elmSize ();}
T1(TYPE)  UInt  MemcThreadSafe<TYPE>::memUsage()C {return _MemcThreadSafe::memUsage();}

T1(TYPE)  TYPE*  MemcThreadSafe<TYPE>::lockedData (     ) {DEBUG_ASSERT(elmSize()==SIZE(TYPE) || elms()<=1, "'MemcThreadSafe.data' Can't cast to C++ pointer after using 'replaceClass'."); return (TYPE*)_MemcThreadSafe::lockedData();}
T1(TYPE)  TYPE*  MemcThreadSafe<TYPE>::lockedAddr (Int i) {return  (TYPE*)_MemcThreadSafe::lockedAddr (i);}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedElm  (Int i) {return *(TYPE*)_MemcThreadSafe::lockedElm  (i);}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedFirst(     ) {return *(TYPE*)_MemcThreadSafe::lockedFirst( );}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedLast (     ) {return *(TYPE*)_MemcThreadSafe::lockedLast ( );}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedNew  (     ) {return *(TYPE*)_MemcThreadSafe::lockedNew  ( );}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedNewAt(Int i) {return *(TYPE*)_MemcThreadSafe::lockedNewAt(i);}

T1(TYPE)  TYPE  MemcThreadSafe<TYPE>::popFirst(       Bool keep_order) {lock(); TYPE temp=lockedFirst( ); remove    (0, keep_order); unlock(); return temp;}
T1(TYPE)  TYPE  MemcThreadSafe<TYPE>::pop     (Int i, Bool keep_order) {lock(); TYPE temp=lockedElm  (i); remove    (i, keep_order); unlock(); return temp;}
T1(TYPE)  TYPE  MemcThreadSafe<TYPE>::pop     (                      ) {lock(); TYPE temp=lockedLast ( ); removeLast(             ); unlock(); return temp;}

T1(TYPE)  C TYPE*  MemcThreadSafe<TYPE>::lockedData (     )C {return ConstCast(T).lockedData ( );}
T1(TYPE)  C TYPE*  MemcThreadSafe<TYPE>::lockedAddr (Int i)C {return ConstCast(T).lockedAddr (i);}
T1(TYPE)  C TYPE&  MemcThreadSafe<TYPE>::lockedElm  (Int i)C {return ConstCast(T).lockedElm  (i);}
T1(TYPE)  C TYPE&  MemcThreadSafe<TYPE>::lockedFirst(     )C {return ConstCast(T).lockedFirst( );}
T1(TYPE)  C TYPE&  MemcThreadSafe<TYPE>::lockedLast (     )C {return ConstCast(T).lockedLast ( );}

T1(TYPE)  Int   MemcThreadSafe<TYPE>::index   (C TYPE *elm)C {return _MemcThreadSafe::index   (elm);}
T1(TYPE)  Bool  MemcThreadSafe<TYPE>::contains(C TYPE *elm)C {return _MemcThreadSafe::contains(elm);}

T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::removeLast(                            ) {_MemcThreadSafe::removeLast(               ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::remove    (  Int   i  , Bool keep_order) {_MemcThreadSafe::remove    (i  , keep_order); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::removeData(C TYPE *elm, Bool keep_order) {_MemcThreadSafe::removeData(elm, keep_order); return T;}

T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::setNum    (Int num) {       _MemcThreadSafe::setNum    (num); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::setNumZero(Int num) {       _MemcThreadSafe::setNumZero(num); return T;}
T1(TYPE)  Int                    MemcThreadSafe<TYPE>::addNum    (Int num) {return _MemcThreadSafe::addNum    (num);          }

T1(TYPE) T1(VALUE)  Bool  MemcThreadSafe<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _MemcThreadSafe::binarySearch(&value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::          sort(Int compare(C TYPE &a, C TYPE &b)) {_MemcThreadSafe::          sort((Int(*)(CPtr, CPtr))compare); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::  reverseOrder(                                 ) {_MemcThreadSafe::  reverseOrder(                           ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::randomizeOrder(                                 ) {_MemcThreadSafe::randomizeOrder(                           ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::   rotateOrder(Int offset                       ) {_MemcThreadSafe::   rotateOrder(offset                     ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::     swapOrder(Int i  , Int j                   ) {_MemcThreadSafe::     swapOrder(i, j                       ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::     moveElm  (Int elm, Int new_index           ) {_MemcThreadSafe::     moveElm  (elm, new_index             ); return T;}

T1(TYPE)  void  MemcThreadSafe<TYPE>::  lock()C {_MemcThreadSafe::  lock();}
T1(TYPE)  void  MemcThreadSafe<TYPE>::unlock()C {_MemcThreadSafe::unlock();}

T1(TYPE)  MemcThreadSafe<TYPE>::MemcThreadSafe() : _MemcThreadSafe(SIZE(TYPE), ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
/******************************************************************************/
// MEMT
/******************************************************************************/
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::clear()
{
   if(ClassFunc<TYPE>::HasDel())REPA(T)T[i].~TYPE();
  _elms=0;
   return T;
}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::del()
{
   clear();
   if(_data){Free(_data); _max_elms=SIZE(_temp)/elmSize();}
   return T;
}

template<typename TYPE, Int size>  Int   Memt<TYPE, size>::elms    ()C {return _elms;}
template<typename TYPE, Int size>  UInt  Memt<TYPE, size>::elmSize ()C {return SIZE(TYPE);}
template<typename TYPE, Int size>  UInt  Memt<TYPE, size>::memUsage()C {return SIZE(T) + (_data ? elmSize()*_max_elms : 0);}

template<typename TYPE, Int size>  TYPE*  Memt<TYPE, size>::data      (     ) {                          return _data ? _data    :  (TYPE*)_temp    ;}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::operator[](Int i) {  RANGE_ASSERT(i, _elms); return _data ? _data[i] : ((TYPE*)_temp)[i];}
template<typename TYPE, Int size>  TYPE*  Memt<TYPE, size>::addr      (Int i) {return InRange(i, _elms) ? &T[i] : null;}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::first     (     ) {return T[       0];}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::last      (     ) {return T[elms()-1];}

template<typename TYPE, Int size>  TYPE  Memt<TYPE, size>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
template<typename TYPE, Int size>  TYPE  Memt<TYPE, size>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
template<typename TYPE, Int size>  TYPE  Memt<TYPE, size>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

template<typename TYPE, Int size>  C TYPE*  Memt<TYPE, size>::data      (     )C {return ConstCast(T).data ( );}
template<typename TYPE, Int size>  C TYPE*  Memt<TYPE, size>::addr      (Int i)C {return ConstCast(T).addr (i);}
template<typename TYPE, Int size>  C TYPE&  Memt<TYPE, size>::operator[](Int i)C {return ConstCast(T)      [i];}
template<typename TYPE, Int size>  C TYPE&  Memt<TYPE, size>::first     (     )C {return ConstCast(T).first( );}
template<typename TYPE, Int size>  C TYPE&  Memt<TYPE, size>::last      (     )C {return ConstCast(T).last ( );}

template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside 'Memt.operator()(Int i)'");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::New  (     ) {return T[addNum(1)];}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::NewAt(Int i)
{
   Clamp(i, 0, elms());
   Int old_elms=elms(); _elms++;
   if(elms()>_max_elms)
   {
     _max_elms=CeilPow2(elms());
      Ptr next=Alloc(_max_elms*elmSize());
      Copy((Byte*)next                , &T[0],           i *elmSize());
      Copy((Byte*)next+(i+1)*elmSize(), &T[i], (old_elms-i)*elmSize());
      Free(_data); _data=(TYPE*)next;
   }else
   if(i<old_elms)
   {
      Copy(&T[i+1], &T[i], (old_elms-i)*elmSize());
   }
   TYPE &elm=T[i]; new(&elm)TYPE; return elm;
}

template<typename TYPE, Int size>  Int  Memt<TYPE, size>::index(C TYPE *elm)C
{
   UIntPtr i=UIntPtr(elm)-UIntPtr(data());
   if(i<UIntPtr(elms()*elmSize()))return i/elmSize(); // unsigned compare will already guarantee "i>=0 && "
   return -1;
}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::contains(C TYPE *elm)C {return index(elm)>=0;}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::remove(Int i, Bool keep_order)
{
   if(InRange(i, T))
   {
      T[i].~TYPE();
      if(i<elms()-1) // if this is not the last element
      {
         if(keep_order)Copy(&T[i], &T[     i+1], elmSize()*(elms()-i-1));
         else          Copy(&T[i], &T[elms()-1], elmSize());
      }
     _elms--;
   }
   return T;
}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::removeLast(                            ) {return remove(elms()-1              );}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::removeData(C TYPE *elm, Bool keep_order) {return remove(index(elm), keep_order);}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::reserve(Int num)
{
   if(num>_max_elms)
   {
     _max_elms=CeilPow2(num);
      Ptr next=Alloc(_max_elms*elmSize());
      Copy(next, data(), elms()*elmSize());
      Free(_data); _data=(TYPE*)next;
   }
   return T;
}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::setNum(Int num)
{
   MAX(num, 0);
   if( num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num;
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE;
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=num; i<elms(); i++)T[i].~TYPE();
     _elms=num;
   }
   return T;
}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::setNumZero(Int num)
{
   MAX(num, 0);
   if( num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num;
      Zero(&T[old_elms], elmSize()*(elms()-old_elms));
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE;
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=num; i<elms(); i++)T[i].~TYPE();
     _elms=num;
   }
   return T;
}
template<typename TYPE, Int size>  Int  Memt<TYPE, size>::addNum(Int num) {Int index=elms(); setNum(elms()+num); return index;}

template<typename TYPE, Int size> T1(VALUE)  Bool  Memt<TYPE, size>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearch(data(), elms(), elmSize(), &value, index, (Int(*)(CPtr, CPtr))compare);}

template<typename TYPE, Int size>  Memt<TYPE, size>&   Memt<TYPE, size>::sort(Int compare(C TYPE &a, C TYPE &b)) {_Sort(data(), elms(), elmSize(), (Int(*)(CPtr, CPtr))compare); return T;}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::  reverseOrder(                      ) {  _ReverseOrder(data(), elms(), elmSize()                ); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::randomizeOrder(                      ) {_RandomizeOrder(data(), elms(), elmSize()                ); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::   rotateOrder(Int offset            ) {   _RotateOrder(data(), elms(), elmSize(), offset        ); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::     moveElm  (Int elm, Int new_index) {     _MoveElm  (data(), elms(), elmSize(), elm, new_index); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::     swapOrder(Int i  , Int j        ) {if(InRange(i, T) && InRange(j, T))        Swap(T[i], T[j]); return T;}

template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C Mems  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C Memc  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C Memt  <TYPE,     size> &src) {if(this!=Ptr(&src     )){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int size> template<Int src_size>  Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C Memt  <TYPE, src_size> &src) {if(this!=Ptr(&src     )){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C Memb  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C Memx  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C Meml  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size> template<Int src_size>  Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C MemPtr<TYPE, src_size> &src) {if(this!=Ptr(src._memt)){setNum(src.elms()); FREPAO(T)=src[i];} return T;}

template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C Mems   <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C Memc   <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C Memt   <TYPE,  src_size> &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C MemtN  <TYPE, Memt_elms> &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C Memb   <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C Memx   <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C Meml   <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C MemPtr<TYPE,  src_size> &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::saveRawData(File &f)C {return f.putN(data(), elms());}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::loadRawData(File &f)  {return f.getN(data(), elms());}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::saveRaw(File &f)C {       f.cmpUIntV(elms()) ; saveRawData(f); return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::loadRaw(File &f)  {setNum(f.decUIntV(      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}

#if EE_PRIVATE
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::loadRawDataFast(File &f) {return f.getFastN(data(), elms());}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::_saveRaw(File &f)C {       f.putInt  (elms()) ; saveRawData(f); return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::_loadRaw(File &f)  {setNum(f.getInt  (      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}
#endif

template<typename TYPE, Int size>  Memt<TYPE, size>::~Memt(           )          {del();}
template<typename TYPE, Int size>  Memt<TYPE, size>:: Memt(           )          {_data=null; _elms=0; _max_elms=SIZE(_temp)/elmSize();} // '_data' being set to 'null' instead of '_temp' allows for moving 'Memt' into another memory address
template<typename TYPE, Int size>  Memt<TYPE, size>:: Memt(C Memt &src) : Memt() {T=src;}
/******************************************************************************/
// MEMB
/******************************************************************************/
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::clear() {_Memb::clear(); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::del  () {_Memb::del  (); return T;}

T1(TYPE)  Int   Memb<TYPE>::  elms    ()C {return _Memb::  elms    ();}
T1(TYPE)  UInt  Memb<TYPE>::  elmSize ()C {return _Memb::  elmSize ();}
T1(TYPE)  UInt  Memb<TYPE>::blockElms ()C {return _Memb::blockElms ();}
T1(TYPE)  UInt  Memb<TYPE>::  memUsage()C {return _Memb::  memUsage();}

T1(TYPE)  TYPE*  Memb<TYPE>::addr      (Int i) {return  (TYPE*)_Memb::addr      (i);}
T1(TYPE)  TYPE&  Memb<TYPE>::operator[](Int i) {return *(TYPE*)_Memb::operator[](i);}
T1(TYPE)  TYPE&  Memb<TYPE>::operator()(Int i) {return *(TYPE*)_Memb::operator()(i);}
T1(TYPE)  TYPE&  Memb<TYPE>::first     (     ) {return *(TYPE*)_Memb::first     ( );}
T1(TYPE)  TYPE&  Memb<TYPE>::last      (     ) {return *(TYPE*)_Memb::last      ( );}
T1(TYPE)  TYPE&  Memb<TYPE>::New       (     ) {return *(TYPE*)_Memb::New       ( );}
T1(TYPE)  TYPE&  Memb<TYPE>::NewAt     (Int i) {return *(TYPE*)_Memb::NewAt     (i);}

T1(TYPE)  TYPE  Memb<TYPE>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
T1(TYPE)  TYPE  Memb<TYPE>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
T1(TYPE)  TYPE  Memb<TYPE>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

T1(TYPE)  C TYPE*  Memb<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  Memb<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  Memb<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  Memb<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  Int   Memb<TYPE>::index   (C TYPE *elm)C {return _Memb::index   (elm);}
T1(TYPE)  Bool  Memb<TYPE>::contains(C TYPE *elm)C {return _Memb::contains(elm);}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::removeLast(                            ) {_Memb::removeLast(               ); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::remove    (  Int   i  , Bool keep_order) {_Memb::remove    (i  , keep_order); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::removeData(C TYPE *elm, Bool keep_order) {_Memb::removeData(elm, keep_order); return T;}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::setNum    (Int num) {       _Memb::setNum    (num); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::setNumZero(Int num) {       _Memb::setNumZero(num); return T;}
T1(TYPE)  Int          Memb<TYPE>::addNum    (Int num) {return _Memb::addNum    (num);          }

T1(TYPE) T1(VALUE)  Bool  Memb<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearch(T, &value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::sort(Int compare(C TYPE &a, C TYPE &b)) {_Sort(T, (Int(*)(CPtr, CPtr))compare); return T;}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::reverseOrder(                      ) {_Memb::reverseOrder(              ); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::   swapOrder(Int i  , Int j        ) {_Memb::   swapOrder(i  , j        ); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::     moveElm(Int elm, Int new_index) {_Memb::     moveElm(elm, new_index); return T;}

T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C Mems  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C Memc  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memb<TYPE>&  Memb<TYPE>::operator=(C Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C Memb  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C Memx  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memb<TYPE>&  Memb<TYPE>::operator=(C MemPtr<TYPE, size>  &src) {if(this!=src._memb){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(  Memb  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE) T1(EXTENDED)  Memb<TYPE>&  Memb<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>();           _Memb::_reset(SIZE(EXTENDED), _block_elms, ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )               Memb<TYPE>::operator   Memb<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memb<BASE>*)this;}
T1(TYPE) T1(BASE    )               Memb<TYPE>::operator C Memb<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memb<BASE>*)this;}

T1(TYPE)  Bool  Memb<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memb<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memb<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Bool  Memb<TYPE>::saveRaw(File &f)C {return _Memb::saveRaw(f);}
T1(TYPE)  Bool  Memb<TYPE>::loadRaw(File &f)  {return _Memb::loadRaw(f);}

T1(TYPE)  Memb<TYPE>::Memb(Int block_elms) : _Memb(SIZE(TYPE)   ,     block_elms , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Memb<TYPE>::Memb(C Memb  &src  ) : _Memb(src.elmSize(), src.blockElms(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Memb<TYPE>::Memb(  Memb &&src  ) : _Memb(            0,               0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// MEMB ABSTRACT
/******************************************************************************/
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::clear() {_Memb::clear(); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::del  () {_Memb::del  (); return T;}

T1(TYPE)  Int   MembAbstract<TYPE>::  elms   ()C {return _Memb::  elms   ();}
T1(TYPE)  UInt  MembAbstract<TYPE>::  elmSize()C {return _Memb::  elmSize();}
T1(TYPE)  UInt  MembAbstract<TYPE>::blockElms()C {return _Memb::blockElms();}

T1(TYPE)  TYPE*  MembAbstract<TYPE>::addr      (Int i) {return  (TYPE*)_Memb::addr      (i);}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::operator[](Int i) {return *(TYPE*)_Memb::operator[](i);}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::operator()(Int i) {return *(TYPE*)_Memb::operator()(i);}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::first     (     ) {return *(TYPE*)_Memb::first     ( );}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::last      (     ) {return *(TYPE*)_Memb::last      ( );}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::New       (     ) {return *(TYPE*)_Memb::New       ( );}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::NewAt     (Int i) {return *(TYPE*)_Memb::NewAt     (i);}

T1(TYPE)  C TYPE*  MembAbstract<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  MembAbstract<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  MembAbstract<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  MembAbstract<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  Int   MembAbstract<TYPE>::index   (C TYPE *elm)C {return _Memb::index   (elm);}
T1(TYPE)  Bool  MembAbstract<TYPE>::contains(C TYPE *elm)C {return _Memb::contains(elm);}

T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::removeLast(                            ) {_Memb::removeLast(               ); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::remove    (  Int   i  , Bool keep_order) {_Memb::remove    (i  , keep_order); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::removeData(C TYPE *elm, Bool keep_order) {_Memb::removeData(elm, keep_order); return T;}

T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::setNum    (Int num) {       _Memb::setNum    (num); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::setNumZero(Int num) {       _Memb::setNumZero(num); return T;}
T1(TYPE)  Int                  MembAbstract<TYPE>::addNum    (Int num) {return _Memb::addNum    (num);          }

T1(TYPE) T1(EXTENDED)  MembAbstract<TYPE>&  MembAbstract<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>();           _Memb::_reset(SIZE(EXTENDED), _block_elms, ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )                       MembAbstract<TYPE>::operator   Memb<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memb<BASE>*)this;}
T1(TYPE) T1(BASE    )                       MembAbstract<TYPE>::operator C Memb<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memb<BASE>*)this;}

T1(TYPE)  MembAbstract<TYPE>::MembAbstract(Int block_elms) : _Memb(0, block_elms, null, null) {}
/******************************************************************************/
// MEMB CONST
/******************************************************************************/
T1(TYPE)  TYPE*  MembConst<TYPE>::addr      (Int i)C {return ConstCast(T).Memb<TYPE>::addr      (i);}
T1(TYPE)  TYPE&  MembConst<TYPE>::operator[](Int i)C {return ConstCast(T).Memb<TYPE>::operator[](i);}
T1(TYPE)  TYPE&  MembConst<TYPE>::first     (     )C {return ConstCast(T).Memb<TYPE>::first     ( );}
T1(TYPE)  TYPE&  MembConst<TYPE>::last      (     )C {return ConstCast(T).Memb<TYPE>::last      ( );}

T1(TYPE) T1(BASE)  MembConst<TYPE>::operator   MembConst<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(  MembConst<BASE>*)this;}
T1(TYPE) T1(BASE)  MembConst<TYPE>::operator C MembConst<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(C MembConst<BASE>*)this;}
/******************************************************************************/
// MEMX
/******************************************************************************/
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::clear() {_Memx::clear(); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::del  () {_Memx::del  (); return T;}

T1(TYPE)  Int   Memx<TYPE>::  absElms ()C {return _Memx::  absElms ();}
T1(TYPE)  Int   Memx<TYPE>::validElms ()C {return _Memx::validElms ();}
T1(TYPE)  Int   Memx<TYPE>::     elms ()C {return _Memx::     elms ();}
T1(TYPE)  UInt  Memx<TYPE>::  elmSize ()C {return _Memx::  elmSize ();}
T1(TYPE)  UInt  Memx<TYPE>::  memUsage()C {return _Memx::  memUsage();}

T1(TYPE)  TYPE&  Memx<TYPE>::    absElm(Int i) {return *(TYPE*)_Memx::    absElm(i);}
T1(TYPE)  TYPE&  Memx<TYPE>::  validElm(Int i) {return *(TYPE*)_Memx::  validElm(i);}
T1(TYPE)  TYPE*  Memx<TYPE>::      addr(Int i) {return  (TYPE*)_Memx::      addr(i);}
T1(TYPE)  TYPE&  Memx<TYPE>::operator[](Int i) {return *(TYPE*)_Memx::operator[](i);}
T1(TYPE)  TYPE&  Memx<TYPE>::     first(     ) {return *(TYPE*)_Memx::     first( );}
T1(TYPE)  TYPE&  Memx<TYPE>::      last(     ) {return *(TYPE*)_Memx::      last( );}
T1(TYPE)  TYPE&  Memx<TYPE>::     New  (     ) {return *(TYPE*)_Memx::     New  ( );}
T1(TYPE)  TYPE&  Memx<TYPE>::     NewAt(Int i) {return *(TYPE*)_Memx::     NewAt(i);}

T1(TYPE)  C TYPE&  Memx<TYPE>::    absElm(Int i)C {return ConstCast(T).  absElm(i);}
T1(TYPE)  C TYPE&  Memx<TYPE>::  validElm(Int i)C {return ConstCast(T).validElm(i);}
T1(TYPE)  C TYPE*  Memx<TYPE>::      addr(Int i)C {return ConstCast(T).    addr(i);}
T1(TYPE)  C TYPE&  Memx<TYPE>::operator[](Int i)C {return ConstCast(T)         [i];}
T1(TYPE)  C TYPE&  Memx<TYPE>::     first(     )C {return ConstCast(T).   first( );}
T1(TYPE)  C TYPE&  Memx<TYPE>::      last(     )C {return ConstCast(T).    last( );}

T1(TYPE)  Int   Memx<TYPE>::validToAbsIndex(  Int valid)C {return _Memx::validToAbsIndex(valid);}
T1(TYPE)  Int   Memx<TYPE>::absToValidIndex(  Int   abs)C {return _Memx::absToValidIndex(abs  );}
T1(TYPE)  Int   Memx<TYPE>::validIndex     (C TYPE *elm)C {return _Memx::validIndex     (elm  );}
T1(TYPE)  Int   Memx<TYPE>::  absIndex     (C TYPE *elm)C {return _Memx::  absIndex     (elm  );}
T1(TYPE)  Bool  Memx<TYPE>::  contains     (C TYPE *elm)C {return _Memx::  contains     (elm  );}

T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeAbs  (  Int   i  , Bool keep_order) {_Memx::removeAbs  (i  , keep_order); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeValid(  Int   i  , Bool keep_order) {_Memx::removeValid(i  , keep_order); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeData (C TYPE *elm, Bool keep_order) {_Memx::removeData (elm, keep_order); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeLast (                            ) {_Memx::removeLast (               ); return T;}

T1(TYPE) T1(VALUE)  Bool  Memx<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearch(T, &value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::sort(Int compare(C TYPE &a, C TYPE &b)) {_Sort(T, (Int(*)(CPtr, CPtr))compare); return T;}

T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::reverseOrder(                      ) {_Memx::reverseOrder(              ); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::   swapOrder(Int i  , Int j        ) {_Memx::   swapOrder(i  , j        ); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::moveElm     (Int elm, Int new_index) {_Memx::moveElm     (elm, new_index); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::moveToStart (Int elm               ) {_Memx::moveToStart (elm           ); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::moveToEnd   (Int elm               ) {_Memx::moveToEnd   (elm           ); return T;}

T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C Mems  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C Memc  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memx<TYPE>&  Memx<TYPE>::operator=(C Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C Memb  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C Memx  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memx<TYPE>&  Memx<TYPE>::operator=(C MemPtr<TYPE, size>  &src) {if(this!=src._memx){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(  Memx  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE) T1(EXTENDED)  Memx<TYPE>&  Memx<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>();           _Memx::_reset(SIZE(EXTENDED), _abs.blockElms(), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )               Memx<TYPE>::operator   Memx<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memx<BASE>*)this;}
T1(TYPE) T1(BASE    )               Memx<TYPE>::operator C Memx<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memx<BASE>*)this;}

T1(TYPE)  Bool  Memx<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memx<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memx<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Memx<TYPE>::Memx(Int block_elms) : _Memx(SIZE(TYPE)   , block_elms          , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Memx<TYPE>::Memx(C Memx  &src  ) : _Memx(src.elmSize(), src._abs.blockElms(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Memx<TYPE>::Memx(  Memx &&src  ) : _Memx(            0,                    0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// MEMX ABSTRACT
/******************************************************************************/
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::clear() {_Memx::clear(); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::del  () {_Memx::del  (); return T;}

T1(TYPE)  Int   MemxAbstract<TYPE>::  absElms ()C {return _Memx::  absElms ();}
T1(TYPE)  Int   MemxAbstract<TYPE>::validElms ()C {return _Memx::validElms ();}
T1(TYPE)  Int   MemxAbstract<TYPE>::     elms ()C {return _Memx::     elms ();}
T1(TYPE)  UInt  MemxAbstract<TYPE>::  elmSize ()C {return _Memx::  elmSize ();}
T1(TYPE)  UInt  MemxAbstract<TYPE>::  memUsage()C {return _Memx::  memUsage();}

T1(TYPE)  TYPE&  MemxAbstract<TYPE>::    absElm(Int i) {return *(TYPE*)_Memx::    absElm(i);}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::  validElm(Int i) {return *(TYPE*)_Memx::  validElm(i);}
T1(TYPE)  TYPE*  MemxAbstract<TYPE>::      addr(Int i) {return  (TYPE*)_Memx::      addr(i);}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::operator[](Int i) {return *(TYPE*)_Memx::operator[](i);}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::     first(     ) {return *(TYPE*)_Memx::     first( );}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::      last(     ) {return *(TYPE*)_Memx::      last( );}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::     New  (     ) {return *(TYPE*)_Memx::     New  ( );}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::     NewAt(Int i) {return *(TYPE*)_Memx::     NewAt(i);}

T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::    absElm(Int i)C {return ConstCast(T).  absElm(i);}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::  validElm(Int i)C {return ConstCast(T).validElm(i);}
T1(TYPE)  C TYPE*  MemxAbstract<TYPE>::      addr(Int i)C {return ConstCast(T).    addr(i);}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::operator[](Int i)C {return ConstCast(T)         [i];}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::     first(     )C {return ConstCast(T).   first( );}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::      last(     )C {return ConstCast(T).    last( );}

T1(TYPE)  Int   MemxAbstract<TYPE>::validToAbsIndex(  Int valid)C {return _Memx::validToAbsIndex(valid);}
T1(TYPE)  Int   MemxAbstract<TYPE>::absToValidIndex(  Int   abs)C {return _Memx::absToValidIndex(abs  );}
T1(TYPE)  Int   MemxAbstract<TYPE>::validIndex     (C TYPE *elm)C {return _Memx::validIndex     (elm  );}
T1(TYPE)  Int   MemxAbstract<TYPE>::  absIndex     (C TYPE *elm)C {return _Memx::  absIndex     (elm  );}
T1(TYPE)  Bool  MemxAbstract<TYPE>::  contains     (C TYPE *elm)C {return _Memx::  contains     (elm  );}

T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeAbs  (  Int   i  , Bool keep_order) {_Memx::removeAbs  (i  , keep_order); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeValid(  Int   i  , Bool keep_order) {_Memx::removeValid(i  , keep_order); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeData (C TYPE *elm, Bool keep_order) {_Memx::removeData (elm, keep_order); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeLast (                            ) {_Memx::removeLast (               ); return T;}

T1(TYPE) T1(EXTENDED)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>();           _Memx::_reset(SIZE(EXTENDED), _abs.blockElms(), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )                       MemxAbstract<TYPE>::operator   Memx<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memx<BASE>*)this;}
T1(TYPE) T1(BASE    )                       MemxAbstract<TYPE>::operator C Memx<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memx<BASE>*)this;}

T1(TYPE)  MemxAbstract<TYPE>::MemxAbstract(Int block_elms) : _Memx(0, block_elms, null, null) {}
/******************************************************************************/
// MEML
/******************************************************************************/
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::del  () {_Meml::del  (); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::clear() {_Meml::clear(); return T;}

T1(TYPE)  Int   Meml<TYPE>::elms    ()C {return _Meml::elms    ();}
T1(TYPE)  UInt  Meml<TYPE>::elmSize ()C {return _Meml::elmSize ();}
T1(TYPE)  UInt  Meml<TYPE>::memUsage()C {return _Meml::memUsage();}

T1(TYPE)  TYPE*  Meml<TYPE>::addr      (Int       i   ) {return  (TYPE*)_Meml::addr      (i);}
T1(TYPE)  TYPE&  Meml<TYPE>::operator[](Int       i   ) {return *(TYPE*)_Meml::operator[](i);}
T1(TYPE)  TYPE&  Meml<TYPE>::operator()(Int       i   ) {return *(TYPE*)_Meml::operator()(i);}
T1(TYPE)  TYPE&  Meml<TYPE>::operator[](MemlNode *node) {return *(TYPE*) node->data      ( );}
T1(TYPE)  TYPE&  Meml<TYPE>::New       (              ) {return *(TYPE*)_Meml::New       ( );}
T1(TYPE)  TYPE&  Meml<TYPE>::NewAt     (Int       i   ) {return *(TYPE*)_Meml::NewAt     (i);}

T1(TYPE)  C TYPE*  Meml<TYPE>::addr      (Int       i   )C {return ConstCast(T).addr(i   );}
T1(TYPE)  C TYPE&  Meml<TYPE>::operator[](Int       i   )C {return ConstCast(T)     [i   ];}
T1(TYPE)  C TYPE&  Meml<TYPE>::operator[](MemlNode *node)C {return ConstCast(T)     [node];}

T1(TYPE)  MemlNode*  Meml<TYPE>::add      (              ) {return _Meml::add      (    );}
T1(TYPE)  MemlNode*  Meml<TYPE>::addBefore(MemlNode *node) {return _Meml::addBefore(node);}
T1(TYPE)  MemlNode*  Meml<TYPE>::addAfter (MemlNode *node) {return _Meml::addAfter (node);}

T1(TYPE)  MemlNode*  Meml<TYPE>::first()C {return _Meml::first();}
T1(TYPE)  MemlNode*  Meml<TYPE>::last ()C {return _Meml::last ();}

T1(TYPE)  Int   Meml<TYPE>::index   (C TYPE *elm)C {return _Meml::index   (elm);}
T1(TYPE)  Bool  Meml<TYPE>::contains(C TYPE *elm)C {return _Meml::contains(elm);}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeFirst(                Bool keep_order) {_Meml::removeFirst(      keep_order); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeLast (                               ) {_Meml::removeLast (                ); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::remove     (MemlNode *node, Bool keep_order) {_Meml::remove     (node, keep_order); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeData (C TYPE   *elm , Bool keep_order) {_Meml::removeData (elm , keep_order); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeIndex(Int       i   , Bool keep_order) {_Meml::removeIndex(i   , keep_order); return T;}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::setNum    (Int num) {_Meml::setNum    (num); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::setNumZero(Int num) {_Meml::setNumZero(num); return T;}

T1(TYPE) T1(VALUE)  Bool  Meml<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearch(T, &value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::sort(Int compare(C TYPE &a, C TYPE &b)) {_Sort(T, (Int(*)(CPtr, CPtr))compare); return T;}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::reverseOrder(            ) {_Meml::reverseOrder(    ); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::   swapOrder(Int i, Int j) {_Meml::   swapOrder(i, j); return T;}

T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C Mems<TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C Memc<TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Meml<TYPE>&  Meml<TYPE>::operator=(C Memt<TYPE, size>  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C Memb<TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C Memx<TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C Meml<TYPE      >  &src) {if(this!=&src      ){setNum(src.elms()); for(MemlNode *d=first(), *s=src.first(); d && s; d=d->next(), s=s->next())T[d]=src[s];} return T;}
T1(TYPE) template<Int size>  Meml<TYPE>&  Meml<TYPE>::operator=(C MemPtr<TYPE, size>  &src) {if(this!= src._meml){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(  Meml  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE)  Bool  Meml<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; MFREP(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Meml<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; MFREP(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Meml<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); MFREP(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Meml<TYPE>::Meml(            ) : _Meml(SIZE(TYPE)   , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Meml<TYPE>::Meml(C Meml  &src) : _Meml(src.elmSize(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Meml<TYPE>::Meml(  Meml &&src) : _Meml(            0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// MEMP
/******************************************************************************/
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       null_t                              ) {_mode=     PTR ; _ptr =null     ; _elms=        0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       TYPE             &src               ) {_mode=     PTR ; _ptr =&src     ; _elms=        1; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       TYPE             *src, Int src_elms ) {_mode=     PTR ; _ptr = src     ; _elms= src_elms; return T;}
template<typename TYPE, Int Memt_size> template<Int src_elms>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       TYPE            (&src)    [src_elms]) {_mode=     PTR ; _ptr = src     ; _elms= src_elms; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Mems  <TYPE           > &src               ) {_mode=     MEMS; _mems=&src     ; _elms=        0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memc  <TYPE           > &src               ) {_mode=     MEMC; _memc=&src     ; _elms=        0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memt  <TYPE, Memt_size> &src               ) {_mode=     MEMT; _memt=&src     ; _elms=        0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memb  <TYPE           > &src               ) {_mode=     MEMB; _memb=&src     ; _elms=        0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memx  <TYPE           > &src               ) {_mode=     MEMX; _memx=&src     ; _elms=        0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Meml  <TYPE           > &src               ) {_mode=     MEML; _meml=&src     ; _elms=        0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(MemPtr<TYPE, Memt_size> &src               ) {_mode=src._mode; _ptr = src._ptr; _elms=src._elms; return T;}

template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::clear()
{
   switch(_mode)
   {
      case PTR :  if(_elms)Exit("'MemPtr.clear' does not support PTR mode"); break;
      case MEMS: _mems->clear(); break;
      case MEMC: _memc->clear(); break;
      case MEMT: _memt->clear(); break;
      case MEMB: _memb->clear(); break;
      case MEMX: _memx->clear(); break;
      case MEML: _meml->clear(); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::del()
{
   switch(_mode)
   {
      case PTR :  if(_elms)Exit("'MemPtr.del' does not support PTR mode"); break;
      case MEMS: _mems->del(); break;
      case MEMC: _memc->del(); break;
      case MEMT: _memt->del(); break;
      case MEMB: _memb->del(); break;
      case MEMX: _memx->del(); break;
      case MEML: _meml->del(); break;
   }
   return T;
}

template<typename TYPE, Int Memt_size>  Int  MemPtr<TYPE, Memt_size>::elms()C
{
   switch(_mode)
   {
      default  : return _elms; // PTR
      case MEMS: return _mems->elms();
      case MEMC: return _memc->elms();
      case MEMT: return _memt->elms();
      case MEMB: return _memb->elms();
      case MEMX: return _memx->elms();
      case MEML: return _meml->elms();
   }
}
template<typename TYPE, Int Memt_size>  UInt  MemPtr<TYPE, Memt_size>::elmSize()C
{
   switch(_mode)
   {
      default  : return  SIZE(TYPE); // PTR
      case MEMS: return _mems->elmSize();
      case MEMC: return _memc->elmSize();
      case MEMT: return _memt->elmSize();
      case MEMB: return _memb->elmSize();
      case MEMX: return _memx->elmSize();
      case MEML: return _meml->elmSize();
   }
}
template<typename TYPE, Int Memt_size>  UInt  MemPtr<TYPE, Memt_size>::memUsage()C
{
   switch(_mode)
   {
      default  : return  elms()*elmSize(); // PTR
      case MEMS: return _mems->memUsage();
      case MEMC: return _memc->memUsage();
      case MEMT: return _memt->memUsage();
      case MEMB: return _memb->memUsage();
      case MEMX: return _memx->memUsage();
      case MEML: return _meml->memUsage();
   }
}
template<typename TYPE, Int Memt_size>  TYPE*  MemPtr<TYPE, Memt_size>::data()
{
   switch(_mode)
   {
      default  : return _ptr; // PTR
      case MEMS: return _mems->data();
      case MEMC: return _memc->data();
      case MEMT: return _memt->data();
      case MEMB: Exit("'MemPtr.data' does not support MEMB mode"); return null;
      case MEMX: Exit("'MemPtr.data' does not support MEMX mode"); return null;
      case MEML: Exit("'MemPtr.data' does not support MEML mode"); return null;
   }
}
template<typename TYPE, Int Memt_size>  TYPE*  MemPtr<TYPE, Memt_size>::addr(Int i)
{
   switch(_mode)
   {
      default  : return  InRange(i, _elms) ? &_ptr[i] : null; // PTR
      case MEMS: return _mems->addr(i);
      case MEMC: return _memc->addr(i);
      case MEMT: return _memt->addr(i);
      case MEMB: return _memb->addr(i);
      case MEMX: return _memx->addr(i);
      case MEML: return _meml->addr(i);
   }
}
template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::operator[](Int i)
{
   switch(_mode)
   {
      default  : RANGE_ASSERT(i, _elms); return _ptr[i]; // PTR
      case MEMS: return (*_mems)[i];
      case MEMC: return (*_memc)[i];
      case MEMT: return (*_memt)[i];
      case MEMB: return (*_memb)[i];
      case MEMX: return (*_memx)[i];
      case MEML: return (*_meml)[i];
   }
}
template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::operator()(Int i)
{
   switch(_mode)
   {
      default  : if(!InRange(i, _elms))Exit("'MemPtr.operator(Int)' PTR mode out of range"); return _ptr[i]; // PTR
      case MEMS: return (*_mems)(i);
      case MEMC: return (*_memc)(i);
      case MEMT: return (*_memt)(i);
      case MEMB: return (*_memb)(i);
      case MEMX: Exit("'MemPtr.operator(Int)' does not support MEMX mode"); return *(TYPE*)null;
      case MEML: return (*_meml)(i);
   }
}
template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::first() {return T[0       ];}
template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::last () {return T[elms()-1];}

template<typename TYPE, Int Memt_size>  C TYPE*  MemPtr<TYPE, Memt_size>::data      (     )C {return ConstCast(T).data ( );}
template<typename TYPE, Int Memt_size>  C TYPE*  MemPtr<TYPE, Memt_size>::addr      (Int i)C {return ConstCast(T).addr (i);}
template<typename TYPE, Int Memt_size>  C TYPE&  MemPtr<TYPE, Memt_size>::operator[](Int i)C {return ConstCast(T)      [i];}
template<typename TYPE, Int Memt_size>  C TYPE&  MemPtr<TYPE, Memt_size>::first     (     )C {return ConstCast(T).first( );}
template<typename TYPE, Int Memt_size>  C TYPE&  MemPtr<TYPE, Memt_size>::last      (     )C {return ConstCast(T).last ( );}

template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::New()
{
   switch(_mode)
   {
      default  : Exit("'MemPtr.New' does not support PTR mode"); return *(TYPE*)null; // PTR
      case MEMS: return _mems->New();
      case MEMC: return _memc->New();
      case MEMT: return _memt->New();
      case MEMB: return _memb->New();
      case MEMX: return _memx->New();
      case MEML: return _meml->New();
   }
}
template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::NewAt(Int i)
{
   switch(_mode)
   {
      default  : Exit("'MemPtr.NewAt' does not support PTR mode"); return *(TYPE*)null; // PTR
      case MEMS: return _mems->NewAt(i);
      case MEMC: return _memc->NewAt(i);
      case MEMT: return _memt->NewAt(i);
      case MEMB: return _memb->NewAt(i);
      case MEMX: return _memx->NewAt(i);
      case MEML: return _meml->NewAt(i);
   }
}
template<typename TYPE, Int Memt_size>  Int  MemPtr<TYPE, Memt_size>::index(C TYPE *elm)C
{
   switch(_mode)
   {
      case PTR : {Int i=elm-_ptr; if(InRange(i, _elms))return i;} break;
      case MEMS: return _mems->index(elm);
      case MEMC: return _memc->index(elm);
      case MEMT: return _memt->index(elm);
      case MEMB: return _memb->index(elm);
      case MEMX: return _memx->validIndex(elm);
      case MEML: return _meml->index(elm);
   }
   return -1;
}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::contains(C TYPE *elm)C {return index(elm)>=0;}

template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::removeLast()
{
   switch(_mode)
   {
      case PTR : if(_elms)Exit("'MemPtr.removeLast' does not support PTR mode"); break;
      case MEMS: _mems->removeLast(); break;
      case MEMC: _memc->removeLast(); break;
      case MEMT: _memt->removeLast(); break;
      case MEMB: _memb->removeLast(); break;
      case MEMX: _memx->removeLast(); break;
      case MEML: _meml->removeLast(); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::remove(Int i, Bool keep_order)
{
   switch(_mode)
   {
      case PTR : if(InRange(i, _elms))Exit("'MemPtr.remove' does not support PTR mode"); break;
      case MEMS: _mems->remove     (i, keep_order); break;
      case MEMC: _memc->remove     (i, keep_order); break;
      case MEMT: _memt->remove     (i, keep_order); break;
      case MEMB: _memb->remove     (i, keep_order); break;
      case MEMX: _memx->removeValid(i, keep_order); break;
      case MEML: _meml->removeIndex(i, keep_order); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::removeData(C TYPE *elm, Bool keep_order)
{
   switch(_mode)
   {
      case PTR : if(InRange(index(elm), _elms))Exit("'MemPtr.removeData' does not support PTR mode"); break;
      case MEMS: _mems->removeData(elm, keep_order); break;
      case MEMC: _memc->removeData(elm, keep_order); break;
      case MEMT: _memt->removeData(elm, keep_order); break;
      case MEMB: _memb->removeData(elm, keep_order); break;
      case MEMX: _memx->removeData(elm, keep_order); break;
      case MEML: _meml->removeData(elm, keep_order); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::setNum(Int num)
{
   switch(_mode)
   {
      case PTR : if(_elms!=num)Exit("'MemPtr.setNum' does not support PTR mode"); break;
      case MEMS: _mems->setNum(num); break;
      case MEMC: _memc->setNum(num); break;
      case MEMT: _memt->setNum(num); break;
      case MEMB: _memb->setNum(num); break;
      case MEMX: _memx->setNum(num); break;
      case MEML: _meml->setNum(num); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::setNumZero(Int num)
{
   switch(_mode)
   {
      case PTR : if(_elms!=num)Exit("'MemPtr.setNumZero' does not support PTR mode"); break;
      case MEMS: _mems->setNumZero(num); break;
      case MEMC: _memc->setNumZero(num); break;
      case MEMT: _memt->setNumZero(num); break;
      case MEMB: _memb->setNumZero(num); break;
      case MEMX:  Exit("'MemPtr.setNumZero' does not support MEMX mode"); break;
      case MEML: _meml->setNumZero(num); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  Int  MemPtr<TYPE, Memt_size>::addNum(Int num)
{
   switch(_mode)
   {
      default  : if(num)Exit("'MemPtr.addNum' does not support PTR mode"); return _elms; // PTR
      case MEMS: return _mems->addNum(num);
      case MEMC: return _memc->addNum(num);
      case MEMT: return _memt->addNum(num);
      case MEMB: return _memb->addNum(num);
      case MEMX: return _memx->addNum(num);
      case MEML: return _meml->addNum(num);
   }
}

template<typename TYPE, Int Memt_size> T1(VALUE)  Bool  MemPtr<TYPE, Memt_size>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C
{
   switch(_mode)
   {
      default  : return        BinarySearch(_ptr, _elms, value, index, compare); // PTR
      case MEMS: return _mems->binarySearch(             value, index, compare);
      case MEMC: return _memc->binarySearch(             value, index, compare);
      case MEMT: return _memt->binarySearch(             value, index, compare);
      case MEMB: return _memb->binarySearch(             value, index, compare);
      case MEMX: return _memx->binarySearch(             value, index, compare);
      case MEML: return _meml->binarySearch(             value, index, compare);
   }
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::sort(Int compare(C TYPE &a, C TYPE &b))
{
   switch(_mode)
   {
      case PTR :        Sort(_ptr, _elms, compare); break;
      case MEMS: _mems->sort(             compare); break;
      case MEMC: _memc->sort(             compare); break;
      case MEMT: _memt->sort(             compare); break;
      case MEMB: _memb->sort(             compare); break;
      case MEMX: _memx->sort(             compare); break;
      case MEML: _meml->sort(             compare); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::reverseOrder()
{
   switch(_mode)
   {
      case PTR :        ReverseOrder(_ptr, _elms); break;
      case MEMS: _mems->reverseOrder(           ); break;
      case MEMC: _memc->reverseOrder(           ); break;
      case MEMT: _memt->reverseOrder(           ); break;
      case MEMB: _memb->reverseOrder(           ); break;
      case MEMX: _memx->reverseOrder(           ); break;
      case MEML: _meml->reverseOrder(           ); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::swapOrder(Int i, Int j)
{
   switch(_mode)
   {
      case PTR :  if(InRange(i, _elms) && InRange(j, _elms))Swap(&T[i], &T[j], elmSize()); break;
      case MEMS: _mems->swapOrder(i, j); break;
      case MEMC: _memc->swapOrder(i, j); break;
      case MEMT: _memt->swapOrder(i, j); break;
      case MEMB: _memb->swapOrder(i, j); break;
      case MEMX: _memx->swapOrder(i, j); break;
      case MEML: _meml->swapOrder(i, j); break;
   }
   return T;
}

template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C TYPE                    &src           ) {                     setNum(         1);      T[0]=src   ;  return T;}
template<typename TYPE, Int Memt_size> template<Int src_elms>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C TYPE                   (&src)[src_elms]) {                     setNum(src_elms  ); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C Mems  <TYPE           > &src           ) {if(_mems!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C Memc  <TYPE           > &src           ) {if(_memc!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size> template<Int src_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C Memt  <TYPE,  src_size> &src           ) {if(_memt!=Ptr(&src)){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C Memb  <TYPE           > &src           ) {if(_memb!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C Memx  <TYPE           > &src           ) {if(_memx!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C Meml  <TYPE           > &src           ) {if(_meml!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C MemPtr<TYPE, Memt_size> &src           ) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int Memt_size> template<Int src_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C MemPtr<TYPE,  src_size> &src           ) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}

template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C TYPE                     &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_elms>  MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C TYPE                    (&src)[src_elms]) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C Mems   <TYPE           > &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C Memc   <TYPE           > &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C Memt   <TYPE,  src_size> &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C Memb   <TYPE           > &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C Memx   <TYPE           > &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C Meml   <TYPE           > &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C MemPtr <TYPE,  src_size> &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C MemPtrN<TYPE, Memt_elms> &src           ) {MemPtr<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}

#if EE_PRIVATE
template<typename TYPE, Int Memt_size>  void  MemPtr<TYPE, Memt_size>::copyTo(TYPE *dest)C
{
   switch(_mode)
   {
      case PTR :  CopyN(dest, _ptr, _elms); break;
      case MEMS: _mems->copyTo(dest); break;
      case MEMC: _memc->copyTo(dest); break;
      case MEMT: _memt->copyTo(dest); break;
      case MEMB: _memb->copyTo(dest); break;
      case MEMX: _memx->copyTo(dest); break;
      case MEML: _meml->copyTo(dest); break;
   }
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::copyFrom(C TYPE *src)
{
   switch(_mode)
   {
      case PTR :  CopyN(_ptr, src, _elms); break;
      case MEMS: _mems->copyFrom(src); break;
      case MEMC: _memc->copyFrom(src); break;
      case MEMT: _memt->copyFrom(src); break;
      case MEMB: _memb->copyFrom(src); break;
      case MEMX: _memx->copyFrom(src); break;
      case MEML: _meml->copyFrom(src); break;
   }
   return T;
}
#endif
template<typename TYPE, Int Memt_size>        MemPtr<TYPE, Memt_size>::operator Bool()C {return _mode!=PTR || elms();}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::resizable    ()C {return _mode!=PTR;}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::continuous   ()C {return _mode==PTR || _mode==MEMS || _mode==MEMC || _mode==MEMT;}

template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::save(File &f)C
{
   switch(_mode)
   {
      default  : f.cmpUIntV(elms()); FREPA(T)if(!T[i].save(f))return false; return f.ok(); // PTR
      case MEMS: return _mems->save(f);
      case MEMC: return _memc->save(f);
      case MEMT: return _memt->save(f);
      case MEMB: return _memb->save(f);
      case MEMX: return _memx->save(f);
      case MEML: return _meml->save(f);
   }
}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::save(File &f)
{
   switch(_mode)
   {
      default  : f.cmpUIntV(elms()); FREPA(T)if(!T[i].save(f))return false; return f.ok(); // PTR
      case MEMS: return _mems->save(f);
      case MEMC: return _memc->save(f);
      case MEMT: return _memt->save(f);
      case MEMB: return _memb->save(f);
      case MEMX: return _memx->save(f);
      case MEML: return _meml->save(f);
   }
}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::load(File &f)
{
   switch(_mode)
   {
      default  : if(f.decUIntV()!=elms())return false; FREPA(T)if(!T[i].load(f))return false; return f.ok(); // PTR
      case MEMS: return _mems->load(f);
      case MEMC: return _memc->load(f);
      case MEMT: return _memt->load(f);
      case MEMB: return _memb->load(f);
      case MEMX: return _memx->load(f);
      case MEML: return _meml->load(f);
   }
}

template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::saveRaw(File &f)C
{
   switch(_mode)
   {
      default  : f.cmpUIntV(elms()); f.putN(_ptr, elms()); return f.ok(); // PTR
      case MEMS: return _mems->saveRaw(f);
      case MEMC: return _memc->saveRaw(f);
      case MEMT: return _memt->saveRaw(f);
      case MEMB: return _memb->saveRaw(f);
      case MEMX: return _memx->saveRaw(f);
      case MEML: return _meml->saveRaw(f);
   }
}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::loadRaw(File &f)
{
   switch(_mode)
   {
      default  : if(f.decUIntV()!=elms())return false; f.getN(_ptr, elms()); return f.ok(); // PTR
      case MEMS: return _mems->loadRaw(f);
      case MEMC: return _memc->loadRaw(f);
      case MEMT: return _memt->loadRaw(f);
      case MEMB: return _memb->loadRaw(f);
      case MEMX: return _memx->loadRaw(f);
      case MEML: return _meml->loadRaw(f);
   }
}
/******************************************************************************/
// COUNTED POINTER
/******************************************************************************/
T1(TYPE)  void  CountedPtr<TYPE>::DecRef(TYPE *data) {ASSERT_BASE_EXTENDED<PtrCounter, TYPE>(); SCAST(PtrCounter, *data).decRef(ClassFunc<TYPE>::Unload   );}
T1(TYPE)  void  CountedPtr<TYPE>::IncRef(TYPE *data) {ASSERT_BASE_EXTENDED<PtrCounter, TYPE>(); SCAST(PtrCounter, *data).incRef(ClassFunc<TYPE>::LoadEmpty);}

T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::clear    (                   ) {            DecRef(T._data);        T._data=      null ;  return T;}
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(  TYPE       * data) {if(T!=data){DecRef(T._data); IncRef(T._data=      data);} return T;}
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(C CountedPtr & eptr) {if(T!=eptr){DecRef(T._data); IncRef(T._data=eptr._data);} return T;}
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(  CountedPtr &&eptr) {Swap(_data, eptr._data);                                  return T;}
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(  null_t           ) {clear();                                                  return T;}

T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(  null_t           ) {       T._data=      null ;}
T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(  TYPE       * data) {IncRef(T._data=      data);}
T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(C CountedPtr & eptr) {IncRef(T._data=eptr._data);}
T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(  CountedPtr &&eptr) {       T._data=eptr._data ; eptr._data=null;}
T1(TYPE)  CountedPtr<TYPE>::~CountedPtr(                   ) {clear();}
/******************************************************************************/
// CACHE
/******************************************************************************/
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::clear         (               ) {                   _Cache::clear         (         ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::del           (               ) {                   _Cache::del           (         ); return T;}
T1(TYPE)  CACHE_MODE    Cache<TYPE>::mode          (CACHE_MODE mode) {return (CACHE_MODE)_Cache::mode          (mode     );          }
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::caseSensitive (Bool  sensitive) {                   _Cache::caseSensitive (sensitive); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemove   (Flt   time     ) {                   _Cache::delayRemove   (time     ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemoveNow(               ) {                   _Cache::delayRemoveNow(         ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemoveInc(               ) {                   _Cache::delayRemoveInc(         ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemoveDec(               ) {                   _Cache::delayRemoveDec(         ); return T;}

T1(TYPE)  TYPE*  Cache<TYPE>::find      (C Str &file, CChar *path) {return (TYPE*)_Cache::_find   (file, path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::find      (C UID &id  , CChar *path) {return (TYPE*)_Cache::_find   (id  , path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::get       (C Str &file, CChar *path) {return (TYPE*)_Cache::_get    (file, path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::get       (C UID &id  , CChar *path) {return (TYPE*)_Cache::_get    (id  , path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::operator()(C Str &file, CChar *path) {return (TYPE*)_Cache::_require(file, path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::operator()(C UID &id  , CChar *path) {return (TYPE*)_Cache::_require(id  , path, false);}

T1(TYPE)  CChar*  Cache<TYPE>::name    (C TYPE *data, CChar *path)C {return _Cache::_name    (data,  path);}
T1(TYPE)  UID     Cache<TYPE>::id      (C TYPE *data             )C {return _Cache::_id      (data       );}
T1(TYPE)  Bool    Cache<TYPE>::contains(C TYPE *data             )C {return _Cache::_contains(data       );}
T1(TYPE)  Bool    Cache<TYPE>::dummy   (C TYPE *data             )C {return _Cache::_dummy   (data       );}
T1(TYPE)  void    Cache<TYPE>::dummy   (C TYPE *data, Bool  dummy)  {       _Cache::_dummy   (data, dummy);}

T1(TYPE)    Int           Cache<TYPE>::  elms      (     )C {return         _Cache::  elms      ( );}
T1(TYPE)    void          Cache<TYPE>::  lock      (     )C {               _Cache::  lock      ( );}
T1(TYPE) C _Cache::Desc&  Cache<TYPE>::  lockedDesc(Int i)C {return         _Cache::  lockedDesc(i);}
T1(TYPE)    TYPE       &  Cache<TYPE>::  lockedData(Int i)  {return *(TYPE*)_Cache::  lockedData(i);}
T1(TYPE) C  TYPE       &  Cache<TYPE>::  lockedData(Int i)C {return *(TYPE*)_Cache::  lockedData(i);}
T1(TYPE)    void          Cache<TYPE>::unlock      (     )C {               _Cache::unlock      ( );}

T1(TYPE)  void  Cache<TYPE>::removeData(C TYPE *data) {return _Cache::_removeData(data);}

T1(TYPE)  void  Cache<TYPE>::update() {return _Cache::update();}

T1(TYPE)  void  Cache<TYPE>::setLoadUser(Ptr user) {_Cache::setLoadUser(ClassFunc<TYPE>::LoadUser, user);}

T1(TYPE) T1(EXTENDED)  Cache<TYPE>&  Cache<TYPE>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); lock(); del(); /*_data_offset=OFFSET(typename Cache<EXTENDED>::Elm, data);*/ _desc_offset=OFFSET(typename Cache<EXTENDED>::Elm, desc); _memx.replaceClass<typename Cache<EXTENDED>::Elm>(); unlock(); return T;}

T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::operator=(C Cache<TYPE> &src) {if(this!=&src){lock(); src.lock(); _lockedFrom(src); FREPA(T)lockedData(i)=src.lockedData(i); src.unlock(); unlock();} return T;}

T1(TYPE)  Cache<TYPE>::Cache(CChar8 *name, Int block_elms) : _Cache(name, block_elms, ClassFunc<TYPE>::Load) {replaceClass<TYPE>();}

         inline Int Elms(C _Cache       &cache) {return cache.elms();}
T1(TYPE) inline Int Elms(C  Cache<TYPE> &cache) {return cache.elms();}
/******************************************************************************/
// CACHE ELEMENT POINTER
/******************************************************************************/
template<typename TYPE, Cache<TYPE> &CACHE>  CChar*  CacheElmPtr<TYPE,CACHE>::name (CChar *path )C {return CACHE._name (_data, path );}
template<typename TYPE, Cache<TYPE> &CACHE>  UID     CacheElmPtr<TYPE,CACHE>::id   (            )C {return CACHE._id   (_data       );}
template<typename TYPE, Cache<TYPE> &CACHE>  Bool    CacheElmPtr<TYPE,CACHE>::dummy(            )C {return CACHE._dummy(_data       );}
template<typename TYPE, Cache<TYPE> &CACHE>  void    CacheElmPtr<TYPE,CACHE>::dummy(Bool   dummy)  {       CACHE._dummy(_data, dummy);}

template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::clear    (                    ) {            CACHE._decRef(T._data);               T._data=      null ;  return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(  TYPE        * data) {if(T!=data){CACHE._decRef(T._data); CACHE._incRef(T._data=      data);} return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C CacheElmPtr & eptr) {if(T!=eptr){CACHE._decRef(T._data); CACHE._incRef(T._data=eptr._data);} return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(  CacheElmPtr &&eptr) {Swap(_data, eptr._data);                                                return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(  null_t            ) {clear();                                                                return T;}

template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (CChar  *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._find   (    file , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (CChar8 *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._find   (Str(file), path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (C Str  &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._find   (    file , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (C Str8 &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._find   (Str(file), path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (C UID  &id  , CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._find   (    id   , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (CChar  *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._get    (    file , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (CChar8 *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._get    (Str(file), path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (C Str  &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._get    (    file , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (C Str8 &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._get    (Str(file), path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (C UID  &id  , CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._get    (    id   , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (CChar  *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(    file , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (CChar8 *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(Str(file), path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (C Str  &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(    file , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (C Str8 &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(Str(file), path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (C UID  &id  , CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(    id   , path, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(CChar  *file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(    file , null, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(CChar8 *file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(Str(file), null, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C Str  &file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(    file , null, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C Str8 &file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(Str(file), null, true); CACHE._decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C UID  &id               ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._require(    id   , null, true); CACHE._decRef(old); return T;}

template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  null_t            ) {              T._data=      null ;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  TYPE        * data) {CACHE._incRef(T._data=      data);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C CacheElmPtr & eptr) {CACHE._incRef(T._data=eptr._data);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  CacheElmPtr &&eptr) {              T._data=eptr._data ; eptr._data=null;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  CChar       * file) {              T._data=(TYPE*)CACHE._require(    file , null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  CChar8      * file) {              T._data=(TYPE*)CACHE._require(Str(file), null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C Str         & file) {              T._data=(TYPE*)CACHE._require(    file , null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C Str8        & file) {              T._data=(TYPE*)CACHE._require(Str(file), null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C UID         & id  ) {              T._data=(TYPE*)CACHE._require(    id   , null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>::~CacheElmPtr(                    ) {clear();}
/******************************************************************************/
// MAP
/******************************************************************************/
T2(KEY, DATA)  Map<KEY, DATA>&  Map<KEY, DATA>::del  () {_Map::del  (); return T;}
T2(KEY, DATA)  Map<KEY, DATA>&  Map<KEY, DATA>::clear() {_Map::clear(); return T;}

T2(KEY, DATA)  ThreadSafeMap<KEY, DATA>&  ThreadSafeMap<KEY, DATA>::del  () {_MapTS::del  (); return T;}
T2(KEY, DATA)  ThreadSafeMap<KEY, DATA>&  ThreadSafeMap<KEY, DATA>::clear() {_MapTS::clear(); return T;}

T2(KEY, DATA)  Int  Map<KEY, DATA>::elms    ()C {return _Map::elms    ();}
T2(KEY, DATA)  Int  Map<KEY, DATA>::dataSize()C {return _Map::dataSize();}

T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::elms    ()C {return _MapTS::elms    ();}
T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::dataSize()C {return _MapTS::dataSize();}

T2(KEY, DATA)  DATA*  Map<KEY, DATA>::find      (C KEY &key) {return (DATA*)_Map::find      (&key);}
T2(KEY, DATA)  DATA*  Map<KEY, DATA>::get       (C KEY &key) {return (DATA*)_Map::get       (&key);}
T2(KEY, DATA)  DATA*  Map<KEY, DATA>::operator()(C KEY &key) {return (DATA*)_Map::operator()(&key);}

T2(KEY, DATA)  DATA*  ThreadSafeMap<KEY, DATA>::find      (C KEY &key) {return (DATA*)_MapTS::find      (&key);}
T2(KEY, DATA)  DATA*  ThreadSafeMap<KEY, DATA>::get       (C KEY &key) {return (DATA*)_MapTS::get       (&key);}
T2(KEY, DATA)  DATA*  ThreadSafeMap<KEY, DATA>::operator()(C KEY &key) {return (DATA*)_MapTS::operator()(&key);}

T2(KEY, DATA)  Int  Map<KEY, DATA>::   findAbsIndex(C KEY &key)C {return _Map::   findAbsIndex(&key);}
T2(KEY, DATA)  Int  Map<KEY, DATA>::    getAbsIndex(C KEY &key)  {return _Map::    getAbsIndex(&key);}
T2(KEY, DATA)  Int  Map<KEY, DATA>::requireAbsIndex(C KEY &key)  {return _Map::requireAbsIndex(&key);}

T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::   findAbsIndex(C KEY &key)C {return _MapTS::   findAbsIndex(&key);}
T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::    getAbsIndex(C KEY &key)  {return _MapTS::    getAbsIndex(&key);}
T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::requireAbsIndex(C KEY &key)  {return _MapTS::requireAbsIndex(&key);}

T2(KEY, DATA)    Bool  Map<KEY, DATA>::containsKey   (C KEY  &key )C {return        _Map::containsKey      (&key );}
T2(KEY, DATA)    Bool  Map<KEY, DATA>::containsData  (C DATA *data)C {return        _Map::containsData     ( data);}
T2(KEY, DATA)  C KEY*  Map<KEY, DATA>::dataToKey     (C DATA *data)C {return  (KEY*)_Map::dataToKey        ( data);}
T2(KEY, DATA)  C KEY*  Map<KEY, DATA>::dataInMapToKey(C DATA *data)C {return  (KEY*)_Map::dataInMapToKeyPtr( data);}
T2(KEY, DATA)  C KEY&  Map<KEY, DATA>::dataInMapToKey(C DATA &data)C {return *(KEY*)_Map::dataInMapToKeyRef(&data);}
T2(KEY, DATA)    Int   Map<KEY, DATA>::dataToIndex   (C DATA *data)C {return        _Map::dataToIndex      ( data);}

T2(KEY, DATA)    Bool  ThreadSafeMap<KEY, DATA>::containsKey   (C KEY  &key )C {return        _MapTS::containsKey      (&key );}
T2(KEY, DATA)    Bool  ThreadSafeMap<KEY, DATA>::containsData  (C DATA *data)C {return        _MapTS::containsData     ( data);}
T2(KEY, DATA)  C KEY*  ThreadSafeMap<KEY, DATA>::dataToKey     (C DATA *data)C {return  (KEY*)_MapTS::dataToKey        ( data);}
T2(KEY, DATA)  C KEY*  ThreadSafeMap<KEY, DATA>::dataInMapToKey(C DATA *data)C {return  (KEY*)_MapTS::dataInMapToKeyPtr( data);}
T2(KEY, DATA)  C KEY&  ThreadSafeMap<KEY, DATA>::dataInMapToKey(C DATA &data)C {return *(KEY*)_MapTS::dataInMapToKeyRef(&data);}
T2(KEY, DATA)    Int   ThreadSafeMap<KEY, DATA>::dataToIndex   (C DATA *data)C {return        _MapTS::dataToIndex      ( data);}

T2(KEY, DATA)  C KEY &  Map<KEY, DATA>::key       (Int i)C {return *(KEY *)_Map::key       (i);}
T2(KEY, DATA)    DATA&  Map<KEY, DATA>::operator[](Int i)  {return *(DATA*)_Map::operator[](i);}
T2(KEY, DATA)  C DATA&  Map<KEY, DATA>::operator[](Int i)C {return *(DATA*)_Map::operator[](i);}

T2(KEY, DATA)  C KEY &  Map<KEY, DATA>::absKey (Int abs_i)C {return *(KEY *)_Map::absKey (abs_i);}
T2(KEY, DATA)    DATA&  Map<KEY, DATA>::absData(Int abs_i)  {return *(DATA*)_Map::absData(abs_i);}
T2(KEY, DATA)  C DATA&  Map<KEY, DATA>::absData(Int abs_i)C {return *(DATA*)_Map::absData(abs_i);}

T2(KEY, DATA)  C KEY &  ThreadSafeMap<KEY, DATA>::lockedKey (Int i)C {return *(KEY *)_MapTS::key       (i);}
T2(KEY, DATA)    DATA&  ThreadSafeMap<KEY, DATA>::lockedData(Int i)  {return *(DATA*)_MapTS::operator[](i);}
T2(KEY, DATA)  C DATA&  ThreadSafeMap<KEY, DATA>::lockedData(Int i)C {return *(DATA*)_MapTS::operator[](i);}

T2(KEY, DATA)  C KEY &  ThreadSafeMap<KEY, DATA>::lockedAbsKey (Int abs_i)C {return *(KEY *)_MapTS::absKey (abs_i);}
T2(KEY, DATA)    DATA&  ThreadSafeMap<KEY, DATA>::lockedAbsData(Int abs_i)  {return *(DATA*)_MapTS::absData(abs_i);}
T2(KEY, DATA)  C DATA&  ThreadSafeMap<KEY, DATA>::lockedAbsData(Int abs_i)C {return *(DATA*)_MapTS::absData(abs_i);}

T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::  lock()C {_MapTS::  lock();}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::unlock()C {_MapTS::unlock();}

T2(KEY, DATA)  MAP_MODE            Map<KEY, DATA>::mode(MAP_MODE mode) {return (MAP_MODE)_Map  ::mode(mode);}
T2(KEY, DATA)  MAP_MODE  ThreadSafeMap<KEY, DATA>::mode(MAP_MODE mode) {return (MAP_MODE)_MapTS::mode(mode);}

T2(KEY, DATA)  void  Map<KEY, DATA>::remove    (  Int   i               ) {       _Map::remove    ( i   );}
T2(KEY, DATA)  void  Map<KEY, DATA>::removeKey (C KEY  &key             ) {       _Map::removeKey (&key );}
T2(KEY, DATA)  void  Map<KEY, DATA>::removeData(C DATA *data            ) {       _Map::removeData( data);}
T2(KEY, DATA)  Bool  Map<KEY, DATA>::replaceKey(C KEY  &src, C KEY &dest) {return _Map::replaceKey(&src, &dest);}

T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::remove    (  Int   i               ) {       _MapTS::remove    ( i   );}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::removeKey (C KEY  &key             ) {       _MapTS::removeKey (&key );}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::removeData(C DATA *data            ) {       _MapTS::removeData( data);}
T2(KEY, DATA)  Bool  ThreadSafeMap<KEY, DATA>::replaceKey(C KEY  &src, C KEY &dest) {return _MapTS::replaceKey(&src, &dest);}

T2(KEY, DATA) T1(EXTENDED)            Map<KEY, DATA>&            Map<KEY, DATA>::replaceClass() {ASSERT_BASE_EXTENDED<DATA, EXTENDED>();         del(); _key_offset=UIntPtr(&((typename Map<KEY, EXTENDED>::Elm*)null)->key); /*_data_offset=UIntPtr(&((typename Map<KEY, EXTENDED>::Elm*)null)->data);*/ _desc_offset=UIntPtr(&((typename Map<KEY, EXTENDED>::Elm*)null)->desc); _data_size=SIZE(EXTENDED); _memx.replaceClass<typename Map<KEY, EXTENDED>::Elm>();           return T;}
T2(KEY, DATA) T1(EXTENDED)  ThreadSafeMap<KEY, DATA>&  ThreadSafeMap<KEY, DATA>::replaceClass() {ASSERT_BASE_EXTENDED<DATA, EXTENDED>(); lock(); del(); _key_offset=UIntPtr(&((typename Map<KEY, EXTENDED>::Elm*)null)->key); /*_data_offset=UIntPtr(&((typename Map<KEY, EXTENDED>::Elm*)null)->data);*/ _desc_offset=UIntPtr(&((typename Map<KEY, EXTENDED>::Elm*)null)->desc); _data_size=SIZE(EXTENDED); _memx.replaceClass<typename Map<KEY, EXTENDED>::Elm>(); unlock(); return T;}

T2(KEY, DATA)            Map<KEY, DATA>&            Map<KEY, DATA>::operator=(C           Map<KEY, DATA> &src) {if(this!=&src){                    from(src); FREPA(T)         T[i]=src           [i];                        } return T;}
T2(KEY, DATA)  ThreadSafeMap<KEY, DATA>&  ThreadSafeMap<KEY, DATA>::operator=(C ThreadSafeMap<KEY, DATA> &src) {if(this!=&src){lock(); src.lock(); from(src); FREPA(T)lockedData(i)=src.lockedData(i); src.unlock(); unlock();} return T;}

T2(KEY, DATA)            Map<KEY, DATA>::          Map(Int compare(C KEY &a, C KEY &b), Bool create(DATA &data, C KEY &key, Ptr user), Ptr user, Int block_elms) : _Map  (block_elms, (Int(*)(CPtr, CPtr))compare, (Bool(*)(Ptr, CPtr, Ptr))create, user, ClassFunc<KEY>::Copy) {replaceClass<DATA>();}
T2(KEY, DATA)  ThreadSafeMap<KEY, DATA>::ThreadSafeMap(Int compare(C KEY &a, C KEY &b), Bool create(DATA &data, C KEY &key, Ptr user), Ptr user, Int block_elms) : _MapTS(block_elms, (Int(*)(CPtr, CPtr))compare, (Bool(*)(Ptr, CPtr, Ptr))create, user, ClassFunc<KEY>::Copy) {replaceClass<DATA>();}

inline Int Elms(C _Map &map) {return map.elms();}
/******************************************************************************/
// GRID
/******************************************************************************/
T1(TYPE)  void         Grid<TYPE>:: del(                )  {                    _Grid::del (            );} // delete all      cells
T1(TYPE)  void         Grid<TYPE>:: del(Cell<TYPE> *cell)  {                    _Grid::del ((_Cell*)cell);} // delete selected cell
T1(TYPE)  Cell<TYPE>&  Grid<TYPE>:: get(C VecI2 &xy     )  {return (Cell<TYPE>&)_Grid::get (xy          );} // get 'xy' cell, create it   if not found
T1(TYPE)  Cell<TYPE>*  Grid<TYPE>::find(C VecI2 &xy     )C {return (Cell<TYPE>*)_Grid::find(xy          );} // get 'xy' cell, return null if not found
T1(TYPE)  Bool         Grid<TYPE>::size(  RectI &rect   )C {return              _Grid::size(rect        );} // get rectangle covering all grid cells, false on fail (if no grid cells are present)

T1(TYPE)               Grid<TYPE>&  Grid<TYPE>::fastAccess  (C RectI *rect) {_Grid::fastAccess                  (rect); return T;} // optimize accessing cells (via 'find/get' methods) within 'rect' rectangle, normally cells are accessed recursively, however after calling this method all cells within the rectangle will be available instantly, if null is provided then the optimization is disabled
T1(TYPE) T1(EXTENDED)  Grid<TYPE>&  Grid<TYPE>::replaceClass(             ) {_Grid::replaceClass<TYPE, EXTENDED>(    ); return T;} // replace the type of class stored in the grid, all grid cells are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

// call custom function on grid cells
T1(TYPE)                void  Grid<TYPE>::func      (               void func(Cell<TYPE> &cell, Ptr        user)                 ) {_Grid::func      (      (void (*)(_Cell &cell, Ptr user))func,  null);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (               void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user) {_Grid::func      (      (void (*)(_Cell &cell, Ptr user))func,  user);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (               void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user) {_Grid::func      (      (void (*)(_Cell &cell, Ptr user))func, &user);} // call 'func' on all existing grid cells
T1(TYPE)                void  Grid<TYPE>::func      (C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user)                 ) {_Grid::func      (rect, (void (*)(_Cell &cell, Ptr user))func,  null);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user) {_Grid::func      (rect, (void (*)(_Cell &cell, Ptr user))func,  user);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user) {_Grid::func      (rect, (void (*)(_Cell &cell, Ptr user))func, &user);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE)                void  Grid<TYPE>::funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user)                 ) {_Grid::funcCreate(rect, (void (*)(_Cell &cell, Ptr user))func,  null);} // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user) {_Grid::funcCreate(rect, (void (*)(_Cell &cell, Ptr user))func,  user);} // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user) {_Grid::funcCreate(rect, (void (*)(_Cell &cell, Ptr user))func, &user);} // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)

// call custom function on grid cells (multi-threaded version)
T1(TYPE)                void  Grid<TYPE>::mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, Ptr        user, Int thread_index)                 ) {_Grid::mtFunc(threads,       (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  null);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, USER_DATA *user, Int thread_index), USER_DATA *user) {_Grid::mtFunc(threads,       (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  user);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, USER_DATA &user, Int thread_index), USER_DATA &user) {_Grid::mtFunc(threads,       (void (*)(_Cell &cell, Ptr user, Int thread_index))func, &user);} // call 'func' on all existing grid cells
T1(TYPE)                void  Grid<TYPE>::mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user, Int thread_index)                 ) {_Grid::mtFunc(threads, rect, (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  null);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user, Int thread_index), USER_DATA *user) {_Grid::mtFunc(threads, rect, (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  user);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user, Int thread_index), USER_DATA &user) {_Grid::mtFunc(threads, rect, (void (*)(_Cell &cell, Ptr user, Int thread_index))func, &user);} // call 'func' on all existing grid cells in specified rectangle
/******************************************************************************/
// GAME::OBJ_MEMX
/******************************************************************************/
namespace Game
{
   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::clear() {_map.clear(); return T;}

   T1(TYPE)  Int  ObjMap<TYPE>::elms   ()C {return _map.elms    ();}
   T1(TYPE)  Int  ObjMap<TYPE>::elmSize()C {return _map.dataSize();}

   T1(TYPE)    TYPE&  ObjMap<TYPE>::operator[](Int i)  {return _map[i];}
   T1(TYPE)  C TYPE&  ObjMap<TYPE>::operator[](Int i)C {return _map[i];}

   T1(TYPE)  Bool  ObjMap<TYPE>::containsId (C UID  &obj_id)C {return obj_id.valid() ? _map.containsKey (obj_id) : false;}
   T1(TYPE)  Bool  ObjMap<TYPE>::containsObj(C TYPE *obj   )C {return                  _map.containsData(obj   )        ;}

   T1(TYPE)  TYPE*  ObjMap<TYPE>::find(C UID &obj_id) {return obj_id.valid() ? _map.find(obj_id) : null;}

   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::remove   (  Int   i     ) {                  _map.remove    (i     ); return T;}
   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::removeId (C UID  &obj_id) {if(obj_id.valid())_map.removeKey (obj_id); return T;}
   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::removeObj(C TYPE *data  ) {                  _map.removeData(data  ); return T;}

   T1(TYPE) T1(BASE)  ObjMap<TYPE>::operator    ObjMap<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(  ObjMap<BASE>*)this;}
   T1(TYPE) T1(BASE)  ObjMap<TYPE>::operator  C ObjMap<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(C ObjMap<BASE>*)this;}

   T1(TYPE)  ObjMap<TYPE>::ObjMap(Int block_elms) : _map(Compare, null, null, block_elms) {}
}
T1(TYPE)  Int  Elms(C Game::ObjMap<TYPE> &obj_map) {return obj_map.elms();}
/******************************************************************************/
// LIST
/******************************************************************************/
T1(TYPE)  _List&  _List::setData    (TYPE       *data, Int elms, C MemPtr<Bool> &visible, Bool keep_cur) {return _setData(     data  ,      elms  ,     SIZE(TYPE), visible, keep_cur);}
T1(TYPE)  _List&  _List::setData    (Mems<TYPE> &mems,           C MemPtr<Bool> &visible, Bool keep_cur) {return _setData(mems.data(), mems.elms(), mems.elmSize(), visible, keep_cur);}
T1(TYPE)  _List&  _List::setDataNode(Memx<TYPE> &memx,           C MemPtr<Bool> &visible, Bool keep_cur) {return _setData(memx       ,      OFFSET(TYPE, children), visible, keep_cur); Memx<TYPE> &temp=MEMBER(TYPE, children);} // temp assignment verifies that 'children' member can be casted to 'Memx<TYPE>' which is a requirement
/******************************************************************************/
// INTERPOLATOR
/******************************************************************************/
T1(TYPE)  AngularInterpolator<TYPE>::AngularInterpolator() {_value=_prev=_cur=_next=0;}
T1(TYPE)   LinearInterpolator<TYPE>:: LinearInterpolator() {_value=_prev=_cur=_next=0;}
T1(TYPE)   SplineInterpolator<TYPE>:: SplineInterpolator() {_value=_prev2=_prev=_cur=_next=0;}

T1(TYPE)  void AngularInterpolator<TYPE>::add(C TYPE &value, C InterpolatorTemp &temp)
{
   switch(temp.op)
   {
      case 0: _prev=_value=value; break; // initialize '_value' already so we can access it ASAP
      case 1:               _cur =_prev+AngleDelta(_prev, value); break;
      case 2: _prev=_value; _cur =_prev+AngleDelta(_prev, value); break; // start interpolating from current value
      case 3:               _next=_cur +AngleDelta(_cur , value); break;
   }
}
T1(TYPE)  void LinearInterpolator<TYPE>::add(C TYPE &value, C InterpolatorTemp &temp)
{
   switch(temp.op)
   {
      case 0: _prev=_value=value; break; // initialize '_value' already so we can access it ASAP
      case 1:               _cur =value; break;
      case 2: _prev=_value; _cur =value; break; // start interpolating from current value
      case 3:               _next=value; break;
   }
}
T1(TYPE)  void SplineInterpolator<TYPE>::add(C TYPE &value, C InterpolatorTemp &temp)
{
   switch(temp.op)
   {
      case 0: _prev=_prev2=_value=value; break; // initialize '_value' already so we can access it ASAP
      case 1:               _cur =value; break;
      case 2: _prev=_value; _cur =value; break; // start interpolating from current value
      case 3:               _next=value; break;
   }
}

T1(TYPE)  void AngularInterpolator<TYPE>::update(C InterpolatorTemp &temp) {if(temp.op){              _prev=_cur; _cur=_next;} _value=Lerp (        _prev, _cur,                  temp.frac);}
T1(TYPE)  void  LinearInterpolator<TYPE>::update(C InterpolatorTemp &temp) {if(temp.op){              _prev=_cur; _cur=_next;} _value=Lerp (        _prev, _cur,                  temp.frac);}
T1(TYPE)  void  SplineInterpolator<TYPE>::update(C InterpolatorTemp &temp) {if(temp.op){_prev2=_prev; _prev=_cur; _cur=_next;} _value=Lerp4(_prev2, _prev, _cur, _cur-_prev+_cur, temp.frac);} // predict next instead of using '_next' because we may not know it in all cases
/******************************************************************************/
// IO
/******************************************************************************/
T1(TYPE)  void  FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, TYPE *user), TYPE *user) {FList(path, (FILE_LIST_MODE (*)(C FileFind &ff, Ptr user))func,  user);}
T1(TYPE)  void  FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, TYPE &user), TYPE &user) {FList(path, (FILE_LIST_MODE (*)(C FileFind &ff, Ptr user))func, &user);}
/******************************************************************************/
// GUI
/******************************************************************************/
T1(TYPE)  ListColumn::ListColumn(TYPE &member                     , Flt width, C Str &name) : md(member) {create(null                           , width, name);}
T1(TYPE)  ListColumn::ListColumn(Str (*data_to_text)(C TYPE &data), Flt width, C Str &name)              {create((Str(*)(CPtr data))data_to_text, width, name);}
/******************************************************************************/
// GRAPHICS
/******************************************************************************/
#if EE_PRIVATE
INLINE Int Display::maxShaderMatrixes()C
{
#if DX9
   return MAX_MATRIX_DX9;
#elif DX11
   return MAX_MATRIX_DX10;
#elif GL
   #if VARIABLE_MAX_MATRIX
      return MeshBoneSplit ? MAX_MATRIX_DX9 : MAX_MATRIX_DX10;
   #else
      return MAX_MATRIX_DX9;
   #endif
#endif
}
INLINE Bool Display::meshStorageSigned()C
{
#if DX9
   return false;
#elif DX11
   return true;
#elif GL
   return true;
#endif
}
INLINE Bool Display::meshBoneSplit()C
{
#if DX9
   return true;
#elif DX11
   return false;
#elif GL
   #if VARIABLE_MAX_MATRIX
      return MeshBoneSplit;
   #else
      return true;
   #endif
#endif
}
INLINE Bool Display::hwInstancing()C
{
#if DX9
   return false;
#elif DX11
   return true;
#elif GL
   return notShaderModelGLES2(); // GL 3.1 or GLES 3+
#endif
}
INLINE Bool Display::signedNrmRT()C // #SIGNED_NRM_RT
{
#if DX9
   return false;
#elif DX11
   return true;
#elif GL
   return false;
#endif
}
INLINE Bool Display::signedVelRT()C // #SIGNED_VEL_RT
{
#if DX9
   return false;
#elif DX11
   return true;
#elif GL
   return false;
#endif
}
#endif
/******************************************************************************/
// SOUND
/******************************************************************************/
inline Int SndOpusEncoder::frequency   ()C {return _frequency;}
inline Int SndOpusEncoder::bytes       ()C {return _encoder.bytes();}
inline Int SndOpusEncoder::bits        ()C {return _encoder.bits();}
inline Int SndOpusEncoder::channels    ()C {return _encoder.channels();}
inline Int SndOpusEncoder::block       ()C {return _encoder.block();}
inline Int SndOpusEncoder::frameSamples()C {return _encoder.frameSamples();}
/******************************************************************************/
// EDIT
/******************************************************************************/
namespace Edit
{
   inline Int _Undo::changes()C {return _changes.elms();}

   T1(TYPE)  Undo<TYPE>::Undo(Bool full, Ptr user, Flt time) : _Undo(full, user, time) {replaceClass<TYPE>();}

   T1(TYPE)  TYPE*  Undo<TYPE>::getNextUndo() {return (TYPE*)_Undo::getNextUndo();}
   T1(TYPE)  TYPE*  Undo<TYPE>::getNextRedo() {return (TYPE*)_Undo::getNextRedo();}

   T1(TYPE)  TYPE&  Undo<TYPE>::operator[](Int i) {return (TYPE&)_Undo::operator[](i);}
   T1(TYPE)  TYPE*  Undo<TYPE>::addr      (Int i) {return (TYPE*)_Undo::addr      (i);}

   T1(TYPE)  TYPE*  Undo<TYPE>::set(CPtr change_type, Bool force_create, Flt extra_time) {return (TYPE*)_Undo::set(change_type, force_create, extra_time);}
   T1(TYPE)  TYPE*  Undo<TYPE>::set( Int change_type, Bool force_create, Flt extra_time) {return (TYPE*)_Undo::set(change_type, force_create, extra_time);}

   T1(TYPE) T1(CHANGE)  Undo<TYPE>&  Undo<TYPE>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, CHANGE>(); _changes.replaceClass<CHANGE>(); return T;}
}
/******************************************************************************/
#if EE_PRIVATE

#if (defined _M_IX86 || defined __i386__) || (defined _M_X64 || defined __x86_64__) || (ARM && X64) || WEB // x86 32/64 and ARM 64 can do unaligned reads. When using WebAssembly (WASM) for WEB platform, unaligned access is supported, however when executed on platforms without native unaligned access support (Arm32) it will be extremely slow, however since Arm32 is in extinction then it's better to enable unaligned access to get better performance on majority of platforms that support it.
   T1(TYPE) C TYPE& Unaligned(              C TYPE &src) {return src;}
   T1(TYPE)   void  Unaligned(TYPE   &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(Byte   &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(UShort &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(Int    &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(UInt   &dest, C TYPE &src) {  dest=src;}
#else
   T1(TYPE)   TYPE  Unaligned(              C TYPE &src) {if(SIZE(TYPE)==1)return src;else{TYPE temp; CopyFast(Ptr(&temp), CPtr(&src), SIZE(TYPE)); return temp;}} // !! these functions must casted to 'Ptr', because without it, compiler may try to inline the 'memcpy' when it detects that both params are of the same type and in that case it will assume that they are memory aligned and crash will occur !!
   T1(TYPE)   void  Unaligned(TYPE   &dest, C TYPE &src) {if(SIZE(TYPE)==1)  dest=src;else{           CopyFast(Ptr(&dest), CPtr(&src), SIZE(TYPE));             }} // !! these functions must casted to 'Ptr', because without it, compiler may try to inline the 'memcpy' when it detects that both params are of the same type and in that case it will assume that they are memory aligned and crash will occur !!
   T1(TYPE)   void _Unaligned(Byte   &dest, C TYPE &src) {                   dest=Unaligned(src) ;                                                               }
   T1(TYPE)   void _Unaligned(UShort &dest, C TYPE &src) {Unaligned(dest, (UShort)Unaligned(src));                                                               }
   T1(TYPE)   void _Unaligned(Int    &dest, C TYPE &src) {Unaligned(dest, (Int   )Unaligned(src));                                                               }
   T1(TYPE)   void _Unaligned(UInt   &dest, C TYPE &src) {Unaligned(dest, (UInt  )Unaligned(src));                                                               }
#endif

T2(TA, TB)  File&  File::putMulti(C TA &a, C TB &b)
{
 C Int size=SIZE(a)+SIZE(b);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   T<<buf;
   return T;
}
T2(TA, TB)  File&  File::getMulti(TA &a, TB &b)
{
 C Int size=SIZE(a)+SIZE(b);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   return T;
}
T3(TA, TB, TC)  File&  File::putMulti(C TA &a, C TB &b, C TC &c)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   T<<buf;
   return T;
}
T3(TA, TB, TC)  File&  File::getMulti(TA &a, TB &b, TC &c)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   return T;
}
T4(TA, TB, TC, TD)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   T<<buf;
   return T;
}
T4(TA, TB, TC, TD)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   return T;
}
T5(TA, TB, TC, TD, TE)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   T<<buf;
   return T;
}
T5(TA, TB, TC, TD, TE)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   return T;
}
T6(TA, TB, TC, TD, TE, TF)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   T<<buf;
   return T;
}
T6(TA, TB, TC, TD, TE, TF)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   return T;
}
T7(TA, TB, TC, TD, TE, TF, TG)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   T<<buf;
   return T;
}
T7(TA, TB, TC, TD, TE, TF, TG)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   return T;
}
T8(TA, TB, TC, TD, TE, TF, TG, TH)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   Unaligned((TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]), h);
   T<<buf;
   return T;
}
T8(TA, TB, TC, TD, TE, TF, TG, TH)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g, TH &h)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   Unaligned(h, (TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]));
   return T;
}
T9(TA, TB, TC, TD, TE, TF, TG, TH, TI)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h, C TI &i)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   Unaligned((TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]), h);
   Unaligned((TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]), i);
   T<<buf;
   return T;
}
T9(TA, TB, TC, TD, TE, TF, TG, TH, TI)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g, TH &h, TI &i)
{
 C Int size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   Unaligned(h, (TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]));
   Unaligned(i, (TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]));
   return T;
}
#endif
/******************************************************************************/
