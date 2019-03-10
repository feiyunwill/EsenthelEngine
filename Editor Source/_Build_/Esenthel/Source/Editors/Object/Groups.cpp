/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   int         GroupRegion::VisibleParts(C MeshLod &lod       ) {int n=0;   REPA(   lod)if(!(lod.parts[i].part_flag&MSHP_HIDDEN))n++; return n;}
   int         GroupRegion::VisiblePartI(C MeshLod &lod, int i) {         FREPAD(p, lod)if(!(lod.parts[p].part_flag&MSHP_HIDDEN))if(!i--)return p; return -1;}
   int             GroupRegion::AbsPartI(C MeshLod &lod, int i) {int v=0; FREPAD(p, lod)if(!(lod.parts[p].part_flag&MSHP_HIDDEN)){if(p==i)return v; v++;} return -1;}
   C MeshPart* GroupRegion::VisiblePart(C MeshLod &lod, int i) {i=VisiblePartI(lod, i); return lod.parts.addr(i);}
   Str GroupRegion::IndexToGroupName(C Index &index)
   {
      if(Enum *e=ObjEdit.mesh.drawGroupEnum())return e->elmName(index.i);
      return S;
   }
   Str GroupRegion::IndexToPartName(C Index &index)
   {
      if(C MeshPart *part=VisiblePart(ObjEdit.getLod(), index.i))return part->name;
      return S;
   }
   Str GroupRegion::IndexToPartGroup(C Index &index)
   {
      if(C MeshPart *part=VisiblePart(ObjEdit.getLod(), index.i))
      {
         int group=part->drawGroup();
         if(Enum *e=ObjEdit.mesh.drawGroupEnum())if(InRange(group, *e))return (*e)[group].name;
         return group;
      }
      return S;
   }
   void GroupRegion::DragGroup(ptr index, GuiObj *go, C Vec2 &screen_pos)
   {
      int group=intptr(index); if(group>=-1 && group<32)
      {
         MeshLod &lod=ObjEdit.getLod();
         if(InRange(ObjEdit.lit_part, lod))
         {
            ObjEdit.mesh_undos.set("drawGroup");
            SetDrawGroup(ObjEdit.mesh, lod, ObjEdit.lit_part, group, ObjEdit.mesh.drawGroupEnum());
            ObjEdit.setChangedMesh(true, false);
         }
      }
   }
   void GroupRegion::SetGroup(Button &button)
   {
      SetObjOp(button() ? OP_OBJ_SET_GROUP : OP_OBJ_NONE);
      REPA(ObjEdit.group.set_groups)if(&ObjEdit.group.set_groups[i]!=&button)ObjEdit.group.set_groups[i].set(false, QUIET);
   }
   int GroupRegion::getSetGroup()
   {
      FREPA(set_groups)if(set_groups[i]())return i;
      return -1;
   }
   void GroupRegion::highlight(int part)
   {
      parts_l.lit=AbsPartI(ObjEdit.getLod(), part);
   }
   GroupRegion& GroupRegion::create()
   {
      ::EE::Region::create(Rect_LU(0, 0, 1.2f, 0.8f)).skin(&TransparentSkin, false); kb_lit=false;
      ts.reset().size=0.043f; ts.align.set(0, 1);
      T+=groups_r.create(Rect(0.01f, -rect().h()+0.01f, 0.48f, -0.06f));
      T+= parts_r.create(Rect(groups_r.rect().max.x+0.01f, -rect().h()+0.01f, rect().w()-0.01f, -0.06f));
      T+=groups_t.create(groups_r.rect().up()+Vec2(0, 0.005f), "Enum Draw Groups" , &ts);
      T+= parts_t.create( parts_r.rect().up()+Vec2(0, 0.005f), "Object Mesh Parts", &ts);

      flt iw=0.05f*0, w=groups_r.rect().w()-groups_r.slidebarSize()-iw;
      ListColumn groups_lc[]=
      {
       //ListColumn(MEMBER(Index, i), iw, null  ),
         ListColumn(IndexToGroupName,  w, "Name"),
      };
      groups_r+=groups_l.create(groups_lc, Elms(groups_lc)).elmHeight(0.037f).textSize(0, 1).columnHeight(0.05f); groups_l.cur_mode=LCM_MOUSE; FlagDisable(groups_l.flag, LIST_SORTABLE); groups_l.flag|=LIST_SCALABLE; groups_l.desc("Drag and drop groups onto mesh parts to assign them");

      iw=0.05f*0, w=parts_r.rect().w()-parts_r.slidebarSize()-iw;
      ListColumn parts_lc[]=
      {
       //ListColumn(MEMBER(Index, i), iw    , null   ),
         ListColumn(IndexToPartName , w*0.35f, "Name" ),
         ListColumn(IndexToPartGroup, w*0.65f, "Group"),
      };
      parts_r+=parts_l.create(parts_lc, Elms(parts_lc)).elmHeight(0.037f).textSize(0, 1).columnHeight(0.05f); parts_l.cur_mode=LCM_MOUSE; FlagDisable(parts_l.flag, LIST_SORTABLE); parts_l.flag|=LIST_RESIZABLE_COLUMNS|LIST_SCALABLE;

      return T;
   }
   void GroupRegion::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      if(gpc.visible && visible())
      {
         ObjEdit.resetDrawGroupEnum(); // dynamically adjust draw group in each frame in case the enum got changed
         Enum    *group_enum=ObjEdit.mesh.drawGroupEnum();
         MeshLod &lod       =ObjEdit.getLod();
         int      parts     =VisibleParts(lod),
                  groups    =(group_enum ? group_enum->elms().elms() : 0),
                  max       =Max(parts, groups);
         bool     reset     =false;
         if(max   >  indexes.elms()         ){indexes.setNum(max); REPAO(indexes).i=i; reset=true;}
         if(parts != parts_l.elms() || reset) parts_l.setData(indexes.data(),  parts); // reset because of potential mem address change
         if(groups!=groups_l.elms() || reset)groups_l.setData(indexes.data(), groups); // reset because of potential mem address change
         if(groups!=set_groups.elms() || !Equal(set_groups_zoom, groups_l.zoom()))
         {
            set_groups.setNum(groups);
            set_groups_zoom=groups_l.zoom();
            flt x=groups_l.columnWidth(0), y=-groups_l.columnHeight(), s=groups_l.elmHeight()*groups_l.zoom();
            FREPA(set_groups)if(!set_groups[i].is())
            {
               Button &set_group=set_groups[i];
               groups_r+=set_group.create().func(SetGroup, set_group).focusable(false).desc("Click this button to enable assigning this group to object parts.\nWhen this button is pushed you can click on object parts and this group will get assigned to them.");
               set_group.mode=BUTTON_TOGGLE;
               set_group.image=Proj.arrow_right;
            }
            REPAO(set_groups).rect(Rect_RU(x, y-i*s, s, s));
         }
         if(Ms.bp(0))
         {
            if(Gui.ms()==&groups_l)Gui.drag(DragGroup, ptr(groups_l.cur));else // start list group elm drag
            if(Gui.ms()==& parts_l) // if click on part list
            {
               int part=VisiblePartI(lod, parts_l.lit);
               if(OpObj==OP_OBJ_SET_GROUP && getSetGroup()>=0 && InRange(part, lod)) // if group assigning is enabled
               {
                  ObjEdit.mesh_undos.set("drawGroup");
                  SetDrawGroup(ObjEdit.mesh, lod, part, getSetGroup(), group_enum); // assign group
                  ObjEdit.setChangedMesh(true, false);
               }
            }
         }
      }
   }
GroupRegion::GroupRegion() : set_groups_zoom(0) {}

GroupRegion::Index::Index() : i(0) {}

/******************************************************************************/
