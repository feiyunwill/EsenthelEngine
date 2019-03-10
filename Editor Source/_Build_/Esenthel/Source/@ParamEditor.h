/******************************************************************************/
/******************************************************************************/
class ParamEditor : Region
{
   class RenameWindow : ClosableWindow
   {
      Param      data;
      UID          id;
      TextLine   dest;
      ParamEditor &pe;

      RenameWindow(ParamEditor &pe);

      void create();
      void rename(C Str &src, PARAM_TYPE type, C UID &id);
      static bool Rename(ParamEditor &pe, EditObject &obj, C RenameWindow &rename);
      virtual void update(C GuiPC &gpc)override;
   };

   class ParamWindow : ClosableWindow
   {
      static const flt elm_h;
      enum MODE
      {
         CUR,
         BASE,
         BASE_OVR,
      };

      class EditParamEx : EditParam
      {
         bool          cur, base, base_ovr;
       C GuiEditParam *gui;

         EditParamEx& create(C EditParam &src, MODE mode);

public:
   EditParamEx();
      };
      class Param
      {
         class GuiColor : GuiCustom
         {
            byte alpha;

            virtual void update(C GuiPC &gpc)override;
            virtual void draw(C GuiPC &gpc)override;

public:
   GuiColor();
         };

         bool           cur, base, base_ovr, valid_type, valid_value,
                        multi_obj , // if belongs to multiple objects
                        multi_cur , // if has different 'cur ' on different objects
                        multi_base, // if has different 'base' on different objects
                        multi_val ; // if has different values on different objects
         int            priority;
         UID            id;
        ::Param          src;
         CheckBox       ovr;
         Button         remove, clear;
         ComboBox       type;
         TextLine       name;
         CheckBox       val_checkbox;
         TextLine       val_textline;
         ComboBox       val_combobox;
         GuiColor       val_color;
         Button         val_adjust;
         ColorPicker    cp;
         ParamEditor   *pe;
         EditObject    *owner; // this is used only for memory address comparison
         bool           min_use  , max_use  ;
         dbl            min_value    , max_value;
         flt            mouse_edit_speed, custom_mouse_edit_speed;
         Vec2           mouse_edit_value;
       C GuiEditParam  *gui;
         PROP_MOUSE_EDIT_MODE mouse_edit_mode;

         static void ChangedOvr  (Param &param); // if(override parameter) {copy from base to cur} else {remove parameter}
         static void RemoveParam (Param &param);
         static void ClearValue  (Param &param);
         static void ChangedType (Param &param);
         static void ChangedName (Param &param);
         static void ChangedCheck(Param &param);
         static void ChangedCombo(Param &param);
         static void ChangedColor(Param &param);
         static void ChangedText (Param &param);

         static void MouseEdit(Param &param);

         bool forceEnum()C;           
         int  enumValue()C;           
         bool contains(C GuiObj*obj)C;
         void setSkin();
         void toGui();
         Param& moveToTop();
         Param& setRect(int i, flt list_w);
         void desc(C Str &desc);
         dbl clamp(dbl value);
         void set(C Vec2 &value);
         Param& create(ParamEditor &pe, EditObject &owner, EditParamEx &src); // !! do not store pointer to 'src' because it might be temporary !!
         void enumChanged();

public:
   Param();
      };
      class SubObj
      {
         static void RemoveSubObj(SubObj &sub_obj);

         UID          id;
         Button       remove, obj;
         ParamEditor *pe;

         SubObj& setRect(int i, flt list_w);
         SubObj& create(ParamEditor &pe, EditObject::SubObj &src, int i, flt list_w);

public:
   SubObj();
      };

      bool         is_class;
      Panel        panel;
      Text         type, name, value;
      Button       new_par;
      Region       region;
      Memx<Param>  params;
      Memx<SubObj> sub_objs;
      TextBlack    ts;
      ParamEditor &pe;

      ParamWindow(ParamEditor &pe);

      static void     NewParam(ParamEditor &pe);        
      static int  CompareParam(C Param &p0, C Param&p1);

      ParamWindow& create(C Rect &rect, bool is_class);
      virtual Rect sizeLimit()C override;
      flt listWidth();                    
    C Rect& rect()C;                      
      virtual ParamWindow& rect(C Rect &rect)override;
      virtual void update(C GuiPC &gpc)override;
      virtual ParamWindow& hide()override;      

      Param* findParam(GuiObj *go);
      void addParams(EditObject &obj);
      void addSubObjs(EditObject &obj);
      void setSkin();                
      void toGui();
   };

   class Change : Edit::_Undo::Change
   {
      EditObject data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   class MeshVariation
   {
      Str  name;
      uint id;

    C Str& originalName()C; // if valid 'id' then return 'name', if invalid then it means it was created from default variation which has no name

      void set(C Str &name, uint id);

public:
   MeshVariation();
   };

   Button       multi, undo, redo;
   Text         t_class, t_path, t_mesh_var, t_editor_type, t_const;
   ComboBox     v_class, v_path, v_mesh_var, v_editor_type;
   CheckBox     o_class, o_const, v_const, o_path, o_mesh_var;
   Button       b_class;
   TextLine     t_obj;
   ParamWindow   param_window;
   RenameWindow rename_window;
   TextBlack    ts_black;
   TextWhite    ts_white;
   bool         world;
   EditObject   temp_p, *p;
   bool         changed;
   Memc<MeshVariation> mesh_variations;
   Edit::Undo<Change> undos;   void undoVis();

   static NameDesc path_mode[]
; ASSERT(OBJ_PATH_CREATE==0 && OBJ_PATH_IGNORE==1 && OBJ_PATH_BLOCK==2);
   static NameDesc etype_mode[]
; ASSERT(EDIT_OBJ_MESH==0 && EDIT_OBJ_LIGHT_POINT==1 && EDIT_OBJ_LIGHT_CONE==2 && EDIT_OBJ_PARTICLES==3 && EDIT_OBJ_NUM==4);

   template<typename TYPE> void multiFunc(bool Func(ParamEditor &pe, EditObject &obj, C TYPE &user), C TYPE &user     );
          void multiFunc(bool Func(ParamEditor &pe, EditObject &obj, cptr    user), cptr    user=null);

   static bool ChangedOConst(ParamEditor &pe, EditObject &obj, cptr user=null);
   static void ChangedOConst(ParamEditor &pe);

   static bool ChangedConst(ParamEditor &pe, EditObject &obj, cptr user=null);
   static void ChangedConst(ParamEditor &pe);

   static void ChangedOClass(ParamEditor &pe); // Access is merged with Type/Class

   class AccessType
   {
      OBJ_ACCESS access;
      UID        type;

public:
   AccessType();
   };
   static bool ChangedClass(ParamEditor &pe, EditObject &obj, C AccessType &at);
   static void ChangedClass(ParamEditor &pe); // Access is merged with Type/Class

   static void ChangedEditorType(ParamEditor &pe);

   static bool ChangedOPath(ParamEditor &pe, EditObject &obj, cptr user=null);
   static void ChangedOPath(ParamEditor &pe);

   static bool ChangedPath(ParamEditor &pe, EditObject &obj, cptr user=null);
   static void ChangedPath(ParamEditor &pe);

   static bool ChangedOMeshVariation(ParamEditor &pe, EditObject &obj, cptr user=null);
   static void ChangedOMeshVariation(ParamEditor &pe);

   static bool ChangedMeshVariation(ParamEditor &pe, EditObject &obj, cptr user=null);
   static void ChangedMeshVariation(ParamEditor &pe);

   static void ToBase(ParamEditor &pe);

   static void ChangedMulti(ParamEditor &edit); // refresh parameter list because their names will display differently when 'multi' mode is enabled
   static void Undo        (ParamEditor &edit);
   static void Redo        (ParamEditor &edit);

   ParamWindow::Param* findParam(GuiObj *go);

   void setUndo(cptr change_type=null, bool force_create=false);
   void setChanged(bool refresh_gui=true);
   void toGuiMeshVariation(uint mesh_variation_id);
   void toGui(bool params=true);
   void setSkin();            

   void create(GuiObj &parent, bool is_class=false, bool world=false);
   void enumChanged();
   void meshVariationChanged();
   void objChanged(C UID *obj_id=null);
   virtual ParamEditor& move(C Vec2 &delta)override;
   virtual void paramWindowHidden();  

   void newSubObj(C UID &elm_obj_id);
   void removeSubObj(ParamWindow::SubObj &sub_obj);

   static bool NewParam(ParamEditor &pe, EditObject &obj, cptr user=null);
   void newParam();

   static bool NewParam(ParamEditor &pe, EditObject &obj, C Param &param);
   void newParam(ParamWindow::Param &src, bool refresh_gui=true); // override 'src' parameter (this is done only if the object has a base containing that parameter)

   static bool RemoveParam(ParamEditor &pe, EditObject &obj, C Param &src);
   void removeParam(ParamWindow::Param &src);

   static bool ClearValue(ParamEditor &pe, EditObject &obj, C Param &src);
   void clearValue(ParamWindow::Param &src);

   static bool SetType(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src);
   void setType(ParamWindow::Param &src);

   static bool SetName(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src);
   void setName(ParamWindow::Param &src);

   static bool SetBool(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src);
   void setBool(ParamWindow::Param &src);

   static bool SetEnum(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src);
   void setEnum(ParamWindow::Param &src);

   static bool SetColor(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src);
   void setColor(ParamWindow::Param &src); // don't refresh gui because it will delete color pickers

   class ParamIDs : Param
   {
      Memc<UID> ids;
   };

   static bool SetText(ParamEditor &pe, EditObject &obj, C ParamIDs &src);
   void setText(ParamWindow::Param &src); // don't refresh gui because it will lose textline keyboard focus and cursor/selection

   static bool SetBase(ParamEditor &pe, EditObject &obj, C EditObjectPtr &base);
   void setBase(C EditObjectPtr &base);

   static bool SetID(ParamEditor &pe, EditObject &obj, C ParamIDs &src);
   static bool IncludeID(ParamEditor &pe, EditObject &obj, C ParamIDs &src);
   void drag(Memc<UID> &elms, GuiObj* &obj, C Vec2 &screen_pos);

public:
   ParamEditor();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
