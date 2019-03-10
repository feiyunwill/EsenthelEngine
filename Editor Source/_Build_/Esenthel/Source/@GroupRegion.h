/******************************************************************************/
/******************************************************************************/
class GroupRegion : Region
{
   class Index
   {
      int i;

public:
   Index();
   };
   Memc<Index>  indexes;
   Memx<Button> set_groups;
   flt          set_groups_zoom;
   Text         groups_t, parts_t;
   Region       groups_r, parts_r;
   List<Index>  groups_l, parts_l;
   TextWhite    ts;

   static int         VisibleParts(C MeshLod &lod       );
   static int         VisiblePartI(C MeshLod &lod, int i); // vis to abs
   static int             AbsPartI(C MeshLod &lod, int i); // abs to vis
   static C MeshPart* VisiblePart (C MeshLod &lod, int i);

   static Str IndexToGroupName(C Index &index);
   static Str IndexToPartName(C Index &index);
   static Str IndexToPartGroup(C Index &index);

   static void DragGroup(ptr index, GuiObj *go, C Vec2 &screen_pos);
   static void SetGroup(Button &button);

   int getSetGroup();
   void highlight(int part);
   GroupRegion& create();
   virtual void update(C GuiPC &gpc)override;

public:
   GroupRegion();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
