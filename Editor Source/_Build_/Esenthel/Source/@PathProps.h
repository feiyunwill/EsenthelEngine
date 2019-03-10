/******************************************************************************/
/******************************************************************************/
class PathProps : Region
{
   Memx<Property> props;
   Text           tcell, vcell, tctrl_r, vctrl_r                    ; // t-text, v-value
   Button         dcell, icell, dctrl_r, ictrl_r, apply, copy, paste; // d-dec , i-inc
   TextWhite      ts, tsr;

   static ElmWorld* WS(); // world settings

   static void Changed(C Property &prop);

   static void CellSize  (PathProps &pp, C Str &text);   static Str CellSize  (C PathProps &pp);
   static void CtrlRadius(PathProps &pp, C Str &text);   static Str CtrlRadius(C PathProps &pp);
   static void CtrlHeight(PathProps &pp, C Str &text);   static Str CtrlHeight(C PathProps &pp);
   static void MaxSlope  (PathProps &pp, C Str &text);   static Str MaxSlope  (C PathProps &pp);
   static void MaxClimb  (PathProps &pp, C Str &text);   static Str MaxClimb  (C PathProps &pp);

   static void IncCellSize(PathProps &pp);
   static void DecCellSize(PathProps &pp);

   static void IncCtrlRadius(PathProps &pp);
   static void DecCtrlRadius(PathProps &pp);

   static void Apply(PathProps &pp);

   static void Copy (PathProps &pp);
   static void Paste(PathProps &pp);

   PathProps& create();
   void toGui();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
