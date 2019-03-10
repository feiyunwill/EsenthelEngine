/******************************************************************************/
/******************************************************************************/
class ObjListClass : ClosableWindow
{
   class Elm
   {
      bool      opened;
      uint      type;
      flt       offset;
      Obj      *obj;
      Str       group, obj_type, base;
      Vec       pos;
      Memx<Elm> children;

public:
   Elm();
   };
   bool      _changed;
   Button     refresh;
   Text      tgroup, tparams, tgrass;
   Tabs       group;
   TextLine   filter;
   CheckBox   params, grass;
   Region     region;
   List<Elm>  list;
   Memx<Elm>  data;
   Memc<Str>  opened;
   TextBlack  ts, ts2;

   static void  GroupChanged(ObjListClass &ol);
   static void FilterChanged(ObjListClass &ol);
   static void       Refresh(ObjListClass &ol);
   static void          Hide(ObjListClass &ol);
   static void    SelChanged(ObjListClass &ol);

   void setChanged();     
   ObjListClass& create();

   static int CompareOL(C Elm &elm, C Str &group);
   Elm& dataNew(C Str &group);

   static Str ReplaceSlash(C Str &str);                                
   static bool SetVisible(Memt<bool> &visible, C Str&filter, bool filter_params, bool hide_grass, Memx<Elm> &data, bool parent_visible=true);
   void setVisible();
   static void SetType(Memx<Elm> &data, uint type);
   void set();
   void selChanged();

   virtual Rect sizeLimit()C override;
           C Rect& rect()C;                   
   virtual Window& rect(C Rect &rect)override;
   virtual void update(C GuiPC &gpc)override;
   void removed(Obj &obj);

public:
   ObjListClass();
};
/******************************************************************************/
/******************************************************************************/
extern ObjListClass ObjList;
/******************************************************************************/
