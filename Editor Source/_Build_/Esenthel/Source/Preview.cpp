/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
flt GetDist(C Box &box)
{
   Box b=box-box.center(); // center
   b|=b*Matrix3().setRotateY(PI_4); // include the box when it's rotated at 45 deg
   Vec size=b.size();
   flt dist= size.y/(2*Tan(D.viewFovY()/2));
   MAX(dist, size.x/(2*Tan(D.viewFovX()/2)));
   MAX(dist, size.z/(2*Tan(D.viewFovX()/2)));
   return dist;
}
void SetCam(Camera &cam, C Box &box, flt yaw, flt pitch, flt zoom) // require custom 'cam' to calculate correct velocities
{
   cam.yaw=yaw;
   cam.pitch=pitch;
   cam.roll=0;
   cam.dist=GetDist(box)*zoom;
   cam.at=box.center();
   cam.setSpherical().updateVelocities(CAM_ATTACH_FREE).set();
}
/******************************************************************************/
PreviewClass Preview;
/******************************************************************************/

/******************************************************************************/
   void PreviewClass::create()
   {
      Gui+=::EE::Window::create().level(1); // set level so it will be displayed above other editor windows
      ball.create(1).parts[0].base.create(Ball(1), VTX_TEX0|VTX_NRM_TAN_BIN, 16);
      ball.setRender().setBox();
      objs_container.create(S);
   }
   void PreviewClass::Render() {Preview.render();}
          void PreviewClass::render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            switch(elm_type)
            {
               case ELM_OBJ :
               case ELM_MESH: if(mesh)
               {
                  SetVariation(mesh_variation); mesh->MeshLod::draw(MatrixIdentity);
                  SetVariation();
               }break;

               case ELM_MTRL: ball.draw(MatrixIdentity); break;

               case ELM_ANIM: if(mesh)mesh->MeshLod::draw(anim_skel); break;
            }

            if(elm_type==ELM_WATER_MTRL)LightDir(Matrix3().setRotateXY(PI_4, PI_4).z                              , 1-D.ambientColor()).add(false);
            else                        LightDir(!(ActiveCam.matrix.z*2 + ActiveCam.matrix.x - ActiveCam.matrix.y), 1-D.ambientColor()).add(false);
         }break;

         case RM_BLEND:
         case RM_PALETTE:
         case RM_PALETTE1:
         {
            // particle
         }break;
      }
      if(Renderer()==RM_BLEND && elm_type==ELM_OBJ   && body)body->drawBlend(MatrixIdentity, VecZero, VecZero, &Vec4(1, 1, 1, 0.32f));
      if(Renderer()==RM_CLOUD && elm_type==ELM_IMAGE && image && image->mode()==IMAGE_3D)
      {
         image->drawVolume(WHITE, TRANSPARENT, OBox(Box(1)), 0.05f);
      }
   }
   void PreviewClass::closeElm(C UID &elm_id)
   {
      if(video_id==elm_id){video.del(); video_id.zero();}
   }
   void PreviewClass::elmChanged(C UID &elm_id)
   {
      if(elm_id==objs_last_id)objs_last_id.zero();
      if(elm_id==    sound_id)    sound_id.zero();
      if(elm_id==    video_id)    video_id.zero();
      if(elm_id== mini_map_id) mini_map_id.zero();
   }
   void PreviewClass::clearProj()
   {
          sound_id.zero();
          video_id.zero(); video.del();
       mini_map_id.zero();
      objs_last_id.zero();
   }
   void PreviewClass::drawBack(C Rect &rect)
   {
      if(GuiSkin *skin=Gui.skin())
      {
         if(skin->region.normal        ){skin->region.normal->draw(skin->region.normal_color, rect); return;}
         if(skin->region.normal_color.a){               rect.draw(skin->region.normal_color      ); return;}
      }
      rect.draw(Gui.backgroundColor(), true );
      rect.draw(Gui.    borderColor(), false);
   }
   void PreviewClass::draw(Elm &elm, C Rect &rect)
   {
      CurrentEnvironment().set();
      AMBIENT_MODE ambient  =D.  ambientMode(); D.  ambientMode(AMBIENT_FLAT);
      DOF_MODE     dof      =D.      dofMode(); D.      dofMode(    DOF_NONE);
      bool         eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(       false);
      bool         astros   =AstrosDraw       ; AstrosDraw     =false;
      bool         ocean    =Water.draw       ; Water.draw     =false;
      Camera c=ActiveCam; Rect r=D.viewRect(); flt fov=D.viewFov(), from=D.viewFrom(), range=D.viewRange(); // remember

      Color rect_color=Gui.borderColor();
      mesh_variation=0;
      switch(elm_type=elm.type)
      {
         case ELM_OBJ:
         {
            if(ElmObj *obj_data=elm.objData())
            if(obj=Proj.gamePath(elm.id))
            {
               UID body_id=UIDZero; if(Elm *mesh_elm=Proj.findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh_elm->meshData())body_id=mesh_data->body_id;
               mesh=obj->mesh(); // take this from "obj->mesh" instead of "Proj.gamePath(obj_data.mesh_id)" in case object has its mesh taken from the base
               mesh_variation=obj->meshVariationIndex();
               body=Proj.gamePath(body_id);

               D.viewRect(rect).viewFov(PreviewFOV);
               flt dist=Max(0.1f, mesh ? GetDist(mesh->ext) : 0);
               D.viewFrom(dist*0.01f).viewRange(dist*5); if(mesh)SetCam(cam, mesh->ext, Time.appTime(), (mesh->ext.size().y<mesh->ext.size().max()*0.3f) ? -PI_4 : 0); // set
               Renderer(PreviewClass::Render);
               D.viewRect(r).viewFov(fov).viewFrom(from).viewRange(range); c.set(); // restore
               rect.draw(rect_color, false);
            }
         }break;

         case ELM_MESH:
         {
            mesh=Proj.gamePath(elm.id);

            D.viewRect(rect).viewFov(PreviewFOV);
            flt dist=Max(0.1f, mesh ? GetDist(mesh->ext) : 0);
            D.viewFrom(dist*0.01f).viewRange(dist*5); if(mesh)SetCam(cam, mesh->ext, Time.appTime(), (mesh->ext.size().y<mesh->ext.size().max()*0.3f) ? -PI_4 : 0); // set
            Renderer(PreviewClass::Render);
            D.viewRect(r).viewFov(fov).viewFrom(from).viewRange(range); c.set(); // restore
            rect.draw(rect_color, false);
         }break;

         case ELM_MTRL:
         {
            mtrl=Proj.gamePath(elm.id);
            ball.material(mtrl).setShader();

            D.viewRect(rect).viewFov(PreviewFOV);
            flt dist=GetDist(ball.ext);
            D.viewFrom(dist*0.01f).viewRange(dist*3); SetCam(cam, ball.ext, Time.appTime()/2); // set
            Renderer(PreviewClass::Render);
            D.viewRect(r).viewFov(fov).viewFrom(from).viewRange(range); c.set(); // restore
            rect.draw(rect_color, false);
         }break;

         case ELM_WATER_MTRL:
         {
            WaterMtrl temp; Swap(temp, SCAST(WaterMtrl, Water)); PlaneM water_plane=Water.plane; Water.draw=true; Water.plane.set(Vec(0, -1, 0), Vec(0, 1, 0));
            if(water_mtrl=Proj.gamePath(elm.id))SCAST(WaterMtrl, Water)=*water_mtrl;

            D.viewRect(rect).viewFov(PreviewFOV).viewFrom(0.1f).viewRange(100); SetCam(cam, Box(5), 0, -PI_6); // set
            Renderer(PreviewClass::Render);
            D.viewRect(r).viewFov(fov).viewFrom(from).viewRange(range); c.set(); // restore
            Swap(temp, SCAST(WaterMtrl, Water)); Water.plane=water_plane;

            rect.draw(rect_color, false);
         }break;

         case ELM_ANIM:
         {
            UID mesh_id=Proj.animToMesh(&elm); // get mesh from anim->skel->mesh
            if(!mesh_id.valid())if(Elm *obj=Proj.firstParent(&elm, ELM_OBJ))if(ElmObj *obj_data=obj->objData())mesh_id=obj_data->mesh_id; // get mesh from firstObjParent->mesh
            if( mesh_id.valid())
            {
               anim=Animations(Proj.gamePath(elm .id));
               mesh=           Proj.gamePath(mesh_id) ;
               skel=mesh->skeleton();
               if(skel)
               {
                  if(anim_skel.skeleton()!=skel || anim_skel.bones.elms()!=skel->bones.elms() || anim_skel.slots.elms()!=skel->slots.elms())anim_skel.create(skel);
                  skel_anim.create(*skel, *anim);
                  flt time=(anim->length() ? Frac(Time.appTime(), (dbl)anim->length()) : 0);
                  anim_skel.clear().animate(skel_anim, time).animateRoot(*anim, time).updateMatrix().updateVelocities(false, false);
               }else
               {
                  anim_skel.del().updateMatrix().updateVelocities();
               }

               D.viewRect(rect).viewFov(PreviewFOV);
               flt dist=Max(0.1f, mesh ? GetDist(mesh->ext) : 0);
               D.viewFrom(dist*0.01f).viewRange(dist*5); if(mesh)SetCam(cam, mesh->ext, Time.appTime()); // set
               Renderer(PreviewClass::Render);
               D.viewRect(r).viewFov(fov).viewFrom(from).viewRange(range); c.set(); // restore
               rect.draw(rect_color, false);
            }
         }break;

         case ELM_FONT:
         {
            TextStyle ts; ts.size=rect.h()/16; ts.font(Proj.gamePath(elm.id)); if(Font *font=ts.font()())
            {
               if(font->images().elms() && font->images()[0].mode()==IMAGE_SOFT)
               {
                  drawBack(rect);
                  D.text(TextStyleParams(true), rect, "Font is software based\nand can't be previewed");
               }else
               {
                  Rect u=rect, d=rect; u.min.y=d.max.y=rect.centerY();
                  cchar8 *t="ABCDEFGHIJKLMN\nOPQRSTUVWXYZ\nabcdefghijklmn\nopqrstuvwxyz\n0123456789\n!@#$%^&*+-=_()[]{}<>,.:;'\"\\|/?";
                  u.draw(BLACK); ts.color=WHITE; ts.shadow=255; D.clip(u); D.text(ts, u, t); D.clip();
                  d.draw(WHITE); ts.color=BLACK; ts.shadow=  0; D.clip(d); D.text(ts, d, t); D.clip();
                  rect.draw(rect_color, false);
               }
            }
         }break;

         case ELM_TEXT_STYLE: if(TextStylePtr ts=Proj.gamePath(elm.id))
         {
            cchar8 *t="ABCDEFGHIJKLMN\nOPQRSTUVWXYZ\nabcdefghijklmn\nopqrstuvwxyz\n0123456789\n!@#$%^&*+-=_()[]{}<>,.:;'\"\\|/?";
            Color   c=ColorInverse(ts->color); c.a=255;
            rect.draw(c); D.clip(rect); D.text(*ts, rect, t); D.clip();
            rect.draw(rect_color, false);
         }break;

         case ELM_PANEL_IMAGE:
         {
            drawBack(rect); if(PanelImagePtr panel=Proj.gamePath(elm.id))DrawPanelImage(*panel, rect);
         }break;

         case ELM_PANEL:
         {
            drawBack(rect); if(PanelPtr panel=Proj.gamePath(elm.id))panel->draw(Rect_C(rect.center(), rect.size()*0.5f));
         }break;

         case ELM_GUI_SKIN:
         {
            if(GuiSkinPtr skin=Proj.gamePath(elm.id))
            {
               GuiSkinPtr temp=Gui.skin; Gui.skin=skin;
               rect.draw(skin->background_color);
               Window window; window.create(Rect_C(0, 0, rect.w()*0.75f, rect.h()*0.5f), "Window");
               Text   text  ; text  .create(window.clientRect().lerp(0.5f, 0.65f), "Hello!");
               Button button; button.create(Rect_C(window.clientRect().lerp(0.5f, 0.35f), 0.3f, 0.06f), "OK");
               GuiPC gpc;
               gpc.clip=gpc.client_rect=rect;
               gpc.visible=gpc.enabled=true;
               gpc.offset=rect.center();
               window.draw(gpc);
               text  .draw(gpc);
               button.draw(gpc);
               rect.draw(rect_color, false);
               Gui.skin=temp;
            }
         }break;

         case ELM_GUI:
         {
            GuiSkinPtr temp=Gui.skin; if(GuiSkinPtr skin=Proj.appGuiSkin())Gui.skin=skin;
            if(objs_last_id!=elm.id)
            {
               objs_last_id=elm.id;
               objs.load(Proj.gamePath(elm.id));
               objs_container+=objs;
            }
            rect.draw(BackgroundColor());
            GuiPC gpc; gpc.visible=gpc.enabled=true; gpc.offset=(Vec2(0, 0)&rect); gpc.clip=gpc.client_rect=rect; // try to set the 'offset' as close to screen center as possible (so the preview will give similar results to what's actually available)
            FREP(objs_container.childNum())objs_container.child(i)->draw(gpc); // draw children manually and in order as they're listed in the parent
            Gui.skin=temp;
         }break;

         case ELM_IMAGE:
         {
            drawBack(rect);
            if(image=Proj.gamePath(elm.id))
            {
               if(image->mode()==IMAGE_2D  )image->drawFit(rect);else
               if(image->mode()==IMAGE_CUBE)
               {
                  D.viewRect(rect).viewFov(DefaultFOV); SetCam(cam, Box(0), Time.appTime()/2);
                  Sky.skybox(image);
                  Renderer(PreviewClass::Render);
                  Sky.atmospheric();
                  D.viewRect(r).viewFov(fov); c.set();
               }else
               if(image->mode()==IMAGE_3D)
               {
                  D.viewRect(rect).viewFov(PreviewFOV); SetCam(cam, Box(1), Time.appTime()/2);
                  Renderer(PreviewClass::Render);
                  D.viewRect(r).viewFov(fov); c.set();
               }else D.text(TextStyleParams(true), rect, "Image is software based\nand can't be previewed");
            }
         }break;

         case ELM_IMAGE_ATLAS:
         {
            drawBack(rect);
            if(atlas=Proj.gamePath(elm.id))if(int parts=atlas->parts.elms())
            {
               int p=(Time.curTimeMs()/400)%parts;
               ImageAtlas::Part &part=atlas->parts[p];
               if(InRange(part.image_index, atlas->images))
               {
                  Image &image=atlas->images[part.image_index];
                  Vec2 rect_size=rect.size(), tex_size=part.original_size;
                                            tex_size*=rect_size.x/tex_size.x; // scale to fit X
                  if(tex_size.y>rect_size.y)tex_size*=rect_size.y/tex_size.y; // scale to fit Y
                  Vec2 scale=tex_size/part.original_size*0.9f;
                  D.clip(Rect(rect).extend(-D.pixelToScreenSize()));
                  Rect_LU draw_rect(rect.center()+part.center_offset*scale, part.trimmed_size*scale);

                  // draw image
                  if(part.rotated)image.drawPartVertical(draw_rect, part.tex_rect);
                  else            image.drawPart        (draw_rect, part.tex_rect);

                  Rect_C(rect.center(), part.original_size*scale).draw(GREY, false); // original rect
                  draw_rect.draw(RED, false); // trimmed rect
                  flt l=0.03f;
                  D.lineY(GREEN, rect.centerX(), rect.centerY()-l, rect.centerY()+l); // center
                  D.lineX(GREEN, rect.centerY(), rect.centerX()-l, rect.centerX()+l);
                  D.clip();
               }
            }
         }break;

         case ELM_ICON:
         {
            drawBack(rect); if(image=Proj.gamePath(elm.id))image->drawFit(rect);
         }break;

         case ELM_SOUND:
         {
            if(sound_id!=elm.id)
            {
               sound_id=elm.id;
               SoundHeader sound; sound.load(Proj.gamePath(elm));
               sound_info=S+"Length: "+TextReal(sound.length, -2)+"s\nChannels: "+sound.channels+"\nFrequency: "+sound.frequency+"\nKbps: "+DivRound(sound.bit_rate, 1000)+"\nSize: "+FileSize(FSize(Proj.gamePath(elm)))+"\nCodec: "+sound.codecName();
            }
            drawBack(rect); D.clip(rect); D.text(TextStyleParams(true), rect, sound_info); D.clip();
         }break;

         case ELM_VIDEO:
         {
            if(video_id!=elm.id){video_id=elm.id; video.create(Proj.gamePath(elm), true); video_time=0;} // start from the beginning to avoid freezes
            video.update(video_time); video_time+=Time.ad(); // Editor works in background so app time gets updated even when minimized, update time only when actually drawing
            rect.draw(BLACK); video.drawFit(rect);
            rect.draw(rect_color, false);
         }break;

         case ELM_ENV:
         {
            env=Proj.gamePath(elm.id); env->set();
            D.viewRect(rect).viewFov(PreviewFOV).viewFrom(0.01f).viewRange(300); SetCam(cam, Box(0), Time.appTime()/2); // set
            Renderer(PreviewClass::Render);
            D.viewRect(r).viewFov(fov).viewFrom(from).viewRange(range); c.set(); // restore
            rect.draw(rect_color, false);
         }break;

         case ELM_MINI_MAP:
         {
            if(mini_map_id!=elm.id)
            {
               mini_map_id=elm.id;
               mini_map.load(Proj.gamePath(elm));
            }
            drawBack(rect);
            D.clip(Rect(rect).extend(-D.pixelToScreenSize()));
            for(int y=-4; y<4; y++)
            for(int x=-4; x<4; x++)
            {
               Image &img=mini_map(VecI2(x, y));
               if(img.is())img.draw(Rect_LD(x, y, 1, 1)*0.19f+rect.center());
            }
            D.clip();
         }break;

      }

      D.      dofMode(dof      );
      D.  ambientMode(ambient  );
      D.eyeAdaptation(eye_adapt);
      AstrosDraw     =astros;
      Water.draw     =ocean;

      if(elm.importing()){TextStyleParams ts; ts.size*=0.6f; ts.align.set(0, 1); D.text(ts, rect.down(), (ImportRemovedElms || elm.finalExists()) ? "Processing" : "Not Processed");}
   }
   void PreviewClass::clipRect(Rect &rect)
   {
      flt t=D.h(); if(Mode.visibleTabs())MIN(t, Mode.rect().min.y);
      if(rect.max.y> t    )rect+=Vec2(0,  t    -rect.max.y);
      if(rect.min.y<-D.h())rect+=Vec2(0, -D.h()-rect.min.y);
   }
   void PreviewClass::drawAround(Elm &elm, C Rect &rect, flt y)
   {
      D.clip();
      Vec2 size(D.w(), D.h()); size*=0.5f; size=size.avg();
      Rect r;
      if(rect.min.x-(-D.w())>D.w()-rect.max.x)r=Rect_R(rect.min.x-0.02f, y, size.x, size.y);
      else                                    r=Rect_L(rect.max.x+0.02f, y, size.x, size.y);
      clipRect(r);
      draw(elm, r);
   }
   GuiObj* PreviewClass::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel){return null;}
   void    PreviewClass::update(C GuiPC &gpc){}
   void    PreviewClass::draw(C GuiPC &gpc)
{
      if(ListElm *list_elm=Proj.list.visToData(Proj.list.lit))
      {
         CodeEdit.drawPreview(list_elm);
         if(Elm *elm=list_elm->elm)
         {
            D.clip();
            Vec2   size(D.w(), D.h()); size*=0.75f; size=size.avg(); if(elm->type==ELM_SOUND)size.set(0.38f, 0.31f);
            Rect_L rect(Proj.rect().max.x, Ms.pos().y, size.x, size.y); clipRect(rect);
            if(elm->type==ELM_GUI)rect=EditRect();
            draw(*elm, rect);
         }
      }
      if(MergeSimilarMaterials::Data *list_elm=MSM.list.visToData(MSM.list.lit))
         if(Elm *elm=Proj.findElm(list_elm->id))
      {
         D.clip();
         Vec2   size(D.w(), D.h()); size*=0.75f; size=size.avg();
         Rect   rect=MSM.region.screenRect();
         if(rect.min.x-(-D.w())>D.w()-rect.max.x)rect=Rect_R(rect.min.x-0.02f, Ms.pos().y, size.x, size.y);
         else                                    rect=Rect_L(rect.max.x+0.02f, Ms.pos().y, size.x, size.y);
         clipRect(rect);
         draw(*elm, rect);
      }
      if(EraseRemovedElms::Elm *list_elm=EraseRemoved.list.visToData(EraseRemoved.list.lit))
         if(Elm *elm=Proj.findElm(list_elm->id))
      {
         D.clip();
         Vec2   size(D.w(), D.h()); size*=0.75f; size=size.avg();
         Rect   rect=EraseRemoved.region.screenRect();
         if(rect.min.x-(-D.w())>D.w()-rect.max.x)rect=Rect_R(rect.min.x-0.02f, Ms.pos().y, size.x, size.y);
         else                                    rect=Rect_L(rect.max.x+0.02f, Ms.pos().y, size.x, size.y);
         clipRect(rect);
         draw(*elm, rect);
      }
      if(MaterialRegion::Texture *tex=CAST(MaterialRegion::Texture, Gui.ms()))
      {
         D.clip();
         Vec2   size(D.w(), D.h()); size=size.avg(); if(tex->type==MaterialRegion::TEX_RFL_ALL)size.x*=4.0f/3;
         flt    x=D.w();
         if(     MtrlEdit.visible())MIN(x,      MtrlEdit.rect().min.x);
         if(WaterMtrlEdit.visible())MIN(x, WaterMtrlEdit.rect().min.x);
         Rect_R rect(x, Ms.pos().y, size.x, size.y); clipRect(rect);
         tex->draw(rect);
      }
      if(ObjPaintClass::Object *obj=ObjPaint.objects_list.visToData(ObjPaint.objects_list.lit))
         if(Elm *elm=Proj.findElm(obj->id))
      {
         D.clip();
         Vec2   size(D.w(), D.h()); size*=0.5f; size=size.avg();
         Rect_R rect(ObjPaint.rect().min.x, Ms.pos().y, size.x, size.y); clipRect(rect);
         draw(*elm, rect);
      }
      // Object Parameter
      {
         ParamEditor::ParamWindow::Param *param=null;
         if(!param)param=  ObjClassEdit      .findParam(Gui.ms());
         if(!param)param=  ObjEdit.param_edit.findParam(Gui.ms());
         if(!param)param=WorldEdit.param_edit.findParam(Gui.ms());
         if( param)
         {
            if(param->src.type==PARAM_ID && (Gui.ms()==&param->val_textline || Gui.ms()==&param->name))if(Elm *elm=Proj.findElm(param->src.asID()))drawAround(*elm, param->name.screenRect()|param->val_textline.screenRect());
            return;
         }
      }

      // textline
      if(Gui.ms() && Gui.ms()->type()==GO_TEXTLINE)if(Elm *elm=Proj.findElm(Gui.ms()->asTextLine()()))
      {
         drawAround(*elm, Gui.ms()->screenRect());
         return;
      }
   }
PreviewClass::PreviewClass() : skel(null), anim(null), sound_id(UIDZero), video_id(UIDZero), mini_map_id(UIDZero), mesh_variation(0), video_time(0), objs_last_id(UIDZero) {}

/******************************************************************************/
