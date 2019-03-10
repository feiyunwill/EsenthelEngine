/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   TODO: investigate 'initSlicedFindPath' function?
   TODO: should we switch to DT_POLYREF64?

   Recast PathMesh binary data has 'userId' member
      which is set to memory address of '_PathMesh' object owning that data.

   PathMesh has 2 data:
               'data' recast data without any obstacles
      'obstacle_data' recast data without     obstacles

   MaxTiles+MaxPolys can use up to 22 bits, salt uses 10 bits, total=32 bits, however after modification of Recast sources, salt uses 3 bits

   PM_OBSTACLE will not create triangles at all
   PM_BLOCK    will     create triangles but with type that is not accessible

/******************************************************************************/
ASSERT(PM_OBSTACLE==RC_NULL_AREA
    && PM_BLOCKED ==RC_WALKABLE_AREA);
/******************************************************************************

   Sample compression results:
              chf           =105127
       RLE    chf_compressed= 77362
       LZ4-16 chf_compressed= 32275
      ZLIB-9  chf_compressed= 16624
      ZSTD-22 chf_compressed= 13848
      LZMA-3  chf_compressed= 10999
      LZMA-4  chf_compressed= 10999
      LZMA-5  chf_compressed=  9300
      LZMA-6  chf_compressed=  9300
      LZMA-9  chf_compressed=  9200

/******************************************************************************/
static const COMPRESS_TYPE CompressType =COMPRESS_LZMA;
static const Int           CompressLevel=5;

const UInt PMF_ALL=PMF_WALK|PMF_SWIM;
const Bool MonotonePartitioning=false, // monotone enabled offers worse region building (partitioning) but faster building time (0.28 seconds vs 2.72 seconds on average world 11x11 areas), keep this disabled because when enabled generates considerably worse results
           BuildDetailMesh     =false; // used only for rendering? building takes too much time, so skip it

const Int  MaxTiles            =512*512, // 18-bits
           MaxPolys            = 2048  , // 11-bits (1024 was tested and needed for some maps, 2048 is just for safety)
           MaxSearchNodes      = 4096  , // 1..65536
           MaxVtxsPerPoly      =    6;
const Flt  EdgeMaxLen          =12.0f,
           EdgeMaxError        = 1.3f, // 0.7
           RegionMinSize       = 8.0f,
           RegionMergeSize     =20.0f,
           DetailSampleDist    = 6.0f,
           DetailSampleMaxError= 1.0f;
/******************************************************************************/
// HELPERS
/******************************************************************************/
struct RecastConfig : rcConfig
{
   RecastConfig(Flt cell_size, Flt cell_height, Flt ctrl_r, Flt ctrl_h, Flt max_climb)
   {
      Zero(T);

      cs=cell_size;
      ch=cell_height;

      walkableRadius=Ceil (ctrl_r    /cs);
      walkableHeight=Ceil (ctrl_h    /ch);
      walkableClimb =Floor( max_climb/ch);

      borderSize            =walkableRadius+3; // Reserve enough padding
      maxVertsPerPoly       =Mid(MaxVtxsPerPoly, 3, DT_VERTS_PER_POLYGON);
      maxSimplificationError=EdgeMaxError;
      maxEdgeLen            =Trunc(EdgeMaxLen/cs);
        minRegionArea       =Trunc(Sqr(RegionMinSize  )); // area=size*size
      mergeRegionArea       =Trunc(Sqr(RegionMergeSize)); // area=size*size
      detailSampleDist      =(DetailSampleDist<0.9f ? 0 : cs*DetailSampleDist);
      detailSampleMaxError  =                             ch*DetailSampleMaxError;
   }
};
/******************************************************************************/
struct RecastHeightfield
{
   operator rcHeightfield&            () {return *_;}
            rcHeightfield* operator-> () {return  _;}

   void drawVoxel(float minx, float miny, float minz, float maxx, float maxy, float maxz)
   {
      Box(Vec(minx, miny, minz), Vec(maxx, maxy, maxz)).draw(Color(255, 255, 0, 128), true);
   }
   void drawVoxels()
   {
      SetMatrix();

      const float* orig=T->bmin;
      const float  cs=T->cs;
      const float  ch=T->ch;

      const int w=T->width;
      const int h=T->height;

      REPD(y, h)
      REPD(x, w)
      {
	      float  fx=orig[0]+x*cs;
	      float  fz=orig[2]+y*cs;
	    C rcSpan *s=T->spans[x+y*w];
	      for(; s; s=s->next)drawVoxel(fx, orig[1]+s->smin*ch, fz, fx+cs, orig[1]+s->smax*ch, fz+cs);
      }
   }

  ~RecastHeightfield() {   rcFreeHeightField(_); _=null;}
   RecastHeightfield() {_=rcAllocHeightfield(); if(!_)Exit("Out of memory");}

private:
	rcHeightfield *_;
};
/******************************************************************************/
struct RecastContourSet
{
   operator rcContourSet&            () {return *_;}
            rcContourSet* operator-> () {return  _;}

  ~RecastContourSet() {   rcFreeContourSet(_); _=null;}
   RecastContourSet() {_=rcAllocContourSet(); if(!_)Exit("Out of memory");}

private:
	rcContourSet *_;
};
/******************************************************************************/
struct RecastPolyMesh
{
   operator rcPolyMesh&            () {return *_;}
            rcPolyMesh* operator-> () {return  _;}

  ~RecastPolyMesh() {   rcFreePolyMesh(_); _=null;}
   RecastPolyMesh() {_=rcAllocPolyMesh(); if(!_)Exit("Out of memory");}

private:
	rcPolyMesh *_;
};
/******************************************************************************/
struct RecastPolyMeshDetail
{
   operator rcPolyMeshDetail&            () {return *_;}
            rcPolyMeshDetail* operator-> () {return  _;}

  ~RecastPolyMeshDetail() {   rcFreePolyMeshDetail(_); _=null;}
   RecastPolyMeshDetail() {_=rcAllocPolyMeshDetail(); if(!_)Exit("Out of memory");}

private:
	rcPolyMeshDetail *_;
};
/******************************************************************************/
void RecastCompactHeightfield::operator=(C RecastCompactHeightfield &src)
{
   if(this!=&src)
   {
      del();
      Copy(T, src);
      cells=(rcCompactCell *)rcAlloc(SIZE(rcCompactCell )*width*height, RC_ALLOC_PERM); CopyN(cells, src.cells, width*height);
	   spans=(rcCompactSpan *)rcAlloc(SIZE(rcCompactSpan )*spanCount   , RC_ALLOC_PERM); CopyN(spans, src.spans, spanCount   );
	   dist =(unsigned short*)rcAlloc(SIZE(unsigned short)*spanCount   , RC_ALLOC_PERM); CopyN(dist , src.dist , spanCount   );
	   areas=(unsigned char *)rcAlloc(SIZE(unsigned char )*spanCount   , RC_ALLOC_PERM); CopyN(areas, src.areas, spanCount   );
   }
}
/******************************************************************************/
void RecastCompactHeightfield::del()
{
   rcFree(cells);
   rcFree(spans);
   rcFree(areas);
   rcFree(dist );
   zero();
}
/******************************************************************************/
Bool RecastCompactHeightfield::is()C
{
   return width || height || spanCount;
}
/******************************************************************************/
void RecastCompactHeightfield::clean() // this removes all spans of RC_NULL_AREA
{
   // count 'cleaned' and set 'old_to_new' remap
   Memt<Int> old_to_new; old_to_new.setNum(spanCount);
   Int       cleaned=0;
   FREP(spanCount)
   {
      if(areas[i]==RC_NULL_AREA)old_to_new[i]=-1;
      else                      old_to_new[i]=cleaned++;
   }

   // clean
   if(cleaned!=spanCount)
   {
      // set Span to XY
      Memt<VecI2> span_xy; span_xy.setNum(spanCount);
      REPD(y, height)
      REPD(x, width )
      {
         rcCompactCell &cell=cells[x+y*width];
         REP(cell.count)span_xy[cell.index+i].set(x, y);
      }

      // spans + area id (before cells)
      rcCompactSpan *new_spans=(rcCompactSpan*)rcAlloc(SIZE(*new_spans)*cleaned, RC_ALLOC_PERM);
      unsigned char *new_areas=(unsigned char*)rcAlloc(SIZE(*new_areas)*cleaned, RC_ALLOC_PERM);
      FREPA(old_to_new)
      {
         Int n =old_to_new[i];
         if( n>=0)
         {
          C VecI2         &s_xy    =span_xy  [i];
            rcCompactSpan &new_span=new_spans[n];
            new_span    =spans[i];
            new_areas[n]=areas[i];
            REPD(dir, 4)
            {
               Int connection =rcGetCon(new_span, dir);
               if( connection!=RC_NOT_CONNECTED)
               {
                  Int new_connection=RC_NOT_CONNECTED,
                      nx=s_xy.x+rcGetDirOffsetX(dir),
                      ny=s_xy.y+rcGetDirOffsetY(dir);
                  if(InRange(nx, width )
                  && InRange(ny, height))
                  {
                     Int            valid      =0;
                     rcCompactCell &neighb_cell=cells[nx+ny*width];
                     FREP(neighb_cell.count)
                     {
                        Int           span_index=neighb_cell.index+i;
                        if(old_to_new[span_index]>=0)
                        {
						         // code taken from Recast 'rcBuildCompactHeightfield'
						         rcCompactSpan &test_span=spans[span_index];
						         Int bot=Max(new_span.y           , test_span.y            ),
						             top=Min(new_span.y+new_span.h, test_span.y+test_span.h);
						         if(top-bot>=walkableHeight && Abs(Int(test_span.y)-Int(new_span.y))<=walkableClimb) // Check that the gap between the spans is walkable, and that the climb height between the gaps is not too high.
						         {
						            if(InRange(valid, RC_NOT_CONNECTED))new_connection=valid;
							         break;
						         }
						         valid++;
						      }
					      }
                  }
                  rcSetCon(new_span, dir, new_connection);
               }
            }
         }
      }
      rcFree(spans); spans=new_spans;
      rcFree(areas); areas=new_areas;

      // adjust cells (after spans)
      REP(width*height)
      {
         rcCompactCell &cell=cells[i];
         Int new_index=0, new_count=0;
         FREP(cell.count)
         {
            Int n =old_to_new[cell.index+i];
            if( n>=0)
            {
               if(!new_count)new_index=n;
                   new_count++;
            }
         }
         cell.index=new_index;
         cell.count=new_count;
      }

      // dist
      if(dist)
      {
         unsigned short *new_dist=(unsigned short*)rcAlloc(SIZE(*new_dist)*cleaned, RC_ALLOC_PERM);
         FREPA(old_to_new)
         {
            Int n =old_to_new[i];
            if( n>=0)new_dist[n]=dist[i];
         }
         rcFree(dist); dist=new_dist;
      }

      // count
      spanCount=cleaned;
   }
}
/******************************************************************************/
Bool RecastCompactHeightfield::save(File &f)C
{
   f.cmpUIntV(0); // version
   f.cmpUIntV(width).cmpUIntV(height).cmpUIntV(spanCount).cmpUIntV(walkableHeight).cmpUIntV(walkableClimb).cmpUIntV(borderSize).cmpUIntV(maxDistance).cmpUIntV(maxRegions);
   f<<bmin<<bmax<<cs<<ch;
   f.putN(cells, width*height);
   f.putN(spans, spanCount   );
   f.putN(areas, spanCount   );
   f.putBool(dist!=null); if(dist)f.putN(dist, spanCount);
   return f.ok();
}
/******************************************************************************/
Bool RecastCompactHeightfield::load(File &f)
{
   del();

   switch(f.decUIntV())
   {
      case 0:
      {
	      width=f.decUIntV(); height=f.decUIntV(); spanCount=f.decUIntV(); walkableHeight=f.decUIntV(); walkableClimb=f.decUIntV(); borderSize=f.decUIntV(); maxDistance=f.decUIntV(); maxRegions=f.decUIntV();
         f>>bmin>>bmax>>cs>>ch;
                         cells=(rcCompactCell *)rcAlloc(SIZE(*cells)*width*height, RC_ALLOC_PERM); f.getN(cells, width*height);
                         spans=(rcCompactSpan *)rcAlloc(SIZE(*spans)*spanCount   , RC_ALLOC_PERM); f.getN(spans, spanCount   );
                         areas=(unsigned char *)rcAlloc(SIZE(*areas)*spanCount   , RC_ALLOC_PERM); f.getN(areas, spanCount   );
         if(f.getBool()){dist =(unsigned short*)rcAlloc(SIZE(*dist )*spanCount   , RC_ALLOC_PERM); f.getN(dist , spanCount   );}
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
static void ExtendPoly2D(Memt<Vec2> &poly, Flt r, Bool add_points)
{
   if(r && poly.elms()>=3)
   {
      if(add_points)
      {
         Memt<Vec2> temp; temp.setNum(poly.elms()*2);
         Vec2 &prev=poly.last(), &cur=poly[0], prev_nrm=PerpN(prev-cur);
         FREPA(poly)
         {
            Vec2 &cur=poly[i], &next=poly[(i+1)%poly.elms()], next_nrm=PerpN(cur-next);
            temp[i*2+0]=cur+prev_nrm*r;
            temp[i*2+1]=cur+next_nrm*r;
            prev_nrm=next_nrm;
         }
         Swap(poly, temp);
      }else
      {
         Vec2 &prev=poly.last(), cur_0=poly[0], prev_nrm=PerpN(prev-cur_0); // cur_0 is 'cur' and backup of poly[0]
         FREPA(poly)
         {
            Int   next_index=(i+1)%poly.elms();
            Vec2 &cur=poly[i], &next=(next_index ? poly[next_index] : cur_0), next_nrm=PerpN(cur-next), avg_nrm=prev_nrm+next_nrm;
            avg_nrm.setLength(r);
            cur+=avg_nrm;
            prev_nrm=next_nrm;
         }
      }
   }
}
static void MarkConvexArea(rcContext &ctx, Vec *point, Int points, Flt ctrl_r, Flt ctrl_h, PATH_MESH_TYPE type, rcCompactHeightfield &chf)
{
   if(point && points>=3)
   {
      Vec min, max; MinMax(point, points, min, max);
      Memt<Vec2> poly; CreateConvex2Dxz(poly, point, points);
      if(poly.elms()>=3)
      {
         ExtendPoly2D(poly, ctrl_r, true);

         // rcMarkConvexPolyArea expects 'XYZ' vec, but we have 'Vec2', so convert it
         Int elms=poly.elms(); // first remember number of vtxs
         poly.addNum((elms+1)/2); // we need 'elms' more floats, with each new element we get 2 floats (because this is Vec2 container), so divide by 2 with ceil
         Vec2 *vec2=poly.data();
         Vec  *vec =(Vec*)vec2; // now we can cast to Vec
         REP(elms)vec[i].set(vec2[i].x, 0, vec2[i].y); // it's important to go from the end to don't overwrite existing elements

         rcMarkConvexPolyArea(&ctx, vec->c, elms, min.y-ctrl_h, max.y, type, chf); // this function ignores 'poly.y' and uses only (hmin..hmax)
      }
   }
}
// !! Warning: 'chf' will get modified !!
static Bool Build(RecastCompactHeightfield &chf, Shape *shape, Int shapes, Flt ctrl_r, Flt ctrl_h, Flt max_climb, C VecI2 &xy, Ptr user, Mems<Byte> &data)
{
   {
      RecastConfig cfg(chf.cs, chf.ch, ctrl_r, ctrl_h, max_climb);
      rcContext    ctx(false);

      // Mark areas.
      REP(shapes)
      {
       C Shape &s=shape[i];
         switch(s.type)
         {
            // 'rcMarkCylinderArea' accepts bottom position (height parameter extends up only)
            // don't use PM_OBSTACLE because that could result in empty mesh (no data), it would fail to set that mesh into recast world, let's use PM_BLOCKED instead which results in creating the mesh but with polygons that are not crossable
            case SHAPE_POINT  : {Box b=s.point; b.extendX(ctrl_r).extendZ(ctrl_r).extendY(cfg.ch).min.y-=ctrl_h;                rcMarkBoxArea(&ctx, b.min.c, b.max.c, PM_BLOCKED, chf);} break;
            case SHAPE_BOX    : {Box b=s.box  ; b.extendX(ctrl_r).extendZ(ctrl_r)                .min.y-=ctrl_h;                rcMarkBoxArea(&ctx, b.min.c, b.max.c, PM_BLOCKED, chf);} break;
            case SHAPE_BALL   : {Flt h=s.ball   .r*2; rcMarkCylinderArea(&ctx, (s.ball   .pos-Vec(0, h*0.5f+ctrl_h, 0)).c, s.ball   .r+ctrl_r+cfg.cs*0.50f, h+ctrl_h, PM_BLOCKED, chf);} break;
            case SHAPE_CAPSULE: {Flt h=s.capsule.h  ; rcMarkCylinderArea(&ctx, (s.capsule.pos-Vec(0, h*0.5f+ctrl_h, 0)).c, s.capsule.r+ctrl_r+cfg.cs*0.50f, h+ctrl_h, PM_BLOCKED, chf);} break;
            case SHAPE_TUBE   : {Flt h=s.tube   .h  ; rcMarkCylinderArea(&ctx, (s.tube   .pos-Vec(0, h*0.5f+ctrl_h, 0)).c, s.tube   .r+ctrl_r+cfg.cs*0.50f, h+ctrl_h, PM_BLOCKED, chf);} break;
            case SHAPE_OBOX   : {Vec points[8]; s.obox   .toCorners(points);                 MarkConvexArea(ctx, points, Elms(points), ctrl_r+cfg.cs*0.25f,   ctrl_h, PM_BLOCKED, chf);} break;
            case SHAPE_PYRAMID: {Vec points[5]; s.pyramid.toCorners(points);                 MarkConvexArea(ctx, points, Elms(points), ctrl_r+cfg.cs*0.25f,   ctrl_h, PM_BLOCKED, chf);} break;
            case SHAPE_EDGE   :
            {
               Flt r=ctrl_r+cfg.cs*0.25f;
               Vec z=s.edge.delta(); z.y=0; if(!z.setLength(r))z.set(0, 0, r);
               Vec x=Perp(z.xz()).x0y();
               Vec points[4]=
               {
                  s.edge.p[0]-z-x,
                  s.edge.p[0]-z+x,
                  s.edge.p[1]+z-x,
                  s.edge.p[1]+z+x,
               };
               MinMax(s.edge.p[0].y, s.edge.p[1].y, points[0].y, points[1].y); points[0].y-=cfg.ch; points[1].y+=cfg.ch; // extend Y by cell height
               MarkConvexArea(ctx, points, Elms(points), 0, ctrl_h, PM_BLOCKED, chf);
            }break;
         }
      }

      if(MonotonePartitioning)
      {
	      // Partition the walkable surface into simple regions without holes.
	      if(!rcBuildRegionsMonotone(&ctx, chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))goto error;
      }else
      {
	      // Prepare for region partitioning, by calculating distance field along the walkable surface.
	      if(!rcBuildDistanceField(&ctx, chf))goto error;

	      // Partition the walkable surface into simple regions without holes.
	      if(!rcBuildRegions(&ctx, chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))goto error;
      }

      // Create contours.
      RecastContourSet cset;
      if(!rcBuildContours(&ctx, chf, cfg.maxSimplificationError, cfg.maxEdgeLen, cset))goto error;
    //if(cset->nconts==0)return true; // mesh is empty

      // Build polygon navmesh from the contours.
      RecastPolyMesh pmesh;
      if(!rcBuildPolyMesh(&ctx, cset, cfg.maxVertsPerPoly, pmesh))goto error;
	
      // Build detail mesh.
      RecastPolyMeshDetail dmesh;
      if(BuildDetailMesh)
	      if(!rcBuildPolyMeshDetail(&ctx, pmesh, chf, cfg.detailSampleDist, cfg.detailSampleMaxError, dmesh))goto error;

      if(pmesh->nverts>0x10000)goto error; // Recast uses U16 indexes

      // Update poly flags from areas.
      REP(pmesh->npolys)switch(pmesh->areas[i])
      {
         default       : pmesh->flags[i]=0       ; break; // PM_OBSTACLE
         case PM_GROUND: pmesh->flags[i]=PMF_WALK; break;
         case PM_WATER : pmesh->flags[i]=PMF_SWIM; break;
      }

      dtNavMeshCreateParams params; Zero(params);
      params.verts=pmesh->verts;
      params.vertCount=pmesh->nverts;
      params.polys=pmesh->polys;
      params.polyAreas=pmesh->areas;
      params.polyFlags=pmesh->flags;
      params.polyCount=pmesh->npolys;
      params.nvp=pmesh->nvp;
      if(BuildDetailMesh)
      {
         params.detailMeshes=dmesh->meshes;
         params.detailVerts=dmesh->verts;
         params.detailVertsCount=dmesh->nverts;
         params.detailTris=dmesh->tris;
         params.detailTriCount=dmesh->ntris;
      }
    /*params.offMeshConVerts=geom->getOffMeshConnectionVerts();
      params.offMeshConRad=geom->getOffMeshConnectionRads();
      params.offMeshConDir=geom->getOffMeshConnectionDirs();
      params.offMeshConAreas=geom->getOffMeshConnectionAreas();
      params.offMeshConFlags=geom->getOffMeshConnectionFlags();
      params.offMeshConUserID=geom->getOffMeshConnectionId();
      params.offMeshConCount=geom->getOffMeshConnectionCount();*/
      params.walkableRadius=ctrl_r;
      params.walkableHeight=ctrl_h;
      params.walkableClimb =Max(max_climb, cfg.ch); // set minimum value from 'cell height' because sometimes neighbor tiles couldn't connect (due to small changes between their Y positions)
      params.tileX=xy.x;
      params.tileY=xy.y;
      params.tileLayer=0;
      rcVcopy(params.bmin, pmesh->bmin);
      rcVcopy(params.bmax, pmesh->bmax);
      params.cs=cfg.cs;
      params.ch=cfg.ch;
      params.buildBvTree=true;
      params.userId=(UIntPtr)user;

      unsigned char *navData=null;
      int            navDataSize=0;
      if(dtCreateNavMeshData(&params, &navData, &navDataSize))
      {
         data.setNum(navDataSize).copyFrom(navData); dtFree(navData); return true;
      }
   }
error:
   data.del(); return false;
}
/******************************************************************************/
// _PATH MESH
/******************************************************************************/
Bool _PathMesh::is()C {return data.elms()>0;}
/******************************************************************************/
Box _PathMesh::box()
{
   Box box;
   if(C dtMeshHeader *header=(dtMeshHeader*)data.data()) // if mesh is available then use it
   {
      box.min.set(header->bmin[0], header->bmin[1], header->bmin[2]);
      box.max.set(header->bmax[0], header->bmax[1], header->bmax[2]);
   }else
   if(getChf()) // try accessing 'chf'
   {
      box.min.set(chf.bmin[0], chf.bmin[1], chf.bmin[2]);
      box.max.set(chf.bmax[0], chf.bmax[1], chf.bmax[2]);
   }else box.zero();
   return box;
}
/******************************************************************************/
void _PathMesh::link(PathWorld *world)
{
   if(T.world!=world) // if different
   {
      if(T.world) // unlink from current world
      {
         T.world->_set(null, xy, false);
         T.world=null; // clear in case '_set' won't do this for some reason
      }
      if(world)world->_set(this, xy, false); // link to new world, it will handle setting 'T.world' on success
   }
}
void _PathMesh::del()
{
   link(null); xy.zero();
   data.del(); obstacle_data.del(); chf_compressed.del();
   chf .del();
}
Bool _PathMesh::getChf()
{
   if(chf.is())return true; // we already have it
   if(chf_compressed.elms()) // if we have 'chf_compressed'
   {
      File compressed(chf_compressed.data(), chf_compressed.elms()), decompressed; // read from 'chf_compressed'
      if(Decompress(compressed, decompressed, true)) // decompress 'compressed' into 'decompressed'
      {
         chf_compressed.del(); // no longer needed so release it
         decompressed.pos(0); return chf.load(decompressed); // load 'chf' from 'decompressed'
      }
   }
   return false;
}
/******************************************************************************/
static void SetChfCompressed(C RecastCompactHeightfield &chf, Mems<Byte> &chf_compressed)
{
   File decompressed; chf.save(decompressed.writeMem()); // save 'chf' into 'decompressed'
   File   compressed; decompressed.pos(0); if(!Compress(decompressed, compressed.writeMem(), CompressType, CompressLevel, false))Exit("SetChfCompressed"); // compress 'decompressed' into 'compressed'
   chf_compressed.setNum(compressed.size()); compressed.pos(0); compressed.get(chf_compressed.data(), chf_compressed.elms()); // store 'compressed' into 'chf_compressed'
}
void _PathMesh::preSave()
{
   if(is() && !chf_compressed.elms())SetChfCompressed(chf, chf_compressed);
}
Bool _PathMesh::save(File &f)C
{
   f.cmpUIntV(0); // version
   if(data._saveRaw(f))
   {
      if(is())
      {
         Mems<Byte> chf_compressed_temp, *chf_compressed=&ConstCast(T.chf_compressed);
         if(!chf_compressed->elms()){chf_compressed=&chf_compressed_temp; SetChfCompressed(chf, *chf_compressed);} // compress into temp so we won't keep it in the memory afterwards
         if(!chf_compressed->_saveRaw(f))return false;
      }
      return f.ok();
   }
   return false;
}
Bool _PathMesh::load(File &f)
{
   del(); switch(f.decUIntV()) // version
   {
      case 0: if(data._loadRaw(f))
      {
         if(is())if(!chf_compressed._loadRaw(f))goto error;
         if(dtMeshHeader *header=(dtMeshHeader*)data.data())
         {
            header->userId=(UIntPtr)this; // 'userId' points to memory address of '_PathMesh'
            xy.set(header->x, header->y);
         }
         if(f.ok())return true;
      }break;
   }
error:
   del(); return false;
}
/******************************************************************************/
// PATH SETTINGS
/******************************************************************************/
PathSettings& PathSettings::reset()
{
  _area_size=32;
  _ctrl_r   =0.33f;
  _ctrl_h   =2.0f;
  _max_climb=0.7f;
  _max_slope=PI_4;
  _cell_size=1.0f/3;
  _cell_h   =0.1f;
   return T;
}
PathSettings& PathSettings::areaSize  (Flt size ) {_area_size=Max(size ,        EPS); return T;}
PathSettings& PathSettings::ctrlRadius(Flt r    ) {_ctrl_r   =Max(r    ,          0); return T;}
PathSettings& PathSettings::ctrlHeight(Flt h    ) {_ctrl_h   =Max(h    ,          0); return T;}
PathSettings& PathSettings:: maxClimb (Flt climb) {_max_climb=Max(climb,          0); return T;}
PathSettings& PathSettings:: maxSlope (Flt slope) {_max_slope=Mid(slope, 0.0f, PI_2); return T;}
PathSettings& PathSettings::cellSize  (Flt size ) {_cell_size=Max(size ,     0.001f); return T;}
PathSettings& PathSettings::cellHeight(Flt h    ) {_cell_h   =Max(h    ,     0.001f); return T;}

Bool PathSettings::operator!=(C PathSettings &path)C {return !(T==path);}
Bool PathSettings::operator==(C PathSettings &path)C
{
   return Equal(_area_size, path._area_size)
       && Equal(_ctrl_r   , path._ctrl_r   ) && Equal(_ctrl_h   , path._ctrl_h   )
       && Equal(_max_climb, path._max_climb) && Equal(_max_slope, path._max_slope)
       && Equal(_cell_size, path._cell_size) && Equal(_cell_h   , path._cell_h   );
}

Bool PathSettings::save(File &f)C
{
   f.cmpUIntV(0);
   f<<_area_size<<_ctrl_r<<_ctrl_h<<_max_climb<<_max_slope<<_cell_size<<_cell_h;
   return f.ok();
}
Bool PathSettings::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>_area_size>>_ctrl_r>>_ctrl_h>>_max_climb>>_max_slope>>_cell_size>>_cell_h;
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// PATH OBSTACLE
/******************************************************************************/
static Bool ObstacleShapeSupported(SHAPE_TYPE type)
{
   switch(type)
   {
      case SHAPE_POINT  :
      case SHAPE_EDGE   :
      case SHAPE_BOX    :
      case SHAPE_BALL   :
      case SHAPE_CAPSULE:
      case SHAPE_TUBE   :
      case SHAPE_OBOX   :
      case SHAPE_PYRAMID: return true;

      default: return false;
   }
}
void PathObstacle::del()
{
   if(_world)
   {
      if(_world->_obstacles.contains(_shape))
      {
        _world->changed(*_shape);
        _world->_obstacles.removeData(_shape);
      }
     _world=null;
   }
  _shape=null;
}
Bool PathObstacle::create(C Shape &shape, C PathWorld &world)
{
   PathWorld &pw=ConstCast(world);
   if(T._world==&pw) // if is already linked with this world
   {
      T.shape(shape); // adjust the shape
      return T._world!=null || shape.type==SHAPE_NONE;
   }
   del();
   if(ObstacleShapeSupported(shape.type))
   {
      T._world=&pw;
      T._shape=&pw._obstacles.New();
     *T._shape= shape;
      pw.changed(shape);
      return true;
   }
   return shape.type==SHAPE_NONE;
}
Shape PathObstacle::shape()
{
   if(_world && _shape && _world->_obstacles.contains(_shape))return *_shape;
   return Shape(); // return empty
}
void PathObstacle::shape(C Shape &shape)
{
   if(ObstacleShapeSupported(shape.type) && _world && T._shape && _world->_obstacles.contains(T._shape))
   {
     _world->changed(*T._shape); // call 'changed' before making the change so old area will get updated too
      *T._shape=shape;
     _world->changed(*T._shape); // call 'changed' after  making the change so new area will get updated
   }else del();
}
/******************************************************************************/
// PATH MESH
/******************************************************************************/
PathMesh::~PathMesh() {del(); Delete(_pm);}
PathMesh:: PathMesh() {       New   (_pm);}
/******************************************************************************/
void PathMesh::del    ()  {       _pm->del    ();}
Bool PathMesh::is     ()C {return _pm->is     ();}
void PathMesh::preSave()  {       _pm->preSave();}
/******************************************************************************/
Bool PathMesh::create(MeshBase &mesh, C VecI2 &area_xy, C PathSettings &settings)
{
   del();

   rcContext  ctx(false);
   Memt<VecI> tri_ind ; // triangle indexes combined from mesh tri+quad
   Memt<Byte> tri_type; // triangle types
              tri_type.setNum(mesh.trisTotal());
   if(mesh.quad.elms())
   {
      tri_ind.setNum(mesh.trisTotal());
      Int t=0;
      FREPA(mesh.tri ){tri_ind[t]=mesh.tri .ind(i)       ; tri_type[t]=(mesh.tri .flag() ? mesh.tri .flag(i) : PM_GROUND); t++;}
      FREPA(mesh.quad){tri_ind[t]=mesh.quad.ind(i).tri0(); tri_type[t]=(mesh.quad.flag() ? mesh.quad.flag(i) : PM_GROUND); t++;
                       tri_ind[t]=mesh.quad.ind(i).tri1(); tri_type[t]=(mesh.quad.flag() ? mesh.quad.flag(i) : PM_GROUND); t++;}
   }else
   {
      if(mesh.tri.flag())tri_type.copyFrom(mesh.tri.flag());
      else       SetMemN(tri_type.data(), PM_GROUND, tri_type.elms());
   }

   const float *verts=mesh.vtx.pos ()->c;
   const int   nverts=mesh.vtx.elms();
   const int   ntris =(tri_ind.elms() ? tri_ind.elms() : mesh.tri.elms()   );
   const int   *tris =(tri_ind.elms() ? tri_ind[0].c   : mesh.tri.ind ()->c);

   // area_size=tile_res*cell_size
   Int tile_res =Max(1, RoundPos(settings.areaSize()/settings.cellSize()));
   Flt cell_size=                settings.areaSize()/tile_res;
   Box box=mesh;
   box.min.x=area_xy.x*settings.areaSize(); box.max.x=box.min.x+settings.areaSize();
   box.min.z=area_xy.y*settings.areaSize(); box.max.z=box.min.z+settings.areaSize();

   // set configuration
   RecastConfig cfg(cell_size, settings.cellHeight(), settings.ctrlRadius(), settings.ctrlHeight(), settings.maxClimb());
   cfg.walkableSlopeAngle=RadToDeg(settings.maxSlope());
   cfg.tileSize          =tile_res;
   cfg.width             =cfg.tileSize+cfg.borderSize*2;
   cfg.height            =cfg.tileSize+cfg.borderSize*2;

   cfg.bmin[0]=box.min.x; cfg.bmin[1]=box.min.y; cfg.bmin[2]=box.min.z;
   cfg.bmax[0]=box.max.x; cfg.bmax[1]=box.max.y; cfg.bmax[2]=box.max.z;
   cfg.bmin[0]-=cfg.borderSize*cfg.cs;
   cfg.bmin[2]-=cfg.borderSize*cfg.cs;
   cfg.bmax[0]+=cfg.borderSize*cfg.cs;
   cfg.bmax[2]+=cfg.borderSize*cfg.cs;

   // Allocate voxel heightfield where we rasterize our input data to.
   RecastHeightfield solid;
   if(!rcCreateHeightfield(&ctx, solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))return false;

	// Verify triangle slope
	Flt max_slope_cos=Cos(settings.maxSlope());
	REP(ntris)
	{
    C int *tri=&tris[i*3];
	   Bool walkable=(GetNormal(mesh.vtx.pos(tri[0]), mesh.vtx.pos(tri[1]), mesh.vtx.pos(tri[2])).y>=max_slope_cos);
	   if( !walkable)tri_type[i]=PM_OBSTACLE;
	}
	Int merge_height=cfg.walkableClimb; // based on this value, recast will merge spans
	rcRasterizeTriangles(&ctx, verts, nverts, tris, tri_type.data(), ntris, solid, merge_height);

   // Once all geometry is rasterized, we do initial pass of filtering to remove unwanted overhangs caused by the conservative rasterization as well as filter spans where the character cannot possibly stand.
   rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, solid);
   rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, solid);
   rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, solid);

   // Compact the heightfield so that it is faster to handle from now on. This will result more cache coherent data as well as the neighbours between walkable cells will be calculated.
   if(!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, solid, _pm->chf))return false;

   // Erode the walkable area by agent radius.
   if(cfg.walkableRadius)
   {
      if(!rcErodeWalkableArea(&ctx, cfg.walkableRadius, _pm->chf))return false;
     _pm->chf.clean(); // clean after erosion
   }

   // set params
  _pm->xy    =area_xy;
  _pm->chf.cs=cfg.cs;
  _pm->chf.ch=cfg.ch;
   return Build(_pm->chf, null, 0, settings.ctrlRadius(), settings.ctrlHeight(), settings.maxClimb(), area_xy, _pm, _pm->data);
}
/******************************************************************************/
Bool PathMesh::save(File &f)C {return _pm->save(f);}
Bool PathMesh::load(File &f)  {return _pm->load(f);}
/******************************************************************************/
// PATH WORLD
/******************************************************************************/
static Bool ThreadFunc(Thread &thread) {((PathWorld*)thread.user)->threadFunc(); return true;}
void PathWorld::threadFunc()
{
   if(_build.elms())
   {
      Build build;
      {
         SyncLocker lock(_lock);
         if(_build.elms()){Swap(build, _build[0]); _build.remove(0, true);}else goto wait; // goto will also unlock the 'lock'
      }
      Built built;
      if(!build.shapes.elms() // if we don't have any shapes, then we don't need to build, but just return empty 'data', which will be set to 'obstacle_data', and because 'obstacle_data' will be empty, then 'data' will be used (which has no shapes). Even though this operation is fast and could be performed on the main thread, we still need to process it here on the build thread, to preserve the queue order. For example if we would generate 'built' on the main thread, but on this builder thread would be another build going on that was queued before, then once it completes, it would overwrite the empty 'built'.
      || ::Build(build.chf, build.shapes.data(), build.shapes.elms(), _ctrl_r, _ctrl_h, _max_climb, build.xy, build.user, built.data))
      {
         // we need to store 'xy' and 'user' in case 'data' is empty
         built.xy  =build.xy;
         built.user=build.user;
         SyncLocker lock(_lock);
         Swap(_built.New(), built);
      }
   }else
   {
   wait:
     _event.wait();
   }
}
void PathWorld::zero     () {_area_size=_ctrl_r=_ctrl_h=_max_climb=0; _mesh=null; _query=null; _filter=null;}
     PathWorld::PathWorld() {zero();}
void PathWorld::del      ()
{
   // delete the thread first
  _thread.stop();
   if(_build.elms()){SyncLocker lock(_lock); _build.del();} // cancel build requests
  _event .on (); // wake up thread
  _thread.del();

   // unlink path obstacles?

   // unlink path meshes
   if(C dtNavMesh *mesh=_mesh)
      REP(_mesh->getMaxTiles())
         if(C dtMeshTile *tile=mesh->getTile(i))
	         if(C dtMeshHeader *header=tile->header)
	            if(_PathMesh *path_mesh=(_PathMesh*)header->userId)path_mesh->world=null;

   // free
  _changed  .del  ();
  _build    .del  ();
  _built    .del  ();
  _obstacles.clear(); // use 'clear' to not delete the helper memory, in case there are some PathObstacle's still pointing to that memory
   dtFreeNavMeshQuery(_query );
   dtFreeNavMesh     (_mesh  );
   DeleteN           (_filter);
   zero();
}
/******************************************************************************/
Bool PathWorld::create(Flt area_size)
{
   del();

   New(_filter, PMF_ALL+1); REP(PMF_ALL+1)_filter[i].setIncludeFlags(i);
  _mesh =dtAllocNavMesh     (); if(!_mesh )Exit("Out of Memory");
  _query=dtAllocNavMeshQuery(); if(!_query)Exit("Out of Memory");

   dtNavMeshParams params; Zero(params);
   params.orig[0]   =0;
   params.orig[1]   =0;
   params.orig[2]   =0;
   params.tileWidth =area_size;
   params.tileHeight=area_size;
   params.maxTiles  =MaxTiles;
   params.maxPolys  =MaxPolys;

   if(dtStatusFailed(_mesh ->init(&params)))return false;
   if(dtStatusFailed(_query->init(_mesh, MaxSearchNodes)))return false;
   T._area_size=area_size;
  _thread.create(ThreadFunc, this, 0, false, "EE.PathWorld");
   return true;
}
/******************************************************************************/
Box PathWorld::obstacleBox(C Shape &shape)C
{
   Box    box=shape; box.extendX(_ctrl_r).extendZ(_ctrl_r).min.y-=_ctrl_h;
   return box;
}
/******************************************************************************/
_PathMesh* PathWorld::pathMesh(C VecI2 &area_xy)C
{
   if(_mesh)
	   if(C dtMeshTile *tile=_mesh->getTileAt(area_xy.x, area_xy.y, 0))
	      if(C dtMeshHeader *header=tile->header)
	         return (_PathMesh*)header->userId;

	return null;
}
/******************************************************************************/
void PathWorld::update()
{
   if(_changed.elms() || _built.elms())
   {
      Bool wake_up=(_changed.elms()>0);
      {
         Memt<Shape> area_shapes;
         SyncLocker  lock(_lock);

         // queue elements for processing
         FREPA(_changed) // process in order
         {
           _PathMesh &path_mesh=*_changed[i]; if(path_mesh.is() && path_mesh.getChf())
            {
               Box mesh_box=path_mesh.box();
               REPA(_obstacles)
               {
                C Shape &shape=_obstacles[i];
                  if(Cuts(obstacleBox(shape), mesh_box))area_shapes.add(shape);
               }
               // we always need to queue on the build thread, even if 'area_shapes' is empty, to preserve the build queue order
               Build *build=null; REPA(_build)if(_build[i].xy==path_mesh.xy){build=&_build[i]; break;} if(!build)build=&_build.New(); // if there's already a build queued up for this area, then reuse it with latest data but with the same queue position, otherwise create a new at the end of the queue
               build->chf   = path_mesh.chf;
               build->shapes= area_shapes;
               build->xy    = path_mesh.xy;
               build->user  =&path_mesh;
               area_shapes.clear();
            }
         }
        _changed.clear();

      #if DEBUG && 0 // perform building on this thread
         #pragma message("!! Warning: Use this only for debugging !!")
         for(wake_up=false; _build.elms(); )threadFunc();
      #endif

         // process finished elements
         REPA(_built) // !! process from latest (this is to handle cases where we've got multiple builds for the same area, we will get the latest one, and inside unlinking, we delete all others) !!
            if(InRange(i, _built)) // !! check this, because inside unlinking, we may delete some '_built', and 'i' index may be out of range, so keep going down to zero !!
         {
            Built &built=_built[i];
            if(_PathMesh *pm=pathMesh(built.xy)) // find path mesh at that location
               if(pm==built.user) // if the same path mesh ('user' points to memory address of '_PathMesh')
            {
               Mems<Byte> data; Swap(data, built.data); // move to temp, because unlinking will delete 'built'
               // !! don't access 'built' anymore because it gets deleted !!
               pm->link(null);  Swap(pm->obstacle_data, data); // unlink before changing data, in case unlinking requires the same data that was used for linking
               pm->link(this);
               // !! don't access 'built' anymore because it gets deleted !!
            }
            // !! don't access 'built' anymore because it gets deleted !!
         }
        _built.clear();
      }
      if(wake_up)_event.on(); // wake up thread
   }
}
/******************************************************************************/
void PathWorld::changed(C Shape &shape)
{
   Box   box=obstacleBox(shape);
   RectI areas=worldToArea(box);
   for(Int y=areas.min.y; y<=areas.max.y; y++)
   for(Int x=areas.min.x; x<=areas.max.x; x++)
      if(_PathMesh *pm=pathMesh(VecI2(x, y)))
         if(Cuts(box, pm->box()))_changed.include(pm);
}
/******************************************************************************/
Bool PathWorld::_set(_PathMesh *path_mesh, C VecI2 &area_xy, Bool set_obstacles)
{
  _PathMesh *old=T.pathMesh(area_xy); if(old==path_mesh)return true;
   if(old) // 'old.world' should be set to this
   {
      old->world=null; // unlink from this world
     _changed.exclude(old); // remove from 'changed' as this PathMesh no longer belongs to this world
      // TODO: what about areas being currently built on the thread?
      if(_built.elms()) // !! this is important, because in PathWorld.update we're processing from the end (latest) and we need to remove all others for this area, so in that loop we process only one and latest 'built' but delete the others !!
      {
         SyncLocker lock(_lock);
         REPA(_built)if(dtMeshHeader *header=(dtMeshHeader*)_built[i].data.data())if(header->userId==(UIntPtr)old)_built.remove(i, true);
      }
   }
   if(path_mesh)path_mesh->link(null); // if we want to link 'path_mesh' to this world, then unlink it from any other world first

   if(_mesh)
   {
     _mesh->removeTile(_mesh->getTileRefAt(area_xy.x, area_xy.y, 0), null, null); // remove any previous tile
      if(path_mesh && path_mesh->is())
      {
         // verify area coordinates
         if(path_mesh->xy!=area_xy)return false;

         if(!_ctrl_r) // set world controller settings (before setting obstacles)
            if(dtMeshHeader *header=(dtMeshHeader*)path_mesh->data.data()){_ctrl_r=header->walkableRadius; _ctrl_h=header->walkableHeight; _max_climb=header->walkableClimb;}

         // set current obstacles on the path mesh (after getting controller settings)
         if(set_obstacles)
         {
            Memt<Shape> area_shapes;
            Box         mesh_box=path_mesh->box();
            REPA(_obstacles)
            {
             C Shape &shape=_obstacles[i];
               if(Cuts(obstacleBox(shape), mesh_box))area_shapes.add(shape);
            }
            if(!area_shapes.elms())path_mesh->obstacle_data.del();else // if no shapes are needed, then delete 'obstacle_data' and use the main 'data' which we already have
            {
               if(!path_mesh->getChf())return false;
               RecastCompactHeightfield chf=path_mesh->chf; // operate on 'chf' copy because it gets modified in 'Build'
               if(!::Build(chf, area_shapes.data(), area_shapes.elms(), _ctrl_r, _ctrl_h, _max_climb, area_xy, path_mesh, path_mesh->obstacle_data))return false; // 'userId' points to memory address of '_PathMesh'
            }
         }

         // set path mesh data into recast
         Mems<Byte> &data=(path_mesh->obstacle_data.elms() ? path_mesh->obstacle_data : path_mesh->data);
         if(data.elms())
         {
            if(dtStatusFailed(_mesh->addTile(data.data(), data.elms(), 0, 0, null)))return false;
            path_mesh->world=this; // link only after 'addTile' succeeded
         }
      }
      return true;
   }
   return path_mesh==null;
}
Bool PathWorld::set(PathMesh *path_mesh, C VecI2 &area_xy) {return _set(path_mesh ? path_mesh->_pm : null, area_xy, true);}
/******************************************************************************/
Bool PathWorld::find(C Vec &start, C Vec &end, MemPtr<Vec> path, Int max_steps, UInt walkable_flags, Bool allow_partial_paths, C Vec &end_extents)C
{
   path.clear();
   if(_query)
   {
      ConstCast(T).update();
      dtQueryFilter *filter=&T._filter[walkable_flags&PMF_ALL];
      Vec extents(1, 16, 1); // use high vertical extent because when there are no detail meshes, path mesh can be located at big differences when compared to original mesh
      dtPolyRef start_ref=0; _query->findNearestPoly(start.c, extents.c, filter, &start_ref, null);
      if(start_ref)
      {
         dtPolyRef end_ref=0; _query->findNearestPoly(end.c, end_extents.c, allow_partial_paths ? &T._filter[PMF_ALL] : filter, &end_ref, null); // use all filters to find the destination (for example if 'start' is on water, but 'end' is on ground, and 'walkable_flags' is set only to swim, then 'end' would not be found, and we wouldn't swim even a little towards the 'end', when 'end' is found using all filters, then we can just swim a little towards it)
         if(end_ref)
         {
            dtPolyRef polys[2048]; Int npolys=0;
		     _query->findPath(start_ref, end_ref, start.c, end.c, filter, polys, &npolys, Elms(polys));
            if(npolys)
            {
               Vec path_points[2048], new_end=end; Int path_length=0;
			      if(polys[npolys-1]!=end_ref) // if there's no direct path
			      {
			         if(allow_partial_paths)_query->closestPointOnPoly(polys[npolys-1], end.c, new_end.c, null); // In case of partial path, make sure the end point is clamped to the last polygon
			         else return false;
			      }

               if(dtStatusSucceed(_query->findStraightPath(start.c, end.c, polys, npolys, path_points[0].c, null, null, &path_length, (max_steps>0) ? Min(max_steps, Elms(path_points)) : Elms(path_points))))
               {
                  path.setNum(path_length).copyFrom(path_points);
                  return true;
               }
            }
         }
      }
   }
   return false;
}
/******************************************************************************/
Bool PathWorld::nearestSurface(C Vec &pos, C Vec &extents, Vec &surface_pos, UInt walkable_flags)C
{
   if(_query)
   {
      ConstCast(T).update();
      dtQueryFilter *filter=&T._filter[walkable_flags&PMF_ALL];
      dtPolyRef pos_ref;
      if(dtStatusSucceed(_query->findNearestPoly(pos.c, extents.c, filter, &pos_ref, surface_pos.c)))return true;
   }
   return false;
}
/******************************************************************************/
Bool PathWorld::nearestWall(C Vec &pos, Flt max_distance, Flt *hit_distance, Vec *hit_pos, Vec *hit_normal, UInt walkable_flags)C
{
   if(_query)
   {
      ConstCast(T).update();
      dtQueryFilter *filter=&T._filter[walkable_flags&PMF_ALL];
      Flt dist;
      Vec extents(max_distance), hp, hn;
      dtPolyRef pos_ref=0; _query->findNearestPoly(pos.c, extents.c, filter, &pos_ref, null);
      if(pos_ref && dtStatusSucceed(_query->findDistanceToWall(pos_ref, pos.c, max_distance, filter, &dist, hp.c, hn.c)))
         if(dist<max_distance)
      {
         if(hit_distance)*hit_distance=dist;
         if(hit_pos     )*hit_pos     =hp  ;
         if(hit_normal  )*hit_normal  =hn  ;
         return true;
      }
   }
   if(hit_distance)*hit_distance=0;
   if(hit_pos     ) hit_pos   ->zero();
   if(hit_normal  ) hit_normal->zero();
   return false;
}
/******************************************************************************/
Bool PathWorld::ray(C Vec &start, C Vec &end, Flt *hit_frac, Vec *hit_pos, Vec *hit_normal, UInt walkable_flags)C
{
   if(_query)
   {
      ConstCast(T).update();
      dtQueryFilter *filter=&T._filter[walkable_flags&PMF_ALL];
      Flt t;
      Vec extents(1, 16, 1), hn; // use high vertical extent because when there are no detail meshes, path mesh can be located at big differences when compared to original mesh
      dtPolyRef start_ref=0; _query->findNearestPoly(start.c, extents.c, filter, &start_ref, null);
      if(start_ref && dtStatusSucceed(_query->raycast(start_ref, start.c, end.c, filter, &t, hn.c, null, null, 0)))
         if(t<=1)
      {
         if(hit_frac  )*hit_frac  =t;
         if(hit_pos   )*hit_pos   =Lerp(start, end, t);
         if(hit_normal)*hit_normal=hn;
         return true;
      }
   }
   if(hit_frac  )*hit_frac=1;
   if(hit_pos   )*hit_pos =end;
   if(hit_normal) hit_normal->zero();
   return false;
}
/******************************************************************************/
static float DistancePtLine2d(C float* pt, C float* p, C float* q)
{
   float pqx=q[0] - p[0];
   float pqz=q[2] - p[2];
   float dx=pt[0] - p[0];
   float dz=pt[2] - p[2];
   float d=pqx*pqx+pqz*pqz;
   float t=pqx*dx +pqz*dz;
   if(d)t/=d;
   dx=p[0]+t*pqx-pt[0];
   dz=p[2]+t*pqz-pt[2];
   return dx*dx + dz*dz;
}
static void DrawPolyBoundaries(C dtMeshTile* tile, C Color &col, C float linew, bool inner)
{
   if(!col.a)return;
   VI.color(col);

 C float thr=0.01f*0.01f;

   FREP(tile->header->polyCount)
   {
	 C dtPoly* p=&tile->polys[i];
	   if(p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION) continue;
	 C dtPolyDetail* pd=&tile->detailMeshes[i];

	   for(int j=0, nj=p->vertCount; j<nj; ++j)
	   {
		   Color c=col;
		   if(inner)
		   {
			   if(p->neis[j]==0)continue;
			   if(p->neis[j]&DT_EXT_LINK)
			   {
				   bool con=false;
				   for(unsigned int k=p->firstLink; k!=DT_NULL_LINK; k=tile->links[k].next)if(tile->links[k].edge == j){con=true; break;}
				   if(con)c.set(255, 255, 255, 48);
				   else   c.set(  0,   0,   0, 48);
			   }else     c.set(  0,  48,  64, 32);
		   }else if(p->neis[j])continue;

		 C float* v0=&tile->verts[p->verts[j]*3];
		 C float* v1=&tile->verts[p->verts[(j+1)%nj]*3];

		   for(int k=0; k<pd->triCount; ++k) // Draw detail mesh edges which align with the actual poly edge. This is really slow.
		   {
			 C unsigned char* t=&tile->detailTris[(pd->triBase+k)*4];
			 C float* tv[3];
			   for(int m=0; m<3; ++m)
			   {
				   if(t[m]<p->vertCount)tv[m]=&tile->verts[p->verts[t[m]]*3];
				   else                 tv[m]=&tile->detailVerts[(pd->vertBase+(t[m]-p->vertCount))*3];
			   }
			   for(int m=0, n=2; m<3; n=m++)
			   {
				   if(((t[3] >> (n*2)) & 0x3) == 0)continue; // Skip inner detail edges.
				   if(DistancePtLine2d(tv[n],v0,v1)<thr
				   && DistancePtLine2d(tv[m],v0,v1)<thr)
				   {
				    C float *a=tv[n], *b=tv[m];
				      VI.line(Vec(a[0], a[1], a[2]), Vec(b[0], b[1], b[2]));
				   }
			   }
		   }
	   }
   }
   VI.end();
}
static void DrawMeshTile(C dtMeshTile *tile, Byte surface_color_alpha, C Color &outer_edge_color, C Color &inner_edge_color)
{
   if(tile && tile->header)
   {
      if(surface_color_alpha)
      {
         for(int i=0; i<tile->header->polyCount; ++i)
         {
	       C dtPoly *p=&tile->polys[i];
	         if(p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)continue; // Skip off-mesh links.
	       C dtPolyDetail *pd=&tile->detailMeshes[i];
	         
	         Color surface_color;
	         switch(p->getArea())
	         {
	            case PM_BLOCKED: surface_color.set(0,   0,   0,                   0); break;
	            case PM_GROUND : surface_color.set(0, 255,   0, surface_color_alpha); break;
	            case PM_WATER  : surface_color.set(0, 192, 255, surface_color_alpha); break;
	         }

	         if(surface_color.a)
	            for(int j=0; j<pd->triCount; ++j)
	         {
		       C unsigned char *t=&tile->detailTris[(pd->triBase+j)*4];
		         Vec vtx[3];
		         for(int k=0; k<3; ++k)
		         {
		          C float *vtx_pos;
			         if(t[k]<p->vertCount)vtx_pos=&tile->verts[p->verts[t[k]]*3];
			         else                 vtx_pos=&tile->detailVerts[(pd->vertBase+t[k]-p->vertCount)*3];
				      vtx[k].set(vtx_pos[0], vtx_pos[1], vtx_pos[2]);
		         }
		         VI.tri(surface_color, vtx[0], vtx[1], vtx[2]);
	         }
         }
         VI.end();
      }
      DrawPolyBoundaries(tile, inner_edge_color, 1.5f, true );
      DrawPolyBoundaries(tile, outer_edge_color, 2.5f, false);
   }
}
void PathWorld::draw(Byte surface_color_alpha, Flt y_offset, C Color &outer_edge_color, C Color &inner_edge_color)C
{
   ConstCast(T).update();
   SetMatrix(Matrix(Vec(0, y_offset, 0)));
   if(C dtNavMesh *cmesh=_mesh)REP(cmesh->getMaxTiles())DrawMeshTile(cmesh->getTile(i), surface_color_alpha, outer_edge_color, inner_edge_color);
}
/******************************************************************************/
}
/******************************************************************************/
