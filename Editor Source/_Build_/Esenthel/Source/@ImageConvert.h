/******************************************************************************/
class ImageConvert
{
   enum FAMILY
   {
      ELM_IMAGE      ,
      ELM_IMAGE_ATLAS,
      ELM_FONT       ,
      ELM_PANEL_IMAGE,
      IMAGE          ,
   };
   bool       pow2     ,
              clamp     ,
              alpha_lum,
              has_color ,
              has_alpha ,
           ignore_alpha, 
            mtrl_base_1;
   byte        downsize;
   int         mip_maps,
               max_size;
   VecI2           size;
   FAMILY      family  ;
   Str        src, dest       ; // src and dest files
   DateTime   time            ; // date time to set after converting
   int        type, mode;

   static SyncLock Lock;

   void set(C Str &src, C Str &dest, C DateTime &time, int type, bool ignore_alpha, bool clamp, bool mtrl_base_1=false, byte downsize=0, int max_size=0);
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmImage &data, IMAGE_TYPE type);
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmIcon &data, IMAGE_TYPE type);
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmImageAtlas &data, IMAGE_TYPE type);
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmFont &data, IMAGE_TYPE type);
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmPanelImage &data, IMAGE_TYPE type);

   static bool SkipOptimize(int &type, DateTime &time); // skip formats which are slow to convert

   void process(C bool *stop=null)C;

public:
   ImageConvert();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
