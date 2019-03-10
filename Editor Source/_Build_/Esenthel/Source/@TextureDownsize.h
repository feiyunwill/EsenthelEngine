/******************************************************************************/
/******************************************************************************/
class TextureDownsize : Viewport4Region
{
   enum MODE
   {
      MATERIAL,
      OBJECT  ,
      MODE_NUM,
   };
   static cchar8 *ModeName[]
;
   class Settings
   {
      int global,
          base_0,
          base_1;

      int base0()C;
      int base1()C;

public:
   Settings();
   };
   class DownsizedMtrl : Material
   {
      MaterialPtr original;

      static bool Create(DownsizedMtrl &mtrl, C UID &original_id, ptr user);
             void create(                     C UID &original_id          );
      void setTextures();
      void update();
   };

   Menu                    menu;
   Tabs                    mode;
   GuiImage                mtrl_image;
   Region                  region;
   Button                  light_dir, downsize[3][2], revert, apply, prev, next;
   Text                    info[3][2];
   Memx<Property>          props;
   TextBlack               prop_ts, left_ts, right_ts;
   Map<UID, Settings>      settings; // key=Material ID, doesn't need to be thread-safe
   Elm                    *elm; // can be OBJ or MTRL
   UID                     mtrl_id,
                            obj_id;
   Memc<UID>               secondary;
   int                     secondary_i;
   UID                     normal_base[2];
   MaterialPtr             normal_mtrl;
   MeshPtr                 normal_mesh;
   Image                   downsized_base[2];
   Map<UID, DownsizedMtrl> downsized_mtrls; // key=Material ID, doesn't need to be thread-safe
   MeshLod                 downsized_mesh;
   bool                    draw_as_obj, different;
   Vec2                    light_angle;

   Vec previewLight()C;

   static int MaterialUses(C MeshLod &lod, C MaterialPtr &mtrl, int variation); // how many 'lod' parts use 'mtrl' in 'variation'
   static int VariationWithMostMaterialUses(C Mesh &mesh, C MaterialPtr &mtrl);

   static void Render();
          void render();

   static void Draw(Viewport &viewport);        
          void draw(Edit::Viewport4::View&view);

   static void ModeChanged(TextureDownsize &editor);
   static void LightMode  (TextureDownsize &editor);
   
   static void Mode0(TextureDownsize &td);
   static void Mode1(TextureDownsize &td);

   static Str  Global(C TextureDownsize &td             );
   static void Global(  TextureDownsize &td, C Str &text);

   static Str  Base0(C TextureDownsize &td             );
   static void Base0(  TextureDownsize &td, C Str &text);

   static Str  Base1(C TextureDownsize &td             );
   static void Base1(  TextureDownsize &td, C Str &text);

   static void Prev(TextureDownsize &editor);
   static void Next(TextureDownsize &editor);

   static void PrevS(TextureDownsize &editor);
   static void NextS(TextureDownsize &editor);

   void nextSecondary(int dir);
   bool nextSecondary(int dir, C UID (&old_base)[2]);
   bool Next(int dir=1); // find next compatible element

   static cchar8 *DialogID;

   void closeConfirm();

   static bool AdjustTexSrc(Str &src, TimeStamp &src_time, C Str &original, C Str &downsized, C TimeStamp &time);
   static VecI2 ImageSize(C ImagePtr &image, int down);
   static void ApplyConfirm(TextureDownsize &editor);
          void applyConfirm(bool revert=false);

   static void Revert(TextureDownsize &editor);

   static void Apply(TextureDownsize &editor);
          void applyDo();

   static void Dec(Property &prop);
   static void Inc(Property &prop);

   Settings* curSettings(); 
 C Settings* curSettings()C;

                    C Rect& rect(            );        
   virtual TextureDownsize& rect(C Rect &rect)override;
   int getPart(GuiObj *go, C Vec2 &screen_pos, Vec *hit_pos=null);
   bool selectionZoom(flt &dist);
   virtual void camCenter(bool zoom)override;
   bool selected       ()C;        
   void selectedChanged();
   void toggle();
   void close();
   void clearProj();
   TextureDownsize& create();
   void toGui();           
   void setShader();       
   void setDrawAs();

   static Str ImageInfo(C Image *image, int &size, bool side);

   static void SetTextures(C Property &prop);
          void setTextures();
   void setTexIDs();
   void setMtrl();
   void setObj();
   void refresh();                     
   void setMtrlObj(C UID &mtrl_id, C UID &obj_id);
   void getSecondaryList();
   void setSecondary(int i);
   void setElm(Elm *elm, bool prev=false);
   virtual void update(C GuiPC &gpc)override;

public:
   TextureDownsize();
};
/******************************************************************************/
/******************************************************************************/
extern TextureDownsize TexDownsize;
/******************************************************************************/
