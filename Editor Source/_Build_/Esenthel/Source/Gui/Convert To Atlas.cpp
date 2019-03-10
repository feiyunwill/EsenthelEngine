/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ConvertToAtlasClass ConvertToAtlas;
/******************************************************************************/

/******************************************************************************/
   cchar8 *ConvertToAtlasClass::mode_t[]=
   {
      "Create as new objects (use for testing)",
      "Replace existing objects (and keep Original Elements)",
      "Replace existing objects (and Disable Publishing of Original Elements)",
      "Replace existing objects (and Remove Original Elements)",
   };
   const flt ConvertToAtlasClass::h=0.05f;
/******************************************************************************/
      void ConvertToAtlasClass::Mtrl::setScaleText(C VecI2 &size) {t_scaled_size.set(TexSize(size));}
      void ConvertToAtlasClass::Mtrl::setScale(             ) {scaled_size=Round(original_size*scale*ConvertToAtlas.scale); setScaleText(scaled_size);}
      void ConvertToAtlasClass::Mtrl::setScale(flt scale    ) {T.scale=scale; setScale();}
      void ConvertToAtlasClass::Mtrl::posY(flt y        )
      {
         del .pos(Vec2(del .pos().x, y));
         name.pos(Vec2(name.pos().x, y));
         y-=del.rect().h()/2;
         prop           .pos(Vec2(prop.name      .pos().x, y));
         t_original_size.pos(Vec2(t_original_size.pos().x, y));
         t_scaled_size  .pos(Vec2(t_scaled_size  .pos().x, y));
      }
      void ConvertToAtlasClass::Mtrl::pack(MtrlImages &atlas)
      {
         ThreadMayUseGPUData();
         edit.load(Proj.editPath(id));
         VecI2 size=packed_rect.size(); if(rotated)size.swap();
         MtrlImages src; src.fromMaterial(edit, Proj, false, size, true);
         Color normal(128, 128, 255);
         REPD(y, size.y)
         REPD(x, size.x)
         {
            int dest_x=x, dest_y=y; if(rotated)Swap(dest_x, dest_y);
            dest_x+=packed_rect.min.x;
            dest_y+=packed_rect.min.y;

            flt   glow; if(src.glow  .is()){Color c=src.glow.color(x, y); glow=c.lum()*c.a/255.0f;}else glow=255;
            Color n   ; if(src.normal.is())
            {
               n=src.normal.color(x, y);
               if(src.flip_normal_y)n.g=255-n.g;
               if(rotated)Swap(n.r, n.g); // in rotated mode, normal XY channels need to be swapped
               if(!Equal(edit.rough, 1))
               {
                  n.r=Mid(Round((n.r-128)*edit.rough+128), 0, 255);
                  n.g=Mid(Round((n.g-128)*edit.rough+128), 0, 255);
               }
            }else n=normal;

            Color c; if(src.color.is())
            {
               Vec4 cf=src.color.colorF(x, y);
               cf.xyz*=edit.color.xyz;
               c=cf;
            }else c=WHITE;

            atlas.color   .color(dest_x, dest_y, c);
            atlas.alpha   .pixB (dest_x, dest_y)=          (src.alpha   .is() ? Mid(Round(src.alpha   .pixelF(x, y)*255                       ), 0, 255) : 255);
            atlas.bump    .pixB (dest_x, dest_y)=          (src.bump    .is() ? Mid(Round(src.bump    .pixelF(x, y)*255                       ), 0, 255) : 255);
            atlas.specular.pixB (dest_x, dest_y)=Mid(Round((src.specular.is() ?           src.specular.color (x, y).lum() : 255)*edit.specular), 0, 255); // bake
            atlas.glow    .pixB (dest_x, dest_y)=Mid(Round(                                      glow                           *edit.glow    ), 0, 255); // bake
            atlas.normal  .color(dest_x, dest_y, n);
         }
         tex=src.color.is()*BT_COLOR | src.alpha.is()*BT_ALPHA | src.bump.is()*BT_BUMP | src.normal.is()*BT_NORMAL | src.specular.is()*BT_SPECULAR | src.glow.is()*BT_GLOW;
         AtomicOr(atlas.tex, tex);
      }
      void ConvertToAtlasClass::Preview::draw(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            D.clip(gpc.clip);
            Rect r=rect()+gpc.offset;
            if(ConvertToAtlas.tex_size.all())
            {
               r=Fit(flt(ConvertToAtlas.tex_size.x)/ConvertToAtlas.tex_size.y, r);
               ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
               REPA(ConvertToAtlas.mtrls)
               {
                C Mtrl &mtrl=ConvertToAtlas.mtrls[i];
                  Rect  mr  =mtrl.packed_rect; mr/=Vec2(ConvertToAtlas.tex_size);
                  mr.setY(1-mr.max.y, 1-mr.min.y); // inverse Y because textures coordinates increasing down, and gui drawing increasing up
                  mr.min=r.lerp(mr.min);
                  mr.max=r.lerp(mr.max);
                  if(Image *image=mtrl.base_0())
                  {
                     if(mtrl.rotated)image->drawVertical(mr);
                     else            image->draw        (mr);
                  }
                  mr.draw(RED, false);
               }
               D.alpha(alpha);
            }else
            if(ConvertToAtlas.mtrls.elms())
            {
               D.text(r, "Textures too big\nDecrease scale");
            }
            r.draw(Gui.borderColor(), false);
         }
      }
      Str  ConvertToAtlasClass::Warning::MeshName(C Error &error) {return Proj.elmFullName(error.mesh_id);}
      Str  ConvertToAtlasClass::Warning::MtrlName(C Error &error) {return Proj.elmFullName(error.mtrl_id);}
      void ConvertToAtlasClass::Warning::Proceed(ptr) {ConvertToAtlas.convertPerform();}
      void ConvertToAtlasClass::Warning::create()
      {
         Gui+=::EE::Window::create(Rect_C(0, 0, 1.7f, 1), "Convert To Atlas").hide(); button[2].show();
         T+=text.create(Vec2(clientWidth()/2, -0.06f), "Warning: Following meshes use wrapped texture coordinates.\nConverting to Atlas may introduce some artifacts.");
         T+=proceed.create(Rect_D(clientWidth()/2, -clientHeight()+0.03f, 0.34f, 0.055f), "Proceed Anyway").func(Proceed);
         T+=region.create(Rect(0, proceed.rect().max.y, clientWidth(), text.rect().min.y-0.04f).extend(-0.02f, -0.03f));
         ListColumn lc[]=
         {
            ListColumn(MeshName, LCW_DATA, "Mesh"),
            ListColumn(MtrlName, LCW_DATA, "Material"),
            ListColumn(MEMBER(Error, error), 0.18f, "UV Error"),
         };
         region+=list.create(lc, Elms(lc));
      }
      void ConvertToAtlasClass::Warning::display()
      {
         list.setData(ConvertToAtlas.errors);
         activate();
      }
   Str  ConvertToAtlasClass::TexSize(C VecI2 &size) {return S+size.x+'x'+size.y;}
   void ConvertToAtlasClass::Scale(Mtrl &mtrl, C Str &text) {mtrl.setScale(TextFlt(text)); ConvertToAtlas.refresh();}
   void ConvertToAtlasClass::Del(Mtrl &mtrl) {ConvertToAtlas.mtrls.removeData(&mtrl, true); ConvertToAtlas.refresh();}
   void ConvertToAtlasClass::Refresh(C Property &prop) {ConvertToAtlas.refresh();}
   void ConvertToAtlasClass::ChangedScale(C Property &prop) {REPAO(ConvertToAtlas.mtrls).setScale(); ConvertToAtlas.refresh();}
   int  ConvertToAtlasClass::CompareData(C Mtrl &a, C Mtrl &b) {return ComparePathNumber(a.name(), b.name());}
   void ConvertToAtlasClass::Convert(ConvertToAtlasClass &cta) {cta.convertDo();}
   void ConvertToAtlasClass::PackMaterial(Mtrl &mtrl, MtrlImages &atlas, int thread_index) {mtrl.pack(atlas);}
   void ConvertToAtlasClass::setErrors(Mesh &mesh, C Memc<UID> &mtrl_ids, C UID &mesh_id)
   {
      const Rect tex_rect(0, 1);
      REPD(l, mesh.lods())
      {
         MeshLod &lod=mesh.lod(l); REPA(lod)
         {
            MeshPart &part=lod.parts[i];
            MeshBase &base=part.base; if(base.vtx.tex0())REP(4)
            {
               UID mtrl_id=part.multiMaterial(i).id(); if(mtrl_id.valid() && mtrl_ids.binaryHas(mtrl_id, Compare))
               {
                  base.explodeVtxs().fixTexOffset();
                  flt error=0; REPA(base.vtx)MAX(error, Dist(base.vtx.tex0(i), tex_rect)); // verify that all tex uv's are within allowed tex rect
                  if( error>0.01f) // allow some tolerance
                  {
                     Error *e=null; REPA(errors){Error &err=errors[i]; if(err.mesh_id==mesh_id && err.mtrl_id==err.mtrl_id){e=&err; break;}}
                     if(!e){e=&errors.New(); e->mesh_id=mesh_id; e->mtrl_id=mtrl_id;}
                     MAX(e->error, error);
                  }
                  break;
               }
            }
         }
      }
   }
   bool ConvertToAtlasClass::Create(int &occurence, C UID &id, ptr user) {occurence=0; return true;}
   void ConvertToAtlasClass::convertMeshes(C UID &atlas_id)
   {
      MaterialPtr atlas_mtrl=Proj.gamePath(atlas_id);
      Memc<UID> mtrl_ids, obj_ids; REPA(mtrls)mtrl_ids.binaryInclude(mtrls[i].id, Compare);
      REPA(Proj.elms) // iterate all elements
      {
       C Elm &obj=Proj.elms[i]; if(C ElmObj *obj_data=obj.objData()) // if that's an object
         if(C Elm *mesh=Proj.findElm(obj_data->mesh_id))if(C ElmMesh *mesh_data=mesh->meshData())
         {
            REPA(mesh_data->mtrl_ids)if(mtrl_ids.binaryHas(mesh_data->mtrl_ids[i], Compare)) // if that mesh has one of the materials
            {
               obj_ids.binaryInclude(obj.id, Compare);
               break;
            }
         }
      }
      Memc<UID> duplicated; Proj.duplicate(obj_ids, duplicated, (mode==NEW) ? " (Atlas)" : " (Atlas Backup)");
      //       obj_ids duplicated
      // NEW              atlas
      // REPLACE         backup
      if(mode==NEW)Swap(duplicated, obj_ids);
      Memc<UID> &atlased=obj_ids, &non_atlased=duplicated;

      REPA(atlased)if(Elm *obj=Proj.findElm(atlased[i]))if(ElmObj *obj_data=obj->objData())
                   if(Elm *mesh_elm=Proj.findElm(obj_data->mesh_id, ELM_MESH))if(ElmMesh *mesh_data=mesh_elm->meshData())
      {
         if(ObjEdit.mesh_elm==mesh_elm)ObjEdit.flushMeshSkel();
         Mesh mesh; if(Load(mesh, Proj.editPath(mesh_elm->id), Proj.game_path))
         {
            REPD(l, mesh.lods())
            {
               MeshLod &lod=mesh.lod(l); REPA(lod)
               {
                  MeshPart   &part=lod.parts[i];
                  Mtrl       *mtrl=null;
                  MaterialPtr mtrls[4]; REPA(mtrls)
                  {
                     MaterialPtr &mtrl_ptr=mtrls[i]; if(mtrl_ptr=part.multiMaterial(i))
                     {
                        UID mtrl_id=mtrl_ptr.id(); REPA(T.mtrls)if(T.mtrls[i].id==mtrl_id){mtrl=&T.mtrls[i]; mtrl_ptr=atlas_mtrl; break;}
                     }
                  }
                  if(mtrl)
                  {
                     part.multiMaterial(mtrls[0], mtrls[1], mtrls[2], mtrls[3]);
                     MeshBase &base=part.base; if(base.vtx.tex0())
                     {
                        Vec2 mul=Vec2(mtrl->packed_rect.size())/tex_size,
                             add=Vec2(mtrl->packed_rect.min   )/tex_size;
                        base.explodeVtxs().fixTexOffset();
                        REPA(base.vtx)
                        {
                           Vec2 &t=base.vtx.tex0(i);
                           Clamp(t.x, 0, 1);
                           Clamp(t.y, 0, 1);
                           if(mtrl->rotated)Swap(t.x, t.y);
                           t*=mul; t+=add;
                        }
                        base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, do not remove degenerate faces because they're not needed because we're doing this only because of 'explodeVtxs'
                     }
                  }
               }
            }
            mesh_data->newVer(); mesh_data->file_time.getUTC();
            Save(mesh, Proj.editPath(mesh_elm->id), Proj.game_path);
            Proj.makeGameVer(*mesh_elm);
            Server.setElmLong(mesh_elm->id);
            if(ObjEdit.mesh_elm==mesh_elm)ObjEdit.reloadMeshSkel();
         }
      }

      // adjust parents, publishing and removed status
      TimeStamp time; time.getUTC();

      if(atlased.elms()==1) // if there's only one object created, then put the atlas inside of it
         if(Elm *atlas=Proj.findElm(atlas_id))
      {
         atlas->setParent(atlased[0], time); Server.setElmParent(*atlas);
      }

      // process objects
      if(non_atlased.elms()==atlased.elms())
      {
         REPA(non_atlased)if(Elm *elm=Proj.findElm(non_atlased[i]))
         {
            if(mode==REPLACE_NO_PUBLISH || mode==REPLACE_REMOVE){elm->setParent(atlased[i], time); Server.setElmParent(*elm);} // move 'non_atlased' to 'atlased'
            if(mode==REPLACE_NO_PUBLISH)elm->setNoPublish(true, time);else
            if(mode==REPLACE_REMOVE    )elm->setRemoved  (true, time);
         }
         if(mode==REPLACE_NO_PUBLISH)Server.noPublishElms(non_atlased, true, time);else
         if(mode==REPLACE_REMOVE    )Server.removeElms   (non_atlased, true, time);

         /*if(mode==NEW_NO_PUBLISH || mode==NEW_REMOVE) // move sub elements from 'non_atlased' -> 'atlased'
         {
            REPA(Proj.elms)
            {
               Elm &sub=Proj.elms[i]; int index;
               if(non_atlased.binarySearch(sub.parent_id, index, Compare) // if is located in one of 'non_atlased'
               && !mtrl_ids.binaryHas(sub.id, Compare) // this is not one of the original materials (let's keep original materials in objects)
               && ElmVisible(sub.type)) // this is not one of the special element of original object (mesh/skel/phys)
               {
                  sub.setParent(atlased[index], time); Server.setElmParent(sub);
               }
            }
         }*/
         if(mode!=NEW) // move materials to old ones, from 'atlased' -> 'non_atlased'
         {
            REPA(mtrl_ids)if(Elm *mtrl=Proj.findElm(mtrl_ids[i]))
            {
               int index; if(atlased.binarySearch(mtrl->parent_id, index, Compare)) // if is located in one of 'atlased'
               {
                  mtrl->setParent(non_atlased[index], time); Server.setElmParent(*mtrl);
               }
            }
         }
      }

      // process materials
      REPA(mtrl_ids)if(Elm *elm=Proj.findElm(mtrl_ids[i]))
      {
         if(mode==REPLACE_NO_PUBLISH)elm->setNoPublish(true, time);else
         if(mode==REPLACE_REMOVE    )elm->setRemoved  (true, time);
      }
      if(mode==REPLACE_NO_PUBLISH)Server.noPublishElms(mtrl_ids, true, time);else
      if(mode==REPLACE_REMOVE    )Server.removeElms   (mtrl_ids, true, time);

      if(mode==REPLACE_NO_PUBLISH || mode==REPLACE_REMOVE)REPA(Proj.elms) // move material sub elements to atlas
      {
         Elm &sub=Proj.elms[i]; if(mtrl_ids.binaryHas(sub.parent_id, Compare))
         {
            sub.setParent(atlas_id, time); Server.setElmParent(sub);
         }
      }
   }
   void ConvertToAtlasClass::AddMap(Str &dest, C Str &src, C Mtrl &mtrl, bool normal, C Vec &mul)
   {
      if(src.is())
      {
         Mems<Edit::FileParams> fps=Edit::FileParams::Decode(dest);
         Edit::FileParams &fp=fps.New(); fp=src;
         VecI2 size=mtrl.packed_rect.size();
         if(mtrl.edit.flip_normal_y && normal) fp.params.New().set("inverseG"); // !! this needs to be done before 'swapRG' !!
         if(mtrl.rotated                     ){fp.params.New().set("swapXY"); if(normal)fp.params.New().set("swapRG");} // !! this needs to be done before 'resize' !!
         if(!Equal(mul, Vec(1))              ) fp.params.New().set(normal ? "scaleXY" : "mulRGB", TextVecEx(mul));
                                               fp.params.New().set("resizeClamp", VecI2AsText(size));
         if(mtrl.packed_rect.min.any()       ) fp.params.New().set("position"   , S+mtrl.packed_rect.min.x+','+mtrl.packed_rect.min.y);
         dest=Edit::FileParams::Encode(fps);
      }
   }
   void ConvertToAtlasClass::checkSide(Str &dest, int filled)
   {
      if(dest.is() && !filled) // if there's at least one texture, but it doesn't fill the target fully, then we need to force the size
      {
         Mems<Edit::FileParams> fps=Edit::FileParams::Decode(dest);
         Edit::FileParams &fp=fps.New(); // insert a dummy empty source
         fp.params.New().set("position", S+tex_size.x+','+tex_size.y); // with only position specified to force the entire texture size
         dest=Edit::FileParams::Encode(fps);
      }
   }
   void ConvertToAtlasClass::convertPerform()
   {
      // convert before hiding because that may release resources
      if(mtrls.elms())
      {
         MtrlImages atlas_images; if(atlas_images.create(tex_size))
         {
            Map<UID, int> parent_occurence(Compare, Create); // this is to calculate which parent is most frequently used for stored materials
            REPA(mtrls)
            {
               Mtrl &mtrl=mtrls[i];
               MtrlEdit.flush(mtrl.id);
               if(Elm *elm=Proj.findElm(mtrl.id))(*parent_occurence(elm->parent_id))++;
            }
            int occurences=0; UID parent_id=UIDZero; REPA(parent_occurence)if(parent_occurence[i]>occurences){occurences=parent_occurence[i]; parent_id=parent_occurence.key(i);}
            atlas_images.clear();
            WorkerThreads.process1(mtrls, PackMaterial, atlas_images);
            atlas_images.compact();
          //atlas_images.Export("d:/", "bmp");

            ImporterClass::Import::MaterialEx atlas;
            uint  tex=CreateBaseTextures(atlas.base_0, atlas.base_1, atlas_images.color, atlas_images.alpha, atlas_images.bump, atlas_images.normal, atlas_images.specular, atlas_images.glow);
            VecI2 tex_filled=0; // x=which textures are filled in X, y=which textures are filled in Y
            atlas.name="Atlas";
            atlas.mtrl.cull=true;
            atlas.mtrl.color=0;
            atlas.mtrl.ambient=0;
            atlas.mtrl.sss=0;
            atlas.mtrl.glow=0;
            atlas.mtrl.rough=0;
            atlas.mtrl.bump=0;
            atlas.mtrl.specular=0;
            atlas.mtrl.reflect=0;
            bool has_alpha_map=false; flt alpha=0; int alpha_num=0; MATERIAL_TECHNIQUE tech=MTECH_DEFAULT; // parameters for alpha materials
             REPA(mtrls)if(mtrls[i].edit.alpha_map.is()){has_alpha_map=true; break;} // if at least one 'alpha_map' is specified
            FREPA(mtrls)
            {
               Mtrl &mtrl=mtrls[i];
               atlas.mtrl.cull    &=mtrl.edit.cull; // if at least one material requires cull disabled, then disable for all
               atlas.mtrl.color   +=mtrl.edit.color;
               atlas.mtrl.ambient +=mtrl.edit.ambient;
               atlas.mtrl.sss     +=mtrl.edit.sss;
               atlas.mtrl.glow    +=mtrl.edit.glow;
               atlas.mtrl.rough   +=mtrl.edit.rough;
               atlas.mtrl.bump    +=mtrl.edit.bump;
               atlas.mtrl.specular+=mtrl.edit.specular;
               atlas.mtrl.reflect +=mtrl.edit.reflection;

               if(mtrl.edit.tech){alpha+=mtrl.edit.color.w; alpha_num++; tech=mtrl.edit.tech;}

               Str alpha_map=mtrl.edit.alpha_map;
               if(has_alpha_map) // if at least one material has 'alpha_map', then we need to specify all of them, in case: alpha in one material comes from 'color_map', or it will in the future
                  if(!alpha_map.is()) // if not yet specified
               {
                  alpha_map=mtrl.edit.color_map; // set from 'color_map'
                  SetTransform(alpha_map, "channel", "a"); // use alpha channel of 'color_map'
               }

               AddMap(atlas.   color_map, mtrl.edit.   color_map, mtrl, false, mtrl.edit.color.xyz);
               AddMap(atlas.   alpha_map,              alpha_map, mtrl);
               AddMap(atlas.    bump_map, mtrl.edit.    bump_map, mtrl);
               AddMap(atlas.  normal_map, mtrl.edit.  normal_map, mtrl, true , mtrl.edit.rough   );
               AddMap(atlas.specular_map, mtrl.edit.specular_map, mtrl, false, mtrl.edit.specular);
               AddMap(atlas.    glow_map, mtrl.edit.    glow_map, mtrl, false, mtrl.edit.glow    );
               
               if(mtrl.packed_rect.includesX(tex_size.x)) // if this packed rect includes the right side
               {
                  if(mtrl.edit.   color_map.is())tex_filled.x|=BT_COLOR;
                  if(             alpha_map.is())tex_filled.x|=BT_ALPHA;
                  if(mtrl.edit.    bump_map.is())tex_filled.x|=BT_BUMP;
                  if(mtrl.edit.  normal_map.is())tex_filled.x|=BT_NORMAL;
                  if(mtrl.edit.specular_map.is())tex_filled.x|=BT_SPECULAR;
                  if(mtrl.edit.    glow_map.is())tex_filled.x|=BT_GLOW;
               }
               if(mtrl.packed_rect.includesY(tex_size.y)) // if this packed rect includes the bottom side
               {
                  if(mtrl.edit.   color_map.is())tex_filled.y|=BT_COLOR;
                  if(             alpha_map.is())tex_filled.y|=BT_ALPHA;
                  if(mtrl.edit.    bump_map.is())tex_filled.y|=BT_BUMP;
                  if(mtrl.edit.  normal_map.is())tex_filled.y|=BT_NORMAL;
                  if(mtrl.edit.specular_map.is())tex_filled.y|=BT_SPECULAR;
                  if(mtrl.edit.    glow_map.is())tex_filled.y|=BT_GLOW;
               }
            }

            tex_filled.x&=tex_filled.y; // we need both sides to be filled
            checkSide(atlas.   color_map, tex_filled.x&BT_COLOR   );
            checkSide(atlas.   alpha_map, tex_filled.x&BT_ALPHA   );
            checkSide(atlas.    bump_map, tex_filled.x&BT_BUMP    );
            checkSide(atlas.  normal_map, tex_filled.x&BT_NORMAL  );
            checkSide(atlas.specular_map, tex_filled.x&BT_SPECULAR);
            checkSide(atlas.    glow_map, tex_filled.x&BT_GLOW    );

            if(tex&BT_COLOR   ){atlas.mtrl.color.xyz=                                1;                           }else atlas.mtrl.color.xyz/=mtrls.elms();
            if(tex&BT_ALPHA   ){atlas.mtrl.color.w  =(alpha_num ? alpha/alpha_num : 1); atlas.mtrl.technique=tech;}else atlas.mtrl.color.w  /=mtrls.elms(); // if we ended up having alpha    map, then set parameters from alpha materials only
            if(tex&BT_BUMP    ){                                                                                  }     atlas.mtrl.bump     /=mtrls.elms();
            if(tex&BT_NORMAL  ){atlas.mtrl.rough    =1;                                                           }else atlas.mtrl.rough    /=mtrls.elms(); // if we ended up having normal   map, then it means we've used the baked textures, for which we need to set the full rough    multiplier
            if(tex&BT_SPECULAR){atlas.mtrl.specular =1;                                                           }else atlas.mtrl.specular /=mtrls.elms(); // if we ended up having specular map, then it means we've used the baked textures, for which we need to set the full specular multiplier
            if(tex&BT_GLOW    ){atlas.mtrl.glow     =1;                                                           }else atlas.mtrl.glow     /=mtrls.elms(); // if we ended up having glow     map, then it means we've used the baked textures, for which we need to set the full glow     multiplier
                                                                                                                        atlas.mtrl.ambient  /=mtrls.elms();
                                                                                                                        atlas.mtrl.sss      /=mtrls.elms();
                                                                                                                        atlas.mtrl.reflect  /=mtrls.elms();

            IMAGE_TYPE ct; ImageProps(atlas.base_0, &atlas.base_0_id, &ct, ForceHQMtrlBase0 ? FORCE_HQ : 0); if(Importer.includeTex(atlas.base_0_id))atlas.base_0.copyTry(atlas.base_0, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, false, false, false, false);
                           ImageProps(atlas.base_1, &atlas.base_1_id, &ct, ForceHQMtrlBase1 ? FORCE_HQ : 0); if(Importer.includeTex(atlas.base_1_id))atlas.base_1.copyTry(atlas.base_1, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, false, false, false, true );

            UID atlas_id=Proj.newMtrl(atlas, parent_id).id;
            Server.setElmFull(atlas_id);
            convertMeshes(atlas_id);
            Proj.setList();
         }
      }
      warning.hide();
      hide();
   }
   void ConvertToAtlasClass::convertDo()
   {
      warning.hide();
      Memc<UID> mtrl_ids; REPA(mtrls)mtrl_ids.binaryInclude(mtrls[i].id, Compare);

      // check if can (meshes need to have non-wrapped UV's)
      errors.clear();
      REPA(Proj.elms) // iterate all elements
      {
         Elm &elm=Proj.elms[i]; if(ElmMesh *mesh_data=elm.meshData()) // if that's a mesh
         {
            REPA(mesh_data->mtrl_ids)if(mtrl_ids.binaryHas(mesh_data->mtrl_ids[i], Compare)) // if that mesh has one of the materials
            {
               Mesh mesh; if(ObjEdit.mesh_elm==&elm)mesh.create(ObjEdit.mesh);else Load(mesh, Proj.editPath(elm.id), Proj.game_path); // load edit so we can have access to MeshBase
               setErrors(mesh, mtrl_ids, elm.id);
               break;
            }
         }
      }

      if(errors.elms())warning.display();else convertPerform();
   }
   void ConvertToAtlasClass::clearProj()
   {
      mtrls.clear();
      warning.hide();
   }
   void ConvertToAtlasClass::create()
   {
      warning.create();
               add("Force Square"   , MEMBER(ConvertToAtlasClass, force_square)).changed(Refresh);
               add("Auto Stretch"   , MEMBER(ConvertToAtlasClass, auto_stretch)).changed(Refresh);
Property &mode=add("Atlased Objects", MEMBER(ConvertToAtlasClass, mode)).setEnum(mode_t, Elms(mode_t)).desc("When creating material atlases, existing object meshes need to have their UV adjusted.\nWith this option you can control if the adjusted objects:\n-Replace the old ones (keeping their Element ID)\nor\n-They are created as new objects (with new Element ID)");
               add("Global Scale"   , MEMBER(ConvertToAtlasClass, scale)).range(1.0f/8, 8).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).changed(ChangedScale);
      Rect r=::PropWin::create("Convert To Atlas"); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); mode.combobox.resize(Vec2(0.63f, 0));
      autoData(this);
      T+=region.create(Rect_LU(0.02f, r.min.y-0.02f, 1.70f, 0.7f));
      T+=preview.create(Rect_LU(region.rect().ru()+Vec2(0.02f, 0), 0.7f));
      Vec2 size(preview.rect().max.x, Max(-region.rect().min.y, -preview.rect().min.y));
      rect(Rect_C(0, size+0.02f+defaultInnerPaddingSize()));
      T+=columns.create(Vec2(1.07f, region.rect().max.y+ts.size.y*0.8f), "Scale       Original Size      Scaled Size", &ts);
      T+=t_tex_size.create(preview.rect().up()+Vec2(0, 0.04f));
      T+=convert.create(Rect_U(clientWidth()/2, -0.05f, 0.3f, 0.055f), "Convert").func(Convert, T);
   }
   void ConvertToAtlasClass::autoStretch()
   {
      if(mtrls.elms())
      {
         RectI r=mtrls.last().packed_rect; REPA(mtrls)r|=mtrls[i].packed_rect; if(r.size().all())
         {
            if(r.min.any() || r.max!=tex_size)
            {
               RectI tex_rect(0, tex_size);
               Vec2  scale=Vec2(tex_size)/r.size(), offset=-r.min*scale;
               REPA(mtrls)
               {
                  RectI &rect=mtrls[i].packed_rect;
                  rect=Round(Rect(rect)*scale+offset)&tex_rect;
               }
            }
         }
      }
      // convert to inclusive
      REPAO(mtrls).packed_rect.max--;
      REPA (mtrls)
      {
         RectI &r=mtrls[i].packed_rect;
         r.min.x=           0; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.min.x=t.max.x+1;}
         r.max.x=tex_size.x-1; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.max.x=t.min.x-1;}
         r.min.y=           0; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.min.y=t.max.y+1;}
         r.max.y=tex_size.y-1; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.max.y=t.min.y-1;}
      }
      // convert to exclusive
      REPAO(mtrls).packed_rect.max++;
   }
   void ConvertToAtlasClass::refresh()
   {
      flt y=0;
      FREPA(mtrls){mtrls[i].posY(y); y-=h;}
      Memc<RectSizeAnchor> mtrl_sizes; mtrl_sizes.setNum(mtrls.elms()); REPAO(mtrl_sizes).size=mtrls[i].scaled_size;
      Memc<RectI         > mtrl_rects; if(PackRectsUnknownLimit(mtrl_sizes, mtrl_rects, tex_size, true, 0, false, true, ConvertToAtlas.force_square))
      {
         REPA(mtrls)
         {
            Mtrl &mtrl=mtrls[i];
            mtrl.packed_rect=mtrl_rects[i];
            mtrl.rotated=((mtrl.packed_rect.w()>mtrl.packed_rect.h())!=(mtrl.scaled_size.x>mtrl.scaled_size.y));
         }
         if(auto_stretch)autoStretch();
         t_tex_size.set(TexSize(tex_size));
      }else
      {
         REPAO(mtrls).packed_rect.zero();
         tex_size.zero();
       t_tex_size.clear();
      }
      REPA(mtrls){Mtrl &mtrl=mtrls[i]; mtrl.setScaleText(mtrl.packed_rect.size());}
      warning.hide();
   }
   void ConvertToAtlasClass::addElms(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)if(Elm *elm=Proj.findElm(elm_ids[i], ELM_MTRL))
      {
         REPA(mtrls)if(mtrls[i].id==elm->id)goto skip;
         {
            Mtrl &mtrl=mtrls.New();
            mtrl.id=elm->id;
            region+=mtrl.del .create(Rect_LU(0, 0, h, h)).setImage("Gui/close.img").func(Del, mtrl).desc("Remove this Material");
            region+=mtrl.name.create(Rect_LU(h, 0, 0.9f, h), Proj.elmFullName(elm->id)).disabled(true);
            if(ElmMaterial *mtrl_data=elm->mtrlData())
               if(mtrl.base_0=Proj.texPath(mtrl_data->base_0_tex))
                  mtrl.original_size=mtrl.base_0->size();
            mtrl.prop.create(S, MemberDesc(MEMBER(Mtrl, scale)).setTextToDataFunc(Scale)).range(1.0f/8, 8).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).autoData(&mtrl).name.text_style=&ts;
            region+=mtrl.t_original_size.create(Vec2(mtrl.name.rect().max.x+0.33f, 0-h*0.5f), TexSize(mtrl.original_size));
            region+=mtrl.t_scaled_size  .create(Vec2(mtrl.name.rect().max.x+0.56f, 0-h*0.5f));
            mtrl.prop.addTo(region,             Vec2(mtrl.name.rect().max.x+0.04f, 0), 0, h, 0.15f);
            mtrl.setScale();
         }
      skip:;
      }
      mtrls.sort(CompareData);
      refresh();
      activate();
   }
   void ConvertToAtlasClass::setElms(C MemPtr<UID> &elm_ids)
   {
      mtrls.clear();
      addElms(elm_ids);
   }
   void ConvertToAtlasClass::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))
      {
         addElms(elms);
         elms.clear();
      }
   }
   ConvertToAtlasClass& ConvertToAtlasClass::hide()
{
      ::PropWin::hide();
      mtrls.clear(); warning.hide(); // release memory, since we're releasing then we also need to hide the warning to prevent from converting on empty materials
      return T;
   }
ConvertToAtlasClass::ConvertToAtlasClass() : force_square(false), auto_stretch(false), scale(1), mode(NEW), tex_size(0) {}

ConvertToAtlasClass::Mtrl::Mtrl() : id(UIDZero), original_size(0), scaled_size(0), pos(0), packed_rect(0), rotated(false), scale(1) {}

ConvertToAtlasClass::Error::Error() : mesh_id(UIDZero), mtrl_id(UIDZero), error(0) {}

/******************************************************************************/
