/******************************************************************************/
/******************************************************************************/
class Projects
{
   class Elm
   {
      UID      id;
      Str      local_name, server_name, name, path, desc; // 'path' must have 'tailSlash'
      CheckBox synchronize;
      int      sync_val;

public:
   Elm();
   };
   Text     t_proj_path, t_server_login, t_server, t_email, t_pass, t_path_desc, t_login_desc, t_logged_in, t_projects,
            t_editor_network_interface, t_editor_network_interface_desc;
   TextLine proj_path, email, pass, server;
   WindowIO proj_path_io, import_proj_io, import_proj_1_0_io;
   Button   proj_path_explore, proj_path_sel, connect, forgot_pass, disconnect, change_pass, new_proj, import_proj, import_proj_1_0, editor_network_interface, vid_opt;
   ComboBox menu;
   Menu     proj_menu;
   UID      proj_menu_id;

   Memx<Elm>  proj_data;
   List<Elm>  proj_list;
   Region     proj_region;
   MemberDesc proj_list_sync_sort;

   TextBlack ts_left, ts_left_down, ts_small_left_up;

   Elm* findProj(C UID &id);

   static void SelectPath (             Projects &projs);
   static void SelectPath (C Str &name, Projects &projs);
   static void PathExplore(             Projects &projs);
   static void PathChanged(             Projects &projs);
   static void Connect    (             Projects &projs);
   static void Disconnect (             Projects &projs);
   static void ForgotPass (             Projects &projs);
   static void ChangePass (             Projects &projs);
   static void NewProj    (             Projects &projs);

   static void ImportProj(Projects &projs);      
   static void ImportProj(C Str &name, ptr user);

   static void ImportProj_1_0(Projects &projs);      
   static void ImportProj_1_0(C Str &name, ptr user);

   static void HostEditorServer(Projects &projs);

   static void Open       ();
   static void Rename     ();
   static void Remove     ();
   static void ExploreProj();
   static void CompareProj();

   static void ToggleSynchronize(Elm &elm);

   void create();
   void resize();
   void forgotPass();
   void connectDo();
   void refresh();
   void selectProj(C UID &proj_id, bool kb_focus=true);
   bool newProj(C Str &name);
   bool toggleSynchronizeProj(C UID &proj_id, C Str &name);
   bool renameProj(C UID &proj_id, Str name);
   bool removeProj(C UID &proj_id);
   bool open(Elm &proj, bool ignore_lock=false);
   void show();
   void hide();
   void projMenu(C Vec2 &pos, bool touch);
   void update();

public:
   Projects();
};
/******************************************************************************/
/******************************************************************************/
extern Projects Projs;
/******************************************************************************/
