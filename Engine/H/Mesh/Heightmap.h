/******************************************************************************/
#define MAX_HM_RES (128+1) // Maximum allowed Heightmap Resolution for the 'Heightmap' class

enum HM_FLAG
{
   HM_AO             =1<<0, // if apply ambient occlusion effect to vertex colors
   HM_SOFT           =1<<1, // if create only software 'MeshBase' meshes instead of hardware 'MeshRender'
   HM_BUILD_NULL_MTRL=1<<2, // if build heightmap parts which have the null material
};
/******************************************************************************/
struct Heightmap
{
   // get / set
   Bool is        ()C; // if  has any data
   Int  resolution()C; // get heightmap resolution, this is the width and height of heightmap images
   Bool hasColor  ()C; // if  heightmap currently uses color map
   UInt memUsage  ()C; // get approximate memory size usage of the heightmap

   Int          materials(     )C {return _materials.elms();} // get number of materials used by the heightmap
 C MaterialPtr& material (Int i)C {return _materials[i]    ;} // get i-th      material  used by the heightmap

   Int getMaterialIndex (C MaterialPtr &material) {return _materials.getMaterialIndex (material);} // get index of desired 'material' in the material palette, if 'material' is not listed in the palette it will be added, up to 255 materials can be stored, 0 index is always reserved for null material, -1 on fail
   Int getMaterialIndex0(C MaterialPtr &material) {return _materials.getMaterialIndex0(material);} // get index of desired 'material' in the material palette, if 'material' is not listed in the palette it will be added, up to 255 materials can be stored, 0 index is always reserved for null material,  0 on fail

   Flt  height      (Int x, Int y            )C; // get height using no     filtering at x, y image coordinates, 0 on fail (if coordinates out of range)
   Flt  heightLinear(Flt x, Flt y            )C; // get height using linear filtering at x, y image coordinates, 0 on fail (if coordinates out of range)
   void height      (Int x, Int y, Flt height) ; // set height                        at x, y image coordinates to 'height'

   Color color(Int x, Int y                )C; // get color at x, y image coordinates, WHITE on fail (if coordinates out of range)
   void  color(Int x, Int y, C Color &color) ; // set color at x, y image coordinates to 'color'

   Vec  colorF(Int x, Int y              )C; // get color at x, y image coordinates, Vec(1,1,1) on fail (if coordinates out of range)
   void colorF(Int x, Int y, C Vec &color) ; // set color at x, y image coordinates to 'color'

   Bool getMaterial(Int x, Int y,   VecB4       &mtrl_index                                                 ,   VecB4 &mtrl_blend)C; // get material at x, y image coordinates to 'mtrl_index'  material indexes of 'mtrl_blend' intensities (sum of intensities will be equal to   1), false on fail (if coordinates are out of range), 'mtrl_index' represent the indexes of materials which can be accessed by 'material' function
   Bool getMaterial(Int x, Int y,   VecB4       &mtrl_index                                                 ,   Vec4  &mtrl_blend)C; // get material at x, y image coordinates to 'mtrl_index'  material indexes of 'mtrl_blend' intensities (sum of intensities will be equal to 255), false on fail (if coordinates are out of range), 'mtrl_index' represent the indexes of materials which can be accessed by 'material' function
   void setMaterial(Int x, Int y, C VecB4       &mtrl_index                                                 , C VecB4 &mtrl_blend) ; // set material at x, y image coordinates to 'mtrl_index'  material indexes of 'mtrl_blend' intensities (intensities should be in range   0..255),                                                  'mtrl_index' represent the indexes of materials which can be accessed by 'material' and 'getMaterialIndex' functions
   void setMaterial(Int x, Int y, C VecB4       &mtrl_index                                                 , C Vec4  &mtrl_blend) ; // set material at x, y image coordinates to 'mtrl_index'  material indexes of 'mtrl_blend' intensities (intensities should be in range   0..  1),                                                  'mtrl_index' represent the indexes of materials which can be accessed by 'material' and 'getMaterialIndex' functions
   Bool getMaterial(Int x, Int y,   MaterialPtr &m0,   MaterialPtr &m1,   MaterialPtr &m2,   MaterialPtr &m3,   VecB4 &mtrl_blend)C; // get material at x, y image coordinates to 'm0 m1 m2 m3' materials        of 'mtrl_blend' intensities (sum of intensities will be equal to   1), false on fail (if coordinates are out of range)
   Bool getMaterial(Int x, Int y,   MaterialPtr &m0,   MaterialPtr &m1,   MaterialPtr &m2,   MaterialPtr &m3,   Vec4  &mtrl_blend)C; // get material at x, y image coordinates to 'm0 m1 m2 m3' materials        of 'mtrl_blend' intensities (sum of intensities will be equal to 255), false on fail (if coordinates are out of range)
   void setMaterial(Int x, Int y, C MaterialPtr &m0, C MaterialPtr &m1, C MaterialPtr &m2, C MaterialPtr &m3, C VecB4 &mtrl_blend) ; // set material at x, y image coordinates to 'm0 m1 m2 m3' materials        of 'mtrl_blend' intensities (intensities should be in range   0..255)
   void setMaterial(Int x, Int y, C MaterialPtr &m0, C MaterialPtr &m1, C MaterialPtr &m2, C MaterialPtr &m3, C Vec4  &mtrl_blend) ; // set material at x, y image coordinates to 'm0 m1 m2 m3' materials        of 'mtrl_blend' intensities (intensities should be in range   0..  1)
   void addMaterial(Int x, Int y, C MaterialPtr &m ,                                                            Flt    mtrl_blend) ; // add material at x, y image coordinates

 C Image& heightImage()C {return _height;} // get heightmap height image

   // manage
   Heightmap& del   (); // delete manually
   Heightmap& create(Int resolution, Flt height, C MaterialPtr &material, Bool align_height_to_neighbors, C Heightmap *l=null, C Heightmap *r=null, C Heightmap *b=null, C Heightmap *f=null, C Heightmap *lb=null, C Heightmap *lf=null, C Heightmap *rb=null, C Heightmap *rf=null); // create heightmap with 'resolution', final resolution of the heightmap may be different than 'resolution', and can be obtained using 'resolution' method, heightmap pointers are optional and represent heightmap neighbors (l-left, r-right, b-back, f-forward), all given heightmaps must be of the same resolution as 'resolution'
   Heightmap& create(C Heightmap &src); // create from 'src' heightmap

   // operations
   void resize(Int resolution); // resize heightmap to a custom resolution, final resolution of the heightmap may be different than 'resolution', and can be obtained using 'resolution' method

   void clearMaterials(); // clear internal MaterialPalette, this method is faster than 'cleanMaterials' because it completely resets the material palette, without checking/adjusting per-pixel material indexes, it's important to completely reset materials for every pixel after calling this method
   void cleanMaterials(); // cleanup heightmap by removing unused materials
   void cleanColor    (); // cleanup heightmap by removing color map if it's not needed
   void clean         (); // cleanup heightmap, this calls 'cleanMaterials' and 'cleanColor'

   void build(Mesh &dest_mesh, Int quality, Flt tex_scale, UInt flag, C Heightmap *l=null, C Heightmap *r=null, C Heightmap *b=null, C Heightmap *f=null, C Heightmap *lb=null, C Heightmap *lf=null, C Heightmap *rb=null, C Heightmap *rf=null, Mems<Byte> *temp_mem=null); // build heightmap into 'dest_mesh', 'quality'=0-full, 1-half, 2-quarter, 'tex_scale'=texture map coordinates scale, 'flag'=HM_FLAG, heightmap pointers are optional and represent heightmap neighbors (l-left, r-right, b-back, f-forward), all given heightmaps must be of the same resolution as this heightmap, 'temp_mem'=building heightmap mesh requires certain amount of temporary memory allocated during the process, to avoid dynamic memory allocation everytime this method is called, you may optionally pass 'temp_mem' which this method will use to allocate the memory, you may then reuse the same 'temp_mem' object for subsequent calls to this method which will reuse this memory

#if EE_PRIVATE
   void mtrlBlendHP();
   void createFromQuarter(C Heightmap &src, Bool right, Bool forward                        , C Game::WorldSettings &settings);
   void createFromQuad   (C Heightmap *lb, C Heightmap *rb, C Heightmap *lf, C Heightmap *rf, C Game::WorldSettings &settings);
#endif

   // io
   Bool save(File &f, CChar *path=null)C; // save to   'f' file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load from 'f' file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

#if !EE_PRIVATE
private:
#endif
   Image           _height,
                   _color,
                   _mtrl_index,
                   _mtrl_blend;
   MaterialPalette _materials;

#if EE_PRIVATE
   struct BuildMem;
   struct BuildMemSoft;
   Bool buildEx2(Mesh &mesh, Int quality, Flt tex_scale, UInt flag, BuildMemSoft &mem, C Heightmap *l, C Heightmap *r, C Heightmap *b, C Heightmap *f, C Heightmap *lb, C Heightmap *lf, C Heightmap *rb, C Heightmap *rf); // 'quality'=0-full, 1-half, 2-quarter, 'tex_scale'=texture map coordinates scale, 'flag'=HM_FLAG, heightmap pointers are optional and represent heightmap neighbors (l-left, r-right, b-back, f-forward), all given heightmaps must be of the same resolution as this heightmap, false on fail (out of memory)
   Bool buildEx (Mesh &mesh, Int quality, Flt tex_scale, UInt flag, BuildMemSoft &mem, C Heightmap *l, C Heightmap *r, C Heightmap *b, C Heightmap *f, C Heightmap *lb, C Heightmap *lf, C Heightmap *rb, C Heightmap *rf); // 'quality'=0-full, 1-half, 2-quarter, 'tex_scale'=texture map coordinates scale, 'flag'=HM_FLAG, heightmap pointers are optional and represent heightmap neighbors (l-left, r-right, b-back, f-forward), all given heightmaps must be of the same resolution as this heightmap, false on fail (out of memory)
#endif
};
/******************************************************************************/
