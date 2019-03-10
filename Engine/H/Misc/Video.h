/******************************************************************************

   Use 'Video' to display video files.

/******************************************************************************/
enum VIDEO_CODEC : Byte
{
   VIDEO_NONE  ,
   VIDEO_THEORA, // doesn't support random seeking
   VIDEO_VP9   , // doesn't support random seeking
};
#if EE_PRIVATE
struct VideoCodec
{
   enum RESULT
   {
      ERROR,
      OK   ,
      END  ,
   };

   virtual Bool   create      (Video &video           ) {return false;}
   virtual RESULT nextFrame   (Video &video, Flt &time) {return ERROR;}
   virtual void   frameToImage(Video &video           ) {}

   virtual ~VideoCodec() {} // set virtual destructor so 'Delete' can be used together with extended classes
};
#endif
/******************************************************************************/
struct Video // Video Decoder
{
   enum MODE
   {
      DEFAULT, // default video mode, use for most cases, allows 'Video.draw'      methods, but not 'Video.image'
      IMAGE  , // video will be accessible as texture   , allows 'Video.draw'      methods  AND     'Video.image'
      ALPHA  , // video will be used only as alpha channel for   'Video.drawAlpha' methods
   };

   // manage
   void del   (                                               ); // delete manually
   Bool create(C Str &name, Bool loop=false, MODE mode=DEFAULT); // create from 'name' file name   , 'loop'=if this video should be looped, false on fail
   Bool create(C UID &id  , Bool loop=false, MODE mode=DEFAULT); // create from 'id'   file name ID, 'loop'=if this video should be looped, false on fail

   // get
   Bool        is       ()C {return _codec!=VIDEO_NONE;} // if  created
   VIDEO_CODEC codec    ()C {return _codec            ;} // get video codec
   CChar8*     codecName()C;                             // get video codec name
   Flt         time     ()C {return _time             ;} // get current time position of the video
   Int         width    ()C {return _w                ;} // get video width
   Int         height   ()C {return _h                ;} // get video height
   Flt         aspect   ()C {return Flt(_w)/_h        ;} // get video aspect ratio "width/height"
   Int         bitRate  ()C {return _br               ;} // get video bit rate
   Flt         fps      ()C {return _fps              ;} // get video frames per second

 C Image& image()C; // get video as image, this will be valid only if the video was created with 'mode'=IMAGE, you may no longer use this image once the video got deleted

   // update
   Bool update(Flt time); // update to 'time' video time, returns false when finished playing, some codecs don't support random seeking, only continuous playback is supported

   // fit
   Rect fit(C Rect &rect, FIT_MODE fit=FIT_FULL)C {return Fit(aspect(), rect, fit);} // get rectangle that can be used for drawing of the video to the 'rect' destination while preserving video proportions according to specified 'fit' mode

   // draw
   void drawFs (FIT_MODE fit=FIT_FULL)C; // draw full screen
   void drawFit(C Rect &rect         )C; // draw to fit best in given space, while preserving video proportions
   void draw   (C Rect &rect         )C; // draw to specified rectangle

   // draw using alpha from another 'Video'
   void drawAlphaFs (C Video &alpha, FIT_MODE fit=FIT_FULL)C; // 'alpha'=video to use as opacity, draw full screen
   void drawAlphaFit(C Video &alpha, C Rect &rect         )C; // 'alpha'=video to use as opacity, draw to fit best in given space, while preserving video proportions
   void drawAlpha   (C Video &alpha, C Rect &rect         )C; // 'alpha'=video to use as opacity, draw to specified rectangle

#if EE_PRIVATE
   void zero        ();
   void release     ();
   Bool frameToImage(Int w, Int h, Int w2, Int h2, CPtr lum_data, CPtr u_data, CPtr v_data, Int lum_pitch, Int u_pitch, Int v_pitch);
   void frameToImage();
   Bool nextFrame   ();
#endif

  ~Video() {del();}
   Video();

#if !EE_PRIVATE
private:
#endif
   VIDEO_CODEC _codec;
   MODE        _mode;
   Bool        _loop;
   Int         _w, _h, _br;
   Flt         _time, _time_past, _fps;
   File        _file;
   Image       _lum, _u, _v, _tex, *_tex_ptr;
#if EE_PRIVATE
   VideoCodec *_d;
#else
   Ptr         _d;
#endif

   NO_COPY_CONSTRUCTOR(Video);
};
/******************************************************************************/
CChar8* CodecName(VIDEO_CODEC codec);
#if EE_PRIVATE
#if DX9
extern Memx<Image> VideoTextures;

void VideoTexturesLost ();
void VideoTexturesReset();
#endif
#endif
/******************************************************************************/
