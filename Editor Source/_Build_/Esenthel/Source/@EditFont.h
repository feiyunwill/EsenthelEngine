/******************************************************************************/
/******************************************************************************/
class EditFont
{
   static cchar8 *Ascii;

   bool diagonal_shadow,
        clear_type, // works better when font is small
        software,
        ascii, german, french, polish, chinese, japanese, korean, russian;
  sbyte mip_maps;
   int  size;
   flt  scale, 
        weight,
        min_filter,
        shadow_blur,
        shadow_opacity,
        shadow_spread;
   Str  font, custom_chars;
   TimeStamp diagonal_shadow_time,
             clear_type_time,
             software_time,
             ascii_time, german_time, french_time, polish_time, chinese_time, japanese_time, korean_time, russian_time,
             mip_maps_time,
             size_time,
             scale_time,
             weight_time,
             min_filter_time,
             shadow_blur_time,
             shadow_opacity_time,
             shadow_spread_time,
             font_time, custom_chars_time;

   void reset();

   Str chars()C;
   bool make(Font &font, C Str *chars=null)C;

   bool newer(C EditFont &src)C;
   bool equal(C EditFont &src)C;
   void newData();
   bool sync(C EditFont &src);
   bool undo(C EditFont &src);

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   EditFont();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
