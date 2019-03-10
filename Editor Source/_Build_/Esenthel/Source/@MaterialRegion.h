/******************************************************************************/
/******************************************************************************/
class MaterialRegion : Region
{
   enum TEX_TYPE
   {
      TEX_COLOR   ,
      TEX_ALPHA   ,
      TEX_BUMP    ,
      TEX_NORMAL  ,
      TEX_SPEC    ,
      TEX_GLOW    ,
      TEX_DET_COL ,
      TEX_DET_BUMP,
      TEX_DET_NRM ,
      TEX_MACRO   ,
      TEX_LIGHT   ,
      TEX_RFL_L   ,
      TEX_RFL_F   ,
      TEX_RFL_R   ,
      TEX_RFL_B   ,
      TEX_RFL_D   ,
      TEX_RFL_U   ,
      TEX_RFL_ALL ,
   };

   class Change : Edit::_Undo::Change
   {
      EditMaterial data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };
   class Texture : GuiCustom
   {
      TEX_TYPE        type;
      Str             text;
      Str             file;
      Color           rect_color;
      MemberDesc      md_file, md_time;
      Button          remove;
      MaterialRegion *mr;

      static void Load  (C Str &name, Texture &texture);
      static void Remove(             Texture &texture);

      ImagePtr getImage();
      void setDesc();
      void setFile(Str file);
      void toGui();
      Texture& create(TEX_TYPE type, C MemberDesc &md_file, C MemberDesc &md_time, Rect rect, C Str &text, MaterialRegion &mr);

      virtual void update(C GuiPC &gpc)override;
      bool draw(C Rect &rect);
      virtual void draw(C GuiPC &gpc)override;
      
      WindowIO win_io;

public:
   Texture();
   };

   static cchar8 *auto_reload_name;

   ELM_TYPE          elm_type;
   bool              auto_reload;
   Button            set_mtrl, undo, redo, locate, big, close, reload_base_textures;
   ComboBox          texture_options;
   Tabs              preview_mode;
   ViewportSkin      preview, preview_big;
   Mesh              preview_mesh[4];
   Camera            preview_cam;
   flt               min_zoom, max_zoom, mouse_edit_delta;
   Vec               mouse_edit_value;
   Vec2              light_angle;
   Region            sub;
   Button            brightness;
   Property         *red, *green, *blue, *alpha;
   Memx<Property>    props;
   Memx<Texture>     texs;
   TextBlack         ts;
   Material          temp;
   MaterialPtr       game;
   EditMaterial      edit, saved;
   Image             detail_bump;
   Str               detail_bump_file;
   UID               elm_id;
   Elm              *elm;
   bool              changed;
   Edit::Undo<Change> undos;   void undoVis();

   Vec previewLight()C;

   static void Render();
          void render();
   static  void DrawPreview(Viewport &viewport);
   virtual void drawPreview();

   static void PreChanged(C Property &prop); // set all RGB props to have the same change_type so they will not create too manu undos
   static void    Changed(C Property &prop);

   static Str  Tech(C MaterialRegion &mr          );
   static void Tech(  MaterialRegion &mr, C Str &t);

   static cchar8 *DownsizeTexMobileText[]
; ASSERT(MaxMaterialDownsize==3);
   static Str  DownsizeTexMobile(C MaterialRegion &mr          );
   static void DownsizeTexMobile(  MaterialRegion &mr, C Str &t);

   static cchar8 *TexQualityiOSText[]
;
   static Str  TexQualityiOS(C MaterialRegion &mr          );
   static void TexQualityiOS(  MaterialRegion &mr, C Str &t);

   /*static .MaxTexSize max_tex_sizes[]=
   {
      {Edit.MTS_128      ,       "128", "Textures will be resized to 128 if they are bigger"},
      {Edit.MTS_256      ,       "256", "Textures will be resized to 256 if they are bigger"},
      {Edit.MTS_512      ,       "512", "Textures will be resized to 512 if they are bigger"},
      {Edit.MTS_1024     ,      "1024", "Textures will be resized to 1024 if they are bigger"},
      {Edit.MTS_2048     ,      "2048", "Textures will be resized to 2048 if they are bigger"},
      {Edit.MTS_UNLIMITED, "Unlimited", "Textures won't be resized"},
      {Edit.MTS_PUBLISH_SETTINGS, "Use Publish Settings", "Texture size limit will be taken from the value specified in Project Publishing Settings"},
   };
   static Str  MaxTexSize(C MaterialRegion &mr          ) {REPA(max_tex_sizes)if(max_tex_sizes[i].mts==mr.edit.max_tex_size)return i; return S;}
   static void MaxTexSize(  MaterialRegion &mr, C Str &t) {int i=TextInt(t); if(InRange(i, max_tex_sizes)){mr.edit.max_tex_size=max_tex_sizes[i].mts; mr.edit.max_tex_size_time.now();}}*/

   static void RGB(MaterialRegion &mr);

   static Str  Red  (C MaterialRegion &mr          );
   static void Red  (  MaterialRegion &mr, C Str &t);
   static Str  Green(C MaterialRegion &mr          );
   static void Green(  MaterialRegion &mr, C Str &t);
   static Str  Blue (C MaterialRegion &mr          );
   static void Blue (  MaterialRegion &mr, C Str &t);
   static Str  Alpha(C MaterialRegion &mr          );
   static void Alpha(  MaterialRegion &mr, C Str &t);

   static const flt BumpScale;
   static Str  Bump    (C MaterialRegion &mr          );
   static void Bump    (  MaterialRegion &mr, C Str &t);
   static Str  NrmScale(C MaterialRegion &mr          );
   static void NrmScale(  MaterialRegion &mr, C Str &t);
   static Str  FNY     (C MaterialRegion &mr          );
   static void FNY     (  MaterialRegion &mr, C Str &t);

   static Str  Spec(C MaterialRegion &mr          );
   static void Spec(  MaterialRegion &mr, C Str &t);
   static Str  Glow(C MaterialRegion &mr          );
   static void Glow(  MaterialRegion &mr, C Str &t);

   static Str  DetScale(C MaterialRegion &mr          );
   static void DetScale(  MaterialRegion &mr, C Str &t);
   static Str  DetPower(C MaterialRegion &mr          );
   static void DetPower(  MaterialRegion &mr, C Str &t);

   static Str  Reflect(C MaterialRegion &mr          );
   static void Reflect(  MaterialRegion &mr, C Str &t);

   static Str  Cull    (C MaterialRegion &mr          );
   static void Cull    (  MaterialRegion &mr, C Str &t);
   static Str  SSS     (C MaterialRegion &mr          );
   static void SSS     (  MaterialRegion &mr, C Str &t);
   static Str  AmbR    (C MaterialRegion &mr          );
   static void AmbR    (  MaterialRegion &mr, C Str &t);
   static Str  AmbG    (C MaterialRegion &mr          );
   static void AmbG    (  MaterialRegion &mr, C Str &t);
   static Str  AmbB    (C MaterialRegion &mr          );
   static void AmbB    (  MaterialRegion &mr, C Str &t);
   static Str  TexScale(C MaterialRegion &mr          );
   static void TexScale(  MaterialRegion &mr, C Str &t);

   static void Undo  (MaterialRegion &editor);
   static void Redo  (MaterialRegion &editor);
   static void Locate(MaterialRegion &editor);

   static void Hide   (MaterialRegion &editor);
   static void SetMtrl(MaterialRegion &editor);

   static void AutoReload        (MaterialRegion &editor);
   static void ReloadBaseTextures(MaterialRegion &editor);

   static void ResizeBase128 (MaterialRegion &editor);
   static void ResizeBase256 (MaterialRegion &editor);
   static void ResizeBase512 (MaterialRegion &editor);
   static void ResizeBase1024(MaterialRegion &editor);
   static void ResizeBase2048(MaterialRegion &editor);
   static void ResizeBase4096(MaterialRegion &editor);

   static void ResizeBase128x64   (MaterialRegion &editor);
   static void ResizeBase256x128  (MaterialRegion &editor);
   static void ResizeBase512x256  (MaterialRegion &editor);
   static void ResizeBase1024x512 (MaterialRegion &editor);
   static void ResizeBase2048x1024(MaterialRegion &editor);

   static void ResizeBase64x128   (MaterialRegion &editor);
   static void ResizeBase128x256  (MaterialRegion &editor);
   static void ResizeBase256x512  (MaterialRegion &editor);
   static void ResizeBase512x1024 (MaterialRegion &editor);
   static void ResizeBase1024x2048(MaterialRegion &editor);

   static void ResizeBaseQuarter (MaterialRegion &editor);
   static void ResizeBaseHalf    (MaterialRegion &editor);
   static void ResizeBaseOriginal(MaterialRegion &editor);
   static void ResizeBaseDouble  (MaterialRegion &editor);

   static void ResizeBase0_128 (MaterialRegion &editor);
   static void ResizeBase0_256 (MaterialRegion &editor);
   static void ResizeBase0_512 (MaterialRegion &editor);
   static void ResizeBase0_1024(MaterialRegion &editor);
   static void ResizeBase0_2048(MaterialRegion &editor);
   static void ResizeBase0_4096(MaterialRegion &editor);

   static void ResizeBase0_128x64   (MaterialRegion &editor);
   static void ResizeBase0_256x128  (MaterialRegion &editor);
   static void ResizeBase0_512x256  (MaterialRegion &editor);
   static void ResizeBase0_1024x512 (MaterialRegion &editor);
   static void ResizeBase0_2048x1024(MaterialRegion &editor);

   static void ResizeBase0_64x128   (MaterialRegion &editor);
   static void ResizeBase0_128x256  (MaterialRegion &editor);
   static void ResizeBase0_256x512  (MaterialRegion &editor);
   static void ResizeBase0_512x1024 (MaterialRegion &editor);
   static void ResizeBase0_1024x2048(MaterialRegion &editor);

   static void ResizeBase0_Quarter (MaterialRegion &editor);
   static void ResizeBase0_Half    (MaterialRegion &editor);
   static void ResizeBase0_Original(MaterialRegion &editor);
   static void ResizeBase0_Double  (MaterialRegion &editor);

   static void ResizeBase1_128 (MaterialRegion &editor);
   static void ResizeBase1_256 (MaterialRegion &editor);
   static void ResizeBase1_512 (MaterialRegion &editor);
   static void ResizeBase1_1024(MaterialRegion &editor);
   static void ResizeBase1_2048(MaterialRegion &editor);
   static void ResizeBase1_4096(MaterialRegion &editor);

   static void ResizeBase1_128x64   (MaterialRegion &editor);
   static void ResizeBase1_256x128  (MaterialRegion &editor);
   static void ResizeBase1_512x256  (MaterialRegion &editor);
   static void ResizeBase1_1024x512 (MaterialRegion &editor);
   static void ResizeBase1_2048x1024(MaterialRegion &editor);

   static void ResizeBase1_64x128   (MaterialRegion &editor);
   static void ResizeBase1_128x256  (MaterialRegion &editor);
   static void ResizeBase1_256x512  (MaterialRegion &editor);
   static void ResizeBase1_512x1024 (MaterialRegion &editor);
   static void ResizeBase1_1024x2048(MaterialRegion &editor);

   static void ResizeBase1_Quarter (MaterialRegion &editor);
   static void ResizeBase1_Half    (MaterialRegion &editor);
   static void ResizeBase1_Original(MaterialRegion &editor);
   static void ResizeBase1_Double  (MaterialRegion &editor);
   
   static void BumpFromCol  (MaterialRegion &editor);
   static void BumpFromCol2 (MaterialRegion &editor);
   static void BumpFromCol3 (MaterialRegion &editor);
   static void BumpFromCol4 (MaterialRegion &editor);
   static void BumpFromCol5 (MaterialRegion &editor);
   static void BumpFromCol6 (MaterialRegion &editor);
   static void BumpFromCol8 (MaterialRegion &editor);
   static void BumpFromCol12(MaterialRegion &editor);
   static void BumpFromCol16(MaterialRegion &editor);
   static void BumpFromCol24(MaterialRegion &editor);
   static void BumpFromCol32(MaterialRegion &editor);

   static void MulTexCol  (MaterialRegion &editor);
   static void MulTexRough(MaterialRegion &editor);

   bool bigVisible()C;

   void   setRGB         (C Vec            &rgb);
   void resetAlpha       (                     );
   void cull             (bool              on );
   void flipNrmY         (bool              on ); // 'rebuildBase' already calls 'setChanged' and 'toGui'
 //void maxTexSize       (Edit.MAX_TEX_SIZE mts) {if(edit.max_tex_size       !=mts){undos.set("mts"  ); edit.max_tex_size       =mts; edit.       max_tex_size_time.getUTC(); setChanged(); toGui();}}
   void downsizeTexMobile(byte              ds );
   void texQualityiOS    (bool              q  );

   void resizeBase(C VecI2 &size, bool relative=false);
   void resizeBase0(C VecI2 &size, bool relative=false);
   void resizeBase1(C VecI2 &size, bool relative=false);
   void bumpFromCol(int blur);

   virtual   EditMaterial& getEditMtrl  ();
   virtual C ImagePtr    & getBase0     ();
   virtual C ImagePtr    & getBase1     ();
   virtual C ImagePtr    & getDetail    ();
   virtual C ImagePtr    & getReflection();
   virtual C ImagePtr    & getMacro     ();
   virtual C ImagePtr    & getLight     ();

   void setBottom(C Rect &prop_rect);
   void create();

   // operations
   Image* getDetailBump(C Str &file);
   void resize();
   void toGui();
           void flush(C UID &elm_id);
   virtual void flush();
   virtual void setChanged();
   virtual void set(Elm *elm);
   virtual void activate(Elm *elm);
   virtual void toggle(Elm *elm);
           void            hideBig();        
   virtual MaterialRegion& show   ()override;
   virtual MaterialRegion& hide   ()override;

   void set(C MaterialPtr &mtrl);    
   void set(Memt<MaterialPtr> mtrls);
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
   class ImageSource : Edit::FileParams
   {
      int order;

public:
   ImageSource();
   };
   static int Compare(C ImageSource &a, C ImageSource &b);            
   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos);

   virtual void rebuildBase(uint old_base_tex, bool changed_flip_normal_y=false, bool adjust_params=true, bool always=false);
   virtual void rebuildReflection();
   virtual void rebuildDetail();
   virtual void rebuildMacro();
   virtual void rebuildLight();

   virtual void elmChanged(C UID&mtrl_id);
   void erasing(C UID &elm_id);         

   bool winIOContains(GuiObj *go)C;

   // update
   virtual void update(C GuiPC &gpc)override;

public:
   MaterialRegion();
};
/******************************************************************************/
/******************************************************************************/
extern MaterialRegion MtrlEdit;
/******************************************************************************/
