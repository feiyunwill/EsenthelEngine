/******************************************************************************/
const flt TextHeight=0.06;
/******************************************************************************/
enum MODE
{
   MODE_CONNECTION,
   MODE_PROJECTS  ,
   MODE_USERS     ,
}
cchar8 *ModeText[]=
{
   "Connection",
   "Projects"  ,
   "Users"     ,
};
Tabs Mode;
/******************************************************************************/
class ProjectView : Region
{
   List<Project> list;
   Menu          menu;
   UID           menu_id=UIDZero;

   static void Rename          (Project &proj) {RenameProj.activate(proj);}
   static void EraseRemovedElms(Project &proj) { EraseProj.activate(proj);}

   ProjectView& create()
   {
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Project, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      super.create();
      T+=list.create(lc, Elms(lc)); list.flag|=LIST_MULTI_SEL;
      return T;
   }
   void refresh()
   {
      list.setData(Projects);
   }
   void projMenu(C Vec2 &pos)
   {
      Node<MenuElm> node;
      if(list.sel.elms()==1)if(Project *proj=list.absToData(list.sel[0]))
      {
         menu_id=proj.id;
         node.New().create("Rename"                , Rename          , *proj);
         node++;
         node.New().create("Erase Removed Elements", EraseRemovedElms, *proj);
      }
      if(node.children.elms())
      {
         Gui+=menu.create(); menu.list.elmHeight(0.05); menu.setData(node).posRU(pos).activate();
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      REPA(MT)if(MT.guiObj(i)==&list)
      {
         if(MT.bp(i, 1)
         || MT.b (i   ) && MT.touch(i) && MT.life(i)>=LongPressTime && !MT.dragging(i) && !Gui.menu())
         {
            list.cur=list.screenToVisY(MT.pos(i).y);
            if(!list.sel.has(list.visToAbs(list.cur)))list.setCur(list.cur);
            projMenu(MT.pos(i)); break;
         }
      }
   }
}
/******************************************************************************/
class UserView : Region
{
   Memx<ComboBox> access;
   Memx<Button  > edit;
   List<User    > list;
   
   static void AccessChanged(ComboBox &access)
   {
      User &user=*(User*)access.user;
      user.access=USER_ACCESS(access());
      SaveSettings();
      if(user.access==UA_NO_ACCESS)Server.disconnect(user); // disconnect user which now has no access
   }
   static void Edit(User &user) {EditUser.activate(user);}

   static Str RegisterDate(C User &user) {DateTime local=user.  reg_date; local.toLocal(); return local.asText();}
   static Str    LoginDate(C User &user) {DateTime local=user.login_date; local.toLocal(); return local.asText();}

   UserView& create()
   {
      ListColumn lc[]=
      {
         ListColumn(MEMBER(User, email     ), 0.42, "E-mail"    ),
         ListColumn(MEMBER(User, name      ), 0.36, "Name"      ),
         ListColumn(MEMBER(User, access    ), 0.25, "Access"    ),
         ListColumn(RegisterDate            , 0.40, "Registered"),
         ListColumn(   LoginDate            , 0.40, "Last Login"),
         ListColumn(MEMBER(User, online    ), 0.12, "Online"    ),
      };
      super.create();
      T+=list.create(lc, Elms(lc)); list.draw_column=-1; FlagDisable(list.flag, LIST_SORTABLE); // disable sorting so user edit buttons have correct positions
      return T;
   }
   void refresh()
   {
      ListColumn lct[]=
      {
         ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };

      access.clear();
      edit  .clear();
      list  .setData(Users);
      Rect rect=list.column(2).rect();
      flt  h   =list.elmHeight();
      FREPA(Users)
      {
         flt   y=-list.columnHeight()-i*h;
         User &user=Users[i];
         ComboBox &access=T.access.New().create(Rect_LU(rect.min.x, y, rect.w(), h)).setColumns(lct, Elms(lct)).setData(UserAccessText, UA_NUM);
         access.menu.list.setElmDesc(MEMBER(NameDesc, desc));
         access.set(user.access, QUIET);
         access.func(AccessChanged, access); access.user=&user; FlagDisable(access.flag, COMBOBOX_MOUSE_WHEEL); T+=access;
         T+=edit.New().create(Rect_LU(list.column(5).rect().max.x, y, 0.14, h), "Edit").func(Edit, user);
      }
   }
}
/******************************************************************************/
class PropWin : ClosableWindow
{
   Memx<Property> props;
   TextStyle      ts;

   Rect create(C Str &name, C Vec2 &lu=Vec2(0.02, -0.02), C Vec2 &text_scale=0.036, flt property_height=0.043, flt value_width=0.3)
   {
      ts.reset(true).size=text_scale; ts.align.set(1, 0);
      Gui+=super.create(name).hide();
      Rect r=AddProperties(props, T, lu, property_height, value_width, &ts);
      MAX(r.max.x, defaultBarFullWidth()+button[2].rect().w()); // here 'button[2]' is hidden so add it manually
      Vec2 padd=defaultInnerPaddingSize()+0.02; rect(Rect_RD(D.w(), -D.h(), r.max.x+padd.x, -r.min.y+padd.y));
      return r;
   }
   Property&    add (C Str &name=S, C MemberDesc &md=MemberDesc()) {Property &p=props.New(); p.create(name, md); return p;}
   PropWin&   toGui (cptr data) {REPAO(props).  toGui (data); return T;}
   PropWin& fromGui ( ptr data) {REPAO(props).fromGui (data); return T;}
   PropWin& autoData( ptr data) {REPAO(props).autoData(data); return T;}   ptr autoData()C {FREPA(props)if(ptr data=props[i].autoData())return data; return null;}
   PropWin& changed (void (*changed)(C Property &prop), void (*pre_changed)(C Property &prop)=null) {REPAO(props).changed(changed, pre_changed); return T;}

   virtual PropWin& hide()override {super.hide(); REPAO(props).close(); return T;}
}
/******************************************************************************/
class NewUserWin : PropWin
{
   class UserData
   {
      Str email, name, pass;
      USER_ACCESS access;
   }
   UserData user_data;
   Button   ok;

   static void OK(NewUserWin &nuw) {if(CreateUser(nuw.user_data.email, nuw.user_data.name, nuw.user_data.pass, nuw.user_data.access))nuw.hide();}

   void create()
   {
      ListColumn lct[]=
      {
         ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };

                 props.New().create("E-mail"  , MemberDesc(MEMBER(UserData, email )));
                 props.New().create("Name"    , MemberDesc(MEMBER(UserData, name  )));
                 props.New().create("Password", MemberDesc(MEMBER(UserData, pass  )));
Property &access=props.New().create("Access"  , MemberDesc(MEMBER(UserData, access))).setEnum();
      access.combobox.setColumns(lct, Elms(lct)).setData(UserAccessText, UA_NUM).menu.list.setElmDesc(MEMBER(NameDesc, desc));

      super.create("Create New User", Vec2(0.02, -0.02), 0.05, 0.065, 0.6); autoData(&user_data); button[2].show();
      T+=ok.create(Rect_U(clientWidth()/2, -clientHeight()-0.01, 0.3, 0.06), "OK").func(OK, T);
      Rect rect=T.rect(); rect.min.y-=0.10; T.rect(rect); pos(size()*Vec2(-0.5, 0.5));
   }
}
/******************************************************************************/
class EditUserWin : PropWin
{
   class UserData
   {
      UID id;
      Str email, name;
   }
   UserData user_data;
   Button   ok, change_pass, remove;

   static void OK(EditUserWin &euw)
   {
      if(!ValidEmail(euw.user_data.email))Gui.msgBox(S, "Invalid e-mail address");else
      {
         if(User *user=FindUser(euw.user_data.id))
         {
            user.email=euw.user_data.email;
            user.name =euw.user_data.name ;
            SaveSettings();
         }
         euw.hide();
      }
   }
   static void ChangePass(EditUserWin &euw) {.ChangePass.activate(euw.user_data.id);}
   static void Remove    (EditUserWin &euw) { RemoveUser.activate(euw.user_data.id);}

   void activate(User &user)
   {
      super.activate();
      user_data.id   =user.id;
      user_data.email=user.email;
      user_data.name =user.name;
      toGui(&user_data);
   }
   void create()
   {
      props.New().create("E-mail", MemberDesc(MEMBER(UserData, email)));
      props.New().create("Name"  , MemberDesc(MEMBER(UserData, name )));

      super.create("Edit User", Vec2(0.02, -0.02), 0.05, 0.065, 0.6); autoData(&user_data); button[2].show();
      T+=ok         .create(Rect_U(clientWidth()/2, -clientHeight()-0.01, 0.4, 0.06), "OK"             ).func(OK        , T);
      T+=change_pass.create(Rect_U(clientWidth()/2, -clientHeight()-0.08, 0.4, 0.06), "Change Password").func(ChangePass, T);
      T+=remove     .create(Rect_U(clientWidth()/2, -clientHeight()-0.15, 0.4, 0.06), "Delete User"    ).func(Remove    , T);
      Rect rect=T.rect(); rect.min.y-=0.26; T.rect(rect); pos(size()*Vec2(-0.5, 0.5));
   }
}
/******************************************************************************/
class ChangePassWin : ClosableWindow
{
   UID      user_id;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Enter New Password").level(EditUser.level()+1).hide(); button[2].show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void activate(C UID &user_id) {T.user_id=user_id; textline.clear(); super.activate();}
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))
         {
            if(!ValidPass(textline()))Gui.msgBox(S, "Invalid password");else
            {
               if(User *user=FindUser(user_id))
               {
                  user.pass=PassToMD5(textline());
                  SaveSettings();
               }
               hide(); Kb.eatKey();
            }
         }
      }
   }
}
/******************************************************************************/
class RemoveUserWin : ClosableWindow
{
   Text   t_name;
   Button     ok;
   UID   user_id;

   static void OK(RemoveUserWin &ruw)
   {
      if(User *user=FindUser(ruw.user_id))
      {
         Users.removeData(user, true);
         SaveSettings();
         UV.refresh();
      }
      ruw       .hide();
      EditUser  .hide();
      ChangePass.hide();
   }

   void activate(C UID &user_id)
   {
      if(User *user=FindUser(user_id))
      {
         setTitle(S+"Delete User \""+user.email+"\"");
         super.activate();
         T.user_id=user_id;
      }
   }
   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 0.85, 0.37), "Delete User").level(EditUser.level()+1).hide(); button[2].show();
      T+=t_name.create(Rect_C(clientWidth()/2, -0.11, 0.7, 0.0), "Are you sure you wish to delete selected user?"); t_name.auto_line=AUTO_LINE_SPACE_SPLIT;
      T+=    ok.create(Rect_C(clientWidth()/2, -0.23, 0.3, 0.06), "OK").func(OK, T);
   }
}
/******************************************************************************/
class EraseRemovedElms : ClosableWindow
{
   Text   text;
   Button ok;
   UID    proj_id;

   static void OK(EraseRemovedElms &ere)
   {
      if(Server.clients.elms())Gui.msgBox(S, "Can't Erase Removed Elements while there are Clients connected, as they will immediately reupload removed elements.");else
      if(ProjectHierarchy *proj=FindProject(ere.proj_id))
      {
         proj.setHierarchy(); // needed for 'eraseRemoved'
         proj.eraseRemoved();
      }
      ere.hide();
   }

   void activate(Project &proj)
   {
      setTitle(S+"Erase Removed Elements in \""+proj.name+"\"");
      super.activate();
      proj_id=proj.id;
   }
   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1.10, 0.60), "Erase Removed Elements").level(EditUser.level()+1).hide(); button[2].show();
      T+=text.create(Rect_C(clientWidth()/2, -0.22, clientWidth()-0.09, 0.0), "Are you sure you wish to erase all removed elements from the project?\nWarning: This operation cannot be undone!\n\nThis will remove files only on the server - when clients connect, they will reupload the elements.\nMake sure to erase removed elements on all clients while being disconnected from the server."); text.auto_line=AUTO_LINE_SPACE_SPLIT;
      T+=ok  .create(Rect_C(clientWidth()/2, -0.47, 0.3, 0.06), "OK").func(OK, T);
   }
}
/******************************************************************************/
class RenameProjWin : ClosableWindow
{
   Text   t_name;
   TextLine name;
   Button     ok;
   UID   proj_id;

   static void OK(RenameProjWin &rpw) {if(RenameProject(rpw.proj_id, rpw.name()))rpw.hide();}

   void activate(Project &proj)
   {
      setTitle(S+"Rename Project \""+proj.name+"\"");
      super.activate();
      proj_id=proj.id;
      name.set(proj.name).selectAll().activate();
   }
   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 0.75, 0.37), "Rename Project").hide(); button[2].show();
      T+=t_name.create(Vec2  (clientWidth()/2, -0.05), "New Project Name");
      T+=  name.create(Rect_C(clientWidth()/2, -0.12, 0.65, 0.06));
      T+=    ok.create(Rect_C(clientWidth()/2, -0.23, 0.3, 0.06), "OK").func(OK, T);
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(Gui.window()==this && Kb.k(KB_ENTER)){OK(T); Kb.eatKey();}
   }
}
/******************************************************************************/
ProjectView      PV;
UserView         UV;
NewUserWin       NewUser;
EditUserWin      EditUser;
ChangePassWin    ChangePass;
RemoveUserWin    RemoveUser;
RenameProjWin    RenameProj;
EraseRemovedElms EraseProj;
Button           bLocalAddressCopy, bGlobalAddressCopy, bGlobalAddressResolve;
Button           AllowUserRegistration, bNewUser;
Progress         UpdateProgress;
/******************************************************************************/
void  LocalAddressCopy   (ptr user=null) {ClipSet(Server. localAddress().asText());}
void GlobalAddressCopy   (ptr user=null) {ClipSet(Server.globalAddress().asText());}
void GlobalAddressResolve(ptr user=null) {        Server.globalAddress(); bGlobalAddressResolve.hide();}

void NewUserDo(ptr) {NewUser.activate();}

void InitGui()
{
   Gui+=Mode.create(ModeText, Elms(ModeText)).valid(true).set(MODE_CONNECTION);

   Mode.tab(MODE_CONNECTION)+= bLocalAddressCopy   .create("Copy"   ).func( LocalAddressCopy   ).desc(  "Local Address (Local IP) is the address visible in the Local Network.\nYou can use it to connect with other computers in your network.");
   Mode.tab(MODE_CONNECTION)+=bGlobalAddressCopy   .create("Copy"   ).func(GlobalAddressCopy   ).desc("Global Address (Public IP) is the address visible in the Global Network.\n\nYou must use it if you want to connect with computers\nwhich are not located in your network.");
   Mode.tab(MODE_CONNECTION)+=bGlobalAddressResolve.create("Resolve").func(GlobalAddressResolve).desc("Global Address (Public IP) is the address visible in the Global Network.\n\nYou must use it if you want to connect with computers\nwhich are not located in your network.\n\nResolving Public IP address requires connecting to external websites.");

   Mode.tab(MODE_PROJECTS)+=PV.create();

   Mode.tab(MODE_USERS)+=UV.create();
   Mode.tab(MODE_USERS)+=AllowUserRegistration.create("Allow User Registration"); AllowUserRegistration.mode=BUTTON_TOGGLE;
   Mode.tab(MODE_USERS)+=bNewUser             .create("New User"               ).func(NewUserDo);

   NewUser   .create();
   EditUser  .create();
   ChangePass.create();
   RemoveUser.create();
   RenameProj.create();
   EraseProj .create();
}
void ResizeGui()
{
   Mode.rect(Rect_LU(-D.w(), D.h(), 0.9, 0.065));

   PV.rect(Rect(-D.w(), -D.h(), D.w(), Mode.rect().min.y).extend(-0.01));

   bNewUser             .rect(Rect_U(-D.w()*0.5, Mode.rect().min.y-0.01, 0.3, 0.065));
   AllowUserRegistration.rect(Rect_U( D.w()*0.5, Mode.rect().min.y-0.01, 0.6, 0.065));
   UV                   .rect(Rect  (-D.w(), -D.h(), D.w(), bNewUser.rect().min.y).extend(-0.01));

   flt y=Mode.rect().min.y-0.01, h=TextHeight;
    bLocalAddressCopy   .rect(Rect_RU(D.w(), y, 0.3, h)); y-=h;
   bGlobalAddressCopy   .rect(Rect_RU(D.w(), y, 0.3, h));
   bGlobalAddressResolve.rect(Rect_RU(D.w(), y, 0.3, h)); y-=h;
}
/******************************************************************************/
