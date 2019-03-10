/******************************************************************************/
class PhysMtrlEditor : PropWin
{
   class Change : Edit._Undo.Change
   {
      EditPhysMtrl data;

      virtual void create(ptr user)override
      {
         data=PhysMtrlEdit.edit;
         PhysMtrlEdit.undoVis();
      }
      virtual void apply(ptr user)override
      {
         PhysMtrlEdit.edit.undo(data);
         PhysMtrlEdit.setChanged();
         PhysMtrlEdit.toGui();
         PhysMtrlEdit.undoVis();
      }
   }

   static cchar8 *mode_t[]=
   {
      "Average" , // 0
      "Multiply", // 1
      "Minimum" , // 2
      "Maximum" , // 3
   }; ASSERT(PhysMtrl.MODE_AVG==0 && PhysMtrl.MODE_MUL==1 && PhysMtrl.MODE_MIN==2 && PhysMtrl.MODE_MAX==3);

   enum SIM_TYPE
   {
      SIM_NONE    ,
      SIM_FRIC    ,
      SIM_BOUNCE  ,
      SIM_DAMPING ,
      SIM_ADAMPING,
   }

   UID          elm_id=UIDZero;
   Elm         *elm=null;
   bool         changed=false;
   Button       undo, redo, locate;
   ViewportSkin viewport;
   SIM_TYPE     sim_type=SIM_NONE;
   flt          prop_max_x=0, sim_time=0, sim_length=3;
   Camera       cam;
   EditPhysMtrl edit;
   PhysMtrl    *game=null, temp; // temp has everything except density
   Memc<Actor>  actor_dyn, actor_stat;
   Property    *fric_stat=null, *fric_dyn=null, *fric_mode=null, *boun=null, *boun_mode=null, *damp=null, *adamp=null, *df=null, *dfs=null, *dfd=null, *dfb=null;
   Edit.Undo<Change> undos(true);   void undoVis() {SetUndo(undos, undo, redo);}

   static void Draw(Viewport &viewport) {PhysMtrlEdit.draw();}
          void draw()
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
         Rect_D(pos, 0.025, 0.04).draw(ColorAlpha(BLACK, 0.5));
      }
   }

   static void PreChanged(C Property &prop) {PhysMtrlEdit.undos.set(&prop);}
   static void    Changed(C Property &prop) {PhysMtrlEdit.setChanged();}

   static void FricStat(  PhysMtrlEditor &me, C Str &t) {me.edit.friction_static=TextFlt(t); me.edit.friction_static_time.getUTC();}
   static Str  FricStat(C PhysMtrlEditor &me          ) {return me.edit.friction_static;}
   static void FricDyn (  PhysMtrlEditor &me, C Str &t) {me.edit.friction_dynamic=TextFlt(t); me.edit.friction_dynamic_time.getUTC();}
   static Str  FricDyn (C PhysMtrlEditor &me          ) {return me.edit.friction_dynamic;}
   static void FricMode(  PhysMtrlEditor &me, C Str &t) {me.edit.friction_mode=PhysMtrl.MODE(TextInt(t)); me.edit.friction_mode_time.getUTC();}
   static Str  FricMode(C PhysMtrlEditor &me          ) {return me.edit.friction_mode;}
   static void Boun    (  PhysMtrlEditor &me, C Str &t) {me.edit.bounciness=TextFlt(t); me.edit.bounciness_time.getUTC();}
   static Str  Boun    (C PhysMtrlEditor &me          ) {return me.edit.bounciness;}
   static void BounMode(  PhysMtrlEditor &me, C Str &t) {me.edit.bounciness_mode=PhysMtrl.MODE(TextInt(t)); me.edit.bounciness_mode_time.getUTC();}
   static Str  BounMode(C PhysMtrlEditor &me          ) {return me.edit.bounciness_mode;}
   static void Density (  PhysMtrlEditor &me, C Str &t) {me.edit.density=TextFlt(t); me.edit.density_time.getUTC();}
   static Str  Density (C PhysMtrlEditor &me          ) {return me.edit.density;}
   static void Damping (  PhysMtrlEditor &me, C Str &t) {me.edit.damping=TextFlt(t); me.edit.damping_time.getUTC();}
   static Str  Damping (C PhysMtrlEditor &me          ) {return me.edit.damping;}
   static void ADamping(  PhysMtrlEditor &me, C Str &t) {me.edit.adamping=TextFlt(t); me.edit.adamping_time.getUTC();}
   static Str  ADamping(C PhysMtrlEditor &me          ) {return me.edit.adamping;}

   static void Undo  (PhysMtrlEditor &editor) {editor.undos.undo();}
   static void Redo  (PhysMtrlEditor &editor) {editor.undos.redo();}
   static void Locate(PhysMtrlEditor &editor) {Proj.elmLocate(editor.elm_id);}

   ElmPhysMtrl* data()C {return elm ? elm.physMtrlData() : null;}

   void simType(SIM_TYPE sim_type)
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
               Matrix m; m.setRotateX(i*-0.19); m.move(Vec((i-3)*2, 0, 0));
               actor_stat.New().create(OBox(Box(1.1, 1, 10, Vec(0, -0.5, -4.5)), m), 0);
               actor_dyn .New().create(OBox(Box(  1, 1,  1, Vec(0,  0.5,  0  )), m));
            }
         }break;
         
         case SIM_BOUNCE:
         {
            actor_stat.New().create(Box(10, 1, 10, Vec(0, -3, 0)), 0);
            actor_dyn .New().create(Ball(0.5, Vec(0, 3, 0)));
         }break;

         case SIM_DAMPING:
         {
            actor_dyn.New().create(Ball(0.4, Vec(-12, 0, 0))).vel(Vec(18, 10, 0));
         }break;

         case SIM_ADAMPING:
         {
            actor_dyn.New().create(Box(4, Vec(0, 0, 0))).gravity(false).maxAngVel(100).angVel(Vec(7, 7, 0));
         }break;
      }
      setMtrl();
   }
   SIM_TYPE desiredSimType()C
   {
      if(fric_stat && fric_stat.contains(Gui.ms())
      || fric_dyn  && fric_dyn .contains(Gui.ms())
      || fric_mode && fric_mode.contains(Gui.ms()))return SIM_FRIC;
      if(boun      && boun     .contains(Gui.ms())
      || boun_mode && boun_mode.contains(Gui.ms()))return SIM_BOUNCE;
      if(damp      && damp     .contains(Gui.ms()))return SIM_DAMPING;
      if(adamp     && adamp    .contains(Gui.ms()))return SIM_ADAMPING;
      return SIM_NONE;
   }

   void create()
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
      prop_max_x=super.create("Physics Material Editor", Vec2(0.02, -0.07), 0.036, 0.043, 0.2).max.x; super.changed(Changed, PreChanged); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=undo  .create(Rect_LU(0.02, -0.01     , 0.05, 0.05)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo.rect().ru(), 0.05, 0.05)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01, 0), 0.14, 0.05), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      // set text after creating properties so its length will not affect spacing
      df .name.set(S+"Default Material (Green) :");
      dfs.name.set(S+"Friction Static: " +Physics.mtrl_default.frictionStatic ());
      dfd.name.set(S+"Friction Dynamic: "+Physics.mtrl_default.frictionDynamic());
      dfb.name.set(S+"Bounciness: "      +Physics.mtrl_default.bounciness     ());
      T+=viewport.create(Draw); viewport.fov=PreviewFOV;
      rect(Rect_C(0, 0, Min(1.7, D.w()*2), Min(1.12, D.h()*2)));
      cam.setSpherical(Vec(0), 0, 0, 0, 24);
   }
   void setMtrl()
   {
      REPAO(actor_dyn).material( null); edit.copyTo(temp); temp.density(1); // set to null in case changing material props would affect all actors
      REPAO(actor_dyn).material(&temp);
   }

   virtual PhysMtrlEditor& hide     (            )  override {set(null); super.hide(); return T;}
   virtual Rect            sizeLimit(            )C override {Rect r=super.sizeLimit(); r.min.set(1.0, 0.70); return r;}
   virtual PhysMtrlEditor& rect     (C Rect &rect)  override
   {
      super   .rect(rect);
      viewport.rect(Rect(prop_max_x, -clientHeight(), clientWidth(), 0).extend(-0.02));
      return T;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         if(Gui.ms   ()==&viewport)if(Ms.b(0) || Ms.b(4)){cam.yaw-=Ms.d().x; cam.pitch+=Ms.d().y; Ms.freeze();}
         if(Gui.wheel()==&viewport)Clamp(cam.dist*=ScaleFactor(Ms.wheel()*-0.2), 10, 36);
         REPA(Touches)if(Touches[i].guiObj()==&viewport && Touches[i].on()){cam.yaw-=Touches[i].ad().x*2.0; cam.pitch+=Touches[i].ad().y*2.0;}

         sim_time+=Time.d();
         if(SIM_TYPE sim=desiredSimType())if(sim!=sim_type)simType(sim);
         if(sim_time>sim_length          )simType(sim_type);
      }
   }

   void flush()
   {
      if(elm && changed)
      {
         if(ElmPhysMtrl *data=elm.physMtrlData())data.newVer(); // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(elm.id)); edit.copyTo(*game);
         Save(*game, Proj.gamePath(elm.id)); Proj.savedGame(*elm);
         Server.setElmLong(elm.id);
      }
      changed=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         edit.copyTo(*game);
         if(ElmPhysMtrl *data=elm.physMtrlData())data.newVer();
         setMtrl();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_PHYS_MTRL)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm){edit.load(Proj.editPath(*elm)); game=PhysMtrls(Proj.gamePath(*elm));}else{edit.reset(); game=null;}
         toGui();
         setMtrl();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
         if(!T.elm)simType(SIM_NONE);
      }
   }
   void toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         undos.set(null, true);
         EditPhysMtrl temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp)){edit.copyTo(*game); toGui(); setMtrl();}
      }
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
}
PhysMtrlEditor PhysMtrlEdit;
/******************************************************************************/
