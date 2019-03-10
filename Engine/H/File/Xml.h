/******************************************************************************

   Use 'TextData' to handle automatic text data saving and loading.
   Use  'XmlData' to handle automatic xml  data saving and loading.

/******************************************************************************/
// TEXT
/******************************************************************************/
struct TextParam
{
   Str name , // parameter name
       value; // parameter value

   // set / get
   TextParam& setName(C Str &name=S) {T.name=name; return T;}

   TextParam& setValue(C Str   &value=S) {T.value=         value           ; return T;}   void getValue(Str   &value)C {value=asText ();}
   TextParam& setValue(C Str8  &value  ) {T.value=         value           ; return T;}   void getValue(Str8  &value)C {value=asText ();}
   TextParam& setValue(CChar   *value  ) {T.value=         value           ; return T;}
   TextParam& setValue(CChar8  *value  ) {T.value=         value           ; return T;}
   TextParam& setValue(  Int    value  ) {T.value=         value           ; return T;}   void getValue(Int   &value)C {value=asInt  ();}
   TextParam& setValue(  UInt   value  ) {T.value=         value           ; return T;}   void getValue(UInt  &value)C {value=asUInt ();}
   TextParam& setValue(  Long   value  ) {T.value=         value           ; return T;}   void getValue(Long  &value)C {value=asLong ();}   
   TextParam& setValue(  ULong  value  ) {T.value=         value           ; return T;}   void getValue(ULong &value)C {value=asULong();}
   TextParam& setValue(  Flt    value  ) {T.value=TextReal(value      , -6); return T;}   void getValue(Flt   &value)C {value=asFlt  ();}
   TextParam& setValue(  Dbl    value  ) {T.value=TextReal(value      , -9); return T;}   void getValue(Dbl   &value)C {value=asDbl  ();}
   TextParam& setValue(C Vec2  &value  ) {T.value=         value.asText(-6); return T;}   void getValue(Vec2  &value)C {value=asVec2 ();}
   TextParam& setValue(C VecD2 &value  ) {T.value=         value.asText(-9); return T;}   void getValue(VecD2 &value)C {value=asVecD2();}
   TextParam& setValue(C VecI2 &value  ) {T.value=         value           ; return T;}   void getValue(VecI2 &value)C {value=asVecI2();}
   TextParam& setValue(C Vec   &value  ) {T.value=         value.asText(-6); return T;}   void getValue(Vec   &value)C {value=asVec  ();}
   TextParam& setValue(C VecD  &value  ) {T.value=         value.asText(-9); return T;}   void getValue(VecD  &value)C {value=asVecD ();}
   TextParam& setValue(C VecI  &value  ) {T.value=         value           ; return T;}   void getValue(VecI  &value)C {value=asVecI ();}
   TextParam& setValue(C Vec4  &value  ) {T.value=         value.asText(-6); return T;}   void getValue(Vec4  &value)C {value=asVec4 ();}
   TextParam& setValue(C VecD4 &value  ) {T.value=         value.asText(-9); return T;}   void getValue(VecD4 &value)C {value=asVecD4();}
   TextParam& setValue(C VecI4 &value  ) {T.value=         value           ; return T;}   void getValue(VecI4 &value)C {value=asVecI4();}
                                                                                          Bool getValue(UID   &value)C {return value.fromText(asText());}

            TextParam& setValueHex(  Flt   value      );                                               // set value to 'value' in hex text format
            TextParam& setValueHex(  Dbl   value      );                                               // set value to 'value' in hex text format
            TextParam& setValueHex(C UID  &value      ) {T.value=value.asHex();             return T;} // set value to 'value' in hex text format
            TextParam& setValueFN (C UID  &value      ) {EncodeFileName(T.value, value   ); return T;} // set value to 'value'          encoded using 'EncodeFileName' function
   T1(TYPE) TextParam& setValueRaw(C TYPE &value      ) {return setValueRaw(&value, SIZE(value));    } // set value to 'value'          encoded using 'EncodeRaw'      function
            TextParam& setValueRaw(CPtr data, Int size) {EncodeRaw     (value, data, size); return T;} // set value to 'data' of 'size' encoded using 'EncodeRaw'      function

   T1(TYPE) Bool getValueRaw(TYPE &value       )C {return getValueRaw(&value, SIZE(value));} // get value and store it in 'value'          decoded using 'DecodeRaw' function, returns false on fail (if 'TextParam.value' length does not match  size  of 'value')
            Bool getValueRaw(Ptr data, Int size)C {return   DecodeRaw( value, data, size );} // get value and store it in 'data' of 'size' decoded using 'DecodeRaw' function, returns false on fail (if 'TextParam.value' length does not match 'size' of 'data' )

   TextParam& set(C Str &name, C Str   &value=S) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C Str8  &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, CChar   *value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, CChar8  *value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name,   Int    value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name,   UInt   value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name,   Long   value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name,   ULong  value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name,   Flt    value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name,   Dbl    value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C Vec2  &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C VecD2 &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C VecI2 &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C Vec   &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C VecD  &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C VecI  &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C Vec4  &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C VecD4 &value  ) {return setName(name).setValue(value);}
   TextParam& set(C Str &name, C VecI4 &value  ) {return setName(name).setValue(value);}

            TextParam& setHex(C Str &name,   Flt   value      ) {return setName(name).setValueHex(value     );}
            TextParam& setHex(C Str &name,   Dbl   value      ) {return setName(name).setValueHex(value     );}
            TextParam& setHex(C Str &name, C UID  &value      ) {return setName(name).setValueHex(value     );}
            TextParam& setFN (C Str &name, C UID  &value      ) {return setName(name).setValueFN (value     );}
   T1(TYPE) TextParam& setRaw(C Str &name, C TYPE &value      ) {return setName(name).setValueRaw(value     );}
            TextParam& setRaw(C Str &name, CPtr data, Int size) {return setName(name).setValueRaw(data, size);}

 C Str&  asText ()C {return           value ;}
   Bool  asBool ()C {return TextBool (value);}
   Bool  asBool1()C {return TextBool1(value);}
   Int   asInt  ()C {return TextInt  (value);}
   UInt  asUInt ()C {return TextUInt (value);}
   Long  asLong ()C {return TextLong (value);}
   ULong asULong()C {return TextULong(value);}
   Flt   asFlt  ()C {return TextFlt  (value);}
   Dbl   asDbl  ()C {return TextDbl  (value);}
   Vec2  asVec2 ()C {return TextVec2 (value);}
   VecD2 asVecD2()C {return TextVecD2(value);}
   VecI2 asVecI2()C {return TextVecI2(value);}
   Vec   asVec  ()C {return TextVec  (value);}
   VecD  asVecD ()C {return TextVecD (value);}
   VecI  asVecI ()C {return TextVecI (value);}
   Vec4  asVec4 ()C {return TextVec4 (value);}
   VecD4 asVecD4()C {return TextVecD4(value);}
   VecI4 asVecI4()C {return TextVecI4(value);}
   Color asColor()C {return TextColor(value);}
   UID   asUID  ()C {return TextUID  (value);}

   TextParam& clear() {name.clear(); value.clear(); return T;}
   TextParam& del  () {name.del  (); value.del  (); return T;}
   
            TextParam() {}
   explicit TextParam(C Str &name, C Str &value=S) {set(name, value);}
};
/******************************************************************************/
STRUCT(TextNode , TextParam)
//{
   Memc<TextNode> nodes;

   // get / set
   TextNode* findNode(C Str &name, Int i=0) ; // find i-th node which name is equal to 'name', null on fail (if not found)
 C TextNode* findNode(C Str &name, Int i=0)C; // find i-th node which name is equal to 'name', null on fail (if not found)
   TextNode&  getNode(C Str &name         ) ; // get       node which name is equal to 'name', New  on fail (if not found)

   TextNode& setName(C Str &name=S) {super::setName(name); return T;}

   // manage
   TextNode& clear() {super::clear(); nodes.clear(); return T;} // clear
   TextNode& del  () {super::del  (); nodes.del  (); return T;} // delete manually

#if EE_PRIVATE
   Bool save    (FileText &f, Bool just_values)C;
   Char load    (FileText &f, Bool just_values, Char first_char);
   Char loadJSON(FileText &f, Bool just_values, Char first_char);
   Char loadYAML(FileText &f, Bool just_values, Char first_char, const Int node_spaces, Int &cur_spaces);
#endif

   TextNode() {}
   TextNode(C XmlNode &xml); // create from 'XmlNode'
};
/******************************************************************************/
struct TextData
{
   Memc<TextNode> nodes;

   // get
   TextNode* findNode(C Str &name, Int i=0) ; // find i-th node which name is equal to 'name', null on fail (if not found)
 C TextNode* findNode(C Str &name, Int i=0)C; // find i-th node which name is equal to 'name', null on fail (if not found)
   TextNode&  getNode(C Str &name         ) ; // get       node which name is equal to 'name', New  on fail (if not found)

   // manage
   TextData& clear() {nodes.clear(); return T;} // clear
   TextData& del  () {nodes.del  (); return T;} // delete manually

   // io
   Bool save(C Str    &name, ENCODING encoding=UTF_8, INDENT indent=INDENT_TABS, const_mem_addr Cipher *cipher=null)C; // save to file, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool save(FileText &f                                                                                           )C; // save to file, false on fail, 'f' file must be already opened for writing

   Bool load(C Str    &name, const_mem_addr Cipher *cipher=null); // load from file, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool load(C UID    &id  , const_mem_addr Cipher *cipher=null); // load from file, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool load(FileText &f                                       ); // load from file, false on fail, 'f' file must be already opened for reading

   Bool loadJSON(C Str    &name, const_mem_addr Cipher *cipher=null); // load from file in JSON format, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool loadJSON(FileText &f                                       ); // load from file in JSON format, false on fail, 'f' file must be already opened for reading

   Bool loadYAML(C Str    &name, const_mem_addr Cipher *cipher=null); // load from file in YAML format, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
   Bool loadYAML(FileText &f                                       ); // load from file in YAML format, false on fail, 'f' file must be already opened for reading

   TextData() {}
   TextData(C XmlData &xml); // create from 'XmlData'
};
/******************************************************************************/
// XML
/******************************************************************************/
typedef TextParam XmlParam; // Xml Node Parameter
/******************************************************************************/
struct XmlNode // Xml Node
{
   Str            name  ; // node name
   Memc<XmlParam> params; // node parameters
   Memc<Str     > data  ; // node data
   Memc<XmlNode > nodes ; // node children

   XmlNode& setName(C Str &name) {T.name=name; return T;}

   // get
   XmlParam* findParam(C Str &name, Int i=0) ; // find i-th parameter which name is equal to 'name', null on fail (if not found)
 C XmlParam* findParam(C Str &name, Int i=0)C; // find i-th parameter which name is equal to 'name', null on fail (if not found)
   XmlParam&  getParam(C Str &name         ) ; // get       parameter which name is equal to 'name', New  on fail (if not found)
   XmlNode * findNode (C Str &name, Int i=0) ; // find i-th node      which name is equal to 'name', null on fail (if not found)
   XmlNode &  getNode (C Str &name         ) ; // get       node      which name is equal to 'name', New  on fail (if not found)

   // manage
   XmlNode& clear() {name.clear(); params.clear(); data.clear(); nodes.clear(); return T;} // clear
   XmlNode& del  () {name.del  (); params.del  (); data.del  (); nodes.del  (); return T;} // delete manually

#if EE_PRIVATE
   Bool save(FileText &f, Bool params_in_separate_lines)C;
   Bool load(FileText &f, Char first_char);
#endif

   XmlNode() {}
   XmlNode(C TextNode &text); // create from 'TextNode'
};
/******************************************************************************/
struct XmlData // Xml Data
{
   Memc<XmlNode> nodes;

   // get
   XmlNode* findNode(C Str &name, Int i=0); // find i-th node which name is equal to 'name', null on fail (if not found)
   XmlNode&  getNode(C Str &name         ); // get       node which name is equal to 'name', New  on fail (if not found)

   // manage
   XmlData& clear() {nodes.clear(); return T;} // clear
   XmlData& del  () {nodes.del  (); return T;} // delete manually

   // io
   Bool save(C Str    &name, Bool params_in_separate_lines=false, ENCODING encoding=UTF_8)C; // save to file, false on fail
   Bool save(FileText &f   , Bool params_in_separate_lines=false                         )C; // save to file, false on fail, 'f' file must be already opened for writing

   Bool load(C Str    &name); // load from file, false on fail
   Bool load(C UID    &id  ); // load from file, false on fail
   Bool load(FileText &f   ); // load from file, false on fail, 'f' file must be already opened for reading

   XmlData() {}
   XmlData(C TextData &text); // create from 'TextData'
};
/******************************************************************************/
  TextNode* FindNode (  MemPtr<TextNode>  nodes, C Str &name, Int i=0); // find i-th node which name is equal to 'name', null on fail (if not found)
C TextNode* FindNodeC(C MemPtr<TextNode> &nodes, C Str &name, Int i=0); // find i-th node which name is equal to 'name', null on fail (if not found)
  TextNode&  GetNode (  MemPtr<TextNode>  nodes, C Str &name         ); // get       node which name is equal to 'name', New  on fail (if not found)

XmlNode* FindNode(MemPtr<XmlNode> nodes, C Str &name, Int i=0); // find i-th node which name is equal to 'name', null on fail (if not found)
XmlNode&  GetNode(MemPtr<XmlNode> nodes, C Str &name         ); // get       node which name is equal to 'name', New  on fail (if not found)
/******************************************************************************/
