/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   There are 3 types of shaders:

      2D Shaders - lines, images, text, rectangles, circles, ...
         vertex position affected by Coords (in DX9 Coords are adjusted by pixel center)

      3D Shaders - 3d models
         vertex position affected by 'ProjMatrix' (in DX9 'ProjMatrix' is adjusted by pixel center)

      PostProcess Shaders - bloom, ambient occlusion, motion blur, depth of field, sun rays, ..
         vertex position calculated completely on the CPU (in DX9 adjusted by pixel center in the shader)

/******************************************************************************/
// SETTINGS
/******************************************************************************/
void Display::ViewportSettings::get()
{
   from    =D.viewFrom   ();
   range   =D.viewRange  ();
   fov     =D.viewFov    ();
   fov_mode=D.viewFovMode();
   rect    =D.viewRect   ();
}
void Display::ViewportSettings::set()C
{
   D.view(rect, from, range, fov, fov_mode);
}
/******************************************************************************/
// VIEWPORT
/******************************************************************************/
Display::Viewport& Display::Viewport::set3DFrom(C Viewport &src)
{
   from    =src.from;
   range   =src.range;
   fov     =src.fov;
   fov_sin =src.fov_sin;
   fov_cos =src.fov_cos;
   fov_tan =src.fov_tan;
   fov_mode=src.fov_mode;
   return T;
}
/******************************************************************************/
Display::Viewport& Display::Viewport::setRect(C RectI &recti)
{
   T.recti=recti;

   Clamp(T.recti.min.x, 0, Renderer.resW()-1); Clamp(T.recti.max.x, T.recti.min.x, Renderer.resW());
   Clamp(T.recti.min.y, 0, Renderer.resH()-1); Clamp(T.recti.max.y, T.recti.min.y, Renderer.resH());

   T.full=(T.recti.min.x==0 && T.recti.min.y==0 && T.recti.max.x==Renderer.resW() && T.recti.max.y==Renderer.resH());

   return T;
}
/******************************************************************************/
Display::Viewport& Display::Viewport::setFrom (Flt from ) {T.from =from ; return T;}
Display::Viewport& Display::Viewport::setRange(Flt range) {T.range=range; return T;}
/******************************************************************************/
Display::Viewport& Display::Viewport::setFov()
{
   Flt aspect=(D._view_square_pixel ? 1 : D._pixel_aspect)*recti.aspect();
   if(FovPerspective(fov_mode))
   {
      switch(fov_mode)
      {
         case FOV_X:
            Clamp(fov.x, EPS, PI-EPS);
            fov.y=2*Atan(Tan(fov.x*0.5f)/aspect);
         break;

         case FOV_Y:
            Clamp(fov.y, EPS, PI-EPS);
            fov.x=2*Atan(Tan(fov.y*0.5f)*aspect);
         break;

         case FOV_XY:
            Clamp(fov.x, EPS, PI-EPS);
            Clamp(fov.y, EPS, PI-EPS);
         break;
      }
      CosSin(fov_cos.x, fov_sin.x, fov.x*0.5f);
      CosSin(fov_cos.y, fov_sin.y, fov.y*0.5f);
      fov_tan=fov_sin/fov_cos;
   }else
   {
      switch(fov_mode)
      {
       /*case FOV_ORTHO:
            fov.x=fov.x;
            fov.y=fov.y;
         break;*/

         case FOV_ORTHO_X:
          //fov.x=fov.x;
            fov.y=fov.x/aspect;
         break;

         case FOV_ORTHO_Y:
          //fov.y=fov.y;
            fov.x=fov.y*aspect;
         break;
      }
      fov_tan=fov;
   }
   return T;
}
Display::Viewport& Display::Viewport::setFov(C Vec2 &fov, FOV_MODE fov_mode)
{
   T.fov     =fov;
   T.fov_mode=fov_mode;
   return setFov();
}
Display::Viewport& Display::Viewport::set(C RectI &recti, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode)
{
   return setRect(recti).setFrom(from).setRange(range).setFov(fov, fov_mode);
}
/******************************************************************************
inline Vec2 FracToPosXY(Vec2 screen) // return view space xy position at z=1
{
   (((screen-Viewport.min)/Viewport.size)-0.5)/Vec2(0.5,-0.5)*z*Viewport.fov_tan;
   (screen/Viewport.size-(Viewport.min/Viewport.size+0.5))*z*(Viewport.fov_tan/Vec2(0.5,-0.5))
   ( screen * (Viewport.fov_tan/Vec2(0.5,-0.5)/Viewport.size) - ((Viewport.min/Viewport.size+0.5)*(Viewport.fov_tan/Vec2(0.5,-0.5))) ) * z
   return screen * (Viewport.fov_tan/Vec2(0.5,-0.5)) - (0.5*(Viewport.fov_tan/Vec2(0.5,-0.5)));
   return screen * Viewport.FracToPosXY.xy           + Viewport.FracToPosXY.zw                ;
}
inline Vec2 ScreenToPosXY(Vec2 screen) // return view space xy position at z=1
{
   return screen * (Viewport.fov_tan/Vec2(0.5,-0.5)/Viewport.size) - ((Viewport.min/Viewport.size+0.5)*(Viewport.fov_tan/Vec2(0.5,-0.5)));
   return screen * Viewport.ScreenToPosXY.xy                       + Viewport.ScreenToPosXY.zw                                           ;
}
inline Vec2 PosToScreen(Vec pos)
{
   return (pos.xy/pos.z) * ((Vec2(0.5,-0.5)/Viewport.fov_tan+0.5)*Viewport.size) + Viewport.min;
}
inline Vec2 PosToScreen(Vec4 pos)
{
   Vec2 screen=pos.xy/pos.w*Vec2(0.5,-0.5)+0.5; return screen*Viewport.size+(Viewport.min+0.5/RTSizeI);
   return (pos.xy/pos.w*Vec2(0.5,-0.5)+0.5)*Viewport.size+(Viewport.min+0.5/RTSizeI);
   return (pos.xy/pos.w) * (Vec2(0.5,-0.5)*Viewport.size) + (0.5*Viewport.size+Viewport.min+0.5/RTSizeI);
   return (pos.xy/pos.w) * Viewport.PosToScreen.xy        + Viewport.PosToScreen.zw                     ;
}
/******************************************************************************/
Display::Viewport& Display::Viewport::setViewport(Bool allow_proj_matrix_update)
{
#if DX9
   D.viewport(recti, allow_proj_matrix_update);
#elif DX11
   D.viewport(recti, allow_proj_matrix_update);
#elif GL
   if(D.mainFBO())
   {
      RectI r; r.setX(recti.min.x, recti.max.x); r.min.y=Renderer.resH()-recti.max.y; r.max.y=r.min.y+recti.h(); D.viewport(r, allow_proj_matrix_update);
   }else D.viewport(recti, allow_proj_matrix_update);
#endif
   return T;
}
/******************************************************************************/
#pragma pack(push, 4)
struct GpuViewport
{
   Flt  from, range;//, ortho;
   Vec2 center, size, size_fov_tan;
   Vec2 FracToPosXY[2], ScreenToPosXY[2], PosToScreen[2]; // helpers
};
#pragma pack(pop)
Display::Viewport& Display::Viewport::setShader(Flt *offset)
{
   // helpers
   Vec2 size =Renderer.res(),
        v_min=Vec2(recti.min)/size,
        v_ft =fov_tan/Vec2(0.5f, -0.5f);

   GpuViewport v;
   v.from        =from ;
   v.range       =range;
 //v.ortho       =FovOrthogonal(fov_mode);
   v.center      =recti.centerF()/size;
   v.size        =recti.size   ()/size;
   v.size_fov_tan=v.size/fov_tan;

   v.FracToPosXY[0]=      v_ft;
   v.FracToPosXY[1]=-0.5f*v_ft;

   v.ScreenToPosXY[0]=                     v_ft/v.size;
   v.ScreenToPosXY[1]=-(v_min/v.size+0.5f)*v_ft;

#if DX9 || DX11
   v.PosToScreen[0]=Vec2(0.5f, -0.5f)*v.size;
   v.PosToScreen[1]=            0.5f *v.size + v_min;
#elif GL
   v.PosToScreen[0]=Vec2(0.5f, 0.5f)*v.size;
   v.PosToScreen[1]=           0.5f *v.size + v_min;
#endif

   if(offset)
   {
      Flt o=*offset*v.ScreenToPosXY[0].x*0.25f;
      v.  FracToPosXY[1].x-=o;
      v.ScreenToPosXY[1].x-=o;
    //v.PosToScreen  [1].x this shouldn't be modified because we're referring to screen which is already offsetted (this was tested on rendering fur which uses 'PosToScreen')
   }

   Sh.h_Viewport->set(v);
   return T;
}
inline Dbl DelinearizeDepth(Dbl z, Dbl mp_z_z, Dbl mp_w_z, Bool perspective)
{
   return perspective ? (z*mp_z_z+mp_w_z)/z : z*mp_z_z+REVERSE_DEPTH;
}
inline Dbl LinearizeDepth(Dbl w, Dbl mp_z_z, Dbl mp_w_z, Bool perspective)
{
   return perspective ? mp_w_z/(w-mp_z_z) : (w-REVERSE_DEPTH)/mp_z_z;
}
Flt DepthError(Dbl from, Dbl range, Dbl z, Bool perspective, Int bits)
{
   Dbl v, mp_z_z, mp_w_z;
   if(perspective) // in perspective we have viewport depth ranges from "from .. range"
   {
      v=range;
   }else // in orthogonal we have viewport depth ranges from "0 .. range"
   {
      from=0;
      v   =1;
   }
   v/=range-from;
#if REVERSE_DEPTH
   if(perspective)
   {
      mp_z_z=-v+1;
      mp_w_z= v*from;
   }else
   {
      mp_z_z=-v;
      mp_w_z= 1;
   }
#else
   {
      mp_z_z=v;
      mp_w_z=v*-from;
   }
#endif
   Dbl w=DelinearizeDepth(z , mp_z_z, mp_w_z, perspective), w1=w+1.0/(1ull<<bits),
      z1=  LinearizeDepth(w1, mp_z_z, mp_w_z, perspective);
   return Abs(z1-z);
}
Display::Viewport& Display::Viewport::setProjMatrix(Bool set_frustum) // !! must be the same as "Flt DepthError" !!
{
   Dbl z, from, range=T.range; // use 'Dbl' to perform computations in best precision because we need 'ProjMatrix' to be as precise as possible
   if(FovPerspective(fov_mode)) // in perspective we have viewport depth ranges from "from .. range"
   {
      from=T.from;
      z   =range;
      ProjMatrix.x  .x=1/fov_tan.x;
      ProjMatrix.y  .y=1/fov_tan.y;
      ProjMatrix.z  .w=1;
      ProjMatrix.pos.w=0;
   }else // in orthogonal we have viewport depth ranges from "0 .. range"
   {
      from=0;
      z   =1;
      ProjMatrix.x  .x=1/fov.x;
      ProjMatrix.y  .y=1/fov.y;
      ProjMatrix.z  .w=0;
      ProjMatrix.pos.w=1;
   }
   z/=range-from;
#if REVERSE_DEPTH
   if(FovPerspective(fov_mode))
   {
      ProjMatrix.z  .z=-z+1;
      ProjMatrix.pos.z= z*from;
   }else
   {
      ProjMatrix.z  .z=-z;
      ProjMatrix.pos.z= 1;
   }
#else
   {
      ProjMatrix.z  .z=z;
      ProjMatrix.pos.z=z*-from;
   }
#endif
#if DEBUG && 0
   #pragma message("!! Warning: Use this only for debugging !!")
   Vec4 p; Dbl w;
   p=Vec4(0, 0,     from        , 1)*ProjMatrix; p.z/=p.w; w=DelinearizeDepth(    from        , ProjMatrix.z.z, ProjMatrix.pos.z, FovPerspective(fov_mode)); z=LinearizeDepth(w, ProjMatrix.z.z, ProjMatrix.pos.z, FovPerspective(fov_mode));
   p=Vec4(0, 0,           range , 1)*ProjMatrix; p.z/=p.w; w=DelinearizeDepth(          range , ProjMatrix.z.z, ProjMatrix.pos.z, FovPerspective(fov_mode)); z=LinearizeDepth(w, ProjMatrix.z.z, ProjMatrix.pos.z, FovPerspective(fov_mode));
   p=Vec4(0, 0, Avg(from, range), 1)*ProjMatrix; p.z/=p.w; w=DelinearizeDepth(Avg(from, range), ProjMatrix.z.z, ProjMatrix.pos.z, FovPerspective(fov_mode)); z=LinearizeDepth(w, ProjMatrix.z.z, ProjMatrix.pos.z, FovPerspective(fov_mode)); // should be 0.5f for orthogonal (but not perspective)
#endif

   SetProjMatrix();

   if(set_frustum)Frustum.set();

   return T;
}
/******************************************************************************/
}
/******************************************************************************/
