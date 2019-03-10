/******************************************************************************/
class EmbedObj
{
   Mesh        mesh;
   MeshBase    phys_path , // phys for path  (has phys mesh/convex/shapes)
               phys_actor; // phys for actor (has phys mesh/convex       ), shapes will be taken from phys_shapes
   Rect        mesh_rect, phys_path_rect, phys_actor_rect;
   Memc<Shape> phys_shapes;
   OBJ_PATH    path;

   void set(C Matrix &matrix, C Object &src, bool set_mesh, bool set_phys, OBJ_PATH *ovr_path, uint *ovr_mesh_variation_id); // this is called on secondary thread

public:
   EmbedObj();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
