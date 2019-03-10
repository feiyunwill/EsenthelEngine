/******************************************************************************/
class VideoEditor : PropWin
{
   class Custom : GuiCustom
   {
      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return null;}
      virtual void    draw(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            D.clip(gpc.clip);
            Rect r=rect()+gpc.offset; 
            VideoEditor &ve=*(VideoEditor*)user;
            ve.video.update (ve.video_time); ve.video_time+=Time.ad(); // Editor works in background so app time gets updated even when minimized, update time only when actually drawing
            ve.video.drawFit(r);
            r.draw(Gui.borderColor(), false);
         }
      }
   }

   UID       elm_id=UIDZero;
   Elm      *elm=null;
   bool      changed=false;
   Video     video;
   flt       video_time=0;
   Custom    custom;
   Button    locate;
   Property *width=null, *height=null, *kbps=null, *fps=null, *codec=null;

   static void Locate(VideoEditor &editor) {Proj.elmLocate(editor.elm_id);}

   void create()
   {
      width =&add();
      height=&add();
    //kbps  =&add(); don't create because Theora and VP9 fail to detect it correctly
      fps   =&add();
      codec =&add();
      autoData(this);

      Rect r=super.create("Video Player", Vec2(0.02, -0.02)); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=custom.create(this);
      rect(Rect_C(0, 0, Min(1.7, D.w()*2), Min(1.07, D.h()*2)));
      T+=locate.create(Rect_U(0.11, r.min.y-0.02, 0.15, 0.055), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
   }
   void setInfo()
   {
      if(width )width .name.set(S+"Width: " +video.width ());
      if(height)height.name.set(S+"Height: "+video.height());
      if(kbps  )kbps  .name.set(S+"Kbps: "  +DivRound(video.bitRate(), 1000));
      if(fps   )fps   .name.set(S+"FPS: "   +TextReal(video.fps(), -1));
      if(codec )codec .name.set(S+"Codec: " +video.codecName());
   }

   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(gpc.visible && visible())setInfo();
   }
   virtual VideoEditor& hide     (            )  override {set(null); super.hide(); return T;}
   virtual Rect         sizeLimit(            )C override {Rect r=super.sizeLimit(); r.min.set(1.0, 0.45); return r;}
   virtual VideoEditor& rect     (C Rect &rect)  override
   {
      super.rect(rect);
      flt  x=0; if(props.elms())x=0.22; //props[0].button.rect().max.x;
      Rect r(x, -clientHeight(), clientWidth(), 0); r.extend(-0.02);
      custom.rect(r);
      return T;
   }

   void flush()
   {
      if(elm && changed)
      {
         if(ElmVideo *data=elm.videoData())data.newVer(); // modify just before saving/sending in case we've received data from server after edit
         Preview.elmChanged(elm.id);
      }
      changed=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmVideo *data=elm.videoData())data.newVer();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_VIDEO)elm=null;
      if(T.elm!=elm)
      {
         flush();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         if(elm){video.create(Proj.gamePath(*elm), true); video_time=0;}else video.del(); // start from the beginning to avoid freezes
         setInfo();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void toggle  (Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void closeElm  (C UID &elm_id) {if(elm && elm.id==elm_id)video.del();}
   void erasing   (C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)
      {
         video.create(Proj.gamePath(*elm), true); video_time=0; setInfo(); // start from the beginning to avoid freezes
      }
   }
}
VideoEditor VideoEdit;
/******************************************************************************/
