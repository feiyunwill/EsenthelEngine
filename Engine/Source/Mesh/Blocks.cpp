/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
// BLOCKS OCCLUSION
/******************************************************************************

   Some nodes can repeat themself in other parents, but it's all good,
      because they have occlusion calculated based on their parents.
      For example:
                -> Node B -> Node D0 (same coordinates as Node D1)
         Node A
                -> Node C -> Node D1 (same coordinates as Node D0)

/******************************************************************************

   Following code iterates through points in a cube, and processes only those inside a Ball
   It then calculates the average normalized direction vector X component, which in the end gives a result of 0.5

   Int r=256, num=0;
   Dbl sum=0;
   for(Int x=-r; x<=r; x++)
   for(Int y=-r; y<=r; y++)
   for(Int z=-r; z<=r; z++)
   {
      VecD dir(x, y, z);
      if(dir.length()>r)continue;
      dir.normalize(); dir.abs();
      sum+=dir.x;
      num++;
   }
   sum/=num;
   // sum==2

/******************************************************************************/
static Int MaxBlocks(C Blocks &blocks, C BlocksOcclusion *occl)
{
   if(occl)if(Int steps=occl->maxSteps())
   {
      if(blocks.subDivision()>1)steps=DivCeil(steps, blocks.subDivision());
      return steps;
   }
   return 0;
}
/******************************************************************************/
static void PrepareAndClean(Memc<BlocksOcclusion::Node> &nodes, C BlocksOcclusion &occl)
{
   const Flt threshold=0.01f;
   REPA(nodes)
   {
      BlocksOcclusion::Node &node=nodes[i];
      Flt frac;
      switch(occl.aoCombineMode())
      {
         default                  : frac=    node.fraction[0]+ node.fraction[1] ; break; // ADD
         case BlocksOcclusion::MAX: frac=Max(node.fraction[0], node.fraction[1]); break; // MAX
      }
      if(frac<=threshold)nodes.remove(i);else
      {
         node.fraction_axis[3]=RoundU(frac*65536);
         if(occl.angles())
         {
            Vec dir=node.pos+0.5f; dir.normalize(); dir.abs(); dir*=frac*(65536/0.5f); // div by 0.5 because the average normal component based on codes at the top is 0.5, so we need to recompensate that
            node.fraction_axis[0]=Round(dir.x);
            node.fraction_axis[1]=Round(dir.y);
            node.fraction_axis[2]=Round(dir.z);
         }else
         {
            node.fraction_axis[0]=
            node.fraction_axis[1]=
            node.fraction_axis[2]=node.fraction_axis[3];
         }
         node.fraction_axis[3]/=2; // div by 2 because for this "axis" we're checking both hemispheres
         PrepareAndClean(node.nodes, occl);
      }
   }
}
BlocksOcclusion::BlocksOcclusion()
{
   REPAO(_steps)=0; REPAO(_strength)=0;
  _aocm=ADD;
  _angles=false;
}
void BlocksOcclusion::create(Int steps, Flt strength, Int steps_1, Flt strength_1, COMBINE_MODE ao_combine_mode, Bool angles)
{
  _nodes.clear();
  _steps   [0]=Clamp(steps  , 0, 1024);
  _steps   [1]=Clamp(steps_1, 0, 1024);
  _strength[0]=SAT  (strength  );
  _strength[1]=SAT  (strength_1);
  _angles     =angles;
  _aocm       =ao_combine_mode;
   if(steps_1>steps){Swap(steps, steps_1); Swap(strength, strength_1);} // make #0 always the higher
   Int steps_1_sqr=Sqr(steps_1);

   Int rays=4096,
       rays_res=Max(1, SqrtI(rays/6)); // rays resolution in 2D, in one of 6 cube faces
   rays=Sqr(rays_res)*6; // total number of rays

   if(steps>0)
   {
      Flt ray_fraction =1.0f/rays;
          ray_fraction*=  2; // multiply by   2 because we're interested in hemispheres only (when calculating AO only half of nodes will be processed and we can get only up to half of rays)
          ray_fraction*=255; // multiply by 255 because we're creating Color objects which are made from bytes
      Flt ray_fraction_0=ray_fraction*strength  ,
          ray_fraction_1=ray_fraction*strength_1;

      Flt ray_step=PI_2/rays_res;
      Vec ray_base; ray_base.z=1;
      REPD(x, rays_res)
      {
         ray_base.x=Tan(x*ray_step-PI_4);
         REPD(y, rays_res)
         {
            ray_base.y=Tan(y*ray_step-PI_4);
            Vec n=ray_base; n.normalize(); // normalized
            REPD(f, 6) // 6 cube faces
            {
               Vec ray; switch(f)
               {
                  case 0: ray.set( n.x,  n.y,  n.z); break;
                  case 1: ray.set(-n.z,  n.y,  n.x); break;
                  case 2: ray.set(-n.x, -n.y, -n.z); break;
                  case 3: ray.set( n.z, -n.y, -n.x); break;
                  case 4: ray.set( n.x,  n.z, -n.y); break;
                  case 5: ray.set(-n.x, -n.z,  n.y); break;
               }
               Memc<Node> *nodes=&_nodes;
               for(VoxelWalker walker(ray*EPS, ray*(steps-EPS)); walker.active(); walker.step()) // don't start at zero, but from a small step along the ray, also end a bit earlier so we don't accidentally add blocks out of range
               {
                C VecI &pos =walker.pos();
                  Node *node=null; REPA(*nodes)if((*nodes)[i].pos==pos){node=&(*nodes)[i]; break;} if(!node)node=&nodes->New().init(pos); // get node with such position (create new if not found)
                                                       node->fraction[0]+=ray_fraction_0; // add ray to ray counter (as fraction of total amount of rays)
                  if((pos+0.5f).length2()<=steps_1_sqr)node->fraction[1]+=ray_fraction_1; // add ray to ray counter (as fraction of total amount of rays)
                  nodes=&node->nodes;
               }
            }
         }
      }
      PrepareAndClean(_nodes, T);
   }
}
BlocksOcclusion& BlocksOcclusion::steps        (Int          steps   ) {create(  steps  , T.strength(), T.steps1(), T.strength1(), T.aoCombineMode(), T.angles()); return T;}
BlocksOcclusion& BlocksOcclusion::steps1       (Int          steps   ) {create(T.steps(), T.strength(),   steps   , T.strength1(), T.aoCombineMode(), T.angles()); return T;}
BlocksOcclusion& BlocksOcclusion::strength     (Flt          strength) {create(T.steps(),   strength  , T.steps1(), T.strength1(), T.aoCombineMode(), T.angles()); return T;}
BlocksOcclusion& BlocksOcclusion::strength1    (Flt          strength) {create(T.steps(), T.strength(), T.steps1(),   strength   , T.aoCombineMode(), T.angles()); return T;}
BlocksOcclusion& BlocksOcclusion::aoCombineMode(COMBINE_MODE aocm    ) {create(T.steps(), T.strength(), T.steps1(), T.strength1(),   aocm           , T.angles()); return T;}
BlocksOcclusion& BlocksOcclusion::angles       (Bool         on      ) {create(T.steps(), T.strength(), T.steps1(), T.strength1(), T.aoCombineMode(),   on      ); return T;}

BlocksOcclusion::Node& BlocksOcclusion::Node::init(C VecI &pos)
{
   fraction[0]=fraction[1]=0;
   dir=(((pos.x>=0) ? DIRF_RIGHT   : DIRF_LEFT)
      | ((pos.y>=0) ? DIRF_UP      : DIRF_DOWN)
      | ((pos.z>=0) ? DIRF_FORWARD : DIRF_BACK));
   T.pos=pos; return T;
}

/*static void TestBlocksOcclusion()
{
      void save(FileText &f)
      {
         f++;
         f.put("pos      = ", pos);
         f.put("fraction = ", (Dbl)fraction);
         FREPAO(nodes).save(f);
         f--;
      }

   FileText f; f.write("c:/rays.txt");
   BlocksOcclusion occl; occl.create(BlocksBorder); FREPAO(occl._nodes).save(f);
}
/******************************************************************************/
// BLOCKS MAP
/******************************************************************************/
BlocksMap::BlocksMap() {_resolution=0; _base_matrix.zero();}
void BlocksMap::create(Int resolution, C Matrix &base_matrix)
{
   T._resolution =resolution;
   T._base_matrix=base_matrix;
}
Matrix BlocksMap::matrix(Int x, Int y)C
{
   Matrix m=_base_matrix;
   m.pos+=m.x*(x*resolution());
   m.pos+=m.z*(y*resolution());
   return m;
}
Bool BlocksMap::raycast(C Vec &start, C Vec &move, Vec *hit_pos, Vec *hit_normal)C
{
   Vec block_start=start,
       block_move =move ;

   block_start/=_base_matrix      ; block_start/=resolution();
   block_move /=_base_matrix.orn(); block_move /=resolution();

   for(PixelWalker walker(block_start.xz(), (block_start+block_move).xz()); walker.active(); walker.step())
      if(Blocks *blocks=findBlocks(walker.pos().x, walker.pos().y))
         if(blocks->raycast(start, move, hit_pos, hit_normal, &matrix(walker.pos().x, walker.pos().y)))return true;
   return false;
}
Vec BlocksMap::light(C Vec &pos, C BlocksOcclusion *occl, C MemPtr<Blocks::Light> &lights)C
{
   Vec   local_pos=pos/_base_matrix; Vec block_pos=local_pos/resolution();
   VecI2 xz=Floor(block_pos.xz());
   Vec   lum=1; // assume full light at start
   if(occl)
      if(Flt max_strength=((occl->steps()>occl->steps1()) ? occl->strength() : occl->strength1()))
         if(Blocks *blocks=findBlocks(xz.x, xz.y))
   {
      local_pos.x-=xz.x*resolution();
      local_pos.z-=xz.y*resolution();
      Int  min_level_i=Max(0, blocks->findLevelI(Round(local_pos.y)-MaxBlocks(*blocks, occl)));
      if(blocks->subDivision()>1)local_pos*=blocks->subDivision();
      VecI local_posi=Round(local_pos);
      Byte b=blocks->brightness(occl, local_posi.x, local_posi.y, local_posi.z, AXIS_TYPE(3), DIRF_ALL, min_level_i, Blocks::Neighbors(this, xz.x, xz.y, resolution()));
      Flt  o=(255-b)/(255.0f*max_strength); // take normalized occlusion 0..1
      lum=Sat(1-(o*2-1)*max_strength); // convert it to brightness, where full brightness is at 0.5 occl, and zero brightness is at 1.0 occl (and take occlusion strength into account)
   }
   REPA(lights)
   {
    C Blocks::Light &light=lights[i];
      Vec delta=light.ball.pos-pos;
      Flt dist2=delta.length2(), radius2=Sqr(light.ball.r);
      if( dist2<radius2)if(!raycast(pos, delta))lum+=light.color*(1-dist2/radius2);
   }
   return lum;
}
/******************************************************************************/
// BLOCKS NEIGHBORS
/******************************************************************************/
void Blocks::Neighbors::clear()
{
   Zero(T);
}
void Blocks::Neighbors::set(C Blocks *l, C Blocks *r, C Blocks *b, C Blocks *f, C Blocks *lb, C Blocks *lf, C Blocks *rb, C Blocks *rf, Int res)
{
   T.l =((l  && l ->resolution()==res) ? l  : null);
   T.r =((r  && r ->resolution()==res) ? r  : null);
   T.b =((b  && b ->resolution()==res) ? b  : null);
   T.f =((f  && f ->resolution()==res) ? f  : null);
   T.lb=((lb && lb->resolution()==res) ? lb : null);
   T.lf=((lf && lf->resolution()==res) ? lf : null);
   T.rb=((rb && rb->resolution()==res) ? rb : null);
   T.rf=((rf && rf->resolution()==res) ? rf : null);
}
Blocks::Neighbors::Neighbors(C Blocks *l, C Blocks *r, C Blocks *b, C Blocks *f, C Blocks *lb, C Blocks *lf, C Blocks *rb, C Blocks *rf, Int resolution)
{
   set(l, r, b, f, lb, lf, rb, rf, resolution);
}
Blocks::Neighbors::Neighbors(C BlocksMap *map, Int x, Int y, Int resolution)
{
   if(map)set(map->findBlocks(x-1, y  ), map->findBlocks(x+1, y  ), map->findBlocks(x  , y-1), map->findBlocks(x  , y+1),
              map->findBlocks(x-1, y-1), map->findBlocks(x-1, y+1), map->findBlocks(x+1, y-1), map->findBlocks(x+1, y+1), resolution);
   else   clear();
}
/******************************************************************************/
// BLOCKS LEVEL
/******************************************************************************/
void Blocks::Level::del()
{
   Free(_map);
   actor.del(); // delete actor before physical body
   phys .del();
}
void Blocks::Level::createPhysBody(C Matrix &matrix, Int res, C Level *down, C Level *up)
{
   Memt<Vec  > vtxs;
   Memt<VecI4> quads;

#if 0 // double-sided physics mesh - unavailable on PhysX
   // X walls
   FREPD(x, res+1)
   {
      FREPD(z, res)if(wallX(x, z, res))
      {
         Int to=z; for(; ; to++)
         {
            if(InRange(to+1, res) && wallX(x, to+1, res))continue;
            break;
         }
         Int v=vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(x, y  , z   );
         vtxs.New().set(x, y+1, z   );
         vtxs.New().set(x, y+1, to+1);
         vtxs.New().set(x, y  , to+1);
         z=to;
      }
   }

   // Z walls
   FREPD(z, res)
   {
      FREPD(x, res)if(wallZ(z, x, res))
      {
         Int to=x; for(; ; to++)
         {
            if(InRange(to+1, res) && wallZ(to+1, z, res))continue;
            break;
         }
         Int v=vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(x   , y  , z);
         vtxs.New().set(x   , y+1, z);
         vtxs.New().set(to+1, y+1, z);
         vtxs.New().set(to+1, y  , z);
         x=to;
      }
   }
#else
   const Bool continuous=true; // if generate fewer, but larger faces

   // back/forward walls
   FREPD(z, res)
   {
      // back
      FREPD(x, res)if(wallBack(x, z, res))
      {
         Int to=x; for(Int i=x+1; i<res; i++)if(wallBack(i, z, res))to=i;else if(!continuous || !map(i, z, res))break;
         Int v =vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(x   , y  , z);
         vtxs.New().set(x   , y+1, z);
         vtxs.New().set(to+1, y+1, z);
         vtxs.New().set(to+1, y  , z);
         x=to;
      }
      // forward
      FREPD(x, res)if(wallForward(x, z, res))
      {
         Int to=x; for(Int i=x+1; i<res; i++)if(wallForward(i, z, res))to=i;else if(!continuous || !map(i, z, res))break;
         Int v =vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(to+1, y  , z+1);
         vtxs.New().set(to+1, y+1, z+1);
         vtxs.New().set(x   , y+1, z+1);
         vtxs.New().set(x   , y  , z+1);
         x=to;
      }
   }

   // left/right walls
   FREPD(x, res)
   {
      // left
      FREPD(z, res)if(wallLeft(x, z, res))
      {
         Int to=z; for(Int i=z+1; i<res; i++)if(wallLeft(x, i, res))to=i;else if(!continuous || !map(x, i, res))break;
         Int v =vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(x, y  , to+1);
         vtxs.New().set(x, y+1, to+1);
         vtxs.New().set(x, y+1, z   );
         vtxs.New().set(x, y  , z   );
         z=to;
      }
      // right
      FREPD(z, res)if(wallRight(x, z, res))
      {
         Int to=z; for(Int i=z+1; i<res; i++)if(wallRight(x, i, res))to=i;else if(!continuous || !map(x, i, res))break;
         Int v =vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(x+1, y  , z   );
         vtxs.New().set(x+1, y+1, z   );
         vtxs.New().set(x+1, y+1, to+1);
         vtxs.New().set(x+1, y  , to+1);
         z=to;
      }
   }
#endif

   // bottom/top walls
   FREPD(z, res)
   {
      // bottom
      FREPD(x, res)if(wallBottom(x, z, res, down))
      {
         Int to=x; for(Int i=x+1; i<res; i++)if(wallBottom(i, z, res, down))to=i;else if(!continuous || !map(i, z, res))break;
         Int v =vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(x   , y, z+1);
         vtxs.New().set(x   , y, z  );
         vtxs.New().set(to+1, y, z  );
         vtxs.New().set(to+1, y, z+1);
         x=to;
      }
      // top
      FREPD(x, res)if(wallTop(x, z, res, up))
      {
         Int to=x; for(Int i=x+1; i<res; i++)if(wallTop(i, z, res, up))to=i;else if(!continuous || !map(i, z, res))break;
         Int v =vtxs.elms(); quads.New().set(v, v+1, v+2, v+3);
         vtxs.New().set(x   , y+1, z  );
         vtxs.New().set(x   , y+1, z+1);
         vtxs.New().set(to+1, y+1, z+1);
         vtxs.New().set(to+1, y+1, z  );
         x=to;
      }
   }

   MeshBase base(vtxs.elms(), 0, 0, quads.elms());
   CopyN(base.vtx .pos(), vtxs .data(), vtxs .elms());
   CopyN(base.quad.ind(), quads.data(), quads.elms());
   base.transform(matrix);

   actor.del(); // delete actor before changing physical body
   phys .createMesh(base);
}
/******************************************************************************/
Bool Blocks::Level::save(File &f, Int resolution, Bool include_mesh_and_phys_body)C
{
   f.cmpUIntV(0);
   f.cmpIntV (y);
   f.putN    (_map, resolution*resolution);
   if(include_mesh_and_phys_body)if(!phys.save(f))return false;
   return f.ok();
}
Bool Blocks::Level::load(File &f, Int resolution, Bool include_mesh_and_phys_body)
{
   actor.del(); // manually delete actor before changing physical body
   switch(f.decUIntV())
   {
      case 0:
      {
         init(resolution, f.decIntV());
         if(f.getN(_map, resolution*resolution))
         {
            if(include_mesh_and_phys_body)if(!phys.load(f))goto error;
            if(f.ok())return true;
         }
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
// BLOCKS::PART
/******************************************************************************/
void Blocks::Part::create(MeshBase &base)
{
   base.create(vtxs.elms(), 0, 0, quads.elms(), VTX_TEX0|VTX_COLOR);

   // vertexes
   REPA(vtxs)
   {
    C Vtx &vtx=vtxs[i];
      base.vtx.pos  (i)=vtx.pos;
      base.vtx.tex0 (i)=vtx.tex;
      base.vtx.color(i)=vtx.col;
   }

   // quads
   CopyN(base.quad.ind(), quads.data(), base.quads());
}
/******************************************************************************/
// BLOCKS::LEVEL BRIGHTNESS
/******************************************************************************/
Blocks::LevelBrightness::LevelBrightness(C Blocks &blocks, C Neighbors &neighbors, C BlocksOcclusion *occl, C BlocksMap *map, C VecI2 &blocks_pos, C MemtN<LightEx, 256> &lights) : blocks(blocks), neighbors(neighbors), occl(occl), map(map), blocks_pos(blocks_pos), lights(lights)
{
   T.size=blocks.resolution()*blocks.subDivision()+1;
   T.points.setNum(size*size);
}
void Blocks::LevelBrightness::clear()
{
   REPD(z, size)
   REPD(x, size)point(x, z).clear();
}
void Blocks::LevelBrightness::clear(Int min_x, Int min_z, Int max_x, Int max_z)
{
   Int sd=blocks.subDivision(); if(sd>1){min_x*=sd; min_z*=sd; max_x*=sd; max_z*=sd;}
   max_x++; // increase x because when computing brightness for block x we use "x" and "x+1" points
   max_z++; // increase z because when computing brightness for block z we use "z" and "z+1" points
   for(Int z=min_z; z<=max_z; z++)
   for(Int x=min_x; x<=max_x; x++)point(x, z).clear();
}
Blocks::LevelBrightness& Blocks::LevelBrightness::setLevelI(Int level_i)
{
   min_level_i=Max(0, level_i-MaxBlocks(blocks, occl));
   return T;
}
C Color& Blocks::LevelBrightness::brightness(Int x, Int y, Int z, DIR_ENUM dir)
{
   Point &point=T.point(x, z);
#if DEBUG && 0 // confirm that we're reusing pre-computed brightness
   if(point.computed_y==y && (point.computed_dir&DirToFlag(dir)))return 0;
#endif
   if(point.computed_y!=y) // point Y is different than requested
   {
      point.computed_y  =y;
      point.computed_dir=0; // clear all computed directions
   }
   UInt dir_flag=DirToFlag(dir);
   if(!(point.computed_dir&dir_flag)) // if direction was not yet computed
   {
      point.computed_dir|=dir_flag; // set as computed
      Byte  brightness=blocks.brightness(occl, x, y, z, DirToAxis(dir), dir_flag, min_level_i, neighbors); // calculate
      Color color(brightness, brightness, brightness);
      if(   brightness<255) // if not max then add local lights
      {
         Vec col=0;
         REPA(lights)
         {
          C LightEx &light=lights[i];
            Vec pos(x, y, z); if(blocks.subDivision()>1)pos/=blocks.subDivision();
            Vec delta=light.ball.pos-pos;
            Flt dist2=delta.length2(), radius2=Sqr(light.ball.r);
            if( dist2<radius2)
            {
               Flt dot=1;
               if(dist2){Flt dist=Sqrt(dist2); delta/=dist; dot=Dot(VecDir[dir], delta); pos+=delta*EPSL;} // normalize and move slightly towards the light, so we don't start from the wall itself
               if(dot>0)
               {
                  if(map)pos*=map->matrix(blocks_pos.x, blocks_pos.y); // convert to world space, needed for 'BlocksMap.raycast'
                  if(map ? !map->raycast(pos, light.world_pos-pos) : !blocks.raycast(pos, light.ball.pos-pos))col+=((1-dist2/radius2)*dot)*light.color;
               }
            }
         }
         color=ColorAdd(color, col);
      }
      point.brightness[dir]=color;
   }
   return point.brightness[dir];
}
/******************************************************************************/
// BLOCKS
/******************************************************************************/
void Blocks::zero()
{
  _resolution=_sub_division=0;
}
Blocks::Blocks() {zero();}
Blocks& Blocks::del()
{
  _mesh       .del();
  _levels     .del();
  _materials  .del();
  _mtrl_combos.del();
   zero(); return T;
}
Blocks& Blocks::create(Int resolution, Int sub_division)
{
   del();
   T._resolution=Max(resolution, 0);
   subDivision(sub_division);
   return T;
}
/******************************************************************************/
Bool Blocks::hasBlock(C VecI &pos)C
{
   if(InRange(pos.x, resolution())
   && InRange(pos.z, resolution()))
      if(C Level *level=findLevel(pos.y))
         return level->map(pos.x, pos.z, resolution())!=0;
   return false;
}
Bool Blocks::hasBlock(C VecI &pos, Int min_level_i)C
{
   if(InRange(pos.x, resolution())
   && InRange(pos.z, resolution()))
      if(C Level *level=findLevel(pos.y, min_level_i))
         return level->map(pos.x, pos.z, resolution())!=0;
   return false;
}
MaterialPtr Blocks::material(C VecI &pos)C
{
   if(InRange(pos.x, resolution())
   && InRange(pos.z, resolution()))
      if(C Level *level=findLevel(pos.y))
   {
      Byte m=level->map(pos.x, pos.z, resolution());
      if(m && InRange(m, _mtrl_combos))
      {
       C MtrlCombo &mc=_mtrl_combos[m];
         if(InRange(mc.top, _materials))return _materials[mc.top];
      }
   }
   return null;
}
void Blocks::get(C VecI &pos, MaterialPtr &top, MaterialPtr &side, MaterialPtr &bottom)C
{
   // don't clear materials at start, because those are unnecessary operations, which can be done on fail instead
   if(InRange(pos.x, resolution())
   && InRange(pos.z, resolution()))
      if(C Level *level=findLevel(pos.y))
   {
      Byte m=level->map(pos.x, pos.z, resolution());
      if(m && InRange(m, _mtrl_combos))
      {
       C MtrlCombo &mc=_mtrl_combos[m];
         if(InRange(mc.top   , _materials))top   =_materials[mc.top   ];else top   .clear();
         if(InRange(mc.side  , _materials))side  =_materials[mc.side  ];else side  .clear();
         if(InRange(mc.bottom, _materials))bottom=_materials[mc.bottom];else bottom.clear();
         return;
      }
   }
   top.clear(); side.clear(); bottom.clear();
}
Bool Blocks::set(Int x, Int y, Int z, Byte b)
{
   if(InRange(x, resolution())
   && InRange(z, resolution()))
      if(Level *level=(b ? &getLevel(y) : findLevel(y))) // insert block ? always get level : find if exists to remove
   {
      Byte &m=level->map(x, z, resolution());
      if(m!=b){m=b; return true;}
   }
   return false;
}
Bool Blocks::set(Int x, Int y, Int z, C MaterialPtr &top, C MaterialPtr &side, C MaterialPtr &bottom)
{
   if(!top && !side && !bottom)return set(x, y, z, 0); // if clear
   if( top &&  side &&  bottom) // if all are set (we can't set materials only if some of them are set)
   {
      Int t=_materials.getMaterialIndex(top   ),
          s=_materials.getMaterialIndex(side  ),
          b=_materials.getMaterialIndex(bottom);
      if(t>=0 && s>=0 && b>=0) // if all were found
      {
         Int mc=-1;
         if(!_mtrl_combos.elms())_mtrl_combos.New().set(0, 0, 0); // 0-th material combo is reserved for null materials
         FREPA(_mtrl_combos){C MtrlCombo &mtrl_combo=_mtrl_combos[i]; if(mtrl_combo.top==t && mtrl_combo.side==s && mtrl_combo.bottom==b){mc=i; break;}} // find existing
         if(mc< 0 && _mtrl_combos.elms()<256){mc=_mtrl_combos.elms(); _mtrl_combos.New().set(t, s, b);} // support indexes only up to 255
         if(mc>=0)return set(x, y, z, mc);
      }
   }
   return false; // no change was made
}
Bool Blocks::set(Int x, Int y, Int z, C MaterialPtr &material)
{
   return set(x, y, z, material, material, material);
}
/******************************************************************************/
Blocks& Blocks::subDivision(Int steps)
{
   T._sub_division=Max(1, steps);
   return T;
}
Blocks& Blocks::setMesh(Flt tex_scale, C BlocksOcclusion *occl, C BoxI *local_box, C VecI2 &blocks_pos, C BlocksMap *map, C MemPtr<Light> &lights, Bool optimize, Flt max_face_length)
{
   BoxI box; if(local_box)
   {
      box=*local_box; box.extend(Max(1, MaxBlocks(T, occl))); // we must extend the box by at least one, because block face creation depends on its neighbors, we also extend it by 'BlocksOcclusion' because the blocks can affect the lighting of the nearby blocks
      if(box.min.x>=resolution() || box.min.z>=resolution() || box.max.x<0 || box.max.z<0)return T; // update box is outside of range

      Clamp(box.min.x, 0, resolution()-1);
      Clamp(box.max.x, 0, resolution()-1);
      Clamp(box.min.z, 0, resolution()-1);
      Clamp(box.max.z, 0, resolution()-1);
   }else
   {
      box.setX(0, resolution()-1).setZ(0, resolution()-1);
      if(_levels.elms())box.setY(_levels.first().y, _levels.last().y);else box.setY(0, -1);
   }

   Neighbors neighbors(map, blocks_pos.x, blocks_pos.y, resolution());

   MemtN<Part, 256> parts;
   if(_levels.elms())
   {
      // add local lights
      MemtN<LightEx, 256> local_lights;
      if(lights.elms())
      {
         Box boxf=box; boxf.max+=1;
         Matrix m=(map ? map->matrix(blocks_pos.x, blocks_pos.y) : MatrixIdentity);
         REPA(lights)
         {
            LightEx light; SCAST(Light, light)=lights[i];
            light.world_pos=light.ball.pos; if(map)light.ball/=m;
            if(Cuts(boxf, light.ball))Swap(local_lights.New(), light);
         }
      }
      LevelBrightness level_brightness[2]={LevelBrightness(T, neighbors, occl, map, blocks_pos, local_lights),
                                           LevelBrightness(T, neighbors, occl, map, blocks_pos, local_lights)};
      REPA(level_brightness)
      {
         LevelBrightness &lb=level_brightness[i]; if(local_box)lb.clear(box.min.x, box.min.z, box.max.x, box.max.z);else lb.clear();
      }
      for(Int i=(local_box ? findLevelI(box.min.y) : 0); i<_levels.elms(); i++)
      {
         Level &level=_levels[i]; if(local_box && level.y>box.max.y)break;
         buildLevel(tex_scale, parts, box.min.x, box.min.z, box.max.x, box.max.z, i, neighbors, level_brightness);
      }
   }

   // mesh
   Memt<Int> sel;
  _mesh.parts.setNum(local_box ? Max(_mesh.parts.elms(), parts.elms()) : parts.elms()); // for updating only include new parts if any
   REPA(parts)if(InRange(i+1, _materials)) // 'parts[i]' are mapped to 'mesh.parts[i]' and 'materials[i+1]'
   {
      Part     & part=      parts[i];
      MeshPart &mpart=_mesh.parts[i];
      MeshBase & base=mpart.base;

      Bool removed=false;
      if(local_box) // for update, we need to first remove existing faces in the box
      {
         REPA(base.quad)
         {
          C VecI4 &ind   =base.quad.ind(i);
            Vec    center=Avg(base.vtx.pos(ind.x), base.vtx.pos(ind.y), base.vtx.pos(ind.z)) - base.vtx.nrm(ind.x)*0.5f;
            VecI   pos   =Floor(center);
            if(Cuts(pos, box))sel.add(i^SIGN_BIT);
         }
         if(sel.elms())
         {
            removed=true;
            base.removeFaces(sel); sel.clear();
         }
      }

      if(!local_box || removed || part.is()) // process only if resetting, something was removed or something is to be added
      {
         if(!local_box || !base.is())part.create(base);else{MeshBase temp; part.create(temp); base+=temp;} // if resetting or the base is empty then create it, otherwise add to it
         mpart.material(_materials[i+1]);
         base.setNormals(); // set vtx normals before welding because they need to be tested as well (otherwise holes on the sides could appear on blocks)
         base.weldVtx(VTX_TEX0|VTX_COLOR|VTX_NRM, 0.01f, EPS_COL_COS, -1);
         mpart.setAutoTanBin();
         if(optimize)
         {
            MeshBase optimized(base); optimized.weldCoplanarFaces(EPS_COL_COS, -1, false, max_face_length);
            mpart.render.create(optimized, ~0, false);
            mpart.setShader(0);
         }else mpart.setRender(false);
      }
   }
  _mesh.setBox();
   return T;
}
/******************************************************************************/
Blocks& Blocks::setPhysBody(C Matrix &matrix, C BoxI *local_box, Bool create_actor)
{
   BoxI box;
   if(local_box)
   {
      box=*local_box; box.extend(1); // we must extend the box by at least one, because block face creation depends on its neighbors
      if(box.min.x>=resolution() || box.min.z>=resolution() || box.max.x<0 || box.max.z<0)return T;
   }

   for(Int i=(local_box ? findLevelI(box.min.y) : 0); i<_levels.elms(); i++)
   {
      Level &level=_levels[i]; Int y=level.y; if(local_box && y>box.max.y)break;
    C Level *down =_levels.addr(i-1); if(down && down->y!=y-1)down=null;
    C Level *up   =_levels.addr(i+1); if(up   && up  ->y!=y+1)up  =null;
      level.createPhysBody(matrix, resolution(), down, up); if(create_actor)level.createActor();
   }
   return T;
}
Blocks& Blocks::   delActor() {REPAO(_levels).actor.del  (); return T;}
Blocks& Blocks::createActor() {REPAO(_levels).createActor(); return T;}
/******************************************************************************/
Bool Blocks::raycast(C Vec &start, C Vec &move, Vec *hit_pos, Vec *hit_normal, C Matrix *matrix)C
{
   if(_levels.elms())
   {
      Vec local_start=start, local_move=move;
      if(matrix)
      {
         local_start/=*matrix;
         local_move /= matrix->orn();
      }
      Flt min_y=_levels.first().y  ,
          max_y=_levels.last ().y+1;
      Box box(Vec(0, min_y, 0), Vec(resolution(), max_y, resolution())); box.extend(0.1f); // extend the box so we won't start inside, but from 1 block outside
      Vec hp; if(SweepPointBox(local_start, local_move, box, null, null, &hp))
      {
         Int    last_level_i=SIGN_BIT;
         VecI   last_pos=SIGN_BIT;
         Vec   local_end=local_start+local_move;
         Bool was_inside=false;
         for(VoxelWalker walker(hp, local_end); walker.active(); walker.step())
         {
          C VecI &pos=walker.pos();
            if(InRange(pos.x, resolution())
            && InRange(pos.z, resolution()))
            {
               was_inside=true;
               if(last_level_i==SIGN_BIT)last_level_i=findLevelI(pos.y);
               if(C Level *level=toLevel(pos.y, last_level_i))
               {
                  if(level->map(pos.x, pos.z, resolution()))
                  {
                     if(hit_pos)
                     {
                        if(last_pos.x==SIGN_BIT)
                        {
                           *hit_pos=hp;
                           if(matrix)*hit_pos*=*matrix;
                        }else
                        {
                           Flt frac=1;
                           if(pos.x>last_pos.x)MIN(frac, LerpR(local_start.x, local_end.x, (Flt)     pos.x));else // moving right
                           if(pos.x<last_pos.x)MIN(frac, LerpR(local_start.x, local_end.x, (Flt)last_pos.x));     // moving left
                           if(pos.y>last_pos.y)MIN(frac, LerpR(local_start.y, local_end.y, (Flt)     pos.y));else // moving up
                           if(pos.y<last_pos.y)MIN(frac, LerpR(local_start.y, local_end.y, (Flt)last_pos.y));     // moving down
                           if(pos.z>last_pos.z)MIN(frac, LerpR(local_start.z, local_end.z, (Flt)     pos.z));else // moving forward
                           if(pos.z<last_pos.z)MIN(frac, LerpR(local_start.z, local_end.z, (Flt)last_pos.z));     // moving back
                          *hit_pos=start+move*frac;
                        }
                     }
                     if(hit_normal)
                     {
                        if(last_pos.x==SIGN_BIT)hit_normal->zero();
                        else                  {*hit_normal=last_pos-pos; if(matrix)*hit_normal*=matrix->orn(); hit_normal->normalize();}
                     }
                     return true;
                  }
               }
            }else if(was_inside)return false; // moved outside of range
            last_pos=pos;
         }
      }
   }
   return false;
}
/******************************************************************************/
Blocks& Blocks::cleanLevels()
{
   REPA(_levels)
   {
    C Level &level=_levels[i];
      REPD(z, resolution())
      REPD(x, resolution())if(level.map(x, z, resolution()))goto have_block;
     _levels.remove(i, true); // when no blocks were found on this level then remove it
   have_block:;
   }
   return T;
}
/******************************************************************************/
Bool Blocks::cleanMtrlCombos(Bool is[256], Byte remap[256])
{
   for(Int i=1; i<_mtrl_combos.elms(); i++)if(!is[i])
   {
      remap[0]=0;
      for(Int i=1, next=1;             i<_mtrl_combos.elms(); i++)if( is[i])remap[i]=next++;else remap[i]=0;
      for(Int i=_mtrl_combos.elms(); --i>=1                 ;    )if(!is[i])_mtrl_combos.remove(i, true); // need to go from the end
      return true;
   }
   return false;
}
Blocks& Blocks::cleanMaterials()
{
   if(_materials.elms()>1)
   {
      // get used material combos
      Bool mc_is[256]; Zero(mc_is); // assume all are unused
      REPA(_levels)
      {
         Level &level=_levels[i];
         REPD(z, resolution())
         REPD(x, resolution())mc_is[level.map(x, z, resolution())]=true;
      }

      // get used materials
      Bool  mtrl_is[256]; ZeroN(mtrl_is, _materials.elms()); // assume all are unused
      REPA(_mtrl_combos)if(mc_is[i])
      {
         MtrlCombo &mc=_mtrl_combos[i];
         mtrl_is[mc.top   ]=true;
         mtrl_is[mc.side  ]=true;
         mtrl_is[mc.bottom]=true;
      }

      // 'mesh.parts[i]' are mapped to 'materials[i+1]'
      REPA(_materials)if(!mtrl_is[i] && i)_mesh.remove(i-1, false); // need to go from the end

      Byte remap[256];
      
      // remap material combos
      if(cleanMtrlCombos(mc_is, remap))REPA(_levels)
      {
         Level &level=_levels[i];
         REPD(z, resolution())
         REPD(x, resolution()){Byte &m=level.map(x, z, resolution()); m=remap[m];}
      }

      // remap materials
      if(_materials.clean(mtrl_is, remap))REPA(_mtrl_combos)
      {
         MtrlCombo &mc=_mtrl_combos[i];
         mc.top   =remap[mc.top   ];
         mc.side  =remap[mc.side  ];
         mc.bottom=remap[mc.bottom];
      }
   }
   return T;
}
/******************************************************************************/
Bool Blocks::save(File &f, Bool include_mesh_and_phys_body, CChar *path)C
{
   f.cmpUIntV(1); // version
   f.cmpUIntV(resolution()).cmpUIntV(subDivision());
   if(_materials  .save   (f, path))
   if(_mtrl_combos.saveRaw(f))
   {
      f.putBool (include_mesh_and_phys_body);
      f.cmpUIntV(_levels.elms()); FREPA(_levels)if(!_levels[i].save(f, resolution(), include_mesh_and_phys_body))return false;
      if(include_mesh_and_phys_body)if(!_mesh.save(f))return false;
      return f.ok();
   }
   return false;
}
/******************************************************************************/
Bool Blocks::load(File &f, CChar *path)
{
   del(); switch(f.decUIntV()) // version
   {
      case 1:
      {
        _resolution=f.decUIntV(); _sub_division=f.decUIntV();
         if(_materials  .load   (f, path))
         if(_mtrl_combos.loadRaw(f))
         {
            Bool include_mesh_and_phys_body=f.getBool();
           _levels.setNum(f.decUIntV()); FREPA(_levels)if(!_levels[i].load(f, resolution(), include_mesh_and_phys_body))goto error;
            if(include_mesh_and_phys_body)if(!_mesh.load(f))goto error;
            if(f.ok())return true;
         }
      }break;

      case 0:
      {
         f>>_resolution; _sub_division=1;
         if(_materials.load(f, path))
         {
           _mtrl_combos.setNum(_materials.elms()); REPA(_mtrl_combos){MtrlCombo &mc=_mtrl_combos[i]; mc.top=mc.side=mc.bottom=i;}
            Bool include_mesh_and_phys_body=f.getBool();
           _levels.setNum(f.getInt()); FREPA(_levels)if(!_levels[i].load(f, resolution(), include_mesh_and_phys_body))goto error;
            if(include_mesh_and_phys_body)if(!_mesh.load(f))goto error;
            if(f.ok())return true;
         }
      }break;
   }
error:;
   del(); return false;
}
/******************************************************************************/
Int            Blocks::findLevelI(Int y           )C {Int i;     _levels.binarySearch(y, i, Blocks::CompareLevel); return i;}
Blocks::Level* Blocks::findLevel (Int y, Int  from)  {for(; from<_levels.elms(); from++){Level &l=_levels[from]; if(l.y==y)return &l; if(l.y>y)break;} return null;}
Blocks::Level* Blocks::findLevel (Int y           )  {    return _levels.binaryFind  (y,    Blocks::CompareLevel);}
Blocks::Level& Blocks:: getLevel (Int y           )  {Int i; if(!_levels.binarySearch(y, i, Blocks::CompareLevel))_levels.NewAt(i).init(resolution(), y); return _levels[i];}
Blocks::Level* Blocks::  toLevel (Int y, Int &last)
{
   if(_levels.elms())
   {
      Clamp(last, 0, _levels.elms()-1);
      Level &level=_levels[last]; Int dir=Sign(y-level.y); if(!dir)return &level;
      for(last+=dir; InRange(last, _levels); )
      {
         Level &level=_levels[last]; Int new_dir=Sign(y-level.y); if(!new_dir)return &level;
         if(dir!=new_dir)break; // if went too far and didn't find along the way
      }
   }
   return null;
}
/******************************************************************************/
UInt Blocks::occlusion(C BlocksOcclusion::Node &node, C VecI &pos, AXIS_TYPE axis, Int min_level_i, C Neighbors &neighbors)C
{
   VecI p=pos+node.pos; if(subDivision()>1)
   {
      p.x=DivFloor(p.x, subDivision());
      p.y=DivFloor(p.y, subDivision());
      p.z=DivFloor(p.z, subDivision());
   }
   if(InRange(p.x, resolution())
   && InRange(p.z, resolution())) // center
   {
      if(C Level *level=findLevel(p.y, min_level_i))if(level->map(p.x, p.z, resolution()))return node.fraction_axis[axis]; // if(hasBlock(p, min_level_i))
   }else
   if(p.x<0) // left
   {
      if(p.z<0) // back
      {
         if(neighbors.lb && neighbors.lb->hasBlock(p+VecI(resolution(), 0,  resolution())))return node.fraction_axis[axis];
      }else
      if(p.z<resolution()) // middle
      {
         if(neighbors.l  && neighbors.l ->hasBlock(p+VecI(resolution(), 0,             0)))return node.fraction_axis[axis];
      }else // forward
      {
         if(neighbors.lf && neighbors.lf->hasBlock(p+VecI(resolution(), 0, -resolution())))return node.fraction_axis[axis];
      }
   }else
   if(p.x<resolution()) // middle
   {
      if(p.z<0) // back
      {
         if(neighbors.b && neighbors.b->hasBlock(p+VecI(0, 0,  resolution())))return node.fraction_axis[axis];
      }else // forward
      {
         if(neighbors.f && neighbors.f->hasBlock(p+VecI(0, 0, -resolution())))return node.fraction_axis[axis];
      }
   }else // right
   {
      if(p.z<0) // back
      {
         if(neighbors.rb && neighbors.rb->hasBlock(p+VecI(-resolution(), 0,  resolution())))return node.fraction_axis[axis];
      }else
      if(p.z<resolution()) // middle
      {
         if(neighbors.r  && neighbors.r ->hasBlock(p+VecI(-resolution(), 0,             0)))return node.fraction_axis[axis];
      }else // forward
      {
         if(neighbors.rf && neighbors.rf->hasBlock(p+VecI(-resolution(), 0, -resolution())))return node.fraction_axis[axis];
      }
   }
   UInt   occl=0; REPA(node.nodes)occl+=occlusion(node.nodes[i], pos, axis, min_level_i, neighbors);
   return occl;
}
Byte Blocks::brightness(C BlocksOcclusion *occl, Int x, Int y, Int z, AXIS_TYPE axis, UInt dir_flag, Int min_level_i, C Neighbors &neighbors)C
{
   if(occl)
   {
      UInt o=0;
      VecI pos(x, y, z);
      REPA(occl->_nodes)
      {
       C BlocksOcclusion::Node &node=occl->_nodes[i];
         if(node.dir&dir_flag) // this node is on the positive side of the AO hemisphere
            o+=occlusion(node, pos, axis, min_level_i, neighbors);
      }
      return 255-Min(o>>16, 255);
   }
   return 255;
}
/******************************************************************************/
static void AdjustOrder(Blocks::Part &part) // depending on color difference, rotate quad order, so 2 triangles will have more smooth lighting
{
   Int i=part.vtxs.elms()-4;
 C Blocks::Part::Vtx &a=part.vtxs[i], &b=part.vtxs[i+1], &c=part.vtxs[i+2], &d=part.vtxs[i+3];
   if(ColorDiffSum(a.col, c.col)<ColorDiffSum(b.col, d.col))part.quads.last().rotateOrder();
}
void Blocks::buildLevel(Flt tex_scale, MemPtrN<Part, 256> parts, Int min_x, Int min_z, Int max_x, Int max_z, Int level_i, C Neighbors &neighbors, LevelBrightness (&lb)[2])
{
   Level &level=_levels     [level_i  ]; Int y=level.y;
 C Level *d_lvl=_levels.addr(level_i-1); if(d_lvl && d_lvl->y!=y-1)d_lvl=null;
 C Level *u_lvl=_levels.addr(level_i+1); if(u_lvl && u_lvl->y!=y+1)u_lvl=null;
 C Level *l_lvl=(neighbors.l ? neighbors.l->findLevel(y) : null),
         *r_lvl=(neighbors.r ? neighbors.r->findLevel(y) : null),
         *b_lvl=(neighbors.b ? neighbors.b->findLevel(y) : null),
         *f_lvl=(neighbors.f ? neighbors.f->findLevel(y) : null);

   Flt   sd=1.0f/subDivision();
   FREPD(sy,     subDivision())
   {
      Int Y=y*subDivision()+sy, Y1=Y+1;
      Flt fy=Y*sd, fy1=Y1*sd;

      // swap 'lb_low' and 'lb_high' with each step to reuse their values
      LevelBrightness &lb_low =lb[Y &1].setLevelI(level_i),
                      &lb_high=lb[Y1&1].setLevelI(level_i+(sy==subDivision()-1)); // increase min level only if we're at the last step

      for(Int z=min_z; z<=max_z; z++)
      for(Int x=min_x; x<=max_x; x++)if(Byte b=level.map(x, z, resolution()))if(InRange(b, _mtrl_combos))
      {
         MtrlCombo &mc    =_mtrl_combos[b]; // 'parts[i]' are mapped to 'mesh.parts[i]' and 'materials[i+1]'
         Part      &top   =parts(mc.top   -1),
                   &side  =parts(mc.side  -1),
                   &bottom=parts(mc.bottom-1);
         Int L=x*subDivision(), R=L+subDivision();
         Int B=z*subDivision(), F=B+subDivision();

         // left
         if((x<=0) ? (l_lvl ? !l_lvl->map(resolution()-1, z, resolution()) : true) : !level.map(x-1, z, resolution()))
         {
            if(subDivision()==1)
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               side.vtxs.New().set(Vec(x, y+1, z+1), Vec2(-z-1, -y-1)*tex_scale, lb_high.brightness(x, y+1, z+1, DIR_LEFT)); // lu
               side.vtxs.New().set(Vec(x, y+1, z  ), Vec2(-z  , -y-1)*tex_scale, lb_high.brightness(x, y+1, z  , DIR_LEFT)); // ru
               side.vtxs.New().set(Vec(x, y  , z  ), Vec2(-z  , -y  )*tex_scale, lb_low .brightness(x, y  , z  , DIR_LEFT)); // rd
               side.vtxs.New().set(Vec(x, y  , z+1), Vec2(-z-1, -y  )*tex_scale, lb_low .brightness(x, y  , z+1, DIR_LEFT)); // ld
               AdjustOrder(side);
            }else
            REPD(sz, subDivision())
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               Int Z=z*subDivision()+sz, Z1=Z+1; Flt fz=Z*sd, fz1=fz+sd;
               side.vtxs.New().set(Vec(x, fy1, fz1), Vec2(-fz1, -fy1)*tex_scale, lb_high.brightness(L, Y1, Z1, DIR_LEFT)); // lu
               side.vtxs.New().set(Vec(x, fy1, fz ), Vec2(-fz , -fy1)*tex_scale, lb_high.brightness(L, Y1, Z , DIR_LEFT)); // ru
               side.vtxs.New().set(Vec(x, fy , fz ), Vec2(-fz , -fy )*tex_scale, lb_low .brightness(L, Y , Z , DIR_LEFT)); // rd
               side.vtxs.New().set(Vec(x, fy , fz1), Vec2(-fz1, -fy )*tex_scale, lb_low .brightness(L, Y , Z1, DIR_LEFT)); // ld
               AdjustOrder(side);
            }
         }

         // right
         if((x>=resolution()-1) ? (r_lvl ? !r_lvl->map(0, z, resolution()) : true) : !level.map(x+1, z, resolution()))
         {
            if(subDivision()==1)
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               side.vtxs.New().set(Vec(x+1, y+1, z  ), Vec2(z  , -y-1)*tex_scale, lb_high.brightness(x+1, y+1, z  , DIR_RIGHT)); // lu
               side.vtxs.New().set(Vec(x+1, y+1, z+1), Vec2(z+1, -y-1)*tex_scale, lb_high.brightness(x+1, y+1, z+1, DIR_RIGHT)); // ru
               side.vtxs.New().set(Vec(x+1, y  , z+1), Vec2(z+1, -y  )*tex_scale, lb_low .brightness(x+1, y  , z+1, DIR_RIGHT)); // rd
               side.vtxs.New().set(Vec(x+1, y  , z  ), Vec2(z  , -y  )*tex_scale, lb_low .brightness(x+1, y  , z  , DIR_RIGHT)); // ld
               AdjustOrder(side);
            }else
            REPD(sz, subDivision())
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               Int Z=z*subDivision()+sz, Z1=Z+1; Flt fz=Z*sd, fz1=fz+sd;
               side.vtxs.New().set(Vec(x+1, fy1, fz ), Vec2(fz , -fy1)*tex_scale, lb_high.brightness(R, Y1, Z , DIR_RIGHT)); // lu
               side.vtxs.New().set(Vec(x+1, fy1, fz1), Vec2(fz1, -fy1)*tex_scale, lb_high.brightness(R, Y1, Z1, DIR_RIGHT)); // ru
               side.vtxs.New().set(Vec(x+1, fy , fz1), Vec2(fz1, -fy )*tex_scale, lb_low .brightness(R, Y , Z1, DIR_RIGHT)); // rd
               side.vtxs.New().set(Vec(x+1, fy , fz ), Vec2(fz , -fy )*tex_scale, lb_low .brightness(R, Y , Z , DIR_RIGHT)); // ld
               AdjustOrder(side);
            }
         }

         // back
         if((z<=0) ? (b_lvl ? !b_lvl->map(x, resolution()-1, resolution()) : true) : !level.map(x, z-1, resolution()))
         {
            if(subDivision()==1)
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               side.vtxs.New().set(Vec(x  , y+1, z), Vec2(x  , -y-1)*tex_scale, lb_high.brightness(x  , y+1, z, DIR_BACK)); // lu
               side.vtxs.New().set(Vec(x+1, y+1, z), Vec2(x+1, -y-1)*tex_scale, lb_high.brightness(x+1, y+1, z, DIR_BACK)); // ru
               side.vtxs.New().set(Vec(x+1, y  , z), Vec2(x+1, -y  )*tex_scale, lb_low .brightness(x+1, y  , z, DIR_BACK)); // rd
               side.vtxs.New().set(Vec(x  , y  , z), Vec2(x  , -y  )*tex_scale, lb_low .brightness(x  , y  , z, DIR_BACK)); // ld
               AdjustOrder(side);
            }else
            REPD(sx, subDivision())
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               Int X=x*subDivision()+sx, X1=X+1; Flt fx=X*sd, fx1=fx+sd;
               side.vtxs.New().set(Vec(fx , fy1, z), Vec2(fx , -fy1)*tex_scale, lb_high.brightness(X , Y1, B, DIR_BACK)); // lu
               side.vtxs.New().set(Vec(fx1, fy1, z), Vec2(fx1, -fy1)*tex_scale, lb_high.brightness(X1, Y1, B, DIR_BACK)); // ru
               side.vtxs.New().set(Vec(fx1, fy , z), Vec2(fx1, -fy )*tex_scale, lb_low .brightness(X1, Y , B, DIR_BACK)); // rd
               side.vtxs.New().set(Vec(fx , fy , z), Vec2(fx , -fy )*tex_scale, lb_low .brightness(X , Y , B, DIR_BACK)); // ld
               AdjustOrder(side);
            }
         }

         // forward
         if((z>=resolution()-1) ? (f_lvl ? !f_lvl->map(x, 0, resolution()) : true) : !level.map(x, z+1, resolution()))
         {
            if(subDivision()==1)
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               side.vtxs.New().set(Vec(x+1, y+1, z+1), Vec2(-x-1, -y-1)*tex_scale, lb_high.brightness(x+1, y+1, z+1, DIR_FORWARD)); // lu
               side.vtxs.New().set(Vec(x  , y+1, z+1), Vec2(-x  , -y-1)*tex_scale, lb_high.brightness(x  , y+1, z+1, DIR_FORWARD)); // ru
               side.vtxs.New().set(Vec(x  , y  , z+1), Vec2(-x  , -y  )*tex_scale, lb_low .brightness(x  , y  , z+1, DIR_FORWARD)); // rd
               side.vtxs.New().set(Vec(x+1, y  , z+1), Vec2(-x-1, -y  )*tex_scale, lb_low .brightness(x+1, y  , z+1, DIR_FORWARD)); // ld
               AdjustOrder(side);
            }else
            REPD(sx, subDivision())
            {
               Int v=side.vtxs.elms(); side.quads.New().set(v, v+1, v+2, v+3);
               Int X=x*subDivision()+sx, X1=X+1; Flt fx=X*sd, fx1=fx+sd;
               side.vtxs.New().set(Vec(fx1, fy1, z+1), Vec2(-fx1, -fy1)*tex_scale, lb_high.brightness(X1, Y1, F, DIR_FORWARD)); // lu
               side.vtxs.New().set(Vec(fx , fy1, z+1), Vec2(-fx , -fy1)*tex_scale, lb_high.brightness(X , Y1, F, DIR_FORWARD)); // ru
               side.vtxs.New().set(Vec(fx , fy , z+1), Vec2(-fx , -fy )*tex_scale, lb_low .brightness(X , Y , F, DIR_FORWARD)); // rd
               side.vtxs.New().set(Vec(fx1, fy , z+1), Vec2(-fx1, -fy )*tex_scale, lb_low .brightness(X1, Y , F, DIR_FORWARD)); // ld
               AdjustOrder(side);
            }
         }

         //  down
         if(sy==0)
            if(!d_lvl || !d_lvl->map(x, z, resolution()))
         {
            if(subDivision()==1)
            {
               Int v=bottom.vtxs.elms(); bottom.quads.New().set(v, v+1, v+2, v+3);
               bottom.vtxs.New().set(Vec(x  , y  , z  ), Vec2(x  , z  )*tex_scale, lb_low.brightness(x  , y, z  , DIR_DOWN)); // lu
               bottom.vtxs.New().set(Vec(x+1, y  , z  ), Vec2(x+1, z  )*tex_scale, lb_low.brightness(x+1, y, z  , DIR_DOWN)); // ru
               bottom.vtxs.New().set(Vec(x+1, y  , z+1), Vec2(x+1, z+1)*tex_scale, lb_low.brightness(x+1, y, z+1, DIR_DOWN)); // rd
               bottom.vtxs.New().set(Vec(x  , y  , z+1), Vec2(x  , z+1)*tex_scale, lb_low.brightness(x  , y, z+1, DIR_DOWN)); // ld
               AdjustOrder(bottom);
            }else
            REPD(sx, subDivision()){Int X=x*subDivision()+sx; Flt fx=X*sd, fx1=fx+sd;
            REPD(sz, subDivision())
            {
               Int v=bottom.vtxs.elms(); bottom.quads.New().set(v, v+1, v+2, v+3);
               Int Z=z*subDivision()+sz; Flt fz=Z*sd, fz1=fz+sd;
               bottom.vtxs.New().set(Vec(fx , y  , fz ), Vec2(fx , fz )*tex_scale, lb_low.brightness(X  , Y, Z  , DIR_DOWN)); // lu
               bottom.vtxs.New().set(Vec(fx1, y  , fz ), Vec2(fx1, fz )*tex_scale, lb_low.brightness(X+1, Y, Z  , DIR_DOWN)); // ru
               bottom.vtxs.New().set(Vec(fx1, y  , fz1), Vec2(fx1, fz1)*tex_scale, lb_low.brightness(X+1, Y, Z+1, DIR_DOWN)); // rd
               bottom.vtxs.New().set(Vec(fx , y  , fz1), Vec2(fx , fz1)*tex_scale, lb_low.brightness(X  , Y, Z+1, DIR_DOWN)); // ld
               AdjustOrder(bottom);
            }}
         }

         //  up
         if(sy==subDivision()-1)
            if(!u_lvl || !u_lvl->map(x, z, resolution()))
         {
            if(subDivision()==1)
            {
               Int v=top.vtxs.elms(); top.quads.New().set(v, v+1, v+2, v+3);
               top.vtxs.New().set(Vec(x  , y+1, z+1), Vec2(x  , -z-1)*tex_scale, lb_high.brightness(x  , y+1, z+1, DIR_UP)); // lu
               top.vtxs.New().set(Vec(x+1, y+1, z+1), Vec2(x+1, -z-1)*tex_scale, lb_high.brightness(x+1, y+1, z+1, DIR_UP)); // ru
               top.vtxs.New().set(Vec(x+1, y+1, z  ), Vec2(x+1, -z  )*tex_scale, lb_high.brightness(x+1, y+1, z  , DIR_UP)); // rd
               top.vtxs.New().set(Vec(x  , y+1, z  ), Vec2(x  , -z  )*tex_scale, lb_high.brightness(x  , y+1, z  , DIR_UP)); // ld
               AdjustOrder(top);
            }else
            REPD(sx, subDivision()){Int X=x*subDivision()+sx; Flt fx=X*sd, fx1=fx+sd;
            REPD(sz, subDivision())
            {
               Int v=top.vtxs.elms(); top.quads.New().set(v, v+1, v+2, v+3);
               Int Z=z*subDivision()+sz; Flt fz=Z*sd, fz1=fz+sd;
               top.vtxs.New().set(Vec(fx , y+1, fz1), Vec2(fx , -fz1)*tex_scale, lb_high.brightness(X  , Y1, Z+1, DIR_UP)); // lu
               top.vtxs.New().set(Vec(fx1, y+1, fz1), Vec2(fx1, -fz1)*tex_scale, lb_high.brightness(X+1, Y1, Z+1, DIR_UP)); // ru
               top.vtxs.New().set(Vec(fx1, y+1, fz ), Vec2(fx1, -fz )*tex_scale, lb_high.brightness(X+1, Y1, Z  , DIR_UP)); // rd
               top.vtxs.New().set(Vec(fx , y+1, fz ), Vec2(fx , -fz )*tex_scale, lb_high.brightness(X  , Y1, Z  , DIR_UP)); // ld
               AdjustOrder(top);
            }}
         }
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
