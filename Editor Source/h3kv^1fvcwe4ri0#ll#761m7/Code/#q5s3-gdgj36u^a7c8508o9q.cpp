/******************************************************************************/
class EditSkeleton
{
   class Node // node as imported from FBX, this is set only one time during import
   {
      Str     name; // original node name as imported from FBX (can be an empty string)
      int     parent=-1; // parent node index, -1=none
      OrientP orient_pos; // as imported from FBX

      uint memUsage()C {return name.memUsage();}
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<name<<parent<<orient_pos;
         return f.ok();
      }
      bool load(File &f)
      {
         switch(f.decUIntV())
         {
            case 0:
            {
               f>>name>>parent>>orient_pos;
               if(f.ok())return true;
            }break;
         }
         return false;
      }
      bool loadOld(File &f)
      {
         switch(f.decUIntV())
         {
            case 0:
            {
               GetStr2(f, name); f>>parent>>orient_pos;
               if(f.ok())return true;
            }break;
         }
         return false;
      }
   }
   class Bone : Mems<IndexWeight> // links 1 'SkelBone' -> multiple 'Node'
   {
      Str8 name; // 'SkelBone' name

      uint memUsage()C {return name.memUsage()+super.memUsage();}

      void init(C Str &name, int node) {T.name=name; super.setNum(1)[0].set(node, 1);}

      void addWeight(int index, flt weight)
      {
         if(index>=0)
         {
            REPA(T)if(T[i].index==index){T[i].weight+=weight; return;}
            New().set(index, weight);
         }
      }
      int mainNode()C // get the first node with highest weight
      {
         int node_i=-1; flt weight;
         REPA(T)
         {
          C IndexWeight &iw=T[i];
            if(node_i<0 || iw.weight>weight){node_i=iw.index; weight=iw.weight;}
         }
         return node_i;
      }

      bool save(File &f)C {f<<name; return super.saveRaw(f);}
      bool load(File &f)  {f>>name; return super.loadRaw(f);}

      bool loadOld(File &f) {name=GetStr2(f); return super.loadRaw(f);}
   }

   int        root=-1; // root node index, -1=none
   Mems<Node> nodes;
   Mems<Bone> bones; // links 'SkelBone' -> 'Node'

   uint memUsage()C
   {
      uint mem=0;
      REPA(nodes)mem+=nodes[i].memUsage();
      REPA(bones)mem+=bones[i].memUsage();
      return mem;
   }
   void del()
   {
      root=-1;
      nodes.del();
      bones.del();
   }
   void create(C Skeleton &skel, C MemPtr<Str> &node_names=null)
   {
      root=-1;
      nodes.setNum(skel.bones.elms()); REPA(nodes)
      {
         Node     &node =     nodes[i];
       C SkelBone &bone =skel.bones[i];
         node.name      =(InRange(i, node_names) ? node_names[i] : Str(bone.name)); // remember that 'node_names' are allowed to be empty strings
         node.parent    =skel.boneParent(i);
         node.orient_pos=bone;
      }
      bones.setNum(nodes.elms()); REPAO(bones).init(skel.bones[i].name, i); // have to use 'SkelBone' name
   }
   void set(Skeleton &skel)C
   {
      skel.del().bones.setNum(nodes.elms()); REPA(nodes)
      {
       C Node     &node=     nodes[i];
         SkelBone &bone=skel.bones[i];
         SCAST(OrientP, bone)=node.orient_pos;
         Set(bone.name, TextInt(i));
             bone.parent=(InRange(node.parent, skel.bones) ? node.parent : 0xFF);
      }
      skel.sortBones();
   }
   enum MAPPING
   {
      KEEP                   ,
      BONE_NAME_IS_NODE_INDEX,
      BONE_NAME_IS_NODE_NAME ,
   }
   void set(Mems<Mems<IndexWeight>> &weights, C Skeleton &old_skel, C Skeleton &new_skel, MAPPING mapping)C
   {
      MemtN<int, 256> node_to_bone; // converts node index to 'old_skel' bone index
      if(mapping!=KEEP)
      {
         node_to_bone.setNum(nodes.elms()); switch(mapping)
         {
            case BONE_NAME_IS_NODE_INDEX: REPAO(node_to_bone)=old_skel.findBoneI((Str8)TextInt(i)     ); break;
            case BONE_NAME_IS_NODE_NAME : REPAO(node_to_bone)=old_skel.findBoneI((Str8)  nodes[i].name); break;
         }
      }
      weights.setNum(new_skel.bones.elms()); REPA(weights)
      {
         Mems<IndexWeight> &weight=weights[i];
         if(C Bone *bone=findBone(new_skel.bones[i].name))
         {
            weight=*bone;
            if(mapping!=KEEP)REPA(weight)
            {
               IndexWeight &w=weight[i]; w.index=(InRange(w.index, node_to_bone) ? node_to_bone[w.index] : -1); if(w.index<0)weight.remove(i, true);
            }
         }else weight.clear();
      }
   }

   Str nodeUID(int i)C // unique string identifying a node !! needs to be the same as 'Import.nodeUID' !!
   {
      Str path; Memt<int> parents; for(; InRange(i, nodes) && parents.include(i); )
      {
       C Node &node=nodes[i];
         path+=node.name; // node name
         int parent=node.parent, child_index=0; REPD(j, i)if(nodes[j].parent==parent && nodes[j].name==node.name)child_index++; if(child_index){path+=CharAlpha; path+=child_index;} // node child index in parent (only children with same names are counted)
         path+='/'; // separator
         i=parent;
      }
      return path;
   }

   bool rootZero()C {return InRange(root, nodes) && Equal(nodes[root].orient_pos.pos, VecZero);}

   bool   hasNode (C Str &name)  {return findNodeI(name)>=0;}
   Node* findNode (C Str &name)  {return nodes.addr(findNodeI(name));}
 C Node* findNode (C Str &name)C {return ConstCast(T).findNode(name);}
   int   findNodeI(C Str &name, C Str &path=S)C
   {
      int index=-1, difference=INT_MAX;
      REPA(nodes)if(nodes[i].name==name) // always check even if 'name' is empty (because original full name can be empty)
      {
         int d=Difference(path, nodeUID(i));
         if(!d)return i; // exact match
         if( d<difference){difference=d; index=i;}
      }
      return index;
   }

   int    findBoneI(C Str &name)C {REPA(bones)if(bones[i].name==name)return i; return -1;}
   Bone*  findBone (C Str &name)  {return bones.addr(findBoneI(name));}
 C Bone*  findBone (C Str &name)C {return ConstCast(T).findBone(name);}
   void removeBone (C Str &name)  {bones.remove(findBoneI(name), true);}
   void renameBone (C Str &old_name, C Str &new_name) {if(Bone *bone=findBone(old_name))bone.name=new_name;}

   int nodeToBone(int node_i)C // will return only direct mapping (if 1 weight and 1 bone link) only
   {
      int bone_i=-1;
      if( node_i>=0)FREPA(bones) // go from the start to find the first parent linked with this node
      {
       C Bone &bone=bones[i];
         FREPAD(w, bone)if(bone[w].index==node_i) // if bone is linked to this node
         {
            if(bone_i>=0 || bone.elms()!=1)return -1; // we need direct mapping
            bone_i=i;
         }
      }
      return bone_i;
   }
   int boneToNode(int bone_i)C // find the first node with highest weight
   {
      return InRange(bone_i, bones) ? bones[bone_i].mainNode() : -1;
   }
   int boneToNode(C Str &name)C // find the first node with highest weight
   {
      if(C Bone *bone=findBone(name))return bone.mainNode();
      return -1;
   }

   Bone* getBone(C Str &name)
   {
      if(name.is())
      {
         if(Bone *bone=findBone(name))return bone;
            Bone &bone=bones.New(); bone.name=name; return &bone;
      }
      return null;
   }

   void animate(C AnimSkel &anim_skel, C MemPtrN<Matrix, 256> &matrixes)
   {
      REPAD(n, nodes)
      {
         OrientP &orient_pos=nodes[n].orient_pos, temp; temp.zero(); bool processed=false;
         REPAD(b, bones) // have to iterate all bones to check which are linked to this node
         {
          C Bone &bone=bones[b]; REPA(bone)if(bone[i].index==n)
            {
               int        anim_bone=anim_skel.findBoneI(bone.name)+1; // +root
               if(InRange(anim_bone, matrixes))
               {
                  processed=true;
                  temp+=(orient_pos*matrixes[anim_bone])*bone[i].weight;
               }
            }
         }
         if(processed){temp.fix(); orient_pos=temp;}
      }
   }

   class NodePtr
   {
      bool is=false; // have to use 'is' instead of doing "name.is() ? " because names are allowed to be empty
      Str  name;

      void set(int node_i, C EditSkeleton &skel) {if(is=InRange(node_i, skel.nodes))name=skel.nodes[node_i].name;}
   }
   void add(C EditSkeleton &src_skel, bool replace) // assumes that 'bones' names are unique in both skeletons, but 'nodes' names can overlap
   {
      if(this==&src_skel)return;

      // nodes
      Memc<NodePtr> parents; parents.setNum(nodes.elms()); REPAO(parents).set(nodes[i].parent, T); // first set parents of existing nodes
      FREPA(src_skel.nodes) // add 'src_skel' nodes in order
      {
       C Node &src_node  =src_skel.nodes[i];
         int       node_i=findNodeI(src_node.name); // check if that node already exists in this skeleton
         if(node_i<0){nodes.add    (src_node); parents.New()  .set(src_node.parent, src_skel);}else // if not found
         if(replace ){nodes[node_i]=src_node ; parents[node_i].set(src_node.parent, src_skel);}     // found and replace
      }
      // add rest after all nodes have been copied

      // parents
      REPA(parents)nodes[i].parent=(parents[i].is ? findNodeI(parents[i].name) : -1);

      // root
      if(InRange(src_skel.root, src_skel.nodes) && (replace || root<0))root=findNodeI(src_skel.nodes[src_skel.root].name);

      // bones
      int offset=bones.addNum(src_skel.bones.elms()); REPA(src_skel.bones)
      {
         Bone &dest=bones[i+offset]; dest=src_skel.bones[i]; REPA(dest)
         {
            IndexWeight &weight=dest[i]; if(InRange(weight.index, src_skel.nodes))
            {
                  weight.index=findNodeI(src_skel.nodes[weight.index].name); // find src node in this skeleton
               if(weight.index>=0)continue; // if found this node then continue
            }
            dest.remove(i, true); // for some reason didn't find a node, so remove this weight
         }
         if(!dest.elms())bones.remove(i+offset, true); // if no weights were found, then remove this bone
      }
   }

   // io
   bool save(File &f)C
   {
      f.cmpUIntV(2);
      f<<root; nodes.save(f); bones.save(f);
      return f.ok();
   }
   bool load(File &f)
   {
      switch(f.decUIntV())
      {
         case 2: f>>root; if(nodes.load(f) && bones.load(f) && f.ok())return true; break;

         case 1:
         {
            f>>root;
            nodes.setNum(f.decUIntV()); FREPA(nodes)if(!nodes[i].loadOld(f))goto error;
            bones.setNum(f.decUIntV()); FREPA(bones)if(!bones[i].loadOld(f))goto error;
            if(f.ok())return true;
         }break;

         case 0:
         {
            Mems<Bone0> bones0; if(bones0.load(f) && f.ok())
            {
               del();
               nodes.setNum(bones0.elms()); REPA(nodes)
               {
                  nodes[i].name      =bones0[i].original_name;
                  nodes[i].orient_pos=bones0[i].orient_pos;
               }
               REPAO(nodes).parent=findNodeI(bones0[i].parent_original_name); // get parent after all nodes were set
               REPA (bones0)if(bones0[i].dest_name.is())bones.New().init(bones0[i].dest_name, i);
               return true;
            }
         }break;
      }
   error:
      del(); return false;
   }
   bool load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      del(); return false;
   }

private:
   class Bone0
   {
      Str8    original_name, dest_name, // 'original_name'=as imported from FBX (can be an empty string), 'dest_name'=as renamed to a custom bone
       parent_original_name; // 'parent_original_name'=parent name as imported from FBX
      OrientP orient_pos;

      bool load(File &f)
      {
         if(f.decUIntV()==0)
         {
            original_name=GetStr2(f); dest_name=GetStr2(f); parent_original_name=GetStr2(f); f>>orient_pos;
            if(f.ok())return true;
         }
         return false;
      }
   }
}
/******************************************************************************/
