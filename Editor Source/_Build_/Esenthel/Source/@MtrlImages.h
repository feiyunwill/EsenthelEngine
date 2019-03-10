/******************************************************************************/
/******************************************************************************/
class MtrlImages
{
   bool  flip_normal_y;
   int   tex;
   Image color, alpha, bump, normal, specular, glow;
   
   bool create(C VecI2 &size);
   void clear();
   void compact();
   void Export(C Str &name, C Str &ext)C;
   static void Crop(Image &image, C Rect &frac);
   void crop(C Rect &frac);
   void resize(C VecI2 &size);
   void fromMaterial(C EditMaterial &material, C Project &proj, bool changed_flip_normal_y, C VecI2 &size=-1, bool process_alpha=false);
   uint createBaseTextures(Image &base_0, Image &base_1)C;
   void baseTextureSizes(VecI2 *size0, VecI2 *size1);
   void processAlpha();

public:
   MtrlImages();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
