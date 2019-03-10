/******************************************************************************/
Heightmap heightmap;
Mesh      mesh;
PhysPart  phys;
Actor     actor;
UID       grass_id=UID(3015359194, 1248539670, 3822661250, 1853126960),
           path_id=UID(2519161163, 1078307672, 184317862, 2816697732);
/******************************************************************************/
void Build(Heightmap &heightmap, Mesh &mesh, PhysPart &phys, Actor &actor) // build 'mesh', 'phys' and 'actor' from 'heightmap'
{
   actor.del();

   // build mesh
   heightmap.build(mesh, 0, 6, HM_AO|HM_SOFT, null, null, null, null, null, null, null, null);

   // adjust mesh scale and position
   flt scale=32;
   Vec pos(0, 0, 0);
   mesh.scaleMoveBase(scale, pos);

   // simplify the mesh
   mesh.simplify (1, 0.05, 1, 0.02, 0.02, 1, PI, true, SIMPLIFY_PLANES); // simplify
   mesh.setRender(); // set rendering version from software version

   // create physical body
   MeshBase base; base.createPhys(mesh); base.simplify(1, 0.05, 1, 1, 1, 1, PI, true, SIMPLIFY_PLANES); // create a 1 MeshBase from all MeshParts in mesh and simplify it again
   phys .createMesh(base); // create physical body from that MeshBase
   actor.create    (phys); // create actor         from physical body

   mesh.delBase(); // mesh software version is no longer needed
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);

   Cam.dist=24;
   Cam.pitch=-1.3;
   Cam.at.set(16, 0, 16);

   Sky.atmospheric();
}
/******************************************************************************/
bool Init()
{
   Physics.create(EE_PHYSX_DLL_PATH);

   heightmap.create(32, 0, grass_id, false, null, null, null, null, null, null, null, null); // create heightmap

   Build(heightmap, mesh, phys, actor); // build mesh phys actor from heightmap

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.01, 50, CAMH_ZOOM|(Ms.b(1) ? CAMH_MOVE : CAMH_ROT));

   // adjust the heightmap
   {
      // set material
      heightmap.setMaterial(Random(32), Random(32), Random(2) ? grass_id : path_id, null, null, null, VecB4(255, 0, 0, 0));

      // set height
      Vec2 pos; CosSin(pos.x, pos.y, Time.time()); pos=pos*8+16;
      REPD(x, heightmap.resolution())
      REPD(y, heightmap.resolution())
      {
         flt d=Dist(Vec2(x,y), pos);
         heightmap.height(x, y, BlendSmoothCube(d/10)*0.1);
      }
   }

   // rebuild after making changes
   Build(heightmap, mesh, phys, actor);

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         mesh.draw(MatrixIdentity); // draw mesh

         LightDir(Cam.matrix.z, 1-D.ambientColor()).add();
      }break;
   }
}
/******************************************************************************/
void Draw()
{
   Renderer.wire=Kb.b(KB_TILDE);
   Renderer(Render);

   if(Ms.b(0))
   {
      Renderer.setDepthForDebugDrawing();
      Physics.draw();
   }
   
   D.text(0, 0.9, "Press LMB to draw Physics");
   D.text(0, 0.8, "Press Tilde for Wireframe view");
   D.text(0, 0.7, S+Time.fps());
}
/******************************************************************************/
