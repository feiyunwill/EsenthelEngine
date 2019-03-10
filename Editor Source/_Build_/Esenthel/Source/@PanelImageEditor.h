/******************************************************************************/
/******************************************************************************/
class PanelImageEditor : PropWin
{
   class Params : EditPanelImage
   {
   };
   class Change : Edit::_Undo::Change
   {
      EditPanelImage data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   UID               elm_id;
   Elm              *elm;
   bool              changed;
   flt               props_w, prop_h, prop_x;
   Params            params;
   PanelImage        panel_image;
   Image             soft, depth;
   Button            undo, redo, locate, copy, paste, draw_depth, draw_lines;
   Region            region;
   Tabs              preview_mode;
   TextLine          filter;
   ViewportSkin      viewport;
   SyncEvent         event;
   Thread            thread;
   SyncLock          lock;
   cptr              force_undo_change_type;
   Edit::Undo<Change> undos;   void undoVis();

  ~PanelImageEditor(); // delete thread before other members

   static void Preview(Viewport &viewport);
          void preview();
   void drawDepth(C Rect &rect);
   static bool Make(Thread &thread);
          bool make();

   static void Undo  (PanelImageEditor &editor);
   static void Redo  (PanelImageEditor &editor);
   static void Locate(PanelImageEditor &editor);
   static void Copy  (PanelImageEditor &editor);
   static void Paste (PanelImageEditor &editor);

                     C Rect& rect(            )C;        // set/get rectangle
   virtual PanelImageEditor& rect(C Rect &rect)override;

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void SetCutLeft  (Params &p, C Str &t);
   static void SetCutRight (Params &p, C Str &t);
   static void SetCutTop   (Params &p, C Str &t);
   static void SetCutBottom(Params &p, C Str &t);

   static void SetCutCorner00(Params &p, C Str &t);
   static void SetCutCorner01(Params &p, C Str &t);
   static void SetCutCorner10(Params &p, C Str &t);
   static void SetCutCorner11(Params &p, C Str &t);

   static void SetCutCornerSlope (Params &p, C Str &t);
   static void SetCutCornerAmount(Params &p, C Str &t);
   
   static void SetForceUniformStretch0(Params &p, C Str &t);
   static void SetForceUniformStretch1(Params &p, C Str &t);

   static void SetCompressed  (Params &p, C Str &t);
   static void SetMipMaps     (Params &p, C Str &t);
   static void SetResolution  (Params &p, C Str &t);
   static void SetWidth       (Params &p, C Str &t);
   static void SetHeight      (Params &p, C Str &t);

   static void SetRoundCorners  (Params &p, C Str &t);
   static void SetLeftSlope     (Params &p, C Str &t);
   static void SetRightSlope    (Params &p, C Str &t);
   static void SetBorderSize    (Params &p, C Str &t);
   static void SetMaxSideStretch(Params &p, C Str &t);
   static void SetExtend        (Params &p, C Str &t);

   static void ExtendInnerPaddLR(  Params &p, C Str &t);
   static Str  ExtendInnerPaddLR(C Params &p          );
   static void ExtendInnerPaddT (  Params &p, C Str &t);
   static Str  ExtendInnerPaddT (C Params &p          );
   static void ExtendInnerPaddB (  Params &p, C Str &t);
   static Str  ExtendInnerPaddB (C Params &p          );

   static void SetColor      (Params &p, C Str &t);
   static void SetColorTop   (Params &p, C Str &t);
   static void SetColorBottom(Params &p, C Str &t);
   static void SetColorLeft  (Params &p, C Str &t);
   static void SetColorRight (Params &p, C Str &t);

   static void SetOuterGlowColor (Params &p, C Str &t);
   static void SetOuterGlowRadius(Params &p, C Str &t);
   static void SetOuterGlowSpread(Params &p, C Str &t);

   static void SetInnerGlowColor (Params &p, C Str &t);
   static void SetInnerGlowRadius(Params &p, C Str &t);

   static void SetDepth        (Params &p, C Str &t);
   static void SetRoundDepth   (Params &p, C Str &t);
   static void SetInnerDistance(Params &p, C Str &t);
   static void SetSmoothDepth  (Params &p, C Str &t);

   static void SetShadowRadius (Params &p, C Str &t);
   static void SetShadowOpacity(Params &p, C Str &t);
   static void SetShadowSpread (Params &p, C Str &t);

   static Str          SetTopImage(C Params &p          );
   static void         SetTopImage(  Params &p, C Str &t);
   static Str       SetBottomImage(C Params &p          );
   static void      SetBottomImage(  Params &p, C Str &t);
   static Str       SetCenterImage(C Params &p          );
   static void      SetCenterImage(  Params &p, C Str &t);
   static Str         SetLeftImage(C Params &p          );
   static void        SetLeftImage(  Params &p, C Str &t);
   static Str        SetRightImage(C Params &p          );
   static void       SetRightImage(  Params &p, C Str &t);
   static Str      SetTopLeftImage(C Params &p          );
   static void     SetTopLeftImage(  Params &p, C Str &t);
   static Str     SetTopRightImage(C Params &p          );
   static void    SetTopRightImage(  Params &p, C Str &t);
   static Str   SetBottomLeftImage(C Params &p          );
   static void  SetBottomLeftImage(  Params &p, C Str &t);
   static Str  SetBottomRightImage(C Params &p          );
   static void SetBottomRightImage(  Params &p, C Str &t);

   static void SetImagesSize       (Params &p, C Str &t);
   static void SetTopHeight        (Params &p, C Str &t);
   static void SetBottomHeight     (Params &p, C Str &t);
   static void SetLeftRightWidth   (Params &p, C Str &t);
   static void SetTopCornerWidth   (Params &p, C Str &t);
   static void SetBottomCornerWidth(Params &p, C Str &t);

   static void SetAmbientLight(Params &p, C Str &t);

   template<int i>   static void LightEnabled             (Params &p, C Str &t);
   template<int i>   static void LightAngle               (Params &p, C Str &t);
   template<int i>   static void LightIntensity           (Params &p, C Str &t);
   template<int i>   static void LightBack                (Params &p, C Str &t);
   template<int i>   static void LightHighlight           (Params &p, C Str &t);
   template<int i>   static void LightHighlightCut        (Params &p, C Str &t);
   template<int i>   static void LightSpecular            (Params &p, C Str &t);
   template<int i>   static void LightSpecularBack        (Params &p, C Str &t);
   template<int i>   static void LightSpecularExp         (Params &p, C Str &t);
   template<int i>   static void LightSpecularHighlight   (Params &p, C Str &t);
   template<int i>   static void LightSpecularHighlightCut(Params &p, C Str &t);

   template<int i>   static void SectionSize     (Params &p, C Str &t);
   template<int i>   static void SectionTopOffset(Params &p, C Str &t);

   template<int i>   static void SectionRoundDepth   (Params &p, C Str &t);
   template<int i>   static void SectionOuterDepth   (Params &p, C Str &t);
   template<int i>   static void SectionInnerDepth   (Params &p, C Str &t);
   template<int i>   static void SectionInnerDistance(Params &p, C Str &t);
   template<int i>   static void SectionSmoothDepth  (Params &p, C Str &t);

   template<int i>   static void SectionSpecular        (Params &p, C Str &t);
   template<int i>   static void SectionColor           (Params &p, C Str &t);
   template<int i>   static void SectionOuterColor      (Params &p, C Str &t);
   template<int i>   static void SectionInnerColor      (Params &p, C Str &t);
   template<int i>   static void SectionColorTop        (Params &p, C Str &t);
   template<int i>   static void SectionColorBottom     (Params &p, C Str &t);
   template<int i>   static void SectionColorLeft       (Params &p, C Str &t);
   template<int i>   static void SectionColorRight      (Params &p, C Str &t);
   template<int i>   static void SectionOuterBorderColor(Params &p, C Str &t);
   template<int i>   static void SectionInnerBorderColor(Params &p, C Str &t);
   template<int i>   static void SectionPrevBorderColor (Params &p, C Str &t);

   template<int i>   static Str  SectionDepthOverlay       (C Params &p          );
   template<int i>   static void SectionDepthOverlay         (Params &p, C Str &t);
   template<int i>   static void SectionDepthOverlayBlur     (Params &p, C Str &t);
   template<int i>   static void SectionDepthOverlayBlurClamp(Params &p, C Str &t);
   template<int i>   static void SectionDepthOverlayUVScale  (Params &p, C Str &t);
   template<int i>   static void SectionDepthOverlayUVOffset (Params &p, C Str &t);
   template<int i>   static void SectionDepthOverlayIntensity(Params &p, C Str &t);
   template<int i>   static void SectionDepthOverlayMode     (Params &p, C Str &t);

   template<int i>   static void SectionDepthNoiseBlur     (Params &p, C Str &t);
   template<int i>   static void SectionDepthNoiseUVScale  (Params &p, C Str &t);
   template<int i>   static void SectionDepthNoiseIntensity(Params &p, C Str &t);
   template<int i>   static void SectionDepthNoiseMode     (Params &p, C Str &t);

   template<int i>   static Str  SectionColorOverlay       (C Params &p          );
   template<int i>   static void SectionColorOverlay         (Params &p, C Str &t);
   template<int i>   static void SectionColorOverlayBlur     (Params &p, C Str &t);
   template<int i>   static void SectionColorOverlayBlurClamp(Params &p, C Str &t);
   template<int i>   static void SectionColorOverlayUVScale  (Params &p, C Str &t);
   template<int i>   static void SectionColorOverlayUVOffset (Params &p, C Str &t);
   template<int i>   static void SectionColorOverlayUVWarp   (Params &p, C Str &t);
   template<int i>   static void SectionColorOverlayIntensity(Params &p, C Str &t);
   template<int i>   static void SectionColorOverlayMode     (Params &p, C Str &t);

   template<int i>   static void SectionColorNoiseBlur     (Params &p, C Str &t);
   template<int i>   static void SectionColorNoiseUVScale  (Params &p, C Str &t);
   template<int i>   static void SectionColorNoiseUVWarp   (Params &p, C Str &t);
   template<int i>   static void SectionColorNoiseIntensity(Params &p, C Str &t);
   template<int i>   static void SectionColorNoiseMode     (Params &p, C Str &t);

   template<int i>   static Str  SectionReflection       (C Params &p          );
   template<int i>   static void SectionReflection         (Params &p, C Str &t);
   template<int i>   static void SectionReflectionIntensity(Params &p, C Str &t);

   static cchar8 *image_modes[]
; ASSERT(PanelImageParams::ImageParams::MULTIPLY==0 && PanelImageParams::ImageParams::SCALE==1 && PanelImageParams::ImageParams::ADD==2 && PanelImageParams::ImageParams::ADD_SIGNED==3 && PanelImageParams::ImageParams::BLEND==4);

   static void Filter(PanelImageEditor &editor);

   void create();
   void toGui(); 

   void stopThread();
   void refresh   ();

   virtual PanelImageEditor& del ()  override;
   virtual PanelImageEditor& hide()  override;
   virtual Rect         sizeLimit()C override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);               
   void toggle  (Elm *elm);               
   void elmChanged(C UID &panel_image_id);
   void erasing(C UID &elm_id);         

public:
   PanelImageEditor();
};
/******************************************************************************/
/******************************************************************************/
extern PanelImageEditor PanelImageEdit;
/******************************************************************************/
