/******************************************************************************

   Use 'Display' to handle all display related functions.
   Including:
      -setting rendering options
      -handling basic 2D drawing

/******************************************************************************/
enum ASPECT_MODE : Byte // Aspect Ratio Mode, controls setting the sizes of screen coordinates - D.w and D.h
{
   ASPECT_Y      , // sets D.h to 1.0, D.w will be proportional to D.h depending on the display aspect and selected resolution
   ASPECT_X      , // sets D.w to 1.0, D.h will be proportional to D.w depending on the display aspect and selected resolution
   ASPECT_SMALLER, // this mode is useful for Mobile devices which can be rotated and changed aspect will not affect the display scale, 1.0 will be set to D.w and D.h will be set proportionally (if width is smaller than height) and 1.0 will be set to D.h and D.w will be set proportionally (if height is smaller than width)
   ASPECT_NUM    , // number of aspect ratio modes
};
enum BUMP_MODE : Byte // Bump Mapping Mode
{
   BUMP_FLAT    , // flat
   BUMP_NORMAL  , // normal
   BUMP_PARALLAX, // parallax
   BUMP_RELIEF  , // relief
   BUMP_NUM     , // number of bump mapping modes
};
enum AMBIENT_MODE : Byte // Ambient Occlusion Mode
{
   AMBIENT_FLAT , // flat
   AMBIENT_LOW  , // low    quality
   AMBIENT_MED  , // medium quality
   AMBIENT_HIGH , // high   quality
   AMBIENT_ULTRA, // ultra  quality
   AMBIENT_NUM  , // number of ambient modes
};
enum AMBIENT_SOFT_MODE : Byte
{
   AMBIENT_SOFT_NUM=6,
};
enum SHADOW_MODE : Byte // Shadowing Mode
{
   SHADOW_NONE, // none
   SHADOW_MAP , // shadow mapping
   SHADOW_NUM , // number of shadowing modes
};
enum SHADOW_SOFT_MODE : Byte
{
   SHADOW_SOFT_NUM=6,
};
enum MOTION_MODE : Byte // Motion Blur Mode
{
   MOTION_NONE         , // none
   MOTION_CAMERA       , // screen is blurred according to camera velocities only, objects velocities are not taken into account, objects are treated as if they were all stationary (their velocity is zero)
   MOTION_CAMERA_OBJECT, // screen is blurred according to camera and object velocities, available only in RT_DEFERRED renderer
   MOTION_NUM          , // number of motion blur modes
};
enum DILATE_MODE : Byte // Motion Blur Velocity Dilate Mode
{
   DILATE_ORTHO , // orthogonal mode is the fastest, but makes diagonal velocities a bit shorter
   DILATE_MIXED , // achieves quality and performance between DILATE_ORTHO and DILATE_ORTHO2
   DILATE_ORTHO2, // slower than DILATE_ORTHO but handles diagonal velocities better
   DILATE_ROUND , // best quality but slowest
   DILATE_NUM   , // number of dilate modes
};
enum DOF_MODE : Byte // Depth of Field Mode
{
   DOF_NONE    , // none
   DOF_GAUSSIAN, // based on Gaussian Blur, fast but not realistic
   DOF_NUM     , // number of depth of field modes
};
enum EDGE_DETECT_MODE : Byte // Edge Detect Mode
{
   EDGE_DETECT_NONE, // disabled
   EDGE_DETECT_THIN, // thin         edges
   EDGE_DETECT_FAT , // fat smoothed edges (slower)
   EDGE_DETECT_NUM , // number of edge detect modes
};
enum EDGE_SOFTEN_MODE : Byte // Edge Softening Mode
{
   EDGE_SOFTEN_NONE, // disabled
   EDGE_SOFTEN_FXAA, //     Fast Approximate   Anti Aliasing
#if SUPPORT_MLAA
   EDGE_SOFTEN_MLAA, //          Morphological Anti Aliasing
#endif
   EDGE_SOFTEN_SMAA, // Subpixel Morphological Anti Aliasing
   EDGE_SOFTEN_NUM , // number of edge softening modes
};
enum TEXTURE_USAGE : Byte // Information about using additional texture maps in material
{
   TEX_USE_DISABLE, // map is always disabled
   TEX_USE_SINGLE , // map is enabled for single materials only
   TEX_USE_MULTI  , // map is enabled for single materials and multiple materials which are blended together
   TEX_USE_NUM    , // number of texture maps usage modes
};
enum FOV_MODE : Byte // Field of View mode, determines calculating actual fov values
{
   FOV_Y , // vertical   fov will be taken from given value, horizontal fov will be calculated proportionally 
   FOV_X , // horizontal fov will be taken from given value, vertical   fov will be calculated proportionally
   FOV_XY, // both horizontal and vertical fov's will be taken from given value
#if EE_PRIVATE
   FOV_ORTHO_Y, // orthogonal, vertical   fov will be taken from 'fov', horizontal will be calculated proportionally
   FOV_ORTHO_X, // orthogonal, horizontal fov will be taken from 'fov', vertical   will be calculated proportionally
   FOV_ORTHO  , // orthogonal, custom     fov will be taken from 'fov'
#endif
};
enum SHADER_MODEL : Byte
{
   SM_UNKNOWN, // unknown
   SM_GL_ES_2, //           (OpenGL ES 2.0     )
   SM_GL_ES_3, //           (OpenGL ES 3.0     )
   SM_GL     , //           (OpenGL for Desktop)
   SM_3      , // Model 3.0 (DirectX  9        )
   SM_4      , // Model 4.0 (DirectX 10        )
   SM_4_1    , // Model 4.1 (DirectX 10.1      )
   SM_5      , // Model 5.0 (DirectX 11        )
};
#if EE_PRIVATE
inline Bool FovPerspective(FOV_MODE mode) {return mode< FOV_ORTHO_Y               ;} // if fov mode is perspective
inline Bool FovOrthogonal (FOV_MODE mode) {return mode>=FOV_ORTHO_Y               ;} // if fov mode is orthogonal
inline Bool FovHorizontal (FOV_MODE mode) {return mode==FOV_X || mode==FOV_ORTHO_X;} // if fov mode is horizontal
#endif
/******************************************************************************/
struct Display : DisplayState, DisplayDraw // Display Control
{
   Bool (*lost )(), // pointer to custom function (may be null) called when display is lost , it must return true on success and false on fail
        (*reset)(); // pointer to custom function (may be null) called when display is reset, it must return true on success and false on fail

   void (*screen_changed)(Flt old_width, Flt old_height); // pointer to custom function (may be null) called when screen proportions have changed

   Flt (*shadow_step)(Int i, Int num); // pointer to custom function (may be null) which allows to manually specify the fraction range of the i-th out of 'num' shadow map splits

   Int (*image_load_shrink)(ImageHeader &image_header, C Str &name); // pointer to custom function (may be null) called when 'Image' is being loaded from a File allowing to load the Image as smaller than how it is stored in the File. 'name'=file name of the image. You can use this for example when targeting devices with small amount of RAM and wishing to load the Images as half their size. Inside the function you should return by how many steps the final size should be divided by 2. For example, if an Image is being loaded with its original size = 1024x1024, based on returned value the final size will be adjusted accordingly: return value 0 -> 1024x1024 (original size), return value 1 -> 512x512 (half size), return value 2 -> 256x256 (quarter size), and so on.

   void (*set_shader)(); // pointer to custom function (may be null) called when Mesh shaders need to be updated, when changing some display options during application runtime, meshes require to have their shaders updated, Meshes created by using Cache are processed automatically, however manually created meshes need to be processed manually, to do this - create a global function which calls 'Mesh.setShader()' on all manually created meshes and then in 'InitPre' function set 'D.set_shader' to point to that function

   // get
 C Vec2&        size           ()C {return _size          ;} // get Application           Screen     Size     (in screen coordinates, depends on selected resolution, aspect mode and scale
   Flt          w              ()C {return _size.x        ;} // get Application           Screen     Width    (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 1.3)
   Flt          h              ()C {return _size.y        ;} // get Application           Screen     Height   (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 1.0)
 C Vec2&        size2          ()C {return _size2         ;} // get Application           Screen     Size  *2 (in screen coordinates, depends on selected resolution, aspect mode and scale
   Flt          w2             ()C {return _size2.x       ;} // get Application           Screen     Width *2 (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 2.6)
   Flt          h2             ()C {return _size2.y       ;} // get Application           Screen     Height*2 (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 2.0)
 C VecI2&       res            ()C {return _res           ;} // get Application           Resolution          (in pixels)
   Int          resW           ()C {return _res.x         ;} // get Application           Resolution Width    (in pixels)
   Int          resH           ()C {return _res.y         ;} // get Application           Resolution Height   (in pixels)
 C VecI2&       render         ()C {return _render_res    ;} // get Application Rendering Resolution          (in pixels), which is Resolution affected by 'density'
   Int          renderW        ()C {return _render_res.x  ;} // get Application Rendering Resolution Width    (in pixels), which is Resolution affected by 'density'
   Int          renderH        ()C {return _render_res.y  ;} // get Application Rendering Resolution Height   (in pixels), which is Resolution affected by 'density'
   VecI2        screen         ()C;                          // get          Main Display Screen              (in pixels) at the current moment
   Int          screenW        ()C {return  screen().x    ;} // get          Main Display Screen     Width    (in pixels) at the current moment
   Int          screenH        ()C {return  screen().y    ;} // get          Main Display Screen     Height   (in pixels) at the current moment
   Int          freq           ()C {return _freq_got      ;} // get               Display Screen     Frequency (refresh rate)
   SHADER_MODEL shaderModel    ()C {return _shader_model  ;} // get available Shader Model
   Str8         shaderModelName()C;                          // get available Shader Model in text format
   Str8         apiName        ()C;                          // get Rendering API          in text format
   Int          maxTexFilter   ()C {return _max_tex_filter;} // get maximum available Anisotropic Filtering
   Int          maxTexSize     ()C {return _max_tex_size  ;} // get maximum available Texture     Size
 C Mems<VecI2>& modes          ()C {return _modes         ;} // get fullscreen display modes, where x=width, y=height (in pixels)
 C Str8&        deviceName     ()C {return _device_name   ;} // get GPU device name
   Long         deviceMemory   ()C {return _device_mem    ;} // get GPU memory (-1 if unknown)
   Bool         canDraw        ()C {return _can_draw      ;} // if  drawing graphics is available (this can be false when APP_ALLOW_NO_GPU was enabled and GPU was not found)
   Bool         smallSize      ()C;                          // if  display device is of a small size (phone size)
   Flt          browserZoom    ()C;                          // get current browser zoom level (1.0=100%, 2.0=200%, etc), this is valid only for the WEB platform (others return 1.0)

   // settings
   Display& mode  (Int w=-1, Int h=-1, Int full=-1  );                 // set    Display Resolution  Mode, -1=keep original value, if the method fails then previous mode is restored, if previous mode can't be restored then Exit is called
   Display& toggle(           Bool window_size=false);                 // toggle Fullscreen/Windowed Mode, 'window_size'=if when switching to fullscreen want to set resolution from current window size instead of desktop size
   Display& full  (Bool full, Bool window_size=false);                 // set    Fullscreen Mode         , 'window_size'=if when switching to fullscreen want to set resolution from current window size instead of desktop size
   Bool     full  (                                 )C {return _full;} // if in  Fullscreen Mode (true/false, default=false)

#if EE_PRIVATE
   void     setSync          (                             );
   Bool     densityFast      (Byte             density     );
   void     densityUpdate    (                             );
   void     gammaSet         (                             );
                                                                Bool              shaderModelGLES2()C {return   GL_ES  && shaderModel()==SM_GL_ES_2;} // if shaderModel()==SM_GL_ES_2
                                                                Bool           notShaderModelGLES2()C {return (!GL_ES) || shaderModel()!=SM_GL_ES_2;} // if shaderModel()!=SM_GL_ES_2
                                                                Bool             densityUsed      ()C {return _density!=127  ;} // get if Density is != 1.0
                                                                Bool             densityUpsample  ()C {return _density> 127  ;} // get if Density is >  1.0
                                                                Byte             densityByte      ()C {return _density       ;} // get    Density Byte
                                                                Bool             multiSample      ()C {return _samples>1     ;} // get if Multi Sampling is used
                                                                Bool             highPrecNrmCalcIs()C {return highPrecNrmCalc() && !highPrecNrmRT();}
                                                                void             aspectRatioEx    (Bool force=true, Bool quiet=false);
                                                                Int              maxShaderMatrixes()C; // max number of matrixes that can be set in shaders
                                                                Bool             meshStorageSigned()C; // if supports signed storage for mesh vtx components
                                                                Bool             meshBoneSplit    ()C; // if requires usage of Bone Splits for meshes with big amount of bones
                                                                Bool             hwInstancing     ()C; // if hardware instancing is supported
                                                                Bool             signedNrmRT      ()C; // if Normal   Render Target is signed
                                                                Bool             signedVelRT      ()C; // if Velocity Render Target is signed
                                                                Bool         highMonitorPrecision ()C; // if high Monitor Precision
#endif
                                                                Rect             rect             ()C {return Rect(-w(), -h(), w(), h());} // get full screen rectangle
   Display& exclusive        (Bool             exclusive   );   Bool             exclusive        ()C {return _exclusive      ;} // get/set if fullscreen mode should be exclusive (true/false                         , default=             true                             ), this affects only Windows DirectX fullscreen mode, exclusive offers better performance, non-exclusive offers faster Alt+Tab switching
   Display& density          (Flt              density     );   Flt              density          ()C;                           // get/set Rendering Pixel Density                (0..2                               , default=                1                             ), density affects the number of pixels used during rendering, density=2 makes the rendering use 2x bigger render targets (super sampling) but slower performance, density=0.5 makes the rendering use 2x smaller render targets making the result more blurry but with faster performance, the change is NOT instant, avoid calling real-time
   Display& densityFilter    (FILTER_TYPE      filter      );   FILTER_TYPE      densityFilter    ()C {return _density_filter ;} // get/set Density Filtering                      (FILTER_TYPE                        , default=FILTER_CUBIC_FAST (FILTER_LINEAR   for Mobile)), density filter affects filtering used when up-scaling the image, this method supports only FILTER_NONE, FILTER_LINEAR, FILTER_CUBIC_FAST and FILTER_CUBIC, the change is instant, you can call it real-time
   Display& samples          (Byte             samples     );   Byte             samples          ()C {return _samples        ;} // set/get Sample Count for Multi Sampling AA     (1..16                              , default=                1                             ), multi-sampling for RT_DEFERRED renderer can be enabled only for ShaderModel>=4.1, enabling multi-sampling for ShaderModel<=4.0 disables all depth-based effects, the change is NOT instant, avoid calling real-time
   Display& scale            (Flt              scale       );   Flt              scale            ()C {return _scale          ;} // get/set Draw Scale                             (0..Inf                             , default=                1                             ), changing display scale affects display screen sizes D.w and D.h, the change is NOT instant, avoid calling real-time
   Display& aspectMode       (ASPECT_MODE      mode        );   ASPECT_MODE      aspectMode       ()C {return _aspect_mode    ;} // set/get Aspect Mode                            (ASPECT_MODE                        , default=         ASPECT_Y (ASPECT_SMALLER  for Mobile))
   Display& aspectRatio      (Flt              aspect_ratio);   Flt              aspectRatio      ()C {return _aspect_ratio   ;} // set/get Display Aspect Ratio                   (0=autodetect, 4/3, 5/4, 16/9, 16/10, default=                0                             )
   Display& bumpMode         (BUMP_MODE        mode        );   BUMP_MODE        bumpMode         ()C {return _bump_mode      ;} // set/get Bump Mapping Mode                      (BUMP_MODE                          , default=    BUMP_PARALLAX (BUMP_FLAT       for Mobile)), using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
   Display& gamma            (Flt              gamma       );   Flt              gamma            ()C {return _gamma          ;} // set/get Gamma value                            (-Inf..Inf                          , default=                0                             ), using gamma modifies hardware color processing and does not reduce rendering performance
   Display& monitorPrecision (IMAGE_PRECISION  precision   );   IMAGE_PRECISION  monitorPrecision ()C {return _monitor_prec   ;} // set/get                      Monitor Precision (IMAGE_PRECISION                    , default=IMAGE_PRECISION_8                             ), use this function to specify the exact precision of your Monitor Screen, use IMAGE_PRECISION_8 for 8-bit per-channel (24-bit total) screens, and IMAGE_PRECISION_10 for 10-bit per channel (30-bit total) screens, avoid setting higher precision than what your screen can actually support because instead of getting higher quality results you will get lower quality
   Display& highPrecColRT    (Bool             on          );   Bool             highPrecColRT    ()C {return _hp_col_rt      ;} // set/get Color     Render Target High Precision (true/false                         , default=            false                             ), enabling high precision render targets     gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Color     Render Target is used for storing colors                   in Deferred Renderer , this affects precision of color textures adjusted by material colors, the change is NOT instant, avoid calling real-time
   Display& highPrecNrmRT    (Bool             on          );   Bool             highPrecNrmRT    ()C {return _hp_nrm_rt      ;} // set/get Normal    Render Target High Precision (true/false                         , default=            false                             ), enabling high precision render targets     gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Normal    Render Target is used for storing normal vectors           in Deferred Renderer , this affects lighting calculations (especially specular) to be more precise, an alternative option to increase the light precision is to use 'highPrecNrmCalc' instead, the change is NOT instant, avoid calling real-time
   Display& highPrecLumRT    (Bool             on          );   Bool             highPrecLumRT    ()C {return _hp_lum_rt      ;} // set/get Light     Render Target High Precision (true/false                         , default=            false                             ), enabling high precision render targets     gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Light     Render Target is used for storing light  intensity         in Deferred Renderer , most importantly it allows to store light intensities with values higher than 1.0 (which is the limit for low precision render targets), the change is NOT instant, avoid calling real-time
   Display& litColRTPrecision(IMAGE_PRECISION  precision   );   IMAGE_PRECISION  litColRTPrecision()C {return _lit_col_rt_prec;} // set/get Lit Color Render Target      Precision (IMAGE_PRECISION                    , default=IMAGE_PRECISION_8                             ), enabling high precision render targets     gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Lit Color Render Target is used for storing colors adjusted by light in all      Renderers, the change is NOT instant, avoid calling real-time
   Display& highPrecNrmCalc  (Bool             on          );   Bool             highPrecNrmCalc  ()C {return _hp_nrm_calc    ;} // set/get Normal Calculation   in High Precision (true/false                         , default=             true                             ), enabling high precision normal calculation gives higher quality graphics at the cost of slower performance                      , high precision means that the Normal Vector from the Normal Render Target in Deferred Renderer will be reconstructed using more complex calculation, giving more precision but slightly slower performance, this affects lighting calculations (especially specular) to be more precise, this is ignored when 'highPrecNrmRT' is enabled as it's not needed in that case, the change is instant, you can call it real-time
   Display& dither           (Bool             on          );   Bool             dither           ()C {return _dither         ;} // set/get Color Dithering                        (true/false                         , default=             true                             ), the change is instant, you can call it real-time
   Display& sync             (Bool             sync        );   Bool             sync             ()C {return _sync           ;} // set/get Screen Synchronization                 (true/false                         , default=             true                             ), the change is NOT instant, avoid calling real-time
   Display& maxLights        (Byte             max_lights  );   Byte             maxLights        ()C {return _max_lights     ;} // set/get Maximum Number of Lights               (0=unlimited, 1..255                , default=                0                             ), this will automatically limit the number of lights on the scene, the change is instant, you can call it real-time
   Display& materialBlend    (Bool             per_pixel   );   Bool             materialBlend    ()C {return _mtrl_blend     ;} // set/get Multi Material Blending Quality        (true/false                         , default=             true                             ), false=per vertex, true=per pixel, in order to get correct results, your materials should have a bump map, the change is NOT instant, avoid calling real-time
   Display& texFilter        (Byte             filter      );   Byte             texFilter        ()C {return _tex_filter     ;} // set/get Texture Filtering Quality              (0..maxTexFilter()                  , default=               16 (2               for Mobile)), 0=no filtering, 1=linear filtering, 2 and more=anisotropic filtering, the change is NOT instant, avoid calling real-time
   Display& texMipFilter     (Bool             on          );   Bool             texMipFilter     ()C {return _tex_mip_filter ;} // set/get Texture MipMap Filtering               (true/false                         , default=             true (false           for Mobile)), enables or disables filtering between different mip map levels, the change is NOT instant, avoid calling real-time
   Display& texLod           (Byte             lod         );   Byte             texLod           ()C {return _tex_lod        ;} // set/get Texture Lod Level Quality              (0..16                              , default=                0                             ), values bigger than zero indicate lower texture quality (smaller mip maps will be used), but faster rendering, the change is NOT instant, avoid calling real-time
   Display& texMacro         (Bool             use         );   Bool             texMacro         ()C {return _tex_macro      ;} // set/get Macro      Textures usage              (true/false                         , default=             true                             ), determines usage of additional Macro      Textures in Materials, using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
   Display& texDetail        (TEXTURE_USAGE    usage       );   TEXTURE_USAGE    texDetail        ()C {return _tex_detail     ;} // set/get Detail     Textures usage              (TEXTURE_USAGE                      , default=    TEX_USE_MULTI (TEX_USE_DISABLE for Mobile)), determines usage of additional Detail     Textures in Materials, using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
   Display& texReflection    (TEXTURE_USAGE    usage       );   TEXTURE_USAGE    texReflection    ()C {return _tex_reflect    ;} // set/get Reflection Textures usage              (TEXTURE_USAGE                      , default=    TEX_USE_MULTI                             ), determines usage of additional Reflection Textures in Materials, using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), currently reflection textures do not support 'TEX_USE_MULTI' mode, the change is NOT instant, avoid calling real-time
   Display& fontSharpness    (Flt              value       );   Flt              fontSharpness    ()C {return _font_sharpness ;} // set/get Font Sharpness                         (-Inf..Inf                          , default=             0.75                             ), specifies Mip Map Texture Bias for the Font Images, value <0 decreases sharpness, value==0 doesn't change sharpness, value>0 increases sharpness, the change is NOT instant, avoid calling real-time
   Display& bendLeafs        (Bool             on          );   Bool             bendLeafs        ()C {return _bend_leafs     ;} // set/get Leafs Bending animation                (true/false                         , default=             true                             ), using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
   Display& outlineAffectSky (Bool             on          );   Bool          outlineAffectSky    ()C {return _outline_sky    ;} // set/get Mesh Outline Sky Affect                (true/false                         , default=            false                             ), if this is enabled then outline will be visible on sky pixels in EDGE_DETECT_FAT mode, enabling this option makes outline effect run slower, the change is instant, you can call it real-time
   Display& outlineMode      (EDGE_DETECT_MODE mode        );   EDGE_DETECT_MODE outlineMode      ()C {return _outline_mode   ;} // set/get Mesh Outline   Mode                    (EDGE_DETECT_MODE                   , default= EDGE_DETECT_THIN                             ), the change is instant, you can call it real-time
   Display& edgeDetect       (EDGE_DETECT_MODE mode        );   EDGE_DETECT_MODE edgeDetect       ()C {return _edge_detect    ;} // set/get Edge Detect    Mode                    (EDGE_DETECT_MODE                   , default= EDGE_DETECT_NONE                             ), the change is instant, you can call it real-time
   Display& edgeSoften       (EDGE_SOFTEN_MODE mode        );   EDGE_SOFTEN_MODE edgeSoften       ()C {return _edge_soften    ;} // set/get Edge Softening Mode                    (EDGE_SOFTEN_MODE                   , default= EDGE_SOFTEN_NONE                             ), this is fake Anti-Aliasing, it is not used when Multi Sampling is enabled, the change is instant, you can call it real-time
   Display& smaaThreshold    (Flt              threshold   );   Flt              smaaThreshold    ()C {return _smaa_threshold ;} // set/get Edge Softening SMAA Threshold          (   0..1                            , default=            0.1                               ), this affects SMAA edge softening quality, lower values give better quality, while higher values give better performance, recommended values: 0.05=high quality, 0.1=medium, 0.15=high performance
   Display& eyeDistance      (Flt              dist        );   Flt              eyeDistance      ()C {return _eye_dist       ;} // set/get distance between the eyes              (-Inf..Inf                          , default=            0.064                             ), interpupillary distance (distance between the eye pupils) used for stereoscopic rendering, the change is instant, you can call it real-time
   Display& drawNullMaterials(Bool             on          );   Bool           drawNullMaterials  ()C {return _draw_null_mtrl ;} // set/get if draw mesh parts with no material    (true/false                         , default=            false                             ), the change is NOT instant, avoid calling real-time
   Display& secondaryOpenGLContexts(Byte       contexts    );   Int         secondaryOpenGLContexts()C;                          // set/get number of secondary OpenGL contexts to create during application initialization (this does not include the main context for the main thread which is always created), each secondary context allows 1 secondary thread to perform operations on the GPU data, for more information please check the 'ThreadMayUseGPUData' global function, this should be called only in 'InitPre', after that, calls to this method are ignored, this value is used only for OpenGL renderer, for DirectX it is ignored, default=1
                                                                Bool canUseGPUDataOnSecondaryThread()C;                          //     get if display supports operating on GPU data on a secondary thread, this is always true for DirectX, for OpenGL it will be true only if secondary OpenGL contexts were successfully created during the display initialization
                                                         static Bool created                       ();                           //     get if display is created, this can be false when the Engine still hasn't finished initializing (before 'Init'), or on Linux if XDisplay is not available and APP_ALLOW_NO_XDISPLAY was specified

   // screen fading
   Bool      fading()C;                                      // if    fading is currently enabled
   void   setFade  (Flt seconds, Bool previous_frame=false); // start fading the nearest screen result for the following 'seconds', 'previous_frame'=if use result of previous frame instead of the next frame
   void clearFade  (                                      ); // clear any active fading, the change is instant, you can call it real-time

   // Color Palette
   Display& colorPaletteAllow(  Bool      on     );   Bool      colorPaletteAllow()C {return _color_palette_allow;} // set/get if RM_PALETTE/RM_PALETTE1 rendering modes are allowed, disabling them increases rendering performance, default=true (false for Mobile)
   Display& colorPalette     (C ImagePtr &palette); C ImagePtr& colorPalette     ()C {return _color_palette[0]   ;} // set/get color palette image that is used during RM_PALETTE  rendering mode, palette textures need to have a height of 4 pixels (width can be set freely), where each row represents an intensity color palette for respectively (red, green, blue, alpha) components when rendering in RM_PALETTE  mode, default=ImagePtr().get("Img/color palette.img")
   Display& colorPalette1    (C ImagePtr &palette); C ImagePtr& colorPalette1    ()C {return _color_palette[1]   ;} // set/get color palette image that is used during RM_PALETTE1 rendering mode, palette textures need to have a height of 4 pixels (width can be set freely), where each row represents an intensity color palette for respectively (red, green, blue, alpha) components when rendering in RM_PALETTE1 mode, default=null

   // Particles
   Display& particlesSoft      (Bool on);   Bool particlesSoft      ()C {return _particles_soft  ;} // set/get Particles Softing          (true/false, default=true (false for Mobile)), the change is instant, you can call it real-time
   Display& particlesSmoothAnim(Bool on);   Bool particlesSmoothAnim()C {return _particles_smooth;} // set/get Particles Smooth Animation (true/false, default=true (false for Mobile)), if enabled then particles with animated images will be displayed with better quality by smooth blending between animation frames, the change is instant, you can call it real-time

   // Eye Adaptation
   Display& eyeAdaptation          (  Bool on          );   Bool eyeAdaptation          ()C {return _eye_adapt           ;} // set/get Eye Adaptation usage                (                true/false                  , default=false        ), enables automatic screen brightness adjustment, the change is instant, you can call it real-time
   Display& eyeAdaptationBrightness(  Flt  brightness  );   Flt  eyeAdaptationBrightness()C {return _eye_adapt_brightness;} // set/get Eye Adaptation brightness           (                   0..Inf                   , default= 0.37        ), total scale of lighting, the change is instant, you can call it real-time
   Display& eyeAdaptationMaxDark   (  Flt  max_dark    );   Flt  eyeAdaptationMaxDark   ()C {return _eye_adapt_max_dark  ;} // set/get Eye Adaptation maximum   darkening  (                   0..eyeAdaptationMaxBright, default= 0.5         ), the change is instant, you can call it real-time
   Display& eyeAdaptationMaxBright (  Flt  max_bright  );   Flt  eyeAdaptationMaxBright ()C {return _eye_adapt_max_bright;} // set/get Eye Adaptation maximum brightening  (eyeAdaptationMaxDark..Inf                   , default= 2.0         ), the change is instant, you can call it real-time
   Display& eyeAdaptationSpeed     (  Flt  speed       );   Flt  eyeAdaptationSpeed     ()C {return _eye_adapt_speed     ;} // set/get Eye Adaptation speed                (                   1..Inf                   , default= 6.5         ), the change is instant, you can call it real-time
   Display& eyeAdaptationWeight    (C Vec &weight      ); C Vec& eyeAdaptationWeight    ()C {return _eye_adapt_weight    ;} // set/get Eye Adaptation color weight         (           (0, 0, 0)..(1, 1, 1)             , default=(0.9, 1, 0.7)), the change is instant, you can call it real-time
   Display& resetEyeAdaptation     (  Flt  brightness=1);                                                                   // reset   Eye Adaptation value, eye adaptation changes over time according to screen colors, this method resets the adaptation to its original state, 'brightness'=initial brightness (0..Inf), the change is NOT instant, avoid calling real-time

   // Bloom, setting 'original' value to 1 and 'scale' to 0 disables bloom and increases rendering performance, optionally you can disable it with "bloomAllow(false)"
   Display&  glowAllow   (Bool allow   );   Bool  glowAllow   ()C {return  _glow_allow   ;} // set/get Allow Glow Effect     (true/false, default=true  (false for Mobile)), this can work only if 'bloomAllow' is enabled, the change is instant, you can call it real-time
   Display& bloomAllow   (Bool allow   );   Bool bloomAllow   ()C {return _bloom_allow   ;} // set/get Allow Bloom           (true/false, default=true  (false for Mobile)), the change is instant, you can call it real-time
   Display& bloomOriginal(Flt  original);   Flt  bloomOriginal()C {return _bloom_original;} // set/get Bloom Original Color  (   0..Inf , default=1.0                     ), the change is instant, you can call it real-time
   Display& bloomScale   (Flt  scale   );   Flt  bloomScale   ()C {return _bloom_scale   ;} // set/get Bloom Scale           (   0..Inf , default=0.5                     ), the change is instant, you can call it real-time
   Display& bloomCut     (Flt  cut     );   Flt  bloomCut     ()C {return _bloom_cut     ;} // set/get Bloom Cutoff          (   0..Inf , default=0.3                     ), the change is instant, you can call it real-time
   Display& bloomSaturate(Bool saturate);   Bool bloomSaturate()C {return _bloom_sat     ;} // set/get Bloom Saturation      (true/false, default=false                   ), the change is instant, you can call it real-time, true=faster and saturates the colors, false=slower and preserves original colors
   Display& bloomMaximum (Bool on      );   Bool bloomMaximum ()C {return _bloom_max     ;} // set/get Bloom Maximum Filter  (true/false, default=false                   ), the change is instant, you can call it real-time
   Display& bloomHalf    (Bool half    );   Bool bloomHalf    ()C {return _bloom_half    ;} // set/get Bloom Half/Quarter    (true/false, default=true  (false for Mobile)), this specifies whether bloom should be calculated using half or quarter sized render targets (half is more precise but slower, quarter is more blurred), the change is instant, you can call it real-time
   Display& bloomBlurs   (Byte blurs   );   Byte bloomBlurs   ()C {return _bloom_blurs   ;} // set/get Bloom Number of Blurs (   0..4   , default=1                       ), the change is instant, you can call it real-time
   Display& bloomSamples (Bool high    );   Bool bloomSamples ()C {return _bloom_samples ;} // set/get Bloom Sample Count    (true/false, default=true  (false for Mobile)), if set to true then 6 texture reads are performed in the shader, if set to false then 4 texture reads are performed, the change is instant, you can call it real-time
                                            Bool bloomUsed    ()C;                          //     if  Bloom post process is going to be used

   // Ambient Light
#if EE_PRIVATE
   Bool aoWant()C;
#endif
   Display& ambientMode    (AMBIENT_MODE mode    );   AMBIENT_MODE ambientMode    ()C {return _amb_mode       ;} // set/get Ambient Mode           (AMBIENT_MODE         , default=AMBIENT_FLAT), the change is instant, you can call it real-time
   Display& ambientSoft    (  Byte       soft    );   Byte         ambientSoft    ()C {return _amb_soft       ;} // set/get Ambient Softing        (0..AMBIENT_SOFT_NUM-1, default=           1), if soften the AO result, the change is instant, you can call it real-time
   Display& ambientJitter  (  Bool       jitter  );   Bool         ambientJitter  ()C {return _amb_jitter     ;} // set/get Ambient Jittering      (true/false           , default=        true), jittering enables per pixel randomization of the AO samples, the change is instant, you can call it real-time
   Display& ambientNormal  (  Bool       normal  );   Bool         ambientNormal  ()C {return _amb_normal     ;} // set/get Ambient Normal Map Use (true/false           , default=        true), if include per pixel normal vectors from the render target, the change is instant, you can call it real-time
   Display& ambientRes     (  Flt        scale   );   Flt          ambientRes     ()C;                           // set/get Ambient Resolution     (  0..1               , default=         0.5), this determines the size of the buffers used for calculating the Ambient Occlusion effect, 1=full size, 0.5=half size, 0.25=quarter size, .., smaller sizes offer faster performance but worse quality, the change is NOT instant, avoid calling real-time
   Display& ambientPower   (  Flt        power   );   Flt          ambientPower   ()C {return _amb_color.max();} // set/get Ambient Power          (  0..2               , default=         0.4), this is equivalent to using 'ambientColor' with RGB components set to the same value, the change is instant, you can call it real-time
   Display& ambientColor   (C Vec       &color   ); C Vec&         ambientColor   ()C {return _amb_color      ;} // set/get Ambient Color          (  0..2               , default=         0.4), the change is instant, you can call it real-time
   Display& ambientContrast(  Flt        contrast);   Flt          ambientContrast()C {return _amb_contrast   ;} // set/get Ambient Contrast       (  0..Inf             , default=         1.2), the change is instant, you can call it real-time
   Display& ambientRange   (C Vec2      &range   ); C Vec2&        ambientRange   ()C {return _amb_range      ;} // set/get Ambient 2D Range       (  0..Inf             , default=         0.3), the change is instant, you can call it real-time
   Display& ambientScale   (  Flt        scale   );   Flt          ambientScale   ()C {return _amb_scale      ;} // set/get Ambient 3D Scale       (  0..Inf             , default=         2.5), the change is instant, you can call it real-time
   Display& ambientBias    (  Flt        bias    );   Flt          ambientBias    ()C {return _amb_bias       ;} // set/get Ambient Bias           (  0..1               , default=         0.3), the change is instant, you can call it real-time

   // Night Shade
   Display& nightShadeColor(C Vec &color);   C Vec& nightShadeColor()C {return _ns_color;} // set/get Night Shade color (0..1, default=0), the change is instant, you can call it real-time, setting color to 0 disables Night Shade effect

   // Shadowing
   Display& shadowMode         (SHADOW_MODE mode    );   SHADOW_MODE shadowMode         ()C {return _shd_mode      ;} // set/get Shadow Mode                                  (SHADOW_MODE         , default=SHADOW_MAP (SHADOW_NONE for Mobile)), the change is instant, you can call it real-time
   Display& shadowSoft         (Byte        soft    );   Byte        shadowSoft         ()C {return _shd_soft      ;} // set/get Shadow Softing                               (0..SHADOW_SOFT_NUM-1, default=         0                         ), available only in RT_DEFERRED renderer, the change is instant, you can call it real-time
   Display& shadowJitter       (Bool        jitter  );   Bool        shadowJitter       ()C {return _shd_jitter    ;} // set/get Shadow Jittering                             (true/false          , default=     false                         ), works best when combined with shadow softing, the change is instant, you can call it real-time
   Display& shadowReduceFlicker(Bool        reduce  );   Bool        shadowReduceFlicker()C {return _shd_reduce    ;} // set/get Shadow Flickering Decreasing                 (true/false          , default=     false                         ), this option reduces directional light shadow map flickering when rotating the camera, however at the expense of slightly increasing the shadow map blockiness, enable only when the flickering is really disturbing, the change is instant, you can call it real-time
   Display& shadowFrac         (Flt         frac    );   Flt         shadowFrac         ()C {return _shd_frac      ;} // set/get Shadow Range Fraction for Directional Lights (  0..1              , default=         1                         ), this option can limit shadowing range to a fraction of the viewport range, the change is instant, you can call it real-time
   Display& shadowFade         (Flt         fade    );   Flt         shadowFade         ()C {return _shd_fade      ;} // set/get Shadow Fade  Fraction for Directional Lights (  0..1              , default=         1                         ), this option specifies at which part of the shadowing range, shadow fading occurs, the change is instant, you can call it real-time
   Display& shadowMapSize      (Int         map_size);   Int         shadowMapSize      ()C {return _shd_map_size  ;} // set/get Shadow Map Size                              (  1..Inf            , default=      1024                         ), this option specifies the size of a single shadow map (in pixels), bigger size results in more precise shadows but smaller performance, the change is NOT instant, avoid calling real-time
   Display& shadowMapSizeLocal (Flt         frac    );   Flt         shadowMapSizeLocal ()C {return _shd_map_size_l;} // set/get Shadow Map Size Fraction for    Local Lights (  0..1              , default=         1                         ), this option specifies global size factor for computing the size of shadow maps for local lights (point/cone), final size of shadow maps for local lights is calculated using following formula: "shadowMapSize()*shadowMapSizeLocal()"                    , this means that for shadowMapSizeLocal()==1 full shadowMapSize() is used, for shadowMapSizeLocal()==0.5 half of shadowMapSize() is used, and so on, the change is instant, you can call it real-time
   Display& shadowMapSizeCone  (Flt         factor  );   Flt         shadowMapSizeCone  ()C {return _shd_map_size_c;} // set/get Shadow Map Size Factor   for    Cone  Lights (  0..2              , default=         1                         ), this option specifies global size factor for computing the size of shadow maps for cone  lights             , final size of shadow maps for cone  lights is calculated using following formula: "shadowMapSize()*shadowMapSizeLocal()*shadowMapSizeCone()", this means that for shadowMapSizeCone ()==1 full shadowMapSize() is used, for shadowMapSizeCone ()==0.5 half of shadowMapSize() is used, for shadowMapSizeCone()==2 double of shadowMapSize() is used, the range for shadowMapSizeCone is 0..2, which means that if set to 2, it can double the default shadow quality for all cone lights, the change is instant, you can call it real-time
   Display& shadowMapNum       (Byte        map_num );   Byte        shadowMapNum       ()C {return _shd_map_num   ;} // set/get Number of Shadow Maps for Directional Lights (  1..6              , default=         6                         ), this option specifies the number of shadow maps used when rendering directional lights, the more shadow maps are used the better quality but smaller performance, RT_FORWARD renderer supports only even numbers of maps, the change is instant, you can call it real-time
   Display& shadowMapSplit     (C Vec2     &factor  ); C Vec2&       shadowMapSplit     ()C {return _shd_map_split ;} // set/get Split Factor used for calculating Map Ranges (                    , default=    Vec2(2, 1)                     ), this option affects calculation of shadow map split ranges from the formula "Pow(x, factor.x + factor.y*x)". Ideal value is Vec2(2, 0) - "Pow(x, 2 + 0*x)" which gives equal distribution among near/far splits. However games may prefer better quality closer to the camera, while sacrificing the far quality, in that case it is beneficial to pass bigger values than Vec2(2, 0).
   Display& cloudsMapSize      (Int         map_size);   Int         cloudsMapSize      ()C {return _cld_map_size  ;} // set/get Clouds Shadow Map Size                       (  1..Inf            , default=       128                         ), the change is NOT instant, avoid calling real-time
#if EE_PRIVATE
   void     shadowJitterSet    ();
   Int      shadowMapNumActual ()C;
   Int      shadowMapSizeActual()C {return _shd_map_size_actual;}
   Bool     shadowSupported    ()C;
#endif

   // Motion Blur
   Display& motionMode  (MOTION_MODE mode );   MOTION_MODE motionMode  ()C {return _mtn_mode  ;} // set/get Motion Blur Mode           (MOTION_MODE, default=MOTION_NONE  ), the change is instant, you can call it real-time
   Display& motionDilate(DILATE_MODE mode );   DILATE_MODE motionDilate()C {return _mtn_dilate;} // set/get Motion Blur Mode           (DILATE_MODE, default=DILATE_ORTHO2), the change is instant, you can call it real-time
   Display& motionScale (Flt         scale);   Flt         motionScale ()C {return _mtn_scale ;} // set/get Motion Blur Velocity Scale (  0..Inf   , default=         0.04), the change is instant, you can call it real-time
   Display& motionRes   (Flt         scale);   Flt         motionRes   ()C;                      // set/get Motion Blur Resolution     (  0..1     , default=          1/3), this determines the size of the buffers used for calculating the Motion Blur effect, 1=full size, 0.5=half size, 0.25=quarter size, .., smaller sizes offer faster performance but worse quality, the change is NOT instant, avoid calling real-time

   // Depth of Field
#if EE_PRIVATE
   #define EPS_DOF (1.0f/1024)
   Bool dofWant()C {return dofMode()!=DOF_NONE && dofIntensity()>EPS_DOF;}
#endif
   Display& dofMode     (DOF_MODE mode     );   DOF_MODE dofMode     ()C {return _dof_mode     ;} // set/get Depth of Field Mode                 ( DOF_MODE , default=DOF_NONE), the change is instant, you can call it real-time
   Display& dofFocusMode(Bool     realistic);   Bool     dofFocusMode()C {return _dof_foc_mode ;} // set/get Depth of Field Focus Mode           (true/false, default=   false), this affects how the focus is calculated based on depth, simple mode (realistic=false) operates on 'dofFocus' and 'dofRange' only, while realistic mode (realistic=true) operates on 'dofFocus' only
   Display& dofFocus    (Flt      z        );   Flt      dofFocus    ()C {return _dof_focus    ;} // set/get Depth of Field Focus Depth Position (   0..Inf , default=       0), this is the distance from camera which should be completely focused and not blurred, the change is instant, you can call it real-time
   Display& dofRange    (Flt      range    );   Flt      dofRange    ()C {return _dof_range    ;} // set/get Depth of Field Focus Depth Range    (   0..Inf , default=      30), this is the distance from 'dofFocus' (Focus Depth Position) where blurring should be set to maximum, used only in simple focus mode "dofFocusMode==false" (the change is instant, you can call it real-time
   Display& dofIntensity(Flt      intensity);   Flt      dofIntensity()C {return _dof_intensity;} // set/get Depth of Field Intensity            (   0..Inf , default=       1), intensity of the depth of field effect, the change is instant, you can call it real-time

   // Level of Detail
#if EE_PRIVATE
   void lodSetCurrentFactor();
   void lodUpdateFactors   ();
#endif
   Display& lod            (Flt general, Flt shadow, Flt mirror);                              // set     Level of Detail factors, the change is instant, you can call it real-time
   Display& lodFactor      (Flt factor);   Flt lodFactor      ()C {return _lod_factor       ;} // set/get Level of Detail general factor which determines                                    Lod selection, 0..Inf, default=1, it's a scaling factor: values from 0..1 increase details and values from 1..Inf decrease details, the change is instant, you can call it real-time
   Display& lodFactorShadow(Flt factor);   Flt lodFactorShadow()C {return _lod_factor_shadow;} // set/get Level of Detail shadows factor which is also applied to shadows                    Lod selection, 0..Inf, defailt=2, it's a scaling factor: values from 0..1 increase details and values from 1..Inf decrease details, the change is instant, you can call it real-time
   Display& lodFactorMirror(Flt factor);   Flt lodFactorMirror()C {return _lod_factor_mirror;} // set/get Level of Detail mirror  factor which is also applied to meshes rendered in mirrors Lod selection, 0..Inf, defailt=2, it's a scaling factor: values from 0..1 increase details and values from 1..Inf decrease details, the change is instant, you can call it real-time

   // Tesselation
   Display& tesselation         (Bool on     );   Bool tesselation         ()C {return _tesselation          ;} // set/get Tesselation               (true/false, default=false), Tesselation smoothes the mesh polygons, it is available only on Shader Model>=5.0, the change is NOT instant, avoid calling real-time
   Display& tesselationHeightmap(Bool on     );   Bool tesselationHeightmap()C {return _tesselation_heightmap;} // set/get Tesselation of Heightmaps (true/false, default=false), the change is NOT instant, avoid calling real-time
   Display& tesselationDensity  (Flt  density);   Flt  tesselationDensity  ()C {return _tesselation_density  ;} // set/get Tesselation Density       (  0..Inf  , default=60   ), the change is instant, you can call it real-time
#if EE_PRIVATE
   Display& tesselationAllow    (Bool on     );   Bool tesselationAllow    ()C {return _tesselation_allow    ;} // set/get Tesselation Allow         (true/false, default=true ), the change is instant, you can call it real-time
#endif

   // Grass
   Display& grassRange  (Flt  range  );   Flt  grassRange  ()C {return _grass_range  ;} // set/get Grass Range             (  0..Inf  , default=50                ), grass objects above specified range will not be rendered, the change is instant, you can call it real-time
   Display& grassDensity(Flt  density);   Flt  grassDensity()C {return _grass_density;} // set/get Grass Density           (  0..1    , default=1 (0.5 for Mobile)), controls the amount of grass objects rendered, the change is instant, you can call it real-time
   Display& grassShadow (Bool on     );   Bool grassShadow ()C {return _grass_shadow ;} // set/get Grass Shadow Casting    (true/false, default=false             ), if grass objects should cast shadows, the change is instant, you can call it real-time
   Display& grassMirror (Bool on     );   Bool grassMirror ()C {return _grass_mirror ;} // set/get Grass Drawing in Mirror (true/false, default=false             ), if grass objects should be drawn in mirrors, the change is instant, you can call it real-time
   Display& grassUpdate (            );                                                 // update  Grass and Leafs bending animation, call this each frame to update the animation, the change is instant, you can call it real-time

   // Fur
   Display& furStaticGravity (Flt gravity  );   Flt furStaticGravity ()C {return _fur_gravity  ;} //                     gravity affecting fur on non-animated meshes which aren't controlled by AnimatedSkeleton (AnimatedSkeleton.fur_* parameters), -Inf..Inf, default=-1   , the change is instant, you can call it real-time
   Display& furStaticVelScale(Flt vel_scale);   Flt furStaticVelScale()C {return _fur_vel_scale;} // how much does mesh movement affect    fur on non-animated meshes which aren't controlled by AnimatedSkeleton (AnimatedSkeleton.fur_* parameters), -Inf..Inf, default=-0.75, the change is instant, you can call it real-time

   // Volumetric Lighting
   Display& volLight(Bool on );   Bool volLight()C {return _vol_light;} // set/get Volumetric Lighting       (true/false, default=false), the change is instant, you can call it real-time
   Display& volAdd  (Bool on );   Bool volAdd  ()C {return _vol_add  ;} // set/get Volumetric Apply Mode     (true/false, default=false), when true is set, Volumetric Light will be added to the scene, if false is set, Lerp to 1 will be performed, the change is instant, you can call it real-time
   Display& volMax  (Flt  max);   Flt  volMax  ()C {return _vol_max  ;} // set/get Volumetric Global Maximum (   0..Inf , default=1.0  ), the change is instant, you can call it real-time

   // Viewport Settings
#if EE_PRIVATE
   void     setViewFovTan();
   void     viewUpdate();
   void     viewReset ();
   Display& view      (C RectI &rect, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode);
   Display& view      (C Rect  &rect, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode);
                                                       Flt      viewFromActual()C {return _view_from        ;} // get actual viewport near clip plane (this depends on FovMode)
                                                     C Vec2 &   viewCenter    ()C {return _view_center      ;} // get viewport center
                                                     C Vec2 &   viewFovTanGui ()C {return _view_fov_tan_gui ;} // get viewport fov tan, used for point transformation between 3D and VR 'GuiTexture'
                                                     C Vec2 &   viewFovTanFull()C {return _view_fov_tan_full;} // get viewport fov tan, used for point transformation between 3D and Full Screen
#endif
   Display& viewRect (C RectI *rect               ); C RectI&   viewRectI   ()C {return _view_main.recti   ;} // set/get viewport rectangle (null for fullscreen), custom viewport rectangle is reset at start of each frame to fullscreen, that's why when used it must be set manually in each frame
   Display& viewRect (C Rect  &rect               ); C Rect &   viewRect    ()C {return _view_rect         ;} // set/get viewport rectangle                      , custom viewport rectangle is reset at start of each frame to fullscreen, that's why when used it must be set manually in each frame
   Display& viewFrom (Flt from                    );   Flt      viewFrom    ()C {return _view_main.from    ;} // set/get viewport near clip plane       , default=0.05
   Display& viewRange(Flt range                   );   Flt      viewRange   ()C {return _view_main.range   ;} // set/get viewport far  clip plane       , default=100
   Display& viewFov  (Flt fov, FOV_MODE mode=FOV_Y);   Flt      viewFov     ()C {return _view_fov          ;} // set/get viewport          field of view, default={DegToRad(70), FOV_Y}
   Display& viewFov  (C Vec2 &fov                 ); C Vec2 &   viewFovXY   ()C {return _view_main.fov     ;} // set/get viewport          field of view from both horizontal and vertical values
                                                       Flt      viewFovX    ()C {return _view_main.fov.x   ;} //     get actual horizontal field of view
                                                       Flt      viewFovY    ()C {return _view_main.fov.y   ;} //     get actual vertical   field of view
                                                       FOV_MODE viewFovMode ()C {return _view_main.fov_mode;} //     get viewport          field of view mode, default=FOV_Y
                                                       Flt      viewQuadDist()C;                              //     get viewport quad max distance from camera
   Display& viewForceSquarePixel(Bool square=false);                                                          //   force pixel aspect ratio to 1, this may be needed for rendering to textures, default rendering should have this disabled, default=false

   // Convert Coordinates                                                          src.X           src.Y                          dest.X          dest.Y
   static Vec2  screenToUV              (C Vec2  &screen); // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to UV          (   0 .. 1     ,    0 .. 1     )
   static Rect  screenToUV              (C Rect  &screen); // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to UV          (   0 .. 1     ,    0 .. 1     )
   static Vec2  screenToPixel           (C Vec2  &screen); // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
   static VecI2 screenToPixelI          (C Vec2  &screen); // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
   static Rect  screenToPixel           (C Rect  &screen); // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
   static RectI screenToPixelI          (C Rect  &screen); // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
   static Vec2  screenToPixelSize       (C Vec2  &screen); // from screen size (   0 .. D.w*2 ,    0 .. D.h*2 ) to pixel  size (   0 .. D.resW,    0 .. D.resH), use this function for widths and heights
   static Vec2  UVToScreen              (C Vec2  &uv    ); // from UV          (   0 .. 1     ,    0 .. 1     ) to screen      (-D.w .. D.w   , -D.h .. D.h   )
   static Vec2  pixelToScreen           (C Vec2  &pixel ); // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
   static Vec2  pixelToScreen           (C VecI2 &pixel ); // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
   static Rect  pixelToScreen           (C Rect  &pixel ); // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
   static Rect  pixelToScreen           (C RectI &pixel ); // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
   static Vec2  pixelToScreenSize       (  Flt    pixel ); // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ), use this function for widths and heights
   static Vec2  pixelToScreenSize       (C Vec2  &pixel ); // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ), use this function for widths and heights
   static Vec2  pixelToScreenSize       (C VecI2 &pixel ); // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ), use this function for widths and heights
   static Vec2  screenAlignedToPixel    (C Vec2  &screen); // get   screen space position  aligned to nearest pixel
   static Vec2  alignScreenToPixelOffset(C Vec2  &screen); // get   offset needed for aligning screen space position to nearest pixel
   static void  alignScreenToPixel      (  Vec2  &screen); // align screen space position  to nearest pixel
   static void  alignScreenToPixel      (  Rect  &screen); // align screen space rectangle so its top-left corner will be aligned to nearest pixel while preserving the rectangle size
        C Vec2& pixelToScreenSize       (               )C {return _pixel_size;} // get screen size of a single pixel
#if EE_PRIVATE
   static void  alignScreenXToPixel     (  Flt   &screen_x); // align screen space x position to nearest pixel
   static void  alignScreenYToPixel     (  Flt   &screen_y); // align screen space y position to nearest pixel
   static Vec2  windowPixelToScreen     (C Vec2  &pixel   ); // from pixel  (   0 .. D.resW,    0 .. D.resH) to screen (-D.w .. D.w   , -D.h .. D.h   ) taking into account System Window -> VR Gui
   static Vec2  windowPixelToScreen     (C VecI2 &pixel   ); // from pixel  (   0 .. D.resW,    0 .. D.resH) to screen (-D.w .. D.w   , -D.h .. D.h   ) taking into account System Window -> VR Gui
   static VecI2 screenToWindowPixelI    (C Vec2  &screen  ); // from screen (-D.w .. D.w   , -D.h .. D.h   ) to pixel  (   0 .. D.resW,    0 .. D.resH) taking into account System Window -> VR Gui
   static RectI screenToWindowPixelI    (C Rect  &screen  ); // from screen (-D.w .. D.w   , -D.h .. D.h   ) to pixel  (   0 .. D.resW,    0 .. D.resH) taking into account System Window -> VR Gui
#endif

   // Clear Screen
   static void clear     (C Color &color=TRANSPARENT); // clear screen viewport to 'color' and clear depth buffer
   static void clearCol  (C Color &color=TRANSPARENT); // clear screen viewport to 'color'
   static void clearDepth(                          ); // clear                                      depth buffer
#if EE_PRIVATE
   static void clearDS     (Byte s=0                ); // clear depth buffer and stencil (if available)
   static void clearStencil(Byte s=0                ); // clear                  stencil
#if DX9
   static void clearCol    (Int i, C Color &color   ); // clear i-th full (not viewport) RT to 'color'
#else
   static void clearCol    (       C Vec4  &color   ); // clear screen viewport to 'color'
   static void clearCol    (Int i, C Vec4  &color   ); // clear i-th full (not viewport) RT to 'color'
#endif
#endif

   // operations
   void setShader(C Material *material=null); // manually trigger resetting shaders for all meshes, 'material'=if reset shaders only for meshes having specified material (use null for all meshes)

#if !EE_PRIVATE
private:
#endif
   struct ViewportSettings
   {
      FOV_MODE fov_mode;
      Flt      from, range, fov;
      Rect     rect;

      void get() ; // get viewport settings from current display settings "T=D"
      void set()C; // set viewport settings as   current display settings "D=T"
   };
   struct Viewport
   {
      // 2D
      RectI    recti;
      Bool     full;

      // 3D
      FOV_MODE fov_mode;
      Flt      from, range;
      Vec2     fov, fov_sin, fov_cos, fov_tan;

   #if EE_PRIVATE
      Viewport& set3DFrom    (C Viewport &src);
      Viewport& setRect      (C RectI &recti);
      Viewport& setFrom      (Flt from);
      Viewport& setRange     (Flt range);
      Viewport& setFov       (); // this needs to be called after 'setRect'
      Viewport& setFov       (                                     C Vec2 &fov, FOV_MODE fov_mode);
      Viewport& set          (C RectI &recti, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode);
      Viewport& setViewport  (Bool allow_proj_matrix_update=true);
      Viewport& setShader    (Flt *offset=null);
      Viewport& setProjMatrix(Bool set_frustum);
   #endif
   };
   struct Monitor
   {
      RectI full, work;
      Bool  primary;

   #if EE_PRIVATE
      #if WINDOWS_OLD
         Bool set(HMONITOR monitor);
      #elif WINDOWS_NEW
         void set(C DXGI_OUTPUT_DESC &desc);
      #endif
      Monitor();
   #endif
   };

   ASPECT_MODE       _aspect_mode;
   BUMP_MODE         _bump_mode;
   AMBIENT_MODE      _amb_mode;
   SHADOW_MODE       _shd_mode;
   MOTION_MODE       _mtn_mode;
   DILATE_MODE       _mtn_dilate;
   DOF_MODE          _dof_mode;
   EDGE_DETECT_MODE  _edge_detect, _outline_mode;
   EDGE_SOFTEN_MODE  _edge_soften;
   TEXTURE_USAGE     _tex_detail, _tex_reflect;
   SHADER_MODEL      _shader_model;
   IMAGE_PRECISION   _monitor_prec, _lit_col_rt_prec;
   FILTER_TYPE       _density_filter;
   Bool              _full, _sync, _exclusive, _hp_col_rt, _hp_nrm_rt, _hp_lum_rt, _hp_nrm_calc, _dither, _bend_leafs, _particles_soft, _particles_smooth, _tex_mip_filter, _tex_macro, _eye_adapt, _bloom_sat, _bloom_max, _bloom_half, _bloom_samples,
                     _tesselation, _tesselation_heightmap, _tesselation_allow,
                     _bloom_allow, _glow_allow, _amb_jitter, _amb_normal, _shd_jitter, _shd_reduce, _grass_shadow, _grass_mirror, _vol_light, _vol_add, _dof_foc_mode, _outline_sky, _color_palette_allow,
                     _gamma_all, _tex_pow2_3d, _tex_pow2_cube, _mrt_const_bit_size, _mrt_post_process, _view_square_pixel, _initialized, _resetting, _began, _no_gpu, _can_draw, _fade_get, _fade_flipped, _allow_stereo, _draw_null_mtrl, _mtrl_blend, _shader_tex_lod;
   Byte              _tex_pow2, _density, _samples, _max_tex_filter, _max_vtx_attribs, _bloom_blurs, _max_rt,
                     _amb_soft, _amb_res,
                     _shd_soft, _shd_map_num,
                     _mtn_res,
                     _max_lights,
                     _tex_filter, _tex_lod;
   UShort            _gamma_array[3][256];
   Int               _shd_map_size, _shd_map_size_actual, _cld_map_size, _freq_want, _freq_got, _max_tex_size;
   Long              _device_mem;
   VecI2             _res, _render_res;
   Flt               _aspect_ratio, _aspect_ratio_want, _pixel_aspect, _gamma, _font_sharpness, _scale,
                     _amb_contrast, _amb_scale, _amb_bias,
                     _eye_adapt_brightness, _eye_adapt_max_dark, _eye_adapt_max_bright, _eye_adapt_speed,
                     _eye_dist,
                     _shd_frac, _shd_fade, _shd_map_size_l, _shd_map_size_c,
                     _bloom_original, _bloom_scale, _bloom_cut,
                     _mtn_scale,
                     _dof_focus, _dof_range, _dof_intensity,
                     _vol_max,
                     _grass_range, _grass_range_sqr, _grass_density,
                     _lod_factor, _lod_factor_shadow, _lod_factor_mirror, _lod_factors[2][2], _lod_factors_fov[2][2], _lod_fov2, _lod_current_factor,
                     _tesselation_density,
                     _fur_gravity, _fur_vel_scale,
                     _view_fov, _view_from,
                     _fade_len, _fade_step,
                     _smaa_threshold;
   Vec2              _unscaled_size, _size, _size2, _pixel_size, _pixel_size_2, _pixel_size_inv,
                     _window_pixel_to_screen_mul, _window_pixel_to_screen_add, _window_pixel_to_screen_scale,
                     _amb_range, _shd_map_split;
   Vec               _amb_color, _ns_color, _eye_adapt_weight;
   Vec2              _view_center, _view_fov_tan_gui, _view_fov_tan_full;
   Rect              _view_rect, _view_eye_rect[2];
   Viewport          _view_main, _view_active;
   Str8              _device_name;
   ImagePtr          _color_palette[2];
   Image             _color_palette_soft[2];
   Mems<VecI2>       _modes;
   SyncLock          _lock;
 C Material*         _set_shader_material;
   Map<Ptr, Monitor> _monitors;

#if EE_PRIVATE
   // get
#if DX9
   Bool validUsage(UInt usage, D3DRESOURCETYPE res_type, Int image_type);
#endif

   // manage
   void init        ();
   void del         ();
   Bool create      ();
   void createDevice();
   void androidClose();
   void androidOpen ();

   // operations
   enum RESET_RESULT
   {
      RESET_OK                  ,
      RESET_DEVICE_NOT_CREATED  ,
      RESET_CUSTOM_LOST_FAILED  ,
      RESET_CUSTOM_RESET_FAILED ,
      RESET_DEVICE_RESET_FAILED ,
      RESET_RENDER_TARGET_FAILED,
   };
   static CChar8*      AsText     (RESET_RESULT result);
   static void         ResetFailed(RESET_RESULT New, RESET_RESULT old);
          RESET_RESULT ResetTry   ();
          void         Reset      ();
          RESET_RESULT modeTry    (Int w=-1, Int h=-1, Int full=-1); // try setting Display Mode, -1=keep original value
          void         modeSet    (Int w=-1, Int h=-1, Int full=-1); //     set     Display Mode, -1=keep original value

          Bool findMode      ();
          void getGamma      ();
          void getCaps       ();
          void after         (Bool resize_callback);
          Bool initialized   ()C {return _initialized;}
   static void end           ();
   static void begin         ();
   static Bool flip          ();
   static void flush         ();
   static void finish        ();
          void adjustWindow  ();
          void screenChanged (Flt old_width, Flt old_height);
          void   sizeChanged ();
          void validateCoords(Int eye=-1);
          void fadeUpdate    ();
          void fadeDraw      ();

   Bool deferredUnavailable  ()C;
   Bool deferredMSUnavailable()C;

   void     monitor(RectI &full, RectI &work, VecI2 &max_normal_win_client_size, VecI2 &maximized_win_client_size, C Monitor *monitor)C;
   void  curMonitor(RectI &full, RectI &work, VecI2 &max_normal_win_client_size, VecI2 &maximized_win_client_size) ;
   void mainMonitor(RectI &full, RectI &work, VecI2 &max_normal_win_client_size, VecI2 &maximized_win_client_size)C;
#endif

   Display();
}extern
   D; // Main Display Control
/******************************************************************************/
#if EE_PRIVATE

#if DX9
   extern IDirect3DDevice9 *D3D;
#elif DX11
   extern ID3D11Device         *D3D;
   extern ID3D11DeviceContext  *D3DC;
   extern ID3D11DeviceContext1 *D3DC1;
   #if WINDOWS_OLD
      extern IDXGISwapChain       *SwapChain;
      extern DXGI_SWAP_CHAIN_DESC  SwapChainDesc;
   #else
      extern IDXGISwapChain1      *SwapChain;
      extern DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
   #endif
   extern D3D_FEATURE_LEVEL FeatureLevel;
#elif GL
   struct GLContext
   {
      Bool locked;
   #if WINDOWS
      HGLRC         context;
   #elif MAC
      CGLContextObj context;
   #elif LINUX
      GLXContext    context;
   #elif ANDROID
      EGLSurface    surface;
      EGLContext    context;
   #elif IOS
      EAGLContext  *context;
   #elif WEB
      EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
   #endif

      Bool is()C;
      GLContext();
     ~GLContext() {del();}
      void         del();
      Bool createSecondary();
      void   lock();
      void unlock();
   }extern
      MainContext;

   extern UInt FBO, VAO;
   #define VARIABLE_MAX_MATRIX 0 // GeForce 1080 has limit of 1024 uniform Vec4's which means that new GPU's don't support more than required, TODO: this could be done only if DX10+ style constant buffers are implemented for GL
   #if     VARIABLE_MAX_MATRIX
      extern Bool MeshBoneSplit;
   #endif
   #if LINUX
      extern GLXFBConfig GLConfig;
   #endif
#endif

   Bool     SetDisplayMode(Int mode=1); // 0=always off (use at shutdown), 1=if want full and app is active, 2=if want full (use at init)
   void RequestDisplayMode(Int w, Int h, Int full);

#if WINDOWS_OLD
   IDirect3DDevice9* GetD3D9();
#elif WINDOWS_NEW
   extern Flt ScreenScale;

   Int DipsToPixels(Flt dips);
   Flt PixelsToDips(Int pix );
#elif MAC
   extern NSOpenGLContext *OpenGLContext;
#elif LINUX
   extern ::Display *XDisplay;
#elif IOS
   extern Flt ScreenScale;
#endif

#endif
/******************************************************************************/
