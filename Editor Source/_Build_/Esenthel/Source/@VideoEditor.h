/******************************************************************************/
/******************************************************************************/
class VideoEditor : PropWin
{
   class Custom : GuiCustom
   {
      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
      virtual void    draw(C GuiPC &gpc)override;
   };

   UID       elm_id;
   Elm      *elm;
   bool      changed;
   Video     video;
   flt       video_time;
   Custom    custom;
   Button    locate;
   Property *width, *height, *kbps, *fps, *codec;

   static void Locate(VideoEditor &editor);

   void create();
   void setInfo();

   virtual void update(C GuiPC &gpc)override;
   virtual VideoEditor& hide     (            )  override;
   virtual Rect         sizeLimit(            )C override;
   virtual VideoEditor& rect     (C Rect &rect)  override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void closeElm  (C UID &elm_id);
   void erasing   (C UID &elm_id);
   void elmChanged(C UID &elm_id);

public:
   VideoEditor();
};
/******************************************************************************/
/******************************************************************************/
extern VideoEditor VideoEdit;
/******************************************************************************/
