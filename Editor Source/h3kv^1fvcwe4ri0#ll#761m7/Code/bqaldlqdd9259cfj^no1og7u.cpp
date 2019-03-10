/******************************************************************************/
class TextureDownsize : Viewport4Region
{
   enum MODE
   {
      MATERIAL,
      OBJECT  ,
      MODE_NUM,
   }
   static cchar8 *ModeName[]=
   {
      "Material",
      "Object"  ,
   };
   class Settings
   {
      int global=0,
          base_0=0,
          base_1=0;

      int base0()C {return global+base_0;}
      int base1()C {return global+base_1;}
   }
   class DownsizedMtrl : Material
   {
      MaterialPtr original;

      static bool Create(DownsizedMtrl &mtrl, C UID &original_id, ptr user) {mtrl.create(original_id); return true;}
             void create(                     C UID &original_id          )
      {
         if(original=Proj.gamePath(original_id))SCAST(Material, T)=*original; // copy everything
         setTextures();
      }
      void setTextures()
      {
         if(original)
         {
            base_0=((TexDownsize.normal_base[0].valid() && TexDownsize.normal_base[0]==original->base_0.id() && TexDownsize.downsized_base[0].is()) ? ImagePtr(&TexDownsize.downsized_base[0]) : original->base_0);
            base_1=((TexDownsize.normal_base[1].valid() && TexDownsize.normal_base[1]==original->base_1.id() && TexDownsize.downsized_base[1].is()) ? ImagePtr(&TexDownsize.downsized_base[1]) : original->base_1);
         }
      }
      void update()
      {
         // update all parameters from 'original' but keep changed textures
         if(original)
         {
            ImagePtr base0, base1;
            Swap(base0, T.base_0);
            Swap(base1, T.base_1);
            SCAST(Material, T)=*original;
            Swap(base0, T.base_0);
            Swap(base1, T.base_1);
         }
      }
   }

   Menu                    menu;
   Tabs                    mode;
   GuiImage                mtrl_image;
   Region                  region;
   Button                  light_dir, downsize[3][2], revert, apply, prev, next;
   Text                    info[3][2];
   Memx<Property>          props;
   TextBlack               prop_ts, left_ts, right_ts;
   Map<UID, Settings>      settings(Compare); // key=Material ID, doesn't need to be thread-safe
   Elm                    *elm=null; // can be OBJ or MTRL
   UID                     mtrl_id=UIDZero,
                            obj_id=UIDZero;
   Memc<UID>               secondary;
   int                     secondary_i=0;
   UID                     normal_base[2];
   MaterialPtr             normal_mtrl;
   MeshPtr                 normal_mesh;
   Image                   downsized_base[2];
   Map<UID, DownsizedMtrl> downsized_mtrls(Compare, DownsizedMtrl.Create); // key=Material ID, doesn't need to be thread-safe
   MeshLod                 downsized_mesh;
   bool                    draw_as_obj=true, different=false;
   Vec2                    light_angle=PI_4;

   Vec previewLight()C {return Matrix3().setRotateXY(light_angle.y-ActiveCam.pitch, light_angle.x-ActiveCam.yaw).z;}

   static int MaterialUses(C MeshLod &lod, C MaterialPtr &mtrl, int variation) // how many 'lod' parts use 'mtrl' in 'variation'
   {
      int    uses=0; REPA(lod)if(lod.parts[i].variation(variation)==mtrl)uses++;
      return uses;
   }
   static int VariationWithMostMaterialUses(C Mesh &mesh, C MaterialPtr &mtrl)
   {
      int    best=-1, best_uses=0; FREP(mesh.variations()){int uses=MaterialUses(mesh, mtrl, i); if(best<0 || uses>best_uses){best=i; best_uses=uses;}} // go from start to prefer the default variation
      return best;
   }

   static void Render() {TexDownsize.render();}
          void render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            bool interval=(Trunc(Time.appTime()*3.0)&1);
            if(draw_as_obj) // draw as object
            {
               if(normal_mesh)
               {
                  SetVariation(VariationWithMostMaterialUses(*normal_mesh, normal_mtrl)); (interval ? downsized_mesh : SCAST(MeshLod, *normal_mesh)).draw(MatrixIdentity);
                  SetVariation();
               }
               LightDir(light_dir() ? !Vec(-1, -1, -1) : ActiveCam.matrix.z, 1-D.ambientColor()).add(false);
            }else
            if(normal_mtrl) // draw as material
            {
               Preview.ball.material(interval ? MaterialPtr(downsized_mtrls(mtrl_id)) : normal_mtrl).setShader();
               Preview.ball.draw(ActiveCam.matrix.pos+ActiveCam.matrix.z*(1.1+ActiveCam.dist));

               LightDir(previewLight(), 1-D.ambientColor()).add(false);
            }
         }break;
      }
   }

   static void Draw(Viewport &viewport) {if(Edit.Viewport4.View *view=TexDownsize.v4.getView(&viewport))TexDownsize.draw(*view);}
          void draw(Edit.Viewport4.View &view)
   {
      // environment
      EnvironmentPtr env=EnvEdit.cur(); if(!env)env=&DefaultEnvironment; env->set();

      DOF_MODE    dof      =D.      dofMode(); D.      dofMode(   DOF_NONE);
      MOTION_MODE motion   =D.   motionMode(); D.   motionMode(MOTION_NONE);
      bool        eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(      false);
      bool        astros   =AstrosDraw       ; AstrosDraw     =false;
      bool        ocean    =Water.draw       ; Water.draw     =false;
      Camera      temp=ActiveCam; view.camera.set();

      Renderer(Render);

      temp.set();
      D.      dofMode(dof      );
      D.   motionMode(motion   );
      D.eyeAdaptation(eye_adapt);
      AstrosDraw     =astros;
      Water.draw     =ocean;

      if(!elm)D.text(D.viewRect(), "No Material/Object Selected\nLeft-click on a Project Element");
   }

   static void ModeChanged(TextureDownsize &editor) {editor.setDrawAs();}
   static void LightMode  (TextureDownsize &editor) {editor.light_dir.push();}
   
   static void Mode0(TextureDownsize &td) {td.mode.set(0);}
   static void Mode1(TextureDownsize &td) {td.mode.set(1);}

   static Str  Global(C TextureDownsize &td             ) {if(C Settings *s=td.curSettings())return s.global; return S;}
   static void Global(  TextureDownsize &td, C Str &text) {if(  Settings *s=td.curSettings())       s.global=TextInt(text);}

   static Str  Base0(C TextureDownsize &td             ) {if(C Settings *s=td.curSettings())return s.base_0; return S;}
   static void Base0(  TextureDownsize &td, C Str &text) {if(  Settings *s=td.curSettings())       s.base_0=TextInt(text);}

   static Str  Base1(C TextureDownsize &td             ) {if(C Settings *s=td.curSettings())return s.base_1; return S;}
   static void Base1(  TextureDownsize &td, C Str &text) {if(  Settings *s=td.curSettings())       s.base_1=TextInt(text);}

   static void Prev(TextureDownsize &editor) {editor.Next(-1);}
   static void Next(TextureDownsize &editor) {editor.Next(+1);}

   static void PrevS(TextureDownsize &editor) {editor.nextSecondary(-1);}
   static void NextS(TextureDownsize &editor) {editor.nextSecondary(+1);}

   void nextSecondary(int dir)
   {
      if(elm && elm.type==ELM_MTRL)setSecondary(secondary_i+dir);else Next(dir);
   }
   bool nextSecondary(int dir, C UID (&old_base)[2])
   {
      if(elm && elm.type==ELM_OBJ)
         for(; InRange(secondary_i+dir, secondary); )
      {
         setSecondary(secondary_i+dir);
         if(normal_base[0]!=old_base[0] || normal_base[1]!=old_base[1])return true;
      }
      return false;
   }
   bool Next(int dir=1) // find next compatible element
   {
      setTexIDs(); // always get latest texture ID's in case they were changed in between
      UID old_base[]={normal_base[0], normal_base[1]};
      if(nextSecondary(dir, old_base))return true; // iterate through OBJ materials first
      int c=Proj.list.cur; REP(Proj.list.elms())
      {
         c=Mod(c+dir, Proj.list.elms());
         if(Elm *elm=Proj.list.visToElm(c))if(elm.type==ELM_MTRL || elm.type==ELM_OBJ)
         {
            setElm(elm, dir<0); // set secondary as first/last based on 'dir'
            if(normal_base[0]!=old_base[0] || normal_base[1]!=old_base[1]
            || nextSecondary(dir, old_base)){Proj.list.setCur(c).scrollTo(c, false, 0.5); return true;}
         }
      }
      return false;
   }

   static cchar8 *DialogID="TextureDownsize.apply";

   void closeConfirm() {Gui.closeMsgBox(DialogID);}

   static bool AdjustTexSrc(Str &src, TimeStamp &src_time, C Str &original, C Str &downsized, C TimeStamp &time)
   {
      if(src==original && src!=downsized){src=downsized; src_time=time; return true;}
      return false;
   }
   static VecI2 ImageSize(C ImagePtr &image, int down)
   {
      if(image && image->is())return VecI2(Max(1, Shr(image->w(), down)), Max(1, Shr(image->h(), down)));
      return 0;
   }
   static void ApplyConfirm(TextureDownsize &editor) {editor.applyConfirm();}
          void applyConfirm(bool revert=false)
   {
      bool upscaled=false;
      closeConfirm();
      if(Settings *settings=curSettings())
      {
         bool relative=revert;
         int  base0=settings.base0(),
              base1=settings.base1();
         if(revert)base0=base1=0;
         if( base0 || base1 || relative) // if we want any change
         {
            EditMaterial original; if(!normal_mtrl || !Proj.mtrlGet(mtrl_id, original)){Gui.msgBox(S, "Can't access Material"); return;}
            if(!original.base_0_tex.valid())base0=0; // if texture is not present, then we can't modify it
            if(!original.base_1_tex.valid())base1=0; // if texture is not present, then we can't modify it
            if(base0 || base1 || relative) // if we want any change
               if(original.base_0_tex.valid() || original.base_1_tex.valid()) // and we have any texture
            {
               // check if all texture source files are present
               Str invalid;
               if(Proj.invalidTexSrc(original.   color_map, &invalid)
               || Proj.invalidTexSrc(original.   alpha_map, &invalid)
               || Proj.invalidTexSrc(original.    bump_map, &invalid)
               || Proj.invalidTexSrc(original.  normal_map, &invalid)
               || Proj.invalidTexSrc(original.specular_map, &invalid)
               || Proj.invalidTexSrc(original.    glow_map, &invalid)){Gui.msgBox(S, S+"Can't find:\n"+invalid.replace(' ', Nbsp)); return;}

               EditMaterial downsized=original;
               TimeStamp    time; time.getUTC();

               VecI2 base0_size, base1_size;
               if(relative)
               {
                  base0_size=base0;
                  base1_size=base1;
               }else
               {
                  base0_size=ImageSize(normal_mtrl->base_0, base0);
                  base1_size=ImageSize(normal_mtrl->base_1, base1);
               }
               if(base0_size!=base1_size)downsized.separateBaseTexs(Proj, time);
               VecI2 alpha_size=(downsized.hasBase1Tex() ? base1_size : base0_size); // alpha can go into Base0/Base1, #MaterialTextureChannelOrder

               // resize images, #MaterialTextureChannelOrder
               Proj.forceImageSize(downsized.   color_map, base0_size, relative, downsized.   color_map_time, time);
               Proj.forceImageSize(downsized.   alpha_map, alpha_size, relative, downsized.   alpha_map_time, time);
               Proj.forceImageSize(downsized.    bump_map, base0_size, relative, downsized.    bump_map_time, time);
               Proj.forceImageSize(downsized.  normal_map, base1_size, relative, downsized.  normal_map_time, time);
               Proj.forceImageSize(downsized.specular_map, base1_size, relative, downsized.specular_map_time, time);
               Proj.forceImageSize(downsized.    glow_map, base1_size, relative, downsized.    glow_map_time, time);
               downsized.cleanupMaps();

               if(Proj.mtrlSync(mtrl_id, downsized, true, false))
               if(Proj.mtrlGet (mtrl_id, downsized)) // get after syncing, because of reload base textures, we should now have new Tex ID's
               {
                  // check which textures have been changed
                  bool changed_base0=(original.base_0_tex!=downsized.base_0_tex),
                       changed_base1=(original.base_1_tex!=downsized.base_1_tex);
                  upscaled=((changed_base0 && base0<0)
                         || (changed_base1 && base1<0));
                  // process all other materials, if they share the same original tex ID, then adjust their sources as well
                  EditMaterial test;
                  REPA(Proj.elms)
                  {
                   C Elm &elm=Proj.elms[i];
                     if(C ElmMaterial *mtrl_data=elm.mtrlData())if(elm.id!=mtrl_id)
                        if(changed_base0 && mtrl_data.base_0_tex==original.base_0_tex  // first check if we're changing this texture, then compare if it's the same
                        || changed_base1 && mtrl_data.base_1_tex==original.base_1_tex) // first check if we're changing this texture, then compare if it's the same
                           if(Proj.mtrlGet(elm.id, test))
                              if(AdjustTexSrc(test.   color_map, test.   color_map_time, original.   color_map, downsized.   color_map, time) // !! use '|' and not '||' because we need to process all !!
                              |  AdjustTexSrc(test.   alpha_map, test.   alpha_map_time, original.   alpha_map, downsized.   alpha_map, time)
                              |  AdjustTexSrc(test.    bump_map, test.    bump_map_time, original.    bump_map, downsized.    bump_map, time)
                              |  AdjustTexSrc(test.  normal_map, test.  normal_map_time, original.  normal_map, downsized.  normal_map, time)
                              |  AdjustTexSrc(test.specular_map, test.specular_map_time, original.specular_map, downsized.specular_map, time)
                              |  AdjustTexSrc(test.    glow_map, test.    glow_map_time, original.    glow_map, downsized.    glow_map, time))
                     {
                        if(changed_base0 && test.base_0_tex==original.base_0_tex)test.base_0_tex=downsized.base_0_tex;
                        if(changed_base1 && test.base_1_tex==original.base_1_tex)test.base_1_tex=downsized.base_1_tex;
                        Proj.mtrlSync(elm.id, test, false, false); // we don't need to reload here, because since texture ID's are the same as 'original', then we can just re-use the generated texture ID's in the 'downsized'
                     }
                  }
               }
            }
         }
      }
      settings.removeKey(mtrl_id); // we have processed this material, so clear its settings
      if(revert || upscaled || !Next())refresh(); // proceed to next one
   }

   static void Revert(TextureDownsize &editor) {editor.applyConfirm(true);}

   static void Apply(TextureDownsize &editor) {editor.applyDo();}
          void applyDo()
   {
      if(different) // if there was any change applied
      {
         Dialog &dialog=Gui.getMsgBox(DialogID);
         dialog.set("Downsize Material Textures", "Are you sure you want to downsize material textures?", Memt<Str>().add("Yes").add("No")); dialog.button[2].show();
         dialog.activate();
         dialog.buttons[0].func(ApplyConfirm, T).kbSet();
         dialog.buttons[1].func(Hide, SCAST(GuiObj, dialog));
         return;
      }
      Next(); // proceed to next one
   }

   static void Dec(Property &prop) {prop.set(prop.asInt()-1);}
   static void Inc(Property &prop) {prop.set(prop.asInt()+1);}

   Settings* curSettings()  {return mtrl_id.valid() ?           settings      (mtrl_id) : null;}
 C Settings* curSettings()C {return mtrl_id.valid() ? ConstCast(settings).find(mtrl_id) : null;}

                    C Rect& rect(            ) {return super.rect();}
   virtual TextureDownsize& rect(C Rect &rect)override
   {
      super.rect(rect);
      Rect r=T.rect()-T.rect().lu();
      mode      .rect(Rect_U (r.up(), 0.40, 0.06));
      mtrl_image.rect(Rect_RD(r.rd(), 0.25));

      Vec2 offset=r.down()-region.rect().down();
      region.move(offset);
      prev  .move(offset);
      next  .move(offset);
      return T;
   }
   int getPart(GuiObj *go, C Vec2 &screen_pos, Vec *hit_pos=null)
   {
      int hit_part=-1;
      if(normal_mesh)
         if(Edit.Viewport4.View *view=v4.getView(go))
      {
       C MeshLod &lod=*normal_mesh;
         view.setViewportCamera();
         Vec pos, dir; ScreenToPosDir(screen_pos, pos, dir);
         pos+=(D.viewFrom ()/Dot(dir, ActiveCam.matrix.z))*dir;
         dir*= D.viewRange();
         flt frac, f; Vec hp;
         REPA(lod)if(Sweep(pos, dir, lod.parts[i], null, &f, &hp, null, true, -1, false))if(hit_part<0 || f<frac)
         {
            hit_part=i; frac=f; if(hit_pos)*hit_pos=hp;
         }
      }
      return hit_part;
   }
   bool selectionZoom(flt &dist)
   {
      if(normal_mesh)
      {
         flt size=normal_mesh->ext.size().avg();
         if( size>0)
         {
            dist=size/Tan(v4.perspFov()/2);
            return true;
         }
      }
      return false;
   }
   virtual void camCenter(bool zoom)override
   {
      Vec hit_pos; bool hit=(getPart(Gui.ms(), Ms.pos(), &hit_pos)>=0); flt dist;
      v4.moveTo(hit ? hit_pos : normal_mesh ? normal_mesh->ext.center : VecZero); if(zoom && selectionZoom(dist))v4.dist(hit ? dist/5 : dist); // use smaller dist because here we focus on texture quality, so zoom up close
   }
   bool selected       ()C {return Mode()==MODE_TEX_DOWN;}
   void selectedChanged()
   {
      closeConfirm();
      menu.enabled(selected());
      if(!selected())setElm(null); // clear elm to null so it will be completely reloaded next time
   }
   void toggle()
   {
      if(Mode()==MODE_TEX_DOWN)close();else
      {
         Mode.tabAvailable(MODE_TEX_DOWN, true);
         Mode.set(MODE_TEX_DOWN); HideBig();
      }
   }
   void close()
   {
      Mode.tabAvailable(MODE_TEX_DOWN, false);
      setElm(null); // clear elm to null so it will be completely reloaded next time
   }
   void clearProj()
   {
      setElm(null);
      settings.del();
   }
   TextureDownsize& create()
   {
      const flt padd=0.6, prop_h=0.06;

      super.create(Draw, false, 0, PI); v4.toggleHorizontal();
      wire.del(); cam_spherical.hide(); cam_lock.pos(cam_spherical.pos()); cam_tabs.hide();
      T+=mode      .create(ModeName, Elms(ModeName)).valid(true).set(OBJECT).func(ModeChanged, T);
      T+=light_dir .create(Rect_LU(ctrls.rect().ld(), 0.05, 0.05)).setImage(Proj.icon_env).focusable(false).desc("Set Vertical Light Direction\nKeyboard Shortcut: Alt+L"); light_dir.mode=BUTTON_TOGGLE;
      T+=mtrl_image.create().hide(); mtrl_image.fit=true; mtrl_image.alpha_mode=ALPHA_NONE;
      T+=region    .create().removeSlideBars().skin(&TransparentSkin).hide(); region.kb_lit=false;

      prop_ts.reset(); prop_ts.align.set(1, 0); prop_ts.size=0.055;
      props.New().create("Global", MemberDesc(DATA_INT).setFunc(Global, Global)).desc("How much to Downsize both Base0 and Base1 Material Textures.");
      props.New().create("Base 0", MemberDesc(DATA_INT).setFunc(Base0 , Base0 )).desc("How much to Downsize Base0 Material Texture, such as Color, Alpha, Bump."    ); // #MaterialTextureChannelOrder
      props.New().create("Base 1", MemberDesc(DATA_INT).setFunc(Base1 , Base1 )).desc("How much to Downsize Base1 Material Texture, such as Normal, Specular, Glow."); // #MaterialTextureChannelOrder
      Rect r=AddProperties(props, region, Vec2(padd, -0.005), prop_h, 0.25, &prop_ts); REPAO(props).autoData(this).range(-1, 10).mouseEditSpeed(1).changed(SetTextures);

      Node<MenuElm> menu; Viewport4Region &v4=T;
      menu.New().create("Camera Reset"   , CamReset  , v4).kbsc(KbSc(KB_BACK                           )).flag(MENU_HIDDEN);
      menu.New().create("Camera Center"  , CamCenter , v4).kbsc(KbSc(KB_TILDE, KBSC_CTRL_CMD           )).flag(MENU_HIDDEN);
      menu.New().create("Camera Center Z", CamCenterZ, v4).kbsc(KbSc(KB_TILDE, KBSC_CTRL_CMD|KBSC_SHIFT)).flag(MENU_HIDDEN);
      menu.New().create("Light Direction", LightMode , T).kbsc(KbSc(KB_L, KBSC_ALT)).flag(MENU_TOGGLABLE);
      menu.New().create("Mode0"          , Mode0     , T).kbsc(KbSc(KB_F1));
      menu.New().create("Mode1"          , Mode1     , T).kbsc(KbSc(KB_F2));
      menu.New().create("Prev"           , Prev      , T, true).kbsc(KbSc(KB_COMMA, KBSC_REPEAT));
      menu.New().create("Next"           , Next      , T, true).kbsc(KbSc(KB_DOT  , KBSC_REPEAT));
      menu.New().create("Prev S"         , PrevS     , T, true).kbsc(KbSc(KB_COMMA, KBSC_SHIFT|KBSC_REPEAT));
      menu.New().create("Next S"         , NextS     , T, true).kbsc(KbSc(KB_DOT  , KBSC_SHIFT|KBSC_REPEAT));
      menu.New().create("Apply"          , Apply     , T).kbsc(KbSc(KB_ENTER));
      int kbsc=menu.children.elms();
      menu.New().create("Dec Global"     , Dec       , props[0]).kbsc(KbSc(Kb.qwerty(KB_R))).kbsc2(KbSc(KB_SUB));
      menu.New().create("Inc Global"     , Inc       , props[0]).kbsc(KbSc(Kb.qwerty(KB_T))).kbsc2(KbSc(KB_EQUAL));
      menu.New().create("Dec Base 0"     , Dec       , props[1]).kbsc(KbSc(Kb.qwerty(KB_F))).kbsc2(KbSc(KB_LBR));
      menu.New().create("Inc Base 0"     , Inc       , props[1]).kbsc(KbSc(Kb.qwerty(KB_G))).kbsc2(KbSc(KB_RBR));
      menu.New().create("Dec Base 1"     , Dec       , props[2]).kbsc(KbSc(Kb.qwerty(KB_V))).kbsc2(KbSc(KB_SEMI));
      menu.New().create("Inc Base 1"     , Inc       , props[2]).kbsc(KbSc(Kb.qwerty(KB_B))).kbsc2(KbSc(KB_APO));
      Gui+=T.menu.create(menu);

      REPD(p, 3)REP(2)
      {
         MenuElm &elm=menu.children[kbsc+p*2+i];
         Str      desc=S+(i ? "Increase" : "Decrease")+"\nKeyboard Shortcut: "+elm.kbsc().asText(); if(elm.kbsc2().is())desc.space()+=elm.kbsc2().asText();
         region+=downsize[p][i].create(props[p].button.rect()+Vec2(prop_h+i*props[p].button.rect().w(), 0)).setImage(i ? "Gui/arrow_right_big.img" : "Gui/arrow_left_big.img").func(i ? Inc : Dec, props[p]).focusable(false);
                 downsize[p][i].desc(desc);
                 downsize[p][i].mode=BUTTON_IMMEDIATE;
      }
      left_ts.reset().size=0.05; left_ts.align.set(-1, 0); right_ts=left_ts; right_ts.align.set(1, 0);
      Vec2 xs(r.min.x-0.05, downsize[0][1].rect().max.x+0.05);
      REPD(p, 3)REP(2)region+=info[p][i].create(Vec2(xs.c[i], downsize[p][0].rect().centerY()), S, i ? &right_ts : &left_ts);

      region+=revert.create(Rect_U(padd/2, -region.virtualHeight()-0.01, 0.22, 0.07), "Revert").func(Revert, T).focusable(false).desc("Revert Material texture sizes to the source file");
      region+=apply.create(Rect_U(padd+(region.virtualWidth()-padd)/2, revert.rect().max.y, 0.25, 0.07), "Downsize").func(Apply, T).focusable(false).desc("Keyboard Shortcut: Enter");

      region.size(Vec2(region.virtualWidth()+padd, region.virtualHeight()));

      T+=prev.create(Rect_R(apply.rect().left ()-Vec2(0.02, 0), 0.1, 0.07), "<").func(Prev, T, true).focusable(false).desc("Previous Element\nKeyboard Shortcut: <");
      T+=next.create(Rect_L(apply.rect().right()+Vec2(0.02, 0), 0.1, 0.07), ">").func(Next, T, true).focusable(false).desc(    "Next Element\nKeyboard Shortcut: >");

      return T;
   }
   void toGui() {REPAO(props).toGui();}
   void setShader() {downsized_mesh.setShader(0);}
   void setDrawAs()
   {
      draw_as_obj=(mode()==OBJECT && normal_mesh); // try as mesh first
      light_dir.visible(draw_as_obj);
   }

   static Str ImageInfo(C Image *image, int &size, bool side)
   {
      if(image && image.is())
      {
         int mem_size=image->typeMemUsage(); size+=mem_size; // use 'typeMemUsage' because we need this only for stats
         return side ? S+FileSize(mem_size)+"  "+image->w()+'x'+image->h()+"  "+ImageTI[image->type()].name
                     : S+ImageTI[image->type()].name+"  "+image->w()+'x'+image->h()+"  "+FileSize(mem_size);
      }
      return S;
   }

   static void SetTextures(C Property &prop) {TexDownsize.setTextures();}
          void setTextures()
   {
      different=false;
      Settings *settings=curSettings();
      int d;

      d=((settings && normal_mtrl && normal_mtrl->base_0) ? settings.base0() : 0);
      different|=(d!=0);
      if(d>0)
      {
         normal_mtrl->base_0->copyTry(downsized_base[0], Max(1, normal_mtrl->base_0->w()>>d), Max(1, normal_mtrl->base_0->h()>>d), -1, -1, -1, normal_mtrl->base_0->mipMaps()-d, FILTER_BEST, true, false, false, false);
      }else downsized_base[0].del();

      d=((settings && normal_mtrl && normal_mtrl->base_1) ? settings.base1() : 0);
      different|=(d!=0);
      if(d>0)
      {
         normal_mtrl->base_1->copyTry(downsized_base[1], Max(1, normal_mtrl->base_1->w()>>d), Max(1, normal_mtrl->base_1->h()>>d), -1, -1, -1, normal_mtrl->base_1->mipMaps()-d, FILTER_BEST, true, false, false, true);
      }else downsized_base[1].del();

      int            size[]={0, 0};
    C Image *normal_image[]={normal_mtrl ? normal_mtrl->base_0() : null, normal_mtrl ? normal_mtrl->base_1() : null};
      info[1][0].set(ImageInfo(                                              normal_image[0], size[0], false));
      info[1][1].set(ImageInfo(downsized_base[0].is() ? &downsized_base[0] : normal_image[0], size[1], true ));
      info[2][0].set(ImageInfo(                                              normal_image[1], size[0], false));
      info[2][1].set(ImageInfo(downsized_base[1].is() ? &downsized_base[1] : normal_image[1], size[1], true ));
      info[0][0].set(normal_mtrl ? S+"(Original)  "+FileSize(size[0])  : S);
      info[0][1].set(normal_mtrl ? S+FileSize(size[1])+"  (Downsized)" : S);
      
      REPAO(downsized_mtrls).setTextures();
   }
   void setTexIDs()
   {
      if(normal_mtrl=Proj.gamePath(mtrl_id))
      {
         normal_base[0]=normal_mtrl->base_0.id();
         normal_base[1]=normal_mtrl->base_1.id();
      }else REPAO(normal_base).zero();
   }
   void setMtrl()
   {
      setTexIDs();
      mtrl_image.set(normal_mtrl ? normal_mtrl->base_0 : ImagePtr()); mtrl_image.visible(mtrl_image.image!=null); // !! DO NOT MERGE INTO SINGLE COMMAND !!
      setTextures();
   }
   void setObj()
   {
      downsized_mtrls.clear();

         normal_mesh.clear(); if(C Elm *obj=Proj.findElm(obj_id))if(C ElmObj *obj_data=obj.objData())normal_mesh=Proj.gamePath(obj_data.mesh_id);
      if(normal_mesh)
      {
         downsized_mesh.create(*normal_mesh); REPA(downsized_mesh)
         {
            MeshPart &part=downsized_mesh.parts[i]; REP(part.variations())if(C MaterialPtr &mtrl=part.variation(i))
            {
               if(normal_base[0].valid() && normal_base[0]==mtrl->base_0.id()
               || normal_base[1].valid() && normal_base[1]==mtrl->base_1.id())
                  part.variation(i, downsized_mtrls(mtrl.id()));
            }
         }
      }else downsized_mesh.del();
      setDrawAs();
   }
   void refresh() {setMtrl(); toGui();}
   void setMtrlObj(C UID &mtrl_id, C UID &obj_id)
   {
      bool changed_mtrl=(T.mtrl_id!=mtrl_id),
           changed_obj =(T. obj_id!= obj_id);
      if(changed_mtrl               ){T.mtrl_id=mtrl_id; refresh();}
      if(changed_mtrl || changed_obj){T. obj_id= obj_id; setObj ();}
   }
   void getSecondaryList()
   {
      secondary.clear();
      if(elm)switch(elm.type)
      {
         case ELM_MTRL: // get all objects using this material's textures
         {
            if(ElmMaterial *mtrl_data=elm.mtrlData())
            {
               bool allow_removed=elm.finalRemoved(); // if this material is removed, then allow listing removed objects

               // get material textures
               UID  base0_id   =mtrl_data.base_0_tex,
                    base1_id   =mtrl_data.base_1_tex;
               bool base0_valid=base0_id.valid(),
                    base1_valid=base1_id.valid();

               // get materials using these textures
               Memt<UID> mtrls;
               FREPA(Proj.elms) // 'elms' are sorted by ID, so process in order so we don't need to sort 'mtrls' later
               {
                  Elm &elm=Proj.elms[i];
                  if(ElmMaterial *mtrl_data=elm.mtrlData())
                     if(base0_valid && base0_id==mtrl_data.base_0_tex
                     || base1_valid && base1_id==mtrl_data.base_1_tex)mtrls.add(elm.id);
               }
               // get objects using these materials
               FREPA(Proj.elms)
               {
                  Elm &elm=Proj.elms[i];
                  if(allow_removed || elm.finalExists())if(ElmObj *obj_data=elm.objData())if(Elm *mesh=Proj.findElm(obj_data.mesh_id))if(ElmMesh *mesh_data=mesh.meshData())
                     REPA(mesh_data.mtrl_ids)if(mtrls.binaryHas(mesh_data.mtrl_ids[i], Compare)){secondary.add(elm.id); break;}
               }
            }
         }break;

         case ELM_OBJ: // get all materials used by this object
         {
            if(ElmObj *obj_data=elm.objData())
            {
            #if 0 // don't do this because this is sorted by ID, and we want to sort by Part/Variation
               if(Elm *mesh=Proj.findElm(obj_data.mesh_id))if(ElmMesh *mesh_data=mesh.meshData())
               REPA(mesh_data.mtrl_ids)secondary.binaryInclude(mesh_data.mtrl_ids[i], Compare);
            #else
               if(MeshPtr mesh=Proj.gamePath(obj_data.mesh_id))
                  FREPD (v, mesh->variations()) // iterate variations first
                  FREPAD(p, mesh->parts       ) // iterate all parts
                     if(C MaterialPtr &mtrl=mesh->parts[p].variation(v))secondary.include(mtrl.id());
            #endif
            }
         }break;
      }
      mode.visible(secondary.elms()>0);
   }
   void setSecondary(int i)
   {
      if(elm)
      {
         secondary_i=MAX(MIN(i, secondary.elms()-1), 0); // maximize last, so index is not <0
         UID secondary_id=(InRange(i, secondary) ? secondary[i] : UIDZero);
         if(elm.type==ELM_MTRL)setMtrlObj(elm.id, secondary_id);else
         if(elm.type==ELM_OBJ )setMtrlObj(secondary_id, elm.id);
      }else                    setMtrlObj(UIDZero, UIDZero);
   }
   void setElm(Elm *elm, bool prev=false)
   {
      if(elm)if(elm.type!=ELM_OBJ && elm.type!=ELM_MTRL)elm=null;
      if(T.elm!=elm)
      {
         T.elm=elm;
         getSecondaryList();
         setSecondary(prev ? secondary.elms()-1 : 0);
         region.visible(elm!=null);
         closeConfirm();
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         Elm *elm=Proj.list(); if(elm)if(elm.type!=ELM_OBJ && elm.type!=ELM_MTRL)elm=null;
         if( !elm)elm=MtrlEdit.elm;
         setElm(elm);

         if(draw_as_obj)
         {
            if(normal_mesh)
            {
               if(Ms.b(0))
               {
                  int part=getPart(Gui.ms(), Ms.pos()); if(InRange(part, *normal_mesh))
                  {
                     UID mtrl_id=normal_mesh->parts[part].variation(VariationWithMostMaterialUses(*normal_mesh, normal_mtrl)).id();
                     MtrlEdit.activate(Proj.findElm(mtrl_id));
                  }
               }
               if(Ms.bd(0) && v4.getView(Gui.ms()))ObjEdit.activate(Proj.findElm(obj_id));
            }
         }else
         {
            if(Edit.Viewport4.View *view=v4.getView(Gui.ms()))
            {
               if(Ms.b(1)                        ){light_angle+=Ms.d()*Vec2(-1, 1);                        Ms.freeze();}
               if(Ms.b(0) && !Ms.b(2) && !Ms.b(4)){view.camera.yaw-=Ms.d().x; view.camera.pitch+=Ms.d().y; Ms.freeze();}
               if(Ms.bd(0)                       )MtrlEdit.activate(Proj.findElm(mtrl_id));
            }
         }
         if(Ms.bp(0) && Gui.ms()==&mtrl_image)MtrlEdit.activate(Proj.findElm(mtrl_id));
         if(Ms.bp(2))if(Dialog *dialog=Gui.findMsgBox(DialogID))if(dialog.contains(Gui.ms()))dialog.hide();
         
         REPAO(downsized_mtrls).update();
      }
   }
}
TextureDownsize TexDownsize;
/******************************************************************************/
