/******************************************************************************

   Properties are helper functions for manipulating class member values using Gui controls.

   In this tutorial we're going to present how to use Properties on a Sample Class and its Object.

/******************************************************************************/
enum SAMPLE_ENUM // Sample Enum
{
   SE_ENUM_0,
   SE_ENUM_1,
   SE_ENUM_2,
}
cchar8 *enum_names[]=
{
   "SE_ENUM_0",
   "SE_ENUM_1",
   "SE_ENUM_2",
};
class Data
{
   bool        bool_member=false;
   int          int_member=0;
   flt          flt_member=0;
   SAMPLE_ENUM enum_member=SE_ENUM_0;
   Color      color_member(255, 255, 255, 255);

   int  privateMember(     )C {return private_member  ;}
   void privateMember(int x)  {       private_member=x;}

private:
   int private_member=0;
};
/******************************************************************************/
void PrivateMember(  Data &data, C Str &text) {         data.privateMember(TextInt(text));}
Str  PrivateMember(C Data &data             ) {return S+data.privateMember(             );}
/******************************************************************************/
Memx<Property> props ; // container of properties (each property will be assigned to exactly one class member)
Window         window; // window containing the properties
Data           data  ; // object of the sample class
TextStyle      ts    ;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.x=0;
   App.y=0;
   D.mode(1024, 768);
}
/******************************************************************************/
bool Init()
{
   Gui+=window.create(Rect_C(0, 0, 0.8, 0.55), "Properties"); // create window

   // setup properties
   props.New().create(   "bool_member", MEMBER(Data,  bool_member));                                        // set property for    'bool_member'
   props.New().create(    "int_member", MEMBER(Data,   int_member));                                        // set property for     'int_member'
   props.New().create(    "flt_member", MEMBER(Data,   flt_member)).range   (0, 1);                         // set property for     'flt_member' and optionally clamp the values range
   props.New().create(   "enum_member", MEMBER(Data,  enum_member)).setEnum (enum_names, Elms(enum_names)); // set property for    'enum_member'
   props.New().create(  "color_member", MEMBER(Data, color_member)).setColor();                             // set property for   'color_member'
   props.New().create("private_member", MemberDesc(DATA_INT).setFunc(PrivateMember, PrivateMember));        // set property for 'private_member'

   // add properties to the window
   ts.reset(true); ts.align.set(-1, 0); // set text style
   AddProperties(props, window, Vec2(0.04, -0.04), 0.06, 0.3, &ts);

   // initialize GUI values from the Class Object
   REPAO(props).toGui(&data);

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Gui.update();

   // set Class Object values from the GUI
   REPAO(props).fromGui(&data);

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   
   // draw the data member values on the screen to visualize them in each frame
   D.text(0, 0.9, S+   "bool_member: "+data. bool_member);
   D.text(0, 0.8, S+    "int_member: "+data.  int_member);
   D.text(0, 0.7, S+    "flt_member: "+data.  flt_member);
   D.text(0, 0.6, S+   "enum_member: "+data. enum_member);
   D.text(0, 0.5, S+  "color_member: "+data.color_member.v4);
   D.text(0, 0.4, S+"private_member: "+data.privateMember());

   Gui.draw();
}
/******************************************************************************/
