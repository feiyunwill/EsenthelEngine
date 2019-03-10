/******************************************************************************/
class ProjectHierarchy : Project
{
        ElmNode  root;
   Memc<ElmNode> hierarchy;

   virtual ProjectHierarchy& del()override;

   void floodRemoved(Memc<UID> &removed, ElmNode &node, bool parent_removed=false);

   void floodHierarchy(ElmNode &node);
   void addHierarchy(ElmNode &node, int node_i, ElmNode &target, int target_i);
   void setHierarchy();

   bool contains(C Elm &a, C Elm *b)C; // if 'a' contains 'b'
   int depth(C Elm *elm)C;
   Elm* firstParent(Elm *elm, ELM_TYPE type);
   Elm* firstVisibleParent(Elm *elm);
   Str elmSrcFileFirst(C Elm *elm)C;
   virtual Elm* findElmByPath(C Str &path)override;
           Str elmFullName(C UID &elm_id, int max_elms=-1)C;         
   virtual Str elmFullName(C Elm *elm   , int max_elms=-1)C override;

   virtual void eraseRemoved();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
