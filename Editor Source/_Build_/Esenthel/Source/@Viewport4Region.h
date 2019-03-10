/******************************************************************************/
/******************************************************************************/
class Viewport4Region : Region
{
   Edit::Viewport4 v4;
   Button         cam_reset, cam_center, cam_spherical, cam_lock, wire;
   Slider         fpp_speed;
   Tabs           cam_tabs;
   ComboBox       ctrls;
   Str            prefix;

   static void CamChanged   (Viewport4Region &vr);
   static void CamSpherical (Viewport4Region &vr);
   static void CamMode      (Viewport4Region &vr);
   static void CamLocked    (Viewport4Region &vr);
   static void CamLock      (Viewport4Region &vr);
   static void CamReset     (Viewport4Region &vr);
   static void CamCenter    (Viewport4Region &vr);
   static void CamCenterZ   (Viewport4Region &vr);
   static void CamCenterAuto(Viewport4Region &vr);
   static void CamTop       (Viewport4Region &vr);
   static void CamDiagonal  (Viewport4Region &vr);
   static void CamFront     (Viewport4Region &vr);
   static void CamLeft      (Viewport4Region &vr);
   static void Wire         (Viewport4Region &vr);
   static void FppSpeed     (Viewport4Region &vr);

   static void CtrlsDPad   (Viewport4Region &vr);
   static void CtrlsAPad   (Viewport4Region &vr);
   static void CtrlsDrag   (Viewport4Region &vr);
   static void CtrlsArrows (Viewport4Region &vr);
   static void CtrlsOrient (Viewport4Region &vr);
   static void CtrlsOrientS(Viewport4Region &vr);
   static void CtrlsZoom   (Viewport4Region &vr);

   void setOrientMenu();
   void setMoveMenu();
   void setZoomMenu();
   void   moveMode(Edit::Viewport4::  MOVE_MODE mode);   void toggleMoveMode  (Edit::Viewport4::  MOVE_MODE mode);
   void orientMode(Edit::Viewport4::ORIENT_MODE mode);   void toggleOrientMode(Edit::Viewport4::ORIENT_MODE mode);
   void zoom      (bool                       on  ); void toggleZoom      (                               );    

   void setMenu(bool on);                           
   void setMenu(Node<MenuElm> &menu, C Str &prefix);
   Viewport4Region& create(void (*draw)(Viewport&), bool world_editor, flt default_pitch=0, flt default_yaw=0, flt default_dist=1, flt from=0.01f, flt range=1000);
   virtual void resize();
   void setView(int view);
   void toggleView(int view);
   virtual void camCenter(bool zoom);  
   virtual void update   (C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
