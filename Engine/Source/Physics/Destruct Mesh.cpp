/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_DSTR CC4('D','S','T','R')
/******************************************************************************/
Cache<DestructMesh> DestructMeshes("Destruct Mesh");
/******************************************************************************/
// MANAGE
/******************************************************************************/
Bool DestructMesh::Part::create(Mesh &src, Int vtx_limit)
{
   // optimize
   mesh.create(src).triToQuad().weldCoplanarFaces(EPS_COL_COS, -1, false).setAutoTanBin().setRender();

   // phys
   if(!vtx_limit)phys.del();else
   {
      MeshBase temp; temp.createPhys(mesh); if(vtx_limit>0)temp.createConvex(temp.vtx.pos(), temp.vtxs(), vtx_limit);
      phys.createConvexTry(temp); if(!phys.is())return false;
   }

   mesh.delBase();

   return true;
}
/******************************************************************************/
static void SelectNeighbors(Index &vtx_vtx, Memc<Bool> &vtxs, Int vtx)
{
   if(InRange(vtx, vtxs) && !vtxs[vtx]) // if not yet added
   {
      vtxs[vtx]=true; // add vertex to selection
      IndexGroup &ig=vtx_vtx.group[vtx]; REPA(ig)SelectNeighbors(vtx_vtx, vtxs, ig[i]); // process all neighbors
   }
}
DestructMesh& DestructMesh::del()
{
  _parts .del();
  _joints.del();
   return T;
}
DestructMesh& DestructMesh::create(Mesh &mesh, Int cuts, C MaterialPtr &material, Flt tex_scale, Int phys_vtx_limit, Bool cut_affects_biggest_part, Bool simple_cuts)
{
   del();

   UInt       flag_and=(VTX_POS|VTX_NRM_TAN_BIN|VTX_TEX_ALL|VTX_COLOR|VTX_MATERIAL|VTX_SIZE);
   Bool       cur=false;
   Memb<Mesh> temp[2]; // use Memb so 'New' will not change memory address of previous elements
   FREP(cuts) // order is important
   {  
      Plane clip_plane(Random(mesh.ext), Random(Ball(1), false));
      if(!i) // first step
      {
         if(simple_cuts)SplitMesh     (mesh, null, temp[cur].New(), temp[cur].New(), clip_plane,                      flag_and);
         else           SplitMeshSolid(mesh, null, temp[cur].New(), temp[cur].New(), clip_plane, material, tex_scale, flag_and);
      }else
      {
         const bool size_by_surface=true;
         if(cut_affects_biggest_part)
         {
            Int biggest=-1;
            Flt size;
            Vec center;
            REPA(temp[cur])
            {
             C Mesh &mesh=temp[cur][i];
               Vec c;
               Flt s=(size_by_surface ? mesh.area(&c) : mesh.ext.volume());
               if(biggest<0 || s>size){biggest=i; size=s; center=c;}
            }
            if(biggest>=0)
            {
               Mesh &mesh=temp[cur][biggest];
               clip_plane.pos=(size_by_surface ? center : Random(mesh.ext));
               if(simple_cuts)SplitMesh     (mesh, null, mesh, temp[cur].New(), clip_plane,                      flag_and);
               else           SplitMeshSolid(mesh, null, mesh, temp[cur].New(), clip_plane, material, tex_scale, flag_and);
            }
         }else
         {
            REPA(temp[cur]) // order is important
            {
               if(simple_cuts)SplitMesh     (temp[cur][i], null, temp[!cur].New(), temp[!cur].New(), clip_plane,                      flag_and);
               else           SplitMeshSolid(temp[cur][i], null, temp[!cur].New(), temp[!cur].New(), clip_plane, material, tex_scale, flag_and);
            }
            temp[cur].del(); cur^=1;
         }
      }
      REPA(temp[cur])if(!temp[cur][i].is())temp[cur].remove(i); // remove empty meshes
   }

   // separate mesh parts which aren't physically connected
   FREPA(temp[cur]) // order important
   {
      Mesh &src=temp[cur][i];
      if(   src.lods()==1)
      {
         MeshBase   joined ; joined.create(src, ~0, true).setVtxDup(); // create 1 part from all parts (and keep id as index of the part)
         Index      vtx_vtx; joined.linkVtxVtxOnFace(vtx_vtx);         // link all vertexes together
         Memc<Bool> vtxs   ; vtxs  .setNumZero(joined.vtxs());         // array of selected vertexes
         SelectNeighbors(vtx_vtx, vtxs, 0);                            // select 0-th vertex and all its neighbors

         // set connected triangles and quads
         Memt<Bool> tris, quads; tris.setNum(joined.tris()); quads.setNum(joined.quads());
         Int        tri_count=0, quad_count=0;
         FREPA(joined.tri ){VecI  ind=joined.tri .ind(i); ind.remap(joined.vtx.dup()); if(tris [i]=(vtxs[ind.x] || vtxs[ind.y] || vtxs[ind.z]               )) tri_count++;}
         FREPA(joined.quad){VecI4 ind=joined.quad.ind(i); ind.remap(joined.vtx.dup()); if(quads[i]=(vtxs[ind.x] || vtxs[ind.y] || vtxs[ind.z] || vtxs[ind.w]))quad_count++;}

         if(tri_count!=joined.tris() || quad_count!=joined.quads()) // if connected faces don't cover the full mesh, then we should split it
         {
            MeshBase connected; joined.splitFaces(connected, null, tris, quads); // move the connected faces into 'connected'

            // convert back to meshes
            Mesh connected_mesh, unknown_mesh;
            connected.copyId(connected_mesh); connected_mesh.MeshLod::copyParams(src); FREP(Min(src.parts.elms(), connected_mesh.parts.elms()))connected_mesh.parts[i].keepOnly(src.parts[i].base.flag()).copyParams(src.parts[i]);
            joined   .copyId(  unknown_mesh);   unknown_mesh.MeshLod::copyParams(src); FREP(Min(src.parts.elms(),   unknown_mesh.parts.elms()))  unknown_mesh.parts[i].keepOnly(src.parts[i].base.flag()).copyParams(src.parts[i]);

            // remove empty parts
            REPA(connected_mesh)if(!connected_mesh.parts[i].is())connected_mesh.remove(i, false);
            REPA(  unknown_mesh)if(!  unknown_mesh.parts[i].is())  unknown_mesh.remove(i, false);

            // put them into the processing container
            Swap(src            , connected_mesh); // connected is processed so we can set it back
            Swap(temp[cur].New(),   unknown_mesh); // unknown needs to be added to the array for checking
         }
      }
   }

   // put meshes into the right container
   FREPA(temp[cur])if(temp[cur][i].is())if(!_parts.New().create(temp[cur][i], phys_vtx_limit))_parts.removeLast();

   // create joints
   Memc<MeshBase> phys; FREPA(_parts)phys.New().create(_parts[i].phys).quadToTri().setFaceNormals(); // create mesh from phys convex body
   REPAD(a, _parts) // iterate through all pairs of parts
   {
      REPD(b, a)
      {
         if(Cuts(Extent(_parts[a].mesh.ext).extend(0.1f), _parts[b].mesh.ext))
         {
            MeshBase &phys_a=phys[a],
                     &phys_b=phys[b];
            REPA(phys_a.tri)
            {
               VecI ind=phys_a.tri.ind(i);
               Tri  tri_a(phys_a.vtx.pos(ind.x), phys_a.vtx.pos(ind.y), phys_a.vtx.pos(ind.z), &phys_a.tri.nrm(i));
               Vec  tri_a_center=tri_a.center();
               REPA(phys_b.tri)
               {
                  Vec &tri_nrm_b=phys_b.tri.nrm(i);
                  if(Abs(Dot(tri_a.n, tri_nrm_b))>=0.97f) // parallel
                  {
                     VecI ind=phys_b.tri.ind(i);
                     if(Abs(DistPointPlane(phys_b.vtx.pos(ind.x), tri_a))<=0.03f) // on the same plane
                     {
                        Tri tri_b(phys_b.vtx.pos(ind.x), phys_b.vtx.pos(ind.y), phys_b.vtx.pos(ind.z), &phys_b.tri.nrm(i));
                        Vec tri_b_center =tri_b.center();
                        Tri tri_a_smaller=(tri_a-tri_a_center)*0.9f+tri_a_center,
                            tri_b_smaller=(tri_b-tri_b_center)*0.9f+tri_b_center;
                        if(Dist(tri_a_smaller, tri_b_smaller)<=0.01f) // distance between triangles
                        {
                          _joints.New().set(a, b);
                           goto next_part;
                        }
                     }
                  }
               }
            }
         }
      next_part:;
      }
   }
   return T;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
Bool          DestructMesh::adjustStorage (Bool universal, Bool physx, Bool bullet, Bool *changed) {if(changed)*changed=false; Bool ok=true, c; REPA (_parts){ok&=_parts[i].phys.adjustStorage (universal, physx, bullet, &c); if(changed)*changed|=c;} return ok;}
DestructMesh& DestructMesh::freeHelperData(                                                      ) {                                            REPAO(_parts)              .phys.freeHelperData(                            );                          return T ;}
void          DestructMesh::setShader     (                                                      ) {                                            REPAO(_parts)              .mesh.setShader     (                            );                                    }
/******************************************************************************/
// DRAW
/******************************************************************************/
void DestructMesh::drawMesh(Int highlight_part)C
{
   Flt scale=(Sin(Time.appTime()*1)*0.5f+0.5f)*1;
   REPA(_parts)
   {
      Matrix m; m.setPos(_parts[i].mesh.ext.pos*scale);

      if(i==highlight_part)SetHighlight(RED);else REPAD(j, _joints)if((_joints[j].a==i || _joints[j].b==i) && (_joints[j].a==highlight_part || _joints[j].b==highlight_part))SetHighlight(BLUE);
     _parts[i].mesh.draw(m);
      SetHighlight();
   }
}
void DestructMesh::drawPhys()C
{
   Flt scale=(Sin(Time.appTime()*1)*0.5f+0.5f)*1;
   REPA(_parts)
   {
      Matrix m; m.setPos(_parts[i].mesh.ext.pos*scale);

      SetMatrix(m); _parts[i].phys.draw(WHITE);
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
void DestructMesh::Part::del()
{
   mesh.del();
   phys.del();
}
Bool DestructMesh::Part::save(File &f)C
{
   if(mesh.save(f))
   if(phys.save(f))
      return f.ok();
   return false;
}
Bool DestructMesh::Part::load(File &f)
{
   if(mesh.load(f))
   if(phys.load(f))
      if(f.ok())return true;
   del(); return false;
}
Bool DestructMesh::save(C Str &name)
{
   File f; if(f.writeTry(name))
   {
      f.putUInt (CC4_DSTR);
      f.cmpUIntV(0); // version
      if(_parts .save   (f))
      if(_joints.saveRaw(f))
         if(f.flushOK())return true;

      f.del(); FDelFile(name);
   }
   return false;
}
Bool DestructMesh::load(C Str &name)
{
   File f; if(f.readTry(name) && f.getUInt()==CC4_DSTR)switch(f.decUIntV())
   {
      case 0:
      {
         if(_parts .load   (f))
         if(_joints.loadRaw(f))
            if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
