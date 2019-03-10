/******************************************************************************

   Use 'Param' to store a custom parameter consisting of name and value.

/******************************************************************************/
enum PARAM_TYPE : Byte // Parameter Type
{
   PARAM_BOOL    , // boolean
   PARAM_INT     , // int
   PARAM_FLT     , // float
   PARAM_STR     , // string
   PARAM_ENUM    , // enum
   PARAM_VEC2    , // vector 2D
   PARAM_VEC     , // vector 3D
   PARAM_VEC4    , // vector 4D
   PARAM_COLOR   , // color
   PARAM_ID      , // ID
   PARAM_ID_ARRAY, // ID array
   PARAM_NUM     , // number of valid parameter types
};
/******************************************************************************/
struct Param // Parameter
{
   PARAM_TYPE type     ; // parameter type
   Enum      *enum_type; // pointer to 'Enum' object from 'Enums' cache, this is valid when "type==PARAM_ENUM"
   Str        name     ; // parameter name
   struct Value
   {
      union
      {
         Bool  b ; // bool
         Int   i ; // int
         Flt   f ; // float
         Vec2  v2; // vector 2D
         Vec   v ; // vector 3D
         Vec4  v4; // vector 4D
         Color c ; // color
         UID   id; // ID
      };
      Str s; // string

      Value() {} // needed because of union
   }value; // parameter value

   // get
            Str   asText (Int precision=-6  )C; // ""        on fail
            Bool  asBool (                  )C; // false     on fail
            Int   asInt  (                  )C; // 0         on fail
            Flt   asFlt  (                  )C; // 0         on fail
            Vec2  asVec2 (                  )C; // (0,0)     on fail
            Vec   asVec  (                  )C; // (0,0,0)   on fail
            Vec4  asVec4 (                  )C; // (0,0,0,0) on fail
            Color asColor(                  )C; // (0,0,0,0) on fail
            UID   asID   (                  )C; // UIDZero   on fail
            UID   asID   (Int id_index      )C; // UIDZero   on fail, use this method for PARAM_ID_ARRAY type, 'id_index'=index of the ID in the array
            UID   asIDMod(Int id_index      )C; // UIDZero   on fail, use this method for PARAM_ID_ARRAY type, 'id_index'=index of the ID in the array which is converted modulo to always fit in the array range "id_index=Mod(id_index, IDs())"
            Int   asEnum (                  )C; // -1        on fail
   T1(TYPE) TYPE  asEnum (TYPE invalid_value)C {Int e=asEnum(); return (e<0) ? invalid_value : TYPE(e);} // use this method for custom enum types, 'invalid_value'=enum value to use when desired value was not found

   Int  IDs     ()C; // get number of ID's stored in this parameter, use this method for PARAM_ID_ARRAY type, for PARAM_ID type this will always return 1, and for other types this will always return 0
   UInt memUsage()C; // get memory usage

   Bool hasID(C UID &id)C; // check if 'id' is in the list of parameter ID's

   // set
   Param& clearValue(            ); // clear value of this parameter          without changing the parameter type
   Param&   setValue(  Int    i  ); // set   value of this parameter to 'i'   without changing the parameter type
   Param&   setValue(C Str   &s  ); // set   value of this parameter to 's'   without changing the parameter type
   Param&   setValue(C Param &src); // set   value of this parameter to 'src' without changing the parameter type

   Param& setType(PARAM_TYPE type, Enum *enum_type=null); // set type to 'type' and automatically convert the parameter value to match the new type

   Param& setTypeValue(C Param &src); // set type and value of this parameter to 'src'

   Param&     setAsIDArray(C MemPtr<UID> &ids, Bool allow_PARAM_ID_type=true); // set type to PARAM_ID_ARRAY and set value to 'ids', 'allow_PARAM_ID_type'=if number of id's is equal to 1 then use PARAM_ID type instead
   Param& includeAsIDArray(C MemPtr<UID> &ids, Bool allow_PARAM_ID_type=true); // set type to PARAM_ID_ARRAY and set value to 'ids', 'allow_PARAM_ID_type'=if number of id's is equal to 1 then use PARAM_ID type instead, this method preserves existing ID's and adds 'ids' which aren't present yet

   // io
   Bool save(File &f, CChar *path=null)C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
#if EE_PRIVATE
   Int  arrayIDs(       )C; // !! only for PARAM_ID_ARRAY !!
   void loadOld (File &f);
   void zero    (       );
#endif

   Param();
};
/******************************************************************************/
Int Compare     (C Param &p0, C Param &p1);
Int CompareValue(C Param &p0, C Param &p1);
/******************************************************************************/
