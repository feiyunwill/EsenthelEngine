/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
SkyClass Sky;
/******************************************************************************/
SkyClass::SkyClass()
{
   frac                       (0.8f); // !! if changing default value, then also change in 'Environment.Sky' !!
   atmosphericDensityExponent (1);
   atmosphericHorizonExponent (3.5f); // !! if changing default value, then also change in 'Environment.Sky' !!
   atmosphericColor           (Vec4(0.32f, 0.46f, 0.58f, 1.0f), Vec4(0.16f, 0.36f, 0.54f, 1.0f)); // !! if changing default value, then also change in 'Environment.Sky' !!
   atmosphericStarsOrientation(MatrixIdentity.orn());
#if MOBILE
   atmosphericPrecision       (false);
#else
   atmosphericPrecision       (true);
#endif
   skyboxBlend                (0.5f);
}
SkyClass& SkyClass::del()
{
  _mshr         .del  ();
   REPAO(_image).clear();
  _stars        .clear();
   return T;
}
SkyClass& SkyClass::create()
{
   Sh.h_SkyHorCol  ->set(_hor_col  );
   Sh.h_SkySkyCol  ->set(_sky_col  );
   Sh.h_SkyBoxBlend->set(_box_blend);
   Sh.h_SkyStarOrn ->set(_stars_m  );
   Flt temp=_hor_exp; _hor_exp=-1; atmosphericHorizonExponent(temp); // set -1 to force reset
       temp=_dns_exp; _dns_exp=-1; atmosphericDensityExponent(temp); // set -1 to force reset

   MeshBase mshb;
   mshb.createIco(Ball(1), 0, 3); // 3 give 'dist'=0.982246876
  _mshr.create(mshb.reverse());
   #define SKY_MESH_MIN_DIST 0.98f // it's good to make it a bit smaller than 'dist' to have some epsilon for precision issues, this is the closest point on the mesh to the Vec(0,0,0), it's not equal to radius=1, because the mesh is composed out of triangles, and the triangle surfaces are closer
#if DEBUG && 0 // calculate actual distance
   Flt dist=1; C Vec *pos=mshb.vtx.pos();
   REPA(mshb.tri ){C VecI  &t=mshb.tri .ind(i); MIN(dist, Dist(VecZero, Tri (pos[t.x], pos[t.y], pos[t.z])));}
   REPA(mshb.quad){C VecI4 &q=mshb.quad.ind(i); MIN(dist, Dist(VecZero, Quad(pos[q.x], pos[q.y], pos[q.z], pos[q.w])));}
#endif
   return T;
}
/******************************************************************************/
Bool SkyClass::wantDepth()C {return frac()<1;}
/******************************************************************************/
SkyClass& SkyClass::clear()
{
  _is=false;
   return T;
}
SkyClass& SkyClass::atmospheric()
{
   T._is      =true;
   T._image[0]=null;
   T._image[1]=null;
   return T;
}
SkyClass& SkyClass::skybox(C ImagePtr &image)
{
   T._is      =(image!=null);
   T._image[0]=image;
   T._image[1]=null ;
   return T;
}
SkyClass& SkyClass::skybox(C ImagePtr &a, C ImagePtr &b)
{
   T._is=(a || b);
   if(a && b){T._image[0]=a   ; T._image[1]=b   ;}else
   if(a     ){T._image[0]=a   ; T._image[1]=null;}else
   if(     b){T._image[0]=b   ; T._image[1]=null;}else
             {T._image[0]=null; T._image[1]=null;}
   return T;
}
/******************************************************************************/
SkyClass& SkyClass::frac                       (  Flt       frac     ) {SAT(frac  );                                                                 T._frac       =frac             ; return T;}
SkyClass& SkyClass::atmosphericHorizonExponent (  Flt       exp      ) {MAX(exp, 0);            if(exp      !=T._hor_exp                           ){T._hor_exp    =exp              ; if(Sh.h_SkyHorExp  )Sh.h_SkyHorExp  ->set(Max(T._hor_exp, EPS_GPU));} return T;} // avoid zero in case "Pow(1-Sat(inTex.y), SkyHorExp)" in shader would cause NaN or slow-downs
SkyClass& SkyClass::atmosphericHorizonColor    (C Vec4     &color    ) {Flt alpha=Sat(color.w); if(color.xyz!=T._hor_col.xyz || alpha!=T._hor_col.w){T._hor_col.set(color.xyz, alpha); if(Sh.h_SkyHorCol  )Sh.h_SkyHorCol  ->set(    T._hor_col          );} return T;} // alpha must be saturated
SkyClass& SkyClass::atmosphericSkyColor        (C Vec4     &color    ) {Flt alpha=Sat(color.w); if(color.xyz!=T._sky_col.xyz || alpha!=T._sky_col.w){T._sky_col.set(color.xyz, alpha); if(Sh.h_SkySkyCol  )Sh.h_SkySkyCol  ->set(    T._sky_col          );} return T;} // alpha must be saturated
SkyClass& SkyClass::skyboxBlend                (  Flt       blend    ) {SAT(blend );            if(blend    !=T._box_blend                         ){T._box_blend  =blend            ; if(Sh.h_SkyBoxBlend)Sh.h_SkyBoxBlend->set(    T._box_blend        );} return T;}
SkyClass& SkyClass::atmosphericStars           (C ImagePtr &cube     ) {                                                                             T._stars      =cube             ; return T;}
SkyClass& SkyClass::atmosphericStarsOrientation(C Matrix3  &orn      ) {                                                                            {T._stars_m    =orn              ; if(Sh.h_SkyStarOrn )Sh.h_SkyStarOrn ->set(    T._stars_m          );} return T;}
SkyClass& SkyClass::atmosphericPrecision       (  Bool      per_pixel) {                                                                             T._precision  =per_pixel        ; return T;}
SkyClass& SkyClass::atmosphericDensityExponent (  Flt       exp      )
{
   SAT(exp); if(exp!=T._dns_exp)
   {
      T._dns_exp=exp;
      if(Sh.h_SkyDnsExp)
      {
         /* shader uses the formula based on "Flt AccumulatedDensity(Flt density, Flt range) {return 1-Pow(1-density, range);}"
            "1-Pow(SkyDnsExp, alpha)" but that gives the range 0..(1-SkyDnsExp), however we need it normalized, so:
            (1-Pow(SkyDnsExp, alpha)) / (1-SkyDnsExp) gives the range 0..1
              -Pow(SkyDnsExp, alpha) / (1-SkyDnsExp) + 1/(1-SkyDnsExp)
               Pow(SkyDnsExp, alpha) * -(1/(1-SkyDnsExp)) + 1/(1-SkyDnsExp)
               Pow(SkyDnsExp, alpha) *   mul              + add
         */
         Flt v=1-exp; if(v)v=1/v;
         Sh.h_SkyDnsExp   ->set(Max(T._dns_exp, EPS_GPU)); // avoid zero in case "Pow(0, alpha)" in shader would cause NaN or slow-downs
         Sh.h_SkyDnsMulAdd->set(Vec2(-v, v));
      }
   }
   return T;
}
/******************************************************************************/
void SkyClass::setFracMulAdd()
{
   // !! in this method we use 'SkyFracMulAdd' as Object Opacity, and not Sky Opacity, so we use "1-sky_opacity" (reversed compared to drawing the sky) !!
   if(isActual())
   {
      Flt  from, to;
      Bool can_read_depth=Renderer.safeCanReadDepth(); // use 'safe' version because 'Renderer._ds' can be null here (for example when using RS_REFLECTION)
      if( !can_read_depth && Renderer.type()==RT_SIMPLE && !Renderer.simplePrecision()) // if we cannot apply sky frac, but we are in simple vertex fog mode, then use it instead
      {
         from=D.viewRange()*Renderer.simpleVertexFogStart();
         to  =D.viewRange()*Renderer.simpleVertexFogEnd  ();
      }else
      {
         from=(can_read_depth ? D.viewRange()*frac() : D.viewRange()); // we're using fraction only if we have depth access
         to  =D.viewRange();
      }
      MIN(from, to-EPS_SKY_MIN_LERP_DIST); // make sure there's some distance between positions to avoid floating point issues, move 'from' instead of 'to' to make sure we always have zero opacity at the end

      //Flt obj_opacity=Length(O.pos)*SkyFracMulAdd.x+SkyFracMulAdd.y;
      //              1=       from  *SkyFracMulAdd.x+SkyFracMulAdd.y;
      //              0=       to    *SkyFracMulAdd.x+SkyFracMulAdd.y;
      Vec2 mul_add; mul_add.x=1/(from-to); mul_add.y=-to*mul_add.x;
      Sh.h_SkyFracMulAdd->set(mul_add);
   }else
   {
      Sh.h_SkyFracMulAdd->set(Vec2(0, 1));
   }
}
/******************************************************************************/
INLINE Shader* SkyTF(Int textures,                                    Bool cloud, Bool dither) {Shader* &s=Sh.h_SkyTF[textures-1][cloud][dither]                       ; if(SLOW_SHADER_LOAD && !s)s=Sh.getSkyTF(textures, cloud, dither);                          return s;}
INLINE Shader* SkyT (Int textures,                              Int multi_sample, Bool dither) {Shader* &s=Sh.h_SkyT [textures-1][multi_sample][dither]                ; if(SLOW_SHADER_LOAD && !s)s=Sh.getSkyT (textures, multi_sample, dither);                   return s;}
INLINE Shader* SkyAF(Bool per_vertex, Bool stars,                     Bool cloud, Bool dither) {Shader* &s=Sh.h_SkyAF[per_vertex][stars][cloud][dither]                ; if(SLOW_SHADER_LOAD && !s)s=Sh.getSkyAF(per_vertex, stars, cloud, dither);                 return s;}
INLINE Shader* SkyA (Bool per_vertex, Bool stars, Bool density, Int multi_sample, Bool dither) {Shader* &s=Sh.h_SkyA [per_vertex][stars][density][multi_sample][dither]; if(SLOW_SHADER_LOAD && !s)s=Sh.getSkyA (per_vertex, stars, density, multi_sample, dither); return s;}

void SkyClass::draw()
{
   if(isActual())
   {
      Shader *shader, *shader_multi=null;
      Bool    blend,
              density=(atmosphericDensityExponent()<1-EPS_GPU),
              dither =(D.dither() && !Renderer._col->highPrecision()),
              vertex = !_precision,
              stars  =((_stars   !=null) && (_hor_col.w<1-EPS_COL || _sky_col.w<1-EPS_COL)),
              cloud  =(Clouds.draw && Clouds.layered.merge_with_sky && Clouds.layered.layers() && Clouds.layered.layer[0].image && Clouds.layered.layer[0].color.a && (Clouds.layered.draw_in_mirror || !Renderer.mirror()));
      Int     tex    =((_image[0]!=null) + (_image[1]!=null)),
              multi  =(Renderer._col->multiSample() ? ((Renderer._cur_type==RT_DEFERRED) ? 1 : 2) : 0);
      Flt     from   =(Renderer.canReadDepth() ? D.viewRange()*frac() : D.viewRange()), // we're using fraction only if we have depth access
              to     =D.viewRange();
      blend=(from<to-EPS_SKY_MIN_LERP_DIST); // set blend mode if 'from' is far from 'to', and yes use < and not <= in case of precision issues for big values

      if(tex)
      {
         if(blend){shader=SkyT (tex,     0, dither); if(multi)shader_multi=SkyT(tex, multi, dither);}
         else      shader=SkyTF(tex, cloud, dither);
      }else
      {
         if(blend){shader=SkyA (vertex, stars, density, 0, dither); if(multi)shader_multi=SkyA(vertex, stars, density, multi, dither);}
         else      shader=SkyAF(vertex, stars, cloud, dither);
      }

      // set shader parameters
      if(tex)
      {
         Sh.h_ImageRfl[0]->set(_image[0]());
         Sh.h_ImageCub   ->set(_image[1]());
      }else
      if(stars)
      {
         Sh.h_ImageRfl[0]->set(_stars());
      }

      if(AstrosDraw && Sun.is())
      {
         Sh.h_SkySunHighlight->set(Vec2(Sun.highlight_front, Sun.highlight_back));
         Sh.h_SkySunPos      ->set(Sun.pos);
      }else
      {
         Sh.h_SkySunHighlight->set(Vec2(0));
      }

      if(cloud)Clouds.layered.commit();

      Bool ds=true;
      Flt  sky_ball_mesh_size; if(blend)
      {
         //Flt sky_opacity=Length(O.pos)*SkyFracMulAdd.x+SkyFracMulAdd.y;
         //              0=       from  *SkyFracMulAdd.x+SkyFracMulAdd.y;
         //              1=       to    *SkyFracMulAdd.x+SkyFracMulAdd.y;
         Vec2 mul_add; mul_add.x=1/(to-from); mul_add.y=-from*mul_add.x;
         Sh.h_SkyFracMulAdd->set(mul_add);

         sky_ball_mesh_size=from;
       //sky_ball_mesh_size-=DepthError(D.viewFrom(), D.viewRange(), sky_ball_mesh_size, FovPerspective(D.viewFovMode()), ImageTI[Renderer._ds->hwType()].d); // draw smaller by DepthError to avoid depth precision issues
         if(sky_ball_mesh_size*SKY_MESH_MIN_DIST<=FrustumMain.view_quad_max_dist){sky_ball_mesh_size=to; ds=false;} // if the closest point on the mesh surface is in touch with the view quad, it means that the ball would not render fully, we have to render it with full size and with depth test disabled
      }else sky_ball_mesh_size=to;
   #if !REVERSE_DEPTH // for low precision depth we need to make sure that sky ball mesh is slightly smaller than view range, to avoid mesh clipping, this was observed on OpenGL with viewFrom=0.05, viewRange=1024, Cam.yaw=0, Cam.pitch=PI_2
      MIN(sky_ball_mesh_size, to*EPS_SKY_MIN_VIEW_RANGE); // alternatively we could try using D3DRS_CLIPPING, DepthClipEnable, GL_DEPTH_CLAMP
   #endif
      Renderer.set(Renderer._col(), Renderer._ds(), true, blend ? NEED_DEPTH_READ : NO_DEPTH_READ); // specify correct mode because without it the sky may cover everything completely
      D.alpha     (blend ? ALPHA_BLEND_DEC : ALPHA_NONE);
      D.depthWrite(false);
      D.depthFunc (FUNC_LESS_EQUAL); // to make sure we draw at the end of viewRange
    //D.cull      (true ); ignore changing culling, because we're inside the sky ball, so we will always see its faces, we could potentially set false (to ignore overhead on the GPU for cull testing if any) however we choose to just ignore it to reduce GPU state changes on the CPU which are probably more costly
      D.sampler3D (     ); // set in case of drawing clouds
      if(shader_multi)D.stencil(STENCIL_MSAA_TEST);
     _mshr.set();
      SetOneMatrix(MatrixM(sky_ball_mesh_size, CamMatrix.pos)); // normally we have to set matrixes after 'setEyeViewport', however since matrixes are always relative to the camera, and here we set exactly at the camera position, so the matrix will be the same for both eyes
      REPS(Renderer._eye, Renderer._eye_num)
      {
         Renderer.setEyeViewport();
         if(shader_multi){D.depth((multi==1) ? false : ds); D.stencilRef(STENCIL_REF_MSAA); shader_multi->begin(); _mshr.drawFull(); ShaderEnd(); D.stencilRef(0);} // MS edges for deferred must not use depth testing
                          D.depth(                     ds);                                 shader      ->begin(); _mshr.drawFull(); ShaderEnd();
      }
      D.sampler2D  (    );
      D.depthWrite (true);
      D.depthFunc  (FUNC_LESS);
      D.stencil    (STENCIL_NONE);
      MaterialClear(    );
   #if DX11
      D.flush(); // FIXME this is a workaround for Nvidia GeForce bug https://devtalk.nvidia.com/default/topic/1038873/directx-and-direct-compute/geforce-1050-ti-bug/ remove this line once the bug is fixed
   #endif
   }
}
/******************************************************************************/
}
/******************************************************************************/
