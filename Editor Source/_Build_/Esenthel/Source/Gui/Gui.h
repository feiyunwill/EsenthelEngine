/******************************************************************************/
extern Progress UpdateProgress;
extern Thread   UpdateThread;
extern Image    WhiteImage;
extern ImagePtr GlowBorder;
extern Panel    WindowNoShadow, ActiveWindowNoShadow;
extern GuiSkin  DarkSkin, LightSkin, TransparentSkin, HalfTransparentSkin, LitSkin, RedSkin, NoComboBoxImage, NoShadowSkin;
extern cchar8 *ModeText[]
;
extern OP_HM_MODE  OpHm ;
extern OP_OBJ_MODE OpObj;
/******************************************************************************/
bool BigVisible  (          );
void HideBig     (          );
void HideProjsAct(GuiObj &go);
void HideProjAct (GuiObj &go);
void HideEditAct (GuiObj &go);
template<typename TYPE> int Elms(C ObjPtrs<TYPE> &obj_ptrs);
void Highlight(C Rect &rect, flt alpha, C Color &color=Color(255, 64, 64, 0), flt e=0.08f);
Color MaterialColor(C MaterialPtr &mtrl);
ImagePtr MaterialImage(C MaterialPtr &mtrl);
Color MaterialColor(C WaterMtrlPtr &mtrl);
ImagePtr MaterialImage(C WaterMtrlPtr &mtrl);
void MakeRed(Color &col);
void MakeLit(Color &col);
void MakeTransparent(Color &col);
void MakeTransparent(GuiSkin::Button &button);
void MakeTransparent(GuiSkin::ButtonImage &button);
void SetGuiSkin(UID id=Gui.default_skin);
void InitGui();
void ShutGui();
void SetHmOp(OP_HM_MODE op);
void SetObjOp(OP_OBJ_MODE op);
void Drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos);
/******************************************************************************/
