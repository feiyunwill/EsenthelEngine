/******************************************************************************/
/******************************************************************************/
class GuiView : Region
{
   static const flt   MinSize;
   static       Color TextRectColor;
   static       void  ViewportDraw(Viewport &viewport);
   static       bool  IgnoreSelected;

   class Button2    : GuiObjs::ObjName<Button   > {~Button2   ();   Button2   ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                       };
   class CheckBox2  : GuiObjs::ObjName<CheckBox > {~CheckBox2 ();   CheckBox2 ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                         virtual void update(C GuiPC &gpc)override;                       };
   class ComboBox2  : GuiObjs::ObjName<ComboBox > {~ComboBox2 ();   ComboBox2 ();                      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                         virtual void update(C GuiPC &gpc)override;  };
   class GuiCustom2 : GuiObjs::ObjName<GuiCustom> {~GuiCustom2();   GuiCustom2();                virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                         virtual void draw(C GuiPC &gpc)override;             };
   class Desktop2   : GuiObjs::ObjName<Desktop  > {~Desktop2  ();   Desktop2  (); /*virtual GuiObj* test(C Vec2 &pos, GuiObj* &mouse_wheel)override {return (IgnoreSelected && GuiEdit.sel.has(this)) ? null : super.test(pos, mouse_wheel);}*/ };
   class GuiImage2  : GuiObjs::ObjName<GuiImage > {~GuiImage2 ();   GuiImage2 ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                                        };
   class List2      : GuiObjs::ObjName<_List    > {~List2     ();   List2     ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                                        };
   class Menu2      : GuiObjs::ObjName<Menu     > {~Menu2     ();   Menu2     ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                                        };
   class MenuBar2   : GuiObjs::ObjName<MenuBar  > {~MenuBar2  ();   MenuBar2  ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                                       };
   class Progress2  : GuiObjs::ObjName<Progress > {~Progress2 ();   Progress2 ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                                        };
   class Region2    : GuiObjs::ObjName<Region   > {~Region2   ();   Region2   ();                virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                         };
   class SlideBar2  : GuiObjs::ObjName<SlideBar > {~SlideBar2 ();   SlideBar2 ();                      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                      };
   class Slider2    : GuiObjs::ObjName<Slider   > {~Slider2   ();   Slider2   ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                                        };
   class Tabs2      : GuiObjs::ObjName<Tabs     > {~Tabs2     ();   Tabs2     ();                                                                         virtual GuiObj* test(C GuiPC&gpc, C Vec2&pos, GuiObj*&mouse_wheel)override;                                                        };
   class Text2      : GuiObjs::ObjName<Text     > {~Text2     ();   Text2     ();                       virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                             virtual void draw(C GuiPC &gpc)override;                                            };
   class TextBox2   : GuiObjs::ObjName<TextBox  > {~TextBox2  ();   TextBox2  ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                             virtual void update(C GuiPC &gpc)override;                          virtual void draw(C GuiPC&gpc)override; };
   class TextLine2  : GuiObjs::ObjName<TextLine > {~TextLine2 ();   TextLine2 ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                             virtual void update(C GuiPC &gpc)override;                          virtual void draw(C GuiPC&gpc)override; };
   class Viewport2  : GuiObjs::ObjName<Viewport > {~Viewport2 ();   Viewport2 ();virtual GuiObj*test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                             virtual void draw(C GuiPC &gpc)override;                        };
   class Window2    : GuiObjs::ObjName<Window   > {~Window2   ();   Window2   ();                virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;                                                             virtual void update(C GuiPC &gpc)override;        };

   class Objects
   {
      GuiObj *parent;

      Memx<Button2   > button  ;
      Memx<CheckBox2 > checkbox;
      Memx<ComboBox2 > combobox;
      Memx<GuiCustom2> custom  ;
      Memx<Desktop2  > desktop ;
      Memx<GuiImage2 > image   ;
      Memx<List2     > list    ;
      Memx<Menu2     > menu    ;
      Memx<MenuBar2  > menubar ;
      Memx<Progress2 > progress;
      Memx<Region2   > region  ;
      Memx<SlideBar2 > slidebar;
      Memx<Slider2   > slider  ;
      Memx<Tabs2     > tabs    ;
      Memx<Text2     > text    ;
      Memx<TextBox2  > textbox ;
      Memx<TextLine2 > textline;
      Memx<Viewport2 > viewport;
      Memx<Window2   > window  ;

      // get
      Memx<GuiObj>* container(GUI_OBJ_TYPE type);
      bool containsMain(GuiObj *go);            // if 'go' is stored in this objects
      bool contains    (GuiObj *go); // if 'go' is stored in this objects

      // manage
      void del();
      void create(C GuiObjs &objs); // create from
      void copyTo(GuiObjs &objs)C;

      // operations
      GuiObj* New(GUI_OBJ_TYPE type);
      void remove(GuiObj *go);

      // io
      bool save(File &f, cchar *path)C;
      bool load(File &f, cchar *path);
      bool save(C Str &name)C;
      bool load(C Str &name);

public:
   Objects();
   };

   static void PosX (  GuiObj &go, C Str &t);
   static Str  PosX (C GuiObj &go          );
   static void PosY (  GuiObj &go, C Str &t);
   static Str  PosY (C GuiObj &go          );
   static void SizeX(  GuiObj &go, C Str &t);
   static Str  SizeX(C GuiObj &go          );
   static void SizeY(  GuiObj &go, C Str &t);
   static Str  SizeY(C GuiObj &go          );
   static void Desc (  GuiObj &go, C Str &t);
   static Str  Desc (C GuiObj &go          );

   static void CheckBoxOn   (  CheckBox &checkbox, C Str &t);
   static Str  CheckBoxOn   (C CheckBox &checkbox          );
   static void ProgressValue(  Progress &progress, C Str &t);
   static Str  ProgressValue(C Progress &progress          );
   static void RegionSbc    (  Region   &region  , C Str &t);
   static Str  RegionSbc    (C Region   &region            );

   static void SlideBarLength     (  SlideBar &slidebar, C Str &t);
   static Str  SlideBarLength     (C SlideBar &slidebar          );
   static void SlideBarLengthTotal(  SlideBar &slidebar, C Str &t);
   static Str  SlideBarLengthTotal(C SlideBar &slidebar          );
   static void SlideBarFrac       (  SlideBar &slidebar, C Str &t);
   static Str  SlideBarFrac       (C SlideBar &slidebar          );

   static void SliderValue(  Slider &slider, C Str &t);
   static Str  SliderValue(C Slider &slider          );

                     static Str  TabsValid   (C Tabs &tabs          );
                     static void TabsValid   (  Tabs &tabs, C Str &t);
                     static Str  TabsLayout  (C Tabs &tabs          );
                     static void TabsLayout  (  Tabs &tabs, C Str &t);
                     static Str  TabsSpace   (C Tabs &tabs          );
                     static void TabsSpace   (  Tabs &tabs, C Str &t);
                     static Str  TabsAutoSize(C Tabs &tabs          );
                     static void TabsAutoSize(  Tabs &tabs, C Str &t);
   template<int i>   static Str  TabsTabText (C Tabs &tabs          );
   template<int i>   static void TabsTabText (  Tabs &tabs, C Str &t);

   static void TextCodeFunc(  Text &text, C Str &t);
   static Str  TextCodeFunc(C Text &text          );

   static Str  TextLineText     (C TextLine &textline          );
   static void TextLineText     (  TextLine &textline, C Str &t);
   static Str  TextLinePassword (C TextLine &textline          );
   static void TextLinePassword (  TextLine &textline, C Str &t);
   static Str  TextLineMaxLength(C TextLine &textline          );
   static void TextLineMaxLength(  TextLine &textline, C Str &t);

   static Str  TextBoxText     (C TextBox &textbox          );
   static void TextBoxText     (  TextBox &textbox, C Str &t);
   static Str  TextBoxMaxLength(C TextBox &textbox          );
   static void TextBoxMaxLength(  TextBox &textbox, C Str &t);

   static void WindowLevel     (  Window &window, C Str &t);
   static Str  WindowLevel     (C Window &window          );
   static void WindowMovable   (  Window &window, C Str &t);
   static Str  WindowMovable   (C Window &window          );
   static void WindowResizable (  Window &window, C Str &t);
   static Str  WindowResizable (C Window &window          );
   static void WindowBarVisible(  Window &window, C Str &t);
   static Str  WindowBarVisible(C Window &window          );
 //static void WindowBarHeight (  Window &window, C Str &t) {       window.barHeight (TextFlt (t));}
 //static Str  WindowBarHeight (C Window &window          ) {return window.barHeight ();}

   static void ButtonImage  (Button   &button  , C Str &t);
   static void ButtonSkin   (Button   &button  , C Str &t);
   static void CheckBoxSkin (CheckBox &checkbox, C Str &t);
   static void ComboBoxSkin (ComboBox &combobox, C Str &t);
   static void ImageImage   (GuiImage &image   , C Str &t);
   static void ProgressSkin (Progress &progress, C Str &t);
   static void SlideBarSkin (SlideBar &slidebar, C Str &t);
   static void SliderSkin   (Slider   &slider  , C Str &t);
   static void RegionSkin   (Region   &region  , C Str &t);
   static void TabsSkin     (Tabs     &tabs    , C Str &t);
   static void TextSkin     (Text     &text    , C Str &t);
   static void TextLineSkin (TextLine &textline, C Str &t);
   static void TextBoxSkin  (TextBox  &textbox , C Str &t);
   static void WindowSkin   (Window   &window  , C Str &t);
   static void TextTextStyle(Text     &text    , C Str &t);

   static Str ButtonImage  (C Button   &button  );
   static Str ButtonSkin   (C Button   &button  );
   static Str CheckBoxSkin (C CheckBox &checkbox);
   static Str ComboBoxSkin (C ComboBox &combobox);
   static Str ImageImage   (C GuiImage &image   );
   static Str ProgressSkin (C Progress &progress);
   static Str SlideBarSkin (C SlideBar &slidebar);
   static Str SliderSkin   (C Slider   &slider  );
   static Str RegionSkin   (C Region   &region  );
   static Str TabsSkin     (C Tabs     &tabs    );
   static Str TextSkin     (C Text     &text    );
   static Str TextLineSkin (C TextLine &textline);
   static Str TextBoxSkin  (C TextBox  &textbox );
   static Str WindowSkin   (C Window   &window  );
   static Str TextTextStyle(C Text     &text    );

   class ObjEdit : PropWin
   {
      static GuiSkinPtr Skin;

      static void PreChanged(C Property &prop); // set skin because that may affect object properties (for example slidebar button visibility, window client rectangle)
      static void    Changed(C Property &prop);

      Rect create(GUI_OBJ_TYPE type, int level);
      virtual void draw(C GuiPC &gpc)override;
   };

   static GuiObj* MainObject(GuiObj *go);
   static GuiObj* MainParent(GuiObj *parent, bool allow_tabs);
   static GuiObj* FirstContainer(GuiObj *go, bool allow_tabs);

   enum OP
   {
      OP_NEW     ,
      OP_DEL     ,
      OP_MOVE    ,
      OP_SIZE    ,
      OP_COPY    ,
      OP_SEPARATE,
      OP_SCALE   ,
      OP_PARENT  ,
      OP_NUM     ,
   };
   static GUI_OBJ_TYPE obj_new[]
; static int obj_new_elms;

   class Panel : Window
   {
      flt            grid_size;
      Button         draw_proportions, grid_align, undo, redo, locate, align_sel, aspect, move_up, move_down;
      Tabs           op, obj_type;
      Memx<Property> props;

      static void AlignSel(Panel &panel);
      static void MoveUp  (Panel &panel);
      static void MoveDown(Panel &panel);
      static void Aspect  (Panel &panel);
      static void Undo    (Panel &panel);
      static void Redo    (Panel &panel);
      static void Locate  (Panel &panel);

      GUI_OBJ_TYPE objType()C;
      void align(Vec2 &pos, bool force=false);
      void alignTrunc(Vec2 &pos);
      Panel& create();

public:
   Panel();
   };

   class Change : Edit::_Undo::Change
   {
      GuiObjs objs;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   bool              changed, selecting, disable_selection;
   Vec2              sel_pos, helper;
   UID               elm_id;
   Elm              *elm;
   Objects           objs;
   Panel             panel;
   ObjEdit           obj_edit[GO_NUM];
   Button            new_tab;
   int               op;
   GuiObj           *sel_parent, *last;
   Memc<GuiObj*>     sel, lit, temp;
   Edit::Undo<Change> undo;   void undoVis();

   bool selected()C;      
   void selectedChanged();
   void flush();
   void set(Elm *elm);
   void activate(Elm *elm);
   void toggle  (Elm *elm);
   void erasing(C UID &elm_id);

   static void DelTab(ptr user);
   static void NewTab(GuiView &gv);
   GuiView& createPropsTabs(Tabs *tabs);
   void createProps();
   GuiView& create(GuiObj &parent);
   void resize();                   
   virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
   void elmChanged(C UID &elm_id);
   void setChanged(bool to_gui=false);

   // get
   GuiObj*          cur(                 );
   GuiObj*          cur(GUI_OBJ_TYPE type);
   bool selectedParents(GuiObj *obj);    // if object or its parents are selected
   bool        editable(GuiObj *obj);    
   Rect         selRect(           );    
   bool      hasKbFocus(           );

   // operations
   void removed(GuiObj*obj);
   void sort();              static int CompareOrder(GuiObj*C &a, GuiObj*C &b);
   void alignSel();
   void moveUpSel  (); // process from end  , but keep selection order
   void moveDownSel(); // process from start, but keep selection order
   void resetAspect();
   void processSel();
   GuiView& toGui();
   GuiObj* copy(GuiObj &obj, GuiObj *parent, C Vec2 *screen_pos, C Vec2 *offset);
   void clean(); // clean all references which point to invalid files
   virtual void update(C GuiPC &gpc)override;
   static void Highlight(GuiObj &go, C Color &color, flt a);
   void drawPreview(GuiObj &obj, C GuiPC &gpc, GuiObj &parent);
   virtual void draw(C GuiPC &gpc)override;
   void drawPre();
   void draw();

   // drag
   void drag(Memc<UID> &elms, GuiObj* &obj, C Vec2 &screen_pos);

public:
   GuiView();
};
/******************************************************************************/
/******************************************************************************/
extern GuiView GuiEdit;
/******************************************************************************/
