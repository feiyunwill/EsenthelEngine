/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
DetectSimilarTextures DST;
/******************************************************************************/

/******************************************************************************/
   const bool DetectSimilarTextures::Decompress=false;
   const int  DetectSimilarTextures::MaxTexSize=Decompress ? 64 : 128;
/******************************************************************************/
      void DetectSimilarTextures::Data::set(C UID &id)
      {
         T.id=id;
         T.name=EncodeFileName(id);
      }
      void DetectSimilarTextures::Pair::set(C UID &a, C UID &b) {T.a=a; T.b=b;}
      void DetectSimilarTextures::ImageEx::create(bool mtrl_base_1, C Image &src)
      {
         T.mtrl_base_1=mtrl_base_1;
         mips.setNum(src.mipMaps());
         REPA(mips)src.extractMipMap(mips[i], Decompress ? IMAGE_R8G8B8A8 : -1, -1, i);
      }
   void DetectSimilarTextures::CompareImage(UID &a_id, UID &b_id, int thread_index)
   {
      if(ImageEx *a_image=DST.loaded_texs.find(a_id))
      if(ImageEx *b_image=DST.loaded_texs.find(b_id))
      {
         if(DST.mtrl_base_1!=(a_image->mtrl_base_1 || b_image->mtrl_base_1))goto compared;
         ImageCompare ic;
         flt  avg_difference_mip=DST.avg_difference+0.041f; // this is used for mip-maps, because mip-maps (especially due to compression) can have bigger difference than the biggest mip-map. One image in tests with 0.002233495 'avg_dif2' on biggest mip had 0.043038268 'avg_dif2' on one of the mip maps.
         int  mips=Min(a_image->mips.elms(), b_image->mips.elms());
         FREP(mips)
         {
            if(!ic.compare(a_image->mips[a_image->mips.elms()-1-i], b_image->mips[b_image->mips.elms()-1-i], DST.similar_dif, false))goto compared; // start comparing from smallest mip-map, if failed to compare
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
   int DetectSimilarTextures::ImageLoad(ImageHeader &header, C Str &name)
   {
      if(GetThreadId()==DST.io_thread_id) // process only on the 'IOThread'
      {
         header.mode=IMAGE_SOFT;
         int    shrink=0; for(uint max_size=header.size.max(); max_size>DST.MaxTexSize; max_size/=2)shrink++;
         return shrink;
      }
      return 0;
   }
   bool DetectSimilarTextures::IOThread(Thread &t) {return DST.ioThread();}
          bool DetectSimilarTextures::ioThread()
   {
      io_thread_id=GetThreadId();
      Image              img;
      Memt<Threads::Call> calls;
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
   void DetectSimilarTextures::Changed(C Property &prop) {DST.reset();}
   void DetectSimilarTextures::reset()
   {
      stop2();

      Memc<UID> &texs=proj_texs;
      Proj.getTextures(texs, true); // process only existing
      texs.sort(Compare);
      REPA(Proj.elms)
      {
         Elm &elm=Proj.elms[i];
         if(ElmMaterial *mtrl_data=elm.mtrlData())if(mtrl_data->base_1_tex.valid())
         {
            int texs_i; if(texs.binarySearch(mtrl_data->base_1_tex, texs_i, Compare))proj_texs[texs_i].mtrl_base_1=true;
         }
      }
      REPA(loaded_texs)if(!texs.binaryHas(loaded_texs.lockedKey(i), Compare))loaded_texs.remove(i); // remove loaded textures if they're no longer present in the project

      // request compare on all loaded textures up to this point
      Memt<Threads::Call> calls;
      REPA(loaded_texs)
      {
       C UID &tex_id=loaded_texs.lockedKey(i);
         REPD(j, i)calls.New().set(ConstCast(tex_id), CompareImage, ConstCast(loaded_texs.lockedKey(j)));
      }
      threads.queue(calls);

      io_thread.create(IOThread); // start thread at the end
   }
  DetectSimilarTextures::~DetectSimilarTextures() {stop();}
   void DetectSimilarTextures::stop2()
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
   void DetectSimilarTextures::stop()
   {
      stop2();
      threads.del();
      loaded_texs.del();
   }
   DetectSimilarTextures& DetectSimilarTextures::show()
{
      if(hidden())
      {
         ::EE::Window::show();
         rect(Rect_C(0, 0, 1.6f, D.h()*1.75f));
         progress.clear().show();

         threads.create(false);
         reset();
      }
      return T;
   }
   DetectSimilarTextures& DetectSimilarTextures::hide()
{
      if(visible())
      {
         stop();
         ::PropWin::hide();
      }
      return T;
   }
   void DetectSimilarTextures::clearProj() {hide();}
   void DetectSimilarTextures::addSimilar(C UID &a, C UID &b)
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
   void DetectSimilarTextures::addSimilarAll(C UID &a, C UID &b)
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
   Rect DetectSimilarTextures::sizeLimit()C {Rect r=::EE::Window::sizeLimit(); r.min.set(0.6f, 0.4f); return r;}
                          C Rect& DetectSimilarTextures::rect()C {return ::EE::Window::rect();}
   DetectSimilarTextures& DetectSimilarTextures::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      progress.rect(Rect_LU(0, 0, clientWidth(), 0.017f));
      region  .rect(Rect(region.rect().min.x, -clientHeight()+0.03f, clientWidth()-0.02f, -0.03f));
      return T;
   }
   void DetectSimilarTextures::CurChanged(DetectSimilarTextures &dst)
   {
      if(Data *data=dst.list())Proj.elmActivate(Proj.findElmByTexture(data->id));
   }
   void DetectSimilarTextures::create()
   {
      add("Average Difference", MEMBER(DetectSimilarTextures, avg_difference)).range(0, 0.2f).desc("Total average difference between texture pixels").mouseEditSpeed(0.02f);
      add("Material Base 1"   , MEMBER(DetectSimilarTextures, mtrl_base_1   )).desc("If compare Material Base 1 Textures, such as Normal, Specular and Glow");
   #if 0 // not needed
      add("And"); // use AND because OR would list more textures
      add("Similar Portion", MEMBER(DetectSimilarTextures, similar    )).range(0, 1).desc("Portion of the entire texture that is similar.\nFor example if 2 textures have the same top half, but the bottom half is different, then this will be 0.5\nEach pixels are considered similar if their color difference is smaller than \"Similar Limit\".");
      add("Similar Limit"  , MEMBER(DetectSimilarTextures, similar_dif)).range(0, 1).desc("Max difference between pixel colors to consider them similar");
   #endif
      flt h=0.043f;
      Rect params=::PropWin::create("Detect Similar Textures", Vec2(0.02f, -0.02f), 0.036f, h, 0.15f); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      T.flag|=WIN_RESIZABLE;
      autoData(this).changed(Changed);

      T+=progress.create();
      T+=region  .create(Vec2(params.max.x+0.05f, 0));
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Data, name), LCW_DATA, "Texture"),
      };
      region+=list.create(lc, Elms(lc), true).elmHeight(0.038f).textSize(0, 1).curChanged(CurChanged, T); FlagDisable(list.flag, LIST_SORTABLE); list.cur_mode=LCM_ALWAYS;
   }
   void DetectSimilarTextures::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
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
DetectSimilarTextures::DetectSimilarTextures() : io_thread_id(0), loaded_texs(Compare), pause(false), compared(0), mtrl_base_1(false), avg_difference(0.02f), similar(0.0f), similar_dif(0.1f) {}

DetectSimilarTextures::Data::Data() : id(UIDZero) {}

DetectSimilarTextures::UIDEx::UIDEx() : mtrl_base_1(false) {}

DetectSimilarTextures::ImageEx::ImageEx() : mtrl_base_1(false) {}

/******************************************************************************/
