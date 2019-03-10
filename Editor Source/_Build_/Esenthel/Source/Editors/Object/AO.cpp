/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
MeshAOClass MeshAO;
/******************************************************************************/

/******************************************************************************/
   const cchar8 *MeshAOClass::Func_t[]=
   {
      "Full"      , // 0
      "Quartic"   , // 1
      "Square"    , // 2
      "Linear"    , // 3
      "Linear Rev", // 4
      "Square Rev", // 5
   };
/******************************************************************************/
   void MeshAOClass::PreviewToggle(MeshAOClass &editor) {if(editor.preview_prop)editor.preview_prop->set(!editor.preview_prop->asBool());}
   void MeshAOClass::OK(MeshAOClass &editor)
   {
      //if(!editor.finished)Gui.msgBox(S, "Processing not finished yet");else
      {
         editor.hide();
         if(ObjEdit.mesh_elm)
         {
            ObjEdit.mesh_undos.set("ao");
            Mesh &mesh=ObjEdit.mesh;
            mesh.setVtxAO(editor.strength, editor.bias, editor.max, editor.ray_length*ObjEdit.posScale(), EPS*ObjEdit.posScale(), 1024, editor.func, &WorkerThreads).setRender();
          //Swap(mesh.newLod(), editor.baked);
            ObjEdit.setChangedMesh(true, false);
         }
      }
   }
   void MeshAOClass::ChangedParams(C Property &prop) {MeshAO.changedParams();}
          void MeshAOClass::changedParams()
   {
      change_id++;
      finished=false;
      if(info)info->name.set("Processing..");
   }
   bool MeshAOClass::needRebuild()C {return src_id!=baked_src_id || change_id!=baked_change_id;}
   bool MeshAOClass::Process(Thread &thread) {return ((MeshAOClass*)thread.user)->process();}
          bool MeshAOClass::process()
   {
      SyncLockerEx locker(lock);
      if(needRebuild())
      {
         baked_src_id   =   src_id;
         baked_change_id=change_id;
         flt scale=T.scale;

         MeshLod temp; Swap(src, temp);
         locker.off();

         ThreadMayUseGPUData();

         MeshLod processed; processed.create(temp);
         processed.setVtxAO(strength, bias, max, ray_length*scale, EPS*scale, 256, func, &WorkerThreads).setRender(false);

         locker.on();
         if(baked_src_id==src_id)
         {
            Swap(temp, src);
          //if(baked_change_id==change_id)
            {
               Swap(processed, T.processed);
               processed_ready=true;
            }
         }
      }else
      {
         locker.off();
         Time.wait(1);
      }
      return true;
   }
   void MeshAOClass::startThread() {if(!thread.active())thread.create(Process, this);}
   void MeshAOClass::createSrc()
   {
      {
         MeshLod temp; temp.create(ObjEdit.getLod()).setBase().delRender(); SyncLocker locker(lock); Swap(temp, src); src_id++; scale=ObjEdit.posScale(); changedParams(); baked.del(); baked_valid=finished=false;
      }
      startThread();
   }
           MeshAOClass& MeshAOClass::activate(){if(hidden()){createSrc(); thread.cancelStop(); ObjEdit.lod.edit_dist.set(-1);} ::EE::GuiObj::activate(); return T;}
   MeshAOClass& MeshAOClass::hide(){                          thread.      stop();                                 ::EE::Window::hide    (); return T;}
  MeshAOClass::~MeshAOClass() {thread.del();}
   MeshAOClass& MeshAOClass::del(){thread.del(); ::EE::Window::del(); return T;}
   MeshAOClass& MeshAOClass::create()
   {
      ::EE::Window::create("Set Per-Vertex AO").hide(); button[2].show();
      preview_prop=&props.New().create("Preview"         , MEMBER(MeshAOClass, preview   )).desc("Keyboard Shortcut: Alt+P");
                    props.New();
                    props.New().create("Strength"        , MEMBER(MeshAOClass, strength  )).range(0, 2).mouseEditSpeed(0.4f).desc("AO Intensity");
                    props.New().create("Bias"            , MEMBER(MeshAOClass, bias      )).range(0, 1).mouseEditSpeed(0.4f).setSlider();
                    props.New().create("Limit"           , MEMBER(MeshAOClass, max       )).range(0, 1).mouseEditSpeed(0.4f).setSlider().desc("Max limit for applying AO");
                    props.New().create("Ray Length"      , MEMBER(MeshAOClass, ray_length)).range(0, 1024).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
                    props.New().create("Falloff Function", MEMBER(MeshAOClass, func      )).setEnum(Func_t, Elms(Func_t));
              info=&props.New().create(S);
      ts.reset().size=0.045f; ts.align.set(1, 0); Rect r=AddProperties(props, T, Vec2(0.02f, -0.02f), 0.05f, 0.22f, &ts); REPAO(props).autoData(&MeshAO).changed(ChangedParams); preview_prop->changed(null, null);
      T+=ok.create(Rect_U(r.down()-Vec2(0, 0), 0.25f, 0.06f), "OK").func(OK, T);
      r|=ok.rect(); Vec2 size=Vec2(r.max.x, -r.min.y)+0.02f+defaultInnerPaddingSize();
      rect(Rect_RU(D.w(), D.h(), size.x, size.y));
      return T;
   }
   void MeshAOClass::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(visible() && gpc.visible)
      {
         if(needRebuild())startThread();
         if(processed_ready)
         {
            SyncLocker locker(lock);
            if(processed_ready)
            {
               processed_ready=false;
               baked_valid    =true;
               Swap(processed, baked);
               if(baked_change_id==change_id)
               {
                  finished=true;
                  if(info)info->name.clear();
               }
            }
         }
      }
   }
MeshAOClass::MeshAOClass() : preview(true), baked_valid(false), processed_ready(false), finished(false), strength(0.5f), bias(0), max(1), ray_length(1), scale(1), func(MAF_FULL), src_id(0), baked_src_id(0), change_id(0), baked_change_id(0), info(null), preview_prop(null) {}

/******************************************************************************/
