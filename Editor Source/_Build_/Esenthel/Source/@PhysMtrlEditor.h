/******************************************************************************/
/******************************************************************************/
class PhysMtrlEditor : PropWin
{
   class Change : Edit::_Undo::Change
   {
      EditPhysMtrl data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   static cchar8 *mode_t[]
; ASSERT(PhysMtrl::MODE_AVG==0 && PhysMtrl::MODE_MUL==1 && PhysMtrl::MODE_MIN==2 && PhysMtrl::MODE_MAX==3);

   enum SIM_TYPE
   {
      SIM_NONE    ,
      SIM_FRIC    ,
      SIM_BOUNCE  ,
      SIM_DAMPING ,
      SIM_ADAMPING,
   };

   UID          elm_id;
   Elm         *elm;
   bool         changed;
   Button       undo, redo, locate;
   ViewportSkin viewport;
   SIM_TYPE     sim_type;
   flt          prop_max_x, sim_time, sim_length;
   Camera       cam;
   EditPhysMtrl edit;
   PhysMtrl    *game, temp; // temp has everything except density
   Memc<Actor>  actor_dyn, actor_stat;
   Property    *fric_stat, *fric_dyn, *fric_mode, *boun, *boun_mode, *damp, *adamp, *df, *dfs, *dfd, *dfb;
   Edit::Undo<Change> undos;   void undoVis();                             

   static void Draw(Viewport &viewport);
          void draw();

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void FricStat(  PhysMtrlEditor &me, C Str &t);
   static Str  FricStat(C PhysMtrlEditor &me          );
   static void FricDyn (  PhysMtrlEditor &me, C Str &t);
   static Str  FricDyn (C PhysMtrlEditor &me          );
   static void FricMode(  PhysMtrlEditor &me, C Str &t);
   static Str  FricMode(C PhysMtrlEditor &me          );
   static void Boun    (  PhysMtrlEditor &me, C Str &t);
   static Str  Boun    (C PhysMtrlEditor &me          );
   static void BounMode(  PhysMtrlEditor &me, C Str &t);
   static Str  BounMode(C PhysMtrlEditor &me          );
   static void Density (  PhysMtrlEditor &me, C Str &t);
   static Str  Density (C PhysMtrlEditor &me          );
   static void Damping (  PhysMtrlEditor &me, C Str &t);
   static Str  Damping (C PhysMtrlEditor &me          );
   static void ADamping(  PhysMtrlEditor &me, C Str &t);
   static Str  ADamping(C PhysMtrlEditor &me          );

   static void Undo  (PhysMtrlEditor &editor);
   static void Redo  (PhysMtrlEditor &editor);
   static void Locate(PhysMtrlEditor &editor);

   ElmPhysMtrl* data()C;

   void simType(SIM_TYPE sim_type);
   SIM_TYPE desiredSimType()C;

   void create();
   void setMtrl();

   virtual PhysMtrlEditor& hide     (            )  override;
   virtual Rect            sizeLimit(            )C override;
   virtual PhysMtrlEditor& rect     (C Rect &rect)  override;
   virtual void update(C GuiPC &gpc)override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void toggle(Elm *elm);         
   void activate(Elm *elm);       
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id); 

public:
   PhysMtrlEditor();
};
/******************************************************************************/
/******************************************************************************/
extern PhysMtrlEditor PhysMtrlEdit;
/******************************************************************************/
