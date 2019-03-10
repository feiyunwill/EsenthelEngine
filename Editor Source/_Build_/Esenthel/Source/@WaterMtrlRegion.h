/******************************************************************************/
/******************************************************************************/
class WaterMtrlRegion : MaterialRegion
{
   class Change : MaterialRegion::Change
   {
      EditWaterMtrl data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   WaterMtrl     temp;
   WaterMtrlPtr  game;
   EditWaterMtrl edit;

   static void Render();
          void render();
   virtual void drawPreview()override;

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static Str  Density              (C WaterMtrlRegion &mr          );
   static void Density              (  WaterMtrlRegion &mr, C Str &t);
   static Str  DensityAdd           (C WaterMtrlRegion &mr          );
   static void DensityAdd           (  WaterMtrlRegion &mr, C Str &t);
   static Str  DensityUnderwater    (C WaterMtrlRegion &mr          );
   static void DensityUnderwater    (  WaterMtrlRegion &mr, C Str &t);
   static Str  DensityUnderwaterAdd (C WaterMtrlRegion &mr          );
   static void DensityUnderwaterAdd (  WaterMtrlRegion &mr, C Str &t);
   static Str  ScaleColor           (C WaterMtrlRegion &mr          );
   static void ScaleColor           (  WaterMtrlRegion &mr, C Str &t);
   static Str  ScaleNormal          (C WaterMtrlRegion &mr          );
   static void ScaleNormal          (  WaterMtrlRegion &mr, C Str &t);
   static Str  ScaleBump            (C WaterMtrlRegion &mr          );
   static void ScaleBump            (  WaterMtrlRegion &mr, C Str &t);
   static Str  NrmScale             (C WaterMtrlRegion &mr          );
   static void NrmScale             (  WaterMtrlRegion &mr, C Str &t);
   static Str  ReflectTex           (C WaterMtrlRegion &mr          );
   static void ReflectTex           (  WaterMtrlRegion &mr, C Str &t);
   static Str  ReflectWorld         (C WaterMtrlRegion &mr          );
   static void ReflectWorld         (  WaterMtrlRegion &mr, C Str &t);
   static Str  Refract              (C WaterMtrlRegion &mr          );
   static void Refract              (  WaterMtrlRegion &mr, C Str &t);
   static Str  RefractReflection    (C WaterMtrlRegion &mr          );
   static void RefractReflection    (  WaterMtrlRegion &mr, C Str &t);
   static Str  RefractUnderwater    (C WaterMtrlRegion &mr          );
   static void RefractUnderwater    (  WaterMtrlRegion &mr, C Str &t);
   static Str  Specular             (C WaterMtrlRegion &mr          );
   static void Specular             (  WaterMtrlRegion &mr, C Str &t);
   static Str  WaveScale            (C WaterMtrlRegion &mr          );
   static void WaveScale            (  WaterMtrlRegion &mr, C Str &t);
   static Str  FresnelPow           (C WaterMtrlRegion &mr          );
   static void FresnelPow           (  WaterMtrlRegion &mr, C Str &t);
   static Str  FresnelRough         (C WaterMtrlRegion &mr          );
   static void FresnelRough         (  WaterMtrlRegion &mr, C Str &t);
   static Str  FresnelColor         (C WaterMtrlRegion &mr          );
   static void FresnelColor         (  WaterMtrlRegion &mr, C Str &t);
   static Str  Col                  (C WaterMtrlRegion &mr          );
   static void Col                  (  WaterMtrlRegion &mr, C Str &t);
   static Str  ColorUnderwater0     (C WaterMtrlRegion &mr          );
   static void ColorUnderwater0     (  WaterMtrlRegion &mr, C Str &t);
   static Str  ColorUnderwater1     (C WaterMtrlRegion &mr          );
   static void ColorUnderwater1     (  WaterMtrlRegion &mr, C Str &t);

   static Str  FNY (C WaterMtrlRegion &mr          );
   static void FNY (  WaterMtrlRegion &mr, C Str &t);

   virtual   EditMaterial& getEditMtrl  ()override;
   virtual C ImagePtr    & getBase0     ()override;
   virtual C ImagePtr    & getBase1     ()override;
 //virtual C ImagePtr    & getDetail    ()override {return game->   detail_map  ;}
   virtual C ImagePtr    & getReflection()override;
 //virtual C ImagePtr    & getMacro     ()override {return game->    macro_map  ;}
 //virtual C ImagePtr    & getLight     ()override {return game->    light_map  ;}

   void create();

   // operations
   virtual void flush()override;
   virtual void setChanged()override;
   virtual void set(Elm *elm)override;

   void set(C WaterMtrlPtr &mtrl);

   virtual void rebuildBase(uint old_base_tex, bool changed_flip_normal_y=false, bool adjust_params=true, bool always=false)override;
   virtual void rebuildReflection()override;
   virtual void rebuildDetail()override;
   virtual void rebuildMacro()override;
   virtual void rebuildLight()override;

   virtual void elmChanged(C UID &mtrl_id)override;

public:
   WaterMtrlRegion();
};
/******************************************************************************/
/******************************************************************************/
extern WaterMtrlRegion WaterMtrlEdit;
/******************************************************************************/
