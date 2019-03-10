/******************************************************************************/
/******************************************************************************/
class ListElm
{
   bool    size_known, tex_size_calculated;
   int     depth;
   flt     offset;
   Color   color, color_temp;
   Elm    *elm ;
   EEItem *item;
   Image  *opened_icon, *icon; // use regular pointers to avoid ref counting for ImagePtr
   Str     name, desc;
   long    size;
   union 
   {
      int      vis_parent_i; // stored as index first, because we're using Memc for the list data, adding new elements could change memory address
      ListElm *vis_parent  ;
   };

   static int CompareIndex(C ListElm &a, C ListElm &b); // we can use memory address because we're using 'Memc'
   static int CompareName (C ListElm &a, C ListElm &b);
   static int CompareSize(C ListElm &a, C ListElm &b);
   static int CompareTexSharp(C ListElm &a, C ListElm &b);
   static Str Size(C ListElm &data);
   static Str TexSharp(C ListElm &data);

   static void IncludeTex(Memt<UID> &texs, C UID &tex_id);
   static void IncludeTex(Memt<UID> &texs, C Elm &elm);
   static void IncludeTex(Memt<UID> &texs, C ElmNode &node);
   void calcTexSize(); // because texture size calculation is slow, it is calculated only on demand, it is slow because first we need to get all unique texture ID's, and then sum sizes of those textures, if we would sum all encountered texture ID's then we would get bigger values because the same texture ID's could be encountered multiple times
   long fileSize()C;

   flt texSharpness()C;

   void resetColor();              
   void highlight ();              
   bool hasVisibleChildren()C;                    
   void hasVisibleChildren(bool has, bool opened);

   bool close();
   void includeValidTexSize(C UID &tex_id); // assumes that 'tex_id' is valid
   void includeSize(Elm &elm);
   void includeSize(C ListElm &src);

   ListElm& set(ELM_TYPE type, C Str &name, bool edited, bool importing, bool removed, int depth, int vis_parent);
   ListElm& set(Elm &elm, ElmNode &node, int depth, int vis_parent, bool parent_removed);
   ListElm& set(EEItem &item, bool opened, int depth, bool parent_removed);

public:
   ListElm();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
