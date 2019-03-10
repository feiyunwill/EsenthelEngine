/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   cchar8 *EditFont::Ascii="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()[]<>{}`~_-+=;:,.?/|\\'\" ";
/******************************************************************************/
   void EditFont::reset() {T=EditFont();}
   Str EditFont::chars()C
   {
      Str c;
      if(ascii   )c+=Ascii;
      if(chinese )c+=LanguageSpecific(CN);
      if(japanese)c+=LanguageSpecific(JP);
      if(korean  )c+=LanguageSpecific(KO);
      if(german  )c+=LanguageSpecific(DE);
      if(french  )c+=LanguageSpecific(FR);
      if(polish  )c+=LanguageSpecific(PL);
      if(russian )c+=LanguageSpecific(RU);
      return c+custom_chars;
   }
   bool EditFont::make(Font &font, C Str *chars)C
   {
      Font::Params fp;
      fp.system_font    =T.font;
      fp.characters     =(chars ? *chars : T.chars());
      fp.size           =size;
      fp.scale          =scale;
      fp.mode           =(clear_type ? Font::SMOOTHED : Font::DEFAULT);
      fp.software       =(chars      ? false         : software    ); // if 'chars' is provided then it means we're creating only a preview, so use HW to be able to draw
      fp.weight         =weight         ;
      fp.minimum_filter =min_filter     ;
      fp.mip_maps       =mip_maps       ;
      fp.shadow_blur    =shadow_blur    ;
      fp.shadow_diagonal=diagonal_shadow;
      fp.shadow_opacity =shadow_opacity ;
      fp.shadow_spread  =shadow_spread  ;
      if(chars
      || fp.software)fp.image_type=IMAGE_L8A8; // if 'chars' is provided then it means we're creating only a preview, so use IMAGE_L8A8 for fast processing (use L8A8 also for software modes)
      return font.create(fp);
   }
   bool EditFont::newer(C EditFont &src)C
   {
      return diagonal_shadow_time>src.diagonal_shadow_time
          || clear_type_time>src.clear_type_time
          || software_time>src.software_time
          || ascii_time>src.ascii_time || german_time>src.german_time || french_time>src.french_time || polish_time>src.polish_time || chinese_time>src.chinese_time || japanese_time>src.japanese_time || korean_time>src.korean_time || russian_time>src.russian_time
          || mip_maps_time>src.mip_maps_time
          || size_time>src.size_time
          || scale_time>src.scale_time
          || weight_time>src.weight_time
          || min_filter_time>src.min_filter_time
          || shadow_blur_time>src.shadow_blur_time
          || shadow_opacity_time>src.shadow_opacity_time
          || shadow_spread_time>src.shadow_spread_time
          || font_time>src.font_time
          || custom_chars_time>src.custom_chars_time;
   }
   bool EditFont::equal(C EditFont &src)C
   {
      return diagonal_shadow_time==src.diagonal_shadow_time
          && clear_type_time==src.clear_type_time
          && software_time==src.software_time
          && ascii_time==src.ascii_time && german_time==src.german_time && french_time==src.french_time && polish_time==src.polish_time && chinese_time==src.chinese_time && japanese_time==src.japanese_time && korean_time==src.korean_time && russian_time==src.russian_time
          && mip_maps_time==src.mip_maps_time
          && size_time==src.size_time
          && scale_time==src.scale_time
          && weight_time==src.weight_time
          && min_filter_time==src.min_filter_time
          && shadow_blur_time==src.shadow_blur_time
          && shadow_opacity_time==src.shadow_opacity_time
          && shadow_spread_time==src.shadow_spread_time
          && font_time==src.font_time
          && custom_chars_time==src.custom_chars_time;
   }
   void EditFont::newData()
   {
      diagonal_shadow_time++;
      clear_type_time++;
      software_time++;
      ascii_time++; german_time++; french_time++; polish_time++; chinese_time++; japanese_time++; korean_time++; russian_time++;
      mip_maps_time++;
      size_time++;
      scale_time++;
      weight_time++;
      min_filter_time++;
      shadow_blur_time++;
      shadow_opacity_time++;
      shadow_spread_time++;
      font_time++; custom_chars_time++;
   }
   bool EditFont::sync(C EditFont &src)
   {
      bool changed=false;
      changed|=Sync(diagonal_shadow_time, src.diagonal_shadow_time, diagonal_shadow, src.diagonal_shadow);
      changed|=Sync(     clear_type_time, src.     clear_type_time,      clear_type, src.     clear_type);
      changed|=Sync(       software_time, src.       software_time,        software, src.       software);
      changed|=Sync(          ascii_time, src.          ascii_time,           ascii, src.          ascii);
      changed|=Sync(         german_time, src.         german_time,          german, src.         german);
      changed|=Sync(         french_time, src.         french_time,          french, src.         french);
      changed|=Sync(         polish_time, src.         polish_time,          polish, src.         polish);
      changed|=Sync(        chinese_time, src.        chinese_time,         chinese, src.        chinese);
      changed|=Sync(       japanese_time, src.       japanese_time,        japanese, src.       japanese);
      changed|=Sync(         korean_time, src.         korean_time,          korean, src.         korean);
      changed|=Sync(        russian_time, src.        russian_time,         russian, src.        russian);
      changed|=Sync(       mip_maps_time, src.       mip_maps_time,        mip_maps, src.       mip_maps);
      changed|=Sync(           size_time, src.           size_time,            size, src.           size);
      changed|=Sync(          scale_time, src.          scale_time,           scale, src.          scale);
      changed|=Sync(         weight_time, src.         weight_time,          weight, src.         weight);
      changed|=Sync(     min_filter_time, src.     min_filter_time,      min_filter, src.     min_filter);
      changed|=Sync(    shadow_blur_time, src.    shadow_blur_time,     shadow_blur, src.    shadow_blur);
      changed|=Sync( shadow_opacity_time, src. shadow_opacity_time,  shadow_opacity, src. shadow_opacity);
      changed|=Sync(  shadow_spread_time, src.  shadow_spread_time,   shadow_spread, src.  shadow_spread);
      changed|=Sync(           font_time, src.           font_time,            font, src.           font);
      changed|=Sync(   custom_chars_time, src.   custom_chars_time,    custom_chars, src.   custom_chars);
      return changed;
   }
   bool EditFont::undo(C EditFont &src)
   {
      bool changed=false;
      changed|=Undo(diagonal_shadow_time, src.diagonal_shadow_time, diagonal_shadow, src.diagonal_shadow);
      changed|=Undo(     clear_type_time, src.     clear_type_time,      clear_type, src.     clear_type);
      changed|=Undo(       software_time, src.       software_time,        software, src.       software);
      changed|=Undo(          ascii_time, src.          ascii_time,           ascii, src.          ascii);
      changed|=Undo(         german_time, src.         german_time,          german, src.         german);
      changed|=Undo(         french_time, src.         french_time,          french, src.         french);
      changed|=Undo(         polish_time, src.         polish_time,          polish, src.         polish);
      changed|=Undo(        chinese_time, src.        chinese_time,         chinese, src.        chinese);
      changed|=Undo(       japanese_time, src.       japanese_time,        japanese, src.       japanese);
      changed|=Undo(         korean_time, src.         korean_time,          korean, src.         korean);
      changed|=Undo(        russian_time, src.        russian_time,         russian, src.        russian);
      changed|=Undo(       mip_maps_time, src.       mip_maps_time,        mip_maps, src.       mip_maps);
      changed|=Undo(           size_time, src.           size_time,            size, src.           size);
      changed|=Undo(          scale_time, src.          scale_time,           scale, src.          scale);
      changed|=Undo(         weight_time, src.         weight_time,          weight, src.         weight);
      changed|=Undo(     min_filter_time, src.     min_filter_time,      min_filter, src.     min_filter);
      changed|=Undo(    shadow_blur_time, src.    shadow_blur_time,     shadow_blur, src.    shadow_blur);
      changed|=Undo( shadow_opacity_time, src. shadow_opacity_time,  shadow_opacity, src. shadow_opacity);
      changed|=Undo(  shadow_spread_time, src.  shadow_spread_time,   shadow_spread, src.  shadow_spread);
      changed|=Undo(           font_time, src.           font_time,            font, src.           font);
      changed|=Undo(   custom_chars_time, src.   custom_chars_time,    custom_chars, src.   custom_chars);
      return changed;
   }
   bool EditFont::save(File &f)C
   {
      f.cmpUIntV(5);
      f<<diagonal_shadow<<clear_type<<software<<ascii<<german<<french<<polish<<chinese<<japanese<<korean<<russian
       <<mip_maps<<size<<scale<<weight<<min_filter<<shadow_blur<<shadow_opacity<<shadow_spread
       <<diagonal_shadow_time<<clear_type_time<<software_time<<ascii_time<<german_time<<french_time<<polish_time<<chinese_time<<japanese_time<<korean_time<<russian_time
       <<mip_maps_time<<size_time<<scale_time<<weight_time<<min_filter_time<<shadow_blur_time<<shadow_opacity_time<<shadow_spread_time<<font_time<<custom_chars_time
       <<font<<custom_chars;
      return f.ok();
   }
   bool EditFont::load(File &f)
   {
      reset(); switch(f.decUIntV())
      {
         case 5:
         {
            f>>diagonal_shadow>>clear_type>>software>>ascii>>german>>french>>polish>>chinese>>japanese>>korean>>russian
             >>mip_maps>>size>>scale>>weight>>min_filter>>shadow_blur>>shadow_opacity>>shadow_spread
             >>diagonal_shadow_time>>clear_type_time>>software_time>>ascii_time>>german_time>>french_time>>polish_time>>chinese_time>>japanese_time>>korean_time>>russian_time
             >>mip_maps_time>>size_time>>scale_time>>weight_time>>min_filter_time>>shadow_blur_time>>shadow_opacity_time>>shadow_spread_time>>font_time>>custom_chars_time
             >>font>>custom_chars;
            if(f.ok())return true;
         }break;

         case 4:
         {
            f>>diagonal_shadow>>clear_type>>ascii>>german>>french>>polish>>chinese>>japanese>>korean>>russian
             >>mip_maps>>size>>scale>>weight>>min_filter>>shadow_blur>>shadow_opacity>>shadow_spread
             >>diagonal_shadow_time>>clear_type_time>>ascii_time>>german_time>>french_time>>polish_time>>chinese_time>>japanese_time>>korean_time>>russian_time
             >>mip_maps_time>>size_time>>scale_time>>weight_time>>min_filter_time>>shadow_blur_time>>shadow_opacity_time>>shadow_spread_time>>font_time>>custom_chars_time
             >>font>>custom_chars;
            if(f.ok())return true;
         }break;

         case 3:
         {
            f>>diagonal_shadow>>clear_type>>ascii>>german>>french>>polish>>chinese>>japanese>>korean>>russian
             >>mip_maps>>size>>scale>>weight>>min_filter>>shadow_blur>>shadow_opacity>>shadow_spread
             >>diagonal_shadow_time>>clear_type_time>>ascii_time>>german_time>>french_time>>polish_time>>chinese_time>>japanese_time>>korean_time>>russian_time
             >>mip_maps_time>>size_time>>scale_time>>weight_time>>min_filter_time>>shadow_blur_time>>shadow_opacity_time>>shadow_spread_time>>font_time>>custom_chars_time;
            GetStr2(f, font); GetStr2(f, custom_chars);
            if(f.ok())return true;
         }break;

         case 2:
         {
            f>>diagonal_shadow>>clear_type>>ascii>>german>>french>>polish>>chinese>>japanese>>korean>>russian
             >>mip_maps>>size>>weight>>min_filter>>shadow_blur>>shadow_opacity>>shadow_spread
             >>diagonal_shadow_time>>clear_type_time>>ascii_time>>german_time>>french_time>>polish_time>>chinese_time>>japanese_time>>korean_time>>russian_time
             >>mip_maps_time>>size_time>>weight_time>>min_filter_time>>shadow_blur_time>>shadow_opacity_time>>shadow_spread_time>>font_time>>custom_chars_time;
             GetStr2(f, font); GetStr2(f, custom_chars);
            if(f.ok())return true;
         }break;

         case 1:
         {
            f>>diagonal_shadow>>clear_type>>ascii>>german>>french>>polish>>chinese>>japanese>>korean>>russian
             >>mip_maps>>size>>weight>>min_filter>>shadow_blur>>shadow_opacity
             >>diagonal_shadow_time>>clear_type_time>>ascii_time>>german_time>>french_time>>polish_time>>chinese_time>>japanese_time>>korean_time>>russian_time
             >>mip_maps_time>>size_time>>weight_time>>min_filter_time>>shadow_blur_time>>shadow_opacity_time>>font_time>>custom_chars_time;
            GetStr(f, font); GetStr(f, custom_chars);
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>diagonal_shadow>>clear_type>>ascii>>german>>french>>polish>>chinese>>russian
             >>mip_maps>>size>>weight>>min_filter>>shadow_blur>>shadow_opacity
             >>diagonal_shadow_time>>clear_type_time>>ascii_time>>german_time>>french_time>>polish_time>>chinese_time>>russian_time
             >>mip_maps_time>>size_time>>weight_time>>min_filter_time>>shadow_blur_time>>shadow_opacity_time>>font_time>>custom_chars_time;
            GetStr(f, font); GetStr(f, custom_chars);
            if(f.ok())return true;
         }break;
      }
      reset(); return false;
   }
   bool EditFont::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
EditFont::EditFont() : diagonal_shadow(true), clear_type(false), software(false), ascii(true), german(false), french(false), polish(false), chinese(false), japanese(false), korean(false), russian(false), mip_maps(0), size(48), scale(1), weight(0), min_filter(0), shadow_blur(0.04f), shadow_opacity(1.0f), shadow_spread(0.0f), font("Arial") {}

/******************************************************************************/
