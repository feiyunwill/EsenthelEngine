/******************************************************************************

   Use 'ImageAtlas' for storing multiple images in one texture.

/******************************************************************************/
struct ImageAtlas // Image Atlas, set of multiple images packed together into as few textures as possible
{
   struct Source // source image used to create the atlas
   {
      ImagePtr image;
      Str      name ;

      void set(C ImagePtr &image, C Str &name) {T.image=image; T.name=name;}
   };

   struct Part // single image stored in one of the textures
   {
      Byte  image_index  ; // index of the image in 'images' in which the part is located, or 0xFF if this part is completely transparent and is not stored in any image
      Bool  rotated      ; // if this part is stored in the image as rotated and needs vertical drawing
      Rect  tex_rect     ; // UV texture coordinates of the part (after trimming)
      Vec2  center_offset; // offset which you can apply for drawing assuming that the anchor point was located in the center of the original image, it's equal to "Vec2(trim_pos.x-original_size.x/2, -trim_pos.y+original_size.y/2)" (in pixels)
      VecI2 original_size, // size of the image (in pixels, without trimming)
            trimmed_size , // size of the image (in pixels, after   trimming)
            trim_pos     ; // position in the original image where the trim has started (in pixels)
      Str   name         ; // name of the part
   };

   Mems<Image> images;
   Mems<Part > parts;

   // get
   Part* findPart (C Str &name); // find 'name' part      , null on fail
   Int   findPartI(C Str &name); // find 'name' part index, -1   on fail
   Part*  getPart (C Str &name); //  get 'name' part      , Exit on fail
   Int    getPartI(C Str &name); //  get 'name' part index, Exit on fail

   // manage
   void del   ();
   Bool create(C MemPtr<Source> &images, IMAGE_TYPE image_type, Int mip_maps=0, Bool allow_rotate=true, Int border=4, Bool align_for_compression=false, Bool only_square=false, Int max_tex_size=2048, Bool trim_transparent=true, Bool transparent_to_neighbors=true); // create Image Atlas from array of 'images', 'image_type'=image type of the target image, 'mip_maps'=amount of mip maps the target image should have (0=autodetect), 'allow_rotate'=if allow images to be stored as rotated, 'border'=number of pixels to be used as empty space between images, 'align_for_compression'=if align rectangles on a 4-pixel boundary to allow for equal compression on all images (enabled 'allow_rotate' may prevent this option from best effect), 'only_square'=if allow textures to be created only in the form of a square (if false then rectangle is also allowed), 'max_tex_size'=maximum allowed texture size during creation of the atlas, 'trim_transparent'=if trim transparent pixels from source images and don't store them in the output, 'transparent_to_neighbors'=if convert transparent pixel RGB values to values taken from neighbors

   // draw
   void draw(Int part_index, C Vec2 &pos, Flt pixel_size)C; // draw specified part index at 'pos' screen position, with 'pixel_size'

   // io
   void operator=(C Str  &name) ; // load, Exit  on fail
   Bool save     (C Str  &name)C; // save, false on fail
   Bool load     (C Str  &name) ; // load, false on fail
   Bool save     (  File &f   )C; // save, false on fail
   Bool load     (  File &f   ) ; // load, false on fail
};
/******************************************************************************/
DECLARE_CACHE(ImageAtlas, ImageAtlases, ImageAtlasPtr); // 'ImageAtlases' cache storing 'ImageAtlas' objects which can be accessed by 'ImageAtlasPtr' pointer
/******************************************************************************/
