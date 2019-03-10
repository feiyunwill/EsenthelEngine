/******************************************************************************

   Use 'Property' to handle manipulating class members using Gui controls.

/******************************************************************************/
enum PROP_MOUSE_EDIT_MODE : Byte // Property Mouse Edit Mode
{
   PROP_MOUSE_EDIT_LINEAR, // works by      adding/subtracting value
   PROP_MOUSE_EDIT_SCALAR, // works by multiplying/dividing    value
};
/******************************************************************************/
const_mem_addr struct Property // Class Member Gui Control !! must be stored in constant memory address !!
{
   Dbl                  min_value, max_value; // -Inf..Inf , default=0
   Bool                 min_use  , max_use  ; // true/false, default=false
   PROP_MOUSE_EDIT_MODE mouse_edit_mode     ; //             default=PROP_MOUSE_EDIT_LINEAR
   Flt                  mouse_edit_speed    ; //    0..Inf , default=1.0 (40 for Int's)
   Int                  real_precision      ; //             default=MAX_INT (auto-detect)

   MemberDesc md      ;
   Text       name    ;
   CheckBox   checkbox;
   TextLine   textline;
   Button     button  ;
   ComboBox   combobox;
   Slider     slider  ;

   // manage
   T1(TYPE) Property& create   (C Str  &name  ,   TYPE       &member) {return create(name, MemberDesc(member));}
            Property& create   (C Str  &name  , C MemberDesc &md=MemberDesc());
            Property& setColor (                                    ); // set as color  type
            Property& setFile  (C Str  &ext=S , C Str        &desc=S); // set as file   type, 'ext'=list of supported extensions separated with '|' symbol (null for all kinds of extensions), 'desc'=description of supported files types (if specified then it will be used in the window title bar instead of the 1st extension)
            Property& setEnum  (                                    ); // set as enum   type
            Property& setEnum  (CChar8 *data[],   Int         elms  ); // set as enum   type and automatically set enum values
            Property& setEnum  (CChar  *data[],   Int         elms  ); // set as enum   type and automatically set enum values
            Property& setSlider(                                    ); // set as slider type

   // get / set
   Property& desc(C Str &desc);   C Str& desc()C; // set/get property description

   Property& autoData(Ptr object);   Ptr autoData()C {return _auto_data;} // set/get automatic data setting

   Property& set(  Bool   value, SET_MODE mode=SET_DEFAULT);   Bool  asBool ()C;   Bool  asBool (CPtr object)C {return md.asBool (object                );} // set  /  get from gui  /  get from data, false       on fail
   Property& set(  Int    value, SET_MODE mode=SET_DEFAULT);   Int   asInt  ()C;   Int   asInt  (CPtr object)C {return md.asInt  (object                );} // set  /  get from gui  /  get from data, 0           on fail
   Property& set(  Long   value, SET_MODE mode=SET_DEFAULT);   Long  asLong ()C;   Long  asLong (CPtr object)C {return md.asLong (object                );} // set  /  get from gui  /  get from data, 0           on fail
   Property& set(  Flt    value, SET_MODE mode=SET_DEFAULT);   Flt   asFlt  ()C;   Flt   asFlt  (CPtr object)C {return md.asFlt  (object                );} // set  /  get from gui  /  get from data, 0           on fail
   Property& set(  Dbl    value, SET_MODE mode=SET_DEFAULT);   Dbl   asDbl  ()C;   Dbl   asDbl  (CPtr object)C {return md.asDbl  (object                );} // set  /  get from gui  /  get from data, 0           on fail
   Property& set(C Vec2  &value, SET_MODE mode=SET_DEFAULT);   Vec2  asVec2 ()C;   Vec2  asVec2 (CPtr object)C {return md.asVec2 (object                );} // set  /  get from gui  /  get from data, Vec2 (0, 0) on fail
   Property& set(C VecD2 &value, SET_MODE mode=SET_DEFAULT);   VecD2 asVecD2()C;   VecD2 asVecD2(CPtr object)C {return md.asVecD2(object                );} // set  /  get from gui  /  get from data, VecD2(0, 0) on fail
   Property& set(C Str   &value, SET_MODE mode=SET_DEFAULT);   Str   asText ()C;   Str   asText (CPtr object)C {return md.asText (object, real_precision);} // set  /  get from gui  /  get from data, ""          on fail
                                                                                   Int   asEnum (CPtr object)C {return md.asEnum (object                );} // set  /  get from gui  /  get from data, -1          on fail

   Property& min  (Dbl value       ) {min_use=        true; min_value=value;              return T;} // set minimum allowed value
   Property& max  (Dbl value       ) {max_use=        true; max_value=value;              return T;} // set maximum allowed value
   Property& range(Dbl min, Dbl max) {min_use=max_use=true; min_value=min; max_value=max; return T;} // set         allowed value range

   Property& mouseEditDel  (                          );                                    // disable ability of changing the value with a mouse
   Property& mouseEditMode (PROP_MOUSE_EDIT_MODE mode ) {mouse_edit_mode =mode ; return T;} //         default=PROP_MOUSE_EDIT_LINEAR
   Property& mouseEditSpeed(Flt                  speed) {mouse_edit_speed=speed; return T;} // 0..Inf, default=1.0 (40 for Int's)

   Property& precision(Int precision) {real_precision=precision; return T;} // set real value precision

   Property& changed(void (*changed)(C Property &prop), void (*pre_changed)(C Property &prop)=null); // set functions called after and before a property value is changed

   // get value clamped according to specified limits
   Bool  clamp(Bool  value);
   Int   clamp(Int   value);
   UInt  clamp(UInt  value);
   Long  clamp(Long  value);
   ULong clamp(ULong value);
   Flt   clamp(Flt   value);
   Dbl   clamp(Dbl   value);

   Bool contains(C GuiObj *child)C; // if property contains 'child' object

   // operations
   void fromGui( Ptr object); // set data from gui
   void   toGui(CPtr object); // set gui  from data
   void   toGui(           ); // set gui  from 'autoData'

   Property& hide         (       );                    // hide    property
   Property& show         (       );                    // show    property
   Property& visible      (Bool on);   Bool visible()C; // set/get visibility
   Property& visibleToggle(       );                    // toggle  visibility

   Property& pos (C Vec2 &pos  ); // set position
   Property& move(C Vec2 &delta); // move by delta

   Property& close(); // will hide any visible windows (including Color Picker)

   Property& parent(GuiObj &parent); // change parent from current to 'parent'

   Property& enabled(Bool enabled); // set if enabled

   Rect addTo(GuiObj &parent, C Vec2 &pos, Flt text_width, Flt height, Flt value_width); // instead of calling this method manually, it is recommended to call 'AddProperties'. This method adds self to 'parent' and sets the rectangles of its members. Returns rectangle of this property

  ~Property();
   Property();

#if !EE_PRIVATE
private:
#endif
   STRUCT(Color , GuiCustom)
   //{
      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);
   };

   GUI_OBJ_TYPE _value_type;
   VecD2        _mouse_edit_value;
   Ptr          _auto_data;
   Color        _color;
   Str          _win_io_ext, _win_io_ext_desc;
   ColorPicker *_cp;
   WindowIO    *_win_io;
   void       (*_changed)(C Property &prop), (*_pre_changed)(C Property &prop);

#if EE_PRIVATE
   static void  MouseEdit(Property &prop);
   static void SelectFile(Property &prop);

   Int actualPrecision()C; // get actual value precision
#endif

   NO_COPY_CONSTRUCTOR(Property);
};
/******************************************************************************/
Rect AddProperties(Memx<Property> &properties, GuiObj &parent, C Vec2 &left_up, Flt property_height, Flt value_width, C TextStylePtr &text_style=null); // add 'properties' properties to 'parent' gui object, 'left_up'=coordinates in parent space where to start adding elements, 'property_height'=height of a single property element, 'value_width'=width of gui elements specified for value presentation, returns rectangle containing the properties, 'ts'=text style used for describing properties (the object is not copied, only the pointer to the object is remembered, therefore it must point to a constant memory address !!)

void SaveProperties(C Memx<Property> &properties,   MemPtr<TextNode>  nodes, Char space_replacement=' ', Bool handle_same_names=false); // save properties to   text nodes, 'space_replacement'=replace spaces with this character, 'handle_same_names'=if handle case when some properties have same names
void LoadProperties(  Memx<Property> &properties, C MemPtr<TextNode> &nodes, Char space_replacement=' ', Bool handle_same_names=false); // load properties from text nodes, 'space_replacement'=replace spaces with this character, 'handle_same_names'=if handle case when some properties have same names

void SaveProperties(C Memx<Property> &properties,   XmlNode &node, Char space_replacement='\0'); // save properties to   xml node, since xml does not support parameters with spaces in their names, all spaces need to be replaced, you can use 'space_replacement' to specify custom character as the replacement
void LoadProperties(  Memx<Property> &properties, C XmlNode &node, Char space_replacement='\0'); // load properties from xml node, since xml does not support parameters with spaces in their names, all spaces need to be replaced, you can use 'space_replacement' to specify custom character as the replacement
/******************************************************************************/
