/******************************************************************************/
class MeshVariations : Window
{
   class RenameVariation : ClosableWindow
   {
      int      index=-1; // number of elements before this one with similar name
      Str      name;
      TextLine textline;

      static void Hide(RenameVariation &rv) {rv.hide(); ObjEdit.mesh_variations.activate();}

      void create()
      {
         Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Rename Variation").hide(); button[2].func(Hide, T).show();
         T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
      }
      void activate(int variation)
      {
         Mesh &mesh=ObjEdit.mesh;
         if(!variation)Gui.msgBox(S, "Variation #0 can't be renamed, it's name is always NULL.");else
         if(InRange(variation, mesh.variations()))
         {
            name =mesh.variationName(variation);
            index=0; FREP(variation)if(name==mesh.variationName(i))index++;
            textline.set(name).selectAll().activate();
            super.activate();
         }
      }
      virtual void update(C GuiPC &gpc)override
      {
         super.update(gpc);

         if(Gui.window()==this)
         {
            if(Kb.k(KB_ENTER)){Kb.eatKey(); ObjEdit.mesh_variations.renameVariation(index, name, textline()); button[2].push();}
         }
      }
   }
   class Variation
   {
      bool   removed=false;
      Color  color=BLACK;
      int    index;
      Button remove;

      void setColor() {color=GuiListTextColor(); if(removed)color.a/=2;}
   }
   static Str Removed(C Variation &p) {return S;}
   static Str Name   (C Variation &v) {return v.index ? ObjEdit.mesh.variationName(v.index) : "Default";}

   static void Remove(Variation &v)
   {
      ObjEdit.mesh_undos.set("varRemove");
      ObjEdit.mesh_variations.erasedVariation(v.index);
      ObjEdit.mesh.variationRemove(v.index);
      ObjEdit.setChangedMesh(true, false);
      ObjEdit.selVariation(ObjEdit.selVariation());
      Proj.meshVariationChanged();
   }
   static void Remove()
   {
      ObjEdit.mesh_undos.set("varRemove");
      REPA(ObjEdit.mesh_variations.list.sel)
      {
         int v=ObjEdit.mesh_variations.list.sel[i];
         ObjEdit.mesh_variations.erasedVariation(v);
         ObjEdit.mesh.variationRemove(v);
      }
      ObjEdit.setChangedMesh(true, false);
      ObjEdit.selVariation(ObjEdit.selVariation());
      Proj.meshVariationChanged();
   }
   static void Rename()
   {
      if(ObjEdit.mesh_variations.list.sel.elms())ObjEdit.mesh_variations.rename.activate(ObjEdit.mesh_variations.list.sel[0]);
   }
   void renameVariation(int index, C Str &old_name, C Str &new_name)
   {
      if(!Equal(old_name, new_name, true))
      {
         ObjEdit.mesh_undos.set("varRename");
         Mesh &mesh=ObjEdit.mesh;
         FREP(mesh.variations())if(old_name==mesh.variationName(i))if(!index--)
         {
            mesh.variationName(i, new_name);
            ObjEdit.setChangedMesh(true, false);
            Proj.meshVariationChanged();
            break;
         }
      }
   }
   static void ClearName()
   {
      ObjEdit.mesh_undos.set("varRename");
      REPA(ObjEdit.mesh_variations.list.sel)
      {
         int v=ObjEdit.mesh_variations.list.sel[i];
         ObjEdit.mesh.variationName(v, S8);
      }
      ObjEdit.setChangedMesh(true, false);
      Proj.meshVariationChanged();
   }
   void newVariation(C Str *name=null)
   {
      if(ObjEdit.getMeshElm())
      {
         int i=ObjEdit.mesh.variations(); // index of new element
         ObjEdit.mesh.variations(ObjEdit.mesh.variations()+1); // create new
         refresh(); // refresh, make sure list has the element so we can select it
         ObjEdit.selVariation(i); // select
         ObjEdit.mesh.variationName(i, name ? *name : S+"Variation"); // set name
         if(!name)rename.activate(i); // rename
      }
   }
   static void Add(MeshVariations &mv)
   {
      ObjEdit.mesh_undos.set("varNew");
      mv.newVariation();
      ObjEdit.setChangedMesh(true, false);
      Proj.meshVariationChanged();
   }
   static void ListChanged(MeshVariations &mv)
   {
      ObjEdit.selVariation(mv.list.visToAbs(mv.list.cur), QUIET);
   }

   class ListVariation : List<Variation>
   {
      int getPos(C Vec2 &pos) {int p=screenToVisY(pos.y-elmHeight()/2); return (p>=0) ? p : visibleElms();}

      virtual void draw(C GuiPC &gpc)override
      {
         if(visible() && gpc.visible)
         {
            GuiSkin skin; 
            int elm=-1; if(Gui.dragging() && Gui.objAtPos(Gui.dragPos())==this)elm=getPos(Gui.dragPos());
            if( elm>=0)
            {
               if(Gui.skin)skin=*Gui.skin; skin.list.highlight_color.zero(); T.skin(&skin);
            }
            super.draw(gpc);
            if(elm>=0)
            {
               T.skin(null);
               D.clip(gpc.clip);
               flt y=visToScreenPos(elm, &gpc).y, b=0.005;
               Rect(-D.w(), y-b, D.w(), y+b).draw(ColorAlpha(TURQ, 0.5));
            }
         }
      }
   }

   Memx<Variation> data;
   ListVariation   list;
   Button          add;
   Region          region;
   Menu            menu;
   RenameVariation rename;

   virtual Rect sizeLimit()C override
   {
      Rect r;
      r.min=0.2;
      r.max.set(D.w(), D.h()*1.5);
      return r;
   }
                   C Rect& rect() {return super.rect();}
   virtual MeshVariations& rect(C Rect &rect)override
   {
      super.rect(rect);
      flt h=0.05;
      add   .rect(Rect_LU(0.01, -0.01, h*5, h));
      region.rect(Rect(0, -clientHeight(), clientWidth(), add.rect().min.y).extend(-0.01));
      return T;
   }

   void newMesh() {rename.hide();}
   void showMenu(C Vec2 &pos)
   {
      if(list.sel.elms())
      {
         Node<MenuElm> node;
         if(list.sel.elms()==1)node.New().create("Rename"    , Rename   ).desc("Rename variation");
                               node.New().create("Remove"    , Remove   ).desc("Remove selected variations");
                               node.New().create("Clear Name", ClearName).desc("Clear name to empty for selected variations");
         Gui+=menu.create(node).posRU(pos); menu.activate();
      }
   }

   MeshVariations& create()
   {
      flt height=0.037;
      ListColumn lc[]=
      {
         ListColumn(Removed                 , height  , S     ), // 0 remove/restore
         ListColumn(MEMBER(Variation, index), LCW_DATA, "#"   ), // 1
         ListColumn(Name                    , LCW_DATA, "Name"), // 2
      };
      super.create("Variations"); flag|=WIN_RESIZABLE;
      T+=region.create().slidebarSize(0.04);
      T+=add.create("New Variation").func(Add, T).focusable(false).desc("Create a new variation");
      region+=list.create(lc, Elms(lc)).elmHeight(height).textSize(0, 1).columnHeight(0.05).selChanged(ListChanged, T); list.flag|=LIST_MULTI_SEL; FlagDisable(list.flag, LIST_SORTABLE); // disable sorting so we can re-arrange variation order by drag and drop
      list.cur_mode=LCM_ALWAYS; list.setElmTextColor(MEMBER(Variation, color)).desc("Double click to rename variation\nDrag and drop to change order");
      rect(Rect_LU(0, -0.6, 0.4, 0.5));
      rename.create();
      return T;
   }
   static void DragVariations(MeshVariations &mv, GuiObj *obj, C Vec2 &screen_pos) {mv.dragVariations(obj, screen_pos);}
          void dragVariations(                    GuiObj *obj, C Vec2 &screen_pos)
   {
      if(obj==&list)
      {
         list.sel.sort(Compare); // sort to make sure we're processing from first to last
         int target=list.visToAbs(list.getPos(screen_pos));
         if(list.sel.elms() && target>list.sel.first())target--;
         if(target<0)target=list.elms()-1;

         // if before 'target' there are other elements selected, then adjust 'target' to the first selected element (which is connected to target)
         int ti=list.sel.find(target); if(ti>=0)for(; InRange(ti-1, list.sel); )
         {
            int prev_target=list.sel[ti-1];
            if(list.absToVis(prev_target)==list.absToVis(target)-1){ti--; target=prev_target;}else break;
         }

         // remember cursor/selection
         uint       list_cur=ObjEdit.mesh.variationID(list.visToAbs(list.cur));
         Memt<uint> var_id; FREPA(list.sel)var_id.add(ObjEdit.mesh.variationID(list.sel[i]));

         // move variations
         if(var_id.elms())
         {
            ObjEdit.mesh_undos.set("varMove", true);
            FREPA(var_id)
            {
               int src=ObjEdit.mesh.variationFind(var_id[i]); ObjEdit.mesh.variationMove(src, target);
               if( src>=target)target++;
            }
            ObjEdit.setChangedMesh(true, false);
            ObjEdit.selVariation(ObjEdit.mesh.variationFind(list_cur));
            list.sel.clear(); FREPA(var_id){int v=ObjEdit.mesh.variationFind(var_id[i]); if(v>=0)list.sel.include(v);}
            Proj.meshVariationChanged();
         }
      }
   }
   void erasedVariation(int variation)
   {
      REPA(list.sel){int &v=list.sel[i]; if(v==variation)list.sel.remove(i, true);else if(v>=variation)v--;}
      ObjEdit.selVariation(ObjEdit.selVariation(), QUIET);
   }
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj)) // create variations from materials
      {
         Memt<UID> mtrl_ids; FREPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_MTRL))mtrl_ids.include(elms[i]);
         if(mtrl_ids.elms())
         {
            ObjEdit.mesh_undos.set("varNew");
            Mesh &mesh=ObjEdit.mesh;
            int  start=Mid(list.getPos(screen_pos), 0, mesh.variations()), added=0;
            FREPA(mtrl_ids)if(Elm *elm=Proj.findElm(mtrl_ids[i]))
            {
               newVariation(&elm.name); // create a new variation using material name
               REP(mesh.lods()) // set material on all lods and parts
               {
                  MeshLod &lod=mesh.lod(i);  REPA(lod)
                  {
                     MeshPart &part=lod.parts[i];
                     part.variations(Max(part.variations(), mesh.variations())) // make room first for specified variation
                         .variation (                       mesh.variations()-1, Proj.gamePath(elm.id));
                  }
               }
               mesh.variationMove(mesh.variations()-1, start+added); added++; // move last added to start
            }
            if(added)
            {
               ObjEdit.setChangedMesh(true, false);
               Proj.meshVariationChanged();
               Proj.refresh(false, false); // refresh in case the mesh had invalid refs and now it won't
               ObjEdit.selVariation(start); // select first added variation
               list.sel.clear(); FREP(added)list.sel.add(start+i); // select all added
            }
         }
      }
   }
   void refresh()
   {
      if(ObjEdit.obj_elm) // don't refresh when there's no object at all, so we don't lose list selection when unloading
      {
         data.setNum(ObjEdit.mesh.variations());
         Memt<bool> visible;
         FREPA(data)
         {
            Variation &v=data[i];
            v.index=i;
            v.remove.create(Rect_LU(0, list.elmHeight())).func(Remove, v).focusable(false);
            if(v.removed=false)v.remove.setText ("R"            ).desc("Restore this variation");
            else               v.remove.setImage("Gui/close.img").desc( "Remove this variation");
            v.setColor();
            visible.add(!v.removed);
            list.addChild(v.remove, i, 0);
         }
         list.setData(data, visible, true);
      }
   }
   virtual void update(C GuiPC &gpc)override
   {
      if(gpc.visible && visible())
      {
         super.update(gpc);
         if(ObjEdit.mesh.variations()!=data.elms())refresh();
         if(Ms.bp(1) && Gui.ms()==&list)
         {
            if(!list.sel.has(list.visToAbs(list.lit)) || list.sel.elms()<2)
            {
               list.cur=list.lit;
               list.processSel(list.visToAbs(list.cur), LSM_SET);
            }
            showMenu(Ms.pos());
         }
         if(Gui.ms()==&list && Ms.bp(0))
         {
            if(Ms.bd(0))rename.activate(list.visToAbs(list.cur));
            if(Variation *v=list())if(list.selMode()==LSM_SET)Gui.drag(DragVariations, T, null);
         }
      }
   }
}
/******************************************************************************/
