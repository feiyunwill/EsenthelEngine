/******************************************************************************/
/******************************************************************************/
class EnvEditor : ClosableWindow
{
   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   class PropWin : ::PropWin
   {
      static void Hide(PropWin &pw);
      PropWin& create(C Str &name, flt value_width);
   };
   class Change : Edit::_Undo::Change
   {
      EditEnv env;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   UID               elm_id;
   Elm              *elm;
   bool              changed;
   EditEnv           edit;
   Environment       temp;
   EnvironmentPtr    game;
   Button            undo, redo, locate;
   PropWin           ambient,  bloom,  clouds,  fog,  sky,  sun;
   CheckBox         cambient, cbloom, cclouds, cfog, csky, csun;
   Button           bambient, bbloom, bclouds, bfog, bsky, bsun;
   Vec2              sun_angle;
   Edit::Undo<Change> undos;   void undoVis();

   static void AmbientColor(  EditEnv &env, C Str &text);
   static Str  AmbientColor(C EditEnv &env             );

   static void NightShadeColor(  EditEnv &env, C Str &text);
   static Str  NightShadeColor(C EditEnv &env             );

   static void BloomOriginal(  EditEnv &env, C Str &text);
   static Str  BloomOriginal(C EditEnv &env             );
   static void BloomScale   (  EditEnv &env, C Str &text);
   static Str  BloomScale   (C EditEnv &env             );
   static void BloomCut     (  EditEnv &env, C Str &text);
   static Str  BloomCut     (C EditEnv &env             );
   static void BloomSaturate(  EditEnv &env, C Str &text);
   static Str  BloomSaturate(C EditEnv &env             );
   static void BloomMaximum (  EditEnv &env, C Str &text);
   static Str  BloomMaximum (C EditEnv &env             );
   static void BloomHalf    (  EditEnv &env, C Str &text);
   static Str  BloomHalf    (C EditEnv &env             );
   static void BloomBlurs   (  EditEnv &env, C Str &text);
   static Str  BloomBlurs   (C EditEnv &env             );

   static void CloudsScaleY (  EditEnv &env, C Str &text);
   static Str  CloudsScaleY (C EditEnv &env             );
   static void CloudsRayMask(  EditEnv &env, C Str &text);
   static Str  CloudsRayMask(C EditEnv &env             );

   template<int i>   static void CloudsColor(  EditEnv &env, C Str &text);
   template<int i>   static Str  CloudsColor(C EditEnv &env             );
   template<int i>   static void CloudsScale(  EditEnv &env, C Str &text);
   template<int i>   static Str  CloudsScale(C EditEnv &env             );
   template<int i>   static void CloudsVel  (  EditEnv &env, C Str &text);
   template<int i>   static Str  CloudsVel  (C EditEnv &env             );
   template<int i>   static void CloudsImage(  EditEnv &env, C Str &text);
   template<int i>   static Str  CloudsImage(C EditEnv &env             );

   static void FogSky    (  EditEnv &env, C Str &text);
   static Str  FogSky    (C EditEnv &env             );
   static void FogDensity(  EditEnv &env, C Str &text);
   static Str  FogDensity(C EditEnv &env             );
   static void FogColor  (  EditEnv &env, C Str &text);
   static Str  FogColor  (C EditEnv &env             );

   static void SkyFrac  (  EditEnv &env, C Str &text);
   static Str  SkyFrac  (C EditEnv &env             );
   static void SkyDnsExp(  EditEnv &env, C Str &text);
   static Str  SkyDnsExp(C EditEnv &env             );
   static void SkyHorExp(  EditEnv &env, C Str &text);
   static Str  SkyHorExp(C EditEnv &env             );
   static void SkyHorCol(  EditEnv &env, C Str &text);
   static Str  SkyHorCol(C EditEnv &env             );
   static void SkySkyCol(  EditEnv &env, C Str &text);
   static Str  SkySkyCol(C EditEnv &env             );
   static void SkyStars (  EditEnv &env, C Str &text);
   static Str  SkyStars (C EditEnv &env             );
   static void SkyBox   (  EditEnv &env, C Str &text);
   static Str  SkyBox   (C EditEnv &env             );

   static void SunBlend (  EditEnv &env, C Str &text);
   static Str  SunBlend (C EditEnv &env             );
   static void SunGlow  (  EditEnv &env, C Str &text);
   static Str  SunGlow  (C EditEnv &env             );
   static void SunSize  (  EditEnv &env, C Str &text);
   static Str  SunSize  (C EditEnv &env             );
   static void SunImage (  EditEnv &env, C Str &text);
   static Str  SunImage (C EditEnv &env             );
   static void SunImageC(  EditEnv &env, C Str &text);
   static Str  SunImageC(C EditEnv &env             );
   static void SunLight (  EditEnv &env, C Str &text);
   static Str  SunLight (C EditEnv &env             );
   static void SunLitF  (  EditEnv &env, C Str &text);
   static Str  SunLitF  (C EditEnv &env             );
   static void SunLitB  (  EditEnv &env, C Str &text);
   static Str  SunLitB  (C EditEnv &env             );
   static void SunRays  (  EditEnv &env, C Str &text);
   static Str  SunRays  (C EditEnv &env             );
   static void SunAngleX(  EditEnv &env, C Str &text);
   static Str  SunAngleX(C EditEnv &env             );
   static void SunAngleY(  EditEnv &env, C Str &text);
   static Str  SunAngleY(C EditEnv &env             );

   static void CAmbient(EnvEditor &ee);
   static void CBloom  (EnvEditor &ee);
   static void CClouds (EnvEditor &ee);
   static void CFog    (EnvEditor &ee);
   static void CSun    (EnvEditor &ee);
   static void CSky    (EnvEditor &ee);

   static void BAmbient(EnvEditor &ee);
   static void BBloom  (EnvEditor &ee);
   static void BClouds (EnvEditor &ee);
   static void BFog    (EnvEditor &ee);
   static void BSun    (EnvEditor &ee);
   static void BSky    (EnvEditor &ee);

   static void Undo  (EnvEditor &editor);
   static void Redo  (EnvEditor &editor);
   static void Locate(EnvEditor &editor);

   Environment* cur();

   Rect getRect(C Rect &rect);
   void toggleWin(GuiObj &obj);
   void setSunAngle(flt x, flt y);

   void create();
   void toGame();
   void toGui ();

   virtual EnvEditor& hide()override;
   virtual EnvEditor& show()override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id); 

public:
   EnvEditor();
};
/******************************************************************************/
/******************************************************************************/
extern EnvEditor EnvEdit;
/******************************************************************************/
