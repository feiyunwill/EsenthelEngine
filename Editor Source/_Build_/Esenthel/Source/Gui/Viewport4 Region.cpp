/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   void Viewport4Region::CamChanged(Viewport4Region &vr) {vr.setView( vr.cam_tabs     ());}
   void Viewport4Region::CamSpherical(Viewport4Region &vr) {vr.v4.fpp (!vr.cam_spherical()); vr.fpp_speed.visible(vr.v4.fpp());}
   void Viewport4Region::CamMode(Viewport4Region &vr) {if(vr.cam_spherical.visible())vr.cam_spherical.push();}
   void Viewport4Region::CamLocked(Viewport4Region &vr) {vr.v4.lock( vr.cam_lock     ());}
   void Viewport4Region::CamLock(Viewport4Region &vr) {vr.cam_lock.push();}
   void Viewport4Region::CamReset(Viewport4Region &vr) {vr.v4.resetPosOrn();}
   void Viewport4Region::CamCenter(Viewport4Region &vr) {vr.camCenter(false);}
   void Viewport4Region::CamCenterZ(Viewport4Region &vr) {vr.camCenter(true );}
   void Viewport4Region::CamCenterAuto(Viewport4Region &vr) {vr.camCenter(Kb.shift());}
   void Viewport4Region::CamTop(Viewport4Region &vr) {vr.toggleView(Edit::Viewport4::VIEW_TOP     );}
   void Viewport4Region::CamDiagonal(Viewport4Region &vr) {vr.toggleView(Edit::Viewport4::VIEW_DIAGONAL);}
   void Viewport4Region::CamFront(Viewport4Region &vr) {vr.toggleView(Edit::Viewport4::VIEW_FRONT   );}
   void Viewport4Region::CamLeft(Viewport4Region &vr) {vr.toggleView(Edit::Viewport4::VIEW_LEFT    );}
   void Viewport4Region::Wire(Viewport4Region &vr) {vr.wire.push();}
   void Viewport4Region::FppSpeed(Viewport4Region &vr) {vr.v4.fppSpeed(2*Pow(75, 1-vr.fpp_speed()));}
   void Viewport4Region::CtrlsDPad(Viewport4Region &vr) {vr.toggleMoveMode  (Edit::Viewport4::MOVE_DPAD  );}
   void Viewport4Region::CtrlsAPad(Viewport4Region &vr) {vr.toggleMoveMode  (Edit::Viewport4::MOVE_APAD  );}
   void Viewport4Region::CtrlsDrag(Viewport4Region &vr) {vr.toggleMoveMode  (Edit::Viewport4::MOVE_DRAG  );}
   void Viewport4Region::CtrlsArrows(Viewport4Region &vr) {vr.toggleMoveMode  (Edit::Viewport4::MOVE_ARROWS);}
   void Viewport4Region::CtrlsOrient(Viewport4Region &vr) {vr.toggleOrientMode(Edit::Viewport4::ORIENT_TOP_LEFT);}
   void Viewport4Region::CtrlsOrientS(Viewport4Region &vr) {vr.toggleOrientMode(Edit::Viewport4::ORIENT_RIGHT   );}
   void Viewport4Region::CtrlsZoom(Viewport4Region &vr) {vr.toggleZoom      ();}
   void Viewport4Region::setOrientMenu()
   {
      ctrls.menu("Orientation"       , v4.orientMode()==Edit::Viewport4::ORIENT_TOP_LEFT, QUIET);
      ctrls.menu("Orientation (side)", v4.orientMode()==Edit::Viewport4::ORIENT_RIGHT   , QUIET);
   }
   void Viewport4Region::setMoveMenu()
   {
      ctrls.menu("Digital Pad", v4.moveMode()==Edit::Viewport4::MOVE_DPAD  , QUIET);
      ctrls.menu("Analog Pad" , v4.moveMode()==Edit::Viewport4::MOVE_APAD  , QUIET);
      ctrls.menu("Drag Pad"   , v4.moveMode()==Edit::Viewport4::MOVE_DRAG  , QUIET);
      ctrls.menu("Arrows"     , v4.moveMode()==Edit::Viewport4::MOVE_ARROWS, QUIET);
   }
   void Viewport4Region::setZoomMenu()
   {
      ctrls.menu("Zoom", v4.drawZoom(), QUIET);
   }
   void   Viewport4Region::moveMode(Edit::Viewport4::  MOVE_MODE mode) {v4.  moveMode(mode); setMoveMenu  ();}
   void Viewport4Region::toggleMoveMode(Edit::Viewport4::  MOVE_MODE mode) {v4.toggleMoveMode  (mode); setMoveMenu  ();}
   void Viewport4Region::orientMode(Edit::Viewport4::ORIENT_MODE mode) {v4.orientMode(mode); setOrientMenu();}
   void Viewport4Region::toggleOrientMode(Edit::Viewport4::ORIENT_MODE mode) {v4.toggleOrientMode(mode); setOrientMenu();}
   void Viewport4Region::zoom(bool                       on  ) {v4.drawZoom  (on  ); setZoomMenu  ();}
   void Viewport4Region::toggleZoom(                               ) {v4.toggleZoom      (    ); setZoomMenu  ();}
   void Viewport4Region::setMenu(bool on) {Proj.menu.setCommand(prefix+"View", on);}
   void Viewport4Region::setMenu(Node<MenuElm> &menu, C Str &prefix)
   {
      T.prefix=prefix;
      {
         Node<MenuElm> &v=(menu+=prefix+"View"); v.display("View").disabled=true;
         v.New().create("Camera Top"     , CamTop     , T).kbsc(KbSc(KB_1    , KBSC_CTRL_CMD           )).flag(MENU_HIDDEN);
         v.New().create("Camera Diagonal", CamDiagonal, T).kbsc(KbSc(KB_2    , KBSC_CTRL_CMD           )).flag(MENU_HIDDEN);
         v.New().create("Camera Front"   , CamFront   , T).kbsc(KbSc(KB_3    , KBSC_CTRL_CMD           )).flag(MENU_HIDDEN);
         v.New().create("Camera Left"    , CamLeft    , T).kbsc(KbSc(KB_4    , KBSC_CTRL_CMD           )).flag(MENU_HIDDEN);
         v.New().create("Camera Reset"   , CamReset   , T).kbsc(KbSc(KB_BACK                           )).flag(MENU_HIDDEN);
         v.New().create("Camera Center"  , CamCenter  , T).kbsc(KbSc(KB_TILDE, KBSC_CTRL_CMD           )).flag(MENU_HIDDEN);
         v.New().create("Camera Center Z", CamCenterZ , T).kbsc(KbSc(KB_TILDE, KBSC_CTRL_CMD|KBSC_SHIFT)).flag(MENU_HIDDEN);
         v.New().create("Camera Mode"    , CamMode    , T).kbsc(KbSc(KB_TAB                            )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
         v.New().create("Camera Unlock"  , CamLock    , T).kbsc(KbSc(KB_TAB  , KBSC_CTRL_CMD           )).flag(MENU_HIDDEN|MENU_TOGGLABLE).display(MLTC(null, PL, u"Odblokuj Kamery")).desc("Allow to move each camera separately");
         v.New().create("Wireframe"      , Wire       , T).kbsc(KbSc(KB_W    , KBSC_ALT                )).flag(MENU_HIDDEN|MENU_TOGGLABLE);
      }
   }
   Viewport4Region& Viewport4Region::create(void (*draw)(Viewport&), bool world_editor, flt default_pitch, flt default_yaw, flt default_dist, flt from, flt range)
   {
      Str cam_desc[]=
      {
         S+"Top Left Camera\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+1",
         S+"Top Right Camera\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+2",
         S+"Bottom Left Camera\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+3",
         S+"Bottom Right Camera\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+4",
      };
      Node<MenuElm> csc;
      csc.New().create("Digital Pad", CtrlsDPad  , T).desc("Draw camera movement digital pad").flag(MENU_TOGGLABLE);
      csc.New().create("Analog Pad" , CtrlsAPad  , T).desc("Draw camera movement analog pad" ).flag(MENU_TOGGLABLE);
      csc.New().create("Drag Pad"   , CtrlsDrag  , T).desc("Draw camera movement drag pad"   ).flag(MENU_TOGGLABLE);
      csc.New().create("Arrows"     , CtrlsArrows, T).desc("Draw camera movement arrows"     ).flag(MENU_TOGGLABLE);
      csc++;
      csc.New().create("Orientation"       , CtrlsOrient , T).desc("Draw camera orientation cube\n\nYou can click on its parts to rotate the camera.\nDrag it with LeftClick for smooth rotation.\nDrag it with RightClick for camera movement.\nDrag it with left and RightClick for alternative camera movement.").flag(MENU_TOGGLABLE);
      csc.New().create("Orientation (side)", CtrlsOrientS, T).desc("Draw camera orientation cube\n\nYou can click on its parts to rotate the camera.\nDrag it with LeftClick for smooth rotation.\nDrag it with RightClick for camera movement.\nDrag it with left and RightClick for alternative camera movement.").flag(MENU_TOGGLABLE);
      csc++;
      csc.New().create("Zoom", CtrlsZoom, T).desc("Draw camera zoom pad").flag(MENU_TOGGLABLE);

      ::EE::Region::create(Rect_LU(0, 0, 3, 2)).removeSlideBars().skin(&EmptyGuiSkin, false); kb_lit=false; flt h=0.05f; // set initial size so sub-windows can be aligned to sides
      T+=v4.create(draw, default_pitch, default_yaw, default_dist, from, range).sel(Edit::Viewport4::VIEW_FRONT).perspFov(VidOpt.advanced.fov);
      T+=cam_tabs     .create(Rect_LU(0, 0, 2*0.03f, 2*0.03f), 0, (cchar8**)null, 4).func(CamChanged, T).set(v4.sel(), QUIET); cam_tabs.tab(0).rect(Rect(cam_tabs.rect().left(), cam_tabs.rect().up())); cam_tabs.tab(1).rect(Rect(cam_tabs.rect().center(), cam_tabs.rect().ru())); cam_tabs.tab(2).rect(Rect(cam_tabs.rect().ld(), cam_tabs.rect().center())); cam_tabs.tab(3).rect(Rect(cam_tabs.rect().down(), cam_tabs.rect().right())); FREPA(cam_tabs)cam_tabs.tab(i).setCornerTab(i&1, !(i&2)).desc(cam_desc[i]);
      T+=cam_spherical.create(Rect_LU(cam_tabs     .rect().ld(), h)).func(CamSpherical , T).focusable(false).desc(S+"Camera mode (Spherical or FPP)\nKeyboard Shortcut: Tab"); cam_spherical.mode=BUTTON_TOGGLE; cam_spherical.set(true); cam_spherical.image="Gui/Misc/spherical.img";
      T+=cam_lock     .create(Rect_LU(cam_spherical.rect().ld(), h)).func(CamLocked    , T).focusable(false).desc(S+"Lock camera's position and orientation\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Tab"); cam_lock.mode=BUTTON_TOGGLE; cam_lock.set(true).hide(); cam_lock.image="Gui/Misc/lock.img";
      T+=cam_reset    .create(Rect_LU(cam_tabs     .rect().ru(), h), "R").func(CamReset, T).focusable(false).desc(S+"Reset camera position and orientation\nKeyboard Shortcut: Backspace");
      T+=cam_center   .create(Rect_LU(cam_reset    .rect().ru(), h)).func(CamCenterAuto, T).focusable(false).desc(S+"Center camera on object\n"+(world_editor ? "Double click to lock centering\n" : "")+"Keyboard Shortcut: "+Kb.ctrlCmdName()+"+Tilde\nOptionally hold Shift to zoom in"); cam_center.image="Gui/Misc/center.img";
      T+=fpp_speed    .create(Rect_LU(cam_spherical.rect().ld(), h, h*2).extendY(-h/4)).func(FppSpeed, T).set(0.5f).desc("Fpp Camera Speed\nToggle with MouseWheel").hide();

      T+=ctrls.create(Rect_LU(cam_center.rect().ru()+Vec2(h, 0), 0.20f, h)).setData(csc).focusable(false); ctrls.text="Controls"; ctrls.flag|=COMBOBOX_CONST_TEXT; FlagDisable(ctrls.flag, COMBOBOX_MOUSE_WHEEL); ctrls.desc(S+"Set which screen controls should be visible.\nBesides screen controls you can also manipulate the camera using following keys:\n          W S A D Q E  Arrow Keys - Move Camera\n                            "+Kb.ctrlCmdName()+"+Alt+MMB - Zoom in/out\nSpace       / 4MB        /  Alt+MMB - Move Camera\nSpace+Win / 4MB+5MB / "+Kb.ctrlCmdName()+"+MMB - Move Camera (alternative)\n         Win /        5MB /       MMB - Rotate Camera");
      T+=wire .create(Rect_LU(ctrls     .rect().ld()           , h      )).focusable(false).desc("Use wireframe rendering\nKeyboard Shortcut: Alt+W"); wire.mode=BUTTON_TOGGLE; wire.image="Gui/Misc/wireframe.img";

      setOrientMenu(); setMoveMenu(); setZoomMenu();
      return T;
   }
   void Viewport4Region::resize()
   {
         rect(EditRect());
      v4.rect(Rect_LU(0, 0, rect().w()+D.pixelToScreenSize(Vec2(0.5f)).x, rect().h())); // increase size because due to float imprecisions the mouse cursor may not cut the rect when being at the right end of the screen
   }
   void Viewport4Region::setView(int view)
   {
      v4.sel(view);
      cam_tabs.set(v4.sel(), QUIET);
      cam_lock.visible(v4.sel()<0);
      fpp_speed.pos((cam_lock.visible() ? cam_lock.rect().ld() : cam_spherical.rect().ld())-Vec2(0, fpp_speed.rect().w()/4));
   }
   void Viewport4Region::toggleView(int view)
   {
      v4.toggleView(view);
      cam_tabs.set(v4.sel(), QUIET);
      cam_lock.visible(v4.sel()<0);
      fpp_speed.pos((cam_lock.visible() ? cam_lock.rect().ld() : cam_spherical.rect().ld())-Vec2(0, fpp_speed.rect().w()/4));
   }
   void Viewport4Region::camCenter(bool zoom) {}
   void Viewport4Region::update(C GuiPC &gpc)
{
      if(visible() && gpc.visible)
      {
         v4.update(); // this sets D.viewRect
         if(fpp_speed.visible() && Ms.wheel() && v4.focus() && !Kb.ctrlCmd() && !Kb.shift())fpp_speed.set(fpp_speed()+Ms.wheel()*0.1f); // ctrl and shift are reserved for terrain brush params

         // set keyboard focus to this region so that Space can be used for moving the camera
         if((Ms.pixelDelta().any() || Ms.wheel()) && contains(Gui.ms()))
         {
            GUI_OBJ_TYPE kb_type=(Gui.kb() ? Gui.kb()->type() : GO_NONE);
            if(kb_type!=GO_TEXTLINE && kb_type!=GO_TEXTBOX) // don't change if keyboard focus is on text edit
          //if(Gui.window()!=&RenameElm && Gui.window()!=&ReplaceName && Gui.window()!=&RenameBone && Gui.window()!=&RenameSlot && Gui.window()!=&ObjEdit.mesh_variations.rename) not needed since textline/textbox checked above
               if(!contains(Gui.kb()) || kb_type!=GO_VIEWPORT)
                  if(Edit::Viewport4::View *view=v4.focus())view->viewport.kbSet();
         }
      }
      ::EE::Region::update(gpc);
   }
/******************************************************************************/
