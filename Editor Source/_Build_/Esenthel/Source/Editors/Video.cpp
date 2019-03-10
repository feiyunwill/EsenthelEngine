/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
VideoEditor VideoEdit;
/******************************************************************************/

/******************************************************************************/
      GuiObj* VideoEditor::Custom::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel){return null;}
      void    VideoEditor::Custom::draw(C GuiPC &gpc)
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
   void VideoEditor::Locate(VideoEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void VideoEditor::create()
   {
      width =&add();
      height=&add();
    //kbps  =&add(); don't create because Theora and VP9 fail to detect it correctly
      fps   =&add();
      codec =&add();
      autoData(this);

      Rect r=::PropWin::create("Video Player", Vec2(0.02f, -0.02f)); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=custom.create(this);
      rect(Rect_C(0, 0, Min(1.7f, D.w()*2), Min(1.07f, D.h()*2)));
      T+=locate.create(Rect_U(0.11f, r.min.y-0.02f, 0.15f, 0.055f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
   }
   void VideoEditor::setInfo()
   {
      if(width )width ->name.set(S+"Width: " +video.width ());
      if(height)height->name.set(S+"Height: "+video.height());
      if(kbps  )kbps  ->name.set(S+"Kbps: "  +DivRound(video.bitRate(), 1000));
      if(fps   )fps   ->name.set(S+"FPS: "   +TextReal(video.fps(), -1));
      if(codec )codec ->name.set(S+"Codec: " +video.codecName());
   }
   void VideoEditor::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(gpc.visible && visible())setInfo();
   }
   VideoEditor& VideoEditor::hide(            )  {set(null); ::PropWin::hide(); return T;}
   Rect         VideoEditor::sizeLimit(            )C {Rect r=::EE::Window::sizeLimit(); r.min.set(1.0f, 0.45f); return r;}
   VideoEditor& VideoEditor::rect(C Rect &rect)  
{
      ::EE::Window::rect(rect);
      flt  x=0; if(props.elms())x=0.22f; //props[0].button.rect().max.x;
      Rect r(x, -clientHeight(), clientWidth(), 0); r.extend(-0.02f);
      custom.rect(r);
      return T;
   }
   void VideoEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmVideo *data=elm->videoData())data->newVer(); // modify just before saving/sending in case we've received data from server after edit
         Preview.elmChanged(elm->id);
      }
      changed=false;
   }
   void VideoEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmVideo *data=elm->videoData())data->newVer();
      }
   }
   void VideoEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_VIDEO)elm=null;
      if(T.elm!=elm)
      {
         flush();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm){video.create(Proj.gamePath(*elm), true); video_time=0;}else video.del(); // start from the beginning to avoid freezes
         setInfo();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void VideoEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void VideoEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void VideoEditor::closeElm(C UID &elm_id) {if(elm && elm->id==elm_id)video.del();}
   void VideoEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
   void VideoEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         video.create(Proj.gamePath(*elm), true); video_time=0; setInfo(); // start from the beginning to avoid freezes
      }
   }
VideoEditor::VideoEditor() : elm_id(UIDZero), elm(null), changed(false), video_time(0), width(null), height(null), kbps(null), fps(null), codec(null) {}

/******************************************************************************/
