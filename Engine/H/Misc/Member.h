/******************************************************************************

   Use 'MemberDesc' to describe a C++ structure/class member,
      including its type, size, and offset in the class.

/******************************************************************************/
enum DATA_TYPE : Byte // Data Type
{
   DATA_NONE     , // none
   DATA_CHAR     , // Char[]                16-bit character array forming a text
   DATA_CHAR_PTR , // Char*     pointer to  16-bit character array forming a text
   DATA_CHAR8    , // Char8[]                8-bit character array forming a text
   DATA_CHAR8_PTR, // Char8*    pointer to   8-bit character array forming a text
   DATA_STR      , // Str       String with 16-bit per character
   DATA_STR8     , // Str8      String with  8-bit per character
   DATA_BOOL     , // Bool      boolean
   DATA_INT      , // Int       integer
   DATA_UINT     , // UInt      unsigned integer
   DATA_REAL     , // Real      floating point
   DATA_PTR      , // Ptr       memory   address
   DATA_VEC2     , // Vec2      vector2D floating point
   DATA_VEC      , // Vec       vector3D floating point
   DATA_VEC4     , // Vec4      vector4D floating point
   DATA_VECI2    , // VecI2     vector2D integer
   DATA_VECI     , // VecI      vector3D integer
   DATA_VECI4    , // VecI4     vector4D integer
   DATA_VECU2    , // VecU2     vector2D unsigned integer
   DATA_VECU     , // VecU      vector3D unsigned integer
   DATA_VECU4    , // VecU4     vector4D unsigned integer
   DATA_COLOR    , // Color     color
   DATA_IMAGE    , // Image     texture
   DATA_IMAGE_PTR, // Image*    pointer to image object
   DATA_IMAGEPTR , // ImagePtr  pointer to image object in cache
   DATA_DATE_TIME, // DateTime  date and time
   DATA_UID      , // UID       unique ID
   DATA_KBSC     , // KbSc      keyboard shortcut
#if EE_PRIVATE
   DATA_CHECK    , // if checked
   DATA_MENU_PTR , // Menu*     pointer to menu
#endif
};
/******************************************************************************/
struct MemberDesc // Member Description - Description of a Member in a C++ class
{
   DATA_TYPE type  ; // data type
   UInt      offset; // memory offset of member in class
   Int       size  ; // member memory size in bytes

   // conversion functions
   T1(TYPE) MemberDesc& setDataToTextFunc(Str (*data_to_text)(C TYPE &object)                                                 ) {T._data_to_text=(Str (*)(CPtr object             ))data_to_text; return T;} // set data to text custom function
   T1(TYPE) MemberDesc& setTextToDataFunc(                                     void (*text_to_data)(TYPE &object, C Str &text)) {T._text_to_data=(void(*)( Ptr object, C Str &text))text_to_data; return T;} // set text to data custom function
   T1(TYPE) MemberDesc& setFunc          (Str (*data_to_text)(C TYPE &object), void (*text_to_data)(TYPE &object, C Str &text)) {return    setDataToTextFunc(data_to_text).setTextToDataFunc(text_to_data);} // set              custom functions

   T1(TYPE) MemberDesc& setCompareFunc(Int (*compare)(C TYPE &object0, C TYPE &object1)) {T._compare=(Int(*)(CPtr object0, CPtr object1))compare; return T;} // set custom compare function

   // get
   Int compare(CPtr object0, CPtr object1)C; // compare members of 'object0' and 'object1' C++ objects of class that this member belongs to

   Bool integer     ()C; // if this member doesn't use floating point values
   Bool dblPrecision()C; // if this member stores real values as 'Dbl'

   Bool  asBool (CPtr object                       )C; // get member value as Bool                  , 'object'=pointer to C++ object of class that this member belongs to, false             on fail
   Int   asInt  (CPtr object                       )C; // get member value as Int                   , 'object'=pointer to C++ object of class that this member belongs to, 0                 on fail
   Long  asLong (CPtr object                       )C; // get member value as Long                  , 'object'=pointer to C++ object of class that this member belongs to, 0                 on fail
   Flt   asFlt  (CPtr object                       )C; // get member value as Flt                   , 'object'=pointer to C++ object of class that this member belongs to, 0                 on fail
   Dbl   asDbl  (CPtr object                       )C; // get member value as Dbl                   , 'object'=pointer to C++ object of class that this member belongs to, 0                 on fail
   Vec2  asVec2 (CPtr object                       )C; // get member value as Vec2                  , 'object'=pointer to C++ object of class that this member belongs to, Vec2 (0, 0)       on fail
   VecD2 asVecD2(CPtr object                       )C; // get member value as VecD2                 , 'object'=pointer to C++ object of class that this member belongs to, VecD2(0, 0)       on fail
   Vec4  asColor(CPtr object                       )C; // get member value as Color (in Vec4 format), 'object'=pointer to C++ object of class that this member belongs to, Vec4 (0, 0, 0, 0) on fail
   Str   asText (CPtr object, Int precision=INT_MAX)C; // get member value as Text                  , 'object'=pointer to C++ object of class that this member belongs to, ""                on fail
   UID   asUID  (CPtr object                       )C; // get member value as UID                   , 'object'=pointer to C++ object of class that this member belongs to, UIDZero           on fail
   Int   asEnum (CPtr object                       )C; // get member value as Enum Int              , 'object'=pointer to C++ object of class that this member belongs to, -1                on fail
#if EE_PRIVATE
   Image* asImage(CPtr object                     )C; // get member value as Image pointer         , 'object'=pointer to C++ object of class that this member belongs to
#endif

   T1(TYPE) TYPE& as(Ptr object)C {return *(TYPE*)((Byte*)object+offset);} // get reference to member, 'object'=pointer to C++ object of class that this member belongs to !! specified TYPE should be the exact same type of the class member !!

   // operations
   void fromBool (Ptr object,   Bool  value); // set member value from Bool , 'object'=pointer to C++ object of class that this member belongs to
   void fromInt  (Ptr object,   Int   value); // set member value from Int  , 'object'=pointer to C++ object of class that this member belongs to
   void fromUInt (Ptr object,   UInt  value); // set member value from UInt , 'object'=pointer to C++ object of class that this member belongs to
   void fromLong (Ptr object,   Long  value); // set member value from Long , 'object'=pointer to C++ object of class that this member belongs to
   void fromULong(Ptr object,   ULong value); // set member value from ULong, 'object'=pointer to C++ object of class that this member belongs to
   void fromFlt  (Ptr object,   Flt   value); // set member value from Flt  , 'object'=pointer to C++ object of class that this member belongs to
   void fromDbl  (Ptr object,   Dbl   value); // set member value from Dbl  , 'object'=pointer to C++ object of class that this member belongs to
   void fromVec2 (Ptr object, C Vec2 &value); // set member value from Vec2 , 'object'=pointer to C++ object of class that this member belongs to
   void fromColor(Ptr object, C Vec4 &color); // set member value from Color, 'object'=pointer to C++ object of class that this member belongs to
   void fromText (Ptr object, C Str  &text ); // set member value from Text , 'object'=pointer to C++ object of class that this member belongs to
   void fromUID  (Ptr object, C UID  &uid  ); // set member value from UID  , 'object'=pointer to C++ object of class that this member belongs to

            MemberDesc(                                         ) {set (DATA_NONE, 0, 0   );} // create as empty
            MemberDesc(DATA_TYPE type, UInt offset=0, Int size=0) {set (type, offset, size);} // create manually
   T1(TYPE) MemberDesc(TYPE     &member                         ) {set (member            );} // create using auto-detection, 'member'=custom structure/class member - 'MEMBER' macro usage is required
            MemberDesc(  MemberDesc &md                         ) {Copy(T, md             );} // create as copy of 'md'
            MemberDesc(C MemberDesc &md                         ) {Copy(T, md             );} // create as copy of 'md'

   // manual setting
   MemberDesc& setType(DATA_TYPE type) {T.type=type; return T;}

   MemberDesc& set(DATA_TYPE type, UInt offset, Int size);

   T1(TYPE)             MemberDesc& set(TYPE     &member       ) {return setDefault(DATA_NONE     , member);} // 'member'=custom structure/class member - 'MEMBER' macro usage is required
   template<Int elms>   MemberDesc& set(Char    (&member)[elms]) {return setDefault(DATA_CHAR     , member);}
                        MemberDesc& set(Char*    &member       ) {return setDefault(DATA_CHAR_PTR , member);}
                        MemberDesc& set(CChar*   &member       ) {return setDefault(DATA_CHAR_PTR , member);}
   template<Int elms>   MemberDesc& set(Char8   (&member)[elms]) {return setDefault(DATA_CHAR8    , member);}
                        MemberDesc& set(Char8*   &member       ) {return setDefault(DATA_CHAR8_PTR, member);}
                        MemberDesc& set(CChar8*  &member       ) {return setDefault(DATA_CHAR8_PTR, member);}
                        MemberDesc& set(Str      &member       ) {return setDefault(DATA_STR      , member);}
                        MemberDesc& set(Str8     &member       ) {return setDefault(DATA_STR8     , member);}
                        MemberDesc& set(Bool     &member       ) {return setDefault(DATA_BOOL     , member);}
                        MemberDesc& set(SByte    &member       ) {return setDefault(DATA_INT      , member);}
                        MemberDesc& set(Short    &member       ) {return setDefault(DATA_INT      , member);}
                        MemberDesc& set(Int      &member       ) {return setDefault(DATA_INT      , member);}
                        MemberDesc& set(Long     &member       ) {return setDefault(DATA_INT      , member);}
                        MemberDesc& set(Byte     &member       ) {return setDefault(DATA_UINT     , member);}
                        MemberDesc& set(UShort   &member       ) {return setDefault(DATA_UINT     , member);}
                        MemberDesc& set(UInt     &member       ) {return setDefault(DATA_UINT     , member);}
                        MemberDesc& set(ULong    &member       ) {return setDefault(DATA_UINT     , member);}
                        MemberDesc& set(Half     &member       ) {return setDefault(DATA_REAL     , member);}
                        MemberDesc& set(Flt      &member       ) {return setDefault(DATA_REAL     , member);}
                        MemberDesc& set(Dbl      &member       ) {return setDefault(DATA_REAL     , member);}
                        MemberDesc& set(Ptr      &member       ) {return setDefault(DATA_PTR      , member);}
                        MemberDesc& set(VecH2    &member       ) {return setDefault(DATA_VEC2     , member);}
                        MemberDesc& set(Vec2     &member       ) {return setDefault(DATA_VEC2     , member);}
                        MemberDesc& set(VecD2    &member       ) {return setDefault(DATA_VEC2     , member);}
                        MemberDesc& set(VecH     &member       ) {return setDefault(DATA_VEC      , member);}
                        MemberDesc& set(Vec      &member       ) {return setDefault(DATA_VEC      , member);}
                        MemberDesc& set(VecD     &member       ) {return setDefault(DATA_VEC      , member);}
                        MemberDesc& set(VecH4    &member       ) {return setDefault(DATA_VEC4     , member);}
                        MemberDesc& set(Vec4     &member       ) {return setDefault(DATA_VEC4     , member);}
                        MemberDesc& set(VecD4    &member       ) {return setDefault(DATA_VEC4     , member);}
                        MemberDesc& set(VecSB2   &member       ) {return setDefault(DATA_VECI2    , member);}
                        MemberDesc& set(VecI2    &member       ) {return setDefault(DATA_VECI2    , member);}
                        MemberDesc& set(VecSB    &member       ) {return setDefault(DATA_VECI     , member);}
                        MemberDesc& set(VecI     &member       ) {return setDefault(DATA_VECI     , member);}
                        MemberDesc& set(VecSB4   &member       ) {return setDefault(DATA_VECI4    , member);}
                        MemberDesc& set(VecI4    &member       ) {return setDefault(DATA_VECI4    , member);}
                        MemberDesc& set(VecB2    &member       ) {return setDefault(DATA_VECU2    , member);}
                        MemberDesc& set(VecUS2   &member       ) {return setDefault(DATA_VECU2    , member);}
                        MemberDesc& set(VecB     &member       ) {return setDefault(DATA_VECU     , member);}
                        MemberDesc& set(VecUS    &member       ) {return setDefault(DATA_VECU     , member);}
                        MemberDesc& set(VecB4    &member       ) {return setDefault(DATA_VECU4    , member);}
                        MemberDesc& set(Color    &member       ) {return setDefault(DATA_COLOR    , member);}
                        MemberDesc& set(Image    &member       ) {return setDefault(DATA_IMAGE    , member);}
                        MemberDesc& set(Image*   &member       ) {return setDefault(DATA_IMAGE_PTR, member);}
                        MemberDesc& set(ImagePtr &member       ) {return setDefault(DATA_IMAGEPTR , member);}
                        MemberDesc& set(DateTime &member       ) {return setDefault(DATA_DATE_TIME, member);}
                        MemberDesc& set(UID      &member       ) {return setDefault(DATA_UID      , member);}
                        MemberDesc& set(KbSc     &member       ) {return setDefault(DATA_KBSC     , member);}
#if EE_PRIVATE
                        MemberDesc& set(Menu*    &member       ) {return setDefault(DATA_MENU_PTR , member);}
#endif

#if !EE_PRIVATE
private:
#endif
   Str  (*_data_to_text)(CPtr object                 );
   void (*_text_to_data)( Ptr object , C Str &text   );
   Int  (*_compare     )(CPtr object0,  CPtr  object1);

   T1(TYPE) MemberDesc& setDefault(DATA_TYPE type, TYPE &member) {return set(type, UInt(UIntPtr(&member)), SIZE(member));}
};
/******************************************************************************/
#if EE_PRIVATE
  Bool      DataIsImage (DATA_TYPE type); // if 'type' is of image data type
  Bool      DataIsText  (DATA_TYPE type); // if 'type' is of text  data type
  Flt       DataAlign   (DATA_TYPE type); // get default text align for data type
C ImagePtr& DataGuiImage(CPtr data, C ListColumn &list_col, Color &color); // convert data to gui image
#endif
/******************************************************************************/
