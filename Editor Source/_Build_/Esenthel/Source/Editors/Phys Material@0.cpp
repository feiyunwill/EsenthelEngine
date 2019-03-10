/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
PhysMtrlEditor PhysMtrlEdit;
/******************************************************************************/

/******************************************************************************/
   cchar8 *PhysMtrlEditor::mode_t[]=
   {
      "Average" , // 0
      "Multiply", // 1
      "Minimum" , // 2
      "Maximum" , // 3
   };
/******************************************************************************/
      void PhysMtrlEditor::Change::create(ptr user)
{
         data=PhysMtrlEdit.edit;
         PhysMtrlEdit.undoVis();
      }
      void PhysMtrlEditor::Change::apply(ptr user)
{
         PhysMtrlEdit.edit.undo(data);
         PhysMtrlEdit.setChanged();
         PhysMtrlEdit.toGui();
         PhysMtrlEdit.undoVis();
      }
   void PhysMtrlEditor::undoVis() {SetUndo(undos, undo, redo);}
   void PhysMtrlEditor::Draw(Viewport &viewport) {PhysMtrlEdit.draw();}
          void PhysMtrlEditor::draw()
   {
      if(sim_type)
      {
         cam.setSpherical().set();
         D.clear(TURQ);
         REPAO(actor_stat).draw(GREEN);
         REPAO(actor_dyn ).draw(WHITE);
         SetMatrix();
         MatrixIdentity.draw();
         Vec2 pos=viewport.screenPos();
         pos.x+=viewport.rect().w()*sim_time/sim_length;
         pos.y-=viewport.rect().h();
         Rect_D(pos, 0.025f, 0.04f).draw(ColorAlpha(BLACK, 0.5f));
      }
   }
   void PhysMtrlEditor::PreChanged(C Property &prop) {PhysMtrlEdit.undos.set(&prop);}
   void    PhysMtrlEditor::Changed(C Property &prop) {PhysMtrlEdit.setChanged();}
   void PhysMtrlEditor::FricStat(  PhysMtrlEditor &me, C Str &t) {me.edit.friction_static=TextFlt(t); me.edit.friction_static_time.getUTC();}
   Str  PhysMtrlEditor::FricStat(C PhysMtrlEditor &me          ) {return me.edit.friction_static;}
   void PhysMtrlEditor::FricDyn(  PhysMtrlEditor &me, C Str &t) {me.edit.friction_dynamic=TextFlt(t); me.edit.friction_dynamic_time.getUTC();}
   Str  PhysMtrlEditor::FricDyn(C PhysMtrlEditor &me          ) {return me.edit.friction_dynamic;}
   void PhysMtrlEditor::FricMode(  PhysMtrlEditor &me, C Str &t) {me.edit.friction_mode=PhysMtrl::MODE(TextInt(t)); me.edit.friction_mode_time.getUTC();}
   Str  PhysMtrlEditor::FricMode(C PhysMtrlEditor &me          ) {return me.edit.friction_mode;}
   void PhysMtrlEditor::Boun(  PhysMtrlEditor &me, C Str &t) {me.edit.bounciness=TextFlt(t); me.edit.bounciness_time.getUTC();}
   Str  PhysMtrlEditor::Boun(C PhysMtrlEditor &me          ) {return me.edit.bounciness;}
   void PhysMtrlEditor::BounMode(  PhysMtrlEditor &me, C Str &t) {me.edit.bounciness_mode=PhysMtrl::MODE(TextInt(t)); me.edit.bounciness_mode_time.getUTC();}
   Str  PhysMtrlEditor::BounMode(C PhysMtrlEditor &me          ) {return me.edit.bounciness_mode;}
   void PhysMtrlEditor::Density(  PhysMtrlEditor &me, C Str &t) {me.edit.density=TextFlt(t); me.edit.density_time.getUTC();}
   Str  PhysMtrlEditor::Density(C PhysMtrlEditor &me          ) {return me.edit.density;}
   void PhysMtrlEditor::Damping(  PhysMtrlEditor &me, C Str &t) {me.edit.damping=TextFlt(t); me.edit.damping_time.getUTC();}
   Str  PhysMtrlEditor::Damping(C PhysMtrlEditor &me          ) {return me.edit.damping;}
   void PhysMtrlEditor::ADamping(  PhysMtrlEditor &me, C Str &t) {me.edit.adamping=TextFlt(t); me.edit.adamping_time.getUTC();}
   Str  PhysMtrlEditor::ADamping(C PhysMtrlEditor &me          ) {return me.edit.adamping;}
   void PhysMtrlEditor::Undo(PhysMtrlEditor &editor) {editor.undos.undo();}
   void PhysMtrlEditor::Redo(PhysMtrlEditor &editor) {editor.undos.redo();}
   void PhysMtrlEditor::Locate(PhysMtrlEditor &editor) {Proj.elmLocate(editor.elm_id);}
   ElmPhysMtrl* PhysMtrlEditor::data()C {return elm ? elm->physMtrlData() : null;}
   void PhysMtrlEditor::simType(SIM_TYPE sim_type)
   {
      sim_time=0;
      actor_dyn .clear();
      actor_stat.clear();
      switch(T.sim_type=sim_type)
      {
         case SIM_FRIC:
         {
            REP(7)
            {
               Matrix m; m.setRotateX(i*-0.19f); m.move(Vec((i-3)*2, 0, 0));
               actor_stat.New().create(OBox(Box(1.1f, 1, 10, Vec(0, -0.5f, -4.5f)), m), 0);
               actor_dyn .New().create(OBox(Box(  1, 1,  1, Vec(0,  0.5f,  0  )), m));
            }
         }break;
         
         case SIM_BOUNCE:
         {
            actor_stat.New().create(Box(10, 1, 10, Vec(0, -3, 0)), 0);
            actor_dyn .New().create(Ball(0.5f, Vec(0, 3, 0)));
         }break;

         case SIM_DAMPING:
         {
            actor_dyn.New().create(Ball(0.4f, Vec(-12, 0, 0))).vel(Vec(18, 10, 0));
         }break;

         case SIM_ADAMPING:
         {
            actor_dyn.New().create(Box(4, Vec(0, 0, 0))).gravity(false).maxAngVel(100).angVel(Vec(7, 7, 0));
         }break;
      }
      setMtrl();
   }
   ::PhysMtrlEditor::SIM_TYPE PhysMtrlEditor::desiredSimType()C
   {
      if(fric_stat && fric_stat->contains(Gui.ms())
      || fric_dyn  && fric_dyn ->contains(Gui.ms())
      || fric_mode && fric_mode->contains(Gui.ms()))return SIM_FRIC;
      if(boun      && boun     ->contains(Gui.ms())
      || boun_mode && boun_mode->contains(Gui.ms()))return SIM_BOUNCE;
      if(damp      && damp     ->contains(Gui.ms()))return SIM_DAMPING;
      if(adamp     && adamp    ->contains(Gui.ms()))return SIM_ADAMPING;
      return SIM_NONE;
   }
   void PhysMtrlEditor::create()
   {
      fric_stat=&add("Friction Static" , MemberDesc(DATA_REAL).setFunc(FricStat, FricStat)).desc("Object friction when it is not moving").min(0);
      fric_dyn =&add("Friction Dynamic", MemberDesc(DATA_REAL).setFunc(FricDyn , FricDyn )).desc("Object friction when it is moving").min(0);
      fric_mode=&add("Friction Mode"   , MemberDesc(         ).setFunc(FricMode, FricMode)).setEnum(mode_t, Elms(mode_t)).desc("Operation used to calculate the friction with another object");
      boun     =&add("Bounciness"      , MemberDesc(DATA_REAL).setFunc(Boun    , Boun    )).desc("How much should the object bounce from others").range(0, 1);
      boun_mode=&add("Bounciness Mode" , MemberDesc(         ).setFunc(BounMode, BounMode)).setEnum(mode_t, Elms(mode_t)).desc("Operation used to calculate the bounciness with another object");
      damp     =&add("Damping"         , MemberDesc(DATA_REAL).setFunc(Damping , Damping )).desc("Damping reduces object velocity over time").min(0);
      adamp    =&add("Angular Damping" , MemberDesc(DATA_REAL).setFunc(ADamping, ADamping)).desc("Angular Damping reduces object angular velocity over time").min(0);
                 add("Density"         , MemberDesc(DATA_REAL).setFunc(Density , Density )).desc("Density is a multiplier applied to object mass\nValue of zero forces objects to be static (unmovable)").min(0);
                 add();
      df       =&add();
      dfs      =&add();
      dfd      =&add();
      dfb      =&add();
      autoData(this);
      prop_max_x=::PropWin::create("Physics Material Editor", Vec2(0.02f, -0.07f), 0.036f, 0.043f, 0.2f).max.x; ::PropWin::changed(Changed, PreChanged); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=undo  .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      // set text after creating properties so its length will not affect spacing
      df ->name.set(S+"Default Material (Green) :");
      dfs->name.set(S+"Friction Static: " +Physics.mtrl_default.frictionStatic ());
      dfd->name.set(S+"Friction Dynamic: "+Physics.mtrl_default.frictionDynamic());
      dfb->name.set(S+"Bounciness: "      +Physics.mtrl_default.bounciness     ());
      T+=viewport.create(Draw); viewport.fov=PreviewFOV;
      rect(Rect_C(0, 0, Min(1.7f, D.w()*2), Min(1.12f, D.h()*2)));
      cam.setSpherical(Vec(0), 0, 0, 0, 24);
   }
   void PhysMtrlEditor::setMtrl()
   {
      REPAO(actor_dyn).material( null); edit.copyTo(temp); temp.density(1); // set to null in case changing material props would affect all actors
      REPAO(actor_dyn).material(&temp);
   }
   PhysMtrlEditor& PhysMtrlEditor::hide(            )  {set(null); ::PropWin::hide(); return T;}
   Rect            PhysMtrlEditor::sizeLimit(            )C {Rect r=::EE::Window::sizeLimit(); r.min.set(1.0f, 0.70f); return r;}
   PhysMtrlEditor& PhysMtrlEditor::rect(C Rect &rect)  
{
      ::EE::Window::rect(rect);
      viewport.rect(Rect(prop_max_x, -clientHeight(), clientWidth(), 0).extend(-0.02f));
      return T;
   }
   void PhysMtrlEditor::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(visible() && gpc.visible)
      {
         if(Gui.ms   ()==&viewport)if(Ms.b(0) || Ms.b(4)){cam.yaw-=Ms.d().x; cam.pitch+=Ms.d().y; Ms.freeze();}
         if(Gui.wheel()==&viewport)Clamp(cam.dist*=ScaleFactor(Ms.wheel()*-0.2f), 10, 36);
         REPA(Touches)if(Touches[i].guiObj()==&viewport && Touches[i].on()){cam.yaw-=Touches[i].ad().x*2.0f; cam.pitch+=Touches[i].ad().y*2.0f;}

         sim_time+=Time.d();
         if(SIM_TYPE sim=desiredSimType())if(sim!=sim_type)simType(sim);
         if(sim_time>sim_length          )simType(sim_type);
      }
   }
   void PhysMtrlEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmPhysMtrl *data=elm->physMtrlData())data->newVer(); // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(elm->id)); edit.copyTo(*game);
         Save(*game, Proj.gamePath(elm->id)); Proj.savedGame(*elm);
         Server.setElmLong(elm->id);
      }
      changed=false;
   }
   void PhysMtrlEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         edit.copyTo(*game);
         if(ElmPhysMtrl *data=elm->physMtrlData())data->newVer();
         setMtrl();
      }
   }
   void PhysMtrlEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_PHYS_MTRL)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm){edit.load(Proj.editPath(*elm)); game=PhysMtrls(Proj.gamePath(*elm));}else{edit.reset(); game=null;}
         toGui();
         setMtrl();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
         if(!T.elm)simType(SIM_NONE);
      }
   }
   void PhysMtrlEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void PhysMtrlEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void PhysMtrlEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         undos.set(null, true);
         EditPhysMtrl temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp)){edit.copyTo(*game); toGui(); setMtrl();}
      }
   }
   void PhysMtrlEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
PhysMtrlEditor::PhysMtrlEditor() : elm_id(UIDZero), elm(null), changed(false), sim_type(SIM_NONE), prop_max_x(0), sim_time(0), sim_length(3), game(null), fric_stat(null), fric_dyn(null), fric_mode(null), boun(null), boun_mode(null), damp(null), adamp(null), df(null), dfs(null), dfd(null), dfb(null), undos(true) {}

/******************************************************************************/
