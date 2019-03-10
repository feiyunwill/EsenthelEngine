/******************************************************************************/
class ListElm
{
   bool    size_known=true, tex_size_calculated=false;
   int     depth=0;
   flt     offset=0;
   Color   color=BLACK, color_temp=BLACK;
   Elm    *elm =null;
   EEItem *item=null;
   Image  *opened_icon=null, *icon=null; // use regular pointers to avoid ref counting for ImagePtr
   Str     name, desc;
   long    size=0;
   union 
   {
      int      vis_parent_i; // stored as index first, because we're using Memc for the list data, adding new elements could change memory address
      ListElm *vis_parent  ;
   };

   static int CompareIndex(C ListElm &a, C ListElm &b) {return Compare(ptr(&a), ptr(&b));} // we can use memory address because we're using 'Memc'
   static int CompareName (C ListElm &a, C ListElm &b)
   {
      MemtN<C ListElm*, 128> as, bs;
      for(C ListElm *p=&a; ; ){as.add(p); p=p.vis_parent; if(!p)break;}
      for(C ListElm *p=&b; ; ){bs.add(p); p=p.vis_parent; if(!p)break;}
      int  shared_parents=Min(as.elms(), bs.elms());
      FREP(shared_parents)
      {
       C ListElm &a=*as[as.elms()-1-i],
                 &b=*bs[bs.elms()-1-i];
         if(int c=CompareNumber(a.name, b.name))return c;
         if(int c=CompareIndex (a     , b     ))return c;
      }
      return CompareIndex(a, b); // compare by index instead of returning 0, because only shared parents were checked, if 'a' is child of 'b' then we need to make sure that 'a' is listed after
   }
   static int CompareSize(C ListElm &a, C ListElm &b)
   {
      MemtN<C ListElm*, 128> as, bs;
      for(C ListElm *p=&a; ; ){as.add(p); p=p.vis_parent; if(!p)break;}
      for(C ListElm *p=&b; ; ){bs.add(p); p=p.vis_parent; if(!p)break;}
      int  shared_parents=Min(as.elms(), bs.elms());
      FREP(shared_parents)
      {
       C ListElm &a=*as[as.elms()-1-i],
                 &b=*bs[bs.elms()-1-i];
         if(int c=Compare     (b.fileSize(), a.fileSize()))return c; // swap order, because most likely we'll be interested in biggest file sizes
         if(int c=CompareIndex(a           , b           ))return c;
      }
      return CompareIndex(a, b); // compare by index instead of returning 0, because only shared parents were checked, if 'a' is child of 'b' then we need to make sure that 'a' is listed after
   }
   static int CompareTexSharp(C ListElm &a, C ListElm &b)
   {
      MemtN<C ListElm*, 128> as, bs;
      for(C ListElm *p=&a; ; ){as.add(p); p=p.vis_parent; if(!p)break;}
      for(C ListElm *p=&b; ; ){bs.add(p); p=p.vis_parent; if(!p)break;}
      int  shared_parents=Min(as.elms(), bs.elms());
      FREP(shared_parents)
      {
       C ListElm &a=*as[as.elms()-1-i],
                 &b=*bs[bs.elms()-1-i];
         if(int c=Compare     (a.texSharpness(), b.texSharpness()))return c;
         if(int c=CompareIndex(a               , b               ))return c;
      }
      return CompareIndex(a, b); // compare by index instead of returning 0, because only shared parents were checked, if 'a' is child of 'b' then we need to make sure that 'a' is listed after
   }
   static Str Size(C ListElm &data)
   {
      long size=data.fileSize(); if(!size)return S;
      Str    s=FileSize(size); if(!data.size_known)s+='+';
      return s;
   }
   static Str TexSharp(C ListElm &data) {flt sharpness=data.texSharpness(); if(sharpness<2)return sharpness; return S;}

   static void IncludeTex(Memt<UID> &texs, C UID &tex_id) {if(tex_id.valid())texs.binaryInclude(tex_id, Compare);}
   static void IncludeTex(Memt<UID> &texs, C Elm &elm)
   {
      // material
      if(C ElmMaterial *mtrl_data=elm.mtrlData())
      {
         IncludeTex(texs, mtrl_data.base_0_tex);
         IncludeTex(texs, mtrl_data.base_1_tex);
      }

      // object
      if(Proj.list.include_texture_size_in_object)
         if(C ElmObj *obj_data=elm.objData())if(C Elm *mesh=Proj.findElm(obj_data.mesh_id))if(C ElmMesh *mesh_data=mesh.meshData()) // check for Obj->Mesh, and not directly Mesh, because ELM_MESH are always hidden, and wouldn't be processed for ICS_NEVER
            REPA(mesh_data.mtrl_ids)if(C Elm *mtrl=Proj.findElm(mesh_data.mtrl_ids[i]))IncludeTex(texs, *mtrl);
   }
   static void IncludeTex(Memt<UID> &texs, C ElmNode &node)
   {
      FREPA(node.children)
      {
         int child_i=node.children[i];
       C Elm &elm=Proj.elms[child_i];
         if( !elm.removed() || Proj.show_removed())
         if(  elm.publish() || Proj.list.include_unpublished_elm_size) // can use 'publish' instead of 'finalPublish' because if this function is called, then the parent was already checked
         {
            IncludeTex(texs, elm);
            IncludeTex(texs, Proj.hierarchy[child_i]); // we shouldn't check for ICS_ALWAYS or ICS_FOLDED here
         }
      }
   }
   void calcTexSize() // because texture size calculation is slow, it is calculated only on demand, it is slow because first we need to get all unique texture ID's, and then sum sizes of those textures, if we would sum all encountered texture ID's then we would get bigger values because the same texture ID's could be encountered multiple times
   {
      if(!tex_size_calculated)
      {
         tex_size_calculated=true;
         if(Proj.list.its && elm)
         if(elm.finalPublish() || Proj.list.include_unpublished_elm_size) // have to use 'finalPublish' because it's not called recursively, but can be called for any element at any time
         {
            Memt<UID> texs;
            IncludeTex(texs, *elm);
            if(Proj.list.ics)
               if(Proj.list.ics==Proj.list.ICS_ALWAYS || !(elm.opened() || Proj.list.list_all_children)) // ICS_ALWAYS or ICS_FOLDED
            {
               int i=Proj.elms.validIndex(elm); if(InRange(i, Proj.hierarchy))IncludeTex(texs, Proj.hierarchy[i]);
            }
            REPA(texs)includeValidTexSize(texs[i]);
         }
      }
   }
   long fileSize()C {ConstCast(T).calcTexSize(); return size;}

   flt texSharpness()C
   {
      flt sharpness=3;
      if(elm)if(C ElmMaterial *mtrl_data=elm.mtrlData())
      {
         if(mtrl_data.base_0_tex.valid())if(C TextureInfo *tex_info=TexInfos.find(mtrl_data.base_0_tex))MIN(sharpness, tex_info.sharpness);
       //if(mtrl_data.base_1_tex.valid())if(C TextureInfo *tex_info=TexInfos.find(mtrl_data.base_1_tex))MIN(sharpness, tex_info.sharpness); ignore base1
      }
      return sharpness;
   }

   void resetColor() {color=color_temp;}
   void highlight () {color.g=255;}
   bool hasVisibleChildren()C {return opened_icon!=null;}
   void hasVisibleChildren(bool has, bool opened)
   {
      if(!has  )opened_icon=null;else
      if(opened)opened_icon=Proj.arrow_down ();else
                opened_icon=Proj.arrow_right();
   }

   bool close()
   {
      if(hasVisibleChildren())
      {
         if(item && item.opened  ){item.opened=false ; return true;}
         if(elm  && elm .opened()){elm .opened(false); return true;}
      }
      return false;
   }
   void includeValidTexSize(C UID &tex_id) // assumes that 'tex_id' is valid
   {
      if(C TextureInfo *tex_info=TexInfos.find(tex_id))if(tex_info.knownFileSize()){size+=tex_info.file_size; return;}
      size_known=false;
   }
   void includeSize(Elm &elm)
   {
      if(Proj.list.its!=Proj.list.ITS_TEX) // add element size
      {
         if(elm.file_size<0) // if element file size is unknown
         {
            // TODO: what about 'ElmInFolder' (Worlds and MiniMaps)
            if(!ElmGame(elm.type))elm.file_size=0; // if doesn't have a game file, then currently we won't detect it, so set it as known
         }
         if(elm.file_size<0)size_known=false; // if at least one element has unknown size, then mark this as unknown too
         else               size+=elm.file_size; // increase the size
      }
   }
   void includeSize(C ListElm &src)
   {
      size_known&=src.size_known;
      size      +=src.size;
   }

   ListElm& set(ELM_TYPE type, C Str &name, bool edited, bool importing, bool removed, int depth, int vis_parent)
   {
      T.name  =name;
      T.depth =depth;
      T.offset=(Proj.list.flat_is ? 0 : depth*Proj.list.textSizeActual()*0.6);
      T.vis_parent_i=vis_parent;

      // icon
      icon=Proj.elmIcon(type)();

      // color
      if(edited )color=(importing ? PURPLE : RED);else if(importing)color.set(0, 128, 255, 255);else if(TextStyle *text_style=Proj.list.getTextStyle())color=text_style.color;else color=BLACK;
      if(removed)color.a=128;
      color_temp=color; // remember color in temp for fast restoring in 'elmHighlight'
      return T;
   }
   ListElm& set(Elm &elm, ElmNode &node, int depth, int vis_parent, bool parent_removed)
   {
      T.elm=&elm;
      return set(elm.type, elm.name, FlagTest(node.flag, ELM_EDITED), elm.importing(), elm.removed() || parent_removed, depth, vis_parent);
   }
   ListElm& set(EEItem &item, bool opened, int depth, bool parent_removed)
   {
      T.item=&item;
      hasVisibleChildren(item.children.elms()>0, opened);
      return set(item.type, item.base_name, FlagTest(item.flag, ELM_EDITED), false, parent_removed, depth, -1);
   }
}
/******************************************************************************/
Mems<int> ElmOrderArray;
int       ElmOrder(ELM_TYPE type) {return InRange(type, ElmOrderArray) ? ElmOrderArray[type] : ElmOrderArray.elms()+type;}
void  InitElmOrder()
{
   Memt<ELM_TYPE> elms;
   elms.add(ELM_NONE       );
   elms.add(ELM_FOLDER     );
   elms.add(ELM_ENUM       );
   elms.add(ELM_IMAGE      );
   elms.add(ELM_IMAGE_ATLAS);
   elms.add(ELM_FONT       );
   elms.add(ELM_TEXT_STYLE );
   elms.add(ELM_PANEL_IMAGE);
   elms.add(ELM_PANEL      );
   elms.add(ELM_GUI_SKIN   );
   elms.add(ELM_GUI        );
   elms.add(ELM_SHADER     );
   elms.add(ELM_MTRL       );
   elms.add(ELM_WATER_MTRL );
   elms.add(ELM_PHYS_MTRL  );
   elms.add(ELM_ANIM       );
   elms.add(ELM_OBJ_CLASS  );
   elms.add(ELM_OBJ        );
   elms.add(ELM_MESH       );
   elms.add(ELM_SKEL       );
   elms.add(ELM_PHYS       );
   elms.add(ELM_ICON_SETTS );
   elms.add(ELM_ICON       );
   elms.add(ELM_ENV        );
   elms.add(ELM_WORLD      );
   elms.add(ELM_MINI_MAP   );
   elms.add(ELM_SOUND      );
   elms.add(ELM_VIDEO      );
   elms.add(ELM_FILE       );
   elms.add(ELM_CODE       );
   elms.add(ELM_LIB        );
   elms.add(ELM_APP        );
   FREP(ELM_NUM)elms.include(ELM_TYPE(i)); // include elements that were not listed above (just in case)
   ElmOrderArray.setNum(elms.elms()); REPA(elms)ElmOrderArray(elms[i])=i;
}
/******************************************************************************/
class TexInfoGetter
{
   int       got_new_data=false;
   Str       tex_path;
   Memc<UID> tex_to_process;
   Memb<UID> tex_to_process1; // need to use 'Memb' to have const_mem_addr, because these can be added on the fly
   uintptr   thread_id=0;

  ~TexInfoGetter() {stopAndWait();} // stop processing before deleting other memebers

   void stop       () {BackgroundThreads.cancelFunc(CalcTexSharpness);}
   void stopAndWait()
   {
      stop();
      BackgroundThreads.waitFunc(CalcTexSharpness);
      tex_path       .clear();
      tex_to_process .clear();
      tex_to_process1.clear();
   }

   static int ImageLoad(ImageHeader &header, C Str &name)
   {
      if(GetThreadId()==TIG.thread_id) // process only inside 'TIG'
      {
         header.mode=IMAGE_SOFT;
       //if(ImageTI[header.type].compressed)header.type=IMAGE_R8G8B8A8; no need to do that, because there are only one decompressions per mip-map (1. extracting 2nd mip map to RGBA, 2. comparing 1st mip map with upscaled), doing this would only increase memory usage
         MIN(header.mip_maps, 2); // we need only 2 mip maps
      }
      return 0;
   }
   static flt ImageSharpness(C Image &image)
   {
      if(image.mipMaps()>=2)
      {
         Image mip; image.extractMipMap(mip, IMAGE_R8G8B8A8, IMAGE_SOFT, 1); // get 2nd mip-map
         mip.copy(mip, image.w(), image.h(), image.d(), -1, -1, 1, FILTER_LINEAR, false); // upscale smaller mip-map to full image size, use linear filtering because we simulate GPU filtering
         ImageCompare ic; if(ic.compare(image, mip))return ic.avg_dif2;
      }
      return -1; // can't calculate
   }
   static void CalcTexSharpness(UID &tex_id, ptr user, int thread_index)
   {
      Image img;

      // no need for 'ThreadMayUseGPUData' because we use only IMAGE_SOFT
      Images.lock(); // lock because other threads may modify 'image_load_shrink' too
      TIG.thread_id=GetThreadId();
      int (*image_load_shrink)(ImageHeader &image_header, C Str &name)=D.image_load_shrink; // remember current
      D.image_load_shrink=ImageLoad        ; bool ok=img.load(TIG.tex_path+EncodeFileName(tex_id));
      D.image_load_shrink=image_load_shrink; // restore
      Images.unlock();

      flt sharpness =ImageSharpness(img);
      if( sharpness>=0)
      {
         TexInfos(tex_id).sharpness=sharpness;
         AtomicSet(TIG.got_new_data, true);
      }
   }

   void getTexSharpnessFromProject()
   {
      if(Proj.list.tex_sharpness)
      {
         stopAndWait(); // stop first

         tex_path=Proj.tex_path;

         // get textures to process
         Memc<UID> proj_texs; Proj.getTextures(proj_texs, true); // process only existing
         FREPA(proj_texs)
         {
          C UID &tex_id=proj_texs[i];
            if(C TextureInfo *tex_info=TexInfos.find(tex_id))if(tex_info.knownSharpness())continue; // no need to process
            tex_to_process.add(tex_id);
         }

         // now when all array has been allocated and elements won't change their mem address, we can start processing
         FREPA(tex_to_process)BackgroundThreads.queue(tex_to_process[i], CalcTexSharpness);
      }else stop();
   }
   void savedTex(C UID &tex_id)
   {
      if(Proj.list.tex_sharpness)
      {
         UID &process=tex_to_process1.New(); process=tex_id;
         BackgroundThreads.queue(process, CalcTexSharpness); // use 'process' and not 'tex_id' to have const_mem_addr
      }
   }
}
TexInfoGetter TIG;
/******************************************************************************/
