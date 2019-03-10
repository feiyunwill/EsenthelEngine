/******************************************************************************/
class IDReplace
{
   UID from, to;

   void set(C UID &from, C UID &to) {T.from=from; T.to=to;}

   static int Compare(C IDReplace &a, C IDReplace &b) {return .Compare(a.from, b.from);}
   static int Compare(C IDReplace &a, C UID       &b) {return .Compare(a.from, b     );}
}
/******************************************************************************/
class MergeSimilarMaterials : PropWin
{
   class Mtrl
   {
      Str  name, color_name;
      UID  elm_id, base_0_tex, base_1_tex, detail_tex, macro_tex, reflect_tex, light_tex;
      bool cull;
      MATERIAL_TECHNIQUE tech;
      Vec4 col;
      flt  bump, spec, glow, reflect, uv_scale;

      bool similar(C Mtrl &m)C
      {
         if(MSM.            name   &&        name!=m.       name)return false;
         if(MSM.      color_name   &&  color_name!=m. color_name)return false;
         if(MSM.       color_tex   &&  base_0_tex!=m. base_0_tex)return false;
         if(MSM.      normal_tex   &&  base_1_tex!=m. base_1_tex)return false;
         if(MSM.      detail_tex   &&  detail_tex!=m. detail_tex)return false;
         if(MSM.       macro_tex   &&   macro_tex!=m.  macro_tex)return false;
         if(MSM.     reflect_tex   && reflect_tex!=m.reflect_tex)return false;
         if(MSM.       light_tex   &&   light_tex!=m.  light_tex)return false;
         if(MSM.            cull   &&        cull!=m.       cull)return false;
         if(MSM.            tech   &&        tech!=m.       tech)return false;
         if(MSM.  color_value_on() && Abs(col     -m.col     ).max()>MSM.  color_value)return false;
         if(MSM.   bump_value_on() && Abs(bump    -m.bump    )      >MSM.   bump_value)return false;
         if(MSM.   spec_value_on() && Abs(spec    -m.spec    )      >MSM.   spec_value)return false;
         if(MSM.   glow_value_on() && Abs(glow    -m.glow    )      >MSM.   glow_value)return false;
         if(MSM.reflect_value_on() && Abs(reflect -m.reflect )      >MSM.reflect_value)return false;
         if(MSM.     uv_scale_on() && Abs(uv_scale-m.uv_scale)      >MSM.     uv_scale)return false;
         return true;
      }
      Mtrl& set(C UID &elm_id, C EditMaterial &m)
      {
         if(Elm *elm=Proj.findElm(elm_id))name=elm.name;
         T.elm_id=elm_id;
         color_name=GetBase(Edit.FileParams(m.color_map).name);
         base_0_tex=m.base_0_tex;
         base_1_tex=m.base_1_tex;
         detail_tex=m.detail_tex;
          macro_tex=m.macro_tex;
        reflect_tex=m.reflection_tex;
          light_tex=m.light_tex;
         cull=m.cull;
         tech=m.tech;
         col=m.color;
         bump=m.bump;
         spec=m.specular;
         glow=m.glow;
         reflect=m.reflection;
         uv_scale=m.tex_scale;
         return T;
      }
   }
   static int CompareMtrl(C Mtrl &a, C Mtrl &b) {return ComparePathNumber(Proj.elmFullName(b.elm_id), Proj.elmFullName(a.elm_id));} // compare reversed because we're going to process materials in reversed order later
   static void Detect(MergeSimilarMaterials &msm)
   {
      MtrlEdit.flush(); // flush Material Editor changes first

           Memc<Mtrl>  mtrls;
      Memc<Memc<Mtrl>> similar;

      // first load all existing materials
      FREPA(Proj.elms)
      {
         Elm &elm=Proj.elms[i]; if(elm.type==ELM_MTRL && elm.finalExists())
         {
            EditMaterial edit_mtrl;
            if(edit_mtrl.load(Proj.editPath(elm)))
               if(!msm.color_is || edit_mtrl.base_0_tex.valid())
                  mtrls.New().set(elm.id, edit_mtrl);
         }
      }
      mtrls.sort(CompareMtrl);

      // detect similar ones
      for(; mtrls.elms(); )
      {
         Mtrl &mtrl=mtrls.last();

         // first check in already merged groups
         REPA(similar)
         {
            Memc<Mtrl> &group=similar[i];
            if(mtrl.similar(group[0])) // compare with the first one in the group only
            {
               Swap(group.New(), mtrl);
               mtrls.removeLast();
               goto merged;
            }
         }

         // now check all single materials
         REPD(i, mtrls.elms()-1)
         {
            Mtrl &test=mtrls[i];
            if(mtrl.similar(test)) // 2 materials are similar
            {
               Memc<Mtrl> &group=similar.New(); // create a new group
               Swap(group.New(), mtrl);
               Swap(group.New(), test);
               mtrls.removeLast(       ); // first remove the one with higher index
               mtrls.remove    (i, true); // now   remove the one with lower  index
               goto merged;
            }
         }

         mtrls.removeLast(); // no similar were detected so just remove it

         merged:;
      }

      // setup data
      MSM.replace.clear();
      MSM.data   .clear();
      FREPA(similar)
      {
       C Memc<Mtrl> &src=similar[i];
         for(int i=1; i<src.elms(); i++)MSM.replace.New().set(src[i].elm_id, src[0].elm_id);
         if(MSM.data.elms())MSM.data.New(); FREPA(src)MSM.data.New().id=src[i].elm_id;
      }
      MSM.replace.sort(IDReplace.Compare);
      MSM.list.setData(MSM.data);
   }
   void display(C MemPtr<UID> &elm_ids)
   {
      Memc<UID> mtrl_ids; REPA(elm_ids)if(Elm *mtrl=Proj.findElm(elm_ids[i], ELM_MTRL))mtrl_ids.add(mtrl.id);
      if(mtrl_ids.elms()>1)
      {
         replace .clear();
         data    .clear();
         mtrl_ids.sort(CompareProjPath);
         FREPA(mtrl_ids)
         {
            if(i)replace.New().set(mtrl_ids[i], mtrl_ids[0]);
            data.New().id=mtrl_ids[i];
         }
         replace.sort(IDReplace.Compare);
         list.setData(data);
         activate();
      }
   }
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))
      {
         display(elms);
         elms.clear();
      }
   }
   static void Merge(MergeSimilarMaterials &msm)
   {
      if(msm.replace.elms())
      {
         ReplaceIDs=msm.replace;
         msm.clearProj();
         IDReplaceState.set(StateFadeTime);
      }
   }

   class Data
   {
      UID id=UIDZero;

      static Str AsText(C Data &data) {return Proj.elmFullName(data.id);}
   }
   Memc<IDReplace> replace;
   Memc<Data>      data;
   List<Data>      list;
   Region          region;

   bool     name=false, color_name=false, color_is=false, color_tex=true, normal_tex=true, detail_tex=true, macro_tex=true, reflect_tex=true, light_tex=true, tech=true, cull=true;
   flt      color_value=0.1, bump_value=0.1, spec_value=0.1, glow_value=0.1, reflect_value=0.1, uv_scale=0.1;
   CheckBox color_value_on, bump_value_on, spec_value_on, glow_value_on, reflect_value_on, uv_scale_on;
   Button   detect, merge;
   Text     detected;

   void clearProj()
   {
      replace.clear();
      data   .clear();
      list   .clear();
   }
   void create()
   {
      add("Require Same:");
      add("Material Name"    , MEMBER(MergeSimilarMaterials, name)).desc("Materials will be tested against their name in the project");
      add("Color Source Name", MEMBER(MergeSimilarMaterials, color_name)).desc("Materials will be tested against the source file name of the color texture.\nOnly the base part of the file name (without the path) is checked.");
      add("Color Texture"    , MEMBER(MergeSimilarMaterials, color_tex )).desc("Materials will be tested against the actual color texture image");
      add("Normal Texture"   , MEMBER(MergeSimilarMaterials, normal_tex));
      add("Detail Texture"   , MEMBER(MergeSimilarMaterials, detail_tex));
      add("Macro Texture"    , MEMBER(MergeSimilarMaterials, macro_tex));
      add("Reflect Texture"  , MEMBER(MergeSimilarMaterials, reflect_tex));
      add("Light Texture"    , MEMBER(MergeSimilarMaterials, light_tex));
      add("Technique"        , MEMBER(MergeSimilarMaterials, tech));
      add("Cull"             , MEMBER(MergeSimilarMaterials, cull));
      add("Color Value"      , MEMBER(MergeSimilarMaterials, color_value  )).desc("Tolerance").range(0, 2);
      add("Bump Value"       , MEMBER(MergeSimilarMaterials, bump_value   )).desc("Tolerance").range(0, 1);
      add("Specular Value"   , MEMBER(MergeSimilarMaterials, spec_value   )).desc("Tolerance").range(0, 1);
      add("Glow Value"       , MEMBER(MergeSimilarMaterials, glow_value   )).desc("Tolerance").range(0, 1);
      add("Reflection Value" , MEMBER(MergeSimilarMaterials, reflect_value)).desc("Tolerance").range(0, 1);
      add("UV Scale"         , MEMBER(MergeSimilarMaterials, uv_scale     )).desc("Tolerance").min  (0);
      add();
      add("Merge Only If:");
      add("Color Texture Exists", MEMBER(MergeSimilarMaterials, color_is)).desc("When this option is selected, then Materials will not be merged if they don't have a color texture image set");
      flt h=0.043;
      super.create("Merge Similar Materials", Vec2(0.02, -0.02), 0.036, h, 0.15); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      int c=0;
      FREPA(props)if(props[i].textline.is())
      {
         CheckBox *check=null;
         switch(c++)
         {
            case 0: check=&  color_value_on; break;
            case 1: check=&   bump_value_on; break;
            case 2: check=&   spec_value_on; break;
            case 3: check=&   glow_value_on; break;
            case 4: check=&reflect_value_on; break;
            case 5: check=&     uv_scale_on; break;
         }
         if(check)T+=check.create(Rect_LU(props[i].textline.pos(), props[i].textline.size().y), true);
         props[i].textline.move(Vec2(h, 0));
         props[i].button  .move(Vec2(h, 0));
      }
      autoData(this);
      Vec2 params=rect().size(); params.x+=h; Vec2 size=params; size.x+=1.3; size.y+=0.6;
      rect(Rect_C(0, size));
      T+=detected.create(Vec2(Avg(params.x, size.x)-0.05, -0.04), "Detected:", &ts);
      T+=detect.create(Rect_D(size.x*1/3, -clientHeight()+0.04, 0.3, 0.06), "Detect").func(Detect, T).desc("Detect similar materials");
      T+= merge.create(Rect_D(size.x*2/3, -clientHeight()+0.04, 0.3, 0.06), "Merge" ).func(Merge , T).desc("Merge detected materials");

      T+=region.create(Rect(params.x, detect.rect().max.y+0.04, size.x-0.02, detected.pos().y-0.03));
      ListColumn lc[]=
      {
         ListColumn(Data.AsText, LCW_DATA, "File"),
      };
      region+=list.create(lc, Elms(lc), true).elmHeight(0.038).textSize(0, 1); FlagDisable(list.flag, LIST_SORTABLE); list.cur_mode=LCM_MOUSE;
   }
   virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(visible() && gpc.visible)
      {
         if(Ms.tappedFirst(0) && Gui.ms()==&list)if(Data *data=list())if(Elm *elm=Proj.findElm(data.id))MtrlEdit.toggle(elm);
      }
   }
}
MergeSimilarMaterials MSM;
Memc<IDReplace> ReplaceIDs;
State           IDReplaceState(UpdateIDReplace, DrawIDReplace, InitIDReplace, ShutIDReplace);
C IDReplace*    ReplaceID(C UID &src, Memc<IDReplace> &replace=ReplaceIDs) {return src.valid() ? replace.binaryFind(src, IDReplace.Compare) : null;}
/******************************************************************************/
bool ThreadIDReplace(Thread &thread)
{
   ThreadMayUseGPUData();
   FREPA(Proj.elms)
   {
      if(thread.wantStop())return false;
      Elm &elm=Proj.elms[i]; if(elm.data)
      {
         if(                    ReplaceID(elm.       id))elm.setRemoved(true     ); // if this element is replaced with another one, then remove it
         if(C IDReplace *parent=ReplaceID(elm.parent_id))elm.setParent (parent.to); // if parent of this element is being removed, then assign this element to the replacement
         FREPA(ReplaceIDs)if(elm.data.mayContain(ReplaceIDs[i].from))
         {
            bool changed=false;
            switch(elm.type)
            {
               case ELM_MESH:
               {
                  Mesh mesh; if(Load(mesh, Proj.editPath(elm), Proj.game_path))
                  {
                     REPD (l, mesh.lods( ))
                     REPAD(p, mesh.lod (l))
                     {
                        MeshPart   &part=mesh.lod(l).parts[p];
                        bool        changed_multi_mtrl=false;
                        MaterialPtr mtrls[4]; REPA(mtrls)
                        {
                           MaterialPtr &mtrl=mtrls[i];
                           mtrl=part.multiMaterial(i);
                           if(C IDReplace *id=ReplaceID(mtrl.id())){changed_multi_mtrl=true; mtrl=Proj.gamePath(id.to);}
                        }
                        if(changed_multi_mtrl)
                        {
                           changed=true;
                           part.multiMaterial(mtrls[0], mtrls[1], mtrls[2], mtrls[3]);
                        }
                        REP(part.variations())if(i)
                           if(C IDReplace *id=ReplaceID(part.variation(i).id())){changed=true; part.variation(i, Proj.gamePath(id.to));}
                     }
                     if(changed)
                     {
                        ElmMesh *mesh_data=elm.meshData();
                        mesh_data.newVer();
                        mesh_data.fromMtrl(mesh);
                        mesh_data.file_time.getUTC();
                        Skeleton *body_skel; Proj.getMeshSkels(mesh_data, null, &body_skel);
                        Mesh game; EditToGameMesh(mesh, game, body_skel, Proj.getEnum(mesh_data.draw_group_id), &mesh_data.transform());
                        Save(mesh, Proj.editPath(elm), Proj.game_path);
                        Save(game, Proj.gamePath(elm)); Proj.savedGame(elm);
                     }
                  }
               }break;

               case ELM_OBJ:
               case ELM_OBJ_CLASS:
               {
                  EditObject obj; if(obj.load(Proj.editPath(elm)))
                  {
                     REPA(obj)
                     {
                        EditParam &param=obj[i];
                        if(ParamTypeID(param.type))REP(param.IDs())if(ReplaceID(param.asID(i))) // if any ID needs to be replaced
                        {
                           Memt<UID> ids;
                           FREP(param.IDs()) // list in original order
                           {
                              UID id=param.asID(i); if(C IDReplace *replace=ReplaceID(id))id=replace.to;
                              ids.add(id);
                           }
                           changed=true;
                           param.setAsIDArray(ids);
                           break;
                        }
                     }
                     if(changed)
                     {
                        elm.data.newVer();
                        Save(obj, Proj.editPath(elm));
                        Proj.makeGameVer(elm);
                     }
                  }
               }break;

               case ELM_WORLD:
               {
                  // TODO: replace terrain materials (watch out for heightmap material palette having both replacement and original)
                  // TODO: replace world object params
               }break;
            }
            if(changed)Proj.elmChanged(elm); // this may cause conflicts with 'Gui.update' (if needed then move to 'ShutIDReplace')
            break;
         }
      }
      UpdateProgress.set(i, Proj.elms.elms());
   }
   return false;
}
bool InitIDReplace()
{
   SetKbExclusive();
   Proj.pause();
   UpdateProgress.create(Rect_C(0, -0.05, 1, 0.045));
   UpdateThread  .create(ThreadIDReplace);
   return true;
}
void ShutIDReplace()
{
   UpdateThread  .del();
   UpdateProgress.del();
   Proj.refresh().resume();
   WindowSetNormal();
   WindowFlash();
}
/******************************************************************************/
bool UpdateIDReplace()
{
   if(Kb.bp(KB_ESC)){SetProjectState(); Gui.msgBox(S, "Merging breaked on user request");}
   if(!UpdateThread.active())SetProjectState();

   WindowSetProgress(UpdateProgress());
   Time.wait(1000/30);
     //Gui.update(); this may cause conflicts with 'Proj.elmChanged'
    Server.update(null, true);
   if(Ms.bp(3))WindowToggle();
   return true;
}
/******************************************************************************/
void DrawIDReplace()
{
   D.clear(BackgroundColor());
   D.text(0, 0.05, "Merging Elements");
   GuiPC gpc;
   gpc.visible=gpc.enabled=true; 
   gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
   gpc.offset.zero();
   UpdateProgress.draw(gpc);
   D.clip();
}
/******************************************************************************/
