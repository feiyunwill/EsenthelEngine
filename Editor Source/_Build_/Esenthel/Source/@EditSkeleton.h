/******************************************************************************/
/******************************************************************************/
class EditSkeleton
{
   class Node // node as imported from FBX, this is set only one time during import
   {
      Str     name; // original node name as imported from FBX (can be an empty string)
      int     parent; // parent node index, -1=none
      OrientP orient_pos; // as imported from FBX

      uint memUsage()C;   
      bool save(File &f)C;
      bool load(File &f);
      bool loadOld(File &f);

public:
   Node();
   };
   class Bone : Mems<IndexWeight> // links 1 'SkelBone' -> multiple 'Node'
   {
      Str8 name; // 'SkelBone' name

      uint memUsage()C;

      void init(C Str &name, int node);

      void addWeight(int index, flt weight);
      int mainNode()C; // get the first node with highest weight

      bool save(File &f)C;
      bool load(File &f); 

      bool loadOld(File &f);
   };

   int        root; // root node index, -1=none
   Mems<Node> nodes;
   Mems<Bone> bones; // links 'SkelBone' -> 'Node'

   uint memUsage()C;
   void del();
   void create(C Skeleton &skel, C MemPtr<Str> &node_names=null);
   void set(Skeleton &skel)C;
   enum MAPPING
   {
      KEEP                   ,
      BONE_NAME_IS_NODE_INDEX,
      BONE_NAME_IS_NODE_NAME ,
   };
   void set(Mems<Mems<IndexWeight> > &weights, C Skeleton &old_skel, C Skeleton &new_skel, MAPPING mapping)C;

   Str nodeUID(int i)C; // unique string identifying a node !! needs to be the same as 'Import.nodeUID' !!

   bool rootZero()C;

   bool   hasNode (C Str &name);               
   Node* findNode (C Str &name);               
 C Node* findNode (C Str &name)C;              
   int   findNodeI(C Str &name, C Str&path=S)C;

   int    findBoneI(C Str &name)C;                   
   Bone*  findBone (C Str &name);                    
 C Bone*  findBone (C Str &name)C;                   
   void removeBone (C Str &name);                    
   void renameBone (C Str &old_name, C Str&new_name);

   int nodeToBone(int node_i)C; // will return only direct mapping (if 1 weight and 1 bone link) only
   int boneToNode(int bone_i)C; // find the first node with highest weight
   int boneToNode(C Str &name)C; // find the first node with highest weight

   Bone* getBone(C Str &name);

   void animate(C AnimSkel &anim_skel, C MemPtrN<Matrix, 256> &matrixes);

   class NodePtr
   {
      bool is; // have to use 'is' instead of doing "name.is() ? " because names are allowed to be empty
      Str  name;

      void set(int node_i, C EditSkeleton &skel);

public:
   NodePtr();
   };
   void add(C EditSkeleton &src_skel, bool replace); // assumes that 'bones' names are unique in both skeletons, but 'nodes' names can overlap

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

private:
   class Bone0
   {
      Str8    original_name, dest_name, // 'original_name'=as imported from FBX (can be an empty string), 'dest_name'=as renamed to a custom bone
       parent_original_name; // 'parent_original_name'=parent name as imported from FBX
      OrientP orient_pos;

      bool load(File &f);
   };

public:
   EditSkeleton();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
