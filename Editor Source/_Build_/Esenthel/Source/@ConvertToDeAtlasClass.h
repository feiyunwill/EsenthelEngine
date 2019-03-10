/******************************************************************************/
/******************************************************************************/
class ConvertToDeAtlasClass : PropWin
{
   class Preview : GuiCustom
   {
      virtual void draw(C GuiPC &gpc)override;
   };

   enum MODE
   {
      NEW,
      REPLACE_KEEP,
      REPLACE_NO_PUBLISH,
      REPLACE_REMOVE,
   };
   static cchar8 *mode_t[]
;

   Memc<UID> objs, mtrls; // objects and materials to process
   UID       base_0_tex, base_1_tex; // base textures of the materials (this should be the same for all materials)
   Preview   preview;
   MODE      mode;
   RectI     source_rect;
   VecI2     dest_size, tex_size;
   Text    t_tex_size;
   Button    convert;
   Property *w, *h, *sw, *sh;

   static Str  TexSize(C VecI2 &size);            
   static void Convert(ConvertToDeAtlasClass&cta);

   void convertMeshes(Memc<IDReplace> &mtrl_replace, C Rect *frac);
   static Str Process(C Str &name, C VecI2 &size, C Rect *crop, C VecI2 *resize);
   void convertDo();
   void clearProj();
   void create();
   VecI2 finalSize()C;                  
   void setElms(C MemPtr<UID> &elm_ids);
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
   virtual ConvertToDeAtlasClass& hide()override;
   virtual void update(C GuiPC &gpc)override;

public:
   ConvertToDeAtlasClass();
};
/******************************************************************************/
/******************************************************************************/
extern ConvertToDeAtlasClass ConvertToDeAtlas;
/******************************************************************************/
