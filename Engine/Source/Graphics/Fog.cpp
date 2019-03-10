/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define MAX_DENSITY 0.98f // was "1-EPS_GPU" but big values generate unnatural focus points in the FogBall Center
/******************************************************************************/
FogClass Fog;
/******************************************************************************/
FogClass::FogClass()
{
   draw      =false;
   affect_sky=false;
   density   =0.02f;
   color     =0.5f ;
}
/******************************************************************************/
void FogClass::Draw(Bool after_sky)
{
   if(after_sky==affect_sky)
   {
      if(draw && Renderer.canReadDepth() && Sh.h_Fog[0])
      {
         Int multi=(Renderer._ds->multiSample() ? (Renderer._cur_type==RT_DEFERRED ? 1 : 2) : 0);
         Renderer.set(Renderer._col(), Renderer._ds(), true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
         Renderer.setMainViewport();
         Sh.h_FogColor_Density->set(Vec4(color, Mid(density, 0.0f, MAX_DENSITY))); // avoid 1 in case "Pow(1-density, ..)" in shader would cause NaN or slow-downs
         D .alpha(ALPHA_BLEND_DEC);
         if(multi && Sh.h_Fog[multi])
         {
            if(Renderer.hasStencilAttached())
            {
               D.stencil(STENCIL_MSAA_TEST);
               REPS(Renderer._eye, Renderer._eye_num)
               {
                  Rect *rect=Renderer.setEyeParams();
                  D.stencilRef(STENCIL_REF_MSAA); /*if(!affect_sky && multi!=1)D.depth2DOn();*/ Sh.h_Fog[multi]->draw(null, rect);                D.depth2DOff(); // never enable 'depth2DOn' for deferred renderer as it's needed, it was tested that enabling this for forward renderer actually makes it slower (19 vs 25 fps on 200 draws)
                  D.stencilRef(               0);   if(!affect_sky            )D.depth2DOn();   Sh.h_Fog[0    ]->draw(null, rect); if(!affect_sky)D.depth2DOff();
               }
               D.stencil(STENCIL_NONE);
            }else // if don't have stencil available then have to write all as multi-sampled
            {
               REPS(Renderer._eye, Renderer._eye_num)Sh.h_Fog[multi]->draw(null, Renderer.setEyeParams());
            }
            return;
         }else
         if(Sh.h_Fog[0])
         {
            if(!affect_sky)D.depth2DOn (); REPS(Renderer._eye, Renderer._eye_num)Sh.h_Fog[0]->draw(null, Renderer.setEyeParams());
            if(!affect_sky)D.depth2DOff();
            return;
         }
      }
      // clear because BlendLight shaders always use Fog
      Sh.h_FogColor_Density->set(Vec4Zero);
   }
}
/******************************************************************************/
void FogDraw(C OBox &obox, Flt density, C Vec &color)
{
   if(Frustum(obox) && Renderer.canReadDepth())
   {
      Sh.loadFogBoxShaders();
      Renderer.needDepthRead();
      D .alpha(ALPHA_BLEND_DEC);
      Sh.h_LocalFogColor_Density->set(Vec4(color, Mid(density, 0.0f, MAX_DENSITY))); // avoid 1 in case "Pow(1-density, ..)" in shader would cause NaN or slow-downs

      Vec delta=CamMatrix.pos-obox.center(), inside;
      inside.x=Dot(delta, obox.matrix.x);
      inside.y=Dot(delta, obox.matrix.y);
      inside.z=Dot(delta, obox.matrix.z);

      Vec size=obox.box.size()*0.5f;

      Matrix temp;
      temp.pos=obox.center();
      temp.x  =obox.matrix.x*size.x;
      temp.y  =obox.matrix.y*size.y;
      temp.z  =obox.matrix.z*size.z;
      SetOneMatrix(temp);

      Flt e=Frustum.view_quad_max_dist;
      if(inside.x>=-size.x-e && inside.x<=size.x+e
      && inside.y>=-size.y-e && inside.y<=size.y+e
      && inside.z>=-size.z-e && inside.z<=size.z+e)
      {
         Sh.h_LocalFogInside->set(inside);
            
         if(inside.x>=-size.x+e && inside.x<=size.x-e
         && inside.y>=-size.y+e && inside.y<=size.y-e
         && inside.z>=-size.z+e && inside.z<=size.z-e)Sh.h_FogBox1->draw(Renderer._ds_1s);
         else                                         Sh.h_FogBox0->draw(Renderer._ds_1s);
      }else
      {
         D .depth     (true );
         D .depthWrite(false);
         D .cull      (true );
         Sh.h_FogBox->begin(); MshrBox.set().drawFull();
         ShaderEnd();
      }
   }
}
/******************************************************************************/
void FogDraw(C Ball &ball, Flt density, C Vec &color)
{
   if(Frustum(ball) && Renderer.canReadDepth())
   {
      Sh.loadFogBallShaders();
      Renderer.needDepthRead();
      D .alpha(ALPHA_BLEND_DEC);
      Sh.h_LocalFogColor_Density->set(Vec4(color, Mid(density, 0.0f, MAX_DENSITY))); // avoid 1 in case "Pow(1-density, ..)" in shader would cause NaN or slow-downs

      Vec inside=CamMatrix.pos-ball.pos;

      Matrix temp;
      temp.pos=ball.pos;
      temp.x  .set(ball.r, 0, 0);
      temp.y  .set(0, ball.r, 0);
      temp.z  .set(0, 0, ball.r);
      SetOneMatrix(temp);

      Flt e=Frustum.view_quad_max_dist,
          i=inside.length();
      if( i<=ball.r+e)
      {
         Sh.h_LocalFogInside->set(inside);

         if(i<=ball.r-e)Sh.h_FogBall1->draw(Renderer._ds_1s);
         else           Sh.h_FogBall0->draw(Renderer._ds_1s);
      }else
      {
         D .depth     (true );
         D .depthWrite(false);
         D .cull      (true );
         Sh.h_FogBall->begin(); MshrBall.set().drawFull();
         ShaderEnd();
      }
   }
}
/******************************************************************************/
void HeightFogDraw(C OBox &obox, Flt density, C Vec &color)
{
   if(Frustum(obox) && Renderer.canReadDepth())
   {
      Sh.loadFogHgtShaders();
      Renderer.needDepthRead();
      D .alpha(ALPHA_BLEND_DEC);
      Sh.h_LocalFogColor_Density->set(Vec4(color, Mid(density, 0.0f, MAX_DENSITY))); // avoid 1 in case "Pow(1-density, ..)" in shader would cause NaN or slow-downs

      Vec delta=CamMatrix.pos-obox.center(), inside;
      inside.x=Dot(delta, obox.matrix.x);
      inside.y=Dot(delta, obox.matrix.y);
      inside.z=Dot(delta, obox.matrix.z);

      Vec size=obox.box.size()*0.5f;

      Matrix temp;
      temp.pos=obox.center();
      temp.x  =obox.matrix.x*size.x;
      temp.y  =obox.matrix.y*size.y;
      temp.z  =obox.matrix.z*size.z;
      SetOneMatrix(temp);

      Flt e=Frustum.view_quad_max_dist;
      if(inside.x>=-size.x-e && inside.x<=size.x+e
      && inside.y>=-size.y-e && inside.y<=size.y+e
      && inside.z>=-size.z-e && inside.z<=size.z+e)
      {
         Sh.h_LocalFogInside->set(inside);
            
         if(inside.x>=-size.x+e && inside.x<=size.x-e
         && inside.y>=-size.y+e && inside.y<=size.y-e
         && inside.z>=-size.z+e && inside.z<=size.z-e)Sh.h_FogHgt1->draw(Renderer._ds_1s);
         else                                         Sh.h_FogHgt0->draw(Renderer._ds_1s);
      }else
      {
         D .depth     (true );
         D .depthWrite(false);
         D .cull      (true );
         Sh.h_FogHgt->begin(); MshrBox.set().drawFull();
         ShaderEnd();
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
