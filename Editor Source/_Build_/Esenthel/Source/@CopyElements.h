/******************************************************************************/
/******************************************************************************/
class CopyElements : ClosableWindow
{
   class ProjListElm // project as list element
   {
      Str name, path;
      UID id;

      ProjListElm(C Projects::Elm &src);
      ProjListElm(C Project      &src); 

public:
   ProjListElm();
   };

   class ReplaceElms : ClosableWindow
   {
      class ElmListElm // project element as list element
      {
         Str       name;
         UID       id;
         Memc<UID> children; // hidden elements merged with this (the parent)

         ElmListElm(C UID &elm_id);

         static int Compare(C ElmListElm &a, C ElmListElm &b);

public:
   ElmListElm();
      };

      Text             text;
      Memc<ElmListElm> data;
      List<ElmListElm> list;
      Region           region;
      Button           copy;

      static void Copy(ReplaceElms &re);
             void copyDo();

      void create();
      void check();
   };

   Memc<Str        > elms_data;
   List<Str        > elms_list;
   Memc<ProjListElm> proj_data;
   List<ProjListElm> proj_list;
   Text              elms_text, proj_text;
   Region            elms_region, proj_region;
   Button            include_children, include_dependencies, include_parents, copy;
   Memc<UID>         sel, elms_to_copy, elms_copied, elms_replaced, changed_body, processed_dep, texs_to_copy; // need to be binary sorted
   int               src_ver;
   ProjectHierarchy  temp_src , *src ;
   Project           temp_dest, *dest;
   ProjListElm       target;
   UID               root;
   ReplaceElms       replace_elms;
   Pak               esenthel_project_pak;

   static void Refresh(CopyElements &ce);
   static void Copy   (CopyElements &ce);

   bool canCopy()C;
   void create();
   void floodSelected(ElmNode &node, bool parent_selected=false, bool parent_removed=false);
   void includeTex(C UID &tex_id);
   void includeDep(C UID &elm_id);
   void includeDep(C GuiObj &obj); // here all objects should include their dependencies
   void refresh();
   void resize();
   void display(C Str &esenthel_project_file=S);
   void copyDo();
   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos);
   void close();

public:
   CopyElements();
};
/******************************************************************************/
/******************************************************************************/
extern CopyElements CopyElms;
/******************************************************************************/
