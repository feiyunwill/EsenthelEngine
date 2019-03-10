/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
struct Overlay
{
   Flt          time    ;
   Extent       ext     ;
 C MaterialPtr *material;
   Matrix       matrix  ;
};
static void AddOverlay(Cell<Area> &cell, Overlay &overlay)
{
 C Area &area=cell(); if(area.loaded() && area._data)
   {
    C MeshGroup &mshg=area._data->mesh; FREPA(mshg)
      {
       C Mesh &mesh=mshg.meshes[i]; if(Cuts(overlay.ext, mesh.ext))
         {
            MeshOverlay mo; if(mo.createStatic(mesh, *overlay.material, overlay.matrix))
            {
               WorldManager::MeshOverlay2 &mo2 =area.world()->_mesh_overlays.New(); mo2.time=overlay.time;
                             MeshOverlay  &mo2_=mo2; Swap(mo2_, mo);
            }
         }
      }
   }
}
WorldManager& WorldManager::terrainAddOverlay(C MaterialPtr &material, C Matrix &overlay_matrix, Flt time_to_fade_out)
{
   if(material)
   {
      Overlay overlay;
      overlay.time    = time_to_fade_out;
      overlay.material=&material;
      overlay.ext.set(1)*=overlay_matrix;
      overlay.matrix=overlay_matrix;

     _grid.func(worldToArea(overlay.ext.rectXZ()), AddOverlay, overlay);
   }
   return T;
}
/******************************************************************************/
WorldManager& WorldManager::terrainAddDecal(C Color &color, C MaterialPtr &material, C Matrix &decal_matrix, Flt time_to_fade_out)
{
   if(material)
   {
      Decal2 &decal=_decals.New();
      decal.time        =time_to_fade_out;
      decal.terrain_only=true;
      decal.color       =color.asVec4();
      decal.matrix      =decal_matrix;
      decal.material    (material);
   }
   return T;
}
/******************************************************************************/
}}
/******************************************************************************/
