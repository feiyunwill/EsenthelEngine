/******************************************************************************

   Use 'Image' for handling images (textures).

/******************************************************************************/
enum FILTER_TYPE : Byte // Filtering Type
{
   FILTER_NONE             , // 1.00 speed, worst  quality, uses 1x1 samples no 2D filtering
   FILTER_LINEAR           , // 0.92 speed, low    quality, uses 2x2 samples in 2D filtering
   FILTER_CUBIC_FAST       , // 0.17 speed, high   quality, uses 4x4 samples in 2D filtering, low    sharpening is applied
   FILTER_CUBIC_FAST_SMOOTH, // 0.17 speed, blurry quality, uses 4x4 samples in 2D filtering, no     sharpening is applied, result will appear blurry however without aliasing
   FILTER_CUBIC_FAST_SHARP , // 0.17 speed, high   quality, uses 4x4 samples in 2D filtering, high   sharpening is applied (best for down-scaling)
   FILTER_CUBIC            , // 0.10        high   quality, uses 6x6 samples in 2D filtering, medium sharpening is applied (best for   up-scaling)
   FILTER_CUBIC_SHARP      , // 0.10        high   quality, uses 6x6 samples in 2D filtering, high   sharpening is applied
   FILTER_BEST             , // automatically choose the best filter (currently FILTER_CUBIC_FAST_SHARP for down-scaling and FILTER_CUBIC for up-scaling)
   FILTER_NO_STRETCH       , // does not perform any stretching, pixels out of range are either wrapped or clamped
#if EE_PRIVATE
   FILTER_DOWN=FILTER_CUBIC_FAST_SHARP, // best filter used for down-scaling
#endif
};
/******************************************************************************/
enum LOCK_MODE : Byte
{
#if EE_PRIVATE
   LOCK_NONE      =0, // no lock
#endif
   LOCK_READ      =1, // lock for reading only
   LOCK_WRITE     =2, // lock for writing only (if the image is stored in the GPU then any previous image data may get lost when using this mode, use if you wish to replace the whole image data)
   LOCK_READ_WRITE=3, // lock for reading and writing
#if EE_PRIVATE
   LOCK_APPEND    =4, // lock for writing only without overwriting existing data
#endif
};
/******************************************************************************/
enum IMAGE_TYPE : Byte // Image Type, comments specify in which mode the type is available (Soft: Software, DX9: DirectX 9, DX10: DirectX 10, DX11: DirectX 11, GL: Desktop OpenGL, partial: may be supported on some devices but not all of them)
{
   IMAGE_NONE, // none

   IMAGE_B8G8R8A8, // 32-bit (R,G,B,A), Soft, DX9, DX10+
   IMAGE_R8G8B8A8, // 32-bit (R,G,B,A), Soft,      DX10+, GL, GL ES 2.0+
   IMAGE_R8G8B8  , // 24-bit (R,G,B,1), Soft
   IMAGE_R8G8    , // 16-bit (R,G,0,1), Soft,      DX10+, GL, GL ES 3.0+
   IMAGE_R8      , //  8-bit (R,0,0,1), Soft,      DX10+, GL, GL ES 3.0+

   IMAGE_A8  , //  8-bit           alpha (0,0,0,A), Soft, DX9, DX10+, GL, GL ES 2.0+
   IMAGE_L8  , //  8-bit luminance       (L,L,L,1), Soft, DX9       , GL, GL ES 2.0+
   IMAGE_L8A8, // 16-bit luminance alpha (L,L,L,A), Soft, DX9       , GL, GL ES 2.0+

   IMAGE_BC1, // BC1/DXT1 4-bit lossy RGBA compression with 1-bit  alpha            , Soft, DX9, DX10+, GL, partial Android
   IMAGE_BC2, // BC2/DXT3 8-bit lossy RGBA compression with sharp  alpha transitions, Soft, DX9, DX10+, GL, partial Android
   IMAGE_BC3, // BC3/DXT5 8-bit lossy RGBA compression with smooth alpha transitions, Soft, DX9, DX10+, GL, partial Android

   IMAGE_I8   , //      8-bit integer              , Soft
   IMAGE_I16  , //     16-bit integer              , Soft
   IMAGE_I24  , //     24-bit integer              , Soft
   IMAGE_I32  , //     32-bit integer              , Soft
   IMAGE_F16  , //     16-bit float                , Soft, DX9, DX10+, GL, GL ES 3.0+
   IMAGE_F32  , //     32-bit float                , Soft, DX9, DX10+, GL, GL ES 3.0+
   IMAGE_F16_2, // 2 x 16-bit float ( 32-bit total), Soft, DX9, DX10+, GL, GL ES 3.0+
   IMAGE_F32_2, // 2 x 32-bit float ( 64-bit total), Soft, DX9, DX10+, GL, GL ES 3.0+
   IMAGE_F16_3, // 3 x 16-bit float ( 48-bit total), Soft              GL, GL ES 3.0+
   IMAGE_F32_3, // 3 x 32-bit float ( 96-bit total), Soft,      DX10+, GL, GL ES 3.0+
   IMAGE_F16_4, // 4 x 16-bit float ( 64-bit total), Soft, DX9, DX10+, GL, GL ES 3.0+
   IMAGE_F32_4, // 4 x 32-bit float (128-bit total), Soft, DX9, DX10+, GL, GL ES 3.0+

   // compressed formats for iOS (compressing images to these formats is available only on Desktop platforms when 'SupportCompressPVRTC' was called in 'InitPre', decompressing these formats is available on all platforms, for GPU's that don't support these formats natively, the engine will keep them as R8G8B8A8 in the memory, decompression and especially compression may be slow, formats are recommended to be used only on iOS)
   IMAGE_PVRTC1_2, // PVRTC1 2-bit lossy RGBA compression, Soft, iOS, partial Android
   IMAGE_PVRTC1_4, // PVRTC1 4-bit lossy RGBA compression, Soft, iOS, partial Android

   // compressed format for Android (compressing images to these formats is available only on when 'SupportCompressETC' was called in 'InitPre', decompressing these formats is available on all platforms, for GPU's that don't support these formats natively, the engine will keep them as R8G8B8A8 in the memory, decompression and especially compression may be slow, formats are recommended to be used only on Android)
   IMAGE_ETC1   , // Ericsson 4-bit lossy RGB  compression with no    alpha (R,G,B,1     ), Soft, Android
   IMAGE_ETC2   , // Ericsson 4-bit lossy RGB  compression with no    alpha (R,G,B,1     ), Soft, GL ES 3.0+
   IMAGE_ETC2_A1, // Ericsson 4-bit lossy RGBA compression with 1-bit alpha (R,G,B,0 or 1), Soft, GL ES 3.0+
   IMAGE_ETC2_A8, // Ericsson 8-bit lossy RGBA compression with 8-bit alpha (R,G,B,A     ), Soft, GL ES 3.0+

   IMAGE_BC7, // BC7 8-bit lossy RGBA high quality compression, Soft, DX11+, partial GL (compressing images to this format is available only on when 'SupportCompressBC7' was called in 'InitPre')

   IMAGE_R10G10B10A2, // 32-bit (R,G,B,A), Soft, DX9, DX10+, GL, GL ES 3.0+

   IMAGE_TYPES, // number of types
#if EE_PRIVATE
   IMAGE_B4G4R4X4     ,
   IMAGE_B4G4R4A4     ,
   IMAGE_B5G5R5X1     ,
   IMAGE_B5G5R5A1     ,
   IMAGE_B5G6R5       ,
   IMAGE_B8G8R8       ,
   IMAGE_B8G8R8X8     ,
   IMAGE_R8G8B8X8     ,
   IMAGE_R8G8B8A8_SIGN, // 32-bit (R,G,B,A), Soft, partial DX9, DX10+, GL, GL ES 3.0+
   IMAGE_R8G8_SIGN    , // 16-bit (R,G,0,1), Soft, partial DX9, DX10+, GL, GL ES 3.0+
   IMAGE_R8_SIGN      , //  8-bit (R,0,0,1), Soft,              DX10+, GL, GL ES 3.0+
   IMAGE_D16  ,
   IMAGE_D24X8,
   IMAGE_D24S8,
   IMAGE_D32  ,
   IMAGE_RAWZ ,
   IMAGE_INTZ ,
   IMAGE_DF24 ,
   IMAGE_NULL ,
   IMAGE_ALL_TYPES, // number of all types
#if DX9
   IMAGE_DEFAULT=IMAGE_B8G8R8A8, // default format for DX9 is BGRA (RGBA may not be supported)
#else
   IMAGE_DEFAULT=IMAGE_R8G8B8A8, // this is the default format
#endif
#endif
   // alternative names
   IMAGE_DXT1=IMAGE_BC1,
   IMAGE_DXT3=IMAGE_BC2,
   IMAGE_DXT5=IMAGE_BC3,
};
enum IMAGE_MODE : Byte // Image Mode
{
   IMAGE_2D       , // Hardware 2D   Texture
   IMAGE_3D       , // Hardware 3D   Texture (not supported on OpenGL ES 2)
   IMAGE_CUBE     , // Hardware Cube Texture
   IMAGE_SOFT     , // Software      Image   (this type is used for software processing only - it can't be drawn on the screen)
   IMAGE_SOFT_CUBE, // Software Cube Image   (this type is used for software processing only - it can't be drawn on the screen)
   IMAGE_RT       , // Hardware RenderTarget (only this mode can be used as custom rendering destination for 'Renderer.target', after you have rendered to this image you can treat it as typical IMAGE_2D texture, except that you can't lock it) !! Images of this mode must be deleted in a custom function set into 'D.lost' !!
#if EE_PRIVATE
   IMAGE_SURF_SCRATCH, // System   Surface from scratch
   IMAGE_SURF_SYSTEM , // System   Surface
   IMAGE_SURF        , // Hardware Surface
   IMAGE_RT_CUBE     , // Hardware RenderTarget Cube
   IMAGE_DS          , // Hardware Depth Stencil Buffer
   IMAGE_DS_RT       , // Hardware Depth Stencil Buffer RenderTarget
   IMAGE_SHADOW_MAP  , // Hardware Shadow Map (Depth Texture)
#endif
};
#if EE_PRIVATE
Bool IsSoft(IMAGE_MODE mode); // if this is a software image     (IMAGE_SOFT, IMAGE_SOFT_CUBE)
Bool IsHW  (IMAGE_MODE mode); // if this is a hardware image NOT (IMAGE_SOFT, IMAGE_SOFT_CUBE)
Bool IsCube(IMAGE_MODE mode); // if this is a cube     image     (IMAGE_CUBE, IMAGE_SOFT_CUBE or IMAGE_RT_CUBE)
#endif
enum IMAGE_PRECISION : Byte // Image Precision
{
   IMAGE_PRECISION_8  , // up to 8-bits
   IMAGE_PRECISION_10 , //      10-bits
   IMAGE_PRECISION_16 , //      16-bits
   IMAGE_PRECISION_24 , //      24-bits
   IMAGE_PRECISION_32 , //      32-bits
   IMAGE_PRECISION_64 , //      64-bits
   IMAGE_PRECISION_NUM, // number of precisions
};
#if EE_PRIVATE
inline IMAGE_PRECISION Min(IMAGE_PRECISION a, IMAGE_PRECISION b) {return (a<b) ? a : b;}
inline IMAGE_PRECISION Max(IMAGE_PRECISION a, IMAGE_PRECISION b) {return (a>b) ? a : b;}
#endif
/******************************************************************************/
struct ImageTypeInfo // Image Type Information
{
  CChar8          *name      ; // type name
   Bool            compressed; // if the type is a compressed type
   Byte            byte_pp   , // bytes per pixel
                   bit_pp    , // bits  per pixel
                   r         , // number of red     bits
                   g         , // number of green   bits
                   b         , // number of blue    bits
                   a         , // number of alpha   bits
                   d         , // number of depth   bits
                   s         , // number of stencil bits
                   channels  ; // number of channels
   IMAGE_PRECISION precision ;

   Bool highPrecision()C {return precision>IMAGE_PRECISION_8;} // more than 8 bits

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   mutable GPU_API(D3DFORMAT, DXGI_FORMAT, UInt) format;
#else
   mutable UInt format;
#endif
};extern ImageTypeInfo
   const ImageTI[]; // Image Type Info Array, allows obtaining information about specified IMAGE_TYPE, sample usage: ImageTI[IMAGE_R8G8B8A8].name -> "R8G8B8A8"
/******************************************************************************/
struct Image // Image (Texture)
{
   // get
   Int   w()C {return      _size.x;} // get width                              (in pixels)
   Int   h()C {return      _size.y;} // get height                             (in pixels)
   Int   d()C {return      _size.z;} // get depth                              (in pixels)
   Int  lw()C {return _lock_size.x;} // get width  of currently locked mip map (in pixels)
   Int  lh()C {return _lock_size.y;} // get height of currently locked mip map (in pixels)
   Int  ld()C {return _lock_size.z;} // get depth  of currently locked mip map (in pixels)
   Int hwW()C {return   _hw_size.x;} // get width  as it is stored on the GPU  (in pixels), this can be different than 'w' if the size is a non-power of 2, but the GPU does not support non-power of 2 textures, in which case the hardware size will be rounded up to the nearest power of 2
   Int hwH()C {return   _hw_size.y;} // get height as it is stored on the GPU  (in pixels), this can be different than 'h' if the size is a non-power of 2, but the GPU does not support non-power of 2 textures, in which case the hardware size will be rounded up to the nearest power of 2
   Int hwD()C {return   _hw_size.z;} // get depth  as it is stored on the GPU  (in pixels), this can be different than 'd' if the size is a non-power of 2, but the GPU does not support non-power of 2 textures, in which case the hardware size will be rounded up to the nearest power of 2

 C VecI2&     size ()C {return      _size.xy;} // get image size (in pixels)
 C VecI &     size3()C {return      _size   ;} // get image size (in pixels)
 C VecI2&   hwSize ()C {return   _hw_size.xy;} // get image size as it is stored on the GPU (in pixels)
 C VecI &   hwSize3()C {return   _hw_size   ;} // get image size as it is stored on the GPU (in pixels)
 C VecI2& lockSize ()C {return _lock_size.xy;} // get lock  size (in pixels)

   Bool         is   ()C {return _type!=IMAGE_NONE;} // if  valid
   IMAGE_TYPE   type ()C {return _type            ;} // get image  type
   IMAGE_TYPE hwType ()C {return _hw_type         ;} // get image  type in which it is stored on the GPU (this can be different than 'type' if it is not supported directly on the hardware, for example image was created as compressed format which the GPU does not support, 'type' will be set to the compressed format but 'hwType' may be set to R8G8B8A8 format as stored on the GPU)
   IMAGE_MODE mode   ()C {return _mode            ;} // get image  mode
   Int        mipMaps()C {return _mms             ;} // get number of mipmaps
   Byte       samples()C {return _samples         ;} // get number of samples per pixel
   Bool  multiSample ()C {return _samples>1       ;} // if  this   is a multi sampled image
   Bool       partial()C {return _partial         ;} // if  'hwSize' is different than 'size'
   Int          faces()C;                            // get how many faces this image has (0=empty, 1=default, 6=cube)
#if EE_PRIVATE
   Bool          soft()C {return IsSoft(mode())   ;} // if this is a software image     (IMAGE_SOFT, IMAGE_SOFT_CUBE)
   Bool            hw()C {return IsHW  (mode())   ;} // if this is a hardware image NOT (IMAGE_SOFT, IMAGE_SOFT_CUBE)
   Bool          cube()C {return IsCube(mode())   ;} // if this is a cube     image     (IMAGE_CUBE, IMAGE_SOFT_CUBE or IMAGE_RT_CUBE)
   DIR_ENUM lCubeFace()C {return _lcf             ;} // get locked cube face
#endif

   Int  lMipMap()C {return _lmm   ;} // get index              of locked mip map
   UInt  pitch ()C {return _pitch ;} // get width        pitch of locked mip map
   UInt  pitch2()C {return _pitch2;} // get width*height pitch of locked mip map
   Byte* data  ()  {return _data  ;} // get address            of locked data, memory accessed using this method should be interpreted according to 'hwType' (and not 'type')
 C Byte* data  ()C {return _data  ;} // get address            of locked data, memory accessed using this method should be interpreted according to 'hwType' (and not 'type')

   Flt                    aspect()C {return Flt(w())/h()                     ;} // get          aspect ratio of image "width/height"
   Flt                 invAspect()C {return Flt(h())/w()                     ;} // get inversed aspect ratio of image "height/width"
   UInt                 memUsage()C;                                            // get actual  memory usage of the image, this method operates on 'hwType' (what the image is using right now)
   UInt             typeMemUsage()C;                                            // get desired memory usage of the image, this method operates on   'type' (what the image would use if it was stored in its desired type)
   Int                    bytePP()C {return         _byte_pp                 ;} // get number of bytes per pixel
   Bool               compressed()C {return ImageTI[_hw_type].   compressed  ;} // if  hardware type is compressed
   IMAGE_PRECISION     precision()C {return ImageTI[_hw_type].    precision  ;} // get image precision
   Bool            highPrecision()C {return ImageTI[_hw_type].highPrecision();} // if  any channel of the image uses more than 8 bits

   // manage
   Image& del          (                                                                                             );                                                                       // delete
#if EE_PRIVATE
   Bool   createTryEx  (Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Byte samples=1, IMAGE_TYPE type_on_fail=IMAGE_NONE, C Image *src=null);
#endif
   Bool   createTry    (Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps=0, Bool rgba_on_fail=true);                                                                       // create                 image, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, false on fail
   Image& create       (Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps=0, Bool rgba_on_fail=true);                                                                       // create                 image, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, Exit  on fail
   Bool   create2DTry  (Int w, Int h,        IMAGE_TYPE type,                  Int mip_maps=0, Bool rgba_on_fail=true) {return createTry(w, h, 1, type, IMAGE_2D  , mip_maps, rgba_on_fail);} // create hardware 2D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, false on fail
   Image& create2D     (Int w, Int h,        IMAGE_TYPE type,                  Int mip_maps=0, Bool rgba_on_fail=true) {return create   (w, h, 1, type, IMAGE_2D  , mip_maps, rgba_on_fail);} // create hardware 2D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, Exit  on fail
   Bool   create3DTry  (Int w, Int h, Int d, IMAGE_TYPE type,                  Int mip_maps=1, Bool rgba_on_fail=true) {return createTry(w, h, d, type, IMAGE_3D  , mip_maps, rgba_on_fail);} // create hardware 3D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, false on fail
   Image& create3D     (Int w, Int h, Int d, IMAGE_TYPE type,                  Int mip_maps=1, Bool rgba_on_fail=true) {return create   (w, h, d, type, IMAGE_3D  , mip_maps, rgba_on_fail);} // create hardware 3D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, Exit  on fail
   Bool   createCubeTry(Int w,               IMAGE_TYPE type,                  Int mip_maps=1, Bool rgba_on_fail=true) {return createTry(w, w, 1, type, IMAGE_CUBE, mip_maps, rgba_on_fail);} // create hardware cube texture, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, false on fail
   Image& createCube   (Int w,               IMAGE_TYPE type,                  Int mip_maps=1, Bool rgba_on_fail=true) {return create   (w, w, 1, type, IMAGE_CUBE, mip_maps, rgba_on_fail);} // create hardware cube texture, 'mip_maps'=number of mip-maps (0=autodetect), 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, Exit  on fail
   Bool   createSoftTry(Int w, Int h, Int d, IMAGE_TYPE type,                  Int mip_maps=1                        ) {return createTry(w, h, d, type, IMAGE_SOFT, mip_maps,        false);} // create software        image, 'mip_maps'=number of mip-maps (0=autodetect),                                                                                      false on fail
   Image& createSoft   (Int w, Int h, Int d, IMAGE_TYPE type,                  Int mip_maps=1                        ) {return create   (w, h, d, type, IMAGE_SOFT, mip_maps,        false);} // create software        image, 'mip_maps'=number of mip-maps (0=autodetect),                                                                                      Exit  on fail

   Bool copyTry(Image &dest, Int w=-1, Int h=-1, Int d=-1, Int type=-1, Int mode=-1, Int mip_maps=-1, FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool alpha_weight=false, Bool keep_edges=false, Bool mtrl_base_1=false, Bool rgba_on_fail=true)C; // copy to 'dest', -1=keep original value, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE (this method does not support IMAGE_3D), 'mip_maps'=number of mip-maps (0=autodetect), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color, 'keep_edges'=if preserve the edges of the image when resizing, 'mtrl_base_1'=if this image is 'Material.base_1' texture, 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, false on fail
   void copy   (Image &dest, Int w=-1, Int h=-1, Int d=-1, Int type=-1, Int mode=-1, Int mip_maps=-1, FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool alpha_weight=false, Bool keep_edges=false, Bool mtrl_base_1=false, Bool rgba_on_fail=true)C; // copy to 'dest', -1=keep original value, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE (this method does not support IMAGE_3D), 'mip_maps'=number of mip-maps (0=autodetect), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color, 'keep_edges'=if preserve the edges of the image when resizing, 'mtrl_base_1'=if this image is 'Material.base_1' texture, 'rgba_on_fail'=if try using uncompressed RGBA type if given 'type' is not supported, Exit  on fail

   Bool   toCube(C Image &src, Int size=-1, Int type=-1, IMAGE_MODE mode=IMAGE_SOFT_CUBE, FILTER_TYPE filter=FILTER_BEST); // convert from 6x1 'src' image                 (containing in order : left, front, right, back, down, up views) to cube texture, 'size'=desired resolution of the image (-1=keep), 'type'=IMAGE_TYPE (-1=keep), 'mode'=IMAGE_MODE, 'filter'=what kind of filtering to use when source is of different size than the target
   Bool fromCube(C Image &src,              Int type=-1, IMAGE_MODE mode=IMAGE_SOFT                                     ); // convert from 'src' cube texture to 6x1 image (containing in order : left, front, right, back, down, up views)                ,                                                   'type'=IMAGE_TYPE (-1=keep), 'mode'=IMAGE_MODE

   // lock
#if EE_PRIVATE
   Byte* softData()  {return _data_all;} // get software image data without locking the image
 C Byte* softData()C {return _data_all;} // get software image data without locking the image
   Byte* softData(Int mip_map, DIR_ENUM cube_face=DIR_RIGHT);                                                     // get software image data for 'mip_map' and 'cube_face' without locking the image
 C Byte* softData(Int mip_map, DIR_ENUM cube_face=DIR_RIGHT)C {return ConstCast(T).softData(mip_map, cube_face);} // get software image data for 'mip_map' and 'cube_face' without locking the image

   void lockSoft();
   Bool setFrom(CPtr data, Int data_pitch, Int mip_map=0, DIR_ENUM cube_face=DIR_RIGHT);
#endif
   Bool     lock    (LOCK_MODE lock=LOCK_READ_WRITE, Int mip_map=0, DIR_ENUM cube_face=DIR_RIGHT) ; //   lock image for editing specified 'mip_map', this needs to be called before manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), 'cube_face'=desired cube face (this is used only for IMAGE_CUBE modes)
   Bool     lockRead(                                Int mip_map=0, DIR_ENUM cube_face=DIR_RIGHT)C; //   lock image for reading specified 'mip_map', this needs to be called before manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), 'cube_face'=desired cube face (this is used only for IMAGE_CUBE modes), this method has the same effect as calling "lock(LOCK_READ, mip_map, cube_face)", however unlike 'lock' method it has 'const' modifier and can be called on "const Image" objects
   Image& unlock    (                                                                           ) ; // unlock image                                , this needs to be called after  manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), if you want the mip maps to be updated according to any change applied during the lock then you must call 'updateMipMaps' after 'unlock'
 C Image& unlock    (                                                                           )C; // unlock image                                , this needs to be called after  manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), if you want the mip maps to be updated according to any change applied during the lock then you must call 'updateMipMaps' after 'unlock'

   Image& updateMipMaps(FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool alpha_weight=false, Bool mtrl_base_1=false, Int mip_start=0); // update mip maps of the image, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color, 'mtrl_base_1'=if this image is 'Material.base_1' texture, 'mip_start'=index of the mip map to start with (this mip map will be taken, and downsampled to following mip maps)

   Image& freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)

   // pixel
   UInt  pixel (Int x, Int y)C;   void pixel (Int x, Int y,   UInt   pixel); // get/set pixel UInt value (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt   pixelF(Int x, Int y)C;   void pixelF(Int x, Int y,   Flt    pixel); // get/set pixel Flt  value (these methods may not support all compressed types, instead try using 'copy' method first)
   Color color (Int x, Int y)C;   void color (Int x, Int y, C Color &color); // get/set color Byte color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4  colorF(Int x, Int y)C;   void colorF(Int x, Int y, C Vec4  &color); // get/set color Flt  color (these methods may not support all compressed types, instead try using 'copy' method first)

   void blend(Int x, Int y, C Vec4 &color); // apply 'color' pixel using ALPHA_BLEND formula
   void merge(Int x, Int y, C Vec4 &color); // apply 'color' pixel using ALPHA_MERGE formula

   Flt pixelFLinear         (Flt x, Flt y, Bool clamp=true)C; // get pixel Flt with Linear            interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixelFCubicFast      (Flt x, Flt y, Bool clamp=true)C; // get pixel Flt with Cubic Fast        interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixelFCubicFastSmooth(Flt x, Flt y, Bool clamp=true)C; // get pixel Flt with Cubic Fast Smooth interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixelFCubicFastSharp (Flt x, Flt y, Bool clamp=true)C; // get pixel Flt with Cubic Fast Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixelFCubic          (Flt x, Flt y, Bool clamp=true)C; // get pixel Flt with Cubic             interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixelFCubicSharp     (Flt x, Flt y, Bool clamp=true)C; // get pixel Flt with Cubic Sharp       interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)

   Vec4 colorFLinear         (Flt x, Flt y, Bool clamp=true, Bool alpha_weight=false)C; // get color Vec4 with Linear            interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 colorFCubicFast      (Flt x, Flt y, Bool clamp=true, Bool alpha_weight=false)C; // get color Vec4 with Cubic Fast        interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 colorFCubicFastSmooth(Flt x, Flt y, Bool clamp=true, Bool alpha_weight=false)C; // get color Vec4 with Cubic Fast Smooth interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 colorFCubicFastSharp (Flt x, Flt y, Bool clamp=true, Bool alpha_weight=false)C; // get color Vec4 with Cubic Fast Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 colorFCubic          (Flt x, Flt y, Bool clamp=true, Bool alpha_weight=false)C; // get color Vec4 with Cubic             interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 colorFCubicSharp     (Flt x, Flt y, Bool clamp=true, Bool alpha_weight=false)C; // get color Vec4 with Cubic Sharp       interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
#if EE_PRIVATE
   Vec4 colorFLinearTTNF32_4(Flt x, Flt y, Bool clamp)C; // optimized version specifically for 'transparentToNeighbor', 'clamp'=if use clamping when filtering pixels
#endif

   // pixel 3D
   UInt  pixel3D (Int x, Int y, Int z)C;   void pixel3D (Int x, Int y, Int z,   UInt   pixel); // get/set pixel 3D UInt value (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt   pixel3DF(Int x, Int y, Int z)C;   void pixel3DF(Int x, Int y, Int z,   Flt    pixel); // get/set pixel 3D Flt  value (these methods may not support all compressed types, instead try using 'copy' method first)
   Color color3D (Int x, Int y, Int z)C;   void color3D (Int x, Int y, Int z, C Color &color); // get/set color 3D Byte color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4  color3DF(Int x, Int y, Int z)C;   void color3DF(Int x, Int y, Int z, C Vec4  &color); // get/set color 3D Flt  color (these methods may not support all compressed types, instead try using 'copy' method first)

   Flt pixel3DFLinear         (Flt x, Flt y, Flt z, Bool clamp=true)C; // get 3D pixel Flt with Linear            interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixel3DFCubicFast      (Flt x, Flt y, Flt z, Bool clamp=true)C; // get 3D pixel Flt with Cubic Fast        interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixel3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp=true)C; // get 3D pixel Flt with Cubic Fast Smooth interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixel3DFCubicFastSharp (Flt x, Flt y, Flt z, Bool clamp=true)C; // get 3D pixel Flt with Cubic Fast Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixel3DFCubic          (Flt x, Flt y, Flt z, Bool clamp=true)C; // get 3D pixel Flt with Cubic             interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
   Flt pixel3DFCubicSharp     (Flt x, Flt y, Flt z, Bool clamp=true)C; // get 3D pixel Flt with Cubic Sharp       interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)

   Vec4 color3DFLinear         (Flt x, Flt y, Flt z, Bool clamp=true, Bool alpha_weight=false)C; // get 3D color Vec4 with Linear            interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 color3DFCubicFast      (Flt x, Flt y, Flt z, Bool clamp=true, Bool alpha_weight=false)C; // get 3D color Vec4 with Cubic Fast        interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 color3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp=true, Bool alpha_weight=false)C; // get 3D color Vec4 with Cubic Fast Smooth interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 color3DFCubicFastSharp (Flt x, Flt y, Flt z, Bool clamp=true, Bool alpha_weight=false)C; // get 3D color Vec4 with Cubic Fast Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 color3DFCubic          (Flt x, Flt y, Flt z, Bool clamp=true, Bool alpha_weight=false)C; // get 3D color Vec4 with Cubic             interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 color3DFCubicSharp     (Flt x, Flt y, Flt z, Bool clamp=true, Bool alpha_weight=false)C; // get 3D color Vec4 with Cubic Sharp       interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)

   // pixel area
   Vec4 areaColorAverage        (C Vec2 &pos, C Vec2 &size, Bool clamp=true, Bool alpha_weight=false)C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Average           interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 areaColorLinear         (C Vec2 &pos, C Vec2 &size, Bool clamp=true, Bool alpha_weight=false)C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Linear            interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 areaColorCubicFast      (C Vec2 &pos, C Vec2 &size, Bool clamp=true, Bool alpha_weight=false)C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast        interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 areaColorCubicFastSmooth(C Vec2 &pos, C Vec2 &size, Bool clamp=true, Bool alpha_weight=false)C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast Smooth interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 areaColorCubicFastSharp (C Vec2 &pos, C Vec2 &size, Bool clamp=true, Bool alpha_weight=false)C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 areaColorCubic          (C Vec2 &pos, C Vec2 &size, Bool clamp=true, Bool alpha_weight=false)C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic             interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
   Vec4 areaColorCubicSharp     (C Vec2 &pos, C Vec2 &size, Bool clamp=true, Bool alpha_weight=false)C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Sharp       interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)

   // operations
   Image& clear                (                                                                                                                                                     ) ; // clear to 0 (transparent black)
   Image& normalize            (               Bool red=true, Bool green=true, Bool blue=true, Bool alpha=true, C BoxI *box=null                                                     ) ; // normalize selected components to 0..1 range, 'box'=optional box in which perform the operation (use null for entire image)
   Image& mulAdd               (             C Vec4 &mul, C Vec4 &add                                         , C BoxI *box=null                                                     ) ; // transform color to "color*mul+add"         , 'box'=optional box in which perform the operation (use null for entire image)
   void   bumpToNormal         (  Image &dest, Flt  scale, Bool high_precision=false                                                                                                 )C; // convert bump map to normal map, 'scale'=bump scaling factor 0..Inf, 'high_precision'=if create signed float image type, or unsigned byte image type
   void   crop                 (  Image &dest, Int x, Int y,        Int w, Int h                                                                                                     )C; // crop      image
   void   crop3D               (  Image &dest, Int x, Int y, Int z, Int w, Int h, Int d                                                                                              )C; // crop   3D image
   Image& resize               (               Int w, Int h,        FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool alpha_weight=false, Bool keep_edges=false                  ) ; // resize    image, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color, 'keep_edges'=if preserve the edges of the image
   Image& resize3D             (               Int w, Int h, Int d, FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool alpha_weight=false, Bool keep_edges=false                  ) ; // resize 3D image, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color, 'keep_edges'=if preserve the edges of the image
   Image& mirrorX              (                                                                                                                                                     ) ; // mirror    image horizontally
   Image& mirrorY              (                                                                                                                                                     ) ; // mirror    image   vertically
   Image& alphaFromKey         (               C Color &key=PURPLE                                                                                                                   ) ; // transform to ((pixel color==key) ? (0, 0, 0, 0) : (r, g, b, 255))
   Image& alphaFromBrightness  (                                                                                                                                                     ) ; // transform to (r  , g  , b  , brightness)
   Image& divRgbByAlpha        (                                                                                                                                                     ) ; // transform to (r/a, g/a, b/a, a)
   Bool   stats                (               Vec4 *min=null, Vec4 *max=null, Vec4 *avg=null, Vec4 *median=null, Vec4 *mode=null, Vec *avg_alpha_weight=null, C BoxI *box=null      )C; // get image statistics               , such as: minimum, maximum, average, median and mode color values, 'box'=optional box in which perform the operation (use null for entire image), false on fail
   Bool   statsSat             (               Flt  *min=null, Flt  *max=null, Flt  *avg=null, Flt  *median=null, Flt  *mode=null, Flt *avg_alpha_weight=null, C BoxI *box=null      )C; // get image statistics for saturation, such as: minimum, maximum, average, median and mode       values, 'box'=optional box in which perform the operation (use null for entire image), false on fail
   Bool   monochromatic        (                                                                                                                                                     )C; // check if image is monochromatic (all RGB values are the same)
   Bool   extractMipMap        (  Image &dest, Int type, Int mode, Int mip_map, DIR_ENUM cube_face=DIR_RIGHT                                                                         )C; // extract specified mipmap to   'dest' 0-th mipmap, false on fail, 'type'=IMAGE_TYPE (-1=keep), 'mode'=IMAGE_MODE (-1=keep)
   Bool    injectMipMap        (C Image &src ,                     Int mip_map, DIR_ENUM cube_face=DIR_RIGHT, FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool mtrl_base_1=false) ; //  inject specified mipmap from 'src'  0-th mipmap, false on fail, 'filter'=what kind of filtering to use when source is of different size than the target, 'clamp'=if use clamping when filtering pixels, 'mtrl_base_1'=if this image is 'Material.base_1' texture
   Image& downSample           (               FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool alpha_weight=false                                                              ) ; // downsample to half resolution size, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color
   Bool   averageX             (  Image &dest,   Int   range, Bool clamp, Threads *threads=null                                                                                      )C; // horizontal average, 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Bool   averageY             (  Image &dest,   Int   range, Bool clamp, Threads *threads=null                                                                                      )C; // vertical   average, 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Bool   averageZ             (  Image &dest,   Int   range, Bool clamp, Threads *threads=null                                                                                      )C; // depth      average, 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Bool   average              (  Image &dest, C VecI &range, Bool clamp, Threads *threads=null                                                                                      )C; //            average, 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Image& average              (               C VecI &range, Bool clamp, Threads *threads=null                                                                                      ) ; //            average, 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads
   Bool   blurX                (  Image &dest,   Flt   range, Bool clamp, Threads *threads=null                                                                                      )C; // horizontal blur   , 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Bool   blurY                (  Image &dest,   Flt   range, Bool clamp, Threads *threads=null                                                                                      )C; // vertical   blur   , 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Bool   blurZ                (  Image &dest,   Flt   range, Bool clamp, Threads *threads=null                                                                                      )C; // depth      blur   , 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Bool   blur                 (  Image &dest, C Vec  &range, Bool clamp, Threads *threads=null                                                                                      )C; //            blur   , 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads, false on fail
   Image& blur                 (               C Vec  &range, Bool clamp, Threads *threads=null                                                                                      ) ; //            blur   , 'range'=range of blurring (in pixels), 'threads'=optional threads allowing to perform the operation on multiple threads
   Image& sharpen              (                 Flt   power, Byte range, Bool clamp, Bool blur                                                                                      ) ; // sharpen image, 'power'=0..Inf (default=1.0), 'range'=blurring range, 'clamp'=blurring clamp, 'blur'=if use blur or averaging
   Image& noise                (                 Byte  red  , Byte green, Byte blue , Byte alpha                                                                                     ) ; // add noise of selected scale per component to image
   Image& RGBToHSB             (                                                                                                                                                     ) ; // convert Red Green Blue image to Hue Saturation Brightness (Alpha will be kept)
   Image& HSBToRGB             (                                                                                                                                                     ) ; // convert Hue Saturation Brightness image to Red Green Blue (Alpha will be kept)
   Image& tile                 (               Int  range, Bool horizontally=true, Bool vertically=true                                                                              ) ; // make tileable, 'range'=number of pixels to blend
   Image& minimum              (               Flt  distance                                                                                                                         ) ; // apply minimum filter, 'distance'=pixel range (0..1)
   Image& maximum              (               Flt  distance                                                                                                                         ) ; // apply maximum filter, 'distance'=pixel range (0..1)
   Image& transparentToNeighbor(               Bool clamp=true, Flt step=1                                                                                                           ) ; // replace transparent pixels with neighbors
   Bool   getSameColorNeighbors(               Int x, Int y, MemPtr<VecI2> pixels, Bool diagonal=true                                                                                )C; // get a list of all neighbor pixels   with the same color, 'diagonal'=if allow diagonal movements, false on fail
   Image& fill                 (               Int x, Int y, C Color      &color , Bool diagonal=true                                                                                ) ; // fill image at specified coordinates with given    color, 'diagonal'=if allow diagonal movements

   Image& createShadow(C Image &src   , Int blur, Flt shadow_opacity=1.0f, Flt shadow_spread=0.0f, Bool border_padd=true                                                        ); // create shadow image IMAGE_A8 from 'src' alpha channel, 'blur'=blur range (in pixels), 'shadow_opacity'=shadow opacity (0..1), 'shadow_spread'=shadow spread (0..1)
   Image&  applyShadow(C Image &shadow,      C Color &shadow_color  =BLACK,                                               C VecI2 &offset=0, Int image_type=0, Bool combine=true); // apply  shadow image to self, 'offset'=offset in pixels where to apply the shadow, 'image_type'=IMAGE_TYPE (-1=keep, 0=autodetect), 'combine'=if combine on to the color map (if false then alpha will be set to shadow intensity)
   Image&    setShadow(                 Int blur, Flt shadow_opacity=1.0f, Flt shadow_spread=0.0f, Bool border_padd=true, C VecI2 &offset=0, Int image_type=0, Bool combine=true); // create shadow and apply to self

#if EE_PRIVATE
   Bool accessible  ()C;
   Bool depthTexture()C;
   Bool compatible  (C Image &image)C;

   Image& fastCrop(Int w, Int h, Int d); // crop by adjusting the internal dimension members only, this will work only if the new dimensions are smaller or equal than hardware dimensions and mip map count is equal to 1

   Color decompress  (Int x, Int y       )C;
   Color decompress3D(Int x, Int y, Int z)C;
#endif

   Bool raycast(C Vec &start, C Vec &move, C Matrix *image_matrix=null, Flt *hit_frac=null, Vec *hit_pos=null, Flt precision=1.0f)C; // perform ray casting from 'start' position along 'move' vector, return true if collision encountered with the image, by default the Image is on XY plane with its pixel values extending it towards the Z axis, 'image_matrix'=image transformation matrix, 'hit_frac'=fraction of the movement where collision occurs, 'hit_pos'=position where collision occurs, 'precision'=affects number of pixels to advance in a single step (lower value makes calculations faster but less precise)

   // !! warning: following methods do not check for coordinates being out of range, image type matching, and image being locked, use only if you know what you're doing !!
   Byte & pixB (Int x, Int y) {return *(Byte *)(_data + x*SIZE(Byte ) + y*_pitch);}   Byte & pixB (C VecI2 &v) {return pixB (v.x, v.y);}
   UInt & pix  (Int x, Int y) {return *(UInt *)(_data + x*SIZE(UInt ) + y*_pitch);}   UInt & pix  (C VecI2 &v) {return pix  (v.x, v.y);}
   Color& pixC (Int x, Int y) {return *(Color*)(_data + x*SIZE(Color) + y*_pitch);}   Color& pixC (C VecI2 &v) {return pixC (v.x, v.y);}
   VecB & pixB3(Int x, Int y) {return *(VecB *)(_data + x*SIZE(VecB ) + y*_pitch);}   VecB & pixB3(C VecI2 &v) {return pixB3(v.x, v.y);}
   VecB4& pixB4(Int x, Int y) {return *(VecB4*)(_data + x*SIZE(VecB4) + y*_pitch);}   VecB4& pixB4(C VecI2 &v) {return pixB4(v.x, v.y);}
   Flt  & pixF (Int x, Int y) {return *(Flt  *)(_data + x*SIZE(Flt  ) + y*_pitch);}   Flt  & pixF (C VecI2 &v) {return pixF (v.x, v.y);}
   Vec2 & pixF2(Int x, Int y) {return *(Vec2 *)(_data + x*SIZE(Vec2 ) + y*_pitch);}   Vec2 & pixF2(C VecI2 &v) {return pixF2(v.x, v.y);}
   Vec  & pixF3(Int x, Int y) {return *(Vec  *)(_data + x*SIZE(Vec  ) + y*_pitch);}   Vec  & pixF3(C VecI2 &v) {return pixF3(v.x, v.y);}
   Vec4 & pixF4(Int x, Int y) {return *(Vec4 *)(_data + x*SIZE(Vec4 ) + y*_pitch);}   Vec4 & pixF4(C VecI2 &v) {return pixF4(v.x, v.y);}

   Byte & pixB (Int x, Int y, Int z) {return *(Byte *)(_data + x*SIZE(Byte ) + y*_pitch + z*_pitch2);}   Byte & pixB (C VecI &v) {return pixB (v.x, v.y, v.z);}
   UInt & pix  (Int x, Int y, Int z) {return *(UInt *)(_data + x*SIZE(UInt ) + y*_pitch + z*_pitch2);}   UInt & pix  (C VecI &v) {return pix  (v.x, v.y, v.z);}
   Color& pixC (Int x, Int y, Int z) {return *(Color*)(_data + x*SIZE(Color) + y*_pitch + z*_pitch2);}   Color& pixC (C VecI &v) {return pixC (v.x, v.y, v.z);}
   VecB & pixB3(Int x, Int y, Int z) {return *(VecB *)(_data + x*SIZE(VecB ) + y*_pitch + z*_pitch2);}   VecB & pixB3(C VecI &v) {return pixB3(v.x, v.y, v.z);}
   VecB4& pixB4(Int x, Int y, Int z) {return *(VecB4*)(_data + x*SIZE(VecB4) + y*_pitch + z*_pitch2);}   VecB4& pixB4(C VecI &v) {return pixB4(v.x, v.y, v.z);}
   Flt  & pixF (Int x, Int y, Int z) {return *(Flt  *)(_data + x*SIZE(Flt  ) + y*_pitch + z*_pitch2);}   Flt  & pixF (C VecI &v) {return pixF (v.x, v.y, v.z);}
   Vec2 & pixF2(Int x, Int y, Int z) {return *(Vec2 *)(_data + x*SIZE(Vec2 ) + y*_pitch + z*_pitch2);}   Vec2 & pixF2(C VecI &v) {return pixF2(v.x, v.y, v.z);}
   Vec  & pixF3(Int x, Int y, Int z) {return *(Vec  *)(_data + x*SIZE(Vec  ) + y*_pitch + z*_pitch2);}   Vec  & pixF3(C VecI &v) {return pixF3(v.x, v.y, v.z);}
   Vec4 & pixF4(Int x, Int y, Int z) {return *(Vec4 *)(_data + x*SIZE(Vec4 ) + y*_pitch + z*_pitch2);}   Vec4 & pixF4(C VecI &v) {return pixF4(v.x, v.y, v.z);}

 C Byte & pixB (Int x, Int y)C {return *(Byte *)(_data + x*SIZE(Byte ) + y*_pitch);}   C Byte & pixB (C VecI2 &v)C {return pixB (v.x, v.y);}
 C UInt & pix  (Int x, Int y)C {return *(UInt *)(_data + x*SIZE(UInt ) + y*_pitch);}   C UInt & pix  (C VecI2 &v)C {return pix  (v.x, v.y);}
 C Color& pixC (Int x, Int y)C {return *(Color*)(_data + x*SIZE(Color) + y*_pitch);}   C Color& pixC (C VecI2 &v)C {return pixC (v.x, v.y);}
 C VecB & pixB3(Int x, Int y)C {return *(VecB *)(_data + x*SIZE(VecB ) + y*_pitch);}   C VecB & pixB3(C VecI2 &v)C {return pixB3(v.x, v.y);}
 C VecB4& pixB4(Int x, Int y)C {return *(VecB4*)(_data + x*SIZE(VecB4) + y*_pitch);}   C VecB4& pixB4(C VecI2 &v)C {return pixB4(v.x, v.y);}
 C Flt  & pixF (Int x, Int y)C {return *(Flt  *)(_data + x*SIZE(Flt  ) + y*_pitch);}   C Flt  & pixF (C VecI2 &v)C {return pixF (v.x, v.y);}
 C Vec2 & pixF2(Int x, Int y)C {return *(Vec2 *)(_data + x*SIZE(Vec2 ) + y*_pitch);}   C Vec2 & pixF2(C VecI2 &v)C {return pixF2(v.x, v.y);}
 C Vec  & pixF3(Int x, Int y)C {return *(Vec  *)(_data + x*SIZE(Vec  ) + y*_pitch);}   C Vec  & pixF3(C VecI2 &v)C {return pixF3(v.x, v.y);}
 C Vec4 & pixF4(Int x, Int y)C {return *(Vec4 *)(_data + x*SIZE(Vec4 ) + y*_pitch);}   C Vec4 & pixF4(C VecI2 &v)C {return pixF4(v.x, v.y);}

 C Byte & pixB (Int x, Int y, Int z)C {return *(Byte *)(_data + x*SIZE(Byte ) + y*_pitch + z*_pitch2);}   C Byte & pixB (C VecI &v)C {return pixB (v.x, v.y, v.z);}
 C UInt & pix  (Int x, Int y, Int z)C {return *(UInt *)(_data + x*SIZE(UInt ) + y*_pitch + z*_pitch2);}   C UInt & pix  (C VecI &v)C {return pix  (v.x, v.y, v.z);}
 C Color& pixC (Int x, Int y, Int z)C {return *(Color*)(_data + x*SIZE(Color) + y*_pitch + z*_pitch2);}   C Color& pixC (C VecI &v)C {return pixC (v.x, v.y, v.z);}
 C VecB & pixB3(Int x, Int y, Int z)C {return *(VecB *)(_data + x*SIZE(VecB ) + y*_pitch + z*_pitch2);}   C VecB & pixB3(C VecI &v)C {return pixB3(v.x, v.y, v.z);}
 C VecB4& pixB4(Int x, Int y, Int z)C {return *(VecB4*)(_data + x*SIZE(VecB4) + y*_pitch + z*_pitch2);}   C VecB4& pixB4(C VecI &v)C {return pixB4(v.x, v.y, v.z);}
 C Flt  & pixF (Int x, Int y, Int z)C {return *(Flt  *)(_data + x*SIZE(Flt  ) + y*_pitch + z*_pitch2);}   C Flt  & pixF (C VecI &v)C {return pixF (v.x, v.y, v.z);}
 C Vec2 & pixF2(Int x, Int y, Int z)C {return *(Vec2 *)(_data + x*SIZE(Vec2 ) + y*_pitch + z*_pitch2);}   C Vec2 & pixF2(C VecI &v)C {return pixF2(v.x, v.y, v.z);}
 C Vec  & pixF3(Int x, Int y, Int z)C {return *(Vec  *)(_data + x*SIZE(Vec  ) + y*_pitch + z*_pitch2);}   C Vec  & pixF3(C VecI &v)C {return pixF3(v.x, v.y, v.z);}
 C Vec4 & pixF4(Int x, Int y, Int z)C {return *(Vec4 *)(_data + x*SIZE(Vec4 ) + y*_pitch + z*_pitch2);}   C Vec4 & pixF4(C VecI &v)C {return pixF4(v.x, v.y, v.z);}

   // !! warning: 'gather' methods are written for speed and not safety, they assume that image is locked and that offsets are in range, these methods set 'pixels/colors' array from image values, coordinates are specified in the 'offset' parameters !!
   void gather(Flt   *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C;
   void gather(VecB  *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C;
   void gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C;
   void gather(Vec4  *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C;
   void gather(Flt   *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C;
   void gather(VecB  *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C;
   void gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C;
   void gather(Vec4  *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C;

   // fit
   Rect fit        (C Rect &rect, FIT_MODE fit=FIT_FULL)C {return Fit(   aspect(), rect, fit);} // get rectangle that can be used for drawing of the image to the 'rect' destination while preserving image proportions according to specified 'fit' mode
   Rect fitVertical(C Rect &rect, FIT_MODE fit=FIT_FULL)C {return Fit(invAspect(), rect, fit);} // get rectangle that can be used for drawing of the image to the 'rect' destination while preserving image proportions according to specified 'fit' mode

   // draw
   void draw        (                                    C Rect &rect)C;
   void draw        (C Color &color, C Color &color_add, C Rect &rect)C;
   void drawVertical(                                    C Rect &rect)C; // draw with texture coordinates in vertical mode
   void drawVertical(C Color &color, C Color &color_add, C Rect &rect)C; // draw with texture coordinates in vertical mode

   // draw to fit best in given space, while preserving image proportions
   void drawFit        (                                    C Rect &rect)C {return draw        (                  fit        (rect));}
   void drawFit        (C Color &color, C Color &color_add, C Rect &rect)C {return draw        (color, color_add, fit        (rect));}
   void drawFitVertical(C Color &color, C Color &color_add, C Rect &rect)C {return drawVertical(color, color_add, fitVertical(rect));}

   // draw to fullscreen
   void drawFs(                                                FIT_MODE fit=FIT_FULL, Int filter=-1)C; // draw to fullscreen, 'filter'=custom filtering (FILTER_TYPE) for this parameter you can set any of the FILTER_TYPE enums or use -1 to let the engine decide on filtering (for example linear filtering will be used on Mobile platforms and better filtering on other platforms)
   void drawFs(C Color &color, C Color &color_add=TRANSPARENT, FIT_MODE fit=FIT_FULL, Int filter=-1)C; // draw to fullscreen, 'filter'=custom filtering (FILTER_TYPE) for this parameter you can set any of the FILTER_TYPE enums or use -1 to let the engine decide on filtering (for example linear filtering will be used on Mobile platforms and better filtering on other platforms)

   // draw only part of the image
   void drawPart        (                                    C Rect &screen_rect, C Rect &tex_rect)C;
   void drawPart        (C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect)C;
   void drawPartVertical(                                    C Rect &screen_rect, C Rect &tex_rect)C; // draw with texture coordinates in vertical mode
   void drawPartVertical(C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect)C; // draw with texture coordinates in vertical mode

   // draw rotated
   void drawRotate(                                    C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center=null)C;
   void drawRotate(C Color &color, C Color &color_add, C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center=null)C;

   // draw masked
   void drawMask(C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect)C;

   // draw image as tiled background, 'tex_scale'=texture coordinates scaling
   void drawTile(                                    C Rect &rect, Flt tex_scale=1)C;
   void drawTile(C Color &color, C Color &color_add, C Rect &rect, Flt tex_scale=1)C;

   // draw image as rectangle's border
   void drawBorder(                                    C Rect &rect, Flt border=0.02f, Flt tex_scale=1, Flt tex_offset=0, Bool wrap_mode=false)C;
   void drawBorder(C Color &color, C Color &color_add, C Rect &rect, Flt border=0.02f, Flt tex_scale=1, Flt tex_offset=0, Bool wrap_mode=false)C;

   // draw stretched image from 3x3 parts
   void draw3x3        (C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac=0.25f)C; // 'color'=color that will be multiplied by the texture, 'color_add'=color that will be added to the texture using following formula "final_color = texture_color * color + color_add", 'rect'=screen rectangle at which the image will be drawn, 'border_size'=size of the border inside the screen 'rect' rectangle that will be drawn as image borders (this value should be less than rectangle dimensions), 'tex_frac'=fraction of the image texture that will be used for drawing the borders (this value should be in range of 0 .. 0.5)
   void draw3x3Vertical(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac=0.25f)C; // 'color'=color that will be multiplied by the texture, 'color_add'=color that will be added to the texture using following formula "final_color = texture_color * color + color_add", 'rect'=screen rectangle at which the image will be drawn, 'border_size'=size of the border inside the screen 'rect' rectangle that will be drawn as image borders (this value should be less than rectangle dimensions), 'tex_frac'=fraction of the image texture that will be used for drawing the borders (this value should be in range of 0 .. 0.5), this function will draw with texture coordinates in vertical mode
#if EE_PRIVATE
   void draw3x3Borders (C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac=0.25f)C; // draws only button borders, without the center
#endif

   // draw with custom filtering
   void drawFilter(                                    C Rect &rect, FILTER_TYPE filter=FILTER_BEST)C; // this method will draw the image with custom filtering which allows to achieve better quality than linear filtering, the pixel shader for better filters is very expensive, therefore use it only if performance is not critical and just for few images (for example displaying one image on the screen), this method supports only FILTER_LINEAR, FILTER_CUBIC_FAST and FILTER_CUBIC
   void drawFilter(C Color &color, C Color &color_add, C Rect &rect, FILTER_TYPE filter=FILTER_BEST)C; // this method will draw the image with custom filtering which allows to achieve better quality than linear filtering, the pixel shader for better filters is very expensive, therefore use it only if performance is not critical and just for few images (for example displaying one image on the screen), this method supports only FILTER_LINEAR, FILTER_CUBIC_FAST and FILTER_CUBIC

   // draw cube face
   void drawCubeFace(C Color &color, C Color &color_add, C Rect &rect, DIR_ENUM face)C;

   // draw in 3D space
   void draw3D(C Color &color, Flt size, Flt angle, C Vec &pos, ALPHA_MODE mode=ALPHA_BLEND_DEC)C; // draw as 3D billboard, this can be called in RM_BLEND mode or outside rendering function (in drawing function), this method supports only IMAGE_2D images, this relies on active object matrix which can be set using 'SetMatrix' function

   void drawVolume(C Color &color, C Color &color_add, C OBox &obox, Flt voxel_density_factor=0.01f, Flt precision=1.0f, Int min_steps=2, Int max_steps=64)C; // draw as 3D volumetric image, 'voxel_density_factor'=density factor of a single voxel (0..1), 'precision'=number of steps per voxel (0..Inf), 'min_steps max_steps'=minimum and maximum number of steps (2..1024), this method can be called in RM_CLOUD or RM_BLEND rendering modes, this method supports only IMAGE_3D images

   // io
   void operator=(C Str  &name) ; // load, Exit  on fail
   void operator=(C UID  &id  ) ; // load, Exit  on fail
#if EE_PRIVATE
   Bool saveData (  File &f                                                                                )C; // save, false on fail
   Bool loadData (  File &f, IMAGE_TYPE type_on_fail=IMAGE_DEFAULT, ImageHeader *header=null, C Str &name=S) ; // load, false on fail
#endif
   Bool save     (C Str  &name)C; // save, false on fail
   Bool load     (C Str  &name) ; // load, false on fail
   Bool save     (  File &f   )C; // save, false on fail
   Bool load     (  File &f   ) ; // load, false on fail

#if EE_PRIVATE
   Bool ImportTGA   (C Str &name, Int type   , Int mode=-1, Int mip_maps=-1);   Bool ImportTGA   (File &f, Int type, Int mode=-1, Int mip_maps=-1);
   Bool ImportDDS   (C Str &name, Int type   , Int mode=-1, Int mip_maps=-1);   Bool ImportDDS   (File &f, Int type, Int mode=-1, Int mip_maps=-1);
   Bool ImportBMPRaw( File &f   , Bool ico=false                           );   Bool ExportBMPRaw(File &f, Byte byte_pp, Bool ico=false)C;
#endif

   Bool   ImportTry(C Str  &name, Int type=-1, Int mode=-1, Int mip_maps=-1); // import BMP PNG JPG WEBP TGA TIF DDS PSD ICO, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, false on fail
   Image& Import   (C Str  &name, Int type=-1, Int mode=-1, Int mip_maps=-1); // import BMP PNG JPG WEBP TGA TIF DDS PSD ICO, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, Exit  on fail
   Bool   ImportTry(  File &f   , Int type=-1, Int mode=-1, Int mip_maps=-1); // import BMP PNG JPG WEBP TGA TIF DDS PSD ICO, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, false on fail
   Image& Import   (  File &f   , Int type=-1, Int mode=-1, Int mip_maps=-1); // import BMP PNG JPG WEBP TGA TIF DDS PSD ICO, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, Exit  on fail

   Bool Export(C Str &name, Flt rgb_quality=-1, Flt alpha_quality=-1, Flt compression_level=-1, Int sub_sample=-1)C; // export according to extension, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=best quality), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=best quality), 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size), 'sub_sample'=0..2 (chroma sub-sampling for RGB images, 0=none, 1=half, 2=quarter, -1=default)

   Bool   ImportCubeTry(C Image &right, C Image &left, C Image &up, C Image &down, C Image &forward, C Image &back, Int type=-1, Int mip_maps=1, Bool resize_to_pow2=true, FILTER_TYPE filter=FILTER_BEST); // import                                      as cube texture, 'type'=IMAGE_TYPE (-1=keep original value), false on fail
   Bool   ImportCubeTry(C Str   &right, C Str   &left, C Str   &up, C Str   &down, C Str   &forward, C Str   &back, Int type=-1, Int mip_maps=1, Bool resize_to_pow2=true, FILTER_TYPE filter=FILTER_BEST); // import BMP PNG JPG WEBP TGA TIF DDS PSD ICO as cube texture, 'type'=IMAGE_TYPE (-1=keep original value), false on fail
   Image& ImportCube   (C Str   &right, C Str   &left, C Str   &up, C Str   &down, C Str   &forward, C Str   &back, Int type=-1, Int mip_maps=1, Bool resize_to_pow2=true, FILTER_TYPE filter=FILTER_BEST); // import BMP PNG JPG WEBP TGA TIF DDS PSD ICO as cube texture, 'type'=IMAGE_TYPE (-1=keep original value), Exit  on fail

   Bool ImportBMP (C Str  &name                                          ) ; // import    BMP  from file, false on fail
   Bool ImportBMP (  File &f                                             ) ; // import    BMP  from file, false on fail
   Bool ExportBMP (C Str  &name                                          )C; // export as BMP  to   file, false on fail
   Bool ExportBMP (  File &f                                             )C; // export as BMP  to   file, false on fail
   Bool ImportPNG (C Str  &name                                          ) ; // import    PNG  from file, false on fail
   Bool ImportPNG (  File &f                                             ) ; // import    PNG  from file, false on fail
   Bool ExportPNG (C Str  &name, Flt compression_level=-1                )C; // export as PNG  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
   Bool ExportPNG (  File &f   , Flt compression_level=-1                )C; // export as PNG  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
   Bool ImportJPG (C Str  &name                                          ) ; // import    JPG  from file, false on fail
   Bool ImportJPG (  File &f                                             ) ; // import    JPG  from file, false on fail
   Bool ExportJPG (C Str  &name, Flt quality=-1, Int sub_sample=-1       )C; // export as JPG  to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=best quality), 'sub_sample'=0..2 (chroma sub-sampling for RGB images, 0=none, 1=half, 2=quarter, -1=default)
   Bool ExportJPG (  File &f   , Flt quality=-1, Int sub_sample=-1       )C; // export as JPG  to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=best quality), 'sub_sample'=0..2 (chroma sub-sampling for RGB images, 0=none, 1=half, 2=quarter, -1=default)
   Bool ImportWEBP(C Str  &name                                          ) ; // import    WEBP from file, false on fail
   Bool ImportWEBP(  File &f                                             ) ; // import    WEBP from file, false on fail
   Bool ExportWEBP(C Str  &name, Flt rgb_quality=-1, Flt alpha_quality=-1)C; // export as WEBP to   file, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=lossless), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=lossless)
   Bool ExportWEBP(  File &f   , Flt rgb_quality=-1, Flt alpha_quality=-1)C; // export as WEBP to   file, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=lossless), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=lossless)
   Bool ImportTGA (C Str  &name                                          ) ; // import    TGA  from file, false on fail
   Bool ImportTGA (  File &f                                             ) ; // import    TGA  from file, false on fail
   Bool ExportTGA (C Str  &name                                          )C; // export as TGA  to   file, false on fail
   Bool ExportTGA (  File &f                                             )C; // export as TGA  to   file, false on fail
   Bool ImportTIF (C Str  &name                                          ) ; // import    TIF  from file, false on fail
   Bool ImportTIF (  File &f                                             ) ; // import    TIF  from file, false on fail
   Bool ExportTIF (C Str  &name, Flt compression_level=-1                )C; // export as TIF  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
   Bool ExportTIF (  File &f   , Flt compression_level=-1                )C; // export as TIF  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
   Bool ImportDDS (C Str  &name                                          ) ; // import    DDS  from file, false on fail
   Bool ImportDDS (  File &f                                             ) ; // import    DDS  from file, false on fail
   Bool ExportDDS (C Str  &name                                          )C; // export as DDS  to   file, false on fail
   Bool ImportPSD (C Str  &name                                          ) ; // import    PSD  from file, false on fail
   Bool ImportPSD (  File &f                                             ) ; // import    PSD  from file, false on fail
   Bool ImportICO (C Str  &name                                          ) ; // import    ICO  from file, false on fail
   Bool ImportICO (  File &f                                             ) ; // import    ICO  from file, false on fail
   Bool ExportICO (C Str  &name                                          )C; // export as ICO  to   file, false on fail
   Bool ExportICO (  File &f                                             )C; // export as ICO  to   file, false on fail
   Bool ExportICNS(C Str  &name                                          )C; // export as ICNS to   file, false on fail
   Bool ExportICNS(  File &f                                             )C; // export as ICNS to   file, false on fail

            Image& operator=(C Image &src); // create from 'src' image using 'copy' method, Exit on fail
           ~Image() {del();}
            Image();
            Image(C Image &src                                                         ); // create from 'src' image using 'copy' method, Exit on fail
   explicit Image(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps=0); // create with specified parameters using 'create' method, Exit on fail, 'mip_maps'=number of mip-maps (0=autodetect)

#if EE_PRIVATE
   void     zero     () {Zero(T);}
   Image&   setInfo  (Int w, Int h, Int d, Int        type, IMAGE_MODE mode);
   Image& forceInfo  (Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode);
   void   setPartial ();
   void   setGLParams();
   void   setGLFont  ();

   Bool   map    ();
   void unmap    ();
   void duplicate(C Image &src);

   void discard();
#if DX9
   void clearHw      (C Color &color=TRANSPARENT                                                                                                           ) ; // hardware render target  clear
// there's no 'clearDS' on DX9
#elif DX11
   void clearHw      (C Vec4 &color=Vec4Zero                                                                                                               ) ; // hardware render target  clear
   void clearDS      (  Byte  s    =0                                                                                                                      ) ; // hardware depth  stencil clear
#else
// there's no 'clearHw' on OpenGL
// there's no 'clearDS' on OpenGL
#endif
   void clearFull    (C Vec4 &color=Vec4Zero, Bool restore_rt=false                                                                                        ) ; // clear full          area
   void clearViewport(C Vec4 &color=Vec4Zero, Bool restore_rt=false                                                                                        ) ; // clear main viewport area
   Bool copySoft     (  Image &dest, FILTER_TYPE filter=FILTER_BEST, Bool clamp=true, Bool alpha_weight=false, Bool keep_edges=false, Flt sharp_smooth=1.0f)C; // software             copy, 'sharp_smooth'=factor affecting sharpness/smoothness (0..Inf, the closer to 0.0 then the sharper result, the bigger than 1.0 then the more blurry result, default=1.0)
   void copyMs       (  Image &dest, Bool restore_rt, Bool multi_sample, C RectI *rect    =null                                                            )C; // multi sample texture copy, 'multi_sample'=average samples when writing to single sample, or copy texture on a per sample basis, this is needed because multi-sampled textures can't be sampled smoothly in the shader, this assumes that both source and dest are of the same size
   void copyMs       (  Image &dest, Bool restore_rt, Bool multi_sample, C Rect  &rect                                                                     )C; // multi sample texture copy, 'multi_sample'=average samples when writing to single sample, or copy texture on a per sample basis, this is needed because multi-sampled textures can't be sampled smoothly in the shader, this assumes that both source and dest are of the same size
   void copyHw       (  Image &dest, Bool restore_rt,                    C RectI *rect_src=null, C RectI *rect_dest=null, Bool *flipped=null               )C; // hardware     texture copy
   void copyHw       (  Image &dest, Bool restore_rt,                    C Rect  &rect                                                                     )C; // hardware     texture copy
   Bool capture      (C Image &src);
#endif

#if !EE_PRIVATE
private:
#endif
   IMAGE_TYPE _type, _hw_type;
   IMAGE_MODE _mode;
    LOCK_MODE _lock_mode;
     DIR_ENUM _lcf;
   Byte       _mms, _samples, _lmm, _byte_pp;
   Bool       _partial, _discard;
   Int        _lock_count;
   UInt       _pitch, _pitch2;
   VecI       _size, _hw_size, _lock_size;
   Vec        _part;
   Byte      *_data, *_data_all;
#if EE_PRIVATE
   GPU_API(IDirect3DSurface9       *_surf, ID3D11Texture2D          *_txtr, union{UInt _txtr; Ptr _txtr_ptr;});
   GPU_API(IDirect3DBaseTexture9   *_base, ID3D11Texture3D          *_vol , union{UInt _rb  ; Ptr _rb_ptr  ;});
   GPU_API(IDirect3DTexture9       *_txtr, ID3D11ShaderResourceView *_srv , union{UInt _w_s ; Ptr _w_s_ptr ;});
   GPU_API(IDirect3DVolumeTexture9 *_vol , ID3D11RenderTargetView   *_rtv , union{UInt _w_t ; Ptr _w_t_ptr ;});
   GPU_API(IDirect3DCubeTexture9   *_cube, ID3D11DepthStencilView   *_dsv , union{UInt _w_r ; Ptr _w_r_ptr ;});
   GPU_API(Ptr                      _rdsv, ID3D11DepthStencilView   *_rdsv, union{            Ptr _rdsv    ;});
#else
   Ptr        _ptr[6];
#endif
};
/******************************************************************************/
DECLARE_CACHE(Image, Images, ImagePtr); // 'Images' cache storing 'Image' objects which can be accessed by 'ImagePtr' pointer
/******************************************************************************/
struct ImageHeader
{
   VecI       size;
   Int        mip_maps;
   IMAGE_TYPE type;
   IMAGE_MODE mode;

   void           zero() {size.zero(); mip_maps=0; type=IMAGE_NONE; mode=IMAGE_2D;}
   ImageHeader() {zero();}
};
Bool ImageLoadHeader(  File &f   , ImageHeader &header); // load image header from file, this method is faster than 'Image.load' if you're not interested in the Image data, but only in information about it. After reading the header, 'f' file position is reset to the same place before making this call, false on fail
Bool ImageLoadHeader(C Str  &name, ImageHeader &header); // load image header from file, this method is faster than 'Image.load' if you're not interested in the Image data, but only in information about it.                                                                                                 false on fail
/******************************************************************************/
struct ImageCompare
{
   Bool skipped ; // if comparison was interrupted due to 'skip_dif'
   Flt  max_dif , // max     difference between colors, 0..1
        avg_dif , // average difference between colors, 0..1
        avg_dif2, // average difference between colors, 0..1 (using square method)
        similar , // fraction of the image that is similar (difference is below 'similar_dif'), 0..1
        psnr    ; // peak signal to noise ratio, 0..Inf

   Bool compare(C Image &a, C Image &b, Flt similar_dif=0.01f, Bool alpha_weight=false, Int a_mip=0, Flt skip_dif=1.0f); // compare 2 images, 'similar_dif'=limit used to determine that colors are similar, 'alpha_weight'=if use alpha channel as the weight, 'a_mip'=mip map of 'a' image used for comparison (based on this value, mip map of 'b' image will be selected to match the size of selected 'a' mip map), 'skip_dif'=if a single color difference exceeds this limit then skip remaining processing and return as soon as possible (value >=1 disables skipping), this function returns true on success and false on fail (if couldn't find matching mip map size between images, or if failed to lock the images)
};
/******************************************************************************/
IMAGE_TYPE BytesToImageType(Int byte_pp); // get IMAGE_TYPE needed to store 'byte_pp' amount of bytes per pixel, which is 1->IMAGE_I8, 2->IMAGE_I16, 3->IMAGE_I24, 4->IMAGE_I32, other->IMAGE_NONE

IMAGE_TYPE ImageTypeIncludeAlpha(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has    alpha channel
IMAGE_TYPE ImageTypeExcludeAlpha(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has no alpha channel

Int  GetPVRTCQuality(           ); // get PVRTC compression quality, 0..4
void SetPVRTCQuality(Int quality); // set PVRTC compression quality, 0..4
#if EE_PRIVATE
extern const ImagePtr ImageNull;

Int                                   PaddedWidth      (Int w, Int h,        Int mip, IMAGE_TYPE type);
Int                                   PaddedHeight     (Int w, Int h,        Int mip, IMAGE_TYPE type);
Int                                   ImagePitch       (Int w, Int h,        Int mip, IMAGE_TYPE type);
Int                                   ImageBlocksY     (Int w, Int h,        Int mip, IMAGE_TYPE type);
Int                                   ImageMipSize     (Int w, Int h,        Int mip, IMAGE_TYPE type);
Int                                   ImageMipSize     (Int w, Int h, Int d, Int mip, IMAGE_TYPE type);
UInt                                  ImageSize        (Int w, Int h, Int d,          IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps);
GPU_API(D3DFORMAT, DXGI_FORMAT, UInt) ImageTypeToFormat(Int                                   type  ); // convert from IMAGE_TYPE to API_FORMAT
IMAGE_TYPE                            ImageFormatToType(GPU_API(D3DFORMAT, DXGI_FORMAT, UInt) format); // convert from API_FORMAT to IMAGE_TYPE
Int                                   TotalMipMaps     (Int w, Int h, Int d, IMAGE_TYPE type);
Bool                                  CompatibleLock   (LOCK_MODE cur, LOCK_MODE lock); // if 'lock' is okay to be applied when 'cur' is already applied
void                                  InitSRGB         ();
#if WINDOWS
   HICON CreateIcon(C Image &image, C VecI2 *cursor_hot_spot=null); // 'cursor_hot_spot'=if this is specified then the icon is treated as a mouse cursor with given hot spot, otherwise it's a normal icon
#endif
#if GL
   UInt SourceGLFormat(IMAGE_TYPE type);
   UInt SourceGLType  (IMAGE_TYPE type);
#endif
#endif
/******************************************************************************/
