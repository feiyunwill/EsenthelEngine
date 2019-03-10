/******************************************************************************/
enum IMAGERT_TYPE : Byte // Image Render Target Type, this describes a group of Image Types
{
   IMAGERT_RGBA  , // IMAGE_DEFAULT                                                                        (         32-bit total       with Alpha)
   IMAGERT_RGB   , // IMAGE_R10G10B10A2, IMAGE_DEFAULT                                                     (         32-bit total       no   Alpha)
   IMAGERT_RGB_P , // IMAGE_R10G10B10A2, IMAGE_F16_3, IMAGE_F16_4, IMAGE_DEFAULT                           (at least 10-bit per channel no   Alpha)
   IMAGERT_RGBA_H, // IMAGE_F16_4, IMAGE_DEFAULT                                                           (at least 16-bit per channel with Alpha)
   IMAGERT_RGB_H , // IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_DEFAULT                           (at least 16-bit per channel no   Alpha)
   IMAGERT_RGBA_F, // IMAGE_F32_4, IMAGE_F16_4, IMAGE_DEFAULT                                              (at least 32-bit per channel with Alpha)
   IMAGERT_RGB_F , // IMAGE_F32_3, IMAGE_F32_4, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_DEFAULT (at least 32-bit per channel no   Alpha)
   IMAGERT_RGBA_S, // IMAGE_R8G8B8A8_SIGN, IMAGE_F16_4                                                     (signed   32-bit total       with Alpha)
   IMAGERT_F32   , // IMAGE_F32, IMAGE_F16
   IMAGERT_F16   , // IMAGE_F16, IMAGE_F32
   IMAGERT_ONE   , // (DX9: IMAGE_A8, IMAGE_L8A8, Rest: IMAGE_R8, IMAGE_R8G8), IMAGE_DEFAULT
   IMAGERT_ONE_S , // IMAGE_R8_SIGN, IMAGE_R8G8_SIGN, IMAGE_F16, IMAGE_R8G8B8A8_SIGN, IMAGE_F32
   IMAGERT_TWO   , // IMAGE_R8G8, IMAGE_DEFAULT
   IMAGERT_TWO_S , // IMAGE_R8G8_SIGN, IMAGE_R8G8B8A8_SIGN, IMAGE_F16_2
   IMAGERT_DS    , // (DX9: IMAGE_INTZ, IMAGE_RAWZ, IMAGE_DF24), IMAGE_D24S8, IMAGE_D24X8, IMAGE_D32, IMAGE_D16
   IMAGERT_NUM   , // number of Image render targets

   IMAGERT_RGBA_P =IMAGERT_RGBA_H, // (at least        10-bit per channel with Alpha)
   IMAGERT_RGBA_SP=IMAGERT_RGBA_H, // (at least signed 10-bit per channel with Alpha)
   IMAGERT_RGB_S  =IMAGERT_RGBA_S, // (         signed 32-bit total       no   Alpha)
};
/******************************************************************************/
struct ImageRC : Image // Reference Counted Image
{
   ImageRC() {_ptr_num=0;}

#if EE_PRIVATE
   Bool available()C {return _ptr_num==0;} // if this image is not currently used
#endif
#if !EE_PRIVATE
private:
#endif
   UInt _ptr_num;
   NO_COPY_CONSTRUCTOR(ImageRC);
};
/******************************************************************************/
#if EE_PRIVATE
struct ImageRTDesc // Render Target Description
{
   VecI2        size;
   Byte         samples;
   IMAGERT_TYPE rt_type;

            ImageRTDesc() {}
   explicit ImageRTDesc(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples=1) {size.set(w, h); T.rt_type=rt_type; T.samples=samples;}

//private:
   IMAGE_TYPE _type;
};
#endif
/******************************************************************************/
struct ImageRTPtr // Render Target Pointer
{
#if EE_PRIVATE
   Bool       find  (C ImageRTDesc &desc); // find Render Target, false on fail
   ImageRTPtr& get  (C ImageRTDesc &desc); // find Render Target, Exit  on fail
   ImageRTPtr& getDS(Int w, Int h, Byte samples=1, Bool reuse_main=true);
   ImageRC   * rc   ()C {return _data;}
#endif
   Bool       find(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples=1); // find Render Target, false on fail, 'samples'=number of samples per-pixel (allows multi-sampling)
   ImageRTPtr& get(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples=1); // find Render Target, Exit  on fail, 'samples'=number of samples per-pixel (allows multi-sampling)

   Image* operator ()  (               )C {return  T._data         ;} // access the data, you can use the returned reference as long as this 'ImageRTPtr' object exists and not modified
   Image* operator ->  (               )C {return  T._data         ;} // access the data, you can use the returned reference as long as this 'ImageRTPtr' object exists and not modified
   Image& operator *   (               )C {return *T._data         ;} // access the data, you can use the returned reference as long as this 'ImageRTPtr' object exists and not modified
   Bool   operator ==  (  null_t       )C {return  T._data==null   ;} // if pointers are equal
   Bool   operator !=  (  null_t       )C {return  T._data!=null   ;} // if pointers are different
   Bool   operator ==  (C Image      *p)C {return  T._data==p      ;} // if pointers are equal
   Bool   operator !=  (C Image      *p)C {return  T._data!=p      ;} // if pointers are different
   Bool   operator ==  (C ImageRTPtr &p)C {return  T._data==p._data;} // if pointers are equal
   Bool   operator !=  (C ImageRTPtr &p)C {return  T._data!=p._data;} // if pointers are different
          operator Bool(               )C {return  T._data!=null   ;} // if pointer  is  valid

   ImageRTPtr& clear    (               );                  // clear the pointer to null, this automatically decreases the reference count of current data
   ImageRTPtr& operator=(  null_t       ) {return clear();} // clear the pointer to null, this automatically decreases the reference count of current data
   ImageRTPtr& operator=(C ImageRTPtr &p);                  // set       pointer to 'p' , this automatically decreases the reference count of current data and increases the reference count of the new data
   ImageRTPtr& operator=(  ImageRC    *p);                  // set       pointer to 'p' , this automatically decreases the reference count of current data and increases the reference count of the new data

   ImageRTPtr(  null_t=null  ) {_data=null; _last_index=-1;}
   ImageRTPtr(C ImageRTPtr &p);
   ImageRTPtr(  ImageRC    *p);
#if EE_PRIVATE
   explicit ImageRTPtr(C ImageRTDesc &desc) {_data=null; _last_index=-1; get(desc);}
#endif
  ~ImageRTPtr(               ) {clear();}

#if !EE_PRIVATE
private:
#endif
   ImageRC *_data;
   Int      _last_index;
};
/******************************************************************************/
#if EE_PRIVATE
struct ImageRTPtrRef
{
   ImageRTPtr &ref;

   ImageRTPtr& get(C ImageRTDesc &desc) {return ref.get(desc);}

   Image*   operator() ()C {return  ref();}
   Image*   operator-> ()C {return  ref();}
   Image&   operator*  ()C {return *ref  ;}
   operator ImageRTPtr&()C {return  ref  ;}

   void clear() {ref.clear();}

   explicit ImageRTPtrRef(ImageRTPtr &image_rt_ptr) : ref(image_rt_ptr) {}
           ~ImageRTPtrRef(                        )  {ref.clear();}

   NO_COPY_CONSTRUCTOR(ImageRTPtrRef);
};

IMAGERT_TYPE GetImageRTType(                 Bool       alpha, IMAGE_PRECISION     precision);
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type                                                 );
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha                               );
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha, IMAGE_PRECISION max_precision);
void ResetImageTypeCreateResult();
#endif
/******************************************************************************/
