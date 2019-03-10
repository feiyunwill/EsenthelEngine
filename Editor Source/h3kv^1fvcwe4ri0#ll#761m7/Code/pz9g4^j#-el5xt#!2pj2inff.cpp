/******************************************************************************/
class DetectSimilarTextures : PropWin
{
   static const bool Decompress=false; // performance was the same for both options, so keep 'Decompress' as false, to be able to process bigger sizes for better precision
   static const int  MaxTexSize=Decompress ? 64 : 128; // a very big project can have ~7000 textures, total memory for that would be (7000*128*128)>>20 = 109 MB when using BC7, which is acceptable memory usage

   class Data
   {
      UID id=UIDZero;
      Str name;
      
      void set(C UID &id)
      {
         T.id=id;
         T.name=EncodeFileName(id);
      }
   }
   class Pair
   {
      UID a, b;

      void set(C UID &a, C UID &b) {T.a=a; T.b=b;}
   }
   class UIDEx : UID
   {
      bool mtrl_base_1=false;
   }
   class ImageEx
   {
      bool        mtrl_base_1=false;
      Mems<Image> mips; // since we can't lock multiple mip-maps at the same time, we need to store them as separate images
      
      void create(bool mtrl_base_1, C Image &src)
      {
         T.mtrl_base_1=mtrl_base_1;
         mips.setNum(src.mipMaps());
         REPA(mips)src.extractMipMap(mips[i], Decompress ? IMAGE_R8G8B8A8 : -1, -1, i);
      }
   }
   Memc<Data>                  data;
   List<Data>                  list;
   Region                      region;
   Progress                    progress;
   Threads                     threads;
   Thread                      io_thread;
   uintptr                     io_thread_id=0;
   Memc<UIDEx>                 proj_texs;
   ThreadSafeMap<UID, ImageEx> loaded_texs(Compare); // make thread-safe just in case
   Memc<Pair>                  similar_pair;
   SyncLock                    similar_pair_lock;
   bool                        pause=false;
   int                         compared=0;

   bool mtrl_base_1   =false;
   flt  avg_difference=0.02,
        similar       =0.0,
        similar_dif   =0.1;

   static void CompareImage(UID &a_id, UID &b_id, int thread_index)
   {
      if(ImageEx *a_image=DST.loaded_texs.find(a_id))
      if(ImageEx *b_image=DST.loaded_texs.find(b_id))
      {
         if(DST.mtrl_base_1!=(a_image.mtrl_base_1 || b_image.mtrl_base_1))goto compared;
         ImageCompare ic;
         flt  avg_difference_mip=DST.avg_difference+0.041; // this is used for mip-maps, because mip-maps (especially due to compression) can have bigger difference than the biggest mip-map. One image in tests with 0.002233495 'avg_dif2' on biggest mip had 0.043038268 'avg_dif2' on one of the mip maps.
         int  mips=Min(a_image.mips.elms(), b_image.mips.elms());
         FREP(mips)
         {
            if(!ic.compare(a_image.mips[a_image.mips.elms()-1-i], b_image.mips[b_image.mips.elms()-1-i], DST.similar_dif, false))goto compared; // start comparing from smallest mip-map, if failed to compare
            if( ic.skipped || DST.pause)goto compared; // if comparison was skipped
            if(i==mips-1) // biggest mip-map that we're going to test
            {
               if(ic.avg_dif2>DST.avg_difference // if too different
               || ic.similar <DST.similar        // if not similar enough
               )goto compared;
            }else // for smaller mip-maps, use values with epsilon added
            {
               if(ic.avg_dif2>avg_difference_mip // if too different
               || ic.similar <DST.similar        // if not similar enough
               )goto compared;
            }
         }
         {
            SyncLocker locker(DST.similar_pair_lock);
            DST.similar_pair.New().set(a_id, b_id);
         }
      }
   compared:
      AtomicInc(DST.compared);
   }
   static int ImageLoad(ImageHeader &header, C Str &name)
   {
      if(GetThreadId()==DST.io_thread_id) // process only on the 'IOThread'
      {
         header.mode=IMAGE_SOFT;
         int    shrink=0; for(uint max_size=header.size.max(); max_size>DST.MaxTexSize; max_size/=2)shrink++;
         return shrink;
      }
      return 0;
   }
   static bool IOThread(Thread &t) {return DST.ioThread();}
          bool ioThread()
   {
      io_thread_id=GetThreadId();
      Image              img;
      Memt<Threads.Call> calls;
      REPA(proj_texs)
      {
         if(io_thread.wantStop())break;

       C UIDEx &tex_id=proj_texs[i];
         if(!loaded_texs.find(tex_id)) // if not yet loaded
         {
            // no need for 'ThreadMayUseGPUData' because we use only IMAGE_SOFT
            Images.lock(); // lock because other threads may modify 'image_load_shrink' too
            int (*image_load_shrink)(ImageHeader &image_header, C Str &name)=D.image_load_shrink; // remember current
            D.image_load_shrink=ImageLoad        ; bool ok=img.load(Proj.texPath(tex_id));
            D.image_load_shrink=image_load_shrink; // restore
            Images.unlock();

            if(ok)
            {
               ImageEx tex; tex.create(tex_id.mtrl_base_1, img);
               ImageEx &loaded   =*loaded_texs(tex_id); Swap(loaded, tex);
             C UID     &loaded_id=*loaded_texs.dataToKey(&loaded);
               REPA(loaded_texs)
               {
                C UID &tex_id=loaded_texs.lockedKey(i);
                  if(loaded_id!=tex_id)calls.New().set(ConstCast(loaded_id), CompareImage, ConstCast(tex_id)); // since we can pass only pointers, then use Tex ID pointer to 'loaded_texs.key' and not 'proj_texs'
               }
               threads.queue(calls); calls.clear();
            }else proj_texs.remove(i); // if failed to load, then remove so we can set progress correctly
         }
      }
      return false;
   }

   static void Changed(C Property &prop) {DST.reset();}
   void reset()
   {
      stop2();

      Memc<UID> &texs=proj_texs;
      Proj.getTextures(texs, true); // process only existing
      texs.sort(Compare);
      REPA(Proj.elms)
      {
         Elm &elm=Proj.elms[i];
         if(ElmMaterial *mtrl_data=elm.mtrlData())if(mtrl_data.base_1_tex.valid())
         {
            int texs_i; if(texs.binarySearch(mtrl_data.base_1_tex, texs_i, Compare))proj_texs[texs_i].mtrl_base_1=true;
         }
      }
      REPA(loaded_texs)if(!texs.binaryHas(loaded_texs.lockedKey(i), Compare))loaded_texs.remove(i); // remove loaded textures if they're no longer present in the project

      // request compare on all loaded textures up to this point
      Memt<Threads.Call> calls;
      REPA(loaded_texs)
      {
       C UID &tex_id=loaded_texs.lockedKey(i);
         REPD(j, i)calls.New().set(ConstCast(tex_id), CompareImage, ConstCast(loaded_texs.lockedKey(j)));
      }
      threads.queue(calls);

      io_thread.create(IOThread); // start thread at the end
   }
  ~DetectSimilarTextures() {stop();}
   void stop2()
   {
      io_thread.del(); // delete the thread first, as it may queue calls for 'threads'

      pause=true;
      threads.cancelFunc(CompareImage); // cancel all comparison functions
      threads.  waitFunc(CompareImage); // wait until all finished
      pause=false;
      compared=0;
      similar_pair.clear();
      data.clear();
      list.clear();
   }
   void stop()
   {
      stop2();
      threads.del();
      loaded_texs.del();
   }
   virtual DetectSimilarTextures& show()override
   {
      if(hidden())
      {
         super.show();
         rect(Rect_C(0, 0, 1.6, D.h()*1.75));
         progress.clear().show();

         threads.create(false);
         reset();
      }
      return T;
   }
   virtual DetectSimilarTextures& hide()override
   {
      if(visible())
      {
         stop();
         super.hide();
      }
      return T;
   }
   void clearProj() {hide();}
   void addSimilar(C UID &a, C UID &b)
   {
      FREPA(data)
      {
         Data &d=data[i];
         bool has_a=(d.id==a), has_b=(d.id==b);
         if(  has_a || has_b)
         {
            for(i++; InRange(i, data); i++)
            {
             C UID &id=data[i].id;
               if(!id.valid())break;
               if( id==a || id==b)return;
            }
            if(list.cur>=i)list.cur++;
            data.NewAt(i).set(has_a ? b : a);
            return;
         }
      }
      if(data.elms())data.New(); // add empty separator
      data.New().set(a); // add 'a'
      data.New().set(b); // add 'b'
   }
   void addSimilarAll(C UID &a, C UID &b)
   {
      bool start=true;
      FREPA(data)
      {
         Data &d=data[i];
         if(!d.id.valid())start=true;else
         if(start)
         {
            if(d.id==a)
            {
               for(i++; InRange(i, data) && data[i].id.valid(); )i++; // skip all children
               data.NewAt(i).set(b); // add 'b' as child to the end of the list
               return;
            }
            start=false;
         }
      }
      if(data.elms())data.New(); // add empty separator
      data.New().set(a); // add 'a' as first (parent)
      data.New().set(b); // add 'b' as next  (child)
   }
   virtual Rect sizeLimit()C override {Rect r=super.sizeLimit(); r.min.set(0.6, 0.4); return r;}
                          C Rect& rect()C {return super.rect();}
   virtual DetectSimilarTextures& rect(C Rect &rect)override
   {
      super.rect(rect);
      progress.rect(Rect_LU(0, 0, clientWidth(), 0.017));
      region  .rect(Rect(region.rect().min.x, -clientHeight()+0.03, clientWidth()-0.02, -0.03));
      return T;
   }
   static void CurChanged(DetectSimilarTextures &dst)
   {
      if(Data *data=dst.list())Proj.elmActivate(Proj.findElmByTexture(data.id));
   }
   void create()
   {
      add("Average Difference", MEMBER(DetectSimilarTextures, avg_difference)).range(0, 0.2).desc("Total average difference between texture pixels").mouseEditSpeed(0.02);
      add("Material Base 1"   , MEMBER(DetectSimilarTextures, mtrl_base_1   )).desc("If compare Material Base 1 Textures, such as Normal, Specular and Glow");
   #if 0 // not needed
      add("And"); // use AND because OR would list more textures
      add("Similar Portion", MEMBER(DetectSimilarTextures, similar    )).range(0, 1).desc("Portion of the entire texture that is similar.\nFor example if 2 textures have the same top half, but the bottom half is different, then this will be 0.5\nEach pixels are considered similar if their color difference is smaller than \"Similar Limit\".");
      add("Similar Limit"  , MEMBER(DetectSimilarTextures, similar_dif)).range(0, 1).desc("Max difference between pixel colors to consider them similar");
   #endif
      flt h=0.043;
      Rect params=super.create("Detect Similar Textures", Vec2(0.02, -0.02), 0.036, h, 0.15); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      T.flag|=WIN_RESIZABLE;
      autoData(this).changed(Changed);

      T+=progress.create();
      T+=region  .create(Vec2(params.max.x+0.05, 0));
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Data, name), LCW_DATA, "Texture"),
      };
      region+=list.create(lc, Elms(lc), true).elmHeight(0.038).textSize(0, 1).curChanged(CurChanged, T); FlagDisable(list.flag, LIST_SORTABLE); list.cur_mode=LCM_ALWAYS;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         progress.set(compared, UniquePairs(proj_texs.elms())); progress.visible(progress()<1 && proj_texs.elms()); // !! do not merge into a single instruction !!
         if(similar_pair.elms())
         {
            Memt<Pair> similar;
            {
               SyncLocker locker(similar_pair_lock);
               similar=T.similar_pair; T.similar_pair.clear();
            }
            FREPA(similar)
            {
            #if 1
               addSimilar(similar[i].a, similar[i].b);
            #else // can be slow for lots of pairs
               addSimilarAll(similar[i].a, similar[i].b);
               addSimilarAll(similar[i].b, similar[i].a);
            #endif
            }
            list.setData(data, null, true);
         }
      }
   }
}
DetectSimilarTextures DST;
/******************************************************************************/
