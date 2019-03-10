/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
AnimEditor AnimEdit;
/******************************************************************************/


/******************************************************************************/
   cchar8 *AnimEditor::event_op_t[]=
   {
      "Move",
      "Rename",
      "Insert",
      "Delete",
   };
   cchar8 *AnimEditor::transform_obj_dialog_id="animateObj";
/******************************************************************************/
   flt AnimEditor::TimeEps()
   {
      flt w=AnimEdit.track.rect().w(); w=(w ? 1/w : 0);
      if(AnimEdit.anim)w*=AnimEdit.anim->length();
      return w*0.025f;
   }
      void AnimEditor::Change::create(ptr user)
{
         if(AnimEdit.anim)anim=*AnimEdit.anim;
         if(AnimEdit.skel)skel=*AnimEdit.skel;
         if(ElmAnim *anim_data=AnimEdit.data())T.anim_data=*anim_data;
         AnimEdit.undoVis();
      }
      void AnimEditor::Change::apply(ptr user)
{
         if(AnimEdit.anim)if(ElmAnim *anim_data=AnimEdit.data())
         {
            CacheLock cl(Animations);
            Pose pose=anim_data->transform; // preserve pose, keep what we already have
            if(Elm *skel=Proj.findElm(T.anim_data.skel_id))if(ElmSkel *skel_data=skel->skelData())pose=skel_data->transform; // if the undo used a different skeleton, then we have to set animation pose to match it, because we will use it now, so always when the animation is linked to a skeleton then use its pose
           *AnimEdit.anim=anim;
            AnimEdit.anim->transform(GetTransform(T.anim_data.transform(), pose()), skel); // transform from undo matrix to new matrix
            anim_data->undo(T.anim_data); anim_data->undoSrcFile(T.anim_data); // call 'undoSrcFile' separately because it may not be called in 'undo', however we need it in case speed adjustment was performed
            anim_data->transform=pose;
            AnimEdit.setChanged(false);
            AnimEdit.toGui();
            AnimEdit.undoVis();
            AnimEdit.prepMeshSkel();
            AnimEdit.setOrnTarget();
            Proj.refresh(false, false); // refresh any invalid skeleton link references
         }
      }
      flt AnimEditor::Track::screenToTime(flt x)C
      {
         if(C Animation *anim=AnimEdit.anim)
         {
            Rect r=screenRect();
            return Frac(LerpR(r.min.x, r.max.x, x))*anim->length();
         }
         return 0;
      }
      ::AnimEditor::Track& AnimEditor::Track::create(bool events)
      {
         ::EE::GuiCustom::create();
         T.events=events;
         return T;
      }
      flt AnimEditor::Track::ElmPosX(C Rect &rect, flt time)
      {
         return rect.lerpX(AnimEdit.timeToFrac(time));
      }
      Rect AnimEditor::Track::ElmRect(C Rect &rect, flt time)
      {
         return Rect_C(rect.lerp(AnimEdit.timeToFrac(time), 0.5f), 0.01f, rect.h());
      }
      flt AnimEditor::Track::Slide(flt &time, flt dt, flt length)
      {
         return time=Frac(time+dt, length);
      }
      void AnimEditor::Track::update(C GuiPC &gpc)
{
         AnimEdit.preview.event_lit=-1;
         if(gpc.visible && visible() && Gui.ms()==this)if(Animation *anim=AnimEdit.anim)
         {
            Rect r=rect()+gpc.offset;
            flt  time=Frac(LerpR(r.min.x, r.max.x, Ms.pos().x))*anim->length();

            if(events)
            {
               // get lit
               flt dist=0;
               if(AnimEdit.preview.event_op()>=0 && AnimEdit.preview.event_op()!=EVENT_NEW)REPA(anim->events)
               {
                  flt d=Abs(Ms.pos().x - ElmPosX(r, anim->events[i].time));
                  if(AnimEdit.preview.event_lit<0 || d<dist){AnimEdit.preview.event_lit=i; dist=d;}
               }
               if(dist>0.05f)AnimEdit.preview.event_lit=-1;

               if(AnimEdit.preview.event_op()==EVENT_RENAME && Ms.bp(0))RenameEvent.activate(AnimEdit.preview.event_lit);
               if(AnimEdit.preview.event_op()==EVENT_DEL    && Ms.bp(0))AnimEdit   .delEvent(AnimEdit.preview.event_lit);
               if(AnimEdit.preview.event_op()==EVENT_NEW    && Ms.bp(0))AnimEdit   .newEvent(time);
               if(AnimEdit.preview.event_op()==EVENT_MOVE   && Ms.b (0))
               {
                  if(Ms.bp(0))AnimEdit.preview.event_sel=AnimEdit.preview.event_lit; AnimEdit.moveEvent(AnimEdit.preview.event_sel, time);
                              AnimEdit.preview.event_lit=AnimEdit.preview.event_sel;
               }

               // set time
               if(AnimEdit.preview.event_op()<0 && Ms.b(0))AnimEdit.animTime(time);
            }else
            {
               // set time
               if(Ms.b(0))AnimEdit.animTime(time);

               // move KeyFrame
               if(Ms.b(1))if(AnimKeys *keys=AnimEdit.findKeys(AnimEdit.sel_bone))
               {
                  AnimKeys *keys_parent=AnimEdit.findKeysParent(AnimEdit.sel_bone, false); // skip sliding for root
                  AnimEdit.undos.set("slide");
                  flt dt=Ms.dc().x*anim->length()/rect().w();
                  if(Kb.ctrlCmd()) // all
                  {
                     if(AnimEdit.sel_bone<0) // entire animation
                     {
                        if(AnimEdit.anim->keys.is())dt=(Ms.bp(1) ? -AnimEdit.animTime() : 0); // if have any root keys, then call only one time at the start
                        AnimEdit.anim->slideTime(dt);
                     }else
                     {
                        REPA(keys->orns  )Slide(keys->orns  [i].time, dt, anim->length());
                        REPA(keys->poss  )Slide(keys->poss  [i].time, dt, anim->length());
                        REPA(keys->scales)Slide(keys->scales[i].time, dt, anim->length());
                        if(AnimEdit.op()==OP_ORN2 && keys_parent)
                        {
                           REPA(keys_parent->orns  )Slide(keys_parent->orns  [i].time, dt, anim->length());
                           REPA(keys_parent->poss  )Slide(keys_parent->poss  [i].time, dt, anim->length());
                           REPA(keys_parent->scales)Slide(keys_parent->scales[i].time, dt, anim->length());
                        }
                     }
                     AnimEdit.key_time=AnimEdit.animTime();
                  }else
                  {
                     if(Ms.bp(1))AnimEdit.key_time=AnimEdit.animTime();
                     switch(AnimEdit.op())
                     {
                        case OP_ORN  : if(AnimKeys::Orn   *key=FindOrn  (*keys, AnimEdit.key_time))AnimEdit.key_time=Slide(key->time, dt, anim->length());else AnimEdit.key_time=AnimEdit.animTime(); break;
                        case OP_POS  : if(AnimKeys::Pos   *key=FindPos  (*keys, AnimEdit.key_time))AnimEdit.key_time=Slide(key->time, dt, anim->length());else AnimEdit.key_time=AnimEdit.animTime(); break;
                        case OP_SCALE: if(AnimKeys::Scale *key=FindScale(*keys, AnimEdit.key_time))AnimEdit.key_time=Slide(key->time, dt, anim->length());else AnimEdit.key_time=AnimEdit.animTime(); break;
                        case OP_ORN2 :
                        {
                           if(AnimKeys::Orn *key=FindOrn(*keys, AnimEdit.key_time))
                           {
                              if(keys_parent)if(AnimKeys::Orn *key=FindOrn(*keys_parent, AnimEdit.key_time))Slide(key->time, dt, anim->length()); // !! first process the parent without modifying the 'key_time' !!
                                                                                        AnimEdit.key_time =Slide(key->time, dt, anim->length());
                           }else AnimEdit.key_time=AnimEdit.animTime();
                        }break;
                     }
                  }
                  keys->sortFrames().setTangents(anim->loop(), anim->length());
                  anim->setRootMatrix();
                  flt t=AnimEdit.animTime(); AnimEdit.animTime(Slide(t, dt, anim->length()));
                  AnimEdit.setChanged();
               }
            }
         }
      }
      void AnimEditor::Track::DrawKey(flt time, C Rect &rect, flt y, flt r)
      {
         VI.dot(Vec2(ElmPosX(rect, time), y), r);
      }
      void AnimEditor::Track::DrawKeys(AnimKeys &keys, C Rect &rect, C Color &color, flt y, flt r, int row, bool lit)
      {
         VI.color(color);
         switch(row)
         {
            case TRACK_ORN  : REPA(keys.orns  )DrawKey(keys.orns  [i].time, rect, y, r); break;
            case TRACK_POS  : REPA(keys.poss  )DrawKey(keys.poss  [i].time, rect, y, r); break;
            case TRACK_SCALE: REPA(keys.scales)DrawKey(keys.scales[i].time, rect, y, r); break;
         }
         VI.end();

         if(lit)switch(row)
         {
            case TRACK_ORN  : if(AnimEdit.op()==OP_ORN || AnimEdit.op()==OP_ORN2)if(AnimKeys::Orn   *key=FindOrn  (keys, AnimEdit.animTime())){VI.color(WHITE); DrawKey(key->time, rect, y, r); VI.end();} break;
            case TRACK_POS  : if(AnimEdit.op()==OP_POS                          )if(AnimKeys::Pos   *key=FindPos  (keys, AnimEdit.animTime())){VI.color(WHITE); DrawKey(key->time, rect, y, r); VI.end();} break;
            case TRACK_SCALE: if(AnimEdit.op()==OP_SCALE                        )if(AnimKeys::Scale *key=FindScale(keys, AnimEdit.animTime())){VI.color(WHITE); DrawKey(key->time, rect, y, r); VI.end();} break;
         }
      }
      void AnimEditor::Track::draw(C GuiPC &gpc)
{
         if(gpc.visible && visible())
         {
            D.clip(gpc.clip);
            Rect r=rect()+gpc.offset;
            r.draw(GREY);
            if(C Animation *anim=AnimEdit.getVisAnim())
            {
               ElmRect(r, AnimEdit.animTime()).draw(BLACK); // draw time position
               if(events)
               {
                  TextStyleParams ts; ts.size=0.05f; ts.align.set(0, 1); ts.color=ColorAlpha(0.6f);
                  FREPA(anim->events) // draw events
                  {
                   C AnimEvent &event=anim->events[i];
                     Rect e=ElmRect(r, event.time);
                     e.draw((AnimEdit.preview.event_lit==i) ? LitColor : LitSelColor); if(AnimEdit.preview.event_op()>=0 && AnimEdit.preview.event_lit!=i)D.text(ts, e.up(), event.name);
                  }
                  if(AnimEdit.preview.event_op()>=0 && InRange(AnimEdit.preview.event_lit, anim->events)) // draw highlighted event last to be on top of others
                  {
                   C AnimEvent &event=anim->events[AnimEdit.preview.event_lit];
                     ts.resetColors(false); ts.size*=1.3f; D.text(ts, ElmRect(r, event.time).up(), event.name);
                  }
               }else
               {
                  r.draw(Color(0, 64), false);
                  
                  AnimKeys *sel_keys=AnimEdit.findVisKeys(AnimEdit.sel_bone),
                           *lit_keys=AnimEdit.findVisKeys(AnimEdit.lit_bone, false); // don't display root keys for highlighted
                  const flt s=r.h()/(TRACK_NUM+1)/2;
                  FREP(TRACK_NUM)
                  {
                     bool sel=false;
                     switch(AnimEdit.op())
                     {
                        case OP_ORN  :
                        case OP_ORN2 : sel=(i==TRACK_ORN  ); break;
                        case OP_POS  : sel=(i==TRACK_POS  ); break;
                        case OP_SCALE: sel=(i==TRACK_SCALE); break;
                     }
                     flt y=r.lerpY((TRACK_NUM-i)/flt(TRACK_NUM+1));
                     D.lineX(ColorAlpha(WHITE, sel ? 0.5f : 0.19f), y, r.min.x, r.max.x);
                     Color color=ColorI(i);
                     if(lit_keys)DrawKeys(*lit_keys, r, ColorAlpha(Lerp(color, WHITE, 0.6f), 0.5f), y, s*1.1f, i);
                     if(sel_keys)DrawKeys(*sel_keys, r,                 color                   , y, s*0.5f, i, true);
                  }

                  D.clip();
                  Vec2         p=AnimEdit.screenPos()+Vec2(0.01f, -0.25f);
                  int       bone=AnimEdit.lit_bone;
                  Skeleton *skel=AnimEdit.skel;
                                                       D.text(ObjEdit.ts, p, S+"Time: "+AnimEdit.animTime()+'/'+anim->length()+"s ("+Round(AnimEdit.timeToFrac(AnimEdit.animTime())*100)+"%)"); p.y-=ObjEdit.ts.size.y;
                  if(skel && InRange(bone, skel->bones))D.text(ObjEdit.ts, p, S+"Bone \""+skel->bones[bone].name+"\", Parent: "+(InRange(skel->bones[bone].parent, skel->bones) ? S+'"'+skel->bones[skel->bones[bone].parent].name+'"' : S+"none"));
                  if(Gui.ms()==this)
                  {
                     flt frac=Frac(LerpR(r.min.x, r.max.x, Ms.pos().x)), time=frac*anim->length();
                     TextStyleParams ts; ts.align.set(0, 1); ts.size=0.052f;
                     Str t=S+time+"s ("+Round(frac*100)+"%)";
                     flt w_2=ts.textWidth(t)/2, x=Ms.pos().x; Clamp(x, r.min.x+w_2, r.max.x-w_2);
                     D.text(ts, x, r.max.y, t);
                  }
               }
            }
         }
      }
      void AnimEditor::Preview::Play(  Preview &editor, C Str &t) {AnimEdit.play.set(TextBool(t));}
      Str  AnimEditor::Preview::Play(C Preview &editor          ) {return AnimEdit.play();}
      void AnimEditor::Preview::Loop(  Preview &editor, C Str &t) {AnimEdit.setLoop(TextBool(t));}
      Str  AnimEditor::Preview::Loop(C Preview &editor          ) {if(ElmAnim *d=AnimEdit.data())return d->loop(); return false;}
      void AnimEditor::Preview::Linear(  Preview &editor, C Str &t) {AnimEdit.setLinear(TextBool(t));}
      Str  AnimEditor::Preview::Linear(C Preview &editor          ) {if(ElmAnim *d=AnimEdit.data())return d->linear(); return false;}
      void AnimEditor::Preview::Target(  Preview &editor, C Str &t) {AnimEdit.setTarget(t);}
      Str  AnimEditor::Preview::Target(C Preview &editor          ) {return Proj.elmFullName(Proj.animToObj(AnimEdit.elm));}
      void AnimEditor::Preview::Split(  Preview &editor          ) {SplitAnim.activate(AnimEdit.elm_id);}
      void AnimEditor::Preview::Speed(  Preview &editor          ) {AnimEdit.applySpeed();}
      void AnimEditor::Preview::Render()
      {
         switch(Renderer())
         {
            case RM_PREPARE:
            {
               if(AnimEdit.mesh)AnimEdit.mesh->draw(AnimEdit.anim_skel);

               LightDir(!(ActiveCam.matrix.z*2+ActiveCam.matrix.x-ActiveCam.matrix.y), 1-D.ambientColor()).add(false);
            }break;
         }
      }
      void AnimEditor::Preview::Draw(Viewport &viewport) {AnimEdit.preview.draw();}
             void AnimEditor::Preview::draw()
      {
         AnimEdit.prepMeshSkel();
         if(C Skeleton *skel=AnimEdit.skel)
         {
          C MeshPtr          &mesh     =AnimEdit.mesh;
          C AnimatedSkeleton &anim_skel=AnimEdit.anim_skel;

            // remember settings
            CurrentEnvironment().set();
            AMBIENT_MODE ambient  =D.  ambientMode(); D.  ambientMode(AMBIENT_FLAT);
            DOF_MODE     dof      =D.      dofMode(); D.      dofMode(    DOF_NONE);
            bool         eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(       false);
            bool         astros   =AstrosDraw       ; AstrosDraw     =false;
            bool         ocean    =Water.draw       ; Water.draw     =false;
            flt fov=D.viewFov(), from=D.viewFrom(), range=D.viewRange();

            // render
            Box box(0); if(mesh){if(mesh->is())box=mesh->ext;else if(skel)box=*skel;}
            flt dist=Max(0.1f, GetDist(box));
            D.viewFrom(dist*0.01f).viewRange(dist*24);
            SetCam(cam, box, cam_yaw, cam_pitch, cam_zoom);
            Renderer(Preview::Render);

            Renderer.setDepthForDebugDrawing();
            
            bool line_smooth=D.lineSmooth(true); // this can be very slow, so don't use it everywhere
            SetMatrix();
            if(draw_plane){bool line_smooth=D.lineSmooth(false); Plane(VecZero, Vec(0, 1, 0)).drawInfiniteByResolution(Color(255, 128), 192); D.lineSmooth(line_smooth);} // disable line smoothing because it can be very slow for lots of full-screen lines
            if(draw_axis ){D.depthLock(false); MatrixIdentity.draw(); D.depthUnlock();}
            if(draw_bones || draw_slots)
            {
               D.depthLock(false);
               anim_skel.draw(draw_bones ? ColorAlpha(CYAN, 0.75f) : TRANSPARENT, draw_slots ? ORANGE : TRANSPARENT);
               D.depthUnlock();
            }
            D.lineSmooth(line_smooth);

            // restore settings
            D.viewFov(fov).viewFrom(from).viewRange(range);
            D.      dofMode(dof      );
            D.  ambientMode(ambient  );
            D.eyeAdaptation(eye_adapt);
            AstrosDraw     =astros;
            Water.draw     =ocean;
         }
      }
      Animation* AnimEditor::Preview::anim()C {return AnimEdit.anim;}
      void AnimEditor::Preview::create()
      {
         Property *play, *obj;
 length=&add(); // length
         add("Loop"         , MemberDesc(DATA_BOOL).setFunc(Loop  , Loop  )).desc("If animation is looped");
         add("Linear"       , MemberDesc(DATA_BOOL).setFunc(Linear, Linear)).desc("If KeyFrame interpolation should be Linear, if disabled then Cubic is used.\nLinear is recommended for animations with high framerate.\nCubic is recommended for animations with low framerate.\n\nLinear interpolation is calculated faster than Cubic.\nAnimations with Linear interpolation can be optimized for smaller size than Cubic.\nIf the animation has low framerate, then Cubic will give smoother results than Linear.");
    obj=&add("Target Object", MemberDesc(DATA_STR ).setFunc(Target, Target)).desc("Drag and drop an object here to set it").elmType(ELM_OBJ);
         add();
         add("Display:");
   play=&add("Play"         , MemberDesc(DATA_BOOL).setFunc(Play, Play)).desc(S+"Keyboard Shortcut: "+Kb.ctrlCmdName()+"+P, "+Kb.ctrlCmdName()+"+D, Space");
         add("Anim Speed"   , MEMBER(AnimEditor, anim_speed)).range(-100, 100).desc("Animation speed");
         add("Bones"        , MEMBER(AnimEditor, preview.draw_bones)).desc("Keyboard Shortcut: Alt+B");
         add("Slots"        , MEMBER(AnimEditor, preview.draw_slots));
         add("Axis"         , MEMBER(AnimEditor, preview.draw_axis )).desc("Keyboard Shortcut: Alt+A");
         add("Plane"        , MEMBER(AnimEditor, preview.draw_plane)).desc("Display ground plane\nKeyboard Shortcut: Alt+P");
  event=&add("Events:");
         autoData(&AnimEdit);
         flt  h=0.043f;
         Rect r=::PropWin::create("Animation Editor", Vec2(0.02f, -0.02f), 0.036f, h, 0.35f); button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
         obj->textline.resize(Vec2(h, 0));
         prop_max_x=r.max.x;
         T+=split.create(Rect_RU(r.max.x, r.max.y, 0.15f, 0.055f), "Split").func(Split, T).desc(S+"Split Animation\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+S");
         T+=apply_speed.create(Rect_R(r.max.x, play->name.rect().centerY(), 0.23f, h), "Apply Speed").func(Speed, T).desc("Animation length/speed will be adjusted according to current \"Anim Speed\".");
         T+=edit.create(Rect_L(r.max.x/2, r.min.y, 0.17f, 0.055f), "Edit").func(Fullscreen, AnimEdit).focusable(false).desc(S+"Edit Animation KeyFrames\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+E");
         T+=locate.create(Rect_R(r.max.x/2-0.01f, r.min.y, 0.17f, 0.055f), "Locate").func(Locate, AnimEdit).focusable(false).desc("Locate this element in the Project");
         T+=viewport.create(Draw); viewport.fov=PreviewFOV;
         T+=event_op.create(Rect_LU(0.02f, 0, 0.45f, 0.110f), 0, event_op_t, Elms(event_op_t));
         event_op.tab(EVENT_MOVE  ).rect(Rect(event_op.rect().lerp(0  , 0.5f), event_op.rect().lerp(0.5f, 1  ))).desc("Move event\nKeyboard Shortcut: F1");
         event_op.tab(EVENT_RENAME).rect(Rect(event_op.rect().lerp(0.5f, 0.5f), event_op.rect().lerp(1  , 1  ))).desc("Rename event\nKeyboard Shortcut: F2");
         event_op.tab(EVENT_NEW   ).rect(Rect(event_op.rect().lerp(0  , 0  ), event_op.rect().lerp(0.5f, 0.5f))).desc("Create new event\nHold Ctrl to don't show Rename Window\nKeyboard Shortcut: F3");
         event_op.tab(EVENT_DEL   ).rect(Rect(event_op.rect().lerp(0.5f, 0  ), event_op.rect().lerp(1  , 0.5f))).desc("Delete event\nKeyboard Shortcut: F4");
         T+=track.create(true);
         rect(Rect_C(0, 0, Min(1.7f, D.w()*2), Min(1.07f, D.h()*2)));
      }
      void AnimEditor::Preview::toGui()
      {
         ::PropWin::toGui();
         if(length)length->name.set(S+"Length: "+(anim() ? S+TextReal(anim()->length(), -2)+"s" : S));
      }
      void AnimEditor::Preview::removedEvent(int index)
      {
         if(event_lit==index)event_lit=-1;else if(event_lit>index)event_lit--;
         if(event_sel==index)event_sel=-1;else if(event_sel>index)event_sel--;
      }
      ::AnimEditor::Preview& AnimEditor::Preview::hide(            )  {if(!AnimEdit.fullscreen)AnimEdit.set(null); ::PropWin::hide(); return T;}
      Rect     AnimEditor::Preview::sizeLimit(            )C {Rect r=::EE::Window::sizeLimit(); r.min.set(1.0f, 0.9f); return r;}
      ::AnimEditor::Preview& AnimEditor::Preview::rect(C Rect &rect)  
{
         ::EE::Window::rect(rect);
         track   .rect(Rect(prop_max_x, -clientHeight(), clientWidth(), -clientHeight()+0.09f).extend(-0.02f));
         viewport.rect(Rect(prop_max_x, track.rect().max.y, clientWidth(), 0).extend(-0.02f));
         event_op.move(Vec2(0, track.rect().min.y-event_op.rect().min.y));
         if(event)event->name.pos(event_op.rect().lu()+Vec2(0, 0.025f));
         return T;
      }
      void AnimEditor::Preview::update(C GuiPC &gpc)
{
         flt old_time=AnimEdit.animTime();
         if(visible() && gpc.visible)
         {
            if(contains(Gui.kb()))
            {
               KbSc edit(KB_E, KBSC_CTRL_CMD), split(KB_S, KBSC_CTRL_CMD), play(KB_P, KBSC_CTRL_CMD), play2(KB_D, KBSC_CTRL_CMD), bones(KB_B, KBSC_ALT), axis(KB_A, KBSC_ALT), plane(KB_P, KBSC_ALT);
               if(Kb.bp(KB_F1))event_op.toggle(0);else
               if(Kb.bp(KB_F2))event_op.toggle(1);else
               if(Kb.bp(KB_F3))event_op.toggle(2);else
               if(Kb.bp(KB_F4))event_op.toggle(3);
               if(edit .pd()){edit .eat(); AnimEdit.toggleFullscreen();}else
               if(split.pd()){split.eat(); Split(T);}else
               if(bones.pd()){bones.eat(); draw_bones^=true; toGui();}else
               if(axis .pd()){axis .eat(); draw_axis ^=true; toGui();}else
               if(plane.pd()){plane.eat(); draw_plane^=true; toGui();}else
               if(play .pd() || play2.pd()){play.eat(); play2.eat(); AnimEdit.playToggle();}
               if(Gui.kb()->type()!=GO_TEXTLINE && Gui.kb()->type()!=GO_CHECKBOX && Kb.bp(KB_SPACE)){Kb.eat(KB_SPACE); AnimEdit.playToggle();}
            }
            if(Gui.ms   ()==&viewport)if(Ms.b(0) || Ms.b(4)){cam_yaw-=Ms.d().x; cam_pitch+=Ms.d().y; Ms.freeze();}
            if(Gui.wheel()==&viewport)Clamp(cam_zoom*=ScaleFactor(Ms.wheel()*-0.2f), 0.1f, 32);
            REPA(Touches)if(Touches[i].guiObj()==&viewport && Touches[i].on()){cam_yaw-=Touches[i].ad().x*2.0f; cam_pitch+=Touches[i].ad().y*2.0f;}

            AnimEdit.playUpdate(time_speed);
         }
         ::EE::ClosableWindow::update(gpc); // process after adjusting 'animTime' so clicking on anim track will not be changed afterwards

         flt delta_t=Time.ad()*time_speed; // use default delta
         if(AnimEdit.anim) // if we have animation
            if(flt length=AnimEdit.anim->length()) // if it has time
               {delta_t=Frac(AnimEdit.animTime()-old_time, length); if(delta_t>=length*0.5f)delta_t-=length;} // use delta based on anim time delta
      }
      void AnimEditor::OptimizeAnim::Changed(C Property &prop) {AnimEdit.optimize_anim.refresh();}
      void AnimEditor::OptimizeAnim::Optimize(OptimizeAnim &oa)
      {
         if(AnimEdit.anim)
         {
            AnimEdit.undos.set("optimize");
            oa.optimizeDo(*AnimEdit.anim);
            AnimEdit.setChanged();
         }
         oa.hide();
      }
      void AnimEditor::OptimizeAnim::optimizeDo(Animation &anim)
      {
         anim.optimize(angle_eps, pos_eps, scale_eps);
      }
      Animation* AnimEditor::OptimizeAnim::getAnim()
      {
         Animation *src=AnimEdit.anim;
         if(refresh_needed)
         {
            refresh_needed=false;
            if(src)anim=*src;else anim.del();
            File f;
            if(     file_size){anim.save(f.writeMem());      file_size->name.set(S+     "File Size: "+FileSizeKB(f.size()));}
            optimizeDo(anim);
            if(optimized_size){anim.save(f.reset   ()); optimized_size->name.set(S+"Optimized Size: "+FileSizeKB(f.size()));}
         }
         return preview ? &anim : src;
      }
      void AnimEditor::OptimizeAnim::refresh() {refresh_needed=true;}
      ::AnimEditor::OptimizeAnim& AnimEditor::OptimizeAnim::create()
      {
         add("Preview"           , MEMBER(OptimizeAnim,   preview));
         add("Angle Tolerance"   , MEMBER(OptimizeAnim, angle_eps)).changed(Changed).mouseEditSpeed(0.001f ).range(0, PI);
         add("Position Tolerance", MEMBER(OptimizeAnim,   pos_eps)).changed(Changed).mouseEditSpeed(0.0001f).min(0);
         add("Scale Tolerance"   , MEMBER(OptimizeAnim, scale_eps)).changed(Changed).mouseEditSpeed(0.0001f).min(0);
              file_size=&add();
         optimized_size=&add();
         Rect r=::PropWin::create("Reduce Keyframes", Vec2(0.02f, -0.02f), 0.040f, 0.046f); button[2].show();
         autoData(this);
         resize(Vec2(0, 0.1f));
         T+=optimize.create(Rect_D(clientWidth()/2, -clientHeight()+0.03f, 0.2f, 0.06f), "Optimize").func(Optimize, T);
         hide();
         return T;
      }
      void AnimEditor::TimeRangeSpeed::Apply(TimeRangeSpeed &tsr)
      {
         if(AnimEdit.anim && tsr.speed)
         {
            AnimEdit.undos.set("trs");
            AnimEdit.anim->scaleTime(tsr.start, tsr.end, 1/tsr.speed);
            AnimEdit.setChanged();
         }
         tsr.hide();
      }
      ::AnimEditor::TimeRangeSpeed& AnimEditor::TimeRangeSpeed::create()
      {
         st=&add("Start Time", MEMBER(TimeRangeSpeed, start));
         e =&add("End Time"  , MEMBER(TimeRangeSpeed, end  ));
         sp=&add("Speed"     , MEMBER(TimeRangeSpeed, speed));
         Rect r=::PropWin::create("Time Range Speed", Vec2(0.02f, -0.02f), 0.040f, 0.046f); button[2].show();
         autoData(this);
         resize(Vec2(0, 0.1f));
         T+=apply.create(Rect_D(clientWidth()/2, -clientHeight()+0.03f, 0.2f, 0.06f), "Apply").func(Apply, T);
         hide();
         return T;
      }
      void AnimEditor::TimeRangeSpeed::display()
      {
         if(AnimEdit.anim)
         {
            st->range(0, AnimEdit.anim->length()).set(AnimEdit.animTime());
            e ->range(0, AnimEdit.anim->length()).set(AnimEdit.track.screenToTime(Ms.pos().x));
            sp->set(AnimEdit.anim_speed);
            activate();
         }
      }
   void AnimEditor::Fullscreen(AnimEditor &editor) {editor.toggleFullscreen();}
   void AnimEditor::Render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            if(AnimEdit.draw_mesh() && AnimEdit.mesh)AnimEdit.mesh->draw(AnimEdit.anim_skel);

            LightDir(!(ActiveCam.matrix.z*2+ActiveCam.matrix.x-ActiveCam.matrix.y), 1-D.ambientColor()).add(false);
         }break;
      }
   }
   void AnimEditor::Draw(Viewport &viewport) {if(Edit::Viewport4::View *view=AnimEdit.v4.getView(&viewport))AnimEdit.draw(*view);}
          void AnimEditor::draw(Edit::Viewport4::View &view)
   {
      prepMeshSkel();
      view.camera.set();
      CurrentEnvironment().set();
      bool astros=AstrosDraw; AstrosDraw=false;
      bool ocean =Water.draw; Water.draw=false;
      Renderer.wire=wire();

      Renderer(AnimEditor::Render);

      Renderer.wire=false;
      AstrosDraw=astros;
      Water.draw=ocean;

      // helpers using depth buffer
      bool line_smooth=D.lineSmooth(true); // this can be very slow, so don't use it everywhere
      Renderer.setDepthForDebugDrawing();
      if(     axis ()){SetMatrix(); MatrixIdentity.draw();}
      if(show_grid ()){SetMatrix(); bool line_smooth=D.lineSmooth(false); int size=8; Plane(VecZero, Vec(0, 1, 0)).drawLocal(Color(255, 128), size, false, size*(3*2)); D.lineSmooth(line_smooth);} // disable line smoothing because it can be very slow for lots of full-screen lines
      if(draw_bones())
      {
         SetMatrix();
         D.depthLock(false);
         int sel_bone2=((op()==OP_ORN2) ? boneParent(sel_bone) : -1);
         REPA(anim_skel.bones)
         {
            SkelBone bone=transformedBone(i);
            bool has_keys=false; if(AnimKeys *keys=findVisKeys(i))switch(op())
            {
               case OP_ORN  :
               case OP_ORN2 : has_keys=(keys->orns  .elms()>0); break;
               case OP_POS  : has_keys=(keys->poss  .elms()>0); break;
               case OP_SCALE: has_keys=(keys->scales.elms()>0); break;
            }
            Color col=(has_keys ? PURPLE : CYAN);
            bool  lit=(lit_bone==i), sel=(sel_bone==i || sel_bone2==i);
            if(lit && sel)col=LitSelColor;else 
            if(       sel)col=   SelColor;else
            if(lit       )col=      col  ;else 
                          col.a=140;
            bone.draw(col);
         }
       //if(skel && InRange(sel_bone, skel.bones)) don't check this so we can draw just for root too
         {
            if(op()==OP_ORN || op()==OP_ORN2 )orn_target.draw(RED, 0.005f);
            if(op()==OP_POS || op()==OP_SCALE
           || (op()==OP_ORN || op()==OP_ORN2 || (anim && anim->keys.is()) || settings()==0) && sel_bone<0 // always draw axis for root
            )DrawMatrix(transformedBoneAxis(sel_bone), bone_axis);
         }
         if(sel_bone>=0 && settings()==0)DrawMatrix(transformedBoneAxis(-1), -1); // always draw axis for root when settings are visible
         D.depthUnlock();
      }
      D.lineSmooth(line_smooth);
   }
   void AnimEditor::undoVis() {SetUndo(undos, undo, redo);}
   void AnimEditor::PrevAnim(AnimEditor &editor) {Proj.elmNext(editor.elm_id, -1);}
   void AnimEditor::NextAnim(AnimEditor &editor) {Proj.elmNext(editor.elm_id);}
   void AnimEditor::Mode1(AnimEditor &editor) {editor.op.toggle(0);}
   void AnimEditor::Mode2(AnimEditor &editor) {editor.op.toggle(1);}
   void AnimEditor::Mode3(AnimEditor &editor) {editor.op.toggle(2);}
   void AnimEditor::Mode4(AnimEditor &editor) {editor.op.toggle(3);}
   void AnimEditor::Play(AnimEditor &editor) {editor.playToggle();}
   void AnimEditor::Identity(AnimEditor &editor) {editor.axis    .push();}
   void AnimEditor::Settings(AnimEditor &editor) {editor.settings.toggle(0);}
   void AnimEditor::Start(AnimEditor &editor) {               editor.animTime(0);}
   void AnimEditor::End(AnimEditor &editor) {if(editor.anim)editor.animTime(editor.anim->length());}
   void AnimEditor::PrevFrame(AnimEditor &editor) {editor.frame(-1);}
   void AnimEditor::NextFrame(AnimEditor &editor) {editor.frame(+1);}
   void  AnimEditor::DelFrame(AnimEditor &editor) {editor.delFrame ();}
   void  AnimEditor::DelFrames(AnimEditor &editor) {editor.delFrames();}
   void  AnimEditor::DelFramesAtEnd(AnimEditor &editor) {editor.delFramesAtEnd();}
   void AnimEditor::Optimize(AnimEditor &editor) {editor.optimize_anim.activate();}
   void AnimEditor::TimeRangeSp(AnimEditor &editor) {editor.time_range_speed.display();}
   void AnimEditor::ReverseFrames(AnimEditor &editor) {editor.reverseFrames();}
   void AnimEditor::RemMovement(AnimEditor &editor) {editor.removeMovement();}
   void AnimEditor::FreezeBone(AnimEditor &editor) {editor.freezeBone();}
   void AnimEditor::Mirror(AnimEditor &editor) {if(editor.anim){editor.undos.set("mirror", true); Skeleton temp; editor.anim->mirror   (                            editor.skel ? *editor.skel : temp); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::RotX(AnimEditor &editor) {if(editor.anim){editor.undos.set("rot"   , true); Skeleton temp; editor.anim->transform(Matrix3().setRotateX(PI_2), editor.skel ? *editor.skel : temp); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::RotY(AnimEditor &editor) {if(editor.anim){editor.undos.set("rot"   , true); Skeleton temp; editor.anim->transform(Matrix3().setRotateY(PI_2), editor.skel ? *editor.skel : temp); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::RotZ(AnimEditor &editor) {if(editor.anim){editor.undos.set("rot"   , true); Skeleton temp; editor.anim->transform(Matrix3().setRotateZ(PI_2), editor.skel ? *editor.skel : temp); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::DrawBones(AnimEditor &editor) {editor.draw_bones.push();}
   void AnimEditor::DrawMesh(AnimEditor &editor) {editor.draw_mesh .push();}
   void AnimEditor::Grid(AnimEditor &editor) {editor.show_grid .push();}
   void AnimEditor::TransformObj(AnimEditor &editor)
   {
      Dialog &dialog=Gui.getMsgBox(transform_obj_dialog_id);
      dialog.set("Transform Object", "Warning: this option will open the original object in the Object Editor, clear undo levels (if any) and transform it, including its Mesh and Skeleton according to current animation. This cannot be undone. Are you sure you want to do this?", Memt<Str>().add("Yes").add("Cancel"));
      dialog.buttons[0].func(TransformObjDo, editor);
      dialog.buttons[1].func(Hide          , SCAST(GuiObj, dialog));
      dialog.activate();
   }
   void AnimEditor::TransformObjDo(AnimEditor &editor)
   {
      Gui.closeMsgBox(transform_obj_dialog_id);
      if(Elm *obj=Proj.animToObjElm(editor.elm))
      {
         ObjEdit.activate(obj);
         ObjEdit.animate(editor.anim_skel);
      }else Gui.msgBox(S, "There's no Object associated with this Animation.");
   }
   void AnimEditor::Undo(AnimEditor &editor) {editor.undos.undo();}
   void AnimEditor::Redo(AnimEditor &editor) {editor.undos.redo();}
   void AnimEditor::Locate(AnimEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void AnimEditor::Reload(AnimEditor &editor) {Proj.elmReload(editor.elm_id);}
   void AnimEditor::Loop(AnimEditor &editor) {editor.setLoop  (editor.loop  ());}
   void AnimEditor::Linear(AnimEditor &editor) {editor.setLinear(editor.linear());}
   void AnimEditor::RootDelPos(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootDelPos");
         bool on=!FlagTest(d->flag, ElmAnim::ROOT_DEL_POS);
         editor.root_del_pos_x.set(on, QUIET);
         editor.root_del_pos_y.set(on, QUIET);
         editor.root_del_pos_z.set(on, QUIET);
         FlagSet(d->flag, ElmAnim::ROOT_DEL_POS, on); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(on){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION); editor.setOrnTarget(); editor.toGui();} editor.setChanged();
      }
   }
   void AnimEditor::RootDelRot(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootDelRot");
         bool on=!FlagTest(d->flag, ElmAnim::ROOT_DEL_ROT);
         editor.root_del_rot_x.set(on, QUIET);
         editor.root_del_rot_y.set(on, QUIET);
         editor.root_del_rot_z.set(on, QUIET);
         FlagSet(d->flag, ElmAnim::ROOT_DEL_ROT, on); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(on){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION); editor.setOrnTarget(); editor.toGui();} editor.setChanged();
      }
   }
   void AnimEditor::RootDel(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())if(editor.skel)
      {
         editor.undos.set("rootDel");
         uint flag=d->flag;
         FlagEnable(d->flag, ElmAnim::ROOT_DEL_POS|ElmAnim::ROOT_DEL_ROT);
         if(d->flag!=flag)
         {
            editor.anim->adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, -1, null, ROOT_DEL_POSITION|ROOT_DEL_ROTATION); editor.setOrnTarget(); editor.toGui(); editor.setChanged();
         }
      }
   }
   void AnimEditor::RootDelPosX(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelPos"  ); FlagToggle(d->flag, ElmAnim::ROOT_DEL_POS_X); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d->flag&ElmAnim::ROOT_DEL_POS_X){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION_X); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   void AnimEditor::RootDelPosY(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelPos"  ); FlagToggle(d->flag, ElmAnim::ROOT_DEL_POS_Y); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d->flag&ElmAnim::ROOT_DEL_POS_Y){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION_Y); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   void AnimEditor::RootDelPosZ(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelPos"  ); FlagToggle(d->flag, ElmAnim::ROOT_DEL_POS_Z); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d->flag&ElmAnim::ROOT_DEL_POS_Z){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_POSITION_Z); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   void AnimEditor::RootDelRotX(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelRot"  ); FlagToggle(d->flag, ElmAnim::ROOT_DEL_ROT_X); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d->flag&ElmAnim::ROOT_DEL_ROT_X){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION_X); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   void AnimEditor::RootDelRotY(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelRot"  ); FlagToggle(d->flag, ElmAnim::ROOT_DEL_ROT_Y); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d->flag&ElmAnim::ROOT_DEL_ROT_Y){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION_Y); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   void AnimEditor::RootDelRotZ(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("rootDelRot"  ); FlagToggle(d->flag, ElmAnim::ROOT_DEL_ROT_Z); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d->flag&ElmAnim::ROOT_DEL_ROT_Z){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_DEL_ROTATION_Z); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   void AnimEditor::Root2Keys(AnimEditor &editor) {if(ElmAnim *d=editor.data()){editor.undos.set("root2Keys"   ); FlagToggle(d->flag, ElmAnim::ROOT_2_KEYS   ); /*d.file_time.getUTC(); already changed in 'setChanged' */ if(d->flag&ElmAnim::ROOT_2_KEYS   ){Skeleton temp, &skel=editor.skel ? *editor.skel : temp; editor.anim->adjustForSameTransformWithDifferentSkeleton(skel, skel, -1, null, ROOT_2_KEYS        ); editor.setOrnTarget(); editor.toGui();} editor.setChanged();}}
   void AnimEditor::RootFromBody(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootFromBody"); FlagToggle(d->flag, ElmAnim::ROOT_FROM_BODY); /*d.file_time.getUTC(); already changed in 'setChanged' */
         if(d->flag&ElmAnim::ROOT_FROM_BODY)if(editor.skel)
         {
            editor.anim->adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, Max(0, editor.skel->findBoneI(BONE_SPINE)), null, d->rootFlags()); editor.setOrnTarget(); editor.toGui();
         }
         editor.setChanged();
      }
   }
   void AnimEditor::RootFromBodyX(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())if(editor.skel)
      {
         editor.undos.set("rootFromBody");
         uint flag=d->flag;
         FlagDisable(d->flag, ElmAnim::ROOT_DEL_POS_X);
         FlagEnable (d->flag, ElmAnim::ROOT_DEL_POS_Y|ElmAnim::ROOT_DEL_POS_Z|ElmAnim::ROOT_DEL_ROT|ElmAnim::ROOT_FROM_BODY);
         if(d->flag!=flag)
         {
            editor.anim->adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, Max(0, editor.skel->findBoneI(BONE_SPINE)), null, d->rootFlags()); editor.setOrnTarget(); editor.toGui(); editor.setChanged();
         }
      }
   }
   void AnimEditor::RootFromBodyZ(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())if(editor.skel)
      {
         editor.undos.set("rootFromBody");
         uint flag=d->flag;
         FlagDisable(d->flag, ElmAnim::ROOT_DEL_POS_Z);
         FlagEnable (d->flag, ElmAnim::ROOT_DEL_POS_X|ElmAnim::ROOT_DEL_POS_Y|ElmAnim::ROOT_DEL_ROT|ElmAnim::ROOT_FROM_BODY);
         if(d->flag!=flag)
         {
            editor.anim->adjustForSameTransformWithDifferentSkeleton(*editor.skel, *editor.skel, Max(0, editor.skel->findBoneI(BONE_SPINE)), null, d->rootFlags()); editor.setOrnTarget(); editor.toGui(); editor.setChanged();
         }
      }
   }
   void AnimEditor::RootSetMove(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootMove");
         if(editor.root_set_move())
         {
            d->rootMove(editor.anim->rootTransform().pos/d->transform.scale);
            d->setRoot(*editor.anim);
            editor.setOrnTarget();
         }else d->rootMoveZero();
       /*d.file_time.getUTC(); already changed in 'setChanged' */
         editor.setChanged();
      }
   }
   void AnimEditor::RootSetRot(AnimEditor &editor)
   {
      if(ElmAnim *d=editor.data())
      {
         editor.undos.set("rootRot");
         if(editor.root_set_rot())
         {
            d->rootRot(editor.anim->rootTransform().axisAngle());
            d->setRoot(*editor.anim);
            editor.setOrnTarget();
         }else d->rootRotZero();
       /*d.file_time.getUTC(); already changed in 'setChanged' */
         editor.setChanged();
      }
   }
   Str  AnimEditor::RootMoveX(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d->rootMove())return d->root_move.x*d->transform.scale; if(editor.anim)return editor.anim->rootTransform().pos.x; return S;}
   Str  AnimEditor::RootMoveY(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d->rootMove())return d->root_move.y*d->transform.scale; if(editor.anim)return editor.anim->rootTransform().pos.y; return S;}
   Str  AnimEditor::RootMoveZ(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d->rootMove())return d->root_move.z*d->transform.scale; if(editor.anim)return editor.anim->rootTransform().pos.z; return S;}
   void AnimEditor::RootMoveX(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootMove"); editor.root_set_move.set(true, QUIET); if(!d->rootMove())d->root_move=editor.anim->rootTransform().pos; d->root_move.x=TextFlt(t)/d->transform.scale; d->rootMove(d->root_move); /*d.file_time.getUTC(); already changed in 'setChanged' */ d->setRoot(*editor.anim); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::RootMoveY(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootMove"); editor.root_set_move.set(true, QUIET); if(!d->rootMove())d->root_move=editor.anim->rootTransform().pos; d->root_move.y=TextFlt(t)/d->transform.scale; d->rootMove(d->root_move); /*d.file_time.getUTC(); already changed in 'setChanged' */ d->setRoot(*editor.anim); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::RootMoveZ(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootMove"); editor.root_set_move.set(true, QUIET); if(!d->rootMove())d->root_move=editor.anim->rootTransform().pos; d->root_move.z=TextFlt(t)/d->transform.scale; d->rootMove(d->root_move); /*d.file_time.getUTC(); already changed in 'setChanged' */ d->setRoot(*editor.anim); editor.setOrnTarget(); editor.setChanged();}}
   Str  AnimEditor::RootRotX(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d->rootRot())return TextReal(RadToDeg(d->root_rot.x), 1); if(editor.anim)return TextReal(RadToDeg(editor.anim->rootTransform().axisAngle().x), 1); return S;}
   Str  AnimEditor::RootRotY(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d->rootRot())return TextReal(RadToDeg(d->root_rot.y), 1); if(editor.anim)return TextReal(RadToDeg(editor.anim->rootTransform().axisAngle().y), 1); return S;}
   Str  AnimEditor::RootRotZ(C AnimEditor &editor) {if(ElmAnim *d=editor.data())if(d->rootRot())return TextReal(RadToDeg(d->root_rot.z), 1); if(editor.anim)return TextReal(RadToDeg(editor.anim->rootTransform().axisAngle().z), 1); return S;}
   void AnimEditor::RootRotX(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootRot"); editor.root_set_rot.set(true, QUIET); if(!d->rootRot())d->root_rot=editor.anim->rootTransform().axisAngle(); d->root_rot.x=DegToRad(TextFlt(t)); d->rootRot(d->root_rot); /*d.file_time.getUTC(); already changed in 'setChanged' */ d->setRoot(*editor.anim); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::RootRotY(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootRot"); editor.root_set_rot.set(true, QUIET); if(!d->rootRot())d->root_rot=editor.anim->rootTransform().axisAngle(); d->root_rot.y=DegToRad(TextFlt(t)); d->rootRot(d->root_rot); /*d.file_time.getUTC(); already changed in 'setChanged' */ d->setRoot(*editor.anim); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::RootRotZ(  AnimEditor &editor, C Str &t) {if(ElmAnim *d=editor.data()){editor.undos.set("rootRot"); editor.root_set_rot.set(true, QUIET); if(!d->rootRot())d->root_rot=editor.anim->rootTransform().axisAngle(); d->root_rot.z=DegToRad(TextFlt(t)); d->rootRot(d->root_rot); /*d.file_time.getUTC(); already changed in 'setChanged' */ d->setRoot(*editor.anim); editor.setOrnTarget(); editor.setChanged();}}
   void AnimEditor::SetSelMirror(AnimEditor &editor) {editor.setSelMirror(false);}
   void AnimEditor::SetMirrorSel(AnimEditor &editor) {editor.setSelMirror(true );}
          void AnimEditor::setSelMirror(bool set_other)
   {
      if(skel)
      {
         undos.set("boneMirror", true);
         int bone_i=(sel_bone>=0 ? sel_bone : lit_bone);
         if(SkelBone *bone=skel->bones.addr(bone_i))
         {
            Str bone_name=BoneNeutralName(bone->name);
            REPA(skel->bones)if(i!=bone_i && bone_name==BoneNeutralName(skel->bones[i].name))
            {
               if(set_other)Swap(i, bone_i);
               AnimKeys *src=findKeys(i, false), *dest=(src ? getKeys(bone_i, false) : findKeys(bone_i, false));
               if(dest)
               {
                  if(src)
                  {
                    *dest=*src;
                     dest->mirrorX();
                  }else anim->bones.remove(bone_i, true);
               }
               prepMeshSkel();
               setOrnTarget();
               setChanged();
               break;
            }
         }
      }
   }
   void AnimEditor::SkelBonePosCopy(AnimEditor &editor) {editor.skelBonePosCopy(false);}
   void AnimEditor::SkelBonePosCopyR(AnimEditor &editor) {editor.skelBonePosCopy(true );}
          void AnimEditor::skelBonePosCopy(bool relative)
   {
      prepMeshSkel();
    //copied_bone_pos=anim_skel.boneRoot(sel_bone).pos;
      copied_bone_pos=   transformedBone(sel_bone).pos;
      if(relative && sel_bone>=0)copied_bone_pos/=(Matrix)transformedBone(-1); // set as relative to root
      copied_bone_pos_relative=relative;
   }
   void AnimEditor::SkelBonePosPaste(AnimEditor &editor) {editor.skelBonePosPaste(Vec(1, 1, 1));}
   void AnimEditor::SkelBonePosPasteX(AnimEditor &editor) {editor.skelBonePosPaste(Vec(1, 0, 0));}
   void AnimEditor::SkelBonePosPasteZ(AnimEditor &editor) {editor.skelBonePosPaste(Vec(0, 0, 1));}
          void AnimEditor::skelBonePosPaste(C Vec &mask)
   {
      if(skel && anim)
      {
         undos.set("bonePosPaste");
         prepMeshSkel();
       //Vec pos=anim_skel.boneRoot(sel_bone).pos;
         Vec pos=   transformedBone(sel_bone).pos;
         if(copied_bone_pos_relative && sel_bone>=0)pos/=(Matrix)transformedBone(-1);
         Vec delta=(copied_bone_pos-pos)*mask; if(delta.any())
         {
            anim->offsetRootBones(*skel, delta);
            prepMeshSkel();
            setOrnTarget();
            setChanged();
         }
      }
   }
   bool AnimEditor::selected()C {return Mode()==MODE_ANIM;}
   void AnimEditor::selectedChanged()
   {
      setMenu();
      flush();
   }
   ElmAnim* AnimEditor::data()C {return elm ? elm->animData() : null;}
   Animation* AnimEditor::getVisAnim() {return optimize_anim.visibleFull() ? optimize_anim.getAnim() : anim;}
   flt  AnimEditor::timeToFrac(flt time)C {return (anim && anim->length()) ? time/anim->length() : 0;}
   flt  AnimEditor::animTime(        )C {return _anim_time;}
   void AnimEditor::animTime(flt time)
   {
      if(_anim_time!=time)
      {
        _anim_time=time;
         prepMeshSkel();
         setOrnTarget();
      }
   }
   void AnimEditor::setLoop(bool loop)
   {
      if(ElmAnim *d=data())if(d->loop()!=loop)
      {
         undos.set("loop"); d->loop(loop); d->loop_time.getUTC(); if(anim)anim->loop(loop); toGui(); setOrnTarget(); setChanged(false); // set 'file' to false because the Editor will recreate the animation file upon receiving new data from server
      }
   }
   void AnimEditor::setLinear(bool linear)
   {
      if(ElmAnim *d=data())if(d->linear()!=linear)
      {
         undos.set("linear"); d->linear(linear); d->linear_time.getUTC(); if(anim)anim->linear(linear); toGui(); setOrnTarget(); setChanged(false); // set 'file' to false because the Editor will recreate the animation file upon receiving new data from server
      }
   }
   void AnimEditor::selBone(int bone)
   {
      sel_bone=bone;
      setOrnTarget();
   }
   void AnimEditor::setOrnTarget()
   {
      SkelBone       bone       =transformedBone(           sel_bone ), bone_parent;
      if(sel_bone>=0)bone_parent=transformedBone(boneParent(sel_bone)); // set 'bone_parent' only if we have a bone selected, otherwise (when root is selected) then keep it as identity

      orn_target=bone.to();

      orn_perp=bone.pos-NearestPointOnStr(bone.pos, bone_parent.pos, !(orn_target-bone_parent.pos));
      if(!orn_perp.normalize())orn_perp.set(0, 0, 1);
   }
   void AnimEditor::setMenu()
   {
      ::Viewport4Region::setMenu(selected());
      cmd.menu.enabled(selected());
   }
   void AnimEditor::setMenu(Node<MenuElm> &menu, C Str &prefix)
   {
      ::Viewport4Region::setMenu(menu, prefix);
      FREPA(menu.children)if(menu.children[i].name==prefix+"View")
      {
         Node<MenuElm> &v=menu.children[i];
         v.New().create("Mode 1"     , Mode1     , T, true).kbsc(KbSc(KB_F1));
         v.New().create("Mode 2"     , Mode2     , T, true).kbsc(KbSc(KB_F2));
         v.New().create("Mode 3"     , Mode3     , T, true).kbsc(KbSc(KB_F3));
         v.New().create("Mode 4"     , Mode4     , T, true).kbsc(KbSc(KB_F4));
         v.New().create("Edit"       , Fullscreen, T).kbsc(KbSc(KB_E    , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("Play"       , Play      , T).kbsc(KbSc(KB_P    , KBSC_CTRL_CMD)).kbsc2(KbSc(KB_D, KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("Bones"      , DrawBones , T).kbsc(KbSc(KB_B    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Mesh"       , DrawMesh  , T).kbsc(KbSc(KB_M    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Grid"       , Grid      , T).kbsc(KbSc(KB_G    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Axis"       , Identity  , T).kbsc(KbSc(KB_A    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Settings"   , Settings  , T).kbsc(KbSc(KB_S    , KBSC_ALT     )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Start"      , Start     , T).kbsc(KbSc(KB_HOME , KBSC_CTRL_CMD)).kbsc2(KbSc(KB_LBR  , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("End"        , End       , T).kbsc(KbSc(KB_END  , KBSC_CTRL_CMD)).kbsc2(KbSc(KB_RBR  , KBSC_CTRL_CMD)).flag(MENU_HIDDEN);
         v.New().create("Prev Frame" , PrevFrame , T).kbsc(KbSc(KB_LEFT , KBSC_CTRL_CMD|KBSC_REPEAT)).kbsc2(KbSc(KB_COMMA, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Next Frame" , NextFrame , T).kbsc(KbSc(KB_RIGHT, KBSC_CTRL_CMD|KBSC_REPEAT)).kbsc2(KbSc(KB_DOT  , KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Prev Frames", PrevFrame , T).kbsc(KbSc(KB_LEFT , KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).kbsc2(KbSc(KB_COMMA, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Next Frames", NextFrame , T).kbsc(KbSc(KB_RIGHT, KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).kbsc2(KbSc(KB_DOT  , KBSC_CTRL_CMD|KBSC_SHIFT|KBSC_REPEAT)).flag(MENU_HIDDEN);
         v.New().create("Previous Animation", PrevAnim, T).kbsc(KbSc(KB_PGUP, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Next Animation"    , NextAnim, T).kbsc(KbSc(KB_PGDN, KBSC_CTRL_CMD|KBSC_REPEAT)).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         break;
      }
   }
   AnimEditor& AnimEditor::create()
   {
      ::Viewport4Region::create(Draw, false, 0, PI, 1, 0.01f, 1000); v4.toggleHorizontal();
      flt h=ctrls.rect().h();
      T+=axis      .create(Rect_LU(ctrls     .rect().ld(), h)     ).focusable(false).desc(S+"Display identity matrix axes, where:\nRed = right (x vector)\nGreen = up (y vector)\nBlue = forward (z vector)\nLength of each vector is 1 unit\nPlease note that the camera in Object Editor is by default faced from forward to backward.\n\nKeyboard Shortcut: Alt+A"); axis.mode=BUTTON_TOGGLE; axis.set(true); axis.image="Gui/Misc/axis.img";
      T+=draw_bones.create(Rect_LU(axis      .rect().ru(), h), "B").focusable(false).desc(S+"Display skeleton bones\nKeyboard Shortcut: Alt+B"); draw_bones.mode=BUTTON_TOGGLE; draw_bones.set(true);
      T+=draw_mesh .create(Rect_LU(draw_bones.rect().ru(), h), "M").focusable(false).desc(S+"Display mesh\nKeyboard Shortcut: Alt+M"          ); draw_mesh .mode=BUTTON_TOGGLE; draw_mesh .set(true);
         wire      .pos   (        draw_mesh .rect().ru());
      T+=show_grid .create(Rect_LU(axis      .rect().ld(), h)).focusable(false).desc("Draw grid\nKeyboard Shortcut: Alt+G"); show_grid.mode=BUTTON_TOGGLE; show_grid.set(false); show_grid.image="Gui/Misc/grid.img";
      cam_spherical.hide(); cam_lock.pos(cam_spherical.pos());

      T+=undo  .create(Rect_LU(ctrls.rect().ru()+Vec2(h, 0), 0.05f, 0.05f)     ).func(Undo  , T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU( undo.rect().ru()           , 0.05f, 0.05f)     ).func(Redo  , T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU( redo.rect().ru()           , 0.05f, 0.05f), "L").func(Locate, T).focusable(false).desc("Locate this element in the Project");

      T+=play.create(Rect_LU(locate.rect().ru()+Vec2(h, 0), h), true).desc(S+"Play Animation\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+P, "+Kb.ctrlCmdName()+"+D");
      props.New().create("Play Speed", MEMBER(AnimEditor, anim_speed)).precision(2);
      Rect r=AddProperties(props, T, play.rect().ru()+Vec2(0.01f, 0), play.rect().h(), 0.15f, &ObjEdit.ts); REPAO(props).autoData(this);

      T+=op.create(Rect_LU(r.ru()+Vec2(h, 0), h*OP_NUM, h), 0, (cchar**)null, OP_NUM);
      op.tab(OP_ORN  ).setImage("Gui/Misc/target.img" ).desc(S+"Set Target Orientation KeyFrames\n\nSelect with LeftClick\nTransform with RightClick\nRotate with Alt+RightClick\nHold Shift for more precision\nHold "+Kb.ctrlCmdName()+" to transform all KeyFrames\n\nKeyboard Shortcut: F1");
      op.tab(OP_ORN2 ).setImage("Gui/Misc/target2.img").desc(S+"Set Target Orientation KeyFrames for Bone and its Parent\n\nSelect with LeftClick\nTransform with RightClick\nRotate with Alt+RightClick\nHold Shift for more precision\n\nKeyboard Shortcut: F2");
      op.tab(OP_POS  ).setImage("Gui/Misc/move.img"   ).desc(S+"Set Position Offset KeyFrames\n\nSelect with LeftClick\nTransform with RightClick\nHold Shift for more precision\nHold "+Kb.ctrlCmdName()+" to transform all KeyFrames\nHold Alt to use World Matrix alignment\n\nKeyboard Shortcut: F3");
      op.tab(OP_SCALE).setImage("Gui/Misc/scale.img"  ).desc(S+"Set Scale KeyFrames\n\nSelect with LeftClick\nTransform with RightClick\nHold Shift for more precision\nHold "+Kb.ctrlCmdName()+" to transform all KeyFrames\n\nKeyboard Shortcut: F4");
      Node<MenuElm> n;
      n.New().create("Delete KeyFrame"                 , DelFrame      , T).kbsc(KbSc(KB_DEL, KBSC_CTRL_CMD)).desc("This will delete a single keyframe for selected bone");
      n.New().create("Delete KeyFrames"                , DelFrames     , T).kbsc(KbSc(KB_DEL, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("This will delete all keyframes for selected bone");
      n.New().create("Delete All Bone KeyFrames at End", DelFramesAtEnd, T).kbsc(KbSc(KB_DEL, KBSC_CTRL_CMD|KBSC_WIN_CTRL)).desc("This will delete keyframes located at the end of the animation, for all bones (except root motion).");
      n++;
      n.New().create("Reduce KeyFrames", Optimize, T).kbsc(KbSc(KB_O, KBSC_CTRL_CMD));
      n++;
      n.New().create("Reverse KeyFrames", ReverseFrames, T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_SHIFT)); // avoid Ctrl+R collision with reload project element
      n++;
      n.New().create("Change Speed for Time Range", TimeRangeSp, T).kbsc(KbSc(KB_S, KBSC_CTRL_CMD));
      n++;
    //n.New().create("Remove Movement", RemMovement, T).desc("This option can be used for animations that include actual movement - ending position is not the same as the starting position.\nThis option will adjust the animation so that the ending position is the same as starting position.").kbsc(KbSc(KB_M, KBSC_CTRL_CMD|KBSC_ALT));
      n.New().create("Freeze Bone"    , FreezeBone , T).desc("This option will adjust position offset to the root bone, so that currently selected bone will appear without movement.").kbsc(KbSc(KB_F, KBSC_CTRL_CMD|KBSC_ALT));
      n++;
      n.New().create("Set Mirrored from Selection", SetMirrorSel, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD           )).desc("This option will set bone animation from the other side as mirrored version of the selected bone");
      n.New().create("Set Selection from Mirrored", SetSelMirror, T).kbsc(KbSc(KB_M, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("This option will set selected bone animation as mirrored version of the bone from the other side");
      n++;
      n.New().create( "Copy Bone Position"         , SkelBonePosCopy  , T).kbsc(KbSc(KB_C, KBSC_CTRL_CMD));
      n.New().create( "Copy Bone Position Relative", SkelBonePosCopyR , T).kbsc(KbSc(KB_C, KBSC_CTRL_CMD|KBSC_SHIFT)).desc("Copy selected bone position relative to root");
      n.New().create("Paste Bone Position"         , SkelBonePosPaste , T).kbsc(KbSc(KB_V, KBSC_CTRL_CMD));
      n.New().create("Paste Bone Position Z"       , SkelBonePosPasteZ, T).kbsc(KbSc(KB_V, KBSC_CTRL_CMD|KBSC_SHIFT));
      n.New().create("Paste Bone Position X"       , SkelBonePosPasteX, T).kbsc(KbSc(KB_V, KBSC_CTRL_CMD|KBSC_WIN_CTRL));
      n++;
      n.New().create("Set Root From Body Z"      , RootFromBodyZ, T).kbsc(KbSc(KB_B, KBSC_CTRL_CMD));
      n.New().create("Set Root From Body X"      , RootFromBodyX, T).kbsc(KbSc(KB_B, KBSC_CTRL_CMD|KBSC_SHIFT));
      n.New().create("Del Root Position+Rotation", RootDel      , T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD|KBSC_ALT));
      n++;
      n.New().create("Mirror"  , Mirror, T).desc("Mirror entire animation along X axis");
      n.New().create("Rotate X", RotX  , T).kbsc(KbSc(KB_X, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT)).desc("Rotate entire animation along X axis");
      n.New().create("Rotate Y", RotY  , T).kbsc(KbSc(KB_Y, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT)).desc("Rotate entire animation along Y axis");
      n.New().create("Rotate Z", RotZ  , T).kbsc(KbSc(KB_Z, KBSC_CTRL_CMD|KBSC_ALT|KBSC_REPEAT)).desc("Rotate entire animation along Z axis");
      n++;
      n.New().create("Transform Original Object", TransformObj, T);
      T+=cmd.create(Rect_LU(op.rect().ru(), h), n).focusable(false); cmd.flag|=COMBOBOX_CONST_TEXT;

      T+=edit.create(Rect_LU(cmd.rect().ru()+Vec2(h, 0), 0.12f, h), "Edit").func(Fullscreen, T).focusable(false).desc(S+"Close Editing\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+E");
      cchar8 *settings_t[]={"Settings"};
      T+=settings.create(Rect_LU(edit.rect().ru()+Vec2(h, 0), 0.17f, h), 0, settings_t, Elms(settings_t));
      settings.tab(0).desc("Keyboard Shortcut: Alt+S")+=settings_region.create(Rect_LU(settings.rect().ru(), 0.333f+h*3, 0.5f)).skin(&TransparentSkin, false); settings_region.kb_lit=false;
      {
         flt x=0.01f, y=-0.005f, l=h*1.1f, ll=l*1.1f, w=0.25f, ph=h*0.95f;
         ts.reset().size=0.041f;
         settings_region+=t_settings.create(Vec2(settings_region.clientWidth()/2, y-h/2), "Settings also affect reloading", &ts); y-=ll;
         settings_region+=loop  .create(Rect_U(settings_region.clientWidth()*(1.0f/3.5f), y, 0.15f, h), "Loop"  ).func(Loop  , T); loop  .mode=BUTTON_TOGGLE;
         settings_region+=linear.create(Rect_U(settings_region.clientWidth()*(2.5f/3.5f), y, 0.15f, h), "Linear").func(Linear, T); linear.mode=BUTTON_TOGGLE;
         y-=ll;
         settings_region+=t_settings_root.create(Vec2(x, y-h/2), "Root:", &ObjEdit.ts); y-=l;
         settings_region+=root_del_pos  .create(Rect_LU(x, y, w, h), "Del Position").func(RootDelPos, T);
         settings_region+=root_del_pos_x.create(Rect_LU(root_del_pos  .rect().ru()+Vec2(0.01f, 0), h, h), "X").func(RootDelPosX, T).subType(BUTTON_TYPE_TAB_LEFT      ); root_del_pos_x.mode=BUTTON_TOGGLE;
         settings_region+=root_del_pos_y.create(Rect_LU(root_del_pos_x.rect().ru()              , h, h), "Y").func(RootDelPosY, T).subType(BUTTON_TYPE_TAB_HORIZONTAL); root_del_pos_y.mode=BUTTON_TOGGLE;
         settings_region+=root_del_pos_z.create(Rect_LU(root_del_pos_y.rect().ru()              , h, h), "Z").func(RootDelPosZ, T).subType(BUTTON_TYPE_TAB_RIGHT     ); root_del_pos_z.mode=BUTTON_TOGGLE;
         y-=l;
         settings_region+=root_del_rot  .create(Rect_LU(x, y, w, h), "Del Rotation").func(RootDelRot, T);
         settings_region+=root_del_rot_x.create(Rect_LU(root_del_rot  .rect().ru()+Vec2(0.01f, 0), h, h), "X").func(RootDelRotX, T).subType(BUTTON_TYPE_TAB_LEFT      ); root_del_rot_x.mode=BUTTON_TOGGLE;
         settings_region+=root_del_rot_y.create(Rect_LU(root_del_rot_x.rect().ru()              , h, h), "Y").func(RootDelRotY, T).subType(BUTTON_TYPE_TAB_HORIZONTAL); root_del_rot_y.mode=BUTTON_TOGGLE;
         settings_region+=root_del_rot_z.create(Rect_LU(root_del_rot_y.rect().ru()              , h, h), "Z").func(RootDelRotZ, T).subType(BUTTON_TYPE_TAB_RIGHT     ); root_del_rot_z.mode=BUTTON_TOGGLE;
         y-=l;
       //settings_region+=root_del_scale.create(Rect_LU(x, y, w, h), "Del Scale"   ).func(RootDelScale, T); root_del_scale.mode=BUTTON_TOGGLE; y-=l;
         settings_region+=root_from_body.create(Rect_LU(x, y, w, h), "From Body"   ).func(RootFromBody, T); root_from_body.mode=BUTTON_TOGGLE; y-=l;
         settings_region+=root_2_keys   .create(Rect_LU(x, y, w, h), "2 Keys Only" ).func(Root2Keys   , T); root_2_keys   .mode=BUTTON_TOGGLE; y-=l; root_2_keys.desc("Limit number of keyframes to 2 keys only: Start+End");
         settings_region+=root_set_move .create(Rect_LU(x, y, w, h), "Set Movement").func(RootSetMove , T); root_set_move .mode=BUTTON_TOGGLE; root_set_move.desc("Override Animation's root movement with a custom value");
         settings_region+=root_set_rot  .create(Rect_LU(x, y-3*ph, w, h), "Set Rotation").func(RootSetRot  , T); root_set_rot  .mode=BUTTON_TOGGLE; root_set_rot.desc("Override Animation's root rotation with a custom value");
       //settings_region+=reload        .create(Rect_U (settings_region.clientWidth()/2, y-6*ph-0.01, 0.18, h+0.005), "Reload").func(Reload, T);
         settings_region+=reload        .create(Rect_LD(x, y-6*ph, 0.18f, h+0.005f), "Reload").func(Reload, T);
         root_props.New().create("X", MemberDesc(DATA_REAL).setFunc(RootMoveX, RootMoveX));
         root_props.New().create("Y", MemberDesc(DATA_REAL).setFunc(RootMoveY, RootMoveY));
         root_props.New().create("Z", MemberDesc(DATA_REAL).setFunc(RootMoveZ, RootMoveZ));
         root_props.New().create("X", MemberDesc(DATA_REAL).setFunc(RootRotX , RootRotX )).mouseEditSpeed(15).precision(1);
         root_props.New().create("Y", MemberDesc(DATA_REAL).setFunc(RootRotY , RootRotY )).mouseEditSpeed(15).precision(1);
         root_props.New().create("Z", MemberDesc(DATA_REAL).setFunc(RootRotZ , RootRotZ )).mouseEditSpeed(15).precision(1);
         Rect r=AddProperties(root_props, settings_region, Vec2(x+w+0.01f, y), ph, 0.17f, &ObjEdit.ts); REPAO(root_props).autoData(this); y=Min(r.min.y, reload.rect().min.y);
         settings_region.size(Vec2(settings_region.rect().w(), -y+0.02f));
      }

      T+=start     .create("[").focusable(false).func(Start    , T).desc(S+"Go to animation start\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Home, "+Kb.ctrlCmdName()+"+[");
      T+=prev_frame.create("<").focusable(false).func(PrevFrame, T).desc(S+"Go to previous KeyFrame\nHold Shift to iterate all KeyFrame types\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+LeftArrow, "+Kb.ctrlCmdName()+"+<");
      T+=force_play.create(   ).focusable(false)                   .desc(S+"Play as long as this button is pressed\nHold Shift to play backwards\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+W"); force_play.image=Proj.icon_play; force_play.mode=BUTTON_CONTINUOUS;
      T+=next_frame.create(">").focusable(false).func(NextFrame, T).desc(S+"Go to next KeyFrame\nHold Shift to iterate all KeyFrame types\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+RightArrow, "+Kb.ctrlCmdName()+"+>");
      T+=end       .create("]").focusable(false).func(End      , T).desc(S+"Go to animation end\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+End, "+Kb.ctrlCmdName()+"+]");
      T+=track           .create(false);
      T+=optimize_anim   .create();
      T+=time_range_speed.create();
      preview.create();
      return T;
   }
   SkelBone AnimEditor::transformedBone(int i)C
   {
      SkelBone bone;
      if(skel && InRange(i, skel->bones))bone=skel->bones[i];//else bone is already set to identity in constructor
      bone*=anim_skel.boneRoot(i).matrix();
      return bone;
   }
   Matrix AnimEditor::transformedBoneAxis(int i)C
   {
      Matrix m=transformedBone(i);
      if(op()==OP_POS && Kb.alt())m.orn().identity();
      m.scaleOrn((i>=0) ? SkelSlotSize : 1.0f/3);
      return m;
   }
   int AnimEditor::getBone(GuiObj *go, C Vec2 &screen_pos)
   {
      int index=-1; if(Edit::Viewport4::View *view=v4.getView(go))
      {
         view->setViewportCamera();
         flt dist=0;
         REPA(anim_skel.bones)
         {
            SkelBone bone=transformedBone(i);
            flt d; if(Distance2D(screen_pos, Edge(bone.pos, bone.to()), d, 0))if(index<0 || d<dist){index=i; dist=d;}
         }
         if(dist>0.03f)index=-1;
      }
      return index;
   }
   int AnimEditor::boneParent(int bone)C
   {
      return skel ? skel->boneParent(bone) : -1;
   }
   AnimKeys* AnimEditor::findVisKeys(int sbon_index, bool root)
   {
      if(Animation *anim=getVisAnim())
      {
         if(skel)if(C SkelBone *sbon=skel->bones.addr(sbon_index))return anim->findBone(sbon->name, sbon->type, sbon->type_index, sbon->type_sub); // use types in case animation was from another skeleton and we haven't adjusted types
         if(root)return &anim->keys;
      }
      return null;
   }
   AnimKeys* AnimEditor::findKeys(int sbon_index, bool root)
   {
      if(anim)
      {
         if(skel)if(C SkelBone *sbon=skel->bones.addr(sbon_index))return anim->findBone(sbon->name, sbon->type, sbon->type_index, sbon->type_sub); // use types in case animation was from another skeleton and we haven't adjusted types
         if(root)return &anim->keys;
      }
      return null;
   }
   AnimKeys* AnimEditor::getKeys(int sbon_index, bool root)
   {
      if(anim)
      {
         if(skel)if(C SkelBone *sbon=skel->bones.addr(sbon_index))return &anim->getBone(sbon->name, sbon->type, sbon->type_index, sbon->type_sub); // use types in case animation was from another skeleton and we haven't adjusted types
         if(root)return &anim->keys;
      }
      return null;
   }
   AnimKeys* AnimEditor::findKeysParent(int sbon_index, bool root) {return sbon_index<0 ? null : findKeys(boneParent(sbon_index), root);}
   int AnimEditor::CompareKey(C AnimKeys::Orn   &key, C flt &time) {return Compare(key.time, time);}
   int AnimEditor::CompareKey(C AnimKeys::Pos   &key, C flt &time) {return Compare(key.time, time);}
   int AnimEditor::CompareKey(C AnimKeys::Scale &key, C flt &time) {return Compare(key.time, time);}
   AnimKeys::Orn  * AnimEditor::FindOrn(AnimKeys &keys, flt time) {AnimKeys::Orn   *key=null; flt dist; REPA(keys.orns  ){flt d=Abs(keys.orns  [i].time-time); if(!key || d<dist){key=&keys.orns  [i]; dist=d;}} return (!key || dist>TimeEps()) ? null : key;}
   AnimKeys::Pos  * AnimEditor::FindPos(AnimKeys &keys, flt time) {AnimKeys::Pos   *key=null; flt dist; REPA(keys.poss  ){flt d=Abs(keys.poss  [i].time-time); if(!key || d<dist){key=&keys.poss  [i]; dist=d;}} return (!key || dist>TimeEps()) ? null : key;}
   AnimKeys::Scale* AnimEditor::FindScale(AnimKeys &keys, flt time) {AnimKeys::Scale *key=null; flt dist; REPA(keys.scales){flt d=Abs(keys.scales[i].time-time); if(!key || d<dist){key=&keys.scales[i]; dist=d;}} return (!key || dist>TimeEps()) ? null : key;}
   AnimKeys::Orn  & AnimEditor::GetOrn(AnimKeys &keys, flt time, C Orient &Default) {if(AnimKeys::Orn   *key=FindOrn  (keys, time))return *key; int i; if(keys.orns  .binarySearch(time, i, CompareKey))return keys.orns  [i]; AnimKeys::Orn   &key=keys.orns  .NewAt(i); key.time=time; key.orn  =Default; return key;}
   AnimKeys::Pos  & AnimEditor::GetPos(AnimKeys &keys, flt time, C Vec    &Default) {if(AnimKeys::Pos   *key=FindPos  (keys, time))return *key; int i; if(keys.poss  .binarySearch(time, i, CompareKey))return keys.poss  [i]; AnimKeys::Pos   &key=keys.poss  .NewAt(i); key.time=time; key.pos  =Default; return key;}
   AnimKeys::Scale& AnimEditor::GetScale(AnimKeys &keys, flt time, C Vec    &Default) {if(AnimKeys::Scale *key=FindScale(keys, time))return *key; int i; if(keys.scales.binarySearch(time, i, CompareKey))return keys.scales[i]; AnimKeys::Scale &key=keys.scales.NewAt(i); key.time=time; key.scale=Default; return key;}
   flt AnimEditor::getBlend(C AnimKeys::Key &key )C {return getBlend(key.time);}
   flt AnimEditor::getBlend(           flt  time)C
   {
      flt delta=time-animTime();
      if(anim && anim->loop())
      {
         delta=Frac(delta, anim->length());
         if(delta>anim->length()/2)delta-=anim->length();
      }
      return BlendSmoothCube(delta/blend_range);
   }
   void AnimEditor::update(C GuiPC &gpc)
{
      lit_bone=-1;
      if(visible() && gpc.visible)
      {
         optimize_anim.refresh(); // refresh all the time because animation can be changed all the time (since we're accessing it directly from 'Animations' cache)
         playUpdate();
         prepMeshSkel();

         if(draw_bones())
         {
            // get lit
            lit_bone=getBone(Gui.ms(), Ms.pos());

            // get 'bone_axis'
            if((op()==OP_POS || op()==OP_SCALE)
         // && skel && InRange(sel_bone, skel.bones) don't check this so we can process just root too
            )MatrixAxis(v4, transformedBoneAxis(sel_bone), bone_axis);else bone_axis=-1;

            // select / edit
            if(Edit::Viewport4::View *view=v4.getView(Gui.ms()))
            {
               if(Ms.bp(0))selBone(lit_bone);else
               if(Ms.b (1) && op()>=0
            //&& skel && InRange(sel_bone, skel.bones) don't check this so we can process just root too
               )if(AnimKeys *keys=getKeys(sel_bone))
               {
                  view->setViewportCamera();
                    C SkelBone * sbon=(skel ? skel->bones.addr(sel_bone) : null);
                      SkelBone   bone=  transformedBone      (sel_bone), bone_parent; if(sel_bone>=0)bone_parent=transformedBone(boneParent(sel_bone)); // set 'bone_parent' only if we have a bone selected, otherwise (when root is selected) then keep it as identity
                C AnimSkelBone &asbon=   anim_skel.boneRoot  (sel_bone);
                  Vec2           mul =(Kb.shift() ? 0.1f : 1.0f)*0.5f;
                  bool           all =Kb.ctrlCmd(), rotate=Kb.alt(), use_blend=(blend_range>0);
                  switch(op())
                  {
                     case OP_ORN:
                     {
                        undos.set("orn");
                     op_orn:

                        if(!rotate)
                        {
                           mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                           Vec d=ActiveCam.matrix.x*Ms.d().x*mul.x
                                +ActiveCam.matrix.y*Ms.d().y*mul.y;
                           orn_target+=d;
                           bone.setFromTo(bone.pos, orn_target);
                        }
                        Orient pose=GetAnimOrient(bone, &bone_parent); pose.fix();
                        AnimKeys::Orn *orn=((all && keys->orns.elms()) ? null : &GetOrn(*keys, animTime(), pose)); // if there are no keys then create
                        if(rotate)
                        {
                           flt d=(Ms.d()*mul).sum();
                           if(all)
                           {
                              if(use_blend)REPA (keys->orns)keys->orns[i].orn.rotateDir(getBlend(keys->orns[i])*d);
                              else         REPAO(keys->orns).orn.rotateDir(d);
                           }else // single
                           {
                              orn->orn.rotateDir(d);
                           }
                        }else
                        {
                           if(all)
                           {
                              // always calculate current orientation, because 'asbon.orn' can be zero
                              Vec     p=orn_target-bone.pos; p/=Matrix3(bone_parent); p.normalize();
                              Orient  orn=asbon.orn; if(!orn.fix()){if(sbon /*&& skel - no need to check because 'sbon' already guarantees 'skel'*/)orn=GetAnimOrient(*sbon, skel->bones.addr(sbon->parent));else orn.identity();}
                              Matrix3 transform;
                              if(use_blend)REPA(keys->orns)
                              {
                                 Orient next=orn; next.rotateToDir(p, getBlend(keys->orns[i])); next.fixPerp();
                                 GetTransform(transform, orn, next);
                                 keys->orns[i].orn.mul(transform, true).fix();
                              }else
                              {
                                 Orient next=orn; next.rotateToDir(p); next.fixPerp();
                                 GetTransform(transform, orn, next);
                                 REPAO(keys->orns).orn.mul(transform, true).fix();
                              }
                           }else // single
                           {
                              orn->orn=pose;
                           }
                        }
                        keys->setTangents(anim->loop(), anim->length());
                        anim->setRootMatrix();
                        setChanged();
                     }break;

                     case OP_ORN2:
                     {
                        undos.set("orn");
                        if(!sbon /*|| !skel - no need to check because 'sbon' already guarantees 'skel'*/)goto op_orn;
                      C SkelBone *sbon_parent=skel->bones.addr(sbon->parent);
                        AnimKeys *keys_parent=        getKeys(sbon->parent, false);
                        if(!sbon_parent || !keys_parent)goto op_orn;

                        if(!rotate)
                        {
                           Vec perp=bone.pos-NearestPointOnStr(bone.pos, bone_parent.pos, !(orn_target-bone_parent.pos));
                           if( perp.normalize()<=EPS)switch(sbon->type)
                           {
                              case BONE_FINGER:
                              {
                                 int hand=skel->findParent(sel_bone, BONE_HAND);
                                 if( hand>=0)perp=transformedBone(hand).perp;else perp.set(0, 1, 0);
                              }break;

                              case BONE_HEAD     :
                              case BONE_FOOT     :
                              case BONE_SPINE    :
                              case BONE_NECK     : perp.set(0, 0, bone.pos.z-orn_target.z); break;

                              case BONE_TOE      : perp.set(0, bone.pos.y-orn_target.y, 0); break;

                              case BONE_UPPER_ARM:
                              case BONE_FOREARM  : perp.set(0, 0, -1); break;

                              case BONE_LOWER_LEG: perp.set(0, 0,  1); break;

                              default            : perp=orn_perp; break;
                           }

                           mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                           Vec d=ActiveCam.matrix.x*Ms.d().x*mul.x
                                +ActiveCam.matrix.y*Ms.d().y*mul.y;
                           orn_target+=d;

                           Vec desired_dir=orn_target-bone_parent.pos;

                           flt a_length=Dist(bone_parent.pos, bone.pos), b_length=bone.length,
                             cur_length=Dist(bone_parent.pos, bone.to()),
                         desired_length=Min (desired_dir.length(), a_length+b_length);

                           // first rotate the parent to match the new direction (from parent->child.to to parent->orn_target)
                           Matrix3 m;
                           m.setRotation(!(bone.to()-bone_parent.pos), !desired_dir);
                           SCAST(Orient, bone_parent)*=m;
                           SCAST(Orient, bone       )*=m; // this should include position, but we don't actually need it
                                         perp        *=m;

                           // rotate the parent and bone
                           Vec rotate_axis=CrossN(desired_dir, perp);

                           flt cur_angle=TriABAngle(    cur_length, a_length, b_length),
                           desired_angle=TriABAngle(desired_length, a_length, b_length);
                           flt     delta=desired_angle-cur_angle;
                           SCAST(Orient, bone_parent)*=Matrix3().setRotate(rotate_axis, delta);
                         //SCAST(Orient, bone       )*=Matrix3().setRotate(rotate_axis, delta); child should be transformed too, but instead we're transforming child by 'delta' below

                               cur_angle=TriABAngle(a_length, b_length,     cur_length);
                           desired_angle=TriABAngle(a_length, b_length, desired_length);
                           SCAST(Orient, bone       )*=Matrix3().setRotate(rotate_axis, delta + desired_angle-cur_angle); // add delta because child is transformed by parent

                           Orient pose=GetAnimOrient(bone_parent, &transformedBone(sbon_parent->parent)); pose.fix();
                           AnimKeys::Orn *orn=&GetOrn(*keys_parent, animTime(), pose);
                           orn->orn=pose;

                           pose=GetAnimOrient(bone, &bone_parent); pose.fix();
                           orn=&GetOrn(*keys, animTime(), pose);
                           orn->orn=pose;
                        }else
                        {
                           flt d=(Ms.d()*mul).sum();

                           Matrix3 m;
                           m.setRotate(!(bone.to()-bone_parent.pos), d);
                           SCAST(Orient, bone_parent)*=m;
                           SCAST(Orient, bone       )*=m; // this should include position, but we don't actually need it
                           
                           Orient pose=GetAnimOrient(bone_parent, &transformedBone(sbon_parent->parent)); pose.fix();
                           AnimKeys::Orn *orn=&GetOrn(*keys_parent, animTime(), pose);
                           orn->orn=pose;

                           pose=GetAnimOrient(bone, &bone_parent); pose.fix();
                           orn=&GetOrn(*keys, animTime(), pose);
                           orn->orn=pose;
                        }
                        keys_parent->setTangents(anim->loop(), anim->length());
                        keys       ->setTangents(anim->loop(), anim->length());
                        anim->setRootMatrix();
                        setChanged();
                     }break;

                     case OP_POS:
                     {
                        undos.set("pos");
                        AnimKeys::Pos *pos=((all && keys->poss.elms()) ? null : &GetPos(*keys, animTime(), asbon.pos)); // if there are no keys then create
                        mul*=CamMoveScale(v4.perspective())*MoveScale(*view);
                        Matrix m=transformedBoneAxis(sel_bone);
                        Vec    d; switch(bone_axis)
                        {
                           case  0: d=AlignDirToCam(m.x, Ms.d()  *mul ); break;
                           case  1: d=AlignDirToCam(m.y, Ms.d()  *mul ); break;
                           case  2: d=AlignDirToCam(m.z, Ms.d()  *mul ); break;
                           default: d=ActiveCam.matrix.x*Ms.d().x*mul.x
                                     +ActiveCam.matrix.y*Ms.d().y*mul.y; break;
                        }
                        orn_target+=d;
                        d/=Matrix3(bone_parent);
                        if(all)
                        {
                           if(use_blend)REPA (keys->poss)keys->poss[i].pos+=d*getBlend(keys->poss[i]);
                           else         REPAO(keys->poss).pos+=d;
                        }else // single
                        {
                           pos->pos+=d;
                        }
                        keys->setTangents(anim->loop(), anim->length());
                        anim->setRootMatrix();
                        setChanged();
                     }break;

                     case OP_SCALE:
                     {
                        undos.set("scale");
                        AnimKeys::Scale *scale=((all && keys->scales.elms()) ? null : &GetScale(*keys, animTime(), asbon.scale)); // if there are no keys then create
                        Vec d=0;
                        switch(bone_axis)
                        {
                           case  0: d.x+=AlignDirToCamEx(bone.cross(), Ms.d()*mul)      ; break;
                           case  1: d.y+=AlignDirToCamEx(bone.perp   , Ms.d()*mul)      ; break;
                           case  2: d.z+=AlignDirToCamEx(bone.dir    , Ms.d()*mul)      ; break;
                           default: d  +=                             (Ms.d()*mul).sum(); break;
                        }
                        if(Kb.ctrlCmd()) // all
                        {
                           if(use_blend)REPA (keys->scales)keys->scales[i].scale+=d*getBlend(keys->scales[i]);
                           else         REPAO(keys->scales).scale+=d;
                        }else // single
                        {
                           scale->scale+=d;
                        }
                        keys->setTangents(anim->loop(), anim->length());
                        anim->setRootMatrix();
                        setChanged();
                        setOrnTarget();
                     }break;
                  }
               }
            }
         }
         if(Ms.bp(2)) // close on middle click
         {
            if(settings_region.contains(Gui.ms()))settings.set(-1);
         }
      }
      ::Viewport4Region::update(gpc); // process after adjusting 'animTime' so clicking on anim track will not be changed afterwards
   }
   bool AnimEditor::selectionZoom(flt &dist)
   {
      flt size=(mesh ? mesh->ext.size().avg() : 0);
      if( size>0)
      {
         dist=size/Tan(v4.perspFov()/2);
         return true;
      }
      return false;
   }
   bool AnimEditor::getHit(GuiObj *go, C Vec2 &screen_pos, Vec &hit_pos)
   {
      int hit_part=-1;
      if(mesh)if(Edit::Viewport4::View *view=v4.getView(go))
      {
       C MeshLod  &lod=mesh->getDrawLod(anim_skel.matrix());
         MeshPart  part;
         view->setViewportCamera();
         Vec pos, dir; ScreenToPosDir(screen_pos, pos, dir);
         pos+=(D.viewFrom ()/Dot(dir, ActiveCam.matrix.z))*dir;
         dir*= D.viewRange();
         flt frac, f; Vec hp;
         REPA(lod)
            if(!(lod.parts[i].part_flag&MSHP_HIDDEN))
         {
            part.create(lod.parts[i]).setBase().base.animate(anim_skel);
            if(Sweep(pos, dir, part, null, &f, &hp))if(hit_part<0 || f<frac){hit_part=i; frac=f; hit_pos=hp;}
         }
      }
      return hit_part>=0;
   }
   void AnimEditor::camCenter(bool zoom)
{
      Vec hit_pos; bool hit=getHit(Gui.ms(), Ms.pos(), hit_pos); flt dist;
      v4.moveTo(hit ? hit_pos :
                (skel && InRange(lit_bone, skel->bones)) ? transformedBone(lit_bone).center() :
                (skel && InRange(sel_bone, skel->bones)) ? transformedBone(sel_bone).center() :
                mesh ? mesh->ext.center :
                VecZero);
      if(zoom && selectionZoom(dist))v4.dist(dist);
   }
   void AnimEditor::resize()
{
      ::Viewport4Region::resize();
      track.rect(Rect(Proj.visible() ? 0 : Misc.rect().w(), -clientHeight(), v4.rect().max.x, 0.09f-clientHeight()));
      end       .rect(Rect_RD(track     .rect().ru(), 0.07f, 0.07f));
      next_frame.rect(Rect_RD(end       .rect().ld(), 0.08f, 0.07f));
      force_play.rect(Rect_RD(next_frame.rect().ld(), 0.08f, 0.07f));
      prev_frame.rect(Rect_RD(force_play.rect().ld(), 0.08f, 0.07f));
      start     .rect(Rect_RD(prev_frame.rect().ld(), 0.07f, 0.07f));
      v4.rect(Rect(v4.rect().min.x, Proj.visible() ? track.rect().max.y : Misc.rect().h()-clientHeight(), v4.rect().max.x, v4.rect().max.y));
      optimize_anim.move(Vec2(rect().w(), rect().h()/-2)-optimize_anim.rect().right());
   }
   void AnimEditor::frame(int d)
   {
      if(d)if(AnimKeys *keys=findKeys(sel_bone))switch(Kb.shift() ? -1 : op())
      {
         case OP_ORN :
         case OP_ORN2: if(keys->orns.elms())
         {
            int i=0; for(; i<keys->orns.elms(); i++){flt t=keys->orns[i].time; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
            i+=d; i=Mod(i, keys->orns.elms()); animTime(keys->orns[i].time);
         }break;

         case OP_POS: if(keys->poss.elms())
         {
            int i=0; for(; i<keys->poss.elms(); i++){flt t=keys->poss[i].time; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
            i+=d; i=Mod(i, keys->poss.elms()); animTime(keys->poss[i].time);
         }break;

         case OP_SCALE: if(keys->scales.elms())
         {
            int i=0; for(; i<keys->scales.elms(); i++){flt t=keys->scales[i].time; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
            i+=d; i=Mod(i, keys->scales.elms()); animTime(keys->scales[i].time);
         }break;
         
         default:
         {
            Memt<flt, 16384> times; keys->includeTimes(times, times, times);
            if(times.elms())
            {
               int i=0; for(; i<times.elms(); i++){flt t=times[i]; if(Equal(animTime(), t))break; if(t>=animTime()){if(d>0)d--; break;}}
               i+=d; i=Mod(i, times.elms()); animTime(times[i]);
            }
         }break;
      }
   }
   bool AnimEditor::delFrameOrn(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(AnimKeys::Orn *key=FindOrn(*keys, animTime()))
      {
         undos.set("del"); keys->orns.removeData(key, true);
         if(keys->is())keys->setTangents(anim->loop(), anim->length());else anim->bones.removeData(static_cast<AnimBone*>(keys), true);
         return true;
      }
      return false;
   }
   bool AnimEditor::delFramePos(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(AnimKeys::Pos *key=FindPos(*keys, animTime()))
      {
         undos.set("del"); keys->poss.removeData(key, true);
         if(keys->is())keys->setTangents(anim->loop(), anim->length());else anim->bones.removeData(static_cast<AnimBone*>(keys), true);
         return true;
      }
      return false;
   }
   bool AnimEditor::delFrameScale(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(AnimKeys::Scale *key=FindScale(*keys, animTime()))
      {
         undos.set("del"); keys->scales.removeData(key, true);
         if(keys->is())keys->setTangents(anim->loop(), anim->length());else anim->bones.removeData(static_cast<AnimBone*>(keys), true);
         return true;
      }
      return false;
   }
   bool AnimEditor::delFramesOrn(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(keys->orns.elms()){undos.set("delAll"); keys->orns.del(); if(!keys->is())anim->bones.removeData(static_cast<AnimBone*>(keys), true); return true;}
      return false;
   }
   bool AnimEditor::delFramesPos(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(keys->poss.elms()){undos.set("delAll"); keys->poss.del(); if(!keys->is())anim->bones.removeData(static_cast<AnimBone*>(keys), true); return true;}
      return false;
   }
   bool AnimEditor::delFramesScale(int bone)
   {
      if(AnimKeys *keys=findKeys(bone))if(keys->scales.elms()){undos.set("delAll"); keys->scales.del(); if(!keys->is())anim->bones.removeData(static_cast<AnimBone*>(keys), true); return true;}
      return false;
   }
   void AnimEditor::delFrame()
   {
      bool changed=false;
      if(op()==OP_ORN   || op()<0)changed|=delFrameOrn  (sel_bone);
      if(op()==OP_ORN2           )changed|=delFrameOrn  (sel_bone)|delFrameOrn(boneParent(sel_bone));
      if(op()==OP_POS   || op()<0)changed|=delFramePos  (sel_bone);
      if(op()==OP_SCALE || op()<0)changed|=delFrameScale(sel_bone);
      if(changed){prepMeshSkel(); setOrnTarget(); anim->setRootMatrix(); setChanged();}
   }
   void AnimEditor::delFrames()
   {
      bool changed=false;
      if(op()==OP_ORN   || op()<0)changed|=delFramesOrn  (sel_bone);
      if(op()==OP_ORN2           )changed|=delFramesOrn  (sel_bone)|delFramesOrn(boneParent(sel_bone));
      if(op()==OP_POS   || op()<0)changed|=delFramesPos  (sel_bone);
      if(op()==OP_SCALE || op()<0)changed|=delFramesScale(sel_bone);
      if(changed){prepMeshSkel(); setOrnTarget(); anim->setRootMatrix(); setChanged();}
   }
   void AnimEditor::delFramesAtEnd()
   {
      if(anim)
      {
         bool changed=false;
         undos.set("delAtEnd");
         flt time=anim->length()-EPS;
         REP(anim->bones.elms())
         {
            bool bone_changed=false;
            AnimBone &bone=anim->bones[i];
            if(bone.poss  .elms()>=2 && bone.poss  .last().time>=time && bone.poss  .first().time<=EPS){bone.poss  .removeLast(); bone_changed=true;}
            if(bone.orns  .elms()>=2 && bone.orns  .last().time>=time && bone.orns  .first().time<=EPS){bone.orns  .removeLast(); bone_changed=true;}
            if(bone.scales.elms()>=2 && bone.scales.last().time>=time && bone.scales.first().time<=EPS){bone.scales.removeLast(); bone_changed=true;}
            if(bone_changed){bone.setTangents(anim->loop(), anim->length()); changed=true;}
         }
         if(changed){prepMeshSkel(); setOrnTarget(); /*anim.setRootMatrix(); we don't change root, only bones*/ setChanged();}
      }
   }
   void AnimEditor::reverseFrames()
   {
      if(anim)
      {
         undos.set("reverse");
         if(sel_bone<0                       )anim->reverse();else // reverse entire animation when no bone is selected (instead of just root)
         if(AnimKeys *keys=findKeys(sel_bone)){keys->reverse(anim->length()); anim->setRootMatrix();}
         prepMeshSkel(); setOrnTarget(); setChanged();
      }
   }
   void AnimEditor::removeMovement()
   {
      if(skel && anim)
      {
         undos.set("remMove");
         bool changed=false;
         REPA(skel->bones)if(skel->bones[i].parent==0xFF)if(AnimKeys *keys=findKeys(i))if(keys->poss.elms()>=2)
         {
            Vec   pos_delta=keys->poss[0].pos -          keys->poss.last().pos ;
            flt start_time =keys->poss[0].time, end_time=keys->poss.last().time;
            if(pos_delta.length()>EPS)
            {
               changed=true;
               for(int i=1; i<keys->poss.elms(); i++)
               {
                  AnimKeys::Pos &pos=keys->poss[i];
                  flt f=LerpR(start_time, end_time, pos.time);
                  pos.pos+=pos_delta*f;
               }
               keys->setTangents(anim->loop(), anim->length());
            }
         }
         if(changed){prepMeshSkel(); setOrnTarget(); anim->setRootMatrix(); setChanged();}
      }
   }
   void AnimEditor::freezeBone()
   {
      if(skel && anim)
      {
         if(!InRange(sel_bone, skel->bones)){Gui.msgBox(S, "No bone selected"); return;}
         undos.set("freezeBone");
         anim->freezeBone(*skel, sel_bone);
         prepMeshSkel(); setOrnTarget(); setChanged();
      }
   }
   void AnimEditor::playToggle()
   {
      play.toggle();
      preview.toGui();
   }
   void AnimEditor::playUpdate(flt multiplier)
   {
      bool play=(Kb.b(KB_W) && Kb.ctrlCmd() && !Kb.alt()); if(play)T.force_play.push();
      play|=T.force_play(); if(play && Kb.shift())CHS(multiplier);
      play|=T.play();
      if(anim)
      {
         if(flt length=anim->length())
         {
            if(play && !Ms.b(1))_anim_time+=Time.ad()*anim_speed*multiplier;
            if(_anim_time>=length && _anim_time<length+EPS)_anim_time=length;else _anim_time=Frac(_anim_time, length); // allow being at the end
         }else _anim_time=0;
      }
      if(play){prepMeshSkel(); setOrnTarget();}
   }
   void AnimEditor::prepMeshSkel()
   {
      if(Animation *anim=getVisAnim())if(ElmAnim *anim_data=data())if(Elm *skel_elm=Proj.findElm(anim_data->skel_id))if(ElmSkel *skel_data=skel_elm->skelData())if(skel_data->mesh_id.valid()) // get mesh from anim->skel->mesh
      {
         mesh=Proj.gamePath(skel_data->mesh_id);
         skel=mesh->skeleton();
         if(skel)
         {
            if(anim_skel.skeleton()!=skel || anim_skel.bones.elms()!=skel->bones.elms() || anim_skel.slots.elms()!=skel->slots.elms())anim_skel.create(skel);
            skel_anim.create(*skel, *anim);
            flt time=animTime(); if(anim->loop() && Equal(time, anim->length()))time=Max(0, anim->length()-EPS); // if animation is looped and the time is at the end, then normally it will be wrapped to the beginning due to Frac, however since we're editing/previewing, then we may want to see the last keyframe without wrapping
            anim_skel.clear().animate(skel_anim, time-Time.ad()).animateRoot(*anim, time-Time.ad()).updateMatrix().updateVelocities(false, false); // always set previous frame to setup correct motion blur velocities when manually changing anim time
            anim_skel.clear().animate(skel_anim, time          ).animateRoot(*anim, time          ).updateMatrix().updateVelocities(false, false);
         }else
         {
            anim_skel.del().updateMatrix().updateVelocities(false, false);
         }
         return;
      }
      mesh=null;
      skel=null;
      anim_skel.del();
   }
   void AnimEditor::toGui()
   {
      REPAO(     props).toGui();
      REPAO(root_props).toGui();
                preview.toGui();
      ElmAnim *data=T.data();
      loop          .set(data && data->loop  (), QUIET);
      linear        .set(data && data->linear(), QUIET);
      root_from_body.set(data && FlagTest(data->flag, ElmAnim::ROOT_FROM_BODY), QUIET);
      root_del_pos_x.set(data && FlagTest(data->flag, ElmAnim::ROOT_DEL_POS_X), QUIET);
      root_del_pos_y.set(data && FlagTest(data->flag, ElmAnim::ROOT_DEL_POS_Y), QUIET);
      root_del_pos_z.set(data && FlagTest(data->flag, ElmAnim::ROOT_DEL_POS_Z), QUIET);
      root_del_rot_x.set(data && FlagTest(data->flag, ElmAnim::ROOT_DEL_ROT_X), QUIET);
      root_del_rot_y.set(data && FlagTest(data->flag, ElmAnim::ROOT_DEL_ROT_Y), QUIET);
      root_del_rot_z.set(data && FlagTest(data->flag, ElmAnim::ROOT_DEL_ROT_Z), QUIET);
      root_2_keys   .set(data && FlagTest(data->flag, ElmAnim::ROOT_2_KEYS   ), QUIET);
      root_set_move .set(data && data->rootMove(), QUIET);
      root_set_rot  .set(data && data->rootRot (), QUIET);
   }
   void AnimEditor::applySpeed()
   {
      if(anim && !Equal(anim_speed, 1) && !Equal(anim_speed, 0))if(ElmAnim *data=T.data())
      {
         undos.set("speed");

         // adjust speed file param
         Mems<Edit::FileParams> file_params=Edit::FileParams::Decode(data->src_file);
         if(file_params.elms()==1)
         {
            TextParam &speed=file_params[0].getParam("speed");
            flt set_speed=anim_speed; if(flt cur_speed=speed.asFlt())set_speed*=cur_speed;
            speed.setValue(set_speed);
            data->setSrcFile(Edit::FileParams::Encode(file_params));
         }

         anim->length(anim->length()/anim_speed, true);
         setChanged();
         anim_speed=1;
         toGui();
      }
   }
   void AnimEditor::moveEvent(int event, flt time)
   {
      if(anim && InRange(event, anim->events))
      {
         undos.set("eventMove");
         anim->events[event].time=time;
         setChanged();
      }
   }
   void AnimEditor::newEvent(flt time)
   {
      if(anim)
      {
         undos.set("event"); // keep the same as 'renameEvent' because they're linked
         anim->events.New().set(RenameEvent.textline().is() ? RenameEvent.textline() : S+"Event", time); // reuse last name if available
         setChanged();
         if(!Kb.ctrlCmd())RenameEvent.activate(anim->events.elms()-1); // activate window for renaming created event
      }
   }
   void AnimEditor::delEvent(int index)
   {
      if(anim && InRange(index, anim->events))
      {
         undos.set("eventDel");
         anim->events.remove(index, true);
         setChanged();
         preview.removedEvent(index);
      }
   }
   void AnimEditor::renameEvent(int index, C Str &old_name, C Str &new_name)
   {
      if(anim)FREPA(anim->events)if(old_name==anim->events[i].name)if(!index--)
      {
         undos.set("event"); // keep the same as 'newEvent' because they're linked
         Set(anim->events[i].name, new_name);
         setChanged();
         break;
      }
   }
   void AnimEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmAnim *data=elm->animData()){data->newVer(); if(anim)data->from(*anim);} // modify just before saving/sending in case we've received data from server after edit
         if(anim){Save(*anim, Proj.gamePath(elm->id)); Proj.savedGame(*elm);}
         Server.setElmLong(elm->id);
      }
      changed=false;
   }
   void AnimEditor::setChanged(bool file)
   {
      if(elm)
      {
         changed=true;
         if(ElmAnim *data=elm->animData())
         {
            data->newVer();
            if(anim)data->from(*anim);
            if(file)data->file_time.getUTC();
            optimize_anim.refresh();
         }
      }
   }
   void AnimEditor::validateFullscreen()
   {
      Mode   .tabAvailable(MODE_ANIM, fullscreen && elm);
      preview.visible     (          !fullscreen && elm);
   }
   void AnimEditor::toggleFullscreen()
   {
      fullscreen^=1;
      validateFullscreen();
      activate(elm);
      preview.kbSet();
      toGui();
   }
   void AnimEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_ANIM)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();

         if(skel)
         {
            sel_bone_name=(InRange(sel_bone, skel->bones) ? skel->bones[sel_bone].name : null);
         }

         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm)anim=Animations(Proj.gamePath(elm->id));else anim=null;
         toGui();
         Proj.refresh(false, false);
         validateFullscreen();
         RenameEvent.hide();

         optimize_anim.refresh();
         prepMeshSkel();
         selBone(skel ? skel->findBoneI(sel_bone_name) : -1);
         setOrnTarget();
         optimize_anim.hide();
         time_range_speed.hide();
         Gui.closeMsgBox(transform_obj_dialog_id);
         preview.moveToTop();
      }
   }
   void AnimEditor::focus()
   {
      if(T.elm){if(fullscreen){Mode.set(MODE_ANIM); HideBig();}else preview.activate();}
   }
   void AnimEditor::activate(Elm *elm)
   {
      set(elm); focus();
   }
   void AnimEditor::toggle(Elm *elm)
   {
      if(elm==T.elm && (fullscreen ? selected() : true))elm=null;
      if(fullscreen || preview.maximized())activate(elm);else set(elm); // if editor is in fullscreen mode, or the window is maximized then activate
   }
   void AnimEditor::elmChanging(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         undos.set(null, true);
      }
   }
   void AnimEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         prepMeshSkel();
         setOrnTarget();
         toGui();
      }
   }
   void AnimEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
   void AnimEditor::setTarget(C Str &obj_id)
   {
      if(anim)if(ElmAnim *anim_data=data())
      {
         if(obj_id.is())
         {
            if(Elm *obj=Proj.findElm(obj_id))if(ElmObj *obj_data=obj->objData())
            {
               bool has_skel=false;
               if(Elm *mesh=Proj.findElm( obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())
               if(Elm *skel=Proj.findElm(mesh_data->skel_id))if(ElmSkel *skel_data=skel->skelData())
               {
                  has_skel=true;
                  if(skel->id!=anim_data->skel_id)
                  {
                     undos.set("skel");

                     /*if(0) // don't transform, because for example if original mesh was super small, then it had to be scaled up, making the transform very big, and when transforming to new transform the scale could get big, so just accept the current transform, assume that user has scaled the orignal mesh/skel/anim to match the new mesh/skel/anim
                     {
                        Elm      *old_skel_elm=Proj.findElm(anim_data.skel_id, ELM_SKEL);
                      C Skeleton *old_skel=(old_skel_elm ? Skeletons(Proj.gamePath(*old_skel_elm)) : null),
                                 *new_skel=                Skeletons(Proj.gamePath(     skel.id ));

                        anim.transform(GetTransform(anim_data.transform(), skel_data.transform()), old_skel ? *old_skel : *new_skel); // transform from current to target, if 'old_skel' is not available then try using 'new_skel'
                     }*/
                     if(RenameAnimBonesOnSkelChange)if(Skeleton *new_skel=Skeletons(Proj.gamePath(skel->id)))
                     {
                        if(ObjEdit.skel_elm && ObjEdit.skel_elm->id==skel->id)ObjEdit.flushMeshSkel(); // !! if this skeleton is currently edited, then we have to flush it, for example, it could have been empty at the start and we've just added bones, so 'new_skel' is empty and bones are only in 'ObjEdit.mesh_skel_temp' memory, and before changing 'anim_data.skel_id' !!
                        anim->setBoneNameTypeIndexesFromSkeleton(*new_skel);
                     }

                     anim_data->skel_id  =skel->id; // !! set after 'flushMeshSkel' !!
                     anim_data->skel_time.getUTC();
                     anim_data->transform=skel_data->transform;
                     setChanged();
                     toGui(); Proj.refresh(false, false);
                  }
               }
               if(!has_skel)Gui.msgBox(S, "Can't set target object because it doesn't have a mesh skeleton.");
            }
         }else
         {
            undos.set("skel");
            anim_data->skel_id.zero();
            anim_data->skel_time.getUTC();
            setChanged();
            toGui(); Proj.refresh(false, false);
         }
      }
   }
   void AnimEditor::drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj) || preview.contains(obj))
         REPA(elms)
            if(Elm *obj=Proj.findElm(elms[i], ELM_OBJ))
      {
         setTarget(obj->id.asHex());
         elms.remove(i, true);
         break;
      }
   }
AnimEditor::AnimEditor() : elm_id(UIDZero), elm(null), changed(false), fullscreen(false), copied_bone_pos_relative(false), blend_range(-1), anim(null), skel(null), anim_speed(1), key_time(0), lit_bone(-1), sel_bone(-1), bone_axis(-1), orn_target(0), orn_perp(0), copied_bone_pos(0), undos(true), _anim_time(0) {}

AnimEditor::Preview::Preview() : draw_bones(false), draw_slots(false), draw_axis(false), draw_plane(false), event_lit(-1), event_sel(-1), time_speed(1), prop_max_x(0), cam_yaw(PI), cam_pitch(0), cam_zoom(1), length(null), event(null) {}

AnimEditor::OptimizeAnim::OptimizeAnim() : refresh_needed(true), preview(true), angle_eps(EPS_ANIM_ANGLE), pos_eps(EPS_ANIM_POS), scale_eps(EPS_ANIM_SCALE), file_size(null), optimized_size(null) {}

/******************************************************************************/
