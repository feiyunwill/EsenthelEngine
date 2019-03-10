/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
MaterialRegion MtrlEdit;
/******************************************************************************/
MaterialTech mtrl_techs[]=
{
   {"Default"               , u"Standard rendering of solid (opaque) materials."},
   {"Alpha Test"            , u"Indicates that textures alpha channel will be used as models transparency.\nThis is slightly slower than Default as alpha testing may disable some hardware-level optimizations."},
   {"Fur"                   , u"Mesh will be rendered with fur effect, the mesh will be wrapped with additional fur imitating textures.\nDetail Scale specifies fur intensity, Detail Power specifies fur length.\nSupported only in Deferred Renderer!"},
   {"Grass"                 , u"Mesh vertexes will bend on the wind like grass,\nbending intensity is determined by mesh vertex source Y position,\nwhich should be in the range from 0 to 1."},
   {"Leaf"                  , u"Mesh vertexes will bend on the wind like tree leafs,\nto use this technique mesh must also contain leaf attachment positions,\nwhich can be generated in the Model Editor tool through menu options."},
   {"Blend"                 , u"Mesh will be smoothly blended on the screen using alpha values,\nmesh will not be affected by lighting or shadowing."},
   {"Blend Light"           , u"Works like Blend technique except that mesh will be affected by lighting or shadowing,\nhowever only the most significant directional light will be used (all other lights are ignored)\nDue to additional lighting calculations this is slower than Blend technique."},
   {"Blend Light Grass"     , u"Combination of Blend Light and Grass techniques."},
   {"Blend Light Leaf"      , u"Combination of Blend Light and Leaf techniques."},
   {"Test Blend Light"      , u"Works like Blend Light technique with additional Alpha-Testing and Depth-Writing which enables correct Depth-Sorting."},
   {"Test Blend Light Grass", u"Works like Blend Light Grass technique with additional Alpha-Testing and Depth-Writing which enables correct Depth-Sorting."},
   {"Test Blend Light Leaf" , u"Works like Blend Light Leaf technique with additional Alpha-Testing and Depth-Writing which enables correct Depth-Sorting."},
};                                                                                                                                                           
/******************************************************************************/

/******************************************************************************/
   cchar8 *MaterialRegion::auto_reload_name="Reload on Change";
   cchar8 *MaterialRegion::DownsizeTexMobileText[]=
   {
      "Full",
      "Half",
      "Quarter",
   };
   cchar8 *MaterialRegion::TexQualityiOSText[]=
   {
      "2-bit Low (default)",
      "4-bit High",
   };
   const flt MaterialRegion::BumpScale=0.10f;
/******************************************************************************/
      void MaterialRegion::Change::create(ptr user)
{
         data=MtrlEdit.edit;
         MtrlEdit.undoVis();
      }
      void MaterialRegion::Change::apply(ptr user)
{
         uint changed=MtrlEdit.edit.undo(data);
         MtrlEdit.setChanged();
         MtrlEdit.toGui();
         D.setShader(MtrlEdit.game());
         MtrlEdit.undoVis();
         if(changed&(EditMaterial::CHANGED_BASE|EditMaterial::CHANGED_REFL|EditMaterial::CHANGED_DET|EditMaterial::CHANGED_MACRO|EditMaterial::CHANGED_LIGHT))Proj.mtrlTexChanged();
      }
      void MaterialRegion::Texture::Load(C Str &name, Texture &texture) {texture.setFile(name);}
      void MaterialRegion::Texture::Remove(             Texture &texture) {texture.setFile(S);}
      ImagePtr MaterialRegion::Texture::getImage()
      {
         if(mr)
         {
            EditMaterial &em        =mr->getEditMtrl  ();
          C ImagePtr     &base_0    =mr->getBase0     (),
                         &base_1    =mr->getBase1     (),
                         &macro     =mr->getMacro     (),
                         &detail    =mr->getDetail    (),
                         &reflection=mr->getReflection(),
                         &light     =mr->getLight     ();
            switch(type)
            {
               case TEX_COLOR   : if(em.     color_map.is()                          )return base_0; break;
               case TEX_ALPHA   : if(em.     color_map.is() || em.alpha_map.is()     )return base_1 ? base_1 : base_0; break;
               case TEX_BUMP    : if(                          em.hasBumpMap()       )if(base_1)return base_0; break;
               case TEX_NORMAL  : if(em.    normal_map.is() || em.hasBumpMap()       )return base_1; break;
               case TEX_SPEC    : if(em.  specular_map.is()                          )return base_1; break;
               case TEX_GLOW    : if(em.      glow_map.is() && em.tech==MTECH_DEFAULT)return base_1; break;
               case TEX_LIGHT   : if(em.     light_map.is()                          )return light ; break;
               case TEX_MACRO   : if(em.     macro_map.is()                          )return macro ; break;
               case TEX_DET_COL : if(em. detail_color .is()                          )return detail; break;
               case TEX_DET_BUMP: if(em. detail_bump  .is()                          )return detail; break;
               case TEX_DET_NRM : if(em. detail_normal.is() || em.detail_bump.is()   )return detail; break;
               case TEX_RFL_L   : if(em.reflection_map.is()                          )return reflection; break;
               case TEX_RFL_F   : if(em.reflection_map.is()                          )return reflection; break;
               case TEX_RFL_R   : if(em.reflection_map.is()                          )return reflection; break;
               case TEX_RFL_B   : if(em.reflection_map.is()                          )return reflection; break;
               case TEX_RFL_D   : if(em.reflection_map.is()                          )return reflection; break;
               case TEX_RFL_U   : if(em.reflection_map.is()                          )return reflection; break;
               case TEX_RFL_ALL : if(em.reflection_map.is()                          )return reflection; break;
            }
         }
         return null;
      }
      void MaterialRegion::Texture::setDesc()
      {
         Mems<Edit::FileParams> files=Edit::FileParams::Decode(file); UID image_id; REPA(files)if(DecodeFileName(files[i].name, image_id))files[i].name=Proj.elmFullName(image_id);
         Str desc=Replace(text, '\n', ' '); if(C ImagePtr &image=getImage())desc+=S+", "+image->w()+'x'+image->h();
         if(type==TEX_MACRO  )desc.line()+="Can be set for heightmap materials to decrease repetitiveness of textures.\nBecomes visible at distance of around 100 meters.";
         if(type==TEX_LIGHT  )desc.line()+="Model must have 2nd set of texture coordinates (VTX_TEX1) in order for lightmap to take effect";
         if(type==TEX_RFL_ALL)desc.line()+="Drag and drop an image here to set it for all faces";
         FREPA(files){desc+='\n'; desc+=files[i].encode();}
         T.desc(desc);
      }
      void MaterialRegion::Texture::setFile(Str file)
      {
         // convert multiple lines into separate file params and handle <..> commands
         Mems<Edit::FileParams> fps=Edit::FileParams::Decode(file); REPA(fps)
         {
            Edit::FileParams &fp=fps[i];
            if(Contains(fp.name, '<'))fp.name=GetBaseNoExt(fp.name); // we want to support things like "<bump>" but when entering that into 'WindowIO', it may append the path, so when this command is detected, remove the path (and possible extension too)
         }
         file=Edit::FileParams::Encode(fps);

         T.file=file; setDesc();
         if(mr)
         {
            mr->undos.set(null, true);
            EditMaterial &mtrl=mr->getEditMtrl();
            uint base_tex=mtrl.baseTex(); // get current state of textures before making any change
            if(type>=TEX_RFL_L && type<=TEX_RFL_U)file=SetCubeFile(md_file.asText(&mtrl), type-TEX_RFL_L, file);
            md_time.as<TimeStamp>(&mtrl).now();
            md_file.fromText     (&mtrl, file);

            if(!file.is()) // if removing texture
            {
               Str name, name2; switch(type)
               {
                  case TEX_COLOR: name="<color>"; break;
                  case TEX_SPEC : name="<spec>" ; name2="<specular>"; break;
                  case TEX_BUMP : name="<bump>" ; break;
               }
               if(name.is())REPA(mr->texs)
               {
                  Texture &tex=mr->texs[i]; if(tex.type>=TEX_COLOR && tex.type<=TEX_GLOW) // iterate all base textures
                  {
                     Mems<Edit::FileParams> fps=Edit::FileParams::Decode(tex.md_file.asText(&mtrl)); // get file name of that texture
                     if(fps.elms()==1 && (fps[0].name==name || name2.is() && fps[0].name==name2)) // if that file is made from removed one
                     {
                        tex.md_time.as<TimeStamp>(&mtrl).now();
                        tex.md_file.fromText     (&mtrl, S); // remove too
                        tex.toGui();
                     }
                  }
               }
            }

            // rebuild methods already call 'setChanged'
            if(type>=TEX_COLOR   && type<=TEX_GLOW   )mr->rebuildBase      (base_tex);else
            if(type>=TEX_DET_COL && type<=TEX_DET_NRM)mr->rebuildDetail    ();else
            if(type>=TEX_RFL_L   && type<=TEX_RFL_ALL)mr->rebuildReflection();else
            if(type==TEX_MACRO                       )mr->rebuildMacro     ();else
            if(type==TEX_LIGHT                       )mr->rebuildLight     ();else
                                                      mr->setChanged       ();

            // if adjusting reflection texture, then adjust parameters of all other relfection slots, because they are connected
            if(type>=TEX_RFL_L && type<=TEX_RFL_ALL)REPA(mr->texs)if(mr->texs[i].type>=TEX_RFL_L && mr->texs[i].type<=TEX_RFL_ALL)mr->texs[i].toGui();
         }
      }
      void MaterialRegion::Texture::toGui()
      {
         if(mr)
         {
            file=md_file.asText(&mr->getEditMtrl()); if(type>=TEX_RFL_L && type<=TEX_RFL_U)file=GetCubeFile(file, type-TEX_RFL_L);
            setDesc();
         }
      }
      ::MaterialRegion::Texture& MaterialRegion::Texture::create(TEX_TYPE type, C MemberDesc &md_file, C MemberDesc &md_time, Rect rect, C Str &text, MaterialRegion &mr)
      {
         T.mr=&mr;
         T.type=type;
         T.md_file=md_file;
         T.md_time=md_time;
         ::EE::GuiCustom::create().rect(rect.extend(-0.003f)); T.text=text;
         win_io.create().ext(SUPPORTED_IMAGE_EXT, Replace(text, '\n', ' ')).io(Load, Load, T);
         remove.create(Rect_RU(rect.ru(), 0.035f, 0.035f)).func(Remove, T); remove.image="Gui/close.img";
         return T;
      }
      void MaterialRegion::Texture::update(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            ::EE::GuiObj::update(gpc);
            rect_color=((Gui.msLit()==this && Gui.skin) ? Gui.skin->keyboard_highlight_color : Gui.borderColor());
            REPA(MT)if(MT.bp(i) && MT.guiObj(i)==this)
            {
               UID id;
               if(file.is() && !DecodeFileName(file, id)) // texture path
               {
                  if(Kb.ctrl()) // Ctrl+Click -> explore file path
                  {
                     Mems<Edit::FileParams> fps=Edit::FileParams::Decode(file); if(fps.elms()>=1)Explore(FFirstUp(fps[0].name));
                     goto skip;
                  }else
                  {
                     SetPath(win_io, file);
                  }
               }else
               if(mr)
               {
                  Mems<Edit::FileParams> fps=Edit::FileParams::Decode(mr->getEditMtrl().color_map);
                  Str  mtrl_path=Proj.elmSrcFileFirst(mr->elm); if(FileInfoSystem(mtrl_path).type==FSTD_FILE)mtrl_path=GetPath(mtrl_path);
                  Str color_path; if(fps.elms())color_path=FFirstUp(GetPath(fps[0].name));
                  SetPath(win_io, (mtrl_path.length()>color_path.length()) ? mtrl_path : color_path);
               }
               win_io.activate();
            skip:;
            }
         }
         remove.visible((Gui.ms()==this || Gui.ms()==&remove) && file.is());
      }
      bool MaterialRegion::Texture::draw(C Rect &rect)
      {
         bool tex=false;
         if(mr)
         {
            EditMaterial &em        =mr->getEditMtrl  ();
          C ImagePtr     &base_0    =mr->getBase0     (),
                         &base_1    =mr->getBase1     (),
                         &macro     =mr->getMacro     (),
                         &detail    =mr->getDetail    (),
                         &reflection=mr->getReflection(),
                         &light     =mr->getLight     ();
            ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
            switch(type) // #MaterialTextureChannelOrder
            {
               case TEX_COLOR   : if(em.     color_map.is()                          )if(base_0          ){                                                                                                              base_0->drawFit(rect); tex=true;} break;
               case TEX_ALPHA   : if(em.     color_map.is() || em.alpha_map.is()     )if(base_0 || base_1){VI.shader(ShaderFiles("Main")->get(base_1 ? "DrawTexW" : "DrawTexW"  )); if(base_1)base_1->drawFit(rect);else base_0->drawFit(rect); tex=true;} break;
               case TEX_BUMP    : if(                          em.hasBumpMap()       )if(base_0 && base_1){VI.shader(ShaderFiles("Main")->get(                      "DrawTexW"  ));                                      base_0->drawFit(rect); tex=true;} break;
               case TEX_NORMAL  : if(em.    normal_map.is() || em.hasBumpMap()       )if(          base_1){VI.shader(ShaderFiles("Main")->get(                      "DrawTexNrm"));                                      base_1->drawFit(rect); tex=true;} break;
               case TEX_SPEC    : if(em.  specular_map.is()                          )if(          base_1){VI.shader(ShaderFiles("Main")->get(                      "DrawTexZ"  ));                                      base_1->drawFit(rect); tex=true;} break;
               case TEX_GLOW    : if(em.      glow_map.is() && em.tech==MTECH_DEFAULT)if(          base_1){VI.shader(ShaderFiles("Main")->get(                      "DrawTexW"  ));                                      base_1->drawFit(rect); tex=true;} break;
               case TEX_LIGHT   : if(em.     light_map.is()                          )if(           light){                                                                                                              light ->drawFit(rect); tex=true;} break;
               case TEX_MACRO   : if(em.     macro_map.is()                          )if(           macro){                                                                                                              macro ->drawFit(rect); tex=true;} break;
               case TEX_DET_COL : if(em. detail_color .is()                          )if(          detail){VI.shader(ShaderFiles("Main")->get(                      "DrawTexZ"  ));                                      detail->drawFit(rect); tex=true;} break;
               case TEX_DET_BUMP: if(em. detail_bump  .is()                          )if(          detail){                                                                if(Image *bump=mr->getDetailBump(em.detail_bump))bump->drawFit(rect); tex=true;} break; //{VI.shader(ShaderFiles("Main")->get(                      "DrawTexW"  ));                                      detail->drawFit(rect); tex=true;} break; // Detail Bump may not be available due to BC1 compression or RemoveMtrlDetailBump
               case TEX_DET_NRM : if(em. detail_normal.is() || em.detail_bump.is()   )if(          detail){VI.shader(ShaderFiles("Main")->get(                      "DrawTexNrm"));                                      detail->drawFit(rect); tex=true;} break;
               case TEX_RFL_L   : if(em.reflection_map.is()                          )if(      reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_LEFT   ); tex=true;} break;
               case TEX_RFL_F   : if(em.reflection_map.is()                          )if(      reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_FORWARD); tex=true;} break;
               case TEX_RFL_R   : if(em.reflection_map.is()                          )if(      reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_RIGHT  ); tex=true;} break;
               case TEX_RFL_B   : if(em.reflection_map.is()                          )if(      reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_BACK   ); tex=true;} break;
               case TEX_RFL_D   : if(em.reflection_map.is()                          )if(      reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_DOWN   ); tex=true;} break;
               case TEX_RFL_U   : if(em.reflection_map.is()                          )if(      reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_UP     ); tex=true;} break;
               case TEX_RFL_ALL : if(em.reflection_map.is()                          )if(      reflection)
               {
                  Image &i=*reflection; flt x[5]={rect.min.x, rect.lerpX(1.0f/4), rect.lerpX(2.0f/4), rect.lerpX(3.0f/4), rect.max.x},
                                            y[4]={rect.min.y, rect.lerpY(1.0f/3), rect.lerpY(2.0f/3), rect.max.y};
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[0], y[1], x[1], y[2]), DIR_LEFT   );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[1], y[1], x[2], y[2]), DIR_FORWARD);
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[2], y[1], x[3], y[2]), DIR_RIGHT  );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[3], y[1], x[4], y[2]), DIR_BACK   );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[1], y[0], x[2], y[1]), DIR_DOWN   );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[1], y[2], x[2], y[3]), DIR_UP     );
                  tex=true;
               }break;
            }
            D.alpha(alpha);
         }
         return tex;
      }
      void MaterialRegion::Texture::draw(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            D.clip(gpc.clip);
            Rect   r=rect()+gpc.offset;
            TextStyleParams ts; if(draw(r))ts.reset(false).size=0.040f;else ts.reset(true).size=0.036f;
            D.text(ts, r, text);

            // if mouse cursor is over reload button, and source file is not found, then draw exclamation as a warning
            if((Gui.ms()==&mr->reload_base_textures || Gui.ms()==&mr->texture_options)
            && Proj.invalidTexSrc(file))
               Proj.exclamation->drawFit(Rect_C(r.center(), 0.08f, 0.08f));

            r.draw(rect_color, false);
         }
      }
   void MaterialRegion::undoVis() {SetUndo(undos, undo, redo);}
   Vec MaterialRegion::previewLight()C {return Matrix3().setRotateXY(light_angle.y-ActiveCam.pitch, light_angle.x-ActiveCam.yaw).z;}
   void MaterialRegion::Render() {MtrlEdit.render();}
          void MaterialRegion::render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            if(InRange(preview_mode(), preview_mesh))
            {
               Matrix m; m.setScale((game && game->technique==MTECH_FUR) ? 0.75f : 1.0f);
               preview_mesh[preview_mode()].draw(m);
            }

            LightDir(previewLight(), 1-D.ambientColor()).add(false);
         }break;
      }
   }
   void MaterialRegion::DrawPreview(Viewport &viewport) {((MaterialRegion*)viewport.user)->drawPreview();}
   void MaterialRegion::drawPreview()
   {
      preview_cam.set();

      REPAO(preview_mesh).material(game).setShader();

      CurrentEnvironment().set();
      MOTION_MODE  motion   =D.   motionMode(); D.   motionMode( MOTION_NONE);
      AMBIENT_MODE ambient  =D.  ambientMode(); D.  ambientMode(AMBIENT_FLAT);
      DOF_MODE     dof      =D.      dofMode(); D.      dofMode(    DOF_NONE);
      bool         eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(       false);
      bool         astros   =AstrosDraw       ; AstrosDraw     =false;
      bool         ocean    =Water.draw       ; Water.draw     =false;

      Renderer(Render);

      D.      dofMode(dof      );
      D.   motionMode(motion   );
      D.  ambientMode(ambient  );
      D.eyeAdaptation(eye_adapt);
      AstrosDraw     =astros;
      Water.draw     =ocean;
   }
   void MaterialRegion::PreChanged(C Property &prop) {cptr change_type=&prop; if(change_type==MtrlEdit.green || change_type==MtrlEdit.blue)change_type=MtrlEdit.red; MtrlEdit.undos.set(change_type);}
   void    MaterialRegion::Changed(C Property &prop) {MtrlEdit.setChanged();}
   Str  MaterialRegion::Tech(C MaterialRegion &mr          ) {return mr.edit.tech;}
   void MaterialRegion::Tech(  MaterialRegion &mr, C Str &t) {mr.edit.tech=MATERIAL_TECHNIQUE(TextInt(t)); mr.edit.tech_time.now(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::DownsizeTexMobile(C MaterialRegion &mr          ) {return mr.edit.downsize_tex_mobile;}
   void MaterialRegion::DownsizeTexMobile(  MaterialRegion &mr, C Str &t) {mr.edit.downsize_tex_mobile=TextInt(t); mr.edit.downsize_tex_mobile_time.getUTC();}
   Str  MaterialRegion::TexQualityiOS(C MaterialRegion &mr          ) {return mr.edit.high_quality_ios;}
   void MaterialRegion::TexQualityiOS(  MaterialRegion &mr, C Str &t) {mr.edit.high_quality_ios=TextBool(t); mr.edit.high_quality_ios_time.getUTC();}
   void MaterialRegion::RGB(MaterialRegion &mr)
   {
      mr.undos.set("brightness");
      Vec2 d=0; int on=0, pd=0; REPA(MT)if(MT.b(i) && MT.guiObj(i)==&mr.brightness){d+=MT.ad(i); if(!MT.touch(i))Ms.freeze(); if(MT.bp(i))pd++;else on++;}
      Vec &rgb=mr.edit.color.xyz; if(pd && !on){mr.mouse_edit_value=rgb; mr.mouse_edit_delta=0;} mr.mouse_edit_delta+=d.sum()*0.75f;
      flt  max=mr.mouse_edit_value.max(), lum=max+mr.mouse_edit_delta; if(lum<0){mr.mouse_edit_delta-=lum; lum=0;}
      Vec  v  =mr.mouse_edit_value; if(max)v/=max;else v=1; v*=lum;
      if(mr.red  ){mr.red  ->set(v.x, QUIET); rgb.x=mr.red  ->asFlt();}
      if(mr.green){mr.green->set(v.y, QUIET); rgb.y=mr.green->asFlt();}
      if(mr.blue ){mr.blue ->set(v.z, QUIET); rgb.z=mr.blue ->asFlt();}
      mr.edit.color_time.getUTC(); mr.setChanged();
   }
   Str  MaterialRegion::Red(C MaterialRegion &mr          ) {return mr.edit.color.x;}
   void MaterialRegion::Red(  MaterialRegion &mr, C Str &t) {mr.edit.color.x=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Green(C MaterialRegion &mr          ) {return mr.edit.color.y;}
   void MaterialRegion::Green(  MaterialRegion &mr, C Str &t) {mr.edit.color.y=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Blue(C MaterialRegion &mr          ) {return mr.edit.color.z;}
   void MaterialRegion::Blue(  MaterialRegion &mr, C Str &t) {mr.edit.color.z=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Alpha(C MaterialRegion &mr          ) {return mr.edit.color.w;}
   void MaterialRegion::Alpha(  MaterialRegion &mr, C Str &t) {mr.edit.color.w=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Bump(C MaterialRegion &mr          ) {return mr.edit.bump/BumpScale;}
   void MaterialRegion::Bump(  MaterialRegion &mr, C Str &t) {mr.edit.bump=TextFlt(t)*BumpScale; mr.edit.rough_bump_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::NrmScale(C MaterialRegion &mr          ) {return mr.edit.rough;}
   void MaterialRegion::NrmScale(  MaterialRegion &mr, C Str &t) {mr.edit.rough=TextFlt(t); mr.edit.rough_bump_time.getUTC();}
   Str  MaterialRegion::FNY(C MaterialRegion &mr          ) {return mr.edit.flip_normal_y;}
   void MaterialRegion::FNY(  MaterialRegion &mr, C Str &t) {uint base_tex=mr.edit.baseTex(); mr.edit.flip_normal_y=TextBool(t); mr.edit.flip_normal_y_time.getUTC(); mr.rebuildBase(base_tex, true, false);}
   Str  MaterialRegion::Spec(C MaterialRegion &mr          ) {return mr.edit.specular;}
   void MaterialRegion::Spec(  MaterialRegion &mr, C Str &t) {mr.edit.specular=TextFlt(t); mr.edit.spec_time.getUTC();}
   Str  MaterialRegion::Glow(C MaterialRegion &mr          ) {return mr.edit.glow;}
   void MaterialRegion::Glow(  MaterialRegion &mr, C Str &t) {mr.edit.glow=TextFlt(t); mr.edit.glow_time.getUTC();}
   Str  MaterialRegion::DetScale(C MaterialRegion &mr          ) {return mr.edit.det_scale;}
   void MaterialRegion::DetScale(  MaterialRegion &mr, C Str &t) {mr.edit.det_scale=TextFlt(t); mr.edit.detail_time.getUTC();}
   Str  MaterialRegion::DetPower(C MaterialRegion &mr          ) {return mr.edit.det_power;}
   void MaterialRegion::DetPower(  MaterialRegion &mr, C Str &t) {mr.edit.det_power=TextFlt(t); mr.edit.detail_time.getUTC();}
   Str  MaterialRegion::Reflect(C MaterialRegion &mr          ) {return mr.edit.reflection;}
   void MaterialRegion::Reflect(  MaterialRegion &mr, C Str &t) {mr.edit.reflection=TextFlt(t); mr.edit.reflection_time.getUTC();}
   Str  MaterialRegion::Cull(C MaterialRegion &mr          ) {return mr.edit.cull;}
   void MaterialRegion::Cull(  MaterialRegion &mr, C Str &t) {mr.edit.cull=TextBool(t); mr.edit.cull_time.now();}
   Str  MaterialRegion::SSS(C MaterialRegion &mr          ) {return mr.edit.sss;}
   void MaterialRegion::SSS(  MaterialRegion &mr, C Str &t) {mr.edit.sss=TextFlt(t); mr.edit.sss_time.getUTC();}
   Str  MaterialRegion::AmbR(C MaterialRegion &mr          ) {return mr.edit.ambient.x;}
   void MaterialRegion::AmbR(  MaterialRegion &mr, C Str &t) {mr.edit.ambient.x=TextFlt(t); mr.edit.ambient_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::AmbG(C MaterialRegion &mr          ) {return mr.edit.ambient.y;}
   void MaterialRegion::AmbG(  MaterialRegion &mr, C Str &t) {mr.edit.ambient.y=TextFlt(t); mr.edit.ambient_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::AmbB(C MaterialRegion &mr          ) {return mr.edit.ambient.z;}
   void MaterialRegion::AmbB(  MaterialRegion &mr, C Str &t) {mr.edit.ambient.z=TextFlt(t); mr.edit.ambient_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::TexScale(C MaterialRegion &mr          ) {return mr.edit.tex_scale;}
   void MaterialRegion::TexScale(  MaterialRegion &mr, C Str &t) {mr.edit.tex_scale=TextFlt(t); mr.edit.tex_scale_time.getUTC();}
   void MaterialRegion::Undo(MaterialRegion &editor) {editor.undos.undo();}
   void MaterialRegion::Redo(MaterialRegion &editor) {editor.undos.redo();}
   void MaterialRegion::Locate(MaterialRegion &editor) {Proj.elmLocate(editor.elm_id);}
   void MaterialRegion::Hide(MaterialRegion &editor) {editor.set(null);}
   void MaterialRegion::SetMtrl(MaterialRegion &editor) {SetObjOp(editor.set_mtrl() ? OP_OBJ_SET_MTRL : OP_OBJ_NONE);}
   void MaterialRegion::AutoReload(MaterialRegion &editor) {editor.auto_reload=editor.texture_options.menu(auto_reload_name);}
   void MaterialRegion::ReloadBaseTextures(MaterialRegion &editor) {editor.undos.set("rebuildBase"); editor.rebuildBase(editor.getEditMtrl().baseTex(), false, false, true);}
   void MaterialRegion::ResizeBase128(MaterialRegion &editor) {editor.resizeBase(128);}
   void MaterialRegion::ResizeBase256(MaterialRegion &editor) {editor.resizeBase(256);}
   void MaterialRegion::ResizeBase512(MaterialRegion &editor) {editor.resizeBase(512);}
   void MaterialRegion::ResizeBase1024(MaterialRegion &editor) {editor.resizeBase(1024);}
   void MaterialRegion::ResizeBase2048(MaterialRegion &editor) {editor.resizeBase(2048);}
   void MaterialRegion::ResizeBase4096(MaterialRegion &editor) {editor.resizeBase(4096);}
   void MaterialRegion::ResizeBase128x64(MaterialRegion &editor) {editor.resizeBase(VecI2(128, 64));}
   void MaterialRegion::ResizeBase256x128(MaterialRegion &editor) {editor.resizeBase(VecI2(256, 128));}
   void MaterialRegion::ResizeBase512x256(MaterialRegion &editor) {editor.resizeBase(VecI2(512, 256));}
   void MaterialRegion::ResizeBase1024x512(MaterialRegion &editor) {editor.resizeBase(VecI2(1024, 512));}
   void MaterialRegion::ResizeBase2048x1024(MaterialRegion &editor) {editor.resizeBase(VecI2(2048, 1024));}
   void MaterialRegion::ResizeBase64x128(MaterialRegion &editor) {editor.resizeBase(VecI2(64, 128));}
   void MaterialRegion::ResizeBase128x256(MaterialRegion &editor) {editor.resizeBase(VecI2(128, 256));}
   void MaterialRegion::ResizeBase256x512(MaterialRegion &editor) {editor.resizeBase(VecI2(256, 512));}
   void MaterialRegion::ResizeBase512x1024(MaterialRegion &editor) {editor.resizeBase(VecI2(512, 1024));}
   void MaterialRegion::ResizeBase1024x2048(MaterialRegion &editor) {editor.resizeBase(VecI2(1024, 2048));}
   void MaterialRegion::ResizeBaseQuarter(MaterialRegion &editor) {editor.resizeBase(-2, true);}
   void MaterialRegion::ResizeBaseHalf(MaterialRegion &editor) {editor.resizeBase(-1, true);}
   void MaterialRegion::ResizeBaseOriginal(MaterialRegion &editor) {editor.resizeBase( 0, true);}
   void MaterialRegion::ResizeBaseDouble(MaterialRegion &editor) {editor.resizeBase( 1, true);}
   void MaterialRegion::ResizeBase0_128(MaterialRegion &editor) {editor.resizeBase0(128);}
   void MaterialRegion::ResizeBase0_256(MaterialRegion &editor) {editor.resizeBase0(256);}
   void MaterialRegion::ResizeBase0_512(MaterialRegion &editor) {editor.resizeBase0(512);}
   void MaterialRegion::ResizeBase0_1024(MaterialRegion &editor) {editor.resizeBase0(1024);}
   void MaterialRegion::ResizeBase0_2048(MaterialRegion &editor) {editor.resizeBase0(2048);}
   void MaterialRegion::ResizeBase0_4096(MaterialRegion &editor) {editor.resizeBase0(4096);}
   void MaterialRegion::ResizeBase0_128x64(MaterialRegion &editor) {editor.resizeBase0(VecI2(128, 64));}
   void MaterialRegion::ResizeBase0_256x128(MaterialRegion &editor) {editor.resizeBase0(VecI2(256, 128));}
   void MaterialRegion::ResizeBase0_512x256(MaterialRegion &editor) {editor.resizeBase0(VecI2(512, 256));}
   void MaterialRegion::ResizeBase0_1024x512(MaterialRegion &editor) {editor.resizeBase0(VecI2(1024, 512));}
   void MaterialRegion::ResizeBase0_2048x1024(MaterialRegion &editor) {editor.resizeBase0(VecI2(2048, 1024));}
   void MaterialRegion::ResizeBase0_64x128(MaterialRegion &editor) {editor.resizeBase0(VecI2(64, 128));}
   void MaterialRegion::ResizeBase0_128x256(MaterialRegion &editor) {editor.resizeBase0(VecI2(128, 256));}
   void MaterialRegion::ResizeBase0_256x512(MaterialRegion &editor) {editor.resizeBase0(VecI2(256, 512));}
   void MaterialRegion::ResizeBase0_512x1024(MaterialRegion &editor) {editor.resizeBase0(VecI2(512, 1024));}
   void MaterialRegion::ResizeBase0_1024x2048(MaterialRegion &editor) {editor.resizeBase0(VecI2(1024, 2048));}
   void MaterialRegion::ResizeBase0_Quarter(MaterialRegion &editor) {editor.resizeBase0(-2, true);}
   void MaterialRegion::ResizeBase0_Half(MaterialRegion &editor) {editor.resizeBase0(-1, true);}
   void MaterialRegion::ResizeBase0_Original(MaterialRegion &editor) {editor.resizeBase0( 0, true);}
   void MaterialRegion::ResizeBase0_Double(MaterialRegion &editor) {editor.resizeBase0( 1, true);}
   void MaterialRegion::ResizeBase1_128(MaterialRegion &editor) {editor.resizeBase1(128);}
   void MaterialRegion::ResizeBase1_256(MaterialRegion &editor) {editor.resizeBase1(256);}
   void MaterialRegion::ResizeBase1_512(MaterialRegion &editor) {editor.resizeBase1(512);}
   void MaterialRegion::ResizeBase1_1024(MaterialRegion &editor) {editor.resizeBase1(1024);}
   void MaterialRegion::ResizeBase1_2048(MaterialRegion &editor) {editor.resizeBase1(2048);}
   void MaterialRegion::ResizeBase1_4096(MaterialRegion &editor) {editor.resizeBase1(4096);}
   void MaterialRegion::ResizeBase1_128x64(MaterialRegion &editor) {editor.resizeBase1(VecI2(128, 64));}
   void MaterialRegion::ResizeBase1_256x128(MaterialRegion &editor) {editor.resizeBase1(VecI2(256, 128));}
   void MaterialRegion::ResizeBase1_512x256(MaterialRegion &editor) {editor.resizeBase1(VecI2(512, 256));}
   void MaterialRegion::ResizeBase1_1024x512(MaterialRegion &editor) {editor.resizeBase1(VecI2(1024, 512));}
   void MaterialRegion::ResizeBase1_2048x1024(MaterialRegion &editor) {editor.resizeBase1(VecI2(2048, 1024));}
   void MaterialRegion::ResizeBase1_64x128(MaterialRegion &editor) {editor.resizeBase1(VecI2(64, 128));}
   void MaterialRegion::ResizeBase1_128x256(MaterialRegion &editor) {editor.resizeBase1(VecI2(128, 256));}
   void MaterialRegion::ResizeBase1_256x512(MaterialRegion &editor) {editor.resizeBase1(VecI2(256, 512));}
   void MaterialRegion::ResizeBase1_512x1024(MaterialRegion &editor) {editor.resizeBase1(VecI2(512, 1024));}
   void MaterialRegion::ResizeBase1_1024x2048(MaterialRegion &editor) {editor.resizeBase1(VecI2(1024, 2048));}
   void MaterialRegion::ResizeBase1_Quarter(MaterialRegion &editor) {editor.resizeBase1(-2, true);}
   void MaterialRegion::ResizeBase1_Half(MaterialRegion &editor) {editor.resizeBase1(-1, true);}
   void MaterialRegion::ResizeBase1_Original(MaterialRegion &editor) {editor.resizeBase1( 0, true);}
   void MaterialRegion::ResizeBase1_Double(MaterialRegion &editor) {editor.resizeBase1( 1, true);}
   void MaterialRegion::BumpFromCol(MaterialRegion &editor) {editor.bumpFromCol(-1);}
   void MaterialRegion::BumpFromCol2(MaterialRegion &editor) {editor.bumpFromCol( 2);}
   void MaterialRegion::BumpFromCol3(MaterialRegion &editor) {editor.bumpFromCol( 3);}
   void MaterialRegion::BumpFromCol4(MaterialRegion &editor) {editor.bumpFromCol( 4);}
   void MaterialRegion::BumpFromCol5(MaterialRegion &editor) {editor.bumpFromCol( 5);}
   void MaterialRegion::BumpFromCol6(MaterialRegion &editor) {editor.bumpFromCol( 6);}
   void MaterialRegion::BumpFromCol8(MaterialRegion &editor) {editor.bumpFromCol( 8);}
   void MaterialRegion::BumpFromCol12(MaterialRegion &editor) {editor.bumpFromCol(12);}
   void MaterialRegion::BumpFromCol16(MaterialRegion &editor) {editor.bumpFromCol(16);}
   void MaterialRegion::BumpFromCol24(MaterialRegion &editor) {editor.bumpFromCol(24);}
   void MaterialRegion::BumpFromCol32(MaterialRegion &editor) {editor.bumpFromCol(32);}
   void MaterialRegion::MulTexCol(MaterialRegion &editor) {Proj.mtrlMulTexCol  (editor.elm_id);}
   void MaterialRegion::MulTexRough(MaterialRegion &editor) {Proj.mtrlMulTexRough(editor.elm_id);}
   bool MaterialRegion::bigVisible()C {return visible() && big();}
   void   MaterialRegion::setRGB(C Vec            &rgb) {if(edit.color.xyz          !=rgb){undos.set("rgb1" ); edit.color.xyz          =rgb; edit.              color_time.getUTC(); setChanged(); toGui();}}
   void MaterialRegion::resetAlpha(                     ) {                                  undos.set("alpha"); edit.resetAlpha()           ;                                         setChanged(); toGui(); }
   void MaterialRegion::cull(bool              on ) {if(edit.cull               !=on ){undos.set("cull" ); edit.cull               =on ; edit.               cull_time.getUTC(); setChanged(); toGui();}}
   void MaterialRegion::flipNrmY(bool              on ) {if(edit.flip_normal_y      !=on ){undos.set("fny"  ); edit.flip_normal_y      =on ; edit.      flip_normal_y_time.getUTC(); rebuildBase(edit.baseTex(), true, false);}}
   void MaterialRegion::downsizeTexMobile(byte              ds ) {if(edit.downsize_tex_mobile!=ds ){undos.set("dtm"  ); edit.downsize_tex_mobile=ds ; edit.downsize_tex_mobile_time.getUTC(); setChanged(); toGui();}}
   void MaterialRegion::texQualityiOS(bool              q  ) {if(edit.high_quality_ios   !=q  ){undos.set("tq"   ); edit.high_quality_ios   =q  ; edit.   high_quality_ios_time.getUTC(); setChanged(); toGui();}}
   void MaterialRegion::resizeBase(C VecI2 &size, bool relative)
   {
      undos.set("resizeBase");
      TimeStamp time; time.getUTC();
      VecI2 sizes[2]={size, size};

      if(relative && size.any()) // if we want to have relative size and not original, then first revert to original size
         if(Proj.forceImageSize(edit.   color_map, 0, relative, edit.   color_map_time, time) // !! use '|' because all need to be processed !!
         |  Proj.forceImageSize(edit.   alpha_map, 0, relative, edit.   alpha_map_time, time)
         |  Proj.forceImageSize(edit.    bump_map, 0, relative, edit.    bump_map_time, time)
         |  Proj.forceImageSize(edit.  normal_map, 0, relative, edit.  normal_map_time, time)
         |  Proj.forceImageSize(edit.specular_map, 0, relative, edit.specular_map_time, time)
         |  Proj.forceImageSize(edit.    glow_map, 0, relative, edit.    glow_map_time, time))
      {
         MtrlImages mi; mi.fromMaterial(edit, Proj, false); mi.baseTextureSizes(&sizes[0], &sizes[1]); // calculate actual sizes
         REP(2)
         {
            sizes[i].set(Max(1, Shl(sizes[i].x, size.x)), Max(1, Shl(sizes[i].y, size.y)));
            sizes[i].set(NearestPow2(sizes[i].x), NearestPow2(sizes[i].y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
         }
         if(sizes[0]!=sizes[1])edit.separateBaseTexs(Proj, time);
         relative=false; // we now have the sizes known, so disable relative mode
      }

      if(Proj.forceImageSize(edit.   color_map,                                 sizes[0], relative, edit.   color_map_time, time) // !! use '|' because all need to be processed !!
      |  Proj.forceImageSize(edit.   alpha_map, edit.hasBase1Tex() ? sizes[1] : sizes[0], relative, edit.   alpha_map_time, time) // #MaterialTextureChannelOrder
      |  Proj.forceImageSize(edit.    bump_map,                                 sizes[0], relative, edit.    bump_map_time, time)
      |  Proj.forceImageSize(edit.  normal_map,                      sizes[1]           , relative, edit.  normal_map_time, time)
      |  Proj.forceImageSize(edit.specular_map,                      sizes[1]           , relative, edit.specular_map_time, time)
      |  Proj.forceImageSize(edit.    glow_map,                      sizes[1]           , relative, edit.    glow_map_time, time))
      {
         edit.cleanupMaps();
         rebuildBase(edit.baseTex());
      }
   }
   void MaterialRegion::resizeBase0(C VecI2 &size, bool relative)
   {
      undos.set("resizeBase");
      TimeStamp time; time.getUTC();
      VecI2 size0=size;

      if(relative ? true : (game && game->base_1 && game->base_1->size()!=size))edit.separateBaseTexs(Proj, time); // separate before reverting

      if(relative && size.any()) // if we want to have relative size and not original, then first revert to original size
         if(                       Proj.forceImageSize(edit.color_map, 0, relative, edit.color_map_time, time)  // !! use '|' because all need to be processed !!
         |                         Proj.forceImageSize(edit. bump_map, 0, relative, edit. bump_map_time, time)
         | (!edit.hasBase1Tex() && Proj.forceImageSize(edit.alpha_map, 0, relative, edit.alpha_map_time, time))) // resize alpha only if it's going to be placed in Base0 tex, #MaterialTextureChannelOrder
      {
         MtrlImages mi; mi.fromMaterial(edit, Proj, false); mi.baseTextureSizes(&size0, null); // calculate actual sizes
         size0.set(Max(1, Shl(size0.x, size.x)), Max(1, Shl(size0.y, size.y)));
         size0.set(NearestPow2(size0.x), NearestPow2(size0.y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
         relative=false; // we now have the sizes known, so disable relative mode
      }

      if(                       Proj.forceImageSize(edit.color_map, size0, relative, edit.color_map_time, time)  // !! use '|' because all need to be processed !!
      |                         Proj.forceImageSize(edit. bump_map, size0, relative, edit. bump_map_time, time)
      | (!edit.hasBase1Tex() && Proj.forceImageSize(edit.alpha_map, size0, relative, edit.alpha_map_time, time)) // resize alpha only if it's going to be placed in Base0 tex, #MaterialTextureChannelOrder
      )
      {
         edit.cleanupMaps();
         rebuildBase(edit.baseTex());
      }
   }
   void MaterialRegion::resizeBase1(C VecI2 &size, bool relative)
   {
      undos.set("resizeBase");
      TimeStamp time; time.getUTC();
      VecI2 size1=size;

      if(relative ? true : (game && game->base_0 && game->base_0->size()!=size))edit.separateBaseTexs(Proj, time); // separate before reverting

      if(relative && size.any()) // if we want to have relative size and not original, then first revert to original size
         if(                      Proj.forceImageSize(edit.  normal_map, 0, relative, edit.  normal_map_time, time)  // !! use '|' because all need to be processed !!
         |                        Proj.forceImageSize(edit.specular_map, 0, relative, edit.specular_map_time, time)
         |                        Proj.forceImageSize(edit.    glow_map, 0, relative, edit.    glow_map_time, time)
         | (edit.hasBase1Tex() && Proj.forceImageSize(edit.   alpha_map, 0, relative, edit.   alpha_map_time, time))) // resize alpha only if it's going to be placed in Base1 tex, #MaterialTextureChannelOrder
      {
         MtrlImages mi; mi.fromMaterial(edit, Proj, false); mi.baseTextureSizes(null, &size1); // calculate actual sizes
         size1.set(Max(1, Shl(size1.x, size.x)), Max(1, Shl(size1.y, size.y)));
         size1.set(NearestPow2(size1.x), NearestPow2(size1.y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
         relative=false; // we now have the sizes known, so disable relative mode
      }

      if(                      Proj.forceImageSize(edit.  normal_map, size1, relative, edit.  normal_map_time, time)  // !! use '|' because all need to be processed !!
      |                        Proj.forceImageSize(edit.specular_map, size1, relative, edit.specular_map_time, time)
      |                        Proj.forceImageSize(edit.    glow_map, size1, relative, edit.    glow_map_time, time)
      | (edit.hasBase1Tex() && Proj.forceImageSize(edit.   alpha_map, size1, relative, edit.   alpha_map_time, time)) // resize alpha only if it's going to be placed in Base1 tex, #MaterialTextureChannelOrder
      )
      {
         edit.cleanupMaps();
         rebuildBase(edit.baseTex());
      }
   }
   void MaterialRegion::bumpFromCol(int blur)
   {
      undos.set("bumpFromCol");
      uint base_tex=edit.baseTex(); // get current state of textures before making any change
      edit.bump_map=BumpFromColTransform(edit.color_map, blur); edit.bump_map_time.now();
      rebuildBase(base_tex);
   }
   EditMaterial& MaterialRegion::getEditMtrl() {return edit;}
   C ImagePtr    & MaterialRegion::getBase0() {return game->base_0;}
   C ImagePtr    & MaterialRegion::getBase1() {return game->base_1;}
   C ImagePtr    & MaterialRegion::getDetail() {return game->detail_map;}
   C ImagePtr    & MaterialRegion::getReflection() {return game->reflection_map;}
   C ImagePtr    & MaterialRegion::getMacro() {return game->macro_map;}
   C ImagePtr    & MaterialRegion::getLight() {return game->light_map;}
   void MaterialRegion::setBottom(C Rect &prop_rect)
   {
      reload_base_textures.rect(Rect_LU(0.10f, prop_rect.min.y-0.02f, 0.42f, 0.053f));
           texture_options.rect(Rect_LU(reload_base_textures.rect().ru(), reload_base_textures.rect().h()));
   }
   void MaterialRegion::create()
   {
      Gui+=::EE::Region::create(Rect_LU(0, 0, 0.73f, 1)).skin(&LightSkin, false).hide();
      flt w=rect().w()-slidebarSize(), e=0.01f, we=w-e*2, p=0.007f, h=0.05f, prop_height=0.044f;
        T+=big         .create(Rect_LU(e, 0, h*1.6f, h), "<<").focusable(false); big.mode=BUTTON_TOGGLE;
        T+=set_mtrl    .create(Rect_LU(big.rect().max.x+p, big.rect().max.y, h, h)).func(SetMtrl, T).focusable(false).desc("Enable this and click on the screen to set material at that location"); set_mtrl.mode=BUTTON_TOGGLE; set_mtrl.image="Gui/Misc/set.img";
        T+=undo        .create(Rect_LU(set_mtrl.rect().ru()+Vec2(p, 0), 0.05f, 0.05f )).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
        T+=redo        .create(Rect_LU(undo.rect().ru()               , 0.05f, 0.05f )).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
        T+=locate      .create(Rect_L (redo.rect().right() +Vec2(p, 0), 0.11f, 0.043f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
        T+=close       .create(Rect_RU(w-e, 0, h, h)).func(Hide, T); close.image="Gui/close.img"; close.skin=&EmptyGuiSkin;
        T+=preview_mode.create(Rect   (locate.rect().max.x+p, redo.rect().min.y, close.rect().min.x-p, close.rect().max.y), 0, (cchar**)null, Elms(preview_mesh)).valid(true).set(0);
        T+=preview     .create(Rect_LU(big.rect().ld()-Vec2(0, 0.007f), we, we), DrawPreview); preview.fov=PreviewFOV; preview.user=this;
      Gui+=preview_big .create(DrawPreview).hide(); preview_big.fov=preview.fov; preview_big.user=preview.user;
        T+=sub         .create(Rect_LU(0, 0, rect().w(), 1)).skin(&EmptyGuiSkin, false); sub.kb_lit=false;
      preview_mode.tab(0).setImage("Gui/Misc/circle.img");
      preview_mode.tab(1).setImage("Gui/Misc/box.img");
      preview_mode.tab(2).setImage("Gui/Misc/tube.img");
      preview_mode.tab(3).setImage("Gui/Misc/grid_small.img");

      Property &tech=props.New().create("Technique", MemberDesc(DATA_INT).setFunc(Tech, Tech)).setEnum();
      ListColumn tech_lc[]=
      {
         ListColumn(MEMBER(MaterialTech, name), LCW_DATA, "name"),
      };
      tech.combobox.setColumns(tech_lc, Elms(tech_lc)).setData(mtrl_techs, Elms(mtrl_techs)); tech.combobox.menu.list.setElmDesc(MEMBER(MaterialTech, desc));

      /*Property &max_tex_size=props.New().create("Max Image Size", MemberDesc(DATA_INT).setFunc(MaxTexSize, MaxTexSize)).setEnum();
      ListColumn mts_lc[]=
      {
         ListColumn(MEMBER(.MaxTexSize, name), LCW_DATA, "name"),
      };
      max_tex_size.combobox.setColumns(mts_lc, Elms(mts_lc)).setData(max_tex_sizes, Elms(max_tex_sizes)); max_tex_size.combobox.menu.list.setElmDesc(MEMBER(.MaxTexSize, desc)); */

  red=&props.New().create("Red"  , MemberDesc(DATA_REAL).setFunc(Red  , Red  )).range(0, 4).mouseEditSpeed(0.4f);
green=&props.New().create("Green", MemberDesc(DATA_REAL).setFunc(Green, Green)).range(0, 4).mouseEditSpeed(0.4f);
 blue=&props.New().create("Blue" , MemberDesc(DATA_REAL).setFunc(Blue , Blue )).range(0, 4).mouseEditSpeed(0.4f);
alpha=&props.New().create("Alpha", MemberDesc(DATA_REAL).setFunc(Alpha, Alpha)).range(0, 1);
    //props.New();
      props.New().create("Bump"           , MemberDesc(DATA_REAL).setFunc(Bump    , Bump    )).range(0, 1);
      props.New().create("Normal"         , MemberDesc(DATA_REAL).setFunc(NrmScale, NrmScale)).range(0, 2);
      props.New().create("Flip Normal Y"  , MemberDesc(DATA_BOOL).setFunc(FNY     , FNY     ));
    //props.New();
      props.New().create("Specular"       , MemberDesc(DATA_REAL).setFunc(Spec, Spec)).range(0, 3);
      props.New().create("Glow"           , MemberDesc(DATA_REAL).setFunc(Glow, Glow)).range(0, 1);
      props.New().create("Subsurf Scatter", MemberDesc(DATA_REAL).setFunc(SSS , SSS )).range(0, 1);
      props.New().create("Detail Scale"   , MemberDesc(DATA_REAL).setFunc(DetScale, DetScale)).range(0.01f, 1024).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      props.New().create("Detail Power"   , MemberDesc(DATA_REAL).setFunc(DetPower, DetPower)).range(0, 1);
      props.New();
      props.New();
      props.New().create("Reflection", MemberDesc(DATA_REAL).setFunc(Reflect, Reflect)).range(0, 2);
      props.New();
      props.New();
      props.New();
      props.New();

      props.New().create("Cull"           , MemberDesc(DATA_BOOL).setFunc(Cull    , Cull    ));
      props.New().create("Ambient Red"    , MemberDesc(DATA_REAL).setFunc(AmbR    , AmbR    )).range(0, 1);
      props.New().create("Ambient Green"  , MemberDesc(DATA_REAL).setFunc(AmbG    , AmbG    )).range(0, 1);
      props.New().create("Ambient Blue"   , MemberDesc(DATA_REAL).setFunc(AmbB    , AmbB    )).range(0, 1);
      props.New().create("UV Scale"       , MemberDesc(DATA_REAL).setFunc(TexScale, TexScale)).range(0.01f, 1024).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
Property &mts=props.New().create("Mobile Tex Size", MemberDesc(DATA_INT ).setFunc(DownsizeTexMobile, DownsizeTexMobile)).setEnum(DownsizeTexMobileText, Elms(DownsizeTexMobileText)).desc("If Downsize Textures when making Applications for Mobile platforms");
Property &tqi=props.New().create("iOS Tex Quality", MemberDesc(DATA_BOOL).setFunc(TexQualityiOS    , TexQualityiOS    )).setEnum(    TexQualityiOSText, Elms(    TexQualityiOSText)).desc("Select Texture Quality for iOS platform.\n4-bit per pixel PVRTC format offers higher quality, however it makes the texture use 2x more memory than 2-bit PVRTC.");

      ts.reset().size=0.038f; ts.align.set(1, 0);
      Rect prop_rect=AddProperties(props, sub, 0, prop_height, 0.145f, &ts); REPAO(props).autoData(this).changed(Changed, PreChanged);
      sub+=brightness.create(Rect_RU(red->textline.rect().left(), red->button.rect().w(), prop_height*2)).func(RGB, T).focusable(false).subType(BUTTON_TYPE_PROPERTY_VALUE); brightness.mode=BUTTON_CONTINUOUS;
      tech.combobox.resize(Vec2(0.27f, 0)); // increase size
      mts .combobox.resize(Vec2(0.15f, 0)); // increase size
      tqi .combobox.resize(Vec2(0.15f, 0)); // increase size

      flt tex_size=prop_height*3; int i=-1;
      sub+=texs.New().create(TEX_COLOR   , MEMBER(EditMaterial,      color_map), MEMBER(EditMaterial,      color_map_time), Rect_LU(prop_rect.ru()+Vec2(e           , i*prop_height), tex_size, tex_size), "Color"         , T);
      sub+=texs.New().create(TEX_ALPHA   , MEMBER(EditMaterial,      alpha_map), MEMBER(EditMaterial,      alpha_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*1, i*prop_height), tex_size, tex_size), "Alpha"         , T); i-=3;
      sub+=texs.New().create(TEX_BUMP    , MEMBER(EditMaterial,       bump_map), MEMBER(EditMaterial,       bump_map_time), Rect_LU(prop_rect.ru()+Vec2(e           , i*prop_height), tex_size, tex_size), "Bump"          , T);
      sub+=texs.New().create(TEX_NORMAL  , MEMBER(EditMaterial,     normal_map), MEMBER(EditMaterial,     normal_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*1, i*prop_height), tex_size, tex_size), "Normal"        , T); i-=3;
      sub+=texs.New().create(TEX_SPEC    , MEMBER(EditMaterial,   specular_map), MEMBER(EditMaterial,   specular_map_time), Rect_LU(prop_rect.ru()+Vec2(e           , i*prop_height), tex_size, tex_size), "Spec"          , T);
      sub+=texs.New().create(TEX_GLOW    , MEMBER(EditMaterial,       glow_map), MEMBER(EditMaterial,       glow_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*1, i*prop_height), tex_size, tex_size), "Glow"          , T); i-=3;
      sub+=texs.New().create(TEX_MACRO   , MEMBER(EditMaterial,      macro_map), MEMBER(EditMaterial,      macro_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*1, i*prop_height), tex_size, tex_size), "Macro"         , T);
      sub+=texs.New().create(TEX_DET_COL , MEMBER(EditMaterial,   detail_color), MEMBER(EditMaterial,     detail_map_time), Rect_LU(prop_rect.ru()+Vec2(e           , i*prop_height), tex_size, tex_size), "Detail\nColor" , T); i-=3;
      sub+=texs.New().create(TEX_DET_BUMP, MEMBER(EditMaterial,    detail_bump), MEMBER(EditMaterial,     detail_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*0, i*prop_height), tex_size, tex_size), "Detail\nBump"  , T);
      sub+=texs.New().create(TEX_DET_NRM , MEMBER(EditMaterial,  detail_normal), MEMBER(EditMaterial,     detail_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*1, i*prop_height), tex_size, tex_size), "Detail\nNormal", T); i-=3;
      sub+=texs.New().create(TEX_RFL_ALL , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(e-tex_size*3, i*prop_height), tex_size, tex_size), "Reflect\nAll"  , T);
      sub+=texs.New().create(TEX_RFL_L   , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(e-tex_size*2, i*prop_height), tex_size, tex_size), "Reflect\nLeft" , T);
      sub+=texs.New().create(TEX_RFL_F   , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(e-tex_size*1, i*prop_height), tex_size, tex_size), "Reflect\nFront", T);
      sub+=texs.New().create(TEX_RFL_R   , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*0, i*prop_height), tex_size, tex_size), "Reflect\nRight", T);
      sub+=texs.New().create(TEX_RFL_B   , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*1, i*prop_height), tex_size, tex_size), "Reflect\nBack" , T); i-=3;
      sub+=texs.New().create(TEX_RFL_D   , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*0, i*prop_height), tex_size, tex_size), "Reflect\nDown" , T);
      sub+=texs.New().create(TEX_RFL_U   , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(e+tex_size*1, i*prop_height), tex_size, tex_size), "Reflect\nUp"   , T); i-=3;
      sub+=texs.New().create(TEX_LIGHT   , MEMBER(EditMaterial,      light_map), MEMBER(EditMaterial,      light_map_time), Rect_LU(prop_rect.ru()+Vec2(e           , i*prop_height), tex_size, tex_size), "Light"         , T);
      REPA(texs)sub+=texs[i].remove;

      sub+=reload_base_textures.create("Reload Base Textures").func(ReloadBaseTextures, T).desc("Reload base textures, such as Color, Alpha, Bump, Normal, Specular and Glow, from their original source files.");
      Node<MenuElm> n;
      n.New().create(auto_reload_name, AutoReload, T).flag(MENU_TOGGLABLE).setOn(auto_reload).desc("If this is enabled then base textures will be instantly reloaded when changing them.\nIf you only want to change the source file paths, without actually reloading the textures, then you can disable this option first.");
      {
         Node<MenuElm> &resize=(n+="Resize Base Textures"); resize.desc("This allows to resize the base textures, such as Color, Alpha, Bump, Normal, Specular and Glow to a custom size.");
         resize.New().create( "128x128" , ResizeBase128 , T);
         resize.New().create( "256x256" , ResizeBase256 , T);
         resize.New().create( "512x512" , ResizeBase512 , T);
         resize.New().create("1024x1024", ResizeBase1024, T);
         resize.New().create("2048x2048", ResizeBase2048, T);
         resize.New().create("4096x4096", ResizeBase4096, T);
         Node<MenuElm> &other=(resize+="Other");
         
         other.New().create("128x64"   , ResizeBase128x64, T);
         other.New().create("256x128"  , ResizeBase256x128, T);
         other.New().create("512x256"  , ResizeBase512x256, T);
         other.New().create("1024x512" , ResizeBase1024x512, T);
         other.New().create("2048x1024", ResizeBase2048x1024, T);
         other++;
         other.New().create("64x128"   , ResizeBase64x128, T);
         other.New().create("128x256"  , ResizeBase128x256, T);
         other.New().create("256x512"  , ResizeBase256x512, T);
         other.New().create("512x1024" , ResizeBase512x1024, T);
         other.New().create("1024x2048", ResizeBase1024x2048, T);
         
         resize++;
         resize.New().create("Quarter" , ResizeBaseQuarter , T);
         resize.New().create("Half"    , ResizeBaseHalf    , T);
         resize.New().create("Original", ResizeBaseOriginal, T);
         resize.New().create("Double"  , ResizeBaseDouble  , T);
      }
      {
         Node<MenuElm> &resize=(n+="Resize Base 0 Textures"); resize.desc("This allows to resize the base 0 textures, such as Color and Alpha to a custom size.");
         resize.New().create( "128x128" , ResizeBase0_128 , T);
         resize.New().create( "256x256" , ResizeBase0_256 , T);
         resize.New().create( "512x512" , ResizeBase0_512 , T);
         resize.New().create("1024x1024", ResizeBase0_1024, T);
         resize.New().create("2048x2048", ResizeBase0_2048, T);
         resize.New().create("4096x4096", ResizeBase0_4096, T);
         Node<MenuElm> &other=(resize+="Other");
         
         other.New().create("128x64"   , ResizeBase0_128x64, T);
         other.New().create("256x128"  , ResizeBase0_256x128, T);
         other.New().create("512x256"  , ResizeBase0_512x256, T);
         other.New().create("1024x512" , ResizeBase0_1024x512, T);
         other.New().create("2048x1024", ResizeBase0_2048x1024, T);
         other++;
         other.New().create("64x128"   , ResizeBase0_64x128, T);
         other.New().create("128x256"  , ResizeBase0_128x256, T);
         other.New().create("256x512"  , ResizeBase0_256x512, T);
         other.New().create("512x1024" , ResizeBase0_512x1024, T);
         other.New().create("1024x2048", ResizeBase0_1024x2048, T);
         
         resize++;
         resize.New().create("Quarter" , ResizeBase0_Quarter , T);
         resize.New().create("Half"    , ResizeBase0_Half    , T);
         resize.New().create("Original", ResizeBase0_Original, T);
         resize.New().create("Double"  , ResizeBase0_Double  , T);
      }
      {
         Node<MenuElm> &resize=(n+="Resize Base 1 Textures"); resize.desc("This allows to resize the base 1 textures, such as Alpha, Normal, Specular and Glow to a custom size.");
         resize.New().create( "128x128" , ResizeBase1_128 , T);
         resize.New().create( "256x256" , ResizeBase1_256 , T);
         resize.New().create( "512x512" , ResizeBase1_512 , T);
         resize.New().create("1024x1024", ResizeBase1_1024, T);
         resize.New().create("2048x2048", ResizeBase1_2048, T);
         resize.New().create("4096x4096", ResizeBase1_4096, T);
         Node<MenuElm> &other=(resize+="Other");
         
         other.New().create("128x64"   , ResizeBase1_128x64, T);
         other.New().create("256x128"  , ResizeBase1_256x128, T);
         other.New().create("512x256"  , ResizeBase1_512x256, T);
         other.New().create("1024x512" , ResizeBase1_1024x512, T);
         other.New().create("2048x1024", ResizeBase1_2048x1024, T);
         other++;
         other.New().create("64x128"   , ResizeBase1_64x128, T);
         other.New().create("128x256"  , ResizeBase1_128x256, T);
         other.New().create("256x512"  , ResizeBase1_256x512, T);
         other.New().create("512x1024" , ResizeBase1_512x1024, T);
         other.New().create("1024x2048", ResizeBase1_1024x2048, T);
         
         resize++;
         resize.New().create("Quarter" , ResizeBase1_Quarter , T);
         resize.New().create("Half"    , ResizeBase1_Half    , T);
         resize.New().create("Original", ResizeBase1_Original, T);
         resize.New().create("Double"  , ResizeBase1_Double  , T);
      }
      {
         Node<MenuElm> &bump=(n+="Set Bump from Color");
       //bump.New().create("Blur Auto", BumpFromCol  , T);
         bump.New().create("Blur 2"   , BumpFromCol2 , T);
         bump.New().create("Blur 3"   , BumpFromCol3 , T);
         bump.New().create("Blur 4"   , BumpFromCol4 , T);
         bump.New().create("Blur 5"   , BumpFromCol5 , T);
         bump.New().create("Blur 6"   , BumpFromCol6 , T);
         bump.New().create("Blur 8"   , BumpFromCol8 , T);
         bump.New().create("Blur 12"  , BumpFromCol12, T);
         bump.New().create("Blur 16"  , BumpFromCol16, T);
         bump.New().create("Blur 24"  , BumpFromCol24, T);
         bump.New().create("Blur 32"  , BumpFromCol32, T);
      }
      {
         Node<MenuElm> &extra=(n+="Extra");
         extra.New().create("Multiply Color Texture by Color"     , MulTexCol  , T);
         extra.New().create("Multiply Normal Texture by Roughness", MulTexRough, T);
      }
      sub+=texture_options.create().setData(n); texture_options.flag|=COMBOBOX_CONST_TEXT;

      setBottom(prop_rect);

      preview_mesh[0].create(1).parts[0].base.create(Ball(0.42f      ), VTX_NRM|VTX_TAN|VTX_TEX0, 16);
      preview_mesh[1].create(1).parts[0].base.create(Box (0.42f/SQRT3), VTX_NRM|VTX_TAN|VTX_TEX0    );
      preview_mesh[2].create(1).parts[0].base.create(Tube(0.12f, 0.75f), VTX_NRM|VTX_TAN|VTX_TEX0, 16);
      preview_mesh[3].create(1).parts[0].base.createPlane(2, 2       , VTX_NRM|VTX_TAN|VTX_TEX0    ).transform(Matrix().setRotateX(PI_2).move(-0.5f, 0, -0.5f).scale(16)).texScale(16);
      REPAO(preview_mesh).setRender().setBox();
      preview_cam.setFromAt(Vec(0, 0, -1), VecZero);
   }
   Image* MaterialRegion::getDetailBump(C Str &file)
   {
      if(!EqualPath(file, detail_bump_file))
      {
         detail_bump_file=file;
         Image temp; Proj.loadImage(temp, detail_bump_file);
         temp.copyTry(detail_bump, Min(temp.w(), 128), Min(temp.h(), 128), 1, -1, IMAGE_2D, 1, FILTER_LINEAR); // we only need a preview, so make it small, no mip maps, and fast filtering
      }
      return detail_bump.is() ? &detail_bump : null;
   }
   void MaterialRegion::resize()
   {
          rect(Rect_RU(D.w(), D.h(), rect().w(), D.h()*2));
      sub.rect(Rect(0.01f, -rect().h(), rect().w(), preview.rect().min.y-0.01f));
      sub.virtualSize(&(sub.childrenSize()+Vec2(0, 0.02f))); // add a bit of padding
      if(!sub.slidebar[1].visible())move(Vec2(slidebarSize(), 0));
      preview_big.rect(EditRect(false));
   }
   void MaterialRegion::toGui()
   {
      REPAO(props).toGui();
      REPAO(texs ).toGui();
   }
           void MaterialRegion::flush(C UID &elm_id) {if(T.elm_id==elm_id)flush();}
   void MaterialRegion::flush()
   {
      if(elm && game && changed)
      {
         if(ElmMaterial *data=elm->mtrlData()){data->newVer(); data->from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(elm_id)); edit.copyTo(*game, Proj);
         Save(*game, Proj.gamePath(elm_id)); Proj.savedGame(*elm);
         Proj.mtrlSetAutoTanBin(elm->id);
         Server.setElmLong(elm->id);
         if(saved.downsize_tex_mobile!=edit.downsize_tex_mobile)Proj.mtrlDownsizeTexMobile(elm_id, edit.downsize_tex_mobile); // upon flushing set all materials with same textures to the same 'downsize_tex_mobile'
         if(saved.high_quality_ios   !=edit.high_quality_ios   )Proj.mtrlTexQualityiOS    (elm_id, edit.high_quality_ios   ); // upon flushing set all materials with same textures to the same 'high_quality_ios'
         saved=edit;
      }
      changed=false;
   }
   void MaterialRegion::setChanged()
   {
      if(elm && game)
      {
         changed=true;
         if(ElmMaterial *data=elm->mtrlData()){data->newVer(); data->from(edit);}
         edit.copyTo(*game, Proj);
      }
   }
   void MaterialRegion::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_MTRL)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         if(elm)game=     Proj.gamePath( elm->id) ;else game=&temp;
         if(elm)edit.load(Proj.editPath(*elm   ));else edit.reset(); saved=edit;
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         toGui();
         Proj.refresh(false, false);
         if(!elm)
         {
            MaterialRegion &other=((this==&MtrlEdit) ? WaterMtrlEdit : MtrlEdit);
            hide(); if(other.elm)other.show();
         }
      }
   }
   void MaterialRegion::activate(Elm *elm)
   {
      set(elm); if(T.elm)
      {
         MaterialRegion &other=((this==&MtrlEdit) ? WaterMtrlEdit : MtrlEdit);
         show(); other.hide();
      }
   }
   void MaterialRegion::toggle(Elm *elm)
   {
      if(elm==T.elm && visible())elm=null; activate(elm);
   }
           void            MaterialRegion::hideBig()         {if(bigVisible())big.push();}
   MaterialRegion& MaterialRegion::show(){if(hidden ()){::EE::GuiObj::show();                                           resize(); CodeEdit.resize();} return T;}
   MaterialRegion& MaterialRegion::hide(){if(visible()){::EE::GuiObj::hide(); preview_big.hide(); REPAO(props).close(); resize(); CodeEdit.resize();} return T;}
   void MaterialRegion::set(C MaterialPtr &mtrl) {activate(Proj.findElm(mtrl.id()));}
   void MaterialRegion::set(Memt<MaterialPtr> mtrls)
   {
      REPD(i, mtrls.elms())
      {
         if(!mtrls[i])mtrls.remove(i, true); // remove if null
         else         REPD(j, i)if(mtrls[i]==mtrls[j]){mtrls.remove(i, true); break;} // remove duplicates
      }
      FREPA(mtrls)if(mtrls[i]==game){set(mtrls[(i+1)%mtrls.elms()]); return;} // activate next
                     if(mtrls.elms())set(mtrls[0]); // activate first
   }
   void MaterialRegion::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))
      {
         FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(elm->type==elm_type)
         {
            activate(elm);
            break;
         }else
         if(elm->type==ELM_IMAGE)
         {
            REPA(texs)if(texs[i].contains(focus_obj))
            {
               texs[i].setFile(EncodeFileName(elm->id));
               break;
            }
            break;
         }
         elms.clear(); // processed
      }
   }
   int MaterialRegion::Compare(C ImageSource &a, C ImageSource &b) {return ::Compare(a.order, b.order);}
   void MaterialRegion::drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))REPA(texs)if(texs[i].contains(focus_obj))
      {
         Memc<ImageSource> images; FREPA(names)if(ExtType(GetExt(names[i]))==EXT_IMAGE)images.New().name=CodeEdit.importPaths(names[i]);
         if(images.elms()>1)REPA(images) // detect if there are any special maps
         {
            ImageSource &image=images[i];
            Str base=GetBaseNoExt(image.name);
            if(Contains(base, "ao", false, true) || Contains(base, "AO", true) || Contains(base, "occlusion")){image.order=1; image.params.New().set("mode", "mul"  );}else // AO
            if(Contains(base, "illumination") || Contains(base, "glow") || Contains(base, "emissive")        ){image.order=2; image.params.New().set("mode", "blend");}     // glow
         }
         images.sort(Compare); // sort by order
         texs[i].setFile(Edit::FileParams::Encode(SCAST(Memc<Edit::FileParams>, images)));
         break;
      }
   }
   void MaterialRegion::rebuildBase(uint old_base_tex, bool changed_flip_normal_y, bool adjust_params, bool always)
   {
      if(elm && game)
      {
         bool want_tan_bin=game->wantTanBin();

         uint new_base_tex;
         if(auto_reload || always)
         {
            new_base_tex=Proj.mtrlCreateBaseTextures(edit, changed_flip_normal_y); // set precise
            Time.skipUpdate(); // compressing textures can be slow
         }else
         {
            new_base_tex=edit.baseTex(); // set approximate
         }
         setChanged();
         Proj.mtrlTexChanged();

         if(adjust_params)AdjustMaterialParams(edit, *game, old_base_tex, new_base_tex);
         D.setShader(game());
         toGui();

         if(want_tan_bin!=game->wantTanBin())Proj.mtrlSetAutoTanBin(elm->id);
      }
   }
   void MaterialRegion::rebuildReflection()
   {
      if(elm && game)
      {
         Proj.mtrlCreateReflectionTexture(edit);
         setChanged();
         Proj.mtrlTexChanged();
         Time.skipUpdate(); // compressing textures can be slow

         D.setShader(game());
         toGui();
      }
   }
   void MaterialRegion::rebuildDetail()
   {
      if(elm && game)
      {
         bool want_tan_bin=game->wantTanBin();

         Proj.mtrlCreateDetailTexture(edit);
         setChanged();
         Proj.mtrlTexChanged();
         Time.skipUpdate(); // compressing textures can be slow

         D.setShader(game());
         toGui();

         if(want_tan_bin!=game->wantTanBin())Proj.mtrlSetAutoTanBin(elm->id);
      }
   }
   void MaterialRegion::rebuildMacro()
   {
      if(elm && game)
      {
         Proj.mtrlCreateMacroTexture(edit);
         setChanged();
         Proj.mtrlTexChanged();
         Time.skipUpdate(); // compressing textures can be slow

         D.setShader(game());
         toGui();
      }
   }
   void MaterialRegion::rebuildLight()
   {
      if(elm && game)
      {
         Proj.mtrlCreateLightTexture(edit);
         setChanged();
         Proj.mtrlTexChanged();
         Time.skipUpdate(); // compressing textures can be slow

         D.setShader(game());
         toGui();
      }
   }
   void MaterialRegion::elmChanged(C UID &mtrl_id)
   {
      if(elm && elm->id==mtrl_id)
      {
         undos.set(null, true);
         EditMaterial temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp)){edit.copyTo(*game, Proj); toGui();}
      }
   }
   void MaterialRegion::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
   bool MaterialRegion::winIOContains(GuiObj *go)C {REPA(texs)if(texs[i].win_io.contains(go))return true; return false;}
   void MaterialRegion::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      preview_big.visible(bigVisible());
      if(Gui.ms()==&preview || Gui.ms()==&preview_big)
      {
         bool rot_cam=(Ms.b(0) || Ms.b(4)), rot_light=Ms.b(1);
         preview_cam.transformByMouse(min_zoom, max_zoom, (rot_cam ? CAMH_ROT : 0)|CAMH_ZOOM);
         if(rot_light)light_angle+=Ms.d()*Vec2(-1,  1);
         if(rot_cam || rot_light)Ms.freeze();
         if(Ms.bd(0))big.push();
      }
      if(Ms.bp(2))
      {
         if(contains(Gui.ms()))set(null);else
         if(Gui.ms()==&preview_big)big.push();
      }
      REPA(Touches)if(Touches[i].guiObj()==&preview || Touches[i].guiObj()==&preview_big)if(Touches[i].on()){preview_cam.yaw-=Touches[i].ad().x*2.0f; preview_cam.pitch+=Touches[i].ad().y*2.0f; preview_cam.setSpherical();}
      if(visible() && Kb.k.k && (contains(Gui.kb()) || contains(Gui.ms()) || preview_big.contains(Gui.ms()) || winIOContains(Gui.ms())))
      {
         KbSc prev(KB_PGUP, KBSC_CTRL_CMD|KBSC_REPEAT),
              next(KB_PGDN, KBSC_CTRL_CMD|KBSC_REPEAT);
         if(prev.pd()){prev.eat(); Proj.elmNext(elm_id, -1);}else
         if(next.pd()){next.eat(); Proj.elmNext(elm_id    );}
      }
   }
MaterialRegion::MaterialRegion() : elm_type(ELM_MTRL), auto_reload(true), min_zoom(0.48f), max_zoom(3), mouse_edit_delta(0), mouse_edit_value(0), light_angle(PI_4), red(null), green(null), blue(null), alpha(null), game(&temp), elm_id(UIDZero), elm(null), changed(false), undos(true) {}

MaterialRegion::Texture::Texture() : mr(null) {}

MaterialRegion::ImageSource::ImageSource() : order(0) {}

/******************************************************************************/
