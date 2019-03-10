/******************************************************************************/
#include "stdafx.h"
#include "../Shaders/!Header CPU.h"
namespace EE{
/******************************************************************************/
#define LIGHT_DENSITY_FACTOR 0.5f // 0..1, 0.5 gives good results, should be 1 however because volume rendering is not perfect (when iterating samples, voxel texture coordinates may be chosen between between voxels, which may make them look blurry, darker, or brighter)
/******************************************************************************/
static Image     detail;
       AllClouds Clouds;
/******************************************************************************/
// LAYERED CLOUDS
/******************************************************************************/
LayeredClouds::LayeredClouds()
{
   draw_in_mirror=false;
   merge_with_sky=MOBILE;
   layer[0].color=WHITE; layer[0].scale=1.0f/2.8f; layer[0].position.set(0.00f, 0.0f); layer[0].velocity=0.010f;
   layer[1].color=WHITE; layer[1].scale=1.0f/2.4f; layer[1].position.set(0.25f, 0.2f); layer[1].velocity=0.008f;
   layer[2].color=WHITE; layer[2].scale=1.0f/2.0f; layer[2].position.set(0.50f, 0.1f); layer[2].velocity=0.006f;
   layer[3].color=WHITE; layer[3].scale=1.0f/1.6f; layer[3].position.set(0.75f, 0.8f); layer[3].velocity=0.004f;
   frac(0.9f);
  _scale_y=1.05f; // set manually instead of calling method to avoid accessing shader param map, which could crash if not yet initialized
  _rmc    =4    ; // set manually instead of calling method to avoid accessing shader param map, which could crash if not yet initialized
}
void LayeredClouds::del()
{
  _mshr.del();
   REPAO(layer).image=null;
}
void LayeredClouds::create()
{
   {Flt v=         scaleY(); _scale_y=-1;          scaleY(v);}
   {Flt v=rayMaskContrast(); _rmc    =-1; rayMaskContrast(v);}

   MeshBase mshb;
   mshb.createIcoHalf(Ball(1), 0, 3); // 3 give 'dist'=0.982246876
  _mshr.create(mshb.reverse());
#if DEBUG && 0 // calculate actual distance
   Flt dist=1; C Vec *pos=mshb.vtx.pos();
   REPA(mshb.tri ){C VecI  &t=mshb.tri .ind(i); MIN(dist, Dist(VecZero, Tri (pos[t.x], pos[t.y], pos[t.z])));}
   REPA(mshb.quad){C VecI4 &q=mshb.quad.ind(i); MIN(dist, Dist(VecZero, Quad(pos[q.x], pos[q.y], pos[q.z], pos[q.w])));}
   int z=0;
#endif
#define CLOUD_MESH_MIN_DIST 0.98f // it's good to make it a bit smaller than 'dist' to have some epsilon for precision issues, this is the closest point on the mesh to the Vec(0,0,0), it's not equal to radius=1, because the mesh is composed out of triangles, and the triangle surfaces are closer
}
/******************************************************************************/
LayeredClouds& LayeredClouds::set(Byte active_layers, C ImagePtr &image)
{
  _layers=Mid(active_layers, 0, 4);
   if(image)REP(_layers)layer[i].image=image;
   return T;
}
LayeredClouds& LayeredClouds::frac(Flt frac)
{
   SAT(frac); T._frac=frac;
   return T;
}
LayeredClouds& LayeredClouds::scaleY(Flt scale)
{
   Clamp(scale, 1, 2);
   if(T._scale_y!=scale)SPSet("LCScaleY", T._scale_y=scale);
   return T;
}
LayeredClouds& LayeredClouds::rayMaskContrast(Flt contrast)
{
   MAX(contrast, 1);
   if(T._rmc!=contrast)SPSet("LCMaskContrast", Vec2(T._rmc=contrast, contrast*-0.5f+0.5f));
   return T;
}
/******************************************************************************/
void LayeredClouds::update()
{
   REP(_layers)layer[i].position+=layer[i].velocity*Time.d();
}
#pragma pack(push, 4)
struct GpuCloudLayer
{
   Vec4 color;
   Vec2 scale, position;
};
#pragma pack(pop)
inline void LayeredCloudsFx::load()
{
   if(!shader)
   {
      shader=ShaderFiles("Layered Clouds");

      range=GetShaderParam("LCRange");

      h_CL[0]=GetShaderParam("CL[0]");
      h_CL[1]=GetShaderParam("CL[1]");
      h_CL[2]=GetShaderParam("CL[2]");
      h_CL[3]=GetShaderParam("CL[3]");
   }
}
inline Shader* LayeredCloudsFx::get(Int layers, Bool blend, Bool mask)
{
   Shader* &s=h_Clouds[layers][blend][mask]; if(!s)s=shader->get(S+"Clouds"+(layers+1)+(blend?'B':'\0')+(mask?'M':'\0'));
   return   s;
}
void LayeredClouds::commit()
{
   LC.load();

   REP(_layers)
   {
      GpuCloudLayer cl;
      cl.color   =layer[i].color.asVec4();
      cl.scale   =layer[i].scale/LCScale;
   #if MOBILE
      cl.position=Frac(layer[i].position); // mobile devices can have low precision for tex coords, so use fraction to always keep in 0..1 range
   #else
      cl.position=layer[i].position;
   #endif
      LC.h_CL[i]->set(cl);
   }

   Sh.h_ImageCol[0]->set(layer[0].image());
   Sh.h_ImageCol[1]->set(layer[1].image());
   Sh.h_ImageCol[2]->set(layer[2].image());
   Sh.h_ImageCol[3]->set(layer[3].image());
   MaterialClear();
}
Bool LayeredClouds::wantDepth()C {return _layers && !merge_with_sky && frac()<1;}
void LayeredClouds::draw()
{
   if(_layers && !merge_with_sky && (draw_in_mirror || !Renderer.mirror()))
   {
      commit();

      Renderer.set(Renderer._col(), Renderer._sky_coverage(), null, null, Renderer._ds(), true, WANT_DEPTH_READ); // use DS for depth tests
      Flt from=D.viewRange()*frac(),
          to  =D.viewRange();
      MAX(from, FrustumMain.view_quad_max_dist/CLOUD_MESH_MIN_DIST); // make sure we don't intersect with the near plane
      Bool blend=(Renderer.canReadDepth1S() && from<to-EPS_SKY_MIN_LERP_DIST); // !! set after 'Renderer.set' !! set blend mode if 'from' is far from 'to', and yes use < and not <= in case of precision issues for big values
      if(  blend)
      {
         //Flt sky_opacity=Length(O.pos)*MulAdd.x+MulAdd.y;
         //              0=       from  *MulAdd.x+MulAdd.y;
         //              1=       to    *MulAdd.x+MulAdd.y;
         Vec2 mul_add; mul_add.x=1/(to-from); mul_add.y=-from*mul_add.x;
         LC.range->setConditional(mul_add);
      }
   #if !REVERSE_DEPTH
      MIN(from, to*EPS_SKY_MIN_VIEW_RANGE);
   #endif
      SetOneMatrix(MatrixM(from, CamMatrix.pos));
      D.alpha     (ALPHA_BLEND_DEC);
      D.depthWrite(false);
      D.depthFunc (FUNC_LESS_EQUAL); // to make sure we draw at the end of viewRange
      D.depth     (true );
      D.cull      (true );
      D.sampler3D (     );
      Shader *shader=LC.get(_layers-1, blend, Renderer._sky_coverage!=null);
      REPS(Renderer._eye, Renderer._eye_num)
      {
         Renderer.setEyeViewport();
         shader->begin(); _mshr.set().drawFull(); ShaderEnd();
      }
      D.sampler2D ();
      D.depthWrite(true);
      D.depthFunc (FUNC_LESS);
   }
}
void LayeredClouds::shadowMap()
{
}
/******************************************************************************/
// VOLUMETRIC CLOUD
/******************************************************************************

   Voxel arrangement is set so that height is the first coordinate, to improve cache performance on GPU.
      Normally           : x=left/right size     , y=up/down height        , z=forward/back size
      However clouds have: up/down height (level), then left/right size (x), then forward/back size (y)

   That's why we need to use a CWW sampler (Clamp, Wrap, Wrap).

   Building first sets a software copy of the image in the system, memory, so we don't have to make any locks or use extra GPU memory for a secondary image.
   Once build is complete, then on the main thread, GPU Image is updated from that software data.

/******************************************************************************/
static Vec LightPos(C VolumetricCloud::Settings &s) // !! assumes that 's.light_pos' is normalized !!
{
   Vec pos=s.light_pos;
   if( pos.y<SQRT2_2){pos.y=SQRT2_2; pos.normalize();} // this is how build works, it treats all light positions as if they were above the clouds
   return pos*s.light_power; // apply light power already
}
static Byte Difference(C VolumetricCloud::Settings &a, C VolumetricCloud::Settings &b) // 0=can be ignored, 1=medium difference (schedule a new build, and use it when finished), 2=big difference (schedule a new build and wait for it to finish)
{
   Flt d=Abs (         a.density     -          b.density    )
        +Abs (         a.noise_min   -          b.noise_min  )
        +Abs (         a.noise_max   -          b.noise_max  )
      //+Abs (         a.brightness  -          b.brightness ) already applied onto ambient and light
        +Abs (SqrtFast(a.ambient)    - SqrtFast(b.ambient)   ) //+Abs(a.ambient     - b.ambient    ) use Sqrt because changes are more noticeable in the dark
      //+Abs (         a.light_power -          b.light_power) check light_pos together with light_power in 'LightPos'
        +Dist(LightPos(a)            , LightPos(b)           );
   return (d>=0.05f) ? 2 : (d>=1.0f/256 || a.detail!=b.detail) ? 1 : 0; // return difference (0=ignore, 1=update, 2=update now)
}
/******************************************************************************/
          static void SetDensityRow(IntPtr elm_index, Ptr user, Int thread_index) {VolumetricCloud &vc=*(VolumetricCloud*)user; vc.setDensityRow(elm_index);}
void VolumetricCloud::setDensityRow(Int z)
{
   Vec pos;
   pos.y=_level*_noise_scale;
   pos.z= z    *_noise_scale;
   if(_object)REPD(x, width())
   {
      pos.x=x*_noise_scale;
      Flt n=_noise.noise3(pos.x, pos.y, pos.z, 8, _noise_gain);
      voxel(x, _level, z).set(n);
   }else
   REPD(x, width())
   {
      pos.x=x*_noise_scale;
      Flt n=_noise.tiledNoise3(pos.x, pos.y, pos.z, _tile, 8, _noise_gain);
      voxel(x, _level, z).set(n);
   }
}
static void                  SetDensity(VolumetricCloud &vc, Ptr user, Int thread_index) {vc.setDensity();}
       void VolumetricCloud::setDensity()
{
   REPS(_level, height())
   {
      if(_threads)_threads->process1(depth(), SetDensityRow, this);
      else         REPD(z, depth())setDensityRow(z);
   }
  _creating=false; // notify that finished creating
}
/******************************************************************************/
void VolumetricCloud::cancelCreate()
{
   if(_creating) // if not finished
   {
      if(_threads)
      {
        _threads->cancel(T, SetDensity);
        _threads->wait  (T, SetDensity);
      }
   }
}
/******************************************************************************/
#if 1 // this is the best, contrast is not too strong for high density/coverage (noise_min) and doesn't produce dark spots for high density with small coverage (noise_min)
static Flt AOFunc(Flt density) {return 1-SqrtFast(density)*0.31f;}
#elif 1 // contrast too strong
static Flt AOFunc(Flt density) {return 1-(1-Sqr(1-density))*0.31f;}
#else // contrast way too strong
static Flt AOFunc(Flt density) {return 1-density*0.5f;}
#endif
          static void SetImageRow(IntPtr elm_index, Ptr user, Int thread_index) {VolumetricCloud &vc=*(VolumetricCloud*)user; vc.setImageRow(elm_index);}
void VolumetricCloud::setImageRow(Int z)
{
   if(!_building)return; // if cancel was requested

   Int    width=T.width(), height=T.height(), depth=T.depth();
   Flt   *light_cur=_light, *light_prev=light_cur+width*depth; if(_level&1)Swap(light_cur, light_prev); // swap every level
   Byte  *image_data=_image_data + _image.bytePP()*(_level + z*_pitch); Int image_add=_image.bytePP()*height;
 C Voxel *voxel     =_voxels     +                 (_level + z*_pitch); Int voxel_add=SIZE(Voxel)*_image.w();
 C Voxel *detail[3];
   Flt    x_mul, x_add, dy2_dz2;
   if(_object)
   {
          x_mul=2.0f/(width+(2-1)); x_add=-1+x_mul; // +2 because we're treating as if we have 2 extra pixels at the start and end, so first/last actual pixels will have some density and not zero (because that's a waste of memory to have all zero pixels)
      Flt z_mul=2.0f/(depth+(2-1)), z_add=-1+z_mul; // +2 because we're treating as if we have 2 extra pixels at the start and end, so first/last actual pixels will have some density and not zero (because that's a waste of memory to have all zero pixels)
      dy2_dz2=_dy2 + Sqr(z*z_mul + z_add);
   }
   if(_build.detail)
   {
      Int yd=_level, zd=z; REP(_build.detail){yd*=2; zd*=2; detail[i]=_voxels + (yd%height + (zd%depth)*_pitch);}
   }
   FREPD(x, width) // go from the start because of 'image_data' and 'voxel'
   {
    //if(!_building)return; // if cancel was requested
      Flt density;

      Flt light;
      if(_object)
      {
         Flt dx=x*x_mul+x_add;
       //density=Min(1, voxel->positive()-SqrtFast(dx*dx + dy2_dz2)); // do just "Min(1" instead of 'Sat', because we check anyway if "density>0" below
         density=voxel->positive()-SqrtFast(dx*dx + dy2_dz2); // !! if changing the formula then add "Min(1" !! for this formula we don't need "Min(1" because 'voxel->positive' is always 0..1 and from it we subtract
         if(_level==0)light=_build.light_power;else // start
         {
            light=0; REP(_srcs)
            {
             C Src &src=_src[i];
               Int xp=x+src.dir.x, zp=z+src.dir.y;
               if(InRange(xp, width) && InRange(zp, depth))light+=src.mul*light_prev[xp + zp*width]; // TODO: diagonals should have extra density scale, as below, with 'light_cur' not being affected by density, however that would be slower
               else                                        light+=src.mul*_build.light_power; // if coordinates are out of range, then use full light
            }
         }
      }else
      {
         density=Min(1, voxel->density*_noise_mul+_noise_add); // do just "Min(1" instead of 'Sat', because we check anyway if "density>0" below
         if(_level==0)light=_build.light_power;else // start
         {
            light=0; REP(_srcs)
            {
             C Src &src=_src[i];
               Int xp=(x+src.dir.x)%width, zp=(z+src.dir.y)%depth;
               light+=src.mul*light_prev[xp + zp*width]; // TODO: diagonals should have extra density scale, as below, with 'light_cur' not being affected by density, however that would be slower
               // Flt df=1-Pow(1-light_density_factor, Vec(src.dir.x, 1, src.dir.y).length()); this entire line could be precomputed
               // light+=src.mul*light_prev[xp + zp*width]*(1-prev_density*df); 'prev_density' must be of previous voxel, not this one !!
            }
         }
      }
      Byte a;
      if(density>0) // there are a lot of cases which don't have density, so perform some calculations only if we have it
      {
         if(_build.detail) // add details, this must be done here and not in create, because it depends on 'noise_min,noise_max' which are configurable
         {
            Int xd=x; Flt mul=density; REP(_build.detail)
            {
               xd*=2; density+=mul*detail[i][(xd%width)*height].get(); mul*=0.5f; // 'height' is the pitch for X coordinate
            }
            SAT(density);
         }
         density*=_sqr_density;
         light_cur[x + z*width]=light*(1-density*LIGHT_DENSITY_FACTOR); // store current light for use in next level, adjust by density here already
         light+=_build.ambient*AOFunc(density); // this simulates AO (dense voxels are most likely surrounded by others, so ambient light can't reach it much)
         a=FltToByte(density);
      }else
      {
         light_cur[x + z*width]=light; // store current light for use in next level
         light+=_build.ambient;
         a=0;
      }

      Byte l=FltToByte(light); // no need to mul 'light' by 'brightness' because light and AO were already multiplied by it

   #if DX9
      switch(_image.bytePP())
      {
         case 2:                             image_data[0]=l; image_data[1]=a; break; // L8A8
         case 4: image_data[2]=image_data[1]=image_data[0]=l; image_data[3]=a; break; // B8G8R8A8/R8G8B8A8
      }
   #else
      image_data[0]=l; image_data[1]=a; // R8G8/R8G8B8A8
   #endif
      image_data+=image_add;
      voxel=(Voxel*)((Byte*)voxel+voxel_add);
   }
}
/******************************************************************************/
static void                  Build(VolumetricCloud &vc, Ptr user, Int thread_index) {vc.build();}
       void VolumetricCloud::build()
{
   if(_creating) // if hasn't finished creating
   {
      // it's possible that create function hasn't started yet, in that case we need to try to cancel it (remove from the queue) and create it ourselves
      if(_threads->cancel(T, SetDensity))setDensity(); // if removed from queue (hasn't started yet) then create here
      else                              _threads->wait(T, SetDensity); // if didn't remove, then it means it's processing now, so wait for it
   }

   // density = Flt(noise-noise_min)/(noise_max-noise_min)
      _noise_mul=1/Max(EPS, _build.noise_max-_build.noise_min);
   Flt noise_add=-_build.noise_min*_noise_mul, dy_mul=_noise_mul;
   if(MEMBER_SIZE(Voxel, density)==1) // if density stored as byte, then adjust '_noise_mul' '_noise_add'
   {
      // UByteToSFlt(ubyte)*_noise_mul + _noise_add
      // (-1+ubyte*(2.0f/255))*_noise_mul + _noise_add
      // ubyte*(2.0f/255)*_noise_mul - _noise_mul + _noise_add
      noise_add-=_noise_mul;
     _noise_mul*=2.0f/255;
   }

   Flt top=LerpR(PI_4, 0.0f, Acos(_build.light_pos.y)); // same as LerpR(PI_4, PI_2, Asin(light_pos.y));
  _srcs=0;
   if(_build.light_power>EPS_COL)
   {
      if(top>0)
      {
         Src &s=_src[_srcs++];
         s.dir.zero();
         s.mul=top;
      }
      Flt bottom=1-top;
      if( bottom>0)
      {
         MIN(bottom, 1);

         Vec2  light_pos_xy=_build.light_pos.xz();
         VecI2 diagonal(Sign(light_pos_xy.x), Sign(light_pos_xy.y)); if(!diagonal.all())
         {
            Src &s=_src[_srcs++];
            s.dir=diagonal;
            s.mul=bottom;
         }else
         {
            Flt a; VecI2 axis;
            // TODO: this looks orthogonal, probably have to adjust density factor for diagonal movements as mentioned above, then select which option should be used below
            if(1) // produces smoother results
            {
               light_pos_xy.normalize();
               Flt axis_dot;
               if(Abs(light_pos_xy.x)>Abs(light_pos_xy.y)){axis.set(diagonal.x, 0); axis_dot=Abs(light_pos_xy.x);}
               else                                       {axis.set(0, diagonal.y); axis_dot=Abs(light_pos_xy.y);}
               a=LerpR(PI_4, 0.0f, Acos(axis_dot));
            }else // this version matches direction "!light_pos_xy == !Lerp(diagonal, axis, a)"
            {
               if(Abs(light_pos_xy.x)>Abs(light_pos_xy.y))axis.set(diagonal.x, 0);
               else                                       axis.set(0, diagonal.y);
               VecI2 dir=axis-diagonal, normal=Perp(dir); // here 'dir' and 'normal' are always normalized
               Vec2  point=light_pos_xy*DistPointPlaneRay(axis, normal, light_pos_xy); // optimized version of "PointOnPlaneRay(Vec2(0), axis, normal, light_pos_xy);"
               a=DistPointPlane(point, diagonal, dir);
            }

            if(a>0)
            {
               Src &s=_src[_srcs++];
               s.dir=axis;
               s.mul=bottom*a;
            }
            if(a<1)
            {
               Src &s=_src[_srcs++];
               s.dir=diagonal;
               s.mul=bottom*(1-a);
            }
         }
      }
      if(!_object)REP(_srcs) // coords must be >=0, so we can use only % in the loop without checking for sign
      {
         Src &src=_src[i];
         if(src.dir.x<0)src.dir.x+=width();
         if(src.dir.y<0)src.dir.y+=depth();
      }
   }
  _sqr_density=Sqr(_build.density); // use 'Sqr' because it represents density transitions more naturally, if this 'Sqr' would be removed, then 'Difference' should compare 'Sqrt' of densities "Abs(SqrtFast(a.density) - SqrtFast(b.density))"
   Flt y_mul=2.0f/(height()+(2-1)), y_add=-1+y_mul; // +2 because we're treating as if we have 2 extra pixels at the start and end, so first/last actual pixels will have some density and not zero (because that's a waste of memory to have all zero pixels)
   FREPS(_level, height())
   {
      if(!_building)return; // if cancel was requested
      Flt dy=_level*y_mul+y_add; _dy2=Sqr(dy);
     _noise_add=noise_add-dy_mul* // this is only for !object, because object ignores '_noise_add'
         Sqr(LerpRS(0.75f, 1.0f, Abs(dy))); // the best, alternatives: LerpRS(0.65f, 1.0f, _dy2), LerpRS(0.8f, 1.0f, Abs(dy))
      if(_threads)_threads->process1(depth(), SetImageRow, this);
      else         REPD(z, depth())setImageRow(z);
   }
  _build_finished=true;
}
/******************************************************************************/
void VolumetricCloud::cancelBuild()
{
   if(_building)
   {
     _building=0; // set this ASAP so functions can break/return
      if(_threads)
      {
        _threads->cancel(T, Build);
         // TODO: there may be 'SetImageRow' processing that could be cancelled here, however there's no way currently to cancel 'Threads.process', so instead, functions check for '_building' and break/return
        _threads->wait  (T, Build);
      }
     _build_finished=false;
   }
}
/******************************************************************************/
     VolumetricCloud::VolumetricCloud() {zero();}
void VolumetricCloud::zero()
{
  _build_finished=_creating=_object=false;
  _building=_srcs=0;
  _pitch=_level=0;
  _noise_scale=_noise_gain=_noise_mul=_noise_add=_sqr_density=_dy2=0;
  _tile.zero();
  _voxels=null;
  _light=null;
  _image_data=null;
  _threads=null;
  _cur.zero(); _build.zero();
}
void VolumetricCloud::del()
{
   cancelBuild (); // cancel build first because it can create too
   cancelCreate();
   Free(_voxels);
  _image.del();
   zero();
}
void VolumetricCloud::create        (Int size , Int height,            Int frequency, Threads *threads, UInt seed, Flt noise_gain) {createEx(size , height, size , frequency, threads, seed, noise_gain, false);}
void VolumetricCloud::createAsObject(Int width, Int height, Int depth, Flt frequency, Threads *threads, UInt seed, Flt noise_gain) {createEx(width, height, depth, frequency, threads, seed, noise_gain, true );}
void VolumetricCloud::createEx      (Int width, Int height, Int depth, Flt frequency, Threads *threads, UInt seed, Flt noise_gain, Bool object)
{
   del();
   if(width>0 && height>0 && depth>0)
      if(_image.create3DTry(height, width, depth, DX9 ? IMAGE_L8A8 : IMAGE_R8G8, 1))
   {
     _image.clear();
     _object=object;
     _pitch=height*width;
      Int voxels=_pitch*depth,
          voxels_size=SIZE(Voxel)*voxels,
           light_size=SIZE(Flt  )*2*width*depth, // 2 levels (current and previous)
           image_size=_image.bytePP()*voxels;
     _voxels    =(Voxel*)Alloc(voxels_size+light_size+image_size);
     _light     =(Flt*)((Byte*)_voxels+voxels_size);
     _image_data=       (Byte*)_light + light_size;
     _noise=seed;
     _noise_gain=noise_gain;
      if(_object)_noise_scale=frequency;else
      {
         Int freq=Round(frequency);
        _noise_scale=Flt(freq)/width;
        _tile.set(freq, Trunc(height*_noise_scale)*2+1, freq); // set 'tile.y' to bigger than max, to disable wrapping level coordinates
      }
     _threads=threads;

     _creating=true; // set before create so it can clear it when finished
      if(_threads)_threads->queue(T, SetDensity);else setDensity(); // call this last, once all members are set
   }
}
/******************************************************************************/
void VolumetricCloud::update(C Settings &settings)
{
   Settings temp;
   temp.detail     =Mid(settings.detail, 0, 3);
   temp.density    =Sat(settings.density);
   temp.noise_min  =Mid(settings.noise_min, -2.0f, 2.0f);
   temp.noise_max  =Mid(settings.noise_max, -2.0f, 2.0f);
   temp.brightness =Sat(settings.brightness);
   temp.ambient    =Sat(settings.ambient);
   temp.light_power=Sat(settings.light_power);
   temp.light_pos  =    settings.light_pos; temp.light_pos.normalize(); // build funtion and 'LightPos' assume it's normalized

   temp.ambient    *=temp.brightness;
   temp.light_power*=temp.brightness;

   Byte difference=Difference(temp, _cur);
   if( !difference)cancelBuild();else // no build needed
   { // want build
      if(_building) // already building
      {
         if(Difference(temp, _build)<=1) // if similar to what we're already building, here we check 0 and 1, because for 1 there's no hurry, so it can be built in the future
            {_building=difference; return;} // adjust building before returning
         cancelBuild();
      }
     _build=temp; _building=difference; // remember what kind of build we want, before build starts
      if(_threads)_threads->queue(T, Build);else build();
   }
}
/******************************************************************************/
void VolumetricCloud::checkBuild()
{
   if(_building)
   {
   again:
      if(_build_finished) // check if finished first
      {
        _building=0;
        _build_finished=false;
        _image.setFrom(_image_data, _image.w()*_image.bytePP());
        _cur=_build;
      }else
      if(_building==2) // have to wait
      {
         SyncUnlocker unlocker(D._lock); // this may be called in Draw where we have display lock, so unlock it before waiting, so other threads may use it while we wait, this is also needed to avoid potential deadlocks
        _threads->wait(T, Build);
         goto again;
      }
   }
}
/******************************************************************************/
void VolumetricCloud::draw(Flt size, C VecD &pos)
{
   if(_cur.density>0 || _building)
   {
      OBox obox;
      obox.matrix.x.set(0, -1, 0); // rotated XY
      obox.matrix.y.set(1,  0, 0); // rotated XY
      obox.matrix.z.set(0,  0, 1);
      obox.matrix.pos=pos;

      size/=_image.size3().max();

      Flt l=_image.w()*size; obox.box.setX(-l, l);
          l=_image.h()*size; obox.box.setY(-l, l);
          l=_image.d()*size; obox.box.setZ(-l, l);

      if(Frustum(obox))
      {
         checkBuild();
        _image.drawVolume(WHITE, TRANSPARENT, obox, 1, 1, 2, 1024);
      }
   }
}
/******************************************************************************/
// VOLUMETRIC CLOUDS
/******************************************************************************/
VolumetricClouds::VolumetricClouds()
{
   draw_in_mirror=false;
   res_h=1080/2;

   size     =100;
   curve    =0.05f;
   tex_scale=0.5f;
   shadow   =0.35f;
   color    =1;
   pos.zero();
}
/******************************************************************************/
Bool VolumetricClouds::drawable()C {return (cloud._cur.density>0 || cloud._building) && (draw_in_mirror || !Renderer.mirror());} // check if already has density, or is building something, so we can call 'checkBuild' later that will perform updates
/******************************************************************************/
#pragma pack(push, 4)
struct GpuCloud
{
   Flt  AC4_bottom,
        AC4_top   ,
        A2_inv    ,
        max_steps ;
   Vec2 pos       ;
   Vec  pixels    ;
};
struct GpuCloudMap
{
   Flt  curve, curve2, height, thickness, tex_scale, steps, shadow;
   Vec2 pos;
   Vec  cam;
};
#pragma pack(pop)
/******************************************************************************
   clouds = -Cloud.curve*x*x + Cloud.height
   ray    = m*x = dir.y/Length(dir.xz)*x
      m*x = -Cloud.curve*x*x + Cloud.height
      Cloud.curve*x*x + m*x - Cloud.height = 0
    A=Cloud.curve, B=m, C=-Cloud.height
       Delta=B*B - 4*A*C = m*m + 4*Cloud.curve*Cloud.height
   SqrtDelta=Sqrt(m*m + 4*Cloud.curve*Cloud.height)
          x0=(-m-SqrtDelta)/(2*Cloud.curve)
          x1=(-m+SqrtDelta)/(2*Cloud.curve)

          x1=(SqrtDelta-m)/(2*Cloud.curve)
/******************************************************************************/
void VolumetricClouds::draw()
{
   if(drawable())
   {
      VolCloud.load();

      VecI2 res;
      res.y=Max(1, Min(Renderer.fxH(), res_h)); // do Min first and Max last
      res.x=Max(1, Round(res.y*D._unscaled_size.div())); // calculate proportionally to 'res.y' and current mode aspect (do not use 'D.aspectRatio' because that's the entire monitor screen aspect, and not application window), all of this is needed because we need to have square pixels for motion blur render targets, however the main application resolution may not have square pixels
      ImageRTPtr dest(ImageRTDesc(res.x, res.y, IMAGERT_TWO)); // here Red is for brightness, Green is used for density/opacity

      Renderer.set(dest(), null, false); D.alpha(ALPHA_NONE);

      GpuCloud c;
      Flt curve =Max(1.0f/512, T.curve)/size, // use Min because smaller values caused artifacts when looking up (at the top point of the cloud dome)
          height=Max(1, size+pos.y-ActiveCam.matrix.pos.y); // shader currently doesn't support being inside/above the clouds
      c.AC4_bottom=4*curve*(height                                                   ); // bottom cloud layer (under clouds)
      c.AC4_top   =4*curve*(height+size*Flt(cloud.height())/(cloud.width()*tex_scale)); // top    cloud layer (above clouds)
      c.A2_inv    =tex_scale*0.5f/(curve*size); // 0.5f/curve; // 1/(2*curve);
      Flt s=tex_scale/size; c.pos.set(ActiveCam.matrix.pos.x*s-pos.x, ActiveCam.matrix.pos.z*s-pos.z);
      c.max_steps=cloud.height()*2;
      c.pixels   =cloud._image.size3();
      VolCloud.h_Cloud->set(c);

      cloud.checkBuild(); // check if there are any finished image builds
      Sh.h_ImageVol[0]->set(cloud._image); Sh.h_ImageVol[0]->_sampler=&SamplerLinearCWW;
                                           Sh.h_ImageVol[1]->_sampler=&SamplerLinearWrap;

      Rect ext_rect, *rect=null; // set rect, after setting render target
      if(!D._view_main.full){ext_rect=D.viewRect(); rect=&ext_rect.extend(Renderer.pixelToScreenSize(1));} // when not rendering entire viewport, then extend the rectangle, add +1 because of texture filtering, have to use 'Renderer.pixelToScreenSize' and not 'D.pixelToScreenSize'

      VolCloud.h_Clouds->draw(null, rect);
      Sh.h_ImageVol[0]->_sampler=null;
      Sh.h_ImageVol[1]->_sampler=null;

      Renderer.set(Renderer._col(), null, true);
      D.alpha(ALPHA_BLEND_DEC);

      Flt to=D.viewRange(), from=Min(to*Sky.frac(), to-0.01f);
      Vec2 mul_add; mul_add.x=1/(to-from); mul_add.y=-from*mul_add.x;
      Sh.h_SkyFracMulAdd->set(mul_add);

      Sh.h_Color[0]->set(color);
      VolCloud.h_CloudsDraw->draw(dest());
   }
}
void VolumetricClouds::shadowMap()
{
   if(drawable() && Renderer._cld_map.is())
   {
      VolCloud.load();

      GpuCloudMap c;
      c.curve =Max(1.0f/512, T.curve)/size; // use Min because smaller values caused artifacts when looking up (at the top point of the cloud dome)
      c.curve2=c.curve*2;
      c.height=Max(1, size+pos.y-ActiveCam.matrix.pos.y); // shader currently doesn't support being inside/above the clouds
      c.thickness=size*Flt(cloud.height())/(cloud.width()*tex_scale);
      c.tex_scale=tex_scale/size;
      c.steps =cloud.height();
      c.shadow=shadow;
      Flt s=tex_scale/size; c.pos.set(ActiveCam.matrix.pos.x*s-pos.x, ActiveCam.matrix.pos.z*s-pos.z);
      c.cam   =CamMatrix.pos-ActiveCam.matrix.pos; // calculate current camera (light) position relative to the main camera

      VolCloud.h_CloudMap->set(c);

      cloud.checkBuild(); // check if there are any finished image builds
      Sh.h_ImageVol[0]->set(cloud._image);
      Sh.h_ImageVol[0]->_sampler=&SamplerLinearCWW; VolCloud.h_CloudsMap->draw();
      Sh.h_ImageVol[0]->_sampler=null;
   }
}
/******************************************************************************/
void AllClouds::drawAll()
{
   if(draw)
   {
      if(Renderer.canReadDepth())
      {
         volumetric.draw();
      }

      layered.draw();

      if(Renderer.canReadDepth())
      {
         Sky.setFracMulAdd();

         Renderer.set(Renderer._col(), Renderer._sky_coverage(), null, null, Renderer._ds(), true, WANT_DEPTH_READ); Renderer.setDSLookup(); // we may use soft cloud, 'setDSLookup' after 'set'
         D.alpha     (ALPHA_BLEND_DEC);
         D.depthWrite(false); REPS(Renderer._eye, Renderer._eye_num){Renderer.setEyeViewport(); Renderer.mode(RM_CLOUD); Renderer._render();}
         D.depthWrite(true );
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
