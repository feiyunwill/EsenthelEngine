/******************************************************************************/
/******************************************************************************/
class GuiEditParam : EditParam
{
   class NameVal
   {
      cchar *name;
      int    val;
   };

   int  priority;
   Str _desc;
   bool min_use  , max_use  ;
   dbl  min_value    , max_value    ;
   flt                  mouse_edit_speed; // 0=default
   PROP_MOUSE_EDIT_MODE mouse_edit_mode ;
   cchar8              *enum_name;
   Mems<NameVal>        name_vals;

 C Str & desc()C;   GuiEditParam& desc (C Str &desc     );
                    GuiEditParam& min  (dbl    min      );
                    GuiEditParam& max  (dbl    max      );
                    GuiEditParam& range(dbl min, dbl max);

   GuiEditParam& mouseEditSpeed(flt                  speed);
   GuiEditParam& mouseEditMode (PROP_MOUSE_EDIT_MODE mode );

public:
   GuiEditParam();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
