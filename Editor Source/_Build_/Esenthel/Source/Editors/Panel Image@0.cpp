/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
PanelImageEditor PanelImageEdit;
/******************************************************************************/

/******************************************************************************/
   cchar8 *PanelImageEditor::image_modes[]=
   {
      "Multiply"  , // 0
      "Scale"     , // 1
      "Add"       , // 2
      "Add Signed", // 3
      "Blend"     , // 4
   };
/******************************************************************************/
      void PanelImageEditor::Change::create(ptr user)
{
         data=PanelImageEdit.params;
         PanelImageEdit.undoVis();
      }
      void PanelImageEditor::Change::apply(ptr user)
{
         PanelImageEdit.params.undo(data);
         PanelImageEdit.setChanged();
         PanelImageEdit.toGui();
         PanelImageEdit.undoVis();
      }
   void PanelImageEditor::undoVis() {SetUndo(undos, undo, redo);}
  PanelImageEditor::~PanelImageEditor() {del();}
   void PanelImageEditor::Preview(Viewport &viewport) {((PanelImageEditor*)viewport.user)->preview();}
          void PanelImageEditor::preview()
   {
      SyncLocker locker(lock);
      bool       have_image=false;
      Rect       rect=D.viewRect(), image_rect;
      switch(preview_mode())
      {
         case 0:
         {
            have_image=true; image_rect=panel_image.image.fit(rect); panel_image.image.draw(image_rect); image_rect.draw(BLACK, false); if(draw_depth())drawDepth(image_rect); if(draw_lines())panel_image.drawBaseLines(PURPLE, image_rect);
         }break;

         case 1:
         {
            have_image=true; image_rect=rect; image_rect.min.y=rect.centerY(); image_rect=panel_image.image.fit(image_rect); panel_image.image.draw(image_rect); image_rect.draw(TURQ, false); if(draw_depth())drawDepth(image_rect); if(draw_lines())panel_image.drawBaseLines(PURPLE, image_rect);
            Vec2 center=rect.lerp(0.5f, 0.25f);
            flt  h=rect.h()*0.25f, w=h*params.width/params.height, max_w=rect.w()*0.9f; if(w>max_w){h*=max_w/w; w=max_w;}
            Rect_C rc(center, w, h);
            panel_image.draw(rc); if(draw_lines()){panel_image.drawScaledLines(RED, rc); panel_image.drawInnerPadding(GREEN, rc); rc.draw(TURQ, false);}
         }break;

         case 2: DrawPanelImage(panel_image, rect, draw_lines()); break;
      }
      if(have_image && Gui.ms()==&viewport && Cuts(Ms.pos(), image_rect))
      {
         Vec2 f(LerpR(image_rect.min.x, image_rect.max.x, Ms.pos().x)*soft.w(),
                LerpR(image_rect.max.y, image_rect.min.y, Ms.pos().y)*soft.h());
         Color c=soft.colorFLinear(f.x, f.y),
              ca=soft.areaColorAverage(f, 8);
         TextStyleParams tsp; tsp.align.set(0, 1); tsp.color=ColorInverse(ca); tsp.color.a=255; tsp.shadow=tsp.color.lum(); tsp.size=0.045f;
         D.text(tsp, Ms.pos(), S+"R:"+c.r+", G:"+c.g+", B:"+c.b+", A:"+c.a);
      }
   }
   void PanelImageEditor::drawDepth(C Rect &rect)
   {
      Memt<Vec2> points; int y=depth.h()/2; flt w=0.006f; Rect r=rect; r.extend(w*-0.5f);
      FREP(depth.w())
      {
         flt x=i/flt(depth.w()-1);
         points.add(r.lerp(x, 0.5f+0.5f*depth.pixelF(i, y)));
      }
      VI.color(RED); REP(points.elms()-1)VI.line(points[i], points[i+1], w);
      VI.end();
   }
   bool PanelImageEditor::Make(Thread &thread) {return (*(PanelImageEditor*)thread.user).make();}
          bool PanelImageEditor::make()
   {
      if(event.wait() && !thread.wantStop())
      {
         SyncLockerEx locker(lock); Params params=T.params; locker.off();
         ThreadMayUseGPUData();
         PanelImage temp; Image temp_soft, temp_depth; params.make(temp, WorkerThreads, Proj, &temp_soft, &temp_depth, true); {locker.on(); Swap(panel_image, temp); Swap(soft, temp_soft); Swap(depth, temp_depth); locker.off();}
      }
      return true;
   }
   void PanelImageEditor::Undo(PanelImageEditor &editor) {editor.undos.undo();}
   void PanelImageEditor::Redo(PanelImageEditor &editor) {editor.undos.redo();}
   void PanelImageEditor::Locate(PanelImageEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void PanelImageEditor::Copy(PanelImageEditor &editor) {TextData data; FileText f; SaveProperties(editor.props, data.nodes, ' ', true); data.save(f.writeMem()); ClipSet(f.rewind().getAll());}
   void PanelImageEditor::Paste(PanelImageEditor &editor) {TextData data; FileText f; f.writeMem().putText(ClipGet()); data.load(f.rewind()); editor.force_undo_change_type="paste"; LoadProperties(editor.props, data.nodes, ' ', true); editor.force_undo_change_type=null; editor.setChanged();}
                     C Rect& PanelImageEditor::rect(            )C {return ::EE::Window::rect();}
   PanelImageEditor& PanelImageEditor::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      flt p=0.02f;
      region      .rect(Rect   (p, -clientHeight()+p, p+props_w+region.slidebarSize(), -0.07f));
      viewport    .rect(Rect   (region.rect().max.x+p+0.01f, region.rect().min.y, clientWidth()-p, -0.07f));
      preview_mode.rect(Rect_LD(viewport.rect().lu()+Vec2(0, 0.005f), 0.67f, 0.055f), 0, true);
      draw_depth  .rect(Rect_L (preview_mode.rect().right()+Vec2(0.05f, 0), 0.22f, 0.05f));
      draw_lines  .rect(Rect_L (draw_depth  .rect().right()+Vec2(0.01f, 0), 0.22f, 0.05f));
      return T;
   }
   void PanelImageEditor::PreChanged(C Property &prop) {PanelImageEdit.undos.set(PanelImageEdit.force_undo_change_type ? PanelImageEdit.force_undo_change_type : &prop);}
   void    PanelImageEditor::Changed(C Property &prop) {if(!PanelImageEdit.force_undo_change_type)PanelImageEdit.setChanged();}
   void PanelImageEditor::SetCutLeft(Params &p, C Str &t) {p.cut_left  =TextBool(t); p.base.cut_left  .getUTC();}
   void PanelImageEditor::SetCutRight(Params &p, C Str &t) {p.cut_right =TextBool(t); p.base.cut_right .getUTC();}
   void PanelImageEditor::SetCutTop(Params &p, C Str &t) {p.cut_top   =TextBool(t); p.base.cut_top   .getUTC();}
   void PanelImageEditor::SetCutBottom(Params &p, C Str &t) {p.cut_bottom=TextBool(t); p.base.cut_bottom.getUTC();}
   void PanelImageEditor::SetCutCorner00(Params &p, C Str &t) {p.cut_corners[0][0]=TextBool(t); p.base.cut_corners[0][0].getUTC();}
   void PanelImageEditor::SetCutCorner01(Params &p, C Str &t) {p.cut_corners[0][1]=TextBool(t); p.base.cut_corners[0][1].getUTC();}
   void PanelImageEditor::SetCutCorner10(Params &p, C Str &t) {p.cut_corners[1][0]=TextBool(t); p.base.cut_corners[1][0].getUTC();}
   void PanelImageEditor::SetCutCorner11(Params &p, C Str &t) {p.cut_corners[1][1]=TextBool(t); p.base.cut_corners[1][1].getUTC();}
   void PanelImageEditor::SetCutCornerSlope(Params &p, C Str &t) {p.cut_corner_slope =TextFlt(t); p.base.cut_corner_slope .getUTC();}
   void PanelImageEditor::SetCutCornerAmount(Params &p, C Str &t) {p.cut_corner_amount=TextFlt(t); p.base.cut_corner_amount.getUTC();}
   void PanelImageEditor::SetForceUniformStretch0(Params &p, C Str &t) {p.force_uniform_stretch[0]=TextBool(t); p.base.force_uniform_stretch[0].getUTC();}
   void PanelImageEditor::SetForceUniformStretch1(Params &p, C Str &t) {p.force_uniform_stretch[1]=TextBool(t); p.base.force_uniform_stretch[1].getUTC();}
   void PanelImageEditor::SetCompressed(Params &p, C Str &t) {p.base.compressed=TextBool(t); p.base.compressed_time.getUTC();}
   void PanelImageEditor::SetMipMaps(Params &p, C Str &t) {p.base.mip_maps  =TextBool(t); p.base.mip_maps_time  .getUTC();}
   void PanelImageEditor::SetResolution(Params &p, C Str &t) {p.resolution     =TextInt (t); p.base.resolution     .getUTC();}
   void PanelImageEditor::SetWidth(Params &p, C Str &t) {p.width          =TextInt (t); p.base.width          .getUTC();}
   void PanelImageEditor::SetHeight(Params &p, C Str &t) {p.height         =TextInt (t); p.base.height         .getUTC();}
   void PanelImageEditor::SetRoundCorners(Params &p, C Str &t) {p.round_corners   =TextFlt (t); p.base.round_corners   .getUTC();}
   void PanelImageEditor::SetLeftSlope(Params &p, C Str &t) {p.left_slope      =TextFlt (t); p.base.left_slope      .getUTC();}
   void PanelImageEditor::SetRightSlope(Params &p, C Str &t) {p.right_slope     =TextFlt (t); p.base.right_slope     .getUTC();}
   void PanelImageEditor::SetBorderSize(Params &p, C Str &t) {p.border_size     =TextFlt (t); p.base.border_size     .getUTC();}
   void PanelImageEditor::SetMaxSideStretch(Params &p, C Str &t) {p.max_side_stretch=TextFlt (t); p.base.max_side_stretch.getUTC();}
   void PanelImageEditor::SetExtend(Params &p, C Str &t) {p.extend          =TextVec2(t); p.base.extend          .getUTC();}
   void PanelImageEditor::ExtendInnerPaddLR(  Params &p, C Str &t) {p.extend_inner_padd.min.x=p.extend_inner_padd.max.x=TextFlt(t); p.base.extend_inner_padd.getUTC();}
   Str  PanelImageEditor::ExtendInnerPaddLR(C Params &p          ) {return p.extend_inner_padd.min.x;}
   void PanelImageEditor::ExtendInnerPaddT(  Params &p, C Str &t) {p.extend_inner_padd.max.y=-TextFlt(t); p.base.extend_inner_padd.getUTC();}
   Str  PanelImageEditor::ExtendInnerPaddT(C Params &p          ) {return -p.extend_inner_padd.max.y;}
   void PanelImageEditor::ExtendInnerPaddB(  Params &p, C Str &t) {p.extend_inner_padd.min.y=TextFlt(t); p.base.extend_inner_padd.getUTC();}
   Str  PanelImageEditor::ExtendInnerPaddB(C Params &p          ) {return p.extend_inner_padd.min.y;}
   void PanelImageEditor::SetColor(Params &p, C Str &t) {p.color       =TextVec4(t); p.base.color       .getUTC();}
   void PanelImageEditor::SetColorTop(Params &p, C Str &t) {p.color_top   =TextVec4(t); p.base.color_top   .getUTC();}
   void PanelImageEditor::SetColorBottom(Params &p, C Str &t) {p.color_bottom=TextVec4(t); p.base.color_bottom.getUTC();}
   void PanelImageEditor::SetColorLeft(Params &p, C Str &t) {p.color_left  =TextVec4(t); p.base.color_left  .getUTC();}
   void PanelImageEditor::SetColorRight(Params &p, C Str &t) {p.color_right =TextVec4(t); p.base.color_right .getUTC();}
   void PanelImageEditor::SetOuterGlowColor(Params &p, C Str &t) {p.outer_glow_color =TextVec4(t); p.base.outer_glow_color .getUTC();}
   void PanelImageEditor::SetOuterGlowRadius(Params &p, C Str &t) {p.outer_glow_radius=TextFlt (t); p.base.outer_glow_radius.getUTC();}
   void PanelImageEditor::SetOuterGlowSpread(Params &p, C Str &t) {p.outer_glow_spread=TextFlt (t); p.base.outer_glow_spread.getUTC();}
   void PanelImageEditor::SetInnerGlowColor(Params &p, C Str &t) {p.inner_glow_color =TextVec4(t); p.base.inner_glow_color .getUTC();}
   void PanelImageEditor::SetInnerGlowRadius(Params &p, C Str &t) {p.inner_glow_radius=TextFlt (t); p.base.inner_glow_radius.getUTC();}
   void PanelImageEditor::SetDepth(Params &p, C Str &t) {p.depth         =TextFlt (t); p.base.depth         .getUTC();}
   void PanelImageEditor::SetRoundDepth(Params &p, C Str &t) {p.round_depth   =TextFlt (t); p.base.round_depth   .getUTC();}
   void PanelImageEditor::SetInnerDistance(Params &p, C Str &t) {p.inner_distance=TextFlt (t); p.base.inner_distance.getUTC();}
   void PanelImageEditor::SetSmoothDepth(Params &p, C Str &t) {p.smooth_depth  =TextVec2(t); p.base.smooth_depth  .getUTC();}
   void PanelImageEditor::SetShadowRadius(Params &p, C Str &t) {p.shadow_radius =TextFlt(t); p.base.shadow_radius .getUTC();}
   void PanelImageEditor::SetShadowOpacity(Params &p, C Str &t) {p.shadow_opacity=TextFlt(t); p.base.shadow_opacity.getUTC();}
   void PanelImageEditor::SetShadowSpread(Params &p, C Str &t) {p.shadow_spread =TextFlt(t); p.base.shadow_spread .getUTC();}
   Str          PanelImageEditor::SetTopImage(C Params &p          ) {return Proj.elmFullName(p.base.top_id);}
   void         PanelImageEditor::SetTopImage(  Params &p, C Str &t) {                        p.base.top_id=Proj.findElmImageID(t); p.base.top_image.getUTC();}
   Str       PanelImageEditor::SetBottomImage(C Params &p          ) {return Proj.elmFullName(p.base.bottom_id);}
   void      PanelImageEditor::SetBottomImage(  Params &p, C Str &t) {                        p.base.bottom_id=Proj.findElmImageID(t); p.base.bottom_image.getUTC();}
   Str       PanelImageEditor::SetCenterImage(C Params &p          ) {return Proj.elmFullName(p.base.center_id);}
   void      PanelImageEditor::SetCenterImage(  Params &p, C Str &t) {                        p.base.center_id=Proj.findElmImageID(t); p.base.center_image.getUTC();}
   Str         PanelImageEditor::SetLeftImage(C Params &p          ) {return Proj.elmFullName(p.base.left_id);}
   void        PanelImageEditor::SetLeftImage(  Params &p, C Str &t) {                        p.base.left_id=Proj.findElmImageID(t); p.base.left_image.getUTC();}
   Str        PanelImageEditor::SetRightImage(C Params &p          ) {return Proj.elmFullName(p.base.right_id);}
   void       PanelImageEditor::SetRightImage(  Params &p, C Str &t) {                        p.base.right_id=Proj.findElmImageID(t); p.base.right_image.getUTC();}
   Str      PanelImageEditor::SetTopLeftImage(C Params &p          ) {return Proj.elmFullName(p.base.top_left_id);}
   void     PanelImageEditor::SetTopLeftImage(  Params &p, C Str &t) {                        p.base.top_left_id=Proj.findElmImageID(t); p.base.top_left_image.getUTC();}
   Str     PanelImageEditor::SetTopRightImage(C Params &p          ) {return Proj.elmFullName(p.base.top_right_id);}
   void    PanelImageEditor::SetTopRightImage(  Params &p, C Str &t) {                        p.base.top_right_id=Proj.findElmImageID(t); p.base.top_right_image.getUTC();}
   Str   PanelImageEditor::SetBottomLeftImage(C Params &p          ) {return Proj.elmFullName(p.base.bottom_left_id);}
   void  PanelImageEditor::SetBottomLeftImage(  Params &p, C Str &t) {                        p.base.bottom_left_id=Proj.findElmImageID(t); p.base.bottom_left_image.getUTC();}
   Str  PanelImageEditor::SetBottomRightImage(C Params &p          ) {return Proj.elmFullName(p.base.bottom_right_id);}
   void PanelImageEditor::SetBottomRightImage(  Params &p, C Str &t) {                        p.base.bottom_right_id=Proj.findElmImageID(t); p.base.bottom_right_image.getUTC();}
   void PanelImageEditor::SetImagesSize(Params &p, C Str &t) {p.       images_size  =TextFlt(t); p.base.       images_size  .getUTC();}
   void PanelImageEditor::SetTopHeight(Params &p, C Str &t) {p.          top_height=TextFlt(t); p.base.          top_height.getUTC();}
   void PanelImageEditor::SetBottomHeight(Params &p, C Str &t) {p.       bottom_height=TextFlt(t); p.base.       bottom_height.getUTC();}
   void PanelImageEditor::SetLeftRightWidth(Params &p, C Str &t) {p.   left_right_width =TextFlt(t); p.base.   left_right_width .getUTC();}
   void PanelImageEditor::SetTopCornerWidth(Params &p, C Str &t) {p.   top_corner_width =TextFlt(t); p.base.   top_corner_width .getUTC();}
   void PanelImageEditor::SetBottomCornerWidth(Params &p, C Str &t) {p.bottom_corner_width =TextFlt(t); p.base.bottom_corner_width .getUTC();}
   void PanelImageEditor::SetAmbientLight(Params &p, C Str &t) {p.light_ambient=TextFlt(t); p.base.light_ambient.getUTC();}
   void PanelImageEditor::Filter(PanelImageEditor &editor)
   {
      flt y=editor.prop_h*-0.5f; FREPA(editor.props)
      {
         Property &prop=editor.props[i];
         bool visible=(!editor.filter().is() || ContainsAll(prop.name(), editor.filter()));
         prop.visible(visible); if(visible){prop.pos(Vec2(editor.prop_x, y)); y-=editor.prop_h;}
      }
   }
   void PanelImageEditor::create()
   {
      add("Compressed"   , MemberDesc(MEMBER(Params, base.compressed)).setTextToDataFunc(SetCompressed));
      add("Mip Maps"     , MemberDesc(MEMBER(Params, base.mip_maps  )).setTextToDataFunc(SetMipMaps   ));
      add("Resolution"   , MemberDesc(MEMBER(Params, resolution     )).setTextToDataFunc(SetResolution)).range(1, 2048);
      add("Width"        , MemberDesc(MEMBER(Params, width          )).setTextToDataFunc(SetWidth     )).range(1, 8).mouseEditSpeed(1.3f);
      add("Height"       , MemberDesc(MEMBER(Params, height         )).setTextToDataFunc(SetHeight    )).range(1, 8).mouseEditSpeed(1.3f);
      add("Cutoff Left"  , MemberDesc(MEMBER(Params, cut_left       )).setTextToDataFunc(SetCutLeft   ));
      add("Cutoff Right" , MemberDesc(MEMBER(Params, cut_right      )).setTextToDataFunc(SetCutRight  ));
      add("Cutoff Top"   , MemberDesc(MEMBER(Params, cut_top        )).setTextToDataFunc(SetCutTop    ));
      add("Cutoff Bottom", MemberDesc(MEMBER(Params, cut_bottom     )).setTextToDataFunc(SetCutBottom ));
      add("Uniform Horizontal Stretching", MemberDesc(MEMBER(Params, force_uniform_stretch[0])).setTextToDataFunc(SetForceUniformStretch0));
      add("Uniform Vertical Stretching"  , MemberDesc(MEMBER(Params, force_uniform_stretch[1])).setTextToDataFunc(SetForceUniformStretch1));
      
      add();
      add("Cut Top Left Corner"    , MemberDesc(MEMBER(Params, cut_corners[0][0])).setTextToDataFunc(SetCutCorner00));
      add("Cut Top Right Corner"   , MemberDesc(MEMBER(Params, cut_corners[0][1])).setTextToDataFunc(SetCutCorner01));
      add("Cut Bottom Left Corner" , MemberDesc(MEMBER(Params, cut_corners[1][0])).setTextToDataFunc(SetCutCorner10));
      add("Cut Bottom Right Corner", MemberDesc(MEMBER(Params, cut_corners[1][1])).setTextToDataFunc(SetCutCorner11));
      
      add("Cut Corner Slope" , MemberDesc(MEMBER(Params, cut_corner_slope )).setTextToDataFunc(SetCutCornerSlope )).range(0, 2).mouseEditSpeed(0.5f);
      add("Cut Corner Amount", MemberDesc(MEMBER(Params, cut_corner_amount)).setTextToDataFunc(SetCutCornerAmount)).range(0, 1).mouseEditSpeed(0.5f);

      add("Round Corners"       , MemberDesc(MEMBER(Params, round_corners    )).setTextToDataFunc(SetRoundCorners   )).range( 0,  1).mouseEditSpeed(0.5f);
      add("Left Slope"          , MemberDesc(MEMBER(Params, left_slope       )).setTextToDataFunc(SetLeftSlope      )).range(-1,  1).mouseEditSpeed(0.5f);
      add("Right Slope"         , MemberDesc(MEMBER(Params, right_slope      )).setTextToDataFunc(SetRightSlope     )).range(-1,  1).mouseEditSpeed(0.5f);
      add("Border Size"         , MemberDesc(MEMBER(Params, border_size      )).setTextToDataFunc(SetBorderSize     )).range( 0,  1).mouseEditSpeed(0.1f);
      add("Max Side Stretch"    , MemberDesc(MEMBER(Params, max_side_stretch )).setTextToDataFunc(SetMaxSideStretch )).range( 0, 16).mouseEditSpeed(0.2f);
      add("Extend"              , MemberDesc(MEMBER(Params, extend           )).setTextToDataFunc(SetExtend         )).range( 0,  1).mouseEditSpeed(0.2f);
      add("Inner Padding Left Right", MemberDesc(DATA_REAL).setFunc(ExtendInnerPaddLR, ExtendInnerPaddLR)).range(-1,  1).mouseEditSpeed(0.1f);
      add("Inner Padding Top"       , MemberDesc(DATA_REAL).setFunc(ExtendInnerPaddT , ExtendInnerPaddT )).range(-1,  1).mouseEditSpeed(0.1f);
      add("Inner Padding Bottom"    , MemberDesc(DATA_REAL).setFunc(ExtendInnerPaddB , ExtendInnerPaddB )).range(-1,  1).mouseEditSpeed(0.1f);
      add();
      add("Depth"          , MemberDesc(MEMBER(Params, depth         )).setTextToDataFunc(SetDepth        )).range(-1, 1).mouseEditSpeed(0.2f);
      add("Round Depth"    , MemberDesc(MEMBER(Params, round_depth   )).setTextToDataFunc(SetRoundDepth   )).range( 0, 1).mouseEditSpeed(0.5f);
      add("Inner Distance" , MemberDesc(MEMBER(Params, inner_distance)).setTextToDataFunc(SetInnerDistance)).range( 0, 1).mouseEditSpeed(0.5f);
      add("Depth Smoothing", MemberDesc(MEMBER(Params, smooth_depth  )).setTextToDataFunc(SetSmoothDepth  )).range( 0, 1);
      add();
      add("Shadow Radius" , MemberDesc(MEMBER(Params, shadow_radius )).setTextToDataFunc(SetShadowRadius )).range(0, 1).mouseEditSpeed(0.5f);
      add("Shadow Opacity", MemberDesc(MEMBER(Params, shadow_opacity)).setTextToDataFunc(SetShadowOpacity)).range(0, 1);
      add("Shadow Spread" , MemberDesc(MEMBER(Params, shadow_spread )).setTextToDataFunc(SetShadowSpread )).range(0, 1);
      add();
      add("Color"       , MemberDesc(MEMBER(Params, color       )).setTextToDataFunc(SetColor      )).setColor();
      add("Color Top"   , MemberDesc(MEMBER(Params, color_top   )).setTextToDataFunc(SetColorTop   )).setColor();
      add("Color Bottom", MemberDesc(MEMBER(Params, color_bottom)).setTextToDataFunc(SetColorBottom)).setColor();
      add("Color Left"  , MemberDesc(MEMBER(Params, color_left  )).setTextToDataFunc(SetColorLeft  )).setColor();
      add("Color Right" , MemberDesc(MEMBER(Params, color_right )).setTextToDataFunc(SetColorRight )).setColor();
      add();
      add("Outer Glow Color" , MemberDesc(MEMBER(Params, outer_glow_color )).setTextToDataFunc(SetOuterGlowColor )).setColor();
      add("Outer Glow Radius", MemberDesc(MEMBER(Params, outer_glow_radius)).setTextToDataFunc(SetOuterGlowRadius)).range(0, 1).mouseEditSpeed(0.5f);
      add("Outer Glow Spread", MemberDesc(MEMBER(Params, outer_glow_spread)).setTextToDataFunc(SetOuterGlowSpread)).range(0, 1).mouseEditSpeed(0.5f);
      add();
      add("Inner Glow Color" , MemberDesc(MEMBER(Params, inner_glow_color )).setTextToDataFunc(SetInnerGlowColor )).setColor();
      add("Inner Glow Radius", MemberDesc(MEMBER(Params, inner_glow_radius)).setTextToDataFunc(SetInnerGlowRadius)).range(0, 1).mouseEditSpeed(0.5f);
      add();
      add(         "Top Image", MemberDesc(MEMBER(Params, base.         top_id)).setFunc(        SetTopImage,         SetTopImage)).elmType(ELM_IMAGE);
      add(      "Bottom Image", MemberDesc(MEMBER(Params, base.      bottom_id)).setFunc(     SetBottomImage,      SetBottomImage)).elmType(ELM_IMAGE);
      add(      "Center Image", MemberDesc(MEMBER(Params, base.      center_id)).setFunc(     SetCenterImage,      SetCenterImage)).elmType(ELM_IMAGE);
      add(        "Left Image", MemberDesc(MEMBER(Params, base.        left_id)).setFunc(       SetLeftImage,        SetLeftImage)).elmType(ELM_IMAGE);
      add(       "Right Image", MemberDesc(MEMBER(Params, base.       right_id)).setFunc(      SetRightImage,       SetRightImage)).elmType(ELM_IMAGE);
      add(    "Top Left Image", MemberDesc(MEMBER(Params, base.    top_left_id)).setFunc(    SetTopLeftImage,     SetTopLeftImage)).elmType(ELM_IMAGE);
      add(   "Top Right Image", MemberDesc(MEMBER(Params, base.   top_right_id)).setFunc(   SetTopRightImage,    SetTopRightImage)).elmType(ELM_IMAGE);
      add( "Bottom Left Image", MemberDesc(MEMBER(Params, base. bottom_left_id)).setFunc( SetBottomLeftImage,  SetBottomLeftImage)).elmType(ELM_IMAGE);
      add("Bottom Right Image", MemberDesc(MEMBER(Params, base.bottom_right_id)).setFunc(SetBottomRightImage, SetBottomRightImage)).elmType(ELM_IMAGE);
      add(         "Images Size", MemberDesc(MEMBER(Params,         images_size)).setTextToDataFunc(SetImagesSize       )).range(0, 1024);
      add(          "Top Height", MemberDesc(MEMBER(Params,          top_height)).setTextToDataFunc(SetTopHeight        )).range(0, 1024);
      add(       "Bottom Height", MemberDesc(MEMBER(Params,       bottom_height)).setTextToDataFunc(SetBottomHeight     )).range(0, 1024);
      add(    "Left Right Width", MemberDesc(MEMBER(Params,    left_right_width)).setTextToDataFunc(SetLeftRightWidth   )).range(0, 1024);
      add(   "Top Corners Width", MemberDesc(MEMBER(Params,    top_corner_width)).setTextToDataFunc(SetTopCornerWidth   )).range(0, 1024);
      add("Bottom Corners Width", MemberDesc(MEMBER(Params, bottom_corner_width)).setTextToDataFunc(SetBottomCornerWidth)).range(0, 1024);
      add();
      add("Ambient Light", MemberDesc(MEMBER(Params, light_ambient)).setTextToDataFunc(SetAmbientLight)).range(0, 1).mouseEditSpeed(0.2f);
      add();
      add("Light 0:");
      add("Enabled"                  , MemberDesc(MEMBER(Params, lights[0].enabled               )).setTextToDataFunc(LightEnabled             <0>));
      add("Angle"                    , MemberDesc(MEMBER(Params, lights[0].angle                 )).setTextToDataFunc(LightAngle               <0>)).range(-1, 1);
      add("Intensity"                , MemberDesc(MEMBER(Params, lights[0].intensity             )).setTextToDataFunc(LightIntensity           <0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Back Intensity"           , MemberDesc(MEMBER(Params, lights[0].back                  )).setTextToDataFunc(LightBack                <0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Highlight"                , MemberDesc(MEMBER(Params, lights[0].highlight             )).setTextToDataFunc(LightHighlight           <0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Highlight Cutoff"         , MemberDesc(MEMBER(Params, lights[0].highlight_cut         )).setTextToDataFunc(LightHighlightCut        <0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Specular Intensity"       , MemberDesc(MEMBER(Params, lights[0].specular              )).setTextToDataFunc(LightSpecular            <0>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Specular Back Intensity"  , MemberDesc(MEMBER(Params, lights[0].specular_back         )).setTextToDataFunc(LightSpecularBack        <0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Specular Exponent"        , MemberDesc(MEMBER(Params, lights[0].specular_exp          )).setTextToDataFunc(LightSpecularExp         <0>)).range( 0, 256).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Specular Highlight"       , MemberDesc(MEMBER(Params, lights[0].specular_highlight    )).setTextToDataFunc(LightSpecularHighlight   <0>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Specular Highlight Cutoff", MemberDesc(MEMBER(Params, lights[0].specular_highlight_cut)).setTextToDataFunc(LightSpecularHighlightCut<0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add();
      add("Light 1:");
      add("Enabled"                  , MemberDesc(MEMBER(Params, lights[1].enabled               )).setTextToDataFunc(LightEnabled             <1>));
      add("Angle"                    , MemberDesc(MEMBER(Params, lights[1].angle                 )).setTextToDataFunc(LightAngle               <1>)).range(-1, 1);
      add("Intensity"                , MemberDesc(MEMBER(Params, lights[1].intensity             )).setTextToDataFunc(LightIntensity           <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Back Intensity"           , MemberDesc(MEMBER(Params, lights[1].back                  )).setTextToDataFunc(LightBack                <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Highlight"                , MemberDesc(MEMBER(Params, lights[1].highlight             )).setTextToDataFunc(LightHighlight           <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Highlight Cutoff"         , MemberDesc(MEMBER(Params, lights[1].highlight_cut         )).setTextToDataFunc(LightHighlightCut        <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Specular Intensity"       , MemberDesc(MEMBER(Params, lights[1].specular              )).setTextToDataFunc(LightSpecular            <1>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Specular Back Intensity"  , MemberDesc(MEMBER(Params, lights[1].specular_back         )).setTextToDataFunc(LightSpecularBack        <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Specular Exponent"        , MemberDesc(MEMBER(Params, lights[1].specular_exp          )).setTextToDataFunc(LightSpecularExp         <1>)).range( 0, 256).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Specular Highlight"       , MemberDesc(MEMBER(Params, lights[1].specular_highlight    )).setTextToDataFunc(LightSpecularHighlight   <1>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Specular Highlight Cutoff", MemberDesc(MEMBER(Params, lights[1].specular_highlight_cut)).setTextToDataFunc(LightSpecularHighlightCut<1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add();
      add("Section 0:");
      add("Round Depth"       , MemberDesc(MEMBER(Params, sections[0].round_depth       )).setTextToDataFunc(SectionRoundDepth      <0>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Outer Depth"       , MemberDesc(MEMBER(Params, sections[0].outer_depth       )).setTextToDataFunc(SectionOuterDepth      <0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Inner Depth"       , MemberDesc(MEMBER(Params, sections[0].inner_depth       )).setTextToDataFunc(SectionInnerDepth      <0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Inner Distance"    , MemberDesc(MEMBER(Params, sections[0].inner_distance    )).setTextToDataFunc(SectionInnerDistance   <0>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Depth Smoothing"   , MemberDesc(MEMBER(Params, sections[0].smooth_depth      )).setTextToDataFunc(SectionSmoothDepth     <0>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Specular"          , MemberDesc(MEMBER(Params, sections[0].specular          )).setTextToDataFunc(SectionSpecular        <0>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Color"             , MemberDesc(MEMBER(Params, sections[0].color             )).setTextToDataFunc(SectionColor           <0>)).setColor();
      add("Color Outer"       , MemberDesc(MEMBER(Params, sections[0].outer_color       )).setTextToDataFunc(SectionOuterColor      <0>)).setColor();
      add("Color Inner"       , MemberDesc(MEMBER(Params, sections[0].inner_color       )).setTextToDataFunc(SectionInnerColor      <0>)).setColor();
      add("Color Top"         , MemberDesc(MEMBER(Params, sections[0].color_top         )).setTextToDataFunc(SectionColorTop        <0>)).setColor();
      add("Color Bottom"      , MemberDesc(MEMBER(Params, sections[0].color_bottom      )).setTextToDataFunc(SectionColorBottom     <0>)).setColor();
      add("Color Left"        , MemberDesc(MEMBER(Params, sections[0].color_left        )).setTextToDataFunc(SectionColorLeft       <0>)).setColor();
      add("Color Right"       , MemberDesc(MEMBER(Params, sections[0].color_right       )).setTextToDataFunc(SectionColorRight      <0>)).setColor();
      add("Outer Border Color", MemberDesc(MEMBER(Params, sections[0].outer_border_color)).setTextToDataFunc(SectionOuterBorderColor<0>)).setColor();
      add("Inner Border Color", MemberDesc(MEMBER(Params, sections[0].inner_border_color)).setTextToDataFunc(SectionInnerBorderColor<0>)).setColor();

      add("Depth Overlay Image", MemberDesc(MEMBER(Params, base.sections[0].depth_overlay_id)).setFunc(SectionDepthOverlay<0>, SectionDepthOverlay<0>)).elmType(ELM_IMAGE);
      add("Depth Overlay Blur"      , MemberDesc(MEMBER(Params, sections[0].depth_overlay_params.blur      )).setTextToDataFunc(SectionDepthOverlayBlur     <0>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Depth Overlay Blur Clamp", MemberDesc(MEMBER(Params, sections[0].depth_overlay_params.blur_clamp)).setTextToDataFunc(SectionDepthOverlayBlurClamp<0>));
      add("Depth Overlay UV Scale"  , MemberDesc(MEMBER(Params, sections[0].depth_overlay_params.uv_scale  )).setTextToDataFunc(SectionDepthOverlayUVScale  <0>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Depth Overlay UV Offset" , MemberDesc(MEMBER(Params, sections[0].depth_overlay_params.uv_offset )).setTextToDataFunc(SectionDepthOverlayUVOffset <0>)).range( -1,  1).mouseEditSpeed(0.2f);
      add("Depth Overlay Intensity" , MemberDesc(MEMBER(Params, sections[0].depth_overlay_params.intensity )).setTextToDataFunc(SectionDepthOverlayIntensity<0>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Depth Overlay Mode"      , MemberDesc(MEMBER(Params, sections[0].depth_overlay_params.mode      )).setTextToDataFunc(SectionDepthOverlayMode     <0>)).setEnum(image_modes, Elms(image_modes));

      add("Depth Noise Blur"     , MemberDesc(MEMBER(Params, sections[0].depth_noise.blur     )).setTextToDataFunc(SectionDepthNoiseBlur     <0>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Depth Noise UV Scale" , MemberDesc(MEMBER(Params, sections[0].depth_noise.uv_scale )).setTextToDataFunc(SectionDepthNoiseUVScale  <0>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Depth Noise Intensity", MemberDesc(MEMBER(Params, sections[0].depth_noise.intensity)).setTextToDataFunc(SectionDepthNoiseIntensity<0>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Depth Noise Mode"     , MemberDesc(MEMBER(Params, sections[0].depth_noise.mode     )).setTextToDataFunc(SectionDepthNoiseMode     <0>)).setEnum(image_modes, Elms(image_modes));

      add("Color Overlay Image", MemberDesc(MEMBER(Params, base.sections[0].color_overlay_id)).setFunc(SectionColorOverlay<0>, SectionColorOverlay<0>)).elmType(ELM_IMAGE);
      add("Color Overlay Blur"      , MemberDesc(MEMBER(Params, sections[0].color_overlay_params.blur      )).setTextToDataFunc(SectionColorOverlayBlur     <0>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Color Overlay Blur Clamp", MemberDesc(MEMBER(Params, sections[0].color_overlay_params.blur_clamp)).setTextToDataFunc(SectionColorOverlayBlurClamp<0>));
      add("Color Overlay UV Scale"  , MemberDesc(MEMBER(Params, sections[0].color_overlay_params.uv_scale  )).setTextToDataFunc(SectionColorOverlayUVScale  <0>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Color Overlay UV Offset" , MemberDesc(MEMBER(Params, sections[0].color_overlay_params.uv_offset )).setTextToDataFunc(SectionColorOverlayUVOffset <0>)).range( -1,  1).mouseEditSpeed(0.2f);
      add("Color Overlay UV Warp"   , MemberDesc(MEMBER(Params, sections[0].color_overlay_params.uv_warp   )).setTextToDataFunc(SectionColorOverlayUVWarp   <0>)).range( -1,  1).mouseEditSpeed(0.5f);
      add("Color Overlay Intensity" , MemberDesc(MEMBER(Params, sections[0].color_overlay_params.intensity )).setTextToDataFunc(SectionColorOverlayIntensity<0>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Color Overlay Mode"      , MemberDesc(MEMBER(Params, sections[0].color_overlay_params.mode      )).setTextToDataFunc(SectionColorOverlayMode     <0>)).setEnum(image_modes, Elms(image_modes));

      add("Color Noise Blur"     , MemberDesc(MEMBER(Params, sections[0].color_noise.blur     )).setTextToDataFunc(SectionColorNoiseBlur     <0>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Color Noise UV Scale" , MemberDesc(MEMBER(Params, sections[0].color_noise.uv_scale )).setTextToDataFunc(SectionColorNoiseUVScale  <0>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Color Noise UV Warp"  , MemberDesc(MEMBER(Params, sections[0].color_noise.uv_warp  )).setTextToDataFunc(SectionColorNoiseUVWarp   <0>)).range( -1,  1).mouseEditSpeed(0.5f);
      add("Color Noise Intensity", MemberDesc(MEMBER(Params, sections[0].color_noise.intensity)).setTextToDataFunc(SectionColorNoiseIntensity<0>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Color Noise Mode"     , MemberDesc(MEMBER(Params, sections[0].color_noise.mode     )).setTextToDataFunc(SectionColorNoiseMode     <0>)).setEnum(image_modes, Elms(image_modes));

      add("Reflection Image", MemberDesc(MEMBER(Params, base.sections[0].reflection_id)).setFunc(SectionReflection<0>, SectionReflection<0>)).elmType(ELM_IMAGE);
      add("Reflection Intensity", MemberDesc(MEMBER(Params, sections[0].reflection_intensity)).setTextToDataFunc(SectionReflectionIntensity<0>)).range(-1, 1).mouseEditSpeed(0.2f);
      add();
      add("Section 1:");
      add("Size"              , MemberDesc(MEMBER(Params, sections[1].size              )).setTextToDataFunc(SectionSize            <1>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Top Offset"        , MemberDesc(MEMBER(Params, sections[1].top_offset        )).setTextToDataFunc(SectionTopOffset       <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Round Depth"       , MemberDesc(MEMBER(Params, sections[1].round_depth       )).setTextToDataFunc(SectionRoundDepth      <1>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Center Depth"      , MemberDesc(MEMBER(Params, sections[1].outer_depth       )).setTextToDataFunc(SectionOuterDepth      <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Center Distance"   , MemberDesc(MEMBER(Params, sections[1].inner_distance    )).setTextToDataFunc(SectionInnerDistance   <1>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Inner Depth"       , MemberDesc(MEMBER(Params, sections[1].inner_depth       )).setTextToDataFunc(SectionInnerDepth      <1>)).range(-1, 1).mouseEditSpeed(0.2f);
      add("Depth Smoothing"   , MemberDesc(MEMBER(Params, sections[1].smooth_depth      )).setTextToDataFunc(SectionSmoothDepth     <1>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Specular"          , MemberDesc(MEMBER(Params, sections[1].specular          )).setTextToDataFunc(SectionSpecular        <1>)).range( 0, 1).mouseEditSpeed(0.2f);
      add("Color"             , MemberDesc(MEMBER(Params, sections[1].color             )).setTextToDataFunc(SectionColor           <1>)).setColor();
      add("Color Outer"       , MemberDesc(MEMBER(Params, sections[1].outer_color       )).setTextToDataFunc(SectionOuterColor      <1>)).setColor();
      add("Color Inner"       , MemberDesc(MEMBER(Params, sections[1].inner_color       )).setTextToDataFunc(SectionInnerColor      <1>)).setColor();
      add("Color Top"         , MemberDesc(MEMBER(Params, sections[1].color_top         )).setTextToDataFunc(SectionColorTop        <1>)).setColor();
      add("Color Bottom"      , MemberDesc(MEMBER(Params, sections[1].color_bottom      )).setTextToDataFunc(SectionColorBottom     <1>)).setColor();
      add("Color Left"        , MemberDesc(MEMBER(Params, sections[1].color_left        )).setTextToDataFunc(SectionColorLeft       <1>)).setColor();
      add("Color Right"       , MemberDesc(MEMBER(Params, sections[1].color_right       )).setTextToDataFunc(SectionColorRight      <1>)).setColor();
      add("Outer Border Color", MemberDesc(MEMBER(Params, sections[1].outer_border_color)).setTextToDataFunc(SectionOuterBorderColor<1>)).setColor();
      add("Inner Border Color", MemberDesc(MEMBER(Params, sections[1].inner_border_color)).setTextToDataFunc(SectionInnerBorderColor<1>)).setColor();
      add( "Prev Border Color", MemberDesc(MEMBER(Params, sections[1]. prev_border_color)).setTextToDataFunc(SectionPrevBorderColor <1>)).setColor();

      add("Depth Overlay Image", MemberDesc(MEMBER(Params, base.sections[1].depth_overlay_id)).setFunc(SectionDepthOverlay<1>, SectionDepthOverlay<1>)).elmType(ELM_IMAGE);
      add("Depth Overlay Blur"      , MemberDesc(MEMBER(Params, sections[1].depth_overlay_params.blur      )).setTextToDataFunc(SectionDepthOverlayBlur     <1>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Depth Overlay Blur Clamp", MemberDesc(MEMBER(Params, sections[1].depth_overlay_params.blur_clamp)).setTextToDataFunc(SectionDepthOverlayBlurClamp<1>));
      add("Depth Overlay UV Scale"  , MemberDesc(MEMBER(Params, sections[1].depth_overlay_params.uv_scale  )).setTextToDataFunc(SectionDepthOverlayUVScale  <1>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Depth Overlay UV Offset" , MemberDesc(MEMBER(Params, sections[1].depth_overlay_params.uv_offset )).setTextToDataFunc(SectionDepthOverlayUVOffset <1>)).range( -1,  1).mouseEditSpeed(0.2f);
      add("Depth Overlay Intensity" , MemberDesc(MEMBER(Params, sections[1].depth_overlay_params.intensity )).setTextToDataFunc(SectionDepthOverlayIntensity<1>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Depth Overlay Mode"      , MemberDesc(MEMBER(Params, sections[1].depth_overlay_params.mode      )).setTextToDataFunc(SectionDepthOverlayMode     <1>)).setEnum(image_modes, Elms(image_modes));

      add("Depth Noise Blur"     , MemberDesc(MEMBER(Params, sections[1].depth_noise.blur     )).setTextToDataFunc(SectionDepthNoiseBlur     <1>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Depth Noise UV Scale" , MemberDesc(MEMBER(Params, sections[1].depth_noise.uv_scale )).setTextToDataFunc(SectionDepthNoiseUVScale  <1>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Depth Noise Intensity", MemberDesc(MEMBER(Params, sections[1].depth_noise.intensity)).setTextToDataFunc(SectionDepthNoiseIntensity<1>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Depth Noise Mode"     , MemberDesc(MEMBER(Params, sections[1].depth_noise.mode     )).setTextToDataFunc(SectionDepthNoiseMode     <1>)).setEnum(image_modes, Elms(image_modes));

      add("Color Overlay Image", MemberDesc(MEMBER(Params, base.sections[1].color_overlay_id)).setFunc(SectionColorOverlay<1>, SectionColorOverlay<1>)).elmType(ELM_IMAGE);
      add("Color Overlay Blur"      , MemberDesc(MEMBER(Params, sections[1].color_overlay_params.blur      )).setTextToDataFunc(SectionColorOverlayBlur     <1>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Color Overlay Blur Clamp", MemberDesc(MEMBER(Params, sections[1].color_overlay_params.blur_clamp)).setTextToDataFunc(SectionColorOverlayBlurClamp<1>));
      add("Color Overlay UV Scale"  , MemberDesc(MEMBER(Params, sections[1].color_overlay_params.uv_scale  )).setTextToDataFunc(SectionColorOverlayUVScale  <1>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Color Overlay UV Offset" , MemberDesc(MEMBER(Params, sections[1].color_overlay_params.uv_offset )).setTextToDataFunc(SectionColorOverlayUVOffset <1>)).range( -1,  1).mouseEditSpeed(0.2f);
      add("Color Overlay UV Warp"   , MemberDesc(MEMBER(Params, sections[1].color_overlay_params.uv_warp   )).setTextToDataFunc(SectionColorOverlayUVWarp   <1>)).range( -1,  1).mouseEditSpeed(0.5f);
      add("Color Overlay Intensity" , MemberDesc(MEMBER(Params, sections[1].color_overlay_params.intensity )).setTextToDataFunc(SectionColorOverlayIntensity<1>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Color Overlay Mode"      , MemberDesc(MEMBER(Params, sections[1].color_overlay_params.mode      )).setTextToDataFunc(SectionColorOverlayMode     <1>)).setEnum(image_modes, Elms(image_modes));

      add("Color Noise Blur"     , MemberDesc(MEMBER(Params, sections[1].color_noise.blur     )).setTextToDataFunc(SectionColorNoiseBlur     <1>)).range(  0, 64).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).mouseEditSpeed(1);
      add("Color Noise UV Scale" , MemberDesc(MEMBER(Params, sections[1].color_noise.uv_scale )).setTextToDataFunc(SectionColorNoiseUVScale  <1>)).range(-16, 16).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      add("Color Noise UV Warp"  , MemberDesc(MEMBER(Params, sections[1].color_noise.uv_warp  )).setTextToDataFunc(SectionColorNoiseUVWarp   <1>)).range( -1,  1).mouseEditSpeed(0.5f);
      add("Color Noise Intensity", MemberDesc(MEMBER(Params, sections[1].color_noise.intensity)).setTextToDataFunc(SectionColorNoiseIntensity<1>)).range( -2,  2).mouseEditSpeed(0.2f);
      add("Color Noise Mode"     , MemberDesc(MEMBER(Params, sections[1].color_noise.mode     )).setTextToDataFunc(SectionColorNoiseMode     <1>)).setEnum(image_modes, Elms(image_modes));

      add("Reflection Image", MemberDesc(MEMBER(Params, base.sections[1].reflection_id)).setFunc(SectionReflection<1>, SectionReflection<1>)).elmType(ELM_IMAGE);
      add("Reflection Intensity", MemberDesc(MEMBER(Params, sections[1].reflection_intensity)).setTextToDataFunc(SectionReflectionIntensity<1>)).range(-1, 1).mouseEditSpeed(0.2f);
      autoData(&params);

      props_w=::PropWin::create("Panel Image Editor", Vec2(prop_x, 0), 0.036f, prop_h, 0.3f).max.x+0.02f; button[1].show(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
      T+=undo  .create(Rect_LU(0.02f, -0.01f     , 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
      T+=redo  .create(Rect_LU(undo  .rect().ru(), 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      T+=locate.create(Rect_LU(redo  .rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
      T+=copy  .create(Rect_LU(locate.rect().ru()+Vec2(0.01f, 0), 0.11f, 0.05f), "Copy"  ).func(Copy  , T).focusable(false).desc("Copy values to system clipboard");
      T+=paste .create(Rect_LU(copy  .rect().ru()+Vec2(0.01f, 0), 0.12f, 0.05f), "Paste" ).func(Paste , T).focusable(false).desc("Paste values from system clipboard");
      T+=filter.create(Rect_LU(paste .rect().ru()+Vec2(0.01f, 0), 0.28f, 0.05f)).func(Filter, T).desc("Find property by name"); filter.reset.show(); filter.show_find=true; filter.hint="Find Property";
      T+=region.create();
      FREPA(props){Property &prop=props[i]; prop.parent(region).changed(Changed, PreChanged);}
      cchar8 *modes[]={"Image", "Image + Scaled", "4 x Scaled"};
      T+=preview_mode.create(modes, Elms(modes)).valid(true).set(1);
      T+=draw_depth.create("Show Depth"); draw_depth.mode=BUTTON_TOGGLE;
      T+=draw_lines.create("Show Lines"); draw_lines.mode=BUTTON_TOGGLE;
      T+=viewport.create(Preview, this);
      rect(Rect_C(0, 0, 2.2f, 1.5f));
   }
   void PanelImageEditor::toGui() {::PropWin::toGui(); refresh();}
   void PanelImageEditor::stopThread() {thread.stop(); event.on();}
   void PanelImageEditor::refresh() {if(elm){event.on(); if(!thread.active() || thread.wantStop())thread.create(Make, this);}}
   PanelImageEditor& PanelImageEditor::del()  {stopThread(); thread.del(); ::EE::Window::del (); return T;}
   PanelImageEditor& PanelImageEditor::hide()  {stopThread(); set(null   ); ::PropWin::hide(); return T;}
   Rect         PanelImageEditor::sizeLimit()C {Rect r=::EE::Window::sizeLimit(); r.min.set(0.3f, 0.2f); return r;}
   void PanelImageEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmPanelImage *data=elm->panelImageData()){data->newVer(); data->from(params);} // modify just before saving/sending in case we've received data from server after edit
         Save(params, Proj.basePath(*elm));
         PanelImage panel_image; params.make(panel_image, WorkerThreads, Proj); Save(panel_image, Proj.gamePath(*elm)); Proj.savedGame(*elm);
         Server.setElmLong(elm->id);
         Proj.panelImageChanged(elm->id);
      }
      changed=false;
   }
   void PanelImageEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmPanelImage *data=elm->panelImageData()){data->newVer(); data->from(params);}
         refresh();
      }
   }
   void PanelImageEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_PANEL_IMAGE)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         {
            SyncLocker locker(lock);
            if(elm){params.load(Proj.basePath(*elm)); panel_image.load(Proj.gamePath(*elm));}else{params.reset(); panel_image.del();}
         }
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void PanelImageEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void PanelImageEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void PanelImageEditor::elmChanged(C UID &panel_image_id)
   {
      if(elm && elm->id==panel_image_id)
      {
         undos.set(null, true);
         EditPanelImage temp; if(temp.load(Proj.basePath(*elm)))if(params.sync(temp))toGui();
      }
   }
   void PanelImageEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
PanelImageEditor::PanelImageEditor() : elm_id(UIDZero), elm(null), changed(false), props_w(0), prop_h(0.043f), prop_x(0.01f), force_undo_change_type(null), undos(true) {}

/******************************************************************************/
